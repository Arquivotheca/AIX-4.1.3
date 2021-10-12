static char sccsid[] = "src/bos/diag/tu/tok/tu007.c, tu_tok, bos411, 9428A410j 6/20/91 12:41:58";
/*
 * COMPONENT_NAME: (TU_TOK) Token Test Unit
 *
 * FUNCTIONS: tu007
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

Function(s) Test Unit 007 - Change Ring Speed to 4MB rate

Module Name :  tu007.c
SCCS ID     :  1.9

Current Date:  6/20/91, 10:14:48
Newest Delta:  2/27/90, 17:39:38

Test unit alters ring speed on token card by writing to POS register.

*****************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/tokuser.h>

#include "toktst.h"

#define POS_SPEED_REG     0x03
#define RING_4MB_MASK     0x00

#ifdef debugg
extern void detrace();
#endif

int tu007 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int cc;
	unsigned char pos_val;
	extern int write_pos();
	extern int read_pos();
	extern int mktu_rc();

#ifdef debugg
	detrace(0,"\n\ntu007:  Begins...\n");
#endif

	pos_val = 0;
	if (cc = read_pos(fdes, POS_SPEED_REG, &pos_val, tucb_ptr))
		return(cc);
	pos_val |= RING_4MB_MASK;

	if (cc = write_pos(fdes, POS_SPEED_REG, pos_val, tucb_ptr))
		return(cc);
#ifdef debugg
	detrace(0, "tu007:  POS %d = %02x (hex)\n",
		POS_SPEED_REG, RING_4MB);
#endif
	
	pos_val = 0;
	if (cc = read_pos(fdes, POS_SPEED_REG, &pos_val, tucb_ptr))
		return(cc);
	
	if ((pos_val & RING_4MB_MASK) != RING_4MB_MASK)
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
			POS_CMP_ERR + POS_SPEED_REG));

	return(PASSED);
   }
