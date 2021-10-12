/* static char sccsid[] = src/bos/kernext/disp/ccm/inc/cdd_macros.h, dispccm, bos411, 9428A410j 7/5/94 11:34:07 */

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

/*****************************************************************************
 ********************** MODULE HEADER TOP ************************************
 *****************************************************************************

 MODULE NAME:                cdd_macros.h
 
 TITLE:                  Common Character Mode Video Device Driver
                          Macros for Accessing CDD Structures

 PURPOSE:        Defines the macros which access various parts
                of the CDD data structures.
 
 	Users of the cdd interfaces should use these macros instead of
 	direct references to the structures, whenever possible.  Such
 	programming practices make for more manageable code .
 
 	Associated with each macro is the typedef of the value returned
 	by the macro.  This is included as a comment next to the macro.
 	It will save the programmer from having to also reference the
 	cdd.h include file.

 
 INPUTS:          n/a

 OUTPUTS:         n/a

 FUNCTIONS/SERVICES CALLED:        n/a 

 DATA:                n/a


 *****************************************************************************
 ********************** MODULE HEADER BOTTOM *********************************
 *****************************************************************************/

#ifndef __H_CDD_MACROS
#define __H_CDD_MACROS

/*==========================================================================
|
|	Macros which return Boolean values, based on a test of an address
|	pair.  The address pair is a segment register contents (b)
|	and a bus memory address (a)
|
|==========================================================================*/
 

#define CDD_ADDR_IS_IOCC( b , a  )				\
	(    (((b) & CDD_SEG_T_BIT   ) == CDD_SEG_T_BIT )	\
	  && (((b) & CDD_IOCC_SELECT ) == CDD_IOCC_SELECT )	\
	  && (((b) & CDD_BUSx        ) == CDD_BUSx )		\
	  && ( (a) >= CDD_IOCC_BASE )				\
	  && ( (a) < CDD_IOCC_TOP )				\
	)

#define	CDD_ADDR_IS_BUS_IO( b , a )				\
	(    (((b) & CDD_SEG_T_BIT   ) == CDD_SEG_T_BIT )	\
	  && (((b) & CDD_IOCC_SELECT ) == 0 )			\
	  && (((b) & CDD_BUSx        ) == CDD_BUSx )		\
	  && (((a) & CDD_BUS_IO_MASK ) == 0 )			\
	)

#define	CDD_ADDR_IS_BUS_MEM( b , a )				\
	(    (((b) & CDD_SEG_T_BIT   ) == CDD_SEG_T_BIT )	\
	  && (((b) & CDD_IOCC_SELECT ) == 0 )			\
	  && (((b) & CDD_BUSx        ) == CDD_BUSx )		\
	  && (( (a) & CDD_BUS_IO_MASK) != 0  )			\
	)

#define	CDD_ADDR_IS_60x_BUS_MEM( segment , address )					\
	(    (((segment) & CDD_60x_32BIT_SET_T_BIT  ) == CDD_60x_32BIT_SET_T_BIT )	\
	  && ( (((segment) & CDD_60x_MOD250_BUID    ) == CDD_60x_MOD250_BUID ) || 	\
	       (((segment) & CDD_60x_R2G_BUID       ) == CDD_60x_R2G_BUID )  )		\
	  && ( ( (address) >= CDD_60x_BUS_MEM_SPACE_BEGIN ) &&				\
	       ( (address) <= CDD_60x_BUS_MEM_SPACE_END ) ) 				\
	)

#define	CDD_ADDR_IS_60x_CFG_SPACE( segment , address )					\
	(    (((segment) & CDD_60x_32BIT_SET_T_BIT  ) == CDD_60x_32BIT_SET_T_BIT )	\
	  && ( (((segment) & CDD_60x_MOD250_BUID    ) == CDD_60x_MOD250_BUID ) || 	\
	       (((segment) & CDD_60x_R2G_BUID       ) == CDD_60x_R2G_BUID )  )		\
	  && ( ( (address) >= CDD_60x_CFG_SPACE_BEGIN ) &&				\
	       ( (address) <= CDD_60x_CFG_SPACE_END ) ) 				\
	)

