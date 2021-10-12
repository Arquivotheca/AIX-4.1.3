# @(#)92	1.6  src/bos/usr/lib/security/mkuser.default.S, cfgsauth, bos411, 9428A410j 10/7/93 12:13:09
#
#   COMPONENT_NAME: CFGSAUTH
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1989,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

user:
	pgrp = staff
	groups = staff
	shell = /usr/bin/ksh
	home = /home/$USER	

admin:
	pgrp = system
	groups = system
	shell = /usr/bin/ksh
	home = /home/$USER	

