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
 
#include <stdlib.h>
#include "basics.h"
#include "widget.h"
#include "textedit.h"

namespace GUI {

TextEdit::TextEdit():Widget(), canvas(1, 1)
{
	canvas.set_background_color(Color(0, 0, 0, 128));
	
	update_canvas();
	cursorpos=0;
	selectpos=0;
	error_begin=0;
	error_end=0;
	
	colorizer=new TextColorizer::Default;
}

TextEdit::~TextEdit()
{
	delete colorizer;
}

void TextEdit::set_size(int w, int h)
{
	Widget::set_size(w, h);
	canvas.set_size(w/8, h/16);

	update_canvas();
}

void TextEdit::set_text(const std::string& text)
{
	buffer=text;
	
	update_canvas();
}

const std::string& TextEdit::get_text() const
{
	return buffer;
}

void TextEdit::set_error_highlight(int begin, int end)
{
	error_begin=begin;
	error_end=end;
}

void TextEdit::handle_event(SDL_Event& event)
{
	Widget::handle_event(event);
	
	if (event.type==SDL_KEYDOWN && has_mouse_focus())
		switch (event.key.keysym.sym) {
		case SDLK_LEFT:
			if (cursorpos>0) cursorpos--;
			if (!(event.key.keysym.mod&KMOD_SHIFT)) selectpos=cursorpos;
			break;
		case SDLK_RIGHT:
			if (cursorpos<buffer.size()) cursorpos++;
			if (!(event.key.keysym.mod&KMOD_SHIFT)) selectpos=cursorpos;
			break;
		case SDLK_UP:
			{
				int x, y;
				get_gridpos(cursorpos, x, y);
				cursorpos=get_bufferpos(x, y-1);
				if (!(event.key.keysym.mod&KMOD_SHIFT)) selectpos=cursorpos;
			}
			break;
		case SDLK_DOWN:
			{
				int x, y;
				get_gridpos(cursorpos, x, y);
				cursorpos=get_bufferpos(x, y+1);
				if (!(event.key.keysym.mod&KMOD_SHIFT)) selectpos=cursorpos;
			}
			break;
		case SDLK_HOME:
			{
				int x, y;
				get_gridpos(cursorpos, x, y);
				cursorpos=get_bufferpos(0, y);
				if (!(event.key.keysym.mod&KMOD_SHIFT)) selectpos=cursorpos;
			}
			break;
		case SDLK_END:
			{
				int x, y;
				get_gridpos(cursorpos, x, y);
				cursorpos=get_bufferpos(0x7fffffff, y);
				if (!(event.key.keysym.mod&KMOD_SHIFT)) selectpos=cursorpos;
			}
			break;
		case SDLK_RETURN:
			{
				int x, y, ptr;
				get_gridpos(cursorpos, x, y);	// FIXME: this can be made more efficient
				ptr=get_bufferpos(0, y);
				
				buffer.insert(cursorpos++, 1, '\n');
				while (buffer[ptr]==' ' || buffer[ptr]=='\t')
					buffer.insert(cursorpos++, 1, buffer[ptr++]);
				
				selectpos=cursorpos;
				update_canvas();
			}
			break;
		case SDLK_TAB:
			buffer.insert(cursorpos++, 1, '\t');
			selectpos=cursorpos;
			update_canvas();
			break;
		case SDLK_BACKSPACE:
			if (cursorpos>0) {
				buffer.erase(--cursorpos, 1);
				selectpos=cursorpos;
				update_canvas();
			}
			break;
		case SDLK_DELETE:
			if (selectpos!=cursorpos) {
				buffer.erase(std::min(selectpos, cursorpos), abs(selectpos-cursorpos));
				selectpos=cursorpos=std::min(selectpos, cursorpos);
			}
			else if (cursorpos<buffer.size())
				buffer.erase(cursorpos, 1);
			update_canvas();
			break;
		default:
			if (event.key.keysym.mod&KMOD_CTRL) {
				if (event.key.keysym.sym==SDLK_c)
					clipboard.assign(buffer, std::min(cursorpos, selectpos), abs(cursorpos-selectpos));
				if (event.key.keysym.sym==SDLK_v) {
					buffer.insert(cursorpos, clipboard);
					cursorpos+=clipboard.size();
					selectpos=cursorpos;
					update_canvas();
				}
			}
				
			if (event.key.keysym.unicode>=' ' && event.key.keysym.unicode<127) {
				buffer.insert(cursorpos++, 1, event.key.keysym.unicode);
				selectpos=cursorpos;
				update_canvas();
			}
		}
		
	if (event.type==SDL_MOUSEBUTTONDOWN && event.button.button==SDL_BUTTON_LEFT && has_mouse_focus()) {
		int x=(event.button.x-originx+4)/8;
		int y=(event.button.y-originy)/16;
		selectpos=cursorpos=get_bufferpos(x, y);
	}
	
	if (event.type==SDL_MOUSEMOTION && (event.motion.state&SDL_BUTTON(1)) && has_mouse_focus()) {
		int x=(event.motion.x-originx+4)/8;
		int y=(event.motion.y-originy)/16;
		cursorpos=get_bufferpos(x, y);
	}
}

void TextEdit::draw()
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	int cursorx, cursory;
	get_gridpos(cursorpos, cursorx, cursory);
	
