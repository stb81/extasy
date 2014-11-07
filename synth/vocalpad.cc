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
#include <stdlib.h>
#include <math.h>
#include <fftw3.h>
#include "vocalpad.h"

namespace Synth {

class LPC_Filter {
	float*	coeffs;
	float*	buffer;
	int		length;
	int		ptr;
	
public:
	LPC_Filter(float* c, int n)
	{
		coeffs=c;
		buffer=new float[n];
		length=n;
		ptr=0;
		
		for (int i=0;i<n;i++)
			buffer[i]=0.0f;
	}
	
	~LPC_Filter()
	{
		delete[] buffer;
	}
	
	float operator()(float v)
	{
		for (int j=0, p=ptr; j<length; j++) {
			v+=buffer[p] * coeffs[j];
			if (++p==length) p=0;
		}
		
		buffer[ptr++]=v;
		if (ptr==length) ptr=0;
		return v;
	}
};

class VocalPad::Tone:public Synth::Tone {
public:
	Tone(VocalPad&, float, float);
	virtual ~Tone();
	
	virtual void synth(float**, int);

private:
	VocalPad&	vocalpad;
	
	float		t;
	float		tstep;
	float		energy;
	
	LPC_Filter	lpc;
};

VocalPad::Tone::Tone(VocalPad& vp, float omega, float volume):Synth::Tone(vp), vocalpad(vp), lpc(vp.lpc, 12)
{
	t=0.0f;
	tstep=vp.period * omega / (2*M_PI);
	energy=0.0f;
}

VocalPad::Tone::~Tone()
{
}

void VocalPad::Tone::synth(float** out, int count)
{
	float excitation=stopped ? 0.0f : 0.001f;
	
	for (int i=0;i<count;i++) {
		energy+=excitation;
		energy*=0.999f;
		
		int t0=(int) floorf(t);
		float lambda=t - t0;
		
		out[0][i]=out[1][i]=lpc(lerp(vocalpad.padsamples[t0&(vocalpad.padlength-1)], vocalpad.padsamples[(t0+1)&(vocalpad.padlength-1)], lambda)) * energy;
		
		t+=tstep;
		if (t>=vocalpad.padlength)
			t-=vocalpad.padlength;
	}
	
	if (stopped && energy<0.0001f)
		decayed=true;
}

static inline void rotate(float cs, float sn, float& x, float& y)
{
	float tmpx=cs*x + sn*y;
	float tmpy=cs*y - sn*x;
	x=tmpx;
	y=tmpy;
}

static inline void normalize(float& cs, float& sn)
{
	float norm=hypotf(cs, sn);
	if (norm>0) {
		cs/=norm;
		sn/=norm;
	}
}

void solve_qr(float* A, float* b, int n, int m, int r)
{
	for (int i=0;i<m;i++)
		for (int j=i+1;j<n;j++) {
			float cs=A[i*(m+1)];
			float sn=A[i+j*m];
			
			normalize(cs, sn);
			
			for (int k=i;k<m;k++)
				rotate(cs, sn, A[k+i*m], A[k+j*m]);
			
			for (int k=0;k<r;k++)
				rotate(cs, sn, b[k+i*r], b[k+j*r]);
		}
	
	for (int i=m-1;i>=0;i--) {
		for (int j=i+1;j<m;j++)
			b[i]-=b[j] * A[j+i*m];
				
		b[i]/=A[i*(m+1)];
	}
}

static inline float sqr(float x)
{
	return x*x;
}

VocalPad::VocalPad(Mixer& m):Instrument(m)
{
	FILE* file=fopen("../aah.wav", "r");
	fseek(file, 0, SEEK_END);
	int length=(ftell(file)-0x2C)/4;
	
	length&=~1;
	printf("length=%d\n", length);
	
	float* sampledata=new float[length];
	short* tmp=new short[length*2];
	
	fseek(file, 0x2C, SEEK_SET);
	fread(tmp, 4, length, file);	
	fclose(file);
	
	for (int i=0;i<length;i++)
		sampledata[i]=ldexpf(tmp[2*i]+tmp[2*i], -16);
	
	delete[] tmp;
	
	const int n=(length-16)/4;
	float* A=new float[n*12];
	float* b=new float[n];
	
	for (int i=0;i<n;i++) {
		for (int j=0;j<12;j++)
			A[12*i+j]=sampledata[i+j];
		
		b[i]=sampledata[i+12];
	}	
	
	solve_qr(A, b, n, 12, 1);
	
	for (int i=0;i<12;i++) {
		lpc[i]=b[i];
		printf("lpc[%d]=%f\n", i, lpc[i]);
	}
	
	delete[] A;
	delete[] b;
	
	for (int i=length-1;i>=12;i--) {
		float pred=0;
		for (int j=0;j<12;j++)
			pred+=lpc[j] * sampledata[i+j-12];
		
		sampledata[i]-=pred;
	}
	
	double* fftbuf=new double[length];
	double* dctbuf=new double[length/2];
	
	for (int i=0;i<length;i++)
		fftbuf[i]=sampledata[i] * (1.0-cos(2*M_PI*i/length));
	
	fftw_plan fwdfft=fftw_plan_r2r_1d(length, fftbuf, fftbuf, FFTW_R2HC, FFTW_ESTIMATE);
	fftw_execute(fwdfft);
	fftw_destroy_plan(fwdfft);
		
	for (int i=0;i<length/2;i++)
		dctbuf[i]=0;
	for (int i=1;i<length/2 && 48000.0*i/length<2000;i++)
		dctbuf[i]=sqr(fftbuf[i]) + sqr(fftbuf[length-i]);
	
	fftw_plan dct=fftw_plan_r2r_1d(length/2, dctbuf, dctbuf, FFTW_REDFT00, FFTW_ESTIMATE);
	fftw_execute(dct);
	fftw_destroy_plan(dct);
	
	/*file=fopen("spectrum.data", "w");
	for (int i=1;i<length/64;i++)
		fprintf(file, "%f %f\n", 48000.0*i/length, (15*dctbuf[i] + 6*dctbuf[2*i] + dctbuf[3*i] - 6*dctbuf[0]) / 16 / length);
	fclose(file);*/
	
	for (int i=1;i<length/8;i++)
		dctbuf[i]=(dctbuf[3*i] + 6*dctbuf[2*i] + 15*dctbuf[i] - 6*dctbuf[0]) / 16;

	int period=1;
	float bestrel=0;

	for (int i=1;i<length/8;i++)
		if (dctbuf[i]>dctbuf[i-1] && dctbuf[i]>dctbuf[i+1] && dctbuf[i]>0) {
			float rel=dctbuf[i]/dctbuf[0];
			printf("peak at %d, relevance %f%%\n", i, 100.0*rel);
			
			if (rel>bestrel) {
				bestrel=rel;
				period=i;
			}
		}
	
	delete[] fftbuf;
	delete[] dctbuf;
	
	const int chorusdepth=16384;
	
	int nperiods=(length-chorusdepth)/period - 4;
	int shiftrange=length - period*nperiods - chorusdepth - 1;

	int newperiod=period-1;
	newperiod|=newperiod>>1;
	newperiod|=newperiod>>2;
	newperiod|=newperiod>>4;
	newperiod|=newperiod>>8;
	newperiod|=newperiod>>16;
	newperiod++;
	printf("old_period=%d  new_period=%d\n", period, newperiod);
	
	this->period=newperiod;
	
	padlength=262144;
	padsamples=new float[padlength];
	for (int i=0;i<padlength;i++)
		padsamples[i]=0;
		
	int framelength=nperiods*newperiod;
	printf("frame length=%d\n", framelength);
		
	for (int i=0;i<padlength;i+=padlength>>6) {
		int shift=rand() % shiftrange;
		int ptr=i;
		
		Noise noise;
		noise.init(0.01f / newperiod);
		
		for (int j=0;j<framelength;j++) {
			float t=(float) j*period/newperiod;
			t+=noise() * chorusdepth;
			
			int t0=(int) floorf(t);
			t-=t0;
			t0+=shift;
			
			float w=(1.0f - cosf(2*M_PI*j/framelength)) * 0.5f;
			w*=w;
			
			padsamples[ptr]+=lerp(sampledata[t0], sampledata[t0+1], t) * w;
			
			ptr=(ptr+1) & (padlength-1);
		}
	}
	
	delete[] sampledata;
}

VocalPad::~VocalPad()
{
}

Synth::Tone* VocalPad::play_note(int note, int volume)
{
	return new Tone(*this, mixer.note2omega(note), ldexpf(volume, -7));
}

}
