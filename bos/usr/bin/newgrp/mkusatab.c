/* @(#)89	1.7  src/bos/usr/bin/newgrp/mkusatab.c, cmdsuser, bos41J, 9512A_all 3/14/95 15:55:35 */
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

struct	mkusattr	mkusatab [ ] =
{
	{ SC_AUDIT,	NULL,	S_AUDITCLASSES,	NULL,	chkaudit},

	{ SC_PROG,	NULL,	S_SHELL,	NULL,	chkprog},

	{ SC_GECOS,	NULL,	S_GECOS,	NULL,	NULL},

	{ SC_USRENV,	NULL,	S_USRENV,	NULL,	NULL},

	{ SC_SYSENV,	NULL,	S_SYSENV,	NULL,	NULL},

	{ SC_LOGINCHK,	NULL,	S_LOGINCHK,	NULL,	chkbool},

	{ SC_SUCHK,	NULL,	S_SUCHK,	NULL,	chkbool},

	{ SC_RLOGINCHK,	NULL,	S_RLOGINCHK,	NULL,	chkbool},

	{ SC_TELNETCHK,	NULL,	S_TELNETCHK,	NULL,	chkbool},

	{ SC_DAEMONCHK,	NULL,	S_DAEMONCHK,	NULL,	chkbool},

	{ SC_TPATH,	NULL,	S_TPATH,	NULL,	chktpath},

	{ SC_TTYS,	NULL,	S_TTYS,		NULL,	chkttys},

	{ SC_SUGROUPS,	NULL,	S_SUGROUPS,	NULL,	chkmgrps},

	{ SC_ADMGROUPS,	NULL,	S_ADMGROUPS,	NULL,	chkmkadmgroups},

	{ SC_EXPIRATION,NULL,	S_EXPIRATION,	NULL,	chkexpires},

	{ SC_FSIZE,	NULL,	S_UFSIZE,	NULL,	(int(*)(char *,char **))chkmkulimit},

	{ SC_CPU,	NULL,	S_UCPU,		NULL,	NULL},

	{ SC_DATA,	NULL,	S_UDATA,	NULL,	(int(*)(char *,char **))chkmkdata},

	{ SC_STACK,	NULL,	S_USTACK,	NULL,	(int(*)(char *,char **))chkmkstack},

	{ SC_CORE,	NULL,	S_UCORE,	NULL,	NULL},

	{ SC_RSS,	NULL,	S_URSS,		NULL,	NULL},

	{ SC_UMASK,	NULL,	S_UMASK,	NULL,	(int(*)(char *, char **))chkmkumask},

	{ SC_REGISTRY,	NULL,	S_REGISTRY,	NULL,	chkregistry },

	{ SC_AUTHSYSTEM, NULL, 	S_AUTHSYSTEM,	NULL,	chkauthsystem },

	{ S_LOGTIMES,	NULL,	S_LOGTIMES,	NULL,	_dbtomkuser},

	{ S_LOCKED,	NULL,	S_LOCKED,	NULL,	chkbool},

	{ S_LOGRETRIES,	NULL,	S_LOGRETRIES,	NULL,	NULL},

	{ S_PWDWARNTIME,NULL,	S_PWDWARNTIME,	NULL,	NULL},

	{ SC_MINAGE,	NULL,	S_MINAGE,	NULL,	chkmkminage},

	{ SC_MAXAGE,	NULL,	S_MAXAGE,	NULL,	chkmkmaxage},

	{ SC_MAXEXPIRED,NULL,	S_MAXEXPIRED,	NULL,	chkmkmaxexpired},

	{ SC_MINALPHA,	NULL,	S_MINALPHA,	NULL,	chkmkminalpha},

	{ SC_MINOTHER,	NULL,	S_MINOTHER,	NULL,	chkmkminother},

	{ SC_MINDIFF,	NULL,	S_MINDIFF,	NULL,	chkmkmindiff},

	{ SC_MAXREPEAT,	NULL,	S_MAXREPEAT,	NULL,	chkmkmaxrepeats},

	{ SC_MINLEN,	NULL,	S_MINLEN,	NULL,	chkmkminlen},

	{ SC_HISTEXPIRE,NULL,	S_HISTEXPIRE,	NULL,	chkmkhistexpire},

	{ SC_HISTSIZE, 	NULL,	S_HISTSIZE,	NULL,	chkmkhistsize},

	{ SC_PWDCHECKS,	NULL,	S_PWDCHECKS,	NULL,	chkpwdchecks},

	{ SC_DICTION,	NULL,	S_DICTION,	NULL,	chkdictionlist},

	{ SC_USREXPORT,	NULL,	S_USREXPORT,	NULL,	chkbool}

};

int mkusatabsiz = sizeof(mkusatab)/sizeof(struct mkusattr);
