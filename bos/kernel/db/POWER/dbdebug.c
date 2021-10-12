static char sccsid[] = "@(#)50	1.83  src/bos/kernel/db/POWER/dbdebug.c, sysdb, bos41B, 412_41B_sync 12/6/94 14:34:56";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:	debugger, save_data, check_entry, step_or_break, first_screen,
 *		process_cmd, restore_info, rest_disp, tell_reason,
 *		in_real_mem, send_mpc_stop, dbcpu_init, db_resume_others,
 *              hold_cpus_while_stepping
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/

#include <sys/types.h>
#include <sys/mstsave.h>
#include <sys/dbkersym.h>
#include <sys/dbg_codes.h>
#include <sys/systm.h>
#include <sys/lldebug.h>
#include <sys/iplcb.h>
#include <sys/vmker.h>
#include <sys/systemcfg.h>
#include "debvars.h"
#undef DEC
#include "dbdebug.h"
#include "dbbreak.h"
#include "dbfunc.h"
#include "dbfn_init.h"
#include "pr_proc.h"
#include "parse.h"
#include "vdberr.h"
#include "debaddr.h"
#ifdef _POWER_PC
#include <sys/sys_resource.h>
#endif /* #ifdef _POWER_PC */
#include <sys/ppda.h>
#ifdef _POWER_MP
#include <sys/mpc.h>
#endif /* #ifdef _POWER_MP */
#include <sys/machine.h>

/*
 * EXTERNAL PROCEDURES CALLED:
 */

extern int mfdsisr();
extern int mfdar();
extern struct ppda ppda[];
#if defined (_POWER_RS1) || defined(_POWER_RSC)
extern	void	mfeim();
extern	void	mfeis();
extern	void	mteim();
extern	void	mteis();
#endif /* #if defined (_POWER_RS1) || ... */
extern int mfsdr0();
extern int mfsdr1();
extern int mfrtcu();
extern int mfrtcl();
extern int mfdec();

extern	void	mttid();
extern	void	mtdsisr();
extern	void	mtdar();
extern	void	mtsdr0();
extern	void	mtsdr1();
extern	void	mtrtcl();
extern	void	mtdec();
extern	void	mtrtcu();
extern	char	initio();
extern	void	copy_keyboard_map();
extern	void	enable_kbd();
extern	void	wait_for_break();
extern	void	clrdsp();
extern	char	*getterm();		/* get from the terminal */
extern	void	putdsp();
extern 	void	getscn();
extern	int	driver();
extern	int	get_cmd();
extern	setup_branch_table();
extern	steal_pgmck();
extern	restore_pgmck();
extern 	void	tell_reason();
extern  void	d_ttyclose();
extern	int	dump_rv;       /* declared in com/sys/db/vdbprf.c for panic() */
extern	ulong	dbterm;
extern	char	debabend;
extern	char	restore_cntl;		/* restore_cntl = 1, allow restore */
extern	uchar	screen_on;		/*flag:display/do not display scrn*/
extern	ulong	ipl_cb;			/* contains address of ipl control blk*/
#ifdef _POWER_MP
extern int db_get_processornum();
void db_resume_others();
#endif /* #ifdef _POWER_MP */
ulong getaprsegid(ulong,ulong);		/* Gets segid from addr (dbbreak.c) */
int branch_taken(ulong);		/* Says if branch will be taken -
					   from dbdisasm.c. */

#ifdef _POWER_MP
#include <sys/m_param.h>
unsigned char printbuf[MAXCPU * PRINTBUFSIZE];
#else /* #ifdef _POWER_MP */
unsigned char printbuf[PRINTBUFSIZE];	/* circular buf used by kernel printf */
					/* Declared here so it will not be in */
					/*    pinned memory                   */
#endif /* #ifdef _POWER_MP */

struct db_vmsidata db_vp;
extern struct watch_data watch_data;
extern struct brat_data brat_data;

#ifdef _POWER_PC
extern int  db_mftbu();   /* time base register support (move to/from) */
extern int  db_mftbl();
extern void db_mttbu();
extern void db_mttbl();
#endif /* #ifdef _POWER_PC */

/* Macro SREG - Given an address, it returns the segment register number within
 *   the address (the high order nibble).
 */
#define SREG(x)	((x)>>SEGSHIFT)

/* Defines for restore_info */
#define SET_BRATWATCH 1
#define NO_SET_BRATWATCH 0

#ifdef _POWER_PC
/* Determine a bit value in the ioalloc part of the mst */
#define BIT_VALUE(value, bit)	((value) & ((uint)0x80000000 >> (bit)))

ulong_t	ioalloc;	/* ioallocation mask copy from mst */
#endif /* #ifdef _POWER_PC */

/*
 * NAME: debug
 *
 * FUNCTION: Low Level Kernel Debugger
 *
 * (EXECUTION ENVIRONMENT:)
 *
 *	Environment-Specific aspects, such as -
 *	Preemptable        : no
 *	VMM Critical Region: n/a
 *	Runs on Fixed Stack: n/a
 *	May Page Fault     : NO
 *      May Backtrack      : n/a
 *
 * (DATA STRUCTURES:) a plethora of global data structures ...
 *
 */

struct vars debvars[MAXVARS];		/* found in vdbbreak.h */
double fr[NUMFPRS];			/* holds floating point reg values */
ulong sr_on_entry[NUMSEGS]; 		/* holds the real seg regs at the
					   time of entry to the debugger */
#ifdef _POWER_MP

mpc_msg_t mpc_stop;                      /* mpc message to stop other cpus */

volatile struct db_lock debugger_lock = {0,0,-1}; /* to protect db_main */

int  dbg_cpu;                   /* fo kdbx, the cpu being in "debugging" state  */

status_t status[MAXCPU];        /* Set by each cpu when changing its own state
					    used to display the status, and to avoid
					    spurious MPC_stop sendings
					  */
action_t action[MAXCPU];           /* Only modified in db_main, under protection
					    of debugger_lock. Used by the cpu in
					    debugging state to control others (stop, resume)
					  */

