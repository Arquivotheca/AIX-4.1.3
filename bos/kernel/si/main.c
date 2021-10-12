static char sccsid[] = "@(#)93	1.55  src/bos/kernel/si/main.c, syssi, bos41J, 9512A_all 3/20/95 16:15:10";
/*
 * COMPONENT_NAME: (SYSSI) System Initialization
 *
 * FUNCTIONS:	main
 *
 * ORIGINS: 27, 3, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */


#include <sys/types.h>
#include <sys/systm.h>
#include <sys/var.h>
#include <sys/vmker.h>
#include <sys/utsname.h>
#include <sys/user.h>
#include <sys/lockl.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/init.h>
#include <sys/lldebug.h>
#include <sys/low.h>
#include <sys/iplcb.h>
#include <sys/sys_resource.h>
#include <sys/processor.h>
#ifdef _POWER_MP
#include <sys/systemcfg.h>
#endif /* _POWER_MP */
#ifdef _PEGASUS
#include <sys/mdio.h>
#include "pegasus.h"
#endif

extern int dbg_avail;			/* debugger available	*/
extern void proc1init();		/* initial function for process 1 */
void init_anyother_locks();

volatile union {
	struct timestruc_t rtc;		/* POWER & PowerPC 601 rtc format */
	long long unsigned int tb;	/* PowerPC Time Base format */
} start_bs_rtc = {0};

/* processor_present states; should be in iplcb.h */
#define PROC_DISABLED           -2
#define PROC_FAILED             -1
#define PROC_ABSENT             0
#define PROC_RUNNING_AIX        1
#define PROC_LOOPING            2
#define PROC_IN_RESET           3

#ifdef _POWER_PC

#define SYS_RES_EADD(real_add) \
        (uint *)((uint)(sys_resource_ptr) + ((uint)real_add & ~0xFF000000))
#endif

/*
 * NAME: main
 *
 * FUNCTION: kernel initialization
 *
 *	Initialize the kernel by calling a series of subroutines
 *	which perform specific initialization functions.
 *
 * NOTES:
 *	This program starts out single-threaded with interrupts
 *	disabled.  As initialization progresses, interrupts are
 *	enabled and this program turns into a process as multi-
 *	tasking is enabled.  When initialization is completed
 *	this program transforms itself into the swapper process.
 *	The first thing swapper will do is free up the virtual
 *	memory containing the system initialization code, so 
 *	main and its subroutines will disappear.
 *
 * INPUT: none
 *	`start.s' receives machine-dependent arguments at
 *	boot time.  It saves them in global variables known
 *	only to routines specific to that platform.  This
 *	preserves the machine-independence of `main'.
 *
 * EXECUTION ENVIRONMENT:
 *	Runs single-thread
 *	Uses system init stack (passed from start)
 *
 * RETURNS:  Never!
 */
main()
{
	register i;

	/*
	 *  On entry, interrupts are disabled, translation is turned off,
	 *  and kernel bss is inaccessible.
	 */

#if defined(_KDB)
	kdb_init();			/* initialize the kernel debugger */
#endif /* _KDB */

	hardinit();			/* initialize the hardware */
	vmsi();				/* init virtual memory */

	init_locks();			/* initialize lock hash anchors */
 
 	/* Allocate and initialize any other locks--this routine must
 	 * be called IMMEDIATELY AFTER init_locks() is called.
	 *
	 * Any locks referenced prior to this point will not have 
	 * been instrumented. These locks SHOULD BE FREE (i.e., not locked),
	 * therefore, allowing them to be properly allocated and initialized.
 	 *
 	 * Currently, this only has an effect if instrumentation is
 	 * active (instr_avail != 0).
 	 */
	init_anyother_locks();

	ios_init();			/* Initialize IO sub. system */

	/*
	 *  Translation is now on, and non-I/O page faults are allowed.
	 *  Kernel bss is initialized, and may be used.
	 *
	 *  Up to this point, we were running disabled with no processes.
	 */

#if defined(_KDB)
	kdb_pin_symtable();		/* pin unix symbol table */
#endif /* _KDB */

	debugger_init();		/* start the kernel debugger */
	kmem_init();			/* initialize kernel memory heaps */
	intr_init();			/* initialize interrupt management */
	strtdisp();			/* start up the dispatcher */

#ifdef _POWER
	epost();			/* Run any EPOST's */
#endif 

	/* 
	 *  Now, we're enabled and running under process 0.
	 *
	 *  Run through the table of functions needed to be called for
	 *  various initializations.  See `init.h' for the list.
	 *  First, get the kernel non-preemption lock in case any of the
	 *  init_tbl functions need it.
	 */
    	lockl(&kernel_lock, LOCK_SHORT);

	for (i=0; i < (sizeof init_tbl)/(sizeof init_tbl[0]); i++)
	    if (init_tbl[i])
		(*init_tbl[i])();

	unlockl(&kernel_lock);		/* release the kernel lock */

	/* print utsname info if debugger enabled with initial break point.
	 */
	if (dbg_avail == DO_TRAP)
		printf("\n%s Version %s.%s\n",
		       utsname.sysname, utsname.version, utsname.release);

#if defined(_KDB)
	if (dbg_avail != DO_TRAP && __kdb())
		printf("\n%s Version %s.%s\n",
		       utsname.sysname, utsname.version, utsname.release);
#endif /* _KDB */

	initp(1, proc1init, -1, 0, "init");  /* start /etc/init in process 1 */
	ker_dump_init();		/* init dump table to dump the kernel */

#ifdef _POWER_MP
#ifndef _SLICER
	if (__power_mp())
		boot_slave_procs();        /* boot-slave processor initialization */
#endif /* _SLICER */
#endif /* _POWER_MP */
#if defined(_KDB)
	/* Connect DSI and ISI vectors to kernel (instead of KDB) */
	kdb_fix_vectors();
#endif /* _KDB */

#if defined(_POWER_MP) & defined(_SLICER)
	{
		/* Initialize the Slicer at this point in time */
		extern void slicer_init();
		slicer_init();
	}
#endif /* _POWER_MP & _SLICER */

	runsched();			/* run the scheduler in process 0 */

	/*
	 * Note: control does not return.  In fact, sched() will
	 * free the memory associated with system initialization.
	 */
}

