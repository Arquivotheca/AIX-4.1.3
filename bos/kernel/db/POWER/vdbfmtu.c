static char sccsid[] = "@(#)77	1.43.1.12  src/bos/kernel/db/POWER/vdbfmtu.c, sysdb, bos41J, 9507A 2/8/95 14:39:20";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: fmtu, read_uarea, aim_sr, get_proc, pr_u, Switch_ctxt
 *
 * ORIGINS: 27, 83
 *
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/param.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/ppda.h>
#include <sys/buf.h>
#include <sys/pseg.h>			/* needed for pinned/paged constants */
#include <sys/time.h>			/* for the per-process timer stuff   */
#include <sys/systm.h>
#include "debvars.h"			/* access to current GPR15 	*/
#include "debaddr.h"			/* Address structure		*/
#include "parse.h"			/* Parser structure.		*/
#include "vdbfmtu.h"			/* declares for this file 	*/
#define  DEF_STORAGE
#include "add_cmd.h"

/*                                                                   
 * EXTERNAL PROCEDURES CALLED: 
 */

/*                                                                   
 * GLOBAL VARIABLES USED 
 */
struct user *x;
struct ucred Ucred;
extern ulong	g_kxsrval;		/* kernel extension seg reg value */

struct user *read_uarea();

#ifdef _POWER_MP

extern int switch_ctxt, switch_ctxt_sr1, switch_ctxt_sr2, switch_ctxt_sr13;
#ifdef _THREADS
int switch_ctxt_usr1, switch_ctxt_usr2, switch_ctxt_usr13;
#endif /* _THREADS */

#endif /* POWER_MP */

/*
 * NAME: fmtu
 *                                                                    
 * FUNCTION:   Provide a formatted display of U area.
 *                                                                    
 * RETURN VALUE:  none.
 */  
void
fmtu(ps)
struct parse_out *ps;
{
	int seg_id,paged,longflag;
	struct user *user_area;
	ulong pid;

	/* 
	 * Clear the screen.
	 */
	clrdsp();

#ifdef Debug
	jr_debug();
#endif /* Debug */

	/* check arg count - user might have specified "*" for current process */
	if((ps->num_tok == 0) || (!strcmp(ps->token[1].sv,"*"))) {
	    	if(curproc != NULL){
		     printf("using current process:\n");
		     pid = curproc->p_pid;
		}
	    	else {
			printf("usage: user pid \n");
 			return;
		}
	}
	else {
		pid = ps->token[1].hv;
	}

	/* More than two options is always bad */
	if (ps->num_tok > 2) {
		printf("usage: user pid \n");
		return;
	}

	longflag = FALSE;	/* Assume user wants documented short version */
	/* If second option specified, */
	if (ps->num_tok == 2) {
		/* If it isn't the word "long", that's bad */
		if (strcmp(ps->token[2].sv,"long")) {
			printf("usage: user pid \n");
			return;
		}
		/* User wants undocumented long version of user output */
		longflag = TRUE;
	}
	
	/* find the PID we are intersted in */
	if((seg_id=get_proc(pid))<0)  {
#if defined(_DB_PROC_SLOT)
		if (pid >=0 && proc+pid <= max_proc) {	/* find proc slot */
			pid = proc[pid].p_pid;
			if((seg_id=get_proc(pid))<0)
				return;
		} else
#endif /* _DB_PROC_SLOT */
			return;
	}

	/* copy the uarea using Get_from_memory() */
	if((int)(user_area=read_uarea(seg_id,&paged)) == -1) {
		printf("User area unreachable.\n");
		return;
	}

	/* print uarea */
	pr_u(user_area,pid,paged,longflag);

	printf("User area printout terminated.\n");
}



#ifndef _THREADS
#define PINNED_UAREA (PAGESIZE)
#else /* _THREADS */
#define PINNED_UAREA (PAGESIZE - ((int)&U - (int)&__ublock) - ((int)&__ublock % PAGESIZE))
#endif
/*
 *  Function:  set seg reg 2 for us, copy uarea into buffer, reset seg reg 2.
 *		Since uarea is only half pinned, copy it in 2 chunks.
 *
 *  Returns:	-1 on failure; else address of our copy of uarea.
 *		Sets pages_in to the number of pages past the first page 
 *			that were read in.
*/
struct user *
read_uarea(seg_id,pages_in)
int *pages_in;
{
	register int i;
	int orig_sr2;
	int residual, page_pad;

#ifdef _THREADS
#undef u
#define u U
#endif

	*pages_in = 0;
	/* set sr2 for the uarea we are interested in */
	orig_sr2 = aim_sr(seg_id,&u);	/* set up seg reg 2 for our needs */

	/* copy pinned part of uarea to our buffer */

	if(!Get_from_memory(&u,VIRT,(struct user *)cbbuf,PINNED_UAREA)) {
		/* reset sr2 to original value */
		aim_sr(orig_sr2,&u);	
		return ((struct user *) -1);
	}

	/* copy UNpinned part of uarea to our buffer */
	residual = (sizeof(struct user) - PINNED_UAREA) % PAGESIZE;
	page_pad = (residual)?(PAGESIZE - residual):0;
	for (i = PINNED_UAREA; i < sizeof(struct user)+page_pad; i+= PAGESIZE) {
		if(!Get_from_memory(((char *)&u) + i, VIRT, cbbuf + i,
		    ((i + PAGESIZE) > sizeof(struct user))?residual:PAGESIZE)) {
			break;
		}
		*pages_in += 1;
	}

	x = (struct user *)cbbuf;

	/* get ucred and point u_cred at it so it can be accessed normally */
#ifndef _THREADS
	if(x->u_cred != NULL)
		if(!Get_from_memory(x->u_cred,VIRT,&Ucred,sizeof(struct ucred))){
			x->u_cred = NULL;
		}
		else
			x->u_cred = &Ucred;
#else /* _THREADS */
	if(x->U_cred != NULL)
		if(!Get_from_memory(x->U_cred,VIRT,&Ucred,sizeof(struct ucred))){
			x->U_cred = NULL;
		}
		else
			x->U_cred = &Ucred;
#endif

	/* reset sr2 to original value */
	aim_sr(orig_sr2,&u);	

#ifdef _THREADS
#undef u
#endif

	return (x);
}


