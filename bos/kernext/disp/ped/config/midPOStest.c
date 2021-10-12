static char sccsid[] = "@(#)51	1.5  src/bos/kernext/disp/ped/config/midPOStest.c, peddd, bos411, 9428A410j 4/8/94 16:10:32";

/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_POS_test
 *		mid_set_POS2
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












/*-------------------
Called by cfg_init() & mid_ioctl() to verify the POS registers.
-------------------*/

long mid_POS_test ( ddf, pos )
struct  midddf   *ddf;
char    *pos;
{
	volatile char   *pptr;
	volatile char   pbuf2;
	volatile char   pbuf4;
	volatile char   pbuf5;
	volatile char   pbuf7;
	int     i, j;
	char    slot;
	label_t jmpbuf;
	unsigned int save_bus_acc = 0;
	volatile unsigned long HWP;
	char    POSBitPattern[4] = { 0x55, 0xAA, 0xFF, 0x00 };
	char    POSBitMask[4] = { 0xFF, 0x07, 0x00, 0xFF };

	BUGLPR(dbg_middd, BUGNTX, ("Entering mid_POS_test.\n"));

	HWP = IOCC_ATT( BUS_ID, 0 );

	*pos = '\0';

	/* Gain access to the bus               */

	slot = ddf->slot;

	BUGLPR(dbg_middd, BUGACT, ("Adapter is in slot #%d\n", slot));

	/* Access POS register 2                */
	pptr = HWP + POSREG(2,slot) + IO_IOCC;
	pbuf2 = *pptr;

	/* Test POS register 2                  */
	for (i = 0; i < 4; i++)
	{
		*pptr = POSBitPattern[i] & 0xDF;

		if ( ( *pptr & 0xDF ) != ( POSBitPattern[i] & 0xDF ) )
		       break;
	}

	if (i == 4)
	{
		*pos |= MID_POS_2_OK;
		BUGLPR(dbg_middd, BUGACT, ("POS2 OK \n"));
	}

	/* Set POS 5 to 0.                      */
	pptr = HWP + POSREG(5,slot) + IO_IOCC;
	pbuf5 = *pptr;
	*pptr = 0;

	/* Set POS 7 to 0.                      */
	pptr = HWP + POSREG(7,slot) + IO_IOCC;
	pbuf7 = *pptr;
	*pptr = 0;

	/* Set POS 4 to 0.                      */
	pptr = HWP + POSREG(4,slot) + IO_IOCC;
	pbuf4 = *pptr;
	*pptr = 0;

	/* Test POS registers 4, 5, 7.          */
	for (i = 4; i < 8; i++)
	{
		if ( i == 6 ) continue;

		pptr = HWP + POSREG(i,slot) + IO_IOCC;

		for (j = 0; j < 4; j++)
		{
			*pptr = POSBitPattern[j] & POSBitMask[i-4];

			if ( ( *pptr & POSBitMask[i-4] ) !=
				( POSBitPattern[j] & POSBitMask[i-4] ) )
				break;
		}

		if (j == 4)
		{
			*pos |= (1 << i);
			BUGLPR(dbg_middd, BUGACT, ("POS%d OK \n", i));
		}
	}

	/* Restore POS 2.                       */
	pptr = HWP + POSREG(2,slot) + IO_IOCC;
	*pptr = pbuf2;

	/* Restore POS 4.                       */
	pptr = HWP + POSREG(4,slot) + IO_IOCC;
	*pptr = pbuf4;

	/* Restore POS 7.                       */
	pptr = HWP + POSREG(7,slot) + IO_IOCC;
	*pptr = pbuf7;

	/* Restore POS 5.                       */
	pptr = HWP + POSREG(5,slot) + IO_IOCC;
	*pptr = pbuf5;

	IOCC_DET( HWP );

	BUGLPR(dbg_middd, BUGNTX, ("Leaving mid_POS_test.\n"));
	return (0);

}

/*-------------------
Called by mid_ioctl() to set & reset the POS register 2.
-------------------*/

long mid_set_POS2 ( ddf, pos_num )
struct  midddf   *ddf;
int     pos_num;
{

	volatile char   *pptr;
	volatile char   pbuf;
	int     i, j;
	char    slot;
	label_t jmpbuf;
	unsigned int save_bus_acc = 0;
	volatile unsigned long HWP;

	BUGLPR(dbg_middd, BUGNTX, ("Entering mid_set_POS2.\n"));

	HWP = IOCC_ATT( BUS_ID, 0 );

	/* Gain access to the bus               */

	slot = ddf->slot;

	BUGLPR(dbg_middd, BUGACT, ("Adapter is in slot #%d\n", slot));

	/* Set POS register 2.                  */
	pptr = HWP + POSREG(2,slot) + IO_IOCC;

	if (pos_num == MID_POS2_BIT3_SET) *pptr |= POS2_RESERVE1;
	else *pptr &= ~(POS2_RESERVE1);

	IOCC_DET( HWP );

	BUGLPR(dbg_middd, BUGNTX, ("Leaving mid_set_POS2.\n"));
	return (0);

}
