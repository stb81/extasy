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
}

}
