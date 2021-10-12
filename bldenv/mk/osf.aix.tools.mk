# @(#)12       1.40  src/bldenv/mk/osf.aix.tools.mk, ade_build, bos41J 5/4/95 12:33:35
#
# COMPONENT_NAME: BLDPROCESS
#
# FUNCTIONS:
#
# ORIGINS: 27, 71
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Copyright (c) 1991
# Open Software Foundation, Inc.
# 
# Permission is hereby granted to use, copy, modify and freely distribute
# the software in this file and its documentation for any purpose without
# fee, provided that the above copyright notice appears in all copies and
# that both the copyright notice and this permission notice appear in
# supporting documentation.  Further, provided that the name of Open
# Software Foundation, Inc. ("OSF") not be used in advertising or
# publicity pertaining to distribution of the software without prior
# written permission from OSF.  OSF makes no representations about the
# suitability of this software for any purpose.  It is provided "as is"
# without express or implied warranty.
#
#
.if !defined(_OSF_TOOLS_MK_)
_OSF_TOOLS_MK_=

COMPIDSTABLE?=${ODE_TOOLS}/usr/lib/compids.table
UIDTABLE?=${ODE_TOOLS}/usr/lib/uidtable
ADDVPD?=${ODE_TOOLS}/usr/bin/addvpd
ADECOPYRIGHT?=${ODE_TOOLS}/usr/bin/adecopyright -f ${COPYRIGHT_MAP} -t ${COMPIDSTABLE}
ADEINVENTORY?=${ODE_TOOLS}/usr/bin/adeinv
ADELPPNAME?=${ODE_TOOLS}/usr/bin/adelppname
ADEPACKAGE?=${ODE_TOOLS}/usr/bin/adepackage
ADEPRODID?=${ODE_TOOLS}/usr/bin/adeprodid -t ${COMPIDSTABLE}
AR?=${ODE_TOOLS}/usr/bin/ar
AS?=${ODE_TOOLS}/usr/bin/as
AWK?=${ODE_TOOLS}/usr/bin/awk
BACKUP?=/usr/sbin/backbyname
BASENAME?=${ODE_TOOLS}/usr/bin/basename
BDFTOPCF?=${ODE_TOOLS}/usr/bin/bdftopcf
BDFTOSNF?=${ODE_TOOLS}/usr/bin/bdftosnf
BLDCYCLEREMOVE?=${ODE_TOOLS}/usr/bld/bldcycleRemove
BLDDATESORT?=${ODE_TOOLS}/usr/bld/blddatesort
BLDGLOBALPATH?=${ODE_TOOLS}/usr/bld/bldglobalpath
BLDHISTORYPATH?=${ODE_TOOLS}/usr/bld/bldhistorypath
BLDHOSTSFILE?=${ODE_TOOLS}/usr/bld/bldhostsfile
BLDINITFUNC?=${ODE_TOOLS}/usr/bld/bldinitfunc
BLDKSHCONST?=${ODE_TOOLS}/usr/bld/bldkshconst
BLDLOCK?=${ODE_TOOLS}/usr/bld/bldlock
BLDLOG?=${ODE_TOOLS}/usr/bld/bldlog
BLDLOGINIT?=${ODE_TOOLS}/usr/bld/bldloginit
BLDLOGPATH?=${ODE_TOOLS}/usr/bld/bldlogpath
BLDLOGSET?=${ODE_TOOLS}/usr/bld/bldlogset
BLDNODENAMES?=${ODE_TOOLS}/usr/bld/bldnodemames
BLDPERLCONST?=${ODE_TOOLS}/usr/bld/bldperlconst
BLDPERLFUNC?=${ODE_TOOLS}/usr/bld/bldperlfunc
BLDPERLLOG?=${ODE_TOOLS}/usr/bld/bldperllog
BLDQUERYMERGE?=${ODE_TOOLS}/usr/bld/bldquerymerge
BLDRELEASEPATH?=${ODE_TOOLS}/usr/bld/bldreleasepath
BLDREQLIST?=${ODE_TOOLS}/usr/bld/bldreqlist
BLDSETSTATUS?=${ODE_TOOLS}/usr/bld/bldsetstatus
BLDSTATUS?=${ODE_TOOLS}/usr/bld/bldstatus
BLDSTATUSFUNC?=${ODE_TOOLS}/usr/bld/bldstatusfunc
BLDTMPPATH?=${ODE_TOOLS}/usr/bld/bldtmppath
BLDUPDATEPATH?=${ODE_TOOLS}/usr/bld/bldupdatepath
BLDVERIFYREQS?=${ODE_TOOLS}/usr/bld/bldverifyreqs
BOOTEXPAND?=${ODE_TOOLS}/usr/sbin/bootexpand
BRAND?=${ODE_TOOLS}/usr/bin/brand
BRANDDICT?=${BLDENV_TOOLS:U${ODE_TOOLS}}/usr/lib/brandcfg.nls
CANCELPTF?=${ODE_TOOLS}/usr/bld/CancelPtf
CAT?=${ODE_TOOLS}/usr/bin/cat
CHANGEVRMF?=${ODE_TOOLS}/usr/bin/changevrmf
CHECKSTATUS?=${ODE_TOOLS}/usr/bld/CheckStatus
CHECKSYMPTOM?=${ODE_TOOLS}/usr/bld/CheckSymptom
CHMOD?=${ODE_TOOLS}/usr/bin/chmod
CMP?=${ODE_TOOLS}/usr/bin/cmp
COMPRESS?=${ODE_TOOLS}/usr/bin/compress
COPYRIGHT_MAP=${ODE_TOOLS}/usr/lib/copyright.map
CP?=${ODE_TOOLS}/usr/bin/cp
CPIO?=${ODE_TOOLS}/usr/bin/cpio
CPP?=${ODE_TOOLS}/usr/bin/cpp
CUT?=${ODE_TOOLS}/usr/bin/cut
DATE?=${ODE_TOOLS}/usr/bin/date
DD?=${ODE_TOOLS}/usr/bin/dd
DELETESTATUS?=${ODE_TOOLS}/usr/bld/DeleteStatus
DIRS2IDS?=${ODE_TOOLS}/usr/lib/dirs2ids
DSPCAT?=${ODE_TOOLS}/usr/bin/dspcat
DSPMSG?=${ODE_TOOLS}/usr/bin/dspmsg
DTAWKHPTAGCOMMENTS?=${ODE_TOOLS}/opt/dt/etc/AwkHPTagComments
DTBUILD?=${ODE_TOOLS}/opt/dt/etc/build
DTBUILD1?=${ODE_TOOLS}/opt/dt/etc/build1
DTBUILDC1?=${ODE_TOOLS}/opt/dt/etc/buildc1
DTBUILD2?=${ODE_TOOLS}/opt/dt/etc/build2
DTCONTEXT?=${ODE_TOOLS}/opt/dt/etc/context
DTCONTEXT1?=${ODE_TOOLS}/opt/dt/etc/context1
DTCONTEXT2?=${ODE_TOOLS}/opt/dt/etc/context2
DTCONTEXTC1?=${ODE_TOOLS}/opt/dt/etc/contextc1
DTCTAG1?=${ODE_TOOLS}/opt/dt/etc/ctag1
DTELTDEF?=${ODE_TOOLS}/opt/dt/etc/eltdef
DTELTDEF1?=${ODE_TOOLS}/opt/dt/etc/eltdef1
DTELTDEF2?=${ODE_TOOLS}/opt/dt/etc/eltdef2
DTELTDEFC1?=${ODE_TOOLS}/opt/dt/etc/eltdefc1
DTHELPTAG?=${ODE_TOOLS}/opt/dt/etc/helptag
DTHTAG1?=${ODE_TOOLS}/opt/dt/etc/htag1
DTHTAG2?=${ODE_TOOLS}/opt/dt/etc/htag2
DTIMAGEUTIL?=${ODE_TOOLS}/opt/dt/etc/imageutil
DTMERGE?=${ODE_TOOLS}/opt/dt/etc/merge
DTPROCESSMODULES?=${ODE_TOOLS}/opt/dt/etc/processModules
DT2MAN?=PATH=${ODE_TOOLS}/usr/bin ${ODE_TOOLS}/cde/opt/dt/etc/dt2man
CDEAWKHPTAGCOMMENTS?=${ODE_TOOLS}/cde/opt/dt/etc/AwkHPTagComments
CDEBUILD?=${ODE_TOOLS}/cde/opt/dt/etc/build
CDEBUILD1?=${ODE_TOOLS}/cde/opt/dt/etc/build1
CDEBUILDC1?=${ODE_TOOLS}/cde/opt/dt/etc/buildc1
CDEBUILD2?=${ODE_TOOLS}/cde/opt/dt/etc/build2
CDECONTEXT?=${ODE_TOOLS}/cde/opt/dt/etc/context
CDECONTEXT1?=${ODE_TOOLS}/cde/opt/dt/etc/context1
CDECONTEXT2?=${ODE_TOOLS}/cde/opt/dt/etc/context2
CDECONTEXTC1?=${ODE_TOOLS}/cde/opt/dt/etc/contextc1
CDECTAG1?=${ODE_TOOLS}/cde/opt/dt/etc/ctag1
CDEELTDEF?=${ODE_TOOLS}/cde/opt/dt/etc/eltdef
CDEELTDEF1?=${ODE_TOOLS}/cde/opt/dt/etc/eltdef1
CDEELTDEF2?=${ODE_TOOLS}/cde/opt/dt/etc/eltdef2
CDEELTDEFC1?=${ODE_TOOLS}/cde/opt/dt/etc/eltdefc1
CDEHELPTAG?=DTLCXSEARCHPATH=${EXPORTBASE}/cde/usr/dt/config/svc HELPTAGPASS1=${ODE_TOOLS}/cde/opt/dt/etc/dthelp_htag1 HELPTAGPASS2=${ODE_TOOLS}/cde/opt/dt/etc/dthelp_htag2 ${ODE_TOOLS}/cde/opt/dt/etc/dthelptag
CDEHTAG1?=${ODE_TOOLS}/cde/opt/dt/etc/dthelp_htag1
CDEHTAG2?=${ODE_TOOLS}/cde/opt/dt/etc/dthelp_htag2
CDEIMAGEUTIL?=${ODE_TOOLS}/cde/opt/dt/etc/imageutil
CDEMERGE?=LOCPATH=${LOCPATH} ${ODE_TOOLS}/cde/opt/dt/etc/merge
CDEPROCESSMODULES?=${ODE_TOOLS}/cde/opt/dt/etc/processModules
DU?=${ODE_TOOLS}/usr/bin/du
DUMP?=${ODE_TOOLS}/usr/bin/dump
E789CDEF?=${ODE_TOOLS}/usr/lib/hcon/e789cdef
E789KDEF?=${ODE_TOOLS}/usr/lib/hcon/e789kdef
ECHO?=${ODE_TOOLS}/usr/bin/echo
ED?=${ODE_TOOLS}/usr/bin/ed
EGREP?=${ODE_TOOLS}/usr/bin/egrep
ERRINST?=${ODE_TOOLS}/usr/bin/errinstall
ERRPREFIX?=${ODE_TOOLS}/usr/bin/errprefix
ERRTMPLTBLD?=${ODE_TOOLS}/usr/bin/errtmpltbld
ERRUPDATE?=${ODE_TOOLS}/usr/bin/errupdate
EXPR?=${ODE_TOOLS}/usr/bin/expr
FALSE?=${ODE_TOOLS}/bin/false
FC?=${ODE_TOOLS}/usr/bin/fc
FGREP?=${ODE_TOOLS}/usr/bin/fgrep
FINDFILE?=${ODE_TOOLS}/usr/bin/findfile
FINDSHIPFILE?=${ODE_TOOLS}/usr/bin/findshipfile
GENCAT?=LOCPATH=${LOCPATH} ${ODE_TOOLS}/usr/bin/gencat
GENPATH?=${ODE_TOOLS}/usr/bin/genpath
GENXLT?=${ODE_TOOLS}/usr/bin/genxlt
GETOPT?=${ODE_TOOLS}/usr/bin/getopt
GETVPD?=${ODE_TOOLS}/usr/bin/getvpd
GREP?=${ODE_TOOLS}/usr/bin/grep
_HOSTNAME?=${ODE_TOOLS}/usr/bin/hostname
ICONV?=LOCPATH=${ODE_TOOLS}/usr/lib/nls/41loc LC_ALL=C ${ODE_TOOLS}/usr/bin/iconv
KEYCOMP?=${ODE_TOOLS}/usr/bin/keycomp
KEYS?=${ODE_TOOLS}/usr/bin/keys
.if defined(LD_BINDER)
_LD_PATHS_=
_LD_PATHS_=-bbinder:${LD_BINDER}
.else
LD_BINDER?=${ODE_TOOLS}/usr/lib/bind
.endif
.if defined(LD_GLINK)
_LD_PATHS_+=-bglink:${LD_GLINK}
.else
LD_GLINK?=${ODE_TOOLS}/usr/lib/glink.o
.endif
LD?=${ODE_TOOLS}/usr/bin/ld -b"binder:${LD_BINDER} glink:${LD_GLINK}" ${ZLIBDIRS}
LEXER?=${ODE_TOOLS}/usr/ccs/lib/lex/ncform
LEX?=LEXER=${LEXER} ${ODE_TOOLS}/usr/bin/lex
LINT?=${ODE_TOOLS}/usr/bin/lint
LINT1?=${ODE_TOOLS}/usr/lib/lint1
LOCAL?=${ODE_TOOLS}/usr/bin/local
LOCDEF?=${ODE_TOOLS}/usr/bin/locdef
LS?=${ODE_TOOLS}/bin/ls
MAKEMSGS?=${ODE_TOOLS}/usr/bin/makemsgs
MAKEPATH?=${ODE_TOOLS}/usr/bin/makepath
MAKEKEYS?=${ODE_TOOLS}/usr/bin/makekeys
M4?=${ODE_TOOLS}/usr/bin/m4
MD?=${ODE_TOOLS}/usr/bin/md
MKBOOT?=${ODE_TOOLS}/usr/sbin/mkboot
MKCATDEFS?=${ODE_TOOLS}/usr/bin/mkcatdefs
MKDIR?=${ODE_TOOLS}/usr/bin/mkdir
MKFONTDIR?=${ODE_TOOLS}/usr/lpp/X11/bin/mkfontdir
MKFS?=${ODE_TOOLS}/usr/sbin/mkfs
MKMBOOT?=${ODE_TOOLS}/usr/lib/boot/mkmboot
MKODMUPDT?=${ODE_TOOLS}/usr/bin/mkodmupdt
MKRAM?=${ODE_TOOLS}/usr/sbin/mkram
MRI2SF?=${ODE_TOOLS}/usr/bin/mri2sf
MV?=${ODE_TOOLS}/usr/bin/mv
NAMEDB2H?=${ODE_TOOLS}/usr/bin/namedb2h
ODMADD?=${ODE_TOOLS}/usr/bin/odmadd
ODMCHANGE?=${ODE_TOOLS}/usr/bin/odmchange
ODMCREATE?=${ODE_TOOLS}/usr/bin/odmcreate
ODMGET?=${ODE_TOOLS}/usr/bin/odmget
PASTE?=${ODE_TOOLS}/usr/bin/paste
POSTBUILD?=${ODE_TOOLS}/usr/bld/postbuild
POSTPACKAGE?=${ODE_TOOLS}/usr/bld/postpackage
PREBUILD?=${ODE_TOOLS}/usr/bld/prebuild
PROBEIDSBLD?=${ODE_TOOLS}/usr/bin/probeidsbld
PSWRAP?=${ODE_TOOLS}/usr/lpp/DPS/bin/pswrap
PWD?=${ODE_TOOLS}/usr/bin/pwd
PWD_SHELL?=pwd
QUERYSTATUS?=${ODE_TOOLS}/usr/bld/QueryStatus
REP?=${ODE_TOOLS}/usr/lpp/X11/bin/rep
REPGL?=${ODE_TOOLS}/usr/lpp/X11/bin/repgl
RETIDSBLD?=${ODE_TOOLS}/usr/bin/retidsbld
RGB?=${ODE_TOOLS}/usr/bin/rgb
RM?=${ODE_TOOLS}/usr/bin/rm
RPCGEN?=${ODE_TOOLS}/usr/bin/rpcgen
SCAT?=${ODE_TOOLS}/usr/bin/scat
SED?=${ODE_TOOLS}/usr/bin/sed
SETSTATUS?=${ODE_TOOLS}/usr/bld/SetStatus
SORT?=${ODE_TOOLS}/usr/bin/sort
STRIP?=${ODE_TOOLS}/usr/bin/strip
SYM_LOOKUP?=${ODE_TOOLS}/usr/bld/sym_lookup
TAGS?=${ODE_TOOLS}/usr/bin/ctags
TAIL?=${ODE_TOOLS}/usr/bin/tail
TAR?=${ODE_TOOLS}/usr/bin/tar
TDIGEST?=${ODE_TOOLS}/usr/bin/tdigest
TIC?=${ODE_TOOLS}/usr/bin/tic
TOSF?=${ODE_TOOLS}/usr/bin/tosf
TOUCH?=${ODE_TOOLS}/usr/bin/touch
TR?=${ODE_TOOLS}/usr/bin/tr
TRACKSYNC?=${ODE_TOOLS}/usr/bld/tracksync
TRUE?=${ODE_TOOLS}/bin/true
UCONVDEF?=${ODE_TOOLS}/usr/bin/uconvdef
WHAT?=${ODE_TOOLS}/usr/bin/what
WHAT_FILESET?=${ODE_TOOLS}/usr/bin/what_fileset
XARGS?=${ODE_TOOLS}/usr/bin/xargs
XCPP?=${ODE_TOOLS}/usr/lpp/X11/Xamples/util/cpp/cpp
XDT2CDEBYACC=${ODE_TOOLS}/xdt2cde/bin/byacc
XDT2CDEFLEX=${ODE_TOOLS}/xdt2cde/bin/flex
YACCPAR?=${ODE_TOOLS}/usr/lib/yaccpar
YACC?=YACCPAR=${YACCPAR} ${ODE_TOOLS}/usr/bin/yacc

