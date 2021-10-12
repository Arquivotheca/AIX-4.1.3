static char sccsid[] = "@(#)67	1.37.1.9  src/bos/kernel/db/POWER/proc.c, sysdb, bos411, 9428A410j 2/24/94 07:29:07";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: pr_proc, disp_proc, print_p_flag, slotcnt
 *
 * ORIGINS: 27, 83
 *
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
#include <sys/pri.h>
#include <sys/buf.h>
#include <sys/systm.h>
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
 * GLOBAL VARIABLES USED
 */

static char new_header = 0;		/* minus used on command line? */

/*
 * NAME: pr_proc
 *
 * FUNCTION: 	Provide a formatted display of Process Table area.
 *		If users has supplied a PID (in arg1) print a long
 *		listing of only that slot.
 *
 * RETURN VALUE:  none.
 */
void
pr_proc(ps)
struct parse_out *ps;
{
	register int i,screen;
	struct proc *pb = (struct proc *) (cbbuf + sizeof(struct user));
			/* above leaves space for read_uarea to use buffer */
	int arg_loc;

	clrdsp();	/* clear the screen */

#ifdef Debug
	jr_debug();
#endif /* Debug */

	new_header = 0;
	arg_loc = 1;		/* default argument position */

	/* if last token is a number, then this is a long listing */
	if (ps->token[ps->num_tok].tflags & HEX_VALID)
	{
		if (ps->token[1].sv[0] == '-')		/* undoc-ed headings */
		{
		    printf("SLT ST    PID   PPID  PGRP   SID  TTYL  GAL GRL");
		    new_header = 1;
		    arg_loc = 2;
		}
		else
#ifndef _THREADS
		    printf("    ST    PID   PPID  PGRP   UID  EUID PRI CPU");
#else /* _THREADS */
		    printf("    ST    PID   PPID  PGRP   UID  EUID         ");
#endif
	}
	else						/* short listing */
	{
		if ((ps->num_tok > 0) && (ps->token[1].sv[0] == '-'))
		{
		    printf("SLT ST    PID   PPID  PGRP   SID  TTYL  GAL GRL");
		    new_header = 1;
		    arg_loc = 2;
		}
		else
#ifndef _THREADS
		    printf("SLT ST    PID   PPID  PGRP   UID  EUID  PRI CPU");
#else /* _THREADS */
		    printf("SLT ST    PID   PPID  PGRP   UID  EUID         ");
#endif
	}

	/* print the rest of the header */
#ifndef _THREADS
	printf("    EVENT    NAME       FLAGS\n");
#else /* _THREADS */
	printf(" #THREADS    NAME       FLAGS\n");
#endif

	/* Loop retrieving each proc table slot and printing it */
	screen = 1;
	for(i=0;i<NPROC;i++) {
		if(!pre_get_put_data(&proc[i], 0/*read*/, (char *)pb,
			g_kxsrval, VIRT, sizeof(struct proc))){
#ifdef Debug
			printf("Rest of Proc Table paged out.\n");
#endif /* Debug */
			return;
		}
#ifdef Debug
		if(DBG_LVL) {
			printf("&proc[%d]=0x%x \n",i,&proc[i]);
		}
#endif /* Debug */
		/* print out single slot, long listing */
		if(((ps->num_tok == 1) && (ps->token[1].sv[0] != '-')) ||
		   (ps->num_tok == 2))
		{
#if !defined(_DB_PROC_SLOT)
			if(ps->token[arg_loc].hv==pb->p_pid)
#else
			/* match pid or proc table slot */
			if(ps->token[arg_loc].hv==pb->p_pid ||
			  (ps->token[arg_loc].hv==i))
#endif	/* _DB_PROC_SLOT */
			{
				disp_proc(pb,-1);
				break;
			}
		}
		/* or print out all slots */
		else {
			if(pb->p_stat!=SNONE || pb->p_ganchor || pb->p_ttyl) {
				disp_proc(pb,i);
				/* give user one screen at a time */
				if(!(screen++%20)) {
				    if(debpg() == FALSE)
				    	return;
				}
			}
		}
	}
}


/*
 *    Function:	Print a process table slot.  If slot = -1 print long
 *		listing of that slot.
 *		(Note: started with original crash code, then modified it.)
 *
 *    Returns:	nothing.
*/
disp_proc(p,slot)
struct proc *p;		/* proc table pointer */
{
	char	*cp;
	int	i,paged;
	struct user *user_area;

#ifndef _THREADS
#ifdef RunFlag
	if(run && p->p_stat != SRUN)
		return;
#endif
#endif

