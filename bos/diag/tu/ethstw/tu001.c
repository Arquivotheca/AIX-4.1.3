/*****************************************************************************
 * COMPONENT_NAME: tu_ethi_stw 
 *
 * FUNCTIONS: 	int pos_chk(); 
 *		int tu001();
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
Function(s) Test Unit 001 - POS Register Test

Module Name :  tu001.c

STATUS:

DEPENDENCIES: none

RESTRICTIONS: none

EXTERNAL REFERENCES

   OTHER ROUTINES:
        extern int pos_wr();		rw_pos.c
        extern int pos_rd();		rw_pos.c	
        extern int init_pos();		rw_pos.c
        extern int pos_save()           sr_pos.c
        extern int pos_restore()        sr_pos.c

        error codes and constants are defined in tu_type.h 

   DATA AREAS:

   TABLES: none

   MACROS: none

COMPILER / ASSEMBLER

   TYPE, VERSION: AIX C Compiler, version:

   OPTIONS:

NOTES: Nones.

CHANGE ACTIVITIES:

   EC00, Version 00 (Original), 03/15/91 !ML!
***************************************************************************/

/*** header files ***/
#include <stdio.h>
#include "tu_type.h"

/*** external files ***/
        extern int pos_wr();
        extern int pos_rd();
        extern int init_pos();
        extern int pos_save();
        extern int pos_restore();

/*****************************************************************************
Function: int pos_chk()

Function test writes/reads/compares byte values to check all bit positions
in writable POS registers (2-6), but not POS 3.


Disables the adapter and writes/reads/compares all bit positions to
POS registers 2 through 6, and not POS 3.

IF successful 
THEN RETURNs 0
ELSE RETURNs error code relating to read/write/compare problem with a POS
*****************************************************************************/
int pos_chk (int mdd_fdes, TUTYPE *tucb_ptr)
   {
	uchar rd_data;
	ulong pos_addr;
	int i, j, rc = 0; 
	uchar pos_save_area[8];
	uchar test_val;
	int quit = 0;
        static unsigned char tval[] = { 0xaa, 0x55, 0xff, 0x33 };

        /****************************************
         * Save the POS register first before
         * doing any writing.
         ****************************************/

        if (rc = pos_save(mdd_fdes, pos_save_area))
	  {
          PRINT((tucb_ptr->msg_file,"tu001: Unable to save pos register.\n"));
	  return (rc);
	  }

	/**********************
	 * Disable chip enable
	 **********************/	

	if (rc = pos_rd(mdd_fdes, POS2, &rd_data))
	  {
	  PRINT((tucb_ptr->msg_file,"tu001: Unable to read POS2 reg.\n"));
	  return (POS2_RD_ERR);
	  }

	rd_data &= 0xfe;
	if (rc = pos_wr(mdd_fdes, POS2, &rd_data))
	  {
	  PRINT((tucb_ptr->msg_file,"tu001: Unable to write to POS2.\n"));
	  return (POS2_WR_ERR);
	  }

	/***********************************************************
	 * make sure POS0 and POS1 (non-writable) have legal values.
	 ***********************************************************/

	/* Check POS0 */
	if (rc = pos_rd(mdd_fdes, POS0, &rd_data))
          {
          PRINT((tucb_ptr->msg_file,"tu001: Unable to read from POS0.\n"));
       	  return (POS0_RD_ERR);
	  }

	if (rd_data != INTPOS0)
	  {
	  PRINT((tucb_ptr->msg_file,"tu001: POS0 did not match.\n"));
	  return (POS0_CMP_ERR);
	  }

	/*** Check POS1 ***/
        if (rc = pos_rd(mdd_fdes, POS1, &rd_data))
          {
          PRINT((tucb_ptr->msg_file,"tu001: Unable to read from POS1.\n"));
          return (POS1_RD_ERR);
          }

        if (rd_data != INTPOS1)
          {
          PRINT((tucb_ptr->msg_file,"tu001: POS1 did not match.\n"));
          rc = POS1_CMP_ERR;
          }

	/***************************************************************
	 * try to write 0xAA, 0x55, 0xFF, 0x33 in pos regs. 2 through 7.
	 ***************************************************************/

/******************************* POS2 BEGIN **********************************/
        for (i = 0; i < 4; i++) 
           {
           /*************************************
            * mask out card enable to insure that
            * card remains disabled.
            *************************************/

           test_val = tval[i] & 0xfe;

           if (rc = pos_wr(mdd_fdes, POS2, &test_val))
                {
                PRINT((tucb_ptr->msg_file,"tu001: Unable to write to POS2\n"));
		(void) pos_restore(mdd_fdes, pos_save_area);
                return (POS2_WR_ERR);
                }

           if (rc = pos_rd(mdd_fdes, POS2, &rd_data))
                {
                PRINT((tucb_ptr->msg_file,"tu001: Read failed on POS2\n"));
                (void) pos_restore(mdd_fdes, pos_save_area);
                return(POS2_RD_ERR); 
                }

           if (rd_data != test_val)
                {
                PRINT((tucb_ptr->msg_file,"tu001: Values for POS2 didn't match.\n"));
                (void) pos_restore(mdd_fdes, pos_save_area);
                return(POS2_CMP_ERR);
                }
           }
/*********************** POS2 END ******************************************/

	for (j=0,pos_addr = POS4; pos_addr < POS7; j++, pos_addr++)
	  { /* register loop */
	  for (i = 0; i < 4; i++)
	    { /* pattern loop */
	    test_val = tval[i];
	    if (rc = pos_wr(mdd_fdes, pos_addr, &test_val))
	      {
  	      PRINT((tucb_ptr->msg_file,"tu001: Write failed on %x\n",pos_addr));
	      (void) pos_restore(mdd_fdes, pos_save_area);
	      return (POS4_WR_ERR + j);
	      }

     	    if (rc = pos_rd(mdd_fdes, pos_addr, &rd_data))
	      {
	      PRINT((tucb_ptr->msg_file,"tu001: Read failed on %x\n",pos_addr));
	      (void) pos_restore(mdd_fdes, pos_save_area, tucb_ptr);
	      return (POS4_RD_ERR + j);
	      }

	    if (rd_data != test_val)
	      {
   	      PRINT((tucb_ptr->msg_file,"tu001: Reg. %x didn't compared.\n",pos_addr));
	      (void) pos_restore(mdd_fdes, pos_save_area);
	      return (POS4_CMP_ERR + j);
	      }
	    } /* end pattern loop */
	  } /* end pos register loop */ 
	  
/***************** POS register test end ****************/

        /****************************************************
         * prior to returning, restore original POS reg vals.
         ****************************************************/

        if (rc = pos_restore(mdd_fdes, pos_save_area))
	  {
	  PRINT((tucb_ptr->msg_file,"tu001: Unable to restore pos reg.\n"));
	  return (rc);
	  }

        return (0);
    } /* end pos_chk() */

/*****************************************************************************
Function: int tu001()

POS Register Test.
*****************************************************************************/
int tu001 (int fdes, TUTYPE *tucb_ptr)
   {
	int rc = 0; /* return code */

	rc = pos_chk(tucb_ptr->mdd_fd, tucb_ptr);

        (void) init_pos(tucb_ptr->mdd_fd, tucb_ptr);
        return (rc);
  }
