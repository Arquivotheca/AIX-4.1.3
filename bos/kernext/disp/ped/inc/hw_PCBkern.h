/* @(#)58  	1.5.1.4  src/bos/kernext/disp/ped/inc/hw_PCBkern.h, peddd, bos411, 9434B411a 8/3/94 12:33:13 */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: *		
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



#ifndef _H_HW_PCBKERN
#define _H_HW_PCBKERN

 /*-------------------------------------------------------------------------*/
 /* All the macros have been moved out of here and back into com/inc/mid    */
 /*-------------------------------------------------------------------------*/

#include <hw_PCBrms.h>   

typedef struct _MIDdpms
{
        MIDpcbhdr               hdr;    /* SE header info (len/opcode) */
}       MIDdpms, *pMIDdpms;

#define MID_DisplayPowerDownPCB()                                     	\
{                                                                       \
    MIDdpms      SE;                                     		\
                                                                        \
    SE.hdr.halfword.length = ( ushort ) sizeof(MIDdpms) ;		\
    SE.hdr.halfword.op     = ( ushort ) OP_DISPLAY_POWER_DOWN ;      	\
                                                               		\
    MID_WR_PCB( &SE , ( sizeof( MIDdpms ) >> 2 ) );      		\
                                                                        \
}

#define MID_DisplayPowerUpPCB()                                     	\
{                                                                       \
    MIDdpms      SE;                                                    \
                                                                        \
    SE.hdr.halfword.length = ( ushort ) sizeof(MIDdpms) ;               \
    SE.hdr.halfword.op     = ( ushort ) OP_DISPLAY_POWER_UP ;         	\
                                                                        \
    MID_WR_PCB( &SE , ( sizeof( MIDdpms ) >> 2 ) );                     \
                                                                        \
}

#endif /* _H_HW_PCBKERN */
