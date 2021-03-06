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
 
#ifndef INCLUDE_HPANED_H
#define INCLUDE_HPANED_H

namespace GUI {

// FIXME: derive from Container
class HPaned:public Widget {
public:
	HPaned(int, int, int, int);
	~HPaned();
	
	virtual void handle_event(SDL_Event&);
	virtual void draw();
	//virtual void move(int, int);
	
	void add_pane(const char*, Widget*);
	
private:
	void handle_double_click();
	void update_positions();
	
	struct Pane {
		const char*	name;
		Widget*		content;
		int			position;
		bool		visible;
		bool		label_focused;
	};
	
	std::vector<Pane>	panes;
};

}

#endif
