# @(#)87	1.7  src/bos/usr/bin/que/Makefile, cmdque, bos411, 9428A410j 3/16/94 08:39:00
# COMPONENT_NAME:
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

SUBDIRS		= libque bsd att aix rem qadm

PROGRAMS	= pac enq qdaemon qstatus digest
SCRIPTS		= aixv2short aixv2long bsdshort bsdlong attshort attlong \
		  aixshort aixlong qconfig
MSGHDRS		= enq_msg.h pac_msg.h digest_msg.h qstat_msg.h qmain_msg.h
CATFILES	= enq.cat pac.cat digest.cat qstat.cat qmain.cat
HFILES		= ${MSGHDRS}

ILIST		= accrec.h ${PROGRAMS} ${SCRIPTS}
IDIR		= /usr/bin/

NOUPDATE_LIST	= /etc/qconfig

digest_IDIR	= /usr/lib/lpd/
qstatus_IDIR	= /usr/lib/lpd/
qconfig_IDIR	= /etc/
qdaemon_IDIR	= /usr/sbin/
aixv2short_IDIR = /usr/lib/lpd/
aixv2long_IDIR	= /usr/lib/lpd/
bsdshort_IDIR	= /usr/lib/lpd/
bsdlong_IDIR	= /usr/lib/lpd/
attshort_IDIR	= /usr/lib/lpd/
attlong_IDIR	= /usr/lib/lpd/
aixshort_IDIR	= /usr/lib/lpd/
aixlong_IDIR	= /usr/lib/lpd/
pac_IDIR	= /usr/sbin/
accrec.h_IDIR	= /usr/include/sys/


enq_OFILES	= enq.o params.o enqjdf.o ecom.o fullname.o
pac_OFILES	= pac.o ecom.o
qdaemon_OFILES	= qmain.o status.o exec.o start.o qcom.o finish.o
qstatus_OFILES	= qstatus.o qsmisc.o qsdisplay.o qcom.o status.o

LIBFLAGS	= -Llibque

LIBS		= -lque -lqb -ls

.include <${RULES_MK}>
