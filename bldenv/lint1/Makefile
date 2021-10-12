# @(#)59        1.1  src/bldenv/lint1/Makefile, ade_build, bos412, GOLDA411a 4/26/94 12:50:39
#
#   COMPONENT_NAME: BOSBUILD
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH			= ${MAKETOP}bos/usr/ccs/bin/lint:${MAKETOP}bos/usr/ccs/bin/common:${MAKETOP}bos/usr/ccs/bin/lint/${TARGET_MACHINE}:${MAKETOP}bos/usr/ccs/lib/libc/${TARGET_MACHINE}

PROGRAMS		= lint1
MSGHDRS                 = ctools_msg.h

ILIST			= lint1 
IDIR			= /usr/lib/

INCFLAGS		= -I${MAKETOP}bos/usr/ccs/bin/common

CFLAGS			= -DNEWCALL -DHOSTIEEE -DSDBDEBUG -DSINGLE_PRECISION \
		          -DTWOPASS -D_COMPILER -DLINT
HFILES			= ${MSGHDRS}

OFILES			= cgram.o code.o comm1.o local.o messages.o optim.o \
			  pftn.o scan.o tftn.o trees.o xdefs.o

.include <${RULES_MK}>

