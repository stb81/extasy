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
 
#include <math.h>
#include "basics.h"
#include "widget.h"
#include "knob.h"
#include "lineedit.h"

namespace GUI {

Knob::Knob():Widget()
{
	label=nullptr;
	
	position=0;
	midi_controller=-1;
	digits=3;
	
	edit_widget=nullptr;
	double_clicked.connect(sigc::mem_fun(this, &Knob::edit_value));
}

Knob::Knob(int x, int y, int w, int h):Widget(x, y, w, h)
{
	label=nullptr;
	
	position=0;
	midi_controller=-1;
	digits=3;
	
	edit_widget=nullptr;
	double_clicked.connect(sigc::mem_fun(this, &Knob::edit_value));
}

Knob::~Knob()
{
	delete edit_widget;
}

void Knob::handle_event(SDL_Event& event)
{
	if (edit_widget)
		edit_widget->handle_event(event);
		
	Widget::handle_event(event);
	
	if (event.type==SDL_MOUSEMOTION && (flags&MOUSE_DOWN)) {
		/*position=atan2f(event.motion.y-originy-height/2, event.motion.x-originx-width/2) / M_PI;
		if (position>0.5f) position-=2.0f;
		
		if (position<-1.25f)
			position=0;
		else if (position>0.25f)
			position=1;
		else
			position=(position+1.25f) / 1.5f;*/
			
		float x0=event.motion.x - originx - width/2;
		float y0=event.motion.y - originy - width/2 - 16;
		
		position+=(event.motion.yrel*x0 - event.motion.xrel*y0) / (x0*x0 + y0*y0 + 1.0f) / (1.5f*M_PI);
		
		if (position<0.0f) position=0.0f;
		if (position>1.0f) position=1.0f;
			
		value_changed(position, get_value());
	}
	
	if (event.type==SDL_MIDI_EVENT) {
		unsigned char* msg=(unsigned char*) event.user.data1;
		
		if (msg[0]==0xb0 && msg[1]==midi_controller) {
			position=msg[2] / 127.0f;
			
			value_changed(position, get_value());
		}
	}
}

void Knob::draw()
{
	FontSpec fontspec;
	fontspec.font=thin_font;
	fontspec.alignment=0.5f;
	
	if (label)
		textprint(originx+width/2, originy, fontspec, label);
	
	glUseProgram(0);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	glTranslatef(originx+width/2, originy+width/2+16, 0);
	glRotatef((1.25f-1.5f*position)*180, 0, 0, -1);
	glScalef(16, 16, 1);
	
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(0.9f, 0.9f, 0.9f);
	glVertex2f(0, 0);
	if (flags&(MOUSE_DOWN|HAS_FOCUS))
		glColor3f(0.6f, 0.6f, 0.6f);
	else
		glColor3f(0.5f, 0.5f, 0.5f);
	for (int i=0;i<=12;i++)
		glVertex2f(cosf(M_PI*i/6), sinf(M_PI*i/6));
	glEnd();
	
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_BLEND);
	
	glBegin(GL_TRIANGLES);
	glColor3i(0, 0, 0);
	glVertex2f(1,0);
	glVertex2f(-0.8, 0.25f);
	glVertex2f(-0.8, -0.25f);
	glEnd();

	glDisable(GL_BLEND);
	glDisable(GL_POLYGON_SMOOTH);
	
	glPopMatrix();
	
	fontspec.font=thin_font;
	textprintf(originx+width/2, originy+width+16, fontspec, "%.*f", digits, get_value());
	
	if (edit_widget)
		edit_widget->draw();
}

void Knob::set_range(float minv, float maxv, scale_t sc)
{
	min_value=minv;
	max_value=maxv;
	scale=sc;
}

void Knob::set_value(float val)
{
	if (scale==LINEAR)
		position=(val-min_value) / (max_value-min_value);
	else
		position=logf(val/min_value) / logf(max_value/min_value);
	
	if (position<0.0f) position=0.0f;
	if (position>1.0f) position=1.0f;
}

float Knob::get_value() const
{
	if (scale==LINEAR)
		return min_value*(1.0f-position) + max_value*position;
	else
		return min_value * powf(max_value/min_value, position);
}

void Knob::bind_value(float& v)
{
	bind_value_handler.disconnect();
	
	set_value(v);
	bind_value_handler=value_changed.connect(sigc::bind(sigc::ptr_fun(&Knob::update_bound_value), &v));
}

void Knob::update_bound_value(float pos, float val, float* boundval)
{
	*boundval=val;
}

void Knob::edit_value()
{
	if (edit_widget) return;
	
	char tmp[16];
	sprintf(tmp, "%.*f", digits, get_value());
	
	edit_widget=new LineEdit(originx, originy+28, 40, 24);
	edit_widget->set_text(tmp);
	edit_widget->activate_cursor();

	edit_widget->cancelled.connect(sigc::hide(sigc::mem_fun(this, &Knob::edit_cancelled)));
	edit_widget->finished.connect(sigc::hide(sigc::mem_fun(this, &Knob::edit_finished)));
	edit_widget->lost_input_focus.connect(sigc::hide(sigc::mem_fun(this, &Knob::edit_finished)));
}

void Knob::edit_cancelled()
{
	delete edit_widget;
	edit_widget=nullptr;
}

void Knob::edit_finished()
{
	char* endptr;
	float val=strtof(edit_widget->get_text(), &endptr);
	if (!*endptr) {
		set_value(val);
		value_changed(position, get_value());
	}

	edit_cancelled();
}

}
