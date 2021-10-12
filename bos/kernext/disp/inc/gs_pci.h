/* @(#)21	1.1  src/bos/kernext/disp/inc/gs_pci.h, sysxdisp, bos411, 9433B411a 8/15/94 14:57:38 */

#ifndef _H_GS_PCI
#define _H_GS_PCI

/*
 * COMPONENT_NAME: (sysxdisp) Display Sub-System
 *
 * FUNCTIONS:  PCI Bus Configuration Register Definitions
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <graphics/gs_io_trace.h>


/******************************************************************************
* 									      *
*	Keep in mind that PowerPC hardware is Big Endian format so	      *
*	any entry that is a short or word, has to be byte swapped to	      *
*	be correct with the hardware being accessed. The I/O access	      *
*	macros at the bottom of the file can be used to perform the	      *
*	conversion automatically.					      *
* 									      *
* 									      *
******************************************************************************/


/******************************************************************************
* 									      *
*	Macros to perform Big/Little Endian swaps			      *
* 									      *
******************************************************************************/

#define GS_BYTE_REV_L(n)						\
	((((n) & 0x000000FF) << 24) + (((n) & 0x0000FF00) << 8) +	\
	(((n) & 0x00FF0000) >> 8) + (((n) & 0xFF000000) >> 24))

#define GS_BYTE_REV_S(n)						\
        ((((n) & 0x00FF) << 8) + (((n) & 0xFF00) >> 8))


/******************************************************************************
* 									      *
*	PCI Configuration Register Addresses				      *
* 									      *
******************************************************************************/

#ifdef GS_LITTLE_ENDIAN

#define GS_PCI_VEND_ID_CFG_REG				0x00	/* 2 BYTE */
#define GS_PCI_DEV_ID_CFG_REG				0x02	/* 2 BYTE */
#define GS_PCI_CNTL_CFG_REG				0x04	/* 2 BYTE */
#define GS_PCI_STATUS_CFG_REG				0x06	/* 2 BYTE */

#define GS_PCI_REV_ID_CFG_REG				0x08	/* 1 BYTE */
#define GS_PCI_PRGM_INTRFC_CFG_REG			0x09	/* 1 BYTE */
#define GS_PCI_SUB_CLASS_CODE_CFG_REG			0x0A	/* 1 BYTE */
#define GS_PCI_BASE_CLASS_CODE_CFG_REG			0x0B	/* 1 BYTE */

#define GS_PCI_CACH_LINE_SIZE_CFG_REG			0x0C	/* 1 BYTE */
#define GS_PCI_LAT_TIMER_CFG_REG			0x0D	/* 1 BYTE */
#define GS_PCI_HEAD_TYPE_CFG_REG			0x0E	/* 1 BYTE */
#define GS_PCI_BIST_CFG_REG				0x0F	/* 1 BYTE */

#define GS_PCI_BASE_ADDR_REG_1_CFG_REG			0x10	/* 4 BYTE */
#define GS_PCI_BASE_ADDR_REG_2_CFG_REG			0x14	/* 4 BYTE */
#define GS_PCI_BASE_ADDR_REG_3_CFG_REG			0x18	/* 4 BYTE */
#define GS_PCI_BASE_ADDR_REG_4_CFG_REG			0x1C	/* 4 BYTE */
#define GS_PCI_BASE_ADDR_REG_5_CFG_REG			0x20	/* 4 BYTE */
#define GS_PCI_BASE_ADDR_REG_6_CFG_REG			0x24	/* 4 BYTE */

#define GS_PCI_CARD_BUS_CIS_PTR_CFG_REG			0x28	/* 4 BYTE */

#define GS_PCI_SUBSYS_DEV_ID_CFG_REG			0x2C	/* 2 BYTE */
#define GS_PCI_SUBSYS_ID_CFG_REG			0x2E	/* 2 BYTE */

#define GS_PCI_EXP_ROM_BASE_ADDR_CFG_REG		0x30	/* 4 BYTE */

