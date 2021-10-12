static char sccsid[] = "@(#)25  1.3  src/bos/diag/tu/ethstw/tu003.c, tu_ethi_stw, bos411, 9428A410j 12/3/91 08:13:25";
/*****************************************************************************
 * COMPONENT_NAME: tu_ethi_stw 
 *
 * FUNCTIONS: 	int ram_tst(); 
 *		int tu003();
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
Function(s) Test Unit 003 - RAM Test

Module Name :  tu003.c

STATUS:

DEPENDENCIES: none

RESTRICTIONS: none

EXTERNAL REFERENCES

   OTHER ROUTINES:
        extern int io_rd()		rw_io.c
	extern int pos_rd()		rw_pos.c
	extern int pos_wr()		rw_pos.c
        extern int smem_rd()		rw_smem.c
        extern int smem_wr()		rw_smem.c
	extern int start_eth()		st_eth.c
	extern int halt_eth()		st_eth.c

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
*****************************************************************************/

/*** header files ***/
#include <stdio.h>
#include <sys/comio.h>
#include <sys/devinfo.h>
#include <errno.h>
#include "tu_type.h"
#include <memory.h>

/*** external files ***/
        extern int io_rd();
	extern int pos_rd();
	extern int pos_wr();
        extern int smem_rd();
        extern int smem_wr();
	extern int start_eth();
	extern int halt_eth();

/*** constants use in this file ***/
#define         MAX_MEM         0x1000  /* 4K bytes */
#define         LENGTH          0x100   /* 256 bytes */
 
/***************************************************************************
Function: int ram_tst()

Function writes patterns of data to local RAM from offset specified
in IO register 0 and 1, and reads back to compare.

IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to the problem.
*****************************************************************************/
int ram_tst (int fdes, int mdd_fdes, TUTYPE *tucb_ptr)
   {
	ulong start_addr, stop_pnt;
	uchar rd_data;
	uchar buf0[LENGTH], buf1[LENGTH];
	static uchar tval[] = {0x55, 0xaa, 0xff, 0x33};
	uchar pattern[LENGTH];
	int i, j, rc = 10; 
	int result;
	int size = LENGTH/4; /* data size in word */

	/************************************************************
         * Set up the RAM base by reading IO register 0 and 1
         ************************************************************/

	/* start the device */
	if (rc = start_eth(fdes, tucb_ptr))
           {
           PRINT((tucb_ptr->msg_file,"tu003: Unable to start the device\n"));
	   PRINT((tucb_ptr->msg_file,"tu003: start rc = %d\n", rc));
	   if (result = halt_eth(fdes, tucb_ptr))
		PRINT((tucb_ptr->msg_file,"tu003: halt rc = %d\n", result));
           return(rc);
           }

	/* read Hi byte of address */
        if (rc = io_rd(mdd_fdes, IO1, &rd_data))
          {
          PRINT ((tucb_ptr->msg_file,"tu003: Unable to write IO Register 1\n"));
	  if (result = halt_eth(fdes, tucb_ptr))
                PRINT((tucb_ptr->msg_file,"tu003: halt rc = %d\n", result));
          return (IO1_RD_ERR);
          }

	start_addr = rd_data;

	/* read Lower byte of address */
        if (rc = io_rd(mdd_fdes, IO0, &rd_data))
          {
          PRINT ((tucb_ptr->msg_file,"tu003: Unable to write IO Register 0\n"));
	  if (result = halt_eth(fdes, tucb_ptr))    
                PRINT((tucb_ptr->msg_file,"tu003: halt rc = %d\n", result));
          return (IO0_RD_ERR);
          }

	start_addr = (start_addr << 8) | rd_data;

	/* halt the device */
	if (rc = halt_eth(fdes, tucb_ptr))
           {
           PRINT((tucb_ptr->msg_file,"tu003: Unable to halt the device\n"));
	   PRINT((tucb_ptr->msg_file,"tu003: rc = %d\n",rc));
           return(rc);
           }

	/* If start_addr is 0 then RAM offset is 0x0000. 
	   If start_addr is 1 then RAM offset is 0x1000.
	   So we need to shift to the left 12 places. */ 
	start_addr = (start_addr << 12);

        stop_pnt = start_addr + (MAX_MEM - LENGTH); /* stop point */
 
        /**********************************
	 * Enable adapter, address and data
	 * parity by reset POS2.
	 **********************************/

	/* read pos register 2 */
	if (rc = pos_rd(mdd_fdes, POS2, &rd_data))
	  {
          PRINT((tucb_ptr->msg_file,"tu003: Unable to read POS2\n"));
	  return (POS2_RD_ERR);
	  }

	rd_data |= 0x3f; /* enable adapter and parity */
	
	/* write to pos register 2 */
	if (rc = pos_wr(mdd_fdes, POS2, &rd_data))
	  {
          PRINT((tucb_ptr->msg_file,"tu003: Unable to write to POS2\n"));
	  return (POS2_WR_ERR); 
	  }

	/*********************************** 
	 * ---- RAM access test begin ---- 
	 ***********************************/

        for ( ;start_addr <= stop_pnt; start_addr += LENGTH)
           {        /**** start_addr loop ****/
           /* Save the original data into buf0 */
           if (rc = smem_rd(mdd_fdes, start_addr, size, buf0))
              {
              PRINT((tucb_ptr->msg_file,"tu003: Saving RAM at %#x failed.\n",start_addr));
              return (MEM_RD_ERR);
              }

  	  for ( i = 0; i < 4; ++i)
    	     { /**** i loop ***/
    	     for(j=0; j < LENGTH; ++j)
      	  	pattern[j] = tval[(i+j)%4]; /* setup pattern */

	     /* Write the pattern to the RAM starting at start_addr */ 
	     if (rc = smem_wr(mdd_fdes, start_addr, size, pattern))
	        {
    	        PRINT((tucb_ptr->msg_file,"tu003: Unable to write to RAM at %#x.\n",start_addr));
  	        return (MEM_WR_ERR);
	        }

	     /* Read back the written values and store in buf1 */
	     if (rc = smem_rd(mdd_fdes,start_addr, size, buf1))
	        {
	        PRINT((tucb_ptr->msg_file,"tu003: Failed to read from RAM at %#x \n", start_addr));
	        return (MEM_RD_ERR);
	        }

	     /* Compare the pattern with the written values */
	     if (memcmp(pattern, buf1, LENGTH) != 0)
	        {
	        PRINT((tucb_ptr->msg_file,"tu003: RAM didn't match.\n"));
	        PRINT((tucb_ptr->msg_file,"tu003: Failed at %#x\n",start_addr));
	        return (MEM_CMP_ERR);
	        }
	     } /**** end i loop ****/

          /* Restore originally stored data from buf0 */ 
          if (rc = smem_wr(mdd_fdes, start_addr, size, buf0))
             {
             PRINT((tucb_ptr->msg_file,"tu003: Unable to restore RAM data\n"));
             return (MEM_WR_ERR);
             }
          }     /***** end start_addr loop ******/
	
	return (0);
   }  /* end of ram_tst() */

/*****************************************************************************
Function: int tu003()

Ram Test.
*****************************************************************************/
int tu003 (int fdes, TUTYPE *tucb_ptr) 
   {
        int rc = 10; /* initialize the return code */

	rc = ram_tst (fdes, tucb_ptr->mdd_fd, tucb_ptr);

        return (rc);
   }
