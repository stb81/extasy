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
 
#ifndef INCLUDE_CUSTOMSCANNER_H
#define INCLUDE_CUSTOMSCANNER_H

#include <sstream>
#include <map>

namespace Synth {
class CustomScanner;
}

#if !defined(yyFlexLexerOnce)
#include "FlexLexer.h"
#endif

#include "customsynth.h"
#include "customparser.hh"

namespace Synth {

class CustomScanner:public yyFlexLexer {
	typedef CustomParser::token token;
	
	CustomSynth&		synth;
	
	std::istringstream	inputstream;
	
	int								time_variable;
	std::map<std::string, int>		named_varying_expressions;
	
	int discriminate_identifier(CustomParser::semantic_type&);
	
	int yylex(CustomParser::semantic_type&);
	
public:
	CustomScanner(CustomSynth&, const std::string&);
	
	int scan_token(CustomParser::semantic_type&, CustomParser::location_type&);
	
	void assign_variable(const std::string&, int);
	int look_up_variable(const char*);
};

}

#endif
