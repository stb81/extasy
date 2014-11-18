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
 
#ifndef INCLUDE_SYNTH_FILTER_H
#define INCLUDE_SYNTH_FILTER_H

namespace Synth {
	
class IFilterInstance {
public:
	virtual ~IFilterInstance();
	virtual void apply(float**, int) = 0;
	virtual void handle_effect(unsigned short);
};


class VolumeScalerInstance:public IFilterInstance {
	int	volume;
	int	panning;
	
public:
	VolumeScalerInstance();
	
	virtual void apply(float**, int);
	virtual void handle_effect(unsigned short);
};
	

class BiQuadFilterInstance:public IFilterInstance {
	BiQuad::Filter	filterl, filterr;
	
public:
	BiQuadFilterInstance(const BiQuad&);

	virtual void apply(float**, int);
};


class CustomBiQuadFilterInstance:public BiQuadFilterInstance {
	BiQuad			filter;
	
public:
	CustomBiQuadFilterInstance(const BiQuad&);
};


class ChorusInstance:public IFilterInstance {
	VarDelay	delayl;
	VarDelay	delayr;
	LFNoise*	noise;
	
	float		depth;
	int			numvoices;
	
public:
	ChorusInstance(float depth, int voices);
	virtual ~ChorusInstance();

	virtual void apply(float**, int);
};
	
}

#endif
