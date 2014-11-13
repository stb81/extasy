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

using namespace GUI;
using namespace Synth;

#include "formantedit.h"

extern Mixer* mixer;

float FormantEditor::ResponseCurve::evaluate(float x) const
{
	float freq=powf(2.0f, (x-1.0f)*10.0f);
	
	std::complex<float> z=std::polar<float>(1.0f, M_PI*20/24*freq);
	
	std::complex<float> H(0, 0);
	std::complex<float> Hd(0, 0);
	for (int i=0;i<instrument->resonance.size();i++) {
		H+=instrument->resonance[i].filter.response(z) * float(instrument->resonance[i].order);
		Hd+=instrument->resonance[i].filter.dresponse(z) * float(instrument->resonance[i].order);
	}
	
	Hd*=z;
	
	if (type==AMPLITUDE)
		return H.real() / M_LN10 * 0.0625f + 0.5f;
	else if (type==PHASE)
		return H.imag() / M_PI / 2 + 0.5f;
	else
		return H.imag() / freq * 0.001f + 0.5f;
		//return Hd.real()*0.001f + 0.5f;
}

void FormantEditor::FormantColumn::draw(int x0, int y0, int row, bool highlight) const
{
	static const char* typecodes[]={ "EQ", "LS", "HS", "BS", "BP", "LP", "HP", "AP", "RE" };
	
	float c=highlight ? 1.0f : 0.7f;
	
	switch (type) {
	case TYPE:
		textprintf(x0, y0, c, c, c, typecodes[instrument->resonance[row].type]);
		break;
	case FREQUENCY:
		textprintf(x0, y0, c, c, c, "f=%.1f Hz", instrument->resonance[row].freq);
		break;
	case Q:
		textprintf(x0, y0, c, c, c, "Q=%.1f", instrument->resonance[row].Q);
		break;
	case GAIN:
		textprintf(x0, y0, c, c, c, "%.1f dB", instrument->resonance[row].gain);
		break;
	}
}

