%{
#include "formc.h"
#include "parser.hh"
%}

%x COMMENT1
%x COMMENT2
%x CPPCODE
%x CPPEXPR
%x ALIGNMENT_KEYWORD
%s COLUMN_MODEL_ITEM
%s CURVE_MODEL_ITEM

%option stack

%%

form				return FORM;
extends				return EXTENDS;
implements			return IMPLEMENTS;
using				return USING;
namespace			return NAMESPACE;
on					return ON;
set					return SET;
const				return CONST;
var					return VAR;
factory				return FACTORY;
method				return METHOD;
init				return INIT;

"column_model"		return COLUMN_MODEL;
<COLUMN_MODEL_ITEM>"draw"	return COLUMN_MODEL_DRAW;
<COLUMN_MODEL_ITEM>"edit"	return COLUMN_MODEL_EDIT;

"curve_model"		return CURVE_MODEL;
<CURVE_MODEL_ITEM>"eval"	return CURVE_MODEL_EVAL;
<CURVE_MODEL_ITEM>"color:"	return CURVE_MODEL_COLOR;

hbox				return HBOX;
vbox				return VBOX;
free				return FREE;
hstrut				return HSTRUT;
vstrut				return VSTRUT;
element				return ELEMENT;

"name:"				return NAME;
"size:"				return SIZE;
"pos:"				return POSITION;
"args:"				return ARGS;
"halign:"			return HALIGN;
"valign:"			return VALIGN;
"bind:"				return BIND;

"->"				return MAPSTO;
"=>"				return CONNECTTO;

^"#include".*		yylval.str=strdup(yytext); return INCLUDE;

"::"				return DOUBLE_COLON;

"/*"				yy_push_state(COMMENT1);
<COMMENT1>"*/"		yy_pop_state();
<COMMENT1>.			{}

"//"				yy_push_state(COMMENT2);
<COMMENT2>.*		yy_pop_state();

<CPPCODE>[^\{\}]*	yylval.str=strdup(yytext); return CODE;
<CPPCODE>[\{\}]		return *yytext;

<CPPEXPR>[^;,() \r\t\n][^;,()]*		yylval.str=strdup(yytext); return CODE;
<CPPEXPR>[;,()]		return *yytext;

<ALIGNMENT_KEYWORD>"expand"	yy_pop_state(); yylval.align=ALIGN_EXPAND; return ALIGNMENT;
<ALIGNMENT_KEYWORD>"left"	yy_pop_state(); yylval.align=ALIGN_LEFT; return ALIGNMENT;
<ALIGNMENT_KEYWORD>"right"	yy_pop_state(); yylval.align=ALIGN_RIGHT; return ALIGNMENT;
<ALIGNMENT_KEYWORD>"top"	yy_pop_state(); yylval.align=ALIGN_TOP; return ALIGNMENT;
<ALIGNMENT_KEYWORD>"bottom"	yy_pop_state(); yylval.align=ALIGN_BOTTOM; return ALIGNMENT;
<ALIGNMENT_KEYWORD>"center"	yy_pop_state(); yylval.align=ALIGN_CENTER; return ALIGNMENT;

[_A-Za-z][_A-Za-z0-9]*	yylval.str=strdup(yytext); return IDENTIFIER;
["][^"]*["]			yylval.str=strdup(yytext); return STRINGLITERAL;
[\-]?[0-9]+			yylval.inum=atoi(yytext); return INTEGER;

<*>[ \r\t\n]+		{}
.				return *yytext;

%%

void scanner_expect_alignment()
{
	yy_push_state(ALIGNMENT_KEYWORD);
}

void scanner_begin_column_model_items()
{
	yy_push_state(COLUMN_MODEL_ITEM);
}

void scanner_end_column_model_items()
{
	yy_pop_state();
}

void scanner_begin_curve_model_items()
{
	yy_push_state(CURVE_MODEL_ITEM);
}

void scanner_end_curve_model_items()
{
	yy_pop_state();
}

void scanner_begin_cpp_code()
{
	yy_push_state(CPPCODE);
}

void scanner_end_cpp_code()
{
	yy_pop_state();
}

void scanner_begin_cpp_expr()
{
	yy_push_state(CPPEXPR);
}

void scanner_end_cpp_expr()
{
	yy_pop_state();
}

