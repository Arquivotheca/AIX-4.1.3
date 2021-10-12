/* @(#)25	1.9.1.5  src/bos/kernext/disp/gem/inc/gem_mac.h, sysxdispgem, bos411, 9428A410j 1/19/93 12:43:15 */
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		GET_ULPEL_XY
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*;*********************************************************************/
/*;CHANGE HISTORY                                                      */
/*;                                                                    */
/*;MPC  08/24/89   Removed last back-slash ("\") from macro          @1*/
/*;                                                                    */
/***********************************************************************/
#define GET_ULPEL_XY(row,col,st_ulpel)                                  \
{                                                                       \
  st_ulpel.x_ul = (ld->Vttenv.char_box.width * (col - 1)) +             \
					  ld->Vttenv.scr_pos.pel_x;     \
  st_ulpel.y_ul = (ld->Vttenv.char_box.height *                         \
			((ld->Vttenv.ps_size.ht - row) + 1)) +          \
				   ld->Vttenv.scr_pos.pel_y;            \
}                                                                  /*@1*/
