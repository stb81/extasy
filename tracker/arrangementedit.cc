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
 
#include <gui/gui.h>
#include <synth/synthbits.h>
#include <synth/module.h>
#include "arrangementedit.h"

unsigned int glsl_load_shader(int type, const char* filename, const char* preamble=0);
unsigned int glsl_link_program(unsigned int shv, unsigned int shf);


using namespace GUI;
using namespace Synth;


class PatternListBlock:public Draggable, DropReceiver<PatternListBlock> {
	static unsigned int shader;
	
	Pattern*	pattern=nullptr;
	int			channel;
	
	unsigned int	texid;
	
	bool	enabled=true;
	
public:
	PatternListBlock(Pattern*, int);
	virtual ~PatternListBlock();
	
	virtual void draw();
	virtual void handle_event(SDL_Event&);
	
	virtual void on_drop_event(PatternListBlock*, int, int);
	
	void set_pattern_channel(Pattern*, int);
	
private:
	void init_texture();
	
	void draw_internal(int, int);
	
	virtual void draw_dragged(int, int);
};


template<>
class PatternListRow<Module::arrangement_item_t>:public Group {
	const Module&				module;
	Module::arrangement_item_t&	item;
	
	PatternListBlock*			blocks[16];
	
	int							highlight_row=-1;
	
public:
	bool isnew=false;
	
	PatternListRow(Module::arrangement_item_t&);
	
	virtual void draw();
	virtual void handle_event(SDL_Event&);
	
	Pattern* get_pattern()
	{
		return item.pattern;
	}
	
private:
	void on_pattern_changed(int);
};


template<>
class PatternListRow<Pattern*>:public Group {
	const Module&				module;
	Pattern*					pattern;
	
	PatternListBlock*			blocks[16];
	
	int							highlight_row=-1;
	
public:
	bool isnew=false;
	
	PatternListRow(Pattern*);
	
	virtual void draw();
	virtual void handle_event(SDL_Event&);
	
	Pattern* get_pattern()
	{
		return pattern;
	}
};


template<>
class PatternListInsertionPost<Module::arrangement_item_t>:public Widget {
	typedef std::list<Module::arrangement_item_t>::iterator iterator;
	
	iterator	position;
	
public:
	PatternListInsertionPost(Module& module, iterator p):position(p) {}
	
	iterator get_insertion_position()
	{
		return position;
	}
	
	virtual void draw();
};


template<>
class PatternListInsertionPost<Pattern*>:public Widget {
	typedef std::vector<Pattern*>::iterator iterator;
	
	Module&			module;
	Pattern*		before_pattern;
	
public:
	PatternListInsertionPost(Module& module, iterator p):module(module)
	{
		before_pattern=p==module.patterns.end() ? nullptr : *p;	
	}
	
	iterator get_insertion_position()
	{
		return before_pattern ? module.patterns.begin()+before_pattern->get_index() : module.patterns.end();
	}
	
	virtual void draw();
};


unsigned int PatternListBlock::shader=0;


template<typename TContainer>
PatternListEdit<TContainer>::PatternListEdit()
{
	layer=1;
	
	highlighted_row=-1;
	
	list=nullptr;
}


template<typename TContainer>
void PatternListEdit<TContainer>::set_list(TContainer* l)
{
	list=l;

	int y0=0;
	for (auto i=list->begin(); i!=list->end(); i++) {
		auto& item=*i;
		
		PatternListInsertionPost<TItem>* post=new PatternListInsertionPost<TItem>(*module, i);
		post->set_origin(0, y0);
		post->set_size(1024, 16);
		post->double_clicked.connect(sigc::bind(sigc::mem_fun(this, &PatternListEdit<TContainer>::on_double_click_post), post));
		add(post);
		
		y0+=16;
		
		auto row=new PatternListRow<TItem>(item);
		row->set_origin(0, y0);
		row->double_clicked.connect(sigc::bind(sigc::mem_fun(this, &PatternListEdit<TContainer>::on_double_click_entry), row));
		add(row);
		
		y0+=row->get_height();
	}
	
	auto post=new PatternListInsertionPost<TItem>(*module, list->end());
	post->set_origin(0, y0);
	post->set_size(1024, 16);
	post->double_clicked.connect(sigc::bind(sigc::mem_fun(this, &PatternListEdit<TContainer>::on_double_click_post), post));
	add(post);
}


template<typename TContainer>
void PatternListEdit<TContainer>::add(Widget* w)
{
	Container::add(w);
	
	entry_widgets.push_back(w);
}


template<typename TContainer>
void PatternListEdit<TContainer>::on_double_click_entry(PatternListRow<TItem>* entry)
{
	select_pattern(entry->get_pattern());
}


