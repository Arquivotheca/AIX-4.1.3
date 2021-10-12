# @(#)04        1.5  src/bldenv/mk/osf.aix.kernext.mk, ade_build, bos41J 5/3/94 11:36:26
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

_EXPORTS_		=${${.TARGET}_EXPORTS:U${EXPORTS}}
__EXPORTS__		=${_EXPORTS_:S/-bE://g}

_IMPORTS_		=${${.TARGET}_IMPORTS:U${IMPORTS}}
__IMPORTS__		=${_IMPORTS_:S/-bI://g}

_KERNEXT_IMPORTS_	=${KERNEXT_IMPORTS:S/-bI://g}

__LIBS__		=${_LIBS_:!${FINDFILE} ${LIBDIRS} ${_LIBS_}!}

__ENTRY__FLAG__		=-e${${.TARGET}_ENTRYPOINT}
__ENTRY_FLAG__		=${__ENTRY__FLAG__:S/-e$/-bnoentry/}

${KERNEL_EXT} : $${_OFILES_} $${__IMPORTS__} $${_KERNEXT_IMPORTS_} \
		$${__EXPORTS__} ${__LIBS__} $${${.TARGET}_DEP} ${MAKEFILE_DEPENDENCY}
	@if [ -L ${.TARGET}.rmap ]; \
	then \
		${CP} ${.TARGET}.rmap ${.TARGET}.rmap.X; \
		${MV} -f ${.TARGET}.rmap.X ${.TARGET}.rmap; \
	fi
	@if [ -L ${.TARGET}.map ]; \
	then \
		${CP} ${.TARGET}.map ${.TARGET}.map.X; \
		${MV} -f ${.TARGET}.map.X ${.TARGET}.map; \
	fi
	${RM} ${_RMFLAGS_} ${.TARGET}
	${LD} ${_LDFLAGS_KERNEXT_} -o ${.TARGET} ${_OFILES_} \
	${__ENTRY_FLAG__} \
	${__IMPORTS__:@TMP@-bI:${${TMP}:P}@} \
	${_KERNEXT_IMPORTS_:@TMP@-bI:${${TMP}:P}@} \
	${__LIBS__} \
	${__EXPORTS__:@TMP@-bE:${${TMP}:P}@}
