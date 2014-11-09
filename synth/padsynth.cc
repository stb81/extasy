/*   extasy - Extensible Tracker And Synthesizer
 *   Copyright (C) 2014 Stefan T. Boettner
 *
 *   extasy is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   extasy is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with extasy.  If not, see <http://www.gnu.org/licenses/>. */
 
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <fftw3.h>
#include "synthbits.h"
#include "module.h"
#include "sequencer.h"
#include "padsynth.h"

namespace Synth {
	
class PadSynth::Pad {
	short*	samples;
	int		mask;
	
public:
	Pad(const double*, int);
	Pad(const Pad&);
	~Pad();
	
	short operator[](int i) const
	{
		return samples[i&mask];
	}
	
	float operator()(float t) const
	{
		int t0=(int) floor(t);
	
		t-=t0;
	
		return ldexpf((*this)[t0]*(1-t) + (*this)[t0+1]*t, -15);
	}
};
	
class PadSynth::Tone : public ExcitationModelTone {
public:
	Tone(const PadSynth&, float, int);
	virtual ~Tone();
	
	virtual void synth(float**, int);

private:
	struct SmoothStep {
		float	t0;
		float	rate;
		float	a;
		float	b;
		
		float operator()()
		{
			t0+=rate;
			
			return (a*t0+b) * exp(-t0);
		}
		
		SmoothStep() {}
		SmoothStep(float lambda, float y0, float dy0)
		{
			t0=0;
			rate=lambda;
			a=dy0/lambda + y0;
			b=y0;
		}
	};
	
	const PadSynth&	pad;
	const PadSynth::Pad*	samples;
	
	float		stepsize;
	float		pos[2];
	float		energy;
	float		decay;
	
	BiQuad		highpass;
	BiQuad::Filter	hpfilter[2];
	
	BiQuad		allpass;
	BiQuad::Filter	apfilter[2];
	
	SmoothStep	smooth[2];
};


PadSynth::Pad::Pad(const double* in_samples, int length)
{
	samples=new short[length];
	mask=length-1;
	
	double maxval=0;
	for (int i=0;i<length;i++) {
		double v=fabs(in_samples[i]);
		if (v>maxval) maxval=v;
	}
	
	for (int i=0;i<length;i++)
		samples[i]=(short) (32767 * in_samples[i] / maxval);
}

PadSynth::Pad::Pad(const Pad& pad)
{
	mask=pad.mask >> 1;
	samples=new short[mask+1];
	
	for (int i=0;i<=mask;i++) {
		int v=(9*(pad[2*i]+pad[2*i+1]) - pad[2*i-1] - pad[2*i+2]) >> 4;
		if (v>32767) v=32767;
		if (v<-32768) v=-32768;
		samples[i]=v;
	}
}

PadSynth::Pad::~Pad()
{
	delete[] samples;
}

PadSynth::PadSynth(Mixer& m, float exponent):ExcitationModelInstrument(m), exponent(exponent)
{
	bandwidth=0.25f;
	
	band_coefficients=new float[256];
	
	length=262144;
	for (int i=0;i<4;i++) samples[i]=NULL;
	
	create_random_timbre();
}

PadSynth::~PadSynth()
{
	delete[] band_coefficients;
	
	for (int i=0;i<4;i++)
		delete samples[i];
}

void PadSynth::create_random_timbre()
{
	for (int i=1;i<=256;i++)
		band_coefficients[i-1]=-log(ldexpf((rand()&0xffff)+1, -16)) * (1+cosf(ldexpf(rand()&0xffff,-16)*M_PI)) / pow(1+i*i, exponent);
		
	build_pad();
}

void PadSynth::build_pad()
{
	mixer.kill_all_tones_with_instrument(this);
	
	double* tmp=new double[length];
	
	for (int i=0;i<length;i++)
		tmp[i]=0;
	
	for (int i=1;i<=256;i++) {
		double bw=bandwidth;
		
		int bands=(int) ceil(i * 3 / bw);
		
		for (int j=-bands;j<=bands;j++) {
			int freq=512*i + j;
			if (freq<1 || 2*freq>=length) continue;
			
			double amp=bw * j / i;
			amp=exp(-amp*amp) / i / sqrt(length) * band_coefficients[i-1] * sqrtf(-logf(ldexpf((rand()&0xffff)+1, -16)));

			double phase=ldexpf(rand()&0xffff, -15) * M_PI;
			tmp[freq]+=amp * cos(phase);
			tmp[length-freq]+=amp * sin(phase);
		}
	}
	
	fftw_plan ifft=fftw_plan_r2r_1d(length, tmp, tmp, FFTW_HC2R, FFTW_ESTIMATE);
	fftw_execute(ifft);
	fftw_destroy_plan(ifft);
	
	/*double max=0;
	for (int i=0;i<length;i++)
		if (fabs(tmp[i]) > max) max=fabs(tmp[i]);
	for (int i=0;i<length;i++)
		pad[i]=(short) (32767*tmp[i]/max);*/
		
	for (int i=0;i<4;i++) {
		delete samples[i];
		
		if (!i)
			samples[i]=new Pad(tmp, length);
		else
			samples[i]=new Pad(*samples[i-1]);
	}
	
	delete[] tmp;
}

Synth::Tone* PadSynth::play_note(int note, int volume)
{
	return new Tone(*this, note2freq(note), volume);
}

Instrument* PadSynth::create(Mixer& mixer)
{
	return new PadSynth(mixer, 0.5f);
}


PadSynth::Tone::Tone(const PadSynth& ps, float freq, int vol):ExcitationModelTone(ps, freq, vol),pad(ps)
{
	volume=vol;
	
	stepsize=freq * 512 / pad.mixer.get_samplerate();
	
	/*for (int i=0;;i++)
		if (i==3 || stepsize<=1.0f) {
			samples=pad.samples[i];
			break;
		}
		else
			stepsize*=0.5f;*/
	samples=pad.samples[0];
	
	for (int i=0;i<2;i++) {
		pos[i]=ldexpf(rand()&0xfffff, -2);
		smooth[i]=SmoothStep(M_PI*stepsize/256, (*samples)(pos[i]), ((*samples)(pos[i]+1)-(*samples)(pos[i]-1))/2*stepsize );
	}
	
	highpass=BiQuad::highpass(2*M_PI*freq/pad.mixer.get_samplerate(), 1.0f);
	hpfilter[0]=BiQuad::Filter(highpass);
	hpfilter[1]=BiQuad::Filter(highpass);
	
	allpass=BiQuad::lowpass(6*M_PI*freq/pad.mixer.get_samplerate(), 1.0f);
	apfilter[0]=BiQuad::Filter(allpass);
	apfilter[1]=BiQuad::Filter(allpass);
}

PadSynth::Tone::~Tone()
{
}

void PadSynth::Tone::synth(float** samples, int count)
{
	int i,j;

	float energy[count];
	compute_excitation(energy, count);
	
	for (j=0;j<count;j++)
		for (i=0;i<2;i++) {
			float w, v=((*this->samples)(pos[i]) - smooth[i]()) * energy[j];
			/*if (v<-0.5f) w=-0.25f;
			else if (v>0.5f) w=2*v-0.25f;
			else w=v*(v+1);*/
			//w=4.0f*v*(v+0.25f);
			//w=v*(v+1)*(v+1);
			w=apfilter[i](v);
			w=sin(8.0f * v + 0.5f) * 0.125f;
			
			samples[i][j]=lerp(v, hpfilter[i](w), modulation);
			
			pos[i]+=stepsize;
			
			if (pos[i]>=262144)
				pos[i]-=262144;
		}
}

}

