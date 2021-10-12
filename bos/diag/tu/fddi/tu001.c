static char sccsid[] = "@(#)52	1.1  src/bos/diag/tu/fddi/tu001.c, tu_fddi, bos411, 9428A410j 7/9/91 12:43:08";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: dd_mode
 *		tu001
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*****************************************************************************

Function(s) Test Unit 001 - Diagnostic/Download Mode.  

Module Name :  tu001.c
SCCS ID     :  1.11

Current Date:  5/23/90, 11:17:52
Newest Delta:  4/10/90, 18:48:20

This test places the adapter in diagnostic/download (D/D) mode. This is 
accomplished by writing a value of 1 to bit 4 in POS register 2.

NOTE the "cpp" macro, "HSV".  This tag may be defined (-DHSV) at compile
time for the HSV group to slow down testing so that they may 
verify the POS register access.

The machine device driver will be used to access the POS registers.

*****************************************************************************/
#include <stdio.h>
#include <errno.h>
#include "fdditst.h"

#ifdef debugg
extern void detrace();
#endif


/*****************************************************************************

dd_mode
This routine will read POS 2, set bit 4 of POS 2, and reread POS 2 to 
confirm this occured correctly. 

*****************************************************************************/

int dd_mode(fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int		mode;
	unsigned char	p0[2];
	unsigned long	status;
	unsigned char   *data;
	struct htx_data *htx_sp;

	extern int pos_wr();
	extern unsigned char pos_rd();

	/*
	 * Set up a pointer to HTX data structure to increment
	 * counters in case TU was invoked by hardware exerciser.
	 */
	htx_sp = tucb_ptr->fddi_s.htx_sp;

	/*
	 * first, we read POS register 2.
	 */

	p0[0] = pos_rd(2, &status, tucb_ptr);
	if (status)
	   {
#ifdef debugg
		detrace(0,"Initial POS2 read failed. POS2 = %x\n", p0[0]);
		detrace(0,"POS2 read returned status = %x\n", status);
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS2_RD_ERR));
	   }

	/*
	 * Set the Download/Diagnostic bit to 1 and clear out
	 * the Reset bit.
	 */


	p0[0] = p0[0] | MASK_POS2_SET_D_D;
	p0[0] = p0[0] & MASK_POS2_CLEAR_RESET;

#ifdef debugg
		detrace(0,"Setting of Download/Diagnostic bit.");
		detrace(0,"POS2 = %x\n", p0[0]);
#endif
	if (pos_wr(2, p0, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"Setting of Download/Diagnostic bit failed.");
		detrace(0,"POS2 = %x\n", p0[0]);
		detrace(0,"POS2 write returned status = %x\n", status);
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, status));
	   }
	
	/*
	 * Check the new value by rereading POS register 2.
	 */

	p0[0] = pos_rd(2, &status, tucb_ptr);

	if (status)
	   {
#ifdef debugg
		detrace(0,"2nd read of POS2 failed. POS2 = %x\n", p0[0]);
		detrace(0,"POS2 read returned status = %x\n", status);
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS2_RD_ERR));
	   }

	/*
	 * Check to see if Diagnostic/Download bit in POS2 is set.
	 */
	if (!(p0[0] & MASK_POS2_SET_D_D))
	   {
#ifdef debugg
		detrace(0,"D/D bit not set. POS2 = %x\n", p0[0]);
#endif
		if (htx_sp != NULL)
		   (htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS2_CMP_ERR));
	   }

	return(0);
   }



/*****************************************************************************

tu001

*****************************************************************************/

int tu001 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int	rc;

	rc = dd_mode(fdes, tucb_ptr);
	return(rc);
   }