#define	CDD_ADDR_IS_60x_VPD_FRS_SPACE( segment , address )				\
	(    (((segment) & CDD_60x_32BIT_SET_T_BIT  ) == CDD_60x_32BIT_SET_T_BIT )	\
	  && ( (((segment) & CDD_60x_MOD250_BUID    ) == CDD_60x_MOD250_BUID ) || 	\
	       (((segment) & CDD_60x_R2G_BUID       ) == CDD_60x_R2G_BUID )  )		\
	  && ( ( (address) >= CDD_60x_VPD_FRS_SPACE_BEGIN ) &&				\
	       ( (address) <= CDD_60x_VPD_FRS_SPACE_END ) ) 				\
	)

	 
#define CDD_ADDR_BUS_NUM( b )								\
	( ((b) & CDD_BUS_MASK ) >> CDD_BUS_OFFSET )	



	/*===================================================
	|
	| macros which operate on the CDD HEADER
	|
	|====================================================*/
				
#define CDD_command(cp) 						\
	 	/* cdd_command_attrs_t * */				\
		((cp)->p_cdd_command_attrs)

#define CDD_device(cp)							\
		/* cdd_device_attrs_t *   */ 				\
		((cp)->p_cdd_device_attrs)

#define CDD_procs(cp)   						\
		/* cdd_procs_t * 	  */ 				\
		((cp)->p_cdd_procs)

#define CDD_svcs(cp)    						\
		/* cdd_svcs_t * 	   */ 				\
		((cp)->p_cdd_svcs)

#define CDD_ext(cp)      						\
		/* cdd_exts_t *		   */				\
		((cp)->p_cdd_extensions)

#define	CDD_version(cp)							\
		/* ulong		    */				\
		((cp)->version)

 
	/*=================================================
	|
	| macros which operate on the CDD DEVICE ATTRIBUTES
	|
	|==================================================*/

#define CDD_ddf(cp)							\
		/* void *		   */				\
		(CDD_device(cp)->ddf_scratchpad)

#define CDD_ddf_len(cp)							\
		/* ulong		   */				\
		(CDD_device(cp)->ddf_len)

#define	CDD_slot(cp)							\
		/* uchar		*/				\
		(CDD_device(cp)->slot)

#define	CDD_busmem_seg(cp)						\
		/* ulong		*/				\
		(CDD_device(cp)->busmem_att)

#define CDD_iocc_seg(cp)						\
		/* ulong		   */				\
		(CDD_device(cp)->iocc_att)

#define CDD_busmem_base(cp,i)						\
		/* ulong		   */				\
		(CDD_device(cp)->busmem_hw_base[(i)])

#define CDD_busmem_len(cp,i)						\
		/* ulong		   */				\
		(CDD_device(cp)->address_space_length[(i)])

#define	CDD_iocc_base(cp)						\
		/* ulong		*/				\
		(CDD_device(cp)->iocc_addr_base)

#define	CDD_exception(cp)						\
		/* long			   */				\
		(CDD_device(cp)->exception_code)

/*
 * Extensions for 60x bus 
 */

#define CDD_60x_buid( cp, i )						\
		/* ulong */						\
		( CDD_device(cp)->buid_reg[ (i) ])
 
#define CDD_60x_int_server( cp )						\
		/* short */						\
		( CDD_device(cp)->int_server)
 
#define CDD_60x_segment( cp )						\
		/* ulong */						\
		( CDD_device(cp)->segment)
 
 

	/*=================================================
	|
	| macros which operate on the CDD COMMAND ATTRIBUTES
	|
	|==================================================*/

#define CDD_cmd(cp) 							\
		/* ulong		   */				\
		(CDD_command(cp)->command)

#define CDD_DataIn(cp) 							\
		/* void *		   */				\
		(CDD_command(cp)->pDataIn)

#define CDD_DataOut(cp) 						\
		/* void *		   */				\
		(CDD_command(cp)->pDataOut)

#define CDD_len_in(cp)							\
		/* ulong		   */				\
		(CDD_command(cp)->length_in)

#define CDD_len_out(cp)							\
		/* ulong		   */				\
		(CDD_command(cp)->length_out)

#define CDD_rc(cp)     							\
		/* int			   */				\
		(CDD_command(cp)->rc_out)

#define	CDD_component_loc(cp)						\
		/* char *						\
		|  cdd_header_t *	cp				\
		|---------------------------------------*/		\
		(CDD_command(cp)->component_loc)

