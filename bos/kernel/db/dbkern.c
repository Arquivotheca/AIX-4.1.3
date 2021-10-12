static char sccsid[] = "@(#)57	1.32.1.7  src/bos/kernel/db/dbkern.c, sysdb, bos41B, 412_41B_sync 12/6/94 14:23:04";

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: dbstub, kercdtf, ker_dump_init, no_debugging_cpu,
 *            db_check_mpc, db_is_to_call, db_stop_rtc, db_start_rtc,
 *	      db_active
 *
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
#include <sys/param.h>
#include <sys/mstsave.h>
#include <toc.h>
#include <sys/dbkersym.h>
#include <sys/lldebug.h>
#include <sys/dbg_codes.h>
#include <sys/dump.h>
#include <sys/systemcfg.h>
#ifdef _POWER_MP
#include <sys/ppda.h>
#endif /* POWER_MP */
#include "dbdebug.h"

/* Define the dbfunc storage. */
#define DEF_STORAGE
#include <sys/dbexp.h>
#undef DEF_STORAGE

/*
 * these pointers are exported,
 * they are initialized to 0,
 * at initialization, the tty subsystem sets in these pointers
 * the address of the registration functions.
 */
int (* tty_db_register_ptr)() = NULL;
int (* tty_db_unregister_ptr)() = NULL;
int (* tty_db_open_ptr)() = NULL;
int (* tty_db_close_ptr)() = NULL;

/* debug copies of pointers based in streams-related kernel exts */
/* declared here to make sure they are pinned. */
#include <pse/str_stream.h>
struct modsw** dbp_fmodsw = NULL;
struct modsw** dbp_dmodsw = NULL;
struct sth_s** *dbp_sth_open_streams = NULL;
struct msgb** dbp_mh_freelater = NULL;
struct sqh_s* dbp_streams_runq = NULL;


int rtc_to_restart;
extern long pwr_obj_end;
extern long pwr_com_end;
extern long pin_dbg_end;
extern long pin_dbgcom_end;
extern ulong dbterm;
extern char debabend;        /* if interrupt occured in the debugger code */
int dbg_avail = 0;	    /* set by mkboot - indicates if debugger */
				/* is supposed to be available and/or invoked*/

int dbg_pinned = 0;             /* flag indicating whether or not the */
                                /* debugger code has been pinned */

char  restore_cntl;

#ifdef _POWER_MP
#define MAX_WAIT_COUNT 0x20000000

int dbkdbx_lvl;			/* to tell kdbx the level of features that */
				/* are implemented in lldb */


extern volatile struct db_lock debugger_lock;  
 
extern status_t status[];        /* Set by each cpu when changing its own state
				  * used to display the status, and to avoid
				  * spurious MPC_stop sendings
				  */

extern action_t action[];        /* Only modified in db_main, under protection
				  * of debugger_lock. Used by the cpu in
				  * debugging state to control others (stop, resume)
				  */

extern int selected_cpu;         /* set by the cpu command */

extern int step_processing_level;

#endif /* POWER_MP */

int db_active();

/*                                                                   
 * EXTERNAL PROCEDURES CALLED: 
 */

/*
 * NAME:  debugger
 *
 * FUNCTION:                                                         
 *	This routine is the portion of the debugger entry point
 *	that is always pinned.  It determines if the debugger is
 *	invokable and if so if it is pinned or not.  If it is 
 *	invokable, the debugger is pinned if necessary then called.
 *                                                               
 * INPUT:                                                       
 *                                                                
 * OUTPUT:                                                       
 */

