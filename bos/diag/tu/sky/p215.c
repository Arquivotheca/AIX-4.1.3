static char sccsid[] = "@(#)96	1.2  src/bos/diag/tu/sky/p215.c, tu_sky, bos411, 9428A410j 10/29/93 13:40:45";
/*
 *   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: getbaseadd
 *		getdmabaseaddr
 *		getiobaseadd
 *		getvbaseadd
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "skytu.h"
#include <sys/entdisp.h>
extern struct sky_map skydat;

/**********************************************************
* Name     : getvbaseadd                                  *
* Function : returns the base address of the VRAM used    *
*            on the skyway card                           *
*                                                         *
**********************************************************/
getvbaseadd() 
{ return(segreg + skydat.vr_addr);
}

/**********************************************************
* Name     : getbaseadd                                   *
* Function : returns the base address of the CoProcessor  *
*            registers.                                   *
*                                                         *
**********************************************************/
getbaseadd()
{ return (segreg + skydat.cp_addr);
}
/**********************************************************
* Name     : getiobaseadd                                 *
* Function : returns the base address of the direct Access*
*            registers.                                   *
*                                                         *
**********************************************************/
getiobaseadd()
{ return(segreg + skydat.io_addr);
}
/**********************************************************
* Name     : getdmabaseadd                                *
* Function : returns the base address of the DMA Buffer   *
*                                                         *
**********************************************************/
getdmabaseaddr()
{ return(skydat.dma_addr[0]);
}