#define	CDD_product_srn(cp)						\
		/* char *						\
		|  cdd_header_t *	cp				\
		|---------------------------------------*/		\
		(CDD_command(cp)->product_SRN)


	/*=================================================
	|
	| macros which operate on the CDD PROCEDURES FUNCTIONS
	|
	|==================================================*/

#define CDD_set_POS(cp)      						\
		/* int			*/				\
		(*( CDD_procs(cp)->entry_point  ))(cp)

#define CDD_load_ucode(cp)    						\
		/* int			*/				\
		(*( CDD_procs(cp)->entry_point ))(cp)

#define CDD_init(cp)          						\
		/* int			*/				\
		(*( CDD_procs(cp)->entry_point ))(cp)

#define CDD_blit(cp)          						\
		/* int			*/				\
		(*( CDD_procs(cp)->entry_point ))(cp)

#define CDD_qvpd(cp)          						\
		/* int			*/				\
		(*( CDD_procs(cp)->entry_point ))(cp)

#define CDD_enable(cp)         						\
		/* int			*/				\
		(*( CDD_procs(cp)->entry_point ))(cp)

#define CDD_disable(cp)        						\
		/* int			*/				\
		(*( CDD_procs(cp)->entry_point ))(cp)



	/*=================================================
	|
	| macros which operate on the CDD SERVICES FUNCTIONS
	|
	|==================================================*/

#define CDD_lockl(cp,lock_word)						\
		/* int  		*/				\
		(*( CDD_svcs(cp)->lockl ))((lock_word))

#define CDD_unlockl(cp,lock_word)					\
		/* void  		*/				\
		(*( CDD_svcs(cp)->unlockl ))((lock_word))

#define CDD_i_disable(cp,new_priority)					\
		/* int  		*/				\
		(*( CDD_svcs(cp)->i_disable ))(new_priority)

#define CDD_i_enable(cp,priority)					\
		/* int			*/				\
		(*( CDD_svcs(cp)->i_enable ))(priority)

#define	CDD_strncmp( cp, s1, s2, n )					\
		/* int			*/				\
		(*( CDD_svcs(cp)->strncmp ))( (s1),(s2),(n) )

#define CDD_busgetl(cp,seg,ptr,pval)					\
	 	/* int 			*/				\
		(*( CDD_svcs(cp)->bus_get_l ))((seg),(ptr),(pval))


#define CDD_busgets(cp,seg,ptr,pval)					\
		/* int  		*/				\
		(*( CDD_svcs(cp)->bus_get_s ))((seg),(ptr),(pval))


#define CDD_busgetc(cp,seg,ptr,pval)					\
		/* int  		*/				\
		(*( CDD_svcs(cp)->bus_get_c ))((seg),(ptr),(pval))


#define CDD_busputl(cp,seg,ptr,val)					\
		/* int  		*/				\
		(*( CDD_svcs(cp)->bus_put_l ))((seg),(ptr),(val))


#define CDD_busputs(cp,seg,ptr,val)					\
		/* int  		*/				\
		(*( CDD_svcs(cp)->bus_put_s ))((seg),(ptr),(val))


#define CDD_busputc(cp,seg,ptr,val)					\
		/* int  		*/				\
		(*( CDD_svcs(cp)->bus_put_c ))((seg),(ptr),(val))


#define CDD_buscpy(cp,seg,src,dest,len,dir)				\
		/* int  		*/				\
		(*( CDD_svcs(cp)->bus_cpy ))((seg),(src),(dest),(len),(dir))

 

	/*=================================================
	|
	| macros which operate on the CDD EXTENSIONS HEADER
	|
	|==================================================*/

#define CDD_first_ext(cp)						\
		/* cdd_exts_t *		*/				\
      		CDD_ext(cp)

#define CDD_next_ext(ep)       						\
		/* cdd_exts_t *		*/				\
		((ep) -> p_NextExt)

#define CDD_prev_ext(ep)       						\
		/* cdd_exts_t *		*/				\
		((ep) -> p_PrevExt)

#define CDD_ext_name(ep)						\
		/* char *		*/				\
		((ep) -> name )

#define CDD_ext_attr(ep)						\
		/* void *		*/				\
		((ep) -> p_Attr )

#define CDD_ext_proc(ep)						\
		/* void *		*/				\
		((ep) -> p_Proc )



