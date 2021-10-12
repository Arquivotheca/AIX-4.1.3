/* @(#)96	1.5.1.5  src/bos/kernext/disp/gem/inc/gem_rcmdef.h, sysxdispgem, bos411, 9428A410j 1/19/93 12:43:25 */
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		
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


/*;**********************************************************************/
/*;                                                                     */
/*; CHANGE HISTORY:                                                     */
/*; dm  09/07/89     created                                            */
/*; jg  09/13/89    fixed syntax error                           @1     */
/*; lw  09/14/89	Removed defs allocating storage, Renamed file   */
/*; MC  09/14/89    Added SLOT_SIZE defs			      @2*/
/*; lw  09/19/89    Moved domain constants to gem_ldat.h	        */
/*;**********************************************************************/

#define AHEAD 		TRUE
#define FIFO_TOP	FALSE

/************************************************************************/
/* Context slot defines							*/
/************************************************************************/
#define TRVDSVSIZ	2048
