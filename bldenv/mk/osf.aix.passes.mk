# @(#)08        1.23  src/bldenv/mk/osf.aix.passes.mk, ade_build, bos41J, 9519B_all 5/10/95 16:13:25
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
# (C) COPYRIGHT International Business Machines Corp. 1991, 1993
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
# OSF/1 1.1 Snapshot 2

.if !defined(_OSF_OSC_PASSES_MK_)
_OSF_OSC_PASSES_MK_=

#
# Start of PASSES
#

#
#  These list the "tags" associated with each pass
#
.if ${project_name} == "dce"
_PASS_SETUP_TAGS_=SETUP
_PASS_AUTOGEN_TAGS_=SAMIDL
_PASS_FIRSTA_TAGS_=MVDATA
_PASS_FIRSTB_TAGS_=CHKDATA
.endif
_PASS_FIRST_TAGS_=EXPINC
_PASS_SECOND_TAGS_=EXPLIBC EXPCRT0 EXPLIBLD
.if defined(NO_SHARED_LIBRARIES)
_PASS_THIRD_TAGS_=
_PASS_FOURTH_TAGS_=
.else
_PASS_THIRD_TAGS_=EXPLOADER
_PASS_FOURTH_TAGS_=EXPSHLIB
.endif
_PASS_FIFTH_TAGS_=EXPLIB
.if defined(SECURITY_ONLY)
_PASS_BASIC_TAGS_=SECURITY
.else
_PASS_BASIC_TAGS_=STANDARD
.endif

#
#  These list the variables used to define subdirectories to recurse into
#
.if ${project_name} == "dce"
_SETUP_SUBDIRS_=${SETUP_SUBDIRS}
_SAMIDL_SUBDIRS_=${SAMIDL_SUBDIRS}
_MVDATA_SUBDIRS_= ${DATA_SUBDIRS}
_CHKDATA_SUBDIRS_=${DATA_SUBDIRS}
.endif
_EXPINC_SUBDIRS_=${EXPINC_SUBDIRS}
_EXPLIBC_SUBDIRS_=${EXPLIBC_SUBDIRS}
_EXPCRT0_SUBDIRS_=${EXPCRT0_SUBDIRS}
_EXPLIBLD_SUBDIRS_=${EXPLIBLD_SUBDIRS}
_EXPLOADER_SUBDIRS_=${EXPLOADER_SUBDIRS}
_EXPSHLIB_SUBDIRS_=${EXPSHLIB_SUBDIRS}
_EXPLIB_SUBDIRS_=${EXPLIB_SUBDIRS}
_SECURITY_SUBDIRS_=${SEC_SUBDIRS}
_STANDARD_SUBDIRS_=${SUBDIRS}

#
#  For each ACTION define the action for recursion, the passes for the
#  action, the targets for the complete action, and the targets for each
#  pass of the action
#
.if defined(MAKEFILE_PASS)
_BUILD_PASSES_=${MAKEFILE_PASS}
.else
.if ${project_name} != "dce"
_BUILD_PASSES_=FIRST
.else
_BUILD_PASSES_=AUTOGEN
_BUILD_PASSES_+=FIRST
_BUILD_PASSES_+=FIRSTA
_BUILD_PASSES_+=FIRSTB
.endif
_BUILD_PASSES_+=SECOND
.if !defined(NO_SHARED_LIBRARIES)
_BUILD_PASSES_+=THIRD FOURTH
.endif
_BUILD_PASSES_+=FIFTH BASIC
.endif

_BUILD_ACTION_=build
_build_action_=BUILD

.if ${project_name} == "dce"
_BUILD_SAMIDL_TARGETS_=\
	${_EXPORT_SAMIDL_TARGETS_}  ${SAMS_TARGETS}
_BUILD_MVDATA_TARGETS_=\
	${_MVDATA_MVDATA_TARGETS_}
_BUILD_CHKDATA_TARGETS_=\
	${_CHKDATA_CHKDATA_TARGETS_}
.endif
_BUILD_EXPINC_TARGETS_=\
	${_EXPORT_EXPINC_TARGETS_}
_BUILD_EXPLIBC_TARGETS_=\
	${_EXPORT_EXPLIBC_TARGETS_}
_BUILD_EXPCRT0_TARGETS_=\
	${_EXPORT_EXPCRT0_TARGETS_}