#ifdef _POWER_MP
/*
 * NAME: boot_slave_procs
 *
 * FUNCTION: launch each available processor
 *
 * INPUT: scans ipl control block to find out which processors can be started
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 */
boot_slave_procs()
{
        struct ipl_cb *iplcb_ptr;       /* ros ipl cb pointer */
        struct ipl_directory *dir_ptr;  /* ipl dir pointer */
        struct processor_info *proc_ptr;
        int index;

        extern unsigned int *start_bs_proc; /* code entry point */
	extern volatile struct ppda * volatile start_bs_param; 
#ifdef _POWER_PC
	extern long long unsigned int mftb();
#endif /* _POWER_PC */

        iplcb_ptr = (struct ipl_cb *)(vmker.iplcbptr);
        dir_ptr = &(iplcb_ptr->s0);
        proc_ptr = (struct processor_info *) ((char *) iplcb_ptr +
                                              dir_ptr->processor_info_offset); 

        for(index = 0; index < proc_ptr->num_of_structs; index++, 
		proc_ptr = (struct processor_info *)((uint)proc_ptr + proc_ptr->struct_size))
        {
                if( (proc_ptr->processor_present != PROC_LOOPING) &&
                   (proc_ptr->processor_present != PROC_IN_RESET))
                        continue;

                /* assign Base Address of this processor interrupt area */
                ppda[number_of_cpus].intr = SYS_RES_EADD(proc_ptr->proc_int_area);
                /* assign next logical number */
                ppda[number_of_cpus].cpuid = number_of_cpus;
                start_bs_param = &ppda[number_of_cpus]; /* param is the ppda pointer */

                printf("Starting processor #%d ...", number_of_cpus);
#ifdef _PEGASUS
                if (__pegasus())
                        pgs_rtc_stop();
#endif /* _PEGASUS */

#ifdef _POWER_PC
		if (__power_pc() && !__power_601())
			start_bs_rtc.tb = mftb();
		else
#endif /* _POWER_PC */
                        curtime_pwr(&start_bs_rtc.rtc);

                if (proc_ptr->processor_present == PROC_LOOPING) {
                        proc_ptr->link_address = start_bs_proc;
                        __iospace_sync();
                        proc_ptr->link = 1;
                } else {
#ifdef _PEGASUS

                        if (__pegasus()) {
                                if (mdcpuset(index, MCPU_START)) {
                                        pgs_rtc_start();
                                        printf("physical cpu #%d NOT AVAILABLE\n",
                                               index);
                                        continue;
                                }
                        } else 
#endif /* _PEGASUS */
			{
                                printf("Failed to start physical cpu #%d\n",
                                       index);
                                continue;
                        }
                }
                /* Wait for this processor to free start_bs_rtc
                 * This is signalled by the reset of start_bs_rtc,
                 * in start_bs_proc().
                 */
                while (start_bs_rtc.rtc.tv_sec != NULL)
                        ;
#ifdef _PEGASUS
                if (__pegasus())
                        pgs_rtc_start();
#endif /* _PEGASUS */
                /* Wait for this processor to free start_bs_stack and param
                 * This is signalled by the reset of start_bs_param, at end of
                 * start_bs_proc().
                 */
                while (start_bs_param != NULL)
                        ;
                printf(" done.\n");
                proc_ptr->processor_present = PROC_RUNNING_AIX;
                number_of_cpus++;
                v.v_ncpus++;
        }
}

