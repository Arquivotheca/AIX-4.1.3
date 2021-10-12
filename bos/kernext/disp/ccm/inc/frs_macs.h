/* @(#)12	1.4  src/bos/kernext/disp/ccm/inc/frs_macs.h, dispccm, bos411, 9428A410j 7/5/94 11:34:47 */
/*
 *
 * COMPONENT_NAME: (SYSXDISPCCM) Common Character Mode Device Driver
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*========================================================================
|
| This file contains macros that are used to reference the typedefs 
| given in companion file "frs.h".  In general, code which uses the
| structures defined in frs.h should always reference those structures
| with the macros in this file.
|
|========================================================================*/

/*========================================================================
|
| MACROS which operate on the intel_ushort_t structures
|
|=========================================================================*/

	/*========================================
	| THE FOLLOWING MACRO CANNOT BE AN LVALUE
	|=========================================*/ 

#define	RSCAN_USHORT( val )					\
		/* ushort					\
		|  intel_ushort_t	val			\
		|--------------------------------------*/	\
		( (ushort)					\
		     ( ((val).RISC_value.upper_value) << 8 )	\
		  |  ( ((val).RISC_value.lower_value)      ))


/*=========================================================================
|
| macros which operate on the  
| rom_scan_ps2_header structure
|
|==========================================================================*/

	/*================================================
	| THE FOLLOWING MACROS ARE PERMITTED TO BE LVALUES
	|================================================*/ 

#define	RSCAN_PS2						\
		rom_scan_ps2_header_t

#define	RSCAN_PS2_SIZE						\
		/* size_t					\
		|--------------------------------------*/	\
		( sizeof( rom_scan_ps2_header_t) )

#define RSCAN_PS2_id_hex55(ptr)					\
		/* uchar					\
                 | rom_scan_ps2_header_t *ptr 			\
		 |-------------------------------------*/	\
		((ptr) -> id_hex55) 

#define RSCAN_PS2_id_hexAA(ptr)					\
		/* uchar					\
                 | rom_scan_ps2_header_t *ptr 			\
		 |-------------------------------------*/	\
		((ptr) -> id_hexAA) 

#define RSCAN_PS2_Blength(ptr)					\
		/* uchar					\
                 | rom_scan_ps2_header_t *ptr 			\
		 |-------------------------------------*/	\
		((ptr) -> rom_length_in_512B_blocks)

#define RSCAN_PS2_flag(ptr)					\
		/* uchar					\
                 | rom_scan_ps2_header_t *ptr 			\
		 |-------------------------------------*/	\
		((ptr) -> dynamic_rom_flag)


	/*========================================
	| THE FOLLOWING MACROS CANNOT BE LVALUES
	|=========================================*/ 

#define RSCAN_PS2_dynamic_rom_length(ptr)			\
		/* ushort					\
                 | rom_scan_ps2_header_t *ptr 			\
		 |-------------------------------------*/	\
		RSCAN_USHORT( (ptr)->length_of_dynamic_rom )

#define RSCAN_PS2_dynamic_rom_head(ptr)				\
		/* ushort					\
                 | rom_scan_ps2_header_t *ptr 			\
		 |-------------------------------------*/	\
		RSCAN_USHORT( (ptr)->head_of_dynamic_rom )


/*===================================================================
|
| macros which operate on the generic
| rom_scan_R2_block structure
|
|====================================================================*/

	/*========================================
	| THE FOLLOWING MACROS CANNOT BE LVALUES
	|=========================================*/ 

#define RSCAN_ROMBLK_next(ptr)					\
		/* ushort					\
                 | rom_scan_R2_block_t *ptr			\
                 |----------------------------*/		\
		RSCAN_USHORT( (ptr)->offset_to_next_block )

#define RSCAN_ROMBLK_class(ptr)					\
		/* ushort					\
                 | rom_scan_R2_block_t *ptr			\
                 |----------------------------*/		\
		RSCAN_USHORT( (ptr)->block_class )

	/*================================================
	| THE FOLLOWING MACROS ARE PERMITTED TO BE LVALUES
	|================================================*/ 

#define	RSCAN_ROMBLK						\
		rom_scan_R2_block_t

#define	RSCAN_ROMBLK_SIZE					\
		/* size_t					\
		|--------------------------------------*/	\
		( sizeof( rom_scan_R2_block_t) )

#define RSCAN_ROMBLK_id(ptr)					\
		/* char						\
                 | rom_scan_R2_block_t *ptr			\
                 |----------------------------*/		\
		((ptr) -> R2_id_field)


/*========================================================================
|
| macros which operate on the 
| rom_scan_R2_6000_block structure
|
|========================================================================*/

	/*========================================
	| THE FOLLOWING MACROS CANNOT BE LVALUES
	|=========================================*/ 

#define RSCAN_6000_BLK_next(ptr)				\
		/* ushort					\
                 | rom_scan_R2_6000_block_t *ptr		\
                 |--------------------------------*/		\
		RSCAN_ROMBLK_next( ptr )

