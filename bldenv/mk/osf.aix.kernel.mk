# @(#)03        1.5  src/bldenv/mk/osf.aix.kernel.mk, ade_build, bos412, GOLDA411a 5/3/94 11:34:14
# COMPONENT_NAME: BLDPROCESS
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PINNED	=PINNED
PAGED	=PAGED
PAGED_PRIM =PAGED_PRIM
PAGED_BACK =PAGED_BACK
INIT	=INIT
DBG	=DBG
PPC	=PPC
PWR	=PWR

__PPPCDIRS__=`for i in ""${SUBDIRS}; do if [ -f $$i/PPC ]; then ${ECHO} $$i/PPC; fi; done`
__PPCDIRS__=${__PPPCDIRS__:!${ECHO} ${__PPPCDIRS__}!}

${PPC}:  ${PPC_OFILES} ${__PPCDIRS__} ${MAKEFILE_DEPENDENCY}
	@${RM} -f ${.TARGET}
	@[ -z "${SUBDIRS}" ] || `for x in ${SUBDIRS}; do \
	if [ -f $$x/${.TARGET} ]; then \
	${CAT} $$x/PPC | ${SED} "s!^!$$x/!" >> ${.TARGET}; fi; done`
	@[ -z "${PPC_OFILES}" ] || `for x in ${PPC_OFILES}; do \
	${ECHO} $$x >> ${.TARGET}; done`

__PPWRDIRS__=`for i in ""${SUBDIRS}; do if [ -f $$i/PWR ]; then ${ECHO} $$i/PWR; fi; done`
__PWRDIRS__=${__PPWRDIRS__:!${ECHO} ${__PPWRDIRS__}!}

${PWR}:  ${PWR_OFILES} ${__PWRDIRS__} ${MAKEFILE_DEPENDENCY}
	@${RM} -f ${.TARGET}
	@[ -z "${SUBDIRS}" ] || `for x in ${SUBDIRS}; do \
	if [ -f $$x/${.TARGET} ]; then \
	${CAT} $$x/PWR | ${SED} "s!^!$$x/!" >> ${.TARGET}; fi; done`
	@[ -z "${PWR_OFILES}" ] || `for x in ${PWR_OFILES}; do \
	${ECHO} $$x >> ${.TARGET}; done`

__PPINDIRS__=`for i in ""${SUBDIRS}; do if [ -f $$i/PINNED ]; then ${ECHO} $$i/PINNED; fi; done`
__PINDIRS__=${__PPINDIRS__:!${ECHO} ${__PPINDIRS__}!}

${PINNED}:  ${PINNED_OFILES} ${__PINDIRS__} ${MAKEFILE_DEPENDENCY}
	@${RM} -f ${.TARGET}
	@[ -z "${SUBDIRS}" ] || `for x in ${SUBDIRS}; do \
	if [ -f $$x/${.TARGET} ]; then \
	${CAT} $$x/PINNED | ${SED} "s!^!$$x/!" >> ${.TARGET}; fi; done`
	@[ -z "${PINNED_OFILES}" ] || `for x in ${PINNED_OFILES}; do \
	${ECHO} $$x >> ${.TARGET}; done`

__PPGDIRS__=`for i in ""${SUBDIRS}; do if [ -f $$i/PAGED ]; then ${ECHO} $$i/PAGED; fi; done`
__PGDIRS__=${__PPGDIRS__:!${ECHO} ${__PPGDIRS__}!}

__PPGDIRS_PRIM__=`for i in ""${SUBDIRS}; do if [ -f $$i/PAGED_PRIM ]; then ${ECHO} $$i/PAGED_PRIM; fi; done`
__PGDIRS_PRIM__=${__PPGDIRS_PRIM__:!${ECHO} ${__PPGDIRS_PRIM__}!}

