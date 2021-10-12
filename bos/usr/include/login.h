/* @(#)94	1.8  src/bos/usr/include/login.h, cmdsauth, bos411, 9428A410j 5/9/94 16:45:41 */
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

#ifndef _H_LOGIN    
#define _H_LOGIN

/*
 * Security library function definitions
 */

#ifdef	_NO_PROTO
int	ckuserID();
#else	/* ! _NO_PROTO */
int	ckuserID (char *, int);
#endif	/* _NO_PROTO */

/* prolog */

/* modes for checking the user account and identification */
#define	S_LOGIN		0x1		/* check for logins */
#define S_RLOGIN	0x2		/* check for remote logins */
#define	S_SU		0x4		/* check for su */
#define S_TSM		0x8		/* check for Terminal state manager */
#define S_DAEMON	0x10		/* check for allowing batch programs */

#define	S_PRIMARY	0x1		/* primary authentication requested */
#define	S_SECONDARY     0x2		/* secondary authentication requested */

/* for lastlog structure */
#define LL_LINELEN	32		/* terminal path length */
#define LL_HOSTLEN	32		/* hostname length */

/* struct to record last logins */
struct lastlog
{
	long	ll_time;		/* time of last login */
	char	ll_line[LL_LINELEN];	/* terminal where last login took place */
	char	ll_host[LL_HOSTLEN];	/* hostname where last login took place */
};
#endif /* _H_LOGIN */
