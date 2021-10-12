/* @(#)85	1.1  src/htx/usr/lpp/htx/lib/hga/hga_info.h, tu_hga, htx410 6/2/94 11:37:22  */
/*
 *   COMPONENT_NAME: tu_hga
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _HGA_INFO_H
#define _HGA_INFO_H

#define NUM_PCI_SLOTS   2
#define PCI_EQUIP_PRESNT 0x080cL

#define PORT_80C_SLOT1 0x08
#define PORT_80C_SLOT2 0x04

/* PCI Slots  Page 38 in Sandalfoot  spec */
#define PCI_SLOT1_BASE 0x00802000
#define PCI_SLOT2_BASE 0x00804000

#define S3_VENDOR_ID     0x3353
#define S3_DEVICE_ID     0xb088
#define S3_RESPOND_IO    0x0001
#define S3_RESPOND_MEM   0x0002
#define S3_RESPOND_NODAC 0x0020

typedef struct _S3_CONFIG_SPACE {
	ushort vendor_id;            /* 00h Vendor ID = 3355                 */
	ushort device_id;            /* 02h Device ID = b088                 */
	ushort command;              /* 04h Command - set to I/O Enable      */
	ushort status;               /* 06h Status                           */
	uchar  revision_id;          /* 08h Reviosion ID                     */
	uchar  reserved1;            /* 09h reserved                         */
	uchar  interface;            /* 0ah Programming Interface            */
	uchar  reserved2;            /* 0bh reserved                         */
	ushort reserved3;            /* 0ch reserved                         */
	ushort reserved4;            /* 0eh reserved                         */
	ushort base_low;             /* 10h Base Address Low                 */
	ushort base_high;            /* 12h Base Address High                */
} S3_CONFIG_SPACE;


#endif /* _HGA_INFO_H */
