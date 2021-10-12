static char sccsid[] = "src/bos/diag/tu/eth/tu002.c, tu_eth, bos411, 9428A410j 5/12/92 13:01:35";
/*
 * COMPONENT_NAME: (ETHERTU) Ethernet Test Unit
 *
 * FUNCTIONS: post_err, hard_reset, tu002
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990, 1991, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************

Function(s) Test Unit 002 - Hard Reset Test

*****************************************************************************/
#include <stdio.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <errno.h>
#include "ethtst.h"

#ifdef debugg 
extern void detrace();
#endif 

/*****************************************************************************

post_err

Function returns adapter code to correspond to failing return code
from POST.

*****************************************************************************/

int post_err (cmd_reg_val)
   unsigned char cmd_reg_val;
   {
	struct post_struct
	   {
		int tu_rc;
		unsigned char cmd_reg_val;
	   };
	static struct post_struct post_table[] =
	   {
		{ POST_01_ERR, 0x01 },
		{ POST_02_ERR, 0x02 },
		{ POST_03_ERR, 0x03 },
		{ POST_04_ERR, 0x04 },
		{ POST_05_ERR, 0x05 },
		{ POST_06_ERR, 0x06 },
		{ POST_07_ERR, 0x07 },
		{ POST_08_ERR, 0x08 },
		{ POST_09_ERR, 0x09 },
		{ POST_0A_ERR, 0x0a }
	   };
	static int table_size = sizeof(post_table) /
					sizeof(struct post_struct);
	struct post_struct *psp;
	int i;

	psp = post_table;
	for (i = 0; i < table_size; i++, psp++)
		if (psp->cmd_reg_val == cmd_reg_val)
			return(psp->tu_rc);

	return(-1);
   }


/*****************************************************************************

hard_reset

*****************************************************************************/

#define MAX_RESET_ATTEMPTS        4

int hard_reset (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int j;
	int num, rc;
	unsigned short val2;
	unsigned char i,p0,p1;
	unsigned char sval;
	unsigned long status, temp;
	unsigned long tar_time;
	struct htx_data *htx_sp;
	
	extern int hio_ctrl_wr();
	extern unsigned char hio_ctrl_rd();
	extern unsigned char hio_cmd_rd();
	extern unsigned char hio_cmd_rd_q();
	extern unsigned char hio_status_rd();
	extern unsigned char hio_status_rd_q();
	extern int smem_rd();
	extern int mktu_rc();
	extern unsigned char hio_parity_rd();
	extern int hio_parity_wr();

	extern int pos_wr();
	extern unsigned char pos_rd();

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->eth_htx_s.htx_sp;

#ifdef debugg
	detrace(0,"IN the Hard Reset Routine\n"); 
#endif

	/*
	 * specify DIX connector via POS reg 
	 * to turn off transceiver circuitry which could
	 * catch external noise and affect the POST internal wrap.
	 */
	p0 = pos_rd(fdes, 4, &status, tucb_ptr);
	if (status)
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS4_RD_ERR));
	   }

	/*
	 * bit 1 is connector, (0=DIX, 1=BNC)
	 */
	p0 &= 0xfd;
	if (pos_wr(fdes, 4, p0, &status, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS4_WR_ERR));
	   }
	
	/*
	 * Read Status register queue to check for Micro-channel
	 * exceptions or parity errors.
	 */

	rc = hio_status_rd_q(fdes, &status, tucb_ptr);
#ifdef debugg
	detrace(1,"rc value = % \n",rc);	   
