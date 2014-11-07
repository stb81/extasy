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
 
#include "gui/gui.h"
#include "synth/synthbits.h"
#include "synth/module.h"
#include "synth/sequencer.h"
#include "padsynthedit.h"

using namespace GUI;

namespace Synth {
	
PadSynth::Editor::Editor(PadSynth* instr):Window(128, 128, 640, 480)
{
	instrument=instr;
	
	set_title("Edit Pad Synth Instrument");
	set_modal(true);
	
	Button* but=new Button(64, 64, 128, 24, "Close");
	but->clicked.connect(sigc::mem_fun(this, &Window::close));
	add(but);
	
	but=new Button(64, 96, 128, 24, "Random Timbre");
	but->clicked.connect(sigc::mem_fun(instrument, &PadSynth::create_random_timbre));
	add(but);
	
	but=new Button(256, 96, 128, 24, "Update");
	but->clicked.connect(sigc::mem_fun(instrument, &PadSynth::build_pad));
	add(but);
	
	Label* label=new Label(64, 176, 128, 16, "Brightness");
	add(label);
	
	exponent_knob=new Knob(64, 192, 32, 32);
	exponent_knob->set_range(0.0f, 2.0f);
	exponent_knob->set_value(instrument->exponent);
	exponent_knob->value_changed.connect(sigc::hide(sigc::hide(sigc::mem_fun(this, &Editor::knob_changed))));
	add(exponent_knob);
	
	label=new Label(192, 176, 128, 16, "Discrete Exc.");
	add(label);
	
	discrete_knob=new Knob(192, 192, 32, 32);
	discrete_knob->set_range(0.0f, 1.0f);
	discrete_knob->set_value(instrument->discrete_excitation);
	discrete_knob->value_changed.connect(sigc::hide(sigc::hide(sigc::mem_fun(this, &Editor::knob_changed))));
	add(discrete_knob);
	
	label=new Label(320, 176, 128, 16, "Continuous Exc.");
	add(label);
	
	continuous_knob=new Knob(320, 192, 32, 32);
	continuous_knob->set_range(0.0f, 1.0f);
	continuous_knob->set_value(instrument->continuous_excitation);
	continuous_knob->value_changed.connect(sigc::hide(sigc::hide(sigc::mem_fun(this, &Editor::knob_changed))));
	add(continuous_knob);
	
	label=new Label(448, 176, 128, 16, "Decay Rate");
	add(label);
	
	decay_knob=new Knob(448, 192, 32, 32);
	decay_knob->set_range(0.5f, 50.0f, Knob::LOGARITHMIC);
	decay_knob->set_value(instrument->decay_rate);
	decay_knob->value_changed.connect(sigc::hide(sigc::hide(sigc::mem_fun(this, &Editor::knob_changed))));
	add(decay_knob);
	
	label=new Label(64, 240, 128, 16, "Bandwidth");
	add(label);
	
	bw_knob=new Knob(64, 256, 32, 32);
	bw_knob->set_range(0.02f, 1.0f, Knob::LOGARITHMIC);
	bw_knob->set_value(instrument->bandwidth);
	bw_knob->value_changed.connect(sigc::hide(sigc::hide(sigc::mem_fun(this, &Editor::knob_changed))));
	add(bw_knob);
	
	label=new Label(448, 240, 128, 16, "Decay Scale");
	add(label);
	
	decay_knob2=new Knob(448, 256, 32, 32);
	decay_knob2->set_range(0.0f, 1.0f, Knob::LINEAR);
	decay_knob2->set_value(instrument->decay_scale);
	decay_knob2->value_changed.connect(sigc::hide(sigc::hide(sigc::mem_fun(this, &Editor::knob_changed))));
	add(decay_knob2);
}

void PadSynth::Editor::knob_changed()
{
	instrument->exponent=exponent_knob->get_value();
	instrument->discrete_excitation=discrete_knob->get_value();
	instrument->continuous_excitation=continuous_knob->get_value();
	instrument->decay_rate=decay_knob->get_value();
	instrument->decay_scale=decay_knob2->get_value();
	instrument->bandwidth=bw_knob->get_value();
}

Window* PadSynth::Editor::create(Instrument* instr)
{
	PadSynth* pls=dynamic_cast<PadSynth*>(instr);
	
	return pls ? new Editor(pls) : NULL;
}
	
}
