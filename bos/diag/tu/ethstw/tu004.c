static char sccsid[] = "@(#)26  1.3  src/bos/diag/tu/ethstw/tu004.c, tu_ethi_stw, bos411, 9428A410j 10/5/92 14:52:44";
/**********************************************************************
 * COMPONENT_NAME: tu_ethi_stw 
 *
 * FUNCTIONS: 	
*		int init_spos();
 *		int get_vpd();
 *		int vpd_test(); 
 *		int tu004();
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
 **********************************************************************/

/**********************************************************************
Function(s) Test Unit 004 - Vital Product Data (VPD) Test

Module Name : tu004() 

STATUS:

DEPENDENCIES: none

RESTRICTIONS: none

EXTERNAL REFERENCES

   OTHER ROUTINES:
        extern int pos_rd()		rw_pos.c
        extern int pos_wr() 		rw_pos.c        
        extern int crc_gen()	 	crc_gen.c     

        error codes and constants are defined in tu_type.h 

   DATA AREAS:

   TABLES: none

   MACROS: none

COMPILER / ASSEMBLER

   TYPE, VERSION: AIX C Compiler, version:

   OPTIONS:

NOTES: Nones.
***********************************************************************/

/*** header files ***/
#include <stdio.h>
#include "tu_type.h"

/*** external files ***/
        extern int pos_rd();  		/* reads value from POS reg */
        extern int pos_wr();            /* writes value to POS reg */
        extern int crc_gen();           /* Computer CRC */

/*** constants use in this file ***/
#define VPD_MAX_SIZE    0xff    /* Maximum length of VPD PROM */
#define MAX_RETRY	3	/* Maximum read VPD retry */

/*****************************************************************************
Function: init_spos();

Initialize the system POS 6 and 7 with zeroes.

IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to the problem.
******************************************************************************/
int init_spos(int mdd_fdes, TUTYPE *tucb_ptr)
        {
        int rc = 10; /* intialize the rc with a failed return code */
        uchar zero = 0x00;

        /* write 0x00 to nio POS register 6 */
        if (pos_wr(mdd_fdes, SPOS6, &zero))
           {
           PRINT((tucb_ptr->msg_file,"init_spos: Write to SPOS 6 failed.\n"));
           return (SPOS6_WR_ERR);
           }

        /* write 0x00 to nio POS register 7 */
        if (pos_wr(mdd_fdes, SPOS7, &zero))
           {
           PRINT((tucb_ptr->msg_file,"init_spos: Write to POS 7 failed.\n"));
           return (SPOS7_WR_ERR);
           }

        return (0); /* clear nio POS register is successful */
        } /* end init_spos() */
/*****************************************************************************

/*****************************************************************************
Function: get_vpd();

Initialize the system POS 6 and 7 with zeroes.  Write VPD address to POS 6.
Read VPD data from POS 3. Store VPD data in vpd array.


IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to the problem.
******************************************************************************/
int get_vpd (int mdd_fdes, uchar vpd[], TUTYPE *tucb_ptr)
   {

        int rc = 10; /* intialize the rc with a failed return code */
        uchar i;
        uchar   rd_data;
	int 	vpd_size;

	vpd[0] = 0x0;       /* vpd[0] will not be used */

 	/* Get the VPD length from address 5 */ 
	i= 5;
	if (pos_wr(mdd_fdes, SPOS6, &i)) 
	   {
	   PRINT((tucb_ptr->msg_file,"vpd_test: Unable to write to SPOS6\n"));
	   return (SPOS6_WR_ERR);
           }

	/* Read VPD value from nio POS3 */
	if (pos_rd(mdd_fdes, SPOS3, &rd_data)) 
	   { 
	   PRINT((tucb_ptr->msg_file,"vpd_test: Unable to read VPD.\n")); 
	   return (SPOS3_RD_ERR);
	   }

	/*********************************************************************
	 * Get the size of the VPD.  Note that 1/2 of the VPD length, the data
	 * portion, is stored in vpd[4] - the MSB, and vpd[5] - the LSB.
	 * Since, the number is so small, vpd[4] will be zero, which is not 
	 * needed in the calculation.
	 * VPD size is VPD length + the heading, which is 8 bytes.
	 ****************************************************************/

        vpd_size = (int) ((2 * rd_data) + 8);

	/* Check to make sure vpd_size is not exceed the maximum size */
	if (vpd_size > VPD_MAX_SIZE)
	   {
	   PRINT((tucb_ptr->msg_file,"vpd_test: vpd_size exceed maximum size.\n"));
	   return (VPD_LEN_ERR);
	   }

	/*** Now read the rest of vpd ***/
	for (i = 0x01; i < (uchar) vpd_size; i++)
	   { 
	   /* Write VPD address to nio POS6 */
           if (pos_wr(mdd_fdes, SPOS6, &i))
           	{
                PRINT((tucb_ptr->msg_file,"vpd_test: Unable to write to SPOS6\n"));
                return (SPOS6_WR_ERR);
		}

	    /* Read VPD Value from nio POS3 */
            if (pos_rd(mdd_fdes, SPOS3, &rd_data))
		{
		PRINT((tucb_ptr->msg_file,"vpd_test: Unable to read VPD.\n"));
                return (SPOS3_RD_ERR);
		}
	
	   vpd[i] = rd_data; /* Store VPD value in vpd array */
   	   }

	return (0);
   } /* end get_vpd */

