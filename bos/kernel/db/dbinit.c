static char sccsid[] = "@(#)83  1.1  src/bos/kernel/db/dbinit.c, sysdb, bos411, 9428A410j 4/20/94 17:33:47";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: debugger_init, free_dbg
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
#include <sys/dbkersym.h>
#include <sys/lldebug.h>
#include <sys/dbg_codes.h>
#include <sys/systemcfg.h>

extern long pwr_obj_end;
extern long pwr_com_end;
extern long pin_dbg_end;
extern long pin_dbgcom_end;
extern int dbg_avail;              /* set by mkboot - indicates if debugger */
				   /* is supposed to be available and/or invoked*/

extern int dbg_pinned;                 /* flag indicating whether or not the */
				       /* debugger code has been pinned */

#ifdef _POWER_MP
extern int dbkdbx_lvl;                 /* to tell kdbx the level of features that */
				       /* are implemented in lldb */
#endif /* POWER_MP */

/*
 * EXTERNAL PROCEDURES CALLED:
 */

/*
 * NAME:  debugger_init
 *
 * FUNCTION:  determine if the debugger is to be accessable or not,
 *              and whether to invoke it now (at boot time).
 *
 * INPUT:     global dbg_avail  <- set by mkboot
 *
 *
 *
 * OUTPUT:    call the debugger via brkpoint() or not
 *
 */

void
debugger_init()
{
	void dbtty_init();
	void free_dbg(caddr_t  reladdr, int relsize);

#ifdef _POWER_MP
	dbkdbx_lvl=1;   /* tell kdbx that these features are supported : */
			/* switch, cpu, local breakpoints, links between
			/* system-call stack and user stack */
#endif /* _POWER_MP */

	dbg_pinned = 0;
	switch(dbg_avail & LLDB_MASK) {
		case NO_LLDB:
			/* release debugger pages as it is not being used */
			free_dbg(&pwr_obj_end,(int)&pin_dbg_end-(int)&pwr_obj_end);
			free_dbg(&pwr_com_end,(int)&pin_dbgcom_end-(int)&pwr_com_end);
			return;
		case DONT_TRAP:
			pin(&pwr_obj_end,(int)&pin_dbg_end-(int)&pwr_obj_end);
			pin(&pwr_com_end,(int)&pin_dbgcom_end-(int)&pwr_com_end);
			dbg_pinned = 1;
			dbtty_init();
#ifdef _POWER_MP
			if (__power_mp())
				dbcpu_init();
#endif /* POWER_MP */
			return;
		case DO_TRAP:
			pin(&pwr_obj_end,(int)&pin_dbg_end-(int)&pwr_obj_end);
			pin(&pwr_com_end,(int)&pin_dbgcom_end-(int)&pwr_com_end);
			dbg_pinned = 1;
#ifdef _POWER_MP
			if (__power_mp())
				dbcpu_init();
#endif /* POWER_MP */
			break;
	}

#if defined(_KDB)
	_brkpoint();                    /* initial debugger invocation */
#else  /* _KDB */
	brkpoint();                     /* initial debugger invocation */
#endif /* _KDB */
}

/*
 * NAME: free_dbg
 *
 * FUNCTION: free memory used by debugger using vm_release
 *
 * NOTES:
 *          free whole number of pages occupied by the given
 *          memory range.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:  None.
 */

void
free_dbg(caddr_t  reladdr, int relsize)
{
int x;
	/* if starting address is not already page aligned then
	 * align to start of next page and reduce the number
	 * of bytes to be freed
	 */
	if (x = (int)reladdr & (PAGESIZE - 1)) {
		reladdr = (int)reladdr + (PAGESIZE - x);
		relsize = relsize - (PAGESIZE - x);
	}
	/* release only whole number of pages */
	relsize = relsize & ~(PAGESIZE - 1);
	/* release unused pages */
	vm_release(reladdr,relsize);
}
