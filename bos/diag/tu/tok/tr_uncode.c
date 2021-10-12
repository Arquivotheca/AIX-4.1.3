static char sccsid[] = "src/bos/diag/tu/tok/tr_uncode.c, tu_tok, bos411, 9428A410j 6/20/91 12:41:42";
/*
 * COMPONENT_NAME: (TU_TOK) Token Test Unit
 *
 * FUNCTIONS: tr_uncode
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

Function(s) Token Ring Unsuccessful Return Code Value

Module Name :  tr_uncode.c
SCCS ID     :  1.8

Current Date:  6/20/91, 10:14:46
Newest Delta:  4/23/90, 15:14:51

Function looks at code type from GET STATUS and then extracts
the 2-byte code returned from the adapter in the array of
options within the session_blk structure.

IF successful
THEN RETURNs unsuccessful status code
ELSE RETURNs -1

*****************************************************************************/
#include <stdio.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/tokuser.h>
#include "toktst.h"

#define LOW_MASK    0x0000ffff

#ifdef debugg
extern void detrace();
#endif

int tr_uncode (status_sp)
   struct status_block *status_sp;
   {
	static int rc;

#ifdef debugg
detrace(0,"tr_uncode:\n");
detrace(0,"          code = 0x%08x\n", status_sp->code);
detrace(0,"      option 0 = 0x%08x\n", status_sp->option[0]);
detrace(0,"      option 1 = 0x%08x\n", status_sp->option[1]);
detrace(0,"      option 2 = 0x%08x\n", status_sp->option[2]);
detrace(1,"      option 3 = 0x%08x\n", status_sp->option[3]);
#endif

	switch(status_sp->code)
	   {
/*
 * CIO_LOST_BLK renamed to CIO_LOST_STATUS
 */
		case CIO_LOST_STATUS:
				rc = -1;
				break;

		case CIO_NULL_BLK:
				rc = -1;
				break;

		case CIO_ASYNC_STATUS:
				switch(status_sp->option[0])
				   {
					case CIO_HARD_FAIL:
					case CIO_NET_RCVRY_ENTER:
						if (status_sp->option[1] ==
								TOK_MC_ERROR)
						   rc = ASY_CC_ERR;
						else
						   rc = status_sp->option[2] &
								LOW_MASK;
						break;
					case CIO_NET_RCVRY_EXIT:
						rc = 0;
						break;
					default:
						rc = -1;
						break;
				   }
				break;
		
		case CIO_START_DONE:
				switch(status_sp->option[0])
				   {
					case CIO_TIMEOUT:
						rc = OPN_TIM_ERR;
						break;
					case TOK_ADAP_CONFIG:
						rc = OPN_PARM_ERR;
						break;
					case TOK_ADAP_INIT_PARMS_FAIL:
						rc = INIT_CMP_ERR;
						break;
					case TOK_ADAP_INIT_TIMEOUT:
						rc = INIT_TIME_ERR;
						break;
					default:
						rc = status_sp->option[2] &
								LOW_MASK;
						break;
				   }
				break;
		
		case CIO_HALT_DONE:
				rc = NETID_ERR;
				break;
		
		case CIO_TX_DONE:
				rc = status_sp->option[3] >> 16;
				break;
		
#if 0
		case TOK_START_MAC_DONE:
				rc = status_sp->option[1] & LOW_MASK;
				break;
		
		case TOK_HALT_MAC_DONE:
				rc = NETID_ERR;
				break;
		
		case TOK_RING_STATUS:
				rc = status_sp->option[1] & LOW_MASK;
				break;
#endif
			
		case TOK_ADAP_CHECK:
				rc = status_sp->option[1] & LOW_MASK;
				break;
		
		default:
				rc = -1;
				break;
	   }
	return(rc);
   }
