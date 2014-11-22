#include <math.h>
#include "basics.h"
#include "widget.h"
#include "lightbar.h"

unsigned int glsl_load_shader(int type, const char* filename, const char* preamble=0);
unsigned int glsl_link_program(unsigned int shv, unsigned int shf);

namespace GUI {

int LightBar::shader=0;

LightBar::LightBar()
{
	if (!shader) {
		int shv=glsl_load_shader(GL_VERTEX_SHADER, "shader/lightbar.vert");
		int shf=glsl_load_shader(GL_FRAGMENT_SHADER, "shader/lightbar.frag");
		shader=glsl_link_program(shv, shf);
	}
	
	color_scheme[0]=Color(32, 128, 192);
	color_scheme[1]=Color(64, 192, 255);
	color_scheme[2]=Color(128, 255, 255);
	color_scheme[3]=Color(192, 255, 128);
	
	assign_bit<unsigned>(flags, FOCUSABLE, true);
}

void LightBar::handle_event(SDL_Event& event)
{
	Widget::handle_event(event);
	
	if (event.type==SDL_MOUSEMOTION && (flags&MOUSE_DOWN))
		value=(float) (event.motion.x-originx+8)/width;
}

void LightBar::draw()
{
	glUseProgram(shader);
	glUniform1f(glGetUniformLocation(shader, "display_value"), value*width);
	
	float color_tmp[12];
	for (int i=0;i<4;i++) {
		Color col=color_scheme[i];
		
		if (flags&FOCUSABLE)
			col=(flags&(HAS_FOCUS|MOUSE_DOWN)) ? col.shade(160) : col.shade(112);
		
		color_tmp[3*i  ]=ldexpf(col.r*0x10101, -24);
		color_tmp[3*i+1]=ldexpf(col.g*0x10101, -24);
		color_tmp[3*i+2]=ldexpf(col.b*0x10101, -24);
	}
	
	glUniform3fv(glGetUniformLocation(shader, "color_scheme"), 4, color_tmp);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	glBegin(GL_QUADS);
	glTexCoord2f(0,height);
	glVertex2f(originx, originy);
	glTexCoord2f(width,height);
	glVertex2f(originx+width, originy);
	glTexCoord2f(width,0);
	glVertex2f(originx+width, originy+height);
	glTexCoord2f(0,0);
	glVertex2f(originx, originy+height);
	glEnd();
	
	glDisable(GL_BLEND);
}

}