_BUILD_EXPLIBLD_TARGETS_=\
	${_EXPORT_EXPLIBLD_TARGETS_}
_BUILD_EXPLOADER_TARGETS_=\
	${_EXPORT_EXPLOADER_TARGETS_}
_BUILD_EXPSHLIB_TARGETS_=\
	${_EXPORT_EXPSHLIB_TARGETS_}
_BUILD_EXPLIB_TARGETS_=\
	${_EXPORT_EXPLIB_TARGETS_}
_BUILD_SECURITY_TARGETS_=\
	${_COMP_SECURITY_TARGETS_}
_BUILD_STANDARD_TARGETS_=\
	${_COMP_STANDARD_TARGETS_}

.if defined(MAKEFILE_PASS)
_COMP_PASSES_=${MAKEFILE_PASS}
.else
_COMP_PASSES_=BASIC
.endif

_COMP_ACTION_=comp
_comp_action_=COMP

_COMP_SECURITY_TARGETS_=\
	${SEC_PROGRAMS} \
	${SEC_LIBRARIES} \
	${SEC_OBJECTS} \
	${SEC_SCRIPTS} \
	${SEC_DATAFILES} \
	${SEC_OTHERS} \
	${SEC_MANPAGES:=.0} \
	${SEC_DOCUMENTS:S/$/.ps/g} \
	${SEC_DOCUMENTS:S/$/.out/g}
_COMP_STANDARD_TARGETS_=\
	${PROGRAMS} \
	${PROGRAMS++} \
	${LIBRARIES} \
	${SHARED_LIBRARIES} \
	${OBJECTS} \
	${PPC} \
	${PWR} \
	${PINNED} \
	${PAGED_PRIM}  \
	${PAGED}  \
	${PAGED_BACK}  \
	${INIT}  \
	${DBG} \
	${KERNEL_EXT} \
	${KERNEL} \
	${IMAGE} \
	${SCRIPTS} \
	${CATFILES} \
	${DATAFILES} \
	${OTHERS} \
	${COMPRESS_FILES:=.Z} \
	${MANPAGES:=.0} \
	${DOCUMENTS:S/$/.ps/g} \
	${DOCUMENTS:S/$/.out/g}

.if defined(MAKEFILE_PASS)
_CLEAN_PASSES_=${MAKEFILE_PASS}
.else
_CLEAN_PASSES_=BASIC
.endif

_CLEAN_TARGETS_=${_clean_action_:@.ACTION.@${_TARGET_ACTIONS_}@}
_CLEAN_ACTION_=clean
_clean_action_=CLEAN

_CLEAN_SECURITY_TARGETS_=\
	${SEC_PROGRAMS:S/^/clean_/g} \
	${SEC_LIBRARIES:S/^/clean_/g} \
	${SEC_OBJECTS:S/^/clean_/g} \
	${SEC_SCRIPTS:S/^/clean_/g} \
	${SEC_OTHERS:S/^/clean_/g} \
	${SEC_MANPAGES:S/^/clean_/g:S/$/.0/g} \
	${SEC_DOCUMENTS:S/^/clean_/g:S/$/.ps/g} \
	${SEC_DOCUMENTS:S/^/clean_/g:S/$/.out/g}
_CLEAN_STANDARD_TARGETS_=\
	${PROGRAMS:S/^/clean_/g} \
	${LIBRARIES:S/^/clean_/g} \
	${OBJECTS:S/^/clean_/g} \
	${SCRIPTS:S/^/clean_/g} \
	${OTHERS:S/^/clean_/g} \
	${KERNEL_EXT:S/^/clean_/g} \
	${MANPAGES:S/^/clean_/g:S/$/.0/g} \
	${DOCUMENTS:S/^/clean_/g:S/$/.ps/g} \
	${DOCUMENTS:S/^/clean_/g:S/$/.out/g}

.if defined(MAKEFILE_PASS)
_RMTARGET_PASSES_=${MAKEFILE_PASS}
.else
_RMTARGET_PASSES_=BASIC
.endif

_RMTARGET_TARGETS_=${_rmtarget_action_:@.ACTION.@${_TARGET_ACTIONS_}@}
_RMTARGET_ACTION_=rmtarget
_rmtarget_action_=RMTARGET

