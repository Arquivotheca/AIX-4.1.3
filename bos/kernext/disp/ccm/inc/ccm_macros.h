/* @(#)18	1.5  src/bos/kernext/disp/ccm/inc/ccm_macros.h, dispccm, bos41J, 9509A_all 2/7/95 10:49:29  */
/*
 *   COMPONENT_NAME: dispccm
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************
 ********************** MODULE HEADER TOP ************************************
 *****************************************************************************

 MODULE NAME:		ccm_macros.h
 
 TITLE:  		Common Character Mode Video Device Driver
 	 		Macros for Accessing CCM Structures

 PURPOSE:	Defines the macros which access various parts
                of the CCM data structures.  Use of the macros is
		highly recommended, rather than using direct references
		to the code.


 
 INPUTS:  	n/a

 OUTPUTS: 	n/a

 FUNCTIONS/SERVICES CALLED:	n/a 

 DATA:		n/a


 *****************************************************************************
 ********************** MODULE HEADER BOTTOM *********************************
 *****************************************************************************/



#ifndef __H_CCM_MACROS
#define __H_CCM_MACROS



/*=======================================================================
|
|	Macros which access the members of the CCM_DDS structure
|
|=======================================================================*/


#define	CCM_DDS_bus_mem_start( dds,i )					\
		/* ulong		*/				\
		( (dds) -> io_bus_mem_start[(i)] )

#define	CCM_DDS_bus_mem_length( dds,i )					\
		/* ulong		*/				\
		( (dds) -> io_bus_mem_length[(i)] )

#define	CCM_DDS_RISC_rom_addr( dds )					\
		/* ulong		*/				\
		( (dds) -> RISC_rom_addr )

#define CCM_DDS_bus_slot( dds )						\
		/* ushort		*/				\
		( (dds) -> slot_number )

#define CCM_DDS_bus_int_level( dds )					\
		/* short		*/				\
		( (dds) -> int_level )

#define	CCM_DDS_bus_int_priority( dds )					\
		/* short		*/				\
		( (dds) -> int_priority )

#define CCM_DDS_bus_dma_arb_level( dds )				\
		/* short		*/				\
		( (dds) -> dma_arb_level )

#define CCM_DDS_bus_id( dds )						\
		/* ulong		*/				\
		( (dds) -> bus_id )

#define	CCM_DDS_bus_type( dds )						\
		/* ulong		*/				\
		( (dds) -> bus_type )

#define	CCM_DDS_device_num( dds )					\
		/* int			*/				\
		( (dds) -> device_num )

#define	CCM_DDS_bus_type_is_microchannel( dds )				\
		/* n/a			*/				\
		( CCM_DDS_bus_type(dds) == CCM_BUS_TYPE_MICROCHANNEL )

#define	CCM_DDS_bus_type_is_220_BUID40( dds )				\
		/* n/a			*/				\
		( CCM_DDS_bust_type(dds) == CCM_BUS_TYPE_220_BUID40 )

#define	CCM_DDS_screen_width_pel( dds )					\
		/* short		*/				\
		( (dds) -> screen_width_pel )

#define	CCM_DDS_screen_height_pel( dds )				\
		/* short		*/				\
		( (dds) -> screen_height_pel )

#define	CCM_DDS_screen_width_mm( dds )					\
		/* short		*/				\
		( (dds) -> screen_width_mm )

#define	CCM_DDS_screen_height_mm( dds )					\
		/* short		*/				\
		( (dds) -> screen_height_mm )

#define CCM_DDS_config_display_id( dds )				\
		/* ulong		*/				\
		( (dds) -> display_id )

#define CCM_DDS_ksr_color_table( dds )					\
		/* ulong array base	*/				\
		( (dds) -> ksr_color_table )

#define CCM_DDS_ucode_fd( dds )						\
		/* int  		*/				\
		( (dds) -> microcode_fd )

#define CCM_DDS_component( dds )					\
		/* char *		*/				\
		( (dds) -> component )

