static char sccsid[] = "@(#)46  1.3  src/bos/diag/tu/ethstw/sr_io.c, tu_ethi_stw, bos411, 9428A410j 12/3/91 08:10:25";
/*****************************************************************************
 * COMPONENT_NAME: tu_ethi_stw
 *
 * FUNCTIONS: 	int io_save(); 
 *		int io_restore();
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
Function(s) Save/Restore IO  Registers


STATUS:

DEPENDENCIES: none

RESTRICTIONS: none

EXTERNAL REFERENCES

   OTHER ROUTINES:			files
	extern unsigned char io_rd();	rw_io.c
       	extern int io_wr();		rw_io.c

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
#include "tu_type.h"

/*** external files ***/
	extern uchar io_rd();
        extern int io_wr();


/*****************************************************************************
Function: int io_save();

Function saves off IO registers 1 through 5 in the passed-in 
array, "save_area".

IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to read of specific POS register
*****************************************************************************/
int io_save (int mdd_fdes, uchar save_area[])
   {
	int j, rc = 0;
	ulong addr;
	uchar rd_data;

	for (j=0, addr = IO0; addr < IO6; addr++, j++)
	   {
	   if (rc = io_rd(mdd_fdes, addr, &rd_data))
		{
		rc = IO0_RD_ERR + j;
		break;
		}
	   else
		{
		save_area[j] = rd_data;
		rc = 0;
		}
	   } /* end for */
	
	return (rc);
   } /* end io_save */

/*****************************************************************************
io_restore

Function restores IO registers 1 through 5 from the passed in 
array, "save_area".

IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to write of specific POS register
*****************************************************************************/
int io_restore (int mdd_fdes, uchar save_area[]) 
   {
	ulong addr;
	int j, rc = 0; 

	for (j=0, addr = IO0 ; addr < IO6 ; addr++, j++)
	   {
	   if (rc = io_wr(mdd_fdes, addr, &save_area[j]))
		{
		rc = IO0_WR_ERR + j;
		break;
		}
	   else
		rc = 0;
	   } /* end for */

	return (rc);
   } /* end io_restore */
