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
 
#ifndef INCLUDE_SYNTH_PADSYNTH_H
#define INCLUDE_SYNTH_PADSYNTH_H

namespace Synth {

class PadSynth:public ExcitationModelInstrument {
	class Pad;
	
public:
	class Tone;
	class Editor;
	
	PadSynth(Mixer&, float);
	virtual ~PadSynth();
	
	virtual Synth::Tone* play_note(int, int);
	
	static Instrument* create(Mixer&);
	
protected:
	virtual void do_serialize(Serializer&) const;
	virtual void do_deserialize(Deserializer&);

private:
	void	create_random_timbre();
	void	build_pad();
	
	float*	band_coefficients;
	float*	spectrum;
	
	int		length;
	Pad*	samples[8];
	
	float	lowcut;
	float	brightness;
	float	richness;
	float	bandwidth;
	float	bwexponent;
};

}

#endif
