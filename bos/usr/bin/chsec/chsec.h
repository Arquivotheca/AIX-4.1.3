/* @(#)95	1.1  src/bos/usr/bin/chsec/chsec.h, cmdsuser, bos411, 9428A410j 10/8/93 15:05:41 */
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

extern int uexist(char *);
extern int uexists(char *);
extern int gexists(char *);
extern int chkusw(char *);
extern int chkmkdef(char *);
extern int chkflags(char *, char **);
extern int strtolist(char *, char **);

extern int chkbool(char *, char **);
extern int chktpath(char *, char **);
extern int chknames(char *, char **);
extern int chkusrs(char *, char **);
extern int chkaudit(char *, char **);
extern int chkaud(char *, char **);
extern int chkprog(char *, char **);
extern int chkgrp(char *, char **);
extern int chkgrps(char *, char **);
extern int chkumask(char *, unsigned long *);
extern int chkdata(char *, unsigned long *);
extern int chkstack(char *, unsigned long *);
extern int chkulimit(char *, unsigned long *);
extern int chkint(char *, unsigned long *);
extern int chkexpires(char *, char **);
extern int chkgek(char *, char **);
extern int chkttys(char *, char **);
extern int chkregistry(char *, char **);
extern int chkauthsystem(char *, char **);
extern int chkminage(char *, int *);
extern int chkmaxage(char *, int *);
extern int chkmaxexpired(char *, int *);
extern int chkminalpha(char *, int *);
extern int chkminother(char *, int *);
extern int chkmindiff(char *, int *);
extern int chkmaxrepeats(char *, int *);
extern int chkminlen(char *, int *);
extern int chkhistexpire(char *, int *);
extern int chkhistsize(char *, int *);
extern int chkpwdchecks(char *, char **);
extern int chkdictionlist(char *, char **);

extern char *chglist(char *, char *);
extern char *chgbool(char *, char *);
extern char *chgint(char *, char *);
extern char *chghrld(char *, char *);

extern int _usertodb(char *, char **);
extern char *_dbtouser(char *, char *);

extern int chklock(char *, char *);
extern char *chglock(char *, char *);

struct fileattr
{
	int	filenum;
	char	*filename;
	char	*auditname;
};

struct stanzaattr
{
	int	filenum;
	char	*attr;
	int	type;
	int	(*checkstanza)(char *);
	int	(*checkval)(char *, char **);
	char	*(*changeval)(char *, char *);
	int	qwerks;
};

/*
 * Flags for the qwerks field.
 */

#define NONE 0		/* No qwerks					*/
#define QUOTE 1		/* Value should be quoted			*/
#define FAKE 2		/* Value does not exist in the database - it is *
			 * computed					*/

extern struct fileattr filetable[];
extern struct stanzaattr attrtable[];

