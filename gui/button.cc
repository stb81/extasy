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
#include "button.h"

namespace GUI {

Button::Button()
{
	label="";
	
	hotkey_sym=(SDLKey) 0;
	hotkey_mod=KMOD_NONE;
}

Button::Button(int x0, int y0, int w, int h, const char* l):Widget(x0,y0,w,h), label(l)
{
	hotkey_sym=(SDLKey) 0;
	hotkey_mod=KMOD_NONE;
}

Button::~Button()
{
}

void Button::set_label(const char* l)
{
	label=l;
}

void Button::set_hotkey(SDLKey sym, SDLMod mod)
{
	hotkey_sym=sym;
	hotkey_mod=mod;
}

void Button::draw()
{
	glUseProgram(0);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	int x0=originx, y0=originy;
	bool depressed=(flags&(HAS_FOCUS|MOUSE_DOWN))==(HAS_FOCUS|MOUSE_DOWN);
	if (depressed) x0++, y0++;
	
	glBegin(GL_QUADS);
	glColor4f(0.5,0.75,1,0.5);
	glVertex2i(x0, y0);
	glVertex2i(x0+width, y0);
	glVertex2i(x0+width, y0+height);
	glVertex2i(x0, y0+height);
	glEnd();

	if (!depressed) {
		glBegin(GL_LINE_STRIP);
		glColor4f(0,0,0,0.75);
		glVertex2f(x0+1.5, y0+height+0.5);
		glVertex2f(x0+width+0.5, y0+height+0.5);
		glVertex2f(x0+width+0.5, y0+1.5);
		glEnd();
	}
	
	glDisable(GL_BLEND);
	
	glBegin(GL_LINE_LOOP);
	if (has_mouse_focus())
		glColor3f(1,1,1);
	else
		glColor3f(0.6,0.8,1);
			
	glVertex2f(x0+0.5, y0+0.5);
	glVertex2f(x0+width-0.5, y0+0.5);
	glVertex2f(x0+width-0.5, y0+height-0.5);
	glVertex2f(x0+0.5, y0+height-0.5);
	glEnd();
	
	textprint(x0+8, y0+4, 1, 1, 1, label);
}

void Button::handle_event(SDL_Event& event)
{
	Widget::handle_event(event);
	
	if (event.type==SDL_KEYDOWN && event.key.keysym.sym==hotkey_sym && (event.key.keysym.mod&(KMOD_CTRL|KMOD_ALT|KMOD_SHIFT))==hotkey_mod)
		clicked();
}


CheckBox::CheckBox()
{
	clicked.connect(sigc::mem_fun(this, &CheckBox::toggle));
}

CheckBox::CheckBox(int x0, int y0, int w, int h, const char* l):Button(x0, y0, w, h, l)
{
	clicked.connect(sigc::mem_fun(this, &CheckBox::toggle));
}

CheckBox::~CheckBox()
{
}

void CheckBox::draw()
{
	glUseProgram(0);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	int x0=originx, y0=originy;
	bool depressed=(flags&(HAS_FOCUS|MOUSE_DOWN))==(HAS_FOCUS|MOUSE_DOWN);
	if (depressed) x0++, y0++;
	
	glBegin(GL_QUADS);
	glColor4f(0.5,0.75,1,0.5);
	glVertex2i(x0, y0);
	glVertex2i(x0+height, y0);
	glVertex2i(x0+height, y0+height);
	glVertex2i(x0, y0+height);
	glEnd();

	if (!depressed) {
		glBegin(GL_LINE_STRIP);
		glColor4f(0,0,0,0.75);
		glVertex2f(x0+1.5, y0+height+0.5);
		glVertex2f(x0+height+0.5, y0+height+0.5);
		glVertex2f(x0+height+0.5, y0+1.5);
		glEnd();
	}
	
	glDisable(GL_BLEND);
	
	glBegin(GL_LINE_LOOP);
	if (has_mouse_focus())
		glColor3f(1,1,1);
	else
		glColor3f(0.6,0.8,1);
			
	glVertex2f(x0+0.5, y0+0.5);
	glVertex2f(x0+height-0.5, y0+0.5);
	glVertex2f(x0+height-0.5, y0+height-0.5);
	glVertex2f(x0+0.5, y0+height-0.5);
	glEnd();
	
	if (is_checked()) {
		glBegin(GL_QUADS);
		glColor3f(1, 1, 1);
		glVertex2i(x0+4, y0+4);
		glVertex2i(x0+height-4, y0+4);
		glVertex2i(x0+height-4, y0+height-4);
		glVertex2i(x0+4, y0+height-4);
		glEnd();
	}
	
	textprint(originx+height+8, originy, 1, 1, 1, label);
}

bool CheckBox::is_checked() const
{
	return checked;
}

void CheckBox::set_checked(bool c)
{
	checked=c;
}

void CheckBox::toggle()
{
	set_checked(!checked);
}


RadioButton::RadioButton(RadioButtonGroup& g):group(g)
{
	clicked.connect(sigc::mem_fun(this, &RadioButton::select));
}

RadioButton::RadioButton(int x0, int y0, int w, int h, const char* l, RadioButtonGroup& g):Button(x0, y0, w, h, l), group(g)
{
	index=group.next_index++;
	
	clicked.connect(sigc::mem_fun(this, &RadioButton::select));
}

RadioButton::~RadioButton()
{
}

void RadioButton::draw()
{
	glUseProgram(0);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	int x0=originx, y0=originy;
	bool depressed=(flags&(HAS_FOCUS|MOUSE_DOWN))==(HAS_FOCUS|MOUSE_DOWN);
	if (depressed) x0++, y0++;
	
	glBegin(GL_QUADS);
	glColor4f(0.5,0.75,1,0.5);
	glVertex2i(x0, y0);
	glVertex2i(x0+height, y0);
	glVertex2i(x0+height, y0+height);
	glVertex2i(x0, y0+height);
	glEnd();

	if (!depressed) {
		glBegin(GL_LINE_STRIP);
		glColor4f(0,0,0,0.75);
		glVertex2f(x0+1.5, y0+height+0.5);
		glVertex2f(x0+height+0.5, y0+height+0.5);
		glVertex2f(x0+height+0.5, y0+1.5);
		glEnd();
	}
	
	glDisable(GL_BLEND);
	
	glBegin(GL_LINE_LOOP);
	if (has_mouse_focus())
		glColor3f(1,1,1);
	else
		glColor3f(0.6,0.8,1);
			
	glVertex2f(x0+0.5, y0+0.5);
	glVertex2f(x0+height-0.5, y0+0.5);
	glVertex2f(x0+height-0.5, y0+height-0.5);
	glVertex2f(x0+0.5, y0+height-0.5);
	glEnd();
	
	if (is_selected()) {
		glBegin(GL_QUADS);
		glColor3f(1, 1, 1);
		glVertex2i(x0+4, y0+4);
		glVertex2i(x0+height-4, y0+4);
		glVertex2i(x0+height-4, y0+height-4);
		glVertex2i(x0+4, y0+height-4);
		glEnd();
	}
	
	textprint(originx+height+8, originy, 1, 1, 1, label);
}

void RadioButton::select()
{
	group.selected=this;
}

bool RadioButton::is_selected() const
{
	return group.selected==this;
}

int RadioButtonGroup::get_selected_index() const
{
	return selected ? selected->get_index() : -1;
}

}

