# @(#)20	1.5  src/bldenv/rastools/Makefile, bldprocess, bos412, GOLDA411a 1/10/94 13:32:17
#
# COMPONENT_NAME: (BOSBUILD) Tools for building natively on the R2 platform
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1990,1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SCRIPTS		= errtmpltbld retidsbld trcfmtchk getvpd probeidsbld
OTHERS		= dirs2ids probeidsbld.awk

ILIST           = ${SCRIPTS} ${OTHERS}
dirs2ids_IDIR   = /usr/lib/
IDIR            = /usr/bin/

.include <${RULES_MK}>

dirs2ids : dirs2ids.src
	${RM} ${_RMFLAGS_} ${.TARGET}
	${CP} ${dirs2ids.src:P} ${.TARGET}

probeidsbld.awk : probeidsbld.awk.src
	${RM} ${_RMFLAGS_} ${.TARGET}
	${CP} ${probeidsbld.awk.src:P} ${.TARGET}
