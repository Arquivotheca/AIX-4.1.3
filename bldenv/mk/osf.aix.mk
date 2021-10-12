# @(#)06        1.33  src/bldenv/mk/osf.aix.mk, ade_build, bos41J, 9515B_all 4/11/95 17:19:51
#
#   ORIGINS: 27 71
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
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
# OSF/1 1.1

.if !defined(_OSF_AIX_MK_)
_OSF_AIX_MK_=

#
#  Default locale/language information
#

LOCALE?=C
LOCALEPATH=${MAKETOP}usr/lib/nls/loc
MSGLANG?=en_US
MSGLANGPATH?=/usr/lib/nls/msg/${MSGLANG}

#
#  Default location of vista tools
#

VISTAHOME?=/afs/austin.ibm.com/local/bin/veritas.1.4

#
#  Flags to get the linker to strip symbols
#

LDSTRIP=${_ansi_LDFLAGS_} -s

#
#  Shared libraries definitions
#
.if defined(NO_SHARED_LIBRARIES) || defined(USE_STATIC_LIBRARIES)
USE_SHARED_LIBRARIES=0
.else
USE_SHARED_LIBRARIES=1
.endif

#
#  C compiler variations
#

BASE_CC?=${CC}
ANSI_CC?=${XLC}
BASE_THREADED_CC?=${CC_R}
ANSI_THREADED_CC?=${XLC_R}
TRADITIONAL_CC?=${BASE_CC}
WRITABLE_STRINGS_CC?=${BASE_CC} -qnoro
VISTA_CC=${VISTAHOME}/bin/covcc

.if ${project_name} != "dce"
ANSI_LD?=${_GCC_EXEC_PREFIX_}ld
.else
ANSI_LD?=${LD}
.endif
TRADITIONAL_LD?=${ANSI_LD}
ANSI_THREADED_LD?=${ANSI_LD}
BASE_THREADED_LD?=${ANSI_LD}
WRITABLE_STRINGS_LD?=${ANSI_LD}
VISTA_LD=${ANSI_LD}

ANSI_AR?=${_GCC_EXEC_PREFIX_}ar
TRADITIONAL_AR?=${ANSI_AR}
ANSI_THREADED_AR?=${ANSI_AR}
BASE_THREADED_AR?=${ANSI_AR}
WRITABLE_STRINGS_AR?=${ANSI_AR}
VISTA_AR?=${ANSI_AR}

ANSI_RANLIB?=${_GCC_EXEC_PREFIX_}ranlib
TRADITIONAL_RANLIB?=${ANSI_RANLIB}
ANSI_THREADED_RANLIB?=${ANSI_RANLIB}
BASE_THREADED_RANLIB?=${ANSI_RANLIB}
WRITABLE_STRINGS_RANLIB?=${ANSI_RANLIB}
VISTA_RANLIB?=${ANSI_RANLIB}

.if defined(MP)
.if (${MP} == "_mp")
_MACHFLAGS_=-D_POWER_RS -D_POWER_PC -D_POWER_RS1 -D_POWER_RS2 -D_POWER_RSC -D_POWER_601 -D_POWER_603 -D_POWER_604 -D_THREADS -D_POWER_MP -D_PEGASUS
.else
_MACHFLAGS_=-D_POWER_RS -D_POWER_PC -D_POWER_RS1 -D_POWER_RS2 -D_POWER_RSC -D_POWER_601 -D_POWER_603 -D_POWER_604 -D_THREADS
.endif
.endif

.if ${project_name} != "dce"
# Why is -D__STR31__ defined???
_CCDEFS_=-U_IBMR2 -D_${TARGET_MACHINE} -D_AIX -DNLS -D_NLS -DMSG -D__STR31__ -Daiws ${_MACHFLAGS_}
#_CCDEFS_=-U_IBMR2 -D_${TARGET_MACHINE} -D_AIX -DNLS -D_NLS -DMSG -Daiws ${_MACHFLAGS_}
.else
# DCE threads does not support Power PC architecture -- don't want _IBMR2
# undefined.
_CCDEFS_=-D_${TARGET_MACHINE} -D_AIX -DNLS -D_NLS -DMSG -D__STR31__ -Daiws ${_MACHFLAGS_} -D_AIX41
.endif

_O_F_CFLAGS_=${_${_T_M_}_${OBJECT_FORMAT}_CFLAGS_}

_host_NOSTDINC_=
_ansi_NOSTDINC_=
_ansi_threaded_NOSTDINC_=
_base_threaded_NOSTDINC_=
_traditional_NOSTDINC_=
_writable_strings_NOSTDINC_=
_vista_NOSTDINC_=
_CC_NOSTDINC_=${_${_CCTYPE_}_NOSTDINC_}

_host_GENINC_=${x:L:!${GENPATH} -I.!} 
_ansi_GENINC_=${x:L:!${GENPATH} -I.!}
_ansi_threaded_GENINC_=${x:L:!${GENPATH} -I.!}
_base_threaded_GENINC_=${x:L:!${GENPATH} -I.!}
_traditional_GENINC_=${x:L:!${GENPATH} -I.!}
_writable_strings_GENINC_=${x:L:!${GENPATH} -I.!}
_vista_GENINC_=${x:L:!${GENPATH} -I.!}
_CC_GENINC_=${_${_CCTYPE_}_GENINC_}

_host_PICLIB_=
_ansi_PICLIB_=
_ansi_threaded_PICLIB_=
_base_threaded_PICLIB_=
_traditional_PICLIB_=
_writable_strings_PICLIB_=
_vista_PICLIB_=
_CC_PICLIB_=${_${_CCTYPE_}_PICLIB_}

