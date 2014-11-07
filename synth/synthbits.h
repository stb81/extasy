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
 
#ifndef INCLUDE_SYNTH_SYNTHBITS_H
#define INCLUDE_SYNTH_SYNTHBITS_H

#include <math.h>
#include <vector>
#include <complex>
#include <stdlib.h>

namespace Synth {

template<typename TVector>
TVector lerp(TVector v1, TVector v2, float lambda)
{
	return v1*(1.0f-lambda) + v2*lambda;
}


class Delay {
public:
	Delay(int l):length(l),ptr(0)
	{
		buffer=new float[length];
		
		for (int i=0;i<length;i++)
			buffer[i]=0;
	}
	
	~Delay()
	{
		delete[] buffer;
	}
	
	float operator()() const
	{
		return buffer[ptr];
	}
	
	float operator()(float next)
	{
		float tmp=buffer[ptr];
		buffer[ptr++]=next;
		if (ptr==length) ptr=0;
		return tmp;
	}
	
private:
	int length;
	int ptr;
	
	float*	buffer;
};		

class LeakyIntegral {
private:
	float	leakage;
	float	val;

public:
	LeakyIntegral(float l):leakage(1-l),val(0) {}
	
	float operator()() const
	{ return val; }
	
	float operator()(float in)
	{
		val*=leakage;
		val+=in;
		return val;
	}
};


class BiQuad {
public:
	class Filter {
	private:
		const BiQuad*	biquad;
		float	x0, x1, y0, y1;
		
	public:
		Filter() {}
		
		explicit Filter(const BiQuad& b):biquad(&b)
		{
			x0=x1=y0=y1=0;
		}
		
		float operator()(float x2)
		{
			float y2=biquad->b0*x2 + biquad->b1*x1 + biquad->b2*x0 - biquad->a1*y1 - biquad->a2*y0;
			
			x0=x1; x1=x2;
			y0=y1; y1=y2;
			
			return y2;
		}
	};
	
	BiQuad() {}
	
	std::complex<float> response(std::complex<float> z) const
	{
		return log(((b0*z+b1)*z+b2) / ((z+a1)*z+a2));
	}
	
	std::complex<float> dresponse(std::complex<float> z) const
	{
		return (2.0f*b0*z+b1)/((b0*z+b1)*z+b2) - (2.0f*z+a1)/((z+a1)*z+a2);
	}
	
	BiQuad(float _b0, float _b1, float _b2, float _a0, float _a1, float _a2)
	{
		a1=_a1 / _a0;
		a2=_a2 / _a0;
		b0=_b0 / _a0;
		b1=_b1 / _a0;
		b2=_b2 / _a0;
	}
	
	BiQuad operator*(float gain) const
	{
		return BiQuad(b0*gain, b1*gain, b2*gain, 1.0f, a1, a2);
	}
	
	static BiQuad lowpass(float omega, float Q)
	{
		float alpha=sinf(omega) / (2*Q);
		float beta=cosf(omega);
		
		return BiQuad((1-beta)/2, 1-beta, (1-beta)/2, 1+alpha, -2*beta, 1-alpha);
	}
	
	static BiQuad highpass(float omega, float Q)
	{
		float alpha=sinf(omega) / (2*Q);
		float beta=cosf(omega);
		
		return BiQuad((1+beta)/2, -1-beta, (1+beta)/2, 1+alpha, -2*beta, 1-alpha);
	}
	
	static BiQuad bandpass(float omega, float Q)
	{
		float alpha=sinf(omega) / (2*Q);
		
		return BiQuad(alpha*Q, 0, -alpha*Q, 1+alpha, -2*cosf(omega), 1-alpha);
	}
	
	static BiQuad allpass(float omega, float Q)
	{
		float alpha=sinf(omega) / (2*Q);
		
		return BiQuad(1-alpha, -2*cosf(omega), 1+alpha, 1+alpha, -2*cosf(omega), 1-alpha);
	}
	
	static BiQuad bandstop(float omega, float Q)
	{
		float alpha=sinf(omega) / (2*Q);
		
		return BiQuad(1, -2*cosf(omega), 1, 1+alpha, -2*cosf(omega), 1-alpha);
	}
	
