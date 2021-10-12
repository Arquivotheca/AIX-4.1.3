# @(#)17        1.17.1.7  src/bos/etc/security/login.cfg.S, cfgsauth, bos411, 9428A410j 6/7/94 15:45:39
#
# COMPONENT_NAME: CFGSAUTH
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
*******************************************************************************
*
* Valid port attributes:
*
* herald	Specifies the initial message to be printed out when getty or
*		login prompts for a login name.  This value is a string that is
*		written out to the login port.  If the herald is not specified
*		for a port or in the defport stanza, then the default herald is
*		gotten from the message catalog associated with the language
*		set in /etc/environment.
*
* logindelay	The delay (in seconds) between unsuccessful login attempts.
*		This delay is multiplied by the number of unsuccessful logins -
*		i.e. if the value is 2, then the delay between unsuccessful
*		logins will be 2 seconds, then 4 seconds, then 6 seconds, etc.
*		Set this attribute to 0 to disable this feature.
*
* logindisable	The number of unsuccessful login attempts before this port is
*		locked.  Used in conjunction with logininterval.  Set this
*		attribute to 0 to disable this feature.
*
* logininterval	The number of seconds during which logindisable unsuccessful
*		login attempts must occur for a port to be locked.
*
* loginreenable	The number of minutes after a port is locked that it will be
*		automatically unlocked.  Setting this attribute to 0 will cause
*		the port to remain locked until the administrator unlocks it.
*
* logintimes	Defines the times a user can use this port to login.  The value
*		is a comma separated list of items as follows:
*			[!][MMdd[-MMdd]]:hhmm-hhmm
*				or
*			[!]MMdd[-MMdd][:hhmm-hhmm]
*				or
*			[!][w[-w]]:hhmm-hhmm
*				or
*			[!]w[-w][:hhmm-hhmm]
*		where MM is a month number (00=January, 11=December), dd is
*		the day of the month, hh is the hour of the day (00 - 23), mm
*		is the minute of the hour, and w is a weekday (0=Sunday, 7=
*		Saturday).
*
* sak_enabled	Defines whether users are allowed to access the trusted path
*		through this port through the use of the secure attention key
*		sequence (ctrl-x ctrl-r).  Possible values: true or false
*
* synonym	Defines the set of ports which are synonyms for the given port;
*		the ownership and permissions of these ports are set along with
*		the given port's ownership and permissions (and sak_enabled is
*		interpreted to be the same for the given port and all of its
*		synonyms).  This is mainly used for specifying that
*		/dev/console and /dev/tty0 (for example) are synonyms since
*		/dev/tty0 is the system console in this example.  The value is
*		a comma separated list of pathnames to device special files.
*
* The default stanza contains the default values used if no stanza appears for
* a given port.
*
*******************************************************************************

default:
	sak_enabled = false
	logintimes = 
	logindisable = 0
	logininterval = 0
	loginreenable = 0
	logindelay = 0

*/dev/console:
*	synonym = /dev/tty0

*******************************************************************************
*
* Authentication methods:
*
* auth_method:
*	program = /any/program
*
* auth_method corresponds to a custom authentication method specified in an
* AUTH1 or AUTH2 attribute in /etc/security/user, and /any/program is the
* program to run in order to do the authentication.
*
*******************************************************************************

*auth_method:
*	program =

*******************************************************************************
*
* Other security attributes (usw stanza):
*
* shells	The list of valid login shells for a user; chuser and chsh will
*		only change a user's login shell to one of the shells listed
*		here.
*
* maxlogins	The maximum number of simultaneous logins allowed on the
*		system.
*
* logintimeout	The number of seconds the user is given to enter their
*		password.
*
*******************************************************************************

usw:
	shells = /bin/sh,/bin/bsh,/bin/csh,/bin/ksh,/bin/tsh,/usr/bin/sh,/usr/bin/bsh,/usr/bin/csh,/usr/bin/ksh,/usr/bin/tsh
	maxlogins = 2
	logintimeout = 60

