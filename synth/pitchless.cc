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
#include "synthbits.h"
#include "module.h"
#include "sequencer.h"
#include "pitchless.h"

namespace Synth {

class LongAllpass {
	int length, ptr;
	float	gain;
	float*	buffer;
	
public:
	LongAllpass(int, float);
	~LongAllpass();
	
	float operator()(float);
};

LongAllpass::LongAllpass(int l, float g)
{
	length=l;
	ptr=0;
	
	gain=g;
	
	buffer=new float[length];
	
	for (int i=0;i<length;i++)
		buffer[i]=0.0f;
}

LongAllpass::~LongAllpass()
{
	delete[] buffer;
}

float LongAllpass::operator()(float v)
{
	float tmp=buffer[ptr];
	tmp+=-gain*v;
	buffer[ptr]=v+gain*tmp;
	
	if (++ptr==length) ptr=0;
	
	return tmp;
}


class FwdComb {
	int		length, ptr;
	float	gain;
	float*	buffer;
	
public:
	FwdComb(int, float);
	~FwdComb();
	
	float operator()(float);
};

FwdComb::FwdComb(int l, float g)
{
	length=l;
	ptr=0;
	
	gain=g;
	
	buffer=new float[length];
	
	for (int i=0;i<length;i++)
		buffer[i]=0.0f;
}

FwdComb::~FwdComb()
{
	delete[] buffer;
}

float FwdComb::operator()(float v)
{
	float tmp=buffer[ptr];
	buffer[ptr]=v;
	
	if (++ptr==length) ptr=0;
	
	return tmp*gain+v;
}


class PitchlessSynth::Tone:public Synth::Tone {
public:
	Tone(const PitchlessSynth&);
	virtual ~Tone();
	
	virtual void synth(float**, int);

private:
	const PitchlessSynth&	ps;
	
	float	t0;
	float	tstep;
	
	int				numfilters;
	BiQuad::Filter* filters;
	
	FwdComb		comb1;
	FwdComb		comb2;
	FwdComb		comb3;
};


PitchlessSynth::PitchlessSynth(Mixer& m):Instrument(m)
{
	filters.push_back(Filter(500, 0.5f, 0, 250.0f));
	filters[0].init_filter(m);
}

PitchlessSynth::~PitchlessSynth()
{
}

Synth::Tone* PitchlessSynth::play_note(int note, int volume)
{
	return new Tone(*this);
}

void PitchlessSynth::Filter::init_filter(const Mixer& mixer)
{
	filter=BiQuad::bandpass(mixer.freq2omega(freq), Q) * (Q * powf(10.0f, gain/20.0f));
}


PitchlessSynth::Tone::Tone(const PitchlessSynth& ps):Synth::Tone(ps), ps(ps), comb1(2200, 0.75f), comb2(1700, 0.6f), comb3(3036, 0.5f)
{
	t0=0;
	tstep=1.0 / ps.mixer.get_samplerate();
	
	numfilters=ps.filters.size();
	filters=new BiQuad::Filter[4*numfilters];
	
	for (int i=0;i<4*numfilters;i++)
		filters[i]=BiQuad::Filter(ps.filters[i/4].filter);
}

PitchlessSynth::Tone::~Tone()
{
}

void PitchlessSynth::Tone::synth(float** samples, int count)
{
	for (int i=0;i<count;i++) {
		//float phi=sqrtf(t0) * 300 + 20*M_PI*t0;
		//samples[0][i]=samples[1][i]=(sinf(phi) - phi*expf(-phi) - phi*phi*expf(-2*phi)) * expf(-2*t0);
		
		float noisel=sqrtf(-2*logf(ldexpf((rand()&0xffff)+1, -16))) * cosf(M_PI*ldexpf(rand()&0xffff, -15));
		float noiser=sqrtf(-2*logf(ldexpf((rand()&0xffff)+1, -16))) * cosf(M_PI*ldexpf(rand()&0xffff, -15));
		
		float l=0, r=0;
		for (int j=0;j<numfilters;j++) {
			l+=filters[4*j  ](noisel) * powf(10.0f, -ps.filters[j].decay*t0/20.0f);
			//r+=filters[4*j+3](filters[4*j+1](noiser)) * expf(ldexpf(-ps.filters[j].decay, -8)*t0);
		}
		
		r=l=comb1(comb2(comb3(l)));
		
		samples[0][i]=l;
		samples[1][i]=r;
		
		t0+=tstep;
	}
	
	if (t0>4)
		decayed=true;
}

}

