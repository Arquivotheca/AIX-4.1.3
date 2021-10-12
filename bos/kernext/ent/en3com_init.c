static char sccsid[] = "@(#)41  1.9  src/bos/kernext/ent/en3com_init.c, sysxent, bos411, 9428A410j 4/15/94 13:59:24";
/*
 * COMPONENT_NAME: sysxent -- High Performance Ethernet Device Driver
 *
 * FUNCTIONS: 
 *		en3com_init
 *		en3com_getvpd
 *		en3com_fixvpd
 *		en3com_start
 *		en3com_setpos
 *		en3com_getcfg
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/types.h>
#include <sys/lock_def.h>
#include <sys/errno.h>
#include <sys/intr.h>
#include <sys/watchdog.h>
#include <sys/dma.h>
#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/sleep.h>
#include <sys/err_rec.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/ethernet_mibs.h>
#include <sys/cdli_entuser.h>
#include <net/spl.h>



#include "en3com_dds.h"
#include "en3com_mac.h"
#include "en3com_hw.h"
#include "en3com_pio.h"
#include "en3com.h"
#include "en3com_errids.h"

extern ushort gencrc();

void en3com_fixvpd();

/*****************************************************************************/
/*
 * NAME:     en3com_init
 *
 * FUNCTION: Perform the device-specific initialization during the 
 *	     configuration time. Get the VPD and card type.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_cfg_init
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area.
 *
 * RETURNS:  
 *	0 - OK
 *	EINVAL - can't get VPD from the adapter
 *	EIO - I/O error
 */
/*****************************************************************************/
en3com_init(
  en3com_dev_ctl_t  *p_dev_ctl)	/* pointer to the device control area */

{

  int rc;		/* return code */



  TRACE_SYS(HKWD_EN3COM_OTHER, "cciB", (ulong)p_dev_ctl, 0, 0);

  VPD.status = VPD_NOT_READ;

  WRK.dma_fair          = TRUE;   /* DMA Fairness - POS 3 Bit 4             */
  WRK.pos_parity        = TRUE;   /* POS Parity enable - POS 2 Bit 7        */
  WRK.fdbk_intr_en      = TRUE; /* Select feedback intr enable - POS 4 bit 4 */

  if (rc = en3com_getvpd(p_dev_ctl)) {
  	TRACE_BOTH(HKWD_EN3COM_ERR, "cci1", rc, 0, 0);
	return(rc);
  }
  
  /* Test if the vital product data actually read ok                        */
  if (VPD.status != VPD_VALID) {

	/* Run the VPD fix routine to see if this helps                     */
	en3com_fixvpd(p_dev_ctl);

	/* if the VPD is not fixed, log error and return error */
	if (VPD.status != VPD_VALID) {
		en3com_logerr(p_dev_ctl, ERRID_EN3COM_UCODE, __LINE__, 
			__FILE__, WRK.vpd_chk_flags, VPD_OK, 0);
  		TRACE_BOTH(HKWD_EN3COM_ERR, "cci2", EIO, 0, 0);
		return(EIO);
	}

  }


  /*
   * Test for the Prototype adapter                                        
   * This is the internal prototype adptr - ABM = 64                  
   */
  if (WRK.card_type == ADPT_10) {

	/* DMA Address Burst Management-POS 5 bit 0&1 */
	WRK.dma_addr_burst  = 3;  

	/* POS parity enable is disabled due to hardware problems           */
	WRK.pos_parity      = FALSE;

	/* Fairness disabled with AT Form Factor cards                      */
	WRK.dma_fair        = FALSE;

  } /* endif test for prototype adapter                                     */

  /*
  * Test for the Original PS2 Form Factor card                             
  * This is a released adapter                                         
  */
  if (WRK.card_type == ADPT_20) {

	/* Required for first version of the PS/2 form factor card           */
	/* DMA Address Burst Management-POS 5 bit 0&1 */
	WRK.dma_addr_burst  = TRUE;

	/* POS parity enable is disabled due to hardware problems            */
	WRK.pos_parity      = FALSE;

	/* Fairness enabled with updated PS2 FF cards                        */
	WRK.dma_fair        = TRUE;

  } /* endif test for original PS2 form factor card                         */

  /*
   * Test for the PS2 Form Factor card                                      
   * This is a released adapter                                         
   */
   if (WRK.card_type == ADPT_22) {

	/* Required for first version of the PS/2 form factor card           */
	/* DMA Address Burst Management-POS 5 bit 0&1 */
	WRK.dma_addr_burst  = TRUE;

	/* POS parity enable is disabled due to hardware problems            */
	WRK.pos_parity      = FALSE;

	/* Fairness enabled with updated PS2 FF cards                        */
	WRK.dma_fair        = TRUE;

  } /* endif test for PS2 form factor card                                  */

  /*
   * Test for the PS2 Form Factor card - parity update                      
   * This is one of the released adapters                                 
   */
   if ((WRK.card_type == ADPT_225)  ||
        (WRK.card_type == ADPT_23)   ||
        (WRK.card_type == ADPT_235)) {

	/* Required for first version of the PS/2 form factor card           */
	/* DMA Address Burst Management-POS 5 bit 0&1 */
	WRK.dma_addr_burst  = TRUE;

	/* POS parity enable is disabled due to hardware problems            */
	WRK.pos_parity      = TRUE;

	/* Fairness enabled with updated PS2 FF cards                        */
	WRK.dma_fair        = TRUE;

  } /* endif test for PS2 form factor card                                  */

  /* Determine which Network Address to use, either DDS or VPD version      */
  if (DDS.use_alt_addr == 0) {

	/* Use the network address that was in the VPD                   */
	COPY_NADR(WRK.vpd_na, WRK.net_addr);
  }
  else {

	/* Use the network address that was passed in the DDS            */
	COPY_NADR(DDS.alt_addr, WRK.net_addr);
  } 

  TRACE_SYS(HKWD_EN3COM_OTHER, "cciE", 0, 0, 0);

  return(0);


}

