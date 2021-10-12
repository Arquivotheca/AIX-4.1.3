static char sccsid[] = "@(#)29  1.3  src/bos/diag/tu/msla/mslainttst.c, tu_msla, bos411, 9428A410j 11/14/90 11:39:31";
/*
 * COMPONENT_NAME: ( msalinttst ) 
 *
 * FUNCTIONS:  int_rios
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

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include "mslatime.h"
#include "mslatu.h"
#include "mslaerrdef.h"
#include "mslafdef.h"
#include "msladgappl.h"
#include "msla_diag.h"
#include "mslablof.h"

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME : mslainttst.c                                           */
/*                                                                           */
/*         POURPOSE : Test the ability of interrupting the Rios from the     */
/*                    MSLA card and viceversa. 				     */
/*                                                                           */
/*            INPUT : Pointer to the msla structure.    		     */
/*                                                                           */
/*           OUTPUT : Returns the proper code.				     */
/*                                                                           */
/* FUNCTIONS CALLED : ioctl, bload, start_msla, halt_msla, reset_msla.       */ 
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

int
int_rios(gtu)
struct mslatu *gtu;
{
    int rc, fd, k=0;
    ulong_t ioaddress, bussmem;
    ushort_t *flag0, *flag1;
    ushort_t *flag2, *handshk;
    char fname[60];
    struct msla_intr_count intr_count;

    fd = gtu->fd;
    ioaddress = gtu->msla_iobase;
    bussmem = gtu->msla_membase;

    /* Reset, Halt & Clear memory */
    mslaclrm(fd,bussmem,MAX_16BIT,ioaddress);

    flag0 = (unsigned short *) (bussmem | 0x0042);
    flag1 = (unsigned short *) (bussmem | 0x0046);
    flag2 = (unsigned short *) (bussmem | 0x004A);
    handshk = (unsigned short *) (bussmem | 0x0400);

   /* Load 68k processor */
    strcpy(fname,PARITY1_BLO);
    rc =bload(fname,bussmem,fd,7,ioaddress);
    if(rc < 0)
    {
        mkerr(INT_RIOS,GENERAL,UCODE_FILE_NOT_FOUND,&rc);
	return(rc);
    }

    reset_msla(fd,ioaddress);
    sleep(1);
    halt_msla(fd,ioaddress);

   /* Start the diagnostic mode */
    rc = ioctl(fd, MSLA_START_DIAG);
    if( rc < 0)
    {
        mkerr(INT_RIOS,GENERAL,INT_DRIVER_IOCTL,&rc);
	return(rc);
    }

   /* Enable interrups from MSLA to RIOS */
    enb_int_msla(fd,ioaddress); 

    sleep(1);

   /* Start the MSLA 68000 processor */
    start_msla(fd,ioaddress);

    sleep(1);

   /* Chech the Ucode started properly */
    if(*flag0 == 0)
    {
        mkerr(INT_RIOS,GENERAL,INT_UCODE_NOT_STARTED,&rc);
        return(rc);
    }

   /* Go to the general interrupt routine */
    sleep(1);
    if(*handshk != 0xCCDD)
    {
	/* Tells us that the 68k never gave us */
	/* the ok to send the interrupt signal */
	mkerr(INT_RIOS,GENERAL,INT_UCODE_NOT_STARTED,&rc);
	return(rc);
    }

    intrpt_msla(fd,ioaddress);
    sleep(1);
    if ((*flag1 != 0xBEEB) || (*flag2 != 0xBEEB))
    {
        mkerr(INT_RIOS,GENERAL,INTERRUPT_FAIL,&rc);
	return(rc);
    }

    if(*handshk == 0xABCD)
    {
  	/* Good, now go to parity error */
	*handshk = 0xFCDE;
    }
    else
    {
        mkerr(INT_RIOS,GENERAL,UCODE_NOT_FINISHED,&rc);
	return(rc);
    }

    sleep(1);
    if((*flag1 != 0xEEBB) || (*flag2 != 0xEEBB))
    {
        mkerr(INT_RIOS,EVEN,INTERRUPT_FAIL,&rc);
        return(rc);
    }

    sleep(1);
    if(*handshk == 0xABCD)
    {
  	/* Good, now go to parity error on odd byte*/
	*handshk = 0xFCDE;
    }
    else
    {
        mkerr(INT_RIOS,EVEN,UCODE_NOT_FINISHED,&rc);
	return(rc);
    }

    sleep(1);
    if((*flag1 != 0xCACA) || (*flag2 != 0xCACA))
    {
        mkerr(INT_RIOS,ODD,INTERRUPT_FAIL,&rc);
        return(rc);
    }
	
    rc = ioctl(fd, MSLA_QUERY_DIAG, &intr_count);
    if( rc < 0)
    {
        mkerr(INT_RIOS,GENERAL,INT_DRIVER_IOCTL,&rc);
	return(rc);
    }

    if((intr_count.total_cnt == 3) && (intr_count.parity_cnt == 2))
    {
	rc = SUCCESS;
    }
    else
    {
	rc = FAIL;
        mkerr(INT_RIOS,ODD,IOCTL_COUNT_INT_FAIL,&rc);
    }

    rc = ioctl (fd,MSLA_STOP_DIAG);
    if ( rc < 0 )
    {
        mkerr(INT_RIOS,GENERAL,INT_DRIVER_IOCTL,&rc);
	return(rc);
    }
    return(rc);
}
