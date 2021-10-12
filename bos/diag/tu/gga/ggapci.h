/* @(#)74	1.1  src/bos/diag/tu/gga/ggapci.h, tu_gla, bos41J, 9515A_all 4/6/95 09:27:03 */
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: none
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

#define NUM_SUPPORTED_MACHINES 3 /* Sandlefoot, Delmar/Southwind, Carolina */
                                 /* Polo is also supported but via indexed */
                                 /* PCI code which executes seperately.    */
#define NUM_PCI_SLOTS          2
#define PCI_EQUIP_PRESNT       0x80cL

#define PORT_80C_SLOT1         0x08
#define PORT_80C_SLOT2         0x04

/*** PCI Slots ***/

/* Sandalfoot slot addresses */
#define SAND_PCI_SLOT1_BASE         0x80802000L
#define SAND_PCI_SLOT2_BASE         0x80804000L

/* Delmar/Southwind slot addresses */
#define DELSOUTH_PCI_SLOT1_BASE     0x80C00000L
#define DELSOUTH_PCI_SLOT2_BASE     0x80840000L
#define DELSOUTH_PCI_SLOT2_BASE_EC  0x80A00000L

/* Carolina slot addresses */
#define CARO_PCI_SLOT1_BASE         0x80C00000L
#define CARO_PCI_SLOT2_BASE         0x80840000L

/* Polo PCI index/data register addresses */
#define PCI_SLOT_BASE          0x00700080L  /* Byte reversed */
#define PCIINDEX               0x80000cf8L  /* Polo PCI index register */
#define PCIDATA                0x80000cfcL  /* Polo PCI data register  */

#define NON_POLO_MACHINE 0
#define POLO_MACHINE     1

typedef struct
  {
        ushort vendor_id;
        ushort device_id;
        ushort command;
        ushort status;
        uchar  revision_id;
        uchar  reserved1;
        uchar  interface;
        uchar  reserved2;
        ushort reserved3;
        ushort reserved4;
        ushort base_low;
        ushort base_high;
  } PCI_CONFIG_SPACE;

/*****************************************************************************/
/***************************** PCI Register Offsets **************************/
/*****************************************************************************/

#define VENDOR_ID_REG          0x00
#define DEVICE_ID_REG          0x02
#define COMMAND_REG            0x04
#define DEVICE_STATUS_REG      0x06
#define REVISION_ID_REG        0x08
#define STD_PROG_INT_REG       0x09
#define SUBCLASS_CODE_REG      0x0A
#define CLASS_CODE_REG         0x0B
#define CACHE_LINE_SIZE_REG    0x0C
#define LATENCY_TIMER_REG      0x0D
#define HEADER_TYPE            0x0E
#define BIST_CONTROL_REG       0x0F
#define BUFFER_BASE_ADDR_REG   0x10
#define INTERRUPT_LINE_REG     0x3C
#define INTERRUPT_PIN_REG      0x3D
#define MIN_GRANT_REG          0x3E
#define MAX_GRANT_REG          0x3F

/*************************** PCI Register Values  ****************************/

#define IBM_ID                 0x100e
#define P9100_ID               0x9100
#define MEM_ACCESS_ENABLED     0x0002
#define FIXED_AT_ZERO          0x00
#define FIXED_AT_255           0xFF
#define OTHER_VIDEO_ADAPTER    0x80
#define DISPLAY_CONTROLLER     0x03
#define NO_BIST_SUPPORT        0x00
#define INT_TIED_TO_INTA       0x01


