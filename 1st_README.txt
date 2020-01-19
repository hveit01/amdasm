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

Table of Contents
=================

  * Purpose of the software
  * Installation
  * Usage
  * Bugs
  * Version history
  * References



Purpose of the software
=======================

This is a clone of the copyrighted AMD Micro Assembler, which was distributed
many years ago for the CP/M-like AMSYS/29 Bit-Slice development environment.
Even if this software is meanwhile downloadable from the internet (see
https://bitsavers.org/bits/AMD/AM29), it is not so easy to get it running
on modern computers, including data transfer from and to the supposedly
used emulator software.

I have therefore written a clone of this assembler software which attempts
to mimic the original behaviour as close as possible. This new software
uses different command line arguments and options, so it does not replace
the original toolchain, but it should accept the same input files (*.DEF
and *.SRC) as the original, and should produce almost identical print
output files. Several new output options will produce result files (output
maps) which should be processable by other software.

As the above header suggests, this software is free software under the
GNU General Public License GPLv3.

Installation
============

This software is written in a rather baroque dialect of C++ which neglects
almost every modern feature of C++ - no strings, no streams. no stdc++
library, no templates - not because I do not know about it, but to
provide a high degree of portability.

However, I didn't bother to create the also modern autoconf/automake
builder files - unless you have the correct version of Linux running, they
- despite of the intention to be portable - have caused the worst
installation nightmares for me within the last decades. Don't ask further.
What you get is a plain old Makefile, which should (after two simple manual
changes) work under Windows or Linux (haven't tried MacOSX).

No installation is necessary; the result is a single executable file.

The toolchain you need is likewise simple:
 * Windows:   MinGW/Cygwin, Gmake, (Win)Flex and (Win)Bison
 * Linux:     gcc/g++, make, flex, bison

The Makefile needs two small changes:
For Windows: uncomment (remove the #) in the lines
#EXE = .exe
#RM = del

For Unixoids uncomment the following:
#EXE =
#RM = rm

Of course, Cygwin or mingw might already provide an 'rm', and the g++ 
linker might silently add the .exe extension to the binary.

Run "make" to build the target (amdasm.exe or amdasm, resp.).
"make clean" will likewise remove the objects and intermediate files.



Usage
=====

Running amdasm without arguments will show its usage summary.

-------------------------------------------------------------------
AMDASM Microassembler Clone V1.0 (c)2019 Holger Veit
Usage: .\amdasm [-D def][-S src][-1 list1][-2 list2][-ofmt file][-h][-q][-n][-v][-P lpp] [prefix]
        prefix          Use default naming for input and output
        -D def          Override name of DEF input file
        -S src          Override name of SRC input file
        -1 list1        Override name of PHASE1 list file
        -2 list2        Override name of PHASE2 list file
        -h              Addresses as hex (default)
        -q              Addresses as octal
        -n              Suppress listing, unless -1 or -2 is given
        -v              Verbose(r) console output
        -P lpp          Set lines per page (default 66)
        -ofmt file              Set output format
                -ob[PN] BPNF format (X as P or N)
                -oh[01] Byte Hex dump (X as 0 or 1)
                -om     AMD Map format (01X)
                -ovb[01]        Verilog $readmemb (X as 0 or 1)
                -ovh[01]        Verilog $readmemh (X as 0 or 1)


AMDASM CLONE Copyright (C) 2019  Holger Veit <hveit01@web.de>
This program comes with ABSOLUTELY NO WARRANTY; see enclosed GPLv3
license file "GNU-GPL-License.txt".
This is free software, and you are welcome to redistribute it
under certain conditions, see enclosed license file.
-------------------------------------------------------------------

The normal case is that you have a MYFILE.DEF and a MYFILE.SRC.

Then you run it with
    amdasm MYFILE

This will, provided there are no errors, write the two print files
MYFILE.p1l und MYFILE.p2l, which will resemble the normal output of
the original program.

The above command is entirely quiet. To get a hint of what it does,
add the option -v, as in 
	amdadm -v MYFILE

To see even more, use the options -d1, -d2, -d4, -d8, or even -d15 ...
Better don't do this ;-)

Use at least one output option, e.g. "-om MYFILE.map", otherwise no
output will be produced. Multiple output options are allowed. Be aware
that while the standard argument (MYFILE) will silently add the
appropriate extensions if they are missing, the output files need
explicit naming; a "-om MYFILE -obp MYFILE" will first produce a
MAP output file MYFILE and then directly overwrites it with the BPNF output.

When using -S and -D options, the default print files are AMDOUT.p1l/.p2l
unless explicitly renamed with the -1 and -2 options.




Bugs
====

I am sure there are still many bugs in the code; if you find one, contact me
at the e-mail address in the header, but don't expect immediate response and
fixes. This is free software, and I have limited time to actively support it
or add features. Read the fine print of the license.

While we are at "unexpected features":
* the output of the label table is not sorted - maybe I throw it into a sorter
  in a future version...
* Numerous strings are not freed correctly; could be fixed, but memory is
  cheap nowadays, and the program won't run very long. Let the OS remove
  the garbage after termination of the run.
* And the code is surely not an artwork in C++ programming. Yes, then it
  isn't. I needed the function, not the form. Feel free to change it, it
  is free software.


Version history
===============

1.0     20190910    Initial version
1.0.1   20190911    Fixed non-portable itoa()
1.0.2	20190912	Removed linewrap for MAP output (-om)

  

References
==========
1. Original software: https://bitsavers.org/bits/AMD/AM29
2. Original manual: https://bitsavers.org/pdf/amd/amsys29/AMSYS29_uProgSuppSW.pdf
3. Testcase cpuii.def/src is taken from the book "Bit-Slice Microprocessor Design"
   by Mick & Brick, see https://bitsavers.org/components/amd/Am2900/Mick_Bit-Slice_Microprocessor_Design_1980.pdf


Holger Veit

