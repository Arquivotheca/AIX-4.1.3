# @(#)67	1.7  src/bos/usr/bin/uucp/Makefile, cmduucp, bos411, 9428A410j 2/11/94 15:41:25
# COMPONENT_NAME: CMDUUCP
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

SUBDIRS		= uucpadm uucron

PROGRAMS	= ct cu uucp uux uucpd uuxqt uucico uuname uupoll uustat \
		  uuclean uucleanup uusched uucheck uudecode uuencode uuq \
		  uusend
SCRIPTS		= Maxuuscheds Maxuuxqts
MSGHDRS		= uucp_msg.h uusend_msg.h uuencode_msg.h uudecode_msg.h
CATFILES	= uucp.cat uusend.cat uuencode.cat uudecode.cat
HFILES		= ${MSGHDRS}

DATAFILES	= Devices Dialcodes Dialers Nutry Permissions Poll Sysfiles \
		  Systems Uutry remote.unknown uudemon.admin uudemon.cleanu \
		  uudemon.hour uudemon.poll uukick uulog uupick uuto uutry \
		  Dialers.samples

ILIST		= ${PROGRAMS} ${DATAFILES} ${SCRIPTS}
IDIR		= /usr/bin/

uucpd_IDIR	= /usr/sbin/
uuxqt_IDIR	= /usr/sbin/uucp/
uucheck_IDIR	= /usr/sbin/uucp/
uucico_IDIR	= /usr/sbin/uucp/
uuclean_IDIR	= /usr/sbin/uucp/
uucleanup_IDIR	= /usr/sbin/uucp/
uusched_IDIR	= /usr/sbin/uucp/
Devices_IDIR	= /etc/uucp/
Dialcodes_IDIR	= /etc/uucp/
Dialers_IDIR	= /etc/uucp/
Dialers.samples_IDIR	= /etc/uucp/
Maxuuscheds_IDIR = /etc/uucp/
Maxuuxqts_IDIR	= /etc/uucp/
Nutry_IDIR	= /usr/sbin/uucp/
Permissions_IDIR = /etc/uucp/
Poll_IDIR	= /etc/uucp/
Systems_IDIR	= /etc/uucp/
Sysfiles_IDIR	= /etc/uucp/
Uutry_IDIR	= /usr/sbin/uucp/
remote.unknown_IDIR	= /usr/sbin/uucp/
uudemon.admin_IDIR	= /usr/sbin/uucp/
uudemon.cleanu_IDIR	= /usr/sbin/uucp/
uudemon.hour_IDIR	= /usr/sbin/uucp/
uudemon.poll_IDIR	= /usr/sbin/uucp/
uukick_IDIR		= /usr/sbin/uucp/
uutry_IDIR		= /usr/sbin/uucp/

scallers.o_CFLAGS	= -DSTANDALONE
sconn.o_CFLAGS		= -DSTANDALONE
ct.o_CFLAGS		= -DSTANDALONE
cu.o_CFLAGS		= -DSTANDALONE
culine.o_CFLAGS		= -DSTANDALONE
suucpdefd.o_CFLAGS	= -DSTANDALONE
uucpd.o_CFLAGS		= -DBSD -D_BSD

ct_OFILES 	= scallers.o sconn.o ct.o getargs.o line.o uucpdefs.o \
		  interface.o sysfiles.o strsave.o ulockf.o

cu_OFILES	= altconn.o scallers.o sconn.o cu.o getargs.o culine.o \
		  uucpdefs.o interface.o sysfiles.o strsave.o ulockf.o \
		  utility.o systat.o

uucp_OFILES	= chremdir.o cpmv.o expfile.o gename.o getargs.o getpwinfo.o \
		  gnamef.o gtcfile.o gwd.o logent.o permission.o systat.o \
		  ulockf.o utility.o uucp.o uucpdefs.o uucpname.o versys.o \
		  xqt.o sysfiles.o strsave.o mailst.o

uux_OFILES	= chremdir.o cpmv.o expfile.o gename.o getargs.o getprm.o \
		  getpwinfo.o gnamef.o gtcfile.o gwd.o logent.o permission.o \
		  systat.o ulockf.o utility.o uucpdefs.o uucpname.o uux.o \
		  versys.o xqt.o sysfiles.o strsave.o

uuxqt_OFILES	= chremdir.o cpmv.o expfile.o gename.o getargs.o getprm.o \
		  getpwinfo.o gnamef.o gtcfile.o logent.o mailst.o \
		  permission.o shio.o systat.o ulockf.o utility.o uucpdefs.o \
		  uucpname.o uuxqt.o versys.o xqt.o

uucico_OFILES	= anlwrk.o callers.o chremdir.o cico.o cntrl.o conn.o \
		  cpmv.o eio.o expfile.o gename.o getargs.o getpwinfo.o \
		  gio.o gnamef.o gnxseq.o gtcfile.o imsg.o line.o logent.o \
		  mailst.o pk0.o pk1.o pkdefs.o permission.o sys5uux.o \
		  systat.o tio.o ulockf.o utility.o uucpdefs.o uucpname.o \
		  versys.o xio.o xqt.o interface.o sysfiles.o strsave.o

uuname_OFILES	= getpwinfo.o uucpdefs.o uucpname.o uuname.o sysfiles.o \
		  strsave.o

uustat_OFILES	= expfile.o getargs.o getpwinfo.o gnamef.o ulockf.o \
		  utility.o uucpdefs.o uucpname.o uustat.o versys.o \
		  sysfiles.o strsave.o

uuclean_OFILES	= cpmv.o expfile.o mailst.o getargs.o getpwinfo.o \
		  gnamef.o ulockf.o utility.o uuclean.o uucpdefs.o \
		  uucpname.o permission.o

uucleanup_OFILES = cpmv.o expfile.o getargs.o getpwinfo.o gnamef.o \
		   ulockf.o utility.o uucleanup.o uucpdefs.o \
		   uucpname.o

uusched_OFILES	= expfile.o getargs.o getpwinfo.o gnamef.o systat.o \
		  ulockf.o utility.o uucpdefs.o uusched.o

uupoll_OFILES	= cpmv.o expfile.o getargs.o getpwinfo.o gnamef.o logent.o \
		  permission.o strsave.o sysfiles.o systat.o utility.o \
		  uucpdefs.o uucpname.o uupoll.o versys.o 

uucheck_OFILES	= uucheck.o uucpname.o

uuq_OFILES	= expfile.o uuq.o permission.o getargs.o getpwinfo.o \
		  gnamef.o logent.o subdir.o systat.o utility.o \
		  uucpdefs.o uucpname.o

uusend_OFILES	= uusend.o cpmv.o expfile.o gename.o getargs.o \
		  getprm.o getpwinfo.o gnamef.o gnxseq.o logent.o mailst.o \
		  systat.o ulockf.o uucpname.o versys.o xqt.o uucpdefs.o

uucpd_LIBS	= -lsrc

.include <${RULES_MK}>

scallers.c:	callers.c
	${RM} -f scallers.c
	${CP} ${callers.c:P} ${.TARGET}

sconn.c:	conn.c
	${RM} -f sconn.c
	${CP} ${conn.c:P} ${.TARGET}

