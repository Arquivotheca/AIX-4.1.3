/* @(#)70	1.4  src/bos/usr/ccs/lib/libc/POWER/fpxcp_local.h, libccnv, bos411, 9428A410j 5/13/93 08:54:54 */

#ifndef _H_FPXCP_LOC
#define _H_FPXCP_LOC

/*
 *   COMPONENT_NAME: LIBCCNV
 *
 *   FUNCTIONS: AND_PFSCRX
 *		OR_FPSCRX
 *		WRITE_FPSCRX
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


#include <fpxcp.h>
#include <fptrap.h>

/* 
* The Rios hardware does not support sticky status bits for invalid
* conversion (FP_INV_CVI) and invalid square root (FP_INV_SQRT).  These
* are required by IEEE 754, however, and thus are implemented in
* software.  However, since we are also implementing a software fpscr to
* to maintain sticky-bit history along with edge- sensitive interrupts,
* it is actually necessary to have TWO bits each for invalid convert and
* invalid square root.  One corresponds to the hardware, and should be
* true for an edge-sensitive interrupt ("logical hardware") and the
* other corresponds to the software fpscr ("logical software").  The
* masks given to the user in fpxcp.h correspond to the "logical
* hardware", and two extra bits in the software fpscr correspond
* to the "logical software".  Masks are give below for the "logical
* software" versions, as well as for shift distances between the
* hardware and software version.  Moving from hardware to software
* versions must always be a LEFT shift, or code breaks. 
* 
* Note that the only bits that are EVER valid in the software
* fpscr are the exception sticky bits.  To keep exception history,
* the exception sticky bits at any time consists of the 'or' of
* the hardware and software sticky bits.  The exception sticky
* bit(s) which caused the latest trap is computable as all enabled
* sticky bits in the hardware (or "logical hardware") fpscr.
*
* Finally, the routines always attempt to keep the sticky
* bit exception summary bit (FX) up to date in the hardware
* fpscr.  This is because the high order 4 bits of the fpscr
* are shadowed to the condition register, so the compiler can
* insert a branch if we get a sticky bit.
*/

#define FP_INV_SQRT_SOFT     ((fpflag_t) 0x00002000)
#define FP_INV_CVI_SOFT      ((fpflag_t) 0x00001000)
#define FP_INV_VXSOFT_SOFT   ((fpflag_t) 0x00004000)

#define SQRT_OFFSET 4
#define CVI_OFFSET  4
#define VXSOFT_OFFSET 4

/*
 * the following macro are defined to read, write,
 * or-update and and-update the software FPSCR.
 * They are defined as macros to isolate the actual
 * implementation of the software FPSCR as much as is
 * possible 
 */

#ifdef _NO_PROTO
extern fpstat_t _fp_fpscrx();
#else
extern fpstat_t _fp_fpscrx(int i, ...);
#endif

#define FP_FPSCRX_LD	0
#define FP_FPSCRX_ST	1
#define FP_FPSCRX_OR	2
#define FP_FPSCRX_AND	3

#define READ_FPSCRX 	(_fp_fpscrx(FP_FPSCRX_LD))
#define WRITE_FPSCRX(x)	(_fp_fpscrx(FP_FPSCRX_ST, x))
#define OR_FPSCRX(x) 	(_fp_fpscrx(FP_FPSCRX_OR, x))
#define AND_FPSCRX(x) 	(_fp_fpscrx(FP_FPSCRX_AND, x))

/* some other misc. groups of exceptions used in 
 * the fp exceptions code.
 */

#define ALL_HW_STIKYBITS (FP_ALL_XCP)

#endif /* _H_FPXCP_LOC */
