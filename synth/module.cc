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
 
#include <typeinfo>
#include <algorithm>
#include <string.h>
#include "synthbits.h"
#include "module.h"
#include "sequencer.h"
#include "serialization.h"

namespace Synth {

Module::Module()
{
	patterns.push_back(new Pattern(*this));
	
	arrangement.push_back(arrangement_item_t { patterns[0], 0 });
	
	for (int i=0;i<256;i++)
		instruments[i]=0;
}

Module::Module(Deserializer& deser)
{
	deser.set_module(this);
	
	for (int i=0;i<256;i++)
		deser >> instruments[i];
	
	deser >> patterns;
	deser >> tag("arrangement", arrangement);
}

void Module::serialize(Serializer& ser) const
{
	Serializer::Tag _tag(ser, "Module");
	
	for (int i=0;i<256;i++)
		ser << instruments[i];
		
	ser << patterns;
	ser << tag("arrangement", arrangement);
}

Module* Module::deserialize(Deserializer& deser)
{
	Deserializer::Tag _tag(deser, "Module");
	
	return new Module(_tag);
}


void Module::arrangement_item_t::serialize(Serializer& ser) const
{
	ser << pattern->get_index() << channelmask;
}

void Module::arrangement_item_t::deserialize(Deserializer& deser)
{
	int patindex;
	
	deser >> patindex >> channelmask;
	
	pattern=deser.get_module()->patterns[patindex];
}


Pattern::Pattern(const Module& module):module(module)
{
	notes=new Note[16*64];
	
	name="Unnamed Pattern";
	
	for (int i=0;i<16*64;i++) {
		notes[i].flags=0;
		notes[i].note=0;
		notes[i].instrument=0;
		notes[i].volume=0;
		notes[i].effect=0;
	}
}

Pattern::Pattern(Deserializer& deser):module(*deser.get_module())
{
	deser >> tag("name", name);

	notes=new Note[16*64];
	
	for (int i=0;i<16;i++) {
		Deserializer::Tag chan(deser, "channel");

		int row=0;
		while (row<64) {
			unsigned char flags=chan.getc();
			
			while (flags&0x0f) {
				if (row>=64) break;	// FIXME: illegal, throw invalid file exception
				
				auto& note=(*this)(i, row++);
				note.flags=0;
				note.note=0;
				note.instrument=0;
				note.volume=0;
				note.effect=0;
				flags--;
			}
			
			if (row>=64) break;
			
			auto& note=(*this)(i, row++);
			note.flags=flags>>4;
			note.note=(flags&0x10) ? chan.getc() : 0;
			note.instrument=(flags&0x20) ? chan.getc() : 0;
			note.volume=(flags&0x40) ? chan.getc() : 0;
			note.effect=(flags&0x80) ? (chan.getc()<<8) | chan.getc() : 0;
		}
	}
}

Pattern::~Pattern()
{
	delete[] notes;
}

void Pattern::serialize(Serializer& ser) const
{
	Serializer::Tag _tag(ser, "Pattern");
	
	ser << tag("name", name);
	
	for (int i=0;i<16;i++) {
		Serializer::Tag chan(ser, "channel");

		int row=0;
		while (row<64) {
			int skip=0;
			while (skip<16 && row+skip<64 && !(*this)(i, row+skip).flags) skip++;
			
			row+=skip;
			if (skip==16) {
				ser.putc(0x0F);
				continue;
			}

			auto& note=(*this)(i, row++);
			
			ser.putc((note.flags<<4) | skip);
			
			if (note.flags&1)
				ser.putc(note.note);
			if (note.flags&2)
				ser.putc(note.instrument);
			if (note.flags&4)
				ser.putc(note.volume);
			if (note.flags&8) {
				ser.putc(note.effect>>8);
				ser.putc(note.effect&0xff);
			}
		}
	}
}

Pattern* Pattern::deserialize(Deserializer& deser)
{
	Deserializer::Tag _tag(deser, "Pattern");
	
	return new Pattern(_tag);
}

int Pattern::get_index() const
{
	if (index>=0 && index<module.patterns.size() && module.patterns[index]==this)
		return index;
	
	auto i=std::find(module.patterns.begin(), module.patterns.end(), this);
	index=i==module.patterns.end() ? -1 : std::distance(module.patterns.begin(), i);
	
	return index;
}


Instrument::Instrument(Mixer& m):mixer(m)
{
	name="Unnamed Instrument";
	color=0x55aaff;
	
	for (int i=0;i<resonance.size();i++)
		resonance[i].init_filter(mixer);
}

Instrument::~Instrument()
{
}

void Instrument::Resonance::init_filter(const Mixer& mixer)
{
	float omega=mixer.freq2omega(freq);
	float gaintmp=powf(10.0f, gain/(20*order));
	
	switch (type) {
	case PEAKINGEQ:
		filter=BiQuad::peakingEQ(omega, Q, gaintmp);
		break;
	case LOWSHELF:
		filter=BiQuad::lowshelf(omega, Q, gaintmp);
		break;
	case HIGHSHELF:
		filter=BiQuad::highshelf(omega, Q, gaintmp);
		break;
	case BANDSTOP:
		filter=BiQuad::bandstop(omega, Q);
		break;
	case BANDPASS:
		filter=BiQuad::bandpass(omega, Q);
		break;
	case LOWPASS:
		filter=BiQuad::lowpass(omega, Q);
		break;
	case HIGHPASS:
		filter=BiQuad::highpass(omega, Q);
		break;
	case ALLPASS:
		filter=BiQuad::allpass(omega, Q);
		break;
	}
}

void Instrument::Resonance::serialize(Serializer& ser) const
{
	Serializer::Tag _tag(ser, "BiQuad");
 
	int itype=type;
	ser << tag("type", itype) << tag("order", order) << tag("freq", freq) << tag("Q", Q) << tag("gain", gain);
}

void Instrument::Resonance::deserialize(Deserializer& deser)
{
	Deserializer::Tag _tag(deser, "BiQuad");
	
	int itype;
	_tag >> tag("type", itype) >> tag("order", order, 1) >> tag("freq", freq) >> tag("Q", Q) >> tag("gain", gain);
	type=(Resonance::type_t) itype;
	
	init_filter(deser.get_mixer());
}


std::vector<std::pair<const char*, Instrument* (*)(Mixer&)> > Instrument::classes;


Instrument* Instrument::create(Mixer& mixer, const char* classname)
{
	for (int i=0;i<classes.size();i++)
		if (!strcmp(classes[i].first, classname))
			return classes[i].second(mixer);
	
	return NULL;
}

void Instrument::register_class(const char* classname, Instrument* (*creator)(Mixer&))
{
	classes.push_back(std::pair<const char*, Instrument* (*)(Mixer&)>(classname, creator));
}

void Instrument::serialize(Serializer& ser) const
{
	Serializer::Tag _tag(ser, "Instrument");
	
	do_serialize(ser);
}
	
Instrument* Instrument::deserialize(Deserializer& deser)
{
	if (!strcmp(deser.peekstr(), "null"))  {
		Deserializer::Tag _tag(deser, "null");
		return nullptr;
	}
	
	Deserializer::Tag _tag(deser, "Instrument");
	if (!_tag.is_valid()) return nullptr;
	
	std::string classtype;
	_tag >> tag("class", classtype, std::string());
	
	if (classtype.empty())
		return nullptr;
	
	Instrument* instr=create(deser.get_mixer(), classtype.c_str());
	if (!instr) return nullptr;
	
	instr->do_deserialize(_tag);
	
	return instr;
}
	
void Instrument::do_serialize(Serializer& ser) const
{
	ser << tag("name", name) << tag("color", color) << tag("resonance", resonance);
}

void Instrument::do_deserialize(Deserializer& deser)
{
	deser >> tag("name", name) >> tag("color", color) >> tag("resonance", resonance);
}


ExcitationModelInstrument::ExcitationModelInstrument(Mixer& m):Instrument(m)
{
	discrete_excitation=1.0f;
	continuous_excitation=0.5f;
	decay_rate=4.0f;
	decay_scale=1.0f;
}

ExcitationModelInstrument::~ExcitationModelInstrument()
{
}

}
