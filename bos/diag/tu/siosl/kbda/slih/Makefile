# @(#)16        1.6  src/bos/diag/tu/siosl/kbda/slih/Makefile, tu_siosl, bos411, 9428A410j 1/27/94 11:26:50
#
#   COMPONENT_NAME: TU_SIOSL
#
#   FUNCTIONS:
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


VPATH   =${MAKETOP}/bos/kernel/exp:${MAKETOP}/bos/kernel/exp/${TARGET_MACHINE}:${MAKETOP}/bos/kernext/exp

INCFLAGS        = -I${MAKETOP}/bos/diag/tu/siosl

PROGRAMS	= kbd_slih

kbd_slih_DEPENDS = kbd_slih.exp
kbd_slih: ${${.TARGET}_DEPENDS}

LIBS            = ${KERNEXT_LIBS}

STRIP_FLAG      = -s

kbd_slih_ENTRYPOINT	= kbd_interrupt_handler
kbd_slih_IMPORTS = -bI:diagex.exp 
kbd_slih_EXPORTS = -bE:kbd_slih.exp

ILIST           = ${PROGRAMS}
IDIR            = /usr/lpp/diagnostics/slih/

.include <${RULES_MK}>

