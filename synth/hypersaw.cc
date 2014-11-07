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
#include "hypersaw.h"

namespace Synth {

class HyperSaw::Tone:public Synth::Tone {
public:
	Tone(const HyperSaw&, int, int);
	virtual ~Tone();
	
	virtual void synth(float**, int);
	
private:
	float	excitation;
	float	potential;
	
	float	pos[7];
	float	stepsize[7];
	float	gain[7];
};


HyperSaw::HyperSaw(Mixer& m):Instrument(m)
{
}

HyperSaw::~HyperSaw()
{
}

Synth::Tone* HyperSaw::play_note(int note, int volume)
{
	return new Tone(*this, note, volume);
}

Instrument* HyperSaw::create(Mixer& mixer)
{
	return new HyperSaw(mixer);
}


HyperSaw::Tone::Tone(const HyperSaw& hs, int note, int vol):Synth::Tone(hs)
{
	float ss=Instrument::note2freq(note) / hs.mixer.get_samplerate();
	
	for (int i=0;i<7;i++)
		pos[i]=ldexpf(rand()&0xffff, -16);
	
	stepsize[0]=ss * 0.973f;
	stepsize[1]=ss * 0.986f;
	stepsize[2]=ss * 0.997f;
	stepsize[3]=ss;
	stepsize[4]=ss * 1.004f;
	stepsize[5]=ss * 1.017f;
	stepsize[6]=ss * 1.026f;
	
	gain[0]=0.26f;
	gain[1]=0.48f;
	gain[2]=0.76f;
	gain[3]=1.00f;
	gain[4]=0.74f;
	gain[5]=0.52f;
	gain[6]=0.24f;
	
	excitation=ldexpf(vol, -7);
	potential=2*excitation;
}

HyperSaw::Tone::~Tone()
{
}

void HyperSaw::Tone::synth(float** samples, int count)
{
	if (stopped)
		excitation=0;
	
	for (int i=0;i<count;i++) {
		potential+=excitation * 0.0001f;
		potential*=0.9999f;
		
		float v=0, w=0;
		
		for (int j=0;j<7;j++) {
			v+=(2*pos[j] - 1.0f) * gain[j];
			
			float tmp=pos[j] + 0.37f*j;
			tmp-=floorf(tmp);
			w+=(2*tmp - 1.0f) * gain[6-j];
			
			pos[j]+=stepsize[j];
			if (pos[j]>=1.0f)
				pos[j]-=1.0f;
		}
		
		samples[0][i]=v * potential / 4;
		samples[1][i]=w * potential / 4;
	}
	
	if (stopped && potential<1e-5)
		decayed=true;
}

}

