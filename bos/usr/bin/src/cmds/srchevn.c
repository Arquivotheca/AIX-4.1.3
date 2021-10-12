static char sccsid[] = "@(#)72	1.7.1.1  src/bos/usr/bin/src/cmds/srchevn.c, cmdsrc, bos411, 9428A410j 2/2/94 12:35:05";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	srchevn,newhevn
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1984,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



#include "src.h"
#include <sys/signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/resource.h>

#include "src11.h"                       /* subsystem profile structure      */
#include "src10.h"                       /* active subsystem table structure */
#include "srcmstr.h"
#include "srcaudit.h"

static pid_t src_wait(stat)
int *stat;
{
	struct rusage rusage;
	pid_t pid;

        /* get pid of child proces */
	do {
		pid = wait3(stat,WNOHANG,&rusage); 
	} while (pid == -1 && errno == EINTR);

	return(pid);
}


void srchevn ()
{
	/*--------------------------------*/
	/* LOCAL VARIABLES                */
	/*--------------------------------*/
	int    stat_loc           ;     /* Status of subsystem which ended   */
	int    svrpid             ;     /* process id of svr which ended  */

	struct hvn newhvn;


	/* get a terminated child pid and send a termination packet
	 * to our self
	 */
	while((svrpid=src_wait(&stat_loc)) > 0)
	{
		(void) memset(&newhvn, 0, sizeof(newhvn));
		newhvn.demnreq.action   = NEWHVN;
		newhvn.demnreq.dversion = SRCMSGBASE;
		newhvn.svrpid           = svrpid;
	
		newhvn.stat   = stat_loc;
	
		/* send msg telling of the child death */
		dsrcsendpkt(&src_sock_addr,&newhvn,sizeof(newhvn));
	}
	
	return;
}