#
# Output lists for Selective Fix
#
LIBDEPLOG=${BASE}/selfix/PTF/${TD_BLDCYCLE}/${LPP_RELEASE}/libdeplist
UPDATELOG=${BASE}/selfix/PTF/${TD_BLDCYCLE}/${LPP_RELEASE}/lmupdatelist
BLDENVLOG=${BASE}/selfix/PTF/${TD_BLDCYCLE}/${LPP_RELEASE}/lmbldenvlist

RUN850?=${ODE_TOOLS}/usr/lib/lpd/pio/trans1/run850
RUNIBM_437?=${ODE_TOOLS}/usr/lib/lpd/pio/trans1/runIBM-437
RUNIBM_850?=${ODE_TOOLS}/usr/lib/lpd/pio/trans1/runIBM-850
RUNIBM_856?=${ODE_TOOLS}/usr/lib/lpd/pio/trans1/runIBM-856
RUNIBM_1046?=${ODE_TOOLS}/usr/lib/lpd/pio/trans1/runIBM-1046
RUNISO8859_1?=${ODE_TOOLS}/usr/lib/lpd/pio/trans1/runISO8859-1
RUNISO8859_2?=${ODE_TOOLS}/usr/lib/lpd/pio/trans1/runISO8859-2
RUNISO8859_3?=${ODE_TOOLS}/usr/lib/lpd/pio/trans1/runISO8859-3
RUNISO8859_4?=${ODE_TOOLS}/usr/lib/lpd/pio/trans1/runISO8859-4
RUNISO8859_5?=${ODE_TOOLS}/usr/lib/lpd/pio/trans1/runISO8859-5
RUNISO8859_6?=${ODE_TOOLS}/usr/lib/lpd/pio/trans1/runISO8859-6
RUNISO8859_7?=${ODE_TOOLS}/usr/lib/lpd/pio/trans1/runISO8859-7
RUNISO8859_8?=${ODE_TOOLS}/usr/lib/lpd/pio/trans1/runISO8859-8
RUNISO8859_9?=${ODE_TOOLS}/usr/lib/lpd/pio/trans1/runISO8859-9

