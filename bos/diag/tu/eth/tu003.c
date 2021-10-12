static char sccsid[] = "src/bos/diag/tu/eth/tu003.c, tu_eth, bos411, 9428A410j 6/19/91 15:00:43";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: ram_tst, tu003
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

Function(s) Test Unit 003 - RAM Test

Module Name :  tu003.c
SCCS ID     :  1.11

Current Date:  12/13/90, 07:47:33
Newest Delta:  1/22/90, 13:26:38  


*****************************************************************************/
#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include "ethtst.h"

#define         POS0    0
#define         POS1    1
#define         POS2    2
#define         POS3    3
#define         POS4    4
#define         POS5    5
#define         POS6    6
#define         POS7    7

#define         HIORAMPG      0x8

#ifdef debugg
extern void detrace();
#endif

/*****************************************************************************

ram_tst

Function writes patterns of data to adapter RAM from offset specified
in RAM page register and reads back to compare.

*****************************************************************************/

int ram_tst (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	unsigned char subaddr102, subaddr103;
	unsigned char buf0[16], buf1[16], buf2[16];
	unsigned long stat;
	static unsigned char pattern[] =
	   {
               0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
               0X8, 0x9, 0xA, 0XB, 0XC, 0XD, 0XE, 0x0
	   };
	unsigned char pos2_val;
	int i;
	int rc;
	struct htx_data *htx_sp;

	extern int pos_wr();
	extern unsigned char pos_rd();
	extern int hio_wr();
	extern int smem_rd();
	extern int smem_wr();
	extern int mktu_rc();
#ifdef debugg
	unsigned char p0;
	extern int hard_reset();
	extern int init_pos();


	extern unsigned char hio_parity_rd();
	extern unsigned char hio_rampg_rd();
	extern unsigned char hio_ctrl_rd();
	extern unsigned char hio_cmd_rd();
#endif
	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->eth_htx_s.htx_sp;

#ifdef debugg
	int j;
#endif

#ifdef debugg
	detrace(0,"\n\n------------------- IN RAM Test -------------\n");
#endif

#ifdef debugg
	detrace(0,"ram_tst:  Read of POS registers shows...\n");
	for (i = 2; i < 8; i++)
		detrace(0,"ram_tst:  POS %2d = 0x%02x\n", i ,pos_rd(fdes, i, &stat, tucb_ptr));
	detrace(0,"\nram_tst:  HIO regs are....\n");
	detrace(0,"               parity 0x%02x\n", hio_parity_rd(fdes, &stat, tucb_ptr));
	detrace(0,"               rampg  0x%02x\n", hio_rampg_rd(fdes, &stat, tucb_ptr));
	detrace(0,"               ctrl   0x%02x\n", hio_ctrl_rd(fdes, &stat, tucb_ptr));
	detrace(0,"               cmd    0x%02x\n", hio_cmd_rd(fdes, &stat, tucb_ptr));
	detrace(1,"ram_tst:  GOT em'????\n");
#endif
	/*
	 * Form address 0x102 (dec. subaddress register 258)
	 * using POS registers 6 and 7 and then read the value there
	 * using POS register 3.
	 */
	if (rc = pos_wr(fdes, POS7, 0x1, &stat, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS7_WR_ERR));
	   }
	if (rc = pos_wr(fdes, POS6, 0x2, &stat, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS6_WR_ERR));
	   }
	subaddr102 = pos_rd(fdes, POS3, &stat, tucb_ptr);
	if (stat != 0)
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS3_RD_ERR));
	   }
#ifdef debugg
	detrace(0,"subaddr102 is %08X...Expecting 0x6.\n", subaddr102);
#endif
	/*
	 * The byte from subaddr102 should be 0x06 (set at init time)
	 * which correspond to Micro Channel bits 17-24.
	 */
	if (subaddr102 != 0x6)
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, BASE_ADD_ERR));
	   }
	
	/*
	 * let's grab the byte at sub address 0x103, but FIRST
	 * let's try writing zeroes to POS 7 and 6 and then
	 * forming the 0x103 there...
	 */
	if (rc = pos_wr(fdes, POS7, 0x0, &stat, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS7_WR_ERR));
	   }
	if (rc = pos_wr(fdes, POS6, 0x0, &stat, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS6_WR_ERR));
	   }
