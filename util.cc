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

char* copystr(const char* str)
{
    char* s = new char[strlen(str)+1];
    strcpy(s, str);
    return s;
}

void yyerror(const char* msg)
{
    char errmsg[4096];
    
    const char* line = p->Linebuf();
    int col = strlen(line);
    sprintf(errmsg, "--- %s:%d:%d: error: %s\n"
                    "--- %s\n"
                    "--- %*s^~~~~~\n",
                    set->CurFile(), p->Lineno(), col, msg,
                    line,
                    col, "");

	p->AddError(errmsg);
    fprintf(stderr,"%s", errmsg);        
}

void verbose(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    
    if (set->IsDebug(DBG_VERBOSE))
        vfprintf(stderr, fmt, ap);
}

int yywrap()
{
	return 1;
}

int parse_file(int phase) 
{
    char marker;
    const char* infile;
    const char* pfile;
    
    Settings* s = Settings::Instance();    
    s->SetPhase(phase);
    
    const char* phname;
    switch (phase) {
    case 1:
        marker = '{';
        infile = s->DefFile();
        pfile = s->P1File();
        phname = "1";
        break;
    case 2: /* Pass 2a */
        marker = '|';
        infile = s->SrcFile();
        pfile = 0;  /* don't output listing in Pass 2a */
        phname = "2a";
        break;
    case 3: /* Pass 2b */
        marker = '}';
        infile = s->SrcFile();
        pfile = s->P2File();
        phase = 2;
        phname = "2b";
    }
    
	verbose("*** Parsing %s (Phase %s)\n", infile, phname);
    p = new Printer(pfile, phase); 

    s->SetCurFile(infile);
	yyin = fopen(infile, "r");
    if (yyin == 0) {
        fprintf(stderr,"*** File %s does not exist\n", infile);
        exit(1);
    } else {
        fseek(yyin, 0, SEEK_END);
        if (ftell(yyin) == 0) {
            fprintf(stderr,"File %s is empty\n", infile);
            exit(1);
        }
        fseek(yyin, 0, SEEK_SET);
    }
    
	ungetc(marker, yyin);
	yyparse();
	fclose(yyin);
    p->Flush();
    
    int errors = p->Errors();
	if (errors) {
		fprintf(stderr,
            "\n*** Failed to parse %s: %d error(s)\n", infile, errors);
	} else if (marker=='}') {
        p->PrintMap();
        p->PrintSymbols();
        Output::Dump();
    }

    delete p; p = 0;
	return errors;
}

int internal_error(const char* at, int line)
{
    fprintf(stderr,"In FILE=\"%s\", LINE=%d: Internal error\n",
        at, line);
    exit(99);
	
	/*NOTREACHED*/
	return 0;
}

/* convert a number into a binary value - the before used itoa() is non-portable :-( */
char* bin2str(int value, char* buf)
{
    char tmp[20];
    if (value==0)
        strcpy(buf, "0");
    else {
        int i, n;
        for (i=0; i<16; i++) {
            tmp[i] = (value & 1) ? '1' : '0';
            value >>= 1;
        }
        for (i=15; i >= 0; i--)
            if (tmp[i]=='1') break;
        for (n=0; i >= 0; i--, n++)
            buf[n] = tmp[i];
        buf[n] = '\0';
    }
    return buf;
}
