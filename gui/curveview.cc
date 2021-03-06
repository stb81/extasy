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

#include <algorithm>
#include <math.h>
#include "basics.h"
#include "widget.h"
#include "curveview.h"

namespace GUI {

CurveView::CurveView()
{
	description="";
}

CurveView::CurveView(int x, int y, int w, int h):Widget(x, y, w, h)
{
	description="";
}

void CurveView::add_vertical_line(float pos, Color color, int id, const char* label)
{
	vline_highlight=nullptr;
	
	vlines.push_back(vline_t { pos, color, id });
	
	if (label)
		vlines.back().label=label;
}

void CurveView::remove_vertical_line(int id)
{
	vline_highlight=nullptr;
	
	vlines.erase(std::remove_if(vlines.begin(), vlines.end(), [id](vline_t& vl) { return vl.id==id; }), vlines.end());
}

void CurveView::set_vertical_line_position(int id, float pos)
{
	for (auto& vl: vlines)
		if (vl.id==id)
			vl.position=pos;
}

void CurveView::set_vertical_line_color(int id, Color color)
{
	for (auto& vl: vlines)
		if (vl.id==id)
			vl.color=color;
}

void CurveView::set_vertical_line_label(int id, const char* label)
{
	for (auto& vl: vlines)
		if (vl.id==id)
			vl.label=label;
}

int CurveView::get_selected_vertical_line() const
{
	return vline_highlight ? vline_highlight->id : -1;
}

void CurveView::draw()
{
	glUseProgram(0);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	int x0=originx, y0=originy;
	
	glBegin(GL_QUADS);
	glColor4f(0.125,0.1825,0.25,0.5);
	glVertex2i(x0, y0);
	glVertex2i(x0+width, y0);
	glVertex2i(x0+width, y0+height);
	glVertex2i(x0, y0+height);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glColor4f(0,0,0,0.75);
	glVertex2f(x0+1.5, y0+height+0.5);
	glVertex2f(x0+width+0.5, y0+height+0.5);
	glVertex2f(x0+width+0.5, y0+1.5);
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
	
	Scissor scissor(x0+1, y0+1, width-2, height-2);
	
	glEnable(GL_BLEND);
	
	glBegin(GL_LINES);
	glColor4f(0.6f, 0.8f, 1.0f, 0.5f);
	glVertex2i(x0+1, y0+height/2);
	glVertex2i(x0+width-2, y0+height/2);
	
	for (auto& vl: vlines) {
		float x=x0 + width*vl.position;
		
		Color c=&vl==vline_highlight ? vl.color.shade(192) : vl.color;
		
		glColor4ubv(&c.r);
		
		glVertex2f(x, y0+1);
		glVertex2f(x, y0+height-1);
	}
	glEnd();
	
	glEnable(GL_LINE_SMOOTH);
	
	for (int i=0;i<curves.size();i++) {
		int count=width/2;
		float Y[count];
		
		for (int j=0;j<count;j++)
			Y[j]=(1.0f - (*curves[i])((float) (2*j+1)/width)) * (height-2) + y0 + 1;
		
		glLineWidth(3.0f);
		glBegin(GL_LINE_STRIP);
		glColor4f(0,0,0,0.5f);
		
		for (int j=0;j<count;j++)
			glVertex2f(x0+2*j+1, Y[j]);
		
		glEnd();
		
		glLineWidth(1.0f);
		glBegin(GL_LINE_STRIP);
		glColor3ubv(&curves[i]->get_color().r);
		
		for (int j=0;j<count;j++)
			glVertex2f(x0+2*j+1, Y[j]);
		
		glEnd();
	}

	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);
	
	textprint(x0+4, y0+4, 0.6f, 0.8f, 1, description);
	
	if (vline_highlight) {
		FontSpec font;
		font.font=thin_font;
		font.alignment=0.5f;
	
		int x=lrintf(x0 + width*vline_highlight->position);
		textprint(x, y0+4, font, vline_highlight->label.c_str());
	}
}

void CurveView::handle_event(SDL_Event& event)
{
	Widget::handle_event(event);
	
	if (event.type==SDL_MOUSEBUTTONDOWN && vline_highlight)
		vline_dragging=true;
	
	if (event.type==SDL_MOUSEBUTTONUP)
		vline_dragging=false;
	
	if (event.type==SDL_MOUSEMOTION && !(event.motion.state&0x7f)) {
		vline_highlight=nullptr;
		
		if (!has_mouse_focus())
			return;

		int bestdist=16;
		
		for (auto& vl: vlines) {
			int dist=abs(lrintf(originx + width*vl.position) - event.motion.x);
			
			if (dist>bestdist) continue;
			
			bestdist=dist;
			vline_highlight=&vl;
		}
	}
	
	if (event.type==SDL_MOUSEMOTION && vline_dragging && vline_highlight) {
		vline_highlight->position+=(float) event.motion.xrel / width;
		if (vline_highlight->position<0) vline_highlight->position=0.0f;
		if (vline_highlight->position>1) vline_highlight->position=1.0f;
		
		vertical_line_dragged(vline_highlight->id, vline_highlight->position);
	}
}

}