/*======================================================================
|
|	The following define macros that operate on the CDD VPD structure
|
|=======================================================================*/

#define CDD_VPD_num_valid_units( vp )					\
		/* ulong		*/				\
		( (vp) -> num_valid_units )

#define	CDD_VPD_buf_length( vp, i )					\
		/* ulong		*/				\
		( (vp) -> per_unit_data[ i ].buf_length )

#define CDD_VPD_buffer( vp , i )					\
		/* char *		*/				\
		( (vp) -> per_unit_data[ i ].buffer )

#define CDD_VPD_min_ucode_lvl( vp , i )					\
		/* ulong		*/				\
		( (vp) -> per_unit_data[ i ].min_ucode_lvl )

#define	CDD_VPD_dev_version( vp , i  )					\
		/* char *		*/				\
		( (vp) -> per_unit_data[ i ].dev_version )

#define CDD_VPD_dev_max_ucode_lvl( vp , i )				\
		/* ulong		*/				\
		( (vp) -> per_unit_data[ i ].dev_max_ucode_lvl )

#define CDD_VPD_dev_flags( vp, i )					\
		/* ulong		*/				\
		( (vp) -> per_unit_data[ i ].dev_flags)

#define CDD_VPD_dev_slot( vp , i )					\
		/* ulong		*/				\
		( (vp) -> per_unit_data[ i ].dev_slot_id )

#define CDD_VPD_dev_feature_id( vp , i )				\
		/* ulong		*/				\
		( (vp) -> per_unit_data[ i ].dev_feature_id )



	/*===============================================
	|
	| MACROS which operate on the CDD FLAGS FIELD
	|
	|================================================*/


#define	CDD_FLAGS_ROSTEST( flag )					\
		/* rvalue uchar						\
		|  ulong		flag				\
		|----------------------------------*/			\
		( ( (flag) & CDD_FLAG_ROSTEST_MASK) >> 4 )


	/*================================================
	|
	| macros which operate on the CDD INIT structures
	|
	|================================================*/


#define	CDD_INIT_flags( cmd )						\
		/* ulong 		*/				\
		( (cmd) -> cdd_flags )

#define CDD_INIT_architecture( cmd )						\
		/* ulong */						\
		( (cmd) -> architecture)
 
#define CDD_INIT_implementation( cmd )						\
		/* ulong */						\
		( (cmd) -> implementation)
 
#define	CDD_INIT_busmem_base( out , i )					\
		/* ulong		*/				\
		( (out) -> busmem_hw_base[ (i) ] )			\

#define	CDD_INIT_busmem_len( out, i )					\
		/* ulong 		*/				\
		( (out)->address_space_length[ (i) ] )

#define	CDD_INIT_busmem_seg( out )					\
		/* ulong		*/				\
		( (out)->busmem_att )

#define	CDD_INIT_iocc_seg( out )					\
		/* ulong		*/				\
		( (out)->iocc_att )

#define	CDD_INIT_iocc_base( out )					\
		/* ulong		*/				\
		( (out)->iocc_addr_base )

#define	CDD_INIT_max_w( out )						\
		/* ulong		*/				\
		( (out)->max_width )

#define	CDD_INIT_max_h( out )						\
		/* ulong		*/				\
		( (out)->max_height )

#define	CDD_INIT_max_a( out )						\
		/* ulong		*/				\
		( (out)->max_area )

#define	CDD_INIT_origin( out )						\
		/* ulong		*/				\
		( (out)->origin_type )

#define	CDD_INIT_intr_lvl( out )					\
		/* ulong		*/				\
		( (out)->dflt_int_level )

#define	CDD_INIT_intr_pri( out )					\
		/* ulong		*/				\
		( (out)->dflt_int_priority )

#define	CDD_INIT_dma_arb( out )						\
		/* ulong		*/				\
		( (out)->dflt_dma_arb_level )

#define	CDD_INIT_has_intr( out )					\
		/* uchar		*/				\
		( (out)->has_intr_handler )

#define	CDD_INIT_runs_polled( out )					\
		/* uchar		*/				\
		( (out)->can_run_in_polled_mode )

#define	CDD_INIT_product_name( out )					\
		/* char *						\
		|  cdd_init_out_t *	out				\
		|------------------------------------*/			\
		( (out)->product_name )
/*
 * Extensions for 60x bus
 */


