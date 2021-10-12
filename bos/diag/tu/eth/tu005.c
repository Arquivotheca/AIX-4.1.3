static char sccsid[] = "src/bos/diag/tu/eth/tu005.c, tu_eth, bos411, 9428A410j 6/19/91 14:56:06";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: vpd_test, tu005
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
 */
/*****************************************************************************

Function(s) Test Unit 005 - Vital Product Data (VPD) Test

Module Name :  tu005.c
SCCS ID     :  1.10

Current Date:  12/13/90, 07:47:34
Newest Delta:  5/29/90, 08:54:52

*****************************************************************************/
#include <stdio.h>
#include <errno.h>
#include "ethtst.h"

#define VPD_MAX_SIZE	0x104      /* maxium length of VPD PROM */
#define ROS_MAX_LENGTH	10

#define ROS_START	0x13

#ifdef debugg
extern void detrace();
#endif

/*****************************************************************************

check_ros

Function compares "firmware level" from config after reset with the
ROS level in the VPD.

*****************************************************************************/

int check_ros (vpd, tucb_ptr)
   unsigned char vpd[];  
   TUTYPE *tucb_ptr;
   {
	unsigned char *ucp;
	unsigned char ros_string[ROS_MAX_LENGTH + 1];
	int ros_length;
	unsigned long ros_ulong;

	if ((vpd[ROS_START] == 'R') &&
	    (vpd[ROS_START + 1] == 'L'))
	   {
		ucp = &vpd[ROS_START + 3];
		ros_length = 0;
		while ((*ucp != '*') && (ros_length < ROS_MAX_LENGTH))
		   {
			ros_string[ros_length++] = *ucp;
			ucp++;
		   }
		if (ros_length >= ROS_MAX_LENGTH)
			return(ROS_LEN_ERR);
		
		ros_string[ros_length] = '\0';
		ros_ulong = (unsigned long) atol(ros_string);
		if (ros_ulong != tucb_ptr->eth_htx_s.config_table[13])
			return(VPD_ROS_ERR);

	   }
	else
		return(ROS_NF_ERR);
	return(0);
   }

/*****************************************************************************

vpd_test

Function reads the VPD into the array vpd.  This is accomplished by writing
the offsets into the VPD in POS registers 6 and 7 and then performing 
reads of POS register 3.  Once read into the array, function calls
"crc_gen" to calculate the Cyclic Redundancy Checksum (CRC) on the VPD and
compares it to the one written in the adapter.

*****************************************************************************/

int vpd_test (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int i;              /* index */
	int rc;
	unsigned int    vpd_size;
	unsigned int    vpd_data_len;   /* from addr 8 to the end of VPD */
	unsigned int    vpd_crc;        /* crc stored in VPD byte 6 & 7 */
	unsigned int    calc_crc;
	unsigned char sval;
	unsigned char   vpd[VPD_MAX_SIZE];
	unsigned long   status;

	extern unsigned char hio_parity_rd();
	extern int hio_parity_wr();
	extern unsigned char pos_rd();	/* reads value from POS register */
	extern int pos_wr();		/* writes value to POS register */
	extern int crc_gen();		/* CRC calculator */
	extern int mktu_rc();
	extern int set_pause();
	extern int set_resume();
	extern int hard_reset();

	/*
	 * get everything into known state
	 * by doing a hard_reset() first
	 */
	if (rc = hard_reset(fdes, tucb_ptr))
	   {
		rc &= 0x0000ffff;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
	   }
	
        /*
	 * Make sure parity generation is DISABLED
	 * before accessing VPD.
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
	 * Check firmware version.  If > 7, then card needs
	 * PAUSE/RESUME of adapter cpu to read VPD.  Else,
	 * card doesn't!
	 */
	if (tucb_ptr->eth_htx_s.config_table[13] > 7)
	   {
	
		/*
		 * send PAUSE command to execute mailbox -
		 * temporary fix for gate array problem.  Will
		 * be possibly fixed in next pass of silicon so
		 * may get to remove this in the future....
		 */
		if (rc = set_pause(fdes, tucb_ptr))
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
	   }

	rc = 0;
	vpd[0] = 0x0;           /* vpd[0] is a dummy, it will not be used */

	/*
	 * Read the 1st byte to the 7th byte of VPD.
	 */
	for (i = 1; i < 8; i++)
	   {
		rc = pos_wr(fdes, 6, i, &status, tucb_ptr);
	        if (rc)
		   {
			if (tucb_ptr->eth_htx_s.config_table[13] > 7)
				(void) set_resume(fdes, tucb_ptr);
	        	return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				POS6_WR_ERR));
		   }

		rc = pos_wr(fdes, 7, 0, &status, tucb_ptr);
		if (rc)
		   {
			if (tucb_ptr->eth_htx_s.config_table[13] > 7)
				(void) set_resume(fdes, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				POS7_WR_ERR));
		   }

		vpd[i] = pos_rd(fdes, 3, &status, tucb_ptr);
		if (status != 0)
		   {
			if (tucb_ptr->eth_htx_s.config_table[13] > 7)
				(void) set_resume(fdes, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				POS3_RD_ERR));
		   }

