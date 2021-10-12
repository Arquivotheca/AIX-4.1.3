# @(#)73 1.1 src/bos/diag/util/ukey/Makefile, dsaukey, bos411, 9430C411a 7/22/94 04:45:01

# COMPONENT_NAME: DSAUKEY
#
# FUNCTIONS: Diagnostic Service Aid - UKEY (makefile)
#
# ORIGINS: 83
#
# LEVEL 1, 5 Years Bull Confidential Information
#

PROGRAMS	= ukey

MSGHDRS		= ukey_msg.h
CATFILES	= ukey.cat
HFILES		= ${MSGHDRS}
CFLAGS		+= ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

LIBS		=  -lasl -ldiag -lodm -lcfg -lc -lcur -lcurses

INCFLAGS	= -I${MAKETOP}/bos/usr/include/diag -I${MAKETOP}/bos/usr/include

ILIST		= ${PROGRAMS} ukey.cat
IDIR		= /usr/lpp/diagnostics/bin/
ukey.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/
ukey_OFILES	= ukey.o

.include <${RULES_MK}>

