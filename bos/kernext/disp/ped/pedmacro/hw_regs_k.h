/* @(#)20       1.3.1.4  src/bos/kernext/disp/ped/pedmacro/hw_regs_k.h, pedmacro, bos411, 9428A410j 3/24/94 13:55:00 */

/*
 * COMPONENT_NAME: PEDMACRO
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/************************************************************************/
/*                                                                      */
/*      PEDERNALES HARDWARE PROGRAMMING INTERFACE                       */
/*                                                                      */
/************************************************************************/

#ifndef _H_HW_REGS_K
#define _H_HW_REGS_K

/************************************************/
/*                                              */
/* First include the files which are model      */
/* specific and are intended to be customized   */
/* by each model                                */
/*                                              */
/************************************************/

#include <mid/hw_model.h>

/************************************************/
/*                                              */
/* Next define the offsets of the various       */
/* hardware programming bus interface ports     */
/*                                              */
/************************************************/

	/*-----------------------------------------------*
	* the hardware can be addressed from three       *
	* different bus memory pages                     *
	*     1. for kernel accesses                     *
	*     2. for window server accesses              *
	*     3. for DWA user accesses                   *
	*                                                *
	* in addition the hardware has POS registers     *
	* which are addressed as IO space rather         *
	* than as bus memory                             *
	*                                                *
	*                                                *
	* Here we will define things for kernal access   *
	* and the POS register access.                   *
	*                                                *
	*                                                *
	*------------------------------------------------*/

#define MID_KERN_PAGE   (0x0000 + MID_BASE_PTR)


/************************************************/
/* Now define the hardware BIM register offsets */
/* of the kernal page of bus memory for the MID */
/************************************************/

	/*-----------------------------------------------*
	*                                                *
	*  In the comments is a short description of the *
	*  register.  Also, there is a flag stating      *
	*  R/W for RISC6000 read & write, with a "-"     *
	*  if the processor cannot read or write the     *
	*  register                                      *
	*-----------------------------------------------*/

	/*-----------------------------------------------*
	*  Define the kernel page registers and their    *
	*  functions                                     *
	*-----------------------------------------------*/

#define MID_K_DSP_CTL_REG       (MID_PTR_DSP_CONTROL)

	                        /* The control register which resets the
	                           320c30 DSP processor
	                           R/W by RISC6000                      */

#define MID_K_DSP_IO_INTR_MASK_REG      (MID_PTR_CARD_INTR_MASK)

	                        /* Holds the interrupt mask register of
	                           the 320c30 DSP chip, for read only
	                           R/- by RISC6000                      */

#define MID_K_HOST_IO_INTR_MASK_REG     (MID_PTR_HOST_INT_MASK)
	                        /* Holds the interrupt mask register of
	                           the microchannel side of the BIM
	                           R/- by RISC6000                      */

#define MID_K_HOST_STATUS_REG   (MID_PTR_HOST_STATUS)

	                        /* Holds the BIM status register of MID
	                           R/W by RISC6000                      */

#define MID_K_DSP_COMMO_REG     (MID_PTR_CARD_COMMO)

	                        /* Holds the 320c30 interrupt commo
	                           register to BIM, for read only
	                           R/- by RISC6000                      */

#define MID_K_COMMO_REG         (MID_PTR_HOST_COMMO)

	                        /* Holds the word of extra status passed
	                           when the commo interrupt is raised
	                           R/W by RISC6000                      */


	/*-----------------------------------------------*
	*                                                *
	* Bit field definitions for the DSP control      *
	* registers.  This register is found at the      *
	* MID_K_DSP_CTL_REG address defined above.       *
	*                                                *
	* The MID_K_DSP_CTL_REG is R/W by RISC6000       *
	*                                                *
	*-----------------------------------------------*/

#define MID_DSP_CTL_RST         (0x00)
#define MID_DSP_CTL_RUN         (0x01)


	/*-----------------------------------------------*
	*                                                *
	* Bit field definitions for the I/O interrupt    *
	* mask registers on the DSP side. This register  *
	* is found at the MID_K_DSP_IO_INTR_MASK_REG addres  *
	* defined above.                                 *
	*                                                *
	*    if (MID_K_DSP_IO_INTR_MASK_REG & MID_K_DSP_IO_?)    *
	*       Interrupt Enabled                        *
	*    else                                        *
	*       Interrupt Disabled                       *
	*                                                *
	* The MID_K_DSP_IO_INTR_MASK_REG is R/- by RISC6000      *
	*                                                *
	*-----------------------------------------------*/

