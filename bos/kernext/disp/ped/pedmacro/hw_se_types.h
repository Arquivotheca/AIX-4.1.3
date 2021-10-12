/* @(#)48	1.4  src/bos/kernext/disp/ped/pedmacro/hw_se_types.h, pedmacro, bos411, 9428A410j 6/18/93 11:52:18 */
/*
 *   COMPONENT_NAME: PEDMACRO
 *
 *   FUNCTIONS: 
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

#ifndef	_H_MID_HW_SE_TYPES
#define	_H_MID_HW_SE_TYPES

/*********************************************************************************
   PEDERNALES CornerRectangle structure			       
     This rectangle is specified with an upper left (ul) point (x, y) and a 
     lower right point.  The (X) coordinates are relative to an upper left origin. 
 *********************************************************************************/
typedef	struct _MIDCornerRectangle
{
		short  rulx, ruly;   /*	rectangle extent (upper	left  x,y) */
		short  rlrx, rlry;   /*	rectangle extent (lower	right x,y) */
} MIDCornerRectangle, *pMIDCornerRectangle;


/*********************************************************************************
   PEDERNALES 3D Rectangle structure			       
     This rectangle is specified with a lower left (ll) point (x, y) and a 
     upper right point.  The (X) coordinates are relative to a lower left origin. 
 *********************************************************************************/
typedef	struct _MID3DRectangle
{
		short  rllx, rlly;   /*	rectangle extent (lower	left  x,y) */
		short  rurx, rury;   /*	rectangle extent (upper	right x,y) */
} MID3DRectangle, *pMID3DRectangle;


/***************************************************************/
/* PEDERNALES DimensionRectangle structure			  */
/***************************************************************/
typedef	struct _MIDDimensionRectangle
{
		short  rulx, ruly;   /*	rectangle extent (upper	left  x,y)  */
		short  rwth, rht;    /*	rectangle dimesions (width, height) */
} MIDDimensionRectangle, *pMIDDimensionRectangle;

#endif	/* _H_MID_HW_SE_TYPES */
