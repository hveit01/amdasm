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

ColMap::ColMap()
    : ncols(0), sz(0)
{}

void ColMap::AddColumn(int siz)
{
    if (ncols >= 127)
        internal_error(__FILE__, __LINE__);
    col[ncols++] = siz;
    sz += siz;
}

void ColMap::DumpLine(FILE* fd, const char* line) const
{
    int w = set->WordSize();
    if (ncols==0) ((ColMap*)this)->AddColumn(w);

    int i = 0;
    for (int n = ncols-1; n >= 0; n--) {
        if (i > 0) fputc(' ', fd);
        if (i == w) break;
        for (int k = 0; k < col[n]; k++)
            fputc(UN_OVL(line[i++]), fd);
    }
    fputc('\n', fd);
}

Lineout* Lineout::root = 0;
bool Lineout::revflag = false;

Lineout::Lineout()
    : next(Lineout::root), line(0), curdef(0), curvfs(0)
{
    Lineout::root = this;
    sz = set->WordSize();
    address = set->LocPtr();
    line = new char[sz+1]; line[sz] = '\0';
    memset(line, OVL('X'), sz);
    DebugSubst(SUB_NEWLINE);
}

Lineout::~Lineout()
{
    delete line;
}

/* Hey, my LISP finally yields fruit - reversing a list! */
Lineout* Lineout::reverse()
{
    if (root && !Lineout::revflag) { /* ensure it is done once only */
        Lineout* nroot = 0;
        for (Lineout* r = root; r; ) {
            Lineout* n = r->next;
            r->next = nroot;
            nroot = r;
            r = n;
        }
        root = nroot;
        revflag = true;
    }
    return Lineout::root;
}

/* set the current prototype def */
bool Lineout::SetOverlayFormat(const char* name)
{
    Symbol *def = symtab->Lookup(name);
    if (def && def->IsA()==ISA_DEF) {
        curdef = (Def*)def;
        curvfs = 0;
        DebugSubst(SUB_NEWFORMAT);
    } else {
        yyerror("Unknown definition");
        return false;
    }
    return curdef->Init(line);
}

bool Lineout::SubstField(const Field* fi)
{
    if ((fi->Offset() +fi->Size()) > sz)
        return false;
    return fi->Init(line);
}

/* substitute arg # n in current prototype and overlay result */
bool Lineout::SubstArg(const Fdecl& arg)
{
    if (!curdef) return 0;
    const VField* vfs = curdef->GetVfs(curvfs++);
    DebugSubst(SUB_VFS);
    return vfs ? vfs->Subst(line, arg) : false;
}

/* obtain a suitable Fdecl for the name to substitute */
bool Lineout::GetNameArg(const char* name, Fdecl* res) const
{
    /* need to first look for labels, then for EQUs */
    Symbol* s = labels->Lookup(name); /* could be a label */
    if (!s) s = symtab->Lookup(name); /* could be an EQU decl */
    if (s) {
        *res = s->GetValue();
        return true;
    }
    
    /* now assume an untyped constant. Get base from vfs */
    if (!curdef) return false;
    const VField* vfs = curdef->GetVfs(curvfs);
    if (!vfs) return false;
    
    int base = vfs->GetBase();
    if (base == 0) {
        yyerror("Invalid substitution");
        return false;
    }
    int val = strtol(name, 0, base); /* convert number */
    res->Set(base, vfs->Size(), val);
    res->FixSize();
    return true;
}

void Lineout::SkipArg()
{
    curvfs++;
    DebugSubst(SUB_SKIPARG);
}

const char* Lineout::lineno(bool hex)
{
    static char lbuf[80];
    sprintf(lbuf, hex ? "%04X " : "%06o ", address);
    return lbuf;
}

void Lineout::dump_map_line(FILE* fd, bool hex, bool linewrap)
{
    fprintf(fd, "%s", lineno(hex));
    for (int i=0; i < sz; i++) {
        if (i>0) {
            if (linewrap && (i % 64)==0) fprintf(fd, hex ? "\n     " : "\n       ");
            else if ((i % 16)==0) fputc(' ', fd);
        }
        fputc(UN_OVL(line[i]), fd);
    }
    fputc('\n', fd);
}

