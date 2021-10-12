static char sccsid[] = "@(#)88	1.5  src/bos/kernel/ios/POWER/pio_assist.c, sysios, bos411, 9428A410j 6/5/91 12:29:56";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: pio_assist
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/systm.h>
#include <sys/except.h>
#include <sys/syspest.h>

#define	PIO_ROUTINE	0	/* i/o routine should be called flag */ 
#define	PIO_RECOV	1	/* recovery routine should be called flag */ 


/*
 * NAME:  pio_assist
 *
 * FUNCTION:  This routine provides a general mechanism to assist programmed
 *	I/O handling by setting up an exception handler, invoking the callers
 *      I/O routine and catching any exceptions that may be generated due to
 *      an error. If the exception is not an IO exception, this routine 
 *	invokes the next exception handler on the stack of handlers (longjmpx).
 *	If the exception was I/O and the retry count has not been exceeded,
 *	the count is decremented, and the state set to call the recovery
 *	routine. This routine then again sets up the exception 
 *	handler and either invokes the original i/o routine or invokes the 
 *	caller supplied i/o recovery routine. If the number of i/o exceptions
 *	exceeds a system threshold, this routine invokes the io recovery
 *	routine only for performing error logging functions and returns to the
 *	caller with a return code of EIO. If the exception is not an I/O 
 *	exception or if the I/O recovery routines (either iorecov or the
 *	recalled iofunc) return with EXCEPT_NOT_HANDLED, the next exception
 *	handler on the chain will be invoked (via longjmpx).  
 *
 * RESTRICTIONS: This routine does not allow for the freeing of resources 
 *		 (upon exception) allocated by the i/o routine. The caller
 *		 supplied i/o routine must not allocate any resources that 
 *		 must be returned upon exception.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine may be called in the process or interrupt environment.
 *
 *	It may not page fault.
 *
 * DATA STRUCTURES: none
 *
 * RETURN VALUE DESCRIPTION: 	0 if no error
 *				EIO if I/O error retry count exceeded
 *				Return value from i/o routine
 *				Return value from iorecov routine
 *				No return if exception not handled here
 *
 * EXTERNAL PROCEDURES CALLED:
 *	setjmpx 	set up exception handler
 *	clrjmpx 	remove exception handler
 *	longjmpx 	invoke next exception handler on chain
 *	iofunc		device driver's i/o routine
 *	iorecov		device driver's i/o recovery routine
 */

int
pio_assist(ioparms, iofunc, iorecov)

register caddr_t ioparms;		/* pointer to I/O parameters */
register int	(*iofunc)();		/* ptr to I/O  routine	*/
register int	(*iorecov)();		/* ptr to I/O  recovery routine	*/

{
	volatile int e_count = PIO_RETRY_COUNT; /* max error count */
	int rc;
	volatile int oldrc;
	label_t jump_buf;              /* jump buffer */
	volatile int handler = PIO_ROUTINE;
	volatile int iorc = 0;

	for (;;) /* infinite loop handles re-establishment of exception handler
		    for recovery */
	{	
		/*
		 * Setup for context save by doing setjmpx()
		 */ 
		if ((rc = setjmpx(&jump_buf)) == 0) 

			if (handler == PIO_ROUTINE)	/* invoke i/o routine */
				{
				iorc = (*iofunc)(ioparms);
				assert (iorc != EXCEPT_NOT_HANDLED);
				break;
				}
			else 		/* invoke exception processing */ 
				if (--e_count <= 0)  /* retry count exceeded */
					{
					if (iorecov != NULL)
						iorc = (*iorecov)(ioparms,
								PIO_NO_RETRY,
							  (struct pio_except *)
								  csa->except);
					if (iorc != EXCEPT_NOT_HANDLED)
						  iorc = EIO;		       
					break;
					}			
				else 		/* retry count not exceeded */
				if ( iorecov == NULL)
					{	/* recall io routine */ 
					iorc = (*iofunc)(ioparms);
					break;
					}
				else 	
					{	/* call io recovery routine */

					iorc = (*iorecov)(ioparms, PIO_RETRY, 
							  (struct pio_except *)
							  csa->except);
					break;
					}
		else 	/* Exception has occurred or re-occurred*/	

			if (( rc == EXCEPT_IO ) || ( rc == EXCEPT_IO_SLA ) ||
			    ( rc == EXCEPT_IO_SCU ))
				{
				handler = PIO_RECOV;
				oldrc = rc;	/* save old setjmpx code */ 
				}		   
			else longjmpx(rc);	/* exception not handled here */


	} /* end of for loop */

	clrjmpx(&jump_buf);	 /* remove jump_buf from except stack */

	/* if exception not handled pass exception to next handler on chain */

	if (iorc == EXCEPT_NOT_HANDLED) longjmpx(oldrc);
	else return (iorc);

} 	/* end of pio_assist routine */

