/*
 *  Function:  put seg id in debvars so get_put_data() will find
 *		the memory segment we are interested in.
 *		Uses addr to determine which segment register to use.
 *
 *  Returns:	value previously in the seg register
*/
aim_sr(seg_id,addr)
ulong seg_id,addr;
{
	int sid;

	sid = debvars[IDSEGS+(addr>>SEGSHIFT)].hv;
	debvars[IDSEGS+(addr>>SEGSHIFT)].hv = seg_id;
	return sid;
}


/*
 *  Function:	find the proc slot we are intersted in from the pid 
 *		passed in.
 *
 *  Returns:	-1 on error, the seg id where u area is to be
 *		found if successful
*/
get_proc(pid)
int pid;		/* if pid==0 should we have a reasonable default? */
{
	register int i;
	struct proc pb;

	for(i=0;i<NPROC;i++)  {
		if(!pre_get_put_data(&proc[i], 0/*read*/, (char *)&pb,
			g_kxsrval, VIRT, sizeof(struct proc))){
			printf("No match with PID 0x%x.\n",pid);
			return (-1);
		}
#ifdef Debug
		if(DBG_LVL)
		{
			printf("&proc[%d]=%x,pid=0x%x,", i, &proc[i], pid);
			printf("pbpid=0x%x,adspace=0x%x\n", pb.p_pid,
				pb.p_adspace);
		}
#endif /* Debug */
		/* get seg info of uarea */
		if(pb.p_pid == pid) {
			if(pb.p_stat!=SNONE) {
				return (pb.p_adspace);
			}
			else
				return (-1);
		}
	}
	return (-1);
}


/*
 *  Function:	print uarea 
 *
 *  Returns:	nothing.
*/
pr_u(x,pid,pages_in,longflag)
struct user *x;
int longflag;
{
	char *p,buf[UINFOSIZ+1];
	register int i,j;
	int cur_page;

#ifndef _THREADS
	printf("USER AREA FOR %s (PID 0x%08x,PROCTAB 0x%08x)\n", 
		x->u_comm,pid,x->u_procp);
/*
 *		Mstsave structure
 */
	printf("SAVED MACHINE STATE \n");
	printf("    curid:0x%08x  m/q:0x%08x  iar:0x%08x  cr:0x%08x \n",
		x->u_save.curid,x->u_save.mq,x->u_save.iar, x->u_save.cr);
#ifdef _IBMRT
	printf("    ics:0x%04x  ccr_access:0x%02x  mcs&pcs:0x%04x \n",
		x->u_save.ics,x->u_save.ccr_access,x->u_save.mcs_pcs);
#endif /* _IBMRT */
#ifdef _POWER
	printf("    msr:0x%08x  lr:0x%08x  ctr:0x%08x  xer:0x%08x\n",
		x->u_save.msr,x->u_save.lr,x->u_save.ctr,x->u_save.xer);
#endif /* _POWER */
   printf("    *prevmst:0x%08x  *stackfix:0x%08x  intpri:0x%08x \n",
		 x->u_save.prev,x->u_save.stackfix, x->u_save.intpri);
   printf("    backtrace:0x%02x  tid:0x%08x  fpeu:0x%02x  ecr:0x%08x\n",
		x->u_save.backt, x->u_save.tid, x->u_save.fpeu,
#ifdef _IBMRT
		x->u_save.ecr);
#endif /* _IBMRT */
#ifdef _POWER
		x->u_save.excp_type);
/*
	printf("    o_iar:0x%08x  o_toc:0x%08x   o_arg1:0x%08x \n",
		x->u_save.o_iar, x->u_save.o_toc, x->u_save.o_arg1);
*/
#endif /* _POWER */
#ifdef _IBMRT
	printf("    mstpad:0x%08x\n", x->u_save.mstpad);
	printf("    Other Fields\n");
	printf("      clxs:0x%08x  fpset:0x%02x  mvbuff:0x%02x\n",
		x->u_save.clxs, x->u_save.fpset, x->u_save.mvbuff);
	printf("      lkct:0x%04x  fpmask:0x%08x  clr1:0x%08x\n",
		x->u_save.lkct, x->u_save.fpmask, x->u_save.clr1);
	printf("      clr2:0x%08x\n", x->u_save.clr2);

	printf("    Restart Structure\n");
	printf("      ectl:0x%08x   addr:0x%08x   data:0x%08x\n",
		x->u_save.restart.ectl, x->u_save.restart.addr, 
		x->u_save.restart.data);
	printf("      hw1:0x%08x    ectl2:0x%08x  addr2:0x%08x\n",
		x->u_save.restart.hw1, x->u_save.restart.ectl2, 
		x->u_save.restart.addr2);
	printf("      data2:0x%08x  hw2:0x%08x\n",
		x->u_save.restart.data2, x->u_save.restart.hw2);
