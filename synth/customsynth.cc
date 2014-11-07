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
 
#include <stdio.h>
#include <math.h>
#include "synthbits.h"
#include "module.h"
#include "sequencer.h"
#include "serialization.h"
#include "customsynth.h"
#include "customscanner.h"

namespace Synth {

class CustomSynth::Tone:public Synth::Tone {
	friend class CustomSynth;
	
	const CustomSynth&			customsynth;
	
	int							num_varying_expressions;
	varying_eval_t*				varying_evals;
	
	int							num_filter_instances;
	filter_instance_t**			filter_instances;
	
	float						omega;
	int							decay_counter;
	
public:
	Tone(const CustomSynth&, float);
	virtual ~Tone();
	
	virtual void synth(float**, int);
	
	filter_instance_t*	get_filter_instance(int index) const
	{
		return filter_instances[index];
	}
};


CustomSynth::CustomSynth(Mixer& m):Instrument(m)
{
}

Instrument* CustomSynth::create(Mixer& m)
{
	return new CustomSynth(m);
}

Synth::Tone* CustomSynth::play_note(int note, int vol)
{
	return new Tone(*this, mixer.note2omega(note));
}

void CustomSynth::do_serialize(Serializer& ser) const
{
	ser << tag("class", "CustomSynth");
	
	Instrument::do_serialize(ser);
	
	ser << tag("source", source_code);
}

void CustomSynth::do_deserialize(Deserializer& deser)
{
	Instrument::do_deserialize(deser);

	deser >> tag("source", source_code);
}

void CustomSynth::compile()
{
	mixer.kill_all_tones_with_instrument(this);
	
	CustomScanner lexer(*this, source_code);
	CustomParser parser(*this, lexer);
	
	compile_error_msg="Compilation Successful";
	compile_error_location.begin=compile_error_location.end=0;
	
	filters.clear();
	
	varyings.clear();
	if (parser.parse()) {
		varyings.clear();
		output_varyings[0]=output_varyings[1]=-1;
	}
	
	output_varyings[0]=lexer.look_up_variable("left");
	output_varyings[1]=lexer.look_up_variable("right");
}

void CustomSynth::add_filter(const std::string& name, const std::function<filter_instance_t*(const Tone&)>& mkinstance)
{
	filters.push_back(filter_t());
	
	filter_t& filter=filters.back();
	filter.name=name;
	filter.mkinstance=mkinstance;
}

int CustomSynth::add_varying(varying_t v)
{
	int index=varyings.size();
	varyings.push_back(v);
	return index;
}
	
void CustomSynth::report_compile_error(const std::string& msg, const location_t& loc)
{
	compile_error_msg=msg;
	compile_error_location=loc;
	
	if (loc.begin==loc.end)
		compile_error_location.end++;
}

int CustomSynth::get_filter_by_name(const char* name) const
{
	for (int i=0;i<filters.size();i++)
		if (filters[i].name==name)
			return i;
			
	return -1;
}

int CustomSynth::add_varying_for_filter(int filterno, int srcvarying)
{
	return add_varying([filterno, srcvarying](Tone& tone)->varying_eval_t {
		return tone.get_filter_instance(filterno)->mkeval(srcvarying);
	});
}

float CustomSynth::get_uniform_constant(const Tone& tone)
{
	return tone.omega;
}

CustomSynth::Tone::Tone(const CustomSynth& synth, float omega):Synth::Tone(synth), customsynth(synth), omega(omega)
{
	num_filter_instances=synth.filters.size();
	if (num_filter_instances) {
		filter_instances=new filter_instance_t*[num_filter_instances];
		
		for (int i=0;i<num_filter_instances;i++)
			filter_instances[i]=synth.filters[i].mkinstance(*this);
	}
	else
		filter_instances=nullptr;
	
	num_varying_expressions=synth.varyings.size();
	varying_evals=new varying_eval_t[num_varying_expressions];
	
	for (int i=0;i<num_varying_expressions;i++)
		varying_evals[i]=synth.varyings[i](*this);
	
	decay_counter=0;
}

CustomSynth::Tone::~Tone()
{
	for (int i=0;i<num_filter_instances;i++)
		delete filter_instances[i];
	
	delete[] filter_instances;
	delete[] varying_evals;
}

void CustomSynth::Tone::synth(float** buffer, int numsamples)
{
	context_t context;
	
	context.blocklength=numsamples;
	context.buffer=new float[context.blocklength*num_varying_expressions];
	
	for (int i=0;i<num_varying_expressions;i++)
		varying_evals[i](context, i);
	
	const float decayed_threshold=0.0001f;
	
	decay_counter+=numsamples;
	for (int i=0;i<2;i++)
		if (customsynth.output_varyings[i]>=0) {
			float* src=context.buffer + context.blocklength*customsynth.output_varyings[i];
			
			for (int j=0;j<context.blocklength;j++) {
				float v=src[j];
				buffer[i][j]=v;
				if (v>decayed_threshold || v<-decayed_threshold)
					decay_counter=0;
			}
		}
		else
			for (int j=0;j<context.blocklength;j++)
				buffer[i][j]=0.0f;
		
	if (stopped && decay_counter>customsynth.mixer.get_samplerate()/8)
		decayed=true;
	
	delete[] context.buffer;
}

CustomSynth::varying_eval_t CustomSynth::biquad_filter_instance_t::mkeval(int src)
{
	struct eval_t {
		int				src;
		BiQuad::Filter	filter;
		
		eval_t(const BiQuad& coeffs, int src):src(src), filter(coeffs) {}
		
		void operator()(context_t& ctx, int dst)
		{
			float* srcbuf=ctx.buffer + src*ctx.blocklength;
			float* dstbuf=ctx.buffer + dst*ctx.blocklength;
			
			for (int i=0;i<ctx.blocklength;i++)
				*dstbuf++=filter(*srcbuf++);
		}
	};
	
	return eval_t(filter, src);
}

CustomSynth::mkfilterinstance_t CustomSynth::biquad_filter_instance_t::mklowpass(const uniform_t& freq, const uniform_t& Q)
{
	return [freq, Q](const Tone& tone)->filter_instance_t* {
		return new biquad_filter_instance_t(BiQuad::lowpass(freq(tone), Q(tone)));
	};
}

CustomSynth::mkfilterinstance_t CustomSynth::biquad_filter_instance_t::mkhighpass(const uniform_t& freq, const uniform_t& Q)
{
	return [freq, Q](const Tone& tone)->filter_instance_t* {
		return new biquad_filter_instance_t(BiQuad::highpass(freq(tone), Q(tone)));
	};
}

CustomSynth::mkfilterinstance_t CustomSynth::biquad_filter_instance_t::mkbandpass(const uniform_t& freq, const uniform_t& Q)
{
	return [freq, Q](const Tone& tone)->filter_instance_t* {
		return new biquad_filter_instance_t(BiQuad::bandpass(freq(tone), Q(tone)));
	};
}

CustomSynth::mkfilterinstance_t CustomSynth::comb_filter_instance_t::mkcomb(const uniform_t& freq, const uniform_t& alpha, const uniform_t& beta)
{
	return [freq, alpha, beta](const Tone& tone)->filter_instance_t* {
		return new comb_filter_instance_t(freq(tone), alpha(tone), beta(tone));
	};
}

CustomSynth::comb_filter_instance_t::comb_filter_instance_t(float freq, float alpha, float beta):alpha(alpha), beta(beta)
{
	float fracdelay=2*M_PI/freq;
	delaylength=(int) ceilf(fracdelay);
	coeffs[0]=delaylength - fracdelay;
	coeffs[1]=1.0f - coeffs[0];
}

CustomSynth::varying_eval_t CustomSynth::comb_filter_instance_t::mkeval(int src)
{
	struct eval_t {
		const comb_filter_instance_t&	filter;
		int		src;
		float*	buffer;
		int		bufptr;
		
		eval_t(const comb_filter_instance_t& filter, int src):filter(filter), src(src)
		{
			buffer=new float[filter.delaylength];
			bufptr=0;
			
			for (int i=0;i<filter.delaylength;i++)
				buffer[i]=0;
		}
		
		eval_t(const eval_t& copy):eval_t(copy.filter, copy.src) {}
		
		~eval_t()
		{
			delete buffer;
		}
		
		void operator()(context_t& ctx, int dst)
		{
			float* srcbuf=ctx.buffer + src*ctx.blocklength;
			float* dstbuf=ctx.buffer + dst*ctx.blocklength;
			
			for (int i=0;i<ctx.blocklength;i++) {
				/*float v=*srcbuf++;
				
				float w=buffer[bufptr]*filter.coeffs[1];
				
				buffer[bufptr]=v;
				if (++bufptr==filter.delaylength) bufptr=0;
				
				w+=buffer[bufptr]*filter.coeffs[0];
				
				*dstbuf++=v+w;*/
				
				int tmpbufptr=bufptr++;
				if (bufptr==filter.delaylength) bufptr=0;
				
				float u=*srcbuf++;
				float v=buffer[bufptr]*filter.coeffs[0] + buffer[tmpbufptr]*filter.coeffs[1];
				
				u-=v*filter.beta;
				v*=filter.alpha;
				v+=u;
				
				buffer[tmpbufptr]=u;
				*dstbuf++=v;				
			}
		}
	};
	
	return eval_t(*this, src);
}

}
