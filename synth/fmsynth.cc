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
 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "synthbits.h"
#include "module.h"
#include "sequencer.h"
#include "filter.h"
#include "fmsynth.h"

namespace Synth {
	
class FMSynth::Tone:public ExcitationModelTone {
public:
	Tone(FMSynth&, float, int);
	virtual ~Tone();
	
	virtual void synth(float**, int);

private:
	FMSynth&	fmsynth;
	
	BiQuad		allpass_coeffs;
	BiQuad::Filter	allpass_filter;
	BiQuad::Filter	allpass_filter2;
	
	float		omega;
	
	int			ptr;
};

FMSynth::Tone::Tone(FMSynth& sy, float freq, int volume):ExcitationModelTone(sy, freq, volume), fmsynth(sy)
{
	ptr=0;
	
	omega=sy.mixer.freq2omega(freq);
	
	add_filter(new ChorusInstance(250.0f, 8));
	
	allpass_coeffs=BiQuad::allpass(omega*3.0f, 2.0f);
	allpass_filter=BiQuad::Filter(allpass_coeffs);
	allpass_filter2=BiQuad::Filter(allpass_coeffs);
}

FMSynth::Tone::~Tone()
{
}

void FMSynth::Tone::synth(float** out, int count)
{
	float energy[count];
	compute_excitation(energy, count);
	
	for (int i=0;i<count;i++, ptr++) {
		float t=(float) ptr / fmsynth.mixer.get_samplerate();
		
		float v, w;
		
		v=0.0f;
		const float alpha=0.95f * sqrtf(energy[i]);
		float delta=omega*ptr;
		delta=fmsynth.carrier_freq*delta + fmsynth.mod1_depth*powf(energy[i], fmsynth.mod1_decay)*sinf(fmsynth.mod1_freq*delta);
		for (int j=0;j<2;j++)
			v=sinf(alpha*v + delta);
		
		v-=(sinf(alpha*v + delta) - v) / (alpha*cosf(alpha*v + delta) - 1.0f);

		out[0][i]=out[1][i]=v * energy[i];
	}	
}

FMSynth::FMSynth(Mixer& m):ExcitationModelInstrument(m)
{
	carrier_freq=1;
	mod1_freq=1;
	mod2_freq=1;
	
	carrier_decay=1.0f;
	mod1_decay=1.0f;
	mod2_decay=1.0f;
	
	mod1_depth=1.0f;
	mod2_depth=1.0f;
}

FMSynth::~FMSynth()
{
}

Synth::Tone* FMSynth::play_note(int note, int volume)
{
	return new Tone(*this, note2freq(note), volume);
}

}

