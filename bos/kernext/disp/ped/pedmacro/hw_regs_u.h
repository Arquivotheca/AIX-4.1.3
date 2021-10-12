/* @(#)21       1.3.1.4  src/bos/kernext/disp/ped/pedmacro/hw_regs_u.h, pedmacro, bos411, 9428A410j 3/24/94 13:55:11 */

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

#ifndef _H_MID_HW_REGS_U
#define _H_MID_HW_REGS_U

/************************************************/
/* First include the files which are model      */
/* specific and are intended to be customized   */
/* by each model                                */
/*                                              */
/************************************************/

#include <mid/hw_model.h>

/************************************************/
/* First define the offsets of the various      */
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
	*------------------------------------------------*/

#define MID_SERVER_PAGE (0x1000 + MID_BASE_PTR)
#define MID_DWA_PAGE    (0x2000 + MID_BASE_PTR)
#define MID_IND_PAGE    (0x3000 + MID_BASE_PTR)


/************************************************/
/* Now define the hardware BIM register offsets */
/* of the different pages of bus memory that    */
/* the MID supports                             */
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
	*  Define the server page registers and their    *
	*  functions for production hardware             *
	*-----------------------------------------------*/

#define MID_PCB_FIFO            (MID_PTR_PCB_DATA)
#define MID_PCB_FIFO_PIO_SIZE   (8  * 4)   /* bytes */

	                        /* Holds the starting address of the
	                           Priority Command Buffer FIFO
	                           -/W by RISC6000

	                           Note:  max of 8  words can be stored
	                                  at one time
	                                                                */

	/*-----------------------------------------------*
	*                                                *
	* define user space DWA page register and their  *
	* functions                                      *
	*                                                *
	*-----------------------------------------------*/

#define MID_FIFO                (MID_PTR_FIFO_DATA)
#define MID_FIFO_PIO_SIZE       (16 * 4)   /* bytes */

	                        /* Holds the starting address of the data
	                           command element buffer of the MID.
	                           -/W by RISC6000

	                           Note:  max of 16 words can be stored
	                                  at one time
	                                                                */

#define MID_FIFO_FREE_REG       (MID_PTR_FREE_SPACE)

	                        /* Holds the count of free words left in
	                           the FIFO
	                           R/- by RISC6000                      */

#define MID_MEM_IND_CTL_REG     (MID_PTR_IND_CONTROL)

	                        /* The control register which manages the
	                           indirect view control into MID internal
	                           RAM
	                           R/W by RISC6000                      */

/*
 * Bit field locations for the MID_MEM_IND_CTL_REG
 */
	                                        /* R/- by RISC6000 */
#define MID_ICR_DSP_MEM_PROTECT         (0x01)
	                                        /* R/- by RISC6000 */
#define MID_ICR_IO_BUF_PROTECT          (0x02)
	                                        /* R/W by RISC6000 */
#define MID_ICR_MEM_SELECT              (0x0C)
	                                        /* R/W by RISC6000 */
#define MID_ICR_OPER_SELECT             (0x10)
	                                        /* R/W by RISC6000 */
#define MID_ICR_AUTO_INCR               (0x20)
	                                        /* R/W by RISC6000 */
#define MID_ICR_RESET_DSP               (0x40)

/*
 * Common control configurations for MID_MEM_IND_CTL_REG in non-wrap mode.
 */

#define MID_IND_RD_NI       (0x18)   /* Read,  no auto increment            */

#define MID_IND_WR_NI       (0x08)   /* Write, no auto increment            */

#define MID_IND_RD_AI       (0x38)   /* Read,  auto increment               */

#define MID_IND_WR_AI       (0x28)   /* Write, auto increment               */

#define MID_IND_MODE_MASK   (0x38)   /* Mask for RD/WR and AI/NI modes      */



#define MID_MEM_IND_ADR_REG     (MID_PTR_IND_ADDRESS)

	                        /* Holds the offset address into MID
	                           internal RAM for user access
	                           R/W by RISC6000                      */

#define MID_MEM_IND_DATA_REG    (MID_PTR_IND_DATA)

	                        /* Register from which data can be
	                           read, based on the indirect address
	                           and control registers
	                           R/W by RISC6000                      */

/*
 * Definitions used for reading the DSP Priority Status Register in
 * in wrap mode.  This technique is used by the MID_WAIT_PCB_EMPTY routine
 * to determine when the Priority Command Buffer is empty.
 */

#define MID_PSR_MODE            0x00000010  /* Control to Read in Wrap Mode */
#define MID_PSR_OFFSET          0x00000014  /* Offset of Priority Status Reg */
#define MID_PSR_PCB_EMPTY       0x00000010  /* Bit set if PCB is Empty */

/*
 * Definition used for mapping the DSP indirect addresses
 */
#define MID_MAX_DSP_IND_ADR     (0x0003FFFF)

#endif /* _H_MID_HW_REGS_U */
