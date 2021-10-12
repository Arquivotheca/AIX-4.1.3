/* @(#)08	1.6  src/bos/usr/bin/src/include/src11.h, cmdsrc, bos411, 9428A410j 2/26/91 15:09:05 */
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

#ifndef _H_SRC11
#define _H_SRC11


/* subsystem structure definition */
struct subsys {
	char  subsysname [SRCNAMESZ];	/* subsystem name */
	char  synonym [SRCNAMESZ];	/* subsystem synonym */
	char  cmdargs [SRCPATHSZ];	/* subsystem command arguments */
	char  path [SRCPATHSZ];		/* path to executable */
	long uid;			/* UID for subsystem */
	long auditid;			/* Audit id for subsystem */
	char  standin [SRCPATHSZ];	/* standard input file */
	char  standout [SRCPATHSZ];	/* standard output file */
	char  standerr [SRCPATHSZ];	/* standard error file */
	short action;			/* respawn action */
	short multi;			/* multi instance support  */
	short contact;			/* contact type of of
					**  1. signals
					**  2. message queues
					**  3. sockets
					**/
	long svrkey;			/* IPC queue key TEMPORARY     */
	long  svrmtype;			/* IPC mtype for subsystem IPC    */
	short priority;			/* nice value 1-40 */
	short signorm;			/* stop normal */
	short sigforce;			/* stop force */
	short display;			/* display inactive on all or
					** group status 
					**/
	short waittime;			/* stop cancel time to wait before
					** sending a sigkill to the subsystem
					**/
	char grpname[SRCNAMESZ];	/* subsystem group name */
};

#endif
