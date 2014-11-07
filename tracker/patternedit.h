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
 
#ifndef INCLUDE_PATTERNEDIT_H
#define INCLUDE_PATTERNEDIT_H

namespace Synth {
	class Module;
	class Pattern;
}

class PatternEdit:public GUI::Widget {
public:
	PatternEdit();
	
	virtual void handle_event(SDL_Event&);
	virtual void draw();
	
	void set_module(Synth::Module*);
	void set_pattern(Synth::Pattern*);
	
	void set_highlight_row(int);
	
	void set_instrument(int);
	void set_modulation(float);
	
private:
	void update_canvas();
	
	static int key_to_digit(int keysym);
	static int key_to_effect(int keysym);
	static int key_to_note(int keysym);

	GUI::TextCanvas	canvas;
	
	Synth::Module*	module;
	Synth::Pattern*	pattern;
	
	int		current_instrument;
	
	int		cursorx;
	int		cursory;
	int		selectx;
	int		selecty;
	bool	selection;
	
	int		scrollposx;
	int		scrollposy;
	
	int		highlight_row;
	
	int		modulation;
	
	Synth::Tone*	tones[120];
};

#endif
