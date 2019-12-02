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
#ifndef __PRINT_H__
#define __PRINT_H__

#define P_LNO_NO    0
#define P_LNO_DEC   1
#define P_LNO_OCT   2
#define P_LNO_HEX   3

class Printer
{
private:
    FILE* outf;
    int lpp, lcnt;
    int lineno;
    int errcnt;
    bool list;
    int lno_mode;
    int phase;
    char* title;
    char* linebuf;
    char* errorbuf;
    
    void print_lineno();
    void print_errors();
    void count_nl(const char* buf);
    void clear_error() { errorbuf[0] = '\0'; }
    void clear_line() { linebuf[0] = '\0'; }
    
public:
    Printer(const char* file, int ph);
    ~Printer();
    
    void SetTitle(const char* title);
    void SetLPP(int n) { lpp = n; }

    void AddError(const char* s);
    const char* Linebuf() const { return linebuf; }
    
    void Collect(const char* s);
    void Flush();
    void Emit(const char* fmt, ...);    
    void NewLine(int n = 1);

    void Eject();
    void NewPage();
    void Space(int n);

    int Errors() const { return errcnt; }
    void List() { list = true; }
    void Nolist() { list = false; }
    int Lineno() const { return lineno; }
    void SetLineno(int lno) { lineno = lno; }
    
    void PrintSymbols();
    void PrintMap();
};

extern Printer* p;

/* List of outputs to generate */
class Output
{
    struct Output* next;
    char* fmt;
    char* file;
    
    static Output* oroot;

public:
    Output(const char* fm, const char* fil);
    ~Output();
    
    static void Dump();
};

#endif
