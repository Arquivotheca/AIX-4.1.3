/* @(#)11	1.3  src/bos/kernext/disp/ped/pedmacro/hw_HCRkern.h, pedmacro, bos411, 9428A410j 3/17/93 19:44:07 */
/*
 *   COMPONENT_NAME: PEDMACRO
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
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 ***************************************************************
 *
 *  PEDERNALES MACRO PROGRAMMING INTERFACE
 *
 ***************************************************************
 */

#ifndef	_H_MID_HW_HCR_KERN
#define	_H_MID_HW_HCR_KERN

/*
 *******************************************************
 * OPCODES FOR COMMANDS INTO THE HOST COMMO REGISTER
 *******************************************************
*/

	/*
	 *-----------------------------------------------
	 *  Based on Pedernales Software Interface Specification
	 *-----------------------------------------------
	 */

#define	DownloadComplete		0x9001
#define	SwitchContext			0x9002

/*
 ***************************************************************
 *  MACROS FOR PROGRAMMING TO THE HOST COMMO REGISTER
 ***************************************************************
 */

#define	MID_DOWNLOAD_COMPLETE					\
{								\
	MID_WR_REG( MID_K_COMMO_REG , DownloadComplete );	\
}


#define	MID_SWITCH_TO_CONTEXT( context_ID )			\
{								\
	MID_WR_REG( MID_K_COMMO_REG , 				\
		    ( SwitchContext | ( context_ID << 16 ))	\
		   );						\
}


#endif /* _H_MID_HW_HCR_KERN */

