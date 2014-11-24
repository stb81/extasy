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
 
#include "gui/basics.h"
#include "gui/gui.h"
#include "synth/synthbits.h"
#include "synth/module.h"
#include "synth/sequencer.h"
#include "patternedit.h"

using namespace GUI;
using namespace Synth;

const char notenames[] ="CCDDEFFGGAAB" "-#-#--#-#-#-";
const char digits[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char lhexdigits[]="0123456789abcdef";
const char lalldigits[]="0123456789abcdefghijklmnopqrstuvwxyz";
const char cursor_index[]={ 0, 0,0,0, 1, 1,2, 3, 3,4, 5, 5,6,7, 7 };
const char notekeys[]="zsxdcvgbhnjmq2w3er5t6y7ui9o0p";

PatternEdit::PatternEdit():canvas(256,64)
{
	canvas.set_background_color(Color(0,0,0,0));
	
	module=nullptr;
	pattern=nullptr; 
	
	current_instrument=0;
	
	cursorx=0;
	cursory=0;
	selectx=0;
	selecty=0;
	selection=false;
	highlight_row=-1;
	
	modulation=255;
	
	for (int i=0;i<120;i++)
		tones[i]=NULL;
}

void PatternEdit::set_module(Module* mod)
{
	module=mod;
}

void PatternEdit::set_pattern(Pattern* pat)
{
	pattern=pat;
}

void PatternEdit::set_instrument(int instr)
{
	current_instrument=instr;
}

void PatternEdit::set_modulation(float m)
{
	modulation=(int) floorf(m*255.5f);
}

void PatternEdit::update_canvas()
{
	int ch, row, i;
	
	if (!pattern) return;
	
	for (row=0;row<64;row++) {
		Color col;
		
		if (row&3)
			col=0x505050;
		else if (row&15)
			col=0x808080;
		else
			col=0xb0b0b0;
		
		for (i=0;i<256;i++)
			canvas(i,row)=col,
			canvas(i,row).c=0;
		
		canvas(0,row).c=digits[row>>4];
		canvas(1,row).c=digits[row&15];
		
		for (ch=0;ch<16;ch++) {
			const Pattern::Note& note=(*pattern)(ch, row);
			
			if (!note.flags)
				col=0x505050;
			else if ((note.flags&2) && module->instruments[note.instrument])
				col=module->instruments[note.instrument]->get_color();
			else
				col=0xd0d0d0;
			
			col=col.shade((row&3) ? 64 : ((row&15) ? 128 : 192));
			for (int i=4;i<18;i++)
				canvas(15*ch+i,row)=col;
			
			if (note.flags&1)
				if (note.note==NOTE_OFF)
					canvas(15*ch+ 4,row).c=
					canvas(15*ch+ 5,row).c=
					canvas(15*ch+ 6,row).c=197;
				else if (note.note==NOTE_HIT)
					canvas(15*ch+ 4,row).c=
					canvas(15*ch+ 5,row).c=
					canvas(15*ch+ 6,row).c=195;
				else
					canvas(15*ch+ 4,row).c=notenames[note.note%12],
					canvas(15*ch+ 5,row).c=notenames[note.note%12+12],
					canvas(15*ch+ 6,row).c='0'+note.note/12;
			else
				canvas(15*ch+ 4,row).c=
				canvas(15*ch+ 5,row).c=
				canvas(15*ch+ 6,row).c=222;
			
			if (note.flags&2)
				canvas(15*ch+ 8,row).c=digits[note.instrument>>4],
				canvas(15*ch+ 9,row).c=digits[note.instrument&15];
			else if ((note.flags&1) && note.note==NOTE_OFF)
				canvas(15*ch+ 8,row).c=
				canvas(15*ch+ 9,row).c=197;
			else
				canvas(15*ch+ 8,row).c=
				canvas(15*ch+ 9,row).c=222;
				
			if (note.flags&4)
				canvas(15*ch+11,row).c=digits[note.volume>>4],
				canvas(15*ch+12,row).c=digits[note.volume&15];
			else
				canvas(15*ch+11,row).c=
				canvas(15*ch+12,row).c=222;
			
			if (note.flags&8)
				canvas(15*ch+14,row).c=digits[note.effect>>8],
				canvas(15*ch+15,row).c=digits[(note.effect>>4)&15],
				canvas(15*ch+16,row).c=digits[note.effect&15];
			else
				canvas(15*ch+14,row).c=
				canvas(15*ch+15,row).c=
				canvas(15*ch+16,row).c=222;
		}
	}
	
	canvas(0,cursory).r=
	canvas(0,cursory).g=
	canvas(0,cursory).b=
	canvas(1,cursory).r=
	canvas(1,cursory).g=
	canvas(1,cursory).b=255;
	
	if (selection)
		for (row=std::min(selecty,cursory);row<=std::max(selecty,cursory);row++)
			for (ch=std::min(selectx,cursorx)>>3;ch<=std::max(selectx,cursorx)>>3;ch++)
				for (i=0;i<15;i++)
					canvas(15*ch+i+4, row).r=
					canvas(15*ch+i+4, row).g=
					canvas(15*ch+i+4, row).b=255;
	else
		for (i=0;i<15;i++)
			if (cursor_index[i]==(cursorx&7))
				canvas(15*(cursorx>>3)+i+3, cursory).r=
				canvas(15*(cursorx>>3)+i+3, cursory).g=
				canvas(15*(cursorx>>3)+i+3, cursory).b=255;
	
	canvas.update();
}

void PatternEdit::draw()
{
	update_canvas();
	
	glUseProgram(0);
	
	glColor3f(0.25f, 0.375f, 0.5f);
	glBegin(GL_QUADS);
	glVertex2i(originx      , originy);
	glVertex2i(originx+width, originy);
	glVertex2i(originx+width, originy+16);
	glVertex2i(originx      , originy+16);
	glEnd();

	for (int i=0;i<16;i++)
		textprintf(originx+32+i*120, originy, 1, 1, 1, "Channel %d", i+1);
	
	{
		Scissor scissor(originx, originy+16, width, height-16);
		
		if (highlight_row>=0) {
			glUseProgram(0);
			glColor3f(0, 0, 0.5f);
			glBegin(GL_QUADS);
			glVertex2i(originx      , originy+highlight_row*16+16);
			glVertex2i(originx+width, originy+highlight_row*16+16);
			glVertex2i(originx+width, originy+highlight_row*16+32);
			glVertex2i(originx      , originy+highlight_row*16+32);
			glEnd();
		}
		
		glEnable(GL_BLEND);
		canvas.draw(originx, originy+16);
		glDisable(GL_BLEND);
	}
}

void PatternEdit::handle_event(SDL_Event& ev)
{
	int i,instrno;
	
	Widget::handle_event(ev);
	
	if (ev.type==SDL_KEYDOWN && has_mouse_focus())
		switch(ev.key.keysym.sym) {
		case SDLK_RIGHT:
			selection=false;
			cursorx++;
			cursorx&=127;
			break;
		case SDLK_LEFT:
			selection=false;
			cursorx--;
			cursorx&=127;
			break;
		case SDLK_DOWN:
			selection=false;
			cursory++;
			cursory&=63;
			/*if (16*cursory<scrollposy)
				scrollposy=16*cursory;
			if (16*(cursory+2) > scrollposy+height)
				scrollposy=16*(cursory+2)-height;*/
			break;
		case SDLK_UP:
			selection=false;
			cursory--;
			cursory&=63;
			/*if (16*cursory<scrollposy)
				scrollposy=16*cursory;
			if (16*(cursory+2) > scrollposy+height)
				scrollposy=16*(cursory+2)-height;*/
			break;
		case SDLK_DELETE:
		case SDLK_BACKSPACE:
			switch(cursorx&7) {
			case 0:
				(*pattern)(cursorx>>3,cursory).flags=0;
				(*pattern)(cursorx>>3,cursory).note=0;
				(*pattern)(cursorx>>3,cursory).instrument=0;
				(*pattern)(cursorx>>3,cursory).volume=0;
				(*pattern)(cursorx>>3,cursory).effect=0;
				break;
			case 1:
			case 2:
				(*pattern)(cursorx>>3,cursory).flags&=~2;
				(*pattern)(cursorx>>3,cursory).instrument=0;
				break;
			case 3:
			case 4:
				(*pattern)(cursorx>>3,cursory).flags&=~4;
				(*pattern)(cursorx>>3,cursory).volume=0;
				break;
			case 5:
			case 6:
			case 7:
				(*pattern)(cursorx>>3,cursory).flags&=~8;
				(*pattern)(cursorx>>3,cursory).effect=0;
				break;
			}
			break;
		default:
			switch (cursorx&7) {
			case 0:
				if ((i=key_to_note(ev.key.keysym.sym))<0) break;
				instrno=current_instrument;
				
				(*pattern)(cursorx>>3,cursory).flags|=(i==NOTE_OFF) ? 1 : 3;
				(*pattern)(cursorx>>3,cursory).note=i;
				(*pattern)(cursorx>>3,cursory).instrument=(i==NOTE_OFF) ? 0 : instrno;
				
				if (i<120) {
					Instrument* instr=module->instruments[instrno];
					if (instr) {
						tones[i]=instr->play_note(i, 128);
						tones[i]->apply_effect((Pattern::MODULATION<<8) | modulation);
					}
				}
				break;
			case 1:
				if ((i=key_to_digit(ev.key.keysym.sym))<0) break;
				(*pattern)(cursorx>>3,cursory).flags|=2;
				(*pattern)(cursorx>>3,cursory).instrument&=0x0f;
				(*pattern)(cursorx>>3,cursory).instrument|=i<<4;
				cursorx++;
				break;
			case 2:
				if ((i=key_to_digit(ev.key.keysym.sym))<0) break;
				(*pattern)(cursorx>>3,cursory).flags|=2;
				(*pattern)(cursorx>>3,cursory).instrument&=0xf0;
				(*pattern)(cursorx>>3,cursory).instrument|=i;
				cursorx++;
				break;
			case 3:
				if ((i=key_to_digit(ev.key.keysym.sym))<0) break;
				(*pattern)(cursorx>>3,cursory).flags|=4;
				(*pattern)(cursorx>>3,cursory).volume&=0x0f;
				(*pattern)(cursorx>>3,cursory).volume|=i<<4;
				cursorx++;
				break;
			case 4:
				if ((i=key_to_digit(ev.key.keysym.sym))<0) break;
				(*pattern)(cursorx>>3,cursory).flags|=4;
				(*pattern)(cursorx>>3,cursory).volume&=0xf0;
				(*pattern)(cursorx>>3,cursory).volume|=i;
				cursorx++;
				break;
			case 5:
				if ((i=key_to_effect(ev.key.keysym.sym))<0) break;
				(*pattern)(cursorx>>3,cursory).flags|=8;
				(*pattern)(cursorx>>3,cursory).effect&=0x00ff;
				(*pattern)(cursorx>>3,cursory).effect|=i<<8;
				cursorx++;
				break;
			case 6:
				if ((i=key_to_digit(ev.key.keysym.sym))<0) break;
				(*pattern)(cursorx>>3,cursory).flags|=8;
				(*pattern)(cursorx>>3,cursory).effect&=0xff0f;
				(*pattern)(cursorx>>3,cursory).effect|=i<<4;
				cursorx++;
				break;
			case 7:
				if ((i=key_to_digit(ev.key.keysym.sym))<0) break;
				(*pattern)(cursorx>>3,cursory).flags|=8;
				(*pattern)(cursorx>>3,cursory).effect&=0xfff0;
				(*pattern)(cursorx>>3,cursory).effect|=i;
				cursorx++;
				cursorx&=127;
				break;
			}
		}
	
	if (ev.type==SDL_KEYUP) {
		int note=key_to_note(ev.key.keysym.sym);
		if (note>=0 && note<120 && tones[note]) {
			tones[note]->stop();
			tones[note]=NULL;
		}
	}
	
	if (ev.type==SDL_MIDI_EVENT) {
		unsigned char* msg=(unsigned char*) ev.user.data1;
		
		if (msg[0]==0x90 && msg[2]>0) {
			instrno=current_instrument;
			
			// play the note
			Instrument* instr=module->instruments[instrno];
			if (instr) {
				Voice* voice=tones[msg[1]]=instr->play_note(msg[1], msg[2]);
				voice->apply_effect((Pattern::MODULATION<<8) | modulation);
			}
			
			// write the note into the pattern if the cursor is on the 'note' field
			if ((cursorx&7)==0) {
				(*pattern)(cursorx>>3, cursory).flags|=3;
				(*pattern)(cursorx>>3, cursory).note=msg[1];
				(*pattern)(cursorx>>3, cursory).instrument=instrno;
			}
		}
		
		if (msg[0]==0x90 && msg[2]==0 && tones[msg[1]]) {
			tones[msg[1]]->stop();
			tones[msg[1]]=NULL;
		}
	}
	
	if (has_mouse_focus()) {
		if (ev.type==SDL_MOUSEBUTTONDOWN && ev.button.button==SDL_BUTTON_LEFT) {
			int x=(ev.button.x-originx) / 8 - 3;
			int y=(ev.button.y-originy) / 16 - 1;
			
			if (x>=0 && x<240 && y>=0 && cursor_index[x%15]>=0) {
				cursorx=cursor_index[x%15] + (x/15)*8;
				cursory=y;
			}
			
			selection=false;
			selectx=cursorx;
			selecty=cursory;
		}
	
		if (ev.type==SDL_MOUSEMOTION && (ev.motion.state&SDL_BUTTON(SDL_BUTTON_LEFT))) {
			int x=(ev.button.x-originx) / 8 - 3;
			int y=(ev.button.y-originy) / 16 - 1;
			
			if (x>=0 && x<240 && y>=0 && cursor_index[x%15]>=0) {
				x=cursor_index[x%15] + (x/15)*8;
			
				if (selection || (selectx&-8)!=(x&-8) || selecty!=y) {
					selection=true;
					cursorx=x;
					cursory=y;
				}
			}
		}
	}
		
	if (ev.type==SDL_PLAY_POSITION_NOTIFICATION) {
		auto ai=reinterpret_cast<Module::arrangement_item_t*>(ev.user.data2);
		highlight_row=(ai && pattern==*ai) ? ev.user.code : -1;
	}
}

int PatternEdit::key_to_digit(int keysym)
{
	int i;
	
	for (i=0;lhexdigits[i];i++)
		if (lhexdigits[i]==keysym)
			return i;
	
	return -1;
}

int PatternEdit::key_to_effect(int keysym)
{
	int i;
	
	for (i=0;lalldigits[i];i++)
		if (lalldigits[i]==keysym)
			return i;
	
	return -1;
}

int PatternEdit::key_to_note(int keysym)
{
	int i;
	
	if (keysym==SDLK_SPACE)
		return NOTE_OFF;
	
	if (keysym==SDLK_RETURN)
		return NOTE_HIT;
	
	for (i=0;notekeys[i];i++)
		if (notekeys[i]==keysym)
			return i + 36;
	
	return -1;
}

