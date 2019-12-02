#
#   This file is part of the AMD Microassembler Clone software.
#   Copyright (C) 2019  Holger Veit <hveit01@web.de>
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
CC = gcc
CCC = g++
CFLAGS = -g -Wall
YACC = bison -dyvt
LEX = flex -di

# use for windows
EXE = .exe
RM = del

# use for unix
#EXE =
#RM = rm

HEADERS = amdasm.h print.h data.h settings.h out.h field.h
OBJS = main.o parser.o lexer.o data.o print.o util.o settings.o out.o\
       field.o

all:	amdasm$(EXE)

clean:
	-$(RM) amdasm${EXE}
	-$(RM) *.o
	-$(RM) lex.yy.c
	-$(RM) y.tab.h
	-$(RM) y.tab.c
	-$(RM) y.output

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) -c $<

.cc.o: $(HEADERS)
	$(CCC) $(CFLAGS) -c $<

lex.yy.c: amdasm.lex $(HEADERS)
	$(LEX) $<

lexer.o: lex.yy.c y.tab.h $(HEADERS)
	$(CCC) $(CFLAGS) -o $@ -c $<

y.tab.c y.tab.h: amdasm.y $(HEADERS)
	$(YACC) $<

parser.o: y.tab.c $(HEADERS)
	$(CCC) $(CFLAGS) -o $@ -c $<
	
amdasm$(EXE): $(OBJS)
	$(CCC) -o $@ $^