	switch(p->p_stat) {
		case SNONE:       cp = " "; break;
#ifndef _THREADS
		case SSLEEP:      cp = "s"; break;
		case SRUN:        cp = "r"; break;
#else /* _THREADS */
		case SACTIVE:	  cp = "a"; break;
		case SSWAP:	  cp = "o"; break;
#endif
		case SIDL:        cp = "i"; break;
		case SZOMB:       cp = "z"; break;
		case SSTOP:       cp = "t"; break;
		default:          cp = "?"; break;
	}
	if(slot == -1)
		printf("    %s", cp);
	else
		printf("%3d%s%s", slot,
			curproc->p_pid == p->p_pid ? "*" : " ", cp);

	if (new_header)
	{
#ifndef _THREADS
		printf(" %8x %5x %5x %5x %5x  %3x %3x",
			p->p_pid, p->p_ppid, p->p_pgrp,
			slot == -1 ? p->p_suid : p->p_sid,
			slot == -1 ? p->p_uid : slotcnt(p->p_ttyl),
			slot == -1 ? (p->p_pri & PMASK) : slotcnt(p->p_ganchor),
			slot == -1 ? ((p->p_cpu < P_CPU_MAX) ? p->p_cpu :
				P_CPU_MAX) : slotcnt(p->p_pgrpl));
			p->p_wchan == 0 ?
				printf("         ") :
				printf(" %08x", p->p_wchan);
#else /* _THREADS */
		printf(" %8x %5x %5x %5x %5x  %3x %3x  %4d   ",
			p->p_pid, p->p_ppid, p->p_pgrp, p->p_sid,
			slotcnt(p->p_ttyl), slotcnt(p->p_ganchor),
			slotcnt(p->p_pgrpl), p->p_threadcount);
#endif
	}
	else
	{
#ifndef _THREADS
		printf(" %8x %5x %5x %5u %5u  %3u %3u",
			p->p_pid, p->p_ppid, p->p_pgrp, p->p_uid, p->p_suid,
			(p->p_pri & PMASK),
			((p->p_cpu < P_CPU_MAX) ? p->p_cpu : P_CPU_MAX));
			p->p_wchan == 0 ?
				printf("         ") :
				printf(" %08x", p->p_wchan);
#else /* _THREADS */
		printf(" %8x %5x %5x %5u %5u           %4d   ",
			p->p_pid, p->p_ppid, p->p_pgrp, p->p_uid, p->p_suid,
			p->p_threadcount);
#endif
	}

	/* Get command name */
	if(p->p_stat == SNONE)
		cp = "        ";
	else if(p->p_stat == SZOMB)
		cp = "ZOMBIE";
	else if(!p->p_pid)  			/* pid 0 is swapper */
		cp = "swapper";
	else {
		/* copy the uarea using Get_from_memory() */
		if((int)(user_area=read_uarea(p->p_adspace,&paged))<0) {
			cp = "<unknown>";
		}
		else
#ifndef _THREADS
			cp = user_area->u_comm;
#else /* _THREADS */
			cp = user_area->U_comm;
#endif
	}
	if(!p->p_pid) {			/* just to make it all fit on 1 line */
		printf(" %s ", cp);
		}
	else
		printf(" %s  ", cp);
	printf(" 0x%08x\n",p->p_flag);           /* now print p_flag */

	/* end of short (whole proc table) listing */
	if(slot != -1)
		return;
	/* else do a long listing */

/* Session */
	printf("\nSession:  pid:0x%08x  sid:0x%08x  pgrp:0x%08x\n",
		p->p_pid, p->p_sid, p->p_pgrp);

/* Main proc link pointers */
	printf("\nLinks:  *child:0x%08x  *siblings:0x%08x  *uidl:0x%08x\n",
		p->p_child, p->p_siblings, p->p_uidl);

