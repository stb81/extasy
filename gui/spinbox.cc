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
 
#include <limits.h>
#include "basics.h"
#include "widget.h"
#include "spinbox.h"

namespace GUI {
	
class SpinBox::Button:public Widget {
public:
	Button(int, int, const char*);
	
	virtual void draw();
	
private:
	const char*	label;
};

SpinBox::Button::Button(int x0, int y0, const char* l):Widget(x0, y0, 16, 24)
{
	label=l;
}

void SpinBox::Button::draw()
{
	float c=has_mouse_focus() ? 1.0f : 0.7f;
	
	textprint(originx+4, originy+4, c, c, c, label);
}

SpinBox::SpinBox():Widget()
{
	value=125;
	minval=INT_MIN;
	maxval=INT_MAX;
	
	inc_button=new Button(0, 0, "+");
	dec_button=new Button(0, 0, "-");

	inc_button->clicked.connect(sigc::mem_fun(this, &SpinBox::increase));
	dec_button->clicked.connect(sigc::mem_fun(this, &SpinBox::decrease));
}

SpinBox::SpinBox(int x0, int y0, int w, int h):Widget(x0, y0, w, h)
{
	value=125;
	
	inc_button=new Button(x0+w-32, y0, "+");
	dec_button=new Button(x0+w-16, y0, "-");

	inc_button->clicked.connect(sigc::mem_fun(this, &SpinBox::increase));
	dec_button->clicked.connect(sigc::mem_fun(this, &SpinBox::decrease));
}

SpinBox::~SpinBox()
{
	delete inc_button;
	delete dec_button;
}

void SpinBox::set_size(int w, int h)
{
	Widget::set_size(w, h);
	
	inc_button->set_origin(originx+width-32, originy);
	dec_button->set_origin(originx+width-16, originy);
}

void SpinBox::draw()
{
	glUseProgram(0);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	int x0=originx, y0=originy;
	
	glBegin(GL_QUADS);
	glColor4f(0.0f, 0.0f, 0.0f, 0.25f);
	glVertex2i(x0, y0);
	glVertex2i(x0+width, y0);
	glVertex2i(x0+width, y0+height);
	glVertex2i(x0, y0+height);
	glEnd();

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
	
	glBegin(GL_LINES);
	glVertex2f(x0+width-31.5f, y0+0.5f);
	glVertex2f(x0+width-31.5f, y0+height-0.5f);
	glVertex2f(x0+width-15.5f, y0+0.5f);
	glVertex2f(x0+width-15.5f, y0+height-0.5f);
	glEnd();
	
	float c=has_mouse_focus() ? 1.0f : 0.7f;
	textprintf(x0+8, y0+4, c, c, c, "%d", value);
	
	inc_button->draw();
	dec_button->draw();
}

void SpinBox::handle_event(SDL_Event& event)
{
	SDL_Event tmp=event;
	
	Widget::handle_event(event);
	
	if (event.type==SDL_MOUSEBUTTONDOWN && has_mouse_focus()) {
		if (event.button.button==SDL_BUTTON_WHEELUP) increase();
		if (event.button.button==SDL_BUTTON_WHEELDOWN) decrease();
	}
	
	inc_button->handle_event(tmp);
	dec_button->handle_event(tmp);
}

void SpinBox::move(int dx, int dy)
{
	Widget::move(dx, dy);
	
	inc_button->move(dx, dy);
	dec_button->move(dx, dy);
}

void SpinBox::set_value(int v)
{
	value=v;
	
	value_changed(value);
}

void SpinBox::set_minimum(int v)
{
	minval=v;
	
	if (value<minval)
		set_value(minval);
}

void SpinBox::set_maximum(int v)
{
	minval=v;
	
	if (value>maxval)
		set_value(maxval);
}

void SpinBox::set_range(int min, int max)
{
	minval=min;
	maxval=max;
	
	if (value<minval) set_value(minval);
	if (value>maxval) set_value(maxval);
}

void SpinBox::increase()
{
	if (value<maxval)
		value_changed(++value);
}

void SpinBox::decrease()
{
	if (value>minval)
		value_changed(--value);
}

}
