static char sccsid[] = "@(#)tu_dma.c	1.2 2/1/91 14:29:36";
/*****************************************************************************
* TU_DMA.C:		 Test Unit Code for DMA test.		 1/22/90     *
*									     *
* functions:		 tu_dma.c					     *
*									     *
* Origins:		 27 (IBM)					     *
*									     *
* (C) Copyright International Business Machines Corp. 1990		     *
* All Rights Reserved							     *
* Licensed Materials - Property of IBM					     *
*									     *
* US Government Users Restricted Rights - Use, duplication or		     *
* disclosure resticted by GSA ADP Schedule Contract with IBM Corp.	     *
*****************************************************************************/
#include <stdio.h>
#include "psca.h"
#include "pscatu.h"
#include "extern.h"

tu_dma(cb)
TUTYPE	 *cb;
{

#ifdef AIX

     static    BYTE *testbuf;
     static    int  length;
     int       i,j;
     int	rc;

#ifndef HTX
	cb->pr.dmasize = 4096;
	cb->pr.sram_addr = 4096;
#endif

     if (cb->pr.dmasize != length && length != 0) {
	free(testbuf);
	testbuf = NULL;
     }
    
     length = cb->pr.dmasize;
     if (testbuf == NULL && (testbuf = malloc(length)) == NULL) {
	sprintf(MSGBUF,"tu_dma: out of memory\n");
	return(TU_SYS);
     }	

     for(i=0; i<length-2; i++)
	 testbuf[i]=i % 256;
     ROF(write_sram_dma(cb->pr.sram_addr, testbuf, length-2));
     RPT_VERBOSE("DMA WRITE RETURNED\n");
     for(i=0; i<length-2; i++)
	 testbuf[i]=0;
     ROF(read_sram_dma(cb->pr.sram_addr, testbuf, length-2));
     RPT_VERBOSE("DMA READ RETURNED\n");
     for(i=0; i<length-2; i++)
	 if ((j=testbuf[i]) != i % 256 ) {
	    sprintf(MSGBUF,"tu_dma: expected %02x, received %02x\n",i%256,j);
	    return(TU_HARD);
	 }
     RPT_VERBOSE("DMA XFERS WORKED USING DMA_SIZE - 2\n");

     for(i=0; i<length; i++)
	 testbuf[i]=i % 256;
     ROF(write_sram_dma(cb->pr.sram_addr, testbuf, length));
     RPT_VERBOSE("DMA WRITE RETURNED\n");
     for(i=0; i<length; i++)
	 testbuf[i]=0;
     ROF(read_sram_dma(cb->pr.sram_addr, testbuf, length));
     RPT_VERBOSE("DMA READ RETURNED\n");
     for(i=0; i<length; i++)
	 if ((j=testbuf[i]) != i % 256) {
	    sprintf(MSGBUF,"tu_dma: expected %02x, received %02x\n",i%256,j);
	    return(TU_HARD);
	 }
#endif
#ifdef MTEX
     RPT_VERBOSE("DMA not possible under MTEX\n");
#endif

     return(TU_GOOD);
}
