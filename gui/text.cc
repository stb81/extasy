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
#include <stdarg.h>
#include "basics.h"
#include "text.h"

//unsigned int font_texture;

int text_shader=0;
int glyph_shader=0;

namespace GUI {
	
Font* default_font;
Font* thin_font;
Font* monospaced_font;

Font::Font()
{
	data=nullptr;
	glyphwidth=nullptr;
	glyphkern=nullptr;
	
	glGenTextures(1, &texture);
}

Font::~Font()
{
	glDeleteTextures(1, &texture);
	
	delete[] data;
	delete[] glyphwidth;
	delete[] glyphkern;
}

Font* Font::load_monospaced(const char* filename, int w, int h)
{
	Font* font=new Font;
	
	font->maxglyphwidth=w;
	font->maxglyphheight=h;
	font->data=new unsigned char[w*h*256];
	font->glyphwidth=new unsigned char[256];
	font->glyphkern=new unsigned char[256];
	
	for (int i=0;i<256;i++) {
		font->glyphwidth[i]=w;
		font->glyphkern[i]=0;
	}
	
	unsigned char* tmp=new unsigned char[w*h*256];
	
	FILE* file=fopen(filename, "r");
	fseek(file, 0x36, 0);
	fread(tmp, w*h, 256, file);
	fclose(file);
	
	const int rowstride=32*w;
	
	for (int j=0;j<8;j++)
		for (int i=0;i<32;i++)
			for (int l=0;l<h;l++)
				for (int k=0;k<w;k++)
					font->data[k+w*(l+h*(i+32*j))]=tmp[w*i+k+(h*j+l)*rowstride];

	delete[] tmp;
	
	font->update_font();
	
	return font;
}

Font* Font::load_from_file(const char* filename)
{
	FILE* file=fopen(filename, "r");
	if (!file)
		return nullptr;
	
	if (getc(file)!='F' || getc(file)!='t') {
		fclose(file);
		return nullptr;
	}
	
	Font* font=new Font;
	
	font->maxglyphwidth=getc(file);
	font->maxglyphheight=getc(file);
	font->data=new unsigned char[font->maxglyphwidth*font->maxglyphheight*256];
	font->glyphwidth=new unsigned char[256];
	font->glyphkern=new unsigned char[256];
	
	fread(font->glyphwidth, 1, 256, file);
	fread(font->glyphkern, 1, 256, file);
	fread(font->data, font->maxglyphwidth*font->maxglyphheight, 256, file);

	fclose(file);
	
	font->update_font();
	
	return font;
}

void Font::save_to_file(const char* filename)
{
	FILE* file=fopen(filename, "w");

	putc('F', file);
	putc('t', file);
	putc(maxglyphwidth, file);
	putc(maxglyphheight, file);
	
	fwrite(glyphwidth, 1, 256, file);
	fwrite(glyphkern, 1, 256, file);
	fwrite(data, maxglyphwidth*maxglyphheight, 256, file);
	
	fclose(file);
}

void Font::change_maxglyphsize(int w, int h)
{
	unsigned char* tmp=new unsigned char[w*h*256];
	
	for (int i=0;i<256;i++)
		for (int j=0;j<h;j++)
			for (int k=0;k<w;k++)
				tmp[k+w*(j+i*h)]=(k<maxglyphwidth && j<maxglyphheight) ? data[k+maxglyphwidth*(j+i*maxglyphheight)] : 0;
	
	delete[] data;
	data=tmp;
	
	maxglyphwidth=w;
	maxglyphheight=h;
	
	update_font();
}

void Font::update_font()
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_LUMINANCE, maxglyphwidth, maxglyphheight, 256, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Font::bind_texture()
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
}

int Font::get_text_width(std::string::const_iterator begin, std::string::const_iterator end) const
{
	int width=0;
	
	for (auto i=begin;i!=end;i++) {
		width-=glyphkern[*i & 0xff];
		width+=glyphwidth[*i & 0xff];
	}
	
	return width;
}

FontSpec::FontSpec()
{
	font=default_font;
	color=Color(255, 255, 255);
	tracking=0;
	alignment=0.0f;
}

int FontSpec::get_text_width(const char* text) const
{
	int width=0;
	
	for (;*text;text++) {
		width-=font->get_glyph_kern(*text & 0xff);
		width+=font->get_glyph_width(*text & 0xff);
		width+=tracking;
	}
	
	return width;
}