template<typename TContainer>
void PatternListEdit<TContainer>::on_double_click_post(PatternListInsertionPost<TItem>* post)
{
	typename TContainer::iterator ilist=post->get_insertion_position();
	
	auto ientry=entry_widgets.begin();
	std::advance(ientry, std::distance(list->begin(), ilist)<<1);

	ilist=list->insert(ilist, create_new_item());
	
	auto newpost=new PatternListInsertionPost<TItem>(*module, ilist);
	Container::add(newpost);
	newpost->set_size(1024, 16);
	newpost->set_origin(0, (*ientry)->get_originy());
	newpost->double_clicked.connect(sigc::bind(sigc::mem_fun(this, &PatternListEdit<TContainer>::on_double_click_post), newpost));
	ientry=entry_widgets.insert(ientry, newpost);
	
	auto newrow=new PatternListRow<TItem>(*ilist);
	Container::add(newrow);
	newrow->set_size(1024, 96);
	newrow->set_origin(0, (*ientry)->get_originy()+16);
	newrow->isnew=true;
	newrow->double_clicked.connect(sigc::bind(sigc::mem_fun(this, &PatternListEdit<TContainer>::on_double_click_entry), newrow));
	ientry=entry_widgets.insert(++ientry, newrow);
	
	int shift=newpost->get_height() + newrow->get_height();
	while (++ientry!=entry_widgets.end())
		(*ientry)->move(0, shift);
}


template<typename TContainer>
void PatternListEdit<TContainer>::handle_event(SDL_Event& event)
{
	SDL_Event tmp=event;
	
	if (event.type==SDL_MOUSEMOTION) {
		int y0=originy, row=0;
		if (event.motion.y>=y0) {
			highlighted_row=0;
			
			for (Pattern* pat: *list) {
				y0+=16;
				if (event.motion.y<y0)
					break;
				
				highlighted_row++;
				y0+=80;
				if (event.motion.y<y0)
					break;

				highlighted_row++;
			}
		}
		else
			highlighted_row=-1;
	}
	
	Container::handle_event(event);

	for (auto child: entry_widgets)
		child->handle_event(tmp);
}


template<typename TContainer>
void PatternListEdit<TContainer>::draw()
{
	glUseProgram(0);
	
	int x0=originx, y0=originy;
	
	glBegin(GL_QUADS);
	glColor4f(0.0f, 0.0f, 0.0f, 0);
	glVertex2i(x0, y0);
	glVertex2i(x0+width, y0);
	glVertex2i(x0+width, y0+height);
	glVertex2i(x0, y0+height);
	glEnd();
	
	Container::draw();
	
	for (auto child: entry_widgets)
		child->draw();
}


template<typename TContainer>
void PatternListEdit<TContainer>::move(int dx, int dy)
{
	Container::move(dx, dy);
	
	for (auto child: entry_widgets)
		child->move(dx, dy);
}


PatternListRow<Module::arrangement_item_t>::PatternListRow(Module::arrangement_item_t& item):module(item.pattern->get_module()), item(item)
{
	set_size(1024, 80);	// FIXME
	
	for (int j=0;j<16;j++) {
		blocks[j]=new PatternListBlock(item, j);
		blocks[j]->set_origin(64*j+96, 8);
		blocks[j]->set_size(48, 64);
		add(blocks[j]);
	}
		
	SpinBox* indexbox=new SpinBox;
	indexbox->set_origin(16, 16);
	indexbox->set_size(64, 24);
	indexbox->set_range(0, module.patterns.size()-1);
	indexbox->set_value(item.pattern->get_index());
	indexbox->value_changed.connect(sigc::mem_fun(this, &PatternListRow::on_pattern_changed));
	add(indexbox);
}


PatternListRow<Pattern*>::PatternListRow(Pattern* pattern):module(pattern->get_module()), pattern(pattern)
{
	set_size(1024, 80);	// FIXME
	
	for (int j=0;j<16;j++) {
		blocks[j]=new PatternListBlock(pattern, j);
		blocks[j]->set_origin(64*j+96, 8);
		blocks[j]->set_size(48, 64);
		add(blocks[j]);
	}
}


