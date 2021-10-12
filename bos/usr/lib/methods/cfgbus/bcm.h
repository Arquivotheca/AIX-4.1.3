/* @(#)89 1.7  src/bos/usr/lib/methods/cfgbus/bcm.h, cmdbuscf, bos41J, 9516B_all 4/21/95 16:12:22 */
/*
 * COMPONENT_NAME: (CMDBUSCF) HEADER FILE
 *
 * FUNCTIONS: DEFINITIONS for cfgbus
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/* prevent multiple inclusion */
#ifndef _H_BCM
#define _H_BCM

#include <stdio.h>

extern int prntflag;		/* Used for trace output */
extern FILE *trace_file;	/* Used for trace output */

#define CARD_TABLE_SIZE 16      /* Number of slots for busquery */
#define PCI_CARD_TABLE_SIZE 256 /* Number of PCI slots/functions for busquery */
#define BUS_CONFIG_METHOD 0x520 /* Bus device's LED value */
#define XIOBIT 0x80

#define EMPTYSLOT             0xffff
#define PCI_BADVENDEV         0xFFFFFFFF
#define SCSI_ADAPTER          0x778d
#define SCSI2_ADAPTER         0xfc8e
#define SCSI_ADAPTER_POS2     0x00
#define BADISK_ADAPTER        0x9fdf
#define BADISK_ADAPTER_POS2   0x14
#define FNAME_SIZE            256
#define DEVPKG_PREFIX	      "devices"          /* device package prefix */
#define RSPC_DMA_KERNEL_EXT "isa/rspcios"

typedef struct  card_info {
        ulong   cardid;         /* device id of card                */
        int     slot_filled;    /* TRUE adapter present, FALSE no adapt*/
        char    pddv_sstr[80];  /* search string for PdDv entry     */
        char    pkg_name[32];   /* package name                     */
	char	location[16];	/* location code for device */
} CARD_INFO;

#define MCA_TYPE        0               /* signifies Micro Channel bus */
#define PCI_TYPE        1               /* signifies PCI bus           */


/* BUC device types */
#define A_BUS_BRIDGE	5


/* BUC device Id's */
#define PCI_BUS_ID      0x2010
#define ISA_BUS_ID      0x2020
#define PCMCIA_BUS      0x2030
#define DMA_BUC         0x2040

#endif /* _H_BCM */
