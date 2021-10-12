static char sccsid[] = "@(#)tu002.c     1.8 6/26/91 12:33:17";
/*****************************************************************************
 * COMPONENT_NAME: tu_ethi_stw 
 *
 * FUNCTIONS: 	int io_chk(); 
 *		int tu002();
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
Function(s) Test Unit 002 - IO Register Test

Module Name :  tu002.c

STATUS:

DEPENDENCIES: none

RESTRICTIONS: none

EXTERNAL REFERENCES

   OTHER ROUTINES:
        extern int io_wr()		rw_io.c
        extern int io_rd()		rw_io.c
	extern int io_save()		sr_io.c
	extern int io_restore()		sr_io.c
	extern int pos_rd()		rw_io.c
	extern int pos_wr()		wr_io.c

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
        extern int io_wr();
        extern int io_rd();
	extern int io_save();
	extern int io_restore();
	extern int pos_wr();
	extern int pos_rd();

/*****************************************************************************
 *  Function: int io_chk()
 *****************************************************************************/
int io_chk (int mdd_fdes, TUTYPE * tucb_ptr)
   {
	uchar io_save_area[8];
	uchar rd_data, test_val;
	int i, counter, rc = 0; 
	ulong io_addr;

	static uchar tval[] = {0xaa, 0x55, 0xff, 0x33};

	/******************************************
	 * Enable 82596 and disable chip interrupt 
	 ******************************************/

	if (rc = pos_rd(mdd_fdes, POS2, &rd_data))
	  {
	  PRINT((tucb_ptr->msg_file,"tu002: Unable to read POS2.\n"));
	  return (POS2_RD_ERR);
	  }

	rd_data |= 0x01; /* enable bit 0 */
	rd_data &= 0xdf; /* disable 5 of POS2 */
	if(rc = pos_wr(mdd_fdes, POS2, &rd_data))
	  {
 	  PRINT((tucb_ptr->msg_file,"tu002: Unable to write to POS2.\n"));
	  return (POS2_WR_ERR);
	  }

	/*****************************************
         * Save the original data before writing.
         *****************************************/

        if (rc = io_save(mdd_fdes, io_save_area))
          {
          PRINT((tucb_ptr->msg_file,"tu002: Save original IO data failed\n"));
          return (rc);
          }

	/*****************************************************************
 	 * write 0xaa, 0x55, 0xff, 0x33 to the I/O Register, read them back
 	 * and compare with the writes.  If do not match return the error.
 	 ******************************************************************/

	counter = 0;
	for (io_addr = IO0; io_addr < IO6; io_addr++)
	  { /* io register loop */
	  for (i = 0; i < 4; i++)
	    { /* pattern loop */
	    test_val = tval[i];
	    if (rc = io_wr(mdd_fdes, io_addr, &test_val))
	      {
  	      PRINT((tucb_ptr->msg_file,"tu002: Unable to write to %x.\n",io_addr));
	      (void) io_restore(mdd_fdes, io_save_area);
	      return (IO0_WR_ERR + counter);
	      }

	    if (rc = io_rd(mdd_fdes, io_addr, &rd_data))
	      {
  	      PRINT((tucb_ptr->msg_file,"tu002: Unable to read from %x.\n",io_addr));
              (void) io_restore(mdd_fdes, io_save_area);
              return (IO0_RD_ERR + counter);
	      }

	    if (rd_data != test_val) /* compare IO datum with test value */
	      {
  	      PRINT((tucb_ptr->msg_file,"tu002: %x did not compare\n",io_addr));
              (void) io_restore(mdd_fdes, io_save_area);
	      return (IO0_CMP_ERR + counter);
	      }
	    }/* end pattern loop */

	    ++counter;
	  } /* end io register loop */
/************************** Test End Here ***********************************/

	/***********************************
	 * Restore the orignal data
	 ***********************************/

	if (rc = io_restore(mdd_fdes, io_save_area))
	  {
	  PRINT((tucb_ptr->msg_file,"tu002: Unable to restore IO reg data\n"));
	  return (rc);
	  }

	return (0); /* IO register test is successful */
   } /* end io_chk() */ 

/*****************************************************************************
Function: int tu002()

IO Register Test.
*****************************************************************************/
int tu002 (int fdes, TUTYPE *tucb_ptr) 
   {
        int rc = 0; /* return code */
	uchar rd_data;


        rc = io_chk(tucb_ptr->mdd_fd, tucb_ptr); /* Testing */ 

        /**********************
         * Enable chip interrupt
         **********************/

	 /* enable chip interrupt */
	  (void) pos_rd(tucb_ptr->mdd_fd, POS2, &rd_data);
	  rd_data |= 0x20;
	  (void) pos_wr(tucb_ptr->mdd_fd, POS2, &rd_data);

        return (rc);                  
   } 
