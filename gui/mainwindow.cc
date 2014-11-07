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
 
#include "gui.h"

namespace GUI {

MainWindow::MainWindow()
{
	//screen=SDL_SetVideoMode(1440, 960, 32, SDL_OPENGL);
	screen=SDL_SetVideoMode(1680, 1050, 32, SDL_OPENGL | SDL_FULLSCREEN);
	
	if (!screen) {
		fprintf(stderr, "Error setting video mode: %s\n", SDL_GetError());
		exit(1);
	}
	
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_EnableUNICODE(1);

	set_size(screen->w, screen->h);
	
	glewInit();
	
	glMatrixMode(GL_PROJECTION);
	glTranslatef(-1,1,0);
	glScalef(2.0/screen->w, -2.0/screen->h, 1.0);
	
	quit=false;
}

MainWindow::~MainWindow()
{
}

MainWindow* MainWindow::get_root_window()
{
	return this;
}

void MainWindow::main_loop(std::function<bool()> cond)
{
	while (!quit && (!cond || cond())) {
		SDL_Event event;
		
		if (SDL_WaitEvent(&event))
			do {
				if (event.type==SDL_QUIT)
					quit=true;
				else
					handle_event(event);
			} while (SDL_PollEvent(&event));
		
		while (!dispatch_queue.empty()) {
			dispatch_queue.front()();
			dispatch_queue.pop();
		}
			
		glClearColor(0,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT);
		
		draw();
		SDL_GL_SwapBuffers();
	}
}

void MainWindow::dispatch(std::function<void()> fn)
{
	dispatch_queue.push(fn);
}

}

