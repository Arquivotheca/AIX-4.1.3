static char sccsid[] = "@(#)94	1.15  src/bos/kernel/db/POWER/thread.c, sysdb, bos41J, 9512A_all 3/20/95 15:02:55";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: pr_thread, disp_thread, print_t_flags
 *
 * ORIGINS: 27, 83
 *
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#ifdef _THREADS
#include <sys/types.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/pri.h>
#include <sys/buf.h>
#include <sys/systm.h>
#include <sys/sched.h>
#include "debaddr.h"			/* Address structure		*/
#include "parse.h"			/* Parser structure.		*/
#include "pr_proc.h"			/* declares for this file 	*/
#include "debvars.h"			/* for addressing seg reg	*/

#define DEF_STORAGE 1
#include "add_cmd.h"			/* shared buf for cntrl blk display */

/*
 * EXTERNAL PROCEDURES CALLED:
 */

extern struct user *read_uarea();	/* copys the uarea into a buffer */
extern int debpg();			/* prompt user to continue */
extern ulong	g_kxsrval;		/* kernel extension seg reg value */

/*
 * NAME: pr_thread
 *
 * FUNCTION: 	Provide a formatted display of Thread Table area.
 *		If user has supplied a PID (in arg1) print only
 *		slots of threads belonging to that process.
 *		If user has supplied a TID (in arg1) print a long
 *		listing of only that slot.
 *
 * RETURN VALUE:  none.
 */
void
pr_thread(ps)
struct parse_out *ps;
{
	register int i,screen;
	struct thread *tb = (struct thread *) (cbbuf + sizeof(struct user));
			/* above leaves space for read_uarea to use buffer */
	int mode = 0;

	clrdsp();	/* clear the screen */

#ifdef Debug
	jr_debug();
#endif /* Debug */

	if ((ps->num_tok > 0) && (ps->token[1].tflags & HEX_VALID))
	    /* if argument is a tid, then this is a long listing */
	    if (MAYBE_TID(ps->token[1].hv)) {
		mode = 2;
		printf("    ST    TID      PID    CPUID  POLICY PRI CPU");
	    }
	    /* if argument is a pid, then this is a restricted listing */
	    else {
		mode = 1;
		printf("SLT ST    TID      PID    CPUID  POLICY PRI CPU");
	    }
	else
	    /* if there is no valid argument, then this is a short listing */
	    printf("SLT ST    TID      PID    CPUID  POLICY PRI CPU");

	/* print the rest of the header */
	printf("    EVENT  PROCNAME     FLAGS\n");

	/* Loop retrieving each thread table slot and printing it */
	screen = 1;
	for(i=0;i<NTHREAD;i++) {
		if(!pre_get_put_data(&thread[i], 0/*read*/, (char *)tb,
			g_kxsrval, VIRT, sizeof(struct thread))){
#ifdef Debug
			printf("Rest of Thread Table paged out.\n");
#endif /* Debug */
			return;
		}
#ifdef Debug
		if(DBG_LVL)
			printf("&thread[%d]=0x%x \n",i,&thread[i]);
#endif /* Debug */
		switch(mode) {
		    case 2 :
			/* print out single slot, long listing */
			if(ps->token[1].hv==tb->t_tid) {
				disp_thread(tb,-1);
				return;
			}
			break;
		    case 1 :
			/* or print out only slots belonging to a process */
			if(tb->t_state!=TSNONE)
			    if(ps->token[1].hv==tb->t_procp->p_pid) {
				disp_thread(tb,i);
				/* give user one screen at a time */
				if(!(screen++%20))
				    if(debpg() == FALSE)
					return;
			    }
			break;
		    case 0 :
			/* or print out all slots */
			if(tb->t_state!=TSNONE) {
				disp_thread(tb,i);
				/* give user one screen at a time */
				if(!(screen++%20))
				    if(debpg() == FALSE)
					return;
			}
			break;
		}
	}
}


/*
 *    Function:	Print a thread table slot.  If slot = -1 print long
 *		listing of that slot.
 *
 *    Returns:	nothing.
*/
disp_thread(t,slot)
struct thread *t;		/* thread table pointer */
{
	char	*cp;
	int	i,paged;
	struct user *user_area;

#ifdef RunFlag
	if(run && t->t_state != TSRUN)
		return;
#endif