/*****************************************************************************/
/*
 * NAME:     en3com_getvpd
 *
 * FUNCTION: read and store the Vital Product Data from the adapter
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_init
 *	en3com_fixvpd
 *
 * INPUT:
 *      p_dev_ctl - pointer to the dev_ctl area
 *
 * RETURNS:  
 *	0 - OK
 *	EINVAL - wrong device.
 *	EIO - pio error.
 */
/*****************************************************************************/
en3com_getvpd(
  en3com_dev_ctl_t  *p_dev_ctl)		/* pointer to the dev_ctl area */

{
  int     iocc;
  uchar   tmp_pos1;   /* Temporary byte for pio return value                */
  uchar   tmp_pos2;   /* Temporary byte for pio return value                */
  int     index;      /* Loop Counter                                       */
  int     index2;     /* Loop Counter                                       */
  ushort  cal_crc;    /* Calculated value of CRC                            */
  ushort  vpd_crc;    /* Actual VPD value of CRC                            */
  int	  pio_rc = 0; /* pio exception code 				     */


  TRACE_SYS(HKWD_EN3COM_OTHER, "cgvB", (ulong)p_dev_ctl, 0, 0);

  WRK.vpd_chk_flags = 0;

  /* Get access to the IOCC to access POS registers                         */
  iocc = (int)IOCC_ATT((ulong)DDS.bus_id, (ulong)(IO_IOCC + (DDS.slot << 16)));

  /* Test to verify this is correct adapter type                            */
  ENT_GETPOS(iocc + POS_REG_0, &tmp_pos1);   /* POS 0 = 0xF5  */
  ENT_GETPOS(iocc + POS_REG_1, &tmp_pos2);   /* POS 1 = 0x8E  */
  if ((tmp_pos1 != PR0_VALUE) || (tmp_pos2 != PR1_VALUE)) {
      /* Not the correct adapter type or SLOT # - Leave adapter/SLOT alone   */
  	IOCC_DET(iocc);            /* restore IOCC  */
  	TRACE_BOTH(HKWD_EN3COM_ERR, "cgv1", EINVAL, 0, 0);
	return(EINVAL);
  }

  /* Read pos register two - Save this value to restore later            */
  ENT_GETPOS(iocc + POS_REG_2, &tmp_pos2);

  /* Write pos register two with the parity bit turned off               */
  ENT_PUTPOS(iocc + POS_REG_2, (tmp_pos2 & PR2_PEN_MSK));

  /* Initialize POS Registers 6 & 7 to zero                              */
  ENT_PUTPOS(iocc + POS_REG_6, PR6_VALUE);
  ENT_PUTPOS(iocc + POS_REG_7, PR7_VALUE);

  /*
   * Get VPD from adapter for the default length.
   */

  for (index=0; index < EN3COM_VPD_LENGTH; index++) {
	/* Set up the correct address for the VPD read byte                 */
	ENT_PUTPOS( iocc + POS_REG_6, (index + 1));

        /* Read each byte upto 7 times to get non zero value to help        */
        /* compensate for an adapter card interal bus contention problem.   */
        for (index2=0; index2 < 7; index2++) {
		ENT_GETCX(iocc + POS_REG_3, &VPD.vpd[index]);
		if (VPD.vpd[index] != 0x00)
			break;
        } /* end for loop to keep reading each byte until non zero          */
  } /* end for loop to get all of the VPD data                           */
  
  /* if any PIO operation failed, log error */
  if (pio_rc) {
  	IOCC_DET(iocc);            /* restore IOCC  */
  	TRACE_BOTH(HKWD_EN3COM_ERR, "cgv2", EIO, pio_rc, 0);
	return(EIO);
  }
	

  /* Initialize POS Registers 6 & 7 again  */
  ENT_PUTPOS(iocc + POS_REG_6, PR6_VALUE);
  ENT_PUTPOS(iocc + POS_REG_7, PR7_VALUE);

  /* Restore the original value for POS Register two                     */
  ENT_PUTPOS(iocc + POS_REG_2, tmp_pos2);

  /* Test some of the Fields of Vital Product Data                       */
  if ((VPD.vpd[0] == 'V') &&                  /* 'V' = Hex 56            */
      (VPD.vpd[1] == 'P') &&                  /* 'P' = Hex 50            */
      (VPD.vpd[2] == 'D') &&                  /* 'D' = Hex 44            */
      (VPD.vpd[7] == '*'))  {                 /* '*' = Hex 2A            */

	 WRK.vpd_chk_flags |= VPD_FOUND;

         /* Update the Vital Product Data length                             */
         VPD.length = ((2 * ((VPD.vpd[3] << 8) | VPD.vpd[4])) + 7);

         /* Test for which length will be saved - save the smaller           */
         if (VPD.length > EN3COM_VPD_LENGTH) {
            VPD.length = EN3COM_VPD_LENGTH;

            /* Mismatch on the length - can not test crc - assume crc is good*/
            WRK.vpd_chk_flags |= CRC_VALID;
         }
         else {

            /* Put together the CRC value from the adapter VPD               */
            vpd_crc = ((VPD.vpd[5] << 8) | VPD.vpd[6]);

            /* One can only verify CRC if one had enough space to save it all*/
            /* Verify that the CRC is valid                                  */
            cal_crc = en3com_gen_crc(&VPD.vpd[7],(VPD.length - 7));

            /* Test if the checksum is correct */
            if (vpd_crc == cal_crc)
               WRK.vpd_chk_flags |= CRC_VALID;
         }

         /* Get Network Address and ROS Level and Part Number & EC Number    */
         for (index=0; index < (int)VPD.length; index++) {

            /*****************************************************************/
            /*    Get Network Address                                        */
            /*****************************************************************/
            if ((VPD.vpd[(index + 0)] == '*') &&  /* '*' = Hex 2A            */
                (VPD.vpd[(index + 1)] == 'N') &&  /* 'N' = Hex 4E            */
                (VPD.vpd[(index + 2)] == 'A') &&  /* 'A' = Hex 41            */
                (VPD.vpd[(index + 3)] ==  5 )) {  /*  5  = Hex 05            */

               /* Set the Network Address found flag                         */
               WRK.vpd_chk_flags |= NA_FOUND;

               /* Save Network Address in DDS work section                   */
               for (index2 = 0; index2 < ENT_NADR_LENGTH; index2++) 
                  WRK.vpd_na[index2] = VPD.vpd[(index + 4 + index2)];

            } /* endif test for network address header                       */

            /*****************************************************************/
            /*    Get ROS Level                                              */
            /*****************************************************************/
            /* Test for the ROS Level Header                                 */
            if ((VPD.vpd[(index + 0)] == '*') &&  /* '*' = Hex 2A            */
                (VPD.vpd[(index + 1)] == 'R') &&  /* 'R' = Hex 52            */
                (VPD.vpd[(index + 2)] == 'L') &&  /* 'L' = Hex 4C            */
                (VPD.vpd[(index + 3)] ==  4 )) {  /*  4 >= Hex 04  Now = 4   */

               /* Set the ROS Level found flag                               */
               WRK.vpd_chk_flags |= RL_FOUND;

               /* Set the actual number of ROS ascii bytes                   */
               WRK.vpd_ros_length = ((VPD.vpd[(index + 3)] * 2) - 4);

               /* Save ROS Level in work section                         */
               for (index2 = 0; index2 < ROS_LEVEL_SIZE; index2++)
                  WRK.vpd_rosl[index2] = VPD.vpd[(index + 4 + index2)];

               /* Convert the ASCII Decimal digits to hex                    */
               WRK.vpd_hex_rosl = 0;
               for (index2 = 0; index2 < WRK.vpd_ros_length; index2++) {
                  if ((WRK.vpd_rosl[index2] >= '0') &&
                      (WRK.vpd_rosl[index2] <= '9')) {
                     
                     WRK.vpd_hex_rosl = ((10 * WRK.vpd_hex_rosl) +
                                           (WRK.vpd_rosl[index2] - '0'));
                  }
               }

            } /* endif test for ROS Level header                             */

            /*****************************************************************/
            /*    Get Part Number                                            */
            /*****************************************************************/
            /* Test for the Part Number Header                               */
            if ((VPD.vpd[(index + 0)] == '*') &&  /* '*' = Hex 2A            */
                (VPD.vpd[(index + 1)] == 'P') &&  /* 'P' = Hex 50            */
                (VPD.vpd[(index + 2)] == 'N') &&  /* 'N' = Hex 4E            */
                (VPD.vpd[(index + 3)] ==  6 )) {  /*  3 <= L <= 8  now = 6   */

               /* Set the Part Number found flag                             */
               WRK.vpd_chk_flags |= PN_FOUND;

               /* Save Part Number in DDS work section                       */
               for (index2 = 0; index2 < PN_SIZE; index2++)
                  WRK.vpd_pn[index2] = VPD.vpd[(index + 4 + index2)];
            } /* endif test for Part Number header                           */

            /*****************************************************************/
            /*    Get Engineering Change Number                              */
            /*****************************************************************/
            /* Test for the Engineering Change Number Header                 */
            if ((VPD.vpd[(index + 0)] == '*') &&  /* '*' = Hex 2A            */
                (VPD.vpd[(index + 1)] == 'E') &&  /* 'E' = Hex 45            */
                (VPD.vpd[(index + 2)] == 'C') &&  /* 'C' = Hex 43            */
                (VPD.vpd[(index + 3)] ==  5 )) {  /*  3 <= L <= 8  now = 5   */

               /* Set the EC Number found flag                               */
               WRK.vpd_chk_flags |= EC_FOUND;

               /* Save EC Number in DDS work section                         */
               for (index2 = 0; index2 < EC_SIZE; index2++)
                  WRK.vpd_ec[index2] = VPD.vpd[(index + 4 + index2)];
            } /* endif test for EC Number header                             */

            /*****************************************************************/
            /*    Get Device Driver Level Number                             */
            /*****************************************************************/
            /* Test for the Device Driver Level Header                       */
            if ((VPD.vpd[(index + 0)] == '*') &&  /* '*' = Hex 2A            */
                (VPD.vpd[(index + 1)] == 'D') &&  /* 'D' = Hex 44            */
                (VPD.vpd[(index + 2)] == 'D') &&  /* 'D' = Hex 44            */
                (VPD.vpd[(index + 3)] ==  3 )) {  /*  3 >= Hex 03   now = 3  */

               /* Set the EC Number found flag                               */
               WRK.vpd_chk_flags |= DD_FOUND;

               /* Save DD Number in DDS work section                         */
               for (index2 = 0; index2 < DD_SIZE; index2++)
                  WRK.vpd_dd[index2] = VPD.vpd[(index + 4 + index2)];

               /* Convert the ASCII Decimal digits to hex                    */
               WRK.vpd_hex_dd = 0;
               for (index2 = 0; index2 < DD_SIZE; index2++)
               {
                  /* Test for ascii decimal digits                           */
                  if ((WRK.vpd_dd[index2] >= '0') &&
                      (WRK.vpd_dd[index2] <= '9')) {
                     WRK.vpd_hex_dd = ((10 * WRK.vpd_hex_dd)
                                          + (WRK.vpd_dd[index2] - '0'));
                  }
               } /* end for loop for converting ASCII Decimal to hex         */
            } /* endif test for DD Number header                             */
         } /* end for loop for getting VPD fields: NA RL PN EC DD            */

         /* Test the appropriate flags to verify everything is valid         */
         if ((WRK.vpd_chk_flags == VPD_OK) &&        
             /* Network address does not have the multicast bit on           */
             ((WRK.vpd_na[0] & MULTI_BIT_MASK) != MULTI_BIT_MASK)) {

            /* VPD is valid based on the tests we know to check              */
            VPD.status = VPD_VALID;

         }
         else {
            /* VPD failed the test - set the status                          */
            VPD.status = VPD_INVALID;

         } /* endif for test if VPD is valid                                 */
  }
  else {  /* Bad Vital Product Data                       */

         VPD.status = VPD_INVALID;

  } /* endif test of some of the VPD fields                              */

  /* restore IOCC to previous value - done accessing POS Regs               */
  IOCC_DET(iocc);            /* restore IOCC                                */

  TRACE_DBG(HKWD_EN3COM_OTHER, "cgv3", VPD.status, WRK.vpd_chk_flags, 0);

  /* Test if this adapter is the internal Prototype adapter for one of      */
  /* several hardware work arounds: POS Parity Enable, DMA Word boundary.   */
  /* Look specifically for PROT0ETH or 022F9381 or 071F0927 or 071f1152     */
  /* or 071F1182 or 071F1183                                                */

  /* Set the default card type value to unknown                             */
  WRK.card_type = ADPT_LATEST;

  /* Test if this VPD has the Prototype value                               */
  if ((WRK.vpd_pn[0] == 'P') &&
      (WRK.vpd_pn[1] == 'R') &&
      (WRK.vpd_pn[2] == 'O') &&
      (WRK.vpd_pn[3] == 'T') &&
      (WRK.vpd_pn[4] == '0') &&
      (WRK.vpd_pn[5] == 'E') &&
      (WRK.vpd_pn[6] == 'T') &&
      (WRK.vpd_pn[7] == 'H') &&
      (VPD.status == VPD_VALID)) {

      /* This adapter is the ethernet prototype adapter                      */
      WRK.card_type = ADPT_10;
  }

  /* Test if this VPD has the 022F9381 value                                */
  if ((WRK.vpd_pn[0] == '0') &&
      (WRK.vpd_pn[1] == '2') &&
      (WRK.vpd_pn[2] == '2') &&
      (WRK.vpd_pn[3] == 'F') &&
      (WRK.vpd_pn[4] == '9') &&
      (WRK.vpd_pn[5] == '3') &&
      (WRK.vpd_pn[6] == '8') &&
      (WRK.vpd_pn[7] == '1') &&
      (VPD.status == VPD_VALID)) {
  
      /* This adapter is the original PS2 adapter for release 1              */
      WRK.card_type = ADPT_10;
  }

  /* Test if this VPD has the 058F2881 value                                */
  if ((WRK.vpd_pn[0] == '0') &&
      (WRK.vpd_pn[1] == '5') &&
      (WRK.vpd_pn[2] == '8') &&
      (WRK.vpd_pn[3] == 'F') &&
      (WRK.vpd_pn[4] == '2') &&
      (WRK.vpd_pn[5] == '8') &&
      (WRK.vpd_pn[6] == '8') &&
      (WRK.vpd_pn[7] == '1') &&
      (VPD.status == VPD_VALID)) {
  
      /* This adapter is the ethernet prototype adapter                      */
      WRK.card_type = ADPT_20;
  }

  /* Test if this VPD has the 071F0927 value                                */
  if ((WRK.vpd_pn[0] == '0') &&
      (WRK.vpd_pn[1] == '7') &&
      (WRK.vpd_pn[2] == '1') &&
      (WRK.vpd_pn[3] == 'F') &&
      (WRK.vpd_pn[4] == '0') &&
      (WRK.vpd_pn[5] == '9') &&
      (WRK.vpd_pn[6] == '2') &&
      (WRK.vpd_pn[7] == '7') &&
      (VPD.status == VPD_VALID)) {
  
      /* This adapter is the ethernet prototype adapter                      */
      WRK.card_type = ADPT_22;
  }

  /* Test if this VPD has the 071F1152 value                                */
  if ((WRK.vpd_pn[0] == '0') &&
      (WRK.vpd_pn[1] == '7') &&
      (WRK.vpd_pn[2] == '1') &&
      (WRK.vpd_pn[3] == 'F') &&
      (WRK.vpd_pn[4] == '1') &&
      (WRK.vpd_pn[5] == '1') &&
      (WRK.vpd_pn[6] == '5') &&
      (WRK.vpd_pn[7] == '2') &&
      (VPD.status == VPD_VALID)) {
  
      /* This adapter is the ethernet PS2 with embedded fixes                */
      WRK.card_type = ADPT_225;
  }

  /* Test if this VPD has the 071F1182 value                                */
  if ((WRK.vpd_pn[0] == '0') &&
      (WRK.vpd_pn[1] == '7') &&
      (WRK.vpd_pn[2] == '1') &&
      (WRK.vpd_pn[3] == 'F') &&
      (WRK.vpd_pn[4] == '1') &&
      (WRK.vpd_pn[5] == '1') &&
      (WRK.vpd_pn[6] == '8') &&
      (WRK.vpd_pn[7] == '2') &&
      (VPD.status == VPD_VALID)) {
  
      /* This adapter is the ethernet PS2 with embedded fixes                */
      WRK.card_type = ADPT_23;
  }

  /* Test if this VPD has the 071F1183 value                                */
  if ((WRK.vpd_pn[0] == '0') &&
      (WRK.vpd_pn[1] == '7') &&
      (WRK.vpd_pn[2] == '1') &&
      (WRK.vpd_pn[3] == 'F') &&
      (WRK.vpd_pn[4] == '1') &&
      (WRK.vpd_pn[5] == '1') &&
      (WRK.vpd_pn[6] == '8') &&
      (WRK.vpd_pn[7] == '3') &&
      (VPD.status == VPD_VALID)) {
  
      /* This adapter is the ethernet PS2 with all fixes embedded            */
      WRK.card_type = ADPT_235;
  }

  /* Test if this VPD has an unknown part number - ASSUME! version 3 card   */
  if ((WRK.card_type == ADPT_LATEST) && (VPD.status == VPD_VALID)) {
      /* This adapter is the follow on PS2 adapter for release 1+            */
      WRK.card_type = ADPT_30;
  }

  TRACE_SYS(HKWD_EN3COM_OTHER, "cgvE", VPD.status, WRK.card_type, 0);
  return(0);

} 

