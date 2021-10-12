static char sccsid[] = "@(#)65	1.10  src/bos/usr/bin/src/cmds/srcdsrt.c, cmdsrc, bos411, 9428A410j 2/26/91 14:51:07";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	srcdsrt,getsubsys
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



#include "src.h"                         /* SVR request/reply struct  */
#include <signal.h>                      /* IPC message structures        */
#include <fcntl.h>		 /* UNIX file control	*/
#include "src11.h"                       /* subsystem profile                */
#include "src10.h"                       /* RTL request/reply   structure */
#include "srcmstr.h"
#include "srcaudit.h"


int srcdsrt (key,subsysname,envlen,env,parmlen,parm,rstrt)
char  *key;
char *subsysname;
unsigned int rstrt;
short envlen;
short parmlen;
char *env;
char *parm;
{
	extern int  fork()    ;         /* Extern for fork                */
	extern int  srcdchd() ;         /* child process                  */
	extern void exit()    ;         /* terminate this process         */
	extern void free_activsvr();


	int     ret;
	int     status;
	int     fdes[2];

	int    rc;


	/* allocate a new active server table entry */
	curraste = (struct activsvr *)(malloc (sizeof(struct activsvr)));
	if (curraste ==  NULL)
		return(SRC_MMRY);
	bzero(curraste,sizeof(struct activsvr));
	/* environment to be set up */
	if(envlen>0)
	{
		curraste->env = (char *)malloc(envlen+1);
		if (curraste->env ==  NULL)
		{
			free_activsvr(curraste);
			return(SRC_MMRY);
		}
		strncpy(curraste->env, env, (int)envlen);
		curraste->env[envlen]='\0';
		curraste->envlen = envlen;
	}
	/* command arguments to be set up */
	if(parmlen>0)
	{
		curraste->parm = (char *)malloc(parmlen+1);
		if (curraste->parm ==  NULL)
		{
			free_activsvr(curraste);
			return(SRC_MMRY);
		}
		strncpy(curraste->parm, parm, (int)parmlen);
		curraste->parm[parmlen]='\0';
		curraste->parmlen = parmlen;
	}

	/* get the subsystem from the valid subsystem table */
	rc=getsubsys(key,(char *)&curraste->subsys);

	/* did we find a validsubsystem? */
	if ( rc <= 0 )
	{
		free_activsvr(curraste);
		return(rc);
	}

	/* found validsubsystem we may continue starting it */

	/* do we want to tell the caller what the name
	** of the subsystem we are starting is?
	**/
	if(subsysname!=(char *) 0)
		strcpy(subsysname,curraste->subsys->subsysname);

	/* are we allowed to RESPAWN if our subsystem wants to? */
	if (rstrt == (short) RESP_IGNORE)
		curraste->action = ONCE;
	else
		curraste->action = curraste->subsys->action;

	/* Check for multiple instance support
	** do not allow multiple instances of the subsystem
 	**    1. when multi is no and subystem exists 
 	**    2. when contact is by IPC and any process is already using 
	**	 the queue and either process has multi set to no
 	**/
	if (curraste->subsys->multi == SRCNO || curraste->subsys->contact == SRCIPC)
	{
		if(srcstrtfind())
		{
			free_activsvr(curraste);
			return(SRC_MULT);
		}
	}

	/* create a pipe to receive errors from the subsystem on */
	if (pipe(fdes) == -1)
		return(SRC_PIPE);

	/* Close pipe[1] on success exec  */
	fcntl(fdes[1], F_SETFD, 1);

	/* get subsystems socket address? if so desired */
	if (curraste->subsys->contact == SRCSOCKET)
		srcafunixsockaddr(&curraste->sun,1);

	/* spawn our subsystem to be named later */
	rc=fork();

	switch (rc)
	{
	case -1:
		/* for failed, close our pipe, and return */
		close(fdes[1]); /* Close write part of pipe    */
		close(fdes[0]); /* Close read  part of pipe    */
		free_activsvr(curraste);/* free entry in active svr tbl*/
		return(SRC_FEXE);

	case 0 :
		/* we are the child process close read end of the pipe since
		** will never read from it
		**/
		close(fdes[0]);

		/* turn our child process into the subsystem */
		srcdchd(fdes[1]);
		/* we will never return to here since srcdchd will
		** either exec the subsystem successfuly or will
		** fail and exit (however if pays to play it safe)
		**/
		exit(1);

	default:
		/* we are the parent process, proud papa of a subsystem to
		** be named later, close the write portion of our pipe
		** since we will never write to it.
		**/
		close(fdes[1]);

		/* wait for our subsystem to come to life */
		while ((ret = read(fdes[0], &status,
		    sizeof(status))) == -1 &&
		    errno == EINTR);
		close(fdes[0]); 		/* Close read  part of pipe */

		/* did the read fail? */
		if ( ret == -1 )
		{
			free_activsvr(curraste);
			return(SRC_PIPE);
		}
		/* did our subsystem fail on exec? */
		else if ( ret )
		{
			free_activsvr(curraste);
			return(status);
		}

		/* the subsystem execed ok  (it's alive) */

		(void) sigblock(BLOCKMASK);

		/* update our active subsystem table */
		curraste->svr_pid = rc;          /* put svr PID in table      */
		curraste->state  = SRCACT;       /* subsystem is active       */
		curraste->recount   = 0;      	 /* null out restart count    */
		curraste->stoptime = (long)NULL; /* null out stop time        */
		curraste->warntime = (long)NULL; /* null out warned time      */
		curraste->next = NULL;           /* this one doesn't point    */

		/* remember that we have a new instance of this subsys running */
		((struct validsubsys *)curraste->subsys)->forked++;


		/* do we have any subsystem out there in an active like state?
		** 	when we do the last subsystem on the list will then
		**		point to this subsystem
		**	when we don't the first on the list pointer will then
		**		point to this subsystem
		**
		** NOTE: active subsystem table is only a singlely linked list.
		**/
		if (frstaste != NULL) 
		{
			/* place new subsystem on the rear end of the list */
			lastaste->next = curraste;
		}
		else
		{
			/* no active subsystem so set the start of list to
			** this subsystem
			**/
			frstaste = curraste;
		}

		/* new subsystems are always put on the back end of the list */
		lastaste =  curraste;

		(void) sigsetmask(0);           /* release signals */

		auditlog(AUDIT_SRC_START,0,curraste->subsys->subsysname,
		   strlen(curraste->subsys->subsysname));
		
		return(rc);
	}
}
/* find a valid subsystem */
int getsubsys(key,buff)
char *key;
struct validsubsys **buff;
{
	static struct validsubsys *ptr=0;
	static char lastkey[SRCNAMESZ];

	/* do we want to reset our key? */
	if(key!=(char *)0)
	{
		ptr=frstsubsys;
		strcpy(lastkey,key);
	}
	/* we want the next validsubsys that matches */
	else if(ptr!=(struct validsubsys *)0)
		ptr=ptr->nsubsys;


	/* find the validsubsys that we want */
	for(;ptr!=(struct validsubsys *)0;ptr=ptr->nsubsys)
	{
		/* are we looking for a group of subsystems */
		if(key==(char *)0 || strncmp(key,SRCGROUP,1)==0)
		{
			/* is this subsystem in our group? */
			if(strcmp(&lastkey[1],ptr->subsys.grpname) == 0)
			{
				*buff=ptr;
				return(1);
			}
		}
		/* we are looking for a specific subsystem */
		else
		{
			/* is this subsystem we want? */
			if(strcmp(key,ptr->subsys.subsysname) == 0 && ptr->deleted != (char)TRUE)
			{
				*buff=ptr;
				return(1);
			}
		}
	}

	/* sorry we did not find a subsystem. it must be an error? */
	return(SRC_SVND);
}
