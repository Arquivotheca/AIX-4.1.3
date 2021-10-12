/* @(#)14	1.3.1.1  src/bos/usr/lib/nls/loc/jim/jfep/imjim.h, libKJI, bos411, 9428A410j 7/23/92 02:46:42	*/
/*
 * COMPONENT_NAME :	(LIBKJI) Japanese Input Method (JIM)
 *
 * ORIGINS :		27 (IBM)
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _h_imjim_
#define _h_imjim_
/*---------------------------------*/
/* include im.h prior to this file */
/*---------------------------------*/

/*-----------------------------------------------------------------------*
* JIM unique indicator states
*-----------------------------------------------------------------------*/
#define JIM_SH_NULL        0
#define JIM_SH_MASK     0x07
#define JIM_SH_ALPHA    0x00
#define JIM_SH_KATA     0x01
#define JIM_SH_HIRA     0x02
#define JIM_SH_RKCMASK  0x08
#define JIM_SH_RKCOFF   0x00
#define JIM_SH_RKCON    0x08

/*-----------------------------------------------------------------------*
* JIM unique indicator switch
*-----------------------------------------------------------------------*/
#define JIM_WIDTH_FLAG  0x08      /* validate Half/Full-width change */
#define JIM_ALPHA_FLAG  0x400000  /* validate Alpha/Kana change */
#define JIM_RKC_FLAG    0x800000  /* validate Romaji-henkan change */

/*-----------------------------------------------------------------------*
*	JIM specific IMIoctl extension
*-----------------------------------------------------------------------*/
#define IM_SetString            (JIMIOC + 1)
#define IM_ChangeIndicator      (JIMIOC + 2)

#endif /* _h_imjim_ */