#define RSCAN_6000_BLK_class(ptr)				\
		/* ushort					\
                 | rom_scan_R2_6000_block_t *ptr		\
                 |--------------------------------*/		\
		RSCAN_ROMBLK_class( ptr )

	/*================================================
	| THE FOLLOWING MACROS ARE PERMITTED TO BE LVALUES
	|================================================*/ 

#define	RSCAN_6000_BLK						\
		rom_scan_R2_6000_block_t

#define	RSCAN_6000_BLK_SIZE					\
		/* size_t					\
		|--------------------------------------*/	\
		( sizeof( rom_scan_R2_6000_block_t) )

#define RSCAN_6000_BLK_id(ptr)					\
		/* char						\
                 | rom_scan_R2_6000_block_t *ptr		\
                 |--------------------------------*/		\
		RSCAN_ROMBLK_id( ptr )




#define RSCAN_6000_BLK_rom_addr(ptr)				\
		/* ulong					\
                 | rom_scan_R2_6000_block_t *ptr		\
                 |--------------------------------*/		\
		((ptr) -> R2_rom_busmem_addr)

#define RSCAN_6000_BLK_pos2_val(ptr)				\
		/* uchar					\
                 | rom_scan_R2_6000_block_t *ptr		\
                 |--------------------------------*/		\
		((ptr) -> pos2_value)

#define RSCAN_6000_BLK_pos3_val(ptr)				\
		/* uchar					\
                 | rom_scan_R2_6000_block_t *ptr		\
                 |--------------------------------*/		\
		((ptr) -> pos3_value)

#define RSCAN_6000_BLK_pos4_val(ptr)				\
		/* uchar					\
                 | rom_scan_R2_6000_block_t *ptr		\
                 |--------------------------------*/		\
		((ptr) -> pos4_value)

#define RSCAN_6000_BLK_pos5_val(ptr)				\
		/* uchar					\
                 | rom_scan_R2_6000_block_t *ptr		\
                 |--------------------------------*/		\
		((ptr) -> pos5_value)



/*======================================================================
|
| macros which operate on the 
| rom_scan_R2_6001_block structure
|
|=======================================================================*/

	/*========================================
	| THE FOLLOWING MACROS CANNOT BE LVALUES
	|=========================================*/ 

#define RSCAN_6001_BLK_next(ptr)				\
		/* ushort					\
		 | rom_scan_R2_6001_block_t *ptr 		\
		 |-----------------------------------*/		\
		RSCAN_ROMBLK_next( ptr )

#define RSCAN_6001_BLK_class(ptr)				\
		/* ushort					\
		 | rom_scan_R2_6001_block_t *ptr		\
		 |-----------------------------------*/		\
		RSCAN_ROMBLK_class( ptr )

	/*================================================
	| THE FOLLOWING MACROS ARE PERMITTED TO BE LVALUES
	|================================================*/ 

#define	RSCAN_6001_BLK						\
		rom_scan_R2_6001_block_t

#define	RSCAN_6001_BLK_SIZE					\
		/* size_t					\
		|--------------------------------------*/	\
		( sizeof( rom_scan_R2_6001_block_t) )


#define RSCAN_6001_BLK_id(ptr)					\
		/* char	 					\
		 | rom_scan_R2_6001_block_t *ptr		\
		 |-----------------------------------*/		\
		RSCAN_ROMBLK_id( ptr )




#define RSCAN_6001_BLK_rom_addr(ptr)				\
		/* ulong					\
		 | rom_scan_R2_6001_block_t *ptr		\
		 |-----------------------------------*/		\
		((ptr) -> R2_rom_busmem_addr)

#define RSCAN_6001_BLK_pos_val(ptr,i)				\
		/* uchar					\
		 | rom_scan_R2_6001_block_t *ptr		\
		 | uchar		     i			\
		 |-----------------------------------*/		\
		((ptr) -> pos[ (i) ].val)

#define RSCAN_6001_BLK_pos_reg(ptr,i)				\
		/* uchar					\
		 | rom_scan_R2_6001_block_t *ptr		\
		 | uchar		    i			\
		 |-----------------------------------*/		\
		((ptr) -> pos[ (i) ].reg)

#define RSCAN_6001_BLK_num_pos_vals(ptr)			\
		/* uchar					\
		 | rom_scan_R2_6001_block_t *ptr		\
		 |-----------------------------------*/		\
		((ptr) -> num_pos_vals)

#define RSCAN_6001_BLK_sw_version(ptr)				\
		/* uchar					\
		 | rom_scan_R2_6001_block_t *ptr		\
		 |-----------------------------------*/		\
		((ptr) -> sw_version)



/*======================================================================
|
| macros which operate on the 
| rom_scan_R2_6002_block structure
|
|=======================================================================*/

	/*========================================
	| THE FOLLOWING MACROS CANNOT BE LVALUES
	|=========================================*/ 

