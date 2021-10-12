static char sccsid[] = "@(#)59	1.3  src/bos/kernext/disp/ped/config/mid_config.c, peddd, bos411, 9428A410j 11/3/93 11:10:31";

/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_config
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



extern long cfg_init(dev_t, struct uio *);
extern long cfg_term(dev_t);
extern long cfg_qvpd(dev_t, struct uio *);




/*****************************************************************************/
/*									     */
/* IDENTIFICATION:  MID_CONFIG						     */
/*									     */
/* DESCRIPTIVE NAME: MID_CONFIG - Close routine for Mid Level Adapter        */
/*									     */
/* FUNCTION:								     */
/*									     */
/*									     */
/*									     */
/* INPUTS:								     */
/*									     */
/* OUTPUTS:								     */
/*****************************************************************************/

long mid_config(devno, cmd, uiop)
dev_t devno;
long cmd;
struct uio *uiop;

{
	struct ddsmid *ddsptr;
	int rc,min_num,i;
	int minors;
	unsigned long slot;		
	int status;
	label_t jmpbuf;
	unsigned int save_bus_acc = 0;
	struct phys_displays *pd,*last;


	BUGLPR(dbg_middd, BUGVPD, ("Entering mid_config\n"));

	/*-------------------------------------------------------------------*/
	/* This routine switches off of the command passed in and either     */
	/* inits terminates or does a query of the VPD on the adapter.	     */
	/*-------------------------------------------------------------------*/

	switch (cmd)
	{
	case CFG_INIT:

		/*--------------------------------------------------*/
		/* In this case we need to get our driver into the  */
		/* system by allocating a phys display struct and   */
		/* filling it in then adding our driver to the      */
		/* devsw table.					    */
		/*--------------------------------------------------*/

		rc = cfg_init(devno,uiop);
		if (rc != 0)
			return(rc);

		break;


	case CFG_TERM:

		/*--------------------------------------------------*/
		/* In this case we would remove all malloc'd data   */
		/* and remove ourselves from the device switch table*/
		/* if we are the last device of this type.	    */
		/* Calculate pointer to phys_display		    */
		/*--------------------------------------------------*/

		rc = cfg_term(devno);
		if (rc != 0)
			return(rc);

		break;

	
	case CFG_QVPD:

		/*-----------------------------------------------------------*/
		/* This function returns the Vital Product Data from	     */
		/* the adapter						     */
		/*-----------------------------------------------------------*/

		rc = cfg_qvpd(devno,uiop);
		if (rc != 0)
			return(rc);

		break;

	}

BUGLPR(dbg_middd, BUGACT, ("Leaving mid_config\n"));

return (0);

}	/* end of mid_config */




