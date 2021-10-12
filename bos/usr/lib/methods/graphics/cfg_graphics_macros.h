/* @(#)48	1.6  src/bos/usr/lib/methods/graphics/cfg_graphics_macros.h, dispcfg, bos411, 9428A410j 7/5/94 11:29:00 */

/*
 *   COMPONENT_NAME: SYSXDISPCCM
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/******************************************************************************
 **************************** MODULE HEADER TOP *******************************
 ******************************************************************************
 ---------------------------------------------------------------------------

 PURPOSE: 
	 	Defines the macros known by the prefix CFGGRAPH_ and CFG_
		which are used to reference data in the cfg_graphics.h
		structures.

 
 INPUTS:  n/a

 OUTPUTS: n/a

 *************************************************************************
 ************************** MODULE HEADER BOTTOM *************************
 *************************************************************************/


#ifndef _H_CFG_GRAPHICS_MACROS
#define _H_CFG_GRAPHICS_MACROS

/*========================================================================
|
|  macros which access the CFG GRAPHICS DATA structure using the
|  GLOBAL DATA POINTER that is defined in cfg_graphics.c	
|
|=========================================================================*/

/*-----------------------------------
| first, a macro which access the global
| copy of the ccm pathnames structure
|-------------------------------------*/

#define CFGGRAPH_ccm_pathnames						\
		/* ccm_pathnames_t 	*/				\
		( &(__GLOBAL_DATA_PTR -> ccm_pathnames))

/*---------------------------------------
| macros which access the full function 
| objects (non-CCM)
| 
| the full function objects are:
| 	- the dev_dd
|	- the dev_dd_pin
|	- the dev_ucode
|----------------------------------------*/

#define CFGGRAPH_dev_dd_name						\
		/* char *		*/				\
	CCM_PATHNAME_dev_dd( CFGGRAPH_ccm_pathnames )

#define CFGGRAPH_dev_dd_pin						\
		/* char *		*/				\
	CCM_PATHNAME_dev_dd_pin( CFGGRAPH_ccm_pathnames )

#define	CFGGRAPH_dev_ucode						\
		/* char *		*/				\
	CCM_PATHNAME_dev_ucode( CFGGRAPH_ccm_pathnames )

/*---------------------------------------
| macros which access the CCM mode objects
| 
| the CCM Mode objects are:
| 	- the ccm_dd
|	- the ccm_dd_pin
|	- the ccm_ucode
|----------------------------------------*/

#define CFGGRAPH_ccm_dd_name						\
		/* char *		*/				\
	CCM_PATHNAME_ccm_dd( CFGGRAPH_ccm_pathnames )

#define CFGGRAPH_ccm_dd_pin						\
		/* char *		*/				\
	CCM_PATHNAME_ccm_dd_pin( CFGGRAPH_ccm_pathnames )

#define	CFGGRAPH_ccm_ucode						\
		/* char *		*/				\
	CCM_PATHNAME_ccm_ucode( CFGGRAPH_ccm_pathnames )

/*-----------------------------------------
| macros which access the two CDD objects
|
| the CDD objects are:
|	- the CDD entry kmod
|	= the CDD pin kmod
|------------------------------------------*/

#define	CFGGRAPH_cdd_name						\
		/* char *		*/				\
	CCM_PATHNAME_cdd_kmod_entry( CFGGRAPH_ccm_pathnames )

#define CFGGRAPH_cdd_pin						\
		/* char *		*/				\
	CCM_PATHNAME_cdd_kmod_interrupt( CFGGRAPH_ccm_pathnames )

/*-------------------------------------------
| generic location of the device driver names
| useful after the gang-switch from ccm to regular
|-------------------------------------------*/

#define CFGGRAPH_ddname							\
		/* char *		*/				\
		(__GLOBAL_DATA_PTR -> ddname )

#define CFGGRAPH_dd_pin							\
		/* char *		*/				\
		(__GLOBAL_DATA_PTR -> dd_pin )

/*--------------------------------------------
| macros which access the FRS global data 
|
|--------------------------------------------*/

#define	CFGGRAPH_frs_sub_0						\
		/* ulong		*/				\
		(__GLOBAL_DATA_PTR -> frs_sub_0 )

#define	CFGGRAPH_frs_sub_1						\
		/* ulong		*/				\
		(__GLOBAL_DATA_PTR -> frs_sub_1 )

#define	CFGGRAPH_frs_sub_2						\
		/* ulong		*/				\
		(__GLOBAL_DATA_PTR -> frs_sub_2 )

#define	CFGGRAPH_frs_buid_type						\
		/* int			*/				\
		(__GLOBAL_DATA_PTR -> frs_buid_type )

/*-------------------------------------------
| other macros
|--------------------------------------------*/

#define CFGGRAPH_cfg_methods     					\
		/* char *		*/				\
		(__GLOBAL_DATA_PTR -> cfg_methods_name )

#define CFGGRAPH_methods_entry(ptr)					\
                /* int                  */                              \
                (*( __GLOBAL_DATA_PTR ->cfg_methods_entry  ))(ptr)

#define CFGGRAPH_methods_entry_init					\
                /* int                  */                              \
                ( __GLOBAL_DATA_PTR ->cfg_methods_entry  )

