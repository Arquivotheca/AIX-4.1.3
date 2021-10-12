# @(#)11	1.127  src/bldenv/mk/osf.aix.std.mk, ade_build, bos41J 9/13/94 12:12:36
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
# (C) COPYRIGHT International Business Machines Corp. 1991, 1994
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

.if !defined(_OSF_STD_MK_)
_OSF_STD_MK_=

#
#  Default rule - All other rules appear after variable definitions
#
.MAIN: build_all

#
#  Debugging entry for checking environment
#  Probably no need for command to come from build environment.
print_env:
	@/usr/bin/printenv

#
#  Use this as a dependency for any rule which should always be triggered
#  (e.g. recursive makes).
#
ALWAYS=ALWAYS

#
#  Shortened for macro definitions, not to be used within a Makefile.
#
_T_M_=${TARGET_MACHINE}

#
#  Definitions for object file format - A_OUT, COFF or MACHO
#
${_T_M_}_OBJECT_FORMAT?=MACHO
OBJECT_FORMAT?=${${_T_M_}_OBJECT_FORMAT}

#
#  Definitions for archive file format - ASCARCH or COFF
#
${_T_M_}_ARCHIVE_FORMAT?=COFF
ARCHIVE_FORMAT?=${${_T_M_}_ARCHIVE_FORMAT}

#
#  Set defaults for input variables which are not already defined
#

PROF_LIBS=libprof.a

DEF_RMFLAGS?=-f
DEF_ARFLAGS?=crl
DEF_MDFLAGS?=-rm -all

COMPILER_PATH=${BLDENV_TOOLS:U${ODE_TOOLS}}
XLC_CFG?=xlC.cfg
XLF_CFG?=xlf.cfg
C_PP_CFG?=xlC.cfg

#
#  Program macros
#
_AIX3CURFLAGS?=-D_AIX32_CUR
_AIX3CURSESFLAGS?=-D_AIX32_CURSES
# The following are now defined in Makeconf
#_AIX3CURLIBFLAG?=-L${EXPORTBASE}/usr/lib
#_AIX3CURSESLIBFLAG?=-L${EXPORTBASE}/usr/aix3/lib

#
# Define the compilers.
#

CC?=   ${COMPILER_PATH}/usr/bin/cc    -F${COMPILER_PATH}/etc/${XLC_CFG} ${_LD_PATHS_}
CC_R?= ${COMPILER_PATH}/usr/bin/cc_r  -F${COMPILER_PATH}/etc/${XLC_CFG} ${_LD_PATHS_}
XLC?=  ${COMPILER_PATH}/usr/bin/xlc   -F${COMPILER_PATH}/etc/${XLC_CFG} ${_LD_PATHS_}
XLC_R?=${COMPILER_PATH}/usr/bin/xlc_r -F${COMPILER_PATH}/etc/${XLC_CFG} ${_LD_PATHS_}
C++?=LANG=En_US LIBPATH=${C_PP_LIBPATH} NLSPATH=${COMPILER_PATH}/usr/lib/nls/msg/%L/%N:${NLSPATH} ${COMPILER_PATH}/usr/bin/xlC -F${COMPILER_PATH}/etc/${C_PP_CFG} ${_LD_PATHS_}
XLF?=LIBPATH=${COMPILER_PATH}/usr/lib ${COMPILER_PATH}/usr/bin/xlf -F${COMPILER_PATH}/etc/${XLF_CFG} ${_LD_PATHS_}

#
# Include all non-compiler tool symbols.
#
.include <osf.${project_name}.tools.mk>

#
#  Define ${_T_M_}_VA_ARGV to be either VA_ARGV_IS_RECAST
#  to recast to char **, otherwise define VA_ARGV_IS_ROUTINE
#  If not defined here, we become VA_ARGV_UNKNOWN which should invoke
#  a #error directive where needed.
#
HP_M68K_VA_ARGV=VA_ARGV_IS_RECAST
HP300_VA_ARGV=VA_ARGV_IS_RECAST
IBMRT_VA_ARGV=VA_ARGV_IS_RECAST
MACII_VA_ARGV=VA_ARGV_IS_RECAST
MMAX_VA_ARGV=VA_ARGV_IS_RECAST
PMAX_VA_ARGV=VA_ARGV_IS_RECAST
SUN3_VA_ARGV=VA_ARGV_IS_RECAST
SUN4_VA_ARGV=VA_ARGV_IS_RECAST
SUN_VA_ARGV=VA_ARGV_IS_RECAST
VAX_VA_ARGV=VA_ARGV_IS_RECAST
AT386_VA_ARGV=VA_ARGV_IS_RECAST
RIOS_VA_ARGV=VA_ARGV_IS_RECAST
POWER_VA_ARGV=VA_ARGV_IS_RECAST
${_T_M_}_VA_ARGV?=VA_ARGV_UNKNOWN

#
#  Defined whether characters are sign or zero extended
#
HP_M68K_CHAR_EXTEND=CHARS_EXTEND_ZERO
HP300_CHAR_EXTEND=CHARS_EXTEND_SIGN
IBMRT_CHAR_EXTEND=CHARS_EXTEND_ZERO
MACII_CHAR_EXTEND=CHARS_EXTEND_SIGN
MMAX_CHAR_EXTEND=CHARS_EXTEND_ZERO
PMAX_CHAR_EXTEND=CHARS_EXTEND_ZERO
SUN3_CHAR_EXTEND=CHARS_EXTEND_SIGN
SUN4_CHAR_EXTEND=CHARS_EXTEND_SIGN
SUN_CHAR_EXTEND=CHARS_EXTEND_SIGN
VAX_CHAR_EXTEND=CHARS_EXTEND_SIGN
AT386_CHAR_EXTEND=CHARS_EXTEND_SIGN
RIOS_CHAR_EXTEND=CHARS_EXTEND_ZERO
POWER_CHAR_EXTEND=CHARS_EXTEND_ZERO
${_T_M_}_CHAR_EXTEND?=CHARS_EXTEND_UNKNOWN

