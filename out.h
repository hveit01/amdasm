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
#ifndef __OUT_H__
#define __OUT_H__

class ColMap
{
protected:
    int col[128];
    int ncols;
    int sz;
public:
    ColMap();
    ~ColMap() {}
    
    void AddColumn(int sz);
    void DumpLine(FILE* fd, const char* map) const;
    int Size() const { return sz; }
};

extern ColMap* columns;


class Lineout
{
protected:
    Lineout* next;
    char* line;
    int sz;
    int address;
    Def* curdef;
    int curvfs;
    
    static Lineout* root;
    static bool revflag;
    static Lineout* reverse();
    
    const char* lineno(bool hex);
    void dump_map_line(FILE* fd, bool hex, bool linewrap=true);
    void dump_bpnf_line(FILE* fd, bool hex, int xreplace);
    void dump_grouped_line(FILE* fd, bool hex);

/* dmode argument */
#define DM_REPL     0x0ff
#define DM_REPL1    '1'
#define DM_REPL0    '0'
#define DM_ADDR     0x100
#define DM_HEX      0x200
#define DM_SPACE    0x400
    void dump_byte(FILE* fd, int dmode, const char* lp, int n);
    void dump_byte_line(FILE* fd, int dmode);
    void dump_bin_line(FILE* fd, int dmode);
public:
    Lineout();
    ~Lineout();
    
    int LocPtr() const { return address; }
    bool SetOverlayFormat(const char* name);
    bool SubstField(const Field* arg);
    bool SubstArg(const Fdecl& val);
    bool GetNameArg(const char* name, Fdecl* res) const;
    void SkipArg();
    
    static Lineout* First() { return reverse(); }
    Lineout* Next() const { return next; }

    void PrintMapLine(Printer* p, bool hex);

#define SUB_CURMAP 1
#define SUB_NEWLINE 2
#define SUB_SKIPARG 3
#define SUB_NEWFORMAT 4
#define SUB_VFS 5
#define SUB_FIELD 6
    void DebugSubst(int flag);

    static void DumpGrouped(FILE* fd);
    static void DumpMap(FILE* fd);
    static void DumpBPNF(FILE* fd, int dmode);
    static void DumpBytes(FILE* fd, int dmode);
    static void DumpBin(FILE* fd, int dmode);
};

#endif
