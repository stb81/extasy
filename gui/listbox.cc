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
 
#include <stdio.h>
#include "basics.h"
#include "widget.h"
#include "listbox.h"
#include "lineedit.h"

extern SDL_Surface* screen;

namespace GUI {

ListBox::ListBox():Widget()
{
	top=0;
	selection=0;
	
	edit_widget=NULL;
	
	double_clicked.connect(sigc::mem_fun(this, &ListBox::edit_entry));
}

ListBox::ListBox(int x0, int y0, int w, int h):Widget(x0,y0,w,h)
{
	top=0;
	selection=0;
	
	edit_widget=NULL;
	
	double_clicked.connect(sigc::mem_fun(this, &ListBox::edit_entry));
}

ListBox::~ListBox()
{
	for (int i=0;i<columns.size();i++)
		delete columns[i];
}

void ListBox::add_column(Column* col)
{
	col->x0=columns.empty() ? 1 : (columns.back()->x0 + columns.back()->width);
	columns.push_back(col);
}

void ListBox::draw()
{
	glUseProgram(0);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	int x0=originx, y0=originy;
	
	glBegin(GL_QUADS);
	glColor4f(0.125,0.1825,0.25,0.5);
	glVertex2i(x0, y0);
	glVertex2i(x0+width, y0);
	glVertex2i(x0+width, y0+height);
	glVertex2i(x0, y0+height);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glColor4f(0,0,0,0.75);
	glVertex2f(x0+1.5, y0+height+0.5);
	glVertex2f(x0+width+0.5, y0+height+0.5);
	glVertex2f(x0+width+0.5, y0+1.5);
	glEnd();
	
	glDisable(GL_BLEND);
	
	glBegin(GL_LINE_LOOP);
	if (has_mouse_focus())
		glColor3f(1,1,1);
	else
		glColor3f(0.6,0.8,1);
			
	glVertex2f(x0+0.5, y0+0.5);
	glVertex2f(x0+width-0.5, y0+0.5);
	glVertex2f(x0+width-0.5, y0+height-0.5);
	glVertex2f(x0+0.5, y0+height-0.5);
	glEnd();
	
	Scissor scissor(originx, originy, width, height);
	
	for (int i=0;i<numentries;i++)
		for (int j=0;j<columns.size();j++)
			if (!edit_widget || edit_row!=i || edit_column!=j)
				columns[j]->draw(originx + columns[j]->x0, originy+(i-top)*16, i, i==selection);
	
	if (edit_widget)
		edit_widget->draw();
}

void ListBox::handle_event(SDL_Event& event)
{
	if (edit_widget)
		edit_widget->handle_event(event);
		
	Widget::handle_event(event);
	
	switch(event.type) {
	case SDL_MOUSEMOTION:
		selected_column=-1;
		if (!has_mouse_focus()) break;
		
		for (int i=0;i<columns.size();i++)
			if (event.motion.x>=columns[i]->x0+originx && event.motion.x<columns[i]->x0+columns[i]->width+originx)
				selected_column=i;
		
		break;
		
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.button==SDL_BUTTON_LEFT && has_mouse_focus()) {
			int sel=(event.button.y - originy) / 16 + top;
			if (sel>=0 && sel<numentries && sel!=selection) {
				selection=sel;
				
				selection_changed(selection);
			}
		}
		if (event.button.button==SDL_BUTTON_WHEELUP && has_mouse_focus() && top>0) {
			top--;
			
			if (edit_widget)
				edit_widget->set_origin(originx+columns[edit_column]->x0, originy+(selection-top)*16-(edit_widget->get_height()-16)/2);
		}
		
		if (event.button.button==SDL_BUTTON_WHEELDOWN && has_mouse_focus() && top+height/16<numentries) {
			top++;
			
			if (edit_widget)
				edit_widget->set_origin(originx+columns[edit_column]->x0, originy+(selection-top)*16-(edit_widget->get_height()-16)/2);
		}
		
		break;
	}
}

void ListBox::move(int dx, int dy)
{
	Widget::move(dx, dy);
	
	if (edit_widget)
		edit_widget->move(dx, dy);
}

void ListBox::edit_entry()
{
	if (edit_widget) return;
	if (selected_column<0) return;
	
	Widget* ed=columns[selected_column]->edit(selection);
	if (!ed) return;
	
	ed->set_origin(originx+columns[selected_column]->x0, originy+(selection-top)*16-(ed->get_height()-16)/2);
	edit_widget=ed;
	edit_row=selection;
	edit_column=selected_column;
}

void ListBox::edit_finished()
{
	delete edit_widget;
	edit_widget=NULL;
}

void ListBox::edit_cancelled()
{
	delete edit_widget;
	edit_widget=NULL;
}

ListBox::Column::Column(int w):width(w)
{
	x0=0;
}

ListBox::Column::~Column()
{
}

Widget* ListBox::Column::edit(int row)
{
	return NULL;
}

ListBox::IndexColumn::IndexColumn():Column(20)
{
}

void ListBox::IndexColumn::draw(int x0, int y0, int row, bool highlighted) const
{
	float c=highlighted ? 1.0f : 0.7f;
	textprintf(x0, y0, c, c, c, "%02X", row);	
}


SimpleListBox::SimpleListBox(int x0, int y0, int w, int h, int initial_entry_count):ListBox(x0, y0, w, h)
{
	add_column(new IndexColumn());
	add_column(new TextColumn(this));
	
	entries.resize(initial_entry_count);
	set_entry_count(initial_entry_count);
	
	for (int i=0;i<initial_entry_count;i++) {
		char* tmp;
		asprintf(&tmp, "Entry %d", i);
		entries[i]=tmp;
	}
}

void SimpleListBox::append_entry(const char* entry)
{
	entries.push_back(entry);
	
	set_entry_count(entries.size());
}

void SimpleListBox::set_entry(int index, const char* entry)
{
	entries[index]=entry;
}

SimpleListBox::TextColumn::TextColumn(SimpleListBox* listbox):Column(listbox->get_width()-22), listbox(listbox)
{
}

void SimpleListBox::TextColumn::draw(int x0, int y0, int row, bool highlighted) const
{
	float c=highlighted ? 1.0f : 0.7f;
	textprint(x0, y0, c, c, c, listbox->entries[row]);
}

Widget* SimpleListBox::TextColumn::edit(int row)
{
	LineEdit* ed=new LineEdit(0, 0, width-2, 24);
	ed->set_text(listbox->entries[row]);
	ed->activate_cursor();
	
	ed->finished.connect(sigc::hide(sigc::bind(sigc::mem_fun(this, &TextColumn::accept_edit), ed, row)));
	ed->lost_input_focus.connect(sigc::hide(sigc::bind(sigc::mem_fun(this, &TextColumn::accept_edit), ed, row)));
	
	ed->finished.connect(sigc::hide(sigc::mem_fun((ListBox*) listbox, &ListBox::edit_finished)));
	ed->cancelled.connect(sigc::hide(sigc::mem_fun((ListBox*) listbox, &ListBox::edit_cancelled)));
	ed->lost_input_focus.connect(sigc::hide(sigc::mem_fun((ListBox*) listbox, &ListBox::edit_finished)));
	
	return ed;
}

void SimpleListBox::TextColumn::accept_edit(LineEdit* ed, int row)
{
	listbox->set_entry(row, strdup(ed->get_text()));
}

}