/*****************************************************************************/
/*
 * NAME:     en3com_fixvpd
 *
 * FUNCTION: Work around adapter timing problem in order to get valid VPD.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *                                                                           
 *    Sequence: 1) Set POS for a valid adapter configuration.                
 *              2) Force Hard reset to adapter.                             
 *              3) Loop read of command reg. to get execute mailbox address.
 *              4) Read and store adapter configuration.                   
 *              5) Verify firmware version equals 8. If not close down & exit
 *              6) Issue special command to try up on card processor.       
 *              7) Read the stable version of VPD.                         
 *              8) Free up special command.                               
 *              9) Disable adapter.                                      
 *	       10) Return.
 *
 * CALLED FROM:
 *      en3com_init
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the dev_ctl area.
 *
 * RETURNS:  
 *	none.
 */
/*****************************************************************************/
void
en3com_fixvpd(
	en3com_dev_ctl_t  *p_dev_ctl) 	/* pointer to the dev_ctl area  */

{
   int    bus, ioa;
   int    i;       	/* Loop Counter                                       */
   ushort tempsh;
   int	  pio_rc = 0;	/* pio exception code */



  TRACE_SYS(HKWD_EN3COM_OTHER, "cfvB", (ulong)p_dev_ctl, 0, 0);

  if (en3com_start(p_dev_ctl, FALSE)) {
  	TRACE_BOTH(HKWD_EN3COM_ERR, "cfv1", 0, 0, 0);
	return;
  }

  /* Test if the version has the special execute command                    */
  if (WRK.version_num < 0x0008) {
  	TRACE_BOTH(HKWD_EN3COM_ERR, "cfv2", 0, 0, 0);
      	return;
  }

  /* Get access to the I/O bus to access I/O registers                      */
  ioa = (int)BUSIO_ATT((ulong)DDS.bus_id, DDS.io_port);
  bus = (int)BUSMEM_ATT((ulong)DDS.bus_id, DDS.bus_mem_addr);

  /* Set up the special execute mailbox  command                            */
  /* Load the execute mailbox with the command to be executed               */
  ENT_PUTSRX( bus + WRK.exec_mail_box + 0x00, (short)PAUSE_ADPT);
                                                
  /* Turn all bits off - the processor will turn then on                    */
  ENT_PUTSX( bus + WRK.exec_mail_box + 0x02, 0 );

  /* Issue the execute command to the command register                      */
  ENT_PUTCX( ioa + COMMAND_REG, EXECUTE_MSK );

  /* Loop for 1 second waiting for execute command to be run                */
  for (i = 0; i < 1000; i++) {
  
	/* Test if the 2nd word changed from 0 to 0xFFFF                    */
	ENT_GETSX( bus + WRK.exec_mail_box + 2, &tempsh );
	if (tempsh == 0xFFFF)
		break;
	DELAYMS(1);	/* delay 1 milisecond */

  } 

  /* restore IO bus to previous value, done accessing I/O Regs              */
  BUSMEM_DET(bus);
  BUSIO_DET(ioa);

  if (pio_rc) {
  	TRACE_BOTH(HKWD_EN3COM_ERR, "cfv3", pio_rc, 0, 0);
	return;
  }


  /* Test if the execute command actually started working                   */
  if (i > 998) {
  	TRACE_BOTH(HKWD_EN3COM_ERR, "cfv4", 0, 0, 0);
	return;
  }

  /* Read the Vital Product Data again */
  en3com_getvpd(p_dev_ctl);

  /*
   * finish up this fix by disabling the adapter
   */
  en3com_stop(p_dev_ctl);

  TRACE_SYS(HKWD_EN3COM_OTHER, "cfvE", 0, 0, 0);


} 
/*****************************************************************************/
/*
 * NAME:     en3com_start
 *
 * FUNCTION: Initialize and activate the adapter.    
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_open
 *	en3com_fixvpd
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the dev_ctl area.
 *
 * RETURNS:  
 *	0 - OK
 *	EIO - PIO error occurred during the start
 * 	ENOCONNECT - adapter error occurred during the start 
 */
