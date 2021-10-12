# @(#)41	1.2  src/bos/etc/security/lastlog.S, cfgsauth, bos411, 9428A410j 4/9/91 08:47:51
#  COMPONENT_NAME: CFGSAUTH
#
#  ORIGINS: 27
#
#  (C) COPYRIGHT International Business Machines Corp. 1989, 1991
#  All Rights Reserved
#  Licensed Materials - Property of IBM
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
*******************************************************************************
* valid user attributes for /etc/security/lastlog:
* 
* time_last_login		The last time that the user
*				successfully logged into the system.
*
* time_last_unsuccessful_login	The time that the last unsuccessful
*				attempt to log in as the user was made.
*
* tty_last_login		The last tty port that the user
*				successfully logged into.
*
* tty_last_unsuccessful_login	The tty port of the last unsuccessful
*				attempt to log in as the user was
*				made.
*
* host_last_login		The host from which the user logged in
*				from if the tty was not locally
*				attached.  This implies that the user
*				used telnet or rlogin to log into the
*				system.
*
* host_last_unsuccessful_login	The host from which the last
*				unsuccessful attempt to log in as the
*				user was made.
*
* unsuccessful_login_count	The number of attempts to log in as
*				the user since the last successful
*				login.
*
*******************************************************************************