#
#  Include project specific information
#
.include <osf.${project_name}.mk>

#
#  C compiler variations
#
CCTYPE?=traditional
_CCTYPE_=${${.TARGET}_CCTYPE:U${CCTYPE}}

_host_CC_=${HOST_CC:U${CC}}
_ansi_CC_=${ANSI_CC:U${XLC}}
_traditional_CC_=${TRADITIONAL_CC:U${CC}}
_base_threaded_CC_=${BASE_THREADED_CC:U${CC_R}}
_ansi_threaded_CC_=${ANSI_THREADED_CC:U${XLC_R}}
_writable_strings_CC_=${WRITABLE_STRINGS_CC:U${CC}}
_vista_CC_=${VISTA_CC:U${CC}}
_CC_=${_${_CCTYPE_}_CC_}
_XLF_=${XLF}

_host_LD_=${HOST_LD:U${LD}}
_ansi_LD_=${ANSI_LD:U${LD}}
_traditional_LD_=${TRADITIONAL_LD:U${LD}}
_base_threaded_LD_=${BASE_THREADED_LD:U${LD}}
_ansi_threaded_LD_=${BASE_THREADED_LD:U${LD}}
_writable_strings_LD_=${WRITABLE_STRINGS_LD:U${LD}}
_vista_LD_=${VISTA_LD:U${LD}}
_LD_=${_${_CCTYPE_}_LD_} ${ZLIBDIRS}

_host_AR_=${HOST_AR:U${AR}}
_ansi_AR_=${ANSI_AR:U${AR}}
_traditional_AR_=${TRADITIONAL_AR:U${AR}}
_base_threaded_AR_=${BASE_THREADED_AR:U${AR}}
_ansi_threaded_AR_=${BASE_THREADED_AR:U${AR}}
_writable_strings_AR_=${WRITABLE_STRINGS_AR:U${AR}}
_vista_AR_=${VISTA_AR:U${AR}}
_AR_=${_${_CCTYPE_}_AR_}

_host_RANLIB_=${HOST_RANLIB:U${RANLIB}}
_ansi_RANLIB_=${ANSI_RANLIB:U${RANLIB}}
_traditional_RANLIB_=${TRADITIONAL_RANLIB:U${RANLIB}}
_base_threaded_RANLIB_=${BASE_THREADED_RANLIB:U${RANLIB}}
_ansi_threaded_RANLIB_=${BASE_THREADED_RANLIB:U${RANLIB}}
_writable_strings_RANLIB_=${WRITABLE_STRINGS_RANLIB:U${RANLIB}}
_vista_RANLIB_=${VISTA_RANLIB:U${RANLIB}}
_RANLIB_=${_${_CCTYPE_}_RANLIB_}

_host_CFLAGS_=
_ansi_CFLAGS_=${_O_F_CFLAGS_} ${_CCDEFS_} -M
_traditional_CFLAGS_=${_ansi_CFLAGS_}
_base_threaded_CFLAGS_=${_ansi_CFLAGS_}
_ansi_threaded_CFLAGS_=${_ansi_CFLAGS_}
_writable_strings_CFLAGS_=${_ansi_CFLAGS_}
_vista_CFLAGS_=${_ansi_CFLAGS_}
_CC_CFLAGS_=${_${_CCTYPE_}_CFLAGS_}
_XLF_FLAGS_=-M

_host_INCDIRS_=
_ansi_INCDIRS_=${INCDIRS}
_ansi_threaded_INCDIRS_=${INCDIRS}
_base_threaded_INCDIRS_=${INCDIRS}
_traditional_INCDIRS_=${INCDIRS}
_writable_strings_INCDIRS_=${INCDIRS}
_vista_INCDIRS_=${INCDIRS}
_CC_INCDIRS_=${_${_CCTYPE_}_INCDIRS_}
_XLF_INCDIRS_=${INCDIRS}
_C++INCDIRS_=${C_PP_INCDIRS}

_host_LDFLAGS_=
_ansi_LDFLAGS_= ${STRIP_FLAG}
_traditional_LDFLAGS_= ${STRIP_FLAG}
_writable_strings_LDFLAGS_=${_ansi_LDFLAGS_}
_base_threaded_LDFLAGS_=${_ansi_LDFLAGS_}
_ansi_threaded_LDFLAGS_=${_ansi_LDFLAGS_}
_vista_LDFLAGS_=${_ansi_LDFLAGS_} ${_VISTA_COVMON_}
_CC_LDFLAGS_=${_${_CCTYPE_}_LDFLAGS_}

_host_LIBDIRS_=

.if ${USE_SHARED_LIBRARIES}
_ansi_LIBDIRS_=${SHLIBDIRS} ${LIBDIRS}
.else
_ansi_LIBDIRS_=${LIBDIRS}
.endif

_traditional_LIBDIRS_=${_ansi_LIBDIRS_}
_writable_strings_LIBDIRS_=${_ansi_LIBDIRS_}
_base_threaded_LIBDIRS_=${_ansi_LIBDIRS_}
_ansi_threaded_LIBDIRS_=${_ansi_LIBDIRS_}
_vista_LIBDIRS_=${_ansi_LIBDIRS_}
_CC_LIBDIRS_=${_${_CCTYPE_}_LIBDIRS_}

