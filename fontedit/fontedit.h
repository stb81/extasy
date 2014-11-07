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
 
#ifndef INCLUDE_FONTEDIT_H
#define INCLUDE_FONTEDIT_H

class FontEditBase {
public:
	FontEditBase();
	
protected:
	int selected_glyph;
	
	void shift_left();
	void shift_right();
};

class FontMatrix:public GUI::Widget {
public:
	FontMatrix();
	
	virtual void draw();
	virtual void handle_event(SDL_Event&);
	
	int get_selected() const
	{
		return selected;
	}
	
	sigc::signal<void, int> glyph_selected;
	
private:
	int	selected;
	int	highlighted;
	
	void on_clicked();
};

class GlyphMatrix:public GUI::Widget {
public:
	GlyphMatrix();
	
	virtual void draw();
	virtual void handle_event(SDL_Event&);

	void set_font(GUI::Font* f)
	{
		font=f;
		set_size(font->get_max_glyph_width()*32, font->get_max_glyph_height()*32);
	}
	
	void set_glyph(int g)
	{
		glyph=g;
	}

private:
	int		highlighted;
	int		draw_color;

	GUI::Font*	font;
	int		glyph;
};

#endif
