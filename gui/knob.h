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
 
#ifndef INCLUDE_KNOB_H
#define INCLUDE_KNOB_H

namespace GUI {

class LineEdit;

class Knob:public Widget {
public:
	enum scale_t {
		LINEAR,
		LOGARITHMIC
	};
	
	Knob();
	Knob(int, int, int, int);
	virtual ~Knob();
	
	virtual void handle_event(SDL_Event&);
	virtual void draw();
	
	void set_label(const char* l)
	{
		label=l;
	}
	
	void set_range(float minval, float maxval, scale_t scale=LINEAR);
	
	void set_position(float pos)
	{
		position=pos;
	}
	
	float get_position() const
	{
		return position;
	}
	
	void set_value(float);
	float get_value() const;
	
	void set_digits(int d)
	{
		digits=d;
	}
	
	void set_midi_controller(int mc)
	{
		midi_controller=mc;
	}
	
	void bind_value(float&);
	
	sigc::signal<void, float, float>	value_changed;
	
private:
	const char*	label;
	
	float	position;
	
	float	min_value;
	float	max_value;
	scale_t	scale;
	int		digits;
	
	int		midi_controller;
	
	sigc::connection	bind_value_handler;
	
	LineEdit*	edit_widget;
	
	void edit_value();
	void edit_cancelled();
	void edit_finished();
	
	static void update_bound_value(float, float, float*);
};

}

#endif

