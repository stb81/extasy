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
 
#include "synthbits.h"
#include "filter.h"
#include "module.h"

namespace Synth {
	
IFilterInstance::~IFilterInstance()
{
}
	
void IFilterInstance::handle_effect(unsigned short effect)
{
}

	
VolumeScalerInstance::VolumeScalerInstance()
{
	volume=255;
	panning=128;
}

void VolumeScalerInstance::apply(float** samples, int count)
{
	float vol[2];

	vol[0]=ldexpf(volume*(255-panning), -16);
	vol[1]=ldexpf(volume*panning, -16);
	
	for (int i=0;i<count;i++) {
		samples[0][i]*=vol[0];
		samples[1][i]*=vol[1];
	}
}

void VolumeScalerInstance::handle_effect(unsigned short effect)
{
	if ((effect>>8)==Pattern::PANNING)
		panning=effect&0xff;
}


BiQuadFilterInstance::BiQuadFilterInstance(const BiQuad& biquad):filterl(biquad), filterr(biquad)
{
}

void BiQuadFilterInstance::apply(float** samples, int count)
{
	for (int i=0;i<count;i++) {
		samples[0][i]=filterl(samples[0][i]);
		samples[1][i]=filterr(samples[1][i]);
	}
}


CustomBiQuadFilterInstance::CustomBiQuadFilterInstance(const BiQuad& biquad):filter(biquad), BiQuadFilterInstance(filter)
{
}


ChorusInstance::ChorusInstance(float depth, int numvoices):delayl((int) ceilf(depth)), delayr((int) ceilf(depth)), depth(depth), numvoices(numvoices)
{
	noise=new LFNoise[numvoices*2];
	
	for (int i=0;i<numvoices*2;i++)
		noise[i].init(0.0001f + i*0.000015f);
}

ChorusInstance::~ChorusInstance()
{
	delete[] noise;
}

void ChorusInstance::apply(float** samples, int count)
{
	for (int i=0;i<2;i++) {
		VarDelay& delay=i ? delayr : delayl;
		
		for (int j=0;j<count;j++) {
			delay.push(samples[i][j]);
			
			float v=0;
			for (int k=0;k<numvoices;k++)
				v+=delay(noise[2*k+i]()*depth);
			
			samples[i][j]=v / numvoices;
		}
	}
}

}
