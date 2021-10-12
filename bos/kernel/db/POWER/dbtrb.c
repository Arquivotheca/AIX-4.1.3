static char sccsid[] = "@(#)08	1.31  src/bos/kernel/db/POWER/dbtrb.c, sysdb, bos411, 9428A410j 6/22/94 18:10:27";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:	disp_trbmenu, disp_trbmaint, disp_systrb, disp_proctrb,
 *		disp_utrb, disp_activetrb, disp_freetrb, disp_intr, 
 *		disp_const, trbdb, disp_threadtrb
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/
 
#include <sys/types.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/timer.h>
#include <sys/rtc.h>
#include <sys/pseg.h>
#include <sys/intr.h>
#ifdef _POWER_MP
#include <sys/ppda.h>                  /* To be able to access trb     */
#endif /* _POWER_MP */
#include "add_cmd.h"
#include "parse.h"			/* Parser structure		*/
#include "debaddr.h"
#include "dbtrb.h"
#include "vdbfmtu.h"

#ifndef _POWER_MP
extern struct	t_maint	t_maint;	/* trb maintenance structure	*/
#endif /* _POWER_MP */
extern struct	intr	clock_intr;	/* clock interrupt handler struc*/
extern int		debpg();	/* prompt user to continue	*/
extern caddr_t		getterm();	/* get line from term		*/
#ifndef _THREADS
extern struct	user	*read_uarea();	/* read in the user structure	*/
#else /* _THREADS */
extern struct	uthread	*read_utarea();	/* read in the uthread structure*/
#endif


#ifndef _POWER_MP
extern struct trb	*systimer;	/* system wide timer		*/
#endif /* _POWER_MP */

extern struct user *x;
extern struct ucred Ucred;


/*
 * NAME:  disp_trbmenu
 *
 * FUNCTION:  Display the timer request block menu
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * EXTERNAL PROCEDURES CALLED:  printf
 *
 * RETURN VALUE DESCRIPTION:  None
 *
 */
void
disp_trbmenu()

{
	printf("Timer Request Block Information Menu\n\n\n\n");
	printf("1. TRB Maintenance Structure - Routine Addresses\n\n");	/* x */
	printf("2. System TRB\n");					/* x */
#ifndef _THREADS
	printf("3. Process Specified TRB\n");				/* x */
	printf("4. Current Process TRB's\n");
#else /* _THREADS */
	printf("3. Thread Specified TRB\n");				/* x */
	printf("4. Current Thread TRB's\n");				/* x */
#endif
	printf("5. Address Specified TRB\n\n");
	printf("6. Active TRB Chain\n");				/* x */
	printf("7. Free TRB Chain\n\n");				/* x */
	printf("8. Clock Interrupt Handler Information\n");		/* x */
	printf("9. Current System Time - System Timer Constants\n\n");	/* x */
	printf(SELECT_PROMPT);

}  /* end disp_trbmenu */

/*
 * NAME:  disp_trbmaint
 *
 * FUNCTION:  Display the trb maintenance data.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * EXTERNAL PROCEDURES CALLED:  printf
 *
 * RETURN VALUE DESCRIPTION:  None
 *
 */
