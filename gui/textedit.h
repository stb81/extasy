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
 
#ifndef INCLUDE_GUI_TEXTEDIT_H
#define INCLUDE_GUI_TEXTEDIT_H

#include <string>
#include "text.h"

namespace GUI {

class TextColorizer {
public:
	class Default;
	
	virtual std::pair<std::string::iterator, Color> next_token_color(std::string::iterator pos, std::string::iterator end) const =0;
};

class TextColorizer::Default:public TextColorizer {
public:
	virtual std::pair<std::string::iterator, Color> next_token_color(std::string::iterator pos, std::string::iterator end) const;
};

class TextEdit:public Widget {
public:
	TextEdit();
	virtual ~TextEdit();
	
	virtual void set_size(int, int);
	
	virtual void handle_event(SDL_Event&);
	virtual void draw();
	
	void set_text(const std::string&);
	const std::string& get_text() const;
	
	void set_error_highlight(int, int);
	
	void set_colorizer(TextColorizer*);
	
private:
	TextCanvas	canvas;
	
	std::string	buffer;
	std::string	clipboard;
	
	int			cursorpos;
	int			selectpos;
	int			error_begin;
	int			error_end;
	
	TextColorizer*	colorizer;
	
	void update_canvas();
	
	void get_gridpos(int bufpos, int& gridx, int& gridy) const;
	int get_bufferpos(int gridx, int gridy) const;
};

}

#endif
