static char sccsid[] = "@(#)36	1.29  src/bos/usr/bin/que/qcom.c, cmdque, bos411, 9428A410j 1/28/93 10:14:34";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 9, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/* Routines shared between qdaemon and qstatus but not used by enq.	*/
/* 	All for reading the qdir.					*/
 
#include <fcntl.h>
#include <stdio.h>
#include <sys/param.h>
#include <ctype.h>
#include <IN/standard.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include "common.h"
#include "enq.h"
#include <IN/backend.h>
#include <sys/vmount.h>


#include <nl_types.h>
#include "qmain_msg.h"
#include "qstat_msg.h"
#define MAXSTR 		10
#define MSGSTR(num,str)	catgets(catd,MS_QMAIN,num,str)
nl_catd	catd;

char *getqn();

extern  boolean qdaemon;	/* GLOBAL CONTROL FLAG IN COMMON.C*/
/* qiven a single queue q, and a queue entry e, add the entry to 
   this queue in the proper place on its entry list 
	find the right place in the q->q_entlist and insert
*/
qadd(q,e)
register struct q *q;
struct e *e;
{
        register struct e *p1, *p2;
        int compare;


        if (q->q_entlist == NULL || cmppri(e,q->q_entlist) == -1)
        {
                /* we're at the head of the queue */
                e->e_next = q->q_entlist;
                q->q_entlist = e;
                return(0);
        }
        /*
	 * insert at the right spot, highest priority first. 
         * if 2 have equal priorities, then sort by time.
         * when the loop finishes, e belongs between p1 and p2
	 */
        for (p1 = q->q_entlist; (p2=p1->e_next) != NULL; p1 = p2)
        {
                compare = cmppri(e,p2);
                if (compare == -1)              /* e before p2 */
                        break;
        }
	p1->e_next = e;
  	e->e_next = p2;
}



/* make an entry structure corresponding to the file named name */
/* include pri2, which is time if discipline is 'f' and size if it's */
/* 's' (fcfs and sjn). */
struct e *emake(q,name)
char *name;
struct q *q;
{       register struct e *e;
	int gjdfreslt;		/* result of getjdf() call */

	e = Qalloc(sizeof(struct e));
	bzero((char *)e,sizeof(struct e)); 	/* nuke e struct */

	strcpy(e->e_name,name);          /* add name */

	gjdfreslt = getjdf(q,e);
	if (gjdfreslt != OK)	/*  read jdf into e struct  */
        {
		if (gjdfreslt == NOTOK)
                	syswarn(MSGSTR(MSGGJDF,"Bad getjdf on name %s."),name);
		jdffree(e);
		free( (void *)e );
		return(NULL);
        }

	switch(q->q_disc)
	{
	    case FCFS:
		e->e_pri2 = e->e_time;
		break;
	    case SJN:
		e->e_pri2 = e->e_size;
		break;
	}
	return(e);
}




/*
 * put the queue entries onto qlist
 * (used for "enq -q", also for qdaemon).
 * if qarg non-NULL, want this queue only; else want all queues.
 * caller must ensure that we chdir to here first.
 * spec_req is the function we jump to in qdaemon when we encounter a 
 * special request.  Null means ignore special requests.
 */
readqdir(qarg,qlist,spec_req)
register struct q *qarg, *qlist;
int (*spec_req)();
{
       	DIR             *dirp;
       	struct dirent   *dire;	
 
       	if ((dirp=opendir(".")) == NULL)
		syserr((int)EXITFATAL,MSGSTR(MSGFQDR,"Failure to read queue directory."));

	/* keep adding names as long as they keep coming. */
        while ((dire = readdir(dirp)) != NULL)
	{
		char *name;

		name = dire->d_name;
		if (name[0] == '.')     /* . or .., we don't care about*/
			continue;
		if (name[0] == 't')     /* in progress -- leave it alone */
			continue;
		if (name[0] == 'r')     /* special request entry */
		{
			if (qdaemon)	/* GLOBAL CONTROL FLAG. */
			{
				if (spec_req != NULL)
				{
					(*spec_req)(dire->d_name,qlist);
					continue;
				}
			}
			else
				continue;
		}
		if( qarg == NULL )
			queueit(name, qlist, FALSE);
		else
		{
			if( strncmp(qarg->q_name,getqn(name),QNAME) == 0)
				queueit(name,qarg, FALSE);
		}
	}
	closedir(dirp);
	return(0);
}

/* add entry named name somewhere in queue list ql */
queueit(name, ql, quick)
register struct q *ql;
register char *name;
register boolean quick;
{
	register struct q *qlp;
	struct e *emake();
	char qname[QNAME];

	/* quick is set when we have received a message from enq and there
	 * is no need for checking the filename
	 */
	if ( !quick ) {
		if (strcmp(name,"core") == 0)
		{
			if (qdaemon)	/* * GLOBAL CONTROL FLAG. */
			{
				syswarn(MSGSTR(MSGCORE,"Core image in qdir directory. (Moving to .core)"));
				link("core",".core");
				unlink("core");
			}
			return(0);
		}
		else
		{
			if (strcmp(name,".core") == 0)
				return(0);
		}
	}


	strncpy(qname, getqn(name), QNAME);
	for (qlp = ql; qlp; qlp = qlp->q_next)
	{
	    if( strncmp(qname,qlp->q_name,QNAME) == 0 )
	    {
		if ( !qdaemon ) {
		    struct e *e;

		    e = emake(qlp,name);
		    if( e != NULL)
			    qadd(qlp,e);
		}
		else {
		    if ( qlp->q_top == FALSE )
		    {
			struct e *e;
			e = emake(qlp,name);
			if( e != NULL) {
			    if (++qlp->q_entcount >= HICOUNT) {
				qlp->q_top = TRUE;
			    }
			    qadd(qlp,e);
			}
		    }
		}
		return(0);
	    }
	}

	/* no list for this queue. */
	if (qdaemon)
		syswarn(MSGSTR(MSGNOQU,"No queue %s in %s (name = %s/%s)"),
			getqn(name),CONFIG,QUEDIR,name);
	else
		syswarn(MSGSTR(MSGNQU,"No queue %s in %s (name = %s/%s)"),
			getqn(name),CONFIG,QUEDIR,name);
}

