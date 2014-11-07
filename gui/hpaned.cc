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
 
#include "basics.h"
#include "widget.h"
#include "hpaned.h"

namespace GUI {

HPaned::HPaned(int x, int y, int w, int h):Widget(x, y, w, h)
{
	double_clicked.connect(sigc::mem_fun(this, &HPaned::handle_double_click));
}

HPaned::~HPaned()
{
}

void HPaned::add_pane(const char* name, Widget* content)
{
	panes.push_back(Pane());
	panes.back().name=name;
	panes.back().content=content;
	panes.back().visible=true;
	
	update_positions();
}

void HPaned::update_positions()
{
	int pos=0;
	
	for (int i=0;i<panes.size();i++) {
		panes[i].position=pos;
		
		pos+=16;
		panes[i].content->set_origin(originx+pos, originy);
		
		if (panes[i].visible)
			pos+=panes[i].content->get_width();
	}
}

void HPaned::handle_event(SDL_Event& event)
{
	SDL_Event tmp=event;
	
	Widget::handle_event(event);
	
	for (int i=0;i<panes.size();i++) {
		if (event.type==SDL_MOUSEMOTION && !(event.motion.state&SDL_BUTTON(1)))
			panes[i].label_focused=has_mouse_focus() && event.motion.x>=originx+panes[i].position && event.motion.x<originx+panes[i].position+16;
		
		if (panes[i].visible)
			panes[i].content->handle_event(tmp);
	}
}

void HPaned::handle_double_click()
{
	for (int i=0;i<panes.size();i++)
			if (panes[i].label_focused)
				panes[i].visible=!panes[i].visible;
	
	update_positions();
}

void HPaned::draw()
{
	glUseProgram(0);
	
	glBegin(GL_QUADS);
	glColor3f(0.25f, 0.375f, 0.5f);
	glVertex2i(originx,       originy);
	glVertex2i(originx+width, originy);
	glVertex2i(originx+width, originy+height);
	glVertex2i(originx,       originy+height);
	glEnd();
	
	Scissor scissor(originx, originy, width, height);
	
	for (int i=0;i<panes.size();i++) {
		float c=panes[i].label_focused ? 1 : 0.8f;
		
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(originx+panes[i].position, originy+height-4, 0);
		glRotatef(90, 0, 0, -1);
		textprint(0, 0, c, c, c, panes[i].name);
		glPopMatrix();
		
		if (panes[i].visible)
			panes[i].content->draw();
	}
}

}
