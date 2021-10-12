static char sccsid[] = "@(#)28  1.3  src/bos/diag/tu/ethstw/tu006.c, tu_ethi_stw, bos411, 9428A410j 10/5/92 14:53:36";
/*****************************************************************************
 * COMPONENT_NAME: tu_ethi_stw 
 *
 * FUNCTIONS: 	int tu006();
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************************/

/*****************************************************************************
Function(s) Test Unit 006 - Internal Wrap Test

Module Name :  tu006.c

STATUS:

DEPENDENCIES: none

RESTRICTIONS: none

EXTERNAL REFERENCES

   OTHER ROUTINES:
        extern int wrap();
	extern int pos_rd();
	extern int pos_wr();

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
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/entuser.h> 
#include "tu_type.h" 

/*** external files ***/
   extern int wrap();
	extern int pos_rd();
	extern int pos_wr();

/*****************************************************************************
Function: int tu006()

Interanl Wrap Test.
*****************************************************************************/
int tu006 (int fdes,  TUTYPE *tucb_ptr) 
   {
	uchar rd_data;
   int rc = 10; /* initialize rc with a failure return code */ 
	WRAPTEST wrap_op;

	wrap_op.wrap_type = INTERNAL;
	wrap_op.fairness = ENABLE;
	wrap_op.parity = ENABLE;
	wrap_op.card_type = THIN; /* this variable is not used in this test */ 

	/**********************************
    * Enable adapter, address and data
    * parity by reset POS2.
    **********************************/

   if (rc = pos_rd(tucb_ptr->mdd_fd, POS2, &rd_data))
      {
      PRINT((tucb_ptr->msg_file,"tu006: Unable to read POS2\n"));
      return (POS2_RD_ERR);
      }

   rd_data |= 0x3f;
   if (rc = pos_wr(tucb_ptr->mdd_fd, POS2, &rd_data))
      {
      PRINT((tucb_ptr->msg_file,"tu006: Unable to write to POS2\n"));
      return (POS2_WR_ERR);
      }

	rc = wrap(fdes, &wrap_op, tucb_ptr);

   return (rc);
   } /* end of tu006 */
