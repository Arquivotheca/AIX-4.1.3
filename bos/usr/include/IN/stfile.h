/* @(#)00	1.20  src/bos/usr/include/IN/stfile.h, libIN, bos411, 9428A410j 12/14/93 16:12:28 */
/*
 * COMPONENT_NAME: LIBIN
 *
 * FUNCTIONS:
 *
 * ORIGINS: 9,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_STFILE
#define _H_STFILE

#include <IN/q.h>
#include <sys/types.h>
#define S_TO            42
#define S_FROM          64
#define S_TITLE         100
#define S_LASTUSER      24
#define S_DATE          26
#define S_QNAME		QNAME+1
#define S_DNAME		DNAME+1
#define S_CMDLEN	CMD_LEN+1
#define PIOSTATUS_VERMAGIC	1

struct stfile
{
        int s_jobnum;           /* jobnum of currently running job */
        char s_status;          /* see backend.h */
        unsigned s_align;       /* true or false */
        unsigned s_feed;        /* number of feed pages */
        unsigned s_head;        /* header? */
        unsigned s_trail;       /* trailer? */
        unsigned s_copies;      /* number of copies */
        int s_mailonly;         /* use mail, not write for messages*/
        int s_was_idle;         /* was the printer just idle before this job*/
        char s_percent;         /* percent done */
        short s_pages;          /* pages printed */
        long s_charge;          /* charge for this job */
        char s_qdate[S_DATE];   /* date request was queued */
        char s_to[S_TO];        /* output for this person */
        char s_from[S_FROM];    /* and from this one */
        char s_title[S_TITLE];  /* title of this job */
        char s_device_name[S_DNAME];  /* device stanza name in qconfig file*/
        char s_queue_name[S_QNAME];   /*  queue stanza name in qconfig file*/ 
	/* AIX security enhancement 	*/
	/* added s_uid 			*/
	/* added s_acct, but is unused for now */
	uid_t s_uid;		/* real user id			*/
	/*uid_t s_acct;*/	/* temporary,  unused 		*/
	char s_cmdline[S_CMDLEN];	/* original enq cmd for this job */
};

typedef struct piostatus
{
        int s_jobnum;           /* jobnum of currently running job */
        char s_status;          /* see backend.h */
        unsigned s_align;       /* true or false */
        unsigned s_feed;        /* number of feed pages */
        unsigned s_head;        /* header? */
        unsigned s_trail;       /* trailer? */
        unsigned s_copies;      /* number of copies */
        int s_mailonly;         /* use mail, not write for messages*/
        int s_was_idle;         /* was the printer just idle before this job*/
        char s_percent;         /* percent done */
        short s_pages;          /* pages printed */
        long s_charge;          /* charge for this job */
        char s_qdate[S_DATE];   /* date request was queued */
        char s_to[S_TO];        /* output for this person */
        char s_from[S_FROM];    /* and from this one */
        char s_title[S_TITLE];  /* title of this job */
        char s_device_name[S_DNAME];  /* device stanza name in qconfig file*/
        char s_queue_name[S_QNAME];   /*  queue stanza name in qconfig file*/ 
	/* AIX security enhancement 	*/
	/* added s_uid 			*/
	/* added s_acct, but is unused for now */
	uid_t s_uid;		/* real user id			*/
	/*uid_t s_acct;*/	/* temporary,  unused 		*/
	char s_cmdline[S_CMDLEN];	/* original enq cmd for this job */
} piostatus_t;

#if _NO_PROTO
	extern int piogetstatus();
	extern int pioputstatus();
#else
	extern int piogetstatus(int, int, void*);
	extern int pioputstatus(int, int, const void*);
#endif /* _NO_PROTO */

#endif /* _H_STFILE */
