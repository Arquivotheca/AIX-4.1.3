/* @(#)07	1.6  src/bos/usr/bin/src/include/src10.h, cmdsrc, bos411, 9428A410j 2/26/91 15:08:54 */
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1984,1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   


#ifndef _H_SRC10
#define _H_SRC10

/* 
** SRC daemon request structure formats
**/

/* SRC command request information */
struct demnreq
{
	short action;	/* Action requested of SRC */
	short dversion;	/* Packet/message version that will be used
			** to comunicate with SRC and the client
			**/
	int pid;    	/* subsystem process id */
	char subsysname[SRCNAMESZ];	/* Subsystem name */
};

/* Start subsystem request packet */
struct start
{
	struct demnreq demnreq; /* deamon command request information */
	short envlen;           /* length of environment */
	short parmlen;          /* length of arguments */
	short rstrt;		/* allow respawn */
	char parm[PARMSIZE+ENVSIZE];/* subsystem parameters */
};

/* Stop or Status of subsystem request packet */
struct stopstat
{
	struct demnreq demnreq; /* deamon command request information */
	short parm1;		/* first modifier for action */
	short parm2;		/* second modifier for action */
};

/* Child termination notification packet */
struct hvn
{
	struct demnreq demnreq; /* deamon command request information */
	int   svrpid;		/* pid of dead subsystem */
	int   stat;		/* status of dead subsystem */
};

/* Send request packet to subsystem */
struct sndreq
{
	struct demnreq demnreq; /* deamon command request information */
	short replen;           /* length of reply buffer given by client  */
	short reqlen;           /* length of client request */
	char   req [REQSIZE];   /* client request */
};

/* invalid src packet version */
struct srcdver
{
	short rtncode;  /* error code */
	short dversion;	/* Packet/message version that must be used
			** to comunicate with SRC and the client
			**/
};

/* SRC start subsystem reply buffer */
struct strtreply
{
	int	pid;	/* spawned subststem pid */
	char	subsysname[SRCNAMESZ];	/* name of subsystem spawned */
};

#endif