void newhevn(svrpid, svr_stat)
int     svrpid;
int     svr_stat;
{

	extern int  fork()    ;         /* Extern for fork                */
	extern int  srcdchd() ;         /* Extern for child processing    */
	extern void  srcelog() ;         /* Extern for error log     */
	extern void  srcdsvr() ;         /* Extern to delete svr entry*/
	extern long time()    ;         /* Extern for current time        */
	extern void srcnotify();
	/*--------------------------------*/
	/* LOCAL VARIABLES                */
	/*--------------------------------*/
	int    fdes[2]            ;     /* File descriptors of open pipe  */
	int    ret                ;     /* Return code from read of pipe  */
	int    status             ;     /* Status of failed execle()      */
	int    stat               ;     /* Status of subsystem which ended   */
	int    rc                 ;     /* Return code variable           */
	struct activsvr *prevaste ;     /* Ptr to previous aste           */
	struct activsvr *ptr      ;     /* Ptr to an   actve svr tbl entry*/
	long   now                ;     /* current time                   */

	char grpname[SRCNAMESZ];
	char subsysname[SRCNAMESZ];

	union {
		int   stat;
		char  status[4];
	} status_info;

	/* return if there are no active subsystems	*/
	if (frstaste == NULL)
                return;

	/*-------------------------------------------------------------------*/
	/*  Get current time                                                 */
	/*-------------------------------------------------------------------*/

	now = time((long *)0);                        /* get current time    */

	/* find failed subsystem in the active subsystem list */
	for(ptr=frstaste,prevaste=NULL; ptr!=NULL && ptr->svr_pid!=svrpid;ptr=ptr->next)
		prevaste = ptr;

	/* ignore child death if it ain't one of the subsystems we started */
	if (ptr == NULL)
		return;
	
	/* remove the old socket file for this subsystem */
	if(ptr->subsys->contact == SRCSOCKET)
		unlink(ptr->sun.sun_path);

	/* save subsystem and group names for potential notify action */
	strcpy(subsysname,ptr->subsys->subsysname);
	if (ptr->state != SRCSTPG && ptr->state != SRCWARN)
		strcpy(grpname,ptr->subsys->grpname);
	else
		*grpname='\0';


	/* has the subsystem lived longer than our respawn timelimit?
	** when it has we can reset our respawn vars
	**/
	if((now - ptr->stoptime) > (long)ptr->subsys->waittime)
	{
		ptr->stoptime = (long)0;
		ptr->warntime = (long)0;
		ptr->recount = 0;
	}

	/* should we try and respawn our subsystem? */

	    /* must have terminated abnormal. ie not told to stop */
	if( (ptr->state != SRCSTPG  && ptr->state != SRCWARN )
	    /* must be a system that requests respawning on abnormal death */
	    && (ptr->action == RESPAWN)  
	    /* must have not have reached our respawn limit
	    ** or must have no respawns
	    **     respawn limit is RELIMIT respawings within timelimit seconds
	    **/ 
	    && ((now - ptr->stoptime <= (long) ptr->subsys->waittime && ptr->recount < RELIMIT ) 
	       /* no respawns */
	       || ptr->stoptime == (long) NULL))
	{ 

		/* will try and respawn our subystem */
		logerr(ptr->subsys->subsysname,svr_stat,SRC_RSTRT,0);

		/* create our pipe to comunicate with child before the child
		** exec's the subsystem to receive errors
		**/
		if (pipe(fdes) == -1) { 
			logerr(ptr->subsys->subsysname,svr_stat,SRC_FEXE,errno);

			/* remove subsystem from active list and do notify action */
			(void)srcdsvr(ptr, prevaste);
			srcnotify(subsysname,grpname);
			return;                          /* Return                    */
		}

		fcntl(fdes[1], F_SETFD, 1);           /* Close pipe[1] on exec     */
		rc = fork();
		switch (rc) {
		case -1:
			/* our fork failed - we are the parent of course */
			logerr(ptr->subsys->subsysname,0,SRC_FEXE,errno);
			
			close(fdes[1]);    /* Close write part of pipe    */
			close(fdes[0]);    /* Close read  part of pipe    */

			/*-----------------------------*/
			/*  Notify process of svr end  */
			/*-----------------------------*/

			/* remove subsystem from active list and do notify action */
			(void)srcdsvr(ptr, prevaste);
			srcnotify(subsysname,grpname);
			break;

		case 0:
			/* we are the child process */

			close(fdes[0]);   /* Close read  part of pipe       */
			curraste = ptr;   /* current ptr to profile options */
			rc = srcdchd(fdes[1]);    /* child process routine to execl */
			break ;

		default: 
			/* we are the parent process */

			close(fdes[1]);  /* Close write part of pipe */

			while ((ret = read(fdes[0], &status,
			    sizeof(status))) == -1 &&
			    errno == EINTR);

			close(fdes[0]);  /* Close read part of pipe  */
			if ( ret ) {
				logerr(ptr->subsys->subsysname,0,SRC_FEXE,errno);

				/* remove subsystem from active list and do notify action */
				(void)srcdsvr(ptr, prevaste);
				srcnotify(subsysname,grpname);
			}
			else {
				/* remember our abnormal termination */
				ptr->recount += 1 ;

				/* remember that our subsystem is now active */
				ptr->state = SRCACT;

				/* remember our new subsystem pid */
				ptr->svr_pid = rc;

				/* when we have never terminated abnormaly
				** set our termination time
				**/
				if (ptr->recount == 1)
					ptr->stoptime = now;
			}
			break;
		} 
	} 
	else {
		/* at this point the subsystem has:
		**	1. terminated normaly
		**	2. terminated abnormaly and is not a respawn subsystem
		**	3. terminated abnormaly and has reached it's
		**	   respawn limit
		**/

		/* was the subsystem termination normal? */
		if ((ptr->state == SRCSTPG) || (ptr->state == SRCWARN))
		{
			/* remove the subsystem from the active list */
		        auditlog(AUDIT_SRC_STOP,0,ptr->subsys->subsysname,
			    strlen(ptr->subsys->subsysname));
			(void)srcdsvr(ptr, prevaste);
		}
		/* the subsystem terminated abnormaly */
		else {
			/* subsystem would have liked to respawn itself however
			** it has met it's time and retry limit already
			**/
			if(ptr->action==RESPAWN)
				logerr(ptr->subsys->subsysname,0,SRC_TRYX,svr_stat);
			else {
				/* subsystem was terminated abnormaly and is not
				** a respawn subystem
				**/
				status_info.stat = svr_stat;
				if (status_info.status[2] != 0 || status_info.status[3] != 0)
					logerr(ptr->subsys->subsysname,0,SRC_SVKO,svr_stat);
			}

			/* remove subsystem from active list and do notify action */
			(void)srcdsvr(ptr, prevaste);
			srcnotify(subsysname,grpname);
		}

	}
	return;
}                                             /* end srchevn()          */
