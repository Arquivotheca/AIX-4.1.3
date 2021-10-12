# @(#)04        1.22  src/bos/usr/lib/pios/Makefile, cmdpios, bos411, 9438C411e 9/25/94 11:43:48
# COMPONENT_NAME: CMDPIOS
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
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
# LEVEL 1, 5 Years Bull Confidential Information
#

SUBDIRS		= msg fmtrs predef trans1 trans2 bidi

VPATH		= ${MAKETOP}/bos/usr/lib/pios/fmtrs:${MAKETOP}/bos/usr/bin/smit:${MAKETOP}/bos/usr/bin/que:${MAKETOP}/bos/usr/bin/que/qadm

PROGRAMS	= pioformat piodigest splp piobe pioout pioburst \
		  mkvirprt lsvirprt rmvirprt piopredef chvirprt \
		  piofquote piomsg piocnvt pioattred piolsvp pioevattr \
		  piomgpdev piomkapqd piomkpq piolspt piochdfq piohpnpf \
		  piocfapi piodmgrsu

SCRIPTS		= H.ascii H.gl H.ps T.ascii T.gl T.ps bullH.ps bullT.ps \
		  pioupdate piocustp bull.header bull.trailer bull.head bull.trail \
		  piofontin pioqms100 piochpq piomisc_base piomisc_ext \
		  piolpx piorlfb piodmgr pioinit piojetd piomkjetd \
		  bullps.header bullps.trailer

DATAFILES	= F.4201.850 codepage.txt mbcs.tbl \
		  printers.inv local.attach remote.attach xstation.attach \
		  ascii.attach file.attach hpJetDirect.attach

ODMFILES	= sm_menu_opt sm_name_hdr sm_cmd_hdr sm_cmd_opt \
		  sm_menu_opt.vc sm_name_hdr.vc sm_cmd_hdr.vc sm_cmd_opt.vc

MSGHDRS		= piobe_msg.h piosplp_msg.h qcadm_msg.h
CATFILES	= pioattr1.cat piobe.cat piosplp.cat
HFILES		= ${MSGHDRS} piostruct.h pioformat.h pioattred.h piolimits.h \
		  virprt.h pioout.h smit_class.h qcadm.h common.h frontend.h

INCLUDES	= piostruct.h

ILIST		= piostruct.h ${PROGRAMS} ${DATAFILES} ${SCRIPTS} ${CATFILES} \
		  ${ODMFILES}
 
IDIR		= /usr/lib/lpd/pio/etc/

piobe_IDIR	= /usr/lib/lpd/
piostruct.h_IDIR = /usr/include/
F.4201.850_IDIR	= /usr/lib/lpd/pio/fonts/
H.ascii_IDIR	= /usr/lib/lpd/pio/burst/
H.gl_IDIR	= /usr/lib/lpd/pio/burst/
H.ps_IDIR	= /usr/lib/lpd/pio/burst/
T.ascii_IDIR	= /usr/lib/lpd/pio/burst/
T.gl_IDIR	= /usr/lib/lpd/pio/burst/
T.ps_IDIR	= /usr/lib/lpd/pio/burst/
bullH.ps_IDIR	= /usr/lib/lpd/pio/burst/
bullT.ps_IDIR	= /usr/lib/lpd/pio/burst/
splp_IDIR	= /usr/bin/
mkvirprt_IDIR	= /usr/sbin/
lsvirprt_IDIR	= /usr/sbin/
rmvirprt_IDIR	= /usr/sbin/
chvirprt_IDIR	= /usr/sbin/
piopredef_IDIR	= /usr/sbin/
piocnvt_IDIR	= /usr/sbin/
pioattred_IDIR	= /usr/sbin/
piofontin_IDIR	= /usr/sbin/
piomisc_base_IDIR	= /usr/sbin/
sm_cmd_opt_IDIR		= /var/spool/lpd/pio/@local/smit/
sm_cmd_opt.vc_IDIR	= /var/spool/lpd/pio/@local/smit/
sm_menu_opt_IDIR	= /var/spool/lpd/pio/@local/smit/
sm_menu_opt.vc_IDIR	= /var/spool/lpd/pio/@local/smit/
sm_name_hdr_IDIR	= /var/spool/lpd/pio/@local/smit/
sm_name_hdr.vc_IDIR	= /var/spool/lpd/pio/@local/smit/
sm_cmd_hdr_IDIR		= /var/spool/lpd/pio/@local/smit/
sm_cmd_hdr.vc_IDIR	= /var/spool/lpd/pio/@local/smit/