#ifdef debugg
	detrace(0,"ram_tst:  Okay, wrote zeroes to POS6 and 7.\n");
	detrace(0,"ram_tst:  Read of POS registers shows...\n");
	for (i = 2; i < 8; i++)
		detrace(0,"ram_tst:  POS %2d = 0x%02x\n", i ,pos_rd(fdes, i, &stat, tucb_ptr));
	detrace(1,"ram_tst:  Okay?  Ready?\n");
#endif

	/*
	 * Form address 0x103 (dec. subaddress register 259)
	 * using POS registers 6 and 7 and then read the value there
	 * using POS register 3.
	 */
	if (rc = pos_wr(fdes, POS7, 0x1, &stat, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS6_WR_ERR));
	   }
	if (rc = pos_wr(fdes, POS6, 0x3, &stat, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS6_WR_ERR));
	   }
#ifdef debugg
	detrace(0,"ram_tst:  Alright, we wrote 0x103 to POS 6 and 7\n");
	detrace(0,"ram_tst:  Let's sleep for a couple of secs.\n");
	detrace(0,"ram_tst:  Alright, go and read it!\n");
#endif
	subaddr103 = pos_rd(fdes, POS3, &stat, tucb_ptr);
	if (stat != 0)
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS3_WR_ERR));
	   }
	/*
	 * The byte from subaddr103 should be 0x00 (set at init time)
	 * which correspond to Micro Channel bits 25-31.  Note that
	 * since this range reflects 7 bits we don't need or care about
	 * the 8th (msb) bit so we make sure we apply a mask to zero it out.
	 */
	if (subaddr103 & 0x7F)
	   {
#ifdef debugg
	detrace(0,"ram_tst:  Read of POS registers AFTER FAILURE...\n");
	for (i = 2; i < 8; i++)
		detrace(0,"ram_tst:  POS %2d = 0x%02x\n", i ,pos_rd(fdes, i, &stat, tucb_ptr));
	detrace(0,"\nram_tst:  HIO regs are....\n");
	detrace(0,"               parity 0x%02x\n", hio_parity_rd(fdes, &stat, tucb_ptr));
	detrace(0,"               rampg  0x%02x\n", hio_rampg_rd(fdes, &stat, tucb_ptr));
	detrace(0,"               ctrl   0x%02x\n", hio_ctrl_rd(fdes, &stat, tucb_ptr));
	detrace(0,"               cmd    0x%02x\n", hio_cmd_rd(fdes, &stat, tucb_ptr));
	detrace(1,"ram_tst:  GOT em'????\n");
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, BASE_ADD2_ERR));
	   }

	/*
	 * Write to RAM Page Register so that the memory access page
	 * starts at location 0 of the adapter's memory space.
	 */
	if (rc = hio_wr(fdes, HIORAMPG, 0x0, &stat, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RRAM_WR_ERR));
	   }

#ifdef debugg
	detrace(0,"RAM test begins\n");
#endif

	/* ---- RAM access test begin ---- */

	/*
	 * Save F bytes starting at offset of zero into buf0.
	 */
	if (rc = smem_rd(fdes, 0, 0xF, buf0, &stat, tucb_ptr))
	   {
#ifdef debugg
	detrace(0,"tu003:  smem_rd failed reading at 0x0, 0xF bytes\n");
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, MEM_RD_ERR));
	   }
#ifdef debugg
	detrace(0,"Saved off F bytes starting at 0 into buf0.\n");
	detrace(0,"We saved: ");
	for (j = 0; j < 0xF; j++)
		detrace(0,"%02X ", *(buf0 + j));
	detrace(0,"\n");
