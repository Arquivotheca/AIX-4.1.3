static char sccsid[] = "src/bos/diag/tu/tok/tr_stat.c, tu_tok, bos411, 9428A410j 6/20/91 12:41:27";
/*
 * COMPONENT_NAME: (TU_TOK) Token Test Unit
 *
 * FUNCTIONS: tr_stat
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

Function(s) Token Ring Get Status

Module Name :  tr_stat.c
SCCS ID     :  1.7

Current Date:  6/20/91, 10:14:46
Newest Delta:  1/19/90, 16:49:49

Function loops on a ioctl(CIO_GET_STAT) for polling of an expected
status code resulting from a previous operation.  "htx_sp" is passed
in for incrementing good/bad ioctl() calls in the event the
program is running from HTX.

IF successful
THEN RETURNs 0 along with status inside status_sp
ELSE IF ioctl error
     THEN RETURNs -1 with status_sp->code set with errno
     ELSE IF max attempts reached
	  THEN RETURNs 1 with status_sp->code set with 0

*****************************************************************************/
#include <stdio.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/tokuser.h>
#include "hxihtx.h"

#define CODE_MASK   0x0000ffff	/*
				 * for pulling status code - 
				 * additional comments below in usage.
				 */

int tr_stat (fdes, status_sp, htx_sp, expected_code, max_attempts, sleep_time)
   int fdes;
   struct status_block *status_sp;
   struct htx_data *htx_sp;
   unsigned long expected_code;
   int max_attempts;
   int sleep_time;
   {
	register int i;
	static struct status_block first_status;
	extern int errno;

	first_status.code = CIO_NULL_BLK;
	for (i = 0; i < max_attempts; i++)
	   {
		if (ioctl(fdes, CIO_GET_STAT, status_sp) < 0)
		   {
			status_sp->code = (unsigned long) errno;
			if (htx_sp != NULL)
				(htx_sp->bad_others)++;
			return(-1);
		   }
		
		/*
		 * the definition of the status_block in comio.h (ch. 5)
		 * specify that the 16 most-significant bits hold the
		 * device identifier with the remaining 16 bits holding
		 * the actual status code (hence, the bit-AND-operation).
		 */
		if ((status_sp->code & CODE_MASK) == expected_code)
			break;
		else
		   {
			/*
			 * save off first NON NULL nor LOST status returned
			 * if we didn't get the expected code.
			 */
			if ((first_status.code == CIO_NULL_BLK) &&
			    (status_sp->code != CIO_LOST_STATUS) &&
			    (status_sp->code != CIO_NULL_BLK))
			   {
				first_status = *status_sp;
			   }
		   }
		
		if (sleep_time)
			sleep(sleep_time);
	   }
	if (i == max_attempts)
	   {
		/*
		 * never got expected code, so send
		 * back the first one we got instead...
		 */
		status_sp = &first_status;
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(1);
	   }

	if (htx_sp != NULL)
		(htx_sp->good_others)++;
	return(0);
   }