/*****************************************************************************/
en3com_start(
  en3com_dev_ctl_t	*p_dev_ctl)	/* pointer to the dev_ctl area */

{

  int    bus, ioa;
  int    i, j;   	/* Loop Counter */
  uchar host_status_reg;
  uchar host_command_reg;
  int	pio_rc = 0;	/* pio exception code */
  int   ipri;


  TRACE_SYS(HKWD_EN3COM_OTHER, "AstB", (ulong)p_dev_ctl, 0, 0);

  /* Disable interrupt */
  ipri = disable_lock(PL_IMP, &SLIH_LOCK);

  /* Get access to the I/O bus to access I/O registers                      */
  bus = (int)BUSMEM_ATT((ulong)DDS.bus_id, DDS.bus_mem_addr);
  ioa = (int)BUSIO_ATT((ulong)DDS.bus_id, DDS.io_port);

  /* Set POS registers. Enable the card */
  if (en3com_setpos(p_dev_ctl)) {
	BUSIO_DET(ioa);
	BUSMEM_DET(bus);

  	unlock_enable(ipri, &SLIH_LOCK);

 	TRACE_BOTH(HKWD_EN3COM_ERR, "Ast1", ENOCONNECT, 0, 0);
	return(ENOCONNECT);
  }

  /*
   * Clear any possible pending interrupt left from power up,
   * such as parity error.
   */
  ENT_PUTCX(ioa + STATUS_REG, 0);

  unlock_enable(ipri, &SLIH_LOCK);

  /* save the device start time for statistics */
  p_dev_ctl->dev_stime = lbolt;

  /* Hard Reset the adapter to force a known state                          */
  ENT_PUTCX(ioa + CONTROL_REG, HARD_RST_MSK);

  /* Wait for the adapter to know that the hard reset has been set          */
  DELAYMS(10);         /* delay 10 milliseconds               */

  /* reset the control register */
  ENT_PUTCX(ioa + CONTROL_REG, CONTROL_VALUE);

  if (pio_rc) {
	BUSIO_DET(ioa);
	BUSMEM_DET(bus);

  	TRACE_BOTH(HKWD_EN3COM_ERR, "Ast2", EIO, pio_rc, 0);
	return(EIO);
  }

  /* Wait for the adapter to fully reset                                    */
  DELAYMS(1000);           /* delay 1 second                    */

  /* Force RAM page register to the default value                           */
  ENT_PUTCX(ioa + RAM_PAGE_REG, RAM_PAGE_VALUE);

  /* perform I/O parity checking */
  ENT_PUTCX( ioa + PARITY_REG, PAREN_MSK);

  if (pio_rc) {
	BUSIO_DET(ioa);
	BUSMEM_DET(bus);

  	TRACE_BOTH(HKWD_EN3COM_ERR, "Ast3", EIO, pio_rc, 0);
	return(EIO);
  }

  /*
   * Loop for 1 second waiting for hex 00 to be returned for 
   * acknowleging the self test status
   */
  for (i = 0; i < 1000; i++)  {

      /* get the interrupt status register for this card                     */
      ENT_GETCX(ioa + STATUS_REG, &host_status_reg);

      /* Test if command word received interrupt has occurred                */
      if (host_status_reg & CWR_MSK) {

         /* get the host command register for this card                  */
         ENT_GETCX(ioa + COMMAND_REG, &host_command_reg);

         /* Test if the host command register read zero                      */
         if (host_command_reg == SELF_TESTS_OK)
		break; 
      }
      DELAYMS(1);
  } 

  if (pio_rc) {
	BUSIO_DET(ioa);
	BUSMEM_DET(bus);

  	TRACE_BOTH(HKWD_EN3COM_ERR, "Ast4", EIO, pio_rc, 0);
	return(EIO);
  }

  if (host_command_reg != SELF_TESTS_OK) {
  
	BUSIO_DET(ioa);
	BUSMEM_DET(bus);

	en3com_logerr(p_dev_ctl, ERRID_EN3COM_FAIL, __LINE__, __FILE__, 
		(ulong)host_command_reg, 0, 0);

 	TRACE_BOTH(HKWD_EN3COM_ERR, "Ast5", ENOCONNECT, host_command_reg, 0);
	return(ENOCONNECT);
  }

  /* Get the 4 bytes execute mailbox offset  */
  /* Loop for 1 second per byte waiting for execute mailbox to be return    */
  WRK.exec_mail_box = 0;
  for (i=0; i < 4; i++) {
  
      /* Loop for 1 second waiting for execute command to be run             */
      for (j = 0; j < 1000; j++) {

         /* get the interrupt status register for this card                  */
         ENT_GETCX(ioa + STATUS_REG, &host_status_reg);

         /* Test if command word received interrupt has occurred             */
         if (host_status_reg & CWR_MSK) {

            /* get the host command register for this card               */
            ENT_GETCX(ioa + COMMAND_REG, &host_command_reg);

            /* Place the address byte in the proper location                 */
            WRK.exec_mail_box |= (host_command_reg << (i * 8));

            break; 

         } 
         DELAYMS(1);
      } 
      if (j >= 1000)  /* timeout */
	 break;
		
  } 

  if (pio_rc) {
	BUSIO_DET(ioa);
	BUSMEM_DET(bus);

  	TRACE_BOTH(HKWD_EN3COM_ERR, "Ast6", EIO, pio_rc, 0);
	return(EIO);
  }

  /* if didn't get the 4 bytes in time, fail the start */
  if (i < 4) {
	BUSIO_DET(ioa);
	BUSMEM_DET(bus);

	en3com_logerr(p_dev_ctl, ERRID_EN3COM_FAIL, __LINE__, __FILE__, 
		(ulong)host_command_reg, 0, 0);

  	TRACE_BOTH(HKWD_EN3COM_ERR, "Ast7", ENOCONNECT, 0, 0);
	return(ENOCONNECT);
  }

  /* get and save the adapter initial configuration information */
  if (en3com_getcfg(p_dev_ctl, bus)) {

	BUSIO_DET(ioa);
	BUSMEM_DET(bus);

	en3com_logerr(p_dev_ctl, ERRID_EN3COM_FAIL, __LINE__, __FILE__,
		(ulong)host_command_reg, 0, 0);

 	TRACE_BOTH(HKWD_EN3COM_ERR, "Ast8", ENOCONNECT, 0, 0);
        return(ENOCONNECT);
  }

  BUSIO_DET(ioa);
  BUSMEM_DET(bus);
  TRACE_SYS(HKWD_EN3COM_OTHER, "AstE", 0, 0, 0);

  return(0);

   
	
}


