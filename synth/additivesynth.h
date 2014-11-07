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
 
#ifndef INCLUDE_SYNTH_ADDITIVESYNTH_H
#define INCLUDE_SYNTH_ADDITIVESYNTH_H

#include <vector>

namespace Synth {

class Noise;

class AdditiveSynth:public ExcitationModelInstrument {
	class Tone;
	class Editor;
	
public:
	AdditiveSynth(Mixer&);
	virtual ~AdditiveSynth();
	
	virtual Synth::Tone* play_note(int, int);
	
private:
	struct Partial {
		float	freq;
		float	amplitude;
		float	decaypower;
	};
	
	std::vector<Partial>	partials;
};

}

#endif
