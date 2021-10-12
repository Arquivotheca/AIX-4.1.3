# @(#)43        1.1  src/bldenv/sbtools/include/ode/Makefile, bldprocess, bos412, GOLDA411a 1/19/94 17:35:33
#
#   COMPONENT_NAME: BLDPROCESS
#
#   FUNCTIONS: none
#
#   ORIGINS: 27,71
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# @OSF_FREE_COPYRIGHT@
# COPYRIGHT NOTICE
# Copyright (c) 1992, 1991, 1990  
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
# HISTORY
# $Log: Makefile,v $
# Revision 1.4.9.1  1993/11/03  20:58:11  damon
# 	CR 463. More pedantic
# 	[1993/11/03  20:57:41  damon]
#
# Revision 1.4.4.11  1993/05/18  18:45:04  damon
# 	CR 515. Added private dir
# 	[1993/05/18  18:40:08  damon]
# 
# Revision 1.4.4.10  1993/04/28  20:06:40  damon
# 	CR Added sets.h and sandboxes.h
# 	[1993/04/28  20:06:10  damon]
# 
# Revision 1.4.4.9  1993/04/28  19:21:45  damon
# 	CR 463. Pedantic changes
# 	[1993/04/28  19:21:25  damon]
# 
# Revision 1.4.4.8  1993/04/28  17:59:42  damon
# 	CR 463. Added builddata.h
# 	[1993/04/28  17:59:34  damon]
# 
# Revision 1.4.4.7  1993/04/27  20:42:40  damon
# 	CR 463. Added util.h
# 	[1993/04/27  20:42:21  damon]
# 
# 	CR 463. Added util.h
# 
# Revision 1.4.4.6  1993/04/27  20:04:47  damon
# 	CR 463. Added copyrights.h
# 	[1993/04/27  20:04:35  damon]
# 	#
# 	Revision 1.4.4.5  1993/04/27  15:32:08  damon
# 	CR 463. Added misc.h
# 	[1993/04/27  15:26:10  damon]
# 
# Revision 1.4.4.4  1993/03/17  20:38:42  damon
# 	CR 446. Added history.h src_ctl_rcs.h
# 	[1993/03/17  20:37:51  damon]
# 
# Revision 1.4.4.3  1993/02/19  18:51:15  damon
# 	CR 376. Moved sci_rcs.h to sci.h
# 	[1993/02/19  18:51:05  damon]
# 
# Revision 1.4.4.2  1993/01/13  20:45:07  damon
# 	CR 392. Added lockcmds.h
# 	[1993/01/13  20:44:40  damon]
# 
# Revision 1.4.2.5  1992/12/03  19:13:54  damon
# 	ODE 2.2 CR 346. Expanded copyright
# 	[1992/12/03  18:43:08  damon]
# 
# Revision 1.4.2.4  1992/09/24  18:43:31  gm
# 	CR240: Added public to SUBDIRS.
# 	[1992/09/23  18:20:20  gm]
# 
# Revision 1.4.2.3  1992/08/25  21:30:22  damon
# 	CR 240. Added odexm.h
# 	[1992/08/25  21:30:03  damon]
# 
# Revision 1.4.2.2  1992/08/07  19:07:00  damon
# 	CR 236. Added errno.h
# 	[1992/08/07  19:06:46  damon]
# 
# Revision 1.4  1991/12/05  21:03:47  devrcs
# 	First version using library version of SCAPI
# 	[91/07/31  20:10:49  damon]
# 
# 	added <ode/portable.h> header declarations
# 	[91/07/25  14:46:16  ezf]
# 
# 	added versions.h header file for ODE revisions
# 	[91/07/19  14:58:01  ezf]
# 
# 	Changes for Reno make
# 	[91/03/22  16:18:51  mckeen]
# 
# 	Creations/Fixes for project ode lbe
# 	[91/01/17  16:44:14  mckeen]
# 
# 	Changed std_defs.h to odedefs.h
# 	[91/01/09  16:25:34  randyb]
# 
# 	Changed std_defs.h to odedefs.h
# 	[91/01/09  15:18:00  randyb]
# 
# 	Added interface.h
# 	[91/01/08  13:39:10  randyb]
# 
# 	Added interface.h
# 	[91/01/08  13:39:10  randyb]
# 
# Revision 1.3  90/10/07  20:35:21  devrcs
# 	Added EndLog Marker.
# 	[90/09/28  20:57:13  gm]
# 
# Revision 1.2  90/08/09  14:28:07  devrcs
# 	Cleanup for export, install and post-freeze.
# 	[90/08/05  13:33:59  gm]
# 
# 	Moved here from usr/local/sdm/include.
# 	[90/08/05  12:31:24  gm]
# 
# $EndLog$

SUBDIRS			= private public
EXPINC_SUBDIRS		= private public
DATAFILES		= builddata.h \
			  copyrights.h errno.h history.h interface.h \
                          lockcmds.h misc.h \
			  odedefs.h odexm.h parse_rc_file.h portable.h \
			  project.h run.h sandboxes.h sci.h sci_client.h \
                          sets.h src_ctl_rcs.h util.h versions.h
INCLUDES		= ${DATAFILES}

EXPDIR			= /usr/include/ode/

.include <${RULES_MK}>
