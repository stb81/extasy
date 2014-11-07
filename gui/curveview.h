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
	
	CurveView();
	CurveView(int, int, int, int);
	
	virtual void draw();
	
	void set_description(const char* desc)
	{
		description=desc;
	}
	
	void add_curve(Curve* c)
	{
		curves.push_back(c);
	}
	
private:
	const char*	description;

	std::vector<Curve*>	curves;
};

}

#endif
