static char sccsid[] = "@(#)45  1.3  src/bos/diag/tu/ethstw/rw_smem.c, tu_ethi_stw, bos411, 9428A410j 12/3/91 08:00:32";
/******************************************************************************
 * COMPONENT_NAME: tu_ethi_stw 
 *
 * FUNCTIONS: 	int smem_rd();
 *		int smem_wr();
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
 *****************************************************************************/

/*****************************************************************************
Function(s) Read/Write Shared Memory

Module Name : rw_smem.c 

STATUS:

DEPENDENCIES: none

RESTRICTIONS: none

EXTERNAL REFERENCES

   OTHER ROUTINES:
	error codes and constants are defined in tu_type.h

   DATA AREAS:

   TABLES: none

   MACROS: none

COMPILER / ASSEMBLER

   TYPE, VERSION: AIX C Compiler, version:

   OPTIONS:

NOTES: Nones.

*****************************************************************************/
/*** header files ***/
#include <stdio.h>
#include <errno.h>
#include <sys/mdio.h>

/*****************************************************************************
smem_rd
*****************************************************************************/
int smem_rd (int mdd_fdes, ulong ram_offset, ulong size, uchar *buf_ptr)
   {
	int rc;
	MACH_DD_IO ram;

	/*** set up structure for memory read ***/
	ram.md_addr = ram_offset; /* start address to be read */
	ram.md_size = size; /* size of buf_ptr in word if incr is MV_WORD */
			    /* If incr is MV_BYTE size is the number of byte */ 
	ram.md_incr = MV_WORD; /* increment in MV_WORD or in MV_BYTE */ 
	ram.md_data = buf_ptr; /* a pointer to the data to be read */

	/********************************************************
	 * Send the above structure to the machine device driver
	 * for reading the data from the memory.
	 ********************************************************/
	if (ioctl(mdd_fdes, MIOBUSGET, &ram))
	   rc = 1; /* handler couldn't queue command to adapter */
	else
	   rc = 0; /* ioctl passed */

	return (rc);
   }

/*****************************************************************************
smem_wr
*****************************************************************************/
int smem_wr (int mdd_fdes, ulong ram_offset, ulong size, uchar *buf_ptr)
   {
	int  rc;
	MACH_DD_IO ram;

	/* structure for memory write */
	ram.md_addr = ram_offset; /* start address for writing */
	ram.md_size = size; /* size of buf_ptr in word if incr is MV_WORD */
			    /* If incr is MV_BYTE size is the number of byte */ 
	ram.md_incr = MV_WORD; /* the data is a byte long */
	ram.md_data = buf_ptr;  /* data buffer to be written */

	/********************************************************
	 * Send the above structure to the machine device driver
	 * for writing data to the memory.
	 ********************************************************/
	if (ioctl(mdd_fdes, MIOBUSPUT, &ram))
	   rc = 1; /* handler couldn't queue command to adapter */
	else
	   rc = 0; /* ioctl passed */

	return(rc);
   }  /* End smem_wr */
