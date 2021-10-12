/* @(#)15 1.1 src/bos/kernel/sys/POWER/isapnp.h, rspccfg, bos41J, 9513A_all 3/28/95 16:39:05 */
/*
 * COMPONENT_NAME:  LIBCFG   isapnp.h
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* 3/07/95                                                            */
/* Structure map for ISA Bridge in PnP Vendor specific packet         */
 
/* See Plug and Play ISA Specification, Version 1.0a, March 24, 1994. */
/* It (or later versions) is available on Compuserve in the PLUGPLAY  */
/* area.  This code has extensions to that specification, namely new  */
/* short and long tag types for platform dependent information        */
/* Warning: LE notation used throughout this file                     */
 
#ifndef _ISAPNP_
#define _ISAPNP_
 
#define MAX_ISA_INTS    16
 
typedef struct _ISAInfoPack {
   unsigned char  Tag;                 /* large tag = 0x84 Vendor specific   */
   unsigned char  Count0;              /* lo byte of count                   */
   unsigned char  Count1;              /* hi byte of count                   */
                                       /*   count = sizeof(_ISAInfoPack) - 3 */
   unsigned char  Type;                /* = 0x0A (ISA bridge)                */
   unsigned char  IntCtrlType;         /* Interrupt controller type          */
                                       /* enum _IntTypes in pcipnp.h         */  
   unsigned char  IntCtrlNumber;       /* Interrupt controller number        */
                                       /*   0      8259 interrupt            */
                                       /*   0      MPIC interrupt            */
                                       /*   buid   RS6K interrupt            */
   unsigned short Int[MAX_ISA_INTS];   /* Interrupt mapping table            */
                                       /*   index  0 for IRQ0                */
                                       /*   index  1 for IRQ1                */
                                       /*   index 15 for IRQ15               */
                                       /*   0xFFFF if not usable             */
} ISAInfoPack;
 
#endif  /* ndef _ISAPNP_ */
 