void
disp_trbmaint()
{
	extern struct trb *talloc();
	extern int clock(), delay(), itimerdecr(), restimer();
	extern int getinterval(), absinterval(), incinterval();
	extern int gettimer(), settimer(), resabs(), resinc();
	extern int tstop();
	extern void texit(), tfree(), reset_decr(), delay_end();
	extern void tstart(), interval_end();
	extern void reltimerid();
	extern timer_t gettimerid();

	int cpu;

	for (cpu = 0; cpu < NCPUS(); cpu++){
	  printf("TRB Maintenance Structure for processor %d:\n\n", cpu);
	  printf("   Free list anchor...................0x%x\n", TIMER(cpu)->t_free);
	  printf("   Active list anchor.................0x%x\n", TIMER(cpu)->t_active);
	  printf("   Number of TRB's on free list.......0x%x\n", TIMER(cpu)->t_freecnt);
      }

	printf("\nclock.c routines:\n");
	printf("<clock>: 0x%x,   <talloc>: 0x%x,    <tfree>: 0x%x\n",
		clock, talloc, tfree);
	printf("<tstart>: 0x%x,  <tstop>: 0x%x,     <texit>: 0x%x\n",
		tstart, tstop, texit);
	printf("<delay>: 0x%x,   <delay_end>: 0x%x, <reset_decr>: 0x%x\n",
		delay, delay_end, reset_decr);
	printf("\ntscalls.c routines:\n");
	printf("<getinterval>: 0x%x, <gettimer>: 0x%x, <settimer>: 0x%x\n",
		getinterval, gettimer, settimer);
	printf ("<absinterval>: 0x%x, <incinterval>: 0x%x, ", absinterval,
		incinterval);
	printf("<interval_end>:0x%x\n", interval_end);
	printf("<gettimerid>: 0x%x,  <reltimerid>: 0x%x,\n",
		gettimerid, reltimerid);
	printf("<resabs>: 0x%x,      <resinc>: 0x%x,      ",
		resabs, resinc);
	printf ("<restimer>: 0x%x\n", restimer);
	printf("<itimerdecr>: 0x%x\n", itimerdecr);
	printf("\n\n");
}

/*
 * NAME:  disp_systrb
 *
 * FUNCTION:  Display the system wide timer request block.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * EXTERNAL PROCEDURES CALLED:  printf
 *
 * RETURN VALUE DESCRIPTION:  None
 *
 */
void
disp_systrb()
{
	int cpu;
	struct trb *systimer;

	for (cpu = 0; cpu < NCPUS(); cpu++){
	  systimer = TIMER(cpu)->systimer;
	  printf("System Timer for processor %d:\n\n", cpu);
	  printf("   Timer address......................0x%x\n", systimer);
	  printf("   trb->to_next.......................0x%x\n", 
		 systimer->to_next);
	  printf("   trb->knext.........................0x%x\n", systimer->knext);
	  printf("   trb->kprev.........................0x%x\n", systimer->kprev);
	  printf("   trb->id............................0x%x\n", systimer->id);
	  printf("   Timer flags........................0x%x\n", systimer->flags);
	  printf("   trb->timerid.......................0x%x\n", 
		 systimer->timerid);
	  printf("   trb->eventlist.....................0x%x\n", 
		 systimer->eventlist);
	  printf("   trb->timeout.it_interval.tv_nsec...0x%x\n", 
		 systimer->timeout.it_interval.tv_sec);
	  printf("   trb->timeout.it_interval.tv_secs)..0x%x\n", 
		 systimer->timeout.it_interval.tv_nsec);
	  printf("   Next scheduled timeout (secs)......0x%x\n", 
		 systimer->timeout.it_value.tv_sec);
	  printf("   Next scheduled timeout (nanosecs)..0x%x\n", 
		 systimer->timeout.it_value.tv_nsec);
	  printf("   Timeout function (sys_timer()).....0x%x\n", systimer->func);
	  printf("   Timeout function data..............0x%x\n", 
		 systimer->func_data);
	  printf("   trb->timerid.......................0x%x\n", 
		 systimer->timerid);
	  printf("\n");
       }
}

/*
 * NAME:  disp_threadtrb
 *
 * FUNCTION:  Display the trb's associated with a specified thread.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * EXTERNAL PROCEDURES CALLED:  printf
 *
 * RETURN VALUE DESCRIPTION:  None
 *
 */
