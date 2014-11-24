#include <assert.h>
#include "basics.h"
#include "widget.h"
#include "scrollpane.h"

namespace GUI {

ScrollPane::ScrollPane(bool scrollh, bool scrollv):scrollh(scrollh), scrollv(scrollv)
{
}

ScrollPane::~ScrollPane()
{
	delete child;
}

void ScrollPane::set_scroll_position(int sx, int sy)
{
	scrollposx=scrollh ? sx : 0;
	scrollposy=scrollv ? sy : 0;

	scrollposx=std::min(scrollposx, child->get_width()-width);
	scrollposy=std::min(scrollposy, child->get_height()-height);
	if (scrollposx<0) scrollposx=0;
	if (scrollposy<0) scrollposy=0;
	
	child->set_origin(originx-scrollposx, originy-scrollposy);
}

void ScrollPane::draw()
{
	if (!child) return;
	
	Scissor scissor(originx, originy, width, height);
	child->draw();
}

void ScrollPane::handle_event(SDL_Event& event)
{
	SDL_Event tmp=event;
	
	Container::handle_event(event);

	if (event.type==SDL_MOUSEBUTTONDOWN && event.button.button==SDL_BUTTON_WHEELDOWN && scrollv) {
		set_scroll_position(scrollposx, scrollposy+16);
		scroll_position_changed(scrollposx, scrollposy);
	}

	if (event.type==SDL_MOUSEBUTTONDOWN && event.button.button==SDL_BUTTON_WHEELUP && scrollv) {
		set_scroll_position(scrollposx, scrollposy-16);
		scroll_position_changed(scrollposx, scrollposy);
	}
	
	if (event.type==SDL_MOUSEBUTTONDOWN && event.button.button==SDL_BUTTON_RIGHT)
		scrolling=has_mouse_focus();
	if (event.type==SDL_MOUSEBUTTONUP && event.button.button==SDL_BUTTON_RIGHT)
		scrolling=false;
	
	if (event.type==SDL_MOUSEMOTION && scrolling) {
		set_scroll_position(scrollposx-event.motion.xrel, scrollposy-event.motion.yrel);
		scroll_position_changed(scrollposx, scrollposy);
	}

	if (child)
		child->handle_event(tmp);
}

void ScrollPane::add(Widget* w)
{
	assert(child==nullptr);
	child=w;
}

void ScrollPane::remove(Widget* w)
{
	assert(child==w);
	child=nullptr;
}

bool ScrollPane::is_child_in_front(const Widget* w) const
{
	return child==w;
}

}
