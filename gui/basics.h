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
 
#ifndef INCLUDE_BASICS_H
#define INCLUDE_BASICS_H

#include <SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <sigc++/sigc++.h>
/*
#define SDL_AUDIO_CONSUMED	(SDL_USEREVENT + 1)
#define SDL_MIDI_EVENT		(SDL_USEREVENT + 2)
#define SDL_DRAG_DROP_EVENT	(SDL_USEREVENT + 3)
#define SDL_PATTERN_CHANGED	(SDL_USEREVENT + 4)
*/

enum userevent_t {
	SDL_FIRST_USEREVENT=SDL_USEREVENT,
	SDL_AUDIO_CONSUMED,
	SDL_MIDI_EVENT,
	SDL_DRAG_DROP_EVENT,
	SDL_PATTERN_CHANGED
};

namespace GUI {
	
struct Color {
	unsigned char r, g, b, a;
	
	Color() {}
	Color(unsigned char r, unsigned char g, unsigned char b):r(r), g(g), b(b), a(255) {}
	Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a):r(r), g(g), b(b), a(a) {}
	
	Color(uint32_t col)
	{
		r=(col>>16) & 0xff;
		g=(col>>8) & 0xff;
		b=col & 0xff;
		a=col>>24;
	}
	
	operator uint32_t() const
	{
		return (a<<24) | (r<<16) | (g<<8) | b;
	}
	
	static Color mix(Color c1, Color c2, int v)
	{
		int u=256-v;
		
		return Color((c1.r*u+c2.r*v+128)>>8, (c1.g*u+c2.g*v+128)>>8, (c1.b*u+c2.b*v)>>8);
	}

	Color shade(int v)
	{
		Color c1=mix(Color(0), *this, v);
		Color c3=mix(*this, Color(0xffffff), v);
		return mix(mix(c1, *this, v), mix(*this, c3, v), v);
	}
};

template<typename T>
void assign_bit(T& bitfield, T bit, bool state)
{
	if (state)
		bitfield|=bit;
	else
		bitfield&=~bit;
}

template<typename T>
struct first_nonzero_accumulator {
	typedef T result_type;
	template<typename I>
	T operator()(I first, I last) const
	{
		for (; first!=last; first++)
			if (*first!=NULL)
				return *first;
		
		return NULL;
	}
};

class Scissor {
	static Scissor*	cur_scissor;
	Scissor*		prev_scissor;
	
	int		x0, y0, x1, y1;
	
	void set() const;
	
public:
	Scissor(int, int, int, int);
	~Scissor();
	
	bool empty() const
	{
		return x0>=x1 || y0>=y1;
	}
};

}

#endif

