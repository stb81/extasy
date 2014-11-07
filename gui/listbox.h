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
 
#ifndef INCLUDE_LISTBOX_H
#define INCLUDE_LISTBOX_H

namespace GUI {

class LineEdit;

class ListBox:public Widget {
public:
	class Column {
		friend class ListBox;
		
	public:
		Column(int);
		virtual ~Column();
		
	protected:
		int		width;
		
		virtual void draw(int x0, int y0, int row, bool highlighted) const =0;
		virtual Widget* edit(int row);
		
	private:
		int		x0;
	};
	
	class IndexColumn:public Column {
		virtual void draw(int, int, int, bool) const;
		
	public:
		IndexColumn();
	};
	
	ListBox();
	ListBox(int, int, int, int);
	virtual ~ListBox();
	
	void add_column(Column*);

	void set_entry_count(int c)
	{
		numentries=c;
	}
	
	int get_entry_count() const
	{
		return numentries;
	}
	
	void set_selection(int index)
	{
		selection=index;
	}
	
	int get_selection() const
	{
		return selection; 
	}
	
	virtual void handle_event(SDL_Event&);
	virtual void draw();
	virtual void move(int, int);
	
	sigc::signal<void, int> selection_changed;
	sigc::signal<void, int, const char*>	entry_edited;

	void	edit_entry();
	void	edit_finished();
	void	edit_cancelled();
	
private:
	std::vector<Column*>		columns;
	
	int				numentries;
	int				top;
	int				selection;
	int				selected_column;
	
	Widget*			edit_widget;
	int				edit_row;
	int				edit_column;
};

class SimpleListBox:public ListBox {
	class TextColumn:public Column, public sigc::trackable {
		SimpleListBox*	listbox;
		
		virtual void draw(int, int, int, bool) const;
		virtual Widget* edit(int);
		
		void accept_edit(LineEdit*, int);
		
	public:
		TextColumn(SimpleListBox*);
	};
	
public:
	SimpleListBox(int x, int y, int w, int h, int initial_entry_count=0);
	
	void append_entry(const char*);
	void set_entry(int, const char*);
	
private:
	std::vector<const char*>	entries;
};

}

#endif