int FontSpec::get_text_width(std::string::const_iterator begin, std::string::const_iterator end) const
{
	int width=0;
	
	for (auto i=begin;i!=end;i++) {
		width-=font->get_glyph_kern(*i & 0xff);
		width+=font->get_glyph_width(*i & 0xff);
		width+=tracking;
	}
	
	return width;
}

void load_font()
{
	default_font=Font::load_monospaced("sanserif.pgm", 8, 16);
}

void textprint(int x0, int y0, float r, float g, float b, const char* text)
{
	glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D_ARRAY, font_texture);
	default_font->bind_texture();
	
	glUseProgram(glyph_shader);
	glUniform1i(glGetUniformLocation(glyph_shader, "font"), 0);		
	glUniform2f(glGetUniformLocation(glyph_shader, "maxglyphsize"), default_font->get_max_glyph_width(), default_font->get_max_glyph_height());
	
	glBegin(GL_POINTS);
	
	while (*text) {
		int c=*text++ & 0xff;
		
		glColor4f(r, g, b, c);
		x0-=default_font->get_glyph_kern(c);
		glVertex2i(x0, y0);
		x0+=default_font->get_glyph_width(c);
	}
	
	glEnd();
}

void textprint(int x0, int y0, const FontSpec& fontspec, const char* text)
{
	x0-=(int) (fontspec.get_text_width(text) * fontspec.alignment);
	
	glActiveTexture(GL_TEXTURE0);
	fontspec.font->bind_texture();
	
	glUseProgram(glyph_shader);
	glUniform1i(glGetUniformLocation(glyph_shader, "font"), 0);		
	glUniform2f(glGetUniformLocation(glyph_shader, "maxglyphsize"), fontspec.font->get_max_glyph_width(), fontspec.font->get_max_glyph_height());
	
	glBegin(GL_POINTS);
	
	const float r=fontspec.color.r / 255.0f;
	const float g=fontspec.color.g / 255.0f;
	const float b=fontspec.color.b / 255.0f;
	
	while (*text) {
		int c=*text++ & 0xff;
		
		glColor4f(r, g, b, c);
		x0-=fontspec.font->get_glyph_kern(c);
		glVertex2i(x0, y0);
		x0+=fontspec.font->get_glyph_width(c);
		x0+=fontspec.tracking;
	}
	
	glEnd();
}

void textprintf(int x0, int y0, float r, float g, float b, const char* text,...)
{
	va_list args;
	
	va_start(args, text);
	char buf[256];
	vsprintf(buf, text, args);
	va_end(args);
	
	textprint(x0, y0, r, g, b, buf);
}

void textprintf(int x0, int y0, const FontSpec& fontspec, const char* text,...)
{
	va_list args;
	
	va_start(args, text);
	char buf[256];
	vsprintf(buf, text, args);
	va_end(args);
	
	textprint(x0, y0, fontspec, buf);
}

TextCanvas::TextCanvas(int w, int h)
{
	glGenTextures(1, &texture);

	buffer=nullptr;
	set_size(w, h);
	
	background_color=Color(0,0,0,255);
}

TextCanvas::~TextCanvas()
{
	delete[] buffer;

	glDeleteTextures(1, &texture);
}

void TextCanvas::set_size(int w, int h)
{
	width=w;
	height=h;
	
	delete[] buffer;
	buffer=new Char[width*height];
}

void TextCanvas::set_background_color(const Color& color)
{
	background_color=color;
}

void TextCanvas::update()
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texture);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void TextCanvas::draw(int dstx, int dsty)
{
	glActiveTexture(GL_TEXTURE0);
	monospaced_font->bind_texture();
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texture);
	
	glUseProgram(text_shader);
	glUniform1i(glGetUniformLocation(text_shader, "font"), 0);
	glUniform1i(glGetUniformLocation(text_shader, "text"), 1);
	glUniform2f(glGetUniformLocation(text_shader, "origin"), dstx, dsty);
	glUniform4f(glGetUniformLocation(text_shader, "bgcolor"), background_color.r/255.0f, background_color.g/255.0f, background_color.b/255.0f, background_color.a/255.0f);

	glBegin(GL_QUADS);
	glVertex2i(0    , 0);
	glVertex2i(width, 0);
	glVertex2i(width, height);
	glVertex2i(0    , height);
	glEnd();
}

void TextCanvas::clear()
{
	for (int i=0;i<width*height;i++)
		buffer[i].c=' ';
}

}