_RMTARGET_SECURITY_TARGETS_=\
	${SEC_PROGRAMS:S/^/rmtarget_/g} \
	${SEC_LIBRARIES:S/^/rmtarget_/g} \
	${SEC_OBJECTS:S/^/rmtarget_/g} \
	${SEC_SCRIPTS:S/^/rmtarget_/g} \
	${SEC_OTHERS:S/^/rmtarget_/g} \
	${SEC_MANPAGES:S/^/rmtarget_/g:S/$/.0/g} \
	${SEC_DOCUMENTS:S/^/rmtarget_/g:S/$/.ps/g} \
	${SEC_DOCUMENTS:S/^/rmtarget_/g:S/$/.out/g}
_RMTARGET_STANDARD_TARGETS_=\
	${PROGRAMS:S/^/rmtarget_/g} \
	${LIBRARIES:S/^/rmtarget_/g} \
	${OBJECTS:S/^/rmtarget_/g} \
	${SCRIPTS:S/^/rmtarget_/g} \
	${OTHERS:S/^/rmtarget_/g} \
	${KERNEL_EXT:S/^/rmtarget_/g} \
	${MANPAGES:S/^/rmtarget_/g:S/$/.0/g} \
	${DOCUMENTS:S/^/rmtarget_/g:S/$/.ps/g} \
	${DOCUMENTS:S/^/rmtarget_/g:S/$/.out/g}

.if ${project_name} == "dce"
.if defined(EXPSAMIDL_TARGETS)
_EXPORT_SAMIDL_TARGETS_=\
	${EXPSAMIDL_TARGETS}
.else
_EXPORT_SAMIDL_TARGETS_=\
	${SAMIDL_INCLUDES:S/^/export_/g} ${SAMS_TARGETS}
.endif
.endif

.if defined(MAKEFILE_PASS)
_CLOBBER_PASSES_=${MAKEFILE_PASS}
.else
_CLOBBER_PASSES_=BASIC
.endif

_CLOBBER_TARGETS_=${_clobber_action_:@.ACTION.@${_TARGET_ACTIONS_}@}
_CLOBBER_ACTION_=clobber
_clobber_action_=CLOBBER

_CLOBBER_SECURITY_TARGETS_=\
	${SEC_PROGRAMS:S/^/clobber_/g} \
	${SEC_LIBRARIES:S/^/clobber_/g} \
	${SEC_OBJECTS:S/^/clobber_/g} \
	${SEC_SCRIPTS:S/^/clobber_/g} \
	${SEC_OTHERS:S/^/clobber_/g} \
	${SEC_MANPAGES:S/^/clobber_/g:S/$/.0/g} \
	${SEC_DOCUMENTS:S/^/clobber_/g:S/$/.ps/g} \
	${SEC_DOCUMENTS:S/^/clobber_/g:S/$/.out/g}
_CLOBBER_STANDARD_TARGETS_=\
	${PROGRAMS:S/^/clobber_/g} \
	${LIBRARIES:S/^/clobber_/g} \
	${OBJECTS:S/^/clobber_/g} \
	${SCRIPTS:S/^/clobber_/g} \
	${OTHERS:S/^/clobber_/g} \
	${KERNEL_EXT:S/^/clobber_/g} \
	${MANPAGES:S/^/clobber_/g:S/$/.0/g} \
	${DOCUMENTS:S/^/clobber_/g:S/$/.ps/g} \
	${DOCUMENTS:S/^/clobber_/g:S/$/.out/g}

.if defined(MAKEFILE_PASS)
_LINK_PASSES_=${MAKEFILE_PASS}
.else
_LINK_PASSES_=FIRST FIFTH BASIC
.endif

_LINK_TARGETS_=${_link_action_:@.ACTION.@${_TARGET_ACTIONS_}@}
_LINK_ACTION_=link
_link_action_=LINK

.if ${project_name} == "dce"
.if defined(MAKEFILE_PASS)
_SETUP_PASSES_=${MAKEFILE_PASS}
.else
_SETUP_PASSES_=SETUP
.endif
_SETUP_TARGETS_=${_setup_action_:@.ACTION.@${_TARGET_ACTIONS_}@}
_SETUP_ACTION_=setup
_setup_action_=SETUP

_SETUP_SETUP_TARGETS_=\
	${SETUP_PROGRAMS:S/^/setup_/g} \
	${SETUP_SCRIPTS:S/^/setup_/g} \
	${SETUP_OTHERS:S/^/setup_/g} \
	${_EXPORT_SETUP_TARGETS_}

