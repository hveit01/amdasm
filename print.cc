#include "amdasm.h"

Printer::Printer(const char* file, int ph)
    : outf(0), lpp(66), lcnt(66), lineno(1), errcnt(0), list(true), 
      lno_mode(P_LNO_DEC), phase(ph)
{
    lpp = set->LinesPerPage();
    title = 0;

    linebuf = new char[4096];
    clear_line();

    errorbuf = new char[4096];
    clear_error();
    
    if (file) {
        outf = fopen(file,"w");
        verbose("*** Write phase %d listing to %s\n", phase, file);
    }
}

Printer::~Printer()
{
    if (outf) {
        fprintf(outf, "\n\nTOTAL PHASE %d ERRORS = %4d\n", phase, errcnt);
        fclose(outf);
    }
    delete title;
    delete linebuf;
    delete errorbuf;
}

void Printer::Emit(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    
    if (outf && list) {
        vfprintf(outf, fmt, ap);
        count_nl(fmt);
    }
}

void Printer::NewLine(int n)
{
    for (int i=0; i<n; i++)
        Emit("\n");
}

void Printer::NewPage()
{
    if (list && outf) {
        if (lcnt >= (lpp-3)) {
            fprintf(outf,
                "\n\n\nAMDASM MICROASSEMBLER CLONE V%s (C)2019 HOLGER VEIT\n",
                VERSION);
            fprintf(outf, "%s\n\n", title ? title : "");
            lcnt = 6;
        }
    }
}

void Printer::SetTitle(const char* ttl)
{
    title = copystr(ttl);
}

void Printer::AddError(const char* s)
{
    strcat(errorbuf, s);
    strcat(errorbuf, "\n");
    errcnt++;
}

void Printer::Collect(const char* s)
{
    strcat(linebuf, s);
}

void Printer::print_lineno()
{
    switch (lno_mode) {
    case P_LNO_NO:
        return;
    case P_LNO_DEC:
        Emit("%5d ", lineno);
        break;
    case P_LNO_OCT:
        Emit("%06o ", lineno);
        break;
    case P_LNO_HEX:
        Emit("%04X ", lineno);
        break;
    default:
        internal_error(__FILE__, __LINE__);
    }
}

void Printer::count_nl(const char* buf)
{
    int len = strlen(buf);
    for (int i=0; i<len; i++) {
        if (buf[i]=='\n') lcnt++;
        NewPage();
    }
//fprintf(stderr,"buf=<%s> lcnt=%d\n", buf, lcnt);
}

void Printer::print_errors()
{
    /* put into output file */
    if (errorbuf[0] && outf)
        fprintf(outf, "\n%s", errorbuf);

    /* adjust number of lines emitted */
    count_nl(errorbuf);
    clear_error();
}

void Printer::Flush()
{
    NewPage();
    print_lineno();
    Emit(linebuf); NewLine(1);
    lineno++;

    clear_line();
    print_errors();
}

void Printer::Space(int n)
{
    NewLine(n);
}

void Printer::Eject()
{
    NewLine(lpp - lcnt);
}

void Printer::PrintSymbols()
{
    bool hex = set->HexMode();

    list = true;
    Eject();
    NewPage();

    lno_mode = P_LNO_NO;
    NewLine(2);

    Emit("ENTRY POINTS\n");
    NewLine();
    if (labels->PrintSymbols(this, true, hex))
        NewLine();

    Emit("SYMBOLS\n");
    NewLine();
    labels->PrintSymbols(this, false, hex);        
}

void Printer::PrintMap()
{
    bool hex = set->HexMode();
    lno_mode = P_LNO_NO;
    list = true;
    Eject();
    NewPage();
    for (Lineout* lo = Lineout::First(); lo; lo = lo->Next()) {
        lo->PrintMapLine(this, hex);
    }
}

/****************************************************************************/

Output* Output::oroot = 0;

Output::Output(const char* fm, const char* fil)
{
    fmt = copystr(fm);
    file = copystr(fil);
    next = oroot;
    oroot = this;
}

Output::~Output()
{
    delete fmt;
    delete file;
}

/* generate the various output files */
void Output::Dump()
{
    if (oroot==0) {
        verbose("*** No -o option given: no output files produced\n");
        return;
    }

    for (Output* o = oroot; o; o = o->next) {
        FILE* fd = fopen(o->file, "wb");
        if (fd == 0) {
            verbose("*** Cannot open output file %s\n", o->file);
            continue;
        }
        const char* fmt = o->fmt;
        if (!strcasecmp(fmt, "bp"))
            Lineout::DumpBPNF(fd, 'P');
        else if (!strcasecmp(fmt, "bn"))
            Lineout::DumpBPNF(fd, 'N');
        else if (!strcasecmp(fmt, "h0"))
            Lineout::DumpBytes(fd, DM_ADDR|DM_SPACE|DM_HEX|DM_REPL0);
        else if (!strcasecmp(fmt, "h1")) {
            Lineout::DumpBytes(fd, DM_ADDR|DM_SPACE|DM_HEX|DM_REPL1);
        } else if (!strcasecmp(fmt, "q0")) {
            Lineout::DumpBytes(fd, DM_ADDR|DM_SPACE|DM_REPL0);
        } else if (!strcasecmp(fmt, "q1")) {
            Lineout::DumpBytes(fd, DM_ADDR|DM_SPACE|DM_REPL1);
        } else if (!strcasecmp(fmt, "m"))
            Lineout::DumpMap(fd);
        else if (!strcasecmp(fmt, "mg"))
            Lineout::DumpGrouped(fd);
        else if (!strcasecmp(fmt, "vb0")) {
            Lineout::DumpBin(fd, DM_REPL0);
        } else if (!strcasecmp(fmt, "vb1")) {
            Lineout::DumpBin(fd, DM_REPL1);
        } else if (!strcasecmp(fmt, "vh0")) {
            Lineout::DumpBytes(fd, DM_HEX|DM_REPL0);
        } else if (!strcasecmp(fmt, "vh1")) {
            Lineout::DumpBytes(fd, DM_HEX|DM_REPL1);
        } else {
            verbose("*** Unknown output format %s, ignored\n", fmt);
            fclose(fd);
            continue;
        }
        verbose("*** Write output format -o%s to %s\n", fmt, o->file);
        fclose(fd);
    }
}
