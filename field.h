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
#ifndef __FIELD_H__
#define __FIELD_H__

/* fdecl is the non-class-container for YYSTYPE */

/* define the default base*/
#define F_MASK  0x0000001f
#define F_BIN   0x00000002
#define F_OCT   0x00000008
#define F_DEC   0x0000000a
#define F_HEX   0x00000010

/*define the field type */
#define F_DC    0x80000000
#define F_VAR   0x40000000

/* define the V attr */
#define FA_INV	0x00010000		/* * inversion */
#define FA_NEG  0x00020000		/* - negation */
#define FA_TRNC 0x00040000		/* : truncation */
#define FA_RITE	0x00080000		/* % right justify */
#define FA_PAGE 0x001c0000		/* $ paged */
#define FA_XINI 0x00200000		/* initialize with X */
#define FA_VAL  0x00400000      /* has a default value */

/* define the default arg attr */
#define FM_INV	0x00000100		/* * inversion */
#define FM_NEG  0x00000200		/* - negation */
#define FM_TRNC 0x00000400		/* : truncation */
#define FM_RITE	0x00000800		/* % right justify */
#define FM_PAGE 0x00001c00		/* $ paged */



/* YYSTYPE field declaration with/out default */
struct Fdecl
{
	int fmt;
	int sz;
	int value;

    void Set(int f, int s, int v) { fmt=f; sz=s; value=v; }
    void FixSize(); /* correct sz field if it is 0 */
};

/* marker that value may be overwritten */
#define OVL(x) ((x) | 0x80)
#define IS_OVL(x) (((x) & 0x80)==0x80)
#define UN_OVL(x) ((x) & 0x7f)

/*forward*/ class CField;

/* baseclass, stores a don't care field */
class Field
{
protected:
    int sz;
    char* map;
    int offset;
    
    Field(int siz);
    char* mkbuf(int size, int defval);
    bool copy_bit(char* tgt, const char* src) const;
public:
    Field(const Fdecl& fd, int off=0);
    Field(const Field& org);
    virtual ~Field();

    int Size() const { return sz; }

    void SetOffset(int off) { offset = off; }
    int Offset() const { return offset; }
    
    virtual bool IsVField() const { return false; }
    virtual bool Init(char* buf) const;
    virtual void Debug(bool dummy=true) const; 
    virtual void DebugSubst(const char* src) const; 
    virtual Field* Clone() const { return new Field(*this); }
};

/* stores a constant value */
class CField : public Field
{
protected:
    int value;
    int fmt;
    
    CField(int sz, int fmt);
    void init_const(char* buf, int mod, int value, bool overlayable) const;
    bool copy_rev(char* tgt, const char* src) const;
public:
    CField(const Fdecl& fd, int off=0);
    CField(const CField& org);
    ~CField() {}
    
    static int Bitsize(const char* txt, int value, int base);
    static int DecBitsize(int value);
    static void DebugConst(int fmt, int value, int siz, bool putsize);
    static bool ResolveDecimal(const char* name, Fdecl* res);
    
    bool IsVField() const { return false; }
    bool Init(char* buf) const;
    void Debug(bool putsize=true) const;
    void DebugSubst(const char* src) const; 
    Field* Clone() const { return new CField(*this); }
    int GetBase() const { return fmt & F_MASK; }
};

/* stores a Var field, optionally with a default value */
class VField : public CField
{
public:
    VField(const Fdecl& fd, int off=0);
    VField(const VField& org);
    ~VField() {}
    
    bool IsVField() const { return true; }
    bool Init(char* buf) const;
    bool Subst(char* buf, const Fdecl& arg) const;
    void Debug(bool dummy=true) const;
    Field* Clone() const { return new VField(*this); }
};    

#endif
