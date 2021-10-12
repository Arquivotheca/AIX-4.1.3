# @(#)18        1.3  src/bos/diag/util/udsense/Makefile, dsaudsense, bos411, 9435D411a 9/2/94 09:35:10
#
#   COMPONENT_NAME: DSAUDSENSE
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


PROGRAMS        = udsense dsense

SCRIPTS         = labels.dsense rtypes.dsense dsense.lib arrays.dsense
MSGHDRS         = udsense_msg.h
CATFILES        = udsense.cat
HFILES          = ${MSGHDRS}

LIBS            = -lasl -ldiag -lodm -lcfg -lc

ILIST           = ${PROGRAMS} dsense.lib rtypes.dsense labels.dsense \
	                arrays.dsense udsense.cat
IDIR            = /usr/lpp/diagnostics/bin/
udsense.cat_IDIR        = /usr/lpp/diagnostics/catalog/default/
udsense_OFILES  = udsense.o
dsense_OFILES = dsense.o

.include <${RULES_MK}>
