/*
 *   This file is part of the AMD Microassembler Clone software.
 *   Copyright (C) 2019  Holger Veit <hveit01@web.de>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
/* BISON grammar for AMDASM */
%{
#include "amdasm.h"
#include "data.h"

static Fdecl vdecl;
static Sub*  vdef;
static Lineout* outline;
static int ffcnt, vsize;
%}

%union
{
	int   fmt;
	char* str;
	Fdecl fdecl;
}

%token NL
%token <fdecl> CONST NUMBER
%token <fdecl> DCFIELD VARFIELD
%token ALIGN EJECT END EQU FF LIST NOLIST ORG RES SPACE
%token <fmt> DEFFMT
%token <str> TITLE
%token DEF
%token SUB
%token XOPT
%token WORD
%token <str> NAME ENTRY LABEL
%token COMMA AMPERSAND COLON
%token PERCENT DOLLAR
%token LPAREN RPAREN
%token DEFMARK SRCMARK1 SRCMARK2
%token COLS

%left PLUS MINUS TIMES SLASH

%type <fdecl> constant expr expr2
%type <str> label

%%


start
:	DEFMARK deffile
|	SRCMARK1 srcfile1
|	SRCMARK2 srcfile2
;

deffile
: 	def_stmts
	END
    emptylines
;

emptylines
:   /*empty*/
|   NL emptylines
;


def_stmts
:	/*empty*/
|	def_stmts def_stmt
;

printctrl_stmt
:   NL
|	TITLE           { p->SetTitle($1); } NL
|	LIST NL         { p->List(); }
|	NOLIST NL       { p->Nolist(); }
|	SPACE NUMBER NL { p->Space($2.value); }
|	EJECT NL        { p->Eject(); }
;

equ_stmt
:	label EQU expr	{ Equ* vequ = new Equ($1, $3);
                      symtab->Enter(vequ);
                      if (set->IsDebug(DBG_DEFS))
                        vequ->Debug();
                    }
;

microword_stmt
:   label DEF 		{ vdef = new Def($1); }
	fieldlist   	{ symtab->Enter(vdef);
                      if (set->IsDebug(DBG_DEFS))
                        vdef->Debug("DEF");
                      if (vdef->Bitsize() != set->WordSize()) {
                        yyerror("DEF size does not match WORD size");
                        vdef->Debug("DEF");
                        YYERROR; }
                    }
;

subword_stmt
:   label SUB 		{ vdef = new Sub($1); }
	fieldlist   	{ symtab->Enter(vdef);
                      if (set->IsDebug(DBG_DEFS))
                        vdef->Debug("SUB");
                    }
;

label
:	LABEL		    { $$ = $1; }
|	ENTRY		    { $$ = $1; }
;


def_stmt
:   printctrl_stmt
|	WORD NUMBER NL  { set->SetWordSize($2.value); }
|   equ_stmt NL
|	microword_stmt NL
|   subword_stmt NL
|   cols_extension NL
|   error NL
;

cols_extension
:   COLS cols_list  { int w = set->WordSize(), c = columns->Size();
                      if (w != c) {
                        yyerror("COLS size does not match WORD size");
                        if (w>c) columns->AddColumn(w-c);
                        YYERROR;
                      }
                    }
;

cols_list
:   NUMBER                  { columns->AddColumn($1.value); }
|   NUMBER COMMA cols_list  { columns->AddColumn($1.value); }
;

fieldlist
:	field
|	fieldlist COMMA field
;

field
:	DCFIELD				{ if (!vdef->AddField(new Field($1))) YYERROR; }
|	VARFIELD 			{ vdecl = $1; }
	opt_vattr
	opt_defval			{ if (!vdef->AddField(new VField(vdecl))) YYERROR; }
|	constant			{ vdecl = $1;
                          if (!(vdecl.fmt & F_MASK)) vdecl.fmt |= F_DEC;
                          if (!vdef->AddField(new CField(vdecl))) YYERROR;
                        }
|	NAME				{ if (!vdef->Include($1)) YYERROR; }
;

