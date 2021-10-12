static char sccsid[] = "@(#)37  1.2.1.2  src/bos/diag/tu/artic/tu008.c, tu_artic, bos411, 9428A410j 8/19/93 17:49:44";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu008
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include "artictst.h"

/*
 * NAME: tu008
 *
 * FUNCTION: SCC Test Unit
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This function will initiate the SCC test unit.  This is a more
 *          thorough test than the POST test.  This test will check both ports
 *          on the SSC.  It sets the interrupts - external, overrun, TX and
 *          RX.  It test asynchronous mode with the following configurations:
 *                  Even parity, 1 stop bit,    8 bits/char, 19200 baud
 *                  Odd parity,  1 stop bit,    8 bits/char, 9600 baud
 *                  No parity,   2 stop bits,   7 bits/char, 4800 baud
 *                  No parity,   2 stop bits,   6 bits/char, 1200 baud
 *                  No parity,   1.5 stop bits, 5 bits/char, 110 baud 
 *          It will also test the transmit procedure for SDLC and Bisynchronous
 *          protocols.  There is a prerequisite that TU019 be executed before
 *          executing this test unit.
 *
 * RETURNS: The return code from the SCC test.
 */
int tu008 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        extern unsigned char cardnum;
	extern int start_diag_tu();

	return(start_diag_tu(fdes, tucb_ptr, SCC_COM_CODE, SCC_ER));
   }
