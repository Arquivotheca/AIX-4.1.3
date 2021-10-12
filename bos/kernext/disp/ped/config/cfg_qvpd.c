static char sccsid[] = "@(#)54  1.7.2.5  src/bos/kernext/disp/ped/config/cfg_qvpd.c, peddd, bos411, 9428A410j 3/31/94 21:30:33";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: cfg_qvpd
 *		get_vpd_descriptor
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/types.h>
#include <sys/iocc.h>
#include <sys/proc.h>
#include <sys/mdio.h>
#include <sys/conf.h>
#include <sys/file.h>
#include <sys/intr.h>
#include <sys/uio.h>
#include <sys/dma.h>
#include <sys/pin.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/errids.h>
#include <sys/syspest.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include "mid.h"
#define Bool unsigned
#include <sys/aixfont.h>
#include <sys/display.h>

#include "ddsmid.h"
#include "midddf.h"
#include "midhwa.h"
/*
#include "midksr.h"
#include "midrcx.h"
#include "midfifo.h"
*/

BUGXDEF(dbg_middd);
BUGXDEF(dbg_midddi);
BUGXDEF(dbg_middma);
BUGXDEF(dbg_midddf);
BUGXDEF(dbg_midrcx);
BUGXDEF(dbg_midevt);
BUGXDEF(dbg_midtext);

#ifndef BUGPVT
#define BUGPVT	99
#endif

#ifndef BUGVPD				/* Set to 0 in Makefile if debugging */
#define BUGVPD	BUGACT
#endif

char *get_vpd_descriptor();





/***********************************************************************/
/*								       */
/* IDENTIFICATION: CFG_QVPD					       */
/*								       */
/* DESCRIPTIVE NAME: CFG_QVPD - Query VPD routine for MIDDD3D          */
/*								       */
/* FUNCTION:							       */
/*								       */
/*								       */
/*								       */
/* INPUTS:							       */
/*								       */
/* OUTPUTS:							       */
/***********************************************************************/

long cfg_qvpd(devno,uiop)
dev_t devno;
struct uio *uiop;

{
	midddf_t *ddf;
	ulong version = 0;              /* microcode version         */
	ulong configuration = 0;        /* bits describing cards     */
	int card;			/* address each card	     */
	int byte;			/* which byte of the VPD     */
	volatile uchar *pptr;		/* char posreg pointer	     */
	char vpdbuf[MAX_VPD_LEN];	/* buffer vpd data here      */
	volatile unsigned long HWP = 0;
	int b;				/* a temporary		     */
	int		ucfd;
	struct file	*MIDDD3D_ucode_file_ptr;
	int		numbytes;
	int 		status;
	struct phys_displays *pd;
	struct ddsmid *ddsptr;
	int		slot;
	int		rc;
	int             lega = 0;
	label_t jmpbuf;
	unsigned int save_bus_acc = 0;
	char            *vpd_desc;

	typedef struct {
		long		reserved;
		long		version;
	} ucode_t;
	ucode_t 	head;

	BUGLPR(dbg_middd, BUGACT,
		("cfg_qvpd entry point was called.\n"));

	/*----------
	  get slot number from the first word of the VPD buffer.
	  ----------*/
	rc = uiomove(&slot, sizeof(slot), UIO_WRITE, uiop);

	if (rc != 0)
	{
		BUGLPR(dbg_middd, 0,
			("uiomove failure, rc = %d.\n", rc));
		return rc;
	}

	devswqry(devno, &status, &pd);

	while (pd) {
		if (pd->devno == devno)
			break;
		pd = pd->next;
	}