debugger(mst1, rsn_code, ext_arg)
struct mstsave *mst1;
int rsn_code;
int ext_arg;
{
#ifdef _POWER_MP
	unsigned int cur_cpu = 0;
	unsigned int wait_count = 0;


	int i,j;
	char buf[40];
#endif /* POWER_MP */
#if defined(_KDB)
	if (__kdb()) { /* choose which debugger to call */
		extern int kdb_wanted;
		int srval;
		caddr_t cur_addr;
		
		if (mst1)
			cur_addr = mst1->iar;
		else
			cur_addr = (ppda[db_get_processor_num()])._csa->prev->iar;

		srval = (mst1->as).srval[((ulong)cur_addr)>>SEGSHIFT];

		if (kdb_wanted && (rsn_code != DBG_TRAP || !db_is_to_call(cur_addr, srval)) && 
			rsn_code != DBG_STATIC_TRAP && rsn_code != DBG_MPC_STOP )		    
		{
			kdb_main(mst1, rsn_code, ext_arg);
			if (kdb_wanted)
				return(OK);
		}
	}
#endif /* _KDB */
	
	/* else lldb is always to call */

#ifdef _POWER_MP /* The problem of refetching current instr is avoided,
					Come back to standard trap */
	if (rsn_code == DBG_STATIC_TRAP)
		rsn_code = DBG_TRAP;
#endif /* _POWER_MP */


	switch(dbg_avail & LLDB_MASK) {
		case NO_LLDB:
			return(NODEBUGRET);
		case DONT_TRAP:
			if (!dbg_pinned) {
				pin(&pwr_obj_end,(int)&pin_dbg_end-(int)&pwr_obj_end);
				pin(&pwr_com_end,(int)&pin_dbgcom_end-(int)&pwr_com_end);
				dbg_pinned = 1;
#ifdef _POWER_MP
				if (__power_mp())
					dbcpu_init();
#endif /* POWER_MP */
			}
			break;
		case DO_TRAP:
			break;
	}
#ifdef _POWER_MP
	if (__power_mp()){
		cur_cpu = db_get_processor_num();
		/* ignores spurious MPC_STOP */
 		if ((rsn_code == DBG_MPC_STOP) && (action[cur_cpu] != stop)){
			return (OK);
		}
		if (rsn_code != DBG_MPC_STOP) /* ignore the stop action if 
                                                 after an other reasoned entry */
			goto dbg_waiting_loop;
stop_loop: /* wait for action == debug or resume, skip over if action was not stop */
		wait_count = 0;
		if (action[cur_cpu] == stop){
			status[cur_cpu] = stopped;
			while ( action[cur_cpu] == stop) { 
				if (no_debugging_cpu()) {
					if ( ++wait_count > MAX_WAIT_COUNT ){
						if (step_processing_level > 0)
          						/* certainly blocked for a step exec */
							step_processing_level--;
						/* exceptionnal case of self-modifying
						   action to pass test at entry of db_main */
						action[cur_cpu] = debug;
						break;
					}
				}
				else
					wait_count = 0;
			}
			
			
			if (action[cur_cpu] == resume) { /* We are not under debugger lock, so dont
							  * reset action to NONE. leave it to resume.
							  * this won't prevent us to enter the debugger
							  * in case of BP hit.
							  * If an other cpu want to stop us, it will
							  * set action to stop.
							  */
				
				/* to be uncommented when watch command available
				   set_brat_and_dabr(); 
				   */
				status[cur_cpu] = running;
				return (OK);
			}
			/* else action == debug, try to enter the debugging state */
		}
dbg_waiting_loop:

		wait_count = 0;
		status[cur_cpu] = debug_waiting;
		
		/* Try to acquire the debugger lock only if :
		 * - no cpu has been selected by cpu command
		 * - a cpu has been selected and it's me!
		 */
		
		while (TRUE){
			if ((action[cur_cpu] != stop) /* step execution on debug cpu */
			    && ((selected_cpu == NO_SELECTED_CPU) 
			 	|| (selected_cpu == cur_cpu))){
				if (db_lock_try(&debugger_lock,cur_cpu)) {
					/* I got the lock, I stop to loop
					 * and will enter the debugger.
					 */
					break;
				}
			}
			else { 
				if (no_debugging_cpu()) {
					/* try to force the control of debug */
					if ( ++wait_count > MAX_WAIT_COUNT ){
						wait_count = 0;
						/* exceptionnal case of self-modifying
						   action to pass test at entry of db_main */
						selected_cpu = NO_SELECTED_CPU;
						
					}
				}
				else
					wait_count = 0;
			}
			
		}
		
	}
#endif /* _POWER_MP */

	db_main(mst1, rsn_code, ext_arg);

#ifdef _POWER_MP
	if (__power_mp()){
		/* Check action on db_main exit : default is NONE,
		 * but may be stop if the last command was "cpu"
		 * we must then enter the stopped state
		 */
		if (action[cur_cpu] == stop){
			/* special case : the last command was "cpu" :
		 	* we go to the stopped state,
		 	* we have to release the debugger lock to allow
		 	* the selected cpu to enter the debugger
			* if reason was "trap" and current trap was a step,
			* it has been removed from the step table, so change
			* the reason to MPC_STOP.
			*/
			rsn_code = DBG_MPC_STOP;

			db_start_rtc();
			/* We must make sure to unwind all recursively-held
			 * locks, or else the other cpu will hang requesting
			 * the lock.  These may have been created by reentering
			 * the debugger due to, e.g., a DSI while displaying
			 * memory.  We have to get a local copy of the
			 * lockcount beforehand to avoid conflict with other
			 * waiting CPUs.  Note that the full solution might
			 * include a complete backtracking mechanism, but the
			 * assumption is that we're going away anyway.  We
			 * may have a previous debugger MST, but we'll never
			 * get to it (we'll dump before we get there).
			 */
			i = debugger_lock.lockcount;
			for (j=0; j<i; j++)
			  db_unlock(&debugger_lock);

			goto stop_loop;
		}
		else{
			/* standard case, exit the debugger */
			status[cur_cpu] = running;
			/* Here, we don't need to unwind recursive locks, 
			 * because if we have them, we have recursive entry
			 * to the debugger for a debugger problem.  Thus, we
			 * will never return to the previous debugger stack,
			 * because the system will dump and die before then.
			 */
			db_unlock(&debugger_lock);
		}

	}
	db_start_rtc();
#endif /* _POWER_MP */
	return(OK);
	
}