void
disp_threadtrb(tid)
{
	register int i, seg_id;
	struct uthread *ut, *uta;
	struct user *usa;
	int paged;

	/* find the TID we are interested in */
	if((seg_id = get_thread(tid,&ut))<0)
                return;

        /* copy the utarea using Get_from_memory() */
        if(((int)(uta = read_utarea(seg_id,ut))) == -1)
        {
                printf("Uthread area unreachable.\n");
                return;
        }

        /* copy the uarea using Get_from_memory() */
        if((int)(usa = read_uarea(seg_id,&paged)) == -1)
	{
                printf("User area unreachable.\n");
               	return;
	}

        printf("\nPER-PROC TIMER MANAGEMENT\n");
        printf("\n\nREAL/ALARM TIMER (U_timer[TIMERID_ALRM]) = 0x%08x",
                usa->U_timer[TIMERID_ALRM]);
        printf("\nVIRTUAL TIMER (U_timer[TIMERID_VIRTUAL]) = 0x%08x",
                usa->U_timer[TIMERID_VIRTUAL]);
        printf("\nPROF TIMER (U_timer[TIMERID_PROF]) = 0x%08x",
                usa->U_timer[TIMERID_PROF]);
        printf("\nVIRT TIMER (U_timer[TIMERID_VIRT]) = 0x%08x\n",
                usa->U_timer[TIMERID_VIRT]);
        for(i = TIMERID_TOD; i <= (NTIMERS - 1); i++)  {
                printf("POSIX TIMEOFDAY type timer #%d trb..............0x%08x\n",
                        i + 1 - TIMERID_TOD, usa->U_timer[i]);
        }

        printf("\nPER-THREAD TIMER MANAGEMENT\n");
        printf("\n\nREAL1 TIMER (ut_timer[TIMERID_REAL1]) = 0x%08x",
                uta->ut_timer[0]);
        clrdsp();
}

/*
 * NAME:  disp_utrb
 *
 * FUNCTION:  Display the trb's associated with the current process.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * EXTERNAL PROCEDURES CALLED:  printf
 *
 * RETURN VALUE DESCRIPTION:  None
 *
 */
disp_utrb()
{
	register int index;

	printf("Timer information for current process (pid %x)\n\n",
		curproc->p_pid);
	printf("ITIMER_PROF timer trb..........................0x%x\n",
		U.U_timer[ITIMER_PROF]);
	printf("ITIMER_VIRTUAL timer trb.......................0x%x\n",
		U.U_timer[ITIMER_VIRTUAL]);
	printf("ITIMER_REAL and SIGALRM type timer trb.........0x%x\n",
		U.U_timer[ITIMER_REAL]);
	printf("ITIMER_VIRT timer trb..........................0x%x\n",
		U.U_timer[ITIMER_VIRT]);
	for(index = TIMERID_TOD; index <= (NTIMERS - 1); index++)  {
		printf("POSIX TIMEOFDAY type timer #%d trb..............0x%x\n",
			index + 1 - TIMERID_TOD, U.U_timer[index]);
	}

	printf("\nTimer information for current thread (tid %x)\n\n",
		curthread->t_tid);
	printf("ITIMER_REAL1 timer trb.........................0x%x\n",
		u.u_th_timer[ITIMER_VIRT]);
	clrdsp();
}

/*
 * NAME:  disp_activetrb
 *
 * FUNCTION:  Display the trb's on the active chain.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * EXTERNAL PROCEDURES CALLED:  printf
 *
 * RETURN VALUE DESCRIPTION:  None
 *
 */
void
disp_activetrb(int cpu)
{
	register struct	trb	*tmptrb;
	register 	int	current = 1;

	tmptrb = TIMER(cpu)->t_active;

	while(tmptrb != NULL)  {
	   clrdsp();
	   printf("TRB #%d on Active List\n\n", current);
	   printf(" Timer address......................0x%x\n", tmptrb);
	   printf(" trb->to_next.......................0x%x\n", tmptrb->to_next);
	   printf(" trb->knext.........................0x%x\n", tmptrb->knext);
	   printf(" trb->kprev.........................0x%x\n", tmptrb->kprev);
	   printf(" Owner id (-1 for dev drv)..........0x%x\n", tmptrb->id);
	   printf(" Timer flags........................0x%x\n", tmptrb->flags);
	   printf(" trb->timerid.......................0x%x\n", 
		tmptrb->timerid);
	   printf(" trb->eventlist.....................0x%x\n", 
		tmptrb->eventlist);
	   printf(" trb->timeout.it_interval.tv_nsec...0x%x\n", 
		tmptrb->timeout.it_interval.tv_sec);
	   printf(" trb->timeout.it_interval.tv_secs)..0x%x\n", 
		tmptrb->timeout.it_interval.tv_nsec);
	   printf(" Next scheduled timeout (secs)......0x%x\n", 
		tmptrb->timeout.it_value.tv_sec);
	   printf(" Next scheduled timeout (nanosecs)..0x%x\n", 
		tmptrb->timeout.it_value.tv_nsec);
	   printf(" Timeout function...................0x%x\n", 
		tmptrb->func);
	   printf(" Timeout function data..............0x%x\n", 
		tmptrb->func_data);
	   printf("\n");
	   current++;
	   tmptrb = tmptrb->knext;
	   if(debpg() == FALSE)  {
		break;
	   }
	}
}