#define MID_K_DSP_IO_BM_COMPLETE                (0x00001)
#define MID_K_DSP_IO_BM_SUBPAGE_ZERO            (0x00002)
#define MID_K_DSP_IO_BM_FREESPACE_ZERO          (0x00004)
#define MID_K_DSP_IO_PIO_TRANS_TO_IO            (0x00008)
#define MID_K_DSP_IO_PIO_SUBPAGE_ZERO           (0x00010)
#define MID_K_DSP_IO_PIO_FIFO_EMPTY             (0x00020)
#define MID_K_DSP_IO_PIO_HI_WATER_HIT           (0x00040)
#define MID_K_DSP_IO_NEXT_PCB_DATA_REC          (0x00080)
#define MID_K_DSP_IO_PCB_FIFO_EMPTY             (0x00100)
#define MID_K_DSP_IO_PCB_FIFO_FULL              (0x00200)
#define MID_K_DSP_IO_DSP_READ_EMPTY_FIFO        (0x00400)
#define MID_K_DSP_IO_WRITE_HOST_COMMO_REG       (0x00800)
#define MID_K_DSP_IO_HAS_HOST_READ_COMMO        (0x01000)
#define MID_K_DSP_IO_BAD_PARITY_ON_LEPB         (0x02000)
#define MID_K_DSP_IO_CHANNEL_CHECK              (0x04000)
#define MID_K_DSP_IO_OVERRUN_TIME_OUT           (0x08000)
#define MID_K_DSP_IO_BM_TIME_OUT                (0x10000)



	/*-----------------------------------------------*
	*                                                *
	* Bit field definitions for the I/O interrupt    *
	* mask registers on the HOST side. This register *
	* is found at the MID_K_HOST_IO_INTR_MASK_REG addres *
	* defined above.                                 *
	*                                                *
	*    if (MID_K_HOST_IO_INTR_MASK_REG & MID_K_HOST_IO_?)  *
	*       Interrupt Enabled                        *
	*    else                                        *
	*       Interrupt Disabled                       *
	*                                                *
	* The MID_K_HOST_IO_INTR_MASK_REG is R/W by RISC6000 *
	*                                                *
	* These bit fields are also used for read/write  *
	* to the MID_K_HOST_STATUS_REGISTER                      *
	*-----------------------------------------------*/

#define MID_K_HOST_IO_PIO_HI_WATER_HIT          (0x00001)
#define MID_K_HOST_IO_WRITE_DSP_COMMO_REG       (0x00002)
#define MID_K_HOST_IO_CHANNEL_CHECK             (0x00004)
#define MID_K_HOST_IO_HAS_DSP_READ_COMMO        (0x00008)
#define MID_K_HOST_IO_DSP_SOFT_INTR0            (0x00010)
#define MID_K_HOST_IO_PIO_FIFO_AVAILABLE        (0x00020)
#define MID_K_HOST_IO_CONTEXT_SWITCH_IN_PROG    (0x00040)
#define MID_K_HOST_IO_LOW_WATER_MARK            (0x00080)
#define MID_K_HOST_IO_PCB_FIFO_EMPTY            (0x00100)
#define MID_K_HOST_IO_PCB_FIFO_FULL             (0x00200)

/*
 * Following are the defines for POS register offsets
 */
#define MID_POS_REG_0                   (0x00)
#define MID_POS_REG_1                   (0x01)
#define MID_POS_REG_2                   (0x02)
#define MID_POS_REG_3                   (0x03)
#define MID_POS_REG_4                   (0x04)
#define MID_POS_REG_5                   (0x05)
#define MID_POS_REG_6                   (0x06)
#define MID_POS_REG_7                   (0x07)

/*
 * Following are the POS register bit settings
 */
#define MID_POS2_DISABLE                (0x00)
#define MID_POS2_ENABLE                 (0x01)
#define MID_POS2_FAIR                   (0x02)
#define MID_POS2_STREAM                 (0x04)
#define MID_POS2_WAIT_STATE             (0x08)
#define MID_POS2_WAIT_300NS             (0x00)
#define MID_POS2_WAIT_200NS             (0x08)
#define MID_POS2_PARITY                 (0x10)
#define MID_POS2_OPTION                 (0x20)
#define MID_POS2_BYTE_SWAP              (0x40)
#define MID_POS2_SPEED                  (0x80)
#define MID_POS2_SPEED_30NS             (0x00)
#define MID_POS2_SPEED_70NS             (0x80)

#define MID_POS4_INTLVL                 (0x01)
#define MID_POS4_ARBLVL                 (0x10)

#define MID_POS5_AUTOINC                (0x01)
#define MID_POS5_VPD_0                  (0x00)
#define MID_POS5_VPD_1                  (0x02)
#define MID_POS5_VPD_2                  (0x04)
#define MID_POS5_CHKST_DI               (0x40)
#define MID_POS5_CHKST_EN               (0x00)
#define MID_POS5_CHKCH_DI               (0x80)
#define MID_POS5_CHKCH_EN               (0x00)

#define MID_POS6_PARITY_CHK             (0x01)
#define MID_POS6_BAD_ADDR               (0x02)
#define MID_POS6_INVALID_ACC            (0x04)

#endif /* _H_HW_REGS_K */
