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
 
#ifndef INCLUDE_LINEEDIT_H
#define INCLUDE_LINEEDIT_H

#include <string>

namespace GUI {
	
class LineEdit:public Widget {
	std::string	text;
	int			cursor;
	int			scrollpos;
	
	void adjust_scroll_position();
	
public:
	LineEdit();
	LineEdit(int, int, int, int);
	virtual ~LineEdit();
	
	virtual void handle_event(SDL_Event&);
	virtual void draw();
	
	void set_text(const char*);
	const char* get_text() const
	{
		return text.c_str();
	}
	
	void activate_cursor()
	{
		cursor=text.length();
	}
	
	sigc::signal<void, LineEdit*>	finished;
	sigc::signal<void, LineEdit*>	cancelled;
	sigc::signal<void, LineEdit*>	lost_input_focus;
	sigc::signal<void, const std::string&>	text_changed;
};

}

#endif
