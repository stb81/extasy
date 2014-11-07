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
 
#include "kickdrum.h"

namespace Synth {

class KickDrum::Tone:public Synth::Tone {
public:
	Tone(KickDrum&, float);
	virtual ~Tone();
	
	virtual void synth(float**, int);

private:
	KickDrum&	kickdrum;
	
	float	t;
	float	tstep;
};

KickDrum::Tone::Tone(KickDrum& kd, float volume):Synth::Tone(kd), kickdrum(kd)
{
	t=0;
	tstep=1.0f / kickdrum.mixer.get_samplerate();
}

KickDrum::Tone::~Tone()
{
}

void KickDrum::Tone::synth(float** out, int count)
{
	for (int i=0;i<count;i++) {
		float u=pow(t, kickdrum.tpower) * kickdrum.basefreq;
		out[0][i]=out[1][i]=sinf(3.0f*u + 2.0f*cosf(u)*expf(-t*kickdrum.modulator_decay)) * expf(-t*kickdrum.carrier_decay);
		t+=tstep;
	}
	
	if (stopped && t*kickdrum.carrier_decay>10)
		decayed=true;
}

KickDrum::KickDrum(Mixer& m):Instrument(m)
{
	basefreq=50.0f;
	tpower=0.75f;
	carrier_decay=5.0f;
	modulator_decay=2.0f;
}

KickDrum::~KickDrum()
{
}

Synth::Tone* KickDrum::play_note(int note, int volume)
{
	return new Tone(*this, ldexpf(volume, -7));
}

}
