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
#include "mainwindow.h"

namespace GUI {
	
Scissor* Scissor::cur_scissor=NULL;

Scissor::Scissor(int x, int y, int w, int h)
{
	prev_scissor=cur_scissor;
	cur_scissor=this;

	if (prev_scissor) {
		x0=std::max(prev_scissor->x0, x);
		y0=std::max(prev_scissor->y0, y);
		x1=std::min(prev_scissor->x1, x+w);
		y1=std::min(prev_scissor->y1, y+h);
	}
	else
		x0=x, y0=y, x1=x+w, y1=y+h;
	
	glEnable(GL_SCISSOR_TEST);
	set();
}

Scissor::~Scissor()
{
	if (cur_scissor=prev_scissor)
		cur_scissor->set();
	else
		glDisable(GL_SCISSOR_TEST);
}

void Scissor::set() const
{
	glScissor(x0, /*screen->h*/1050-y1, x1-x0, y1-y0);
}


Widget::Widget()
{
	originx=0;
	originy=0;
	width=0;
	height=0;
	
	flags=VISIBLE;
	layer=0;
	
	last_click_time=0;
}

Widget::Widget(int x, int y, int w, int h)
{
	originx=x;
	originy=y;
	width=w;
	height=h;
	
	flags=VISIBLE;
	layer=0;
	
	last_click_time=0;
}

Widget::~Widget()
{
}

void Widget::set_origin(int x, int y)
{
	move(x-originx, y-originy);
}

void Widget::set_size(int w, int h)
{
	width=w;
	height=h;
}

void Widget::draw()
{
}

void Widget::handle_event(SDL_Event& event)
{
	switch(event.type) {
	case SDL_MOUSEMOTION:
		if (flags&MODAL) {
			flags|=HAS_FOCUS;
			event.motion.state|=SDL_FOCUS_CLAIMED;
		}
		else if (!(event.motion.state&SDL_BUTTON(1)) || (flags&MOUSE_DOWN)) {
			bool focus=!(event.motion.state&SDL_FOCUS_CLAIMED) && contains_point(event.motion.x, event.motion.y);
			assign_bit<unsigned>(flags, HAS_FOCUS|HAS_FOCUS_EXCLUSIVE, focus);
			
			if (focus)
				event.motion.state|=SDL_FOCUS_CLAIMED;
		}
		
		break;
		
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.button==SDL_BUTTON_LEFT && has_mouse_focus()) {
			if (parent)
				parent->bring_to_front(this);
				
			unsigned int now=SDL_GetTicks();
			if (now < last_click_time + 250 && has_exclusive_focus())
				double_clicked();
			
			last_click_time=now;
		}
		
		if (event.button.button==SDL_BUTTON_LEFT)
			assign_bit<unsigned>(flags, MOUSE_DOWN, has_mouse_focus());

		break;
		
	case SDL_MOUSEBUTTONUP:
		if (event.button.button==SDL_BUTTON_LEFT && (flags&MOUSE_DOWN)) {
			flags&=~MOUSE_DOWN;
			
			if (has_exclusive_focus())
				clicked();
		}
		break;
	}
}

MainWindow* Widget::get_root_window()
{
	return parent ? parent->get_root_window() : nullptr;
}

void Widget::dispatch(std::function<void()> fn)
{
	if (parent)
		parent->dispatch(fn);
	else
		fprintf(stderr, "dispatch() called in stale widget");
}

void Widget::send_notification(userevent_t notification, void* obj, int code)
{
	SDL_Event event;
	event.type=notification;
	event.user.code=code;
	event.user.data1=this;
	event.user.data2=obj;

	SDL_PushEvent(&event);
}

void Widget::move(int dx, int dy)
{
	originx+=dx;
	originy+=dy;
}


Container::Container()
{
}

Container::~Container()
{
}

void Container::add(Widget* w)
{
	w->parent=this;
	w->move(originx, originy);
}

