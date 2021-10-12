#
# COMPONENT_NAME:  ils-zh_CN
#
# FUNCTIONS: Simplified Chinese Input Method
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS = ZH.im

OFILES = \
        zhedabc.o       \
        zhedabcacc.o    \
        zhedabcbkm.o    \
        zhedabcfio.o    \
        zhedabcupdt.o   \
        zhedabcwp.o     \
        zhedacc.o       \
        zhedclear.o     \
        zhedclose.o     \
        zhedctrl.o      \
        zheden.o        \
        zhedinit.o      \
        zhedmisc.o      \
        zhedproc.o      \
        zhedpyle.o      \
        zhedtj.o        \
        zhedud.o        \
        zhimclose.o     \
        zhimcreat.o     \
        zhimdest.o      \
        zhimdisp.o      \
        zhiminit.o      \
        zhimioctl.o     \
        zhimproca.o     \
        zhimproce.o     \
        zhimprof.o      \
        zhimprof1.o

HFILES = \
	abc.h		\
	abcpy.h		\
	chinese.h	\
	cjk.h		\
	data.h		\
	dictionary.h	\
	extern.h	\
	zhed.h		\
	zhedacc.h	\
	zhedinit.h	\
	zhedud.h	\
	zhimprof1.h

LDFLAGS = -eZHIMInitialize

LIBS = -lIM

ILIST = $(PROGRAMS)
IDIR = /usr/lib/nls/loc/

.include <${RULES_MK}>
