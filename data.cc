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

Symbol::Symbol(const char* nam)
    : next(0)
{
    name = copystr(nam);
}

Symbol::~Symbol()
{
    delete name;
}

Field* Symbol::GetField(int dummy) const
{
    internal_error(__FILE__, __LINE__);
    return 0;
}

const Fdecl& Symbol::GetValue() const
{
    static Fdecl Wall_dummy;    /* make -Wall happy */
    
    fprintf(stderr, "ISA=%d\n", IsA());

    internal_error(__FILE__, __LINE__);

    /*NOTREACHED*/
    return Wall_dummy;
}

void Symbol::Dump(FILE* fd) const
{
    fprintf(fd, "Symbol: %s\n", name);
}

void Symbol::Print(Printer* pr, bool dummy) const
{
    pr->Emit("Symbol: %s\n", name);
}

/****************************************************************************/

Sub::Sub(const char* name, int max)
    : Symbol(name), nf(0), maxf(max), sz(0)
{
    f = new Field*[maxf];
}

Sub::~Sub()
{
    delete[] f;
}

bool Sub::AddField(Field* fi)
{
    if (nf >= maxf) {
        yyerror("Too many fields in SUB/DEF declaration");
        return false;
    }
    
    int fisz = fi->Size();
    if (fisz == 0)
        internal_error(__FILE__, __LINE__);

    fi->SetOffset(sz);
    sz += fisz;

    f[nf++] = fi;
	return true;
}

/* lookup an EQU or SUB, and include it in this def/sub */
bool Sub::Include(const char* name)
{
    Symbol* s = symtab->Lookup(name);
    if (!s) {
        yyerror("Name not defined");
        return false;
    }
    if (s->IsDef()) {
        yyerror("May not include DEF in DEF");
        return false;
    }

    /* Append FieldCnt fields to definition */
    int n = s->FieldCnt();
    for (int i=0; i < n; i++) {
        Field* fd = s->GetField(i);
        if (!AddField(fd)) return false;
    }
    return true;
}

void print_const(int sz, int fmt, int value)
{
    if (fmt & F_BIN) {
        char str[80]; bin2str(value, str);
        fprintf(stderr, "%dB#%s", sz, str);
    } else if (fmt & F_OCT)
        fprintf(stderr, "%dQ#%o", sz/3, value);
    else if (fmt & F_DEC)
        fprintf(stderr, "D#%d", value);
    else if (fmt & F_HEX)
        fprintf(stderr, "%dH#%X", sz/4, value);
    else {
        fprintf(stderr, "\nFORMAT=%08x\n", fmt);
        internal_error(__FILE__, __LINE__);
    }
}

const Field* Sub::get(int i) const
{
    return (i<0 || i>nf) ? f[0] : f[i];
};

void Sub::Debug(const char* pfx)
{
    bool first = true;
    fprintf(stderr,"%s: %s ", Name(), pfx);
    for (int i=0; i<nf; i++) {
        const Field* fd = get(i);
        if (!first) fputc(',', stderr);
        first = false;
        fd->Debug();
    }
    fprintf(stderr, "\t(sz=%d)\n", Bitsize());
}

/****************************************************************************/

bool Def::Init(char* line)
{
    for (int i=0; i<nf; i++) {
        const Field* fd = get(i);
        if (!fd->Init(line)) {
            fprintf(stderr," Init failed %s i=%d!\n", name, i); exit(1);
            return false;
        }
    }
    return true;
}

const VField* Def::GetVfs(int num) const
{
    for (int i=0; i<nf; i++) {
        const Field* vfs = get(i);
        if (vfs->IsVField()) {
            if (num == 0) return (VField*)vfs;
            num--;
        }
    }
    yyerror("Too many VFS arguments");
    return 0;
}

/****************************************************************************/

Equ::Equ(const char* name, const Fdecl& nval)
    : Symbol(name), value(nval)
{
    value.FixSize();
}

void Equ::Debug()
{
    fprintf(stderr, "%s: EQU ", Name());
    CField::DebugConst(value.fmt, value.value, value.sz, true);
    fputc('\n', stderr);
}

/****************************************************************************/

Symtab::Symtab()
{
    for (int i=0; i<TBLSIZE; i++)
        tbl[i] = 0;
}

int Symtab::hash(const char* name)
{
    int h = 0;
    const char* n = name;
    while (*n) {
        h += *n++;
    }
    return h % TBLSIZE;
}

Symbol* Symtab::Lookup(const char* name)
{
    int h = hash(name);
    Symbol* s = tbl[h];
    while (s && strcasecmp(s->Name(), name)) s = s->Next();
    return s;
}

bool Symtab::Enter(Symbol* sym)
{
    const char* name = sym->Name();
    if (Lookup(name)) {
        yyerror("Duplicate declaration");
        return false;
    }
    int h = hash(name);
    sym->next = tbl[h];
    tbl[h] = sym;
    return true;
}

/* lookup a value of an EQU */
bool Symtab::LookupValue(const char* name, Fdecl* res, bool quiet)
{
    Symbol* s = Lookup(name);
    if (s == 0) {
        if (!quiet) yyerror("Undeclared NAME");
        return false;
    }

    if (s->IsA() != ISA_EQU && s->IsA() != ISA_LABEL) {
        yyerror("NAME is not a constant value");
        return false;
    }
    *res = s->GetValue();
    return true;
}

bool Symtab::PrintSymbols(Printer* pr, bool dump_entry, bool hex) const
{
    bool found = false;
    for (int i=0; i<TBLSIZE; i++) {
        Symbol* s = tbl[i];
        while (s) {
            if (s->IsEntry()==dump_entry) {
                s->Print(pr, hex);
                found = true;
            }
            s = s->Next();
        }
    }
    return found;
}

/****************************************************************************/

Label::Label(const char* name, int loc, bool entry)
    : Symbol(name), locptr(loc), isentry(entry)
{}

const Fdecl& Label::GetValue() const
{
    static Fdecl ret;
    ret.Set(F_DEC|FM_PAGE, CField::DecBitsize(locptr), locptr);
    return ret;
}

void Label::Print(Printer* pr, bool hex) const
{
    pr->Emit(hex ? "%-8s %04X\n" : "%-8s %06o\n",
        Name(), locptr);
}

void Label::Dump(FILE* fd) const
{
    fprintf(fd, " %-8s %04X\n", Name(), locptr);
}
