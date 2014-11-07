%{
#include <stack>
#include <stdio.h>
#include <string.h>
#include "formc.h"

int yylex();
void yyerror(const char* err);

void scanner_expect_alignment();
void scanner_begin_cpp_code();
void scanner_end_cpp_code();
void scanner_begin_cpp_expr();
void scanner_end_cpp_expr();
void scanner_begin_column_model_items();
void scanner_end_column_model_items();
void scanner_begin_curve_model_items();
void scanner_end_curve_model_items();

static Form* cur_form;
static std::stack<LayoutItem*> layout_stack;

%}

%union {
	int			inum;
	char*		str;
	Form*		form;
	LayoutItem*	layout_item;
	arglist_t*	arglist;
	argument_t*	argitem;
	std::string*	longstr;
	alignment_t	align;
}

%token FORM EXTENDS IMPLEMENTS USING NAMESPACE ON SET CONST VAR FACTORY METHOD INIT
%token HBOX VBOX FREE HSTRUT VSTRUT ELEMENT
%token COLUMN_MODEL COLUMN_MODEL_DRAW COLUMN_MODEL_EDIT
%token CURVE_MODEL CURVE_MODEL_EVAL CURVE_MODEL_COLOR
%token NAME SIZE POSITION ARGS HALIGN VALIGN BIND
%token MAPSTO CONNECTTO
%token <str> STRINGLITERAL
%token <str> IDENTIFIER
%token <str> CODE
%token <inum> INTEGER
%token <str> INCLUDE
%token <align> ALIGNMENT

%type <form> formdef
%type <layout_item> LayoutItem
%type <arglist> Args
%type <argitem> ArgItem
%type <longstr> TypeSpec TypeSpecList TypeName LongCode LongCodeItem LongExpr LongExprList LongExprItem ReturnType CurveModelItems

%left DOUBLE_COLON

%%

input:
	| input formdef '{' { cur_form=$2; layout_stack.push(cur_form); } ItemDefs '}' { $2->print(); delete $2; cur_form=NULL; layout_stack.pop(); }
	| input NAMESPACE IDENTIFIER '{'
	  {
		fprintf(out_header, "namespace %s {\n\n", $3);
		fprintf(out_source, "namespace %s {\n\n", $3);
	  }
	  input '}'
	  {
		fputs("}\n\n", out_header);
		fputs("}\n\n", out_source);
	  }
	| input INCLUDE { fputs($2, out_header); putc('\n', out_header); free($2); }
	| input USING NAMESPACE IDENTIFIER ';' { fprintf(out_header, "using namespace %s;\n", $4); free($4); }
	| input USING TypeName ';' { fprintf(out_header, "using %s;\n", $3->c_str()); delete $3; }
	;
	
formdef:
	  FORM TypeName { $$=new Form; $$->element_type=strdup($2->c_str()); delete $2; }
	| formdef EXTENDS TypeName '(' Args ')' { $$=$1; $$->extensions.push_back(std::pair<char*, arglist_t*>(strdup($3->c_str()), $5)); delete $3; }
	| formdef IMPLEMENTS TypeName { $$=$1; $$->extensions.push_back(std::pair<char*, arglist_t*>(strdup($3->c_str()), NULL)); delete $3; }
	;

TypeSpec:
	  TypeName { $$=$1; }
	| TypeSpec '*' { *$$+="*"; }
	| TypeSpec '&' { *$$+="&"; }
	| CONST TypeSpec { $$=$2; $$->insert(0, "const "); }
	;

TypeSpecList:
	  TypeSpec { $$=$1; }
	| TypeSpecList ',' TypeSpec { $$=$1; *$$+=", "; *$$+=*$3; delete $3; }
	;
	
TypeName:
	  IDENTIFIER { $$=new std::string($1); }
	| TypeName DOUBLE_COLON TypeName { $$=$1; *$$+="::"; *$$+=*$3; delete $3; }
	| TypeName '<' TypeSpecList '>' { $$=$1; *$$+='<'; *$$+=*$3; *$$+='>'; delete $3; }
	;
	
ReturnType:
	  { $$=new std::string("void"); }
	| MAPSTO TypeSpec { $$=$2; }
	;
	