constant
:	NUMBER CONST		{ $$ = $2; $$.sz = $1.value; }
|	CONST				{ $$ = $1; }
|	NUMBER				{ $$ = $1; }
;

expr
:	constant			{ $$ = $1; }
|	NAME				{ if (!CField::ResolveDecimal($1, &$$) && 
                              !symtab->LookupValue($1, &$$)) YYERROR;
                        }
|	DOLLAR				{ $$.value = set->LocPtr(); $$.sz = 0; }
|	expr PLUS expr		{ $$.value = $1.value + $3.value; $$.sz = 0; } 
|	expr MINUS expr		{ $$.value = $1.value - $3.value; $$.sz = 0; } 
|	expr TIMES expr		{ $$.value = $1.value * $3.value; $$.sz = 0; } 
|	expr SLASH expr		{ $$.value = $1.value / $3.value; $$.sz = 0; } 
;

opt_vattr
:	/*empty*/
|	opt_vattr PERCENT   { vdecl.fmt |= FA_RITE; }
|	opt_vattr TIMES		{ vdecl.fmt |= FA_INV; }
|	opt_vattr MINUS		{ vdecl.fmt |= FA_NEG; }
|	opt_vattr COLON		{ vdecl.fmt |= FA_TRNC; }
|	opt_vattr DOLLAR	{ vdecl.fmt |= FA_PAGE; }
|	opt_vattr XOPT		{ vdecl.fmt |= FA_XINI; }
;

opt_defval
:	/*empty*/
|	DEFFMT opt_vmod     { vdecl.fmt |= $1; }
|	constant opt_vmod   { vdecl.value = $1.value; vdecl.fmt |= ($1.fmt|FA_VAL); }
;

opt_vmod
:	/*empty*/
|	opt_vmod PERCENT    { vdecl.fmt |= FM_RITE; }
|	opt_vmod TIMES	    { vdecl.fmt |= FM_INV; }
|	opt_vmod MINUS	    { vdecl.fmt |= FM_NEG; }
|	opt_vmod COLON	    { vdecl.fmt |= FM_TRNC; }
;

/***************************************************/

srcfile1
:	src_stmts1
	END                 { set->ResetLocPtr(); }
    emptylines
;

src_stmts1
:	/*empty*/
|	src_stmts1 src_stmt1
;

src_stmt1
:   NL
|	TITLE NL
|	LIST NL
|	NOLIST NL
|	SPACE NUMBER NL
|	EJECT NL
|   equ_stmt1 NL
|   ORG expr NL         { set->SetLocPtr($2.value); }
|	RES expr NL         { set->IncLocPtr($2.value); }
|	ALIGN expr NL       { set->AlignLocPtr($2.value); }
|	opt_label1 
    exec_stmt1 NL       { set->IncLocPtr(1); }
;

label1
:	LABEL		        { labels->Enter(new Label($1, set->LocPtr(), false)); }
|	ENTRY		        { labels->Enter(new Label($1, set->LocPtr(), true)); }
;

opt_label1
:	/*empty*/
|	label1
;

equ_stmt1
:	label EQU expr	    { Equ* vequ = new Equ($1, $3);
                          symtab->Enter(vequ);
                          if (set->IsDebug(DBG_DEFS))
                            vequ->Debug();
                        }
;

exec_stmt1
:	FF fffieldlist1
|	overlayformat_list1
;

fffieldlist1
:	fffield1
|	fffieldlist1 COMMA fffield1
;

fffield1
:	DCFIELD				
|   NAME LPAREN expr1 RPAREN /* note: in phase 2, a decimal number is
                              * regarded as an untyped constant, mapped to a name */
|	constant			
|	NAME				
;

overlayformat1
:	NAME opt_vfslist1
;

overlayformat_list1
:	overlayformat1
|	overlayformat_list1 AMPERSAND overlayformat1
;

opt_vfslist1
:	/*empty*/           
|	expr1
|	opt_vfslist1 COMMA expr1
|	opt_vfslist1 COMMA
;