#endif
        if (rc & 0x80)
	{
	    if(htx_sp != NULL)
		(htx_sp->bad_others)++;
	    return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,ADAP_PAR_ERR));
	}

	/*
	 * Clear the command queue in which the device driver stores
	 * command register values upon receiving interrupts.
	 */

	for (i = 0; i < 32; i++)
	   {
		p0 = hio_cmd_rd_q(fdes, &status, tucb_ptr);
		if (status)
		   {
			/*
			 * Check status indicating failure of reading
			 * command queue was due to being empty, else err
			 */
			if (status == CCC_QUE_EMPTY)
				break;
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				QCMD_RD_ERR));
		   }
	   }
	
	/*
	 * Make sure that we were able to clear
	 * the command queue so that our subsequent reads
	 * after the reset will contain expected values.
	 */
	if (status != CCC_QUE_EMPTY)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;	
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				QCMD_EMP_ERR));
	    }

	/*
	 * Set ATTN & RESET on
	 */
	if (hio_ctrl_wr(fdes, 0xc0, &status, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RCTL_WR_ERR));
	   }

	p1 = hio_ctrl_rd(fdes, &status, tucb_ptr);
	if (status != 0)
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RCTL_RD_ERR));
	   }

	/*
	 * Is ATTN & RESET on ?
	 */
	if ((p1 & 0xc0) != 0xc0)
	   {
#ifdef debugg
detrace(0,"hard_reset:  p1 got %x (hex) after hio_ctrl_wr/rd of 0xc0\n", p1);
#endif
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RCTL_CMP_ERR));
	   }

	/*
	 * allow adapter to sit in RESET state for a sec...
	 */

	sleep(2);

	/*
	 * Clear ATTN & RESET, and disable interrupts
	 */
	
	if (hio_ctrl_wr(fdes, 0x00, &status, tucb_ptr))
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RCTL_WR_ERR));
		
	/*
	 * sleep two seconds to allow
	 * adapter to go through reset sequence.
	 */

	sleep(2);

	/*
	 * Read status register to see if the command register is
	 * full (CWR -- bit 5)
	 * allow a max of five seconds for status 
	 */

	tar_time = time((long *)0) +5;
	while((time((long *)0) < tar_time)&(p1 != 0x20))
	{
	   p1 = hio_status_rd(fdes, &status, tucb_ptr);
	   if (status != 0)
	      return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RSTA_RD_ERR));

	   p1&=0x20;
	}
	
	/* 
 	 * Check if word received CWR ON 
	 * if CWR never set on then return with dead card error
	 * Unable to reset if CWR not set...
	 * if CWR is set on the let's go get data from the command reg.
	 */

	if (p1 & 0x20)
  	  {
	    p0 = hio_cmd_rd(fdes, &status, tucb_ptr);
	    if (status)
	       return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RCMD_RD_ERR));

	 /*
	  * Check for reset completion 0x00
	  */

            if (p0 == 0)
	      {	

		temp = 0;
		 for(j = 0; j < 32; j+=8 )
		   {

	/*
	 * Read status register to see if the command register is
	 * full (CWR -- bit 5)
	 * loop for 5 seconds or CWR bit set
	 */	

		     tar_time = time((long *)0) +5;
		     while((time((long *)0) < tar_time)&(p1 != 0x20))   
		     {	
			p1 = hio_status_rd(fdes, &status, tucb_ptr);
		 	if (status != 0)
	   		  return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RSTA_RD_ERR));
			p1&=0x20;

		     }
 
		     if (p1 != 0x20) 
	     	     {
			/*
	 		 * Read status register queue to check for Micro channel
	 		 * exceptions or parity errors.
	 		 */

			rc = hio_status_rd_q(fdes, &status, tucb_ptr);
			if (rc & 0x80)
			{
			   if (htx_sp != NULL)
			      (htx_sp->bad_others)++;
			   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,ADAP_PAR_ERR));
			}
			/* 
			 * If we did not get a parity a micro channel
			 * exception the report it as a bad value in the 
			 * status register
			 */

	   	        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RSTA_ER_ERR));
		     }	

		     p0 = hio_cmd_rd(fdes, &status, tucb_ptr);
		     if (status)
	 	        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RCMD_RD_ERR));

		     temp |=(0xff & p0)<<j;	
		   } /* end of loop */
		p0 = 0x00;
		  
	      }
	   }	
	   else
	   {		

	/*
	 * data is not available at the adapter, so lets check if the 
	 * driver is atoring it in the command queue. 
	 */


        for (j = 0; j < MAX_RESET_ATTEMPTS; j++)
           {
                p1 = hio_cmd_rd_q(fdes, &status, tucb_ptr);
                if (status != 0)
                   {
                        /*
                         * Check if command queue emptied before ever
                         * getting the reset status.
                         */
                        if (status == CCC_QUE_EMPTY)
                                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                        QCMD_EMP2_ERR));

                        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                        QCMD_RD_ERR));
                   }

                if (p1 == 0)
                        break;
                sleep(1);
           }

	if (p1 == 0)
	{

        /*
         * Get adapter's execute mailbox offset.  If we've made it this
         * far, then the driver should still have this info stored
         * in the command queue.
         */
        temp = 0;
        for (i = 0; i < 32; i+=8 )
           {
                p0 = hio_cmd_rd_q(fdes, &status, tucb_ptr);
                if (status != 0)
                   {
#ifdef debugg
                        detrace(1," Command Read Q Status = %x\n",status);
#endif
                        if (status == CCC_QUE_EMPTY)
                                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                        QCMD_EMP3_ERR));
                        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                RCMD_RD_ERR));
                   }

                temp |= (0xff & p0)<<i;

           }  /* End for loop */

	  p0 = 0x00;

	}
	else
	{

	/*
	 * Read status register queue to check for Micro channel
	 * exceptions or parity errors.
	 */

	      rc = hio_status_rd_q(fdes, &status, tucb_ptr);
	      if (rc & 0x80)
	      {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		 return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,ADAP_PAR_ERR));
	      }
	/*
	 * Well the adapter did not get a Micro channel exception or 
	 * parity errors, so we assume the adapter is dead.
	 */

	      return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADAP_RES_ERR));

}

	   }

	/*
	 * Check if reset never completed.
	 */

	/*
	 * Read status register queue to check for Micro channel
	 * exceptions or parity errors.
	 */

	rc = hio_status_rd_q(fdes, &status, tucb_ptr);
	if (rc & 0x80)
	{
	   if(htx_sp != NULL)
		(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,ADAP_PAR_ERR));
	}


	if (p0 == 0xff)
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, HARD_RES_ERR));
	
	/*
	 * If reset passed 0xff, check if non-zero which would indicate
	 * the failure code of an onboard POST.
	 */
	if (p0 != 0x00)
	   {
		rc = post_err(p0);
		if (rc < 0)
			rc = HARD_RES1_ERR;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
	   }
	

        /*
	 * Disable Parity in Parity Register since it
	 * now defaults to turn on after hard reset.
	 */
	sval = hio_parity_rd(fdes, &status, tucb_ptr);
	if (status)
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RPAR_RD_ERR));
	   }	

	sval = sval & 0xfe; 

	if (hio_parity_wr(fdes, sval, &status, tucb_ptr))
	   {
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RPAR_WR_ERR));
	   }

	/*
	 * Save the execute mailbox offset in that specific
	 * variable within the tucb_ptr for any other TUs
	 * that might need it (e.g. Wrap Test).
	 */
	tucb_ptr->eth_htx_s.exec_mbox_offset = temp;

	/*
	 * read values from adapter config. table
	 * NOTE THAT WE ARE BYTE SWAPPING WHEN
	 * WE SAVE THEM INTO OUR ARRAY!!!
	 */
	for (i = 0; i < 14; i++)
	   {
		if (rc = smem_rd(fdes,
			(tucb_ptr->eth_htx_s.exec_mbox_offset + (2 * i)),
			2, &val2, &status, tucb_ptr))
		   {
#ifdef debugg
			detrace(1,"Hard Reset -- Error Reading CONF table\n");	   
#endif

			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				CONF_RD_ERR));
		   }
		/*
		 * save the config. table values in that specific
		 * array within the tucb_ptr for any other TUs
		 * that might need it (e.g. Wrap Test).
		 */
		tucb_ptr->eth_htx_s.config_table[i] =
				((val2<<8)&0xFF00) | ((val2>>8)&0x00FF);
	   } /* end for-i */
