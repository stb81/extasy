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
 
#ifndef INCLUDE_SYNTH_KICKDRUM_H
#define INCLUDE_SYNTH_KICKDRUM_H

#include "synthbits.h"
#include "module.h"
#include "sequencer.h"

namespace Synth {

class KickDrum:public Instrument {
	class Tone;
	class Editor;

public:
	KickDrum(Mixer&);
	virtual ~KickDrum();
	
	virtual Synth::Tone* play_note(int, int);
	
	static Instrument* create(Mixer&);
	
protected:
	virtual void do_serialize(Serializer&) const;
	virtual void do_deserialize(Deserializer&);

private:
	float	basefreq;
	float	tpower;
	float	carrier_decay;
	float	modulator_decay;
};

}

#endif
