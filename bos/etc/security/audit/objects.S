# @(#)70	1.4  src/bos/etc/security/audit/objects.S, cfgsaud, bos411, 9428A410j 3/26/91 14:07:47
#  COMPONENT_NAME: CFGSAUD
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

/etc/security/environ:
	w = "S_ENVIRON_WRITE"

/etc/security/group:
	w = "S_GROUP_WRITE"

/etc/security/limits:
	w = "S_LIMITS_WRITE"

/etc/security/login.cfg:
	w = "S_LOGIN_WRITE"

/etc/security/passwd:
	r = "S_PASSWD_READ"
	w = "S_PASSWD_WRITE"

/etc/security/user:
	w = "S_USER_WRITE"

/etc/security/audit/config:
	w = "AUD_CONFIG_WR"

