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
 
#ifndef INCLUDE_SYNTH_CUSTOMSYNTH_H
#define INCLUDE_SYNTH_CUSTOMSYNTH_H

#include <vector>
#include <string>
#include <functional>
#include "synthbits.h"
#include "module.h"

namespace Synth {
	
class CustomSynth:public Instrument {
public:
	class Tone;
	class Editor;

	struct location_t {
		int	begin;
		int	end;
	};
	
	struct context_t {
		int	blocklength;
		float*	buffer;
		
		float* operator[](int index)
		{
			return buffer + index*blocklength;
		}
	};
	
	typedef std::function<float(const Tone&)> uniform_t;

	typedef std::function<void(context_t&, int)> varying_eval_t;
	typedef std::function<varying_eval_t(Tone&)> varying_t;

	struct filter_instance_t {
		virtual varying_eval_t mkeval(int src) = 0;
	};
	
	typedef std::function<filter_instance_t*(const Tone&)> mkfilterinstance_t;
	
	struct filter_t {
		std::string			name;
		mkfilterinstance_t	mkinstance;
	};
	
	struct biquad_filter_instance_t:filter_instance_t {
		BiQuad	filter;
		
		biquad_filter_instance_t(const BiQuad& f):filter(f) {}

		virtual varying_eval_t mkeval(int src);
		
		static mkfilterinstance_t mklowpass(const uniform_t& freq, const uniform_t& Q);
		static mkfilterinstance_t mkhighpass(const uniform_t& freq, const uniform_t& Q);
		static mkfilterinstance_t mkbandpass(const uniform_t& freq, const uniform_t& Q);
	};
	
	struct comb_filter_instance_t:filter_instance_t {
		int		delaylength;
		float	coeffs[2];
		float	alpha;
		float	beta;
		
		comb_filter_instance_t(float freq, float alpha, float beta);
		
		virtual varying_eval_t mkeval(int src);
		
		static mkfilterinstance_t mkcomb(const uniform_t& freq, const uniform_t& alpha, const uniform_t& beta);
	};
	
	CustomSynth(Mixer&);
	
	static Instrument* create(Mixer&);
	
	virtual Synth::Tone* play_note(int, int);
	
	void add_filter(const std::string& name, const std::function<filter_instance_t*(const Tone&)>& mkinstance);
	
	int add_varying(varying_t v);
	int add_varying_for_filter(int filterno, int srcvarying);
	
	void compile();
	
	void set_source(const std::string& s)
	{
		source_code=s;
	}
	
	const std::string& get_source() const
	{
		return source_code;
	}
	
	void report_compile_error(const std::string&, const location_t&);
	
	int get_filter_by_name(const char* str) const;
	
	static float get_uniform_constant(const Tone&);
	
private:
	virtual void do_serialize(Serializer&) const;
	virtual void do_deserialize(Deserializer&);
	
	std::string					source_code;

	std::string					compile_error_msg;
	location_t					compile_error_location;
	
	std::vector<varying_t>		varyings;
	std::vector<filter_t>		filters;
	
	int		output_varyings[2];
};
	
}

#endif
