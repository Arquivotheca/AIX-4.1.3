static char sccsid[] = "@(#)48	1.1  src/bos/diag/tu/fddi/rw_pos.c, tu_fddi, bos411, 9428A410j 7/9/91 12:41:47";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: pos_wr
 *		pos_rd
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

Function(s) Read/Write POS Register.  Access to all POS registers is done 
through the machine device driver.

Module Name :  rw_pos.c
SCCS ID     :  1.8

Current Date:  5/23/90, 11:17:48
Newest Delta:  1/19/90, 16:26:26

****************************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <sys/mdio.h>
#include <ctype.h>
#include "fdditst.h"

#ifdef debugg
extern void detrace();
#endif

 
/*****************************************************************************

pos_wr

*****************************************************************************/

int pos_wr (pos_reg, data, status, tucb_ptr)
   int pos_reg;
   unsigned char data[2];
   unsigned long *status;
   TUTYPE *tucb_ptr;
   {
	int bytes;
	int rc;
	int slot;
	char slot_ptr[2];
	struct htx_data *htx_sp;
	MACH_DD_IO iob;

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->fddi_s.htx_sp;

	slot_ptr[0] = tucb_ptr->slot;
	slot = atoi(slot_ptr) - 1;

	bytes = 1;
	iob.md_data = data;
	iob.md_incr = MV_BYTE;
	iob.md_size = (ulong) bytes;
	iob.md_addr = POSREG(pos_reg,slot);

	*status = 0;
    	rc = ioctl(tucb_ptr->mdd_fd,MIOCCPUT ,&iob);
    	if (rc != 0)
	{

#ifdef debugg
  detrace(0,"POS write ioctl returned = %x\n pos_reg = %x\n pos_val = %x\n opcode = %x\n ",rc,pos_reg,data[0],MIOCCPUT);
#endif
	   	if (htx_sp != NULL)
	   		(htx_sp->bad_others)++;
    		*status  = MDD_IOCTL_ERR;
	        return(MDD_IOCTL_ERR); 
	}
#ifdef debugg
  detrace(0,"POS write; pos_reg = %x pos_val = %x\n ",iob.md_addr,data[0]);
#endif

	if (htx_sp != NULL)
		(htx_sp->good_others)++;
	return(0);
   }  /* End pos_wr */

/*****************************************************************************

pos_rd

*****************************************************************************/

unsigned char pos_rd (pos_reg, status, tucb_ptr)
   int pos_reg;
   unsigned long *status;
   TUTYPE *tucb_ptr;
   {
	int rc;
	int slot;
	char slot_ptr[2];
	unsigned char data[2];
	MACH_DD_IO iob;
	struct htx_data *htx_sp;

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->fddi_s.htx_sp;
 
	slot_ptr[0] = tucb_ptr->slot;
	slot = atoi(slot_ptr) - 1;

	iob.md_data = data;
	iob.md_incr = MV_BYTE;
	iob.md_size = 1;
	iob.md_addr = POSREG(pos_reg,slot);

	*status = 0;
	rc = ioctl(tucb_ptr->mdd_fd,MIOCCGET,&iob);

	if (rc != 0)
	{
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
#ifdef debugg
  detrace(0,"POS read ioctl returned = %x\n pos_reg = %x\n pos_val = %x\n opcode = %x\n pos_addr = %x\n",rc,pos_reg,data[0],MIOCCGET,iob.md_addr);
#endif
 
		*status = MDD_IOCTL_ERR;
		return(MDD_IOCTL_ERR);
	 }
#ifdef debugg
  detrace(0,"POS read; pos_reg = %x pos_val = %x\n ",iob.md_addr,data[0]);
#endif

	if (htx_sp != NULL)
		(htx_sp->good_others)++;
	return(data[0]);
   }  /* End pos_rd */

