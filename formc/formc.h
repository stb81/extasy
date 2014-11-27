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
 
#ifndef INCLUDE_FORMC_H
#define INCLUDE_FORMC_H

#include <vector>
#include <string>

enum alignment_t {
	ALIGN_EXPAND,
	ALIGN_LEFT,
	ALIGN_RIGHT,
	ALIGN_TOP,
	ALIGN_BOTTOM,
	ALIGN_CENTER
};

struct argument_t {
	char*	type;
	char*	name;
	char*	value;
	
	argument_t() {}
	
	argument_t(char* t, char* n, char* v)
	{
		type=t;
		name=n;
		value=v;
	}
};

typedef std::vector<argument_t*> arglist_t;

class namecounter_t {
	int counter;
	
public:
	namecounter_t();
	
	int operator()();
	
	const char* get_upper();
	const char* get_lower();
};

struct LayoutItem;

struct SignalHandler {
	LayoutItem*	item;
	char*		signal_name;
	arglist_t*	args;
	char*		handler_method;
	char*		body;
	
	std::string	handler_name;
	
	void construct_handler_name();
};

struct LayoutItem {
	char*	varname;
	char*	args;
	
	int		x0, y0, width, height;

	alignment_t	halign;
	alignment_t	valign;

	std::vector<SignalHandler*>	signal_handlers;
	
	LayoutItem();
	
	virtual void add_child(LayoutItem*)=0;
	virtual void move(int, int);
	virtual void print_init()=0;
	virtual void init_names(namecounter_t&)=0;
	virtual void fix_alignment()=0;
	
	void print_code(const char*, FILE*);
};


struct LayoutStrut:LayoutItem {
	virtual void add_child(LayoutItem*);
	virtual void print_init();
	virtual void init_names(namecounter_t&);
	virtual void fix_alignment();
	
	LayoutStrut(int, int);
};


struct LayoutElement:LayoutItem {
	char*			element_type;
	
	std::string		init_code;
	
	virtual void add_child(LayoutItem*);
	virtual void print_init();
	virtual void init_names(namecounter_t&);
	virtual void fix_alignment();
};


struct LayoutBox:LayoutItem {
	enum box_t { HBOX, VBOX };
	
	box_t						orientation;
	std::vector<LayoutItem*>	children;
	
	int		padding;
	
	LayoutBox();
	
	virtual void add_child(LayoutItem*);
	virtual void move(int, int);
	virtual void print_init();
	virtual void init_names(namecounter_t&);
	virtual void fix_alignment();
};


struct FreeLayout:LayoutItem {
	std::vector<LayoutItem*>	children;
	
	FreeLayout();
	
	virtual void add_child(LayoutItem*);
	virtual void move(int, int);
	virtual void print_init();
	virtual void init_names(namecounter_t&);
	virtual void fix_alignment();
};

struct Form:LayoutElement {
	struct FormMethod {
		bool		is_static;
		char*		name;
		arglist_t*	args;
		char*		body;
		char*		rettype;
	};
	
	std::vector<std::pair<char*,arglist_t*> >	extensions;
	std::vector<std::pair<LayoutElement*, std::string*> >	constructors;
	std::vector<FormMethod*>					form_methods;
	std::vector<char*>							subclasses;
	
	namecounter_t	namecounter;
	
	LayoutItem*	child;
	
	Form();
	
	void print();
	
	virtual void add_child(LayoutItem*);
	virtual void print_init();
	virtual void init_names(namecounter_t&);
	virtual void fix_alignment();
};


extern FILE* out_header;
extern FILE* out_source;

#endif