#
#  Compilation optimization level.  This should be set to whatever
#  combination of -O and -g flags you desire.  Defaults to -O.
#
#  Allow these flags to be overridden per target
#
.if defined(OPT_LEVEL)
_CC_OL_=${${.TARGET}_OPT_LEVEL:U${OPT_LEVEL}}
_XLF_OL_=${${.TARGET}_OPT_LEVEL:U${OPT_LEVEL}}
.else
CC_OPT_LEVEL?=-O
_CC_OL_=${${.TARGET}_OPT_LEVEL:U${${.TARGET}_CC_OPT_LEVEL:U${CC_OPT_LEVEL}}}
XLF_OPT_LEVEL?=-O
_XLF_OL_=${${.TARGET}_OPT_LEVEL:U${${.TARGET}_XLF_OPT_LEVEL:U${XLF_OPT_LEVEL}}}
LD_OPT_LEVEL?=
_LD_OL_=${${.TARGET}_OPT_LEVEL:U${${.TARGET}_LD_OPT_LEVEL:U${LD_OPT_LEVEL}}}
.endif

# See if CC_OPT_LEVEL contains '-g'.
.if defined(CC_OPT_LEVEL)
.if (${CC_OPT_LEVEL:M*-g*} == "-g")
_CC_LDFLAGS_+=-lg
STRIP_FLAG=
.endif
.endif

# See if OPT_LEVEL contains '-g'.
.if defined(OPT_LEVEL)
.if (${OPT_LEVEL:M*-g*} == "-g")
_CC_LDFLAGS_+=-lg
STRIP_FLAG=
.endif
.endif

#
#  Program flags for makefile, environment and command line args
#
_INCFLAGS_=\
	${${.TARGET}_INCARGS:U${INCARGS}}\
	${${.TARGET}_INCFLAGS:U${INCFLAGS}}\
	${${.TARGET}_INCENV:U${INCENV}}
_GENINC_=\
	${_CC_GENINC_} ${_INCFLAGS_:!${GENPATH} ${_INCFLAGS_}!}
_LIBFLAGS_=\
	${${.TARGET}_LIBARGS:U${LIBARGS}}\
	${${.TARGET}_LIBFLAGS:U${LIBFLAGS}}\
	${${.TARGET}_LIBENV:U${LIBENV}}
_GENLIB_=\
	${_CC_GENLIB_} ${_LIBFLAGS_:!${GENPATH} ${_LIBFLAGS_}!}
_LIBS_=	${${.TARGET}_LIBSENV:U${LIBSENV}}\
	${${.TARGET}_LIBS:U${LIBS}}\
	${${.TARGET}_LIBSARGS:U${LIBSARGS}} ${TARGET_LIBS}
_C++LIBS_=${${.TARGET}_C++LIBS:U${C++LIBS}} ${C_PP_TARGET_LIBS}
_CCFLAGS_=\
	${_CC_CFLAGS_}\
	${_CC_OL_}\
	${${.TARGET}_CENV:U${CENV}}\
	${${.TARGET}_CFLAGS:U${CFLAGS}} ${TARGET_FLAGS}\
	${${.TARGET}_CARGS:U${CARGS}}\
	${_CC_NOSTDINC_} ${_GENINC_} ${_CC_INCDIRS_}
_C++FLAGS_ =\
	${_CC_CFLAGS_}\
	${_CC_OL_}\
	${${.TARGET}_C_PP_ENV:U${C_PP_ENV}}\
	${${.TARGET}_C++FLAGS:U${C++FLAGS}} ${TARGET_FLAGS} \
	${${.TARGET}_C_PP_ARGS:U${C_PP_ARGS}}\
	${_CC_NOSTDINC_} ${_GENINC_} ${_C++INCDIRS_} 
_XLFFLAGS_=\
	${_XLF_FFLAGS_}\
	${_XLF_OL_}\
	${${.TARGET}_XLFENV:U${XLFENV}}\
	${${.TARGET}_XLFFLAGS:U${XLFFLAGS}} ${TARGET_FLAGS}\
	${${.TARGET}_XLFARGS:U${XLFARGS}}\
	${_XLF_NOSTDINC_} ${_GENINC_} ${_XLF_INCDIRS_}
_M4FLAGS_?= -B32768 -DINCLML=. -DINCL=.\
	${${.TARGET}_M4ENV:U${M4ENV}}\
	${${.TARGET}_M4FLAGS:U${M4FLAGS}}\
	${${.TARGET}_M4ARGS:U${M4ARGS}}\
	${_GENINC_} ${_CC_INCDIRS_} ${_MACHFLAGS_}
_ASFLAGS_=\
        ${${.TARGET}_ASENV:U${ASENV}}\
        ${${.TARGET}_ASFLAGS:U${ASFLAGS}}\
        ${${.TARGET}_ASARGS:U${ASARGS}}
_C++_LDFLAGS_=${${.TARGET}_C++_LIBDIRS:U${C_PP_LIBDIRS}} ${C_PP_ZLIBDIRS}

.if (${LPP} != "bldenv")
_GETVPD_=-bdbg:ldrinfo:${"":L:!${GETVPD} ${DIRS2IDS} ${.TARGET} ${MAKETOP}!}
.endif

_COMMON_LDFLAGS_=\
	${_CC_LDFLAGS_}\
	${_LD_OL_}\
	${${.TARGET}_LDENV:U${LDENV}}\
	${${.TARGET}_LDFLAGS:U${LDFLAGS}}\
	${${.TARGET}_LDARGS:U${LDARGS}}\
	${_CC_NOSTDLIB_} ${_GENLIB_} ${_CC_LIBDIRS_}\
	${_GETVPD_}

.if defined(LIBRARIES) || defined(SHARED_LIBRARIES)
STRIP_FLAG=
.else
STRIP_FLAG?=-s
.endif

.if defined(FORCE_MAKEFILE_DEPEND)
.if exists(makefile)
MAKEFILE_DEPENDENCY?=makefile
#
# Don't allow makefile to be a source for a rule 
#
.INVISIBLE: ${MAKEFILE_DEPENDENCY}

