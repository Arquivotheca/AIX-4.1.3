static char sccsid[] = "src/bos/diag/tu/eth/exectu.c, tu_eth, bos411, 9428A410j 6/25/92 14:34:10";
/*
 * COMPONENT_NAME: (ETHERTU) Ethernet Test Unit
 *
 * FUNCTIONS: exectu
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************

Function(s) Exec TU for HTX Exerciser and Diagnostics

Function called by both the Hardware Exercise, Manufacturing Application,
and Diagnostic Application to invoke a Test Unit (TU).

If the "mfg" member of the struct within TUTYPE is set to INVOKED_BY_HTX,
then exectu() is being invoked by the HTX Hardware Exerciser so
test units know to look at variables in TUTYPE for values from the
rule file.  Else, the test units use pre-defined values while testing.

*****************************************************************************/
#include <stdio.h>
#include <memory.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/entuser.h>
#include <errno.h>
#include "ethtst.h"	/* note that this also includes hxihtx.h */
#ifdef debugg
extern void detrace();
#endif

#define SAVE_SIG	-135
#define RESTORE_SIG	-136

/*
 * Here, we declare a 'global' jumpbuf variable local only
 * to this file for handling user generated interrupts.
 */
static jmp_buf ctxt;

/*****************************************************************************

dma_signal_handler

On invocation (via interrupt), function jumps back into "exectu()" to setjmp
location.  NOTE THAT SETJMP() MUST HAVE BEEN CALLED PREVIOUS TO THIS
INVOCATION (as in "exectu()")!!!

*****************************************************************************/

void dma_signal_handler (sig_type)
   int sig_type;
   {
	void longjmp();

	longjmp(ctxt, 1);
   }

/*****************************************************************************

sr_signal_table

Based on "op", function saves/restores signals.

*****************************************************************************/

int sr_signal_table (op)
   int op;
   {
	static int signal_types[] =
	   {
		SIGHUP, SIGINT, SIGQUIT, SIGALRM, SIGTERM
	   };
	static void (*signal_table[sizeof(signal_types) / sizeof(int)])();
	static int num_sigs = sizeof(signal_types) / sizeof(int);
	int i;

	if (op == SAVE_SIG)
	   {
		for (i = 0; i < num_sigs; i++)
		   {
			signal_table[i] = signal(signal_types[i],
						dma_signal_handler);
			if (signal_table[i] == -1)
				return(SIG_SAV_ERR);
		   }
		return(0);
	   }

	if (op == RESTORE_SIG)
	   {
		for (i = 0; i < num_sigs; i++)
		   {
			if (signal(signal_types[i], signal_table[i]) < 0)
				return(SIG_RES_ERR);
		   }
		return(0);
	   }
	return(SIG_OP_ERR);
   }

/*****************************************************************************

exectu

*****************************************************************************/

