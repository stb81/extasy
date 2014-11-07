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
 
#ifndef INCLUDE_COLORLABELCHOOSER_H
#define INCLUDE_COLORLABELCHOOSER_H

namespace GUI {

class ColorLabelButton:public RadioButton {
public:
	ColorLabelButton(RadioButtonGroup&);
	virtual ~ColorLabelButton();
	
	virtual void draw();
	
	void set_color(const Color& c)
	{
		color=c;
	}
	
	Color get_color() const
	{
		return color;
	}
	
private:
	Color	color;
	
	static int shader;
};

class ColorLabelChooser:public Group {
	RadioButtonGroup	group;
	ColorLabelButton**	buttons;
	
public:
	ColorLabelChooser();
	~ColorLabelChooser();
	
	void set_grid(int, int);
	
	sigc::signal<void, Color>	color_picked;
	
private:
	void color_label_clicked(ColorLabelButton*);
};

}

#endif
