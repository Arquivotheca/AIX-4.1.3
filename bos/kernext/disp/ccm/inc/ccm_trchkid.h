/* @(#)60	1.2  src/bos/kernext/disp/ccm/inc/ccm_trchkid.h, dispccm, bos411, 9428A410j 3/2/93 16:29:23 */
/*
 *   COMPONENT_NAME: SYSXDISPCCM
 *
 *   FUNCTIONS: 
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*****************************************************************************
 ********************** MODULE HEADER TOP ************************************
 *****************************************************************************
 ---------------------------------------------------------------------------

 PURPOSE:	Defines some macros and trace aids useful for the CCM
		VDD.

 
 INPUTS:  	n/a

 OUTPUTS: 	n/a

 FUNCTIONS/SERVICES CALLED:	n/a 

 DATA:		

 *****************************************************************************
 ********************** MODULE HEADER BOTTOM *********************************
 *****************************************************************************/

#ifndef __H_CCM_TRCHKID
#define __H_CCM_TRCHKID


#define  hkwd_DISPLAY_VTSS_VDD_CLOSE         ( 0x131 << 4 )
#define  hkwd_DISPLAY_VTSS_VDD_CONFIG        ( 0x132 << 4 )
#define  hkwd_DISPLAY_VTSS_VDD_IOCTL         ( 0x133 << 4 )
#define  hkwd_DISPLAY_VTSS_VDD_OPEN          ( 0x134 << 4 )
#define	 hkwd_DISPLAY_VTSS_VDD_INTERRUPT     ( 0x135 << 4 )

#define	 hkwd_DISPLAY_VTSS_CDD_ENTRY_KMOD    ( 0x151 << 4 )
#define	 hkwd_DISPLAY_VTSS_CDD_INTERRUPT     ( 0x152 << 4 )
#define  hkwd_DISPLAY_VTSS_CDD_EXTENSION     ( 0x153 << 4 )

	/*-------------------------------------------------
	|  FUNCTION DEFINITIONS FOR VDD hkwd_'s
	|
	|  FUNCTION 0 must always be MAIN
	|  FUNCTION 7 is reserved for ERROR
	|--------------------------------------------------*/

#define  	hkwd_func_DISPLAY_VTSS_VDD_ACT_MAIN    		0
#define  	hkwd_func_DISPLAY_VTSS_VDD_ACT_ERROR		7

#define  	hkwd_func_DISPLAY_VTSS_VDD_CFL_MAIN   		0
#define  	hkwd_func_DISPLAY_VTSS_VDD_CFL_ERROR		7

#define  	hkwd_func_DISPLAY_VTSS_VDD_CLR_MAIN 		0
#define  	hkwd_func_DISPLAY_VTSS_VDD_CLR_ERROR		7

#define  	hkwd_func_DISPLAY_VTSS_VDD_CPL_MAIN 		0
#define  	hkwd_func_DISPLAY_VTSS_VDD_CPL_ERROR		7

#define  	hkwd_func_DISPLAY_VTSS_VDD_DACT_MAIN		0
#define  	hkwd_func_DISPLAY_VTSS_VDD_DACT_ERROR		7

#define  	hkwd_func_DISPLAY_VTSS_VDD_DEFC_MAIN      	0
#define  	hkwd_func_DISPLAY_VTSS_VDD_DEFC_ERROR		7

#define  	hkwd_func_DISPLAY_VTSS_VDD_TERM_MAIN     	0
#define  	hkwd_func_DISPLAY_VTSS_VDD_TERM_ERROR		7

#define  	hkwd_func_DISPLAY_VTSS_VDD_INIT_MAIN    	0
#define  	hkwd_func_DISPLAY_VTSS_VDD_INIT_ERROR		7

#define  	hkwd_func_DISPLAY_VTSS_VDD_MOVC_MAIN   		0
#define  	hkwd_func_DISPLAY_VTSS_VDD_MOVC_ERROR		7

#define  	hkwd_func_DISPLAY_VTSS_VDD_RDS_MAIN        	0
#define  	hkwd_func_DISPLAY_VTSS_VDD_RDS_ERROR		7

#define  	hkwd_func_DISPLAY_VTSS_VDD_TEXT_MAIN      	0
#define  	hkwd_func_DISPLAY_VTSS_VDD_TEXT_ERROR		7

#define  	hkwd_func_DISPLAY_VTSS_VDD_SCR_MAIN      	0
#define  	hkwd_func_DISPLAY_VTSS_VDD_SCR_ERROR		7

#define  	hkwd_func_DISPLAY_VTSS_VDD_SETM_MAIN    	0
#define  	hkwd_func_DISPLAY_VTSS_VDD_SETM_ERROR		7

#define  	hkwd_func_DISPLAY_VTSS_VDD_STCT_MAIN   		0
#define  	hkwd_func_DISPLAY_VTSS_VDD_STCT_ERROR		7

#define		hkwd_func_DISPLAY_VTSS_VDD_CONFIG_MAIN		0
#define		hkwd_func_DISPLAY_VTSS_VDD_CONFIG_DATA		1
#define		hkwd_func_DISPLAY_VTSS_VDD_CONFIG_INIT		2
#define		hkwd_func_DISPLAY_VTSS_VDD_CONFIG_TERM		3
#define		hkwd_func_DISPLAY_VTSS_VDD_CONFIG_QVPD		4
#define		hkwd_func_DISPLAY_VTSS_VDD_CONFIG_ERROR		7

#define		hkwd_func_DISPLAY_VTSS_VDD_CLOSE_MAIN		0
#define		hkwd_func_DISPLAY_VTSS_VDD_CLOSE_ERROR		7

#define		hkwd_func_DISPLAY_VTSS_VDD_IOCTL_MAIN		0
#define		hkwd_func_DISPLAY_VTSS_VDD_IOCTL_ERROR		7

#define		hkwd_func_DISPLAY_VTSS_VDD_OPEN_MAIN		0
#define		hkwd_func_DISPLAY_VTSS_VDD_OPEN_ERROR		7

#define		hkwd_func_DISPLAY_VTSS_VDD_INTERRUPT_MAIN	0
#define		hkwd_func_DISPLAY_VTSS_VDD_INTERRUPT_ERROR	7

#define		hkwd_func_DISPLAY_VTSS_CDD_ENTRY_INIT		1
#define		hkwd_func_DISPLAY_VTSS_CDD_ENTRY_SET_POS	2
#define		hkwd_func_DISPLAY_VTSS_CDD_ENTRY_LOAD_UCODE	3
#define		hkwd_func_DISPLAY_VTSS_CDD_ENTRY_BLIT		4
#define		hkwd_func_DISPLAY_VTSS_CDD_ENTRY_QVPD		5

#endif		/* __H_CCM_TRCHKID */
