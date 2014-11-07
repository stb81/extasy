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
	patterns.push_back(new Pattern(*this));
	patterns.back()->set_name("Another pattern");
	
	for (int i=0;i<4;i++) {
		arrangement_item_t item;
		item.pattern=patterns[i&1];
		item.channelmask=0;
		arrangement.push_back(item);
	}
	
	for (int i=0;i<256;i++)
		instruments[i]=0;
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

Pattern::~Pattern()
{
	delete[] notes;
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
	
	/*resonance.push_back(Resonance(270, 2.0f, 10.0f));
	resonance.push_back(Resonance(2300, 3.0f, 5.0f));
	resonance.push_back(Resonance(3000, 5.0f, 2.5f));*/
	
	for (int i=0;i<resonance.size();i++)
		resonance[i].init_filter(mixer);
}

Instrument::Instrument(Deserializer& deser):mixer(deser.get_mixer())
{
	deser >> name >> resonance;
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
	Deserializer::Tag _tag(deser, "Instrument");
	if (!_tag.is_valid()) return nullptr;
	
	std::string classtype;
	_tag >> tag("class", classtype);
	
	Instrument* instr=create(deser.get_mixer(), classtype.c_str());
	instr->do_deserialize(_tag);
	
	return instr;
}
	
void Instrument::do_serialize(Serializer& ser) const
{
	ser << tag("name", name) << tag("resonance", resonance);
}

void Instrument::do_deserialize(Deserializer& deser)
{
	deser >> tag("name", name) >> tag("resonance", resonance);
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
