# @(#)12	1.7  src/bos/usr/lib/netls/adk/lib/Makefile, libnetls, bos411, 9428A410j 5/26/94 17:57:52
#
#   COMPONENT_NAME: libnetls
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

EXPLIB_TARGETS	= export_libnetls.a \
                  export_libncf.a \
		  export_libnetls_shr.a \
		  export_libnetls_st.a

EXPDIR		= /usr/lib/

IDIR		= /usr/lib/netls/adk/lib/
ILIST		= libnetls_shr.a

.include <${RULES_MK}>

libnetls.a:	libnetls.aB
	${CP} ${libnetls.aB:P} ${.TARGET}

libncf.a:	libncf.aB
	${CP} ${libncf.aB:P} ${.TARGET}

libnetls_shr.a:	libnetls_shr.aB
	${CP} ${libnetls_shr.aB:P} ${.TARGET}

libnetls_st.a:	libnetls_st.aB
	${CP} ${libnetls_st.aB:P} ${.TARGET}