FormantEditor::FormantEditor(Instrument* instr):Window(256, 0, 1056, 1040)
{
	instrument=instr;
	
	set_title("Resonance");
	//set_modal(true);
	
	Button* but=new Button(544, 96, 128, 24, "Close");
	but->clicked.connect(sigc::mem_fun(this, &Window::close));
	add(but);
	
	but=new Button(544, 32, 96, 24, "Add");
	but->clicked.connect(sigc::mem_fun(this, &FormantEditor::add_formant));
	add(but);
	
	but=new Button(544, 64, 96, 24, "Remove");
	but->clicked.connect(sigc::mem_fun(this, &FormantEditor::remove_formant));
	add(but);
	
	formant_list=new ListBox(16, 24, 512, 184);
	formant_list->add_column(new FormantColumn(instrument, FormantColumn::TYPE));
	formant_list->add_column(new FormantColumn(instrument, FormantColumn::FREQUENCY));
	formant_list->add_column(new FormantColumn(instrument, FormantColumn::Q));
	formant_list->add_column(new FormantColumn(instrument, FormantColumn::GAIN));
	formant_list->set_entry_count(instrument->resonance.size());
	formant_list->selection_changed.connect(sigc::hide(sigc::mem_fun(this, &FormantEditor::update_knobs)));
	add(formant_list);
	
	knob_freq=new Knob(544, 160, 32, 64);
	knob_freq->set_range(20, 20000, Knob::LOGARITHMIC);
	knob_freq->value_changed.connect(sigc::hide(sigc::hide(sigc::mem_fun(this, &FormantEditor::knob_changed))));
	add(knob_freq);

	knob_Q=new Knob(592, 160, 32, 64);
	knob_Q->set_range(0.25f, 256.0f, Knob::LOGARITHMIC);
	knob_Q->value_changed.connect(sigc::hide(sigc::hide(sigc::mem_fun(this, &FormantEditor::knob_changed))));
	add(knob_Q);

	knob_gain=new Knob(640, 160, 32, 64);
	knob_gain->set_range(-40.0f, 40.0f);
	knob_gain->value_changed.connect(sigc::hide(sigc::hide(sigc::mem_fun(this, &FormantEditor::knob_changed))));
	add(knob_gain);
	
	order_spinner=new SpinBox(688, 168, 64, 24);
	order_spinner->set_range(1, 8);
	order_spinner->value_changed.connect(sigc::hide(sigc::mem_fun(this, &FormantEditor::knob_changed)));
	add(order_spinner);
	
	ResponseCurve* rc=new ResponseCurve(instrument, ResponseCurve::AMPLITUDE);
	rc->set_color(Color(0, 255, 0));

	ResponseCurve* rc2=new ResponseCurve(instrument, ResponseCurve::PHASE);
	rc2->set_color(Color(255, 0, 0));

	ResponseCurve* rc3=new ResponseCurve(instrument, ResponseCurve::GROUPDELAY);
	rc3->set_color(Color(64, 128, 255));
	
	amp_response=new CurveView(16, 224, 1024, 256);
	amp_response->set_description("Frequency Response");
	amp_response->add_curve(rc);
	add(amp_response);
	
	phase_response=new CurveView(16, 496, 1024, 256);
	phase_response->set_description("Phase Response");
	phase_response->add_curve(rc2);
	add(phase_response);
	
	group_delay=new CurveView(16, 768, 1024, 256);
	group_delay->set_description("Group Delay");
	group_delay->add_curve(rc3);
	add(group_delay);
	
	static const char* typenames[]={ "Peaking EQ", "Low Shelf", "High Shelf", "Band Stop", "Band Pass", "Low Pass", "High Pass", "All Pass", "Resonance" };
	for (int i=0;i<9;i++) {
		rbtypes[i]=new RadioButton(768, 8+i*24, 240, 16, typenames[i], rbgtype);
		rbtypes[i]->clicked.connect(sigc::bind(sigc::mem_fun(this, &FormantEditor::type_changed), i));
		add(rbtypes[i]);
	}
	
	update_knobs();
}

void FormantEditor::add_formant()
{
	mixer->kill_all_tones_with_instrument(instrument);
	
	Instrument::Resonance res(1000, 4, 0);
	res.init_filter(*mixer);
	
	instrument->resonance.push_back(res);	

	formant_list->set_entry_count(instrument->resonance.size());
	formant_list->set_selection(instrument->resonance.size()-1);
	
	update_knobs();
}

void FormantEditor::remove_formant()
{
	mixer->kill_all_tones_with_instrument(instrument);

	int sel=formant_list->get_selection();
	instrument->resonance.erase(instrument->resonance.begin()+sel);
	
	formant_list->set_entry_count(instrument->resonance.size());
	
	update_knobs();	
}

void FormantEditor::update_knobs()
{
	int i=formant_list->get_selection();
	if (i<0 || i>=instrument->resonance.size()) return;
	
	Instrument::Resonance& res=instrument->resonance[i];

	knob_freq->set_value(res.freq);
	knob_Q->set_value(res.Q);
	knob_gain->set_value(res.gain);
	order_spinner->set_value(res.order);
	
	rbtypes[res.type]->select();
}

void FormantEditor::knob_changed()
{
	int i=formant_list->get_selection();
	if (i<0 || i>=instrument->resonance.size()) return;
	
	Instrument::Resonance& res=instrument->resonance[i];
	
	res.freq=knob_freq->get_value();
	res.Q=knob_Q->get_value();
	res.gain=knob_gain->get_value();
	res.order=order_spinner->get_value();
	
	res.init_filter(*mixer);
}

void FormantEditor::type_changed(int type)
{
	int i=formant_list->get_selection();
	if (i<0 || i>=instrument->resonance.size()) return;
	
	Instrument::Resonance& res=instrument->resonance[i];
	
	res.type=(Instrument::Resonance::type_t) type;

	res.init_filter(*mixer);
}