#endif /*_IBMRT */

	printf("    Exception Struct\n");
	printf("      0x%08x  0x%08x  0x%08x  0x%08x  0x%08x\n",
		x->u_save.except[0], x->u_save.except[1],
		x->u_save.except[2], x->u_save.except[3],
		x->u_save.except[4]);


	printf("    Segment Regs\n    ");
	for(i=0;i<NSRS;i++) {
		if(i<10)
			printf(" %d:0x%08x  ",i,x->u_save.as.srval[i]);
		else
			printf("%d:0x%08x  ",i,x->u_save.as.srval[i]);
		if(!((i+1)%4))
			printf("\n    ");
	}
	printf("General Purpose Regs\n    ");
	for(i=0;i<NGPRS;i++) {
		if(i<10)
			printf(" %d:0x%08x  ",i,x->u_save.gpr[i]);
		else
			printf("%d:0x%08x  ",i,x->u_save.gpr[i]);
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
	printf("    Fpscr: 0x%08x \n    ",x->u_save.fpscr);
#endif /* _POWER */
	for(i=0;i<NFPRS;i++) {
		if(i<10)
			printf(" %d:0x%08x 0x%08x ",i,x->u_save.fpr[i]);
		else
			printf("%d:0x%08x 0x%08x ",i,x->u_save.fpr[i]);
		if(!((i+1)%3))
			printf("\n    ");
	}
	if(i%4)		/* we didn't print \n and left margin in loop */
		printf("\n    ");


/*
 *		System Call Info
 */
	printf("\nSYSTEM CALL STATE \n");
	printf("    error count:0x%04x  error code:0x%02x  *kjmpbuf:0x%08x\n",
		x->u_errcnt,x->u_error,x->u_save.kjmpbuf);

	/** Page Here **/
	if (debpg() == FALSE)
		return;

	printf("\nPER-PROCESS TIMER MANAGEMENT\n");
	printf("Real/Alarm Timer (u_timer.t_trb[TIMERID_ALRM]) = 0x%08x\n",
		x->u_timer.t_trb[TIMERID_ALRM]);
	printf("Virtual Timer (u_timer.t_trb[TIMERID_VIRTUAL]) = 0x%08x\n",
		x->u_timer.t_trb[TIMERID_VIRTUAL]);
	printf("Prof Timer (u_timer.t_trb[TIMERID_PROF]) = 0x%08x\n",
		x->u_timer.t_trb[TIMERID_PROF]);
	for(i = TIMERID_TOD; i < NTIMEOFDAY; i++)
	{
		printf("Posix Timer (u_timer.t_trb[POSIX%d]) = 0x%08x\n",
			i, x->u_timer.t_trb[TIMERID_TOD + i]);
	}

	printf ("    *semundo:0x%08x\n", x -> u_semundo);

	/** Page Here **/
	if (debpg() == FALSE)
		return;

/*
 *		Signal management
 */
	printf("\nSIGNAL MANAGEMENT \n");
	printf("    Signals to be blocked (sig#:hi/lo mask,flags,&func)\n    ");
	for(i=1;i<NSIG;i++) {
		if(i<=9)
			printf(" %d:hi 0x%08x,lo 0x%08x,0x%08x,0x%08x  ",i,
				x->u_sigmask[i].hisigs, x->u_sigmask[i].losigs,
				x->u_sigflags[i],x->u_signal[i]);
		else
			printf("%d:hi 0x%08x,lo 0x%08x,0x%08x,0x%08x  ",i,
				x->u_sigmask[i].hisigs, x->u_sigmask[i].losigs,
				x->u_sigflags[i],x->u_signal[i]);
		printf("\n    ");
		if(!(i%16)) {
			/** Page Here **/
			if (debpg() == FALSE)
				return;
			printf("    ");
		}
	}
	printf("\n    *sigsp:0x%x  oldmask:hi 0x%x,lo 0x%x  code:0x%x \n", 
		x->u_sigsp, x->u_oldmask.hisigs,x->u_oldmask.losigs,x->u_code);
	printf("    *sigctx:0x%08x\n",x->u_sigctx);

	/** Page Here **/
	if (debpg() == FALSE)
		return;

/*
 *		User Info
 */
	printf("\nUSER INFORMATION \n");
	if(!x->u_cred)
		printf("    CRED Structure Not Found\n");
	else {
	printf(
	"    euid:0x%04x  egid:0x%04x  ruid:0x%04x  rgid:0x%04x  luid:0x%08x\n",
		x->u_uid, x->u_gid, x->u_ruid, x->u_rgid,x->u_luid);
	printf("    suid:0x%08x  ngrps:0x%04x  *groups:0x%08x  compat:0x%08x\n",
		x->u_suid, x ->u_cred->cr_ngrps, x->u_cred->cr_groups,
		x->u_compatibility);
	printf ("    ref:0x%08x\n", x -> u_cred->cr_ref);
	printf ("    acctid:0x%08x   sgid:0x%08x   epriv:0x%08x\n",
		x->u_acctid, x->u_sgid, x->u_epriv);
	printf ("    ipriv:0x%08x   bpriv:0x%08x   mpriv:0x%08x\n",
		x->u_ipriv, x->u_bpriv, x->u_mpriv);
	} /* end if(cred) not found */

	printf("\n");
	p = x->u_uinfo;
	while(p < &x->u_uinfo[UINFOSIZ]) {
		if(*p=='\0') 
			*p == ' ';
		p++;
	}
	*(p-1)='\0';
	printf("    u_info: %s\n",x->u_uinfo);

/*
 *		Accounting and Profiling Data
 */
    	printf("\nACCOUNTING DATA \n");
    	printf("    start:0x%08x  ticks:0x%08x  acflag:0x%04x  ",
		x->u_start,x->u_ticks,x->u_acflag);
	printf("pr_base:0x%08x\n", x->u_prof.pr_base);
  	printf("    pr_size:0x%08x  pr_off:0x%08x  pr_scale:0x%08x\n",
		x->u_prof.pr_size, x->u_prof.pr_off, x->u_prof.pr_scale);
    	printf("    process times:\n");
	printf("           user:0x%08xs 0x%08xus\n",
		x->u_ru.ru_utime.tv_sec, x->u_ru.ru_utime.tv_usec);
	printf("            sys:0x%08xs 0x%08xus\n",
		x->u_ru.ru_stime.tv_sec, x->u_ru.ru_stime.tv_usec);
    	printf("    children's times:\n");
	printf("           user:0x%08xs 0x%08xus\n",
		x->u_cru.ru_utime.tv_sec, x->u_cru.ru_utime.tv_usec);
	printf("            sys:0x%08xs 0x%08xus\n",
		x->u_cru.ru_stime.tv_sec, x->u_cru.ru_stime.tv_usec);

/*
 *		Controlling TTY info
 */
    	printf("\nCONTROLLING TTY \n");
    	printf("    *ttysid:0x%08x  *ttyp(pgrp):0x%08x\n",
	       x->u_ttysid, x->u_ttyp);
    	printf("    ttyd(evice):0x%08x  ttympx:0x%08x  *ttys(tate):0x%08x\n",
	       x->u_ttyd, x->u_ttympx, x->u_ttys);
	printf("    tty id: 0x%08x  *query function: 0x%08x\n",
		x -> u_ttyid, x -> u_ttyf);

	/** Page Here **/

	if (debpg() == FALSE)
		return;

	/* Pinned profiling buffer */
	printf ("PINNED PROFILING BUFFER\n");
	printf("    *pprof: 0x%08x  *mem desc: 0x%08x\n",x->u_pprof, x->u_dp);

/*
 *		Resource Limits data
 */
    	printf("\nRESOURCE LIMITS AND COUNTERS\n");
	printf("    ior:0x%08x  iow:0x%08x  ioch:0x%08x\n",
		x->u_ior,x->u_iow, x->u_ioch);
	printf("    text:0x%08x  data:0x%08x  stk:0x%08x\n",
		x->u_tsize, x->u_dsize, x->u_ssize);
	printf("    max data:0x%08x  max stk:0x%08x  max file:0x%08x\n",
		x->u_dmax, x->u_smax,x->u_limit);
	printf("    soft core dump:0x%08x  hard core dump:0x%08x\n",
		x->u_rlimit[RLIMIT_CORE].rlim_cur,
		x->u_rlimit[RLIMIT_CORE].rlim_max);
	printf("    soft rss:0x%08x  hard rss:0x%08x\n",
		x->u_rlimit[RLIMIT_RSS].rlim_cur,
		x->u_rlimit[RLIMIT_RSS].rlim_max);
	printf("    cpu soft:0x%08x  cpu hard:0x%08x\n",
		x->u_rlimit[RLIMIT_CPU].rlim_cur,
		x->u_rlimit[RLIMIT_CPU].rlim_max);
	printf("    hard ulimit:0x%08x\n", x->u_rlimit[RLIMIT_FSIZE].rlim_max);
	printf("    minflt:0x%08x   majflt:0x%08x\n",
		x->u_minflt, x->u_majflt);

	/** Page Here **/
	if (debpg() == FALSE)
		return;
/*
 *		Executable header data
 */
    	printf("\nEXECUTABLE HEADER INFORMATION \n");
	printf("    sep I&D flag:0x%08x  intflg:0x%08x  lock:0x%08x\n",
		x->u_sep, x->u_intflg, x->u_lock);
	if(x->u_exh.u_exshell[0]=='#' && x->u_exh.u_exshell[1]=='!')
	{
		x->u_exh.u_exshell[SHSIZE-1] = '\0';
		printf("    command interpreter: %s\n", x->u_exh.u_exshell);
    	}
	else		/* this is really an executable header */
	{
#define fhdr u_exh.u_xcoffhdr.filehdr
	printf(
	   "    File Header: magic:0x%04x  sections:0x%04x  timestamp:0x%08x\n",
		x->fhdr.f_magic, x->fhdr.f_nscns, x->fhdr.f_timdat);
	printf( "     *symtab:0x%08x  opthdr:0x%04x  flags:0x%04x\n",
		x->fhdr.f_symptr, x->fhdr.f_opthdr, x->fhdr.f_flags);
#define ahdr u_exh.u_xcoffhdr.aouthdr
	printf(
	   "    A.out Header: magic:0x%04x  version:0x%04x  text size:0x%08x\n",
		x->ahdr.magic, x->ahdr.vstamp, x->ahdr.tsize);
	printf( "     data size:0x%04x  bss size:0x%04x  entry:0x%08x\n",
		x->ahdr.dsize, x->ahdr.dsize, x->ahdr.entry);
	printf( "     text start:0x%08x  data start:0x%08x  &toc:0x%08x\n",
		x->ahdr.text_start, x->ahdr.data_start, x->ahdr.o_toc);
	printf( "     entry sec#:0x%04x  text sec#:0x%04x  data sec#:0x%04x\n",
		x->ahdr.o_snentry, x->ahdr.o_sntext, x->ahdr.o_sndata);
	printf( 
  "     toc sec#:0x%04x  loader sec#:0x%04x  algntxt:0x%04x  algndata:0x%04x\n",
		x->ahdr.o_sntoc, x->ahdr.o_snloader, x->ahdr.o_algntext,
		x->ahdr.o_algndata);
	}

/*
 *		auditing data
 */
	printf("\nAUDITING INFORMATION \n");
	printf("    auditmask:0x%08x  auditstatus:0x%08x  svc#:0x%04x  result:0x%08x\n",
		x->u_procp->p_auditmask, x->u_auditstatus, x->u_audsvc.svcnum, x->u_audsvc.status);
	printf("    *audbuf:0x%08x  len:0x%08x  cnt:0x%08x  siz:0x%04x\n",
		x->u_audsvc.audbuf, x->u_audsvc.buflen, x->u_audsvc.bufcnt,
		x->u_audsvc.bufsiz);
	printf("    argcnt:0x%04x  arg1:0x%08x  arg2:0x%08x  arg3:0x%08x  \n",  
		x->u_audsvc.argcnt,x->u_audsvc.args[0], x->u_audsvc.args[1],
		x->u_audsvc.args[2]);
	printf("    arg4:0x%08x  arg5:0x%08x  arg6:0x%08x  arg7:0x%08x\n",  
		x->u_audsvc.args[3], x->u_audsvc.args[4], x->u_audsvc.args[5],
		x->u_audsvc.args[6]);
	printf("    arg8:0x%08x  arg9:0x%08x  arg10:0x%08x \n",  
		x->u_audsvc.args[7], x->u_audsvc.args[8], x->u_audsvc.args[9]);

	/** Page Here **/
	if (debpg() == FALSE)
		return;
	printf("\nSEGMENT REGISTER INFORMATION \n");
     	printf("    Reg        Flag      Fileno     Pointer  \n");
	for(i=0;i<NSEGS;i++) {
		if(i<10) 	/* for alignment */
			printf(" ");
		printf("    %d    %8x   %8x   %8x\n", i,x->u_segst[i].segflag,
				x->u_segst[i].segfileno,x->u_segst[i].u_ptrs);
	}
	printf("    adspace:0x%08x\n", x -> u_adspace);

	/** Page Here **/
	if (debpg() == FALSE)
		return;

/*
  *		File System Info
 */
	printf("FILE SYSTEM STATE \n");
    	printf("    *curdir:0x%08x  *rootdir:0x%08x\n",
		x->u_cdir,x->u_rdir);
    	printf("    *parent:0x%08x\n", x->u_pdir);
    	printf("    cmask:0x%04x  *lastname:0x%08x\n",x->u_cmask,x->u_lastname);

    	printf( "    maxindex:0x%04x\n", x->u_maxofile);

	printf("FILE DESCRIPTOR TABLE\n");
	printf("    *ufd: 0x%08x\n", x->u_ufd);	
	if (debpg() == FALSE)
		return;
	for (i = 0, cur_page = 0; i < OPEN_MAX; i++) {
		if (((((int)&x->u_ofile(i) - (int)x)%PAGESIZE) == 0) ||
		    ((((int)&x->u_pofile(i) - (int)x)%PAGESIZE) == 0)) 
			++cur_page;
		if(cur_page > pages_in) {
			printf("Rest of user area paged out. \n");
			return;
		}
		printf("    fd %d:  fp = 0x%08x   flags = 0x%08x\n",i, 
			x->u_ofile(i),x->u_pofile(i));
		if ((i+1)%22 == 0) {
			if (debpg() == FALSE)
				return;
		}
	}
#else /* _THREADS */
	printf("USER AREA FOR %s (PID 0x%08x,PROCTAB 0x%08x)\n", 
		x->U_comm,pid,x->U_procp);

	printf ("    handy_lock:0x%08x  timer_lock:0x%08x\n",
		x->U_handy_lock, x->U_timer_lock);
	printf ("    map:0x%08x  *semundo:0x%08x\n",
		x->U_map, x->U_semundo);
	printf ("    compatibility:0x%08x  lock:0x%08x\n",
		x->U_compatibility, x->U_lock);
	printf ("    ulocks:0x%08x\n",
		x->U_ulocks);

	printf("\nPER-PROCESS TIMER MANAGEMENT\n");
        printf("Real/Alarm Timer (U_timer[TIMERID_ALRM]) = 0x%08x\n",
                x->U_timer[TIMERID_ALRM]);
        printf("Virtual Timer (U_timer[TIMERID_VIRTUAL]) = 0x%08x\n",
                x->U_timer[TIMERID_VIRTUAL]);
        printf("Prof Timer (U_timer[TIMERID_PROF]) = 0x%08x\n",
                x->U_timer[TIMERID_PROF]);
        printf("Virt Timer (U_timer[TIMERID_VIRT]) = 0x%08x\n",
                x->U_timer[TIMERID_VIRT]);
        for(i = TIMERID_TOD; i < NTIMEOFDAY; i++)
        {
                printf("Posix Timer (U_timer[POSIX%d]) = 0x%08x\n",
                        i, x->U_timer[TIMERID_TOD + i]);
        }

	/* If user asked for long output, give U_adspace data */
	if (longflag == TRUE) {
		printf("USER-MODE ADDRESS SPACE\n");
		printf("    allocation flags:0x%08x\n", x->U_adspace.alloc);
		for (i=0; i<16; i+=4) {
			printf("    segment regs %2d - %2d:", i, i+3);
			for (j=i; j<i+4; j++)
				printf(" %08x ", x->U_adspace.srval[j]);
			printf("\n");
		}
	}

	/** Page Here **/
	if (debpg() == FALSE)
		return;

/*
 *		Signal management
 */
	printf("\nSIGNAL MANAGEMENT \n");
	printf("    Signals to be blocked (sig#:hi/lo mask,flags,&func)\n");
	j = 0;  /* Initialize skip pointer to say nothing skipped yet */
	for(i=1,cur_page=1;i<NSIG;i++,cur_page++) {
	        if (longflag == TRUE && i>1) {
			/* Skip every signal item that is identical to the */
			/* previous one.  We will display a single line */
			while (
			    (x->U_sigmask[i].hisigs == x->U_sigmask[i-1].hisigs) && 
			    (x->U_sigmask[i].losigs == x->U_sigmask[i-1].losigs) && 
			    (x->U_sigflags[i] == x->U_sigflags[i-1]) &&
			    (x->U_signal[i] == x->U_signal[i-1])) {
				if (j==0)  /* If this is the first line skipped, */
					j=i;	/* remember where we started. */
				i++;		/* Go on to next item */
				if (i==NSIG)	/* Are we at the end? */
					break;  /* We're done */
			}
			if (j) {		/* Did we skip anything? */
				/* Yes, tell the user what we skipped */
				printf("    Signals %d - %d identical\n",j,i-1);
				cur_page++;	/* Count this printf on this page */
				j = 0;		/* Reset j (we stopped skipping) */
				if (i==NSIG)	/* Make sure we don't display a */
					break;	/* bogus signal (one beyond end) */
			}  			/* End of while loop */
		}				/* end of if */

		printf("    %2d:hi 0x%08x,lo 0x%08x,0x%08x,0x%08x  ",i,
			x->U_sigmask[i].hisigs, x->U_sigmask[i].losigs,
			x->U_sigflags[i],x->U_signal[i]);
		printf("\n");
		if(cur_page>=16) {
			/** Page Here **/
			if (debpg() == FALSE)
				return;
			cur_page = 0;		/* Reset counter for next page */
		}
	}
	printf("    \n");

	/** Page Here **/
	if (debpg() == FALSE)
		return;

/*
 *		User Info
 */
	printf("USER INFORMATION \n");
	printf("    cred_lock:0x%08x\n", x->U_cr_lock);
	if(!x->U_cred)
		printf("    CRED Structure Not Found\n");
	else {
	printf(
	"    euid:0x%04x  egid:0x%04x  ruid:0x%04x  rgid:0x%04x  luid:0x%08x\n",
		x->U_cred->cr_uid, x->U_cred->cr_gid, x->U_cred->cr_ruid,
		x->U_cred->cr_rgid, x->U_cred->cr_luid);
	printf("    suid:0x%08x  ngrps:0x%04x  *groups:0x%08x  compat:0x%08x\n",
		x->U_cred->cr_suid, x->U_cred->cr_ngrps, x->U_cred->cr_groups,
		x->U_compatibility);
	printf ("    ref:0x%08x\n", x ->U_cred->cr_ref);
	printf ("    acctid:0x%08x   sgid:0x%08x   epriv:0x%08x\n",
		x->U_cred->cr_acctid, x->U_cred->cr_sgid, x->U_cred->cr_epriv);
	printf ("    ipriv:0x%08x   bpriv:0x%08x   mpriv:0x%08x\n",
		x->U_cred->cr_ipriv, x->U_cred->cr_bpriv, x->U_cred->cr_mpriv);
	} /* end if(cred) not found */

	printf("\n");
	p = x->U_uinfo;
	while(p < &x->U_uinfo[UINFOSIZ]) {
		if(*p=='\0') 
			*p == ' ';
		p++;
	}
	*(p-1)='\0';
	printf("    u_info: %s\n",x->U_uinfo);

/*
 *		Accounting and Profiling Data
 */
    	printf("\nACCOUNTING DATA \n");
    	printf("    start:0x%08x  ticks:0x%08x  acflag:0x%04x  ",
		x->U_start,x->U_ticks,x->U_acflag);
	printf("pr_base:0x%08x\n", x->U_prof.pr_base);
  	printf("    pr_size:0x%08x  pr_off:0x%08x  pr_scale:0x%08x\n",
		x->U_prof.pr_size, x->U_prof.pr_off, x->U_prof.pr_scale);
    	printf("    process times:\n");
	printf("           user:0x%08xs 0x%08xus\n",
		x->U_ru.ru_utime.tv_sec, x->U_ru.ru_utime.tv_usec);
	printf("            sys:0x%08xs 0x%08xus\n",
		x->U_ru.ru_stime.tv_sec, x->U_ru.ru_stime.tv_usec);
    	printf("    children's times:\n");
	printf("           user:0x%08xs 0x%08xus\n",
		x->U_cru.ru_utime.tv_sec, x->U_cru.ru_utime.tv_usec);
	printf("            sys:0x%08xs 0x%08xus\n",
		x->U_cru.ru_stime.tv_sec, x->U_cru.ru_stime.tv_usec);

	/** Page Here **/

	if (debpg() == FALSE)
		return;

/*
 *		Controlling TTY info
 */
    	printf("\nCONTROLLING TTY \n");
    	printf("    *ttysid:0x%08x  *ttyp(pgrp):0x%08x\n",
	       x->U_ttysid, x->U_ttyp);
    	printf("    ttyd(evice):0x%08x  ttympx:0x%08x  *ttys(tate):0x%08x\n",
	       x->U_ttyd, x->U_ttympx, x->U_ttys);
	printf("    tty id: 0x%08x  *query function: 0x%08x\n",
		x -> U_ttyid, x -> U_ttyf);


	/* Pinned profiling buffer */
	printf ("PINNED PROFILING BUFFER\n");
	printf("    *pprof: 0x%08x  *mem desc: 0x%08x\n",x->U_pprof, x->U_dp);

/*
 *		Resource Limits data
 */
    	printf("\nRESOURCE LIMITS AND COUNTERS\n");
	printf("    ior:0x%08x  iow:0x%08x  ioch:0x%08x\n",
		x->U_ior,x->U_iow, x->U_ioch);
	printf("    text:0x%08x  data:0x%08x  stk:0x%08x\n",
		x->U_tsize, x->U_dsize, x->U_ssize);
	printf("    max data:0x%08x  max stk:0x%08x  max file:0x%08x\n",
		x->U_dmax, x->U_smax,x->U_limit);
	printf("    soft core dump:0x%08x  hard core dump:0x%08x\n",
		x->U_rlimit[RLIMIT_CORE].rlim_cur,
		x->U_rlimit[RLIMIT_CORE].rlim_max);
	printf("    soft rss:0x%08x  hard rss:0x%08x\n",
		x->U_rlimit[RLIMIT_RSS].rlim_cur,
		x->U_rlimit[RLIMIT_RSS].rlim_max);
	printf("    cpu soft:0x%08x  cpu hard:0x%08x\n",
		x->U_rlimit[RLIMIT_CPU].rlim_cur,
		x->U_rlimit[RLIMIT_CPU].rlim_max);
	printf("    hard ulimit:0x%08x\n", x->U_rlimit[RLIMIT_FSIZE].rlim_max);
	printf("    minflt:0x%08x   majflt:0x%08x\n",
		x->U_minflt, x->U_majflt);

/*
 *		auditing data
 */
	printf("\nAUDITING INFORMATION \n");
	printf("    auditmask:0x%08x  auditstatus:0x%08x\n",
		x->U_procp->p_auditmask, x->U_auditstatus);

	/** Page Here **/
	if (debpg() == FALSE)
		return;
	printf("\nSEGMENT REGISTER INFORMATION \n");
     	printf("    Reg     Flag        NumSegs     FileDesc/ShmDesc/Srval\n");
	for(i=0;i<NSEGS;i++) {
		if(i<10) 	/* for alignment */
			printf(" ");
	printf("    %d      0x%04x       0x%04x              0x%08x\n", 
			i, x->U_segst[i].segflag,
			   x->U_segst[i].num_segs, 
                           x->U_segst[i].u_ptrs.srval);
	}
	printf("    adspace:0x%08x\n", x->U_adspace);
	printf("    adspace.lock_word:0x%08x  *adspace.vmm_lock_wait:0x%08x\n",
			x->U_lock_word, x->U_vmm_lock_wait);

	/** Page Here **/
	if (debpg() == FALSE)
		return;

/*
  *		File System Info
 */
	printf("FILE SYSTEM STATE \n");
    	printf("    *curdir:0x%08x  *rootdir:0x%08x\n",
		x->U_cdir,x->U_rdir);
    	printf("    cmask:0x%04x  maxindex:0x%04x\n",x->U_cmask,x->U_maxofile);
	if (longflag == TRUE) {
		printf("    fso_lock:0x%08x lockflag:0x%08x fdevent:0x%08x\n",
			x->U_fso_lock, x->U_lockflag, x->U_fdevent);
	}

	printf("FILE DESCRIPTOR TABLE\n");
	printf("    *ufd: 0x%08x\n", x->U_ufd);	
	if (debpg() == FALSE)
		return;
	for (i = 0, cur_page = 0; i < OPEN_MAX; i++) {
		if (((((int)&x->U_ofile(i) - (int)x)%PAGESIZE) == 0) ||
		    ((((int)&x->U_pofile(i) - (int)x)%PAGESIZE) == 0)) 
			++cur_page;
		if(cur_page > pages_in) {
			printf("Rest of user area paged out. \n");
			return;
		}
		printf("    fd %d:  fp = 0x%08x   flags = 0x%08x",i, 
			x->U_ofile(i),x->U_pofile(i));
		if (longflag == TRUE)
			printf("   count = 0x%04x", x->U_ufd[i].count);
		printf("\n");
		if ((i+1)%22 == 0) {
			if (debpg() == FALSE)
				return;
		}
	}
#endif  /* _THREADS */
}

