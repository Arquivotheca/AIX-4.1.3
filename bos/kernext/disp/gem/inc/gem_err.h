/* @(#)87       1.5.1.6  src/bos/kernext/disp/gem/inc/gem_err.h, sysxdispgem, bos411, 9428A410j 1/19/93 12:42:17 */
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
/*;CHANGE HISTORY                                                       */
/*;LW 09/12/89 Created                                                  */
/*;DM 09/18/89 Added more return codes                                  */
/*;DM 09/21/89 Added more return codes                                  */
/*;                                                                     */
/*;**********************************************************************/

#define GM_NOMEMORY	-1	/* rmalloc failed			*/
#define GM_NOSLOTS	-2	/* no slots available			*/
#define GM_BAD_RCXTYPE	-3	/* invalid rcx type			*/
#define GM_BAD_DDF_CMD	-4	/* invalid dev dep fun opcode		*/
#define GM_NOT_GP	-5	/* calling process not a graphics proc	*/
#define GM_FUFAIL       -6      /* FUWORD of fifo index failed          */
#define GM_MEMCPY       -7      /* failed copy to/from adapter of cxt   */
#define GM_PINNED       -8      /* Tried to mvcxton/off a pinned cxt    */
#define GM_ONADAPT      -9      /* Tried to mvcxton/off a cxt already off*/
#define GM_INKERNEL     -10     /* Tried to mvcxton a cxt not in kernel */

#define GM_DOMAIN       -11     /* domain number not appropriate      @1*/
#define GM_PIN_FAIL     -12     /* pin failed                         @2*/
#define GM_MAX_3D       -13     /* maximum 3d procs allowed             */
#define GM_NO_REAL      -14     /* can't find real WG, WA, or RCX       */