/*
 * Stub (null) routine for unresolved debugger-provided functions.
 *
 * The routines are resolved when the debugger is initialized.
 */
int
dbstub()
{
	return(0);
}

#ifdef _POWER_MP

/*
 * NAME: db_init_bs_proc
 * called from main.c each time a cpu is brought up
 * 
 * updates the number of cpus
 */

int db_init_bs_proc()
{
	int cur_cpu;

	cur_cpu = db_get_processor_num();
	action[cur_cpu] = NONE;
	status[cur_cpu] = running;
}

/*
 * NAME: no_debugging_cpu
 * usefull to know if a waiting cpu has good reasons
 * to wait. Except on step recovery, no way to wait
 * if there is not another debugging cpu.
 * 
 * Return TRUE if no processor in db_main, FALSE else
 */

int no_debugging_cpu()
{
int i;

	for(i=0;i<number_of_cpus;i++)
		if (status[i] == debugging)
			return FALSE;
	return TRUE;

}



/*
 * NAME: db_stop_rtc()
 * Stop the real-time clock.(to limitate scheduling at the debugger exit)
 * 
 * No return
 */

db_stop_rtc()
{
#ifdef _RS6K_SMP_MCA
   if (__power_mp())
	if (__rs6k_smp_mca()){
		struct timestruc_t timer1, timer2;
		/*
		 * nothing to do if rtc is stopped
		 */
		curtime(&timer1);
		curtime(&timer2);
		if ((timer1.tv_sec == timer2.tv_sec) &&
			 (timer1.tv_nsec == timer2.tv_nsec)) {
			rtc_to_restart = 0;
			return;
		}
		rtc_to_restart = 1;
		pgs_rtc_stop();
	}
#endif /* _RS6K_SMP_MCA */
}

