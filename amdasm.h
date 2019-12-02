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
#ifndef __AMDASM_H__
#define __AMDASM_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>

#include "print.h"
#include "field.h"
#include "data.h"
#include "settings.h"
#include "out.h"

#define VERSION "1.0.2"

#define DBG_YACC    0x0001
#define DBG_LEX     0x0002
#define DBG_DEFS    0x0004
#define DBG_SUBST   0x0008
#define DBG_VERBOSE 0x0010

extern int yydebug;
extern int yy_flex_debug;

extern FILE *yyin;
extern int yyparse();
extern int yylex();
extern char *yytext;
extern void yyerror(const char* msg);
extern "C" {
	int yywrap();
}

extern int parse_file(int phase);
extern char* copystr(const char* str);
extern int decimal_bits(int value);
extern void print_const(int sz, int fmt, int value);
extern void verbose(const char* fmt, ...);
extern bool overlay(const Fdecl& fd, char* line);
extern int internal_error(const char* at, int line);
extern char* bin2str(int value, char* buf);

#endif