#ifdef _POWER_MP

/*
 * NAME: Switch_ctxt
 *                                                                    
 * FUNCTION:   Updates the switch_ctxt data, used by Bdisplay.
 *             The parameter is the slot of the target process.
 *                                                       
 * RETURN VALUE:  -1 if pb, 0 if proc, address of target mst if threads
 */  
int
Switch_ctxt(ps)
struct parse_out *ps;
{
	int seg_id,paged, i;
	struct user *user_area;

#ifdef _THREADS
	struct uthread *uthread_area;
	ulong save_sr;
	int switchuk = 0;
#endif /* THREADS */	
	ulong slot;

#ifndef _THREADS
	struct proc pb;
	if ((slot = ps->token[1].hv) > NPROC)
		return -1;
#else /* THREADS */
	struct thread tb;
	if ((slot = ps->token[1].hv) > NTHREAD){
		if (slot != INT_MAX)  /* toggle to switch from kernel to user space or the opposite */
			return -1;
		/* handler of kernel-user toggle */
		if (switch_ctxt == TRUE){
			save_sr = switch_ctxt_sr1;
			switch_ctxt_sr1 = switch_ctxt_usr1;
			switch_ctxt_usr1 = save_sr;
			save_sr = switch_ctxt_sr2;
			switch_ctxt_sr2 = switch_ctxt_usr2;
			switch_ctxt_usr2 = save_sr;
			save_sr = switch_ctxt_sr13;
			switch_ctxt_sr13 = switch_ctxt_usr13;
			switch_ctxt_usr13 = save_sr;
			return;
			}
		else {
			slot = ((int)(ppda[db_get_processor_num()]._curthread) - (int)&thread)
				/sizeof(struct thread);
			switchuk = 1;
		}
	}
#endif /* THREADS */
		
#ifndef _THREADS
	if(!pre_get_put_data(&proc[slot], 0/*read*/, (char *)&pb,
		g_kxsrval, VIRT, sizeof(struct proc)))
		return (-1);
	

