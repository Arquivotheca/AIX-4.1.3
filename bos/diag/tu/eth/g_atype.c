static char sccsid[] = "src/bos/diag/tu/eth/g_atype.c, tu_eth, bos411, 9428A410j 6/5/92 14:00:35";
/*
 * COMPONENT_NAME: (ETHERTU) Ethernet Test Unit
 *
 * FUNCTIONS: get_partno_offset, get_atype
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************

Function(s) Get Adapter Type

                       >>> IMPORTANT NOTE <<<

hard_reset() MUST be called at the beginning of this function
in order to assess the configuration of the card (most importantly -
the firmware level) due to the problem with reading VPD with
PAUSE/RESUME commands (10/03/89).

*****************************************************************************/
#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include "ethtst.h"

#define START_OFFSET	0x12
#define DIAG_LEN	2

#ifdef debugg
extern void detrace();
#endif
extern void detrace();

/*****************************************************************************

get_partno_offset

*****************************************************************************/
int get_partno_offset (fdes, partno_offset, tucb_ptr)
   int fdes;
   int *partno_offset;
   TUTYPE *tucb_ptr;
   {
	int rc, i;
	int vpd_len, vpd_size;
	int got_it_count;
	unsigned long   status;
	unsigned char cval;
	extern unsigned char pos_rd();	/* reads value from POS register */
	extern int pos_wr();		/* writes value to POS register */

	/*
	 * get the size of the VPD (offset 5)
	 */
	rc = pos_wr(fdes, 6, 5, &status, tucb_ptr);
	if (rc)
	   {
		return(POS6_WR_ERR);
	   }

	rc = pos_wr(fdes, 7, 0, &status, tucb_ptr);
	if (rc)
	   {
		return(POS7_WR_ERR);
	   }
	cval =  pos_rd(fdes, 3, &status, tucb_ptr);
	if (status != 0)
	   {
		return(POS3_RD_ERR);
	   }
	/*
	 * Recall that the length is expressed in words (not bytes)
	 * so multiple by word length (2 bytes) to get no. of bytes.
	 */
	vpd_len = cval * 2;

	/*
	 * vpd_len does not include the 8 initial bytes containing
	 * the ascii VPD, the length, and CRC, so add them to
	 * get the end of the VPD.
	 */
	vpd_size = vpd_len + 8;

	got_it_count = 0;
	*partno_offset = 0;
	for (i = START_OFFSET; (i < vpd_size) && (got_it_count < 4); i++)
	   {
		rc = pos_wr(fdes, 6, i, &status, tucb_ptr);
		if (rc)
		   {
			return(POS6_WR_ERR);
		   }

		rc = pos_wr(fdes, 7, 0, &status, tucb_ptr);
		if (rc)
		   {
			return(POS7_WR_ERR);
		   }
		cval =  pos_rd(fdes, 3, &status, tucb_ptr);
		if (status != 0)
		   {
			return(POS3_RD_ERR);
		   }
		switch (got_it_count)
		   {
			case 0:
				if (cval == '*')
					got_it_count++;
				break;
			case 1:
				if (cval == 'P')
					got_it_count++;
				else
					got_it_count = 0;
				break;
			case 2:
				if (cval == 'N')
					got_it_count++;
				else
					got_it_count = 0;
				break;
			case 3:
				/* NOTE binary 6 - no quotes */
				if (cval == 6)
				   {
					got_it_count++;
					*partno_offset = i + 1;
				   }
				else
					got_it_count = 0;
				break;
			default:
				return(1);
		   } /* end switch */
	   } /* end for */
	
	if (got_it_count != 4)
		return(1);
	
	return(0);
   }