int selected_cpu = NO_SELECTED_CPU;      /* set by the cpu command */
int cp_selected_cpu = NO_SELECTED_CPU;      /* a copy for check_entry */

int step_processing_level = 0;           /* a step is being executed,
                                            can be recursive */

int switch_ctxt, switch_ctxt_sr1, switch_ctxt_sr2, switch_ctxt_sr13; /* switch command associated data */

#endif /* #ifdef _POWER_MP */

ulong rosetta_base;
#define init1 0xffffffff
#define init0 0x00000000
#define CACHE_SIZE_IN_BYTES	(64*1024)	/* 64K bytes */

int	debug_init=TRUE;	/* initialize the debugger */
int	restore_screen=TRUE;	/* 1 = restore display 	*/
ulong	debmid=not_a_module;	/* debuggers module id 	*/
char 	*in_string, old_string[40];

int 	loop_count=0;		/* loop counter 	*/
int	step_count=1;		/* step counter		*/
int	testbit,brk_type,max_ipts;
int 	max_real=(1024*1024);
int	step_id;
int  	at_breakpoint,cmd,b8;
char 	step_s1;
ushort	er;			/* ushort entry reason */
struct  parse_out parse_out;	/* parser structure */
struct 	debaddr debaddr;	/* debugger address structure */
struct 	mcs_pcs *mcs_pcs1;
char	*dummystr="dummy";

char	flssrs[16]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

int	save_reason;		/* Var to hold reason for "reason" cmd */
int	save_ext_arg;		/* Same for ext_arg */


struct mstsave *mst;		/* global mst area ptr */
struct ppda *ppda_ptr;		/* pointer to per processor data struc*/

/*
* NAME: db_main
*
* FUNCTION: this is the mainline code for the debugger and it calls
* 	the other functions. The global variables used in the debugger
*	are initialized here and in save_data. Check entry reason code
*	and this will determine why the debugger was called and what should
*	be done.
*
* RETURN VALUE DESCRIPTION: 0 if no errors and debugger has operated normally
*			    1 if trap was not for debugger
*			    negative number if debugger ended abnormally
*/

db_main(mst1, rsn_code, ext_arg)
struct mstsave *mst1;
int rsn_code;
int ext_arg;
{
	char *t, *addr, *goal, i;
	int cache_line_size;
	int cur_cpu = 0;
#ifdef _POWER_MP
	if (__power_mp()){
		cur_cpu = cpunb;
	/* check we had good reasons to enter the debug state, else, go back and wait*/
		if ((action[cur_cpu] == stop)
		    || ((selected_cpu != NO_SELECTED_CPU) && (selected_cpu != cur_cpu))){
			action[cur_cpu] = stop;
			return (OK);
		}
		dbg_cpu = cur_cpu; /* for kdbx usage */
		status[cur_cpu] = debugging; /* we entered the debugging state */
		action[cur_cpu] = NONE; /* action may be reset later */

		if (selected_cpu == NO_SELECTED_CPU){
		/*
		 * have to stop others running processors
		 */

			for(i=0; i< number_of_cpus; i++){
				if (status[i] == running){
					action[i] = stop;
					send_mpc_stop(i);
				}
			}
		}
		cp_selected_cpu = selected_cpu;
		selected_cpu = NO_SELECTED_CPU;
	}
#endif /* #ifdef _POWER_MP */

	/*
	 *	don't forget to increase the size of the register save
	 *	area in vdbdata.h8 for the 32 gprs and fprs!
	 */
#ifdef _POWER_MP
        if (__pegasus())
               db_stop_rtc();
#endif /* #ifdef _POWER_MP */

	/* NOTE: This is the first point at which we can safely do printf's. */
	/* For the kernel printf to work properly from the debugger, we	     */
	/* must have status[cur_cpu] == debugging *and* the rtc must be	     */
	/* stopped on MP systems.					     */

	/*
	 * If the pointer to the mstsave area is NULL, use csa->prev
	 * as the mstsave pointer.
	 */
	if (mst1)
		mst = mst1;		/* assign to global variable */
	else
		mst = (ppda[cur_cpu])._csa->prev;

	save_data();			/* save mstsave area */
					/* and init io */

	save_reason = rsn_code;		/* Save reason code for "reason" cmd */
	save_ext_arg = ext_arg;		/* Ditto for ext_arg */

	if(!check_entry(rsn_code)) {	/* check reason for entering */
		restore_info(TRUE,SET_BRATWATCH);/*restore breakpoints,etc*/

#ifdef _POWER_MP                /* have to resume stopped cpus */
		if (__power_mp()){
			db_resume_others();
			return OK; /* the entry has
			been check by db_is_to_call from p_slih,
			if here, there was a reason to enter the debugger, that
                        may have disappeared because MP */
		}

#endif /* #ifdef _POWER_MP */
		return NOTMYTRAP;	/* if not our trap return to kernel */
	}

	if (step_or_break()) {
		/* breakpt probably for another debugger */
		debabend = OUT;
#ifdef _POWER_MP                /* have to resume stopped cpus */
		if (__power_mp())
			db_resume_others();
#endif /* #ifdef _POWER_MP */
		return OK;		/*leave debugger (looping or stepping)*/
		}

	/* leave at this point if initialization wasn't successful */
	if (debug_init) {
		restore_info(TRUE,NO_SET_BRATWATCH); /*restore regs,etc*/
		debabend = OUT;
#ifdef _POWER_MP                /* have to resume stopped cpus */
		if (__power_mp())
			db_resume_others();
#endif /* #ifdef _POWER_MP */
		return(INITRET);		/* not yet initialized */
	}

	/*
	 * Setup the branch table, i.e., export display/kbd routines
	 * used by panic.
	 * Also, steel the program & machine check vectors;
	 * they're restore in restore_info.
	 */
	 /*setup_branch_table(); */
	 /*steal_pgmck(); */

	/* only do the following if we didn't abend the debugger */
	if (debabend==OUT) {
		debabend = IN;	/* track entry/exit to debugger */
		if(first_screen()<0) {
			/*restore breakpoints,etc*/
			restore_info(TRUE,NO_SET_BRATWATCH);
			debabend = OUT;	/* track entry/exit to debugger */
#ifdef _POWER_MP                /* have to resume stopped cpus */
		if (__power_mp())
			db_resume_others();
#endif /* #ifdef _POWER_MP */
			return(NOTTY);		/* we couldn't open a tty */
		}
	}
	/* error occured in debugger  so don't display screen - that may */
	/* be cause of error */
	else
		printf(" Error: left debugger via interrupt. \n");

	/* display reason for entering debugger*/
	tell_reason(rsn_code,ext_arg);


	i = process_cmd();			/* handle commands */

	/* finished, exit the debugger */
	if ((rsn_code == DBG_WATCH) || (rsn_code == DBG_BTARGET))
		/* Don't reset the IABR or DABR if we are already here      */
		/* because of one, or else we'll go into an infinite loop of*/
		/* brat/watch interrupts.  A step point should have been set*/
		/* previously that will allow us to re-establish the IABR & */
		/* DABR settings later. */
		restore_info(FALSE,NO_SET_BRATWATCH);
	else
		restore_info(FALSE,SET_BRATWATCH);
	debabend = OUT;

	/*
	 * since panic() calls brkpoint() and does not call
	 * the debugger directly, this is how we tell it we
	 * want to take a dump
	 */
	if(i==DUMPRET)
		dump_rv = i;
	else
		dump_rv = 0;

	return(OK);			/* return to kernel */
}

