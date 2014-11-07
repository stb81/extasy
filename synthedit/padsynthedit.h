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
 
#ifndef INCLUDE_PADSYNTHEDIT_H
#define INCLUDE_PADSYNTHEDIT_H

#include "synth/padsynth.h"

namespace Synth {

class PadSynth::Editor:public GUI::Window {
	PadSynth*	instrument;
	
	GUI::Knob*	bw_knob;
	GUI::Knob*	exponent_knob;
	GUI::Knob*	discrete_knob;
	GUI::Knob*	continuous_knob;
	GUI::Knob*	decay_knob;
	GUI::Knob*	decay_knob2;
	
	void knob_changed();
	
public:
	Editor(PadSynth*);
	
	static Window* create(Instrument*);
};

}

#endif