#define	CCM_DDS_ucode_filename( dds )					\
		/* char *		*/				\
		( (dds) -> ucode_name )

#define CCM_DDS_ccmdd_kmid( dds )					\
		/* mid_t		*/				\
		( (dds) -> dd_kmid )

#define CCM_DDS_ccmdd_pin_kmid( dds )					\
		/* mid_t		*/				\
		( (dds) -> dd_pin_kmid )

#define CCM_DDS_cdd_kmid( dds )						\
		/* mid_t		*/				\
		( (dds) -> cdd_kmid )

#define CCM_DDS_cdd_pin_kmid( dds )					\
		/* mid_t		*/				\
		( (dds) -> cdd_pin_kmid )

#define CCM_DDS_cdd_ext_kmid( dds )					\
		/* mid_t		*/				\
		( (dds) -> cdd_ext_kmid )

#define CCM_DDS_cdd_ext_pin_kmid( dds )					\
		/* mid_t		*/				\
		( (dds) -> cdd_ext_pin_kmid )

#define CCM_DDS_global_dd_data( dds )					\
		/* void *		*/				\
		( (dds) -> p_ccm_dd_data )	

#define CCM_DDS_x_min( dds )						\
		/* ulong		*/				\
		( (dds) -> x_min )	

#define CCM_DDS_y_min( dds )						\
		/* ulong		*/				\
		( (dds) -> y_min )	

#define CCM_DDS_x_max( dds )						\
		/* ulong		*/				\
		( (dds) -> x_max )	

#define CCM_DDS_y_max( dds )						\
		/* ulong		*/				\
		( (dds) -> y_max )	

/* 
 * Processor & architecture extension for all busses 
 */

#define CCM_DDS_60x_architecture( dds )	/* POWER_RS or POWER_PC */	\
		( (dds) -> architecture )

#define CCM_DDS_60x_implementation( dds )/* POWER_[RS1 | RS2 | RSC | 601] */\
		( (dds) -> implementation )

/*
 * Extensions for 60x bus
 */

#define CCM_DDS_60x_int_server( dds )					\
		( (dds) -> int_server )

#define CCM_DDS_60x_segment( dds )					\
		( (dds) -> segment_60x )

#define CCM_DDS_60x_buid( dds, i )					\
		( (dds) -> buid_60x[ (i) ] )




/*
 *      The following macros write the specified data to 60x memory.
 *      These macros are statements.
 */

/* GS_IO_MAP_INIT -- This macro can be used to initialize an io_map structure.
 *      Additional function (smaller mapping granularity, readonly) is
 *      available by filling out individual fields in mapping structure
 *
 *      WARNING: region to map can not cross 256Meg (SEGSIZE) boundary
 */

#define GS_IO_MAP_INIT(iomap, bid_parm, busaddr_parm)                   \
{                                                                       \
        (iomap)->key = IO_MEM_MAP;                                      \
        (iomap)->flags = 0;                                             \
        (iomap)->size = SEGSIZE;                                        \
        (iomap)->bid = (bid_parm);                                      \
        (iomap)->busaddr = (busaddr_parm) ;                             \
}

	/* requires sys/adspace.h and sys/ioacc.h */			\


#define CCM_BUS_PUT_60x(p,v,iolen)   					\
	/* iolen must be uchar, ushort or ulong */			\
	/* requires sys/adspace.h and sys/ioacc.h */			\
{									\
    struct io_map   io_map ;						\
    ulong           base_io_addr ;					\
    ulong           actual_io_addr ;					\
									\
	GS_IO_MAP_INIT (&(io_map), REALMEM_BID, (p) & 0xF0000000) ;	\	
	/* inits request to access space of SEGSIZE == 256 Meg */	\
									\
	base_io_addr = GS_ATTACHGRAPHICSBUS(io_map) ;			\
	actual_io_addr = base_io_addr +					\
			 ((ulong)(p) & CCM_60x_UPPER_NIBBLE_MASK);	\
									\
	*(iolen volatile *)(actual_io_addr) = (iolen)(v) ;		\
									\
	GS_DETACHGRAPHICSBUS(base_io_addr);                        	\
}