/*****************************************************************************

get_diag_offset

*****************************************************************************/
int get_diag_offset (fdes, diagno_offset, tucb_ptr)
   int fdes;
   int *diagno_offset;
   TUTYPE *tucb_ptr;
   {
	int rc, i;
	int vpd_len, vpd_size;
	int got_it_count;
	unsigned long   status;
	unsigned char cval;

	extern unsigned char pos_rd();	/* reads value from POS register */
	extern int pos_wr();		/* writes value to POS register */

	/*
	 * get the size of the VPD (offset 5)
	 */
	rc = pos_wr(fdes, 6, 5, &status, tucb_ptr);
	if (rc)
	   {
		return(POS6_WR_ERR);
	   }

	rc = pos_wr(fdes, 7, 0, &status, tucb_ptr);
	if (rc)
	   {
		return(POS7_WR_ERR);
	   }
	cval =  pos_rd(fdes, 3, &status, tucb_ptr);
	if (status != 0)
	   {
		return(POS3_RD_ERR);
	   }
	/*
	 * Recall that the length is expressed in words (not bytes)
	 * so multiple by word length (2 bytes) to get no. of bytes.
	 */
	vpd_len = cval * 2;

	/*
	 * vpd_len does not include the 8 initial bytes containing
	 * the ascii VPD, the length, and CRC, so add them to
	 * get the end of the VPD.
	 */
	vpd_size = vpd_len + 8;

	got_it_count = 0;
	*diagno_offset = 0;
	for (i = START_OFFSET; (i < vpd_size) && (got_it_count < 4); i++)
	   {
		rc = pos_wr(fdes, 6, i, &status, tucb_ptr);
		if (rc)
		   {
			return(POS6_WR_ERR);
		   }

		rc = pos_wr(fdes, 7, 0, &status, tucb_ptr);
		if (rc)
		   {
			return(POS7_WR_ERR);
		   }
		cval =  pos_rd(fdes, 3, &status, tucb_ptr);
		if (status != 0)
		   {
			return(POS3_RD_ERR);
		   }
		switch (got_it_count)
		   {
			case 0:
				if (cval == '*')
					got_it_count++;
				break;
			case 1:
				if (cval == 'D')
					got_it_count++;
				else
					got_it_count = 0;
				break;
			case 2:
				if (cval == 'G')
					got_it_count++;
				else
					got_it_count = 0;
				break;
			case 3:
				/* NOTE binary 3 - no quotes */
				if (cval == 3)
				{
					got_it_count++;
					*diagno_offset = i + 1;
				}
				else
					got_it_count = 0;
				break;
			default:
				return(1);
		   } /* end switch */
	   } /* end for */
	
	if (got_it_count != 4)
		return(1);
	
	return(0);
   }


/*****************************************************************************

get_atype

*****************************************************************************/
int get_atype (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int i, partno_offset, diagno_offset, end_address;
	int rc;
	unsigned char   *ucp;
	unsigned char   diag_number[2];
	unsigned char   part_number[PARTNO_LEN];
	unsigned long   status;
        unsigned char sval;

	struct part_struct
	   {
		char *part_number;
		int adapter_type;
	   };
	static struct part_struct part_table[] =
	   {
		{ AT_PARTNO,   AT  },
		{ AT2_PN_PRO,  AT  },
		{ PS2_PARTNO,  PS2 },
		{ PS2_PN_REV2, PS2 },
		{ PS2_PN_REV3, PS3 }
	   };
	static int part_table_size = sizeof(part_table) / sizeof(struct part_struct);
	struct part_struct *part_sp;

#ifdef debugg
	unsigned char cval;
#endif

        unsigned char gval;
        unsigned char zval;
	extern int hard_reset();
	extern unsigned char pos_rd();	/* reads value from POS register */
	extern int pos_wr();		/* writes value to POS register */
	extern int set_pause();
	extern int set_resume();
        extern unsigned char hio_parity_rd();
        extern int hio_parity_wr();
	
/**********************/
        /*
         * Disable Par in Par Register (uchannel enable) because of the initial
         * RAM problem on power up      ZTK  02/27/92
         * After reset parity will disabled ( by harware reset routine
         * for successful exit ( what about error exits ??? )
         * Also reset +ADPTPAREN -in POS4- this is an internal parity
         */
/**********************/

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

         gval =  pos_rd(fdes, 4, &status, tucb_ptr);
         if (status !=0)
               return(POS4_RD_ERR);
         zval = gval & 0xef;
         rc = pos_wr(fdes, 4, zval, &status, tucb_ptr);
         if (rc)
               return(POS4_WR_ERR);

       /*
	* Do a hard_reset
	*/
	if (rc = hard_reset(fdes, tucb_ptr))
	   {
		/*
		 * mask out the top two bytes which
		 * may have error code info since it
		 * is also called as a separate test unit.
		 */
		rc &= 0x0000ffff;
		return(rc);
	   }

/* restore POS4 after hard reset*/

         rc = pos_wr(fdes, 4, gval, &status, tucb_ptr);
         if (rc)
               return(POS4_WR_ERR);

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
			return(rc);
	   }


