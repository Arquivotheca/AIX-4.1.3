# @(#)38        1.13  src/bos/etc/Makefile, Makefiles, bos411, 9433A411a 8/9/94 10:25:21
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SUBDIRS			= services syslog \
			  consdef filesystems group passwd protocols vfs \
			  security brc environment inittab motd profile \
			  rc swapspaces objrepos rc.powerfail tsh_profile \
		 	  csh.cshrc csh.login

NOUPDATE_LIST = /etc/csh.cshrc \
		/etc/csh.login \
		/etc/environment \
		/etc/filesystems \
		/etc/inittab \
		/etc/motd \
		/etc/profile \
		/etc/rc \
		/etc/swapspaces \
		/etc/passwd \
		/etc/group

.include <${RULES_MK}>
