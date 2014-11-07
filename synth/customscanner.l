%{
#include "customscanner.h"
#include "sequencer.h"

#undef YY_DECL
#define YY_DECL int CustomScanner::yylex(CustomParser::semantic_type& lval)

#define yyterminate() return token::TOK_EOF

using namespace Synth;
%}

%option c++
%option yyclass="CustomScanner"
%option noyywrap

NUMBER			"-"?[0-9]+("."[0-9]+)?

%%

"LowPass"		return token::TOK_LOWPASS;
"HighPass"		return token::TOK_HIGHPASS;
"BandPass"		return token::TOK_BANDPASS;
"CombFilter"	return token::TOK_COMBFILTER;

"dc"			return token::TOK_DC;
"sin"			return token::TOK_SIN;
"cos"			return token::TOK_COS;
"exp"			return token::TOK_EXP;

"time"			if (time_variable<0) time_variable=synth.add_varying([](CustomSynth::Tone& tone)->CustomSynth::varying_eval_t {
					int time=0;
					return [=](CustomSynth::context_t& ctx, int dstslot) mutable->void {
						float* ptr=ctx[dstslot];
						for (int i=0;i<ctx.blocklength;i++)
							*ptr++=time++;
					};
				}); lval.varexpr=time_variable; return token::TOK_VARYING_VARIABLE;

"omega"			lval.uniexpr=new CustomSynth::uniform_t(&CustomSynth::get_uniform_constant); return token::TOK_UNIFORM_CONSTANT;

"noise"			lval.varexpr=synth.add_varying([](CustomSynth::Tone& tone)->CustomSynth::varying_eval_t {
					return [](CustomSynth::context_t& ctx, int dstslot)->void {
						float* ptr=ctx[dstslot];
						for (int i=0;i<ctx.blocklength;i++)
							*ptr++=ldexpf((rand()&255)-(rand()&255), -8);
					};
				}); return token::TOK_VARYING_VARIABLE;

[A-Za-z][A-Za-z0-9]*	return discriminate_identifier(lval);

{NUMBER}		lval.fval=atof(yytext); return token::TOK_NUMBER;
{NUMBER}"Hz"	yytext[strlen(yytext)-2]=0; lval.fval=synth.get_mixer().freq2omega(atof(yytext)); return token::TOK_NUMBER;
{NUMBER}"ms"	yytext[strlen(yytext)-2]=0; lval.fval=atof(yytext); return token::TOK_NUMBER;

"\n"			return token::TOK_EOL;
";"[^\n]*"\n"	return token::TOK_EOL;

[ \t]+			;
.				return *yytext;

%%

CustomScanner::CustomScanner(CustomSynth& synth, const std::string& str):synth(synth), inputstream(str)
{
	switch_streams(&inputstream);
	
	time_variable=-1;
}

int CustomScanner::scan_token(CustomParser::semantic_type& lval, CustomParser::location_type& lloc)
{
	lloc.begin=inputstream.tellg();
	int tok=yylex(lval);
	lloc.end=inputstream.tellg();
	
	return tok;
}

int CustomScanner::discriminate_identifier(CustomParser::semantic_type& lval)
{
	{
		auto i=named_varying_expressions.find(yytext);
		if (i!=named_varying_expressions.end()) {
			lval.varexpr=i->second;
			return token::TOK_VARYING_VARIABLE;
		}
	}

	int filter=synth.get_filter_by_name(yytext);
	if (filter>=0) {
		lval.filter=filter;
		return token::TOK_FILTER;
	}

	lval.str=new std::string(yytext);
	return token::TOK_IDENTIFIER;
}

int CustomScanner::look_up_variable(const char* name)
{
	auto i=named_varying_expressions.find(name);
	return i!=named_varying_expressions.end() ? i->second : -1;
}

void CustomScanner::assign_variable(const std::string& name, int expr)
{
	// FIXME: use emplace once gcc supports it
	named_varying_expressions.insert(std::pair<std::string, int>(name, expr));
}
