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
 
#ifndef INCLUDE_FORMANTEDIT_H
#define INCLUDE_FORMANTEDIT_H
	
class FormantEditor:public GUI::Window {
	
	Instrument*		instrument;
	
	ListBox*	formant_list;
	
	CurveView*	amp_response;
	CurveView*	phase_response;
	CurveView*	group_delay;
	
	Knob*		knob_freq;
	Knob*		knob_Q;
	Knob*		knob_gain;
	
	SpinBox*	order_spinner;
	
	RadioButtonGroup	rbgtype;
	RadioButton*	rbtypes[9];
	
	class ResponseCurve:public CurveView::Curve {
	public:
		enum type_t {
			AMPLITUDE,
			PHASE,
			GROUPDELAY
		};
		
		ResponseCurve(Instrument* instr, type_t t):instrument(instr), type(t) {}
		
	private:
		virtual float evaluate(float) const;

		Instrument*		instrument;
		type_t			type;
	};
	
	class FormantColumn:public ListBox::Column {
	public:
		enum type_t {
			TYPE,
			FREQUENCY,
			Q,
			GAIN
		};
		
		FormantColumn(Instrument* instr, type_t t):Column(t ? 128 : 32), instrument(instr), type(t) {}
		
	private:
		virtual void draw(int x0, int y0, int row, bool highlighted) const;
		
		Instrument*		instrument;
		type_t			type;
	};
	
	void update_knobs();
	void knob_changed();
	void type_changed(int);
	
	void add_formant();
	void remove_formant();
	
public:
	FormantEditor(Instrument*);
};

#endif
