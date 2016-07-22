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
 
#ifndef INCLUDE_SYNTH_MODULE_H
#define INCLUDE_SYNTH_MODULE_H

#include <vector>
#include <list>
#include <string>

class FormantEditor;

namespace Synth {

#define NOTE_OFF	120
#define NOTE_HIT	121


class Serializer;
class Deserializer;


class Module;


class Pattern {
public:
	enum Effect {
		MODULATION=22,
		PANNING=25
	};

	struct Note {
		unsigned char	flags;
		unsigned char	note;
		unsigned char	instrument;
		unsigned char	volume;
		unsigned short	effect;
	};
	
	Pattern(const Module&);
	Pattern(Deserializer&);
	~Pattern();

	void serialize(Serializer&) const;
	static Pattern* deserialize(Deserializer&);
	
	Note& operator()(int ch, int row)
	{ return notes[ch+row*16]; }
	
	const Note& operator()(int ch, int row) const
	{ return notes[ch+row*16]; }
	
	void set_name(const char* n)
	{
		name=n;
	}
	
	const char* get_name() const
	{
		return name.c_str();
	}
	
	const Module& get_module() const
	{
		return module;
	}
	
	int get_index() const;
	
private:
	const Module&	module;
	mutable int		index=-1;
	
	Note*		notes;
	
	std::string	name;
};

class Tone;
class Mixer;

class Instrument {
	friend class Tone;
	friend class ::FormantEditor;
	
public:
	enum InstrumentCapabilities {
		CAP_STEREO=1,
		CAP_DISCRETE_EXCITATION=2,
		CAP_CONTINUOUS_EXCITATION=4
	};
	
	Instrument(Mixer&);
	virtual ~Instrument();
	
	virtual Tone* play_note(int, int)=0;
	
	void set_name(const char* n)
	{
		name=n;
	}
	
	const char* get_name() const
	{
		return name.c_str();
	}
	
	void set_color(uint32_t col)
	{
		color=col;
	}
	
	uint32_t get_color() const
	{
		return color;
	}
	
	Mixer& get_mixer() const
	{
		return mixer;
	}
	
	static float note2freq(int note)
	{
		// FIXME: write out frequencies for a single octave, and then transpose with ldexp
		return 440 * expf((note-69)*M_LN2/12);
	}

	static Instrument* create(Mixer&, const char*);
	static void register_class(const char*, Instrument*(*)(Mixer&));
	static int get_class_count() { return classes.size(); }
	static const char* get_class_name(int index) { return classes[index].first; }
	
	void serialize(Serializer&) const;
	static Instrument* deserialize(Deserializer&);
	
protected:
	Mixer&		mixer;
	
	virtual void do_serialize(Serializer&) const;
	virtual void do_deserialize(Deserializer&);

private:
	struct Resonance {
		enum type_t {
			PEAKINGEQ,
			LOWSHELF,
			HIGHSHELF,
			BANDSTOP,
			BANDPASS,
			LOWPASS,
			HIGHPASS,
			ALLPASS,
			RESONANCE
		};
		
		type_t	type;
		int		order;
		float	freq;
		float	Q;
		float	gain;
		BiQuad	filter;
		
		Resonance() {}
		Resonance(float f, float Q, float gain):freq(f), Q(Q), gain(gain), type(PEAKINGEQ), order(1) {}
		
		void init_filter(const Mixer&);
		
		void serialize(Serializer&) const;
		void deserialize(Deserializer&);
	};
	
	std::string				name;
	std::vector<Resonance>	resonance;
	
	uint32_t				color;
	
	static std::vector<std::pair<const char*, Instrument* (*)(Mixer&)> >	classes;
};


class ExcitationModelInstrument:public Instrument {
	friend class ExcitationModelTone;
	
public:
	class Editor;
	
	ExcitationModelInstrument(Mixer&);
	virtual ~ExcitationModelInstrument();
	
protected:
	float	attack_time;
	float	discrete_excitation;
	float	continuous_excitation;
	float	decay_rate;
	float	decay_scale;
};


struct Module {
	struct arrangement_item_t {
		Pattern*	pattern;
		uint32_t	channelmask;
		
		operator Pattern*() const
		{
			return pattern;
		}
		
		void serialize(Serializer&) const;
		void deserialize(Deserializer&);
	};
	
	Instrument*	instruments[256];
	
	std::vector<Pattern*>			patterns;
	std::list<arrangement_item_t>	arrangement;
	
	Module();
	Module(Deserializer&);
	
	void serialize(Serializer&) const;
	static Module* deserialize(Deserializer&);
};

}

#endif