#define CCM_BUS_PUTL_60x(p,v)   CCM_BUS_PUT_60x(p,v,ulong)
#define CCM_BUS_PUTS_60x(p,v)   CCM_BUS_PUT_60x(p,v,ushort)
#define CCM_BUS_PUTC_60x(p,v)   CCM_BUS_PUT_60x(p,v,uchar)


/*
 *      The following macros read the specified data from 60x memory.
 *      These macros are expressions.
 */

#define CCM_BUS_GET_60x(p,p_val,iolen) 					\
	/* iolen must be uchar, ushort or ulong */			\
	/* requires sys/adspace.h and sys/ioacc.h */			\
{									\
    struct io_map   io_map ;						\
    ulong           base_io_addr ;					\
    ulong           actual_io_addr ;					\
									\
	GS_IO_MAP_INIT (&io_map, REALMEM_BID, (p) & 0xF0000000) ;	\	
	/* inits request to access space of SEGSIZE == 256 Meg */	\
									\
	base_io_addr = GS_ATTACHGRAPHICSBUS(io_map) ;			\
	actual_io_addr = base_io_addr +					\
			 ((ulong)(p) & CCM_60x_UPPER_NIBBLE_MASK);	\
									\
	*(p_val) = (*((iolen volatile *)(actual_io_addr)));		\
									\
	GS_DETACHGRAPHICSBUS(base_io_addr);                        	\
}



#define CCM_BUS_GETL_60x(p,p_val)   CCM_BUS_GET_60x(p,p_val,ulong)
#define CCM_BUS_GETS_60x(p,p_val)   CCM_BUS_GET_60x(p,p_val,ushort)
#define CCM_BUS_GETC_60x(p,p_val)   CCM_BUS_GET_60x(p,p_val,uchar)

/*=======================================================================
|
|	The following are macros that allow various device driver modules
|	in the CCM VDD to access the global pointer to the DD global data.
|
|	These are needed to enable the use of macros to access the
|	various fields of the ccm_dd_data_t structure. 
|
|	Typically, one module (the config module) will use the macro
|	USE_AS_CCM_GLOBAL_PTR to allocate the global variable.
|	All of the other modules will then use the USE_EXTERN_CCM_GLOBAL_PTR
|	macro to set up an extern reference back to the correct place.
|	The assumption is that the whole device driver shares access
|	to one copy of the CCM structure obtained from the config
|	entry point of the device driver.
|
|========================================================================*/


#define	CCMDD_GLOBAL_DATA_DEFINED_HERE					\
	ccm_dd_data_t *		_CCMDD = NULL;

#define	CCMDD_GLOBAL_DATA_DEFINED_EXTERN				\
	extern ccm_dd_data_t *	_CCMDD;

#define	CCMDD_GLOBAL_DATA_PTR_INIT( ptr )				\
	{								\
		_CCMDD = (ccm_dd_data_t *) (ptr);	\
	}

#define	CCMDD_GLOBAL_DATA_PTR						\
		(_CCMDD)




#define	CCMDD_INTR_GLOBAL_DATA_DEFINED_HERE				\
	ccm_dd_intr_data_t *	_CCMDD_INTR;

#define	CCMDD_INTR_GLOBAL_DATA_DEFINED_EXTERN				\
	extern ccm_dd_intr_data_t *	_CCMDD_INTR;

#define	CCMDD_INTR_GLOBAL_DATA_PTR_INIT( ptr )				\
	{								\
	_CCMDD_INTR = (ccm_dd_intr_data_t *) (ptr);\
	}

#define	CCMDD_INTR_GLOBAL_DATA_PTR					\
		(_CCMDD_INTR)


/*=======================================================================
|
|	Macros which access the members of the ccm_dd_data_t structure
|
|=======================================================================*/

