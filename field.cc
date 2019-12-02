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

void Fdecl::FixSize()
{
    if (sz == 0) {
        sz = CField::DecBitsize(value);
        fmt = (fmt & ~F_MASK) | F_DEC;
    }
}

Field::Field(int siz)
    : sz(siz), map(0), offset(0)
{}

char* Field::mkbuf(int size, int defval)
{
    char* buf = new char[size+1];
    buf[size] = '\0';
    memset(buf, defval, size);
    return buf;
}

Field::Field(const Fdecl& fd, int off)
    : sz(fd.sz), offset(off)
{
    if (sz == 0) internal_error(__FILE__, __LINE__);

    map = mkbuf(sz, OVL('X'));
}

Field::Field(const Field& org)
    : sz(org.sz), offset(org.offset)
{
    if (sz == 0) internal_error(__FILE__, __LINE__);

    map = new char[sz+1];
    memcpy(map, org.map, sz+1);
}

Field::~Field()
{
    delete map;
    sz = 0;
}

bool Field::copy_bit(char* tgt, const char* src) const
{
    int tt = *tgt;
    int s = UN_OVL(*src);
    if (IS_OVL(tt)) {
        /* allow overlay, if target is overlayable */
        if (s != 'X') *tgt = *src;
    } else {
        /* otherwise only allow it, if source is the same or X */
        int t = UN_OVL(tt);
        if (t == s || s == 'X') {
            if (s != 'X') *tgt = *src;
        } else {
            yyerror("Field is already set");
            return false; /* try to set value into already set field */
        }
    }
    return true;
}

bool Field::Init(char* buf) const
{
    DebugSubst(map);

    for (int i=0; i<sz; i++) {
        if (!copy_bit(&buf[offset+i], &map[i])) return false;
    }
    return true;
}

void Field::Debug(bool dummy) const
{
    fprintf(stderr,"%dX", sz);
}

void Field::DebugSubst(const char* src) const
{
    if (!set->IsDebug(DBG_SUBST)) return;
    
    fprintf(stderr, "--- @X %03d(%02d) Set value ", offset, sz);
    for (int i=0; i<sz; i++)
        fprintf(stderr,"%c", UN_OVL(src[i]));
    fputc('\n', stderr);   
}

/****************************************************************************/

CField::CField(int sz, int fm)
    : Field(sz), value(0), fmt(fm)
{}

/* stores bits in reversed order! */
void CField::init_const(char* buf, int mod, int val, bool overlayable) const
{
    if (mod & FA_INV) val = ~val;
    if (mod & FA_NEG) val = -val;
    if (mod & FM_INV) val = ~val;
    if (mod & FM_NEG) val = -val;
    for (int i=0; i<16; i++) {
        buf[i] = (val & 1) ? '1' : '0';
        if (overlayable) buf[i] = OVL(buf[i]);
        val >>= 1;
    }
    buf[16] = 0;
}

CField::CField(const Fdecl& fd, int off)
    : Field(fd.sz), value(fd.value), fmt(fd.fmt)
{
    if (sz == 0) internal_error(__FILE__, __LINE__);
    
    offset = off;
    map = mkbuf(17, '\0'); // maximal size of constant
    
    /* constant modifiers are handled. value itself is as defined */
    init_const(map, fmt, value, false);
}

CField::CField(const CField& org)
    : Field(org), value(org.value), fmt(org.fmt)
{}

/* calculate natural bits for decimal, used to set Fdecl.sz in lexer */
int CField::DecBitsize(int value)
{
	if (value==0)
		return 1;
	int i;
	for (i=15; i>=0; i--)
		if (value & (1<<i)) break;
    return i + 1;
}

/* get "natural" bitsize of a number, used to set Fdecl.sz in lexer */
int CField::Bitsize(const char* txt, int value, int base)
{
	int sz = strlen(txt);
	switch (base) {
	case 2:		return sz;
	case 8:		return sz * 3;
	case 16:	return sz * 4;
	case 10: 	return DecBitsize(value);
	default:	return internal_error(__FILE__, __LINE__);
    }
}

bool CField::copy_rev(char* tgt, const char* src) const
{
    DebugSubst(src);
    for (int i=0; i<sz; i++) {
        if (!copy_bit(&tgt[offset+i], &src[sz-i-1])) return false;
    }
    return true;
}

