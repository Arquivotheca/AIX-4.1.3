# @(#)12	1.11  src/bos/usr/ccs/lib/libs2/Makefile, libs, bos41J, 9516B_all 4/20/95 15:09:30
#
#   COMPONENT_NAME: MAKEFILES
#
#   FUNCTIONS: none
#
#   ORIGINS: 27,71
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1988,1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/usr/ccs/lib/libs
MSGHDRS		= libs_msg.h
CATFILES	= libs.cat
HFILES		= ${MSGHDRS}
IDIR		= /usr/lib/
YFLAGS	    	= -d
INCFLAGS	= -I${MAKETOP}/bos/usr/ccs/lib/libs
EXPLIB_TARGETS	= export_libs.a export_libs.a.min
ILIST		= libs.a libs.a.min
BINARIES	= libs.a libs.a.min
LIBS_MIN	= liby.a libl.a libc.a
LIBS_FULL	= libnetls.a libnck.a ${LIBS_MIN}
EXPORTS_MIN	= libs.exp.min
IMPORTS_MIN	=
EXPORTS_FULL	= libs.exp
IMPORTS_FULL	=
OTHER_LIBRARIES	= libs.a libs.a.min

#
# Two libraries are built by this Makefile:
#	libs.a       (FULL)    non-stripped
#	libs.a.min   (MIN)       stripped
#
# libs.a.min is used during boot install time, thus, it does not
# contain login related code (or anything else that we were able
# to carve out).  init uses getuserattr(), getgroupattr() and auditing.
# cp and chmod use ACLS.
#
SHARED_MFILES=	\
	acl_chg.o acl_get.o acl_put.o acl_set.o auditwrite.o	\
	privilege.o tcb.o auditpack.o auditread.o	\
	getconfattr.o getgroupattr.o	\
	getuserattr.o getuserpw.o gettcbattr.o \
	commonattr.o AFsec.o	\
	libsmsg.o libs_open.o libs_read.o libs_util.o		\
	libs_write.o authgram.g.o authgram.lx.o util.o mb.o	\
	pwdbm.o pw_hist.o pw_hist2.o

SHARED_FFILES= ${SHARED_MFILES} \
	setpenv.o getpenv.o setpcred.o getpcred.o checktime.o \
	ckuserID.o newpass.o ckuseracct.o authenticate.o chpass.o \
	getportattr.o pw_rest.o loginlog.o loginrestrictions.o \
	userlicense.o userlicense2.o


.include <${RULES_MK}>

#
# See file src/bldenv/mk/osf.aix.lib.mk and libc/Makefile
#

F__LIBS__	=${LIBS_FULL:!${FINDFILE} ${LIBDIRS} ${LIBS_FULL}!}
F__LIBI__	=${LIBIMPORTS:!${FINDFILE} ${LIBDIRS} ${LIBIMPORTS}!}
F__LIB_S__	=${LIBS_FULL:S/^lib/-l/g:S/.a$//g}

F_EXPORTS_	=${${.TARGET}_EXPORTS:U${EXPORTS_FULL}}
F__EXPORTS__	=${F_EXPORTS_:S/-bE://g}

F_IMPORTS_	=${${.TARGET}_IMPORTS:U${IMPORTS_FULL}}
F__IMPORTS__	=${F_IMPORTS_:S/-bI://g}


M__LIBS__	=${LIBS_MIN:!${FINDFILE} ${LIBDIRS} ${LIBS_MIN}!}
M__LIBI__	=${LIBIMPORTS:!${FINDFILE} ${LIBDIRS} ${LIBIMPORTS}!}
M__LIB_S__	=${LIBS_MIN:S/^lib/-l/g:S/.a$//g}

M_EXPORTS_	=${${.TARGET}_EXPORTS:U${EXPORTS_MIN}}
M__EXPORTS__	=${M_EXPORTS_:S/-bE://g}

M_IMPORTS_	=${${.TARGET}_IMPORTS:U${IMPORTS_MIN}}
M__IMPORTS__	=${M_IMPORTS_:S/-bI://g}

STRIP_FLAG	=

_LIBS_SHLDFLAGS	= -bh:8 -bloadmap:${.PREFIX}.out -bR:${.PREFIX}.map ${_COMMON_LDFLAGS_}


libs.a : $${.TARGET}(shr.o)
	${RM} -f ${.TARGET}
	${AR} ${DEF_ARFLAGS} ${.TARGET} shr.o

libs.a.min : ${SHARED_MFILES} $${M__IMPORTS__} $${M__EXPORTS__} ${M__LIBS__} ${M__LIBI__}
	@${RM} -f ${.TARGET} libsm.a min/shr.o min/shr.o.out min/shr.o.map
	@[ -d min ] || ${MKDIR} min
	${LD} -bh:8 -s -bloadmap:min/shr.o.out -bR:min/shr.o.map \
	${_COMMON_LDFLAGS_} -bM:Sre -o min/shr.o ${SHARED_MFILES} \
	${M__LIBI__} \
	${M__IMPORTS__:@TMP@-bI:${${TMP}:P}@} \
	${M__LIB_S__} ${LIBDIRS} -blibpath:/usr/lib:/lib \
	${M__EXPORTS__:@TMP@-bE:${${TMP}:P}@} && ${RM} -f min/shr.o.map
	${AR} ${DEF_ARFLAGS} libsm.a min/shr.o || ( ${RM} -f libsm.a ; false; )
	-${MV} libsm.a libs.a.min


shr.o : ${SHARED_FFILES} $${F__IMPORTS__} $${F__EXPORTS__} ${F__LIBS__} ${F__LIBI__}
	${LD} ${_LIBS_SHLDFLAGS} -bM:Sre -o ${.TARGET} ${SHARED_FFILES} \
	${F__LIBI__} \
	${F__IMPORTS__:@TMP@-bI:${${TMP}:P}@} \
	${F__LIB_S__} ${LIBDIRS} -blibpath:/usr/lib:/lib \
	${F__EXPORTS__:@TMP@-bE:${${TMP}:P}@}

${SHARED_FFILES} : ${HFILES}


libs.exp : libs.exp.min libs.exp.full
	${CAT} ${libs.exp.full:P} ${libs.exp.min:P} > ${.TARGET}

