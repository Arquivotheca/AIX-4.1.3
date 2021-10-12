static char sccsid[] = "@(#)rw_io.c     1.6 6/26/91 12:24:36";
/*****************************************************************************
 * COMPONENT_NAME: tu_ethi_stw 
 *
 * FUNCTIONS: 	int io_wr(); 
 *		int io_rd();
 *		int init_pos();
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
Function(s) Read/Write POS and I/0 Registers

Module Name : rw_io.c 

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
#include "tu_type.h"


/*****************************************************************************
io_wr
*****************************************************************************/
int io_wr (int mdd_fdes, ulong io_addr, uchar *value)
   {
	int rc;
 	MACH_DD_IO io; 

	io.md_size = 1;
   	io.md_incr = MV_BYTE; 	/* MV_BYTE OR MV_WORD */
   	io.md_addr = io_addr;
  	io.md_data = value;

	if (rc = ioctl(mdd_fdes, MIOBUSPUT, &io) < 0)
	    rc = 1; /* handler couldn't queue command to adapter */
	else
	   rc = 0; /* ioctl passed */

	return (rc);
   }  /* End io_wr */

/*****************************************************************************
io_rd
*****************************************************************************/
int io_rd (int mdd_fdes, ulong io_addr, uchar *return_val)
   {
	int rc;
	MACH_DD_IO io; 

	io.md_size = 1;
	io.md_incr = MV_BYTE;
	io.md_addr = io_addr;
	io.md_data = return_val;

	if (rc = ioctl(mdd_fdes, MIOBUSGET, &io) < 0) 
           rc = 1; /* handler couldn't queue command to adapter */
        else    
           rc = 0; /* ioctl passed */

	return (rc);
   }  /* End io_rd */