#define GS_PCI_INTR_LINE_CFG_REG			0x3C	/* 1 BYTE */
#define GS_PCI_INTR_PIN_CFG_REG				0x3D	/* 1 BYTE */
#define GS_PCI_MIN_GRANT_CFG_REG			0x3E	/* 1 BYTE */
#define GS_PCI_MAX_LAT_CFG_REG				0x3F	/* 1 BYTE */

#else
	/* GS_BIG_ENDIAN */

#define GS_PCI_DEV_ID_CFG_REG				0x00	/* 2 BYTE */
#define GS_PCI_VEND_ID_CFG_REG				0x02	/* 2 BYTE */
#define GS_PCI_STATUS_CFG_REG				0x04	/* 2 BYTE */
#define GS_PCI_CNTL_CFG_REG				0x06	/* 2 BYTE */

#define GS_PCI_BASE_CLASS_CODE_CFG_REG			0x08	/* 1 BYTE */
#define GS_PCI_SUB_CLASS_CODE_CFG_REG			0x09	/* 1 BYTE */
#define GS_PCI_PRGM_INTRFC_CFG_REG			0x0A	/* 1 BYTE */
#define GS_PCI_REV_ID_CFG_REG				0x0B	/* 1 BYTE */

#define GS_PCI_BIST_CFG_REG				0x0C	/* 1 BYTE */
#define GS_PCI_HEAD_TYPE_CFG_REG			0x0D	/* 1 BYTE */
#define GS_PCI_LAT_TIMER_CFG_REG			0x0E	/* 1 BYTE */
#define GS_PCI_CACH_LINE_SIZE_CFG_REG			0x0F	/* 1 BYTE */

#define GS_PCI_BASE_ADDR_REG_1_CFG_REG			0x10	/* 4 BYTE */
#define GS_PCI_BASE_ADDR_REG_2_CFG_REG			0x14	/* 4 BYTE */
#define GS_PCI_BASE_ADDR_REG_3_CFG_REG			0x18	/* 4 BYTE */
#define GS_PCI_BASE_ADDR_REG_4_CFG_REG			0x1C	/* 4 BYTE */
#define GS_PCI_BASE_ADDR_REG_5_CFG_REG			0x20	/* 4 BYTE */
#define GS_PCI_BASE_ADDR_REG_6_CFG_REG			0x24	/* 4 BYTE */

#define GS_PCI_CARD_BUS_CIS_PTR_CFG_REG			0x28	/* 4 BYTE */

#define GS_PCI_SUBSYS_ID_CFG_REG			0x2C	/* 2 BYTE */
#define GS_PCI_SUBSYS_DEV_ID_CFG_REG			0x2E	/* 2 BYTE */

#define GS_PCI_EXP_ROM_BASE_ADDR_CFG_REG		0x30	/* 4 BYTE */

#define GS_PCI_MAX_LAT_CFG_REG				0x3C	/* 1 BYTE */
#define GS_PCI_MIN_GRANT_CFG_REG			0x3D	/* 1 BYTE */
#define GS_PCI_INTR_PIN_CFG_REG				0x3E	/* 1 BYTE */
#define GS_PCI_INTR_LINE_CFG_REG			0x3F	/* 1 BYTE */

#endif


/******************************************************************************
* 									      *
*	Device Control Configuration Register Masks			      *
* 									      *
******************************************************************************/

#define GS_PCI_IO_SPACE_ENABLE_MASK			0x0001
#define GS_PCI_IO_SPACE_DISABLE				0x0000
#define GS_PCI_IO_SPACE_ENABLE				0x0001

#define GS_PCI_MEM_SPACE_ENABLE_MASK			0x0002
#define GS_PCI_MEM_SPACE_DISABLE			0x0000
#define GS_PCI_MEM_SPACE_ENABLE				0x0002

#define GS_PCI_BUS_MASTER_ENABLE_MASK			0x0004
#define GS_PCI_BUS_MASTER_DISABLE			0x0000
#define GS_PCI_BUS_MASTER_ENABLE 			0x0004