YFLAGS		= -d

pioformat_IMPORTS	= -bI:pioformat.imps
pioformat_EXPORTS	= -bE:piof.imps

pioout_IMPORTS		= -bI:pioout.imps
pioout_EXPORTS		= -bE:readprtr.imps

piocfapi_IMPORTS	= -bI:piocfapi.imps
piocfapi_EXPORTS	= -bE:piocfapi.exps

pioformat_OFILES	= pioformat.o piogetvals.o pioparm.o piosubr1.o \
			  piosubr2.o piosecure.o piogetopt.o piomsgipc.o
splp_OFILES		= piosplp.o
mkvirprt_OFILES		= mkvirprt.o virprt.o
lsvirprt_OFILES		= lsvirprt.o virprt.o
rmvirprt_OFILES		= rmvirprt.o virprt.o smit_class.o
chvirprt_OFILES		= chvirprt.o virprt.o
piopredef_OFILES	= piopredef.o virprt.o
piobe_OFILES		= piobe.o piosubr3.o piosubr1.o piosubr2.o \
			  piogetvals.o piogetopt.o pioparm.o piomsgipc.o
pioburst_OFILES		= pioburst.o piosubr1.o piomsgipc.o
pioattred_OFILES	= pioattred.o pioattred_yy.o pioattred_lex.o
piodigest_OFILES	= piodigest.o piorebld.o smit_class.o pioparm.o \
			  piosubr1.o piosubr2.o piosubr3.o pioattred_yy.o \
			  pioattred_lex.o piomsgipc.o
pioevattr_OFILES	= pioevattr.o piogetvals.o pioparm.o piosubr1.o \
			  piosubr2.o piomsgipc.o
piolsvp_OFILES		= piolsvp.o qccom.o
piomkapqd_OFILES	= piomkapqd.o smit_class.o qccom.o
piohpnpf_OFILES		= piohpnpf.o network.o fifo.o log.o status.o
pioout_OFILES		= pioout.o piomsgipc.o
piocfapi_OFILES		= piocfapi.o piogetvals.o pioparm.o piosubr1.o \
			  piosubr2.o piomsgipc.o

CFLAGS			+= ${_AIX3CURSESFLAGS}
LIBFLAGS		+= ${_AIX3CURSESLIBFLAG}

smit_class.o_INCFLAGS	= -I${MAKETOP}/bos/usr/bin/smit
piorebld.o_INCFLAGS	= -I${MAKETOP}/bos/usr/bin/smit
piolsvp.o_INCFLAGS	= -I${MAKETOP}/bos/usr/bin/que \
			  -I${MAKETOP}/bos/usr/bin/que/qadm
qccom.o_INCFLAGS	= -I${MAKETOP}/bos/usr/bin/que \
			  -I${MAKETOP}/bos/usr/bin/que/qadm
piomkapqd.o_INCFLAGS	= -I${MAKETOP}/bos/usr/bin/smit \
			  -I${MAKETOP}/bos/usr/bin/que \
			  -I${MAKETOP}/bos/usr/bin/que/qadm
rmvirprt.o_INCFLAGS	= -I${MAKETOP}/bos/usr/bin/smit

piorebld.o_CFLAGS	= ${CFLAGS} -D_BSD=44

piocfapi_LIBS		= -lqb -lPW
LIBS			= -lqb -lIN -lcurses -lPW -ly -ll -lodm -lcfg -ls

.include <${RULES_MK}>

.SUFFIXES: .msg .def
.def.msg:
	${CP} ${.IMPSRC} ${.TARGET}

sm_menu_opt sm_name_hdr sm_cmd_hdr sm_cmd_opt sm_menu_opt.vc sm_name_hdr.vc \
sm_cmd_hdr.vc sm_cmd_opt.vc:	${MAKETOP}/bos/objclass/smit_class.cre
	ODMDIR=. ${ODMCREATE} -c ${.ALLSRC}
