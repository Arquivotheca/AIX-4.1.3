# @(#)27	1.7  src/bos/kernext/scsi/Makefile, sysxscsi, bos411, 9432A411a 7/30/94 17:19:24
# COMPONENT_NAME: SYSXSCSI
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT	= hscsidd pscsidd pscsiddpin pscsi720dd pscsi720ddpin	\
		  ascsidd ascsiddpin vscsidd vscsiddpin
IDIR		= /usr/lib/drivers/
ILIST		= hscsidd pscsidd pscsiddpin pscsi720dd pscsi720ddpin	\
		  ascsidd ascsiddpin vscsidd vscsiddpin

hscsidd_ENTRYPOINT	= hsc_config
pscsidd_ENTRYPOINT	= psc_config
pscsi720dd_ENTRYPOINT   = p720_config
ascsidd_ENTRYPOINT	= asc_config
vscsidd_ENTRYPOINT	= vsc_config

.ifdef PSC_TRACE
pscsidd_IMPORTS         = -bI:pscsiddpintr.exp
pscsiddpin_EXPORTS      = -bE:pscsiddpintr.exp
.else
pscsidd_IMPORTS         = -bI:pscsiddpin.exp
pscsiddpin_EXPORTS      = -bE:pscsiddpin.exp
.endif

.ifdef P720_TRACE
pscsi720dd_IMPORTS         = -bI:pscsi720ddpintr.exp
pscsi720ddpin_EXPORTS      = -bE:pscsi720ddpintr.exp
.else
pscsi720dd_IMPORTS         = -bI:pscsi720ddpin.exp
pscsi720ddpin_EXPORTS      = -bE:pscsi720ddpin.exp
.endif

ascsidd_IMPORTS         = -bI:ascsiddpin.exp
ascsiddpin_EXPORTS      = -bE:ascsiddpin.exp

vscsidd_IMPORTS         = -bI:vscsiddpin.exp
vscsiddpin_EXPORTS      = -bE:vscsiddpin.exp

LOCALCFLAGS ?= -qmaxmem=-1

LIBS		= ${KERNEXT_LIBS}

hscsidd_OFILES 	= hsctm.o hscintr.o hscmb30intr.o \
		  hscwdog.o hscopncls.o hscinit.o \
		  hscstrat.o hscioctl.o hscmisc.o \
		  hscdump.o

pscsidd_OFILES	= pscsiddt.o wordrev.o

pscsiddpin_OFILES = pscsiddb.o wordrev.o

pscsi720dd_OFILES    = pscsi720ddt.o p720_getfree.o wordrev.o

pscsi720ddpin_OFILES = pscsi720ddb.o p720_getfree.o wordrev.o 

ascsidd_OFILES	= asc_top.o
ascsiddpin_OFILES = asc_bot.o asc_misc.o asc_dump.o \
		  asc_quickrev.o asc_cntlz.o cinval.o

vscsidd_OFILES	= vscsiddt.o
vscsiddpin_OFILES= vscsiddb.o vscgfree.o

STRIP_FLAG ?= -s

LDFLAGS_KERNEXT += ${STRIP_FLAG}

.include <${RULES_MK}>