#define GS_PCI_SPECIAL_CYCLES_MASK			0x0008
#define GS_PCI_IGNORE_SPECIAL_CYCLES			0x0000
#define GS_PCI_MONITOR_SPECIAL_CYCLES			0x0008

#define GS_PCI_MEM_WR_INV_ENABLE_MASK			0x0010
#define GS_PCI_MEM_WR_INV_DISABLE			0x0000
#define GS_PCI_MEM_WR_INV_ENABLE			0x0010

#define GS_PCI_VGA_PAL_SNOOP_MASK			0x0020
#define GS_PCI_RESPOND_TO_VGA_PAL_SNOOPS		0x0000
#define GS_PCI_SNOOP_VGA_PAL_ACC			0x0020

#define GS_PCI_PAR_ERR_RESP_MASK			0x0040
#define GS_PCI_IGNORE_PAR_ERR 				0x0000
#define GS_PCI_NORM_RESP_PAR_ERR			0x0040

#define GS_PCI_WAIT_CYCLE_CNTL_MASK			0x0080
#define GS_PCI_NO_ADDR_DATA_STEP			0x0000
#define GS_PCI_ADDR_DATA_STEP				0x0080

#define GS_PCI_SERR_ENABLE_MASK				0x0100
#define GS_PCI_SERR_DISABLE				0x0000
#define GS_PCI_SERR_ENABLE				0x0100

#define GS_PCI_FAST_BACK_TO_BACK_ENABLE_MASK		0x0200
#define GS_PCI_FAST_BACK_TO_BASK_DISABLE		0x0000
#define GS_PCI_FAST_BACK_TO_BASK_ENABLE			0x0200


/******************************************************************************
* 									      *
*	Device Status Configuration Register Masks			      *
* 									      *
******************************************************************************/

#define GS_PCI_66MHZ_CAPABLE				0x0020
#define GS_PCI_USER_DEF_FEAT_CAPABLE			0x0040
#define GS_PCI_FAST_BACK_TO_BACK_CAPABLE		0x0080
#define GS_PCI_DATA_PARITY_ERR_DETECTED			0x0100

#define GS_PCI_DEVSEL_TIMING_MASK			0x0600
#define GS_PCI_DEVSEL_FAST_TIMING			0x0000
#define GS_PCI_DEVSEL_MEDIUM_TIMING			0x0200
#define GS_PCI_DEVSEL_SLOW_TIMING			0x0400

#define GS_PCI_SIGNAL_TARGET_ABORT			0x0800
#define GS_PCI_REC_TARGET_ABORT				0x1000
#define GS_PCI_REC_MASTER_ABORT				0x2000
#define GS_PCI_SIGNAL_SYS_ERR				0x4000
#define GS_PCI_DETECTED_PAR_ERR				0x8000


/******************************************************************************
* 									      *
*	Sub Class Code Configuration Register Masks			      *
* 									      *
******************************************************************************/

#define GS_PCI_SUB_CLASS_VGA_COMPAT_CNTL_TYPE		0x00
#define GS_PCI_SUB_CLASS_OTH_DISP_CNTL_TYPE		0x80


/******************************************************************************
* 									      *
*	Header Configuration Register Masks				      *
* 									      *
******************************************************************************/

#define GS_PCI_HEADER_MULTI_FUNC_DEV_MASK		0x80
#define GS_PCI_HEADER_SINGLE_FUNC_DEV			0x00
#define GS_PCI_HEADER_MULTI_FUNC_DEV			0x80


/******************************************************************************
* 									      *
*	Base Class Code Configuration Register Masks			      *
* 									      *
******************************************************************************/