#define CCM_cdd_head(ptr)						\
		/* cdd_header_t *		*/			\
		(ptr -> p_cdd_header)

#define CCM_cdd_svcs(ptr)						\
		/* cdd_svcs_t *			*/			\
		(ptr -> p_cdd_svcs)

#define CCM_cdd_proc(ptr)						\
		/* cdd_procs_t *		*/			\
		(ptr -> p_cdd_procs)

#define CCM_cdd_device(ptr)						\
		/* cdd_device_attrs_t *		*/			\
		(ptr -> p_cdd_device_attrs)

#define CCM_cdd_command(ptr)						\
		/* cdd_command_attrs_t *	*/			\
		(ptr -> p_cdd_command_attrs)

#define CCM_path(ptr)						\
		/* ccm_pathnames_t *		*/			\
		(ptr -> p_ccm_paths)

#define CCM_dds(ptr)						\
		/* ccm_dds_t *			*/			\
		(ptr -> p_ccm_dds)

/*=======================================================================
|
|	Macros which access the members of the CCM_DD_DATA structure
|
|=======================================================================*/


#define	CCMDD_cdd							\
		/* cdd_header_t *		*/			\
		(_CCMDD -> p_cdd_header )

#define	CCMDD_cdd_svcs	 						\
		/* cdd_svcs_t *			*/			\
		(_CCMDD -> p_cdd_svcs )

#define	CCMDD_cdd_procs							\
		/* cdd_procs_t  *		*/			\
		(_CCMDD -> p_cdd_procs )

#define	CCMDD_cdd_dev_attrs						\
		/* cdd_device_attrs_t *		*/			\
		(_CCMDD -> p_cdd_device_attrs )

#define	CCMDD_cdd_dev	CCMDD_cdd_dev_attrs

#define	CCMDD_cdd_cmd_attrs						\
		/* cdd_command_attrs_t *	*/			\
		(_CCMDD -> p_cdd_command_attrs )

#define	CCMDD_cdd_cmd	CCMDD_cdd_cmd_attrs

#define	CCMDD_dds							\
		/* ccm_dds_t *			*/			\
		(_CCMDD -> p_ccm_dds )

#define CCMDD_pathnames							\
		/* ccm_pathname_t *		*/			\
		(_CCMDD -> p_ccm_paths )

#define	CCMDD_has_intr_kmod						\
		/* ulong			*/			\
		(_CCMDD -> has_intr_kmod )

#define	CCMDD_kmid_entry						\
		/* mid_t			*/			\
		(_CCMDD -> kmid_entry )

#define	CCMDD_kmid_interrupt						\
		/* mid_t			*/			\
		(_CCMDD -> kmid_interrupt )

#define	CCMDD_kmid_ccmdd_pin						\
		/* mid_t			*/			\
		(_CCMDD -> kmid_ccmdd_pin )

#define	CCMDD_ksr							\
		/* ccm_ksr_data_t *		*/			\
		(_CCMDD -> p_ccm_ksr_data )

#define	CCMDD_fast_io							\
		/* ulong			*/			\
		(_CCMDD -> vtt_fast_io )

#define	CCMDD_busmem_att						\
		/* ulong			*/			\
		(_CCMDD -> vtt_busmem_att )

#define	CCMDD_iocc_att							\
		/* ulong			*/			\
		(_CCMDD -> vtt_iocc_att )


/*========================================================================
|
|	The following are macros that define certain hardware 
|	characteristics expected by the CCM VDD routines.  The
|	characteristics need to be determined by the config method
|	and passed into the device driver.  These macros de-reference
|	the DDS interface.
|
|========================================================================*/


#undef		X_MIN
#define		X_MIN	CCM_DDS_x_min( CCMDD_dds )

#undef		X_MAX
#define		X_MAX	CCM_DDS_x_max( CCMDD_dds )

