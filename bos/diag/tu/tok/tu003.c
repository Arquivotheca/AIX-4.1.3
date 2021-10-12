static char sccsid[] = "src/bos/diag/tu/tok/tu003.c, tu_tok, bos411, 9428A410j 5/8/92 14:21:24";
/*
 * COMPONENT_NAME: (TOKENTU) Token Test Unit
 *
 * FUNCTIONS: tu003
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

Function(s) Test Unit 003 - Close Adapter

This test unit will close the session previously STARTed on the adapter.

*****************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/tokuser.h>

#include "toktst.h"

#define HALT_ATTEMPTS      16
#define SLEEP_INTERVAL      1
#define START_ASYNC_ATTEMPTS  5

#ifdef debugg 
extern void detrace();
#endif 

int tu003 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	struct session_blk halt_s;
	struct status_block status_s;
	int cc, uncode;

	struct htx_data *htx_sp;
	extern int errno;
	extern int tr_stat();
	extern int mktu_rc();
	extern int tr_uncode();

#ifdef debugg
	detrace(0,"\n\ntu003:  Begins...\n");
#endif
	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->token_s.htx_sp;

	/*
	 * set up the netid for the session with a default
	 * as specified by TU_NET_ID (defined in toktst.h) and
	 * then start device.
	 */
	halt_s.netid = TU_NET_ID;
	cc = ioctl(fdes, CIO_HALT, &halt_s);
	if (cc)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		
		if (errno == EIO)
		   {
			cc = tr_stat(fdes, &status_s, htx_sp, TOK_ADAP_CHECK,
					1, 0);
			if (cc < 0)
				return(mktu_rc(tucb_ptr->header.tu,
					LOG_ERR, TOK_HA1_ERR));
			
			uncode = tr_uncode(&status_s);
			if (uncode < 0)
				return(mktu_rc(tucb_ptr->header.tu,
					LOG_ERR, TOK_HA1_ERR));

			return(mktu_rc(tucb_ptr->header.tu, TOK_ERR, uncode));
		   }
		return(mktu_rc(tucb_ptr->header.tu, SYS_ERR, errno));
	   }

#ifdef debugg
	detrace(0,"tu003:  SKIPPING status for HALT DONE\n");
	detrace(0,"tu003:  since dd doesn't support, yet.\n");
#endif


#ifdef debugg
	detrace(0,"tu003:  Before get status for HALT DONE\n");
#endif
	/*
	 * perform loop (inside tr_stat) on get status 'til receive
	 * notification of completion of HALT.  If a non-zero is
	 * returned then got an ioctl error or we never saw a
	 * CIO_HALT_DONE so report what I did get instead.
	 */
	if (cc = tr_stat(fdes, &status_s, htx_sp, CIO_HALT_DONE,
		HALT_ATTEMPTS, SLEEP_INTERVAL))
	   {
#ifdef debugg
		detrace(0,"tu003:  tr_stat while waiting for HALT retn'd %d\n",
				cc);
		if (cc < 0)
			detrace(0, "tu003:  Status code hex:  %x, errno %d\n",
					status_s.code, errno);
#endif
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;

		if (cc < 0)
			/*
			 * the tr_stat failed on its own ioctl
			 * so make the return code with the errno
			 * produced from within tr_stat that was sent
			 * back inside status_s.code.
			 */
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				HA1_DONE_ERR));
		else
		   {
			/*
			 * never got a CIO_HALT_DONE, so lookup
			 * the adapter error in the status structure
			 * to make the return value.
			 */
			uncode = tr_uncode(&status_s);
			if (uncode < 0)
				return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
					HA1_DONE_ERR));

			return(mktu_rc(tucb_ptr->header.tu, TOK_ERR,
				uncode));
		   }
	   }
	
	/*
	 * okay, we did get a CIO_HALT_DONE, but we need to check
	 * that it was a successful completion.
	 */
	if (status_s.option[0] != CIO_OK)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		/*
		 * we got an UNsuccessful halt completion so
		 * grab adapter error code to make return code.
		 */
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
			NETID_ERR));
	   }

	/* Check Device Driver Asynch Status...
	 */

	cc = tr_stat(fdes, &status_s, htx_sp, CIO_ASYNC_STATUS,
		START_ASYNC_ATTEMPTS, SLEEP_INTERVAL);

	if (cc == 1)
	   {
		if (htx_sp != NULL)	   
	      		(htx_sp->bad_others)--;
	   }	
	if (cc == 0)
	   {

#ifdef debugg
	      detrace(0,"tu003: We got a CIO_ASYNC_STATUS \n);
#endif

	     uncode = tr_uncode(&status_s);
	     if (uncode < 0)
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
			HA1_DONE_ERR));
	     return(mktu_rc(tucb_ptr->header.tu, TOK_ERR,
			uncode));
	  }
	
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	return(PASSED);
   }