void CField::DebugSubst(const char* src) const
{
    if (!set->IsDebug(DBG_SUBST)) return;

    fprintf(stderr,"--- @%c %03d(%02d) Set value ", 
            IsVField() ? 'V' : 'C', offset, sz);
    for (int i=0; i<sz; i++) {
        fprintf(stderr,"%c", UN_OVL(src[sz-i-1]));
    }
    fputc('\n', stderr);
}

bool CField::Init(char* buf) const
{
    return copy_rev(buf, map);
}

void CField::DebugConst(int fm, int val, int siz, bool putsize)
{
    int base = fm & F_MASK;
    if (putsize) fprintf(stderr, "%d", siz);
    switch (base) {
    case F_BIN:
        fprintf(stderr, "B#");
        for (int i=siz-1; i>=0; i--)
            fputc((val & (1<<i)) ? '1' : '0', stderr);
        break;
    case F_OCT:
        fprintf(stderr, "Q#%o", val);
        break;
    case F_HEX:
        fprintf(stderr, "H#%X", val);
        break;
    case F_DEC:
        fprintf(stderr, "D#%d", val);
        break;
    default:
        fprintf(stderr,"base=%d fmt=%x val=%x\n", base, fm, val);
        internal_error(__FILE__, __LINE__);
    }
    if (fm & FM_INV) fputc('*', stderr);
    if (fm & FM_NEG) fputc('-', stderr);
    if (fm & FM_TRNC) fputc(':', stderr);
    if (fm & FM_RITE) fputc('%', stderr);
    if (fm & FM_PAGE) fputc('$', stderr);
}

bool CField::ResolveDecimal(const char* name, Fdecl* res)
{
    if (isdigit(name[0])) {
        /* handle default format anaychronism in Phase 2a */
        char* n;
        int v = strtol(name, &n, 10);
        if (n[0] == '\0') {
            res->Set(F_DEC, 0, v);
            return true;
        } /*else
            yyerror("May not use an untyped constant here");*/
    }
    return false;
}

void CField::Debug(bool putsize) const
{
    DebugConst(fmt, value, sz, putsize);
}

/****************************************************************************/

VField::VField(const Fdecl& fd, int off)
    : CField(fd.sz, fd.fmt)
{
    if (sz == 0) internal_error(__FILE__, __LINE__);
    
    offset = off;
    
    map = mkbuf(17, (fmt & FA_XINI) ? OVL('X') : OVL(0));
    
    if (fmt & FA_VAL)
        init_const(map, fmt, fd.value, true);
}

VField::VField(const VField& org)
    : CField(org.sz, org.fmt)
{
    offset = org.offset;
    value = org.value;
    map = mkbuf(17, '\0');
    memcpy(map, org.map, 17);
}

bool VField::Init(char* buf) const
{
    if (!copy_rev(buf, map)) return false;
    for (int i=0; i<sz; i++)
        buf[offset+i] = OVL(buf[offset+i]);
    return true;
}

bool VField::Subst(char* buf, const Fdecl& arg) const
{
    char argbuf[17]; argbuf[16] = '\0';
    
    if (set->IsDebug(DBG_SUBST)) {
        fprintf(stderr,"--- @V %03d(%02d) Initvalue ", offset, sz);
        for (int i=0; i<sz; i++) {
            fprintf(stderr,"%c", UN_OVL(buf[offset+i]));
        }
        fputc('\n', stderr);
    }

    /* attributes and modifiers are handled in init_const */
    init_const(argbuf, arg.fmt | fmt, arg.value, false);
    return copy_rev(buf, argbuf);
}

void VField::Debug(bool dummy) const
{
    fprintf(stderr,"%dV", sz);
    if (fmt & FA_XINI) fputc('X', stderr);
    if (fmt & FA_INV) fputc('*', stderr);
    if (fmt & FA_NEG) fputc('-', stderr);
    if (fmt & FA_TRNC) fputc(':', stderr);
    if (fmt & FA_RITE) fputc('%', stderr);
    if (fmt & FA_PAGE) fputc('$', stderr);
    if (fmt & FA_VAL) CField::Debug(false);
}