	switch(t->t_state) {
		case TSNONE:	cp = " "; break;
		case TSIDL:	cp = "i"; break;
		case TSRUN:	cp = "r"; break;
		case TSSLEEP:	cp = "s"; break;
		case TSSWAP:	cp = "o"; break;
		case TSSTOP:	cp = "t"; break;
		case TSZOMB:	cp = "z"; break;
		default:	cp = "?"; break;
	}
	if(slot == -1)
		printf("    %s", cp);
	else
		printf("%3d%s%s", slot,
			curthread->t_tid == t->t_tid ? "*" : " ", cp);

	printf(" %8x %8x     ", t->t_tid, t->t_procp->p_pid);

	if (t->t_cpuid == PROCESSOR_CLASS_ANY)
		printf(" ANY ");
	else
		printf("%4x ", t->t_cpuid);

	switch(t->t_policy) {
		case SCHED_OTHER:	printf(" OTHER"); break;
	        case SCHED_FIFO:        printf("  FIFO"); break;
	        case SCHED_RR:          printf("RD-ROB"); break;
		default:	        printf("     ?"); break;
	}

	printf(" %3x %3x", t->t_pri & PMASK,
			(t->t_cpu < T_CPU_MAX) ? t->t_cpu : T_CPU_MAX);

	t->t_wchan == 0 ? printf("         ") : printf(" %08x", t->t_wchan);

	/* Get command name */
	if(t->t_procp->p_stat == SNONE)
		cp = "        ";
	else if(t->t_procp->p_stat == SZOMB)
		cp = "ZOMBIE";
	else if(!t->t_procp->p_pid)  			/* pid 0 is swapper */
		cp = "swapper";
	else {
		/* copy the uarea using Get_from_memory() */
		if((int)(user_area=read_uarea(t->t_procp->p_adspace,&paged))<0)
			cp = "<unknown>";
		else
			cp = user_area->U_comm;
	}
	if(!t->t_procp->p_pid)	/* just to make it all fit on 1 line */
		printf(" %s ", cp);
	else
		printf(" %s  ", cp);
	printf(" 0x%08x\n",t->t_flags);           /* now print t_flags */

	/* end of short (whole thread table) listing */
	if(slot != -1)
		return;
	/* else do a long listing */

/* Main thread link pointers */
	printf("\nLinks:  *procp:0x%08x  *uthreadp:0x%08x  *userp:0x%08x\n",
		t->t_procp, t->t_uthreadp, t->t_userp);
	printf("    *prevthread:0x%08x  *nextthread:0x%08x\n",
		t->t_prevthread, t->t_nextthread);

	printf("    *wchan1(real):0x%08x  *wchan2(VMM):0x%08x\n",
		t->t_wchan1,t->t_wchan2);

	printf("    wchan1sid:0x%08x wchan1offset:0x%08x\n",
	        t->t_wchan1sid,t->t_wchan1offset);

