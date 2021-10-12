/* @(#)92	1.11.1.3  src/bos/usr/include/userpw.h, cmdsauth, bos411, 9428A410j 5/9/94 16:44:08 */
/*
 * COMPONENT_NAME: (CMDSAUTH)
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_USERPW    
#define _H_USERPW

#include <standards.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/limits.h>

/* function declarations	*/
#ifdef _NO_PROTO
	extern	struct userpw  	*getuserpw ();
	extern	int		putuserpw ();
	extern	int		setpwdb ();
	extern	int		endpwdb ();
#else	
	extern	struct userpw  	*getuserpw (char *);
	extern	int		putuserpw (struct userpw *);
	extern	int		setpwdb (int);
	extern	int		endpwdb (void);
#endif	/* _NO_PROTO */

/* manifest constants and limits	*/
#define	MAX_PASS	PASS_MAX

#define PW_PASSLEN	8		/* maximum length of a password */
#define PW_NAMELEN	L_cuserid	/* max length of a user's name */
#define PW_CRYPTLEN	16		/* length of the encrypted password */

#define	MIN_MINAGE	 0		/* minimum minage     value */
#define	MAX_MINAGE	52		/* maximum minage     value */
#define	MIN_MAXAGE	 0		/* minimum maxage     value */
#define	MAX_MAXAGE	52		/* maximum maxage     value */
#define	MIN_MAXEXP     (-1)		/* minimum maxexpired value */
#define	MAX_MAXEXP	52		/* maximum maxexpired value */
#define	MIN_MINALPHA	 0		/* minimum minalpha   value */
#define	MAX_MINALPHA	PW_PASSLEN	/* maximum minalpha   value */
#define	MIN_MINOTHER	 0		/* minimum minother   value */
#define	MAX_MINOTHER	PW_PASSLEN	/* maximum minother   value */
#define	MIN_MINDIFF	 0		/* minimum mindiff    value */
#define	MAX_MINDIFF	PW_PASSLEN	/* maximum mindiff    value */
#define	MIN_MAXREP	 0		/* minimum maxrepeats value */
#define	MAX_MAXREP	PW_PASSLEN	/* maximum maxrepeats value */
#define	MIN_MINLEN	 0		/* minimum minlen     value */
#define	MAX_MINLEN	PW_PASSLEN	/* maximum minlen     value */
#define	MIN_HISTSIZE	 0		/* min value for histsize attribute */
#define	MAX_HISTSIZE	50		/* maximum number of passwords kept */
#define	MIN_HISTEXPIRE	 0		/* min value for histexpire attribute */
#define	MAX_HISTEXPIRE	260		/* maximum interval is 5 years */

#define	PWDCHECKS_LIBPATH	"/usr/lib"


/* flags for user password */
#define	PW_NOCHECK	0x1
#define	PW_ADMCHG	0x2
#define	PW_ADMIN	0x4
#define	PW_NOMINAGE	0x8		/* Obsolete */

struct	userpw
{
	char	upw_name[PW_NAMELEN];	/* user's name */
	char	*upw_passwd;		/* user's passwd */
	ulong	upw_flags;		/* flags of restrictions */
	ulong	upw_lastupdate;		/* date of last passwd update */
};

#endif /* _H_USERPW */