R420X_850?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/r420x.850
R420X_437?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/r420x.437
RDP2000?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/rdp2000
RD630?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/rd630
RHPLJ_ECMA?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/rhplj.ecma
RHPLJ_DN?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/rhplj.dn
RHPLJ_IBMUS?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/rhplj.ibmus
RHPLJ_ROMAN8?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/rhplj.roman8
R4216_ROMAN8?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/r4216.roman8
RIBM_437?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/ribm.437
RIBM_850?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/ribm.850
RIBM_851?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/ribm.851
RIBM_852?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/ribm.852
RIBM_853?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/ribm.853
RIBM_855?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/ribm.855
RIBM_857?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/ribm.857
RIBM_860?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/ribm.860
RIBM_862?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/ribm.862
RIBM_863?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/ribm.863
RIBM_864?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/ribm.864
RIBM_865?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/ribm.865
RIBM_869?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/ribm.869
RIBM_1046?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/ribm.1046
R3812_P0?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/r3812.p0
R3812_P1?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/r3812.p1
R3812_P2?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/r3812.p2
R3816_P1?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/r3816.p1
R3816_P2?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/r3816.p2
RASCII?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/rascii
RP9012?=${ODE_TOOLS}/usr/lib/lpd/pio/trans2/rp9012
RUNUCS_2?=${ODE_TOOLS}/usr/lib/lpd/pio/trans1/runUCS-2
RUNUTF_8?=${ODE_TOOLS}/usr/lib/lpd/pio/trans1/runUTF-8