#ifdef debugg
	detrace(1,"get_atype:  Ready for ALL VPD?\n");
	for (i = 0x01; i < 0x6f; i++)
	   {
		rc = pos_wr(fdes, 6, i, &status, tucb_ptr);
		if (rc)
		   {
			if (tucb_ptr->eth_htx_s.config_table[13] > 7)
				(void) set_resume(fdes, tucb_ptr);
			return(POS6_WR_ERR);
		   }

		rc = pos_wr(fdes, 7, 0, &status, tucb_ptr);
		if (rc)
		   {
			if (tucb_ptr->eth_htx_s.config_table[13] > 7)
				(void) set_resume(fdes, tucb_ptr);
			return(POS7_WR_ERR);
		   }
		cval =  pos_rd(fdes, 3, &status, tucb_ptr);
		if (status != 0)
		   {
			if (tucb_ptr->eth_htx_s.config_table[13] > 7)
				(void) set_resume(fdes, tucb_ptr);
			return(POS3_RD_ERR);
		   }
		detrace(0,"VPD off 0x%02x = 0x%02x (%c ascii)\n",
			i, cval, cval);
		if (i && !(i % 16))
			detrace(1,"get_atype:  More VPD to go...?\n");
	   } /* endfor */
#endif /* end if-debugg */

	/*
	 * go find the offset into the VPD where
	 * the part number starts
	 */
	if (get_partno_offset(fdes, &partno_offset, tucb_ptr))
	   {
		if (tucb_ptr->eth_htx_s.config_table[13] > 7)
			(void) set_resume(fdes, tucb_ptr);
		return(PART_NO2_ERR);
	   }

	/*
	 * Step through the VPD starting at the "partno_offset"
	 * to read in the part number.
	 */
	memset(part_number, 0x00, PARTNO_LEN);
	ucp = part_number;
	end_address = partno_offset + PARTNO_LEN;
	for (i = partno_offset; i < end_address; i++)
	   {
		rc = pos_wr(fdes, 6, i, &status, tucb_ptr);
	        if (rc)
		   {
			if (tucb_ptr->eth_htx_s.config_table[13] > 7)
				(void) set_resume(fdes, tucb_ptr);
	        	return(POS6_WR_ERR);
		   }

		rc = pos_wr(fdes, 7, 0, &status, tucb_ptr);
		if (rc)
		   {
			if (tucb_ptr->eth_htx_s.config_table[13] > 7)
				(void) set_resume(fdes, tucb_ptr);
			return(POS7_WR_ERR);
		   }
		*ucp++ = pos_rd(fdes, 3, &status, tucb_ptr);
		if (status != 0)
		   {
			if (tucb_ptr->eth_htx_s.config_table[13] > 7)
				(void) set_resume(fdes, tucb_ptr);
			return(POS3_RD_ERR);
		   }

	   } /* endfor */

	if (pos_wr(fdes, 6, 0x00, &status, tucb_ptr))
        	return(POS6_WR_ERR);
	if (pos_wr(fdes, 7, 0x00, &status, tucb_ptr))
        	return(POS7_WR_ERR);

	/*
	 * Check firmware version.  If > 7, then card needs
	 * PAUSE/RESUME of adapter cpu to read VPD.  So
	 * now that we've finished reading from VPD,
	 * we can "resume" the processor on the adapter.
	 */
	if (tucb_ptr->eth_htx_s.config_table[13] > 7)
	   {
		if (rc = set_resume(fdes, tucb_ptr))
			return(rc);
	   }
	
	/*
	 * Check if part number from VPD is a legal one in
	 * our table.  If so, assign it's "type" (i.e. AT or PS2)
	 * and return.  Else, it's unknown so return error.
	 */
	part_sp = part_table;
	for (i = 0; i < part_table_size; i++, part_sp++)
		if (memcmp(part_number, part_sp->part_number, PARTNO_LEN) == 0)
		   {
			tucb_ptr->eth_htx_s.adapter_type = part_sp->adapter_type;
			return(0);
		   }
	
