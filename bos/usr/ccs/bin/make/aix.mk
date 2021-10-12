# @(#)01        1.1.1.1  src/bos/usr/ccs/bin/make/aix.mk, cmdmake, bos41J, 9507C  2/8/95  10:54:03
#
# COMPONENT_NAME: CMDMAKE      System Build Facilities (make)
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#DEFAULT RULES FOR AIX
#       These are the internal rules that "make" trucks around with it at
#       all times. One could completely delete this entire file and define
#       a file that contains user defined rules. 	
#       That would make the rules dynamically changeable
#       without recompiling make. The user file may be modified to local
#       needs. 
#       The macro $(MAKERULES) defined in "make" can be set in the command line
#       to another file and when "make" is executed the program will
#       use the new file. The recommended way to do this is on the
#       command line as follows:
#               "make MAKERULES=/pathname/filename"

.SUFFIXES: .o .c .c~ .f .f~ .y .y~ .l .l~ .s .s~ .sh .sh~ .h .h~ .C .C~ .a

#PRESET VARIABLES
MAKE=make
AR=ar
ARFLAGS=-rv
YACC=yacc
YFLAGS=
LEX=lex
LFLAGS=
LD=ld
LDFLAGS=
CC=cc
CFLAGS=-O
FC=xlf
FFLAGS=-O
AS=as
ASFLAGS=
GET=get
GFLAGS=
CCC=xlC
CCFLAGS=-O

#SINGLE SUFFIX RULES
.c:
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@
.c~:
	$(GET) $(GFLAGS) -p $< > $*.c
	$(CC) $(CFLAGS) $(LDFLAGS) $*.c -o $*
	-rm -f $*.c
.f:
	$(FC) $(FFLAGS) $(LDFLAGS) $< -o $@
.f~:
	$(GET) $(GFLAGS) -p $< > $*.f
	$(FC) $(FFLAGS) $(LDFLAGS) $*.f -o $*
	-rm -f $*.f
.sh:
	cp $< $@; chmod a+x $@
.sh~:
	$(GET) $(GFLAGS) -p $< > $*.sh
	cp $*.sh $*; chmod a+x $@
	-rm -f $*.sh
.C:
	$(CCC) $(CCFLAGS) $(LDFLAGS) $< -o $@
.C~:
	$(GET) $(GFLAGS) -p $< > $*.C
	$(CCC) $(CCFLAGS) $(LDFLAGS) $*.C -o $*
	-rm -f $*.C

#DOUBLE SUFFIX RULES
.c.o:
	$(CC) $(CFLAGS) -c $<
.c~.o:
	$(GET) $(GFLAGS) -p $< > $*.c
	$(CC) $(CFLAGS) -c $*.c
	-rm -f $*.c
.c~.c:
	$(GET) $(GFLAGS) -p $< > $*.c
.C.o:
	$(CCC) $(CCFLAGS) -c $<
.C~.o:
	$(GET) $(GFLAGS) -p $< > $*.C
	$(CCC) $(CCFLAGS) -c $*.C
	-rm -f $*.C
.C~.C:
	$(GET) $(GFLAGS) -p $< > $*.C; chmod 444 $*.C
.s.o:
	$(AS) $(ASFLAGS) -o $@ $<
.s~.o:
	$(GET) $(GFLAGS) -p $< > $*.s
	$(AS) $(ASFLAGS) -o $*.o $*.s
	-rm -f $*.s
.f.o:
	$(FC) $(FFLAGS) -c $<
.f~.o:
	$(GET) $(GFLAGS) -p $< > $*.f
	$(FC) $(FFLAGS) -c $*.f
	-rm -f $*.f
.f~.f:
	$(GET) $(GFLAGS) -p $< > $@
.y.o:
	$(YACC) $(YFLAGS) $<
	$(CC) $(CFLAGS) -c y.tab.c
	rm y.tab.c
	mv y.tab.o $@
.y~.o:
	$(GET) $(GFLAGS) -p $< > $*.y
	$(YACC) $(YFLAGS) $*.y
	$(CC) $(CFLAGS) -c y.tab.c
	rm -f y.tab.c $*.y
	mv y.tab.o $*.o
.l.o:
	$(LEX) $(LFLAGS) $<
	$(CC) $(CFLAGS) -c lex.yy.c
	rm lex.yy.c
	mv lex.yy.o $@
.l~.o:
	$(GET) $(GFLAGS) -p $< > $*.l
	$(LEX) $(LFLAGS) $*.l
	$(CC) $(CFLAGS) -c lex.yy.c
	rm -f lex.yy.c $*.l
	mv lex.yy.o $*.o
.y.c :
	$(YACC) $(YFLAGS) $<
	mv y.tab.c $@
.y~.c :
	$(GET) $(GFLAGS) -p $< > $*.y
	$(YACC) $(YFLAGS) $*.y
	mv y.tab.c $*.c
	-rm -f $*.y
.l.c :
	$(LEX) $<
	mv lex.yy.c $@
.c.a:
	$(CC) -c $(CFLAGS) $<
	$(AR) $(ARFLAGS) $@ $*.o
	rm -f $*.o
.c~.a:
	$(GET) $(GFLAGS) -p $< > $*.c
	$(CC) -c $(CFLAGS) $*.c
	$(AR) $(ARFLAGS) $@ $*.o
	rm -f $*.[co]
.s~.a:
	$(GET) $(GFLAGS) -p $< > $*.s
	$(AS) $(ASFLAGS) -o $*.o $*.s
	$(AR) $(ARFLAGS) $@ $*.o
	-rm -f $*.[so]
.f.a:
	$(FC) -c $(FFLAGS) $<
	$(AR) $(ARFLAGS) $@ $*.o
	rm -f $*.o
.f~.a:
	$(GET) $(GFLAGS) -p $< > $*.f
	$(FC) -c $(FFLAGS) $*.f
	$(AR) $(ARFLAGS) $@ $*.o
	rm -f $*.[fo]
.C.a:
	$(CCC) -c $(CCFLAGS) $<
	$(AR) $(ARFLAGS) $@ $*.o
	rm -f $*.o
.C~.a:
	$(GET) $(GFLAGS) -p $< > $*.C
	$(CCC) -c $(CCFLAGS) $*.C
	$(AR) $(ARFLAGS) $@ $*.o
	rm -f $*.[Co]
.h~.h:
	$(GET) $(GFLAGS) -p $< > $*.h
