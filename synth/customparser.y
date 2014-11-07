%skeleton "lalr1.cc" /* -*- C++ -*- */

%defines
%define api.token.prefix {TOK_}
%define api.namespace {Synth}
%define api.location.type {CustomSynth::location_t}
%define parser_class_name {CustomParser}
%define parse.error verbose
%param { CustomSynth& synth }
%param { CustomScanner& scanner }

%locations

%{
#include <iostream>
#include "customscanner.h"

#define yylex(lval, lloc, synth, scanner) scanner.scan_token(*lval, *lloc)

#define UNARY_OP(dstvar, op, fn) { int _op=op; dstvar=synth.add_varying([_op](CustomSynth::Tone& tone)->CustomSynth::varying_eval_t { \
			return [_op](CustomSynth::context_t& ctx, int dstslot)->void {	\
				float* dst=ctx[dstslot];	\
				float* src=ctx[_op];	\
				for (int i=0;i<ctx.blocklength;i++)	\
					*dst++=fn(*src++);	\
			};	\
		}); }

#define BINARY_OP(dstvar, op1, op2, fn) { int _op1=op1, _op2=op2; dstvar=synth.add_varying([_op1, _op2](CustomSynth::Tone& tone)->CustomSynth::varying_eval_t { \
			return [_op1, _op2](CustomSynth::context_t& ctx, int dstslot)->void {	\
				float* dst=ctx[dstslot];	\
				float* src1=ctx[_op1];	\
				float* src2=ctx[_op2];	\
				for (int i=0;i<ctx.blocklength;i++)	\
					*dst++=fn(*src1++, *src2++);	\
			};	\
		}); }
%}

%union {
	float			fval;
	std::string*	str;
	int				varexpr;
	int				filter;
	CustomSynth::uniform_t*	uniexpr;
}

%token EOF 0
%token EOL "end of line"
%token LOWPASS HIGHPASS BANDPASS COMBFILTER
%token DC SIN COS EXP
%token <fval> NUMBER
%token <str> IDENTIFIER
%token <varexpr> VARYING_VARIABLE
%token <uniexpr> UNIFORM_CONSTANT
%token <filter> FILTER
%type <varexpr> VaryingExpr
%type <uniexpr> UniformExpr

%left '+' '-'
%left '*'


%%

statement:
	| statement EOL
	| statement LOWPASS IDENTIFIER '(' UniformExpr ',' UniformExpr ')' EOL { synth.add_filter(*$3, CustomSynth::biquad_filter_instance_t::mklowpass(*$5, *$7)); delete $3; delete $5; delete $7; }
	| statement HIGHPASS IDENTIFIER '(' UniformExpr ',' UniformExpr ')' EOL { synth.add_filter(*$3, CustomSynth::biquad_filter_instance_t::mkhighpass(*$5, *$7)); delete $3; delete $5; delete $7; }
	| statement BANDPASS IDENTIFIER '(' UniformExpr ',' UniformExpr ')' EOL { synth.add_filter(*$3, CustomSynth::biquad_filter_instance_t::mkbandpass(*$5, *$7)); delete $3; delete $5; delete $7; }
	| statement COMBFILTER IDENTIFIER '(' UniformExpr ',' UniformExpr ',' UniformExpr ')' EOL { synth.add_filter(*$3, CustomSynth::comb_filter_instance_t::mkcomb(*$5, *$7, *$9)); delete $3; delete $5; delete $7; delete $9; }
	| statement IDENTIFIER '=' VaryingExpr EOL { scanner.assign_variable(*$2, $4); delete $2; }
	;

VaryingExpr:
	  VARYING_VARIABLE { $$=$1; }
	| VaryingExpr '+' VaryingExpr { BINARY_OP($$, $1, $3, [](float a, float b) { return a+b; }) }
	| VaryingExpr '-' VaryingExpr { BINARY_OP($$, $1, $3, [](float a, float b) { return a-b; }) }
	| VaryingExpr '*' VaryingExpr { BINARY_OP($$, $1, $3, [](float a, float b) { return a*b; }) }
	| SIN '(' VaryingExpr ')' { UNARY_OP($$, $3, sinf) }
	| COS '(' VaryingExpr ')' { UNARY_OP($$, $3, cosf) }
	| EXP '(' VaryingExpr ')' { UNARY_OP($$, $3, expf) }
	| DC '(' UniformExpr ')' { CustomSynth::uniform_t& dc=*$3; $$=synth.add_varying([dc](CustomSynth::Tone& tone)->CustomSynth::varying_eval_t {
		float _dc=dc(tone);
		return [_dc](CustomSynth::context_t& ctx, int dstslot)->void {
			float* dst=ctx[dstslot];
			for (int i=0;i<ctx.blocklength;i++)
				*dst++=_dc;
		};
	  } ); delete $3; }
	| FILTER '(' VaryingExpr ')' { $$=synth.add_varying_for_filter($1, $3); }
	| '(' VaryingExpr ')'	{ $$=$2; }
	;

UniformExpr:
	  NUMBER { float c=$1; $$=new CustomSynth::uniform_t([c](const CustomSynth::Tone&)->float { return c; }); }
	| UNIFORM_CONSTANT { $$=$1; }
	| UniformExpr '*' UniformExpr {
		const CustomSynth::uniform_t& op1=*$1;
		const CustomSynth::uniform_t& op2=*$3;
		$$=new CustomSynth::uniform_t([op1, op2](const CustomSynth::Tone& tone)->float { return op1(tone)*op2(tone); });
	  }
	| '(' UniformExpr ')' { $$=$2; }
	;

%%

void Synth::CustomParser::error(const Synth::CustomSynth::location_t& loc, const std::string& msg)
{
	synth.report_compile_error(msg, loc);
	//std::cout << "Line " << loc.begin.line << ": " << msg << std::endl;
}

