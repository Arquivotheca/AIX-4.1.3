/* @(#)84	1.1  src/bos/kernel/sys/POWER/pcipnp.h, rspc_softros, bos41J, 9509A_all 2/24/95 15:34:28  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_PCIPNP
#define _H_PCIPNP

/*
 *         WARNING!!!!   WARNING!!!!
 *
 *  This include file is in transition and will most likely be changing after
 *  release 4.1.1.  It should not be shipped, but is here for internal use
 *  only.  Please contact the rspc_softros component owner if you are using
 *  this file so we can track the users.
 */

/*----------------------------------------------------------------------------*/
/* Structure map for PCI Bridge in PnP Vendor specific packet                 */

/* See Plug and Play ISA Specification, Version 1.0a, March 24, 1994.         */
/* It (or later versions) is available on Compuserve in the PLUGPLAY          */
/* area.  This code has extensions to that specification, namely new          */
/* short and long tag types for platform dependent information                */
/* Warning: LE notation used throughout this file                             */

#ifndef _PCIPNP_
#define _PCIPNP_

#define MAX_PCI_INTS    4

typedef enum _IntTypes {               /* interrupt cntlr types enumerator    */
   IntInvalid = 0,                     /*    invalid value                    */
   IntCtl8259 = 1,                     /*    8259                             */
   IntCtlMPIC = 2,                     /*    MPIC                             */
   IntCtlRS6K = 3                      /*    RS6K                             */
   } _IntTypes;

typedef struct _IntMap {               /* PCI Int to system conversion map    */
   unsigned char  SlotNumber;          /* First slot = 1; Integrated = Slot 0 */
   unsigned char  DevFuncNumber;       /* PCI slot's DeviceFunction number    */
   unsigned char  IntCtrlType;         /* Interrupt controller type           */
   unsigned char  IntCtrlNumber;       /* Interrupt controller number         */
                                       /*   0      8259 interrupt             */
                                       /*   0      MPIC interrupt             */
                                       /*   buid   RS6K interrupt             */
   unsigned short Int[MAX_PCI_INTS];   /* Interrupt mapping table             */
                                       /*    index 0 for INTA                 */
                                       /*    index 1 for INTB                 */
                                       /*    index 2 for INTC                 */
                                       /*    index 3 for INTD                 */
                                       /*    0xFFFF if not usable             */
   } IntMap;

typedef struct _PCIInfoPack {
   unsigned char  Tag;                 /* large tag = 0x84 Vendor specific    */
   unsigned char  Count0;              /* lo byte of count                    */
   unsigned char  Count1;              /* hi byte of count                    */
      /* count = number of pluggable PIC slots * sizeof(IntMap) + 21          */
   unsigned char  Type;                /* = 3 (PCI bridge)                    */
   unsigned char  ConfigBaseAddress[8];/* Base address of PCI Configuration   */
                                       /*    system real address              */
   unsigned char  ConfigBaseData[8];   /* Base address of PCI Config data     */
                                       /*    system real address              */
   unsigned char  BusNumber;           /* PCI Bus number                      */
   unsigned char  Reserved[3];         /* Reserved (padded with 0)            */
   IntMap         Map[1];              /* Interrupt map array for each PCI    */
                                       /*  slots that are pluggable           */
                                       /*  number = (count-21)/sizeof(IntMap) */
   } PCIInfoPack;

#endif  /* ndef _PCIPNP_ */
#endif /* _H_PCIPNP */
