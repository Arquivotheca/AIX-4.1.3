# @(#)96	1.1  src/bos/etc/security/portlog.S, cfgsauth, bos411, 9428A410j 10/4/93 11:28:35
#
#   COMPONENT_NAME: CFGSAUTH
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
*******************************************************************************
* valid port attributes for /etc/security/portlog:
*
* locktime			The time (in seconds since the Epoch) that
*				the given port was locked.  If this value is
*				0, then the port is not locked.
*
* unsuccessful_login_times	A comma separated list of times (in seconds
*				since the Epoch) that unsuccessful login
*				attempts occurred on this port.  This is used
*				in conjunction with logindisable and
*				logininterval in /etc/security/login.cfg to
*				determine when to lock a port.
*
*******************************************************************************