void Container::remove(Widget* w)
{
	w->parent=nullptr;
}

void Container::bring_to_front(Widget* w)
{
}

bool Container::is_child_in_front(const Widget* w) const
{
	return false;
}


Group::Group()
{
}

Group::Group(int x, int y, int w, int h)
{
	set_origin(x, y);
	set_size(w, h);
}

Group::~Group()
{
	for (int i=0;i<children.size();i++)
		if (children[i]->flags&DELETE_RECURSIVELY)
			delete children[i];
}

void Group::handle_event(SDL_Event& event)
{
	SDL_Event tmp=event;
	
	Container::handle_event(event);
	
	for (int i=0;i<children.size();i++)
		if (children[i]->flags & VISIBLE)
			children[i]->handle_event(tmp);
			
	if (tmp.type==SDL_MOUSEMOTION && (tmp.motion.state&SDL_FOCUS_CLAIMED))
		assign_bit<unsigned>(flags, HAS_FOCUS_EXCLUSIVE, false);
}

void Group::draw()
{
	Container::draw();

	for (int i=children.size()-1;i>=0;i--)
		if (children[i]->flags & VISIBLE)
			children[i]->draw();
}

void Group::move(int dx, int dy)
{
	Container::move(dx, dy);

	for (int i=0;i<children.size();i++)
		children[i]->move(dx, dy);
}

void Group::add(Widget& w)
{
	Container::add(&w);
	
	children.push_back(&w);
}

void Group::add(Widget* w)
{
	Container::add(w);
	
	children.push_back(w);
	w->flags|=DELETE_RECURSIVELY;
	
	std::stable_sort(children.begin(), children.end(), [](Widget* a, Widget* b) { return a->layer>b->layer; });
}

void Group::remove(Widget* w)
{
	Container::remove(w);
	
	children.erase(std::remove(children.begin(), children.end(), w), children.end());
	w->flags&=~DELETE_RECURSIVELY;
}

void Group::bring_to_front(Widget* w)
{
	std::remove(children.rbegin(), children.rend(), w);
	children[0]=w;
	
	std::stable_sort(children.begin(), children.end(), [](Widget* a, Widget* b) { return a->layer>b->layer; });
}

bool Group::is_child_in_front(const Widget* w) const
{
	return children[0]==w;
}


Label::Label():Widget()
{
	text="";
}

Label::Label(int x0, int y0, int w, int h, const char* t):Widget(x0, y0, w, h)
{
	text=t ? t : "";
}

void Label::draw()
{
	textprint(originx, originy, 1, 1, 1, text);
}


class Draggable::DragObject:public Widget {
	Draggable&	draggable;
	
public:
	DragObject(Draggable& d):draggable(d)
	{
		layer=1000;
		
		set_size(d.width, d.height);
		set_origin(d.originx, d.originy);
	}
	
	virtual void draw();
	virtual void handle_event(SDL_Event&);
};

void Draggable::DragObject::draw()
{
	draggable.draw_dragged(originx, originy);
}

void Draggable::DragObject::handle_event(SDL_Event& event)
{
	Widget::handle_event(event);

	if (event.type==SDL_MOUSEBUTTONUP)
		send_notification(SDL_DRAG_DROP_EVENT, &draggable);
		
	if (event.type==SDL_MOUSEMOTION)
		move(event.motion.xrel, event.motion.yrel);
		
	if (event.type==SDL_DRAG_DROP_EVENT && event.user.data1==this)
		dispatch([this]() { parent->remove(this); delete this; });
}


void Draggable::handle_event(SDL_Event& event)
{
	Widget::handle_event(event);
	
	if (event.type==SDL_MOUSEBUTTONDOWN && event.button.button==SDL_BUTTON_LEFT && has_mouse_focus())
		dispatch([this]() { get_root_window()->add(new DragObject(*this)); });
}

}

