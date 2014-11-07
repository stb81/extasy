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
#include "basics.h"
#include "widget.h"
#include "button.h"
#include "colorlabelchooser.h"

unsigned int glsl_load_shader(int type, const char* filename, const char* preamble=0);
unsigned int glsl_link_program(unsigned int shv, unsigned int shf);

namespace GUI {

int ColorLabelButton::shader=0;

ColorLabelButton::ColorLabelButton(RadioButtonGroup& rbg):RadioButton(rbg)
{
	if (!shader) {
		int shv=glsl_load_shader(GL_VERTEX_SHADER, "shader/colorlabel.vert");
		int shf=glsl_load_shader(GL_FRAGMENT_SHADER, "shader/colorlabel.frag");
		shader=glsl_link_program(shv, shf);
	}
}

ColorLabelButton::~ColorLabelButton()
{
}

void ColorLabelButton::draw()
{
	int x0=originx, y0=originy;
	bool depressed=(flags&(HAS_FOCUS|MOUSE_DOWN))==(HAS_FOCUS|MOUSE_DOWN);
	if (depressed) x0++, y0++;

	bool sel=is_selected();
	
	glUseProgram(shader);

	if (sel)
		glUniform3f(glGetUniformLocation(shader, "shadefactors"), 0.3f, 0.7f, (flags&HAS_FOCUS) ? 0.6f : 0.4f);
	else
		glUniform3f(glGetUniformLocation(shader, "shadefactors"), 0.1f, 0.6f, (flags&HAS_FOCUS) ? 0.3f : 0.1f);
		
	glBegin(GL_QUADS);
	glColor3ubv(&color.r);

	glTexCoord2f(0, 0);
	glVertex2i(x0, y0);
	glTexCoord2f(1, 0);
	glVertex2i(x0+width, y0);
	glTexCoord2f(1, 1);
	glVertex2i(x0+width, y0+height);
	glTexCoord2f(0, 1);
	glVertex2i(x0, y0+height);
	glEnd();

	glUseProgram(0);

	glBegin(GL_LINE_LOOP);
	if (has_mouse_focus() || sel)
		glColor3f(1,1,1);
	else
		glColor3f(0.6,0.8,1);
			
	glVertex2f(x0+0.5, y0+0.5);
	glVertex2f(x0+width-0.5, y0+0.5);
	glVertex2f(x0+width-0.5, y0+height-0.5);
	glVertex2f(x0+0.5, y0+height-0.5);
	glEnd();

	if (!depressed) {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		
		glBegin(GL_LINE_STRIP);
		glColor4f(0,0,0,0.75);
		glVertex2f(x0+1.5, y0+height+0.5);
		glVertex2f(x0+width+0.5, y0+height+0.5);
		glVertex2f(x0+width+0.5, y0+1.5);
		glEnd();
		
		glDisable(GL_BLEND);
	}
}


ColorLabelChooser::ColorLabelChooser()
{
	buttons=nullptr;
}

ColorLabelChooser::~ColorLabelChooser()
{
	delete[] buttons;
}

void ColorLabelChooser::set_grid(int hcount, int vcount)
{
	int itemwidth=width/hcount;
	int itemheight=height/vcount;

	int vals[]={ 72, 144, 208, 240 };
	
	buttons=new ColorLabelButton*[hcount*vcount];
	
	for (int i=0;i<vcount;i++)
		for (int j=0;j<hcount;j++) {
			ColorLabelButton* but=new ColorLabelButton(group);
			but->set_origin(itemwidth*j, itemheight*i);
			but->set_size(itemwidth-8, itemheight-8);
			but->set_color(Color(vals[1], vals[2], vals[3]));
			but->clicked.connect(sigc::bind(sigc::mem_fun(this, &ColorLabelChooser::color_label_clicked), but));
			
			buttons[i*hcount+j]=but;
			
			add(but);
			int invcnt;
			do {
				invcnt=0;
				std::next_permutation(vals, vals+4);
				
				for (int k=1;k<4;k++)
					for (int l=0;l<k;l++)
						if (vals[l]>vals[k]) invcnt++;
						
			} while (invcnt&1);			
		}
}

void ColorLabelChooser::color_label_clicked(ColorLabelButton* button)
{
	color_picked(button->get_color());
}

}