.if defined(MAKEFILE_PASS)
_SAMIDL_PASSES_=${MAKEFILE_PASS}
.else
_SAMIDL_PASSES_=AUTOGEN
.endif
_SAMIDL_TARGETS_=${_samidl_action_:@.ACTION.@${_TARGET_ACTIONS_}@}
_SAMIDL_ACTION_=samidl
_samidl_action_=SAMIDL

_SAMIDL_SAMIDL_TARGETS_=\
	${SAMS_TARGETS:S/^/samidl_/g} \
	${SAMS_SCRIPTS:S/^/samidl_/g} \
	${SAMS_OTHERS:S/^/samidl_/g} \
	${_EXPORT_SAMIDL_TARGETS_}

.if defined(MAKEFILE_PASS)
_MVDATA_PASSES_=${MAKEFILE_PASS}
.else
_MVDATA_PASSES_=FIRSTA
.endif

_MVDATA_TARGETS_=${_mvdata_action_:@.ACTION.@${_TARGET_ACTIONS_}@}
_MVDATA_ACTION_=mvdata
_mvdata_action_=MVDATA

.if defined(DATAFILES)
_MVDATA_MVDATA_TARGETS_=${DATAFILES:S/$/.local/g}
.endif

.if defined(MAKEFILE_PASS)
_CHKDATA_PASSES_=${MAKEFILE_PASS}
.else
_CHKDATA_PASSES_=FIRSTB
.endif

_CHKDATA_TARGETS_=${_chkdata_action_:@.ACTION.@${_TARGET_ACTIONS_}@}
_CHKDATA_ACTION_=chkdata
_chkdata_action_=CHKDATA

.if defined(DATAFILES) || defined(IDLDATAFILES)
.if defined(IDLDATAFILES)
RESTORE_TARGETS= ${DATAFILES:S/$/.restore/g} ${IDLDATAFILES}
.else
RESTORE_TARGETS= ${DATAFILES:S/$/.restore/g}
.endif
CHKDATA_TARGETS= ${DATAFILES:S/$/.datachk/g}
_CHKDATA_CHKDATA_TARGETS_=${CHKDATA_TARGETS}

#
# if we are not recursing in place; ie: __DIDDATA__ defined; then add the
# restore targets to the list for CHKDATA
#
.if !defined(__DIDDATA__)
_CHKDATA_CHKDATA_TARGETS_ += ${RESTORE_TARGETS}
.ORDER: ${_CHKDATA_CHKDATA_TARGETS_}
.endif
.endif
.endif

.if defined(MAKEFILE_PASS)
_LINT_PASSES_=${MAKEFILE_PASS}
.else
_LINT_PASSES_=BASIC
.endif

_LINT_TARGETS_=${_lint_action_:@.ACTION.@${_TARGET_ACTIONS_}@}
_LINT_ACTION_=lint
_lint_action_=LINT

_LINT_SECURITY_TARGETS_=\
	${SEC_PROGRAMS:S/^/lint_/g} \
	${SEC_LIBRARIES:S/^/lint_/g} \
	${SEC_OBJECTS:S/^/lint_/g}
_LINT_STANDARD_TARGETS_=\
	${PROGRAMS:S/^/lint_/g} \
	${LIBRARIES:S/^/lint_/g} \
	${SHARED_LIBRARIES:S/^/lint_/g} \
	${KERNEL_EXT:S/^/lint_/g} \
	${KERNEL:S/^/lint_/g} \
	${OBJECTS:S/^/lint_/g}

.if defined(MAKEFILE_PASS)
_TAGS_PASSES_=${MAKEFILE_PASS}
.else
_TAGS_PASSES_=BASIC
.endif

_TAGS_TARGETS_=${_tags_action_:@.ACTION.@${_TARGET_ACTIONS_}@}
_TAGS_ACTION_=tags
_tags_action_=TAGS

_TAGS_SECURITY_TARGETS_=\
	${SEC_PROGRAMS:S/^/tags_/g} \
	${SEC_LIBRARIES:S/^/tags_/g} \
	${SEC_OBJECTS:S/^/tags_/g}
_TAGS_STANDARD_TARGETS_=\
	${PROGRAMS:S/^/tags_/g} \
	${LIBRARIES:S/^/tags_/g} \
	${SHARED_LIBRARIES:Dtags_SHARED} \
	${OBJECTS:S/^/tags_/g}