#ifdef _POWER_MP
/*
* NAME: send_mpc_stop
*
* FUNCTION:
*	sends a mpc signal to cpu i
*
*
* RETURN VALUE DESCRIPTION: Nothing
*
*/
send_mpc_stop(cpu_id)
int cpu_id;
{
	mpc_send(cpu_id, mpc_stop);
}


/*
* NAME: mpc_stop_handler
* Called from mpc interrupt
*
* FUNCTION:
*
*	call the debugger with an mpc_stop reason
*
*
*
* RETURN VALUE DESCRIPTION: none
*
*/

int mpc_stop_handler()
{
	debugger(0,DBG_MPC_STOP,0);
}

/*
* NAME: dbcpu_init
* Called from debugger_init
*
* FUNCTION:
*
*	Initialize the cpu initial structures action and status
*	and registers the mpc handler
*
*
* RETURN VALUE DESCRIPTION: None
*
*/

void dbcpu_init()
{
	int i;
	mpc_stop = mpc_register(INTMAX, mpc_stop_handler);
	for (i=0; i< number_of_cpus; i++){
		status[i] = running;
		action[i] = NONE;
	}
}


/*
* NAME: hold_cpus_while_stepping
*
* FUNCTION:
*
*	take care not to let debug_waiting cpus
*	take control of the debugger
*
* RETURN VALUE DESCRIPTION: None
*
*/

void hold_cpus_while_stepping() /* called only if __power_mp() */
{
	int i;
	for (i=0; i< number_of_cpus;i++){
		if (status[i] == debug_waiting)
			action[i] = stop;
	}
}


/*
* NAME: db_resume_others
*
* FUNCTION:
*
*	resume stopped cpu if no break instruction beeing executed
*
*
* RETURN VALUE DESCRIPTION: none
*
*/

void db_resume_others() /* called only if __power_mp() */
{
	int i;
	/* check if to resume stopped cpus */
	if (step_processing_level == 0)
	        for (i=0; i< number_of_cpus;i++){
		if (action[i] == stop)
			action[i] = resume;
	}
}
#endif /* #ifdef _POWER_MP */

/*
* NAME: save_data
*
* FUNCTION: this routine will save certain data areas into local debugger
*	save areas. The mstsave area is almost completely saved off as well
*	as the segment registers. The display is initialized if necessary
*	and the first debugger command (?) is set. Debvars is a pointer
*	to a location where debugger variable information is kept. Initio
*	will set some global variables used for the terminal routines.
*
* PARAMETERS: all data areas used are global
*
* RETURN VALUE: 0 is always returned to the caller
*
*/

