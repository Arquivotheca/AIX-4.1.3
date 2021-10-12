# @(#)12        1.6  src/bos/kernel/db/POWER/machdep.mk, Makefiles, bos41J, 9508A 2/3/95 10:29:23
# COMPONENT_NAME: (SYSDB) Kernel Debugger
#
# FUNCTIONS:
#
# ORIGINS: 27
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

PLAT_CFLAGS	= -DR2NATIVE

POWER_DBG_OFILES	= \
	dbtty_dvr.o \
	dbtty_dvr_rs.o \
	dbtty_dvr_sf.o \
	iorwcludge.o \
	dbdisasm.o \
	dbxdisassembly.o \
	dbxops.o \
	search.o \
	dbtrace.o \
	vdbfmts.o \

dbdisasm.o_INCFLAGS = \
		-I${MAKETOP}../src/bos/usr/ccs/lib/libdbx/${TARGET_MACHINE}

dbxdisassembly.o_INCFLAGS = \
		-I${MAKETOP}../src/bos/usr/ccs/lib/libdbx/${TARGET_MACHINE}

dbxops.o_INCFLAGS = \
		-I${MAKETOP}../src/bos/usr/ccs/lib/libdbx/${TARGET_MACHINE}

dbkern.o_INCFLAGS = \
		-I${MAKETOP}bos/kernel/db/${TARGET_MACHINE} \
		-I${MAKETOP}bos/kernel/ios/${TARGET_MACHINE}

dbtrace.o :dbtrace.h
dbtrace.h: trchkid.h dbtrace.desc
	${CAT} ${dbtrace.desc:P} > ${.TARGET}
	${AWK} '/#define[ \t]+HKWD/{ printf "\t %s, \"%s\", \n", $$2,\
		substr($$2, "6")}' ${trchkid.h:P} >> $@
	${ECHO} "};" >> $@
	
