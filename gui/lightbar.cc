#include <math.h>
#include "basics.h"
#include "widget.h"
#include "lightbar.h"

unsigned int glsl_load_shader(int type, const char* filename, const char* preamble=0);
unsigned int glsl_link_program(unsigned int shv, unsigned int shf);

namespace GUI {

int LightBarArray::shader=0;

LightBarArray::LightBarArray(int numvalues, orientation_t orientation):numvalues(numvalues), orientation(orientation)
{
	if (!shader) {
		int shv=glsl_load_shader(GL_VERTEX_SHADER, "shader/lightbar.vert");
		int shf=glsl_load_shader(GL_FRAGMENT_SHADER, "shader/lightbar.frag");
		shader=glsl_link_program(shv, shf);
	}
	
	values=new float[numvalues];
	
	color_scheme[0]=Color(32, 128, 192);
	color_scheme[1]=Color(64, 192, 255);
	color_scheme[2]=Color(128, 255, 255);
	color_scheme[3]=Color(192, 255, 128);
	
	assign_bit<unsigned>(flags, FOCUSABLE, true);
}

LightBarArray::~LightBarArray()
{
	delete[] values;
}

void LightBarArray::handle_event(SDL_Event& event)
{
	Widget::handle_event(event);
	
	if (event.type==SDL_MOUSEMOTION) {
		if (!(event.motion.state&0x7f)) {
			focused=orientation==HORIZONTAL ? (event.motion.y-originy)/16 : (event.motion.x-originx)/16;
			if (!has_mouse_focus() || focused<0 || focused>=numvalues)
				focused=-1;
		}
	}
	
	if (event.type==SDL_MOUSEMOTION && (flags&MOUSE_DOWN) && focused>=0)
		values[focused]=orientation==HORIZONTAL ? float(event.motion.x-originx+8)/width : float(height-event.motion.y+originy+8)/height;
}

void LightBarArray::draw()
{
	float color_light[12], color_dark[12];
	for (int i=0;i<4;i++) {
		Color col;
		
		col=(flags&FOCUSABLE) ? color_scheme[i].shade(160) : color_scheme[i];
		color_light[3*i  ]=ldexpf(col.r*0x10101, -24);
		color_light[3*i+1]=ldexpf(col.g*0x10101, -24);
		color_light[3*i+2]=ldexpf(col.b*0x10101, -24);
		
		col=(flags&FOCUSABLE) ? color_scheme[i].shade(112) : color_scheme[i];
		color_dark[3*i  ]=ldexpf(col.r*0x10101, -24);
		color_dark[3*i+1]=ldexpf(col.g*0x10101, -24);
		color_dark[3*i+2]=ldexpf(col.b*0x10101, -24);
	}
	
	glUseProgram(shader);
	
	const int length_uniform=glGetUniformLocation(shader, "length");
	const int color_scheme_uniform=glGetUniformLocation(shader, "color_scheme");
	const int display_value_uniform=glGetUniformLocation(shader, "display_value");
	
	const float length=orientation==HORIZONTAL ? width : height;
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	int x0=originx, y0=originy;
	for (int i=0;i<numvalues;i++) {
		
		glUniform1f(length_uniform, length);
		glUniform3fv(color_scheme_uniform, 4, focused==i ? color_light : color_dark);
		glUniform1f(display_value_uniform, values[i]*length);
		
		glBegin(GL_QUADS);
		
		if (orientation==HORIZONTAL) {
			glTexCoord2f(0, 16);
			glVertex2f(x0, y0);
			glTexCoord2f(width, 16);
			glVertex2f(x0+width, y0);
			glTexCoord2f(width,0);
			glVertex2f(x0+width, y0+16);
			glTexCoord2f(0,0);
			glVertex2f(x0, y0+16);
			
			y0+=16;
		}
		else {
			glTexCoord2f(height, 16);
			glVertex2f(x0, y0);
			glTexCoord2f(0, 16);
			glVertex2f(x0, y0+height);
			glTexCoord2f(0,0);
			glVertex2f(x0+16, y0+height);
			glTexCoord2f(height,0);
			glVertex2f(x0+16, y0);
			
			x0+=16;
		}
	
		glEnd();
	}
	
	glDisable(GL_BLEND);
}


LightBar::LightBar():LightBarArray(1, HORIZONTAL)
{
}

void LightBar::set_size(int w, int h)
{
	orientation=w>h ? HORIZONTAL : VERTICAL;
	
	LightBarArray::set_size(w, h);
}

}