/*
 * NAME: main_bs_proc
 *
 * FUNCTION: Boot-slave processor initialization
 *
 *	Initialize the processor by calling a series of subroutines
 *	which perform specific initialization functions.
 *	Only the processor resources need be initialized:
 *	- VMM associated registers
 *	- boot time context
 *	- Interrupt registers: CPPR and al
 *	- RealTimeClock/TimeBase registers
 *
 * NOTES:
 *
 * INPUT: ppda address (in SPRG0)
 *        - ppda->intr
 *        - ppda->cpuid
 *
 * EXECUTION ENVIRONMENT:
 *	Runs on a special stack: start_bs_stk (set up in start_bs_proc)
 *	(there is not one stack per-processor, and so we cannot start
 *	 several processors at the same time)
 *
 * RETURNS:  None.
 */
main_bs_proc()
{
	struct proc *p;

	/* init rtc and signal master that clock has been initialized */ 
#ifdef _POWER_PC
	if (__power_pc() && !__power_601())
		mttb(start_bs_rtc.tb);
	else
#endif /* _POWER_PC */
		update_system_time_pwr(start_bs_rtc.rtc);
        start_bs_rtc.rtc.tv_sec = NULL;

#if defined(_KDB)
	kdb_init_slave(CPUID);
#endif /* _KDB */
	vm_bs_proc();		/* VMM initializations */
	/* context is initialized with unique id */
	p = &proc[CPUID + 2];
	si_context_init(p->p_threadlist->t_tid);
	db_init_bs_proc();      /* lldb multi init */
	tinit();		/* timer initializations */
	strtwait_bs_proc();	/* start wait process */
	ios_bs_proc();		/* I/O Subsystem */
}
#endif /* _POWER_MP */

/*
 * NAME: init_anyother_locks
 *
 * FUNCTION: catch-all for lock initialization codes
 *
 * NOTES:
 *          Got a lock or set of locks that need to be initialized,
 *          but don't know when or where to do it?  Try putting it here.
 *
 *          This function is designed to be the earliest possible location
 *          that locks can be allocated and initialized to take into
 *          account INSTRUMENTATION.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:  None.
 */

void
init_anyother_locks()
{
	register int ipri;

#ifdef _POWER_MP
extern Simple_lock	mpc_lock;	/* lock on mpc_reg_array update   */ 
#endif
	
	ipri = i_disable(INTMAX);

#ifdef _POWER_MP
	init_vmmlocks();
	if (__power_mp()) {
		lock_alloc(&mpc_lock, LOCK_ALLOC_PIN, IOS_LOCK_CLASS, -1);
		simple_lock_init(&mpc_lock);
	}
#endif /* _POWER_MP */

	/* Allocate and Initialize Process Management locks */
	lock_alloc(&proc_tbl_lock, LOCK_ALLOC_PIN, PROC_TBL_CLASS, -1);
	lock_alloc(&ptrace_lock, LOCK_ALLOC_PIN, PTRACE_CLASS, -1);
	lock_alloc(&core_lock, LOCK_ALLOC_PIN, CORE_LOCK_CLASS, -1);
	simple_lock_init(&proc_tbl_lock);
	simple_lock_init(&ptrace_lock);
	lock_init(&core_lock, TRUE);

#ifdef _POWER_MP
	lock_alloc(&proc_int_lock, LOCK_ALLOC_PIN, PROC_INT_CLASS, 1);
	lock_alloc(&proc_base_lock, LOCK_ALLOC_PIN, PROC_INT_CLASS, 2);
	lock_alloc(&tod_lock, LOCK_ALLOC_PIN, TOD_LOCK_CLASS, -1);
	lock_alloc(&uprintf_lock, LOCK_ALLOC_PIN, UPRINTF_CLASS, -1);
	lock_alloc(&watchdog_lock, LOCK_ALLOC_PIN, WATCHDOG_LOCK_CLASS, -1);
	lock_alloc(&time_lock, LOCK_ALLOC_PIN, TIME_LOCK_CLASS, -1);
	simple_lock_init(&proc_int_lock);
	simple_lock_init(&proc_base_lock);
	simple_lock_init(&uprintf_lock);
	simple_lock_init(&tod_lock);
	simple_lock_init(&watchdog_lock);
	simple_lock_init(&time_lock);
#endif

	i_enable(ipri);
}
