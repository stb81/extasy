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
 
#include <stdio.h>
#include "gui/gui.h"
#include "fontedit.h"
#include "fontedit.form.h"

using namespace GUI;

unsigned int glsl_load_shader(int type, const char* filename, const char* preamble=0)
{
	FILE* file=fopen(filename, "r");
	if (!file) {
		fprintf(stderr, "Error opening %s\n", filename);
		abort();
	}
	
	fseek(file, 0, SEEK_END);
	int size=ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char* buffer=new char[size+1];
	fread(buffer, 1, size, file);
	fclose(file);

	buffer[size]=0;
	
	const char* src[]={ preamble?preamble:"", buffer };
	
	unsigned int name=glCreateShader(type);
	glShaderSource(name, 2, (const GLchar**) src, 0);
	delete[] buffer;
	
	glCompileShader(name);
	
	int foo;
	glGetShaderiv(name, GL_COMPILE_STATUS, &foo);
	if (foo==GL_TRUE) return name;
	
	char msg[1024];
	glGetShaderInfoLog(name, 1024, &foo, msg);
	fprintf(stderr, "Error compiling glsl program '%s':\n%s\n", filename, msg);
	
	exit(1);
}

unsigned int glsl_link_program(unsigned int shv, unsigned int shf)
{
	unsigned int sh=glCreateProgram();

	glAttachShader(sh, shv);
	glAttachShader(sh, shf);
	glLinkProgram(sh);

	return sh;
}

unsigned int glsl_link_program(unsigned int shv, unsigned int shg, unsigned int shf, int vert_out, int geom_in, int geom_out)
{
	unsigned int sh=glCreateProgram();

	glAttachShader(sh, shv);
	glAttachShader(sh, shg);
	glAttachShader(sh, shf);
	
	glProgramParameteriEXT(sh, GL_GEOMETRY_VERTICES_OUT_EXT, vert_out);
	glProgramParameteriEXT(sh, GL_GEOMETRY_INPUT_TYPE_EXT, geom_in);
	glProgramParameteriEXT(sh, GL_GEOMETRY_OUTPUT_TYPE_EXT, geom_out);
	
	glLinkProgram(sh);

	int foo;
	glGetProgramiv(sh, GL_LINK_STATUS, &foo);
	if (foo==GL_TRUE) return sh;
	
	char msg[1024];
	glGetProgramInfoLog(sh, 1024, &foo, msg);
	fprintf(stderr, "Error linking glsl program:\n%s\n", msg);
	//return sh;
	exit(1);
}

FontMatrix::FontMatrix():Widget(0, 0, 256, 256)
{
	selected=0;
	highlighted=-1;
	
	clicked.connect(sigc::mem_fun(this, &FontMatrix::on_clicked));
}

void FontMatrix::handle_event(SDL_Event& event)
{
	Widget::handle_event(event);
	
	if (event.type==SDL_MOUSEMOTION)
		highlighted=has_mouse_focus() ? (event.motion.x-originx)/16 + ((event.motion.y-originy)&-16)  : -1;
}

void FontMatrix::on_clicked()
{
	if (highlighted>=0) {
		selected=highlighted;
		glyph_selected(selected);
	}
}

void FontMatrix::draw()
{
	int x, y;
	
	glUseProgram(0);
	
	glBegin(GL_QUADS);
	
	if (highlighted>=0) {
		x=highlighted&15;
		y=highlighted>>4;
		glColor3f(0.125, 0.125, 0.25);
		glVertex2i(originx+x*16, originy+y*16);
		glVertex2i(originx+x*16+16, originy+y*16);
		glVertex2i(originx+x*16+16, originy+y*16+16);
		glVertex2i(originx+x*16, originy+y*16+16);
	}
	
	x=selected&15;
	y=selected>>4;
	glColor3f(0.5, 0.5, 0.5);
	glVertex2i(originx+x*16, originy+y*16);
	glVertex2i(originx+x*16+16, originy+y*16);
	glVertex2i(originx+x*16+16, originy+y*16+16);
	glVertex2i(originx+x*16, originy+y*16+16);
	
	glEnd();
	
	glActiveTexture(GL_TEXTURE0);
	default_font->bind_texture();
	
	glUseProgram(glyph_shader);
	glUniform1i(glGetUniformLocation(glyph_shader, "font"), 0);		
	
	glBegin(GL_POINTS);
	
	for (y=0;y<16;y++)
		for (x=0;x<16;x++) {
			glColor4f(1, 1, 1, 16*y+x);
			glVertex2i(originx+x*16+4, originy+y*16);
		}
	
	glEnd();
}

GlyphMatrix::GlyphMatrix():Widget(0, 0, 513, 513)
{
}

