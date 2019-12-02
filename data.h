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
#ifndef __DATA_H__
#define __DATA_H__

class Symtab;

class Symbol
{
protected:
    char* name;
    Symbol *next;

    Symbol() : name(0), next(0) {}
    friend class Symtab;
public:
    
    Symbol(const char* nam);
    virtual ~Symbol();
    virtual int IsA() const { return 0; }
    const char* Name() const { return name; }
    Symbol* Next() const { return next; }  
    
    virtual bool IsEntry() const { return false; }
    virtual bool IsDef() const { return false; }
    virtual Field* GetField(int dummy=0) const;
    virtual int FieldCnt() const { return 1; }
    virtual const Fdecl& GetValue() const;
    virtual void Print(Printer* pr, bool hex) const;
    virtual void Dump(FILE* fd) const;
    virtual int Bitsize() const { return 0; }
};

#define MAXSUBFIELDS    10
#define ISA_SUB   2
class Sub : public Symbol
{
protected:
    Field ** f;
    int nf, maxf;
    int sz;
    const Field* get(int i) const;
    
public:
    Sub(const char* name, int max=MAXSUBFIELDS);
    ~Sub();
    int IsA() const { return ISA_SUB; }
    
    bool AddField(Field* fi);
    bool Include(const char* name);
    int FieldCnt() const { return nf; }
    Field* GetField(int i) const { return get(i)->Clone(); }
    int Bitsize() const { return sz; }
    void Debug(const char* pfx);
};

#define MAXDEFFIELDS    30
#define ISA_DEF   3
class Def : public Sub
{
public:
    Def(const char* name)
    : Sub(name, MAXDEFFIELDS) {}
    ~Def() {}
    int IsA() const { return ISA_DEF; }
    bool IsDef() const { return true; }
    bool Init(char* line);
    const VField* GetVfs(int num) const;
};

#define ISA_EQU   4
class Equ : public Symbol {
protected:
    Fdecl value;
public:
    Equ(const char* name, const Fdecl& nval);
    ~Equ() {}
    int IsA() const { return ISA_EQU; }
    
    Field* GetField(int dummy=0) const { return new CField(value); }
    const Fdecl& GetValue() const { return value; }
    int Bitsize() const { return value.sz; }
    void Debug();
};

#define TBLSIZE   97
class Symtab { /* Singleton pattern */
protected:
    Symbol* tbl[TBLSIZE];
    int hash(const char* name); 
public:
    Symtab();
    ~Symtab();
    
    Symbol* Lookup(const char* name);
    bool LookupValue(const char* name, Fdecl* res, bool quiet=false);
    bool Enter(Symbol* sym);
    
    bool PrintSymbols(Printer* pr, bool dump_entry, bool hex) const;
};

extern Symtab* symtab;
extern Symtab* labels;

#define ISA_LABEL 5
class Label : public Symbol
{
protected:
    int locptr;
    bool isentry;
public:
    Label(const char* name, int loc, bool entry=false);
    ~Label() {}
    int IsA() const { return ISA_LABEL; }
    
    const Fdecl& GetValue() const;
    bool IsEntry() const { return isentry; }
    void Print(Printer* pr, bool hex) const;
    void Dump(FILE* fd) const;
};

#endif
