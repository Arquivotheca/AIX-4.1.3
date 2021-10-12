static char sccsid[] = "@(#)66	1.3  src/bos/diag/tu/msla/mslaexectu.c, tu_msla, bos411, 9428A410j 7/10/90 11:49:10";
/*
 * COMPONENT_NAME: ( mslaexectu )
 *
 * FUNCTIONS:  	   postest, memtest, regtest, dgsftp19, dgsftp20 
 *		   msladmatest, int_rios, vpdtest
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


#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "mslatu.h"

/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     exectu  ()                                            */
/*                                                                           */
/* DESCRIPTIVE NAME = 							     */
/*                                                                           */
/* FUNCTION         =  Interface function of the HTX with the different TU's */
/*                                                                           */
/* INPUT            =   Pointer to the mslatu structure.                     */
/*                                                                           */
/* OUTPUT           =                                                        */
/*                                                                           */
/* FUNCTIONS CALLED =  postest, vpdtest,memtest, regtest, dgsftp20, 	     */
/*                     dgsftp19, hxfmsg.                                     */
/*                                                                           */
/* EXTERNAL REFERENCES = None                                                */
/*                                                                           */
/*****************************************************************************/

#define FAIL   -1

exectu(fd,gtu)
int   fd;
struct mslatu *gtu;
{
	int rc;

	/* Get the file descriptor */
	gtu->fd = fd;

	/*
   ********************************************
   *      Call selected TU                    *
   ********************************************
   */

	do {
		switch (gtu->header.tu)
		{
		case POSTEST:
			rc=postest(gtu);
			break;

		case VPDTEST:
			rc=vpdtest(gtu);
			break;

		case MEMTEST:
			rc=memtest( gtu) ;
			break;

		case REGTEST:
			rc=regtest( gtu);
			break;

		case FTP20TEST:
			rc= dgsftp20(gtu);
			break;

		case WRAPTEST:
			rc= dgsftp19(gtu);
			break;

		case DMATEST:
			rc= msladmatest(gtu);
			break;

		case INT_RIOS:
			rc= int_rios(gtu);
			break;

		default:
			rc = FAIL;
			break;

		}  /** end switch  **/
		if ( rc != 0 ) {
			break;
		}
	} while ( --gtu->header.loop > 0 );

	return(rc);

}
