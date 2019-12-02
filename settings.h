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
#ifndef __SETTING_H__
#define __SETTING_H__

class Settings
{
private:
    int wordsize;
    
    bool nolist;
    int lpp;
    
    int debug;
    bool hex;
    
    int locptr;
    int phase;
    
    char* deffile;
    char* srcfile;
    char* p1file;
    char* p2file;
    char* prefix;
    
    char* curfile;

    char* build_file(const char* pfx, const char* ext);

    static Settings* _instance;
    Settings();
public:
    static Settings* Instance();
    ~Settings();

    int WordSize() const;
    void SetWordSize(int w);
    
    int LinesPerPage() const { return lpp; }
    void SetLinesPerPage(int l) { lpp = l >= 10 ? l : 66; }
    
    int LocPtr() const;
    void ResetLocPtr() { locptr = 0; }
    void SetLocPtr(int p);
    void IncLocPtr(int n);
    void AlignLocPtr(int align);

    int Phase() const { return phase; }
    void SetPhase(int p) { phase = p; }
   
    bool NoList() const { return nolist; }
    void SetNoList(bool n) { nolist = n; }

    int IsDebug(int flag) const { return debug & flag; }
    void SetDebug(int d);
    
    bool HexMode() const { return hex; }
    void SetHexMode(int h) { hex = h; }   
    
    void SetPrefix(const char* pfx);
    
    const char* DefFile();
    void SetDefFile(const char* name);
    
    const char* SrcFile();
    void SetSrcFile(const char* name);
    
    const char* P1File();
    void SetP1File(const char* name);
    
    const char* P2File();
    void SetP2File(const char* name);
    
    const char* CurFile() const { return curfile; }
    void SetCurFile(const char* fi);
};

extern Settings* set;

#endif
