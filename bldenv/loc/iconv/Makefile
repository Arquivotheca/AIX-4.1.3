# @(#)13	1.3  src/bldenv/loc/iconv/Makefile, cmdiconv, bos412, GOLDA411a 4/25/94 16:48:29
#
#   COMPONENT_NAME: CMDICONV
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM	CONFIDENTIAL --	(IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#   OBJECT CODE	ONLY SOURCE MATERIALS
#
#   (C)	COPYRIGHT International	Business Machines Corp.	1994
#   All	Rights Reserved
#   US Government Users	Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM	Corp.
#

VPATH		= ${MAKETOP}/bos/usr/lib/nls/loc/iconv/fcconv:${MAKETOP}bos/kernel:${MAKETOP}bos/usr/include

PROGRAMS	= IBM-932_IBM-eucJP IBM-eucJP_IBM-932

HFILES		= iconvP.h iconv932.h includes/stdlib.h includes/sys/localedef.h includes/sys/lc_core.h includes/sys/lc_sys.h
INCFLAGS	= -I./includes

ILIST		= ${PROGRAMS}
IDIR		= ${LOCPATH}/iconv/

CFLAGS		= -DAIXV3AR

LDFLAGS		= -einstantiate

.include <${RULES_MK}>

includes/stdlib.h: ${stdlib.h:P}
	${MAKEPATH} ${.TARGET}
	${RM} -f ${.TARGET}
	${CP} ${stdlib.h:P} ${.TARGET}

includes/sys/localedef.h: ${sys/localedef.h:P}
	${MAKEPATH} ${.TARGET}
	${RM} -f ${.TARGET}
	${CP} ${sys/localedef.h:P} ${.TARGET}

includes/sys/lc_core.h: ${sys/lc_core.h:P}
	${MAKEPATH} ${.TARGET}
	${RM} -f ${.TARGET}
	${CP} ${sys/lc_core.h:P} ${.TARGET}

includes/sys/lc_sys.h: ${sys/lc_sys.h:P}
	${MAKEPATH} ${.TARGET}
	${RM} -f ${.TARGET}
	${CP} ${sys/lc_sys.h:P} ${.TARGET}