int exectu (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	register i, loop, tu;
	static int rc;
	static int card_started = 0;
        static int start_err = 0;
	static struct session_blk sess_s;
	struct status_block stat_s;
	unsigned char save_area[10];
	long status;
	extern int start_eth();
	extern int init_pos();
	extern int reg_save();
	extern int reg_restore();
	extern int mktu_rc();
	extern int tu001();
	extern int tu002();
	extern int tu003();
	extern int tu004();
	extern int tu005();
	extern int tu006();
	extern int tu007();
	extern int tu008();
	extern int tu009();
	extern int tu010();
	extern int tu011();
	extern int tu012();

	if (!card_started)
	   {
		sess_s.netid = 0x1234;
		sess_s.length = 2;
		tucb_ptr->eth_htx_s.netid = sess_s.netid;
		if (rc = start_eth(fdes, &sess_s, tucb_ptr))
		   {
#ifdef debugg
        detrace(0,"Card Not Started\n");
#endif
                     /* do not retry for diagnostics */

                     if ( tucb_ptr ->header.mfg == INVOKED_BY_HTX )
                       {
                       start_err++;
                       if (start_err == 3)
                        {
       		         (void) ioctl(fdes, CIO_HALT, &sess_s);
                        }
                        rc= rc | 0xff000000; 
                       }
                     else
                       (void) ioctl(fdes, CIO_HALT, &sess_s);
        	     return(rc);
		   }
		
		/*
		 * initialize POS registers
		 */

		if (rc = init_pos(fdes, tucb_ptr))
		   {
			(void) ioctl(fdes, CIO_HALT, &sess_s);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
		   }
		card_started = 1;
                start_err = 0;
	   }


	tu = tucb_ptr->header.tu;
	loop = tucb_ptr->header.loop;
	if (tucb_ptr->header.mfg != INVOKED_BY_HTX)
		tucb_ptr->eth_htx_s.htx_sp = NULL;
	
	/*
	 * Save off our context here before setting
	 * up our signals.  If we get a user interrupt,
	 * our interrupt handler (dma_signal_handler)
	 * will longjmp to here with a '1'.
	 */
	if (setjmp(ctxt))
	   {

		/*
		 * Free up DMA buf (function checks 
		 * if necessary)
		 */
		(void) access_dma(fdes, FREE_DMA,
			NULL, 0, 0, 0, tucb_ptr);

		(void) ioctl(fdes, CIO_HALT, &sess_s);
		card_started = 0;

		/*
		 * Return error to user letting them know
		 * that we somehow were interrupted.
		 */
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, SIG_REC_ERR));
	   }
	
	/*
	 * Save off user's signals
	 */
	if (rc = sr_signal_table(SAVE_SIG))
	   {
		(void) ioctl(fdes, CIO_HALT, &sess_s);
		card_started = 0;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
	   }

	for (i = 0; i < loop; i++)
	   {
		/*
		 * FIRST, save off all POS and HIO registers.
		 * IF fails, form error code and then fall out of for-loop.
		 */
		if (rc = reg_save(fdes, save_area, &status, tucb_ptr))
		   {
			/*
			 * while the TUs form the complete error code,
			 * reg_save() and reg_restore() only return
			 * the bottom two bytes, so we need to form
			 * the upper two bytes.
			 */
			rc = mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc);
			break;
		   }

		switch(tu)
		   {
			case  1:
				rc = tu001(fdes, tucb_ptr);
				break;

			case  2:  
				rc = tu002(fdes, tucb_ptr);
				break;

			case  3:
				rc = tu003(fdes, tucb_ptr);
				break;

			case  4:
				rc = tu004(fdes, tucb_ptr);
				break;

			case  5:
				rc = tu005(fdes, tucb_ptr);
				break;

			case  6:
				rc = tu006(fdes, tucb_ptr);
				break;

			case  7:
				rc = tu007(fdes, tucb_ptr);
				break;

			case  8:
				rc = tu008(fdes, tucb_ptr);
				break;

			case  9:
				rc = tu009(fdes, tucb_ptr);
				break;

			case 10:
				rc = tu010(fdes, tucb_ptr);
				break;

			case 11:
				rc = tu011(fdes, tucb_ptr);
				break;

			case 12:
				rc = tu012(fdes, tucb_ptr);
				break;

			default:
				card_started = 0;
				(void) ioctl(fdes, CIO_HALT, &sess_s);
				(void) sr_signal_table(RESTORE_SIG);
				return(ILLEGAL_TU_ERR);
		   };
		
		if (rc)
		   {
			/*
			 * if running manuf. diagnostic and a tu returns
			 * an error, then break out of for-loop.
			 */
			if (tucb_ptr->header.mfg != INVOKED_BY_HTX)
				break;
			/*
			 * check on retries keyword.  If set
			 * from rule file stanza, then
			 * continue loop to retry tu, else
			 * break out and return.
			 */
			if (!(*tucb_ptr->eth_htx_s.retries))
				break;
		   }
		/*
		 * after successful completion of TU,
		 * restore all the original settings of
		 * the POS and HIO registers.  If fails,
		 * then form error code and drop out of for-loop.
		 */
		if (rc = reg_restore(fdes, save_area, &status, tucb_ptr))
		   {
			/*
			 * while the TUs form the complete error code,
			 * reg_save() and reg_restore() only return
			 * the bottom two bytes, so we need to form
			 * the upper two bytes.
			 */
			rc = mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc);
			break;
		   }
	   } /* end for */

	/*
	 * an error occurred in a TU or something, so
	 * try to restore the original settings of the POS and HIO
	 * registers, HALT the device, and then return the original
	 * error code.
	 */
	if (rc)
	   {
		(void) reg_restore(fdes, save_area, &status, tucb_ptr);
		(void) ioctl(fdes, CIO_HALT, &sess_s);
		card_started = 0;
		(void) sr_signal_table(RESTORE_SIG);
		return(rc);
	   }
#if 0
	/*
	 * everything has worked fine up to this point,
	 * so try to HALT the device naturally.  If error,
	 * then report it.
	 */
	if (ioctl(fdes, CIO_HALT, &sess_s))
	   {
		if ((tucb_ptr->header.mfg == INVOKED_BY_HTX) &&
		    (tucb_ptr->eth_htx_s.htx_sp != NULL))
			(tucb_ptr->eth_htx_s.htx_sp->bad_others)++;
		(void) sr_signal_table(RESTORE_SIG);
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, HALT_ERR));
	   }
	if ((tucb_ptr->header.mfg == INVOKED_BY_HTX) &&
	    (tucb_ptr->eth_htx_s.htx_sp != NULL))
		(tucb_ptr->eth_htx_s.htx_sp->good_others)++;
#endif

	if (rc = sr_signal_table(RESTORE_SIG))
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
		
	return(rc);
   }