	if(pb.p_stat!=SNONE) {
		seg_id = pb.p_adspace;
		/* copy the uarea using Get_from_memory() */
		if((int)(user_area=read_uarea(seg_id,&paged)) == -1)
			return -1;
				
	
		/* Updates now the switch_ctxt data, used by Bdisplay. */
		switch_ctxt = TRUE;
		switch_ctxt_sr1 = user_area->u_save.as.srval[1];
		switch_ctxt_sr2 = user_area->u_save.as.srval[2];
		switch_ctxt_sr13 = user_area->u_save.as.srval[13];
		return 0;
	}

#else /* THREADS */
	if(!pre_get_put_data(&thread[slot], 0/*read*/, (char *)&tb,
		g_kxsrval, VIRT, sizeof(struct thread)))
		return (-1);

	if(tb.t_state!=TSNONE) {
		seg_id = tb.t_procp->p_adspace;
		/* copy the uarea using Get_from_memory() */
		if((int)(uthread_area=read_utarea(seg_id,tb.t_uaddress.uthreadp)) == -1)
			return -1;
		
		/* Updates now the switch_ctxt data, used by Bdisplay. */
		switch_ctxt = TRUE;
		switch_ctxt_sr1 = uthread_area->ut_save.as.srval[1];
		switch_ctxt_sr2 = uthread_area->ut_save.as.srval[2];
		switch_ctxt_sr13 = uthread_area->ut_save.as.srval[13];
		if((int)(user_area=read_uarea(seg_id,&paged)) == -1) {
			switch_ctxt_usr1 = switch_ctxt_sr1 ;
			switch_ctxt_usr2 = switch_ctxt_sr2 ;
			switch_ctxt_usr13 = switch_ctxt_sr13 ;
		}
		else {
			switch_ctxt_usr1 = (ulong) (user_area->U_adspace).srval[1];
			switch_ctxt_usr2 = (ulong) (user_area->U_adspace).srval[2];
			switch_ctxt_usr13 = (ulong) (user_area->U_adspace).srval[13];
		}
		if (switchuk == TRUE){
			save_sr = switch_ctxt_sr1;
			switch_ctxt_sr1 = switch_ctxt_usr1;
			switch_ctxt_usr1 = save_sr;
			save_sr = switch_ctxt_sr2;
			switch_ctxt_sr2 = switch_ctxt_usr2;
			switch_ctxt_usr2 = save_sr;
			save_sr = switch_ctxt_sr13;
			switch_ctxt_sr13 = switch_ctxt_usr13;
			switch_ctxt_usr13 = save_sr;
		}
		return ((int) (tb.t_uaddress.uthreadp));
	}


#endif /* THREADS */
	return -1;	
}
#endif /* POWER_MP */
