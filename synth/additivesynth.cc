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
#include "additivesynth.h"

namespace Synth {

class AdditiveSynth::Tone:public ExcitationModelTone {
	struct Oscillator {
		float	t;
		float	tstep;
		float	amplitude;
		float	decaypower;
		
		void init(const Partial&, float);
		
		float operator()(float energy)
		{
			float v=sinf(t);
			
			t+=tstep;
			if (t>=2*M_PI) t-=2*M_PI;
			
			return v * amplitude * powf(energy, decaypower);
		}
	};
	
public:
	Tone(AdditiveSynth&, float, int);
	virtual ~Tone();
	
	virtual void synth(float**, int);

private:
	AdditiveSynth&	addsynth;

	Oscillator*		partials;
	int				numpartials;
	
	BiQuad			highpass;
	BiQuad::Filter	hpfilter1;
	BiQuad::Filter	hpfilter2;

	VarDelay	delay;
	Noise		noisel[8];
	Noise		noiser[8];
};

void AdditiveSynth::Tone::Oscillator::init(const AdditiveSynth::Partial& p, float omega)
{
	t=0;
	tstep=p.freq*omega;
	amplitude=p.amplitude;
	decaypower=p.decaypower;
}

AdditiveSynth::Tone::Tone(AdditiveSynth& sy, float freq, int volume):ExcitationModelTone(sy, freq, volume), addsynth(sy), delay(256)
{
	numpartials=addsynth.partials.size();
	partials=new Oscillator[numpartials];
	
	float omega=sy.mixer.freq2omega(freq);
	
	for (int i=0;i<numpartials;i++)
		partials[i].init(addsynth.partials[i], omega);
	
	highpass=BiQuad::highpass(omega*0.5f, 1.0f);
	hpfilter1=BiQuad::Filter(highpass);
	hpfilter2=BiQuad::Filter(highpass);
	
	for (int i=0;i<8;i++) {
		noisel[i].init(0.0001f + i*0.00003f);
		noiser[i].init(0.0001f + i*0.00003f);
	}
}

AdditiveSynth::Tone::~Tone()
{
}

void AdditiveSynth::Tone::synth(float** out, int count)
{
	float energy[count];
	compute_excitation(energy, count);
	
	for (int i=0;i<count;i++) {
		float tmp=0;
		for (int j=0;j<numpartials;j++)
			tmp+=partials[j](energy[i]);

		delay.push(tmp*(tmp*tmp*(1-tmp*tmp)+0.5));
		//delay.push(tmp);

		float v=0, w=0;
		for (int j=0;j<8;j++) {
			v+=delay(noisel[j]() * 250.0f * modulation) * 0.125f;
			w+=delay(noiser[j]() * 250.0f * modulation) * 0.125f;
		}
		
		out[0][i]=v;
		out[1][i]=w;
	}	
}

AdditiveSynth::AdditiveSynth(Mixer& m):ExcitationModelInstrument(m)
{
	Partial p;
	
	p.freq=1.0f;
	p.amplitude=1.0f;
	p.decaypower=1.0f;
	partials.push_back(p);
	
	p.freq=2.0f;
	partials.push_back(p);
}

AdditiveSynth::~AdditiveSynth()
{
}

Synth::Tone* AdditiveSynth::play_note(int note, int volume)
{
	return new Tone(*this, note2freq(note), volume);
}

}

