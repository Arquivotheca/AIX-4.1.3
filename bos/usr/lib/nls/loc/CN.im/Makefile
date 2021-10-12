# @(#)92	1.2  src/bos/usr/lib/nls/loc/CN.im/Makefile, ils-zh_CN, bos41J, 9521A_all 5/22/95 17:00:20
#
#   COMPONENT_NAME: ils-zh_CN
#
# FUNCTIONS: Simplified Chinese Input Method
#
#   ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
#   (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS = CN.im

OFILES = \
	cnimprof1.o \
	cnimproce.o \
	cnimproca.o \
	cnimioctl.o \
        cniminit.o \
        cnimclose.o \
        cnimcreat.o \
        cnimdisp.o \
        cnimprof.o \
        cnimdest.o \
        cnedinit.o \
        cnedrc.o \
        cnedproc.o \
        cnedctrl.o \
        cnedclear.o \
        cnedclose.o \
        cnedmisc.o \
        cnedpyle.o \
        cnedacc.o \
        cneden.o \
        cnedabcacc.o \
        cnedabc.o \
        cnedabcwp.o \
        cnedabcbkm.o \
        cnedabcfio.o \
        cnedud.o \
	cnedabcupdt.o \
	cnedfsacc.o \
	cnedfss.o \
	cnedfssacc.o \
	cnedfs.o

HFILES = \
	abc.h \
	chinese.h \
	cjk.h \
	cned.h \
	cnedacc.h \
	cnedinit.h \
	cnedud.h \
	cnimprof1.h \
	data.h \
	dictionary.h \
	extern.h \
        slbl.h \
	cnimprof1.h

LDFLAGS = -eCNIMInitialize 
LIBS = -lIM

ILIST = $(PROGRAMS)
IDIR = /usr/lib/nls/loc/

.include <${RULES_MK}>