/*****************************************************************************
Function: int vpd_test()

Function reads the VPD into the array vpd. Once read into the array, function
calls "crc_gen" to calculate the Cyclic Redundancy Checksum (CRC) on
the VPD and compares it to the one written in the adapter.

IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to the problem.
***********************************************************************/
int vpd_test (int mdd_fdes, TUTYPE *tucb_ptr)
   { 
	int rc = 10; /* intialize the rc with a failed return code */
	int retry;
	uchar i;
	uchar zero = 0;
	uchar   vpd_size;
	uchar   vpd_len; 	/* from addr 8 to the end of VPD */
	uchar   vpd_crc;        /* crc stored in VPD byte 6 & 7 */
	uchar   calc_crc;
	uchar 	addr;   	/* variables use to store any data */
	uchar   vpd[VPD_MAX_SIZE];
	uchar	rd_data;
	
	/* Initialize NIO POS6 and POS7 */
	if (rc = init_spos(mdd_fdes, tucb_ptr))
  	   {
	   PRINT((tucb_ptr->msg_file,"Error while init. SPOS 6 & 7.\n"));
           return (rc);
           }

	/* Get VPD data */
	retry=0; 
    	do
	   {
	   rc = get_vpd(mdd_fdes, vpd, tucb_ptr);
	   ++retry;
	   }
	while ((rc) && (retry <= MAX_RETRY));

	if (rc)
	   {
	   PRINT((tucb_ptr->msg_file,"Error while getting vpd.\n"));
	   (void) init_spos(mdd_fdes, tucb_ptr);
	   return (rc);
	   }

	/* Reinitialize NIO POS6 and POS7 */
	if (rc = init_spos(mdd_fdes, tucb_ptr)) 
           {
           PRINT((tucb_ptr->msg_file,"Error while reinit SPOS 6 & 7.\n"));
           return (rc);
           }

	/* If the first 3 bytes is not 'VPD', declare failure and exit */
	if ((vpd[1]!='V') || (vpd[2]!='P') || (vpd[3]!='D'))
	   {
	   PRINT((tucb_ptr->msg_file,"VPD heading is incorrect.\n"));
	   return (VPD_HDR_ERR);
	   }

	/* Get vpd_len, which 2 time the value stored in vpd[5]. */
	vpd_len = 2 * vpd[5];

	/* Get vpd_size, which is vpd_len plus the heading - 8 bytes. */
	vpd_size = vpd_len + 8;
 
	/* form CRC by combining MSB and LSB from VPD */
	vpd_crc = (0x100 * vpd[6]) + vpd[7];

	/*** computing the CRC ***/
	calc_crc = crc_gen(&vpd[8], vpd_len); /* compute crc */

	/* Compare the calculated CRC with the VPD CRC */
	if (vpd_crc != calc_crc)
	   {
	   PRINT((tucb_ptr->msg_file,"vpd_test: CRC didn't match.\n"));
	   return (VPD_CRC_ERR);
	   }

        /***************************************************
         * Check the Network Address Field to make sure it 
         * does not have the multicast bit ON.  That is,
         * the most significant bit must be 0.
         ***************************************************/
	for (i=8; i<vpd_size; i++)
	   { /* start for i loop */
	   if ((vpd[i]=='E') && (vpd[i+1]=='T') && (vpd[i+2]=='H') &&
	       (vpd[i+3]=='E') && (vpd[i+4]=='R') && (vpd[i+5]=='N') &&
	       (vpd[i+6]=='E') && (vpd[i+7]=='T'))
	      {
	      if ((vpd[i+12]>>7) != 0)
	         {
	         PRINT((tucb_ptr->msg_file,"vpd_test: The multicast bit is set.\n"));
	         return (NAF_MULT_ON);
	         }
	      break;
	      }

	   else if (i >= (vpd_size - 10))
	      {
	      PRINT((tucb_ptr->msg_file,"vpd_test: Ethernet VPD is not found.\n"));
	      return (NO_ETH_VPD);
	      }
	   } /* end of for i loop */

	return (0);
   } /* end vpd_test */

/**********************************************************************
Function: int tu004()

VPD Test.
***********************************************************************/
int tu004 (int fdes, TUTYPE *tucb_ptr)
   {
        int rc; /* return code */

	rc = vpd_test(tucb_ptr->mdd_fd, tucb_ptr);

        return (rc);
   }
