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
 
#ifndef INCLUDE_SYNTH_FMSYNTH_H
#define INCLUDE_SYNTH_FMSYNTH_H

namespace Synth {

class FMSynth:public ExcitationModelInstrument {
	int		carrier_freq;
	int		mod1_freq;
	int		mod2_freq;
	
	float	carrier_decay;
	float	mod1_decay;
	float	mod2_decay;
	
	float	mod1_depth;
	float	mod2_depth;
	
public:
	class Tone;
	class Editor;
	
	FMSynth(Mixer&);
	virtual ~FMSynth();
	
	virtual Synth::Tone* play_note(int, int);
};

}

#endif