	if (new_header)
	{
		printf("    *ttyl:0x%08x(%x)  *p->p_ganchor:0x%08x(%x)  ",
			p->p_ttyl, slotcnt(p->p_ttyl),
			p->p_ganchor, slotcnt(p->p_ganchor));
		printf("*pgrpl:0x%08x(%x) \n", p->p_pgrpl, slotcnt(p->p_pgrpl));
	}
	else
	{
	    printf("    *ttyl:0x%08x  *p->p_ganchor:0x%08x  *pgrpl:0x%08x \n",
		p->p_ttyl, p->p_ganchor, p->p_pgrpl);
	}

#ifndef _THREADS
	printf("    *wchan1(real):0x%08x  *lcklst:0x%08x \n",
		p->p_wchan1,p->p_locklist);
	printf ("   *selchn: 0x%08x\n", p -> p_selchn);
#else /* _THREADS */
	printf("    p_threadcount:%4d  *p_threadlist:0x%08x\n",
		p->p_threadcount, p->p_threadlist);
	printf("    p_active:%4d  p_suspended:%4d  p_terminating:%4d\n",
		p->p_active, p->p_suspended, p->p_terminating);
	printf("    p_local:%4d\n", p->p_local);
#endif
	
/* Dispatch fields */
#ifndef _THREADS
      printf("Dispatch Fields:  *prior:0x%08x  *next:0x%08x\n",
		p->p_prior,p->p_next);
      printf("    pevent:0x%08x  wevent:0x%08x\n",
		p->p_pevent, p->p_wevent);
      printf("    polevel:0x%08x  *lockwait:0x%08x \n",
	        p->p_polevel,p->p_lockwait);
      printf("    *eventlst:0x%08x  *wchan(hashed):0x%08x  suspend:0x%04x\n",
		p->p_eventlist,p->p_wchan,p->p_suspend);
      printf("    process waiting for: %s%s%s%s%s%s%s%s%s\n",  
				/* be sure number of "%s" entries */
			      /* matches number of expressions below */
		p->p_wtype == SNOWAIT ? " nothing " : "",
		p->p_wtype == SWEVENT ? " event(s) " : "",
		p->p_wtype == SWLOCK ? " serial lock " : "",
		p->p_wtype == SWTIMER ? " timer " : "",
		p->p_wtype == SWCPU ? " cpu (ready) " : "",
		p->p_wtype == SWPGIN ? " page in " : "",
		p->p_wtype == SWPGOUT ? "page out" : "",
		p->p_wtype == SWPLOCK ? " physical lock " : "",
		p->p_wtype == SWFREEF ? " free page frame " : "");
#else /* _THREADS */
	printf("Dispatch Fields:  pevent:0x%08x  *synch:0x%08x\n",
		p->p_pevent, p->p_synch);
#endif

/* Scheduler info */
#ifndef _THREADS
	printf("Scheduler Fields:  pri:%3u" ,p->p_pri);
	(p->p_flag & SFIXPRI) ? printf(" fixed pri:%3u" ,p->p_nice) :
				printf(" nice:%3u" ,EXTRACT_NICE(p));
	printf("  lpri:%3u  wpri:%3u  flags:%2x\n" ,p->p_lockpri,
		p->p_wakepri, p->p_sched_flags);
	printf("  repage:0x%08x  scount:0x%08x\n",
		p->p_repage,p->p_sched_count);
	printf("  *snext:0x%08x  *sback:0x%08x\n",
		p->p_sched_next,p->p_sched_back);
#else /* _THREADS */
	printf("Scheduler Fields:");
	if (!(p->p_flag & SFIXPRI))
		printf(" nice:%3u" ,EXTRACT_NICE(p));
	printf(" repage:0x%08x  scount:0x%08x  cpticks:0x%04x\n",
		p->p_repage,p->p_sched_count,p->p_cpticks);
	printf(" spri:%3u  *snext:0x%08x  *sback:0x%08x\n",
		p->p_sched_pri,p->p_sched_next,p->p_sched_back);
#endif

/* Misc */
       printf("Misc:  adspace:0x%08x  *ttyl:0x%08x\n",p->p_adspace,p->p_ttyl);
       printf("       *p_ipc:0x%08x  *p_dblist:0x%08x  *p_dbnext:0x%08x\n",
		p->p_ipc, p->p_dblist, p->p_dbnext);
#ifdef _THREADS
       printf("       p_kstackseg:0x%08x  p_lock:0x%08x\n",
		p->p_kstackseg, p->p_lock);
#endif

/* Zombie or Sig Info */
	if(p->p_stat == SZOMB) {
	  printf(
	   "Zombie:  exit status:0x%08x  user time:0x%08x  sys time:0x%08x\n",
		p->xp_stat, p->xp_utime, p->xp_stime);
		/*	return; 	*/
	}
	else {	
	/* Signal Fields */
#ifndef _THREADS
	printf("Signal Information:  cursig:0x%02x  sigstate:0x%02x \n",
			p->p_cursig, p->p_sigstate);
	printf("    pending:hi 0x%08x,lo 0x%08x  sigmask:hi 0x%08x,lo 0x%08x\n",
			p->p_sig.hisigs, p->p_sig.losigs,
			p->p_sigmask.hisigs, p->p_sigmask.losigs);
#else /* _THREADS */
	printf("Signal Information:  pending:hi 0x%08x,lo 0x%08x\n",
			p->p_sig.hisigs, p->p_sig.losigs);
#endif
     	printf(
	    "    sigcatch:hi 0x%08x,lo 0x%08x  sigignore:hi 0x%08x,lo 0x%08x\n",
			p->p_sigcatch.hisigs, p->p_sigcatch.losigs,
			p->p_sigignore.hisigs, p->p_sigignore.losigs);
	}

