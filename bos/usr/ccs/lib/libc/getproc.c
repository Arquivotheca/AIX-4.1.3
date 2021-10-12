static char sccsid[] = "@(#)30  1.7  src/bos/usr/ccs/lib/libc/getproc.c, libcproc, bos41J, 9520B_all 5/17/95 11:26:48";
/*
 *   COMPONENT_NAME: LIBCPROC
 *
 *   FUNCTIONS: getproc
 *
 *   ORIGINS: 27
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <procinfo.h>
#include <sys/thread.h>
#include <errno.h>
#include <sys/var.h>
#include <sys/sysconfig.h>

#define PROCINFO_BUFCNT 50

/*
 * NAME: getproc()
 *
 * FUNCTION: Retrieves an image of the process table
 *
 * NOTES: The table is not guaranteed to be consistent or even complete.
 *
 * RETURN VALUES:
 *	number	active processes retrieved
 *      -1	failed, errno indicates cause of failure
 */
getproc(struct procinfo *procinfo, int nproc, int sizproc)
/* struct  procinfo *procinfo;	 pointer to array of procinfo struct	*/
/* int	nproc;			 number of user procinfo struct	*/
/* int	sizproc;		 size of expected procinfo structure	*/
{
	struct procinfo *procinfosave = procinfo;
	register struct procsinfo *p;	/* local procs buffer ptr */
	struct procsinfo *psave;	/* local procs buffer ptr */
	struct thrdsinfo *t,*tsave;	/* local threads buffer ptr */
	struct thrdsinfo *tend;		/* end of local threads buffer */
	int num_act = 0;		/* number of actual processes */
	int procs_toget, pcount, result;
	pid_t pindex;
	tid_t tindex;
	struct var sysvars;		/* system variables */
	int num_thrds;			/* number of threads in table */	

        if (sizproc != sizeof(struct procinfo)) /* parameter ok         */
        {
        	errno = EINVAL;
        	return(-1);
        }

	/* get table sizes for procs and threads */

	if (sysconfig(SYS_GETPARMS, &sysvars, sizeof(struct var)) != 0)
		return(-1);

	if (nproc <= 0)
	{

	  	/* return the maximum proc number in the user's buffer */

	  	procinfo->pi_pid = (pid_t)((struct proc*)sysvars.ve_proc - (struct proc*)sysvars.vb_proc); /* process ID */
          	errno = ENOSPC;
          	return(-1);
	}

	/* get threadsinfo from the threads table */

	num_thrds = ((struct thread *)sysvars.ve_thread - (struct thread*)sysvars.vb_thread);

	t = (struct thrdsinfo *) malloc(num_thrds * sizeof(struct thrdsinfo));

	if (t == NULL) return(-1);		/* abort on error */
	tsave = t;
	tend = t + num_thrds;		/* point to end of buffer */
	tindex = (tid_t)0;
	result = getthrds(-1, t, sizeof(struct thrdsinfo), 
				&tindex, sysvars.v_thread);
	if (result == -1) 			/* abort on error */
	{
		free(tsave);
		return(-1);
	}

	/* get a buffer for the procinfo table */

	p = malloc(PROCINFO_BUFCNT * sizeof(struct procsinfo));
	if (p == NULL) 				/* abort on error */
	{
		free(tsave);
		return(-1);
	}
	psave = p;
	pindex = (pid_t)0;

	/* transfer the info from the proc table to the user buffer */

	for (procs_toget = sysvars.v_proc; procs_toget > 0;
			procs_toget -= result)
	{
		pcount = (procs_toget < PROCINFO_BUFCNT) ? procs_toget : 
							PROCINFO_BUFCNT;

	    	/* get a portion of the procinfo table */

	    	result = getprocs(psave, sizeof(struct procsinfo), NULL,
					0, &pindex, pcount);
		if (result == -1)		/* abort on error */
		{
			free(tsave);
			free(psave);
			return(-1);
		}

	    	num_act += result;		/* count real procs */
	    	if (result < pcount) 
	    	{
	      		/* only copy real procs, but protect buffer */
			pcount = ((nproc - (num_act - result)) < result) ?
					nproc - (num_act - result) : result;
	      		result = procs_toget;/* out of procs, force loop exit*/
	    	}
		else
		{
	      		/* protect user buffer */
			if ((nproc - (num_act - result)) < pcount)
				pcount = nproc - (num_act - result);
		}

	    	/* copy information from local buffer to user's buffer */

		for (p = psave; pcount > 0; pcount--, procinfo++, p++)
		{
			/* find corresponding thread structure.  If it
			 * cannot be found, then skip to the next entry.
			 */ 
			for (t=tsave; (t<tend) && (t->ti_pid!=p->pi_pid); t++);
			if (t >= tend) {
				procinfo--;			
				continue;	
			}

                        procinfo->pi_pid = p->pi_pid; /* process ID */
                        procinfo->pi_ppid = p->pi_ppid; /* parent ID */
                        procinfo->pi_uid = p->pi_uid; /* real user ID */
                        procinfo->pi_suid = p->pi_suid; /* save user ID */
                        procinfo->pi_pgrp = p->pi_pgrp; /* process group */
                        procinfo->pi_sid = p->pi_sid; /* session ID */
                        procinfo->pi_nice = p->pi_nice; /* nice value */
                        procinfo->pi_adspace = p->pi_adspace; /* addr */
                        procinfo->pi_utime = p->pi_utime; /* user time */
                        procinfo->pi_stime = p->pi_stime; /* system time */
                        procinfo->pi_majflt = p->pi_majflt;/* i/o page fault */
                        procinfo->pi_minflt = p->pi_minflt;/* non i/o pfault */
                        procinfo->pi_size = p->pi_size; /* process size */

                        /* flags translation */
                        p->pi_flags &=  ~(SFORKSTACK|SSIGNOCHLD|SSIGSET|
                                                   SJUSTBACKIN|SEXECING|
                                                 SSIGSLIH);
			procinfo->pi_flag = p->pi_flags & ~SPSEARLYALLOC;
                        if (p->pi_flags & SPSEARLYALLOC)
                              procinfo->pi_flag |= 0x10000000;/*SPSEARLYALLOC*/
                        if (t->ti_flag & TOMASK)
                                procinfo->pi_flag |= 0x00000400;/*SOMASK*/
                        if (t->ti_flag & TWAKEONSIG)
                                procinfo->pi_flag |= 0x00000800;/*SWAKEONSIG*/
                        if (t->ti_scount == 0)
                                procinfo->pi_flag |= 0x00001000;/*SUSER*/
                        if (t->ti_flag & TSELECT)
                                procinfo->pi_flag |= 0x00020000;/*SSEL*/
                        if (t->ti_flag & TSIGDELIVERY)
                                procinfo->pi_flag |= 0x01000000;/*PSIGDELIVER*/

			/* state translation */
			switch(t->ti_state) {
			case TSNONE :
				procinfo->pi_stat = 0;		/* SNONE */
				break;
			case TSIDL :
				procinfo->pi_stat = 4;		/* SIDL */
				break;
			case TSSLEEP :
				procinfo->pi_stat = 1;		/* SSLEEP */
				break;
			case TSRUN :
			case TSSWAP :
				procinfo->pi_stat = 3;		/* SRUN */
				break;
			case TSSTOP :
				procinfo->pi_stat = 6;		/* SSTOP */
				break;
			case TSZOMB :
				procinfo->pi_stat = 5;		/* SZOMB */
				break;
			}

			/* other thread fields */
			procinfo->pi_pri = t->ti_pri;
			procinfo->pi_cpu = t->ti_cpu;
			procinfo->pi_wchan = t->ti_wchan;
			procinfo->pi_wtype = t->ti_wtype;
                        /* wait translation */
                        if(t->ti_wtype == TWLOCKREAD)
                        	procinfo->pi_wtype = TWLOCK;
		}
	}
	free(tsave);
	free(psave);
	if (num_act > nproc)
	{
	  	procinfosave->pi_pid = (pid_t)num_act; 	/* process ID	*/
          	errno = ENOSPC;
          	return(-1);
	}
	return(num_act);
}
