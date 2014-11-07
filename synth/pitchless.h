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
 
#ifndef INCLUDE_PITCHLESS_H
#define INCLUDE_PITCHLESS_H

namespace Synth {

class PitchlessSynth:public Instrument {
public:
	class Tone;
	class Editor;
	
	PitchlessSynth(Mixer&);
	virtual ~PitchlessSynth();
	
	virtual Synth::Tone* play_note(int, int);	
	
private:
	struct Filter {
		float	freq;
		float	Q;
		float	gain;
		float	decay;
		BiQuad			filter;
		
		Filter(float f, float Q, float gain, float decay):freq(f), Q(Q), gain(gain), decay(decay) {}
		
		void init_filter(const Mixer&);
	};
	
	std::vector<Filter>	filters;
};

}

#endif