/*
 * NAME: db_start_rtc()
 * Start the real-time clock.
 * 
 * No return
 */

db_start_rtc()
{
#ifdef _RS6K_SMP_MCA
   if (__power_mp())
	if (__rs6k_smp_mca())
		if (rtc_to_restart == 1)
			pgs_rtc_start();
#endif /* _RS6K_SMP_MCA */
}
#endif /* _POWER_MP */


#if defined (_POWER_MP) || defined (_KDB)

/*
 * NAME: db_is_to_call(addr, srval)
 *   Called from p_slih 
 * 
 * Return TRUE if there is a breakpoint to handle, FALSE else
 */

int db_is_to_call(addr, srval)
caddr_t addr;
int srval;
{
	int step_id;
	
	if (dbg_pinned)
		if (( is_step(addr,TRUE,&step_id,srval)) || (is_break(addr,TRUE,srval) ))
			return TRUE;
	
	return FALSE;

}
#endif /* _POWER_MP || _KDB */


/*
 * NAME: kercdtf() - kernel compontent dump table function
 *
 * Called by dmp_do at dump time.
 * Return a pointer to a component dump table (cdt).
 */
static struct {
	struct cdt_head  _cdt_head;
	struct cdt_entry  cdt_entry[1];
} kercdt = {
	{ DMP_MAGIC, "bos", sizeof(kercdt) },
{	{ "kernel", 256*1024*1024/*# bytes to dump*/ , 0 /* start addr */, 0 } }
};

static struct cdt_head *kercdtf()
{

	kercdt.cdt_entry[0].d_len =  256*1024*1024;
	kercdt.cdt_entry[0].d_ptr = 0 ;
	return((struct cdt_head *)&kercdt);
}


/*
 * NAME:      ker_dump_init 
 *
 * FUNCTION:  add the kernel to the component dump table
 *
 * RETURNS:
 *
 */
extern struct cdt *vm_dump();

ker_dump_init()
{
	dmp_add(kercdtf);

	/* Add VMM dump routine to component dump table.
	 */
	dmp_add(vm_dump);
}



/*
 * NAME: dbtty_init
 *
 * FUNCTION: this routine initializes the system console so that kernel
 *	printf's can be printed. 
 *
 * PARAMETERS: all data areas used are global
 *
 * RETURN VALUE: 0 for ok, -1 if can't open port.
 *
 */
void
dbtty_init()
{
	int ttynum, rc;

	for(ttynum=0;(rc=d_ttyopen(ttynum)) != -1;ttynum++) {
		if(rc>0) {
			dbterm = USE_TTY | ttynum ;  /*found a port*/
#ifdef _POWER_MP
			d_ttyclose(ttynum);
#endif /* _POWER_MP */
			break;
		}
	}
}

/*
 * NAME: db_active
 *
 * FUNCTION: this routine indicates whether the debugger is currently
 *	active on the current processor.
 *
 * PARAMETERS: all data areas used are global
 *
 * RETURN VALUE: 1 if debugger is active, 0 if not.
 *
 */
int
db_active()
{
	/* If this is an MP box, we need to look at the debugger_lock,
	 * because this is the only reliable test.
	 */
#ifdef _POWER_MP
	if (__power_mp()) 
		/* Be careful not to reference unpinned memory - if
		 * the debugger isn't pinned, it can't be active
		 */
		if (dbg_pinned)
			return (debugger_lock.lockword &&
				(debugger_lock.owning_cpu == cpunb));
		else
			return 0;
	else
#endif /* _POWER_MP */
	/* In non-MP cases, we aren't using the debugger_lock, so the
	 * valid test is the same that is used in the kernel printf.
	 * Since there's only one processor, debabend is reliable.  Note
	 * that debabend is declared in vdbprf.o, which is always pinned.
	 */
		return (debabend==IN);
}
