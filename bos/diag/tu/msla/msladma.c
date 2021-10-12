static char sccsid[] = "@(#)39  1.6  src/bos/diag/tu/msla/msladma.c, tu_msla, bos41J, 9519A_all 5/10/95 09:56:58";
/*
 * COMPONENT_NAME: ( msladma ) 
 *
 * FUNCTIONS:  msladmatest, rios_to_msla, msla_to_rios
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

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME : msladmatest                                            */
/*                                                                           */
/*         POURPOSE : Test the ability of performing a DMA test between the  */
/*                    Rios and the MSLA card.                                */
/*                                                                           */
/*            INPUT : Pointer to the msla structure.    		     */
/*                                                                           */
/*           OUTPUT : Returns the proper code.				     */
/*                                                                           */
/* FUNCTIONS CALLED : rios_to_msla, msla_to_rios.                            */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include "mslatu.h"
#include "mslatime.h"
#include "mslafdef.h"
#include "mslaerrdef.h"
#include "msladgappl.h"
#include "msla_diag.h"
#include "msladma.h"
#include "mslablof.h"


extern int dma_in_progress = FALSE; /* TMA */

int
msladmatest(gtu)
struct  mslatu *gtu;
{
    int   rc;

    rc = rios_to_msla (gtu);
    if(rc != SUCCESS)
    {
	return(rc);
    }

    rc = msla_to_rios (gtu);
    if(rc != SUCCESS)
    {
	return(rc);
    }

    return(rc);
}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME : msladma.c                                              */
/*                                                                           */
/*         POURPOSE : Test the ability of transfering data from the Rios     */
/*                    to the MSLA card.                                      */
/*                                                                           */
/*            INPUT : Pointer to the msla structure.    		     */
/*                                                                           */
/*           OUTPUT : Returns the proper code.				     */
/*                                                                           */
/* FUNCTIONS CALLED : mslaclrm, bload, ioctl.                                */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

rios_to_msla (gtu)
struct  mslatu *gtu;
{
    int rc, fd;
    char buffer[6], fname[60];
    ulong_t bussmem, ioaddress;
    volatile ushort_t  *flag0; /* TMA */
    volatile unsigned char *ret; /* TMA */
    ulong *addr, *realaddr;
    ulong  temp_loc ;
    struct dma_test_parms dma_parms;

    fd = gtu->fd;
    ioaddress = gtu->msla_iobase;
    bussmem = gtu->msla_membase;

    flag0 = (ushort_t *) (bussmem | DMA_FLAG);
    ret = (unsigned char *) (bussmem | DMA_MSGS);
    addr = (unsigned long *) (bussmem | DMA_ADDR);
 
    buffer[0] = 'x';
    buffer[1] = 'y';
    buffer[2] = 'z';
    buffer[3] = 'w';
 
/* Initialise the realaddr pointer to location where DD will place 
   system memory DMA Xfer address */
    realaddr = & temp_loc ;
    dma_parms.ubuff_adr = (ulong)buffer ;    
    dma_parms.dma_adr_p = realaddr;
    dma_parms.count = 4;

   /* Clear the MSLA memory */
    mslaclrm(fd,bussmem,MAX_16BIT,ioaddress);


   /* Load the ucode to MSLA */
    strcpy(fname,MSLA_DMA2);
    rc = bload(fname,bussmem,fd,7,ioaddress);
    if(rc < 0)
    {
        mkerr(DMATEST,RIOS_TO_MSLA,rc,&rc);
	return(rc);
    }

/*  Get into Diagnostics mode */
    rc = ioctl (fd,MSLA_START_DIAG);
/* refer to mslafdef.h for PRINT and PRNTE debugging aids */
    PRINT(("ioctl MSLA_START_DIAG rc: %d\n", rc));
    if ( rc < 0)
    {
        mkerr(DMATEST,RIOS_TO_MSLA,DRIVER_IOCTL,&rc);
	return(rc);
    }

   /* Setup the Starting point of the DMA */

    dma_in_progress = TRUE; /* TMA */

    rc = ioctl (fd,MSLA_START_DMA,&dma_parms);
    PRINT(("ioctl MSLA_START_DMA rc: %d\n", rc));
    if ( rc < 0)
    {
        mkerr(DMATEST,RIOS_TO_MSLA,DRIVER_IOCTL,&rc);
	return(rc);
    }

   /* Give the address to SRC address to the 68k */
    *addr = *dma_parms.dma_adr_p;

   PRINT(("starting the 68k\n"));
   /* start the 68k */
    start_msla(fd,ioaddress);

   PRINT(("delay\n"));
    DELAY_MS(2);

    PRINT(("*flag0: 0x%x\n", *flag0));
    if(*flag0 != 0xCACA)
    {
        mkerr(DMATEST,RIOS_TO_MSLA,DMA_UCODE_NOT_STARTED,&rc);
	return(rc);
    }