#define RSCAN_6002_BLK_next(ptr)				\
		/* ushort					\
		 | rom_scan_R2_6002_block_t *ptr 		\
		 |-----------------------------------*/		\
		RSCAN_ROMBLK_next( ptr )

#define RSCAN_6002_BLK_class(ptr)				\
		/* ushort					\
		 | rom_scan_R2_6002_block_t *ptr		\
		 |-----------------------------------*/		\
		RSCAN_ROMBLK_class( ptr )

	/*================================================
	| THE FOLLOWING MACROS ARE PERMITTED TO BE LVALUES
	|================================================*/ 

#define	RSCAN_6002_BLK						\
		rom_scan_R2_6002_block_t

#define	RSCAN_6002_BLK_SIZE					\
		/* size_t					\
		|--------------------------------------*/	\
		( sizeof( rom_scan_R2_6002_block_t) )


#define RSCAN_6002_BLK_id(ptr)					\
		/* char	 					\
		 | rom_scan_R2_6002_block_t *ptr		\
		 |-----------------------------------*/		\
		RSCAN_ROMBLK_id( ptr )




#define RSCAN_6002_BLK_rom_addr(ptr)				\
		/* ulong					\
		 | rom_scan_R2_6002_block_t *ptr		\
		 |-----------------------------------*/		\
		((ptr) -> R2_rom_busmem_addr)

#define RSCAN_6002_BLK_cfg_reg_index(ptr,i)			\
		/* uchar					\
		 | rom_scan_R2_6002_block_t *ptr		\
		 | uchar		     i			\
		 |-----------------------------------*/		\
		((ptr) -> cfg_reg_index[ (i) ])

#define RSCAN_6002_BLK_cfg_reg_value(ptr,i)			\
		/* ulong					\
		 | rom_scan_R2_6002_block_t *ptr		\
		 | uchar		    i			\
		 |-----------------------------------*/		\
		((ptr) -> cfg_reg_value[ (i) ])

#define RSCAN_6002_BLK_num_cfg_regs(ptr)			\
		/* uchar					\
		 | rom_scan_R2_6002_block_t *ptr		\
		 |-----------------------------------*/		\
		((ptr) -> num_cfg_regs)

#define RSCAN_6002_BLK_sw_version(ptr)				\
		/* uchar					\
		 | rom_scan_R2_6002_block_t *ptr		\
		 |-----------------------------------*/		\
		((ptr) -> sw_version)


/*======================================================================
|
| macros which operate on the
| rom_scan_R2_6003_block structure
|
|=======================================================================*/

	/*========================================
        | THE FOLLOWING MACROS CANNOT BE LVALUES
        |=========================================*/

#define RSCAN_6003_BLK_next(ptr) 				\
		/* ushort                                       \
                 | rom_scan_R2_6003_block_t *ptr                \
                 |-----------------------------------*/         \
		RSCAN_ROMBLK_next(ptr)

#define RSCAN_6003_BLK_class(ptr) 				\
		/* ushort                                       \
                 | rom_scan_R2_6003_block_t *ptr                \
                 |-----------------------------------*/         \
		RSCAN_ROMBLK_class(ptr)

	/*================================================
        | THE FOLLOWING MACROS ARE PERMITTED TO BE LVALUES
        |================================================*/

#define RSCAN_6003_BLK						\
		rom_scan_R2_6003_block_t

#define RSCAN_6003_BLK_SIZE					\
		/* size_t                                       \
                |--------------------------------------*/       \
		( sizeof(rom_scan_R2_6003_block_t) )

#define RSCAN_6003_BLK_id(ptr) 					\
		/* char                                         \
                 | rom_scan_R2_6003_block_t *ptr                \
                 |-----------------------------------*/         \
		RSCAN_ROMBLK_id(ptr)

#define RSCAN_6003_BLK_rom_addr(ptr) 				\
		/* ulong                                        \
                 | rom_scan_R2_6003_block_t *ptr                \
                 |-----------------------------------*/         \
		( (ptr)->R2_rom_busmem_addr )


/*======================================================================
|
| macros which operate on the generic
| rom_scan_R2_head_t structure
|
|======================================================================*/

#define	RSCAN_R2_HEAD						\
		rom_scan_R2_head_t

#define RSCAN_R2_HEAD_rom_Blength(ptr)                         	\
                /* ushort                                       \
                 | rom_scan_R2_head_t *ptr      		\
                 |-----------------------------------*/         \
                ((ptr) -> rom_length_in_512B_blocks)

#define RSCAN_R2_HEAD_residue(ptr)                            	\
                /* ushort                                       \
                 | rom_scan_R2_head_t *ptr        		\
                 |-----------------------------------*/         \
                ((ptr) -> rom_CRC_residue)

#define RSCAN_R2_HEAD_rom_type(ptr)               		\
                /* ulong                                        \
                 | rom_scan_R2_head_t *ptr       		\
                 |-----------------------------------*/         \
                ((ptr) -> rom_type)

