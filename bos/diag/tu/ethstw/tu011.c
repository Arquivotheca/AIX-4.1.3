static char sccsid[] = "@(#)tu011.c     1.5 6/28/91 08:19:40";
/*****************************************************************************
 * COMPONENT_NAME: tu_ethi_stw 
 *
 * FUNCTIONS:   
 *		int card_chk();
 *		int tu011();
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
Function(s) Test Unit 011 - Riser Card Test

Module Name : tu011.c

STATUS:

DEPENDENCIES: none

RESTRICTIONS: none

EXTERNAL REFERENCES

   OTHER ROUTINES:
        extern int io_rd();	rw_io.c
	extern int init_pos();	rw_pos.c

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
        extern int io_rd();
	extern int init_pos();

/*****************************************************************************
Fuction: int tu011.c

IF SUCCESS
THEN 
	RETURN(riser card code)
	TWISTED_PAIR..Indicate that Twisted Pair card is used.
	THICK.........Indicate that Thick card with good fuse is used.
	THIN..........Indicate that Thin card is used.
ELSE
	RETURN(error code)
*****************************************************************************/
int card_chk (int mdd_fdes, TUTYPE *tucb_ptr)
   {
	uchar rd_data;
	int rc = 0;
	int card;

	/* enable the adapter */

	if (rc = init_pos(mdd_fdes))
	   {
	   PRINT ((tucb_ptr->msg_file,"tu011: Unable to enable the adapter\n"));
	   return (rc);
	   }

	/************************************
 	 * Verify the type of installed card
	 ************************************/

        if (rc = io_rd(mdd_fdes, IO6, &rd_data))
           {
           PRINT ((tucb_ptr->msg_file,"tu011: Unable to read I/O reg 6.\n"));
           return(IO6_RD_ERR);
           }

	card = rd_data & 0x03;
        switch(card)
           {
           case TP_CARD:
                rc = TWISTED_PAIR;
                break;

           case TN_CARD:
		rc = THIN; 
                break;

           case TK_GDFUSE:
                rc = THICK; 
                break;

           case TK_BDFUSE:
                rc = BADFUSE; 
                break;

           default:
	        PRINT ((tucb_ptr->msg_file,"tu011: Invalid Riser card\n")); 
                rc =  RISERCARD_ERR;
                break;
           }/* end of switch */

	return(rc);
   } /* end card_chk() */

/*****************************************************************************
Function: int tu011()

Check the Riser Card.
*****************************************************************************/
int tu011 (int fdes, TUTYPE *tucb_ptr) 
   {
	int rc = 0; /* return code */

        rc = card_chk(tucb_ptr->mdd_fd, tucb_ptr);
	
        return (rc);
   } /* end of tu011() */

