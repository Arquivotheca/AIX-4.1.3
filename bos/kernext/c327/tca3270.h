/* @(#)32	1.3  src/bos/kernext/c327/tca3270.h, sysxc327, bos411, 9428A410j 6/15/90 18:17:34 */
#ifndef	_H_TCA3270
#define	_H_TCA3270

/*
 * COMPONENT_NAME: (SYSXC327) c327 tca 3270 constant defines
 *
 * FUNCTIONS:    N/A
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/*
** tca3270.h - tca/3270 constants and datatypes
*/

/* 3270 command codes */
#define cmdEWRT    0xF5 	 /* Erase/Write */
#define cmdEWRA    0x7E 	 /* Erase/Write Alternate */
#define cmdWRT	   0xF1 	 /* Write */
#define cmdEAU	   0x6F 	 /* Erase All Unprotected */
#define cmdWSF	   0xF3 	 /* Write Structured Field */
#define cmdRDBF    0xF2 	 /* Read Buffer */
#define cmdRDMD    0xF6 	 /* Read Modified */
#define cmdRDMA    0x6E 	 /* Read Modified all */
 
#define cmdCONT    0xFF 	 /* pseudo op - continuation */
/* The WCC byte */
#define wccRESET   0x40 	 /* Reset -
				    if don't support partitions and
				    command is EWRT or EWRA then reset
				    inbound reply mode to field
				    - otherwise see 3270 reference
				 */
#define wccALARM   0x04 	 /* sound the alarm */
#define wccKEYUN   0x02 	 /* unlock the keyboard */
#define wccREMDT   0x01 	 /* reset MDT bits */
 
/* 3270 orders */
#define ordSF	   0x1D 	/* Start Field */
#define ordSFE	   0x29 	/* Start Field Extended */
#define ordSBA	   0x11 	/* Set Buffer Address */
#define ordSA	   0x28 	/* Set Attribute */
#define ordMF	   0x2C 	/* Modify Field */
#define ordIC	   0x13 	/* Insert Cursor */
#define ordPT	   0x05 	/* Program Tab */
#define ordRA	   0x3C 	/* Repeat to Address */
#define ordEUA	   0x12 	/* Erase Unprotected to Address */
#define ordGE	   0x08 	/* Graphic Escape */
 
/* Field Attribute bits */
#define FAgc	0xC0	       /* Graphic Converter bits  */
#define FAprot	0x20	       /* protected */
#define FAnum	0x10	       /* numeric field */
#define FAautsk 0x30	       /* autoskip */
			  /* two bits control intensity/detectability */
#define FAnormi 0x00	       /* normal intensity     00xx */
#define FAlpd	0x04	       /* light pen detectable 01xx */
#define FAilpd	0x08	       /* intense and lpd      10xx */
#define FAinvis 0x0C	       /* invisible	       11xx */
#define FAresv	0x02	       /* reserved    */
#define FAmdt	0x01	       /* the MDT bit */
 
#define FAtlm	0x3C	       /*  prot,num,lpd,ilpd bit mask for table lkup*/
#define FAtls	2	       /*  shift for table lookup */
 
/* some 3270 Aid codes */
#define No327aid 0x60
#define AC327entr 0x7D		/* enter */
#define AC327clr  0x6D		/* clear */

#endif	/* _H_TCA3270 */