#ifdef debugg
	detrace(0,"hard_reset:  Exec mbox        :  %08x\n",
		tucb_ptr->eth_htx_s.exec_mbox_offset);
	detrace(0,"hard_reset:  MAIN offset LSW  :  %08x\n",
		tucb_ptr->eth_htx_s.config_table[1]);
	detrace(0,"hard_reset:  MAIN offset MSW  :  %08x\n",
		tucb_ptr->eth_htx_s.config_table[2]);
	detrace(0,"hard_reset:  Rec'v mbox offset:  %08x\n",
		tucb_ptr->eth_htx_s.config_table[3]);
	detrace(0,"hard_reset:  Trans mbox offset:  %08x\n",
		tucb_ptr->eth_htx_s.config_table[4]);
	detrace(0,"hard_reset:  Exec  mbox offset:  %08x\n",
		tucb_ptr->eth_htx_s.config_table[5]);
	detrace(0,"hard_reset:  Stat Count offset:  %08x\n",
		tucb_ptr->eth_htx_s.config_table[6]);
	detrace(0,"hard_reset:  End RAM    offset:  %08x\n",
		tucb_ptr->eth_htx_s.config_table[7]);
	detrace(0,"hard_reset:  Buf Des. Reg. siz:  %08x\n",
		tucb_ptr->eth_htx_s.config_table[8]);
	detrace(0,"hard_reset:  TX list start off:  %08x\n",
		tucb_ptr->eth_htx_s.config_table[9]);
	detrace(0,"hard_reset:  TX list count    :  %08x\n",
		tucb_ptr->eth_htx_s.config_table[10]);
	detrace(0,"hard_reset:  RV list start off:  %08x\n",
		tucb_ptr->eth_htx_s.config_table[11]);
	detrace(0,"hard_reset:  RV list count    :  %08x\n",
		tucb_ptr->eth_htx_s.config_table[12]);
	detrace(1,"hard_reset:  Firmware version :  %08x\n",
		tucb_ptr->eth_htx_s.config_table[13]);
#endif

	/*
	 * save the main offset in that specific
	 * variable within the tucb_ptr for any other TUs
	 * that might need it (e.g. Wrap Test).
	 */
	tucb_ptr->eth_htx_s.main_offset =
		tucb_ptr->eth_htx_s.config_table[1] +
		(tucb_ptr->eth_htx_s.config_table[2] * 0x10000);

	return(0);
   }

/*****************************************************************************

tu002

*****************************************************************************/

int tu002 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	return(hard_reset(fdes, tucb_ptr));
   }
