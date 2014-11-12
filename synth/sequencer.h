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
 
#ifndef INCLUDE_SYNTH_SEQUENCER_H
#define INCLUDE_SYNTH_SEQUENCER_H

#include <list>

namespace Synth {

#define Voice Tone

class IFilterInstance;

class Tone {
public:
	Tone(const Instrument&);
	virtual ~Tone();
	
	void stop() { stopped=true; }

	bool is_decayed() const { return decayed; }
	
	void generate(float**, int);
	
	void set_channel(int c) { channel=c; }
	int get_channel() const { return channel; }
	
	void set_volume(int v) { volume=v; }
	virtual void apply_effect(unsigned short);
	
	const Instrument* get_instrument() const
	{ return &instrument; }
	
protected:
	virtual void synth(float**, int)=0;
	
	void add_filter(IFilterInstance* filter)
	{
		filters.push_back(filter);
	}
	
	bool	stopped;
	bool	decayed;
	
	int		volume;
	float	modulation;
	
private:
	const Instrument&	instrument;

	std::vector<IFilterInstance*>	filters;
	
	int			channel;
};


class ExcitationModelTone:public Tone {
public:
	ExcitationModelTone(const ExcitationModelInstrument&, float freq, int vol);
	virtual ~ExcitationModelTone();
	
protected:
	void compute_excitation(float*, int);
	
private:
	float	energy;
	float	excitation_rate;
	float	decay_rate;
};


class Mixer {
public:
	Mixer();
	~Mixer();
	
	void set_master_volume(float vol)
	{
		master_volume=vol;
	}
	
	void add_tone(Tone*);
	void stop_all_tones();
	void stop_all_tones_on_channel(int);
	
	void kill_all_tones_with_instrument(Instrument*);
	
	int get_active_tones_count() const
	{
		return tones.size();
	}
	
	void apply_effect_on_channel(int, unsigned short);
	
	void mix(float**, int);
	
	int get_samplerate() const { return samplerate; }
	
	float freq2omega(float freq) const
	{
		return 2 * M_PI * freq / samplerate;
	}
	
	float note2omega(int note) const
	{
		return freq2omega(Instrument::note2freq(note));
	}
	
	float randf()
	{
		return ldexpf(rng()>>40, -24);
	}
	
private:
	int			numchannels;
	int			samplerate;
	
	float		master_volume;
	
	std::list<Tone*>	tones;
	
	xorshift64	rng;
	
	BiQuad			final_highpass_filter;
	BiQuad::Filter		highpass_l;
	BiQuad::Filter		highpass_r;
};

class Sequencer {
public:
	Sequencer(Mixer&);
	~Sequencer();
	
	void set_module(const Module*);
	
	void process_row();
	void reset_position();
	
	int get_current_row() const
	{
		return cur_row;
	}
	
	Pattern* get_current_pattern() const
	{
		return *cur_position;
	}
	
	const Module::arrangement_item_t* get_current_arrangement_item() const
	{
		return &*cur_position;
	}
	
private:
	Mixer&		mixer;
	const Module*	module;
	
	int		cur_row;
	std::list<Module::arrangement_item_t>::const_iterator	cur_position;
};

}

#endif