void PatternListRow<Module::arrangement_item_t>::draw()
{
	glUseProgram(0);
	
	int x0=originx, y0=originy;
	
	glBegin(GL_QUADS);
	
	if (has_mouse_focus())
		glColor4f(0.1f, 0.1f, 0.1f, 0);
	else
		glColor4f(isnew ? 0.2f : 0.05f, 0.05f, 0.05f, 0);
		
	glVertex2i(x0, y0);
	glVertex2i(x0+width, y0);
	glVertex2i(x0+width, y0+height);
	glVertex2i(x0, y0+height);
	glEnd();
	
	if (highlight_row>=0) {
		glBegin(GL_LINES);
		glColor3f(0.5f, 1, 1);
		glVertex2i(x0, y0+highlight_row+8);
		glVertex2i(x0+width, y0+highlight_row+8);
		glEnd();
	}
	
	FontSpec font;
	
	font.font=thin_font;
	textprint(x0+28, y0-8, font, item.pattern->get_name());

	Group::draw();
}


void PatternListRow<Module::arrangement_item_t>::handle_event(SDL_Event& event)
{
	Group::handle_event(event);
	
	if (event.type==SDL_PLAY_POSITION_NOTIFICATION) {
		auto ai=reinterpret_cast<Module::arrangement_item_t*>(event.user.data2);
		highlight_row=ai==&item ? event.user.code : -1;
	}
}


void PatternListRow<Pattern*>::draw()
{
	glUseProgram(0);
	
	int x0=originx, y0=originy;
	
	glBegin(GL_QUADS);
	
	if (has_mouse_focus())
		glColor4f(0.1f, 0.1f, 0.1f, 0);
	else
		glColor4f(isnew ? 0.2f : 0.05f, 0.05f, 0.05f, 0);
		
	glVertex2i(x0, y0);
	glVertex2i(x0+width, y0);
	glVertex2i(x0+width, y0+height);
	glVertex2i(x0, y0+height);
	glEnd();
	
	if (highlight_row>=0) {
		glBegin(GL_LINES);
		glColor3f(0.5f, 1, 1);
		glVertex2i(x0, y0+highlight_row+8);
		glVertex2i(x0+width, y0+highlight_row+8);
		glEnd();
	}
	
	FontSpec font;
	
	font.font=thin_font;
	textprint(x0+32, y0-8, font, pattern->get_name());
	
	font.font=monospaced_font;
	textprintf(x0+8, y0-8, font, "%02X", pattern->get_index());

	Group::draw();
}


void PatternListRow<Pattern*>::handle_event(SDL_Event& event)
{
	Group::handle_event(event);
	
	if (event.type==SDL_PLAY_POSITION_NOTIFICATION) {
		auto ai=reinterpret_cast<Module::arrangement_item_t*>(event.user.data2);
		highlight_row=(ai && pattern==*ai) ? event.user.code : -1;
	}
}


void PatternListRow<Module::arrangement_item_t>::on_pattern_changed(int index)
{
	item.pattern=module.patterns[index];
	
	for (int j=0;j<16;j++)
		blocks[j]->set_pattern_channel(item.pattern, j);
}


void PatternListInsertionPost<Module::arrangement_item_t>::draw()
{
	glUseProgram(0);
	
	int x0=originx, y0=originy;
	
	glBegin(GL_QUADS);
	
	if (has_mouse_focus())
		glColor4f(0.1f, 0.1f, 0.1f, 0);
	else
		glColor4f(0.0f, 0.0f, 0.0f, 0);
		
	glVertex2i(x0, y0);
	glVertex2i(x0+width, y0);
	glVertex2i(x0+width, y0+height);
	glVertex2i(x0, y0+height);
	glEnd();
}


void PatternListInsertionPost<Pattern*>::draw()
{
	glUseProgram(0);
	
	int x0=originx, y0=originy;
	
	glBegin(GL_QUADS);
	
	if (has_mouse_focus())
		glColor4f(0.1f, 0.1f, 0.1f, 0);
	else
		glColor4f(0.0f, 0.0f, 0.0f, 0);
		
	glVertex2i(x0, y0);
	glVertex2i(x0+width, y0);
	glVertex2i(x0+width, y0+height);
	glVertex2i(x0, y0+height);
	glEnd();
}


ArrangementEdit::ArrangementEdit()
{
}


void ArrangementEdit::set_module(Module* m)
{
	module=m;
	set_list(&module->arrangement);
}


template<>
Module::arrangement_item_t PatternListEdit<std::list<Module::arrangement_item_t>>::create_new_item() const
{
	Synth::Module::arrangement_item_t item;
	
	item.pattern=module->patterns[0];
	item.channelmask=0;
	
	return item;
}


template<>
Pattern* PatternListEdit<std::vector<Pattern*>>::create_new_item() const
{
	Pattern* pat=new Pattern(*module);
	
	return pat;
}


PatternStockEdit::PatternStockEdit()
{
}


void PatternStockEdit::set_module(Module* m)
{
	module=m;
	set_list(&module->patterns);
}


