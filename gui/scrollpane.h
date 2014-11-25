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
 
#ifndef INCLUDE_GUI_SCROLLPANE_H
#define INCLUDE_GUI_SCROLLPANE_H

namespace GUI {

class ScrollPane:public Container {
public:
	ScrollPane(bool, bool);
	virtual ~ScrollPane();
	
	virtual void draw();
	virtual void handle_event(SDL_Event&);
	
	virtual void add(Widget*);
	virtual void remove(Widget*);

	virtual void set_size(int, int);
	
	virtual bool is_child_in_front(const Widget* w) const;
	
	void set_scroll_position(int, int);
	
	sigc::signal<void, int, int> scroll_position_changed;
	
private:
	bool	scrollh;
	bool	scrollv;
	bool	scrolling=false;
	
	int		scrollposx=0;
	int		scrollposy=0;
	
	Widget*	child=nullptr;
};

}

#endif
