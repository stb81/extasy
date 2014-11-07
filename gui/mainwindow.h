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
 
#ifndef INCLUDE_GUI_MAINWINDOW_H
#define INCLUDE_GUI_MAINWINDOW_H

#include <queue>

namespace GUI {

class MainWindow:public Group {
	SDL_Surface* screen;
	
	bool	quit;
	
	std::queue<std::function<void()>>	dispatch_queue;

public:
	MainWindow();
	virtual ~MainWindow();
	
	void main_loop(std::function<bool()> cond=nullptr);

	virtual MainWindow* get_root_window();
	
protected:
	virtual void dispatch(std::function<void()>);
};

}

#endif
