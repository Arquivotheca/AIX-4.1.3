static char sccsid[] = "@(#)27  1.4  src/bos/diag/tu/ethstw/tu005.c, tu_ethi_stw, bos411, 9428A410j 10/5/92 14:53:18";
/*****************************************************************************
 * COMPONENT_NAME: tu_ethi_stw 
 *
 * FUNCTIONS:	
 *		int self_test();
 *		int tu005(); 
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
Function(s) Test Unit 005 - Internal Test (Self test)

Module Name :  tu005.c

STATUS:

DEPENDENCIES: none

RESTRICTIONS: none

EXTERNAL REFERENCES

   OTHER ROUTINES:
	extern int pos_rd()		rw_pos.c
	extern int pos_wr()		rw_pos.c

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
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/devinfo.h>
#include <sys/entuser.h>
#include "tu_type.h"

/*** external files ***/
	extern int pos_rd();
	extern int pos_wr();

/*****************************************************************************
Function: int self_test()

Setup pos register 2, start the device, issue ioctl to execute the self-test,
and verify the selt-test result.

IF successful
THEN RETURNs 0
ELSE RETURNs error code(s) of the problem(s). 
*****************************************************************************/
int self_test(int fdes, TUTYPE *tucb_ptr)
   {
	int rc = 0, result; /* return code */
	uchar rd_data;
	uchar sel_reslt;  /* self test result bit:   1=fail, 0=pass */
        uchar dia_reslt;  /* diagnose result bit:    1=fail, 0=pass */
        uchar bus_reslt;  /* bus timer result bit:   1=fail, 0=pass */
        uchar reg_reslt;  /* register result bit:    1=fail, 0=pass */
        uchar rom_reslt;  /* rom content result bit: 1=fail, 0=pass */

	struct
	   {
	   ulong rom_content;
	   ulong test_result;
	   } test_data;

	test_data.test_result = 0xffffffff; /* initialize test_result */

	/************************************
         * Enable adapter, address and data
         * parity, and 82596 
         ************************************/

        if (rc = pos_rd(tucb_ptr->mdd_fd, POS2, &rd_data))
          {
          PRINT ((tucb_ptr->msg_file,"tu005: Unable to read POS2\n"));
          return (POS2_RD_ERR);
          }

        rd_data |= 0x3f;
        if (rc = pos_wr(tucb_ptr->mdd_fd, POS2, &rd_data))
          {
          PRINT((tucb_ptr->msg_file,"tu005: Unable to write to POS2\n"));
          return (POS2_WR_ERR);
          }

	/* start the device */
	if (rc = start_eth(fdes, tucb_ptr))
           {
           PRINT((tucb_ptr->msg_file,"tu005: Unable to start the device\n"));
           PRINT((tucb_ptr->msg_file,"tu005: start rc = %d\n", rc));
           if (result = halt_eth(fdes, tucb_ptr))
                PRINT((tucb_ptr->msg_file,"tu003: halt rc = %d\n", result));
           return(rc);
           }

	/* get the test result */
	if (rc = ioctl(fdes, ENT_SELFTEST, &test_data) < 0)
	   {
	   if (result = halt_eth(fdes, tucb_ptr))
                PRINT((tucb_ptr->msg_file,"tu005: halt rc = %d\n", result));
	   return (ENT_SELFTEST_ERR);
	   }

	/* Halt the device */
	if (rc = halt_eth(fdes, tucb_ptr))
           {
           PRINT((tucb_ptr->msg_file,"tu005: Unable to halt the device\n"));
           PRINT((tucb_ptr->msg_file,"tu005: rc = %d\n",rc));
           return(rc);
           }

	/************************
	 * selftest varification
	 ************************/

	sel_reslt = (test_data.test_result >> 12) & 1; 
	dia_reslt = (test_data.test_result >> 5) & 1;
	bus_reslt = (test_data.test_result >> 4) & 1;
	reg_reslt = (test_data.test_result >> 3) & 1;
	rom_reslt = (test_data.test_result >> 2) & 1;

	/* check test result: 1 is failed; 0 is passed */ 
	if (sel_reslt || dia_reslt || bus_reslt || reg_reslt || rom_reslt)
	   {
	   PRINT((tucb_ptr->msg_file,"tu005: Selftest is Failed.\n"));
	   PRINT((tucb_ptr->msg_file,"tu005: %#x\n",test_data.test_result)); 
	   if (sel_reslt)
             PRINT((tucb_ptr->msg_file,"tu005: selftest result - FAIL.\n"));

           if (dia_reslt)
             PRINT((tucb_ptr->msg_file,"tu005: diagnose result - FAIL.\n"));

           if (bus_reslt)
             PRINT((tucb_ptr->msg_file,"tu005: bus timer result - FAIL.\n"));

           if (reg_reslt)
             PRINT((tucb_ptr->msg_file,"tu005: register result - FAIL.\n"));

           if (rom_reslt)
             PRINT((tucb_ptr->msg_file,"tu005: ROM content result- FAIL\n"));

	   return(SELFTEST_FAIL);
	   }
	
	return(0);
   } /* end self_test() */

/*****************************************************************************
Function: int tu005()

Self-Test

IF SUCCESS
THEN return 0
ELSE return the error code of the problem
*****************************************************************************/
int tu005 (int fdes, TUTYPE *tucb_ptr) 
   {
        int rc; /* return code */
	
        rc = self_test(fdes, tucb_ptr);

        return (rc);
   }
