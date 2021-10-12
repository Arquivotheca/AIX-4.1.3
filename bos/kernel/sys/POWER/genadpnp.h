/* @(#)14 1.1 src/bos/kernel/sys/POWER/genadpnp.h, rspccfg, bos41J, 9513A_all 3/28/95 16:38:46 */
/*
 * COMPONENT_NAME:  LIBCFG   genadpnp.h
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
/* 3/07/95 */
/* Structure map for Generic Address in PnP Vendor specific packet    */
 
/* See Plug and Play ISA Specification, Version 1.0a, March 24, 1994.   */
/* It (or later versions) is available on Compuserve in the PLUGPLAY    */
/* area.  This code has extensions to that specification, namely new    */
/* short and long tag types for platform dependent information          */
/* Warning: LE notation used throughout this file                       */
 
#ifndef _GENADPNP_
#define _GENADPNP_
 
typedef enum _AddrType {
   ISAIOAddr = 1,                      /* ISA I/O address not 10 or 16 bits   */
   IOMemAddr = 2,                      /* I/O Memory address space            */
   SysMemAddr = 3                      /* System memory address > 24 bits     */
} AddrType;
 
typedef struct _GenericAddr {
   unsigned char  Tag;                 /* large tag = 0x84 Vendor specific    */
   unsigned char  Count0;              /* lo byte of count                    */
   unsigned char  Count1;              /* hi byte of count                    */
                                       /*   count = sizeof(_GenericAddr) - 3  */
   unsigned char  Type;                /* = 9 (Generic Address)               */
   unsigned char  AddrType;            /* see enum AddrType                   */
   unsigned char  AddrInfo;            /* info for the AddrType               */
                                       /*    I/O address type: number of bits */
   unsigned char  Reserved[2];         /* reserved                            */
   unsigned char  Address[8];          /* Address                             */
   unsigned char  Length[8];           /* Number of contiguous bytes used     */
} GenericAddr;
 
#endif  /* ndef _GENADPNP_ */
 
