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
 
namespace Synth {

class PWMSynth:public Instrument {
public:
	class Tone;
	
	PWMSynth(Mixer&);
	virtual ~PWMSynth();
	
	static Instrument* create(Mixer&);
	
	virtual Synth::Tone* play_note(int, int);	
	
protected:
	virtual void do_serialize(Serializer&) const;
	virtual void do_deserialize(Deserializer&);
};

}