#ifdef debugg
	detrace(1,"get_atype:  Unknown part_number:\n");
	for (i = 0; i < PARTNO_LEN; i++)
		detrace(0,"VPD offset 0x%02x = 0x%02x (%c ascii)\n",
			partno_offset + i, part_number[i],
			part_number[i]);
	detrace(1,"get_atype:  Returning failure\n");

	return(PART_NO_ERR);
#endif

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
			return(rc);
	}


	/*
	 * go find the offset into the VPD where
	 * the diagnostic number starts
	 */
	if (get_diag_offset(fdes, &diagno_offset, tucb_ptr))
	   {
		if (tucb_ptr->eth_htx_s.config_table[13] > 7)
			(void) set_resume(fdes, tucb_ptr);
		return(PART_NO2_ERR);
	   }

	/*
	 * Step through the VPD starting at the "diagno_offset"
	 * to read in the part number.
	 */
	memset(diag_number, 0x00, 2);
	ucp = diag_number;
	end_address = diagno_offset + 2;
	for (i = diagno_offset; i < end_address; i++)
	   {
		rc = pos_wr(fdes, 6, i, &status, tucb_ptr);
	        if (rc)
		   {
			if (tucb_ptr->eth_htx_s.config_table[13] > 7)
				(void) set_resume(fdes, tucb_ptr);
	        	return(POS6_WR_ERR);
		   }

		rc = pos_wr(fdes, 7, 0, &status, tucb_ptr);
		if (rc)
		   {
			if (tucb_ptr->eth_htx_s.config_table[13] > 7)
				(void) set_resume(fdes, tucb_ptr);
			return(POS7_WR_ERR);
		   }
		*ucp++ = pos_rd(fdes, 3, &status, tucb_ptr);
		if (status != 0)
		   {
			if (tucb_ptr->eth_htx_s.config_table[13] > 7)
				(void) set_resume(fdes, tucb_ptr);
			return(POS3_RD_ERR);
		   }

	   } /* endfor */

	if (pos_wr(fdes, 6, 0x00, &status, tucb_ptr))
        	return(POS6_WR_ERR);
	if (pos_wr(fdes, 7, 0x00, &status, tucb_ptr))
        	return(POS7_WR_ERR);

	/*
	 * Check firmware version.  If > 7, then card needs
	 * PAUSE/RESUME of adapter cpu to read VPD.  So
	 * now that we've finished reading from VPD,
	 * we can "resume" the processor on the adapter.
	 */
	if (tucb_ptr->eth_htx_s.config_table[13] > 7)
	   {
		if (rc = set_resume(fdes, tucb_ptr))
			return(rc);
	   }
	
	if ((diag_number[0] == 0x30) && (diag_number[1] == 0x31))
	{
		tucb_ptr->eth_htx_s.adapter_type = PS3;
		return(0);
	}

	/*
	 * To date, if part number is unrecognizable,
	 * then possibly new one for PS2 so default to PS2.
	 */
	tucb_ptr->eth_htx_s.adapter_type = PS2;
	return(0);
   }