#undef		Y_MIN
#define		Y_MIN	CCM_DDS_y_min( CCMDD_dds )

#undef		Y_MAX
#define		Y_MAX	CCM_DDS_y_max( CCMDD_dds )




/*========================================================================
|
| macros that operate on the CCM PATHNAMES structure
|
|=========================================================================*/

#define CCM_PATHNAME_ucode_file( pp )					\
		/* struct file *	*/				\
		( (pp) -> ucode_file_ptr )	

#define CCM_PATHNAME_dev_ucode( pp )					\
		/* char *		*/				\
		( (pp) -> dev_ucodename )

#define CCM_PATHNAME_ccm_ucode( pp )					\
		/* char *		*/				\
		( (pp) -> ccm_ucodename )

#define	CCM_PATHNAME_dev_dd( pp )					\
		/* char *		*/				\
		( (pp) -> device_driver )

#define	CCM_PATHNAME_dev_dd_pin( pp )					\
		/* char *		*/				\
		( (pp) -> device_pinned )

#define CCM_PATHNAME_ccm_dd( pp )					\
		/* char *		*/				\
		( (pp) -> ccm_dd_name )

#define CCM_PATHNAME_ccm_dd_pin( pp )					\
		/* char *		*/				\
		( (pp) -> ccm_dd_pinned )

#define CCM_PATHNAME_cdd_kmod( pp )					\
		/* char *		*/				\
		( (pp) -> cdd_kmod_name )

#define CCM_PATHNAME_cdd_kmod_entry( pp )				\
		/* char *		*/				\
		( (pp) -> cdd_kmod_entry )

#define CCM_PATHNAME_cdd_kmod_interrupt( pp )				\
		/* char *		*/				\
		( (pp) -> cdd_kmod_interrupt )


/*========================================================================
|
| macros that examine the conternts of a segment register
|
|=========================================================================*/

#define CCM_IOCC_VALID( segment )                              	\
        (    (((segment) & CDD_SEG_T_BIT   ) == CDD_SEG_T_BIT )		\
          && (((segment) & CDD_IOCC_SELECT ) == CDD_IOCC_SELECT )	\
          && (((segment) & CDD_BUSx        ) == CDD_BUSx )		\
          && (((segment) & CDD_ADDR_INCR   ) == CDD_ADDR_INCR )		\
          && (((segment) & CDD_ADDR_CHK    ) == CDD_ADDR_CHK )		\
        )


/*======================================================================
|
|	Macros that operate on CCM Interrupt Data
|
|=======================================================================*/

#define	CCM_INTR_dev( pdata )						\
	/* cdd_intr_dev_attrs_t *	*/				\
	( (pdata)->p_cdd_intr_dev_attrs )

#define	CCM_INTR_svcs( pdata )						\
	/* cdd_svcs_t *			*/				\
	( (pdata)->p_cdd_intr_svcs )

#define	CCM_INTR_head( pdata )						\
	/* cdd_intr_head_t *		*/				\
	( (pdata)->p_cdd_intr_head )

#define	CCM_INTR_entry( pdata )						\
	/* ptr to func returning int	*/				\
	( (pdata)->ccmdd_interrupt )

#define	CCM_INTR_cdd_entry( pdata )					\
	/* ptr to func returning int	*/				\
	( (pdata)->cdd_interrupt )


/*======================================================================
|
|	MACROS which access members of the CCM KSR DATA structure
|
|=======================================================================*/

#define	CCM_KSR_max_w( pk )						\
	/* ulong 		*/					\
	( (pk) -> cdd_max_width )

#define	CCM_KSR_max_h( pk )						\
	/* ulong 		*/					\
	( (pk) -> cdd_max_height )

#define	CCM_KSR_max_a( pk )						\
	/* ulong 		*/					\
	( (pk) -> cdd_max_area )

#define	CCM_KSR_origin_type( pk )					\
	/* ulong		*/					\
	( (pk) -> cdd_origin_type )





#endif	/*__H_CCM_MACROS */