#define GS_PCI_BASE_CLASS_PREV_TYPE			0x00
#define GS_PCI_BASE_CLASS_MASS_STOR_TYPE		0x01
#define GS_PCI_BASE_CLASS_NET_CNTL_TYPE			0x02
#define GS_PCI_BASE_CLASS_DISP_CNTL_TYPE		0x03
#define GS_PCI_BASE_CLASS_MULT_MEDIA_TYPE		0x04
#define GS_PCI_BASE_CLASS_MEM_CNTL_TYPE			0x05
#define GS_PCI_BASE_CLASS_BRDG_DEV_TYPE			0x06
#define GS_PCI_BASE_CLASS_COMM_CNTL_TYPE		0x07
#define GS_PCI_BASE_CLASS_BASE_SYS_TYPE			0x08
#define GS_PCI_BASE_CLASS_IN_DEV_TYPE			0x09
#define GS_PCI_BASE_CLASS_DOCK_STAT_TYPE		0x0A
#define GS_PCI_BASE_CLASS_PROC_TYPE			0x0B
#define GS_PCI_BASE_CLASS_SER_BUS_CNTL_TYPE		0x0C
#define GS_PCI_BASE_CLASS_NO_FIT_TYPE			0xFF


/******************************************************************************
* 									      *
*	BIST Configuration Register Masks			   	      *
* 									      *
******************************************************************************/

#define GS_PCI_BIST_COMPL_CODE_MASK			0x0F
#define GS_PCI_BIST_START_BIST				0x40
#define GS_PCI_BIST_CAPABLE				0x80


/******************************************************************************
*                                                                             *
*       Base Address Configuration Register Masks                             *
*                                                                             *
******************************************************************************/

#define GS_PCI_MEM_SPACE_INDICATOR_MASK         	0x00000001
#define GS_PCI_MEM_SPACE_INDICATOR                      0x00000000
#define GS_PCI_IO_SPACE_INDICATOR                       0x00000001

/* The following bits are only valid for MEM_SPACE */

#define GS_PCI_MEM_SPACE_TYPE_MASK                      0x00000006
#define GS_PCI_MEM_SPACE_LOC_ANYWHERE_32BIT_SPACE       0x00000000
#define GS_PCI_MEM_SPACE_LOC_BELOW_1_MEG                0x00000002
#define GS_PCI_MEM_SPACE_LOC_ANYWHERE_64BIT_SPACE       0x00000004

/* The following bits are only valid for MEM_SPACE */

#define GS_PCI_MEM_SPACE_PREFETCHABLE_MASK              0x00000008
#define GS_PCI_MEM_SPACE_NOT_PREFETCHABLE               0x00000000
#define GS_PCI_MEM_SPACE_PREFETCHABLE                   0x00000008

#define GS_PCI_BASE_ADDR_MASK				0xFFFFFFF0


/******************************************************************************
* 									      *
*	Card Bus CIS Pointer Configuration Register Masks		      *
* 									      *
******************************************************************************/

#define GS_PCI_CARD_BUS_CIS_PTR_ADDR_SPC_INDICATOR_MASK	0x70000000
#define GS_PCI_CARD_BUS_CIS_PTR_ADDR_SPC_OFF_MASK	0x0FFFFFFF


/******************************************************************************
*                                                                             *
*       Expansion ROM Base Address Configuration Register Masks		      *
*                                                                             *
******************************************************************************/

#define GS_PCI_EXP_ROM_BASE_ADDR_MASK			0xFFFFF800
#define GS_PCI_EXP_ROM_BASE_ADDR_DECODE_ENABLE_MASK	0x00000001


/******************************************************************************
*                                                                             *
*       Expansion ROM Header Offset Values				      *
*                                                                             *
******************************************************************************/

#define GS_PCI_EXP_ROM_SIGNATURE_OFF			0x00
#define GS_PCI_EXP_ROM_SIGNATURE_BYTE_1_OFF		0x00
#define GS_PCI_EXP_ROM_SIGNATURE_BYTE_2_OFF		0x01
#define GS_PCI_EXP_ROM_SIGNATURE_BYTE_1			0x55
#define GS_PCI_EXP_ROM_SIGNATURE_BYTE_2			0xAA

#define GS_PCI_EXP_ROM_PCI_DATA_STR_PTR_OFF		0x18
#define GS_PCI_EXP_ROM_PCI_DATA_STR_PTR_LO_BYTE_OFF	0x18
#define GS_PCI_EXP_ROM_PCI_DATA_STR_PTR_HI_BYTE_OFF	0x19

