# @(#)10        1.7  src/bldenv/mk/osf.aix.rules.mk, ade_build, bos41J 2/10/94 10:27:56
#
#   ORIGINS: 71
#
#                    SOURCE MATERIALS
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
# OSF/1 1.1 Snapshot 2

.if ! defined(_OSF_RULES_MK_)
_OSF_RULES_MK_=

.include <osf.${project_name}.std.mk>

.if empty(.TARGETS:Minstall_*)

.if defined(PROGRAMS) || defined(PROGRAMS++)
.include <osf.${project_name}.prog.mk>
.endif

.if defined(LIBRARIES) || defined(SHARED_LIBRARIES)
.include <osf.${project_name}.lib.mk>
.endif

.if defined(KERNEL_BUILD)
.include <osf.${project_name}.kernel.mk>
.endif

.if defined(KERNEXT_BUILD)
.include <osf.${project_name}.kernext.mk>
.endif

.if defined(BINARIES) || defined(KERNEXT_BUILD)
.include <osf.${project_name}.obj.mk>
.endif

.if defined(SCRIPTS)
.include <osf.${project_name}.script.mk>
.endif

.if defined(BFF)
.include <osf.${project_name}.packages.mk>
.endif

.if defined(DEPENDENCIES) && defined(BINARIES) && !defined(BFF)
.include <osf.${project_name}.depend.mk>
.endif

.endif
.endif
