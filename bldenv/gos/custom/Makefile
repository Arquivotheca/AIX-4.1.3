# @(#)42        1.3  src/bldenv/gos/custom/Makefile, custom, gos411, 9428A410i 1/19/94 18:56:52
#
#   COMPONENT_NAME: CUSTOM
#
#   FUNCTIONS: Makefile
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

# BUILD NOTES:
#
#   In a production build, these tools should first be built in the
# src/bldenv/gos/custom directory and published to the AFS build
# environment before building the custom component.
#   As a convenience, they can be run from the sandbox build environment
# without having to wait for them to be published to AFS. To do so, set
# the following symbol either on the build command line, or as an exported
# environment variable in the shell where the build command is run:
#	CUSTOMTOOLS=$SOURCEBASE/$OBJECTDIR/bldenv/gos/custom
#   Note that building these tools and run them from the custom source
# directory (src/gos/apps/custom/tools) generally does not work because of
# differences between the libc.a on the build machines and in the product
# being built.
#   See src/gos/apps/custom/custom.mk for more details.


VPATH		=${MAKETOP}gos/apps/custom/tools:${MAKETOP}bos/usr/ccs/lib/libc
 
PROGRAMS 	= merge
SCRIPTS		= msgtoinc
OTHERS 		= lex31.exe ncform31.src

ILIST 		= ${PROGRAMS} ${SCRIPTS} ${OTHERS:R}
IDIR 		= /usr/bin/
ncform31_IDIR	= /usr/ccs/lib/lex/

merge_CFLAGS 	= -D_SIZE=5000
merge_LIBS 	= -lc -lm
merge_OFILES 	= merge.o

.include <${RULES_MK}>
.include <${MAKETOP}gos/gos.mk>
.include <${MAKETOP}gos/apps/custom/custom.mk>

lex31 : lex31.exe
	${CP} ${.ALLSRC} ${.TARGET}
	${CHMOD} +x ${.TARGET}

ncform31 : ncform31.src
	${CP} ${.ALLSRC} ${.TARGET}
	${CHMOD} +x ${.TARGET}
