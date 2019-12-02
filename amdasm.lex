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
/* FLEX grammar for ASMASM */
%{
#include "amdasm.h"
#include "data.h"
#include "y.tab.h"

#undef YY_NO_UNPUT
static void yyunput( int c, register char *yy_bp );

Printer* p = 0;

static bool iskwd = false;

#define PR p->Collect(yytext)

/* convert label/entry to text */
static int yylval_str2(int token)
{
	yylval.str = copystr(yytext);
    *strchr(yylval.str,':') = '\0'; // strip trailing colon(s) */
    return token;
}

/* Note: this is a memory leak, because I don't worry about
 * freeing these strings again - we have plenty of memory! */
static int yylval_str(int token)
{
    iskwd = false;  /* a string symbol will always terminate keyword mode */
	yylval.str = copystr(yytext);
    return token;
}

static int yylval_tok(int token)
{
	if (iskwd) {
		iskwd = false;
		return token;
	}
    return yylval_str(NAME);
}

/* recognize format from [BQDH] */
static int bqdh_to_base(const char* txt)
{
	switch (txt[0]) {
	case 'B':	return 2;
	case 'Q':	return 8;
	case 'D':	return 10;
	case 'H':	return 16;
	default:	return internal_error(__FILE__, __LINE__);
	}
}

static int yylval_fmt()
{
	yylval.fmt = bqdh_to_base(yytext);
	return DEFFMT;
}

/* start building a {sz}X or {sz}V field */
static int yylval_fdecl(int fmt, int token)
{
    yylval.fdecl.Set(fmt, atol(yytext), 0);
	return token;
}

/* convert a number {optsz}[BQDH]#xxxxx */
static int yylval_sznum(int token)
{
    char* next;
    int sz = strtol(yytext, &next, 10); 
	int base = bqdh_to_base(next);
    int value = strtol(next+2, 0, base);
    int siz = sz ? sz : CField::Bitsize(next+2, value, base);
    yylval.fdecl.Set(base, siz, value);
    return token;
}

/* phase 2 accepts untyped constants for substitution, like "00E",
 * handle these like a special name */
static int yylval_untyped()
{
	/* in phase 1 interpret 00E like a decimal number and bail out at the 'E' later */
    if (set->Phase() == 1) {
		int value = atol(yytext);
        const char* end = yytext + strlen(yytext);

		/* advance to first non-digit */
        char* txt = yytext;
        while (isdigit(*txt)) txt++;

		/* pushback everything after the number */
        while (end > txt) unput(*--end);

		/* and claim it is a decimal number */
        yylval.fdecl.Set(F_DEC, CField::DecBitsize(value), value);
        return NUMBER;
    } else
		/* phase 2 must deal with that mess */
        return yylval_str(NAME);
}

%}

%s title comment vx

ws		[\t\r\f ]
name	[A-Z\.][A-Z0-9\.]*
hex     [0-9][0-9A-F]*
sz      [0-9]+
optsz   [0-9]*
newline	\n

%%
;				        { PR; BEGIN comment; }
<comment>[^\n]*         { PR; BEGIN 0; }
TITLE	                { if (iskwd) BEGIN title; 
                          else return yylval_str(NAME); }
<title>{ws}[^;\r\n]+	{ BEGIN 0; return yylval_str(TITLE); }

{optsz}B#[01]+          { PR; return yylval_sznum(CONST); }
{optsz}Q#[0-7]+         { PR; return yylval_sznum(CONST); }
{optsz}D#[0-9]+         { PR; return yylval_sznum(CONST); }
{optsz}H#[0-9A-F]+      { PR; return yylval_sznum(CONST); }
{sz}X                   { PR; return yylval_fdecl(F_DC,  DCFIELD); }
{sz}V	                { PR; BEGIN vx; return yylval_fdecl(F_VAR, VARFIELD); }
<vx>X[BQDH]				{ PR; BEGIN 0; unput(yytext[1]); return XOPT; }
<vx>X					{ PR; BEGIN 0; return XOPT; }
[BQDH]#                 { PR; return yylval_fmt(); }

{name}::	            { PR; return yylval_str2(ENTRY); }
{name}:		            { PR; return yylval_str2(LABEL); }

ALIGN		            { PR; return yylval_tok(ALIGN); }
COLS                    { PR; return yylval_tok(COLS); /* extension */ } 
DEF			            { PR; return yylval_tok(DEF); }
EJECT		            {     return yylval_tok(EJECT); }
END		            	{ PR; return yylval_tok(END); }
EQU			            { PR; return yylval_tok(EQU); }
FF		            	{ PR; return yylval_tok(FF); }
LIST	            	{     return yylval_tok(LIST); }
NOLIST		            {     return yylval_tok(NOLIST); }
ORG		            	{ PR; return yylval_tok(ORG); }
RES		            	{ PR; return yylval_tok(RES); }
SPACE	            	{     return yylval_tok(SPACE); }
SUB			            { PR; return yylval_tok(SUB); }
WORD		            { PR; return WORD; }
{name}	            	{ PR; return yylval_str(NAME); }

{hex}                   { PR; return yylval_untyped(); }
{newline}\/             { PR; p->Flush(); PR; /* continuation line */ }
{newline}	            { p->Flush(); iskwd = true; BEGIN 0; return NL; }

\+		            	{ PR; return PLUS; }
-	            		{ PR; return MINUS; }
\*	            		{ PR; return TIMES; }
\/		            	{ PR; return SLASH; }
,		            	{ PR; return COMMA; }
&	            		{ PR; return AMPERSAND; }
:	            		{ PR; return COLON; }
%           			{ PR; return PERCENT; }
\$	            		{ PR; return DOLLAR; }
\(	            		{ PR; return LPAREN; }
\)	            		{ PR; return RPAREN; }

\{		            	{ iskwd = true; return DEFMARK; }
\|	            		{ iskwd = true; return SRCMARK1; }
\}	            		{ iskwd = true; return SRCMARK2; }

{ws}*		            { PR; }
<<EOF>>	            	{ return 0; }
%%
