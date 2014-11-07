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
 
#ifndef INCLUDE_SPINBOX_H
#define INCLUDE_SPINBOX_H

namespace GUI {

class SpinBox:public Widget {
	class Button;
	
public:
	SpinBox();
	SpinBox(int, int, int, int);
	virtual ~SpinBox();

	virtual void set_size(int, int);
	
	virtual void draw();
	virtual void handle_event(SDL_Event&);
	virtual void move(int, int);

	sigc::signal<void, int> value_changed;
	
	void set_value(int);
	int get_value() const { return value; }
	
	void set_minimum(int);
	void set_maximum(int);
	void set_range(int, int);

private:
	void	increase();
	void	decrease();
	
	int		value;
	int		minval;
	int		maxval;
	
	Button*	inc_button;
	Button*	dec_button;
};

}

#endif
