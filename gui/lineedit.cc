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
#include "lineedit.h"

namespace GUI {

LineEdit::LineEdit():Widget()
{
	cursor=-1;
	scrollpos=0;
}

LineEdit::LineEdit(int x0, int y0, int w, int h):Widget(x0, y0, w, h)
{
	cursor=-1;
	scrollpos=0;
}

LineEdit::~LineEdit()
{
}

void LineEdit::set_text(const char* t)
{
	text=t;
}

void LineEdit::adjust_scroll_position()
{
	if (cursor<0)
		scrollpos=0;
	else {
		int cursorpos=default_font->get_text_width(text.begin(), text.begin()+cursor);
		
		if (cursorpos<scrollpos)
			scrollpos=cursorpos;
			
		if (cursorpos>scrollpos+width-8)
			scrollpos=cursorpos-width+8;
	}	
}

void LineEdit::draw()
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
	
	Scissor scissor(x0+2, y0+2, width-4, height-4);
	textprint(x0+4-scrollpos, y0+(height-16)/2, 1, 1, 1, get_text());
	
	if (cursor>=0) {
		int cursorpos=default_font->get_text_width(text.begin(), text.begin()+cursor);
		
		glUseProgram(0);
		
		glBegin(GL_LINES);
		glColor3f(1, 1, 1);
		glVertex2f(x0+4+cursorpos-scrollpos, y0+3.5f);
		glVertex2f(x0+4+cursorpos-scrollpos, y0+height-3.5f);
		glEnd();
	}
}

void LineEdit::handle_event(SDL_Event& event)
{
	Widget::handle_event(event);
	
	if (event.type==SDL_KEYDOWN && cursor>=0) {
		if (event.key.keysym.unicode>=' ' && event.key.keysym.unicode<127) {
			char key=event.key.keysym.unicode;
			text.insert(cursor++, &key, 1);
			text_changed(text);
		}
		
		if (event.key.keysym.sym==SDLK_BACKSPACE && cursor>0) {
			text.erase(--cursor, 1);
			text_changed(text);
		}
		
		if (event.key.keysym.sym==SDLK_DELETE && cursor<text.length()) {
			text.erase(cursor, 1);
			text_changed(text);
		}
			
		if (event.key.keysym.sym==SDLK_LEFT && cursor>0) cursor--;
		if (event.key.keysym.sym==SDLK_RIGHT && cursor<text.length()) cursor++;
		
		if (event.key.keysym.sym==SDLK_k && (event.key.keysym.mod&KMOD_CTRL)!=0) {
			text.erase();
			text_changed(text);
			cursor=0;
		}
		
		if (event.key.keysym.sym==SDLK_RETURN) {
			cursor=-1;
			finished(this);
		}
		
		if (event.key.keysym.sym==SDLK_ESCAPE) {
			cursor=-1;
			cancelled(this);
		}
			
		adjust_scroll_position();
	}
	
	if (event.type==SDL_MOUSEBUTTONDOWN && event.button.button==SDL_BUTTON_LEFT) {
		if (has_mouse_focus()) {
			/*cursor=(event.button.x-originx+scrollpos)/8;
			if (cursor<0) cursor=0;
			if (cursor>text.length()) cursor=text.length();*/
			
			int cursorpos=event.button.x-originx+scrollpos-4;
			
			for (cursor=0;cursor<text.size() && cursorpos>0;cursor++) {
				cursorpos+=default_font->get_glyph_kern(text[cursor]);
				cursorpos-=default_font->get_glyph_width(text[cursor]);
			}
		}
		else {
			if (cursor>=0)
				lost_input_focus(this);
			
			cursor=-1;
		}
			
		adjust_scroll_position();
	}
}

}


