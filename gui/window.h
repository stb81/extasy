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
 
namespace GUI {

class Window:public Group {
public:
	Window();
	Window(int, int, int, int);
	virtual ~Window();

	virtual void draw();
	virtual void handle_event(SDL_Event&);
	
	void set_title(const char* t)
	{
		title=t;
	}
	
	void set_modal(bool modal)
	{
		assign_bit<unsigned int>(flags, MODAL, modal);
	}
	
	void close();
	
	void show(MainWindow*);

private:
	static int	shader;
	static int	shader_shadow;
	
	const char*		title;
	
	unsigned int	texture;
	
	bool		dragging;
};


class Dialog:public Window {
	int		result;
	
public:
	Dialog();
	
	int run(MainWindow*);
	
protected:
	void dismiss(int);
};

}

