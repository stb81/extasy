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
 
#ifndef INCLUDE_CURVEVIEW_H
#define INCLUDE_CURVEVIEW_H

namespace GUI {

class CurveView:public Widget {
public:
	class Curve {
		Color	color;

	public:
		Curve()
		{
			color=Color(255, 255, 255);
		}
		
		void set_color(const Color& c)
		{
			color=c;
		}
		
		const Color& get_color() const
		{
			return color;
		}
		
		float operator()(float x) const
		{
			return evaluate(x);
		}
		
	protected:
		virtual float evaluate(float) const =0;
	};
	
	template<typename Fn>
	class TCurve:public Curve {
		Fn	fn;
		
		virtual float evaluate(float x) const
		{
			return fn(x);
		}
		
	public:
		TCurve(Fn fn):fn(fn) {}
	};
	
	CurveView();
	CurveView(int, int, int, int);
	
	virtual void draw();
	virtual void handle_event(SDL_Event&);
	
	void set_description(const char* desc)
	{
		description=desc;
	}
	
	void add_curve(Curve* c)
	{
		curves.push_back(c);
	}
	
	template<typename Fn>
	void add_curve(Fn fn, Color color)
	{
		Curve* curve=new TCurve<Fn>(fn);
		curve->set_color(color);
		add_curve(curve);
	}
	
	void add_vertical_line(float pos, Color color, int id=-1, const char* label=nullptr);
	void remove_vertical_line(int id);
	
	void set_vertical_line_position(int id, float pos);
	void set_vertical_line_color(int id, Color color);
	void set_vertical_line_label(int id, const char* label);
	
	int get_selected_vertical_line() const;
	
	sigc::signal<void, int, float> vertical_line_dragged;
	
private:
	struct vline_t {
		float		position;
		Color		color;
		int			id;
		std::string	label;
	};
	
	const char*				description;

	std::vector<Curve*>		curves;
	std::vector<vline_t>	vlines;
	
	vline_t*				vline_highlight=nullptr;
	bool					vline_dragging=false;
};

}

#endif