	/* Statistics */
	printf("Statistics:  size:0x%08x(pages)  audit:0x%08x\n",
		p->p_size, p->p_auditmask);
	printf ("Page faults (major): 0x%08x (minor) 0x%08x\n",
		p -> p_majflt, p -> p_minflt);

	print_p_flag(p);
}  /* end disp_proc */

print_p_flag(p)
struct proc *p;
{
	unsigned int flag;

	flag = p->p_flag;
	printf("  p_flag:");

	if (flag & SLOAD) printf(" LOAD");
	if (flag & SNOSWAP) printf(" NOSWAP");
	if (flag & STRC) printf(" TRC");
	flag &= ~(SLOAD | SNOSWAP | STRC);

	if (flag & SFIXPRI) printf(" FIXPRI");
	if (flag & SKPROC) printf(" KPROC");
	if (flag & SFORKSTACK) printf(" FORKSTACK");
	flag &= ~(SFIXPRI | SKPROC | SFORKSTACK);

	if (flag & SSIGNOCHLD) printf(" SIGNOCHLD");
	if (flag & SSIGSET) printf(" SIGSET");
	if (flag & SLKDONE) printf(" LKDONE");
	if (flag & STRACING) printf(" TRACING");
	if (flag & SMPTRACE) printf(" MPTRACE");
	flag &= ~(SSIGNOCHLD | SSIGSET | SLKDONE | STRACING | SMPTRACE);

	if (flag & SEXIT) printf(" EXIT");
	if (flag & SORPHANPGRP) printf(" ORPHANPGRP");
	if (flag & SNOCNTLPROC) printf(" NOCNTLPROC");
	if (flag & SPPNOCLDSTOP) printf(" PPNOCLDSTOP");
	flag &= ~(SEXIT | SORPHANPGRP |SNOCNTLPROC |SPPNOCLDSTOP);

	if (flag & SEXECED) printf(" EXECED");
	if (flag & SJOBSESS) printf(" JOBSESS");
	if (flag & SJOBOFF) printf(" JOBOFF");
	if (flag & SEXECING) printf(" EXECING");
	if (flag & SPSEARLYALLOC) printf(" PSEARLYALLOC");
	flag &= ~(SEXECED | SJOBSESS | SJOBOFF | SEXECING|SPSEARLYALLOC);

	if (flag) printf(" unknown: 0x%x",flag);
	printf("\n");

	flag = p->p_atomic;
	printf("  p_atomic:");

	if (flag & SWTED) printf(" WTED");
	if (flag & SFWTED) printf(" FWTED");
	if (flag & SEWTED) printf(" EWTED");
	if (flag & SLWTED) printf(" LWTED");

	flag &= ~(SWTED | SFWTED | SEWTED | SLWTED);

	if (flag) printf(" unknown: 0x%x",flag);
	printf("\n");

	flag = p->p_int;
	printf("  p_int:");

	if (flag & STERM) printf(" TERM");
	if (flag & SSUSPUM) printf(" SUSPUM");
	if (flag & SSUSP) printf(" SUSP");
	if (flag & SPSMKILL) printf(" PSMKILL");
	if (flag & SGETOUT) printf(" GETOUT");
	if (flag & SJUSTBACKIN) printf(" JUSTBACKIN");

	flag &= ~(SPSMKILL | SGETOUT | SJUSTBACKIN | SSUSP);
	flag &= ~(STERM | SSUSPUM);

	if (flag) printf(" unknown: 0x%x",flag);
	printf("\n");
}

int slotcnt(p)
struct proc *p;
{
	int slotpos = 0;

	if (p)
	    slotpos = p - proc;
	return (slotpos);
}
