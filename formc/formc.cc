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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "formc.h"

FILE* out_header;
FILE* out_source;

void yyerror(const char* err)
{
	printf("Parse error: %s\n", err);
	exit(1);
}

int yyparse();


namecounter_t::namecounter_t()
{
	counter=0;
}

int namecounter_t::operator()()
{
	counter=(13*counter + 4913) % 46656;
	return counter;
}

const char* namecounter_t::get_upper()
{
	const static char digits[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	
	int name=(*this)();
	
	static char tmp[4];
	
	tmp[0]=digits[name%36];
	tmp[1]=digits[(name/36)%36];
	tmp[2]=digits[name/1296];
	tmp[3]=0;
	
	return tmp;
}

const char* namecounter_t::get_lower()
{
	const static char digits[]="0123456789abcdefghijklmnopqrstuvwxyz";
	
	int name=(*this)();
	
	static char tmp[4];
	
	tmp[0]=digits[name%36];
	tmp[1]=digits[(name/36)%36];
	tmp[2]=digits[name/1296];
	tmp[3]=0;
	
	return tmp;
}


void SignalHandler::construct_handler_name()
{
	if (handler_method)
		handler_name=handler_method;
	else {
		handler_name="on_";
		handler_name+=item->varname;
		handler_name+="_";
		handler_name+=signal_name;
	}
}


LayoutItem::LayoutItem()
{
	varname=NULL;
	args=NULL;
	
	x0=y0=width=height=0;
	
	halign=ALIGN_LEFT;
	valign=ALIGN_TOP;
}

void LayoutItem::move(int dx, int dy)
{
	x0+=dx;
	y0+=dy;
}

void LayoutItem::print_code(const char* code, FILE* file)
{
	for (;*code;code++)
		if (*code=='@')
			fputs(varname, file);
		else
			putc(*code, file);
}


LayoutStrut::LayoutStrut(int w, int h)
{
	width=w;
	height=h;
}

void LayoutStrut::add_child(LayoutItem* item)
{
	fprintf(stderr, "Error: A strut cannot take any children!\n");
}

void LayoutStrut::print_init()
{
}

void LayoutStrut::init_names(namecounter_t& namecnt)
{
}

void LayoutStrut::fix_alignment()
{
}


void LayoutElement::add_child(LayoutItem* item)
{
	fprintf(stderr, "Error: Layout element '%s' cannot take any children!\n", element_type);
}

void LayoutElement::print_init()
{
	fprintf(out_header, "\t%s\t%s;\n", element_type, varname);
	fprintf(out_source, "\t%s.set_origin(%d, %d);\n", varname, x0, y0);
	fprintf(out_source, "\t%s.set_size(%d, %d);\n", varname, width, height);
	
	print_code(init_code.c_str(), out_source);
	
	fprintf(out_source, "\tadd(%s);\n\n", varname);
}

void LayoutElement::init_names(namecounter_t& namecnt)
{
	if (!varname)
		asprintf(&varname, "anonymous_element_%s", namecnt.get_lower());
}

void LayoutElement::fix_alignment()
{
}


LayoutBox::LayoutBox()
{
	padding=16;
}

void LayoutBox::add_child(LayoutItem* item)
{
	children.push_back(item);
	
	if (orientation==HBOX) {
		if (children.size()>1)
			width+=padding;
			
		item->move(width, 0);
		
		width+=item->width;
		height=std::max(height, item->height);
	}
	else {
		if (children.size()>1)
			height+=padding;
			
		item->move(0, height);
		
		width=std::max(width, item->width);
		height+=item->height;
	}
}

void LayoutBox::move(int dx, int dy)
{
	LayoutItem::move(dx, dy);
	
	for (int i=0;i<children.size();i++)
		children[i]->move(dx, dy);
}

void LayoutBox::print_init()
{
	for (int i=0;i<children.size();i++)
		children[i]->print_init();
}

void LayoutBox::init_names(namecounter_t& namecnt)
{
	for (int i=0;i<children.size();i++)
		children[i]->init_names(namecnt);
}

void LayoutBox::fix_alignment()
{
	for (int i=0;i<children.size();i++) {
		LayoutItem* child=children[i];
		
		if (orientation==HBOX)
			switch (child->valign) {
			case ALIGN_EXPAND:
				child->height=height;
				break;
			case ALIGN_BOTTOM:
				child->y0+=height - child->height;
				break;
			case ALIGN_CENTER:
				child->y0+=(height - child->height) / 2;
				break;
			}
		else
			switch (child->halign) {
			case ALIGN_EXPAND:
				child->width=width;
				break;
			case ALIGN_RIGHT:
				child->x0+=width - child->width;
				break;
			case ALIGN_CENTER:
				child->x0+=(width - child->width) / 2;
				break;
			}
			
		child->fix_alignment();
	}	
}


FreeLayout::FreeLayout()
{
}

void FreeLayout::add_child(LayoutItem* item)
{
	children.push_back(item);
}

void FreeLayout::move(int dx, int dy)
{
	LayoutItem::move(dx, dy);
	
	for (int i=0;i<children.size();i++)
		children[i]->move(dx, dy);
}

void FreeLayout::print_init()
{
	for (int i=0;i<children.size();i++)
		children[i]->print_init();
}

void FreeLayout::init_names(namecounter_t& namecnt)
{
	for (int i=0;i<children.size();i++)
		children[i]->init_names(namecnt);
}

void FreeLayout::fix_alignment()
{
	for (int i=0;i<children.size();i++)
		children[i]->fix_alignment();
}


Form::Form()
{
	child=NULL;
	
	varname="(*this)";
}

void Form::add_child(LayoutItem* item)
{
	if (child) {
		fprintf(stderr, "Error: Form may hold only one layout item!\n");
		exit(1);
	}
	
	child=item;
	child->move(8, 24);

	width=child->width + 16;
	height=child->height + 32;
}

void Form::print()
{
	init_names(namecounter);
	
	fix_alignment();
	
	fprintf(out_header, "class %s:\n", element_type);
	
	for (int i=0;i<extensions.size();i++) {
		fprintf(out_header, "\tpublic %s", extensions[i].first);
		
		if (i+1<extensions.size())
			fputs(",\n", out_header);
		else
			fputs(" {\n", out_header);
	}
	
	for (int i=0;i<subclasses.size();i++)
		fprintf(out_header, "\tclass %s;\n", subclasses[i]);
	
	for (int i=0;i<signal_handlers.size();i++) {
		signal_handlers[i]->construct_handler_name();
		
		if (!signal_handlers[i]->body) continue;
		
		fprintf(out_header, "\tvoid %s(", signal_handlers[i]->handler_name.c_str());
		fprintf(out_source, "void %s::%s(", element_type, signal_handlers[i]->handler_name.c_str());
		
		for (int j=0;j<signal_handlers[i]->args->size();j++) {
			if (j) {
				fputs(", ", out_header);
				fputs(", ", out_source);
			}
			fputs((*signal_handlers[i]->args)[j]->type, out_header);
			fputs((*signal_handlers[i]->args)[j]->type, out_source);
			fputc(' ', out_source);
			fputs((*signal_handlers[i]->args)[j]->name, out_source);
		}
		
		fputs(");\n", out_header);
		fputs(")\n{\n", out_source);
		signal_handlers[i]->item->print_code(signal_handlers[i]->body, out_source);
		fputs("\n}\n\n", out_source);
	}
	
	// constructor
	char* shortname=element_type;
	if (char* tmp=strrchr(shortname, ':')) shortname=tmp+1;
	
	fprintf(out_header, "\npublic:\n\t%s(", shortname);
	fprintf(out_source, "%s::%s(", element_type, shortname);
	
	int argc=0;
	
	for (int i=0;i<extensions.size();i++) {
		if (!extensions[i].second) continue;
		
		arglist_t args=*extensions[i].second;
		
		for (int j=0;j<args.size();j++) {
			if (argc++) {
				fputs(", ", out_header);
				fputs(", ", out_source);
			}
			
			fputs(args[j]->type, out_header);
			fputs(args[j]->type, out_source);
			fputc(' ', out_source);
			fputs(args[j]->name, out_source);
		}		
	}
	
	if (extensions.size() || constructors.size()) {
		fputs(") :\n", out_source);
		
		for (int i=0;i<extensions.size();i++) {
			fprintf(out_source, "\t%s(", extensions[i].first);

			if (extensions[i].second) {		
				arglist_t args=*extensions[i].second;
				if (args.size()) {
					argc=0;
					
					for (int j=0;j<args.size();j++) {
						if (argc++)
							fputs(", ", out_source);
						
						fputs(args[j]->name, out_source);
					}
				}
			}
			
			fputc(')', out_source);
			if (i+1<extensions.size() || constructors.size())
				fputc(',', out_source);
			fputc('\n', out_source);
		}
		
		for (int i=0;i<constructors.size();i++) {
			fprintf(out_source, "\t%s(%s)", constructors[i].first->varname, constructors[i].second->c_str());
			if (i+1<constructors.size())
				fputc(',', out_source);
			fputc('\n', out_source);
		}
	}
	
	fputs(");\n\nprivate:\n", out_header);
	fputs("\n{\n", out_source);
	
	print_init();
	
	for (int i=0;i<signal_handlers.size();i++) {
		SignalHandler& sh=*signal_handlers[i];
		
		//fprintf(out_source, "\t%s.%s.connect(sigc::mem_fun(this, &%s::%s));\n", sh.item->varname, sh.signal_name, element_type, sh.handler_name.c_str());
		
		fprintf(out_source, "\t%s.%s.connect(", sh.item->varname, sh.signal_name);
		for (int j=0;j<sh.args->size();j++) {
			argument_t* arg=(*sh.args)[j];
			
			if (arg->value)
				fprintf(out_source, "sigc::bind<%d>(", j);
		}
		
		fprintf(out_source, "sigc::mem_fun(this, &%s::%s)", element_type, sh.handler_name.c_str());

		for (int j=sh.args->size()-1; j>=0; j--) {
			argument_t* arg=(*sh.args)[j];
			
			if (arg->value)
				fprintf(out_source, ", %s)", arg->value);
		}

		fputs(");\n", out_source);
	}
	
	fputs("}\n\n", out_source);
	
	if (form_methods.size()>0) {
		fprintf(out_header, "\npublic:\n");
		
		for (int i=0;i<form_methods.size();i++) {
			FormMethod* fm=form_methods[i];
			
			//fprintf(out_header, "\tstatic %s* %s(", shortname, fm->name);
			putc('\t', out_header);
			if (fm->is_static) fputs("static ", out_header);
			fputs(fm->rettype, out_header);
			putc(' ', out_header);
			fputs(fm->name, out_header);
			putc('(', out_header);
			
			fprintf(out_source, "\n%s %s::%s(", fm->rettype, element_type, fm->name);
			
			for (int j=0;j<fm->args->size();j++) {
				if (j) {
					fputs(", ", out_header);
					fputs(", ", out_source);
				}
				
				fprintf(out_header, "%s %s", (*fm->args)[j]->type, (*fm->args)[j]->name);
				fprintf(out_source, "%s %s", (*fm->args)[j]->type, (*fm->args)[j]->name);
			}
			
			fputs(");\n", out_header);
			fputs(")\n{\n", out_source);
			fputs(fm->body, out_source);
			fputs("}\n\n", out_source);
		}
	}
	
	fputs("};\n\n", out_header);
}

void Form::print_init()
{
	fprintf(out_source, "\tset_size(%d, %d);\n\n", width, height);
	
	if (child)
		child->print_init();
	
	print_code(init_code.c_str(), out_source);
}

void Form::init_names(namecounter_t& namecnt)
{
	if (child)
		child->init_names(namecnt);
}

void Form::fix_alignment()
{
	if (child)
		child->fix_alignment();
}


int main(int argc, char* argv[])
{
	if (argc<2) {
		fputs("No input file specified!\n", stderr);
		return 1;
	}
	
	if (!freopen(argv[1], "r", stdin)) {
		perror("Error opening input file");
		return 1;
	}
	
	char* tmp=(char*) alloca(strlen(argv[1]) + 4);
	
	strcpy(tmp, argv[1]);
	strcat(tmp, ".h");
	out_header=fopen(tmp, "w");
	
	strcpy(tmp, argv[1]);
	strcat(tmp, ".cc");
	out_source=fopen(tmp, "w");
	
	strcpy(tmp, argv[1]);
	for (char* ptr=tmp; *ptr; ptr++)
		if (islower(*ptr))
			*ptr-=32;
		else if (!isalnum(*ptr))
			*ptr='_';

	fprintf(out_header, "// This file has been generated automatically from %s.\n\n", argv[1]);
	fprintf(out_source, "// This file has been generated automatically from %s.\n\n", argv[1]);
	
	fprintf(out_header, "#ifndef INCLUDE_%s_H\n", tmp);
	fprintf(out_header, "#define INCLUDE_%s_H\n\n", tmp);
	
	strcpy(tmp, argv[1]);
	strcat(tmp, ".h");
	fprintf(out_source, "#include \"%s\"\n\n", basename(tmp));
	
	yyparse();
	
	fputs("\n#endif\n\n", out_header);
	
	fclose(out_header);
	fclose(out_source);
	
	return 0;
}