/*
 * NAME:  disp_freetrb
 *
 * FUNCTION:  Display the trb's on the free chain.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * EXTERNAL PROCEDURES CALLED:  printf
 *
 * RETURN VALUE DESCRIPTION:  None
 *
 */
void
disp_freetrb(int cpu)
{
	register struct	trb	*tmptrb;
	register 	int	current = 1;

	tmptrb = TIMER(cpu)->t_free;

	if(tmptrb == NULL)  {
		printf(NO_FREE);
	}
	else while(tmptrb != NULL)  {
	   clrdsp();
          printf("TRB #%d of %d on Free List\n\n",current,TIMER(cpu)->t_freecnt-1);
	   printf("   Timer address......................0x%x\n",tmptrb);
	   printf("   Timer flags........................0x%x\n",tmptrb->flags);
	   printf("   Timeout function...................0x%x\n",tmptrb->func);
	   printf("   Timeout function data..............0x%x\n",
		tmptrb->func_data);
	   printf("   Scheduled timeout (secs)...........0x%x\n",
		tmptrb->timeout.it_value.tv_sec);
	   printf("   Scheduled timeout (nanosecs).......0x%x\n",
		tmptrb->timeout.it_value.tv_nsec);
	   printf("   trb->to_next.......................0x%x\n",
		tmptrb->to_next);
	   /* printf("   trb->prev..........................0x%x\n",
		tmptrb->prev);*/
	   printf("   trb->timerid.......................0x%x\n",
		tmptrb->timerid);
	   printf("   trb->timeout.it_interval.tv_sec....0x%x\n",
		tmptrb->timeout.it_interval.tv_sec);
	   printf("   trb->timeout.it_interval.tv_nsec...0x%x\n",
		tmptrb->timeout.it_interval.tv_nsec);
	   /*printf("   trb->tmgt..........................0x%x\n",
		tmptrb->tmgt);*/
	   printf("   trb->knext.........................0x%x\n",tmptrb->knext);
	   printf("   trb->kprev.........................0x%x\n",tmptrb->kprev);
	   printf("\n");
	   current++;
	   tmptrb = tmptrb->knext;
	   if(debpg() == FALSE)  {
		break;
	   }
	}
}

/*
 * NAME:  disp_intr
 *
 * FUNCTION:  Present information about the clock interrupt handler.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:  
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 * RETURN VALUE DESCRIPTION:  none
 *
 */
void
disp_intr()
{
	printf("Clock Interrupt Handler Information:\n\n");
	printf("   intr->next...........................................0x%x\n",
	       clock_intr.next);
	printf("   intr->handler (clock())..............................0x%x\n",
	       clock_intr.handler);
	printf("   intr->bus_type.......................................0x%x\n",
	       clock_intr.bus_type);
	printf("   intr->flags..........................................0x%x\n",
	       clock_intr.flags);
	printf("   intr->level..........................................0x%x\n",
	       clock_intr.level);
	printf("   intr->priority.......................................0x%x\n",
	       clock_intr.priority);
	printf("   intr->bid............................................0x%x",
	       clock_intr.bid);
	printf ("\n\n\n\n");
}

/*
 * NAME:  disp_const
 *
 * FUNCTION:  Present a formatted  display of the system timer constants.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:  
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 * RETURN VALUE DESCRIPTION:  none
 *
 */
void
disp_const()
{
	printf("System Timer Constants:\n\n");
	printf("   Ticks Per Second (HZ).................................%d\n",
		HZ);
	printf("   Clock Decrements Per Second (DEC).....................%d\n",
		DEC);
	printf("   Nanoseconds Between Clock Interrupts (NS_PER_SEC / HZ)%d\n",
		NS_PER_SEC / HZ);
	printf("\nCurrent Time:\n");
	printf("   Time Since Epoch (Seconds)..........................0x%x\n",
		tod.tv_sec);
	printf("   Time Since Epoch (Nanoseconds)......................0x%x\n",
		tod.tv_nsec);
#if 0
	printf("   Date (Date Command Format)............................%d\n"
		/* , FILL IN */);
#endif
	printf("\nTRB Flags:\n");
	printf("   Timeout at Absolute Time Requested (T_ABSOLUTE).......%d\n",
		T_ABSOLUTE);
	printf("   Timeout is Pending (T_ACTIVE).........................%d\n",
		T_ACTIVE);
}