	static BiQuad peakingEQ(float omega, float Q, float gain)
	{
		float alpha=sinf(omega) / (2*Q);
		
		return BiQuad(1+alpha*gain, -2*cosf(omega), 1-alpha*gain, 1+alpha/gain, -2*cosf(omega), 1-alpha/gain);
	}
	
	static BiQuad lowshelf(float omega, float Q, float gain)
	{
		float alpha=sinf(omega) / (2*Q);
		
		return BiQuad(gain*((gain+1)-(gain-1)*cosf(omega)+2*sqrtf(gain)*alpha), 2*gain*((gain-1)-(gain+1)*cosf(omega)), gain*((gain+1)-(gain-1)*cosf(omega)-2*sqrtf(gain)*alpha),
						(gain+1)+(gain-1)*cosf(omega)+2*sqrtf(gain)*alpha, -2*((gain-1)+(gain+1)*cosf(omega)), (gain+1)+(gain-1)*cosf(omega)-2*sqrtf(gain)*alpha);
	}
	
	static BiQuad highshelf(float omega, float Q, float gain)
	{
		float alpha=sinf(omega) / (2*Q);
		
		return BiQuad(gain*((gain+1)+(gain-1)*cosf(omega)+2*sqrtf(gain)*alpha), -2*gain*((gain-1)+(gain+1)*cosf(omega)), gain*((gain+1)+(gain-1)*cosf(omega)-2*sqrtf(gain)*alpha),
						(gain+1)-(gain-1)*cosf(omega)+2*sqrtf(gain)*alpha, 2*((gain-1)-(gain+1)*cosf(omega)), (gain+1)-(gain-1)*cosf(omega)-2*sqrtf(gain)*alpha);
	}
	
private:
	float	a1, a2, b0, b1, b2;
};


class VarDelay {
public:
	VarDelay(int length):length(length)
	{
		ptr=0;
		data=new float[length];
		
		for (int i=0;i<length;i++)
			data[i]=0.0f;
	}
	
	~VarDelay()
	{
		delete[] data;
	}
	
	void push(float v)
	{
		data[ptr++]=v;
		if (ptr==length) ptr=0;
	}
	
	float operator()(float t) const
	{
		t+=ptr;
		
		int t0=(int) floorf(t);
		t-=t0;
		
		/*float v[]={ 1.0f, t, t*t, t*t*t };
		float result=0.0f;

		const static float CR[]={
			0.0f, -1.0f, 2.0f, -1.0f,
			2.0f, 0.0f, -5.0f, 3.0f,
			0.0f, 1.0f, 4.0f, -3.0f,
			0.0f, 0.0f, -1.0f, 1.0f
		};
		
		for (int i=0;i<4;i++)
			for (int j=0;j<4;j++)
				result+=CR[4*i+j] * v[j] * data[(t0+i)%length];
		
		return result * 0.5f;*/
		
		return lerp(data[t0%length], data[(t0+1)%length], t);
	}
	
private:
	int		length;
	int		ptr;
	float*	data;
};


class Noise {
	float	coeffs[4];
	
	float	pos;
	float	freq;
	
public:
	void init(float f)
	{
		coeffs[0]=ldexpf(rand()&0xffff, -16);
		coeffs[1]=ldexpf(rand()&0xffff, -16);
		coeffs[2]=ldexpf(rand()&0xffff, -16);
		coeffs[3]=ldexpf(rand()&0xffff, -16);
		
		pos=0;
		freq=f;
	}
	
	float operator()()
	{
		float a=lerp(coeffs[0], coeffs[1], (pos+2)/3);
		float b=lerp(coeffs[1], coeffs[2], (pos+1)/3);
		float c=lerp(coeffs[2], coeffs[3], pos/3);
		
		float result=lerp(
			lerp(a, b, (pos+1)/2),
			lerp(b, c, pos/2),
			pos);
		
		pos+=freq;
		if (pos>=1) {
			pos-=1.0f;
			coeffs[0]=coeffs[1];
			coeffs[1]=coeffs[2];
			coeffs[2]=coeffs[3];
			coeffs[3]=ldexpf(rand()&0xffff, -16);
		}
		
		return result;
	}
};


}

#endif