/* The following offsets are valid for a code type of Intel x86 only	*/

#define GS_PCI_EXP_ROM_INTEL_X86_INIT_SIZE_OFF		0x02
#define GS_PCI_EXP_ROM_INTEL_X86_INIT_SIZE_LEN		1

#define GS_PCI_EXP_ROM_INTEL_X86_ENTRY_PT_OFF		0x03
#define GS_PCI_EXP_ROM_INTEL_X86_ENTRY_PT_LEN		3


/******************************************************************************
*                                                                             *
*       Expansion ROM Data Structure Offset Values			      *
*                                                                             *
******************************************************************************/

#define GS_PCI_EXP_ROM_SIGNATURE_STR_ADDR_OFF		0x00
#define GS_PCI_EXP_ROM_SIGNATURE_STR_ADDR_LEN		4
#define GS_PCI_EXP_ROM_SIGNATURE_STR			"PCIR"

#define GS_PCI_EXP_ROM_VEND_ID_OFF			0x04
#define GS_PCI_EXP_ROM_VEND_ID_LO_BYTE_OFF		0x04
#define GS_PCI_EXP_ROM_VEND_ID_HI_BYTE_OFF		0x05
#define GS_PCI_EXP_ROM_VEND_ID_LEN			2

#define GS_PCI_EXP_ROM_DEV_ID_OFF			0x06
#define GS_PCI_EXP_ROM_DEV_ID_LO_BYTE_OFF		0x06
#define GS_PCI_EXP_ROM_DEV_ID_HI_BYTE_OFF		0x07
#define GS_PCI_EXP_ROM_DEV_ID_LEN			2

#define GS_PCI_EXP_ROM_VPD_PTR_OFF			0x08
#define GS_PCI_EXP_ROM_VPD_PTR_LO_BYTE_OFF		0x08
#define GS_PCI_EXP_ROM_VPD_PTR_HI_BYTE_OFF		0x09
#define GS_PCI_EXP_ROM_VPD_PTR_LEN			2

#define GS_PCI_EXP_ROM_PCI_DATA_STR_LEN_OFF		0x0A
#define GS_PCI_EXP_ROM_PCI_DATA_STR_LEN_LO_BYTE_OFF	0x0A
#define GS_PCI_EXP_ROM_PCI_DATA_STR_LEN_HI_BYTE_OFF	0x0B
#define GS_PCI_EXP_ROM_PCI_DATA_STR_LEN_LEN		2

#define GS_PCI_EXP_ROM_PCI_DATA_STR_REV_OFF		0x0C
#define GS_PCI_EXP_ROM_PCI_DATA_STR_REV_LEN		1

#define GS_PCI_EXP_ROM_CLASS_CODE_OFF			0x0D
#define GS_PCI_EXP_ROM_CLASS_CODE_LEN			3
#define GS_PCI_EXP_ROM_PRGM_INTRFC_OFF			0x0D
#define GS_PCI_EXP_ROM_PRGM_INTRFC_LEN			1
#define GS_PCI_EXP_ROM_SUB_CLASS_CODE_OFF		0x0E
#define GS_PCI_EXP_ROM_SUB_CLASS_CODE_LEN		1
#define GS_PCI_EXP_ROM_BASE_CLASS_CODE_OFF		0x0F
#define GS_PCI_EXP_ROM_BASE_CLASS_CODE_LEN		1

#define GS_PCI_EXP_ROM_IMG_LEN_OFF			0x10
#define GS_PCI_EXP_ROM_IMG_LEN_LO_BYTE_OFF		0x10
#define GS_PCI_EXP_ROM_IMG_LEN_HI_BYTE_OFF		0x11
#define GS_PCI_EXP_ROM_IMG_LEN_LEN			2