	printf("    *slist:0x%08x  *swchan:0x%08x \n",
		t->t_slist,t->t_swchan);

/* Dispatch fields */
      printf("Dispatch Fields:  *prior:0x%08x  *next:0x%08x\n",
		t->t_prior,t->t_next);
      printf("    polevel:0x%08x  ticks:0x%04x  *synch:0x%08x  result:0x%08x\n",
		t->t_polevel, t->t_ticks, t->t_synch, t->t_result);
      printf("    *eventlst:0x%08x  *wchan(hashed):0x%08x  suspend:0x%04x\n",
		t->t_eventlist,t->t_wchan,t->t_suspend);
      printf("    t_wevent:0x%08x  t_pevent:0x%08x",t->t_wevent, t->t_pevent);
      printf("  thread waiting for: %s%s%s%s%s%s%s%s%s%s%s%s\n",  
				/* be sure number of "%s" entries */
			      /* matches number of expressions below */
		t->t_wtype == TNOWAIT ? " nothing " : "",
		t->t_wtype == TWEVENT ? " event(s) " : "",
		t->t_wtype == TWLOCK ? " serial lock " : "",
		t->t_wtype == TWLOCKREAD ? " serial read lock " : "",
		t->t_wtype == TWTIMER ? " timer " : "",
		t->t_wtype == TWCPU ? " cpu (ready) " : "",
		t->t_wtype == TWPGIN ? " page in " : "",
		t->t_wtype == TWPGOUT ? "page out" : "",
		t->t_wtype == TWPLOCK ? " physical lock " : "",
		t->t_wtype == TWFREEF ? " free page frame " : "",
		t->t_wtype == TWMEM ? " memory " : "",
		t->t_wtype == TWUEXCEPT ? " user exception " : "");

/* Scheduler info */
	printf("Scheduler Fields:  cpuid:");
	if (t->t_cpuid == PROCESSOR_CLASS_ANY)
		printf("   ANY");
	else
		printf("0x%04x", t->t_cpuid);
	printf("  scpuid:");
	if (t->t_scpuid == PROCESSOR_CLASS_ANY)
		printf("   ANY");
	else
		printf("0x%04x", t->t_scpuid);
	printf("  pri:%3u", t->t_pri);
	printf("  sav_pri:%3u\n", t->t_sav_pri);
	printf("    policy:");
	switch(t->t_policy) {
		case SCHED_OTHER:	printf(" OTHER"); break;
	        case SCHED_FIFO:        printf("  FIFO"); break;
	        case SCHED_RR:          printf("RD-ROB"); break;
	        default:	        printf("     ?"); break;
	}
	printf("    lpri:%3u  wpri:%3u", t->t_lockpri, t->t_wakepri);
	printf("    time:0x%02x\n", t->t_time);

/* Misc */
	printf("Misc:  t_lockcount:0x%08x  t_ulock:0x%08x\n",
		t->t_lockcount, t->t_ulock);
	printf("    t_dispct:0x%08x  t_fpuct:0x%08x  t_graphics:0x%08x\n",
		t->t_dispct, t->t_fpuct, t->t_graphics);
        printf("    t_stackp:0x%08x  t_boosted:0x%08x  t_lockowner:0x%08x\n",
			t->t_stackp, t->t_boosted, t->t_lockowner);


/* Signal Fields */
	printf("Signal Information:  cursig:0x%02x\n", t->t_cursig);
	printf("    pending:hi 0x%08x,lo 0x%08x  sigmask:hi 0x%08x,lo 0x%08x\n",
			t->t_sig.hisigs, t->t_sig.losigs,
			t->t_sigmask.hisigs, t->t_sigmask.losigs);
	printf("    *scp: 0x%08x\n", t->t_scp);

	print_t_flags(t);
}  /* end disp_thread */

print_t_flags(t)
struct thread *t;
{
	unsigned int flags;

	flags = t->t_flags;
	printf("    t_flags:");

	if (flags & TTERM) printf(" TERM");
	if (flags & TSUSP) printf(" SUSP");
	if (flags & TSIGAVAIL) printf(" SIGAVAIL");
	flags &= ~(TTERM | TSUSP | TSIGAVAIL);

	if (flags & TLOCAL) printf(" LOCAL");
	if (flags & TASSERTSIG) printf(" ASSERTSIG");
	if (flags & TTRCSIG) printf(" TRCSIG");
	flags &= ~(TLOCAL | TASSERTSIG | TTRCSIG ); 

	if (flags & TOMASK) printf(" OMASK");
	if (flags & TWAKEONSIG) printf(" WAKEONSIG");
	flags &= ~(TOMASK | TWAKEONSIG);

	if (flags & TKTHREAD) printf (" KTHREAD");
	if (flags & TFUNNELLED) printf (" FUNNELLED");
	if (flags & TSETSTATE) printf(" SETSTATE");
	flags &= ~(TKTHREAD | TFUNNELLED | TSETSTATE );

	if (flags & TPMSTOP) printf (" PMSTOP");
	if (flags & TPMREQSTOP) printf (" PMREQSTOP");
	if (flags & TBOOSTING) printf (" BOOSTING");
	flags &= ~(TPMSTOP | TPMREQSTOP | TBOOSTING);

	if (flags) printf(" unknown: 0x%x",flags);

	printf("\n");

	flags = t->t_atomic;
	printf("    t_atomic:");

        if (flags & TSIGDELIVERY) printf(" SIGDELIVERY");
        if (flags & TSELECT) printf(" SELECT");
        flags &= ~(TSIGDELIVERY | TSELECT );

        if (flags) printf(" unknown: 0x%x",flags);

	printf("\n");
}
#endif