Args:
	  { $$=new arglist_t; }
	| ArgItem { $$=new arglist_t; $$->push_back($1); }
	| Args ',' ArgItem { $$=$1; $$->push_back($3); }
	;
	
ArgItem: TypeSpec IDENTIFIER {
			$$=new argument_t;
			$$->type=strdup($1->c_str());
			$$->name=$2;
			$$->value=NULL;
			delete $1;
		  }
		| TypeSpec IDENTIFIER '=' LongExpr {
			$$=new argument_t;
			$$->type=strdup($1->c_str());
			$$->name=$2;
			$$->value=strdup($4->c_str());
			delete $1;
			delete $4;
		  }
		;

ItemDefs:
	| ItemDefs LayoutItem {}
	| ItemDefs NAME IDENTIFIER ';' { layout_stack.top()->varname=strdup($3); }
	| ItemDefs SIZE INTEGER ',' INTEGER ';' { LayoutItem* li=layout_stack.top(); li->width=$3; li->height=$5; }
	| ItemDefs POSITION INTEGER ',' INTEGER ';' { LayoutItem* li=layout_stack.top(); li->x0=$3; li->y0=$5; }
	| ItemDefs HALIGN { scanner_expect_alignment(); } ALIGNMENT ';' { layout_stack.top()->halign=$4; }
	| ItemDefs VALIGN { scanner_expect_alignment(); } ALIGNMENT ';' { layout_stack.top()->valign=$4; }
	| ItemDefs ARGS LongExpr ';' { layout_stack.top()->args=strdup($3->c_str()); delete $3; }
	| ItemDefs BIND LongExpr ';'
	  {
		if (LayoutElement* el=dynamic_cast<LayoutElement*>(layout_stack.top())) {
			el->init_code+="\t@.bind_value(";
			el->init_code+=*$3;
			el->init_code+=");\n";
		}

		delete $3;
	  }
	| ItemDefs ON IDENTIFIER '(' Args ')' LongCode
	  {
		SignalHandler* sh=new SignalHandler;
		sh->item=layout_stack.top();
		sh->signal_name=$3;
		sh->handler_method=0;
		sh->body=strdup($7->c_str());
		sh->args=$5;
			
		cur_form->signal_handlers.push_back(sh);
		
		delete $7;
	  }
	| ItemDefs ON IDENTIFIER CONNECTTO IDENTIFIER '(' Args ')' ';'
	  {
		SignalHandler* sh=new SignalHandler;
		sh->item=layout_stack.top();
		sh->signal_name=$3;
		sh->handler_method=$5;
		sh->body=NULL;
		sh->args=$7;
			
		cur_form->signal_handlers.push_back(sh);
	  }
	| ItemDefs SET IDENTIFIER '=' LongExprList ';' { scanner_end_cpp_expr(); }
	  {
		if (LayoutElement* el=dynamic_cast<LayoutElement*>(layout_stack.top())) {
			el->init_code+="\t@.set_";
			el->init_code+=$3;
			el->init_code+="(";
			el->init_code+=*$5;
			el->init_code+=");\n";
		}
		
		free($3);
		delete $5;
	  }
	| ItemDefs FACTORY IDENTIFIER '(' Args ')' LongCode
	  {
		Form::FormMethod* fm=new Form::FormMethod;
		
		fm->is_static=true;
		fm->name=$3;
		fm->args=$5;
		fm->body=strdup($7->c_str());
		
		fm->rettype=(char*) malloc(strlen(cur_form->element_type)+2);
		strcpy(fm->rettype, cur_form->element_type);
		strcat(fm->rettype, "*");
		
		cur_form->form_methods.push_back(fm);
		
		delete $7;
	  }
	| ItemDefs METHOD IDENTIFIER '(' Args ')' ReturnType LongCode
	  {
		Form::FormMethod* fm=new Form::FormMethod;
		
		fm->is_static=false;
		fm->name=$3;
		fm->args=$5;
		fm->body=strdup($8->c_str());
		fm->rettype=strdup($7->c_str());
		
		cur_form->form_methods.push_back(fm);
		
		delete $7;
		delete $8;
	  }
	| ItemDefs INIT LongCode
	  {
		if (LayoutElement* el=dynamic_cast<LayoutElement*>(layout_stack.top()))
			el->init_code+=*$3;
		
		delete $3;
	  }
