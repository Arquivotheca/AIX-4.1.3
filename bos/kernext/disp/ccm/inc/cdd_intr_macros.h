/* @(#)16	1.3  src/bos/kernext/disp/ccm/inc/cdd_intr_macros.h, dispccm, bos411, 9428A410j 7/5/94 11:34:00 */
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

 PURPOSE:        Defines the macros which access various parts
                of the CDD interrupt handler data structures.
 
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

#ifndef __H_CDD_INTR_MACROS
#define __H_CDD_INTR_MACROS


	/*===================================================
	|
	| macros which operate on the CDD INTERRUPT HEADER
	|
	|====================================================*/
				
#define CDD_INTR_device(cp)						\
		/* cdd_intr_dev_attrs_t *   */ 				\
		((cp)->p_cdd_intr_dev_attrs)

#define CDD_INTR_svcs(cp)    						\
		/* cdd_svcs_t * 	   */ 				\
		((cp)->p_cdd_intr_svcs)

#define CDD_INTR_ext(cp)      						\
		/* cdd_exts_t *		   */				\
		((cp)->p_cdd_intr_extensions)

#define	CDD_INTR_rc(cp)							\
		/* int			*/				\
		( (cp)->intr_rc )

#define	CDD_INTR_version(cp)						\
		/* ulong		    */				\
		((cp)->intr_version)

 
	/*=================================================
	|
	| macros which operate on the CDD INTERRUPT DEVICE ATTRIBUTES
	|
	|==================================================*/

#define CDD_INTR_ddf(cp)						\
		/* void *		   */				\
		(CDD_INTR_device(cp)->ddf_intr_scratchpad)

#define CDD_INTR_ddf_len(cp)						\
		/* size_t		   */				\
		(CDD_INTR_device(cp)->ddf_intr_len)

#define	CDD_INTR_busmem_seg(cp)						\
		/* ulong		*/				\
		(CDD_INTR_device(cp)->busmem_att)

#define CDD_INTR_slot(cp)						\
		/* ulong		   */				\
		(CDD_INTR_device(cp)->slot)

#define CDD_INTR_iocc_seg(cp)						\
		/* ulong		   */				\
		(CDD_INTR_device(cp)->iocc_att)

#define CDD_INTR_busmem_base(cp,i)					\
		/* ulong		   */				\
		(CDD_INTR_device(cp)->busmem_hw_base[(i)])

#define CDD_INTR_busmem_len(cp,i)					\
		/* size_t		   */				\
		(CDD_INTR_device(cp)->address_space_length[(i)])

#define	CDD_INTR_iocc_base(cp)						\
		/* ulong		*/				\
		(CDD_INTR_device(cp)->iocc_addr_base)

#define	CDD_INTR_exception(cp)						\
		/* long			   */				\
		(CDD_INTR_device(cp)->exception_code)


	/*=================================================
	|
	| macros which operate on the CDD INTERRUPT SERVICES FUNCTIONS
	|
	|==================================================*/

#define CDD_INTR_lockl(cp,lock_word)					\
		/* int  		*/				\
		(*( CDD_INTR_svcs(cp)->lockl ))((lock_word))

#define CDD_INTR_unlockl(cp,lock_word)					\
		/* void  		*/				\
		(*( CDD_INTR_svcs(cp)->unlockl ))((lock_word))

#define CDD_INTR_i_disable(cp,new_priority)				\
		/* int  		*/				\
		(*( CDD_INTR_svcs(cp)->i_disable ))((new_priority))

#define CDD_INTR_i_enable(cp,priority)					\
		/* int			*/				\
		(*( CDD_INTR_svcs(cp)->i_enable ))((priority))

#define	CDD_INTR_strncmp( cp, s1, s2, n )	

#define CDD_INTR_busgetl(cp,seg,ptr,pval)				\
	 	/* int 			*/				\
		(*( CDD_INTR_svcs(cp)->bus_get_l ))((seg),(ptr),(pval))

#define CDD_INTR_busgets(cp,seg,ptr,pval)				\
		/* int  		*/				\
		(*( CDD_INTR_svcs(cp)->bus_get_s ))((seg),(ptr),(pval))

#define CDD_INTR_busgetc(cp,seg,ptr,pval)				\
		/* int  		*/				\
		(*( CDD_INTR_svcs(cp)->bus_get_c ))((seg),(ptr),(pval))

#define CDD_INTR_busputl(cp,seg,ptr,val)				\
		/* int  		*/				\
		(*( CDD_INTR_svcs(cp)->bus_put_l ))((seg),(ptr),(val))


#define CDD_INTR_busputs(cp,seg,ptr,val)				\
		/* int  		*/				\
		(*( CDD_INTR_svcs(cp)->bus_put_s ))((seg),(ptr),(val))


#define CDD_INTR_busputc(cp,seg,ptr,val)				\
		/* int  		*/				\
		(*( CDD_INTR_svcs(cp)->bus_put_c ))((seg),(ptr),(val))


#define CDD_INTR_buscpy(cp,src,dest,len,dir)				\
		/* int  		*/				\
		(*( CDD_INTR_svcs(cp)->bus_cpy ))((src),(dest),(len),(dir))




#endif        /*__H_CDD_INTR_MACROS */