#define GS_PCI_EXP_ROM_CODE_DATA_REV_OFF		0x12
#define GS_PCI_EXP_ROM_CODE_DATA_REV_LO_BYTE_OFF	0x12
#define GS_PCI_EXP_ROM_CODE_DATA_REV_HI_BYTE_OFF	0x13
#define GS_PCI_EXP_ROM_CODE_DATA_REV_LEN		2

#define GS_PCI_EXP_ROM_CODE_TYPE_OFF			0x14
#define GS_PCI_EXP_ROM_CODE_TYPE_LEN			1
#define GS_PCI_EXP_ROM_CODE_TYPE_INTEL_X86		0x00
#define GS_PCI_EXP_ROM_CODE_TYPE_OPEN_FIRMWARE		0x01

#define GS_PCI_EXP_ROM_INDICATOR_OFF			0x15
#define GS_PCI_EXP_ROM_INDICATOR_LEN			1
#define GS_PCI_EXP_ROM_INDICATOR_LAST_IMG_MASK		0x80
#define GS_PCI_EXP_ROM_INDICATOR_ANOTHER_IMG_FOLLOWS 	0x00
#define GS_PCI_EXP_ROM_INDICATOR_LAST_IMG     		0x80

#define GS_PCI_VPD_LEN					0x100	/* a guess */


/******************************************************************************
* 									      *
*	Macros to Write Configuration Registers				      *
* 									      *
******************************************************************************/

#define GS_PCI_SET_CFG_REGC(base, register, data)			\
{									\
	GS_IO_TRC(GS_IO_TRC_DD_LEVEL_CFG_REG,				\
		((unsigned long)(base) | (register)), (data));		\
									\
	(*((volatile unsigned char *)					\
		((unsigned long)(base) | (register)))) =		\
		((volatile unsigned char)(data));			\
}

#define GS_PCI_SET_CFG_REGS(base, register, data)			\
{									\
	GS_IO_TRC(GS_IO_TRC_AP_LEVEL_CFG_REG,				\
		((unsigned long)(base) | (register)), (data));		\
									\
	(*((volatile unsigned long *)					\
		((unsigned long)(base) | (register)))) =		\
		((volatile unsigned short)(data));			\
}

#define GS_PCI_SET_CFG_REGL(base, register, data)			\
{									\
	GS_IO_TRC(GS_IO_TRC_AP_LEVEL_CFG_REG,				\
		((unsigned long)(base) | (register)), (data));		\
									\
	(*((volatile unsigned long *)					\
		((unsigned long)(base) | (register)))) =		\
		((volatile unsigned long)(data));			\
}


/******************************************************************************
* 									      *
*	Macros to Write Configuration Registers (Byte Swapped)		      *
* 									      *
******************************************************************************/

#define GS_PCI_SET_CFG_REGS_SWAPPED(base, register, data)		\
{									\
	GS_IO_TRC(GS_IO_TRC_AP_LEVEL_CFG_REG,				\
		((unsigned long)(base) | (register)),			\
		(GS_BYTE_REV_S(data)));					\
									\
	(*((volatile unsigned long *)					\
		((unsigned long)(base) | (register)))) =		\
		((volatile unsigned short)(GS_BYTE_REV_S(data)));	\
}

#define GS_PCI_SET_CFG_REGL_SWAPPED(base, register, data)		\
{									\
	GS_IO_TRC(GS_IO_TRC_AP_LEVEL_CFG_REG,				\
		((unsigned long)(base) | (register)),			\
		(GS_BYTE_REV_L(data)));					\
									\
	(*((volatile unsigned long *)					\
		((unsigned long)(base) | (register)))) =		\
		((volatile unsigned long)(GS_BYTE_REV_L(data)));	\
}


/******************************************************************************
* 									      *
*	Macros to Read Configuration Registers				      *
* 									      *
******************************************************************************/

#define GS_PCI_GET_CFG_REGC(base, register)				\
	(volatile unsigned char)					\
		(*((volatile unsigned char *)				\
		((unsigned long)(base) | (register))))

#define GS_PCI_GET_CFG_REGS(base, register)				\
	(volatile unsigned short)					\
		(*((volatile unsigned short *)				\
		((unsigned long)(base) | (register))))