//	| ItemDefs VAR TypeSpec IDENTIFIER '=' IDENTIFIER ';' {}
	| ItemDefs COLUMN_MODEL '(' Args ')' '[' INTEGER ']' '{'
	  {
		scanner_begin_column_model_items();
		
		char modelname[12];
		strcpy(modelname, "Column");
		strcat(modelname, cur_form->namecounter.get_upper());
		
		if (LayoutElement* el=dynamic_cast<LayoutElement*>(layout_stack.top())) {
			el->init_code+="\t@.add_column(new ";
			el->init_code+=modelname;
			el->init_code+="(";
			for (int i=0;i<$4->size();i++) {
				if (!(*$4)[i]->value) { yyerror("column model argument must have initializer"); continue; }
				if (i) el->init_code+=", ";
				el->init_code+=(*$4)[i]->value;
			}
			el->init_code+="));\n";
		}
	  
	    fprintf(out_source, "\nclass %s::%s:public GUI::ListBox::Column {\n", cur_form->element_type, modelname);
		for (int i=0;i<$4->size();i++)
			fprintf(out_source, "\t%s %s;\n", (*$4)[i]->type, (*$4)[i]->name);
	    fprintf(out_source, "public:\n\t%s(", modelname);
		for (int i=0;i<$4->size();i++) {
			if (i) fputs(", ", out_source);
			fprintf(out_source, "%s %s", (*$4)[i]->type, (*$4)[i]->name);
		}
		fprintf(out_source, "):Column(%d)", $7);
		for (int i=0;i<$4->size();i++)
			fprintf(out_source, ", %s(%s)", (*$4)[i]->name, (*$4)[i]->name);
		fputs(" {}\n\n", out_source);
	    
	    cur_form->subclasses.push_back(strdup(modelname));
	  }
	  ColumnModelItems '}'
	  {
		scanner_end_column_model_items();
	    fprintf(out_source, "};\n\n");
	  }
	| ItemDefs CURVE_MODEL '(' Args ')' '{'
	  {
		scanner_begin_curve_model_items();

		char modelname[12];
		strcpy(modelname, "Curve");
		strcat(modelname, cur_form->namecounter.get_upper());
		
		if (LayoutElement* el=dynamic_cast<LayoutElement*>(layout_stack.top())) {
			el->init_code+="\t@.add_curve(new ";
			el->init_code+=modelname;
			el->init_code+="(";
			for (int i=0;i<$4->size();i++) {
				if (!(*$4)[i]->value) { yyerror("curve model argument must have initializer"); continue; }
				if (i) el->init_code+=", ";
				el->init_code+=(*$4)[i]->value;
			}
			el->init_code+="));\n";
		}
	    
	    fprintf(out_source, "\nclass %s::%s:public GUI::CurveView::Curve {\n", cur_form->element_type, modelname);
		for (int i=0;i<$4->size();i++)
			fprintf(out_source, "\t%s %s;\n", (*$4)[i]->type, (*$4)[i]->name);
			
	    cur_form->subclasses.push_back(strdup(modelname));
	  }
	  CurveModelItems '}'
	  {
		char* modelname=cur_form->subclasses.back();
		
	    fprintf(out_source, "public:\n\t%s(", modelname);
		for (int i=0;i<$4->size();i++) {
			if (i) fputs(", ", out_source);
			fprintf(out_source, "%s %s", (*$4)[i]->type, (*$4)[i]->name);
		}
		fprintf(out_source, ")");
		for (int i=0;i<$4->size();i++)
			fprintf(out_source, "%c %s(%s)", i?',':':', (*$4)[i]->name, (*$4)[i]->name);
		fputs(" {\n", out_source);
		fputs($8->c_str(), out_source);

		scanner_end_curve_model_items();
	    fprintf(out_source, "\t}\n};\n\n");
	    
	    delete $8;
	  }
	;
	