save_data()
{
	register int	i;
        struct ipl_cb *iplcb_ptr;
        struct ipl_info *iinfo_ptr;
        struct ipl_directory *idir_ptr;

        /*
	 * get address of ram bit map, get size of bit map,
	 * 	get number of bytes each bit in ram bit map represents
         */
        iplcb_ptr = (struct ipl_cb *) ipl_cb;
        idir_ptr = (struct ipl_directory *)(&iplcb_ptr->s0);
        db_vp.rmapptr  = (uint)ipl_cb + realbyt(&idir_ptr->bit_map_offset,4);
	db_vp.rmapsize = realbyt(&idir_ptr->bit_map_size,4)/4;
	iinfo_ptr = (struct ipl_info *)
			((uint)ipl_cb + realbyt(&idir_ptr->ipl_info_offset,4));
	db_vp.rmapblk = realbyt(&iinfo_ptr->bit_map_bytes_per_bit,4);

	/* force store of & make available floating point registers */
	disown_fp(-1);
	for (i=0; i<NUMGPRS; i++) {
		fr[i] = mst->fpr[i];	/* same # of fprs as gprs */
		debvars[IDGPRS+i].hv = mst->gpr[i];

		if (i<NUMSEGS) {
			debvars[IDSEGS+i].hv = mst->as.srval[i];
			sr_on_entry[i] = mfsr(i);
		}
	}

#ifdef _POWER_PC
	/* pick up the BATs from the mst.  The structure of the BATs in
	   the mst is:

	   struct {
		ulong_t batu;
		ulong_t batl;
		} dbats[NUM_KERNEL_BATS];

	   while the variables are defined as

	"bat0u", "bat1u", "bat2u", "bat0l", "bat1l", "bat2l",
	*/

	if (__power_pc() && !__power_601()) {
		ioalloc = mst->ioalloc;
		for (i=0; i < NUM_KERNEL_BATS; i++) {
			if (BIT_VALUE(ioalloc, i)) {
	    			debvars[IDBATU+i].hv = mst->dbats[i].batu;
	    			debvars[IDBATL+i].hv = mst->dbats[i].batl;
			}
			/* If BAT was not allocated by iomem_att then load
			   upper BAT value with 0 to indicate it is not a
			   valid BAT. */
			else {
	    			debvars[IDBATU+i].hv = 0;
	    			debvars[IDBATL+i].hv = mst->dbats[i].batl;
			}	
	  	}
	}
#endif /* #ifdef _POWER_PC */

	debvars[IDMSR].hv = mst->msr;
	debvars[IDCR].hv = mst->cr;
	debvars[IDLR].hv = mst->lr;
	debvars[IDCTR].hv = mst->ctr;
	debvars[IDXER].hv = mst->xer;
	debvars[IDFPSCR].hv = mst->fpscr;
	debvars[IDTID].hv = mst->tid;

	/* Note: it is invalid to read SRR0 and SRR1 with translate on. */
	/* We should be showing the mst's MSR and IAR values instead of */
	/* reading them directly from the registers, so that is what we */
	/* are now doing. */
	debvars[IDSRR0].hv = mst->iar;
	debvars[IDSRR1].hv = mst->msr;

	/*
	 *	get the register data using assembler instructions
	 */

	ppda_ptr=ppda;
	debvars[IDDSISR].hv = ppda_ptr->dsisr;
	debvars[IDDAR].hv = ppda_ptr->dar;

#if defined (_POWER_RS1) || defined(_POWER_RSC)
        if ( __power_rs1() || __power_rsc() ) {
                mfeim(&debvars[IDEIM0].hv,&debvars[IDEIM1].hv);
                mfeis(&debvars[IDEIS0].hv,&debvars[IDEIS1].hv);
        }
#endif /* #if defined (_POWER_RS1) || ... */

#if defined (_POWER_RS2)
        if ( __power_rs2() ) {
                /* read and clear peis0,1 */
                read_clear_peis();
		debvars[IDILCR].hv = db_mfilcr();
        }
#endif /* #if defined (_POWER_RS2) */

#if defined(_POWER_PC) && defined(_RS6K)
#ifdef _SNOOPY
	if (__snoopy()){
 		debvars[IDXIRR].hv = 0xffffffff;
 		debvars[IDDSIER].hv = 0xffffffff;
	}
	else
#endif /* #ifdef _SNOOPY */
        if ( __power_pc() && __rs6k() ) {
           debvars[IDXIRR].hv = sys_resource_ptr->
              sys_interrupt_space.sys_intr_regs[0].xirr_poll;
           debvars[IDDSIER].hv = sys_resource_ptr->
              sys_interrupt_space.sys_intr_regs[0].dsier;
        }
#endif /* #ifdef _POWER_PC && _RS6K */

	/* SDR0 only exists on RS1's, 2's, & C's.  Load it with deadbeef */
	/* on other platforms. */
#ifdef _POWER_RS
	if (__power_rs()) 
		debvars[IDSDR0].hv = mfsdr0();
	else
#endif /* #ifdef _POWER_RS */
		debvars[IDSDR0].hv = 0xdeadbeef;
	debvars[IDSDR1].hv = mfsdr1();

#ifdef _POWER_PC
	/*  check if we have a 603/604 (time base) or 601 (rtc) */
	if (__power_pc() && !__power_601())
	  {
	    debvars[IDTBU].hv = db_mftbu();
	    debvars[IDTBL].hv = db_mftbl();
	  }
	else
#endif /* #ifdef _POWER_PC */
	/* on 601 or regular POWER architecture use real time counter */
	{
	  debvars[IDRTCU].hv = mfrtcu();
	  debvars[IDRTCL].hv = mfrtcl();
	}
	debvars[IDDEC].hv = mfdec();
	debvars[IDIAR].hv = mst->iar;
	debvars[IDMQ].hv = mst->mq;

	/* parms copied to internal structures  */
	if (debug_init) {	/* if initialization needed */
		strcpy(old_string, "help ");	/* ? is default cmd */
		debug_init = FALSE;
	}

	restore_cntl = FALSE;		/* don't restore yet */
	at_breakpoint = FALSE;		/* assume not a bkpt */
       /* remove watchpoints then bratpoints then breakpoints */
#if defined (_POWER_RS2) || defined (_POWER_PC)
        if ( __power_rs2() || __power_pc() )
        	if (watch_data.active) clear_watch_regs();
        if ( __power_pc() )
        	if (brat_data.active) clear_brat_regs();
#endif /* #if defined (_POWER_RS2) || defined ... */
	remove_breakpoint_traps(); 	/* take out any bkpt traps */
	return(0);
}


#if defined (_POWER_RS2)
/*
* NAME: read_clear_peis
*
* FUNCTION: Read peis0 and peis1 into lldbg storage, DEBVARS[].
*           Then reset each bit in these registers which is set.
*
*
* PARAMETERS:
*
* RETURN VALUE:
*
*/

read_clear_peis()
{

        if( __power_rs2() )
        {
            int plvl;
            ulong peis0, peis1;         /* save area for external int regs    */

            db_mfpeis(&debvars[IDPEIS0].hv,&debvars[IDPEIS1].hv);
            peis0=debvars[IDPEIS0].hv;
            peis1=debvars[IDPEIS1].hv;
            while( peis0 ) {
                plvl = bitindex( &peis0 );
                peis0 &= ~((ulong)(0x80000000) >> plvl);
                db_rs2peis_reset( plvl );
            }
            while( peis1 ) {
                plvl = bitindex( &peis1 );
                peis1 &= ~((ulong)(0x80000000) >> plvl);
                plvl += 32;
                db_rs2peis_reset( plvl );
            }
        }
}
#endif /* #if defined (_POWER_RS2) */