.else
MAKEFILE_DEPENDENCY?=Makefile
#
# Don't allow Makefile to be a source for a rule
#
.INVISIBLE: ${MAKEFILE_DEPENDENCY}

.endif
.endif

.if defined(OTHERS)
OTHERS  : ${MAKEFILE_DEPENDENCY}
.endif

_LDGLOBAL_= -bh:8 ${STRIP_FLAG} -bloadmap:${.PREFIX}.out -bR:${.PREFIX}.map
_LDFLAGSALL_=${_LDGLOBAL_}
_LDFLAGS_=\
	 ${_COMMON_LDFLAGS_}
_SHLDFLAGS_=${_LDGLOBAL_} ${_COMMON_LDFLAGS_}

_LFLAGS_=\
	${${.TARGET}_LENV:U${LENV}}\
	${${.TARGET}_LFLAGS:U${LFLAGS}}\
	${${.TARGET}_LARGS:U${LARGS}}
_YFLAGS_=\
	${${.TARGET}_YENV:U${YENV}}\
	${${.TARGET}_YFLAGS:U${YFLAGS}}\
	${${.TARGET}_YARGS:U${YARGS}}
_LINTFLAGS_=\
	${${.TARGET}_LINTENV:U${LINTENV}}\
	${${.TARGET}_LINTFLAGS:U${LINTFLAGS}}\
	${${.TARGET}_LINTARGS:U${LINTARGS}}\
	${_GENINC_} ${_CC_INCDIRS_}
_TAGSFLAGS_=\
	${${.TARGET}_TAGSENV:U${TAGSENV}}\
	${${.TARGET}_TAGSFLAGS:U${TAGSFLAGS}}\
	${${.TARGET}_TAGSARGS:U${TAGSARGS}}
_RMFLAGS_=\
	${${.TARGET}_DEF_RMFLAGS:U${DEF_RMFLAGS}}
_MDFLAGS_=\
	${${.TARGET}_DEF_MDFLAGS:U${DEF_MDFLAGS}}\
	${${.TARGET}_MDENV:U${MDENV}}\
	${${.TARGET}_MDFLAGS:U${MDFLAGS}}\
	${${.TARGET}_MDARGS:U${MDARGS}}
VISTAFLAGS=-xCSmht
_VISTAFLAGS_=\
	${${.TARGET}_VISTAENV:U${VISTAENV}}\
	${${.TARGET}_VISTAFLAGS:U${VISTAFLAGS}}\
	${${.TARGET}_VISTAARGS:U${VISTAARGS}}
VISTA_COVMON?=cov1.o
_VISTA_COVMON_=${VISTAHOME}/lib/${${.TARGET}_COVMON:U${VISTA_COVMON}}

#
# If the C++ linker is invoked (by setting PROGRAMS++=foo) then it is possible
# for this to be interpreted as PROGRAMS+ += foo.  For this reason any definition
# of PROGRAMS+ will be appended to PROGRAMS++.
#
.if defined(PROGRAMS+)
PROGRAMS++ +=${PROGRAMS+}
.endif

#
#  Define binary targets
#
.if defined(PROGRAMS)
BINARIES+=${PROGRAMS}
.endif

.if defined(PROGRAMS++)
BINARIES+=${PROGRAMS++}
.endif

.if defined(LIBRARIES)
BINARIES+=${LIBRARIES}
.endif

.if defined(SHARED_LIBRARIES)
BINARIES+=${SHARED_LIBRARIES}
.endif

.if defined(OBJECTS)
BINARIES+=${OBJECTS}
.endif

.if defined(KERNEL)
BINARIES+=${KERNEL}
.endif

.if defined(PPC_OFILES)
BINARIES+=${PPC}
.endif

.if defined(PWR_OFILES)
BINARIES+=${PWR}
.endif

.if defined(PINNED_OFILES)
BINARIES+=${PINNED}
.endif

.if defined(PAGED_OFILES_PRIM)
BINARIES+=${PAGED_PRIM}
.endif

.if defined(PAGED_OFILES)
BINARIES+=${PAGED}
.endif

.if defined(PAGED_OFILES_BACK)
BINARIES+=${PAGED_BACK}
.endif

.if defined(INIT_OFILES)
BINARIES+=${INIT}
.endif

.if defined(DBG_OFILES)
BINARIES+=${DBG}
.endif

.if defined(KERNEL_EXT)
BINARIES+=${KERNEL_EXT}
.endif

.if defined(IMAGE)
BINARIES+=${IMAGE}
.endif

