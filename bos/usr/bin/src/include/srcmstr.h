/* @(#)09	1.6  src/bos/usr/bin/src/include/srcmstr.h, cmdsrc, bos411, 9428A410j 6/15/90 23:37:20 */
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
 * (C) COPYRIGHT International Business Machines Corp. 1984,1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   


#ifndef _H_SRCMSTR
#define _H_SRCMSTR

struct activsvr
     {                                 /* Active SUBSYS Table       */
     struct activsvr *next;            /* ptr to next actsvr entry  */
     int   svr_pid       ;             /* server refid, same as PID */
                                       /* for msg type on  requests */
     short recount       ;             /* restart count             */
     short state         ;             /* state of server           */
     long  stoptime      ;             /* time server was stopped   */
     long  warntime      ;             /* time svr was warned tostop*/
     short envlen        ;             /* length of environment strg*/
     short parmlen       ;             /* length of parms string    */
     char  *env;			/* env from shell cmd        */
     char  *parm;			/* parms from shell cmd      */
     struct subsys *subsys;		/* server profile            */
     short action;			/* restart action for this instance
					** of the subsystem
					**/
     struct sockaddr_un sun;		/* afunix socket to communicate
					** with subsystem on
					**/
     };

/* valid subsys table entries */
struct validsubsys
{
     struct subsys subsys;	/* current subsystem */
     struct validsubsys *psubsys;	/* prev subsystem */
     struct validsubsys *nsubsys;	/* next subsystem */
     short forked;		/* number of current subsys forked */
     char deleted;		/* subsytem is to be delete from the
				** the list of vaild subsystems when
				** no occurance of this subsystem
				** is active
				**/
};

/* SRC proper inbound packets */
union ibuf
{
	struct demnreq demnreq;		/* command id
					** all the following structures in this
					** union contain this structure
					**/
	struct start start;		/* START subsystem request packet */
	struct stopstat stopstat;	/* STOP or STATUS requests packet */
	struct hvn hvn;			/* subsystem termination packet */
	struct sndreq sndreq;		/* Request to Subsystem packet */
};

/* SRC proper outbound packets */
union obuf {
	short  demnrep;
	struct strtreply strtreply;
	struct srcdver srcdver;
	struct 
	{
		long mtype;	/* only used on subsys with msg queues */
		struct srchdr srchdr;
		char  req[REQSIZE];	/* Request to be forwarded to
					** a subsystem 
					**/
	} svrreq ;
	struct
	{
		long mtype;	/* only used on subsys with msg queues */
		struct srchdr srchdr;
		struct subreq subreq;	/* SRC request to be sent to
					** a subsystem 
					**/
	} svrstp ;
};

/* global active subsystem table pointers */
extern struct activsvr *frstaste ;     /* Ptr to 1st  actve svr tbl entry*/
extern struct activsvr *curraste ;     /* Ptr to current entry           */
extern struct activsvr *lastaste ;     /* Ptr to last actve svr tbl entry*/

/* global valid subsys table pointers */
extern struct validsubsys *frstsubsys;   /* 1st valid subsys tbl entry */
extern struct validsubsys *lastsubsys;   /* last valid subsys */

/* other globals */
extern struct sockaddr_un src_sock_addr;	/* AF_UNIX SRC socket address */
extern struct sockaddr_un retaddr;		/* Clients return address */

#endif
