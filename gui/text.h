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
 
#ifndef INCLUDE_GUI_TEXT_H
#define INCLUDE_GUI_TEXT_H

#include <string>

extern int text_shader;
extern int glyph_shader;

namespace GUI {

class Font {
	int				maxglyphwidth, maxglyphheight;
	
	unsigned char*	data;
	unsigned char*	glyphwidth;
	unsigned char*	glyphkern;
	
	unsigned int	texture;
	
public:
	Font();
	~Font();
	
	static Font* load_monospaced(const char*, int, int);
	static Font* load_from_file(const char*);
	
	void save_to_file(const char*);
	
	void change_maxglyphsize(int, int);
	
	void update_font();
	void update_glyph(int);
	
	void bind_texture();
	
	int get_max_glyph_width()
	{
		return maxglyphwidth;
	}
	
	int get_max_glyph_height()
	{
		return maxglyphheight;
	}
	
	int get_glyph_width(int g)
	{
		return glyphwidth[g];
	}
	
	int set_glyph_width(int g, int w)
	{
		glyphwidth[g]=w;
	}
	
	int get_glyph_kern(int g)
	{
		return glyphkern[g];
	}
	
	int set_glyph_kern(int g, int k)
	{
		glyphkern[g]=k;
	}
	
	unsigned char* get_glyph_raster(int g)
	{
		return data + g*maxglyphwidth*maxglyphheight;
	}
	
	int get_text_width(std::string::const_iterator, std::string::const_iterator) const;
};

struct FontSpec {
	Font*	font;
	Color	color;
	int		tracking;
	float	alignment;
	
	FontSpec();
	
	int get_text_width(const char*) const;
	int get_text_width(std::string::const_iterator, std::string::const_iterator) const;
};

extern Font* default_font;
extern Font* thin_font;
extern Font* monospaced_font;


void load_font();

void textprint(int x0, int y0, float r, float g, float b, const char* text);
void textprintf(int x0, int y0, float r, float g, float b, const char* text, ...);

void textprint(int x0, int y0, const FontSpec&, const char* text);
void textprintf(int x0, int y0, const FontSpec&, const char* text, ...);

class TextCanvas {
public:
	struct Char {
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char c;
		
		void operator=(Color c)
		{
			r=c.r;
			g=c.g;
			b=c.b;
		}
	};
	
	TextCanvas(int, int);
	~TextCanvas();
	
	void set_size(int, int);
	
	bool is_in_bounds(int x, int y) const
	{
		return x>=0 && y>=0 && x<width && y<height;
	}
	
	Char& operator()(int x, int y)
	{ return buffer[x+y*width]; }
	
	const Char& operator()(int x, int y) const
	{ return buffer[x+y*width]; }
	
	void update();
	void draw(int, int);
	
	void clear();
	
	void set_background_color(const Color&);
	
private:
	int		width;
	int		height;
	
	unsigned int	texture;
	
	Char*		buffer;
	
	Color	background_color;
};

}

#endif
