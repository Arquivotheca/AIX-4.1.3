/* @(#)88	1.4  src/bos/usr/bin/newgrp/lsusatab.c, cmdsuser, bos41J, 9512A_all 3/14/95 15:54:57 */
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: sizeof
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "tcbauth.h"

struct	lsusattr	lsusatab [ ] =
{
	/*
	 * attributes of /etc/passwd
	 */
	{ S_ID,			SEC_INT,	NULL,		0 },
	{ S_PGRP,		SEC_CHAR,	NULL,		0 },
	{ S_GROUPS,		SEC_LIST,	chglist,	0 },
	{ S_HOME,		SEC_CHAR,	NULL,		0 },
	{ S_SHELL,		SEC_CHAR,	NULL,		0 },
	{ S_GECOS,		SEC_CHAR,	NULL,		0 },

	/*
	 * attributes of /etc/security/audit/audit.cfg
	 */
	{ S_AUDITCLASSES,	SEC_LIST,	chglist,	0 },

	/*
	 * attributes of /etc/security/user
	 */
	{ S_LOGINCHK,		SEC_BOOL,	chgbool,	0 },
	{ S_SUCHK,		SEC_BOOL,	chgbool,	0 },
	{ S_RLOGINCHK,		SEC_BOOL,	chgbool,	0 },
	{ S_TELNETCHK,		SEC_BOOL,	chgbool,	0 },
	{ S_DAEMONCHK,		SEC_BOOL,	chgbool,	0 },
	{ S_ADMIN,		SEC_BOOL,	chgbool,	0 },
	{ S_SUGROUPS,		SEC_LIST,	chglist,	0 },
	{ S_ADMGROUPS,		SEC_LIST,	chglist,	0 },
	{ S_TPATH,		SEC_CHAR,	NULL,		0 },
	{ S_TTYS,		SEC_LIST,	chglist,	0 },
	{ S_EXPIRATION,		SEC_CHAR,	NULL,		0 },
	{ S_AUTH1,		SEC_LIST,	chglist,	0 },
	{ S_AUTH2,		SEC_LIST,	chglist,	0 },
	{ S_UMASK,		SEC_INT,	NULL,		0 },
	{ S_REGISTRY,		SEC_CHAR,	NULL,		0 },
	{ S_AUTHSYSTEM,		SEC_CHAR,	NULL,		QUOTES },
	{ S_LOGTIMES,		SEC_LIST,	_dbtouser,	0 },
	{ S_LOGRETRIES,		SEC_INT,	NULL,		0 },
	{ S_PWDWARNTIME,	SEC_INT,	NULL,		0 },
	{ S_LOCKED,		SEC_BOOL,	chgbool,	0 },
	{ S_MINAGE,		SEC_INT,	NULL,		0 },
	{ S_MAXAGE,		SEC_INT,	NULL,		0 },
	{ S_MAXEXPIRED,		SEC_INT,	NULL,		0 },
	{ S_MINALPHA,		SEC_INT,	NULL,		0 },
	{ S_MINOTHER,		SEC_INT,	NULL,		0 },
	{ S_MINDIFF,		SEC_INT,	NULL,		0 },
	{ S_MAXREPEAT,		SEC_INT,	NULL,		0 },
	{ S_MINLEN,		SEC_INT,	NULL,		0 },
	{ S_HISTEXPIRE,		SEC_INT,	NULL,		0 },
	{ S_HISTSIZE,		SEC_INT,	NULL,		0 },
	{ S_PWDCHECKS,		SEC_LIST,	chglist,	0 },
	{ S_DICTION,		SEC_LIST,	chglist,	0 },
	{ S_USREXPORT,		SEC_BOOL,	chgbool,	0 },


	/*
	 * attributes of /etc/security/limits
	 */
	{ S_UFSIZE,		SEC_INT,	NULL,		0 },
	{ S_UCPU,		SEC_INT,	NULL,		0 },
	{ S_UDATA,		SEC_INT,	NULL,		0 },
	{ S_USTACK,		SEC_INT,	NULL,		0 },
	{ S_UCORE,		SEC_INT,	NULL,		0 },
	{ S_URSS,		SEC_INT,	NULL,		0 },
	/*
	 * attributes of /etc/security/environ
	 */
	{ S_SYSENV,		SEC_LIST,	chglist,	QUOTES },
	{ S_USRENV,		SEC_LIST,	chglist,	QUOTES },

	/*
	 * attributes of /etc/security/lastlog
	 */
	{ S_LASTTIME,		SEC_INT,	NULL,		0},
	{ S_ULASTTIME,		SEC_INT,	NULL,		0},
	{ S_LASTTTY,		SEC_CHAR,	NULL,		0},
	{ S_ULASTTTY,		SEC_CHAR,	NULL,		0},
	{ S_LASTHOST,		SEC_CHAR,	NULL,		0},
	{ S_ULASTHOST,		SEC_CHAR,	NULL,		0},
	{ S_ULOGCNT,		SEC_INT,	NULL,		0}

};

int lsusatabsiz = sizeof(lsusatab)/sizeof(struct lsusattr);
