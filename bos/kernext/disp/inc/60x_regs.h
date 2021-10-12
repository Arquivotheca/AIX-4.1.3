/* @(#)50	1.7  src/bos/kernext/disp/inc/60x_regs.h, dispccm, bos411, 9428A410j 7/5/94 11:32:04 */

#ifndef _H_60X_REGS
#define _H_60X_REGS

/*
 *
 * COMPONENT_NAME: (dispcfg)
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/******************************************************************************
*******************************************************************************
**                                                                           **
** Contains the 60X bus register access macros				     **
**                                                                           **
*******************************************************************************
******************************************************************************/

#define GS_ATTACH601BUS(vm_handle, offset) vm_att((vm_handle), (offset))
#define GS_DETACH601BUS(address) vm_det(address)


/******************************************************************************
* 									      *
*       Define the ranges for the VPD/Feature ROM and Standard Configuration  *
*       registers while the 60X bus is in setup mode with pipelining disabled *
* 									      *
*       Architected Feature/VPD ROM Space  : 0xFFA00000 - 0xFFBFFFFF          *
*       Architected Configuration Registers: 0xFF200000 - 0xFF201FFF          *
* 									      *
******************************************************************************/

#define BUS_60X_START_VPD_FEATURE_ROM			0xFFA00000
#define BUS_60X_END_VPD_FEATURE_ROM			0xFFBFFFFF
#define BUS_60X_START_CFG_REGS  	                0xFF200000
#define BUS_60X_END_CFG_REGS             	        0xFF201FFF


/******************************************************************************
* 									      *
*       Define the 60X bus Device Characteristics Register:                   *
*	NOTE: This is a READ-ONLY register. 				      *
* 									      *
******************************************************************************/

#define BUS_60X_DEV_CHAR_REG				0xFF200000
#define BUS_60X_DEV_CHAR_DEV_TYPE_SHIFT				28
#define BUS_60X_DEV_CHAR_DEV_TYPE_MASK				\
	(0xF << BUS_60X_DEV_CHAR_DEV_TYPE_SHIFT)
#define BUS_60X_DEV_CHAR_BUID_SHIFT				26
#define BUS_60X_DEV_CHAR_BUID_MASK				\
	(0x3 << BUS_60X_DEV_CHAR_BUID_SHIFT)
#define BUS_60X_DEV_CHAR_MEM_ALLOC_CNTL_SPACE_SHIFT		20
#define BUS_60X_DEV_CHAR_MEM_ALLOC_CNTL_SPACE_MASK		\
	(0xF << BUS_60X_DEV_CHAR_MEM_ALLOC_CNTL_SPACE_SHIFT)
#define BUS_60X_DEV_CHAR_MEM_ALLOC_DATA_SPACE_SHIFT		16
#define BUS_60X_DEV_CHAR_MEM_ALLOC_DATA_SPACE_MASK		\
	(0xF << BUS_60X_DEV_CHAR_MEM_ALLOC_DATA_SPACE_SHIFT)
#define BUS_60X_DEV_CHAR_VPD_ROM_SIZE_SHIFT			5
#define BUS_60X_DEV_CHAR_VPD_ROM_SIZE_MASK			\
	(0x3 << BUS_60X_DEV_CHAR_VPD_ROM_SIZE_SHIFT)
#define BUS_60X_DEV_CHAR_WORD_ADDR_INCREMENT_SHIFT		2
#define BUS_60X_DEV_CHAR_WORD_ADDR_INCREMENT_MASK		\
	(0x7 << BUS_60X_DEV_CHAR_WORD_ADDR_INCREMENT_SHIFT)


/******************************************************************************
* 									      *
*       Define the 60X bus Device ID Register (for word increment equal 4):   *
*	NOTE: This is a READ-ONLY register.				      *
*	NOTE: If this register contains 0000 0000, then the real device	      *
*	id is read from the first word of ROM on the adapter.		      *
*	The address for this is defined as BUS_60X_ROM_DEV_ID_REG	      *
* 									      *
******************************************************************************/

#define BUS_60X_DEV_ID_REG				0xFF200004
#define BUS_60X_DEV_ID_SHIFT					0
#define BUS_60X_DEV_ID_MASK	(0x00FFFFFF << BUS_60X_DEV_ID_SHIFT)


/******************************************************************************
* 									      *
*       Define the 60X bus Device BUS UNIT ID Registers (word incr equal 4):  *
*	NOTE: These registers are READ/WRITE				      *
* 									      *
******************************************************************************/

#define BUS_60X_BUID_REG_1				0xFF200008
#define BUS_60X_BUID_REG_2				0xFF20000C
#define BUS_60X_BUID_REG_3				0xFF200010
#define BUS_60X_BUID_REG_4				0xFF200014
#define BUS_60X_BUID_ADDR_SHIFT					16	
#define BUS_60X_BUID_ADDR_MASK		(0x1FF << BUS_60X_BUID_ADDR_SHIFT)


/******************************************************************************
* 									      *
*       Define the 60X bus Device Base Real Address Register (word incr = 4): *
*	NOTE: This is a READ/WRITE register.				      *
* 									      *
******************************************************************************/

#define BUS_60X_DEV_BASE_REAL_ADDR_REG			0xFF200018
#define BUS_60X_DEV_BASE_REAL_ADDR_SHIFT			28
#define BUS_60X_DEV_BASE_REAL_ADDR_MASK				\
	(0xF << BUS_60X_DEV_BASE_REAL_ADDR_SHIFT)

/******************************************************************************
* 									      *
*	Define the 60X bus config space for the ROM attached to the	      *
*	60x bus device.	Note these addresses are good for word incr equal 4   *
* 									      *
******************************************************************************/