#define CDD_INIT_buid( p_out, i )						\
		( ( p_out )->buid_reg[ (i) ] )

#define CDD_INIT_int_server( p_out )					\
		( ( p_out )->int_server )

#define CDD_INIT_segment( p_out )						\
		( ( p_out )->segment )



	/*==========================================================
	|
	|	macros which operate on CDD SET POS structures
	|
	|===========================================================*/

#define	CDD_SET_POS_flags( cmd )					\
		/* ulong 		*/				\
		( (cmd)-> cdd_flags )

#define	CDD_SET_POS_use_polled_mode( cmd )				\
		/* uchar		*/				\
		( (cmd)->use_polled_mode)

#define	CDD_SET_POS_int_level( cmd )					\
		/* ulong		*/				\
		( (cmd)->int_level )

#define	CDD_SET_POS_int_priority( cmd )					\
		/* ulong		*/				\
		( (cmd)->int_priority )

#define	CDD_SET_POS_int_server( cmd )					\
		/* ulong		*/				\
		( (cmd)->int_server )

#define	CDD_SET_POS_dma_arb_level( cmd )				\
		/* ulong		*/				\
		( (cmd)->dma_arb_level )

		/*=========================
		| DEFECT 61512 removed the following
		| macros:
		|
		|	CDD_SET_POS_x_max
		|	CDD_SET_POS_x_mon
		|	CDD_SET_POS_y_max
		|	CDD_SET_POS_y_min
		| 
		| They are now in the LOAD UCODE area
		|=========================*/

	/*=================================================
	|
	| macros which operate on the CDD LOAD UCODE  COMMAND 
	|
	|==================================================*/

#define	CDD_LOAD_UCODE_flags( cmd )					\
		/* ulong 		*/				\
		( (cmd)-> cdd_flags )

#define	CDD_LOAD_UCODE_len(p)						\
		/* size_t		*/				\
		( (p) -> length_of_ucode )	

#define	CDD_LOAD_UCODE_ptr(p)						\
		/* char *		*/				\
		( (p) -> p_ucode )

#define CDD_LOAD_UCODE_use_busmem_rom(p)				\
		/* char *		*/				\
		( (p) -> use_R2_rom )

		/*================================
		| DEFECT 61512 added the x and y 
		| out structures
		|================================*/

#define	CDD_LOAD_UCODE_x_max( out )					\
		/* ulong						\
		|  cdd_load_ucode_out_t *	out			\
		|-------------------------------------------*/		\
		( (out)->x_max )


#define	CDD_LOAD_UCODE_x_min( out )					\
		/* ulong						\
		|  cdd_load_ucode_out_t *	out			\
		|-------------------------------------------*/		\
		( (out)->x_min )


#define	CDD_LOAD_UCODE_y_max( out )					\
		/* ulong						\
		|  cdd_load_ucode_out_t *	out			\
		|-------------------------------------------*/		\
		( (out)->y_max )


#define	CDD_LOAD_UCODE_y_min( out )					\
		/* ulong						\
		|  cdd_load_ucode_out_t *	out			\
		|-------------------------------------------*/		\
		( (out)->y_min )



	/*=================================================
	|
	| macros which operate on the CDD QUERY VPD COMMAND 
	|
	|==================================================*/

#define CDD_QVPD_slot(qp)						\
		/* ulong		*/				\
		( (qp) -> start_slot )


	/*=================================================
	|
	| macros which operate on the CDD BLIT COMMAND 
	|
	|==================================================*/

#define CDD_BLIT_pixel_ptr(bp)						\
		/* ulong *		*/				\
		( (bp) -> p_PixelData )

#define CDD_BLIT_pixel_len(bp)						\
		/* ushort		*/				\
		( (bp) -> PixelDataLength )

#define CDD_BLIT_x(bp)							\
		/* short		*/				\
		( (bp) -> dest_x )

#define CDD_BLIT_y(bp)							\
		/* short		*/				\
		( (bp) -> dest_y )

#define CDD_BLIT_w(bp)							\
		/* ushort		*/				\
		( (bp) -> width )

#define CDD_BLIT_h(bp)							\
		/* ushort		*/				\
		( (bp) -> height )

#define	CDD_BLIT_format(bp)						\
		/* ulong		*/				\
		( (bp) -> PixelFormat )
 


#endif        /*__H_CDD_MACROS */