    if(*ret != 0x78)
    {
        mkerr(DMATEST,RIOS_TO_MSLA,TRANSFER_FAIL,&rc);
	return(rc);
    }
/*
    printf("FLAG0 Return: 0x%x\n",*flag0);
    printf("Byte1 Return: 0x%x\n",*ret++);
    printf("Byte2 Return: 0x%x\n",*ret++);
    printf("Byte3 Return: 0x%x\n",*ret++);
    printf("Byte4 Return: 0x%x\n",*ret);
*/

   /*  Stop the DMA mode */
    rc = ioctl (fd,MSLA_STOP_DMA);
    if ( rc < 0 )
    {
        mkerr(DMATEST,RIOS_TO_MSLA,DRIVER_IOCTL,&rc);
	return(rc);
    }

    dma_in_progress = FALSE; /* TMA */


/* 
    Stop the DIAGNOSTIC mode */
    rc = ioctl (fd,MSLA_STOP_DIAG);
    if ( rc < 0 )
    {
        mkerr(DMATEST,RIOS_TO_MSLA,DRIVER_IOCTL,&rc);
	return(rc);
    }

    return(rc);
}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME : msla_to_rios                                           */
/*                                                                           */
/*         POURPOSE : Test the ability of transfering data from the MSLA     */
/*                    card to the Rios.                                      */
/*                                                                           */
/*            INPUT : Pointer to the msla structure.    		     */
/*                                                                           */
/*           OUTPUT : Returns the proper code.				     */
/*                                                                           */
/* FUNCTIONS CALLED : mslaclrm, bload, ioctl.                                */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

msla_to_rios (gtu)
struct  mslatu *gtu;
{
    int rc, fd;
    char buffer[6], fname[20];
    ulong_t bussmem, ioaddress;
    volatile ushort_t  *flag0; /* TMA */
    volatile unsigned char *ret; /* TMA */
    ulong *addr, *realaddr;
    ulong  temp_loc ;
    struct dma_test_parms dma_parms;

    fd = gtu->fd;
    ioaddress = gtu->msla_iobase;
    bussmem = gtu->msla_membase;

    flag0 = (ushort_t *) (bussmem | DMA_FLAG);
    ret = (unsigned char *) (bussmem | DMA_MSGS);
    addr = (unsigned long *) (bussmem | DMA_ADDR);
 
/* Initialise the realaddr pointer to location where DD will place 
   system memory DMA Xfer address */

    realaddr = & temp_loc ;
    dma_parms.ubuff_adr = (ulong)buffer;
    dma_parms.dma_adr_p = (unsigned long *)realaddr;
    dma_parms.count = 4;

   /* Clear the MSLA memory */
    mslaclrm(fd,bussmem,MAX_16BIT,ioaddress);

   /* Load the ucode to MSLA */
    strcpy(fname,MSLA_DMA1);
    rc = bload(fname,bussmem,fd,7,ioaddress);
    if(rc < 0)
    {
        mkerr(DMATEST,MSLA_TO_RIOS,rc,&rc);
	return(FAIL);
    }

   /* Get into Diagnostics mode */
    rc = ioctl (fd,MSLA_START_DIAG);
    if ( rc < 0)
    {
        mkerr(DMATEST,MSLA_TO_RIOS,DRIVER_IOCTL,&rc);
	return(rc);
    }

   /* Setup the Starting point of the DMA */

    dma_in_progress = TRUE; /* TMA */

    rc = ioctl (fd,MSLA_START_DMA,&dma_parms);
    if ( rc < 0)
    {
        mkerr(DMATEST,MSLA_TO_RIOS,DRIVER_IOCTL,&rc);
	return(rc);
    }

   /* Give the destination address to the 68k */
    *addr = *dma_parms.dma_adr_p;

   /* Start the 68k */
    start_msla(fd,ioaddress);

    DELAY_MS(2);

    if(*flag0 != 0xCACA)
    {
        mkerr(DMATEST,MSLA_TO_RIOS,DMA_UCODE_NOT_STARTED,&rc);
	return(rc);
    }

    if(*ret != 0xDE)
    {
        mkerr(DMATEST,MSLA_TO_RIOS,TRANSFER_FAIL,&rc);
	return(rc);
    }

/*
    printf("FLAG0 Return: 0x%x\n",*flag0);
    printf("Byte1 Return: 0x%x\n",*ret++);
    printf("Byte2 Return: 0x%x\n",*ret++);
    printf("Byte3 Return: 0x%x\n",*ret++);
    printf("Byte4 Return: 0x%x\n",*ret);
*/

    rc = ioctl (fd,MSLA_STOP_DMA);
    if ( rc < 0 )
    {
        mkerr(DMATEST,MSLA_TO_RIOS,DRIVER_IOCTL,&rc);
	return(rc);
    }

    dma_in_progress = FALSE; /* TMA */


    rc = ioctl (fd,MSLA_STOP_DIAG);
    if ( rc < 0 )
    {
        mkerr(DMATEST,MSLA_TO_RIOS,DRIVER_IOCTL,&rc);
	return(rc);
    }

    return(rc);
}

