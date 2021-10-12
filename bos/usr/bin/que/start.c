static char sccsid[] = "@(#)43	1.13.1.1  src/bos/usr/bin/que/start.c, cmdque, bos411, 9428A410j 1/28/93 09:05:54";
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

#include <stdio.h>
#include <IN/standard.h>
#include <sys/types.h>
#include <IN/backend.h>
#include <sys/utsname.h>
#include "common.h"
#include <ctype.h>

#include "qmain_msg.h"
#define MAXSTR 		10
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_QMAIN,num,str)
/*
 * queueing daemon --
 *      routines to start up a new job
 */

struct e *nextjob();
char    *scopy();


/* run through all the devices, putting the idle ones to work */
workdevices(qlist)
register struct q *qlist;
{
	struct q	*ql;
	struct d	*dl;
	boolean		valid_dev;
	char		buf[MAXLINE];

	for (ql = qlist; ql; ql = ql->q_next) 
		for (dl = ql->q_devlist; dl; dl = dl->d_next)
			if (dl->d_pid == 0)
				devstart(dl);
		
}


/* return the device now running process pid */
struct d *devpid(ql,pid)
register struct q *ql;
{       register struct d *dl;

	for ( ; ql; ql = ql->q_next)
		for (dl = ql->q_devlist; dl; dl = dl->d_next)
			if (dl->d_pid == pid)
				return(dl);
	return(NULL);   /* not here */
}


/*
 * start up a job running on device dev.
 * be careful -- device or queue might be down.
 * but we know that nothing is running on this device now
 */
devstart(dev)
register struct d *dev; 
{
	struct e	*e;
	struct stfile	*s;
	extern int 	slowdeath;

	/*----If we are not dying gracefully and device/queue is up, 
	      find an entry to run */
	if ((dev->d_q->q_up == TRUE) && (dev->d_up == TRUE) && !slowdeath)
	{
		if ((e = nextjob(dev->d_q->q_entlist,dev)) != NULL)
		{
			/*----Hook up and try running the next job */
			e->e_device = dev;
			dev->d_e = e;
			dev->d_pid = unixexec(dev,e);
			return(0);
		}
	}

	/*----If we get here, device is idle.  Print separator pages */
	/*    if not remote, feed pages wanted, and the last job was a regular job... */
	if (!remote(dev->d_q->q_hostname)	&&
	    dev->d_feed != NOFEED		&&
	    dev->d_user[0] != '\0')
	{
		/*----Create dummy job with feed setting and set previous user to nil */
		e = Qalloc(sizeof(struct e));
		s = Qalloc(sizeof(struct stfile));
		bzero(e, sizeof(struct e));
		e->e_s = s;
		e->e_from = scopy("root");	/* This feed job is from the system */
		stclean(e->e_s,dev);
		e->e_s->s_feed = dev->d_feed;	/* This indicates a feed job to unixexec */
		dev->d_e = e;			/* Attach this job to the device */
		e->e_device = dev;
		e->e_jdfnum = 0;		/* No jdf args (for sanity) */
		e->e_argnum = 0;		/* sanity */
		dev->d_pid = unixexec(dev,e);
	}
	if (!remote(dev->d_q->q_hostname)	&&
	    dev->d_user[0] != '\0')
		dev->d_user[0] = '\0';		/* To prevent this from happening again */
	return(0);
}

/* given an ordered list of queue entries, return the next one ready */
/* to run on device dev.  must be (1) not running now, and (2) meant */
/* for this dev in particular, or any dev on this queue. */
struct e *nextjob(e,dev)
register struct e *e;
register struct d *dev;
{       register char *dn;        /* device name */
	
	while (e)
	{   
		dn = e->e_dnam;

	    	if (e->e_device == NULL &&          /* not running */
		    e->e_hold == 0 && 		    /* not held */	
		   ((strcmp(dn,ANYDEV) == 0) || (strcmp(dn,dev->d_name) == 0)))
			return(e);
		e = e->e_next;
	}
	return(NULL);
}
