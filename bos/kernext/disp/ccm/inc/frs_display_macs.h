/* @(#)11	1.4  src/bos/kernext/disp/ccm/inc/frs_display_macs.h, dispccm, bos411, 9428A410j 7/5/94 11:34:38 */
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
| given in companion file "frs_display.h".  In general, code which uses the
| structures defined in frs_display.h should always reference those structures
| with the macros in this file.
|
|========================================================================*/


/*=========================================================================
|
| macros which operate on the 
| rom_scan_video_head structure
|
|==========================================================================*/

#define	RSCAN_VIDEO_HEAD					\
		rom_scan_video_head_t

#define	RSCAN_VIDEO_HEAD_SIZE					\
		/* size_t					\
		|-------------------------------*/		\
		( sizeof( rom_scan_video_head_t) )

#define RSCAN_VIDEO_HEAD_rom_Blength(ptr)			\
		/* ushort					\
		 | rom_scan_video_head_t *ptr			\
		 |------------------------------*/		\
		((ptr) -> rom_length_in_512B_blocks)

#define RSCAN_VIDEO_HEAD_residue(ptr)				\
		/* ushort					\
		 | rom_scan_video_head_t *ptr			\
		 |------------------------------*/		\
		((ptr) -> rom_CRC_residue)

#define RSCAN_VIDEO_HEAD_rom_type(ptr)				\
		/* ulong					\
		 | rom_scan_video_head_t *ptr			\
		 |------------------------------*/		\
		((ptr) -> rom_type)

#define RSCAN_VIDEO_HEAD_ucode_offset(ptr)			\
		/* ulong					\
		 | rom_scan_video_head_t *ptr			\
		 |------------------------------*/		\
		((ptr) -> offset_to_ucode)

#define RSCAN_VIDEO_HEAD_ucode_len(ptr)				\
		/* ulong					\
		 | rom_scan_video_head_t *ptr			\
		 |------------------------------*/		\
		((ptr) -> length_of_ucode)

#define RSCAN_VIDEO_HEAD_odm_offset(ptr)			\
		/* ulong					\
		 | rom_scan_video_head_t *ptr			\
		 |------------------------------*/		\
		((ptr) -> offset_to_ODM_file)

#define RSCAN_VIDEO_HEAD_odm_len(ptr)				\
		/* ulong					\
		 | rom_scan_video_head_t *ptr			\
		 |------------------------------*/		\
		((ptr) -> length_of_ODM_file)

#define RSCAN_VIDEO_HEAD_kmod_offset(ptr,i)			\
		/* ulong					\
		 | rom_scan_video_head_t *ptr			\
		 |------------------------------*/		\
		((ptr) -> kmod_functions[i].offset_to_kmod )

#define RSCAN_VIDEO_HEAD_kmod_len(ptr,i)			\
		/* ulong					\
		 | rom_scan_video_head_t *ptr			\
		 |------------------------------*/		\
		((ptr) -> kmod_functions[i].length_of_kmod )

#define RSCAN_VIDEO_HEAD_ext_offset(ptr,i)			\
		/* ulong					\
		 | rom_scan_video_head_t *ptr			\
		 |------------------------------*/		\
		((ptr) -> kmod_extensions[i].offset_to_kmod )

#define RSCAN_VIDEO_HEAD_ext_len(ptr,i)				\
		/* ulong					\
		 | rom_scan_video_head_t *ptr			\
		 |------------------------------*/		\
		((ptr) -> kmod_extensions[i].length_of_kmod )

#define RSCAN_VIDEO_HEAD_copyright(ptr)				\
		/* char *					\
		 | rom_scan_video_head_t *ptr			\
		 |------------------------------*/		\
		((ptr) -> copyright_notice )