#endif
	/*
	 * Save F bytes starting at offset 0x3FF0 into buf1.
	 */
	if (rc = smem_rd(fdes, 0x3FF0, 0xF, buf1, &stat, tucb_ptr))
	   {
#ifdef debugg
	detrace(0,"tu003:  smem_rd failed reading at 0x3FF0, 0xF bytes\n");
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, MEM_RD_ERR));
	   }
#ifdef debugg
	detrace(0,"Saved off F bytes starting at 0x3FF0 into buf1.\n");
	detrace(0,"We saved: ");
	for (j = 0; j < 0xF; j++)
		detrace(0,"%02X ", *(buf1 + j));
	detrace(0,"\n");
#endif
	/*
	 * Write F bytes of the pattern starting at offset 0.
	 */
	if (rc = smem_wr(fdes, 0, 0xF, pattern, &stat, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, MEM_WR_ERR));
	   }
#ifdef debugg
	detrace(0,"Wrote F bytes of pattern starting at 0.\n");
	detrace(0,"Pattern was: ");
	for (j = 0; j < 0xF; j++)
		detrace(0,"%02X ", *(pattern + j));
	detrace(0,"\n");
#endif
	if (rc = smem_wr(fdes, 0x3FF0, 0xF, pattern, &stat, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, MEM_WR_ERR));
	   }
#ifdef debugg
	detrace(0,"Wrote F bytes of pattern starting at x3FF0.\n");
	detrace(0,"Pattern was: ");
	for (j = 0; j < 0xF; j++)
		detrace(0,"%02X ", *(pattern + j));
	detrace(0,"\n");
#endif
	if (rc = smem_rd(fdes, 0, 0xF, buf2, &stat, tucb_ptr))
	   {
#ifdef debugg
	detrace(0,"tu003:  smem_rd failed RE-reading at 0x0, 0xF bytes\n");
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, MEM_RD_ERR));
	   }
#ifdef debugg
	detrace(0,"Read F bytes from 0 into buf2.\n");
	detrace(0,"buf2: ");
	for (j = 0; j < 0xF; j++)
		detrace(0,"%02X ", *(buf2 + j));
	detrace(0,"\n");
#endif
	if (memcmp(pattern, buf2, 0xF) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, MEM_CMP_ERR));
	   }
#ifdef debugg
	detrace(0,"Compared SUCCESSFULLY with original pattern!\n");
#endif
	if (rc = smem_rd(fdes, 0x3FF0, 0xF, buf2, &stat, tucb_ptr))
	   {
#ifdef debugg
	detrace(0,"tu003:  smem_rd failed RE-reading at 0x3FF0, 0xF bytes\n");
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, MEM_RD_ERR));
	   }
#ifdef debugg
	detrace(0,"Read F bytes from x3FF0 into buf2.\n");
	detrace(0,"buf2: ");
	for (j = 0; j < 0xF; j++)
		detrace(0,"%02X ", *(buf2 + j));
	detrace(0,"\n");
#endif
	if (memcmp(pattern, buf2, 0xF) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, MEM_CMP_ERR));
	   }
#ifdef debugg
	detrace(0,"Compared SUCCESSFULLY with original pattern!\n");
#endif
	/*
	 * Restore originally stored data back into the RAM from the bufs.
	 */
	if (rc = smem_wr(fdes, 0, 0xF, buf0, &stat, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, MEM_WR_ERR));
	   }
#ifdef debugg
	detrace(0,"Restored original F bytes into loc. 0 from buf0.\n");
#endif
	if (rc = smem_wr(fdes, 0x3FF0, 0xF, buf1, &stat, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, MEM_WR_ERR));
	   }
#ifdef debugg
	detrace(0,"Restored original F bytes into loc. x3FF0 from buf0.\n");
#endif

#ifdef debugg
	detrace(0,"\n\tRAM access test completed successfully!");
#endif

	return(0);
   }  /* end of ram_access_test */

/*****************************************************************************

tu003

*****************************************************************************/

int tu003 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	return(ram_tst(fdes, tucb_ptr));
   }
