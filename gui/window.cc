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
#include "window.h"
#include "mainwindow.h"

unsigned int glsl_load_shader(int type, const char* filename, const char* preamble=0);
unsigned int glsl_link_program(unsigned int shv, unsigned int shf);

void textprint(int x0, int y0, float, float, float, const char* text);

namespace GUI {

int Window::shader=0;
int Window::shader_shadow=0;

Window::Window():Group()
{
	if (!shader) {
		int shv=glsl_load_shader(GL_VERTEX_SHADER, "shader/window.vert");
		int shf=glsl_load_shader(GL_FRAGMENT_SHADER, "shader/window.frag");
		shader=glsl_link_program(shv, shf);
		
		shv=glsl_load_shader(GL_VERTEX_SHADER, "shader/window-shadow.vert");
		shf=glsl_load_shader(GL_FRAGMENT_SHADER, "shader/window-shadow.frag");
		shader_shadow=glsl_link_program(shv, shf);
	}
	
	glGenTextures(1, &texture);
	
	dragging=false;
	title="Unnamed Window";
	layer=256;
}

Window::Window(int x, int y, int w, int h):Group(x,y,w,h)
{
	if (!shader) {
		int shv=glsl_load_shader(GL_VERTEX_SHADER, "shader/window.vert");
		int shf=glsl_load_shader(GL_FRAGMENT_SHADER, "shader/window.frag");
		shader=glsl_link_program(shv, shf);
		
		shv=glsl_load_shader(GL_VERTEX_SHADER, "shader/window-shadow.vert");
		shf=glsl_load_shader(GL_FRAGMENT_SHADER, "shader/window-shadow.frag");
		shader_shadow=glsl_link_program(shv, shf);
	}
	
	glGenTextures(1, &texture);
	
	dragging=false;
	title="Unnamed Window";
	layer=256;
}

Window::~Window()
{
	glDeleteTextures(1, &texture);
}

void Window::draw()
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	if (flags&MODAL) {
		if (parent->is_child_in_front(this)) {
			glUseProgram(0);
			
			glEnable(GL_BLEND);

			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			
			glBegin(GL_QUADS);
			glColor4f(0, 0, 0, 0.5f);
			glVertex2i(-1, -1);
			glVertex2i( 1, -1);
			glVertex2i( 1,  1);
			glVertex2i(-1,  1);
			glEnd();
			
			glPopMatrix();
			
			glDisable(GL_BLEND);
		}
	}
	else {
		// render drop shadow
		glUseProgram(shader_shadow);
		glUniform2f(glGetUniformLocation(shader_shadow, "wndorigin"), originx, originy);
		glUniform2f(glGetUniformLocation(shader_shadow, "wndsize"), width, height);
		
		glEnable(GL_BLEND);

		glBegin(GL_QUADS);
		glVertex2f(0, 0);
		glVertex2f(1, 0);
		glVertex2f(1, 1);
		glVertex2f(0, 1);
		glEnd();
		
		glDisable(GL_BLEND);
	}
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, originx, 1050-originy-height, width, height, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// render window background
	glUseProgram(shader);
	glUniform1i(glGetUniformLocation(shader, "background"), 0);
	
	glColor3f(1,has_mouse_focus()?1:0,0);
	
	glBegin(GL_QUADS);
	glTexCoord2f(0,1);
	glVertex2f(originx, originy);
	glTexCoord2f(1,1);
	glVertex2f(originx+width, originy);
	glTexCoord2f(1,0);
	glVertex2f(originx+width, originy+height);
	glTexCoord2f(0,0);
	glVertex2f(originx, originy+height);
	glEnd();
	
	FontSpec fontspec;
	fontspec.font=default_font;
	fontspec.color=has_mouse_focus() ? Color(255,255,255) : Color(127,179,255);
	fontspec.tracking=1;
	textprint(originx+4, originy+4, fontspec, title);
	
	glUseProgram(0);
	
	// frame
	glBegin(GL_LINE_LOOP);
	glColor3f(0.6,0.8,1.0);
	glVertex2f(originx-0.5, originy-0.5);
	glVertex2f(originx+width-0.5, originy-0.5);
	glVertex2f(originx+width-0.5, originy+height-0.5);
	glVertex2f(originx-0.5, originy+height-0.5);
	glEnd();
	
	glEnable(GL_BLEND);
	
	glBegin(GL_LINE_LOOP);
	glColor4f(0, 0, 0, 0.5);
	glVertex2f(originx-1.5, originy-1.5);
	glVertex2f(originx+width+0.5, originy-1.5);
	glVertex2f(originx+width+0.5, originy+height+0.5);
	glVertex2f(originx-1.5, originy+height+0.5);
	glEnd();
	
	glDisable(GL_BLEND);
	
	Group::draw();
}

void Window::handle_event(SDL_Event& event)
{
	Group::handle_event(event);
	
	switch(event.type) {
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.button==SDL_BUTTON_RIGHT)
			dragging=has_mouse_focus() && contains_point(event.button.x, event.button.y);
		break;
	case SDL_MOUSEBUTTONUP:
		if (event.button.button==SDL_BUTTON_RIGHT)
			dragging=false;
		break;
	case SDL_MOUSEMOTION:
		if (dragging)
			move(event.motion.xrel, event.motion.yrel);
		break;
	}
}

void Window::close()
{
	dispatch([this]() { parent->remove(this); delete this; } );
}

void Window::show(MainWindow* mainwnd)
{
	set_origin((mainwnd->get_width()-get_width())/2, (mainwnd->get_height()-get_height())/2);
	
	layer=mainwnd->get_largest_child_layer() + 1;
	
	mainwnd->add(this);
}


Dialog::Dialog()
{
	set_modal(true);
}

int Dialog::run(MainWindow* mainwnd)
{
	result=-1;

	show(mainwnd);
	mainwnd->main_loop([this]() { return result<0; } );
	mainwnd->remove(this);
	
	return result;
}

void Dialog::dismiss(int res)
{
	result=res;
}

}


