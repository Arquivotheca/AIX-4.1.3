/* @(#)75  1.4  src/bos/kernext/disp/inc/pci_regs.h, dispcfg, bos411, 9428A410j 5/3/94 15:52:56 */
/* pci_regs.h */

#ifndef _H_PCI_REGS
#define _H_PCI_REGS

/*
 *   COMPONENT_NAME: (bbldd)
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

/******************************************************************************
*******************************************************************************
**                                                                           **
** Contains the PCI bus register access macros                               **
**                                                                           **
*******************************************************************************
******************************************************************************/

#define BYTE_REV(n)                                               \
        ((((n) & 0x000000FF) << 24) + (((n) & 0x0000FF00) << 8) + \
        (((n) & 0x00FF0000) >> 8) + (((n) & 0xFF000000) >> 24))

#define BUS_PCI_VENDOR_ID                       0x0000
#define BUS_PCI_COMMAND                         0x0004
#define BUS_PCI_REV_ID                          0x0008

/******************************************************************************
*                                                                             *
*       Define the PCI bus Device Base Real Address Register (word incr = 4): *
*       NOTE: This is a READ/WRITE register.                                  *
*                                                                             *
******************************************************************************/

#define BUS_PCI_DEV_BASE_REAL_ADDR_REG          0x0010

#define BUS_PCI_ROM_BASE_ADDRESS_REG            0x0030

/******************************************************************************
*                                                                             *
*       Define the PCI bus Device Characteristics Register:                   *
*       NOTE: This is a READ-ONLY register.                                   *
*                                                                             *
******************************************************************************/

#define BUS_PCI_DEV_CHAR_REG                    0x0040

/******************************************************************************
*                                                                             *
*       Define the PCI bus Device ID Register (for word increment equal 4):   *
*       NOTE: This is a READ-ONLY register.                                   *
*                                                                             *
******************************************************************************/

#define BUS_PCI_DEV_ID_REG                      0x0044

/******************************************************************************
*                                                                             *
*       Define the PCI bus Device BUS UNIT ID Registers (word incr equal 4):  *
*       NOTE: These registers are READ/WRITE                                  *
*                                                                             *
******************************************************************************/

#define BUS_PCI_BUID_REG_1                      0x0048

#define BUS_PCI_APERTURE_SELECT_REG             0x0070

#define BUS_PCI_START_CFG_REGS                          BUS_PCI_VENDOR_ID
#define BUS_PCI_END_CFG_REGS                            BUS_PCI_APERTURE_SELECT_REG +4

/******************************************************************************
*                                                                             *
*       Define the PCI Configuration Space access macros 		      *                    
*                                                                             *
******************************************************************************/
/*----------------------------------------------------------------------
|
|       FUNCTION DESCRIPTION
|
|   These functions are ONLY for accessing the PCI Standard
|   Configuration Space.  All reads are relative to the beginning 
|   of the configuration space for the bus/slot.
|
|----------------------------------------------------------------------*/
#define rd_pci_std_cfg_reg_w( bus_num, bus_slot, cfg_offset, data_addr ) \
        rw_pci_std_cfg_regs((bus_slot),(cfg_offset),(data_addr),(bus_num), 4, 0 )

#define wr_pci_std_cfg_reg_w( bus_num, bus_slot, cfg_offset, data_addr ) \
        rw_pci_std_cfg_regs((bus_slot),(cfg_offset),(data_addr),(bus_num), 4, 1 )

#define rd_pci_std_cfg_reg_s( bus_num, bus_slot, cfg_offset, data_addr ) \
        rw_pci_std_cfg_regs((bus_slot),(cfg_offset),(data_addr),(bus_num), 2, 0 )

#define wr_pci_std_cfg_reg_s( bus_num, bus_slot, cfg_offset, data_addr ) \
        rw_pci_std_cfg_regs((bus_slot),(cfg_offset),(data_addr),(bus_num), 2, 1 )

#endif /* _H_PCI_REGS */