__PPGDIRS_BACK__=`for i in ""${SUBDIRS}; do if [ -f $$i/PAGED_BACK ]; then ${ECHO} $$i/PAGED_BACK; fi; done`
__PGDIRS_BACK__=${__PPGDIRS_BACK__:!${ECHO} ${__PPGDIRS_BACK__}!}

${PAGED}:${PAGED_OFILES} ${__PGDIRS__} ${MAKEFILE_DEPENDENCY}
	@${RM} -f ${.TARGET}
	@[ -z "${SUBDIRS}" ] || `for x in ${SUBDIRS}; do \
	if [ -f $$x/${.TARGET} ]; then \
	${CAT} $$x/PAGED | ${SED} "s!^!$$x/!" >> ${.TARGET}; fi; done`
	@[ -z "${PAGED_OFILES}" ] || `for x in ${PAGED_OFILES}; do \
	${ECHO} $$x >> ${.TARGET}; done`

${PAGED_PRIM}:${PAGED_OFILES_PRIM} ${__PGDIRS_PRIM__} ${MAKEFILE_DEPENDENCY}
	@${RM} -f ${.TARGET}
	@[ -z "${SUBDIRS}" ] || `for x in ${SUBDIRS}; do \
	if [ -f $$x/${.TARGET} ]; then \
	${CAT} $$x/PAGED_PRIM | ${SED} "s!^!$$x/!" >> ${.TARGET}; fi; done`
	@[ -z "${PAGED_OFILES_PRIM}" ] || `for x in ${PAGED_OFILES_PRIM}; do \
	${ECHO} $$x >> ${.TARGET}; done`

${PAGED_BACK}:${PAGED_OFILES_BACK} ${__PGDIRS_BACK__} ${MAKEFILE_DEPENDENCY}
	@${RM} -f ${.TARGET}
	@[ -z "${SUBDIRS}" ] || `for x in ${SUBDIRS}; do \
	if [ -f $$x/${.TARGET} ]; then \
	${CAT} $$x/PAGED_BACK | ${SED} "s!^!$$x/!" >> ${.TARGET}; fi; done`
	@[ -z "${PAGED_OFILES_BACK}" ] || `for x in ${PAGED_OFILES_BACK}; do \
	${ECHO} $$x >> ${.TARGET}; done`

__IINITDIRS__=`for i in ""${SUBDIRS}; do if [ -f $$i/INIT ]; then ${ECHO} $$i/INIT; fi; done`
__INITDIRS__=${__IINITDIRS__:!${ECHO} ${__IINITDIRS__}!}

${INIT}:${INIT_OFILES} ${__INITDIRS__} ${MAKEFILE_DEPENDENCY}
	@${RM} -f ${.TARGET}
	@[ -z "${SUBDIRS}" ] || `for x in ${SUBDIRS}; do \
	if [ -f $$x/${.TARGET} ]; then \
	${CAT} $$x/INIT | ${SED} "s!^!$$x/!" >> ${.TARGET}; fi; done`
	@[ -z "${INIT_OFILES}" ] || `for x in ${INIT_OFILES}; do \
	${ECHO} $$x >> ${.TARGET}; done`

__DDBGDIRS__=`for i in ""${SUBDIRS}; do if [ -f $$i/DBG ]; then ${ECHO} $$i/DBG; fi; done`
__DBGDIRS__=${__DDBGDIRS__:!${ECHO} ${__DDBGDIRS__}!}

${DBG}:${DBG_OFILES} ${__DBGDIRS__} ${MAKEFILE_DEPENDENCY}
	@${RM} -f ${.TARGET}
	@[ -z "${SUBDIRS}" ] || `for x in ${SUBDIRS}; do \
	if [ -f $$x/${.TARGET} ]; then \
	${CAT} $$x/DBG | ${SED} "s!^!$$x/!" >> ${.TARGET}; fi; done`
	@[ -z "${DBG_OFILES}" ] || `for x in ${DBG_OFILES}; do \
	${ECHO} $$x >> ${.TARGET}; done`