.if ${USE_SHARED_LIBRARIES}
_GLUE_=-%ld," -warn_nopic -glue"
.else
_GLUE_=
.endif
_host_GLUE_=
_ansi_GLUE_=${_GLUE_}
_ansi_threaded_GLUE_=${_GLUE_}
_base_threaded_GLUE_=${_GLUE_}
_traditional_GLUE_=${_GLUE_}
_writable_strings_GLUE_=${_GLUE_}
_vista_GLUE_=${_GLUE_}
_CC_GLUE_=${_${_CCTYPE_}_GLUE_}

_host_GENLIB_=
_ansi_GENLIB_=
_ansi_threaded_GENLIB_=
_base_threaded_GENLIB_=
_traditional_GENLIB_=
_writable_strings_GENLIB_=
_vista_GENLIB_=
_CC_GENLIB_=${_${_CCTYPE_}_GENLIB_}

#
#  Project rules
#

.if defined(MSGHDRS)
${MSGHDRS} : $${.TARGET:_msg.h=.msg}
	@-${RM} ${_RMFLAGS_} ${.TARGET}
	${MKCATDEFS} ${.TARGET:_msg.h=} ${.ALLSRC} >/dev/null
.endif

.if defined(CATFILES)
.if defined(SOURCE_CODESET) && defined(TARGET_CODESET)
${CATFILES} : $${.TARGET:.cat=.msg} ${MAKEFILE_DEPENDENCY}
	@-${RM} ${_RMFLAGS_} ${.TARGET}
	${MKCATDEFS} ${.TARGET:.cat=} ${.ALLSRC} -h | ${ICONV} -f ${SOURCE_CODESET} -t ${TARGET_CODESET} | LC_CTYPE=${MSGLANG} ${GENCAT} ${.TARGET}
.else
${CATFILES} : $${.TARGET:.cat=.msg} ${MAKEFILE_DEPENDENCY}
	@-${RM} ${_RMFLAGS_} ${.TARGET}
	${MKCATDEFS} ${.TARGET:.cat=} ${.ALLSRC} -h | LC_CTYPE=${MSGLANG} ${GENCAT} ${.TARGET}
.endif
.endif

#
#  COMPRESS_FILES rules
#
.if defined(COMPRESS_FILES)
${COMPRESS_FILES:=.Z} : $${COMPRESS_FILES} ${MAKEFILE_DEPENDENCY}
	@${RM} ${_RMFLAGS_} ${.TARGET}
	${CP} ${${.TARGET:.Z=}:P} .
	${COMPRESS} ${.TARGET:.Z=}
.endif

.if ${project_name} == "dce"

#
# The DCE component specific makefiles.
#

.if !empty(MAKESUB:Madmin*)
.include "${MAKETOP}/admin/admin.mk"
.endif

.if !empty(MAKESUB:Mdirectory/cds*)
.include "${MAKETOP}/directory/cds/cds.mk"
.endif

.if !empty(MAKESUB:Mdirectory/gds*)
.include "${MAKETOP}/directory/gds/gds.mk"
.endif

.if !empty(MAKESUB:Mdirectory/xds*)
.include "${MAKETOP}/directory/xds/xds.mk"
.endif

.if !empty(MAKESUB:Mdirectory/xom*)
.include "${MAKETOP}/directory/xom/xom.mk"
.endif

.if !empty(MAKESUB:Mdiskless*)
.if ${TARGET_MACHINE} == "POWER"
CCTYPE?=traditional
.endif
.endif

.if !empty(MAKESUB:Mrpc*)
.include "${MAKETOP}/rpc/rpc.mk"
.endif

.if !empty(MAKESUB:Msecurity*)
.include "${MAKETOP}/security/security.mk"
.endif

.if !empty(MAKESUB:Mtest*)
.include "${MAKETOP}/test/test.mk"
.endif

.if !empty(MAKESUB:Mtest/tet*)
.include "${MAKETOP}/test/tet/tet.mk"
.endif

.if !empty(MAKESUB:Mtime*)
.include "${MAKETOP}/time/time.mk"
.endif

.if !empty(MAKESUB:Mthreads*)
.include "${MAKETOP}/threads/threads.mk"
.else
# The _AIX32_THREADS variable means the threads calls are written
# for POSIX 1003.4a Draft 4.  The threads directory is excluded
# since it contains the interface between Draft 4 and Draft 7, the
# Makefiles within threads will set _AIX32_THREADS when needed.
_CCDEFS_ += -D_AIX32_THREADS=1
.endif

.if ${TARGET_MACHINE}=="PMAX" || ${TARGET_MACHINE}=="HP800"
.if !empty(MAKESUB:Mfile*)
.include "${MAKETOP}/file/file.mk"
.endif
.endif

#
#  dfs kernel definitions
#  For setting "-G 18" and "-nostdinc" options.
#
_O_F_CFLAGS_=${_${_T_M_}_${OBJECT_FORMAT}_CFLAGS_}

TARGET_FLAGS ?= -D_ALL_SOURCE -DAIX32
# D_AES_SOURCE : AES compliant system -- determines how sys/timers.h will be
#                included.
_CCDEFS_ += -D_AES_SOURCE -DAIX_PROD -DOT -qMAXMEM=5000 -M -DDCE_RPC_SVC

#
# End of the DCE component specific makefiles.
#

.endif

.endif
