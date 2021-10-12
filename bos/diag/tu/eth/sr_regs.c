static char sccsid[] = "src/bos/diag/tu/eth/sr_regs.c, tu_eth, bos411, 9428A410j 6/19/91 14:56:54";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: reg_save, reg_restore
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

Function(s) Save/Restore ALL registers (POS and HIO)

Module Name :  sr_regs.c
SCCS ID     :  1.8

Current Date:  6/13/91, 13:11:25
Newest Delta:  1/19/90, 16:29:01

*****************************************************************************/
#include <stdio.h>
#include "ethtst.h"

#ifdef debugg
extern void detrace();
#endif
/*****************************************************************************

reg_save

Function saves off POS registers 2 through 7
RAM Page, Control, and Command) in the passed-in unsigned char array,
"save_area".

IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to read of specific register

*****************************************************************************/

int reg_save (fdes, save_area, status, tucb_ptr)
   int fdes;
   unsigned char save_area[];
   unsigned char *status;
   TUTYPE *tucb_ptr;
   {
	int rc, i;
	unsigned char p0;
	extern int pos_save();
	extern unsigned char hio_parity_rd();
	extern unsigned char hio_rampg_rd();
	extern unsigned char hio_ctrl_rd();
	extern unsigned char hio_cmd_rd();

	if (rc = pos_save(fdes, save_area, status, tucb_ptr))
		return(rc);
	
	i = 6;
	p0 = hio_parity_rd(fdes, status, tucb_ptr);
	if (*status)
		return(RPAR_RD_ERR);
#ifdef debugg
	detrace(0,"reg_save:  parity is 0x%02x\n", p0);
#endif
	save_area[i++] = p0;
	p0 = hio_rampg_rd(fdes, status, tucb_ptr);
	if (*status)
		return(RRAM_RD_ERR);
#ifdef debugg
	detrace(0,"reg_save:  rampg is 0x%02x\n", p0);
#endif
	save_area[i++] = p0;
	p0 = hio_ctrl_rd(fdes, status, tucb_ptr);
	if (*status)
		return(RCTL_RD_ERR);
#ifdef debugg
	detrace(0,"reg_save:  ctrl is 0x%02x\n", p0);
#endif
	save_area[i++] = p0;
	p0 = hio_cmd_rd(fdes, status, tucb_ptr);
	if (*status)
		return(RCMD_RD_ERR);
#ifdef debugg
	detrace(1,"reg_save:  cmd is 0x%02x\n", p0);
#endif
	save_area[i++] = p0;

	return(0);
   }

/*****************************************************************************

reg_restore

Function restores POS registers 2 through 7 AND the HIO registers (Parity,
RAM Page, Control, and Command) from the passed-in unsigned char array,
"save_area".

IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to write of specific register

*****************************************************************************/

int reg_restore (fdes, save_area, status, tucb_ptr)
   int fdes;
   unsigned char save_area[];
   unsigned char *status;
   TUTYPE *tucb_ptr;
   {
	int i;
	int rc;
	extern int pos_restore();
	extern int hio_parity_wr();
	extern int hio_rampg_wr();
	extern int hio_ctrl_wr();
	extern int hio_cmd_wr();

	if (rc = pos_restore(fdes, save_area, status, tucb_ptr))
		return(rc);

	i = 6;
	if (hio_parity_wr(fdes, save_area[i++], status, tucb_ptr))
		return(RPAR_WR_ERR);
	if (hio_rampg_wr(fdes, save_area[i++], status, tucb_ptr))
		return(RRAM_WR_ERR);
	if (hio_ctrl_wr(fdes, save_area[i++], status, tucb_ptr))
		return(RCTL_WR_ERR);
/*
 * don't enact a command to the command register - could cause
 * problems
 *

	if (hio_cmd_wr(fdes, save_area[i++], status, tucb_ptr))
		return(RCMD_WR_ERR);
 *
 */

	return(0);
   }