S83_932?=${ODE_TOOLS}/usr/lib/lpd/pio/transJP/s83_932
S90_932?=${ODE_TOOLS}/usr/lib/lpd/pio/transJP/s90_932
E78_EUCJP?=${ODE_TOOLS}/usr/lib/lpd/pio/transJP/e78_eucJP
E83_EUCJP?=${ODE_TOOLS}/usr/lib/lpd/pio/transJP/e83_eucJP
J78_FOLD7?=${ODE_TOOLS}/usr/lib/lpd/pio/transJP/j78_fold7
J83_FOLD7?=${ODE_TOOLS}/usr/lib/lpd/pio/transJP/j83_fold7
J90_FOLD7?=${ODE_TOOLS}/usr/lib/lpd/pio/transJP/j90_fold7

.if ${project_name} == "dce"

#
# The DCE build tree is a sandbox between the current sandbox and AIX
# backing tree.  To find the DCE tools a search must be made of each
# sandbox from the current sandbox back to the AIX backing tree.
#

TOOLDIRS?=${BACKED_SOURCEDIR:S;:; ;g:S;^;-I;g:S;src$;ode_tools/${target_machine};g}
DEFTOOLBASE?=${BASE}/ode_tools/${target_machine}/usr/bin
ASDEF?=${__ASDEF__:!${FINDFILE} ${INCDIRS} sys/asdef.s!}
COMPILE_ET?=${__COMPILE_ET__:!${FINDFILE} ${TOOLDIRS} usr/bin/compile_et!}
IDL?=IDL_GEN_AUX_FILES=1 ${__IDL__:!${FINDFILE} ${TOOLDIRS} usr/bin/idl!}
KERNEX=${__KERNEX__:!${ECHO} /usr/lib/kernex.exp | ${FINDSHIPFILE}!}
MAVROS?=${__MAVROS__:!${FINDFILE} ${TOOLDIRS} usr/bin/mavros!}
MAVCOD?=${__MAVCOD__:!${FINDFILE} ${TOOLDIRS} usr/bin/mavcod!}
MSG?=${__MSG__:!${FINDFILE} ${TOOLDIRS} usr/bin/msg!}
PARSER_AID?=${__PARSER_AID__:!${FINDFILE} ${TOOLDIRS} usr/bin/parser_aid!}
PRS?=${__PRS__:!${FINDFILE} ${TOOLDIRS} usr/bin/prs!}
SAMS?=GENCAT="${GENCAT}" ${__SAMS__:!${FINDFILE} ${TOOLDIRS} usr/bin/sams!}
SYSCALLS=${__SYSCALLS__:!${ECHO} /usr/lib/syscalls.exp | ${FINDSHIPFILE}!}
ZIC?=${__ZIC__:!${FINDFILE} ${TOOLDIRS} usr/bin/zic!}
.endif

.endif