	if (pd) {               /* adapter has been configured */
		ddsptr = (struct ddsmid *) pd->odmdds;
		ASSERT(ddsptr != NULL);
		slot = ddsptr->slot_number;
		BUGLPR(dbg_middd, BUGVPD, ("slot = %d\n", slot));
		ddf = (midddf_t *) pd->free_area;
		ASSERT(ddf != NULL);

		BUGLPR(dbg_middd, BUGVPD, ("Entering load_head \n"));

		/*----------------------------------------------------*/
		/* Read the ucode version number.                     */
		/*----------------------------------------------------*/

		BUGLPR(dbg_middd, 1 , 
			("ucode file %s\n",&(ddsptr->ucode_name)));


		rc = fp_open (ddsptr->ucode_name, O_RDONLY, NULL, NULL,
			SYS_ADSPACE, &ddf->mid_ucode_file_ptr);
		if (rc != 0)
		{
			BUGLPR(dbg_middd, 0,
				("fp_open failure, rc = %d\n",rc));
			return(rc);
		}

		BUGLPR(dbg_middd, 1,
				("ucode ptr 0x%x.\n",ddf->mid_ucode_file_ptr));

		MIDDD3D_ucode_file_ptr = ddf->mid_ucode_file_ptr;

		if ((ucfd = fp_lseek(MIDDD3D_ucode_file_ptr, 0, 0))
			== -1) {
			MIDDD3D_ucode_file_ptr = NULL;
		}
		else
		if ((ucfd = fp_read(MIDDD3D_ucode_file_ptr, &head,
			sizeof(head), 0, UIO_SYSSPACE, &numbytes)) != 0) {
			MIDDD3D_ucode_file_ptr = NULL;
		}
		fp_close (ddf->mid_ucode_file_ptr);

#ifdef UCODE
		version = head.version;
#else
		version = 0;
#endif
	}

	/*----------------------------------------------------*/
	/* Read the vital product data.                       */
	/*----------------------------------------------------*/

	HWP = IOCC_ATT( BUS_ID, 0 );

	for (card = 0; card < 3; card++)
	{
		BUGLPR(dbg_middd, BUGVPD,
			("card #%d: accessing POSREG 5\n", card));

		/*----------------------------------------------------*/
		/* Put card #  in POS reg 5.  Set auto inc on.        */
		/*----------------------------------------------------*/

		pptr = HWP + POSREG(5, slot) + IO_IOCC;
		*pptr = ((card << 1) | POS5_AUTOINC |
				POS5_CHKST_DI | POS5_CHKCH_DI);

		/*----------------------------------------------------*/
		/* Put address in POS reg 7.                          */
		/*----------------------------------------------------*/

		BUGLPR(dbg_middd, BUGVPD,
			("card #%d: accessing POSREG 7\n", card));

		pptr = HWP + POSREG(7, slot) + IO_IOCC;
		*pptr = 0;

		/*----------------------------------------------------*/
		/* Read VPD out of POS reg 4.                         */
		/*----------------------------------------------------*/

		BUGLPR(dbg_middd, BUGVPD,
			("card #%d: accessing POSREG 7\n", card));

		for (byte = 0; byte < MAX_VPD_LEN; byte++)
		{
			pptr = HWP + POSREG(3, slot) + IO_IOCC;

			vpdbuf[byte] = *pptr;
		}


		if ( !card ) {          /* If the first card.           */
			vpd_desc = get_vpd_descriptor( vpdbuf, "DS" );
			if ( !strncmp( vpd_desc, "GRAPHIC LEGA", 12 ) ||
			     !strncmp( vpd_desc, " GRAPHIC 2D LEGA", 16 ) )
			{
				lega = 1;
				configuration = MID_VPD_PPR;
			}
			else if ( !strncmp( vpd_desc, " GRAPHIC 3D LEGA", 16 ) )
			{
				lega = 1;
				configuration = MID_VPD_PPR | MID_VPD_PPZ;
			}
			else {  /* A mid ped adapter.           */
				configuration = MID_VPD_PPR | MID_VPD_PPZ;

				/* Read POS 2 pipe card bit.  */
				pptr = HWP + POSREG( 2,slot ) + IO_IOCC;
				if ( !( ( *pptr >> 5 ) & 0x01 ) )
					configuration |= MID_VPD_PPC;
			}
		}
		else if ( !lega ) {  /* More configuration           */
			vpd_desc = get_vpd_descriptor( vpdbuf, "DS" );
			if ( !strncmp( vpd_desc, " GRAPHIC PGR", 12 ) )
				configuration |= MID_VPD_PGR;
			if ( !strncmp( vpd_desc, " GRAPHIC POP", 12 ) ||
		             !strncmp( vpd_desc, " GRAPHIC PGR24", 14 ) )
				configuration |= MID_VPD_POP;
		}

		BUGLDM(dbg_middd, BUGVPD,
		"Dump of VPD Register:\n", vpdbuf, MAX_VPD_LEN);
		rc = uiomove(vpdbuf, sizeof(vpdbuf), UIO_READ,
			uiop);
		if (rc != 0)
		{
			BUGLPR(dbg_middd, 0,
			("uiomove failure, rc = %d.\n", rc));
			IOCC_DET( HWP );
			return rc;
		}

	}

