# @(#)12        1.3  src/bldenv/mk/osf.aix.depend.mk, ade_build, bos412, GOLDA411a 1/10/94 15:58:55
#
#   ORIGINS: 27 71
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1991
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
# OSF/1 1.1 Snapshot 2

.if !defined(_OSF_DEPEND_MK_)
_OSF_DEPEND_MK_=

.if exists(depend.mk)
UNLINK_DEPEND_MK!= \
if [ -L depend.mk ]; \
then \
    ${CP} depend.mk depend.mk.X; \
    ${MV} -f depend.mk.X depend.mk; \
fi; \
if [ -L depend.mk.old ]; \
then \
    ${RM} ${_RMFLAGS_} depend.mk.old; \
fi

.include "depend.mk"
.endif

.EXIT:
	${MD} ${_MDFLAGS_} .

.endif