/*****************************************************************************/
/*
 * NAME:     en3com_setpos
 *
 * FUNCTION: Take the DDS information and set the POS regesters to 
 * 	     proper configure the adapter. Enable the adapter.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *	en3com_start
 *
 * INPUT:
 *      p_dev_ctl       - pointers to the dev_ctl area.
 *
 * RETURNS:  0 - OK
 *	     EIO - pio error occurred
 */
/*****************************************************************************/
en3com_setpos(
  en3com_dev_ctl_t	*p_dev_ctl)	/* pointer to the dev_ctl area */

{

  int    iocc;
  int    pos_index;
  uchar  temp;
  int    pio_rc = 0;



  TRACE_SYS(HKWD_EN3COM_OTHER, "cspB", (ulong)p_dev_ctl, 0, 0);

  /* Load the POS array based on known Adapter Defaults    */
  WRK.pos_reg[POS_REG_0] = PR0_VALUE;   /* Default Value =  Hex F5          */
  WRK.pos_reg[POS_REG_1] = PR1_VALUE;   /* Default Value =  Hex 8E          */
  WRK.pos_reg[POS_REG_6] = PR6_VALUE;   /* Default Value =  Hex 00          */
  WRK.pos_reg[POS_REG_7] = PR7_VALUE;   /* Default Value =  Hex 00          */

  /**************************************************************************/
  /* Configure POS reg  2 - I/O Port, RAM Address, POS Parity & Card enable */
  /* Test if POS Parity enable should be turned on                          */
  /* The AT form factor card must not have POS Parity enable bit turned on  */
  /* due to a hardware failure. Leave disabled if unknown.                  */
  /**************************************************************************/
  temp =       PR2_CDEN_MSK;                        /* card enable    */

  if ((WRK.pos_parity) && (VPD.status == VPD_VALID)) {
	temp |= PR2_PEN_MSK;            /* OR in POS Parity enable bit      */
  }

  /**************************************************************************/
  /* I/O Address = 7280, 7290, 7680, 7690, 7A80, 7A90, 7E80 & 7E90          */
  /**************************************************************************/
  temp |= (((ulong)(DDS.io_port) & 0x00000010) >> 3) |
            (((ulong)(DDS.io_port) & 0x00000C00) >> 8);

  /**************************************************************************/
  /* RAM Address = C0000, C4000, C8000, CC000, D0000, D4000, D8000 & DC000  */
  /**************************************************************************/
  temp |= (((ulong)(DDS.bus_mem_addr) & 0x0001C000) >> 10); 
  WRK.pos_reg[POS_REG_2] = temp;

  /**************************************************************************/
  /* Configure POS reg 3 - DMA Level and Fairness                           */
  /**************************************************************************/
  temp = DDS.dma_arbit_lvl & PR3_DMA_MSK;           /* Assign DMA Level     */
  if (WRK.dma_fair) 
	temp |= PR3_FAIR_MSK;           /* OR in DMA Fair enable*/
  WRK.pos_reg[POS_REG_3] = temp;

  /**************************************************************************/
  /* Configure POS reg 4 - Start Enable & BNC/DIX Transceiver select        */
  /**************************************************************************/
  temp = PR4_WDEN_MSK;                              /* Memory Enable        */
  if (DDS.bnc_select) 
	temp |= PR4_BNC_MSK;  			/* Assign BNC select    */
  if (WRK.card_type == ADPT_30)
	temp |= PR4_PAREN_MSK;                  /* Adapter Parity Enable*/
  if ((WRK.card_type == ADPT_30) && WRK.fdbk_intr_en)
	temp |= PR4_FDBK_MSK;                   /* Feedback INTR Enable */
  WRK.pos_reg[POS_REG_4] = temp;

  /**************************************************************************/
  /* Configure POS reg 5 - Window Size, Interrupt level & ABM Mode          */
  /* ABM Mode = 0x00 - disabled, 0x01 - 16, 0x02 - 32 & 0x03 - 64           */
  /**************************************************************************/
  temp = (WRK.dma_addr_burst & 0x00000003);     /* OR in ABM Mode           */

  /**************************************************************************/
  /* Interrupt Level = 0x00 - 9, 0x01 - 10, 0x02 - 11 & 0x03 - 12           */
  /**************************************************************************/
  temp |= (((DDS.intr_level & 0x0000000F) - 9) << 2);      

  /**************************************************************************/
  /* RAM Window Size = 0x00 - 16K, 0x01 - 32K, 0x02 - 64K & 0x03 - 128K     */
  /**************************************************************************/
  temp |= ((DDS.bus_mem_size & 0x00000003) << 4);      
  /* Turn off Channel Check and save POS Reg 5                              */
  WRK.pos_reg[POS_REG_5] = temp | PR5_CHCK_MSK | PR5_BIT_6;

  /**************************************************************************/
  /*  Start writing the Data to the Adapter's POS Registers                 */
  /**************************************************************************/

  /* Get access to the IOCC to access POS registers                         */
  iocc = (int)IOCC_ATT((ulong)DDS.bus_id, (ulong)(IO_IOCC + (DDS.slot << 16)));

  /* Update POS registers 6 & 7 before POS register 3 is updated         */
  ENT_PUTPOS( iocc + POS_REG_6, WRK.pos_reg[POS_REG_6] );
  ENT_PUTPOS( iocc + POS_REG_7, WRK.pos_reg[POS_REG_7] );

  TRACE_DBG(HKWD_EN3COM_ERR, "csp1", pio_rc, 0, 0);

  /* Update POS registers 2 to 5                                         */
  for (pos_index=POS_REG_2; pos_index <= POS_REG_5; pos_index++) {
        ENT_PUTPOS( iocc + pos_index, WRK.pos_reg[pos_index] );

  	TRACE_DBG(HKWD_EN3COM_ERR, "csp2", pio_rc, pos_index, 0);
  }

  /* Read the current values in POS Registers 0 to 7 and save them       */
  for (pos_index=POS_REG_0; pos_index <= POS_REG_7; pos_index++) {
        ENT_GETPOS(iocc + pos_index, &WRK.pos_reg[pos_index]);

  	TRACE_DBG(HKWD_EN3COM_ERR, "csp3", pio_rc, pos_index, 0);
  }

  /* Perform dummy read to allow adapter gate array problem to subside      */
  ENT_GETCX(((ulong)iocc | (ulong)0x000f0000), &WRK.gate_array_fix);

  /* restore IOCC to previous value - done accessing POS Regs               */
  IOCC_DET(iocc);                /* restore IOCC                            */

  if (pio_rc) {
  	TRACE_BOTH(HKWD_EN3COM_ERR, "csp4", EIO, pio_rc, 0);
	return(EIO);
  }

  TRACE_SYS(HKWD_EN3COM_OTHER, "cspE", 0, 0, 0);
  return(0);

} 


