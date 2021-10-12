# @(#)32 1.12 src/bos/kernext/tty/Makefile, sysxtty, bos41J 3/23/95 11:02:20
#
# COMPONENT_NAME: (sysxtty) Makefile to generate all needed modules
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1, 5 Years Bull Confidential Information
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

VPATH              +=:${TARGET_MACHINE}:${MAKETOP}/bos/kernext/pty:${MAKETOP}/bos/kernext/pse
VPATH              +=:${MAKETOP}/bos/kernext/tty/ttydbg

SUBDIRS = ttydbg

KERNEL_EXT         = tioc ldterm uc_sjis lc_sjis nls sptr ${${TARGET_MACHINE}_KERNEL_EXT}

IDIR    	       = /usr/lib/drivers/
ILIST    	       = ${KERNEL_EXT}

IMPORTS            = -bI:pse.exp sockets.exp -bI:pse_int.exp -bI:dbtty.exp
LIBS               = ${KERNEXT_LIBS}

ldterm_OFILES      = ldtty.o ldtty_compat.o ldtty_euc.o \
                     tty_common.o
ldterm_ENTRYPOINT  = ldtty_config
ldterm_EXPORTS     = -bE:ldterm.exp

tioc_OFILES        = tioc.o
tioc_ENTRYPOINT    = tioc_config

stream_sjis.o_INCFLAGS = -I$(MAKETOP)/bos/usr/include \
	-I$(MAKETOP)/bos/usr/lib/nls/loc/iconv/fcconv

uc_sjis_OFILES     = uc_sjis.o stream_sjis.o
uc_sjis_ENTRYPOINT = uc_sjis_config

lc_sjis_OFILES     = lc_sjis.o stream_sjis.o
lc_sjis_ENTRYPOINT = lc_sjis_config

nls_OFILES         =  snls.o snls_db.o
#nls_IMPORTS        = ${IMPORTS} -bI:ttydbg.exp
nls_ENTRYPOINT     = str_nls_config

sptr_OFILES        = sptr.o sptr_db.o
#sptr_IMPORTS       = ${IMPORTS} -bI:ttydbg.exp
sptr_ENTRYPOINT    = sptr_config

.include <${TARGET_MACHINE}/machdep.mk>

.include <${RULES_MK}>
