# @(#)40	1.3  src/bos/etc/swapspaces/swapspaces.S, cmdps, bos411, 9428A410j 3/19/91 10:01:16
#  COMPONENT_NAME: (CMDPS) paging space commands
#
#  ORIGINS: 27
#
#  (C) COPYRIGHT International Business Machines Corp. 1991
#  All Rights Reserved
#  Licensed Materials - Property of IBM
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

* /etc/swapspaces
*
* This file lists all the paging spaces that are automatically put into
* service on each system restart (the 'swapon -a' command executed from
* /etc/rc swaps on every device listed here).
*
* WARNING: Only paging space devices should be listed here.
*
* This file is modified by the chps, mkps and rmps commands and referenced
* by the lsps and swapon commands.


hd6:
	dev = /dev/hd6


