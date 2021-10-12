static char sccsid[] = "@(#)30  1.3  src/bos/diag/tu/ethstw/tu008.c, tu_ethi_stw, bos411, 9428A410j 10/5/92 14:54:18";
/******************************************************************************
 * COMPONENT_NAME: tu_ethi_stw 
 *
 * FUNCTIONS: 	int tu008();
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
Function(s) Test Unit 008 - External Loopback Test (DIX Connector)

Module Name :  tu008.c

STATUS:

DEPENDENCIES: none

RESTRICTIONS: none

EXTERNAL REFERENCES

   OTHER ROUTINES:
        extern int wrap();	tu009.c
        extern int io_rd();	rw_io.c
	extern int pos_rd();	rw_pos.c
	extern int pos_wr();	rw_pos.c

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
        extern int io_rd();
	extern int pos_rd();
	extern int pos_wr();

/*****************************************************************************
Function: int tu008()

External Wrap Test (DIX Connector)
*****************************************************************************/
int tu008 (int fdes,  TUTYPE *tucb_ptr) 
   {
	uchar rd_data;
        int rc = 0; /* return code */
	WRAPTEST wrap_op;

	wrap_op.wrap_type = EXT_DISABLE;
	wrap_op.fairness = DISABLE;
	wrap_op.parity = DISABLE;
	wrap_op.card_type = THICK;

	/**********************************
         * Enable adapter, address and data
         * parity by reset POS2.
         **********************************/

        if (rc = pos_rd(tucb_ptr->mdd_fd, POS2, &rd_data))
          {
          PRINT((tucb_ptr->msg_file,"tu008: Unable to read POS2\n"));
          return (POS2_RD_ERR);
          }

        rd_data |= 0x3f;
        if (rc = pos_wr(tucb_ptr->mdd_fd, POS2, &rd_data))
          {
          PRINT((tucb_ptr->msg_file,"tu008: Unable to write to POS2\n"));
          return (POS2_WR_ERR);
          }

	/***********************
         * check the riser cards
         ***********************/

        if (io_rd(tucb_ptr->mdd_fd, IO6, &rd_data))
          {
          PRINT((tucb_ptr->msg_file,"tu008: Unable to read I/O reg 6\n"));
	       PRINT ((tucb_ptr->msg_file,"tu008: DIX wrap test end.\n"));
          return (IO6_RD_ERR);
          }

	    rd_data &= 0x03;
        if(rd_data != TK_GDFUSE)
          {
   	  if(rd_data == TK_BDFUSE)
	    {
	    PRINT((tucb_ptr->msg_file,"tu008: Thick card has a blown fuse.\n"));
	    PRINT ((tucb_ptr->msg_file,"tu008: DIX wrap test end.\n"));
	    return (BADFUSE);
	    }
	
          PRINT ((tucb_ptr->msg_file,"tu008: The riser card is not THICK.\n"));
	  PRINT ((tucb_ptr->msg_file,"tu008: DIX wrap test end.\n"));
          return (RISERCARD_ERR);
          }

	rc = wrap(fdes, &wrap_op, tucb_ptr);

        return (rc);
   }
