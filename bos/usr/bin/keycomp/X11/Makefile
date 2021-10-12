# @(#)30	1.1  src/bos/usr/bin/keycomp/X11/Makefile, cmdimkc, bos411, 9428A410j 2/2/94 22:28:00
# COMPONENT_NAME: (cmdimkc) AIX	Input Method Keymap Compiler
#
# FUNCTIONS: Makefile for keycomp
#
# ORIGINS: 27
#
# (C) COPYRIGHT	International Business Machines	Corp. 1988, 1989, 1993
# All Rights Reserved
# Licensed Materials - Property	of IBM
#
# US Government	Users Restricted Rights	- Use, duplication or
# disclosure restricted	by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/usr/include/X11

OTHERS		= ks_names.h aix_ks_names.h

.include <${RULES_MK}>

ks_names.h: keysymdef.h
	${AWK} 'BEGIN {							\
		printf("/*\n * This is a machine generated file.");	 \
		printf(" * Do not put under source control. \n */\n");	\
	};								\
	/^#define/ {							\
		len = length($$2)-3;					\
		printf("{ \"%s\", %s },\n", substr($$2,4,len), $$3);	\
	}' < ${keysymdef.h:P}	>> $@

aix_ks_names.h:	aix_keysym.h
	${AWK} 'BEGIN {							\
		printf("/*\n * This is a machine generated file.");	\
		printf(" * Do not put under source control. \n */\n");	\
	};								\
	/^#define/ {							\
		len = length($$2)-3;					\
		printf("{ \"%s\", %s },\n", substr($$2,4,len), $$3);	\
	}' < ${aix_keysym.h:P}	| ${GREP} -v "keysym_h"	>>	$@

