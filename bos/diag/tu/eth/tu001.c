static char sccsid[] = "src/bos/diag/tu/eth/tu001.c, tu_eth, bos411, 9428A410j 6/19/91 14:59:04";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: pos_chk, tu001
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

Function(s) Test Unit 001 - POS Register Test

Module Name :  tu001.c
SCCS ID     :  1.12

Current Date:  12/12/90, 13:22:53
Newest Delta:  12/12/90, 13:21:02

Test writes/reads/compares byte values to check all bit positions
in writable POS registers (2-7).

NOTE the "cpp" macro, "HSV".  This tag may be defined (-DHSV) at compile
time for the HSV group to slow down testing so that they may 
verify the POS register access.

*****************************************************************************/
#include <stdio.h>
#include <errno.h>
#include "ethtst.h"

#ifdef debugg
extern void detrace();
#endif

/*****************************************************************************

pos_chk

Disables the adapter and writes/reads/compares all bit positions to
POS registers 2 through 7.

IF successful 
THEN RETURNs 0
ELSE RETURNs error code relating to read/write/compare problem with a POS

*****************************************************************************/

int pos_chk (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	unsigned long status;
	unsigned char p0;
	unsigned char pos_save_area[8];
	int rc;
	unsigned char i, test_val;
	unsigned char pos_mask;
	struct htx_data *htx_sp;
        static unsigned char tval[] = { 0xaa, 0x55, 0xff, 0x00 };

	extern int pos_wr();
	extern unsigned char pos_rd();
	extern int pos_save();
	extern int pos_restore();
	extern int mktu_rc();

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->eth_htx_s.htx_sp;

	/*
	 * save off all the POS reg original values before writing...
	 */
	if (rc = pos_save(fdes, pos_save_area, &status, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
	   }

	/*
	 * disable the card by writing to POS 2.
	 */
	p0 = pos_rd(fdes, 2, &status, tucb_ptr);
	if (status)
	  {
#ifdef debugg
		detrace(0,"Unable to read POS2 for initial CDEN-.\n");
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
			POS2_RD_ERR));
	  }
	p0 &= 0xfe;
	if (pos_wr(fdes, 2, p0, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"pos write of POS2 with 0x%02x failed!\n",p0);
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS2_WR_ERR));
	   }
	
	/*
	 * make sure POS0 and POS1 (non-writable) have legal values.
	 */
	p0 = pos_rd(fdes, 0, &status, tucb_ptr);
	if (status)
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS0_RD_ERR));
	   }
	if (p0 != INTPOS0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS0_CMP_ERR));
	   }
	p0 = pos_rd(fdes, 1, &status, tucb_ptr);
	if (status)
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS1_RD_ERR));
	   }
	if (p0 != INTPOS1)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS1_CMP_ERR));
	   }
	

	/*
	 * try to write 0x55, 0xAA, 0xFF, 0x00 in pos regs. 2 through 7.
	 */
	
#ifdef debugg
	detrace(0,"Testing POS2\n");
#endif

	for (i = 0x00; i < 0x04; i++)
	   {
		/*
		 * mask out card enable to insure that
		 * card remains disabled.
		 */
		test_val = tval[i] & 0xfe;

		if (pos_wr(fdes, 2, test_val, &status, tucb_ptr))
		   {
#ifdef debugg
			detrace(0,"Failed to write a %x to POS2\n", test_val);
#endif
			(void) pos_restore(fdes, pos_save_area,
							&status, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
						POS2_WR_ERR));
		   }
#ifdef HSV
		sleep(2);
#endif
		p0 = pos_rd(fdes, 2, &status, tucb_ptr);
		if (status)
		   {
#ifdef debugg
			detrace(0,"Read failed on POS2\n");
#endif
			(void) pos_restore(fdes, pos_save_area,
							&status, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
						POS2_RD_ERR));
		   }
		if ((p0 & 0x7f) != (test_val & 0x7f))
		   {
#ifdef debugg
			detrace(0,"Values for POS2 didn't match.\n");
#endif
			if (htx_sp != NULL)
				(htx_sp->bad_others)++;
			(void) pos_restore(fdes, pos_save_area,
							&status, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
						POS2_CMP_ERR));
		   }
	   }
#ifdef debugg
	detrace(0,"Testing POS3\n");
#endif
	/*
	 * POS3
	 */
	for (i = 0x00; i < 0x04; i++)
	   {
		test_val = tval[i];
		if (pos_wr(fdes, 3, test_val, &status, tucb_ptr))
		   {
#ifdef debugg
			detrace(0,"Failed to write a %x to POS3\n", test_val);
#endif
			(void) pos_restore(fdes, pos_save_area,
							&status, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
					POS3_WR_ERR));
		   }
#ifdef HSV
		sleep(2);
#endif
		p0 = pos_rd(fdes, 3, &status, tucb_ptr);
		if (status)
		   {
#ifdef debugg
			detrace(0,"Read failed on POS3\n");
#endif
			(void) pos_restore(fdes, pos_save_area,
							&status, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
						POS3_RD_ERR));
		   }
		if ((p0 & 0x1f) != (test_val & 0x1f))
		   {
#ifdef debugg
			detrace(0,"Values for POS3 didn't match. p0 = %x test_val = %x\n",p0,test_val);
#endif
			if (htx_sp != NULL)
				(htx_sp->bad_others)++;
			(void) pos_restore(fdes, pos_save_area,
							&status, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
						POS3_CMP_ERR));
		   }
	   }
#ifdef debugg
	detrace(0,"Testing POS4\n");