/*
 * NAME:  trb
 *
 * FUNCTION:  Present a formatted  display of the system timer request blocks.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:  cbbuf, proc, parse_out
 *
 * EXTERNAL PROCEDURES CALLED:
 *	debpg		get "continue" prompt from user
 *	getterm		get a line of input from the terminal
 *	clrdsp
 *
 * RETURN VALUE DESCRIPTION:  none
 *
 */
int
trbdb(ps)
register struct	parse_out	*ps;
{
	register caddr_t   option;
	struct	 parse_out out_line;
	register tid_t	   tid;
	int cpu;

	while(1)  {
		clrdsp();		/* clear the screen		*/
		disp_trbmenu();		/* display main menu		*/

		option = getterm();	/* get line from terminal	*/
		switch(*option)  {
			case '1':	/* Maintenance structure	*/
				clrdsp();
				disp_trbmaint();
				printf(CONT_PROMPT);
				(void) getterm();
				break;
			case '2':	/* System wide timer		*/
				clrdsp();
				disp_systrb();
				printf(CONT_PROMPT);
				(void) getterm();
				break;
			case '3':	/* Process trb's		*/
				printf(TID_PROMPT);
				option = getterm();
				while((*option != 'x') && (*option != 'X'))  {
				   if (*option != '\0')  {
				      parse_line(option, &out_line, " ");
				      tid = (tid_t)out_line.token[0].hv;
				      if (VALIDATE_TID(tid) == NULL) {
					 printf(BAD_TID_MSG, tid);
					 option = getterm();
				      }
				      else  {
					 clrdsp();
					 disp_threadtrb(tid);
					 printf(CONT_PROMPT);
					 (void) getterm();
					 break;
				      }
				   }
				   else {
				      printf(TID_PROMPT);
				      option = getterm();
				   }
				}
				break;
			case '4':	/* System wide timer		*/
				clrdsp();
				disp_utrb();
				printf(CONT_PROMPT);
				(void) getterm();
				break;
			case '5':	/* TRB by address		*/
				clrdsp();
				printf("This feature has not been implemented");
				printf (" yet.\n\n\n\n\n\n\n\n\n\n ");
				printf(CONT_PROMPT);
				(void) getterm();
				break;
			case '6':	/* Active TRB's			*/
			       for (cpu = 0; cpu < NCPUS(); cpu++){
				 if(TIMER(cpu)->t_active == NULL)  {
				   printf("No active trb for processor %d\n", cpu);
				   printf(CONT_PROMPT);
				   (void) getterm();
				 }
				 else  {
				   clrdsp();
				   printf("Active trbs for processor %d\n", cpu);
				   disp_activetrb(cpu);
				 }
			       }
				break;
			case '7':	/* Free TRB's			*/
				for (cpu = 0; cpu < NCPUS(); cpu++){
				  if(TIMER(cpu)->t_free == NULL)  {
				    printf("No free trbs for processor %d\n", cpu);
				    printf(CONT_PROMPT);
				    (void) getterm();
				  }
				  else  {
				    clrdsp();
				    printf("Free trbs for processor %d\n", cpu);
				    disp_freetrb(cpu);
				  }
				}
				break;
			case '8':	/* Clock Interrupt Handler Data	*/
				clrdsp();
				disp_intr();
				printf(CONT_PROMPT);
				(void) getterm();
				break;
			case '9':	/* Time and Timer Constants	*/
				clrdsp();
				disp_const();
				printf(CONT_PROMPT);
				(void) getterm();
				break;
			case 'x':			/* quit */
			case 'X':
				return((int)0);
			case '\0':			/* New line */
				break;
			default:
				printf(BAD_OPT_MSG);
				printf(CONT_PROMPT);
				(void) getterm();
				break;
		}
	}
}
