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
 
#ifndef INCLUDE_ARRANGEMENTEDIT_H
#define INCLUDE_ARRANGEMENTEDIT_H

namespace Synth {
	class Module;
	class Pattern;
}


class PatternListBlock;

template<typename TItem>
class PatternListRow;

template<typename TItem>
class PatternListInsertionPost;

template<typename TContainer>
class PatternListEdit:public GUI::Container {
	std::list<GUI::Widget*>	entry_widgets;
	
protected:
	typedef typename TContainer::value_type TItem;
	
	int	highlighted_row;
	
	Synth::Module*	module;
	TContainer*		list;
	

	PatternListEdit();

	void set_list(TContainer*);

private:
	void on_double_click_entry(PatternListRow<TItem>*);
	void on_double_click_post(PatternListInsertionPost<TItem>*);
	
	TItem create_new_item() const;
	
	virtual void add(Widget*);

public:
	virtual void draw();
	virtual void handle_event(SDL_Event&);
	
	virtual void move(int, int);
	
	sigc::signal<void, Synth::Pattern*> select_pattern;
};


class ArrangementEdit:public PatternListEdit<std::list<Synth::Module::arrangement_item_t>> {
public:
	ArrangementEdit();
	
	void set_module(Synth::Module*);
};


class PatternStockEdit:public PatternListEdit<std::vector<Synth::Pattern*>> {
public:
	PatternStockEdit();
	
	void set_module(Synth::Module*);
};

#endif