#define CFGGRAPH_vpd							\
		/* ccm_vpd_t		*/				\
		(__GLOBAL_DATA_PTR -> vpd)

#define	CFGGRAPH_ccm_dds						\
		/* ccm_dds_t *		*/				\
		(__GLOBAL_DATA_PTR -> ccm_dds )

#define	CFGGRAPH_dev_dds						\
		/* void *		*/				\
		(__GLOBAL_DATA_PTR -> dev_dds )

#define	CFGGRAPH_dds_ptr_init						\
		/* void *		*/				\
		(__GLOBAL_DATA_PTR -> dds )		

#define	CFGGRAPH_dds							\
		/* void *		*/				\
		(*(__GLOBAL_DATA_PTR -> dds ))		

#define CFGGRAPH_dds_length						\
		/* ulong		*/				\
		(__GLOBAL_DATA_PTR -> dds_length )

#define	CFGGRAPH_use_ccm_vdd						\
		/* ulong		*/				\
		(__GLOBAL_DATA_PTR -> use_ccm_vdd)

#define	CFGGRAPH_use_dev_vdd						\
		/* ulong		*/				\
		(__GLOBAL_DATA_PTR -> use_dev_vdd)

#define CFGGRAPH_use_device_ucode					\
		/* ulong		*/				\
		(__GLOBAL_DATA_PTR -> use_dev_ucode)

#define CFGGRAPH_use_ccm_ucode						\
		/* ulong		*/				\
		(__GLOBAL_DATA_PTR -> use_ccm_ucode)

#define CFGGRAPH_ccm_ucode_from_IPLROS					\
		/* ulong		*/				\
		(__GLOBAL_DATA_PTR -> ccm_ucode_from_IPLROS)

#define CFGGRAPH_device_uses_VIDEO_ROS					\
		/* ulong		*/				\
		(__GLOBAL_DATA_PTR -> device_uses_VIDEO_ROS)

#define	CFGGRAPH_cdd							\
		/* cdd_header_t *	*/				\
		&__GLOBAL_DATA_PTR -> cdd 

#define	CFGGRAPH_cfg_func						\
		/* cfg_graphics_func_t *	*/			\
		(__GLOBAL_DATA_PTR -> cfg_func )

#define CFGGRAPH_gen_minor_dev_num					\
		/* int			*/				\
		(*(CFGGRAPH_cfg_func -> gen_dev_minor_num ))

#define CFGGRAPH_build_dds						\
		/* int			*/				\
		(*(CFGGRAPH_cfg_func -> build_dds ))

#define CFGGRAPH_download_ucode						\
		/* int			*/				\
		(*(CFGGRAPH_cfg_func -> download_ucode ))

#define CFGGRAPH_query_vpd						\
		/* int			*/				\
		(*(CFGGRAPH_cfg_func -> query_vpd ))
/* 601_CHANGE */
#define CFGGRAPH_make_special_file                                      \
                /* int                  */                              \
                (*(CFGGRAPH_cfg_func -> make_special_file ))

#define	CFGGRAPH_bus_fd							\
		/* int * (array of 8 )	*/				\
		(__GLOBAL_DATA_PTR -> bus_fd )

#define CFGGRAPH_adapter_present(index)                                   \
                /* ulong                  */                              \
                (__GLOBAL_DATA_PTR -> adapter_present[index] )

#define CFGGRAPH_adapter_bad(index)                                       \
                /* ulong                  */                              \
                (__GLOBAL_DATA_PTR -> adapter_bad[index] )

#define CFGGRAPH_detected_error(index)                                    \
                /* ulong                  */                              \
                (__GLOBAL_DATA_PTR -> detected_error[index] )

#define CFGGRAPH_R2_ROM_type(index)                                 \
                /* ulong                  */                              \
                (__GLOBAL_DATA_PTR -> R2_ROM_type[index] )

#define CFGGRAPH_scan_code_present(index)                                 \
                /* ulong                  */                              \
                (__GLOBAL_DATA_PTR -> scan_code_present[index] )

#define CFGGRAPH_R2_base_address					\
		/* int			*/				\
		(__GLOBAL_DATA_PTR -> R2_VideoRomBaseAddress)

#define CFGGRAPH_read_opcode					\
		/* int			*/				\
		(__GLOBAL_DATA_PTR -> read_opcode)

#define CFGGRAPH_write_opcode					\
		/* int			*/				\
		(__GLOBAL_DATA_PTR -> write_opcode)

/*========================================================================
|
|  macros which access the cfg_graphics_funcs_t structure.
|
|=========================================================================*/

#define CFGGRAPH_funcs							\
		/* cfg_graphics_funcs_t		*/			\
		(&(__GLOBAL_DATA_PTR -> s_cfg_func))

#define CFG_gen_minor_dev_num(ptr)					\
		/* int			*/				\
		( (ptr) -> gen_dev_minor_num )

#define CFG_build_dds(ptr)						\
		/* int			*/				\
		( (ptr) -> build_dds )

#define CFG_download_ucode(ptr)						\
		/* int			*/				\
		( (ptr) -> download_ucode )

#define CFG_query_vpd(ptr)						\
		/* int			*/				\
		( (ptr) -> query_vpd )

/* 601 CHANGE */
#define CFG_make_special_file(ptr)                                      \
                /* int                  */                              \
                ( (ptr) -> make_special_file )

#endif
