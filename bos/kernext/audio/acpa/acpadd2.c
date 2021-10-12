#pragma options opt=0
/* @(#)06 1.2 src/bos/kernext/audio/acpa/acpadd2.c, sysxacpa, bos411, 9428A410j 91/07/11 18:51:29 */

/*
 * COMPONENT_NAME: SYSXACPA     Multimedia Audio Capture and Playback Adapter
 *
 * FUNCTIONS: acpaout_mult
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

/*****  Audio Support Software  *****************************************/
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      The module contains the ACPA routine that sends one block 	*/
/*      of audio data to the adapter.                                   */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*                                                                      */
/*      This module is for the RISC System/6000 only.                   */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      acpaout_mult( port, data, size )                                */
/*      int port;                base address where data goes           */
/*      unsigned short *data;    the data to copy to the adapter        */
/*      int size;                the number of shorts to copy           */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*      See above                                                       */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      N/A                                                             */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/*  RETURN VALUES:                                                      */
/*                                                                      */
/*      N/A                                                             */
/*                                                                      */
/*****  System Include Files  *******************************************/

#include <sys/device.h>		/* Device driver stuff */
#include <sys/ioacc.h>          /* Macros for accessing I/O hardware */
#include <sys/adspace.h>

#ifdef ORIGINAL
#define BUS_ID	0x820C0020
#else
#define BUS_ID  0x820C0020
#endif /* ORIGINAL */

void acpaout_mult(int port, unsigned short *data, int size )
{
  volatile unsigned char *c;
  volatile unsigned short *p;
  volatile ulong bus_val;
  volatile int i;

  bus_val = BUSIO_ATT( BUS_ID, 0 );
  for ( i=0; i<size; i++ )
  {
    c = (char *) bus_val + port;
    p = (unsigned short *) c;
    BUSIO_PUTS( p, *data );
    data++;
  }
  BUSIO_DET(bus_val);
}


