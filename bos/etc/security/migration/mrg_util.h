/* @(#)61	1.1  src/bos/etc/security/migration/mrg_util.h, cfgsauth, bos411, 9428A410j 2/18/94 13:47:54 */
/*
 *   COMPONENT_NAME: CFGSAUTH
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* 4.1 File Locations */
#define	LOGIN41		"/etc/security/login.cfg"
#define	USER41		"/etc/security/user"

/* Migration Dir */
#define	MIGDIR		"/tmp/bos/etc/security"

/* 3.2 (And Config) File Names */
#define	LOGIN32		"login.cfg"
#define	NEWLOGIN41	"login.cfg.new"
#define	PWRESTSAVE	"pwrest.41"
#define	USER32		"user"
#define	NEWUSER41	"user.new"
#define	TMPFILE		"secur.tmp"

extern FILE* getfile(char *, char *);