PatternListBlock::PatternListBlock(Pattern* pat, int channel)
{
	if (!shader) {
		int shv=glsl_load_shader(GL_VERTEX_SHADER, "shader/arrangementblock.vert");
		int shf=glsl_load_shader(GL_FRAGMENT_SHADER, "shader/arrangementblock.frag");
		shader=glsl_link_program(shv, shf);
	}

	glGenTextures(1, &texid);

	set_pattern_channel(pat, channel);
}

PatternListBlock::~PatternListBlock()
{
	glDeleteTextures(1, &texid);
}

void PatternListBlock::set_pattern_channel(Pattern* pat, int ch)
{
	pattern=pat;
	channel=ch;
	
	init_texture();
}

void PatternListBlock::init_texture()
{
	Color tex[64];
	
	enabled=true;
	
	const Module& module=pattern->get_module();
	
	for (int i=0;i<64;i++) {
		auto& note=(*pattern)(channel, i);
		
		if (!note.flags)
			tex[i]=0x191919;
		else {
			Color col=(note.flags&2) && module.instruments[note.instrument] ? module.instruments[note.instrument]->get_color() : 0xaaaaaa;
			col.a=(note.flags&4) ? note.volume : 0xff;
			tex[i]=col;
		}
	}	
	
	glBindTexture(GL_TEXTURE_1D, texid);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void PatternListBlock::handle_event(SDL_Event& event)
{
	Draggable::handle_event(event);
	DropReceiver::handle_event(event);
	
	if (event.type==SDL_PATTERN_CHANGED && event.user.data2==pattern && (event.user.code&(1<<channel)))
		init_texture();
}

void PatternListBlock::on_drop_event(PatternListBlock* dropped, int dropx, int dropy)
{
	if (dropped==this) return;
	if (!contains_point(dropx+width/2, dropy+height/2)) return;
	
	for (int i=0;i<64;i++)
		(*pattern)(channel, i)=(*dropped->pattern)(dropped->channel, i);
	
	send_notification(SDL_PATTERN_CHANGED, pattern, 1<<channel);
}

void PatternListBlock::draw()
{
	draw_internal(originx, originy);
}

void PatternListBlock::draw_dragged(int x0, int y0)
{
	draw_internal(x0, y0);
}

void PatternListBlock::draw_internal(int x0, int y0)
{
	glUseProgram(shader);
	
	glBindTexture(GL_TEXTURE_1D, texid);
	glUniform1i(glGetUniformLocation(shader, "blocktex"), 0);
	glUniform1f(glGetUniformLocation(shader, "highlight"), (has_mouse_focus() ? 1.25f : 0.75f) * (enabled ? 1.0f : 0.5f));
	
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2i(x0, y0);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2i(x0+width, y0);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2i(x0+width, y0+height);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2i(x0, y0+height);
	glEnd();

	glUseProgram(0);

	glBegin(GL_LINE_STRIP);
	glColor3f(0, 0, 0);
	glVertex2f(x0+0.5f, y0+height+1.5f);
	glVertex2f(x0+width+1.5f, y0+height+1.5f);
	glVertex2f(x0+width+1.5f, y0+0.5f);
	glEnd();
	
	glBegin(GL_LINE_LOOP);
	glColor3f(0.3f, 0.3f, 0.3f);
	glVertex2f(x0-0.5f, y0-0.5f);
	glVertex2f(x0+width+0.5f, y0-0.5f);
	glVertex2f(x0+width+0.5f, y0+height+0.5f);
	glVertex2f(x0-0.5f, y0+height+0.5f);
	glEnd();
	
	if (!enabled) {
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		
		glLineWidth(4.0f);
		
		glBegin(GL_LINES);
		glColor3f(0, 0, 0);
		glVertex2f(x0+4.5f, y0+4.5f);
		glVertex2f(x0+width-4.5f, y0+height-4.5f);
		glVertex2f(x0+width-4.5f, y0+4.5f);
		glVertex2f(x0+4.5f, y0+height-4.5f);
		glEnd();
		
		glLineWidth(2.0f);
		
		glBegin(GL_LINES);
		if (has_mouse_focus())
			glColor3f(0.75f, 0.75f, 0.75f);
		else
			glColor3f(0.5f, 0.5f, 0.5f);
		glVertex2f(x0+4.5f, y0+4.5f);
		glVertex2f(x0+width-4.5f, y0+height-4.5f);
		glVertex2f(x0+width-4.5f, y0+4.5f);
		glVertex2f(x0+4.5f, y0+height-4.5f);
		glEnd();

		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);

		glLineWidth(1.0f);
	}
}