#
#  Definitions for clean/rmtarget/clobber
#
_CLEAN_TARGET=${.TARGET:S/^clean_//}

.if !defined(CLEANFILES)
_CLEAN_DEFAULT_=\
	${_CLEAN_TARGET}.X\
	${OFILES:U${${_CLEAN_TARGET}_OFILES:U${_CLEAN_TARGET}.o}}\
	${${_CLEAN_TARGET}_GARBAGE:U${GARBAGE}}
_CLEANFILES_=\
	${CLEANFILES:U${${_CLEAN_TARGET}_CLEANFILES:U${_CLEAN_DEFAULT_}}}
.endif

_RMTARGET_TARGET=${.TARGET:S/^rmtarget_//}

_CLOBBER_TARGET=${.TARGET:S/^clobber_//}
_CLOBBER_DEFAULT_=\
	${_CLOBBER_TARGET}.X\
	${OFILES:U${${_CLOBBER_TARGET}_OFILES:U${_CLOBBER_TARGET}.o}}\
	${${_CLOBBER_TARGET}_GARBAGE:U${GARBAGE}}
_CLOBBERFILES_=${_CLOBBER_TARGET} \
	${CLEANFILES:U${${_CLOBBER_TARGET}_CLEANFILES:U${_CLOBBER_DEFAULT_}}}

#
#  Definitions for lint
#
_LINT_TARGET=${.TARGET:S/^lint_//}

.if !defined(LINTFILES)
_LINT_OFILES_=${OFILES:U${${_LINT_TARGET}_OFILES:U${SHARED_OFILES}:U${_LINT_TARGET}.o}}
LINTFILES=${${_LINT_TARGET}_LINTFILES:U${_LINT_OFILES_:.o=.c}}
.endif

#
#  Definitions for tags
#
_TAGS_TARGET=${.TARGET:S/^tags_//}

.if !defined(TAGSFILES)
_TAGS_OFILES_=${OFILES:U${${_TAGS_TARGET}_OFILES:U${_TAGS_TARGET}.o}}

#
# Strip the .o extension from each file listed in OFILES.
#
_TAGSFILES_=${${_TAGS_TARGET}_TAGSFILES:U${_TAGS_OFILES_:.o=}}
__DUMMY__=""

#
# Go through the list of OFILES specified to find out which files are .c and
# .f files.  This is done so that make will not bomb out looking for strictly
# .c files (when the .c is really a .f file).  The algorithm is:
#
#   Using .c and .f extensions do
#      Using the next file in the OFILES list do
#          if ${SOURCEDIR}/${MAKESUB}$file.$extension exists
#          then
#              TAGSFILES=${TAGFILES}+"$file.$extension "
#          end
#      done
#   done
#
TAGSFILES?=${__DUMMY__:!\
	for extension in c f;\
	do\
		for file in ${_TAGSFILES_};\
		do\
			[ -f ${SOURCEDIR}/${MAKESUB}\$file.\$extension ] &&\
			${ECHO} "\$file.\$extension \c";\
			:;\
		done;\
	done!}
.endif

#
#  Definitions for export
#
_EXPORT_TARGET=${.TARGET:S/^export_//}
_EXPDIR_=${${_EXPORT_TARGET}_EXPDIR:U${EXPDIR:U${${_EXPORT_TARGET}_IDIR:U${IDIR:U/_MISSING_EXPDIR_/}}}}
_EXPLINKS_=${${_EXPORT_TARGET}_EXPLINKS:U${EXPLINKS}}
_DO_EXPLINKS_=\
	(cd ${EXPORTBASE}${_EXPDIR_:H};\
	 ${RM} ${_RMFLAGS_} ${_EXPLINKS_}\
	 ${_EXPLINKS_:@.LINK.@; ${LN} ${_EXPORT_TARGET} ${.LINK.}@})

.if defined(EXPLINKS)
_MAKE_EXPLINKS_=${_DO_EXPLINKS_}
.else
_MAKE_EXPLINKS_=${${_EXPORT_TARGET}_EXPLINKS:U@true:D${_DO_EXPLINKS_}}
.endif

_EXPFILES_=${EXPORTBASE}${_EXPDIR_}${_EXPORT_TARGET}

#
#  Definitions for install/release
#
#_INSTALL_TARGET=${.TARGET:Minstall_*:S/^install_//}
#_INSTALL_CATFILE_TARGET=${.TARGET:Minstallcat_*:S/^installcat_//}
_INSTALL_TARGET=${.TARGET:T}

.if !empty(.TARGETS:Minstall_*)
.if defined(TOSTAGE)

.if defined(NO_STRIP)
_NO_STRIP_=-nostrip
.else
_NO_STRIP_=${${_INSTALL_TARGET}_NOSTRIP:D-nostrip}
.endif

_IDIR_=${${_INSTALL_TARGET}_IDIR:U${IDIR:U/_MISSING_IDIR_/}}
_ILINKS_=${${_INSTALL_TARGET}_ILINKS:U${ILINKS}}

.endif
.endif

#
#  Default single suffix compilation rules
#
.SUFFIXES:
.SUFFIXES: .N .addN .msgN .msg .oB .o .S .cre .s .pp .hN .cN .c .C .h .y .l .sh .csh .txt .f .shN .SN .frm .fsz .ofm .tsz .trm .map .msz .pl .i

#
#
# tenplus rules
#

.fsz.frm: ${MAKEFILE_DEPENDENCY}
	@${RM} -f ${.TARGET}
	${TOSF} $< INeditor_msg.h

.frm.ofm: ${MAKEFILE_DEPENDENCY}
	@${RM} -f ${.TARGET}
	${FC} $<

.tsz.trm: ${MAKEFILE_DEPENDENCY}
	@${RM} -f ${.TARGET}
	${TOSF} $<

.msz.map: ${MAKEFILE_DEPENDENCY}
	@${RM} -f ${.TARGET}
	${TOSF} $< Hkeys.map_msg.h

#
# end of tenplus rules
#

#  Special rules
#

.pl: ${MAKEFILE_DEPENDENCY}
	@${RM} -f ${.TARGET}
	${CP}  ${.IMPSRC} ${.TARGET}.X
	${CHMOD} +x ${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}

.hN.h: ${MAKEFILE_DEPENDENCY}
	@${RM} -f ${.TARGET}
	${BRAND} ${BRANDDICT} < ${.IMPSRC} > ${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE}; }

.msgN.msg: ${MAKEFILE_DEPENDENCY}
	@${RM} -f ${.TARGET}
	${BRAND} ${BRANDDICT} < ${.IMPSRC} > ${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE}; }

.addN.add: ${MAKEFILE_DEPENDENCY}
	@${RM} -f ${.TARGET}
	${BRAND} ${BRANDDICT} < ${.IMPSRC} > ${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE}; }

.N: ${MAKEFILE_DEPENDENCY}
	@${RM} -f ${.TARGET}
	${BRAND} ${BRANDDICT} < ${.IMPSRC} > ${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE}; }
#
#  Use this as a dependency for any rule which should always be triggered
#  (e.g. recursive makes).
#
${ALWAYS}:

#
#  Include pass definitions for standard targets
#
.include <osf.${project_name}.passes.mk>

#
#  Compilation rules
#
all: build_all;@

build_all: $${_all_targets_};@

comp_all: $${_all_targets_};@

#
#  Clean up rules
#
clean_all: $${_all_targets_}
	-${RM} ${_RMFLAGS_} core

.if !empty(_CLEAN_TARGETS_:Mclean_*)
${_CLEAN_TARGETS_:Mclean_*}:
	-${RM} ${_RMFLAGS_} ${_CLEANFILES_}
.endif

rmtarget_all: $${_all_targets_}
	-${RM} ${_RMFLAGS_} core

.if !empty(_RMTARGET_TARGETS_:Mrmtarget_*)
${_RMTARGET_TARGETS_:Mrmtarget_*}:
	-${RM} ${_RMFLAGS_} ${_RMTARGET_TARGET}
.endif

clobber_all: $${_all_targets_}
	-${RM} ${_RMFLAGS_} core depend.mk

.if !empty(_CLOBBER_TARGETS_:Mclobber_*)
${_CLOBBER_TARGETS_:Mclobber_*}:
	-${RM} ${_RMFLAGS_} ${_CLOBBERFILES_}
.endif

#
#  Lint rules
#
lint_all: $${_all_targets_};@

.if !empty(_LINT_TARGETS_:Mlint_*)
${_LINT_TARGETS_:Mlint_*}: $${LINTFILES}
	${LINT} ${_LINTFLAGS_} ${.ALLSRC} > ${.TARGET:S/^lint_//:=.lint}
.endif

#
#  Tags rules
#
tags_all: $${_all_targets_};@

.if !empty(_TAGS_TARGETS_:Mtags_*)
${_TAGS_TARGETS_:Mtags_*}: $${TAGSFILES}
	${TAGS} ${_TAGSFLAGS_} ${.ALLSRC}
.endif

#
#  Export rules
#
.if !defined(EXPORTBASE)

export_all: ${ALWAYS}
	@${ECHO} "You must define EXPORTBASE to do an ${.TARGET}"

.if !empty(_EXPORT_TARGETS_:Mexport_*)
${_EXPORT_TARGETS_:Mexport_*}: ${ALWAYS}
	@${ECHO} "You must define EXPORTBASE to do an ${.TARGET}"
.endif

.else

export_all: $${_all_targets_}

.if !empty(_EXPORT_TARGETS_:Mexport_*)
${_EXPORT_TARGETS_:Mexport_*}: $${_EXPFILES_};@

${_EXPORT_TARGETS_:Mexport_*:S/^export_//g:@_EXPORT_TARGET@${_EXPFILES_}@}:\
		$${.TARGET:T}
	@-noupdate=`${ECHO} ${NOUPDATE_LIST} | ${XARGS} ${ECHO} | \
	  ${GREP} -w ${_EXPDIR_}${_INSTALL_TARGET}` 2>/dev/null ; \
	if [ "${PTF_UPDATE}" = "yes" -a -z "$$noupdate" ] ; \
	then \
	  ${MAKEPATH} ${BLDENVLOG} ; \
	  ${CMP} -s "${.ALLSRC}" "${.TARGET}" || \
	    ( ${ECHO} "Adding ${.TARGET} \
	        to ${BLDENVLOG}...\c" ; \
	      if ${FGREP} -x ${.TARGET} \
		 ${BLDENVLOG} > /dev/null 2>&1 ; \
	      then	${ECHO} "already there." ; \
	      else	${ECHO} "${.TARGET}"\
			 >> ${BLDENVLOG} ; \
			${ECHO} "done" ; \
	      fi ; \
	    ) ; \
	fi ;
	@-${RM} ${_RMFLAGS_} ${.TARGET} ;
	${MAKEPATH} ${.TARGET} ;
	${CP} -p ${.ALLSRC} ${.TARGET}
	@if [ "${LPP}" = "xlC" -a -n "${.TARGET:M*\.h}" ]; \
	then \
		${SED} -e "s|^\(#.*include.*<\)/|\1|g" \
		       ${.TARGET} >${.TARGET}.SED; \
		${MV} -f ${.TARGET}.SED ${.TARGET}; \
	fi
	${.ALLSRC:T:@_EXPORT_TARGET@${_MAKE_EXPLINKS_}@}
.endif

.endif

#
#  odm database creation/addition rule for etc/objrepos, this 
#  hopefully will be expanded for all odm database files
#

.if defined(BUILD_ODM)
${OTHERS}	: ${BUILD_ODM}
.endif

${BUILD_ODM} : ${CRE_LIST} ${ADD_LIST}
	@${RM} -f ${BUILD_LIST}
	@-for i in ${.ALLSRC} ; do \
		temp_file=`${BASENAME} $$i` ; \
		if [ -n "`${ECHO} ${CRE_LIST} | ${GREP} -w $$temp_file`" ] ; \
		then \
			${CP} $$i `$(BASENAME) $$i`.tmp ; \
			${ECHO} "ODMDIR=. ; ${ODMCREATE} -c -p `$(BASENAME) $$i`.tmp" ;\
			(ODMDIR=. ; ${ODMCREATE} -c -p `$(BASENAME) $$i`.tmp > /dev/null);\
			${RM} -f `$(BASENAME) $$i`.tmp ; \
		fi ; \
	done
	@-for i in ${.ALLSRC} ; do \
		temp_file=`${BASENAME} $$i` ; \
		if [ -n "`${ECHO} ${ADD_LIST} | ${GREP} $$temp_file`" ] ; \
		then \
			${ECHO} "ODMDIR=. ; ${ODMADD} $$i" ; \
			(ODMDIR=. ; ${ODMADD} $$i) ; \
		fi ; \
	done
	${TOUCH} ${BUILD_ODM}

#
#  Installation rules
#
.if !empty(.TARGETS:Minstall_*)

.if !defined(TOSTAGE)
install_all: ${ALWAYS}
	@${ECHO} "You must define TOSTAGE to do an ${.TARGET}"

.if !empty(_INSTALL_TARGETS_:Minstall_*)
${_INSTALL_TARGETS_:Minstall_*}: ${ALWAYS}
	@${ECHO} "You must define TOSTAGE to do an ${.TARGET}"
.endif

.else

install_all: $${_all_targets_};@

.if !empty(_INSTALL_TARGETS_:Minstallcat_*)

_INSTCAT_TARGETS_ = ${_INSTALL_TARGETS_:Minstallcat_*:@TMP@${TOSTAGE}${MSGLANGPATH}/${TMP:S/^installcat_//}@}

${_INSTALL_TARGETS_:Minstallcat_*}: ${TOSTAGE}${MSGLANGPATH}/$${.TARGET:S/^installcat_//}

${_INSTCAT_TARGETS_}: $${_INSTALL_TARGET}
	  @[ ! -f ${TOSTAGE}${MSGLANGPATH}/${_INSTALL_TARGET} ] || \
	    ${TOUCH} ${TOSTAGE}${MSGLANGPATH}/${_INSTALL_TARGET}
	  @-${CMP} -s ${TOSTAGE}${MSGLANGPATH}/${_INSTALL_TARGET} \
	      ${${_INSTALL_TARGET}:P} \
	      2>/dev/null || \
	    ( ${ECHO} "Shipping ${MSGLANGPATH}/${_INSTALL_TARGET}" ; \
	      ${MAKEPATH} ${TOSTAGE}${MSGLANGPATH}/ ; \
	      ${RM} ${_RMFLAGS_} \
	        ${TOSTAGE}${MSGLANGPATH}/${_INSTALL_TARGET} ; \
	      ${CP} -p ${_INSTALL_TARGET} \
	        ${TOSTAGE}${MSGLANGPATH}/${_INSTALL_TARGET} ; \
	      noupdate=`${ECHO} ${NOUPDATE_LIST} | ${XARGS} ${ECHO} | \
	        ${GREP} -w ${MSGLANGPATH}/${_INSTALL_TARGET}` \
	        2>/dev/null ; \
	      if [ "${PTF_UPDATE}" = "yes" -a -z ""$$noupdate ] ; \
	      then \
	          ( ${ECHO} "Adding .${MSGLANGPATH}/${_INSTALL_TARGET} \
	            to ${UPDATELOG}...\c"; \
	            if ${FGREP} -x \
	              ".${MSGLANGPATH}/${_INSTALL_TARGET}" \
	              ${UPDATELOG} >/dev/null 2>&1 ; \
	            then \
	              ${ECHO} "already there." ; \
	            else \
	              ${ECHO} ".${MSGLANGPATH}/${_INSTALL_TARGET}" \
	                >> ${UPDATELOG} ; \
	              ${ECHO} "done"; \
	            fi ; \
	          ) ; \
	      fi ; \
	    ) ; 
.endif

.if !empty(_INSTALL_TARGETS_:Minstall_*)

.if (${LPP} == "gos")
# Needed for gos custom Makefiles
DEFAULTLANG = en_US
CONTEXTLANG != ${EXPR} `${ECHO} ${MAKEDIR}` : '.*\(.._..\)' || ${ECHO} ${DEFAULTLANG}
APPCUSTOMLANG ?= ${CONTEXTLANG:S/${DEFAULTLANG}//}
APPDEFAULTSLANG ?= ${CONTEXTLANG:S/${DEFAULTLANG}//}
.endif

_INST_TARGETS_ = ${_INSTALL_TARGETS_:Minstall_*:@TMP@${TOSTAGE}${${TMP:S/^install_//}_IDIR:U${IDIR:U/_MISSING_IDIR_/}}${TMP:S/^install_//}@}

${_INSTALL_TARGETS_:Minstall_*}: ${TOSTAGE}$${$${.TARGET:S/^install_//}_IDIR:U$${IDIR:U/_MISSING_IDIR_/}}$${.TARGET:S/^install_//}

${_INST_TARGETS_}: $${_INSTALL_TARGET}
	@[ ! -f ${TOSTAGE}${_IDIR_}${_INSTALL_TARGET} ] || \
	  ${TOUCH} ${TOSTAGE}${_IDIR_}${_INSTALL_TARGET}
	@-if [ -n "`${ECHO} ${LIBRARIES} ${SHARED_LIBRARIES} ${OTHER_LIBRARIES}\
	  | ${GREP} -w ${_INSTALL_TARGET}`" ] ; \
	then \
	    tmpfile=TEMPORARY_FILE ; \
	    ${RM} -f $$tmpfile ; \
	    objdir=${TOSTAGE}${_IDIR_}inst_updt/${_INSTALL_TARGET} ; \
	    [ -d $$objdir ] || ${MAKEPATH} $$objdir/ ; \
	    _LIB_OFILES_=`${AR} -t ${_INSTALL_TARGET}` ; \
	    [ -z "${LIBRARIES}" ] || \
	       ${AR} x ${_INSTALL_TARGET} ; \
	    for i in ""$${_LIB_OFILES_} ; \
	    do \
	      target=$$objdir/$$i ; \
	      ${CMP} -s $$i $$target || \
	        ( ${ECHO} "$$target" >> $$tmpfile ; \
	        ) ; \
	    done ; \
	    [ -z "${LIBRARIES}" ] || \
	      ${RM} ${_RMFLAGS_} $${_LIB_OFILES_} ; \
	    [ ! -f $$tmpfile ] || \
	      ( noupdate=`${ECHO} ${NOUPDATE_LIST} | ${XARGS} ${ECHO} | \
	            ${GREP} -w ${_IDIR_}${_INSTALL_TARGET}` 2>/dev/null ; \
	        if [ "${PTF_UPDATE}" = "yes" -a -z "$$noupdate" ] ; \
	        then \
	          for i in "`${CAT} $$tmpfile`" ; \
	          do \
	            ${ECHO} "Adding $$i to ${UPDATELOG}...\c" ;\
	            if ${FGREP} -x $$i ${UPDATELOG} \
	              >/dev/null 2>&1 ; \
	            then\
	              ${ECHO} "already there." ; \
	            else\
	              ${ECHO} $$i >> ${UPDATELOG} ; \
	              ${ECHO} $$i" "`${PWD_SHELL}`"/"$$i" NEW"\
	                >> ${LIBDEPLOG} ; \
	              ${ECHO} "done" ; \
	            fi ; \
	          done ; \
	          ${ECHO} "${WHAT_FILESET} -i ${_IDIR_}${_INSTALL_TARGET}" ; \
	          ${WHAT_FILESET} -i ${_IDIR_}${_INSTALL_TARGET} ; \
	        fi ; \
	        ${ECHO} "Shipping ${_IDIR_}${_INSTALL_TARGET}" ; \
	        ${MAKEPATH} ${TOSTAGE}${_IDIR_}inst_updt/${_INSTALL_TARGET}/ ; \
	        ${RM} ${_RMFLAGS_} ${TOSTAGE}${_IDIR_}${_INSTALL_TARGET} ; \
	        ${CP} -p ${${_INSTALL_TARGET}:P} \
	          ${TOSTAGE}${_IDIR_}${_INSTALL_TARGET} ; \
	        [ -z "${${_INSTALL_TARGET}_EXECUTABLE}" ] || \
	          ${CHMOD} +x ${TOSTAGE}${_IDIR_}${_INSTALL_TARGET} ; \
	        current_dir=`${PWD_SHELL}` ; \
	        cd ${TOSTAGE}${_IDIR_}inst_updt/${_INSTALL_TARGET} ; \
	        ${AR} x ${TOSTAGE}${_IDIR_}${_INSTALL_TARGET} ; \
	        cd $$current_dir ; \
	      ) ; \
	  ${RM} -f $$tmpfile ; \
	  [ -z "${_ILINKS_}" ] || \
	    ( ${RM} ${_RMFLAGS_} ${TOSTAGE}${_ILINKS_} ; \
	      ${MAKEPATH} ${TOSTAGE}${_ILINKS_} ; \
	      ${ECHO} "/usr/bin/ln ${TOSTAGE}${_IDIR_}${_INSTALL_TARGET} \
	        ${TOSTAGE}${_ILINKS_}" ; \
	      /usr/bin/ln ${TOSTAGE}${_IDIR_}${_INSTALL_TARGET} \
	        ${TOSTAGE}${_ILINKS_} ; \
	    ) ; \
	else \
	  for line in ""${NOUPDATE_LIST} ; \
	  do \
		if [ "$$line" = ${_IDIR_}${_INSTALL_TARGET} ] ; \
		then \
			noupdate="yes" ; \
			break ; \
		fi ; \
	  done ; \
	  ${CMP} -s ${TOSTAGE}${_IDIR_}${_INSTALL_TARGET} \
	    ${${_INSTALL_TARGET}:P} 2>/dev/null || \
	    ( ${ECHO} "Shipping ${_IDIR_}${_INSTALL_TARGET}" ; \
	      ${MAKEPATH} ${TOSTAGE}${_IDIR_} ; \
	      ${RM} ${_RMFLAGS_} ${TOSTAGE}${_IDIR_}${_INSTALL_TARGET} ; \
	      ${CP} -p ${${_INSTALL_TARGET}:P} \
	        ${TOSTAGE}${_IDIR_}${_INSTALL_TARGET} ; \
	      [ -z "${${_INSTALL_TARGET}_EXECUTABLE}" ] || \
	        ${CHMOD} +x ${TOSTAGE}${_IDIR_}${_INSTALL_TARGET} ; \
	      [ -z "${_ILINKS_}" ] || \
	        ( ${RM} ${_RMFLAGS_} ${TOSTAGE}${_ILINKS_} ; \
	          ${MAKEPATH} ${TOSTAGE}${_ILINKS_} ; \
	          ${ECHO} "/usr/bin/ln ${TOSTAGE}${_IDIR_}${_INSTALL_TARGET}\
	            ${TOSTAGE}${_ILINKS_}" ; \
	          /usr/bin/ln ${TOSTAGE}${_IDIR_}${_INSTALL_TARGET} \
	            ${TOSTAGE}${_ILINKS_} ; \
	        ) ; \
	      if [ "${PTF_UPDATE}" = "yes" -a -z "$$noupdate" ] ; \
	      then \
	          ( ${ECHO} "Adding .${_IDIR_}${_INSTALL_TARGET} \
	              to ${UPDATELOG}...\c"; \
	            if ${FGREP} -x ".${_IDIR_}${_INSTALL_TARGET}" \
	              ${UPDATELOG} >/dev/null 2>&1 ; \
	            then \
	              ${ECHO} "already there." ; \
	            else \
	              ${ECHO} ".${_IDIR_}${_INSTALL_TARGET}" \
	                >> ${UPDATELOG} ; \
	              ${ECHO} "done"; \
	            fi ; \
	          ) ; \
	          ${ECHO} "${WHAT_FILESET} -i ${_IDIR_}${_INSTALL_TARGET}" ; \
	          ${WHAT_FILESET} -i ${_IDIR_}${_INSTALL_TARGET} ; \
	      fi ; \
	    ) ; \
	fi ;
.endif

.endif

.endif

.endif