.if defined(MAKEFILE_PASS)
_EXPORT_PASSES_=${MAKEFILE_PASS}
.else
.if ${project_name} != "dce"
_EXPORT_PASSES_=FIRST
.else
_EXPORT_PASSES_=AUTOGEN
_EXPORT_PASSES_+=FIRST
.endif
_EXPORT_PASSES_+=SECOND
.if !defined(NO_SHARED_LIBRARIES)
_EXPORT_PASSES_+=THIRD FOURTH
.endif
_EXPORT_PASSES_+=FIFTH
.endif

_EXPORT_TARGETS_=${_export_action_:@.ACTION.@${_TARGET_ACTIONS_}@}
_EXPORT_ACTION_=export
_export_action_=EXPORT

.if defined(EXPINC_TARGETS)
_EXPORT_EXPINC_TARGETS_=\
	${EXPINC_TARGETS}
.else
_EXPORT_EXPINC_TARGETS_=\
	${INCLUDES:S/^/export_/g}
.endif
.if ${project_name} == "dce"
.if defined(IDLDATAFILES)
_EXPORT_EXPINC_TARGETS_+=${IDLDATAFILES}
.endif
.endif

_EXPORT_EXPLIBC_TARGETS_=\
	${EXPLIBC_TARGETS}
_EXPORT_EXPCRT0_TARGETS_=\
	${EXPCRT0_TARGETS}
_EXPORT_EXPLIBLD_TARGETS_=\
	${EXPLIBLD_TARGETS}
_EXPORT_EXPLOADER_TARGETS_=\
	${EXPLOADER_TARGETS}
_EXPORT_EXPSHLIB_TARGETS_=\
	${EXPSHLIB_TARGETS}
_EXPORT_EXPLIB_TARGETS_=\
	${EXPLIB_TARGETS}

.if ${project_name} == "dce"
_EXPORT_SETUP_TARGETS_=\
	${SETUP_INCLUDES:S/^/export_/g}
.if defined(EXPSAMIDL_TARGETS)
_EXPORT_SAMIDL_TARGETS_=\
	${EXPSAMIDL_TARGETS}
.else
_EXPORT_SAMIDL_TARGETS_=\
	${SAMIDL_INCLUDES:S/^/export_/g} ${SAMS_TARGETS}
.endif
.endif

.if defined(MAKEFILE_PASS)
_INSTALL_PASSES_=${MAKEFILE_PASS}
.else
_INSTALL_PASSES_=BASIC
.endif

_INSTALL_TARGETS_=${_install_action_:@.ACTION.@${_TARGET_ACTIONS_}@}
_INSTALL_ACTION_=install
_install_action_=INSTALL

_INSTALL_SECURITY_TARGETS_=\
	${SEC_ILIST:S/^/install_/g}
_INSTALL_STANDARD_TARGETS_=\
	${CATFILES:S/^/installcat_/g} \
	${ILIST:S/^/install_/g}

#
#  Magic begins here...
#
#  This sequence of indirect macros basically performs the following
#  expansion inline to generate the correct dependents for each action
#
#  foreach pass (${_${action}_PASSES_})
#     foreach tag (${_PASS_${pass}_TAGS_})
#        foreach subdir (${_${tag}_SUBDIRS_})
#           _SUBDIR/${action}/${pass}/${tag}/${subdir}
#        end
#     foreach tag (${_PASS_${pass}_TAGS_})
#        ${_${action}_${tag}_TARGETS_}
#     end
#  end
#

.if ${project_name} != "dce"
_ALL_ACTIONS_=BUILD COMP CLEAN RMTARGET CLOBBER LINT TAGS EXPORT INSTALL LINK

_SUBDIR_TAGS_=${_${.TAG.}_SUBDIRS_:S;^;_SUBDIR/${.ACTION.}/${.PASS.}/${.TAG.}/;g}
.else
_ALL_ACTIONS_=BUILD COMP CLEAN RMTARGET CLOBBER LINT TAGS EXPORT MVDATA CHKDATA INSTALL SETUP SAMIDL LINK

