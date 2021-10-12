/* @(#)30  1.4  src/bos/kernext/disp/ped/inc/bishm.h, peddd, bos411, 9428A410j 3/19/93 18:51:15 */
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



#ifndef _H_BISHM
#define _H_BISHM

/****************************************************************************/
/*                      BIM Register Offset Definitions                     */
/****************************************************************************/
#define   DMA_IO_PORT     0x00
#define   DMA_HOST_ADDR   0x01
#define   DMA_LENGTH      0x02
#define   DMA_STRIDE      0x03
#define   DMA_WIDTH       0x04
#define   DMA_XFER_COUNT  0x05
#define   DMA_IO_ADDR     0x06
#define   DMA_SUBPAGE_CRT 0x07
#define   DMA_CONTROL     0x08
#define   DMA_STATUS      0x09

#define   PIO_IN_PORT_0   0x0A
#define   PIO_IN_PORT_1   0x0B
#define   PIO_IN_PORT_2   0x0C
#define   PIO_IN_PORT_3   0x0D
#define   PIO_IN_PORT_4   0x0E
#define   PIO_IN_PORT_5   0x0F
#define   PIO_IN_PORT_6   0x10
#define   PIO_IN_PORT_7   0x11
#define   PIO_IN_PORT_8   0x12
#define   PIO_IN_PORT_9   0x13
#define   PIO_IN_PORT_A   0x14
#define   PIO_IN_PORT_B   0x15
#define   PIO_IN_PORT_C   0x16
#define   PIO_IN_PORT_D   0x17
#define   PIO_IN_PORT_E   0x18
#define   PIO_IN_PORT_F   0x19
#define   PIO_HIGH_ADDR   0x1A
#define   PIO_LOW_ADDR    0x1B
#define   PIO_CUR_ADDR    0x1C
#define   PIO_FREE_SPACE  0x1D
#define   PIO_FREE_UPDATE 0x1E
#define   PIO_SUBPAGE_CTR 0x1F
#define   PIO_HIGH_WATER  0x20
#define   PIO_STATUS      0x21

#define   PCB_IN_PORT_0   0x22
#define   PCB_IN_PORT_1   0x23
#define   PCB_IN_PORT_2   0x24
#define   PCB_IN_PORT_3   0x25
#define   PCB_IN_PORT_4   0x26
#define   PCB_IN_PORT_5   0x27
#define   PCB_IN_PORT_6   0x28
#define   PCB_IN_PORT_7   0x29
#define   PCB_OUT_PORT    0x2A
#define   PCB_STATUS      0x2B

#define   IND_CONTROL     0x2C
#define   IND_ADDRESS     0x2D
#define   IND_DATA        0x2E

#define   IO_INT_MASK     0x2F
#define   IO_STATUS       0x30

#define   HOST_INT_MASK   0x31
#define   HOST_STATUS     0x32
#define   HOST_COMMO      0x33

#define   DSP_COMMO       0x34
#define   DSP_SOFT_INT    0x35
#define   BIREGCNT        0x36

#define   POS_REGISTER_0  0x00
#define   POS_REGISTER_1  0x01
#define   POS_REGISTER_2  0x02
#define   POS_REGISTER_3  0x03
#define   POS_REGISTER_4  0x04
#define   POS_REGISTER_5  0x05
#define   POS_REGISTER_6  0x06
#define   POS_REGISTER_7  0x07
#define   BIPOSCNT        0x08

/****************************************************************************/
/*                   BIM Host Register Address Definitions                  */
/****************************************************************************/
#define   H_DMA_IO_PORT     0x00
#define   H_DMA_HOST_ADDR   0xFF
#define   H_DMA_LENGTH      0xFF
#define   H_DMA_STRIDE      0xFF
#define   H_DMA_WIDTH       0xFF
#define   H_DMA_XFER_COUNT  0xFF
#define   H_DMA_IO_ADDR     0xFF
#define   H_DMA_SUBPAGE_CRT 0xFF
#define   H_DMA_CONTROL     0xFF
#define   H_DMA_STATUS      0xFF

#define   H_PIO_IN_PORT_0   0x00
#define   H_PIO_IN_PORT_1   0x04
#define   H_PIO_IN_PORT_2   0x08
#define   H_PIO_IN_PORT_3   0x0C
#define   H_PIO_IN_PORT_4   0x10
#define   H_PIO_IN_PORT_5   0x14
#define   H_PIO_IN_PORT_6   0x18
#define   H_PIO_IN_PORT_7   0x1C
#define   H_PIO_IN_PORT_8   0x20
#define   H_PIO_IN_PORT_9   0x24
#define   H_PIO_IN_PORT_A   0x28
#define   H_PIO_IN_PORT_B   0x2C
#define   H_PIO_IN_PORT_C   0x30
#define   H_PIO_IN_PORT_D   0x34
#define   H_PIO_IN_PORT_E   0x38
#define   H_PIO_IN_PORT_F   0x3C
#define   H_PIO_HIGH_ADDR   0xFF
#define   H_PIO_LOW_ADDR    0xFF
#define   H_PIO_CUR_ADDR    0xFF
#define   H_PIO_FREE_SPACE  0x40
#define   H_PIO_FREE_UPDATE 0xFF
#define   H_PIO_SUBPAGE_CTR 0xFF
#define   H_PIO_HIGH_WATER  0xFF
#define   H_PIO_STATUS      0xFF