expr1
:	constant
|	NAME				
|	DOLLAR				
|	expr1 PLUS expr1	
|	expr1 MINUS expr1	
|	expr1 TIMES expr1
|	expr1 SLASH expr1	
;

/****************************************************************************/

srcfile2
:	/*empty*/           { outline = 0; }
    src_stmts2
	END
    emptylines
;

src_stmts2
:	/*empty*/
|	src_stmts2 src_stmt2
;

src_stmt2
:   printctrl_stmt
|   equ_stmt2 NL
|   ORG expr NL         { set->SetLocPtr($2.value); }
|	RES expr NL         { set->IncLocPtr($2.value); }
|	ALIGN expr NL       { set->AlignLocPtr($2.value); }
|	opt_label2 exec_stmt NL { set->IncLocPtr(1); }
;

equ_stmt2
:	label EQU expr2	    { if (symtab->LookupValue($1, &vdecl)) {
                            if (vdecl.value != $3.value)
                              yyerror("Symbol value changed between phases");
                          }
                        }
;

opt_label2
:	/*empty*/
|	label
;

exec_stmt
:	FF                  { outline = new Lineout(); ffcnt = 0; }
    fffieldlist2        { if (ffcnt != set->WordSize())
                            yyerror("FF length does not match WORD size");
                          outline = 0;
                        }
|	overlayformat_list2 { outline = 0; }
;

fffieldlist2
:	fffield2
|	fffieldlist2 COMMA fffield2 
;

fffield2
:	DCFIELD				{ Field xf($1, ffcnt);
                          if (!outline->SubstField(&xf)) YYERROR;
                          ffcnt += $1.sz;
                        }
|   NAME LPAREN         { if (CField::ResolveDecimal($1, &vdecl))
                            vsize = vdecl.value;
                          else YYERROR;
                        }
    expr2 RPAREN        { $4.sz = vsize;
                          CField xc($4, ffcnt);
                            if (!outline->SubstField(&xc)) YYERROR;
                            ffcnt += vsize;
                        }
|	constant			{ if ($1.sz == 0) {
                            yyerror("Size of constant undefined"); YYERROR;
                          }
                          CField xc($1, ffcnt);
                          if (!outline->SubstField(&xc)) YYERROR;
                          ffcnt += $1.sz;
                        }
|	NAME				{ if (CField::ResolveDecimal($1, &vdecl) ||
                              symtab->LookupValue($1, &vdecl, true) ||
                              labels->LookupValue($1, &vdecl, false)) {
                            CField xc(vdecl, ffcnt);
                            if (!outline->SubstField(&xc)) YYERROR;
                            ffcnt += vdecl.sz;
                          } else YYERROR;
                        }
|   error COMMA
;

overlayformat2
:	NAME                { if (outline == 0) outline = new Lineout(); 
                          outline->SetOverlayFormat($1);
                        }
    opt_vfslist2        
;

overlayformat_list2
:	overlayformat2      { outline->DebugSubst(SUB_CURMAP); }
|	overlayformat_list2 AMPERSAND overlayformat2 { outline->DebugSubst(SUB_CURMAP); }
;

opt_vfslist2
:	/*empty*/           { outline->SkipArg(); }
|	expr2               { outline->SubstArg($1); }
|	opt_vfslist2 COMMA expr2 { outline->SubstArg($3); }
|	opt_vfslist2 COMMA  { outline->SkipArg(); }
;

expr2
:	constant            { $$ = $1; }
|	NAME				{ if (!CField::ResolveDecimal($1, &$$))
                            outline->GetNameArg($1, &$$);
                        }
|	DOLLAR				{ $$.Set(F_DEC, 0, outline->LocPtr()); }
|	expr2 PLUS expr2	{ $$.Set(F_DEC, 0, $1.value + $3.value); } 
|	expr2 MINUS expr2	{ $$.Set(F_DEC, 0, $1.value - $3.value); } 
|	expr2 TIMES expr2	{ $$.Set(F_DEC, 0, $1.value * $3.value); } 
|	expr2 SLASH expr2	{ $$.Set(F_DEC, 0, $1.value / $3.value); } 
;

%%
