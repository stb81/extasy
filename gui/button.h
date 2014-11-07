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
 
#ifndef INCLUDE_BUTTON_H
#define INCLUDE_BUTTON_H

namespace GUI {

class Button:public Widget {
public:
	Button();
	Button(int, int, int, int, const char*);
	virtual ~Button();
	
	virtual void draw();
	virtual void handle_event(SDL_Event& event);
	
	void set_label(const char* l);
	const char* get_label() const { return label; }
	
	void set_hotkey(SDLKey, SDLMod mod=KMOD_NONE);
	
protected:
	const char*	label;
	
private:
	SDLKey	hotkey_sym;
	SDLMod	hotkey_mod;
};


class CheckBox:public Button {
public:
	CheckBox();
	CheckBox(int, int, int, int, const char*);
	virtual ~CheckBox();
	
	virtual void draw();

	bool is_checked() const;
	void set_checked(bool);
	void toggle();
	
private:
	bool	checked;
};


class RadioButton;

class RadioButtonGroup {
	friend class RadioButton;

	RadioButton*	selected;
	int				next_index;
	
public:
	RadioButtonGroup()
	{
		selected=nullptr;
		next_index=0;
	}
	
	RadioButton* get_selected() const
	{
		return selected;
	}
	
	int get_selected_index() const;
};


class RadioButton:public Button {
public:
	RadioButton(RadioButtonGroup&);
	RadioButton(int, int, int, int, const char*, RadioButtonGroup&);
	virtual ~RadioButton();
	
	virtual void draw();
	
	bool is_selected() const;
	void select();
	
	int get_index() const
	{
		return index;
	}
	
private:
	RadioButtonGroup&	group;
	int					index;
};

}

#endif
