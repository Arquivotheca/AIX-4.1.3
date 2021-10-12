static char sccsid[] = "@(#)39	1.13.1.2  src/bos/usr/bin/que/qsmisc.c, cmdque, bos41J, 9511A_all 3/7/95 11:43:16";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <IN/standard.h>
#include <sys/fullstat.h>
#include <sys/types.h>
#include <IN/backend.h>
#include "common.h"
#include "qstatus.h"
#include <ctype.h>

#include "qstat_msg.h"
#define MSGSTR(num,str)	catgets(catd,MS_QSTAT,num,str)
nl_catd	catd;

extern boolean palladium_inst;


/************************************************************
	ADD A QUEUE POINTER TO THE DESIRED QUEUE LIST
************************************************************/
add_qptr(aq_qptr,aq_dptr,aq_parms)
struct q 	*aq_qptr;	/* queue element to add */
struct d	*aq_dptr;	/* device element to add */
struct allparms	*aq_parms;	/* the linked list */
{
	register struct queptr	*aq_newque;

	/*----Allocate the space for new item */
	aq_newque = Qalloc(sizeof(struct queptr));

	/*----Set the value of the item */
	aq_newque->qp_qptr = aq_qptr;
	aq_newque->qp_dptr = aq_dptr;
	aq_newque->qp_next = aq_parms->ap_queues;
	aq_parms->ap_queues = aq_newque;
	return(0);
}

/**************************************************
	ADD USER NAME TO THE USER-NAME LIST
**************************************************/
add_user(au_name,au_parms)
char		*au_name;
struct allparms	*au_parms;
{
	register struct username *au_newuser;

	/*----Allocate tne space for the new item */
	au_newuser = Qalloc(sizeof(struct username));

	/*----Insert new item into list */
	au_newuser->un_name = au_name;
	au_newuser->un_next = au_parms->ap_users;
	au_parms->ap_users = au_newuser;
	return(0);
}

/**************************************************
	ADD A JOB # TO THE DESIRED-JOB LIST
**************************************************/
add_jobnum(aj_num,aj_parms)
int		aj_num;
struct allparms	*aj_parms;
{
	register struct jobnum *aj_newjob;

	/*----Allocate the space for new item */
	aj_newjob = Qalloc(sizeof(struct jobnum));

	/*----Insert new item into list */
	aj_newjob->jn_num = aj_num;
	aj_newjob->jn_next = aj_parms->ap_jobs;
	aj_newjob->jn_found = FALSE;
	aj_parms->ap_jobs = aj_newjob;
	return(0);
}

/***************************************************************
	CHECK TO SEE IF QUEUE/DEV IS IN DESIRED Q:D LIST
***************************************************************/
queue_desired(qd_que,qd_dev,qd_queues)
struct q	*qd_que;
struct d	*qd_dev;
struct queptr   *qd_queues;
{
	register struct queptr	*qd_thisqd;
	register int		qd_found;

	/*----If list empty, assume q:d is desired */
	if(qd_queues == NULL)
		qd_found = 1;

	/*----Otherwise, search list for que:dev or que:NULL */
	else
		for(qd_thisqd = qd_queues,		qd_found = 0;
		    (qd_thisqd != NULL) &&		(!qd_found);
		    qd_thisqd = qd_thisqd->qp_next)
			if((qd_thisqd->qp_qptr == qd_que) &&
			   ((qd_thisqd->qp_dptr == qd_dev) ||
			    (qd_thisqd->qp_dptr == NULL)))
				qd_found++;
	return(qd_found);
}
			
/****************************************************************
	SEARCH JOB LIST TO SEE IF A CERTAIN JOB IS THERE
****************************************************************/
job_desired(jd_entry,jd_jobs)
struct e	*jd_entry;
struct jobnum	*jd_jobs;
{
	register struct jobnum	*jd_thisjob;
	register int		jd_found;

	/*----If job list is empty, then assume job is desired */
	if (jd_jobs == NULL)
		jd_found = 1;

	/*----Otherwise, search list for the job */
	else
		for (jd_thisjob = jd_jobs,		jd_found = 0;
		     (jd_thisjob != NULL) &&		(!jd_found);
		     jd_thisjob = jd_thisjob->jn_next)
			if (jd_thisjob->jn_num == jd_entry->e_jobnum)
			{
				jd_found = 1;
				jd_thisjob->jn_found = TRUE;
				break;
			}
	return(jd_found);
}

/****************************************************************
	SEARCH USER LIST TO SEE IF CERTAIN USER IS THERE
****************************************************************/
user_desired(ud_entry,ud_users)
struct e 	*ud_entry;
struct username	*ud_users;
{
	register struct username	*ud_thisuser;
	register int ud_found;

	/*----If list is empty, assume user is desired */
	if(ud_users == NULL)
		ud_found = 1;