/*****************************************************************************/
/*
 * NAME:     en3com_getcfg
 *
 * FUNCTION: Get and store adapter internal configuration.
 * 	     This routine requires the caller to get the bus access.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *	en3com_start
 *	en3com_setup
 *	en3com_stimer
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the dev_ctl area.
 *      bus             - the address of bus memroy attached 
 *
 * RETURNS:  0 - OK
 *	     ENOCONNECT - configuration parameter verification failed.
 *	     EIO - pio error occurred
 */
/*****************************************************************************/
en3com_getcfg(
	en3com_dev_ctl_t  *p_dev_ctl,	/* pointer to the dev_ctl area */
	int 		bus)		/* address of bus memory attached */

{
  ushort   temp_off;              /* Temp variable, execute mail box offset */
  int    pio_rc = 0;		  /* pio exception code */


  TRACE_SYS(HKWD_EN3COM_OTHER, "cgcB", (ulong)p_dev_ctl, bus, 0);

  /* Get from the adapter the Base/main offset - allow for byte swapping    */
  ENT_GETLRX(bus + WRK.exec_mail_box + 2, &WRK.main_offset);

  /* Get from the adapter the Receive Mail Box - allow for byte swapping    */
  ENT_GETSRX(bus + WRK.exec_mail_box + 6, &temp_off);

  /* Add in the main offset to get the final offset                         */
  WRK.rv_mail_box = WRK.main_offset + temp_off;

  /* Get from the adapter the Transmit Mail Box - allow for byte swapping   */
  ENT_GETSRX(bus + WRK.exec_mail_box + 8, &temp_off);

  /* Add in the main offset to get the final offset                         */
  WRK.tx_mail_box = WRK.main_offset + temp_off;

  /* Get from the adapter the execute  Mail Box - allow for byte swapping   */
  ENT_GETSRX(bus + WRK.exec_mail_box + 10, &temp_off);

  /* Add in the main offset to get the final offset                         */
  temp_off += WRK.main_offset;

  /* Test to verify that the adapter gave us the right data                 */
  if (temp_off != WRK.exec_mail_box) {
	en3com_logerr(p_dev_ctl, ERRID_EN3COM_UCODE, __LINE__, __FILE__,
		(ulong)temp_off, (ulong)WRK.exec_mail_box, 0);
  	TRACE_BOTH(HKWD_EN3COM_ERR, "cgc1", temp_off, WRK.exec_mail_box, 0);
  	return (ENOCONNECT);
  }

  /* Get the Statistics Counters Offset - allow for byte swapping           */
  ENT_GETSRX(bus + WRK.exec_mail_box + 12, &temp_off);

  /* Add in the main offset to get the final offset                         */
  WRK.stat_count_off = WRK.main_offset + temp_off;

  /* Get the Total adapter RAM size     - allow for byte swapping           */
  ENT_GETSRX(bus + WRK.exec_mail_box + 14, &WRK.adpt_ram_size);

  /* Test that the adapter thinks it has some functional RAM                */
  if ((WRK.adpt_ram_size == 0) || (WRK.adpt_ram_size > RAM_SIZE)) {
	en3com_logerr(p_dev_ctl, ERRID_EN3COM_UCODE, __LINE__, __FILE__,
		(ulong)WRK.adpt_ram_size, RAM_SIZE, 0);
  	TRACE_BOTH(HKWD_EN3COM_ERR, "cgc2", WRK.adpt_ram_size, 0, 0);
  	return (ENOCONNECT);
  }

  /* Get the Buffer Descriptor Region size - allow for byte swapping        */
  ENT_GETSRX(bus + WRK.exec_mail_box + 16, &WRK.buf_des_reg_size);

  /* Get the Transmit List Start Offset    - allow for byte swapping        */
  ENT_GETSRX(bus + WRK.exec_mail_box + 18, &temp_off);

  /* Add in the main offset to get the final offset                         */
  WRK.tx_list_off = WRK.main_offset + temp_off;

  /* Get the Transmit List count           - allow for byte swapping        */
  ENT_GETSRX(bus + WRK.exec_mail_box + 20, &WRK.tx_list_cnt);

  /* Get the receive  List Start Offset    - allow for byte swapping        */
  ENT_GETSRX(bus + WRK.exec_mail_box + 22, &temp_off);

  /* Add in the main offset to get the final offset                         */
  WRK.rv_list_off = WRK.main_offset + temp_off;

  /* Get the Receive  List count           - allow for byte swapping        */
  ENT_GETSRX(bus + WRK.exec_mail_box + 24, &WRK.rv_list_cnt);

  /* Get the Firmware Version number       - allow for byte swapping        */
  ENT_GETSRX(bus + WRK.exec_mail_box + 26, &WRK.version_num);

  if (pio_rc) {
  	TRACE_BOTH(HKWD_EN3COM_ERR, "cgc3", EIO, pio_rc, 0);
	return(EIO);
  }

  TRACE_SYS(HKWD_EN3COM_OTHER, "cgcE", 0, 0, 0);
  return (0);

} 
