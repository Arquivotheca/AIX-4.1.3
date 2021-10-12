# @(#)68        1.5  src/bos/usr/include/X11/Makefile, libim, bos411, 9428A410j 4/1/93 13:55:18
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 10,27
#
# IBM CONFIDENTIAL -- (IBM Confidential	Restricted when
# combined with	the aggregated modules for this	product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT	International Business Machines	Corp. 1988, 1989
# All Rights Reserved
#
# US Government	Users Restricted Rights	- Use, duplication or
# disclosure restricted	by GSA ADP Schedule Contract with IBM Corp.
#

EXPDIR	= /usr/include/X11/

DATAFILES	= \
	keysym.h \
	aix_keysym.h \
	aix_ks_names.h \
	ks_names.h \
	keysymdef.h \
	X.h \
	Xlib.h \
	Xutil.h \
	Xosdefs.h \
	Xfuncproto.h

INCLUDES	= ${DATAFILES}
ILIST		= ${DATAFILES}
IDIR		= /usr/include/X11/

.include <${RULES_MK}>

ks_names.h: keysymdef.h
	${AWK} 'BEGIN {							\
	       printf("/*\n * This is a machine generated file.");	\
	       printf("	* Do not put under source control. \n */\n");  \
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
	}' < ${aix_keysym.h:P}	| ${GREP} -v "keysym_h" >>	$@

