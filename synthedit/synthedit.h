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
 
#ifndef INCLUDE_SYNTHEDIT_SYNTHEDIT_H
#define INCLUDE_SYNTHEDIT_SYNTHEDIT_H

namespace GUI {
class Window;
}

namespace Synth {

class Instrument;

template<typename TModel>
class Controller {
protected:
	TModel*		model;
	
	Controller(TModel* m):model(m) {}
};

class IInstrumentEditorProvider {
protected:
	IInstrumentEditorProvider*			next;
	static IInstrumentEditorProvider*	first;
	
	IInstrumentEditorProvider();
	
public:
	virtual GUI::Window* create_instance(Instrument*)=0;
	
	static GUI::Window* create(Instrument*);
};


template<typename TInstrument, typename TEditor>
class InstrumentEditor {
	class Provider:IInstrumentEditorProvider {
		virtual GUI::Window* create_instance(Instrument* instr)
		{
			if (TInstrument* theinstr=dynamic_cast<TInstrument*>(instr))
				return new TEditor(theinstr);
			else
				return next ? next->create_instance(instr) : nullptr;
		}
	};

	static Provider	provider;
	
protected:
	TInstrument*	instrument;
	
	InstrumentEditor(TInstrument* instr):instrument(instr)
	{
		(void) provider;
	}
};

template<typename TInstrument, typename TEditor>
typename InstrumentEditor<TInstrument, TEditor>::Provider InstrumentEditor<TInstrument, TEditor>::provider;

}

#endif
