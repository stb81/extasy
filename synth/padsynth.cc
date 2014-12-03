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
#include "serialization.h"

#define self (*this)

namespace Synth {
	
class PadSynth::Pad {
	float*	samples;
	float*	dsamples;
	int		mask;
	
public:
	Pad(const double*, int, int);
	~Pad();
	
	float operator[](int i) const
	{
		return samples[i&mask];
	}
	
	float operator()(float t) const
	{
		int t0=(int) floor(t);
	
		return herp(self[t0], self[t0+1], dsamples[t0&mask], dsamples[(t0+1)&mask], t-t0);
	}
};
	
class PadSynth::Tone : public ExcitationModelTone {
public:
	Tone(const PadSynth&, float, int);
	virtual ~Tone();
	
	virtual void synth(float**, int);

private:
	const PadSynth&	pad;
	const PadSynth::Pad*	samples;
	
	float		stepsize;
	double		pos[2];
	float		energy;
	float		decay;
};


PadSynth::Pad::Pad(const double* in_samples, int in_length, int length)
{
	samples=new float[length];
	dsamples=new float[length];
	mask=length-1;
	
	double* tmp=new double[length];
	double scale=1.0 / sqrt(length);
	
	tmp[0]=0.0;
	tmp[length/2]=0.0;
	
	for (int i=1;i<length/2;i++) {
		tmp[i]=in_samples[i] * scale;
		tmp[length-i]=in_samples[in_length-i] * scale;
	}
	
	fftw_plan ifft=fftw_plan_r2r_1d(length, tmp, tmp, FFTW_HC2R, FFTW_ESTIMATE);
	fftw_execute(ifft);
		
	double maxval=0;	// FIXME: don't use different scale factors for each octave
	for (int i=0;i<length;i++) {
		double v=fabs(tmp[i]);
		if (v>maxval) maxval=v;
	}
	
	for (int i=0;i<length;i++)
		samples[i]=(float) (tmp[i] / maxval);

	// repeat for the derivative
	tmp[0]=0.0;
	tmp[length/2]=0.0;
	
	scale*=2.0*M_PI/length;
	for (int i=1;i<length/2;i++) {
		tmp[i]=-in_samples[in_length-i] * scale * i;
		tmp[length-i]=in_samples[i] * scale * i;
	}
	
	fftw_execute(ifft);
	fftw_destroy_plan(ifft);
	
	for (int i=0;i<length;i++)
		dsamples[i]=(float) (tmp[i] / maxval);
		
	delete[] tmp;
}

PadSynth::Pad::~Pad()
{
	delete[] samples;
	delete[] dsamples;
}

PadSynth::PadSynth(Mixer& m, float exponent):ExcitationModelInstrument(m)
{
	lowcut=0.5f;
	brightness=1.0f;
	richness=1.0f;
	bandwidth=0.25f;
	bwexponent=1.0f;
	
	band_coefficients=new float[256];
	
	length=262144;
	for (int i=0;i<8;i++)
		samples[i]=nullptr;
	
	spectrum=new float[length/2];
	
	create_random_timbre();
}

PadSynth::~PadSynth()
{
	delete[] band_coefficients;
	delete[] spectrum;
	
	for (int i=0;i<8;i++)
		delete samples[i];
}

void PadSynth::do_serialize(Serializer& ser) const
{
	ser << tag("class", "PadSynth");
	
	Instrument::do_serialize(ser);
	
	ser << tag("lowcut", lowcut) << tag("brightness", brightness) << tag("richness", richness) << tag("bandwidth", bandwidth) << tag("bandwidth_exponent", bwexponent) << tag("length", length);
	
	{
		Serializer::Tag _tag(ser, "bands");
		
		for (int i=0;i<256;i++)
			ser << band_coefficients[i];
	}
}

void PadSynth::do_deserialize(Deserializer& deser)
{
	Instrument::do_deserialize(deser);
	
	deser >> tag("lowcut", lowcut) >> tag("brightness", brightness) >> tag("richness", richness) >> tag("bandwidth", bandwidth) >> tag("bandwidth_exponent", bwexponent) >> tag("length", length);
	
	{
		Deserializer::Tag _tag(deser, "bands");
		
		for (int i=0;i<256;i++)
			_tag >> band_coefficients[i];
	}
	
	build_pad();
}

void PadSynth::create_random_timbre()
{
	xorshift64 rnd(rand());	// FIXME: use better seed
	
	for (int i=0;i<256;i++)
		band_coefficients[i]=ldexpf(rnd()>>40, -24);
		
	build_pad();
}

void PadSynth::build_pad()
{
	mixer.kill_all_tones_with_instrument(this);
	
	xorshift64 rnd(rand());	// FIXME: should use a fixed random seed for exact reproducability
	
	double* tmp=new double[length];
	
	for (int i=0;i<length;i++)
		tmp[i]=0;
	
	for (int i=1;i<=256;i++) {
		double bw=bandwidth * powf(i, -bwexponent);
		
		double bandamp=pow(band_coefficients[i-1], richness) * i * i / pow(lowcut*lowcut + i*i, 1.0+0.5*brightness);
		
		int bands=(int) ceil(3.5 / bw);
		
		for (int j=-bands;j<=bands;j++) {
			int freq=512*i + j;
			if (freq<1 || 2*freq>=length) continue;
			
			double amp=bw * j;
			amp=exp(-amp*amp) * bandamp;// * sqrt(-log(ldexp((rnd()>>12)+1, -52)));

			double phase=ldexp(rnd()>>11, -52) * M_PI;
			tmp[freq]+=amp * cos(phase);
			tmp[length-freq]+=amp * sin(phase);
		}
	}
	
	spectrum[0]=0.0f;
	float maxspectrum=0.0f;
	for (int i=1;i<length/2;i++) {
		spectrum[i]=(float) hypot(tmp[i], tmp[length-i]);
		maxspectrum=std::max(maxspectrum, spectrum[i]);
	}
	for (int i=0;i<length/2;i++)
		spectrum[i]=spectrum[i]>0 ? logf(spectrum[i]/maxspectrum) : -1e+6;
	
	for (int i=0;i<8;i++) {
		delete samples[i];
		samples[i]=new Pad(tmp, length, length>>i);
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
	
	for (int i=0;;i++)
		if (i==7 || stepsize<=1.5f) {
			samples=pad.samples[i];
			break;
		}
		else
			stepsize*=0.5f;
	
	for (int i=0;i<2;i++)
		pos[i]=pad.mixer.randf() * pad.length;
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
			samples[i][j]=(*this->samples)(pos[i]) * energy[j];
			
			pos[i]+=stepsize;
			
			if (pos[i]>=262144)
				pos[i]-=262144;
		}
}

}


