static char sccsid[] = "@(#)96	1.11  src/bos/kernel/db/POWER/vdbfmtut.c, sysdb, bos411, 9428A410j 6/22/94 18:10:32";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: fmtut, read_utarea, get_thread, pr_ut, print_ut_flags
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#ifdef _THREADS
#include <sys/param.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/pseg.h>			/* needed for pinned/paged constants */
#include <sys/time.h>			/* for the per-process timer stuff   */
#include <sys/systm.h>
#include <sys/audit.h>
#include "debvars.h"			/* access to current GPR15 	*/
#include "debaddr.h"			/* Address structure		*/
#include "parse.h"			/* Parser structure.		*/
#include "vdbfmtu.h"			/* declares for this file 	*/
#define  DEF_STORAGE
#include "add_cmd.h"

/*                                                                   
 * GLOBAL VARIABLES USED 
 */
extern ulong g_kxsrval;			/* kernel extension seg reg value */

struct uthread *read_utarea();


/*
 * NAME: fmtut
 *                                                                    
 * FUNCTION:   Provide a formatted display of UT area.
 *                                                                    
 * RETURN VALUE:  none.
 */  
void 
fmtut(ps)
struct parse_out *ps;
{
	int seg_id;
	struct uthread *ut, *uthread_area;
	ulong tid;

	/* 
	 * Clear the screen.
	 */
	clrdsp();

#ifdef Debug
	jr_debug();
#endif /* Debug */

	/* check arg count  */
	if(ps->num_tok < 1) {
	    	if(curthread != NULL){
		     printf("using current thread:\n");
		     tid = curthread->t_tid;
		}
	    	else {
			printf("usage: uthread tid \n");
			return;
		}
	}
	else {
		tid = ps->token[1].hv;
	}
	
	/* find the TID we are interested in */
	if((seg_id=get_thread(tid,&ut))<0)
		return;

	/* copy the utarea using Get_from_memory() */
	if((int)(uthread_area=read_utarea(seg_id,ut)) == -1) {
		printf("Uthread area unreachable.\n");
		return;
	}

	/* print utarea */
	pr_ut(uthread_area,tid);

	printf("Uthread area printout terminated.\n");
}



/*
 *  Function:  set seg reg 2 for us, copy utarea into buffer, reset seg reg 2.
 *		utarea is entirely pinned and less than a page.
 *
 *  Returns:	-1 on failure; else address of our copy of utarea.
 *
*/
struct uthread *
read_utarea(seg_id,ut)
struct uthread *ut;
{
	int orig_sr2;
	struct uthread *x;

	/* set sr2 for the utarea we are interested in */
	orig_sr2 = aim_sr(seg_id,ut);	/* set up seg reg 2 for our needs */

	/* copy utarea to our buffer */

        if (!Get_from_memory ((char *)ut, VIRT, cbbuf, sizeof(struct uthread))){
		/* reset sr2 to original value */
		aim_sr(orig_sr2,ut);
		return ((struct uthread *) -1);
	}

	x = (struct uthread *)cbbuf;

	/* reset sr2 to original value */
	aim_sr(orig_sr2,ut);

	return (x);
}


/*
 *  Function:	find the thread slot we are interested in from the tid 
 *		passed in.
 *
 *  Returns:	-1 on error, the seg id where ut area is to be
 *		found and the ut address if successful
*/
get_thread(tid,ut)
int tid;		/* if tid==0 should we have a reasonable default? */
struct uthread **ut;
{
	register int i;
	struct thread tb;

	for(i=0;i<NTHREAD;i++)  {
		if(!pre_get_put_data(&thread[i], 0/*read*/, (char *)&tb,
			g_kxsrval, VIRT, sizeof(struct thread))){
			printf("No match with TID 0x%x.\n",tid);
			return (-1);
		}
#ifdef Debug
		if(DBG_LVL)
		{
			printf("&thread[%d]=%x,tid=0x%x,", i, &thread[i], tid);
			printf("tbtid=0x%x,procp=0x%x\n", tb.t_tid, tb.t_procp);
		}
#endif /* Debug */
		/* get seg info of utarea */
		if(tb.t_tid == tid) {
			if(tb.t_state!=TSNONE) {
				*ut = tb.t_uthreadp;
				return (tb.t_procp->p_adspace);
			}
			else
				return (-1);
		}
	}
	return (-1);
}


/*
 *  Function:	print utarea 
 *
 *  Returns:	nothing.
*/
pr_ut(x,tid)
struct uthread *x;
{
	char *p,buf[UINFOSIZ+1];
	register int i;
	int cur_page;

	printf("UTHREAD AREA FOR TID 0x%08x\n", tid);
/*
 *		Mstsave structure
 */
	printf("SAVED MACHINE STATE \n");
	printf("    curid:0x%08x  m/q:0x%08x  iar:0x%08x  cr:0x%08x \n",
		x->ut_save.curid,x->ut_save.mq,x->ut_save.iar, x->ut_save.cr);
#ifdef _POWER
	printf("    msr:0x%08x  lr:0x%08x  ctr:0x%08x  xer:0x%08x\n",
		x->ut_save.msr,x->ut_save.lr,x->ut_save.ctr,x->ut_save.xer);
#endif /* _POWER */
   printf("    *prevmst:0x%08x  *stackfix:0x%08x  intpri:0x%08x \n",
		 x->ut_save.prev,x->ut_save.stackfix, x->ut_save.intpri);
   printf("    backtrace:0x%02x  tid:0x%08x  fpeu:0x%02x  ecr:0x%08x\n",
		x->ut_save.backt, x->ut_save.tid, x->ut_save.fpeu,
#ifdef _POWER
		x->ut_save.excp_type);
/*
	printf("    o_iar:0x%08x  o_toc:0x%08x   o_arg1:0x%08x \n",
		x->ut_save.o_iar, x->ut_save.o_toc, x->ut_save.o_arg1);
*/
#endif /* _POWER */