/*
* NAME: check_entry
*
* FUNCTION: Find out why we are in the debugger. Set the entry reason to
*	the correct trap code.
*	Only called if we hit a trap.
*       If not a trap do not bother checking breakpoint table, just
*       return true.  Otherwise check to see if trap is in breakpoint
*       table.
*
* PARAMETERS: rsn_code = reason for entering debugger. also, many globals
*
* RETURN VALUE: FALSE if not my trap (and want to leave debugger), else TRUE
*
*/

check_entry(rsn_code)
{
	ulong virt;	/* virtual addressing mode indicator */
	int cur_cpu = 0;

#ifdef _POWER_MP
	if(rsn_code==DBG_MPC_STOP){		/* in debugger due to "cpu" */
		if (__power_mp()){
			er = E_selected_cpu;
			return TRUE;		/* so stay in debugger */
		} else{
			er = 0;	 /* If this reason and mono, ignore */
			return FALSE;            /* return from debugger to kernel */
		}
	}
#endif /* #ifdef _POWER_MP */


        if(rsn_code == DBG_WATCH) {       /* in debugger due to watchpoint */
#ifdef _POWER_MP
		if (__power_mp())
			cur_cpu = cpunb;
		else
#endif /* POWER_MP */
			cur_cpu=0;
		if (ppda[cur_cpu].dsisr & DSISR_PROT) {  /*check for pageprot*/
                        er = 0;
                        return FALSE;   /* so exit debugger */
                }
                er = W_break;           /* watchpoint */
		return TRUE;		/* so stay in debugger */
	}
	else
        if(rsn_code == DBG_BTARGET) {   /* in debugger due to bratpoint */
                er = B_break;           /* bratpoint */
		return TRUE;		/* so stay in debugger */
	}
	else
	if(rsn_code!=DBG_TRAP){		/* in debugger due to system error */
		er = 0;			/* init this to nothing */
		return TRUE;		/* so stay in debugger */
	}

	virt = INSTT_BIT;	/* instruction addressing mode */
	step_s1 = FALSE;


#ifdef _POWER_MP
	brk_type = is_break_for_me(debvars[IDIAR].hv,virt);
#else /* #ifdef _POWER_MP */
	brk_type = is_break(debvars[IDIAR].hv,virt, FROMDEBVARS);
#endif /* #ifdef _POWER_MP */
	if (is_static_break(debvars[IDIAR].hv,virt)) {
		er = E_static_break;
	}
#ifdef _POWER_MP
	else if (is_step_for_me(debvars[IDIAR].hv,virt,&step_id)) {
#else /* #ifdef _POWER_MP */
	else if (is_step(debvars[IDIAR].hv,virt,&step_id, FROMDEBVARS)) {
#endif /* #ifdef _POWER_MP */
                if (is_watch_step(&step_id))
                        er = W_step;    /* must be a watch step trap */
                else
                	if (is_brat_step(&step_id))
                       		 er = B_step;    /* must be a brat step trap */
                	else
                       		 er = E_step;    /* must be a step trap */
                }
	else if (brk_type > 0)
		er = E_break;		/* breakpoint */
#ifdef _POWER_MP
	else if (is_step(debvars[IDIAR].hv,virt, &step_id, FROMDEBVARS)
		 ||is_break(debvars[IDIAR].hv,virt, FROMDEBVARS))
		er = E_break_not_for_me;	/* step or break not for me */
#endif /* #ifdef _POWER_MP */
	else {
#ifdef _POWER_MP
	if(cp_selected_cpu == cpunb){		/* in debugger due to "cpu" */
		if (__power_mp()){
			er = E_selected_cpu;
			return TRUE;		/* so stay in debugger */
		}
	}
#endif /* #ifdef _POWER_MP */
		er = 0;			/* unknown */
		return FALSE;		/* return from debugger to kernel */
	}
	return TRUE;
}


/*
* NAME: step_or_break
*
* FUNCTION: this routine processes a step, breakpoint or static breakpoint
*	trap.
*
* PARAMETERS: all data areas used are global
*
* RETURN VALUE: 0 is returned to the caller if you want to stay in the
*	debugger else a 1 is returned which forces the debugger to return
*	control and the next instruction is executed.
* NOTE: If you will be returning a 1 (TRUE), then you *must* call restore_info
*       to restore the appropriate registers.  The caller (db_main) doesn't
*	do this for you.
*/

step_or_break()
{
ulong save_iar;

	switch(er) {
#ifdef _POWER_MP

	case E_break_not_for_me:
		/* dont remove the step from step table, skip it */
		if (new_step(TRUE,step_s1,FALSE)) {
			restore_info(TRUE,NO_SET_BRATWATCH);
			if (__power_mp()) /* have to prevent from resuming stopped cpus */
				if (step_processing_level++ ==0)
					hold_cpus_while_stepping();
			return(1);
		}
		else
		{
			er = E_nostep;	/* no place to step */
		}
		break;
#endif /* #ifdef _POWER_MP */

	    case E_step:
#ifdef _POWER_MP
		if (__power_mp())
			if (step_processing_level > 0)
				step_processing_level--;
#endif /* #ifdef _POWER_MP */
		remove_this_step(step_id);  /* make step inactive*/
		if (is_continue(step_id)) { /* single step b/f go */
			if (brk_type == 0) {/* resume execution */
#ifdef _POWER_MP                /* have to resume stopped cpus */
				if (__power_mp())
					db_resume_others();
#endif /* #ifdef _POWER_MP */
				restore_info(TRUE,SET_BRATWATCH);
				return(1);
			}
		}
		else {				/* single or multi-step */
			step_count--;
			if (step_count > 0) {	/* step more if cnt > 0 */
				if (new_step(FALSE,step_s1,FALSE)) {
#ifdef _POWER_MP                       /* prevents from resuming  stopped cpus */
					if (__power_mp())
						if (step_processing_level++ ==0)
							hold_cpus_while_stepping();
#endif /* #ifdef _POWER_MP */
					restore_info(TRUE,NO_SET_BRATWATCH);
					return(1);
				}
				else
					er = E_nostep;	/* no place to step */
			}
		}
		break;

            case W_step:
                remove_this_step(step_id);  /* make step inactive*/
                restore_info(TRUE,SET_BRATWATCH);
		return(1);
                break;

            case B_step:
		/* modify to remove two step points */
                remove_this_step(step_id);  /* make step inactive*/
                restore_info(TRUE,SET_BRATWATCH);
		return(1);
                break;

            case B_break:
		/* No matter whether we want to show this branch to the user */
		/* or not, we need to set a step point so that we can turn   */
		/* off the brat/watch points, let this instruction run, then */
		/* turn them back on again. */
		if (!new_step(TRUE,FALSE,TRUE))  /* If we can't set step pt, */
			return FALSE;		 /* we must drop into debug. */
		/* check if branch will be taken */
		if (branch_taken(debvars[IDIAR].hv)) {
			/* If the segid for the current address matches the  */
			/* brat segid, then show this IA breakpoint.  Note   */
			/* that the instruction translation bit controls     */
			/* whether we treat the current address as real or   */
			/* virtual. */
			if (getaprsegid(brat_data.addr,INSTT_BIT) ==
				brat_data.segid) 
				/* This is a brat point we want to show.  */
				return(FALSE);		/* enter debugger */
		}	/* if (branch_taken()) */
		/* Otherwise, we don't want to show this branch.  Go ahead */
		/* and run the next instruction - the step point set above */
		/* will give control back to us afterwards. */
		restore_info(FALSE,NO_SET_BRATWATCH);
		return(TRUE);
		break;

	    case W_break:
		/* No matter whether we want to show this opcode to the user */
		/* or not, we need to set a step point so that we can turn   */
		/* off the brat/watch points, let this instruction run, then */
		/* turn them back on again. */
		if (!new_step(TRUE,FALSE,TRUE))  /* If we can't set step pt, */
			return FALSE;		 /* we must drop into debug. */
		/* If the segid for the current address doesn't match the */
		/* segid, then ignore this DA breakpoint.  Note that the  */
		/* data translation bit controls whether we treat the     */
		/* current address as real or virtual. */
		if (getaprsegid(watch_data.addr,DATAT_BIT) != watch_data.segid)
		{
			restore_info(FALSE,NO_SET_BRATWATCH);
			return(TRUE);
		}
		/* Otherwise, this is a watch point we want to show.  */
		return(FALSE);		/* enter debugger */
		break;

	    case E_break:
		if (brk_type == TRACEPT) {	/* a trace breakpoint */
			if (new_step(TRUE,step_s1,FALSE)) { /* step 1 if possible */
#ifdef _POWER_MP                /* prevents from resuming stopped cpus */
				if (__power_mp())
					if (step_processing_level++ ==0)
						hold_cpus_while_stepping();
#endif /* #ifdef _POWER_MP */
				restore_info(TRUE,NO_SET_BRATWATCH);
				return(1);
			}
			er = E_nostep;
		}
		else {
			if (brk_type == LOOP) {	/* check for loop */
				loop_count--;
				if (loop_count>0) { /* looping */
					if (new_step(TRUE,step_s1,FALSE)) {
#ifdef _POWER_MP                                /* eventually allows to resume stopped cpus */
				                if (__power_mp())
							if (step_processing_level++ ==0)
								hold_cpus_while_stepping();
#endif /* #ifdef ... */
						restore_info(TRUE,NO_SET_BRATWATCH);
						return(1);
					}
				}
			}
			at_breakpoint = TRUE;
		}
		break;

	    case E_static_break:
		at_breakpoint = TRUE;
		if (debug_init) /* get past trap if initialization failed */
			debvars[IDIAR].hv += sizeof(INSTSIZ); /* instr len */
	}			/* end of switch */
	return(0);
}


/*
* NAME: first_screen
*
* FUNCTION: this routine determines the termianl type and displays the first
*	debugger screen
*
* PARAMETERS: all data areas used are global
*
* RETURN VALUE: 0 for ok, -1 if can't open port.
*
*/

first_screen()
{
	int ttynum, rc;
	ulong orig_dbterm = dbterm;

	if(!dbterm) {		/* first time in debugger */
#ifdef _SNOOPY
		if (__snoopy())
			dbterm= USE_TTY;
		else
#endif /* #ifdef _SNOOPY */
		for(ttynum=0;(rc=d_ttyopen(ttynum)) != -1;ttynum++) {
			if(rc>0) {
				dbterm = USE_TTY | ttynum ;  /*found a port*/
				break;
			}
		}
		if(!dbterm)	/* we didn't find any ttys to open */
			return -1;
	}
	else {
		if (dbterm & USE_TTY)	{	/* async terminal is in use */
#ifdef _SNOOPY
			if (__snoopy()){
				/* nothing */
			}
			else
#endif /* #ifdef _SNOOPY */
			if(d_ttyopen(dbterm & TTY_PORT)<=0){	/* open port */
				return -1;	/* big trouble,we obviously */
			}			/* opened this before */
		}
	}
	restore_cntl = !(dbterm & USE_TTY);	/* allow restore if not 3101 */

	/* if screen isn't active, clear the screen anyway */
	if (screen_on) {
		parse_out.num_tok = 0;	/* initialize parser struct */
		debug_screen(&parse_out,DEFLTSCR);	/* debugger display */
	}
	else
		clrdsp();
	return(0);
}


/*
* NAME: process_cmd
*
* FUNCTION: this routine is the main line command processing part of the
*	debugger. The command is read in and then sent off to be processed
*	by driver. The only way to exit this routine is via the quit, step
*	or go commands.
*
* PARAMETERS: all data areas used are global
*
* RETURN VALUE: value returned from driver(). The only significant value
*		is one indicating kernel should take a dump. To exit the while
*	loop a called routine must return a non-zero value.
*
*/

process_cmd()
{
	int 	retval=0;

	do {
		in_string = getterm();
		cmd = get_cmd(in_string, &parse_out);
		if (cmd != INVALID_ID) {
			if (cmd == DITTO_ID)
				cmd = get_cmd(old_string, &parse_out);
			else
				strcpy(old_string, in_string);

			retval = driver(cmd, &parse_out);
		}
	} while (!retval);
	return(retval);
}

/*
* NAME: restore_info
*
* FUNCTION: this routine restores the information from the debuggers
*	local mst save area into the mst save area and into the segment
*	registers.
*
* PARAMETERS: no_user - flag passed to rest_disp() - currently unused
*	      bratwatch - flag that determines whether the IABR/DABR should
*			be set from the brat/watch data areas.
*
* RETURN VALUE: 0 is always returned by this routine.
*
*/
restore_info(no_user,bratwatch)
char no_user;
int bratwatch;
{
	register int i;

	/* restore breakpoints then bratpoints the watchpoints as necessary */
	set_breakpoint_traps(debvars[IDIAR].hv); 	/* set breaks */

        /* if requested, check the watch and brat data areas to see if the */
	/* watch and/or brat registers need to be set. */
	if (bratwatch == SET_BRATWATCH) {
        	if (watch_data.active)  set_watch_regs();
        	if (brat_data.active)  set_brat_regs();
	}
	for (i=0; i<NUMGPRS; i++) { /* restore info back to mst save area */
		mst->gpr[i] =  debvars[IDGPRS+i].hv;
		mst->fpr[i] =  fr[i];
		if (i<NUMSEGS) {
			mst->as.srval[i] = debvars[IDSEGS+i].hv;
		}
	}

#ifdef _POWER_PC
	/* put the BATs back in the mst. */
	if (__power_pc() && !__power_601()) {
		/* Any BAT modifications will have changed the allocation
		   mask as well so just restore the current BAT values
		   to the mst. */
		for (i=0; i < NUM_KERNEL_BATS; i++) {
	      		mst->dbats[i].batu = debvars[IDBATU+i].hv;
	      		mst->dbats[i].batl = debvars[IDBATL+i].hv;
	  	}
		/* put allocation mask back in the mst */
		mst->ioalloc = ioalloc;
	}
#endif /* #ifdef _POWER_PC */

	mst->iar = debvars[IDIAR].hv;
	mst->msr = debvars[IDMSR].hv;
	mst->cr = debvars[IDCR].hv;
	mst->lr = debvars[IDLR].hv;
	mst->ctr = debvars[IDCTR].hv;
	mst->xer = debvars[IDXER].hv;
	mst->fpscr = debvars[IDFPSCR].hv;
	mst->mq = debvars[IDMQ].hv;
	mst->tid = debvars[IDTID].hv;
	ppda_ptr=ppda;
	ppda_ptr->dsisr=debvars[IDDSISR].hv;
	ppda_ptr->dar = debvars[IDDAR].hv;

#if defined (_POWER_RS1) || defined(_POWER_RSC)
        if ( __power_rs1() || __power_rsc() ) {
                mteim(debvars[IDEIM0].hv, debvars[IDEIM1].hv);
                mteis(debvars[IDEIS0].hv, debvars[IDEIS1].hv);
        }
#endif /* #if defined (_POWER_RS1) || ... */

#if defined (_POWER_RS2)
        if ( __power_rs2() ) {
                update_peis();
        }
#endif /* #if defined (_POWER_RS2) */


#ifdef _POWER_RS
	/* SDR0 only exists on the RS boxes.  It isn't on any 6xx box. */
	if (__power_rs()) {
		mtsdr0(debvars[IDSDR0].hv);
	}
#endif /* #ifdef _POWER_RS */
	mtsdr1(debvars[IDSDR1].hv);

	if ( ! __power_mp() ) {
#ifdef _POWER_PC
	/*  check if we have a 603/604 (time base) or 601 (rtc) */
		if (__power_pc() && !__power_601())
	  	{
	    	db_mttbu(debvars[IDTBU].hv);
	    	db_mttbl(debvars[IDTBL].hv);
	  	}
		else
#endif /* #ifdef _POWER_PC */
	  /* on 601 or regular POWER architecture use real time counter */
	  	{
	    	mtrtcu(debvars[IDRTCU].hv);
	    	mtrtcl(debvars[IDRTCL].hv);
	  	}
	}
	mtdec(debvars[IDDEC].hv);

	rest_disp(no_user);

	/* restore state (i.e. seg regs) to the way it was on entry */
	for(i=0;i<NUMSEGS;i++)
		mtsr(i,sr_on_entry[i]);

	return(0);
}

#if defined (_POWER_RS2)
/*
* NAME: update_peis
*
* FUNCTION: Update peis0 and peis1 according to value specified by
*               user.  If values were not changed, use original values
*               saved on entry to the debugger.
*
*
* PARAMETERS:
*
* RETURN VALUE:
*
*/

update_peis()
{
        if( __power_rs2() ) {

            int i;
            ulong mask;

            if (debvars[IDPEIS0].hv)
                for (i=0; i<32; i++) {
                        mask = ((ulong)(0x80000000) >> i);
                        if (debvars[IDPEIS0].hv & mask)
                                db_rs2peis_set(i);
                } /* end for loop */

            if (debvars[IDPEIS1].hv)
                for (i=0; i<32; i++) {
                        mask = ((ulong)(0x80000000) >> i);
                        if (debvars[IDPEIS1].hv & mask)
                                db_rs2peis_set(i+32);
                } /* end for loop */

        }

}
#endif /* #if defined (_POWER_RS2) */



/*
* NAME: rest_disp
*
* FUNCTION: this routine restores the display info
*
* PARAMETERS: all data areas used are global
*
* RETURN VALUE: 0 is always returned by this routine.
*
*/

rest_disp(no_user)
char	no_user;
{

	if ((dbterm & USE_TTY) || no_user) {
		if (dbterm & USE_TTY) {
#ifdef _SNOOPY
			if (__snoopy()){
				/* nothing */
			}
			else
#endif /* #ifdef _SNOOPY */
			d_ttyclose(dbterm & TTY_PORT); /* close port */
		}
	}

	return(0);
}


/*
 * Name: tell_reason()
 *
 *           This is a debugger routine that displays a message giving the
 *           reason that the debugger was entered.
 *
 * Parameters on entry:
 *      int     reason_code;   Indicates message to be displayed;
 *                               see dbg_codes.h
 *      int     parm;          Parameter for message, if applicable.
 *
 * Returns:
 *      Nothing.
 *
 */

void tell_reason(reason_code,parm)
int    reason_code;			/* Message index */
int    parm;				/* Optional parameter for message */
{
    char   *p;
    int parm2;

    switch (reason_code) {
      case DBG_FPEN:
	p = DBG_FPEN_MSG;
	break;
      case DBG_INVAL:
	p = DBG_INVAL_MSG;
	break;
      case DBG_PRIV:
	p = DBG_PRIV_MSG;
	break;
      case DBG_TRAP:
	p = DBG_TRAP_MSG;
	break;
      case DBG_UNK_PR:
	p = DBG_UNK_PR_MSG;
	break;
      case DBG_MCHECK:
	p = DBG_MCHECK_MSG;
	break;
      case DBG_SYSRES:
	p = DBG_SYSRES_MSG;
	break;
      case DBG_ALIGN:
	p = DBG_ALIGN_MSG;
	break;
      case DBG_VM:
	p = DBG_VM_MSG;
	break;
      case DBG_KBD:
	p = DBG_KBD_MSG;
	break;
      case DBG_RECURSE:
	p = DBG_RECURSE_MSG;
	break;
      case DBG_PANIC:
	p = DBG_PANIC_MSG;
	break;
      case DBG_KBD_NORMAL:
	p = DBG_KBD_NORMAL_MSG;
	break;
      case DBG_KBD_SECURE:
	p = DBG_KBD_SECURE_MSG;
	break;
      case DBG_KBD_SERVICE:
	p = DBG_KBD_SERVICE_MSG;
	break;
      case DBG_KBD_SERVICE1:
	p = DBG_KBD_SERVICE1_MSG;
	break;
      case DBG_KBD_SERVICE2:
	p = DBG_KBD_SERVICE2_MSG;
	break;
      case DBG_KBD_SERVICE4:
	p = DBG_KBD_SERVICE4_MSG;
	break;
      case DBG_DSI_IOCC:
	p = DBG_DSI_IOCC_MSG;
	break;
      case DBG_DSI_SLA:
	p = DBG_DSI_SLA_MSG;
	break;
      case DBG_DSI_SCU:
	p = DBG_DSI_SCU_MSG;
	break;
      case DBG_DSI_PROC:
	p = DBG_DSI_PROC_MSG;
	break;
      case DBG_DSI_SGA:
	p = DBG_DSI_SGA_MSG;
	break;
      case DBG_ISI_PROC:
	p = DBG_ISI_PROC_MSG;
	break;
      case DBG_EXT_DMA:
	p = DBG_EXT_DMA_MSG;
	break;
      case DBG_EXT_SCR:
	p = DBG_EXT_SCR_MSG;
	break;
      case DBG_FPT_UN:
	p = DBG_FPT_UN_MSG;
	break;
      case DBG_HTRAP:
	p = DBG_HTRAP_MSG;
	break;
      case DBG_KBDEXT:
	p = DBG_KBDEXT_MSG;
	break;
      case DBG_BUS_TIMEOUT:
	p = DBG_BUS_TIMEOUT_MSG;
	break;
      case DBG_CHAN_CHECK:
	p = DBG_CHAN_CHECK_MSG;
	break;
      case DBG_BTARGET:
	p = DBG_BTARGET_MSG;
	break;
      case DBG_WATCH:
	p = DBG_WATCH_MSG;
	break;
#ifdef _POWER_MP
      case DBG_MPC_STOP:
	p = DBG_MPC_STOP_MSG;
	break;
#endif /* #ifdef _POWER_MP */
      default:
	p = "Debugger entered for unknown reason. %d %d";
	parm2 = parm;
	parm = reason_code;
	break;
    }

    if (panicstr != NULL)
       p = panicstr;

    printf(p,parm,parm2);
    printf("\n");
}

/*
 * NAME: in_real_mem
 *
 * FUNCTION:  Determine if real memory address is valid.
 *
 * RETURNS: 0 if address is not in real memory
 *	    1 if address is in real memory
 */

int
in_real_mem (real_address)
ulong real_address;
{

        int pno,bit,word,get_ptr,ptr,last_byte;

#ifdef _RS6K_UP_MCA
        if ( __rs6k_up_mca() ) {
		if ((real_address >= 0xff000000) &&
			(real_address <= 0xffffffff))
			return (1);
        }
#endif /* #ifdef _RS6K_UP_MCA */


	/*
	 * extract page number from real address and pass this to db_isbad()
	 */
	pno= (real_address >> 12) & 0x0000ffff;

        /*
         * get pointer to word in ram bit map
         */
        bit = (pno * PSIZE) / db_vp.rmapblk;
        word = bit/32;
        ptr = (db_vp.rmapptr & SOFFSET) + (4*word);
	/*
	 * get pointer to last address of ram bit map
	 */
        last_byte = (db_vp.rmapptr & SOFFSET)  + (db_vp.rmapsize*4);

	if (ptr>last_byte)	/* if out of range of ram bit map */
		return(0); 	/* 	return false		  */
	else {
		get_ptr = realbyt(ptr,4);	/* read a word from bitmap */

		/*
		* move bit to bit position 31 in word,
		* mask and return it.
		*/
        	bit = bit - word*32;
       		bit = (get_ptr >> (31 - bit)) & 0x1;
        	return(!bit);
	}
}

