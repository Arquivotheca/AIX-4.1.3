/* @(#)14	1.3  src/bos/kernext/disp/ccm/inc/cdd_intr.h, dispccm, bos411, 9428A410j 7/5/94 11:33:54 */
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

/******************************************************************************

 PURPOSE: Defines the standard data structures used by the 
	  Video ROM Scan (CDD) interrupt handlers.

	  This file assumes that "cdd.h" was previously included.

          The CDD is a small set of routines that provide a basic BLT
          interface into the adapter.  The only code which uses these
          routines is the CCM VDD code.

	Instances of the following CDD structures need to be pinned 
	because they will be identified in the CDD interrupt header
	structure:

		cdd_svcs_t
		cdd_device_attrs_t
		hardware specific interrupt ddf area

 
 INPUTS:  n/a

 OUTPUTS: n/a

 DATA:    A set of typedefs for the CDD functions:

 *************************************************************************/

#ifndef __H_CDD_INTR
#define __H_CDD_INTR




/*=======================================================================
 |                                                                      |
 | defined symbols and macros                                           |
 |                                                                      |
 | Note: Where possible, we will use ENUM rather than DEFINE to set	|
 | 	 the value of constants.  This is the preferred manner of 	|
 |	 coding and it certainly helps when debugging.			|
 |                                                                      |
 |======================================================================*/

enum
{
	/*-------------------------------------------------
	|  Symbols relating to the Micro Channel bus and
	|  to the segment register contents of the RS/6000
	|  architecture
	--------------------------------------------------*/

	CDD_INTR_DDF_STANDARD_SIZE	= (int) 4096,	/* one page */

	/*--- this is here just to prevent syntax errors of having
	      one too many commas in the list		---*/
	__CDD_LAST_1			= (int) 0 

};



/*=======================================================================
 |                                                                      |
 | cdd_intr_dev_attrs_t							|
 |                                                                      |
 | The following device attributes structure contains data used by the  |
 | CDD interrupt handler.  This device attributes structure should be	|
 | pinned.  All of the structures addressed by this structure should	|
 | also be pinned.							|
 |                                                                      |
 |======================================================================*/

typedef struct
{
	
/* Pointer to the CDD interrupt handler ddf scratchpad .  This
 * data is only used by the interrupt handler and by the other
 * cdd routines inside i_enable/i_disable brackets.  This data
 * must be pinned.
 */

        void *          ddf_intr_scratchpad;
        size_t          ddf_intr_len;

/* These hold the contents of the segment registers */
        ulong           busmem_att;
        ulong           iocc_att;
	ulong		slot;		/* which IOCC bus slot */

/* exception code returned by the bus services  */
        long            exception_code;

/* Base address and length of each address space on the adapter */
        ulong           busmem_hw_base[CDD_MAX_BUSMEM_RANGES];
        size_t          address_space_length[CDD_MAX_BUSMEM_RANGES];
        ulong           iocc_addr_base;

/* These are generally not used, but are required for completeness
   when the target display adapter requires an interrupt handler        */
        ulong           int_level;
        ulong           dma_arb_level;


} cdd_intr_dev_attrs_t;



/*=======================================================================
 |                                                                      |
 | cdd_intr_head_t							|
 |                                                                      |
 | This data structure is the only parameter passed into the CDD 	|
 | interrupt handler by the CCM device driver SLIH.			|
 | All data and procedures referenced by the interrupt handler		|
 | should be pinned.							|
 |                                                                      |
 |======================================================================*/


typedef struct 
{

    cdd_intr_dev_attrs_t *	p_cdd_intr_dev_attrs;
    cdd_svcs_t 		*	p_cdd_intr_svcs;
    cdd_exts_t		*	p_cdd_intr_extensions; 
    int				intr_rc;
    ulong	     		intr_version;
    void		*	p_iplrom_vrs_scratchpad;

} cdd_intr_head_t;   




#endif   /* of test for __H_CDD_INTR   */