	printf("    Exception Struct\n");
	printf("      0x%08x  0x%08x  0x%08x  0x%08x  0x%08x\n",
		x->ut_save.except[0], x->ut_save.except[1],
		x->ut_save.except[2], x->ut_save.except[3],
		x->ut_save.except[4]);

	printf("    Segment Regs\n    ");
	for(i=0;i<NSRS;i++) {
		if(i<10)
			printf(" %d:0x%08x  ",i,x->ut_save.as.srval[i]);
		else
			printf("%d:0x%08x  ",i,x->ut_save.as.srval[i]);
		if(!((i+1)%4))
			printf("\n    ");
	}
	printf("General Purpose Regs\n    ");
	for(i=0;i<NGPRS;i++) {
		if(i<10)
			printf(" %d:0x%08x  ",i,x->ut_save.gpr[i]);
		else
			printf("%d:0x%08x  ",i,x->ut_save.gpr[i]);
		if(!((i+1)%4))
			printf("\n    ");
	}
	if (i%4)
		printf("\n");

	/** Page Here **/
	if (debpg() == FALSE)
		return;

	printf("Floating Point Regs\n    ");
#ifdef _POWER
	printf("    Fpscr: 0x%08x \n    ",x->ut_save.fpscr);
#endif /* _POWER */
	for(i=0;i<NFPRS;i++) {
		if(i<10)
			printf(" %d:0x%08x 0x%08x ",i,x->ut_save.fpr[i]);
		else
			printf("%d:0x%08x 0x%08x ",i,x->ut_save.fpr[i]);
		if(!((i+1)%3))
			printf("\n    ");
	}
	if(i%4)		/* we didn't print \n and left margin in loop */
		printf("\n    ");

	printf("\n\nKernel stack address: 0x%08x\n", x->ut_kstack);

	/** Page Here **/
	if (debpg() == FALSE)
		return;

/*
 *		System Call Info
 */
	printf("\nSYSTEM CALL STATE \n");
	printf("    user msr:0x%08x\n", x->ut_msr);
	printf("    errnop address:0x%08x  error code:0x%02x  *kjmpbuf:0x%08x\n",
		x->ut_errnopp,x->ut_error,x->ut_save.kjmpbuf);
	print_ut_flags(x);

/*
 *		Auditing data
 */
	if (x->ut_audsvc) {
		printf("\nAUDITING INFORMATION \n");
		printf("    svc#:0x%04x  result:0x%08x\n",
			x->ut_audsvc->svcnum, x->ut_audsvc->status);
		printf("    *audbuf:0x%08x  len:0x%08x  cnt:0x%08x  siz:0x%04x\n",
			x->ut_audsvc->audbuf, x->ut_audsvc->buflen,
			x->ut_audsvc->bufcnt, x->ut_audsvc->bufsiz);
		printf("    argcnt:0x%04x  arg1:0x%08x  arg2:0x%08x  arg3:0x%08x\n",
			x->ut_audsvc->argcnt, x->ut_audsvc->args[0],
			x->ut_audsvc->args[1], x->ut_audsvc->args[2]);
		printf("    arg4:0x%08x  arg5:0x%08x  arg6:0x%08x  arg7:0x%08x\n",
			x->ut_audsvc->args[3], x->ut_audsvc->args[4],
			x->ut_audsvc->args[5], x->ut_audsvc->args[6]);
		printf("    arg8:0x%08x  arg9:0x%08x  arg10:0x%08x \n",
			x->ut_audsvc->args[7], x->ut_audsvc->args[8],
			x->ut_audsvc->args[9]);
	}

/*
 *		Timers info
 */
	printf("\nPER-THREAD TIMER MANAGEMENT \n");
	printf("    Real1 Timer (ut_timer[TIMERID_REAL1]) = 0x%08x\n",
		x->ut_timer[0]);

/*
 *		Signal management
 */
	printf("\nSIGNAL MANAGEMENT \n");
	printf("\n    *sigsp:0x%x  oldmask:hi 0x%x,lo 0x%x  code:0x%x \n",
	     x->ut_sigsp, x->ut_oldmask.hisigs,x->ut_oldmask.losigs,x->ut_code);

	/** Page Here **/
	if (debpg() == FALSE)
		return;

/*
 *		Miscellaneous
 */

	printf("\nMiscellaneous fields:\n");
	printf("   fstid:0x%08x   ioctlrv:0x%08x   selchn:0x%08x\n",
		x->ut_fstid, x->ut_ioctlrv,  x->ut_selchn);
}

print_ut_flags(ut)
struct uthread *ut;
{
	unsigned int flags;

	flags = (unsigned int)(ut->ut_flags);
	printf("  ut_flags:");

	if (flags & UTSTILLUSED) printf(" STILLUSED");
        if (flags & UTSCSIG) printf(" SCSIG");
        if (flags & UTNOULIMIT) printf(" NOULIMIT");
        if (flags & UTSIGONSTACK) printf(" SIGONSTACK");

	flags &= ~(UTSTILLUSED|UTSCSIG|UTNOULIMIT|UTSIGONSTACK);

	if (flags) printf(" unknown: 0x%x",flags);
	printf("\n");
}
#endif
