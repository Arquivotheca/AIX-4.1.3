# @(#)57	1.5  src/bos/usr/ccs/lib/libnetsvc/Makefile, libcnet, bos411, 9428A410j 5/20/94 13:14:23
#
#   COMPONENT_NAME: LIBCNET
#
#   FUNCTIONS: none 
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
BINARIES	= yes

.if defined (THREAD_SAFE)
LIBC		= c_r
LIBBIND		= libbind_r
LIBLOCAL	= liblocal_r
LIBNIS		= libnis_r
.else
LIBC		= c
LIBBIND		= libbind
LIBLOCAL	= liblocal
LIBNIS		= libnis
.endif

OTHERS = ${LIBBIND} ${LIBLOCAL} ${LIBNIS}

ILIST		= ${OTHERS} 
IDIR		= /usr/lib/netsvc/

OFILES = gethostbind.o gethostlocal.o gethostnis.o

.if defined(THREAD_SAFE)
CFLAGS = -D_BSD=44 -D_SUN -D_THREAD_SAFE
INCFLAGS = -I${MAKETOP}/bos/usr/ccs/lib/libc -I${MAKETOP}/bos/usr/ccs/lib/libc_r
VPATH = ${MAKETOP}/bos/usr/ccs/lib/libnetsvc:${MAKETOP}/bos/usr/ccs/lib/libc_r
.else
CFLAGS = -D_BSD=44 -D_SUN 
INCFLAGS = -I${MAKETOP}/bos/usr/ccs/lib/libc
.endif

.include <${RULES_MK}>

${LIBBIND}:		gethostbind.o	
	${LD} ${_LDFLAGS_} -blibpath:/usr/lib:/lib -M -egethostfunctions -l${LIBC} -o ${LIBBIND} gethostbind.o

${LIBLOCAL}:		gethostlocal.o
	${LD} ${_LDFLAGS_} -blibpath:/usr/lib:/lib -M -egethostfunctions -l${LIBC} -o ${LIBLOCAL} gethostlocal.o

${LIBNIS}:		gethostnis.o	
	${LD} ${_LDFLAGS_} -blibpath:/usr/lib:/lib -M -egethostfunctions -l${LIBC} -o ${LIBNIS} gethostnis.o

