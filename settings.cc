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
#include "amdasm.h"

Settings* Settings::_instance = 0;
Settings* Settings::Instance()
{
    if (!_instance)
        _instance = new Settings();
    return _instance;
}

Settings::Settings()
    : wordsize(0), nolist(false),
      lpp(66), debug(0), hex(true), locptr(0), phase(0)
{
    yydebug = 0;
    yy_flex_debug = 0;
    
    
    deffile =
    srcfile = 
    p1file =
    p2file =
    curfile = 0;
    prefix = copystr("amdout");
}

Settings::~Settings()
{
    delete deffile;
    delete srcfile;
    delete p1file;
    delete p2file;
    delete prefix;
}

int Settings::WordSize() const
{
    if (wordsize <= 0) {
        yyerror("WORD size not declared. Can't continue");
        exit(1);
    }
    return wordsize;
}

void Settings::SetWordSize(int w)
{
    if (wordsize)
        yyerror("Multiple setting of WORD size");
    else if (w < 1 or w > 128) {
        yyerror("Invalid WORD size. Can't continue");
        exit(1);
    } else
        wordsize = w;
}

void Settings::SetDebug(int flags)
{
    debug = flags;
    yydebug = (debug & DBG_YACC) ? 1 : 0;
    yy_flex_debug = (debug & DBG_LEX) ? 1 : 0;
}

int Settings::LocPtr() const
{
    if (phase == 1) {
        yyerror("May not use $ in phase 1");
        return 0;
    }
    return locptr;        
}

void Settings::SetLocPtr(int p)
{
    if (p < locptr)
        yyerror("May only increase location pointer");
    else
        locptr = p;
}

void Settings::IncLocPtr(int n)
{
    locptr += n;
}

void Settings::AlignLocPtr(int al)
{
    if (al != 2 && al != 4 && al != 8 && al != 16)
        yyerror("ALIGN must be 2, 4, 8, or 16");
    else {
        locptr -= (locptr % al);
        locptr += al;
    }
}

char* Settings::build_file(const char *prefix, const char* ext)
{
	char* name = new char[strlen(prefix)+1+strlen(ext)+1];
    strcpy(name, prefix);
    
    if (!strchr(prefix, '.'))
        strcat(name, ext);
    return name;
}

void Settings::SetPrefix(const char* pfx)
{
    delete prefix;
    prefix = copystr(pfx);
    char* dot = strchr(prefix, '.');
    if (dot) *dot = '\0';
}

const char* Settings::DefFile()
{
    return deffile ? deffile : build_file(prefix, ".def");
}

void Settings::SetDefFile(const char* name)
{
    delete deffile;
    deffile = build_file(name, ".def");
}

const char* Settings::SrcFile()
{
    return srcfile ? srcfile : build_file(prefix, ".src");
}

void Settings::SetSrcFile(const char* name)
{
    delete srcfile;
    srcfile = build_file(name, ".src");
}

const char* Settings::P1File()
{
    return p1file ? p1file : 
           (nolist ? 0 : build_file(prefix, ".p1l"));
}

void Settings::SetP1File(const char* name)
{
    delete p1file;
    p1file = build_file(name, ".p1l");
}

const char* Settings::P2File()
{
    return p2file ? p2file : 
           (nolist ? 0 : build_file(prefix, ".p2l"));
}

void Settings::SetP2File(const char* name)
{
    delete p2file;
    p2file = build_file(name, ".p2l");
}

void Settings::SetCurFile(const char* fname)
{
    delete curfile;
    curfile = copystr(fname);
}

