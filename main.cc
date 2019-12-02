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

Settings* set;
Symtab* symtab;
Symtab* labels;
Output* output;
ColMap* columns;

static int usage(const char *progname)
{
	fprintf(stderr, 
        "AMDASM Microassembler Clone V%s (c)2019 Holger Veit\n",
        VERSION);
	fprintf(stderr, 
        "Usage: %s [-D def][-S src][-1 list1][-2 list2][-ofmt file][-h][-q][-n][-v][-P lpp] [prefix]\n",
		progname);
	fprintf(stderr,
        "\tprefix\t\tUse default naming for input and output\n"
        "\t-D def\t\tOverride name of DEF input file\n"
        "\t-S src\t\tOverride name of SRC input file\n"
        "\t-1 list1\tOverride name of PHASE1 list file\n"
        "\t-2 list2\tOverride name of PHASE2 list file\n"
        "\t-h\t\tAddresses as hex (default)\n"
        "\t-q\t\tAddresses as octal\n"
        "\t-n\t\tSuppress listing, unless -1 or -2 is given\n"
        "\t-v\t\tVerbose(r) console output\n"
        "\t-P lpp\t\tSet lines per page (default 66)\n"
        "\t-ofmt file\t\tSet output format\n"
        "\t\t-ob[PN]\tBPNF format (X as P or N)\n"
        "\t\t-oh[01]\tByte Hex dump (X as 0 or 1)\n"
        "\t\t-om\tAMD Map format (01X)\n"
        "\t\t-ovb[01]\tVerilog $readmemb (X as 0 or 1)\n"
        "\t\t-ovh[01]\tVerilog $readmemh (X as 0 or 1)\n");
    fprintf(stderr,
        "\n\nAMDASM CLONE Copyright (C) 2019  Holger Veit <hveit01@web.de>\n"
        "This program comes with ABSOLUTELY NO WARRANTY; see enclosed GPLv3\n"
        "license file \"GNU-GPL-License.txt\".\n"
        "This is free software, and you are welcome to redistribute it\n"
        "under certain conditions, see enclosed license file.\n");

    exit(2);
}

int main(int argc, char* argv[])
{
	int c;
	int errors;
    bool hasdef = false;
    bool hassrc = false;
    int debug = 0;
    bool verb = false;
    
    set = Settings::Instance();
	symtab = new Symtab();
    labels = new Symtab();
    columns = new ColMap();
    output = 0;
    
	while ((c=getopt(argc, argv, "vqhnd:D:S:1:2:o:l:")) != -1) {
		switch (c) {
		default:
		case '?':
			usage(argv[0]);
			break;
		case 'D':
			set->SetDefFile(optarg);
            hasdef = true;
			break;
		case 'S':
			set->SetSrcFile(optarg);
            hassrc = true;
			break;
        case 'h':
            set->SetHexMode(true);
            break;
        case 'q':
            set->SetHexMode(false);
            break;
		case '1':
			set->SetP1File(optarg);
			break;
		case '2':
			set->SetP2File(optarg);
			break;
		case 'o':
            new Output(optarg, argv[optind]);
            optind++;
			break;
		case 'n':
            set->SetNoList(true);
			break;
		case 'd':
            debug = atol(optarg);
            break;
        case 'P':
            set->SetLinesPerPage(atol(optarg));
            break;
        case 'v':
            verb = true;
		}
	}
	
	if (optind == (argc-1))
        set->SetPrefix(argv[optind]);
	else if (!hasdef || !hassrc)
		usage(argv[0]);

    if (verb) debug |= DBG_VERBOSE;
	set->SetDebug(debug);

    for (int phase = 1; phase <=3; phase++) {
        errors = parse_file(phase);
        if (errors != 0) break;
    }

	verbose("*** Finished: Errors = %d\n", errors);
	exit(errors ? 1 : 0);
}
