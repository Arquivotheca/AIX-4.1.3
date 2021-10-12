static char sccsid[] = "@(#)56	1.3  src/bos/kernext/disp/ped/config/mid_ioctl.c, peddd, bos411, 9428A410j 11/2/93 16:05:25";

/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_ioctl
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/types.h>
#include <sys/ioacc.h>
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











/***********************************************************************/
/*								       */
/* IDENTIFICATION: MID_IOCTL					       */
/*								       */
/* DESCRIPTIVE NAME: MID_IOCTL- Interrupt handler for MIDDD3D	       */
/*								       */
/* FUNCTION:							       */
/*								       */
/*								       */
/*								       */
/* INPUTS:							       */
/*								       */
/* OUTPUTS:							       */
/***********************************************************************/

long mid_ioctl(devno,cmd,arg,devflag,chan,ext)

dev_t devno;		/* The device number of the adapter	       */
long cmd;		/* The command for the ioctl		       */
long arg;		/* The address of paramter block	       */
ulong devflag;		/* Flag indicating type of operation	       */
long chan;		/* Channel number ignored by this routine      */
long ext;		/* Extended system parameter - not used        */


{
	return (0);
#if 0
	struct lft_query  tu_qry_buf;
	struct hfqdiagc tu_cmd;
	struct hfqdiagr tu_resp;
	struct phys_displays    *pd;
	struct midddf   *ddf;
	int     status;
	int     i, j;
	int     rc = 0;
	char    pos;
	label_t jmpbuf;
	unsigned int save_bus_acc = 0;
	volatile unsigned long HWP;

	BUGLPR(dbg_middd, BUGNTX, ("Entering mid_ioctl.\n"));

	/*  Get phys_display pointer and minor number.                  */
	devswqry(devno,&status,&pd);

	/*  Loop through the phys display to get the right structure.   */
	for (; pd; pd = pd->next)
		if (pd->devno == devno)
			break;

	if (!pd) return EINVAL;         /* invalid devno                */

	ddf = (midddf_t *) pd->free_area;

	BUGLPR( dbg_middd,  BUGNTX,
	("ddf = 0x%x, pd = 0x%x, devno = 0x%x.\n", ddf, pd, devno));

	switch (cmd) {
	case HFQUERY:                   /* Request for diagnostic support*/
	{
		if (copyin(arg, &tu_qry_buf, sizeof(struct hfquery))) {

			/* error has occurred       */
			BUGLPR(dbg_middd, BUGNTX,
				("** ERROR - copyin failed.\n"));
			return( EFAULT );
		}

		if (copyin(tu_qry_buf.hf_cmd, &tu_cmd,
			tu_qry_buf.hf_cmdlen)) {

			/* error has occurred       */
			BUGLPR(dbg_middd, BUGNTX,
			("* ERROR - copyin failed.\n"));
			return ( EFAULT );
		}


		if (( tu_cmd.hf_intro[7] == HFQDIAGCH) &&
			( tu_cmd.hf_intro[8] == HFQDIAGCL) &&
			( tu_cmd.hf_device == HF_DISPLAY))
		{
			/* All parameters are correct;                  */
			/* proceed with diagnostic steps.               */
			switch (tu_cmd.hf_testnum)
			{
			case MID_POS_TEST:

				mid_POS_test ( ddf, &pos);

				tu_resp.hf_result[0] = pos;

				if (copyout(&tu_resp,tu_qry_buf.hf_resp,
				tu_qry_buf.hf_resplen))
				{
					BUGLPR(dbg_middd, BUGNTX,
					("* copyout failed.\n"));
					return( EFAULT );
				}

				break;

			case MID_POS2_BIT3_SET:
			case MID_POS2_BIT3_RESET:

				mid_set_POS2 ( ddf, tu_cmd.hf_testnum );

				break;

			default:
				BUGLPR(dbg_middd, BUGNTX,
					("Invalid ioctl test number\n"));
				rc = EINVAL;
			}       /* End of command switch */

		}
		else {
			BUGLPR(dbg_middd, BUGNTX,
			("* ERROR - bad diag query structure.\n"));
			return( EINVAL );
		}
		break;
	}


	default:
		BUGLPR(dbg_middd, BUGNTX, ("Invalid ioctl\n"));
		rc = EINVAL;

	}       /* End of switch */

	BUGLPR(dbg_middd, BUGNTX, ("Returning %d from mid_ioctl.\n", rc));
	return rc;
#endif

}       /* mid_ioctl */