#define BUS_60X_ROM_DEV_ID_REG		0xFFA00000
#define BUS_60X_ROM_CHAR_REG		0xFFA00004
#define BUS_60X_ROM_CRC_REG		0xFFA00008
#define BUS_60X_VPD_TYPE_REG		0xFFA0000C
#define BUS_60X_VPD_ADDR_REG		0xFFA00010
#define BUS_60X_VPD_LEN_REG		0xFFA00014
#define	BUS_60X_FEATURE_ROM_TYPE_REG	0xFFA00018
#define	BUS_60X_FEATURE_ROM_ADDR_REG	0xFFA0001C
#define	BUS_60X_FEATURE_ROM_LEN_REG	0xFFA00020
#define	BUS_60X_DFA_TABLE_ROM_TYPE_REG	0xFFA00024
#define	BUS_60X_DFA_TABLE_ROM_ADDR_REG	0xFFA00028
#define	BUS_60X_DFA_TABLE_ROM_LEN_REG	0xFFA0002C


        /* 
           Two methods of accessing the 60X adapter:

           1.  When T = 1, we use BUID 7F which is a special animal.  This tells
               the processor not to do any boundary checking.  In this case, the
               address (upper 4 bits in the segment register) will specify
               which adapter

           2.  When T = 0, BUID = ??.  In this case, ??

           0   1    2              12        16           24        28      31
         ----------------------------------------------------------------------
         | T | Ks | Kp |   BUID   | 0 |0|0|0|0|   auth.  | I |0|0|0| addr     |
         |   |    |    | (9 bits) |   | | | | | (7 bits) |   | | | | (4 bits) |
         ----------------------------------------------------------------------

         */


#define BUS_60X_BID_PROB_STATE	0xC0000000	/* Problem State Access     */
#define BUS_60X_BID_SUPER_STATE	0x80000000      /* Supervisory State Access */

/*
   After the Device Real Base Address Register has been initialized 
   we can access the adapter with T = 1 mode and BUID = 7F.
*/

#define BUS_60X_BID_SDSS_ACCESS	0x07F00000	/* Special BUS ID for 60X bus access */

/* 
   Use to access the system & device configuration space only.  In normal
   access, we will use segment in iplcb .  Note this is the
   same value we already stored in the Device Real Base Address Register
   during configuration time of the device.

   this constant was defined for bringup.  Will remove it after the coding is complete
*/

#define BUS_60X_SEG_EXT_BITS_FOR_CFG_SPACE	0xf

#define BUS_60X_SLOT_SELECT_CONFIG_REG	0x0f00000c
#define BUS_60X_SLOT_RESET_REG		0x0f000010

/* 
   Bus Slot Reset Register Definition:   (we don't use it - only the machine driver)
   -----------------------------------

   0-2: reserved
   3  : reset for IOCC chip
        0 - reset signal active to IOCC chip
        1 = reset signal is inactive
   4  : reset for graphics slot 
        0 - reset signal active to graphics chip
        1 = reset signal is inactive
   5  : reset not used but available 
        0 - reset signal active
        1 = reset signal inactive
*/

                                       /* turn on bit 4 and 5 */

#define BUS_60X_GRAPHICS_SLOT_RESET_INACTIVE	0x08000000     


/* 
   Bus Slot Config Register Definition:  ( we only pass in bits 29 to 31 (i.e. slot 3) to machine driver)
   -----------------------------------

   0    : configuration enable
          0 - inactivate all CONFIG signals
          1 - activate appropriate CONFIG as descrived in bits 29 - 31
   1-28 : reserved 
   29-31: configuration field
          000 - not implemented
          001 - not implemented
          010 - CONFIG_(2) signal driven active to Memory Controller
          011 - CONFIG_(3) signal driven active to IOCC chip
          100 - CONFIG_(4) signal driven active to graphics slot 
          101 - CONFIG_(5) signal driven active (reserved) 
          11x - reserved
*/

#define BUS_60X_GRAPHICS_SLOT_CONFIG_ENABLE		0x80000004
#define BUS_60X_GRAPHICS_SLOT_CONFIG_DISABLE		0x0

#define BUS_60X_NO_DEVICE_PRESENT 			0xF0000000


/* 
 * from cdd.h with changes for GXT150
 */

	#define	BUS_60X_32BIT_SET_T_BIT	 	0x80000000
	#define	BUS_60X_MOD250_BUID 		0x07f00000



	#define	ADDR_IS_60x_CFG_SPACE( segment , address )				     \
		(     (((segment) & BUS_60X_32BIT_SET_T_BIT  ) == BUS_60X_32BIT_SET_T_BIT )  \
	  	   && (((segment) & BUS_60X_MOD250_BUID    ) == BUS_60X_MOD250_BUID ) 	     \
	  	   && ( ( (address) >= BUS_60X_START_CFG_REGS ) &&			     \
	       	      ( (address) <= BUS_60X_END_CFG_REGS ) ) 				     \
		)

	#define	ADDR_IS_60x_VPD_FRS_SPACE( segment , address )				      \
		(     (((segment) & BUS_60X_32BIT_SET_T_BIT  ) == BUS_60X_32BIT_SET_T_BIT )   \
	  	   && (((segment) & BUS_60X_MOD250_BUID    ) == BUS_60X_MOD250_BUID ) 	      \
	  	   && ( ( (address) >= BUS_60X_START_VPD_FEATURE_ROM ) &&		      \
		      ( (address) <= BUS_60X_END_VPD_FEATURE_ROM ) ) 			      \
		)

#endif /* _H_60X_REGS */