# Some DCE SUBDIRS contain directories of the form dir1/dir2/dir3, this will
# cause problems when _SUBDIR_TAGS_ is create since the directory will no
# longer be just the field after the last '/'.  Change the '/' in SUBDIRS to
# '+++' so the directory will continue to be the field after the last '/'.
# When the directory field is accessed the '+++' are changed back to '/'.
_SUBDIR_TAGS_=${_${.TAG.}_SUBDIRS_:S;/;+++;g:S;^;_SUBDIR/${.ACTION.}/${.PASS.}/${.TAG.}/;g}
.endif
_SUBDIR_PASSES_?=${_PASS_${.PASS.}_TAGS_:@.TAG.@${_SUBDIR_TAGS_}@}
_TARGET_TAGS_=${_PASS_${.PASS.}_TAGS_:@.TAG.@${_${.ACTION.}_${.TAG.}_TARGETS_}@}
#_PASS_ACTIONS_=${_${.ACTION.}_PASSES_:@.PASS.@${_SUBDIR_PASSES_} ${_TARGET_TAGS_}@}
_PASS_ACTIONS_=${_${.ACTION.}_PASSES_:@.PASS.@${_SUBDIR_PASSES_} _TARGET/${.ACTION.}/${.PASS.}@}
_TARGET_ACTIONS_=${_${.ACTION.}_PASSES_:@.PASS.@${_TARGET_TAGS_}@}
_SUBDIR_ACTIONS_=${_${.ACTION.}_PASSES_:@.PASS.@${_SUBDIR_PASSES_}@}

_all_targets_=${_${.TARGET:S;_all$;;}_action_:@.ACTION.@${_PASS_ACTIONS_}@}

#
#  Order the high-level pass information so that we do not make
#  things on the next pass until this pass is finished.
#
.ORDER: ${BUILD:L:@.ACTION.@${_PASS_ACTIONS_}@}

_TARGET_ACTS_=${_${.ACTION.}_PASSES_:@.PASS.@_TARGET/${.ACTION.}/${.PASS.}@}
_TARGET_DEPS_=${.TARGET:H:T:@.ACTION.@${.TARGET:T:@.PASS.@${_TARGET_TAGS_}@}@}

${_ALL_ACTIONS_:@.ACTION.@${_TARGET_ACTS_}@}: $${_TARGET_DEPS_}

#
#  subdir recursion rule
#
#  This expands into targets matching the following pattern
#
#  _SUBDIR/<action>/<pass>/<tag>/<subdir>
#
.MAKE: ${_ALL_ACTIONS_:@.ACTION.@${_SUBDIR_ACTIONS_}@}

${_ALL_ACTIONS_:@.ACTION.@${_SUBDIR_ACTIONS_}@}:
.if ${project_name} != "dce"
	@${ECHO} "[ ${MAKEDIR:/=}/${.TARGET:T} ]"
	${MAKEPATH} ${.TARGET:T}/. && cd ${.TARGET:T} && \
	exec ${MAKE} MAKEFILE_PASS=${.TARGET:H:H:T} \
	      ${_${.TARGET:H:H:H:T}_ACTION_}_all
.else
	@${ECHO} "[ ${MAKEDIR:/=}/${.TARGET:T:S;+++;/;g} ]"
	${MAKEPATH} ${.TARGET:T:S;+++;/;g}/. && cd ${.TARGET:T:S;+++;/;g} && \
	exec ${MAKE} MAKEFILE_PASS=${.TARGET:H:H:T} \
	      ${_${.TARGET:H:H:H:T}_ACTION_}_all
.endif
	@${ECHO} "[ ${MAKEDIR:/=} ]"

#
# End of PASSES
#

_Z_=${_${.TAG.}_SUBDIRS_}
_Y_=${_PASS_${.PASS.}_TAGS_:@.TAG.@${_Z_}@}
_X_=${_${.ACTION.}_PASSES_:@.PASS.@${_Y_}@}
${_ALL_ACTIONS_:@.ACTION.@${_X_}@}: _SUBDIR/BUILD/BASIC/STANDARD/$${.TARGET};@

.if ${project_name} == "dce"
_W=${AUTOGEN:L:@.PASS.@${_SUBDIR_PASSES_} ${_TARGET_TAGS_}@}
_SAMIDL_TARGET_TAGS=${_${.ACTION.}_SAMIDL_TARGETS_}
samidl_all export_all_SAMIDL  : ${EXPORT:L:@.ACTION.@${_W}@}
.endif

.endif
