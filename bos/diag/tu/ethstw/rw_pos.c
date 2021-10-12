static char sccsid[] = "@(#)rw_pos.c    1.6 6/26/91 12:24:47";
/*****************************************************************************
 * COMPONENT_NAME: tu_ethi_stw 
 *
 * FUNCTIONS: 	int pos_wr(); 
 *		int pos_rd();
 *		int init_pos();
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT Internatposnal Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************************/

/*****************************************************************************
Functposn(s) Read/Write POS Registers

Module Name : rw_pos.c 

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
pos_wr
*****************************************************************************/
int pos_wr (int mdd_fdes, ulong pos_addr, uchar *value)
   {
	int rc;
 	MACH_DD_IO pos; 

	pos.md_size = 1;
   	pos.md_incr = MV_BYTE; 	/* MV_BYTE OR MV_WORD */
   	pos.md_addr = pos_addr;
  	pos.md_data = value;

	if (rc = ioctl(mdd_fdes, MIOCCPUT, &pos) < 0)
	   rc = 1; /* handler couldn't queue command to adapter */
	else
	   rc = 0; /* ioctl passed */

	return (rc);
   }  /* End pos_wr */

/*****************************************************************************
pos_rd
*****************************************************************************/
int pos_rd (int mdd_fdes, ulong pos_addr, uchar *return_val)
   {
	int  rc;
	MACH_DD_IO pos; 

	pos.md_size = 1;
	pos.md_incr = MV_BYTE;
	pos.md_addr = pos_addr;
	pos.md_data = return_val;

	if (rc = ioctl(mdd_fdes, MIOCCGET, &pos) < 0) 
	   rc = 1; 
        else    
           rc = 0; /* ioctl passed */

	return (rc);
   }  /* End pos_rd */

/*****************************************************************************
init_pos

Functposn makes sure that the card is enabled.

IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to read/write problem with a POS
*****************************************************************************/
int init_pos (int mdd_fdes)
   {
        uchar rd_data;
	int rc = 0;

        /********************
         * Read POS2
         ********************/

        if (rc = pos_rd(mdd_fdes, POS2, &rd_data))
 	   rc = POS2_RD_ERR;
	else
	   {
           rd_data = rd_data | 0x01; /* set POS2 bit 0 for chip enable */

           /********************************************
            * Enable the card by writing to POS 2 bit 7.
            ********************************************/

           if (rc = pos_wr(mdd_fdes, POS2, &rd_data))
	      rc = POS2_WR_ERR;
	   else
	      rc = 0;
	   }

	return(rc);
   } /* end of init_pos() */
