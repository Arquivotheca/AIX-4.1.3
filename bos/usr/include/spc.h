/* @(#)43	1.11  src/bos/usr/include/spc.h, cmdsrc, bos411, 9428A410j 2/2/94 12:39:47 */
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1984,1994
 * All Rights Reserved
 * Licensec Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* System Resource Controler - Subsystem Process Control include file
**    all subsystems that SRC will control using sockets or message queues
**    should include this file
**/
#ifndef _H_SPC
#define _H_SPC

#include <sys/types.h>
#include <sys/un.h>

#define SRCNAMESZ	30

#define SRCPKTMAX	8192		/* maximum packet size */

/* Reserved obect types */
#define SUBSVR		16
#define SUBSYSTEM	17

/* spawning actions */
#define   RESPAWN  1                /* restart subsystem if it ends   */
#define   ONCE     2                /* do not restart subsystem       */

/* contact options */
#define   SRCIPC	1                /* contact is IPC */
#define   SRCSIGNAL	2                /* contact is signal */
#define   SRCSOCKET	3                /* contact is SOCKETS */

/* SRC true/false */
#define   SRCOFF	0
#define   SRCON		1
#define   SRCNO		SRCOFF
#define   SRCYES        SRCON

#define   TIMELIMIT 	20

/* Respawn as indicated from subsystem object class */
#define RESP_USE     0 
/* Respawn is not allowed */
#define RESP_IGNORE  1

/* SRC action types 
**	SRC reservers the range 0 - 255 for SRC commands
**/
#define START      0
#define STOP       1
#define STATUS     2
#define SRCSTATUS  STATUS
#define TRACE  	   4
#define REFRESH    5

/* Parameter values for parm1 & parm2 */

/* stop normal subsystem or subsvr */
#define NORMAL     0
/* stop forced subsystem or subsvr */
#define FORCED     1
/* stop cancel (subsystem only) */
#define CANCEL     2

#define TRACEOFF	0
#define TRACEON		1

#define SHORTSTAT	0
#define LONGSTAT	1

#define SHORTTRACE	SHORTSTAT
#define LONGTRACE	LONGSTAT

/* subsystem reply is continued
**	CONTINUED from subsystem means that rtnmsg is just to be printed and
**		another packet will follow this one.
**	STATCONTINUED from subsystem means that another packet will
**		follow this one (status only).
**/
#define CONTINUED  1
#define STATCONTINUED 2

/* new request indicator */
#define   NEWREQUEST	3

/* last subsystem response packet indicator */
#define END        0

/* define subsystem functions that return other than an int */
struct srchdr *srcrrqs();

/* src header to be included in all packets that are sent to and 
** received from any subsystem
** In AIX 4.1, added a 4-byte pad to keep the srchdr structure the
** same length as in 3.2.  (The sockaddr_un structure changed.)
**/
struct srchdr 
{
	struct sockaddr_un retaddr;	/* return address for packets */
	char pad[4];                    /* Adjust for 4.1 change to socket */
	short dversion;			/* SRC packet version */
	short cont;			/* continuation indicator */
};

/* subsystem request */
struct subreq 
{
	short object;		/* object type  one of
			        **    1. SUBSYSTEM 
				**    2. subserver code point as defined
				**	 in subserver object class
				**/
	short action;		/* action requested of subsystem */
	short parm1;		/* first modifier for subsystem action */
	short parm2;		/* second modifier for subsystem action */
	char objname[SRCNAMESZ];
				/* name of object that request applies to
				** 	1. subsystem name 
				**	2. subserver object
				**	3. subserver pid
				**/

};

/* Subsystem request receipt */
struct srcreq
{
#ifdef SRCBYQUEUE
	long mtype;		/* mtype for message queue subsystems */
#endif
	struct srchdr srchdr;
	struct subreq subreq;
};

/* subsystem reply */
struct svrreply
{
	short rtncode;		/* subsystems response to the request
				** negative on subsystem error
				** or subsystem unique message returned
				**/
	short objtype;		/* status object type  one of
			        **    1. SUBSYSTEM 
				**    2. subserver code point
				**    3. error code
				**/
	char  objtext[65];	/* text discription to be included in
				** response of subsystem action
				**/
	char  objname[SRCNAMESZ];
				/* name of object (subsystem/subsever) that
				** this response belongs to
				*/
	char rtnmsg[256];	/* subsystem unique message */
};

/* 
**	Subsystem Status Reply
**/
struct statcode 
{
	short objtype;		/* status object type  one of
			        **    1. SUBSYSTEM 
				**    2. subserver code point
				**    3. error code
				**/
	short status;		/* status code */
	char  objtext[65];	/* text discription to be included in
				** printing status for this object
				**/
	char  objname[SRCNAMESZ];
				/* name of object (subsystem/subsever) that
				** this status belongs to
				*/
};

/* Subsystem reply */
struct srcrep
{
	struct srchdr srchdr;
	struct svrreply svrreply;
};

/* Subsystem status reply */
struct statrep
{
	struct srchdr srchdr;
	struct statcode statcode[1];
};


/* Valid SRC status id's */

#define SRCACT    1                     /* started  & active          */
#define SRCINAC   2                     /* inactive                   */
#define SRCSTPG   3                     /* stopping                   */
#define SRCSTAR   6                     /* starting                   */
#define SRCSUSP   4                     /* suspending                 */
#define SRCWARN   5                     /* warned to stop             */
#define SRCOPND   7                     /* opened                     */
#define SRCLOSD   8                     /* closed                     */
#define SRCOPPN   9                     /* open pending               */
#define SRCLSPN  10                     /* close pending              */
#define SRCINOP  11                     /* inoperative                */
#define SRCOBIN  12                     /* open but inactive          */
#define SRCNOSTAT  13                   /* no status         */
#define SRCTSTPN   14                   /* test pending           */
#define SRCTST     15                   /* test active            */

#define FIRST_STATUS_SRC	SRCACT
#define LAST_STATUS_SRC		SRCTST

/* Is the process that will print the error/informative message
**	a shell process or a background deamon process
**	
*/
#define SDEAMON     0
#define SDAEMON     0
#define SSHELL      1

/* group charactor indicator */
#define SRCGROUP	""	/* group char is a '^A' */

/* all subsystems charactor indicator */
#define SRCALLSUBSYS	""	/* all char is a '^B' */


/* error code definitions for daemon support */
#include <srcerrno.h>

#endif