	if (error_begin<error_end) {
		int x0, y0, x1, y1;
		
		get_gridpos(error_begin, x0, y0);
		get_gridpos(error_end, x1, y1);
		
		glUseProgram(0);
		
		glBegin(GL_QUADS);
		glColor3f(1, 0, 0);
		
		if (y0==y1) {
			glVertex2i(originx+x0*8, originy+y0*16);
			glVertex2i(originx+x1*8, originy+y1*16);
			glVertex2i(originx+x1*8, originy+y1*16+16);
			glVertex2i(originx+x0*8, originy+y0*16+16);
		}
		else {
			glVertex2i(originx+x0*8, originy+y0*16);
			glVertex2i(originx+width, originy+y0*16);
			glVertex2i(originx+width, originy+y0*16+16);
			glVertex2i(originx+x0*8, originy+y0*16+16);

			if (y0+1<y1) {
				glVertex2i(originx, originy+y0*16+16);
				glVertex2i(originx+width, originy+y0*16+16);
				glVertex2i(originx+width, originy+y1*16);
				glVertex2i(originx, originy+y1*16);
			}
			
			glVertex2i(originx, originy+y1*16);
			glVertex2i(originx+x1*8, originy+y1*16);
			glVertex2i(originx+x1*8, originy+y1*16+16);
			glVertex2i(originx, originy+y1*16+16);
		}
		
		glEnd();
	}
	
	if (selectpos!=cursorpos) {
		int x0, y0, x1=cursorx, y1=cursory;
		
		get_gridpos(selectpos, x0, y0);
		if (cursorpos<selectpos) {
			std::swap(x0, x1);
			std::swap(y0, y1);
		}
		
		glUseProgram(0);
		
		glBegin(GL_QUADS);
		glColor3f(1, 1, 1);
		
		if (y0==y1) {
			glVertex2i(originx+x0*8, originy+y0*16);
			glVertex2i(originx+x1*8, originy+y1*16);
			glVertex2i(originx+x1*8, originy+y1*16+16);
			glVertex2i(originx+x0*8, originy+y0*16+16);
		}
		else {
			glVertex2i(originx+x0*8, originy+y0*16);
			glVertex2i(originx+width, originy+y0*16);
			glVertex2i(originx+width, originy+y0*16+16);
			glVertex2i(originx+x0*8, originy+y0*16+16);

			if (y0+1<y1) {
				glVertex2i(originx, originy+y0*16+16);
				glVertex2i(originx+width, originy+y0*16+16);
				glVertex2i(originx+width, originy+y1*16);
				glVertex2i(originx, originy+y1*16);
			}
			
			glVertex2i(originx, originy+y1*16);
			glVertex2i(originx+x1*8, originy+y1*16);
			glVertex2i(originx+x1*8, originy+y1*16+16);
			glVertex2i(originx, originy+y1*16+16);
		}
		
		glEnd();
	}
	
	canvas.draw(originx, originy);
	
	glDisable(GL_BLEND);
	
	glUseProgram(0);
	
	glBegin(GL_LINES);
	glColor3f(1, 1, 1);
	glVertex2i(originx+cursorx*8, originy+cursory*16+1);
	glVertex2i(originx+cursorx*8, originy+cursory*16+14);
	glEnd();
}

void TextEdit::update_canvas()
{
	canvas.clear();
	
	int x=0, y=0;
	for (auto i=buffer.begin(); i!=buffer.end();)
		if (*i=='\n')
			y++, x=0, i++;
		else if (*i=='\t')
			x=(x&~3)+4, i++;
		else {
			std::pair<std::string::iterator, Color> token=colorizer->next_token_color(i, buffer.end());
			
			for (; i!=token.first; i++) {
				if (!canvas.is_in_bounds(x, y)) {
					x++;
					continue;
				}
				
				TextCanvas::Char& chr=canvas(x++, y);

				chr.c=*i;
				chr.r=token.second.r;
				chr.g=token.second.g;
				chr.b=token.second.b;
			}
		}
		
	canvas.update();
}

void TextEdit::get_gridpos(int bufpos, int& x, int& y) const
{
	x=y=0;
	
	for (int i=0;i<bufpos;i++)
		if (buffer[i]=='\n')
			x=0, y++;
		else if (buffer[i]=='\t')
			x=(x&~3)+4;
		else
			x++;
}
	
int TextEdit::get_bufferpos(int x, int y) const
{
	if (y<0) return 0;
	
	int cx=0, cy=0;
	
	for (int i=0;i<buffer.size();i++) {
		if (cx==x && cy==y)
			return i;
				
		if (buffer[i]=='\n') {
			if (cy==y)
				return i;
			
			cx=0, cy++;
		}
		else if (buffer[i]=='\t') {
			cx=(cx&~3)+4;
			if (x<cx && y==cy)
				return i;
		}
		else
			cx++;
	}
	
	return buffer.size();
}

void TextEdit::set_colorizer(TextColorizer* col)
{
	delete colorizer;
	colorizer=col;
}

std::pair<std::string::iterator, Color> TextColorizer::Default::next_token_color(std::string::iterator pos, std::string::iterator end) const
{
	return std::pair<std::string::iterator, Color>(++pos, Color(170, 170, 170));
}
	
}