#endif
	/*
	 * POS4
	 */
	for (i = 0x00; i < 0x04; i++)
	   {
		test_val = tval[i];
		if (pos_wr(fdes, 4, test_val, &status, tucb_ptr))
		   {
#ifdef debugg
			detrace(0,"Failed to write a %x to POS4\n", test_val);
#endif
			(void) pos_restore(fdes, pos_save_area,
							&status, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
					POS4_WR_ERR));
		   }
#ifdef HSV
		sleep(2);
#endif
		p0 = pos_rd(fdes, 4, &status, tucb_ptr);
		if (status)
		   {
#ifdef debugg
			detrace(0,"Read failed on POS4\n");
#endif
			(void) pos_restore(fdes, pos_save_area,
							&status, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
					POS4_RD_ERR));
		   }

		/*
		 * Form mask for reserved bits and don't cares.
		 * If not PS3 card, then we don't want to
		 * check the FDBKINTEN, ADPTPAREN
		 */
		if (tucb_ptr->eth_htx_s.adapter_type != PS3)
			pos_mask = 0x7f;
		else pos_mask = 0xd3;

		if ((p0 & pos_mask) != (test_val & pos_mask))
		   {
#ifdef debugg
			detrace(0,"Values for POS4 didn't match.\n");
			detrace(0,"P0=%02X POS_MASK=%02X TEST_VAL=%02X\n",
			p0,pos_mask,test_val);
#endif
			if (htx_sp != NULL)
				(htx_sp->bad_others)++;
			(void) pos_restore(fdes, pos_save_area,
							&status, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
					POS4_CMP_ERR));
		   }
	   }
#ifdef debugg
	detrace(0,"Testing POS5\n");
#endif
	/*
	 * POS5
	 */
	for (i = 0x00; i < 0x04; i++)
	   {
		test_val = tval[i] | 0x40;
		if (pos_wr(fdes, 5, test_val, &status, tucb_ptr))
		   {
#ifdef debugg
			detrace(0,"Failed to write a %x to POS5\n", test_val);
#endif
			(void) pos_restore(fdes, pos_save_area,
							&status, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				POS5_WR_ERR));
		   }
#ifdef HSV
		sleep(2);
#endif
		p0 = pos_rd(fdes, 5, &status, tucb_ptr);
		if (status)
		   {
#ifdef debugg
			detrace(0,"Read failed on POS5\n");
#endif
			(void) pos_restore(fdes, pos_save_area,
							&status, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				POS5_RD_ERR));
		   }
		if (p0 != test_val)
		   {
#ifdef debugg
			detrace(0,"Values for POS5 didn't match.\n");
#endif
			if (htx_sp != NULL)
				(htx_sp->bad_others)++;
			(void) pos_restore(fdes, pos_save_area,
							&status, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				POS5_CMP_ERR));
		   }
	   }
	
#ifdef debugg
	detrace(0,"Testing POS6\n");
#endif
	/*
	 * POS6
	 */
	for (i = 0x00; i < 0x04; i++)
	   {
		test_val = tval[i];
		if (pos_wr(fdes, 6, test_val, &status, tucb_ptr))
		   {
#ifdef debugg
			detrace(0,"Failed to write a %x to POS6\n", test_val);
#endif
			(void) pos_restore(fdes, pos_save_area,
							&status, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				POS6_WR_ERR));
		   }
#ifdef HSV
		sleep(2);
#endif
		p0 = pos_rd(fdes, 6, &status, tucb_ptr);
		if (status)
		   {
#ifdef debugg
			detrace(0,"Read failed on POS6\n");
#endif
			(void) pos_restore(fdes, pos_save_area,
							&status, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				POS6_RD_ERR));
		   }
		if (p0 != test_val)
		   {
#ifdef debugg
			detrace(0,"Values for POS6 didn't match.\n");
#endif
			if (htx_sp != NULL)
				(htx_sp->bad_others)++;
			(void) pos_restore(fdes, pos_save_area,
							&status, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				POS6_CMP_ERR));
		   }
	   }
#ifdef debugg
	detrace(0,"Testing POS7\n");
#endif
	/*
	 * POS7
	 */
	for (i = 0x00; i < 0x04; i++)
	   {
		test_val = tval[i];
		if (pos_wr(fdes, 7, test_val, &status, tucb_ptr))
		   {
#ifdef debugg
			detrace(0,"Failed to write a %x to POS7\n", test_val);
#endif
			(void) pos_restore(fdes, pos_save_area,
							&status, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
					POS7_WR_ERR));
		   }
#ifdef HSV
		sleep(2);
#endif
		p0 = pos_rd(fdes, 7, &status, tucb_ptr);
		if (status)
		   {
#ifdef debugg
			detrace(0,"Read failed on POS7\n");
#endif
			(void) pos_restore(fdes, pos_save_area,
							&status, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
					POS7_RD_ERR));
		   }
		if (p0 != test_val)
		   {
#ifdef debugg
			detrace(0,"Values for POS7 didn't match.\n");
#endif
			if (htx_sp != NULL)
				(htx_sp->bad_others)++;
			(void) pos_restore(fdes, pos_save_area,
							&status, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
					POS7_CMP_ERR));
		   }
	   }


	/*
	 * prior to returning with card enabled, restore
	 * original POS reg vals.
	 */
	if (rc = pos_restore(fdes, pos_save_area, &status, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
	   }

	/*
	 * NOW, insure that card is enabled.
	 */
	p0 = pos_rd(fdes, 2, &status, tucb_ptr);
	if (status)
	  {
#ifdef debugg
		detrace(0,"Unable to read POS2 for final restore.\n");
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
			POS2_RD_ERR));
	  }
	p0 = p0 | 0x01;
	if (pos_wr(fdes, 2, p0, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"Unable to write POS2 for final restore.\n");
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
			POS2_WR_ERR));
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
	return(pos_chk(fdes, tucb_ptr));
   }
