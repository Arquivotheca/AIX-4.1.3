static char sccsid[] = "@(#)sr_pos.c    1.8 6/26/91 12:29:35";
/*****************************************************************************
 * COMPONENT_NAME: tu_ethi_stw 
 *
 * FUNCTIONS: 	int pos_save(); 
 *		int pos_restore();
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
Functposn(s) Save/Restore POS Registers

Module Name :  sr_pos.c

STATUS:

DEPENDENCIES: none

RESTRICTIONS: none

EXTERNAL REFERENCES

   OTHER ROUTINES:			files
	extern unsigned char pos_rd();	rw_pos.c
	extern int pos_wr();		rw_pos.c

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
	extern uchar pos_rd();
 	extern int pos_wr();

/*****************************************************************************
Functposn: int pos_save()

Functposn saves off POS registers 2 through 6 in the passed-in 
array, "save_area".

IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to read of specific POS register
*****************************************************************************/
int pos_save (int mdd_fdes, uchar save_area[])
   {
	int j, rc = 0;
	ulong addr;
	uchar rd_data;

	if (rc = pos_rd(mdd_fdes, POS2, &rd_data))
	   rc = POS2_RD_ERR;
	else
	   save_area[0] = rd_data;

	if (!rc)
	   for (j=0, addr = POS4; addr < POS7; addr++, j++)
	   	{
	   	if ( rc = pos_rd(mdd_fdes, addr, &rd_data))
		   {
		   rc = POS4_RD_ERR + j;
		   break;
	 	   } /* endif */ 
	    	else
		   {
	      	   save_area[j+1] = rd_data;
		   rc = 0;
		   }
	   	} /* end for loop */

	return (rc);
   } /* end pos_save */

/*****************************************************************************
Functposn: int pos_restore()

Functposn restores POS registers 2 through 7 from the passed in unsigned char
array, "save_area".

IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to write of specific POS register
*****************************************************************************/
int pos_restore (int mdd_fdes, uchar save_area[]) 
   {
	ulong addr;
	int j, rc = 0; 

	/******************************************
	 * first, we restore POS registers 4 to 6
	 ******************************************/

	for (j=0, addr = POS4 ; addr < POS7 ; addr++, j++)
	   if (rc = pos_wr(mdd_fdes, addr, &save_area[j+1]))
		{
		rc = POS4_WR_ERR + j;
		break;
		}

	/**************************************************
	 * restore 2 last since it has the card enable bit
	 **************************************************/
	
	if (!rc)
	   if (rc = pos_wr(mdd_fdes, POS2, &save_area[0]))
	      rc = (POS2_WR_ERR);

	return (rc);
   } /* end pos_restore */