	/*----------------------------------------------------*/
	/* If VPD then there is option card.                  */
	/*----------------------------------------------------*/

	BUGLPR(dbg_middd, BUGVPD, ("Microcode level is 0x%x\n", version));
	rc = uiomove(&version, sizeof(version), UIO_READ, uiop);

	if (rc != 0)
	{
		BUGLPR(dbg_middd, 0,
			("uiomove failure, rc = %d.\n", rc));
		IOCC_DET( HWP );
		return rc;
	}

	/*----------------------------------------------*/
	/* Read the hardware configuration.             */
	/*----------------------------------------------*/

	BUGLPR(dbg_middd, 3,
	("Configuration is 0x%x\n", configuration));
	rc = uiomove(&configuration, sizeof(configuration),
	       UIO_READ, uiop);

	if (rc != 0)
	{
	       	BUGLPR(dbg_middd, 0,
	       	("uiomove failure, rc = %d.\n", rc));
		IOCC_DET( HWP );
	       	return rc;
	}

	IOCC_DET( HWP );

	BUGLPR(dbg_middd, BUGACT, ("Leaving cfg_qvpd\n"));

	return(0);

}

/*
 * NAME: get_vpd_descriptor
 *
 * FUNCTION: Reads a descriptor from the VPD for a device
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function is used to decode VPD which is stored in the format
 *      used by the system devices ( CPU's, Planars etc. )
 *
 * NOTES:
 *      VPD is stored as a series of descriptors, each of which
 * is encoded as follows:
 *
 * Byte 0 = '*'
 * Byte 1,2 = mnemonic          ( E.g. "TM", "Z1", etc )
 * Byte 3 = Total length / 2
 * Byte 4.. = data
 *
 *  E.g.:  Byte#     0    1    2    3    4    5    6    7    8    9
 *         Ascii    '*'  'Z'  '1'       '0'  '1'  '2'  '0'  '0'  '1'
 *         Hex       2A   5A   31   05   30   31   32   30   30   31
 *         Oct      052  132  061  005  060  061  062  060  060  061
 *
 * RETURNS:
 *
 *      A pointer to the static char array "result" is returned, with
 *      the array being empty if the descriptor was not present in the
 *      VPD passed in.
 */

char *
get_vpd_descriptor( vpd, name )
register char   *vpd;
char            *name;
{
static char     result[256];
register char   *res_ptr;
register int    bytecount, i;

	res_ptr = result;
	*res_ptr = '\0';

        i = vpd + (MAX_VPD_LEN - 3);

        for ( ; vpd <= i && strncmp( vpd,"VPD",3 ); vpd++ );

        if (vpd >= i)
        {
                /* Something is wrong: we didn't find the VPD string */
                return(result);
        }

        /* here we have found "VPD".  We need to skip over 7 bytes, 3
           over "VPD", 2 over the length field, and 2 over the CRC
           field to get to the very first '*' (i.e, the very first
           VPD descriptor).  For some adapter, the CRC might have a
           byte whose value happens to be the same as that of '*'.
           All the work above is to make sure when we do the search
           after the CRC field to eliminate the confusion
        */

        vpd += 7;

	while( *vpd == '*' )
	{
		if( ( vpd[1] == name[0] ) && ( vpd[2] == name[1] ) )
		{
			/*
			 * This is the correct descriptor
			 */
			bytecount = ((int)vpd[3] << 1 ) - 4;

			vpd += 4;

			while( bytecount-- )
				*res_ptr++ = *vpd++;

			*res_ptr = '\0';
		}
		else
			/*
			 * Skip to next descriptor
			 */
			vpd += ( (int)vpd[3] << 1 );
	}

	return(result);
}

