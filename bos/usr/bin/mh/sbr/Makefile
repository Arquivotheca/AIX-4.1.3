# @(#)70        1.1  src/bos/usr/bin/mh/sbr/Makefile, cmdmh, bos411, 9428A410j 8/5/92 16:52:07
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

LIBRARIES	= libmh.a
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

CFLAGS		= -D_BSD -DBSD42 -DMHE -DMHRC -DSENDMTS -DBERK -DX400
INCFLAGS	= -I../config -I../h -I../zotnet

OFILES		= add.o adios.o admonish.o addrsbr.o advertise.o \
		  advise.o ambigsw.o atooi.o brkstring.o closefds.o \
		  concat.o copy.o copyip.o cpydata.o cpydgst.o discard.o \
		  done.o fdcompare.o gans.o getans.o getanswer.o getcpy.o \
		  help.o fmtcompile.o formataddr.o formatsbr.o m_atoi.o \
		  m_backup.o m_convert.o m_delete.o m_draft.o m_find.o \
		  m_fmsg.o m_foil.o m_getdefs.o m_getfld.o m_getfolder.o \
		  m_gmprot.o m_gmsg.o m_maildir.o m_name.o m_readefs.o \
		  m_remsg.o m_replace.o m_scratch.o m_seq.o m_seqbits.o \
		  m_seqflag.o m_seqnew.o m_setcur.o m_setseq.o m_setvis.o \
		  m_sync.o m_tmpfil.o m_update.o m_whatnow.o makedir.o \
		  path.o peekc.o pidwait.o pidstatus.o printsw.o push.o \
		  putenv.o pwd.o refile.o remdir.o r1bindex.o showfile.o \
	 	  smatch.o sprintb.o ssequal.o strindex.o trimcpy.o uleq.o \
		  uleqx.o uprf.o vfgets.o directory.o x400fldchk.o \
		  parsetime.o

pidwait.o_CFLAGS	= -DBSD
m_sync.o_CFLAGS		= -DBSD_INCLUDES

.include <${RULES_MK}>
