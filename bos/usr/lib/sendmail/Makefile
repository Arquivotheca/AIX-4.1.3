# @(#)31        1.4  src/bos/usr/lib/sendmail/Makefile, cmdsend, bos411, 9428A410j 3/18/93 10:29:37
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= sendmail mailstats semutil
MSGHDRS		= sendmail_msg.h mailstat_msg.h semutil_msg.h 
CATFILES	= sendmail.cat mailstat.cat semutil.cat
HFILES		= ${MSGHDRS}

ILIST		= ${PROGRAMS} sendmail.cf sendmail.nl smdemon.cleanu aliases
IDIR		= /usr/sbin/

semutil_IDIR	= /usr/lib/
smdemon.cleanu_IDIR = /usr/lib/
sendmail.cf_IDIR = /etc/
sendmail.nl_IDIR = /etc/
aliases_IDIR	= /etc/

CFLAGS		= -D_BSD -DLOG -DNOCATMACRO -D_CSECURITY -D_SUN

sendmail_OFILES	= alias.o arpadate.o clock.o collect.o conf.o convtime.o \
		  daemon.o domain.o deliver.o envelope.o err.o \
		  headers.o macro.o main.o nls.o nlsbody.o parseaddr.o \
		  queue.o readcf.o recipient.o savemail.o setenv.o srvrsmtp.o \
		  stab.o stats.o sysexits.o trace.o usersmtp.o util.o version.o

LIBS		= -lPW -lsrc -ls -liconv -ldbm 

.include <${RULES_MK}>