#ifdef debugg
		detrace(0,"\nvpd[%d]=%d",i,vpd[i]);
#endif

	   } /* endfor */
	
	/*
	 * form CRC by combining MSB and LSB from VPD.
	 */
	vpd_crc = (0x100 * vpd[6]) + vpd[7];

	/* If the first 3 bytes is not 'VPD', declare test failure and exit */

	if ((vpd[1] == 'V') && (vpd[2] == 'P') && (vpd[3] == 'D') &&
		(vpd[4] == 0))
	   {
		/*
		 * get the length specified in the VPD.  Note that
		 * the designers of VPD placed the length divided by 2
		 * (thus the number of "words"), so we multiply by 2
		 * to get the total number of bytes.  Also note that
		 * we do NOT need to look at the MSB of the length field
		 * (vpd[4]) for the calculation of the length (i.e.
		 * only vpd[5] - the LSB) since it is known that it
		 * will be zero since the length is rather short.
		 */
		vpd_data_len = 2 * vpd[5];

		/*
		 * Recall that vpd_data_len reflects the number of bytes
		 * within the data portion of the VPD while vpd_size
		 * reflects the TOTAL size of the VPD - i.e. the number
		 * of bytes of the data portion PLUS the number of bytes
		 * holding "VPD", the length specification, and CRC.
		 */
		vpd_size = vpd_data_len + 8;

#ifdef debugg
		detrace(0,"\nthe vpd_data_len=%d, and the vpd_size=%d",
				vpd_data_len,vpd_size);
#endif
		for (i = 8; i <= vpd_size; i++)
		   {
			if (pos_wr(fdes, 6, i, &status, tucb_ptr))
			   {
				if (tucb_ptr->eth_htx_s.config_table[13] > 7)
					(void) set_resume(fdes, tucb_ptr);
				return(mktu_rc(tucb_ptr->header.tu,
					LOG_ERR, POS6_WR_ERR));
			   }
			if (pos_wr(fdes, 7, 0, &status, tucb_ptr))
			   {
				if (tucb_ptr->eth_htx_s.config_table[13] > 7)
					(void) set_resume(fdes, tucb_ptr);
				return(mktu_rc(tucb_ptr->header.tu,
					LOG_ERR, POS7_WR_ERR));
			   }
			vpd[i] = pos_rd(fdes, 3, &status, tucb_ptr);
			if (status != 0)
			   {
				if (tucb_ptr->eth_htx_s.config_table[13] > 7)
					(void) set_resume(fdes, tucb_ptr);
				return(mktu_rc(tucb_ptr->header.tu,
					LOG_ERR, POS3_RD_ERR));
			   }

#ifdef debugg
			detrace(0,"\nthe VPD [%x] is %x",i,vpd[i]);
#endif
		   }
		calc_crc = crc_gen(&vpd[8], vpd_data_len);
#ifdef debugg
		detrace(0,"\nVPD  crc is %08X\n",vpd_crc);
		detrace(0,"\nCalc crc is %08X\n",calc_crc);
#endif
		if (vpd_crc != calc_crc)
		   {
			if (tucb_ptr->eth_htx_s.config_table[13] > 7)
				(void) set_resume(fdes, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				VPD_CRC_ERR));
		   }
		
		if (rc = check_ros(vpd, tucb_ptr))
		   {
			if (tucb_ptr->eth_htx_s.config_table[13] > 7)
				(void) set_resume(fdes, tucb_ptr);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				rc));
		   }


		if (tucb_ptr->eth_htx_s.config_table[13] > 7)
		   {
			if (rc = set_resume(fdes, tucb_ptr))
				return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
		   }

		return(0);
	   }
	(void) set_resume(fdes, tucb_ptr);
	return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, VPD_HDR_ERR));
   } /* End test */

/*****************************************************************************

tu005

*****************************************************************************/

int tu005 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	return(vpd_test(fdes, tucb_ptr));
   }
