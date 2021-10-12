/* @(#)06	1.12  src/bos/usr/bin/src/include/src.h, cmdsrc, bos411, 9428A410j 12/19/91 13:35:54 */
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
#ifndef _H_SRC
#define _H_SRC

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include <memory.h>
#include <locale.h>
#include <spc.h>

/* size of formated status text */
#define TEXTSIZE	80

/* SRC packet version number */
#define FIRSTBASE	1
#define SRCMSGBASE	FIRSTBASE

#define SRCFIRST	1
#define SRCNEXT		2

#define  SRC_BASE       9000
#define  ODM_BASE       5000
#define  SUBSYS_BASE    1000

/* NLS message catalog */
#define SRCMSGCAT	"src.cat"
/* status literal set */
#define SRCSTATSET	1
/* status header line set */
#define SRCSTATHDR	2
/* message set */
#define SRCERRSET	3

#define ENVSIZE 2402                   /* length of environment strng*/
#define PARMSIZE 2402                   /* length of parameter string */
#define REQSIZE 2000                   /* length of request buffer   */
#define HOSTSIZE 256
#define SRCPATHSZ 200

/* reply time limit from SRC or subsystem */
#define REPLYLIMIT	60

/* Wait time limit for open calls */
#define OPENWAIT        15   /* OPENWAIT * 3 MUST BE < REPLYLIMIT */

/* SRC action types
** user available actions and there valid parameters are defined in < spc.h >
**/
#define REQUEST    3                    /* subsystem request will not
					** start subsystem if it is
					** not already active
					**/
#define REQUESTANDSTART  4              /* subsystem request will start
					** subsystem if it is not already
					** active
					**/
#define NEWHVN     6                    /* post child termination processing */
#define ADDSUBSYS   10			/* new valid subsystem to add */
#define DELSUBSYS   11			/* valid subsystem to be deleted */
#define ADDINETSOCKET 12		/* add internet sockets for remote
					** communication
					**/
#define DELINETSOCKET 13		/* delete internet sockets family
					** remote processing has be removed
					**/

#define RELIMIT    2			/* restart retry limit    */

#define MINPRIORITY	0		/* min valid priority */
#define MAXPRIORITY	39		/* max valid priority */

/* macro definitions */

#define  logerr(str1,err0,err1,err2) \
srcelog(str1, err0, err1, err2, __FILE__, __LINE__)

#define  logiferr(rc,str1,err0,err1,err2) \
{\
if(rc < 0)\
      logerr(str1, err0, err1, err2);\
}

/* short reply from src */
#define shortrep(retaddr,rc) \
{ \
obuf.demnrep =(short)rc;\
dsrcsendpkt(retaddr,&obuf, sizeof(short));\
}

/* exit_on_err will log an error msg and exit the program */
#define  exit_on_err(rc,str1,err0,err1,err2) \
{\
   if(rc < 0)\
   {\
      logerr(str1, err0, err1, err2);\
      exit(1); \
   }\
}

#define odmerrmap(errno,newerrno) \
	(((int)errno == (int)NULL) ? newerrno : odmerrno)

#define odmerrset(errno) \
	(((int)errno == (int)NULL) ? SRC_BASE : ODM_BASE)

#define  BLOCKMASK  (SIGMASK(SIGTERM) | SIGMASK(SIGALRM))

/* generic what token do we use */
#define whattoken(rc, errno, token1, token2) \
	((errno == rc) ? token1 : token2)
#endif
