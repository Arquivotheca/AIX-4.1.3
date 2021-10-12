static char sccsid[] = "src/bos/diag/tu/tok/exectu.c, tu_tok, bos411, 9428A410j 6/20/91 12:41:49";
/*
 * COMPONENT_NAME: (TU_TOK) Token Test Unit
 *
 * FUNCTIONS: exectu
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************

Function(s) Exec TU for HTX Exerciser and Diagnostics

Module Name :  exectu.c
SCCS ID     :  1.7

Current Date:  6/20/91, 10:14:44
Newest Delta:  1/19/90, 16:48:47

Function called by both the Hardware Exercise, Manufacturing Application,
and Diagnostic Application to invoke a Test Unit (TU).

If the "mfg" member of the struct within TUTYPE is set to INVOKED_BY_HTX,
then exectu() is being invoked by the HTX Hardware Exerciser so
test units know to look at variables in TUTYPE for values from the
rule file.  Else, the test units use pre-defined values while testing.

Note that test unit tu001() needs to be successfully called before
the other test units as it performs an actual START of the device
and gets the network address being used by the card.

*****************************************************************************/
#include <stdio.h>
#include "toktst.h"	/* note that this also includes hxihtx.h */

int exectu (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	register i, loop, tu;
	static int card_started = 0;
	static int rc;
	TUTYPE tmp_tucb;
	extern int mktu_rc();
	extern int tu001();
	extern int tu002();
	extern int tu003();
	extern int tu007();
	extern int tu008();

	/*
	 * Make copy of tucb_ptr and set up with tu number to perform a HALT.
	 * We do this in case a TU fails.  We can call tu003(), if necessary,
	 * to try a HALT so we can close the adapter cleanly.
	 */
	tmp_tucb = *tucb_ptr;
	tmp_tucb.header.tu = 3;
	tmp_tucb.token_s.htx_sp = NULL;

	tu = tucb_ptr->header.tu;
	loop = tucb_ptr->header.loop;
	if (tucb_ptr->header.mfg != INVOKED_BY_HTX)
		tucb_ptr->token_s.htx_sp = NULL;
	for (i = 0; i < loop; i++)
	   {
		switch(tu)
		   {
			case  1:
				rc = tu001(fdes, tucb_ptr);
				if (!rc)
					card_started = 1;
				else
					card_started = 0;
				break;

			case  2:  
				if (!card_started)
				   {
					/*
					 * set up return code for
					 * device not started....
					 */
					rc = mktu_rc(tu, TOK_ERR,
						ADAP_NOPEN_ERR);
					break;
				   }
				rc = tu002(fdes, tucb_ptr);
				break;

			case  3:
				if (!card_started)
				   {
					/*
					 * set up return code for
					 * device not started....
					 */
					rc = mktu_rc(tu, TOK_ERR,
						ADAP_NOPEN_ERR);
					break;
				   }
				rc = tu003(fdes, tucb_ptr);
				if (!rc)
					card_started = 0;
				break;

#if 0
			case  4:
				rc = tu004(fdes, tucb_ptr);
				break;

			case  5:
				if (!card_started)
				   {
					/*
					 * set up return code for
					 * device not started....
					 */
					rc = mktu_rc(tu, TOK_ERR,
						ADAP_NOPEN_ERR);
					break;
				   }
				rc = tu005(fdes, tucb_ptr);
				break;
#endif /* end if-zero */

			case  7:
				if (card_started)
				   {
					/*
					 * set up return code for
					 * device not started....
					 */
					rc = mktu_rc(tu, TOK_ERR,
						ADAP_OPN_ERR);
					break;
				   }
				rc = tu007(fdes, tucb_ptr);
				break;
			case  8:
				if (card_started)
				   {
					/*
					 * set up return code for
					 * device not started....
					 */
					rc = mktu_rc(tu, TOK_ERR,
						ADAP_OPN_ERR);
					break;
				   }
				rc = tu008(fdes, tucb_ptr);
				break;
#if 0
			case  6:  rc = tu006(fdes, tucb_ptr);
				  break;
			case  9:  rc = tu009(fdes, tucb_ptr);
				  break;
			case 10:  rc = tu010(fdes, tucb_ptr);
				  break;
			case 11:  rc = tu011(fdes, tucb_ptr);
				  break;
#endif
			default:
				return(-1);
		   };
		
		if (rc)
		   {
			/*
			 * if running manuf. diagnostic and a tu returns
			 * an error, then break out and return.
			 */
			if (tucb_ptr->header.mfg != INVOKED_BY_HTX)
				break;
			
			/*
			 * card failed, so halt everything, break and return
			 */
			(void) ioctl(fdes, tmp_tucb);
			card_started = 0;

			/*
			 * check on retries keyword.  If set
			 * from rule file stanza, then
			 * continue loop to retry tu, else
			 * break out and return.
			if (!(*tucb_ptr->token_s.retries))
			 */
			break;
		   }
	   }
	return(rc);
   }