LayoutItem:
	  ELEMENT TypeName '{' 
	  {
		LayoutElement* el=new LayoutElement;
		el->element_type=strdup($2->c_str());
		layout_stack.push(el);
		delete $2;
	  }
	  ItemDefs '}' { $$=layout_stack.top(); layout_stack.pop(); layout_stack.top()->add_child($$); }
	| HBOX '{'
	  {
		LayoutBox* box=new LayoutBox;
		box->orientation=LayoutBox::HBOX;
		layout_stack.push(box);
	  }
	  ItemDefs '}' { $$=layout_stack.top(); layout_stack.pop(); layout_stack.top()->add_child($$); }
	| VBOX '{'
	  {
		LayoutBox* box=new LayoutBox;
		box->orientation=LayoutBox::VBOX;
		layout_stack.push(box);
	  }
	  ItemDefs '}' { $$=layout_stack.top(); layout_stack.pop(); layout_stack.top()->add_child($$); }
	| FREE '{'
	  {
		layout_stack.push(new FreeLayout);
	  }
	  ItemDefs '}' { $$=layout_stack.top(); layout_stack.pop(); layout_stack.top()->add_child($$); }
	| HSTRUT INTEGER ';' { $$=new LayoutStrut($2, 0); layout_stack.top()->add_child($$); }
	| VSTRUT INTEGER ';' { $$=new LayoutStrut(0, $2); layout_stack.top()->add_child($$); }
	;
	

ColumnModelItems:
	| ColumnModelItems COLUMN_MODEL_DRAW '(' IDENTIFIER ',' IDENTIFIER ',' IDENTIFIER ',' IDENTIFIER ')' LongCode
	  {
		fprintf(out_source, "\tvirtual void draw(int %s, int %s, int %s, bool %s) const\n\t{\n%s\n\t}\n\n", $4, $6, $8, $10, $12->c_str());
	  }
	| ColumnModelItems COLUMN_MODEL_EDIT '(' IDENTIFIER ')' LongCode
	  {
		fprintf(out_source, "\tvirtual GUI::Widget* edit(int %s)\n\t{\n%s\n\t}\n\n", $4, $6->c_str());
	  }
	| ColumnModelItems METHOD IDENTIFIER '(' Args ')' ReturnType LongCode
	  {
		fprintf(out_source, "\t%s %s(", $7->c_str(), $3);
		for (int i=0;i<$5->size();i++) {
			if (i) fputs(", ", out_source);
			fprintf(out_source, "%s %s", (*$5)[i]->type, (*$5)[i]->name);
		}
		fprintf(out_source, ")\n\t{\n%s\n\t}\n", $8->c_str());
		
		delete $7;
		delete $8;
	  }
	;
	

CurveModelItems:
	  { $$=new std::string; }
	| CurveModelItems CURVE_MODEL_EVAL '(' IDENTIFIER ')' LongCode
	  {
		$$=$1;
		fprintf(out_source, "\tvirtual float evaluate(float %s) const\n\t{\n%s\n\t}\n\n", $4, $6->c_str());
	  }
	| CurveModelItems CURVE_MODEL_COLOR INTEGER ',' INTEGER ',' INTEGER ';'
	  {
		char tmp[64];
		sprintf(tmp, "\t\tset_color(GUI::Color(%d, %d, %d));\n", $3, $5, $7);
		
		$$=$1;
		$$->append(tmp);
	  }
	;


LongCode: '{' { scanner_begin_cpp_code(); } LongCodeItem '}' { scanner_end_cpp_code(); $$=$3; }

LongCodeItem:
	  { $$=new std::string; }
	| LongCodeItem CODE { $$=$1; $$->append($2); }
	| LongCodeItem '{' LongCodeItem '}' { $$=$1; $$->append("{"); $$->append(*$3); $$->append("}"); delete $3; }
	;
	
LongExpr: { scanner_begin_cpp_expr(); } LongExprItem { scanner_end_cpp_expr(); $$=$2; }

LongExprList:
	  { scanner_begin_cpp_expr(); } LongExprItem { $$=$2; }
	| LongExprList ',' LongExprItem { $$=$1; $$->append(", "); $$->append(*$3); delete $3; }
	;

LongExprItem:
	  { $$=new std::string; }
	| LongExprItem CODE { $$=$1; $$->append($2); }
	| LongExprItem '(' LongExprItem ')' { $$=$1; $$->append("("); $$->append(*$3); $$->append(")"); delete $3; }
	;

