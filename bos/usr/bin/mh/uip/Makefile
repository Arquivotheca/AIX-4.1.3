# @(#)74        1.8  src/bos/usr/bin/mh/uip/Makefile, cmdmh, bos411, 9428A410j 3/4/94 14:03:22
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

PROGRAMS	= ali anno ap burst comp conflict dist dp folder forw \
		  inc install-mh mark mhl mhmail mhpath msgchk msh packf \
		  pick post prompter rcvdist rcvpack rcvstore rcvtty \
		  refile repl rmf rmm scan send show slocal sortm spost \
		  vmh whatnow whom
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

ILIST		= ${PROGRAMS}
IDIR		= /usr/bin/

ap_IDIR		= /usr/lib/mh/
conflict_IDIR	= /usr/lib/mh/
dp_IDIR		= /usr/lib/mh/
install-mh_IDIR	= /usr/lib/mh/
mhl_IDIR	= /usr/lib/mh/
post_IDIR	= /usr/lib/mh/
rcvdist_IDIR	= /usr/lib/mh/
rcvpack_IDIR	= /usr/lib/mh/
rcvstore_IDIR	= /usr/lib/mh/
rcvtty_IDIR	= /usr/lib/mh/
slocal_IDIR	= /usr/lib/mh/
spost_IDIR	= /usr/lib/mh/

CFLAGS		+= -D_BSD -DBSD42 -DMHE -DMHRC -DSENDMTS -DBERK -DX400 \
		${_AIX3CURSESFLAGS}

INCFLAGS	= -I../config -I../mts -I../zotnet -I../h


ali_OFILES	= ali.o aliasbr.o
anno_OFILES	= anno.o annosbr.o
ap_OFILES	= ap.o trmsbr.o bzero.o
comp_OFILES	= comp.o whatnowsbr.o sendsbr.o annosbr.o distsbr.o
conflict_OFILES	= conflict.o aliasbr.o
dist_OFILES	= dist.o whatnowsbr.o sendsbr.o annosbr.o distsbr.o
dp_OFILES	= dp.o trmsbr.o bzero.o
forw_OFILES	= forw.o whatnowsbr.o sendsbr.o annosbr.o distsbr.o bzero.o
inc_OFILES	= inc.o scansbr.o dropsbr.o trmsbr.o bzero.o
mhl_OFILES	= mhl.o trmsbr.o bzero.o mhlsbr.o
msh_OFILES	= msh.o mshcmds.o vmhsbr.o picksbr.o scansbr.o dropsbr.o \
		  mhlsbr.o trmsbr.o bzero.o
packf_OFILES	= packf.o dropsbr.o
pick_OFILES	= pick.o picksbr.o bzero.o
post_OFILES	= post.o discard.o aliasbr.o
rcvdist_OFILES	= rcvdist.o distsbr.o bzero.o
rcvpack_OFILES	= rcvpack.o dropsbr.o
rcvtty_OFILES	= rcvtty.o scansbr.o trmsbr.o ttyw.o bzero.o
repl_OFILES	= repl.o replsbr.o whatnowsbr.o sendsbr.o annosbr.o \
		  distsbr.o bzero.o
scan_OFILES	= scan.o scansbr.o trmsbr.o bzero.o
send_OFILES	= send.o sendsbr.o annosbr.o distsbr.o
show_OFILES	= show.o mhlsbr.o trmsbr.o bzero.o
slocal_OFILES	= slocal.o aliasbr.o dropsbr.o
sortm_OFILES	= sortm.o bzero.o
spost_OFILES	= spost.o aliasbr.o
vmh_OFILES	= vmh.o vmhsbr.o
whatnow_OFILES	= whatnow.o whatnowsbr.o sendsbr.o annosbr.o distsbr.o
whom_OFILES	= whom.o distsbr.o

LIBFLAGS	= -L. -L../mts -L../config -L../sbr -L../zotnet ${_AIX3CURSESLIBFLAG}

LIBS		= -lconf -lmh -lmts -lzot -lvers -lcurses

.include <${RULES_MK}>


version.c:	version.sh
	${CP} ${version.sh:P} .
	${CHMOD} +x version.sh
	version.sh > ${.TARGET}
	${RM} -f version.sh