void GlyphMatrix::draw()
{
	glUseProgram(0);
	
	glBegin(GL_QUADS);
	if (highlighted>=0) {
		int x=highlighted&15, y=highlighted>>4;
		
		glColor3f(0.125, 0.125, 0.25);
		glVertex2i(originx+x*32, originy+y*32);
		glVertex2i(originx+x*32+33, originy+y*32);
		glVertex2i(originx+x*32+33, originy+y*32+33);
		glVertex2i(originx+x*32, originy+y*32+33);
	}
	
	unsigned char* raster=font->get_glyph_raster(glyph);
	const int maxgw=font->get_max_glyph_width();
	const int maxgh=font->get_max_glyph_height();
	
	glColor3f(0.8f, 0.8f, 0.8f);
	for (int i=0;i<maxgh;i++)
		for (int j=0;j<maxgw;j++) {
			unsigned char c=raster[maxgw*i+j];
			glColor3ub(c, c, c);
			
			glVertex2i(originx+j*32, originy+i*32);
			glVertex2i(originx+j*32+32, originy+i*32);
			glVertex2i(originx+j*32+32, originy+i*32+32);
			glVertex2i(originx+j*32, originy+i*32+32);
		}
	
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.25f, 0.25f, 0.25f);
	
	for (int i=0;i<=maxgw;i++) {
		glVertex2i(originx+i*32, originy);
		glVertex2i(originx+i*32, originy+height-1);
	}
	
	for (int i=0;i<=maxgh;i++) {
		glVertex2i(originx, originy+i*32);
		glVertex2i(originx+width-1, originy+i*32);
	}
	
	const int gw=font->get_glyph_width(glyph);
	const int gk=font->get_glyph_kern(glyph);
	
	glColor3f(1.0, 0.8f, 0.2f);
	glVertex2i(originx+gw*32, originy);
	glVertex2i(originx+gw*32, originy+height-1);
	
	glColor3f(0.2, 0.6f, 1.0f);
	glVertex2i(originx+gk*32, originy);
	glVertex2i(originx+gk*32, originy+height-1);
	
	glEnd();
}

void GlyphMatrix::handle_event(SDL_Event& event)
{
	Widget::handle_event(event);
	
	if (event.type==SDL_MOUSEMOTION) {
		highlighted=has_mouse_focus() ? (event.motion.x-originx)/32 + (event.motion.y-originy)/32*font->get_max_glyph_width()  : -1;
		
		if ((event.motion.state&SDL_BUTTON_LMASK) && highlighted>=0) {
			font->get_glyph_raster(glyph)[highlighted]=draw_color;
			font->update_font();
		}
	}
	
	if (event.type==SDL_MOUSEBUTTONDOWN && highlighted>=0) {
		unsigned char* raster=font->get_glyph_raster(glyph);
		
		if (event.button.button==SDL_BUTTON_RIGHT)
			draw_color=128;
		else
			draw_color=raster[highlighted]>128 ? 0 : 255;
			
		raster[highlighted]=draw_color;
		font->update_font();
	}
}

FontMatrix* fontmatrix;
GlyphMatrix* glyphmatrix;

void change_glyph_width(int w)
{
	int glyph=fontmatrix->get_selected();
	if (glyph<0) return;
	
	default_font->set_glyph_width(glyph, w);
}

FontEditBase::FontEditBase()
{
	default_font=Font::load_monospaced("../fonts/sanserif.pgm", 8, 16);
	default_font->change_maxglyphsize(16, 16);
}

void FontEditBase::shift_left()
{
	const int maxgw=default_font->get_max_glyph_width();
	const int maxgh=default_font->get_max_glyph_height();
	
	unsigned char* raster=default_font->get_glyph_raster(selected_glyph);
	
	for (int i=0;i<maxgh;i++) {
		for (int j=1;j<maxgw;j++)
			raster[j-1+i*maxgw]=raster[j+i*maxgw];
		
		raster[(i+1)*maxgw-1]=0;
	}
	
	default_font->update_font();
}

void FontEditBase::shift_right()
{
	const int maxgw=default_font->get_max_glyph_width();
	const int maxgh=default_font->get_max_glyph_height();
	
	unsigned char* raster=default_font->get_glyph_raster(selected_glyph);
	
	for (int i=0;i<maxgh;i++) {
		for (int j=maxgw-1;j>0;j--)
			raster[j+i*maxgw]=raster[j-1+i*maxgw];
		
		raster[i*maxgw]=0;
	}
	
	default_font->update_font();
}

int main(int argn, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	
	MainWindow* root=new FontEditMainWnd();
	
	int shv=glsl_load_shader(GL_VERTEX_SHADER, "../shader/text.vert");
	int shf=glsl_load_shader(GL_FRAGMENT_SHADER, "../shader/text.frag");
	text_shader=glsl_link_program(shv, shf);
	
	int shg;
	shv=glsl_load_shader(GL_VERTEX_SHADER, "../shader/glyph.vert");
	shg=glsl_load_shader(GL_GEOMETRY_SHADER_EXT, "../shader/glyph.geom");
	shf=glsl_load_shader(GL_FRAGMENT_SHADER, "../shader/glyph.frag");
	glyph_shader=glsl_link_program(shv, shg, shf, 8, GL_POINTS, GL_TRIANGLE_STRIP);
	
/*	glyphmatrix=new GlyphMatrix();
	glyphmatrix->set_origin(32, 320);
	glyphmatrix->set_font(default_font);
	glyphmatrix->set_glyph('A');
	root->add(glyphmatrix);
	
	fontmatrix=new FontMatrix();
	fontmatrix->set_origin(32, 32);
	fontmatrix->glyph_selected.connect(sigc::mem_fun(glyphmatrix, &GlyphMatrix::set_glyph));
	root->add(fontmatrix);
	
	SpinBox* glyphwidth=new SpinBox(640, 320, 64, 24);
	glyphwidth->set_range(1, 16);
	glyphwidth->value_changed.connect(sigc::ptr_fun(change_glyph_width));
	root->add(glyphwidth);
	
	Button* shiftleft=new Button(640, 352, 64, 24, "Shift Left");
	shiftleft->clicked.connect(sigc::ptr_fun(on_shift_left));
	root->add(shiftleft);
	
	LineEdit* testline=new LineEdit(32, 864, 512, 24);
	testline->set_text("The quick brown fox jumps over the lazy dog!");
	root->add(testline);*/
	
	root->main_loop();
	delete root;
	SDL_Quit();

	return 0;
}