void Lineout::PrintMapLine(Printer* pr, bool hex)
{
    char lbuf[2];
    
    pr->Collect(lineno(hex));
    for (int i=0; i < sz; i++) {
        if (i>0) {
            if ((i % 64)==0) pr->Collect(hex ? "\n     " : "\n       ");
            else if ((i % 16)==0) pr->Collect(" ");
        }
        lbuf[0] = UN_OVL(line[i]);
        lbuf[1] = '\0';
        pr->Collect(lbuf);
    }
//    pr->Collect("\n");
    pr->Flush();
}

void Lineout::dump_bpnf_line(FILE* fd, bool hex, int xreplace)
{
    fprintf(fd, "%s", lineno(hex));
    fputc('B', fd);
    for (int i=0; i < sz; i++) {
        int bit = UN_OVL(line[i]);
        switch (bit) {
        case '0': bit = 'N'; break;
        case '1': bit = 'P'; break;
        case 'X': bit = xreplace; break;
        default:
            fprintf(stderr,"Invalid value %x found\n", bit);
            internal_error(__FILE__, __LINE__);
        }
        fputc(bit, fd);
    }
    fprintf(fd, "F\n");
}

void Lineout::dump_grouped_line(FILE* fd, bool hex)
{
    fprintf(fd, "%s", lineno(hex));
    columns->DumpLine(fd, line);
}

void Lineout::dump_byte(FILE* fd, int dmode, const char* lp, int n)
{
    int num = 0;
    for (int i=0; i<n; i++) {
        int bit = UN_OVL(lp[i]);
        if (bit == 'X') bit = dmode & DM_REPL;
        num <<= 1;
        if (bit == '1') num |= 1;
    }
    if (dmode & DM_SPACE) fputc(' ', fd);
    fprintf(fd, (dmode & DM_HEX) ? "%02X" : "%03o", num);
}

void Lineout::dump_byte_line(FILE* fd, int dmode)
{
    if (dmode & DM_ADDR) fprintf(fd, "%s", lineno(dmode & DM_HEX));

    int w = set->WordSize();
    int rest = w % 8;
    if (rest)
        dump_byte(fd, dmode, line, rest);
    for (int i=rest; i<w; i += 8)
        dump_byte(fd, dmode, line+i, 8);
    fputc('\n', fd);
}

void Lineout::dump_bin_line(FILE* fd, int dmode)
{
    int w = set->WordSize();
    for (int i=0; i<w; i++) {
        int bit = UN_OVL(line[i]);
        if (bit == 'X') bit = dmode & DM_REPL;
        fputc(bit, fd);
    }
    fputc('\n', fd);
}

void Lineout::DumpMap(FILE* fd)
{
    bool hex = set->HexMode();
    for (Lineout* lo = First(); lo; lo = lo->Next())
        lo->dump_map_line(fd, hex, false);
}

void Lineout::DumpBPNF(FILE* fd, int xreplace)
{
    bool hex = set->HexMode();
    for (Lineout* lo = First(); lo; lo = lo->Next())
        lo->dump_bpnf_line(fd, hex, xreplace);
}

void Lineout::DumpGrouped(FILE* fd)
{
    bool hex = set->HexMode();
    for (Lineout* lo = First(); lo; lo = lo->Next())
        lo->dump_grouped_line(fd, hex);
}

void Lineout::DumpBytes(FILE* fd, int dmode)
{
    for (Lineout* lo = First(); lo; lo = lo->Next())
        lo->dump_byte_line(fd, dmode);
}

void Lineout::DumpBin(FILE* fd, int dmode)
{
    for (Lineout* lo = First(); lo; lo = lo->Next())
        lo->dump_bin_line(fd, dmode);
}

void Lineout::DebugSubst(int flag)
{
    if (set->IsDebug(DBG_SUBST)) {
        switch (flag) {
        case SUB_CURMAP:
            fprintf(stderr, "--- Map line is now:\n");
            dump_map_line(stderr, true);
            fputc('\n', stderr);
            break;
        case SUB_NEWLINE:
            fprintf(stderr, "\n--- Start New Lineout\n");
            break;
        case SUB_SKIPARG:
            fprintf(stderr, "--- Argument skipped, vfs=%d\n", curvfs);
            break;
        case SUB_NEWFORMAT:
            fprintf(stderr, "--- New Format: ");
            curdef->Debug("DEF");
            break;
        case SUB_VFS:
            fprintf(stderr, "--- Substitute VFS %d\n", curvfs-1);
            break;
        }
    }
}

