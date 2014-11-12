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
 
#include "synthbits.h"
#include "filter.h"
#include "module.h"
#include "sequencer.h"

namespace Synth {

Tone::Tone(const Instrument& instr):instrument(instr)
{
	stopped=false;
	decayed=false;
	
	channel=-1;
	volume=0;
	modulation=1.0f;
	
	filters.push_back(new VolumeScalerInstance);
	
	for (auto& res: instr.resonance)
		for (int i=0;i<res.order;i++)
			filters.push_back(new BiQuadFilterInstance(res.filter));
	
	instr.mixer.add_tone(this);
}

Tone::~Tone()
{
	for (auto f: filters)
		delete f;
}

void Tone::generate(float** samples, int count)
{
	synth(samples, count);
	
	for (auto f: filters)
		f->apply(samples, count);
}

void Tone::apply_effect(unsigned short effect)
{
	switch(effect>>8) {
	case Pattern::MODULATION:
		modulation=ldexpf(effect&0xff, -8);
		break;
	default:
		for (auto f: filters)
			f->handle_effect(effect);
	}
}


ExcitationModelTone::ExcitationModelTone(const ExcitationModelInstrument& instr, float freq, int vol):Tone(instr)
{
	decay_rate=instr.decay_rate * powf(freq/440.0f, instr.decay_scale) / instr.mixer.get_samplerate();
	excitation_rate=ldexpf(vol, -7) * instr.continuous_excitation * decay_rate;
	energy=ldexpf(vol, -7) * instr.discrete_excitation;
}

ExcitationModelTone::~ExcitationModelTone()
{
}

void ExcitationModelTone::compute_excitation(float* energy_buffer, int num)
{
	while (num-->0) {
		if (!stopped)
			energy+=excitation_rate;
		
		energy*=1.0f-decay_rate;
		
		*energy_buffer++=energy;
	}
	
	if (stopped && energy<0.0001f)
		decayed=true;
}


Mixer::Mixer():final_highpass_filter(BiQuad::dcstop(0.998f)),highpass_l(final_highpass_filter),highpass_r(final_highpass_filter)
{
	numchannels=2;
	samplerate=48000;
	
	master_volume=1.0f;
}

Mixer::~Mixer()
{
}

void Mixer::add_tone(Tone* tone)
{
	tones.push_back(tone);
}

void Mixer::stop_all_tones()
{
	std::list<Tone*>::iterator ti=tones.begin();
	while (ti!=tones.end()) {		
		(*ti)->stop();
		ti++;
	}
}

void Mixer::stop_all_tones_on_channel(int ch)
{
	std::list<Tone*>::iterator ti=tones.begin();
	while (ti!=tones.end()) {		
		if ((*ti)->get_channel()==ch)
			(*ti)->stop();
		
		ti++;
	}
}

void Mixer::kill_all_tones_with_instrument(Instrument* instr)
{
	std::list<Tone*>::iterator ti=tones.begin();
	while (ti!=tones.end())
		if ((*ti)->get_instrument()==instr) {
			(*ti)->stop();
			
			//FIXME: this crashes - why?
			//delete *ti;
			
			ti=tones.erase(ti);
		}
		else
			ti++;
}

void Mixer::apply_effect_on_channel(int ch, unsigned short effect)
{
	std::list<Tone*>::iterator ti=tones.begin();
	while (ti!=tones.end()) {		
		if ((*ti)->get_channel()==ch)
			(*ti)->apply_effect(effect);
		
		ti++;
	}
}

void Mixer::mix(float** samples, int count)
{
	Tone* active_voices[tones.size()];
	int num_active_voices=0;
	
	std::list<Tone*>::iterator ti=tones.begin();
	while (ti!=tones.end())
		if ((*ti)->is_decayed()) {
			delete *ti;
			ti=tones.erase(ti);
		}
		else {
			active_voices[num_active_voices++]=*ti;
			ti++;
		}
	
	float buffer[num_active_voices*numchannels*count];
	
#pragma omp parallel for
	for (int i=0;i<num_active_voices;i++) {
		float* ptr[2];
		
		ptr[0]=buffer + i*numchannels*count;
		ptr[1]=buffer + (i*numchannels+1)*count;
		
		active_voices[i]->generate(ptr, count);
	}
	
	for (int i=0;i<count;i++) {
		float l=0, r=0;
		
		for (int j=0;j<num_active_voices;j++) {
			l+=buffer[j*numchannels*count+i];
			r+=buffer[(j*numchannels+1)*count+i];
		}
		
		samples[0][i]=highpass_l(l) * master_volume * 0.25f;
		samples[1][i]=highpass_r(r) * master_volume * 0.25f;
	}
}

Sequencer::Sequencer(Mixer& m):mixer(m)
{
	module=0;
	cur_row=0;
}

Sequencer::~Sequencer()
{
}

void Sequencer::set_module(const Module* m)
{
	module=m;
	
	reset_position();
}

void Sequencer::process_row()
{
	Pattern& pattern=*get_current_pattern();

	for (int i=0;i<16;i++) {
		const Pattern::Note& note=pattern(i, cur_row);
		
		if (note.flags&1)
			mixer.stop_all_tones_on_channel(i);
	
		if ((note.flags & 3) == 3) {
			Instrument* instr=module->instruments[note.instrument];
			
			if (instr!=0) {
				Tone* tone=instr->play_note(note.note, (note.flags&4) ? note.volume : 128);
				tone->set_channel(i);
				
				if (note.flags&4)
					tone->set_volume(note.volume);

				if (note.flags & 8)
					tone->apply_effect(note.effect);
			}
		}
	}
	
	cur_row++;
	
	if (cur_row==64) {
		cur_row=0;
		if (++cur_position==module->arrangement.end())
			cur_position=module->arrangement.begin();
	}
}

void Sequencer::reset_position()
{
	cur_row=0;
	cur_position=module->arrangement.begin();
}

}