	/*----Otherwise, search the list for the user */
	else
		for(ud_thisuser = ud_users,		ud_found = 0;
		    (ud_thisuser != NULL) &&		(!ud_found);
		    ud_thisuser = ud_thisuser->un_next)
			if(!strcmp(ud_entry->e_from,ud_thisuser->un_name))
			{
				ud_found = 1;
				break;
			}
	return(ud_found);
}

/******************************************************
	SEARCH QUEUE LIST FOR A PARTICULAR NAME
******************************************************/
get_queue(gq_qname,gq_qlist,gq_q,gq_d)
char		*gq_qname;		/* input- name 2 search 4 */
struct q	*gq_qlist;		/* input- qlist to search */
struct q	**gq_q;			/* output- q that has name or NULL */
struct d	**gq_d;			/* output- d that has name */
{
	register struct q 	*gq_thisq = NULL;
	register struct d 	*gq_thisd = NULL;
	char 			*gq_dname;

	/*----separate the que:dev names */	
	colonsep(gq_qname,&gq_dname);

	/*----Search for a queue name match */
	for(gq_thisq = gq_qlist; 
	    gq_thisq != NULL;
	    gq_thisq = gq_thisq->q_next)
		if (!strncmp(gq_qname,gq_thisq->q_name,QNAME))
		{
			/*----Search for a device name match (if one exists) */
			if (*gq_dname != '\0')
			{
				for (gq_thisd = gq_thisq->q_devlist; 
				     gq_thisd != NULL; 
				     gq_thisd = gq_thisd->d_next) 
					if (!strncmp(gq_dname,gq_thisd->d_name,DNAME))
					{
						*gq_q = gq_thisq;
						*gq_d = gq_thisd;
						return(0); 
					}
			}

			/*----No device name given, just match queue */
			else
			{
				*gq_q = gq_thisq;		
				*gq_d = NULL;
				return(0);
			}
		}

	/*----No match found at all, return a bunch of nothingness */
	*gq_q = NULL;
	*gq_d = NULL;
	return(0);
}

/***********************************************
	SEARCH FOR DEFAULT PRINTER QUEUE
***********************************************/
default_queue(dq_qlist,dq_q,dq_d)
struct q	*dq_qlist;	/* input- qlist 2 search */
struct q	**dq_q;		/* output- q that has PRINTER=name or dq_qlist */
struct d	**dq_d;		/* output- d if user specified PRINTER=name:dev */
{
	register char	*dq_qname;
	extern boolean aixq;

	if((dq_qname = getenv("LPDEST")) || (dq_qname = getenv("PRINTER")))
	{
		get_queue(dq_qname,dq_qlist,dq_q,dq_d);
		if(*dq_q != NULL)
		{
			aixq = TRUE;
			return(0); 
		}
		else if (palladium_inst)
			return(0);	/* assume palladium que if palladium is installed */
	}
	if((dq_qname = getenv("PDPRINTER")) && palladium_inst)
		return(0);

	/*----No environment variable set, give first listed */
	*dq_q = dq_qlist;
	*dq_d = NULL;
	aixq = TRUE;
	return(0);
} 

/***********************************************
	CHECK FOR A STABLE QUEUE (EMPTY, NO HORSES)
***********************************************/
stable_queue(sq_qlist,sq_empty)
struct q	*sq_qlist;
boolean		*sq_empty;
{
	struct q	*sq_thisq;

	*sq_empty = TRUE;
	for(sq_thisq = sq_qlist;
	    sq_thisq != NULL;
	    sq_thisq = sq_thisq->q_next)
		if(sq_thisq->q_entlist != NULL)
			*sq_empty = FALSE;
	return(0);
}

/*****************************************************
	EXTRACT POINTER TO JOB FROM JOB NUMBER
*****************************************************/
struct e *jobnum_to_ent(jt_queue,jt_jobnum)
struct q	*jt_queue;
int		jt_jobnum;
{
	register struct e 	*jt_thisjob;

	/*----Search entry list for match */
	for(jt_thisjob = jt_queue->q_entlist;
	    jt_thisjob != NULL;
	    jt_thisjob = jt_thisjob->e_next)
	{
		if(jt_thisjob->e_jobnum == jt_jobnum)
			break;
	}
	return(jt_thisjob);
}

/***********************************************************************
	SEPARATE QUE:DEV NAME STRING INTO ITS PARTS, QUE AND DEV
***********************************************************************/
colonsep(cs_qname,cs_dname)
char 	*cs_qname;
char	**cs_dname;
{
	/*----Scan up to the colon, if any and change : to \0 */
	for (*cs_dname = cs_qname;
	     **cs_dname != '\0';
             (*cs_dname)++)
		if (**cs_dname == ':')
		{
			**cs_dname = '\0';
			(*cs_dname)++;
			return(0);
		}
	return(0);
}

/******************************************************************
	PRINT ERROR MESSAGE, USAGE MESSAGE, AND EXIT PROGRAM
******************************************************************/
usage()
{
	sysuse( TRUE,
		MSGSTR(MSGUSE1,QMSGUSE1),
		MSGSTR(MSGUSE2,QMSGUSE2),
		(char *)0
	       );
}
