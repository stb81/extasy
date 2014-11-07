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
 
#ifndef INCLUDE_WIDGET_H
#define INCLUDE_WIDGET_H

#include <functional>
#include <vector>

#include "text.h"


#define SDL_FOCUS_CLAIMED	0x80


namespace GUI {

class Container;	
class MainWindow;


class Widget:public sigc::trackable {
	friend class Container;
	friend class Group;
	
public:
	Widget();
	Widget(int, int, int, int);
	virtual ~Widget();
	
	virtual void set_origin(int, int);
	virtual void set_size(int, int);
	
	void set_visible(bool visible)
	{
		assign_bit<unsigned int>(flags, VISIBLE, visible);
	}
	
	virtual void handle_event(SDL_Event&);
	virtual void draw();
	virtual void move(int, int);
	
	bool is_in_front() const;

	bool has_mouse_focus() const
	{
		return !!(flags & HAS_FOCUS);
	}
	
	bool has_exclusive_focus() const
	{
		return !!(flags & HAS_FOCUS_EXCLUSIVE);
	}
	
	int get_width() const
	{
		return width;
	}
	
	int get_height() const
	{
		return height;
	}
	
	int get_originx() const
	{
		return originx;
	}
	
	int get_originy() const
	{
		return originy;
	}
	
	virtual MainWindow* get_root_window();
	
	sigc::signal<void>	clicked;
	sigc::signal<void>	double_clicked;
	
protected:
	enum {
		VISIBLE=1,
		FOCUSABLE=2,
		HAS_FOCUS=4,
		HAS_FOCUS_EXCLUSIVE=8,
		MOUSE_DOWN=16,
		MODAL=32,
		DELETE_RECURSIVELY=64,
	};
	
	bool contains_point(int x, int y) const
	{ return x>=originx && y>=originy && x<(originx+width) && y<(originy+height); }
	
	virtual void dispatch(std::function<void()>);
	
	void send_notification(userevent_t notification, void* obj=nullptr, int code=0);
	
	Container*		parent=nullptr;
	
	int	originx;
	int	originy;
	int	width;
	int	height;
	
	unsigned int	flags;
	int				layer;
	
private:
	unsigned int	last_click_time;
};


class Container:public Widget {
public:
	Container();
	virtual ~Container();
	
	virtual void add(Widget*);
	virtual void remove(Widget*);
	
	virtual void bring_to_front(Widget*);
	virtual bool is_child_in_front(const Widget* w) const;
};


class Group:public Container {
public:
	Group();
	Group(int, int, int, int);
	virtual ~Group();

	virtual void handle_event(SDL_Event&);
	virtual void draw();
	virtual void move(int, int);
	
	virtual void add(Widget*);
	virtual void remove(Widget*);
	
	virtual void bring_to_front(Widget*);
	virtual bool is_child_in_front(const Widget* w) const;
	
protected:
	// special variant for subwidgets which are not to be automatically deleted recursively
	void add(Widget&);

private:
	std::vector<Widget*>	children;
};


inline bool Widget::is_in_front() const
{ return parent!=0 && parent->is_child_in_front(this); }


class Label:public Widget {
public:
	Label();
	Label(int, int, int, int, const char* text=NULL);
	
	virtual void draw();
	
	void set_text(const char* t)
	{
		text=t;
	}
	
private:
	const char*	text;
};


class Draggable:public Widget {
	class DragObject;
	
protected:
	virtual void draw_dragged(int, int) =0;
	
public:
	virtual void handle_event(SDL_Event&);
};


template<typename TDraggable>
class DropReceiver {
protected:
	void handle_event(SDL_Event& event)
	{
		if (event.type==SDL_DRAG_DROP_EVENT) {
			Widget* dragobj=static_cast<Widget*>(event.user.data1);
			TDraggable* draggable=dynamic_cast<TDraggable*>(static_cast<Draggable*>(event.user.data2));
			
			if (draggable)
				on_drop_event(draggable, dragobj->get_originx(), dragobj->get_originy());
		}
	}
	
	virtual void on_drop_event(TDraggable*, int, int) =0;
};

}

#endif


