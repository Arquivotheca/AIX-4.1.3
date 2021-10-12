static char sccsid[] = "src/bos/diag/tu/dca/exectu.c, tu_tca, bos411, 9428A410j 6/19/91 15:30:17";
/*
 * COMPONENT_NAME: (TU_TCA) TCA/DCA Test Unit
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

#include <stdio.h>
#include "tcatst.h"	/* note that this also includes hxihtx.h */

/*
 * NAME: exectu
 *
 * FUNCTION: Invoke the Test Units
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) Function called by both the Hardware Exercise, Manufacturing 
 *          Application, and Diagnostic Application to invoke a Test Unit (TU).
 *
 *          If the "mfg" member of the struct within TUTYPE is set to 
 *          INVOKED_BY_HTX, then exectu() is being invoked by the HTX Hardware
 *          Exerciser so test units know to look at variables in TUTYPE for 
 *          values from the rule file.  Else, the test units use pre-defined 
 *          values while testing.
 *
 * RETURNS: A zero (0) if successful or non-zero on error condition
 *
 */
int exectu (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	register i, loop, tu;
	static int rc;
	extern int mktu_rc();


	tu = tucb_ptr->header.tu;
	loop = tucb_ptr->header.loop;
	if (tucb_ptr->header.mfg != INVOKED_BY_HTX)
		tucb_ptr->tca_htx_s.htx_sp = NULL;
	for (i = 0; i < loop; i++)
	   {
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


			default:
				return(-1);
		   };
		
		if (rc)
		   {
			/* If running manuf. diagnostic and a tu returns */
			/* an error, then break out and return.		 */
			if (tucb_ptr->header.mfg != INVOKED_BY_HTX)
				break;
			/* Check on retries keyword.  If set from rule   */
			/* file stanza, then continue loop to retry TU   */
			/* else break out and return.			 */
			if (!(*tucb_ptr->tca_htx_s.retries))
				break;
		   }
	   }
	return(rc);
   }
