static char sccsid[] = "@(#)57	1.10  src/bos/kernel/ios/POWER/ctlaltnum.c, sysios, bos411, 9428A410j 11/15/93 09:33:42";

/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: ctlaltnum()
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/iocc.h> 		/* defines KEYPOS */
#include <sys/ppda.h> 		/* defines CSA */
#include <sys/mstsave.h> 	/* defines CSA-> */
#include <sys/dbg_codes.h> 	/* defines debugger msgs */
#include <sys/intr.h> 		/* defines INTMAX */
#include <sys/ctlaltnum.h> 	/* defines NUMPAD */
#include <sys/dump.h> 		/* defines DMPD */
#include <sys/mdio.h> 		/* LED stuff */
#include <sys/syspest.h> 	/* defines ASSERT */
#include <sys/adspace.h> 	/* defines io_att & io_det */
#include <sys/sleep.h> 		/* defines EVENT_SYNC */
#include <sys/systm.h> 		/* defines system time structures */
#include <sys/systemcfg.h> 	/* defines system config structure */
#include <sys/sys_resource.h> 	/* defines system resource structure */
#include "dma_hw.h"		/* defines IOCC_HANDLE */
 
#define	LED_SECREADY	(ulong)0x0A6 /* set led's to " c6" to prompt for secondary ready */
#define	LED_NOKPROC	(ulong)0x0A5 /* set led's to " c5" to show no kproc for secondary ready */
#define	LED_CLEAR	(ulong)0xFFF /* set led's to "   " to clear */
uchar get_key_pos();
static int NUMPAD_2_FLAG=FALSE; /* flag for remembering having seen numpad 2 */




/*
 * NAME:  ctlaltnum
 *
 * FUNCTION:  
 *	Handles ctl atl numpad sequences
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called in an interrupt execution environment by a keyboard handler.
 *
 *      It cannot page fault.
 *
 * NOTES:  
 *
 * RETURN VALUE DESCRIPTION:
 *	void
 *
 * EXTERNAL PROCEDURES CALLED:
 *	debugger
 *	dmp_do
 *	nvled
 *	i_disable
 *	i_enable
 *	wipl
 */
void
ctlaltnum( int numpad )

{
	int ipri;			/* integer to save interupt lvl */
	extern pid_t sec_dump_pid;	/* pid of kproc to do secondary dump */
        struct timestruc_t      ct, bt;


	/* 
	 * disable interrupts 
	 */
	ipri=i_disable(INTMAX);

	ASSERT( CSA->prev != NULL );
	switch ( get_key_pos() ) 
	{
		case KEY_POS_NORMAL:
			break;
		case KEY_POS_SECURE:
			break;
		case KEY_POS_SERVICE:
			switch ( numpad )
			{
				case NUMPAD_1:
					dmp_do(DMPD_PRIM_HALT);
					break;
				case NUMPAD_2:
					if (NUMPAD_2_FLAG)
					{
					    NUMPAD_2_FLAG=FALSE;
					    if (sec_dump_pid == -1)
					    {
						nvled(LED_NOKPROC);
					    }
					    else
					    {
						/* wake up kproc to do dump */
						et_post(EVENT_SYNC,sec_dump_pid);
					    }
					}
					else
					{
					    if (sec_dump_pid == -1)
					    {
						nvled(LED_NOKPROC);
					    }
					    else
					    {
						NUMPAD_2_FLAG=TRUE;
						nvled(LED_SECREADY);
					    }
					}
					break;
				case NUMPAD_4:
					(void)debugger(CSA->prev,DBG_KBD_SERVICE4,0);
					break;
				case NUMPAD_7:
        				/*
				         * Update the time of day chip before reboot.
         				 */
				        curtime(&bt);
				        do {
				                curtime(&ct);
				        } while ( bt.tv_sec == ct.tv_sec );
				        write_clock();

		                        /* Call the system reset slih to send us 
					 * back to ROS the parameter of 1 tells 
					 * sr_slih() that we came from AIX and not
					 * the yellow button
		                         */
		                        sr_slih(1);
					break;
				default:
					break;
			}
			break;
	}



	/* 
	 * enable interrupts 
	 */
	i_enable(ipri);

	return;		

}



/*
 * NAME:  get_key_pos()
 *
 * FUNCTION:  
 *	get key position
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called in an interrupt context.
 *
 * NOTES:
 *	This is the machine independant interface exported to
 *	kernel extensions.
 *
 * RETURN VALUE DESCRIPTION:
 *	uchar
 *
 */

uchar get_key_pos()
{
#ifdef _SNOOPY
	if (__snoopy())
		return(KEY_POS_SERVICE);
	else
#endif /* _SNOOPY */
		return(get_pksr() & KEY_POS_MASK);
} /* get_key_pos() */
