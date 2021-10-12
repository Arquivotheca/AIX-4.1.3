/* @(#)76	1.7  src/bos/kernel/net/netopt.h, sockinc, bos411, 9428A410j 6/15/94 17:01:13 */
/* 
 * COMPONENT_NAME: (SOCKET) Socket services
 * 
 * FUNCTIONS: ADD_NETOPT, NETOPTINT 
 *
 * ORIGINS: 26 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989 
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */


#define  MAXOPTLEN  128

struct netopt {
	char *name;		/* name of option/symbol */
	short size;		/* size of object in kernel */
	caddr_t obj;		/* address of object in kernel */
	caddr_t deflt;		/* address of default object in kernel */
	char *format;		/* printf format for this option (%s, %d) */
	struct netopt *next;	/* next in linked list */
	int (*init)();		/* option init routine (may be NULL) */
};

struct optreq {
	char name[MAXOPTLEN];	/* name of option/symbol */
	char data[MAXOPTLEN];	/* data area for option value */
	char getnext;		/* flag */
};

extern struct netopt *netopts;
struct netopt *find_netopt();

#define   add_netopt  ADD_NETOPT
#define   ADD_NETOPT(optname, fmt)  \
	  {							\
		  optname##_opt.name = "optname";			\
		  optname##_opt.size = sizeof( optname );		\
		  optname##_opt.obj = (caddr_t)&optname;		\
		  optname##_opt.deflt = (caddr_t)&optname##_dflt;	\
		  optname##_opt.format = fmt;			\
		  optname##_opt.next = NULL;			\
		  add_netoption(&optname##_opt);			\
	  }

#define   del_netopt   DEL_NETOPT
#define   DEL_NETOPT(optname)   delete_netopt(&optname##_opt)

/* declaration for the stuff that describes integer option */
#define  NETOPTINT(x)  struct netopt x##_opt; extern int x, x##_dflt