#define GS_PCI_GET_CFG_REGL(base, register)				\
	(volatile unsigned long)					\
		(*((volatile unsigned long *)				\
		((unsigned long)(base) | (register))))


/******************************************************************************
* 									      *
*	Macros to Read Configuration Registers (Byte Swapped)		      *
* 									      *
*	NOTE: These macros could not be defined as LHS macros due to	      *
*	the fact that the GS_BYTE_REV(n) macros use the (n) parameter	      *
*	more than once in the definition which would cause unwarranted	      *
*	reads of the hardware.						      *
* 									      *
******************************************************************************/

#define GS_PCI_GET_CFG_REGS_SWAPPED(base, register, data)		\
{									\
	unsigned short	__gs_tmp_s;					\
									\
	__gs_tmp_s = (volatile unsigned short)				\
		(*((volatile unsigned short *)				\
		((unsigned long)(base) | (register))));			\
	(data) = GS_BYTE_REV_S(__gs_tmp_s);				\
}

#define GS_PCI_GET_CFG_REGL_SWAPPED(base, register, data)		\
{									\
	unsigned long __gs_tmp_l;					\
									\
	__gs_tmp_l = (volatile unsigned long)				\
		(*((volatile unsigned long *)				\
		((unsigned long)(base) | (register))));			\
	(data) = GS_BYTE_REV_L(__gs_tmp_l);				\
}


/******************************************************************************
* 									      *
*	Generic PCI Expansion ROM Header Format				      *
* 									      *
******************************************************************************/

typedef struct _gs_pci_ROM_hdr_t {
	unsigned char	rom_sig_byte1;		/* contains 0x55	*/
	unsigned char	rom_sig_byte2;		/* contains 0xAA	*/
	unsigned char	reserved[22];		/* reserved		*/
	unsigned short	pci_data_struct_ptr;	/* in Intel format	*/
} gs_pci_ROM_hdr_t, *pgs_pci_ROM_hdr_t;


/******************************************************************************
* 									      *
*	PC Compatible PCI Expansion ROM Header Format			      *
* 									      *
******************************************************************************/

typedef struct _gs_pc_pci_ROM_hdr_t {
	unsigned char	rom_sig_byte1;		/* contains 0x55	*/
	unsigned char	rom_sig_byte2;		/* contains 0xAA	*/
	unsigned char	init_code_size;		/* in 512 byte units	*/
	unsigned char	init_entry_point[3];	/* 3 byte offset	*/
	unsigned char	reserved[18];		/* reserved		*/
	unsigned short	pci_data_struct_ptr;	/* in Intel format	*/
} gs_pc_pci_ROM_hdr_t, *pgs_pc_pci_ROM_hdr_t;


/******************************************************************************
* 									      *
*	PCI Data Structure Format					      *
* 									      *
******************************************************************************/

typedef struct _gs_pci_ROM_data_struct_t {
	unsigned char	signature[4];		/* contains "PCIR"	*/
	unsigned short	vendor_id;		/* in Intel format	*/
	unsigned short	device_id;		/* in Intel format	*/
	unsigned short	vpd_ptr;		/* in Intel format	*/
	unsigned short	pci_data_struct_len;	/* in Intel format	*/
	unsigned char	pci_data_struct_rev;	/* contains 0x00	*/
	union {
		unsigned char	all[3];		/* 3 byte class code	*/
		struct {
			unsigned char grgm_intrfc;
			unsigned char sub_class;
			unsigned char base_class;
		} sub_code;
	} class_code;				/* class code		*/
	unsigned short	image_len;		/* in Intel format	*/
	unsigned short	code_data_rev_level;	/* code/data specific	*/
	unsigned char	code_type;		/* use #define's above	*/
	unsigned char	indicator;		/* use #define's above	*/
	unsigned short	reserved;
} gs_pci_ROM_data_struct_t, *pgs_pci_ROM_data_struct_t;


#endif /* _H_GS_PCI */

