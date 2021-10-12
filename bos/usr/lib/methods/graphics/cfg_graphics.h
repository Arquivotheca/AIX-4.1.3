/* @(#)44	1.5  src/bos/usr/lib/methods/graphics/cfg_graphics.h, dispcfg, bos411, 9428A410j 7/5/94 11:28:55 */

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
 ---------------------------------------------------------------------------

 PURPOSE: Defines the standard data structures used by the cfg_graphics
	  config methods and by the cfg_graphics_tools.

 
 INPUTS:  n/a

 OUTPUTS: n/a

 DATA:    A set of typedefs:

 *************************************************************************/


#ifndef	_H_CFG_GRAPHICS
#define _H_CFG_GRAPHICS

/*======================================================================
|
| Define the constants used by various programs associated with config
|
|=======================================================================*/


/*=======================================================================
|
| cfg_graphics_funcs_t
|
| access to device specific config methods occurs through these
| four pointer to function.  
|
|=======================================================================*/

typedef struct
{

int				(* gen_dev_minor_num 	)( );
int				(* build_dds 	 	)( );
int				(* download_ucode	)( );
int				(* query_vpd		)( );
int                             (* make_special_file    )( );  /* 601 CHANGE */

} cfg_graphics_funcs_t;


/*========================================================================
|
| cfg_graphics_data_t
|
| this holds all of the global data used by a cfg_graphics config method
|========================================================================*/

typedef struct
{

ccm_pathnames_t			ccm_pathnames;

cdd_vpd_t			vpd;

ccm_dds_t			ccm_dds_struc;
ccm_dds_t *			ccm_dds;

void *				dev_dds;

void **				dds;
ulong				dds_length;

char				ddname[MAXNAMLEN];
char				dd_pin[MAXNAMLEN];
char				cfg_methods_name[MAXNAMLEN];

int 				(*cfg_methods_entry)( );

ulong				use_dev_vdd;
ulong				use_ccm_vdd;
ulong				use_dev_ucode;
ulong				use_ccm_ucode;

ulong				device_uses_VIDEO_ROS;
ulong				ccm_ucode_from_IPLROS;

#define MAX_SLOT_NUM 8

ulong				adapter_present[MAX_SLOT_NUM];
ulong				adapter_bad[MAX_SLOT_NUM];
ulong				detected_error[MAX_SLOT_NUM];
ulong                           R2_ROM_type[MAX_SLOT_NUM];
ulong                           scan_code_present[MAX_SLOT_NUM];
 
int				R2_VideoRomBaseAddress;
int				bus_fd[ 8 ];

int				read_opcode;
int				write_opcode;

cdd_command_attrs_t		cdd_attrs;
cdd_device_attrs_t		cdd_device;
cdd_procs_t			cdd_procs;
cdd_svcs_t			cdd_svcs;
cdd_header_t			cdd;
cfg_graphics_funcs_t *		cfg_func;
cfg_graphics_funcs_t 		s_cfg_func;

ulong				frs_sub_0;
ulong				frs_sub_1;
ulong				frs_sub_2;
int				frs_buid_type;

} cfg_graphics_data_t;



#endif
