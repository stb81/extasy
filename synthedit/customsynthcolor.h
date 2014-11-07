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
 
#ifndef INCLUDE_SYNTHEDIT_CUSTOMSYNTHCOLOR_H
#define INCLUDE_SYNTHEDIT_CUSTOMSYNTHCOLOR_H

namespace Synth {
	
class CustomSynthColorizer:public GUI::TextColorizer {
	CustomSynth*	instr;
	
public:
	CustomSynthColorizer(CustomSynth* instr):instr(instr) {}

	virtual std::pair<std::string::iterator, GUI::Color> next_token_color(std::string::iterator pos, std::string::iterator end) const
	{
		if (isalpha(*pos)) {
			char buf[64];
			int ptr=0;
			
			while (pos!=end && isalnum(*pos)) {
				if (ptr<63) buf[ptr++]=*pos;
				pos++;
			}
			
			buf[ptr]=0;
			if (instr->get_filter_by_name(buf)<0)
				return std::pair<std::string::iterator, GUI::Color>(pos, GUI::Color(85, 255, 85));
			else
				return std::pair<std::string::iterator, GUI::Color>(pos, GUI::Color(255, 85, 255));
		}
		
		if (isdigit(*pos)) {
			while (pos!=end && isdigit(*pos)) pos++;
			if (pos!=end && *pos=='.') {
				pos++;
				while (pos!=end && isdigit(*pos)) pos++;
			}
			return std::pair<std::string::iterator, GUI::Color>(pos, GUI::Color(85, 255, 255));
		}
		
		if (*pos==';') {
			while (pos!=end && *pos!='\n') pos++;
			return std::pair<std::string::iterator, GUI::Color>(pos, GUI::Color(170, 170, 170));
		}
		
		return std::pair<std::string::iterator, GUI::Color>(++pos, GUI::Color(255, 255, 255));
	}
};

}

#endif