#define   H_PCB_IN_PORT_0   0x50
#define   H_PCB_IN_PORT_1   0x54
#define   H_PCB_IN_PORT_2   0x58
#define   H_PCB_IN_PORT_3   0x5C
#define   H_PCB_IN_PORT_4   0x60
#define   H_PCB_IN_PORT_5   0x64
#define   H_PCB_IN_PORT_6   0x68
#define   H_PCB_IN_PORT_7   0x6C
#define   H_PCB_OUT_PORT    0xFF
#define   H_PCB_STATUS      0xFF

#define   H_IND_CONTROL     0x70
#define   H_IND_ADDRESS     0x74
#define   H_IND_DATA        0x78

#define   H_IO_INT_MASK     0x7C
#define   H_IO_STATUS       0xFF

#define   H_HOST_INT_MASK   0x80
#define   H_HOST_STATUS     0x84
#define   H_HOST_COMMO      0x8C

#define   H_DSP_COMMO       0x88
#define   H_DSP_SOFT_INT    0xFF

#define   H_POS_REGISTER_0  0x00
#define   H_POS_REGISTER_1  0x01
#define   H_POS_REGISTER_2  0x02
#define   H_POS_REGISTER_3  0x03
#define   H_POS_REGISTER_4  0x04
#define   H_POS_REGISTER_5  0x05
#define   H_POS_REGISTER_6  0x06
#define   H_POS_REGISTER_7  0x07

/****************************************************************************/
/*                   BIM DSP Register Address Definitions                   */
/****************************************************************************/
#define   D_DMA_IO_PORT     0x00
#define   D_DMA_HOST_ADDR   0x01
#define   D_DMA_LENGTH      0x02
#define   D_DMA_STRIDE      0x03
#define   D_DMA_WIDTH       0x04
#define   D_DMA_XFER_COUNT  0x05
#define   D_DMA_IO_ADDR     0x06
#define   D_DMA_SUBPAGE_CRT 0x07
#define   D_DMA_CONTROL     0x08
#define   D_DMA_STATUS      0x09

#define   D_PIO_IN_PORT_0   0xFF
#define   D_PIO_IN_PORT_1   0xFF
#define   D_PIO_IN_PORT_2   0xFF
#define   D_PIO_IN_PORT_3   0xFF
#define   D_PIO_IN_PORT_4   0xFF
#define   D_PIO_IN_PORT_5   0xFF
#define   D_PIO_IN_PORT_6   0xFF
#define   D_PIO_IN_PORT_7   0xFF
#define   D_PIO_IN_PORT_8   0xFF
#define   D_PIO_IN_PORT_9   0xFF
#define   D_PIO_IN_PORT_A   0xFF
#define   D_PIO_IN_PORT_B   0xFF
#define   D_PIO_IN_PORT_C   0xFF
#define   D_PIO_IN_PORT_D   0xFF
#define   D_PIO_IN_PORT_E   0xFF
#define   D_PIO_IN_PORT_F   0xFF
#define   D_PIO_HIGH_ADDR   0x0B
#define   D_PIO_LOW_ADDR    0x0C
#define   D_PIO_CUR_ADDR    0x0D
#define   D_PIO_FREE_SPACE  0x0E
#define   D_PIO_FREE_UPDATE 0x0F
#define   D_PIO_SUBPAGE_CTR 0x10
#define   D_PIO_HIGH_WATER  0x11
#define   D_PIO_STATUS      0x12

#define   D_PCB_IN_PORT_0   0xFF
#define   D_PCB_IN_PORT_1   0xFF
#define   D_PCB_IN_PORT_2   0xFF
#define   D_PCB_IN_PORT_3   0xFF
#define   D_PCB_IN_PORT_4   0xFF
#define   D_PCB_IN_PORT_5   0xFF
#define   D_PCB_IN_PORT_6   0xFF
#define   D_PCB_IN_PORT_7   0xFF
#define   D_PCB_OUT_PORT    0x13
#define   D_PCB_STATUS      0x14

#define   D_IND_CONTROL     0x15
#define   D_IND_ADDRESS     0xFF
#define   D_IND_DATA        0xFF

#define   D_IO_INT_MASK     0x16
#define   D_IO_STATUS       0x17

#define   D_HOST_INT_MASK   0xFF
#define   D_HOST_STATUS     0xFF
#define   D_HOST_COMMO      0x19

#define   D_DSP_COMMO       0x18
#define   D_DSP_SOFT_INT    0x20

#endif /* bishm.h */
