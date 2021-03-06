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
 
#include <math.h>
#include "synthbits.h"
#include "module.h"
#include "sequencer.h"
#include "filter.h"
#include "pwmsynth.h"
#include "serialization.h"

namespace Synth {

class PWMSynth::Tone:public Synth::Tone {
public:
	Tone(const PWMSynth&, int, int);
	virtual ~Tone();
	
	virtual void synth(float**, int);
	
private:
	float	timestep;
	float	volume;
	float	energy;
	
	float	ramppos[3];
	float	rampstep[3];
	
	float	modpos[3];
	float	modstep[3];
	
	Resample	resample[2];
	IntegralCombFilter	comb[2];
};


PWMSynth::PWMSynth(Mixer& m):Instrument(m)
{
}

PWMSynth::~PWMSynth()
{
}

Instrument* PWMSynth::create(Mixer& m)
{
	return new PWMSynth(m);
}

void PWMSynth::do_serialize(Serializer& ser) const
{
	ser << tag("class", "PWMSynth");
	
	Instrument::do_serialize(ser);
}

void PWMSynth::do_deserialize(Deserializer& deser)
{
	Instrument::do_deserialize(deser);
}

Synth::Tone* PWMSynth::play_note(int note, int volume)
{
	return new Tone(*this, note, volume);
}


PWMSynth::Tone::Tone(const PWMSynth& pwm, int note, int vol):Synth::Tone(pwm)
{
	timestep=2.5f / pwm.mixer.get_samplerate();
	volume=ldexpf(vol, -8);
	energy=0;
	
	float freq=Instrument::note2freq(note);
	int period=(int) floorf(pwm.mixer.get_samplerate() / freq);
	float rate=period * freq;
	printf("rate=%f\n", rate);
	
	for (int i=0;i<2;i++) {
		ramppos[i]=0;
		rampstep[i]=freq / rate;
	
		modpos[i]=0;
		modstep[i]=rampstep[i] * 0.01f;
	}
	
	modpos[1]=1.0f;
	
	resample[0]=Resample(rate/pwm.mixer.get_samplerate());
	resample[1]=Resample(rate/pwm.mixer.get_samplerate());
	
	comb[0].init(period, 1.0f, -0.9f);
	comb[1].init(period, 1.0f, -0.9f);
	
	add_filter(new CustomBiQuadFilterInstance(BiQuad::highpass(freq*2*M_PI/pwm.mixer.get_samplerate(), 1.0f)));
}

PWMSynth::Tone::~Tone()
{
}

void PWMSynth::Tone::synth(float** samples, int count)
{
	for (int i=0;i<count;i++) {
		if (!stopped)
			energy+=volume * timestep;
			
		energy-=energy * timestep;
		
		for (int j=0;j<2;j++) {

			while (!resample[j]) {
				float thres=(modpos[j]>1.0f ? 2.0f-modpos[j] : modpos[j]) * 0.9f + 0.05f;
				//float thres=sinf(modpos[j]*M_PI)*0.45f + 0.5f;
			
				//resample[j](sinf(ramppos[j]*2.0f*M_PI));
				resample[j](comb[j](ramppos[j] < thres ? -1.0f : 1.0f));
			
				ramppos[j]+=rampstep[j];
				if (ramppos[j]>=1.0f) ramppos[j]-=1.0f;
			
				modpos[j]+=modstep[j];
				if (modpos[j]>=2.0f) modpos[j]-=2.0f;
			}
		
			samples[j][i]=*resample[j] * energy;
		}
	}
	
	if (stopped && energy<1e-5f)
		decayed=true;
}

}

