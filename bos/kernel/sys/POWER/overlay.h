/* @(#)30	1.5.1.21  src/bos/kernel/sys/POWER/overlay.h, sysml, bos412, 9445C412a 10/25/94 11:45:55 */

#ifndef _H_OVERLAY
#define _H_OVERLAY
/*
 * COMPONENT_NAME: (SYSML) Kernel Machine Language
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

/* WARNING: Do not change this file without updating ml/POWER/overlay.m4. 
 *	ml/POWER/sysoverlay.m4 may also need updating.
 */

#define MC_FLIH_ADDR	0x0200		/* machine check vector */
#define MC_FLIH_SIZE	0x0028

#define DS_FLIH_ADDR	0x0300		/* DSI vector */
#define DS_FLIH_SIZE	0x000c

#define IS_FLIH_ADDR	0x0400		/* ISI vector */
#define IS_FLIH_SIZE	0x000c

#define AL_FLIH_ADDR	0x0600		/* alignment check handler */
#define AL_FLIH_SIZE	0x0020

#define SC_PPC_ADDR	0x0c00		/* Power SC front end */
#define SC_PPC_SIZE	0x0014

#define DSE_FLIH_ADDR	0x0a00		/* Direct Store Error */
#define DSE_FLIH_SIZE	0x0004

#define TLB_FLIH_IFM_ADDR  0x1000       /* 603 TLB reload handler */
#define TLB_FLIH_DRM_ADDR  0x1100
#define TLB_FLIH_DWM_ADDR  0x1200
#define TLB_FLIH_SIZE      0x0100

#define CS_PWR_ADDR	0x1020		/* Power CS fast SVC */
#define CS_PWR_SIZE	0x0020

#define SC_PWR_ADDR	0x1fe0		/* Power PC SC front end */
#define SC_PWR_SIZE	0x0004

/* addresses of millicode routines required by PPC ABI.  Do not change
 */
#define MULH_ADDR	0x3100
#define MULH_SIZE	0x0080

#define MULL_ADDR	0x3180
#define MULL_SIZE	0x0080

#define DIVSS_ADDR	0x3200
#define DIVSS_SIZE	0x0080

#define DIVUS_ADDR	0x3280
#define DIVUS_SIZE	0x0080

#define QUOSS_ADDR	0x3300
#define	QUOSS_SIZE	0x0080

#define QUOUS_ADDR	0x3380
#define QUOUS_SIZE	0x0080

/*
 * addresses of atomic lock primitives required by PPC ABI.
 * Do not change.
 */

#define _CLEAR_LOCK_ADDR	0x3400
#define _CLEAR_LOCK_SIZE	0x0020

#define _CHECK_LOCK_ADDR	0x3420
#define _CHECK_LOCK_SIZE	0x0040

/* Overlay of machine dependent cs() function
 */
#define	CS_ADDR		0x3460
#define CS_SIZE		0x0040





/*
 * area reserved for future use.  will have user read access.
 */
#define NONPRIV_RESERV_ADDR	0x8000
#define NONPRIV_RESERV_SIZE	PSIZE

/*
 * i_enable/i_disable are aligned to a 256 byte boundry for better
 * cache affinity
 */

/* i_disable/disable_lock go together now--do not modify one without the other! */
#define I_DISABLE_ADDR		0x9000
#define I_DISABLE_SIZE		0x0004
#define DISABLE_LOCK_ADDR	0x9004
#define DISABLE_LOCK_SIZE	0x02FC

/* i_enable/unlock_enable go together now--do not modify one without the other! */
#define I_ENABLE_ADDR		0x9300
#define I_ENABLE_SIZE		0x0004
#define UNLOCK_ENABLE_ADDR	0x9304
#define UNLOCK_ENABLE_SIZE	0x01FC

#define SIMPLE_LOCK_ADDR	0x9500
#define SIMPLE_LOCK_SIZE	0x0400

#define SIMPLE_UNLOCK_ADDR	0x9900
#define SIMPLE_UNLOCK_SIZE	0x0200

#define FETCH_AND_ADD_ADDR	0x9B00
#define FETCH_AND_ADD_SIZE	0x0060

/* The following are overlay addresses for routines that can be called
 * directly from base device drivers.  Do not change the address of these
 * functions.  If it becomes a requirement to change the address wait for
 * a major release.  Changing these address breaks binary compatibility with
 * device drivers.
 */

#define BUSGETC_ADDR	0xA000
#define BUSGETC_SIZE	0X0060

#define BUSGETS_ADDR	0xA060
#define BUSGETS_SIZE	0x0060

#define BUSGETSR_ADDR	0xA0c0
#define BUSGETSR_SIZE	0x0060

#define	BUSGETL_ADDR	0xA120
#define BUSGETL_SIZE	0x0060

#define BUSGETLR_ADDR	0xA180
#define BUSGETLR_SIZE	0x0060

#define BUSPUTC_ADDR	0xA1e0
#define BUSPUTC_SIZE	0x0060

#define BUSPUTS_ADDR	0xA240
#define BUSPUTS_SIZE	0x0060

#define BUSPUTL_ADDR	0xA2a0
#define BUSPUTL_SIZE	0x0060

#define BUSPUTSR_ADDR	0xA300
#define BUSPUTSR_SIZE	0x0060

#define BUSPUTLR_ADDR	0xA360
#define BUSPUTLR_SIZE	0x0060

#define BUSCPY_ADDR	0xA3c0
#define BUSCPY_SIZE	0x00a0

/*
 * Keep all read/write segment registers in the same cache
 * line (64 bytes).  These sizes are exact.  Do not change
 * these address without updating inline.h
 */
#define CHGSR_ADDR	0xA480
#define CHGSR_SIZE	0x0018

#define MTSR_ADDR	0xA498
#define MTSR_SIZE	0x0014

#define MFSR_ADDR	0xA4AC
#define MFSR_SIZE	0x000c

#define MFSRI_ADDR	0xA4B8
#define MFSRI_SIZE	0x0008

#define COMPARE_AND_SWAP_ADDR	0xA4C0
#define COMPARE_AND_SWAP_SIZE	0x0100

#define TEST_AND_SET_ADDR	0xA5C0
#define TEST_AND_SET_SIZE	0x0100

#define SIMPLE_LOCK_TRY_ADDR	0xA6C0
#define SIMPLE_LOCK_TRY_SIZE	0x0200

#define RSIMPLE_LOCK_ADDR	0xA8C0
#define RSIMPLE_LOCK_SIZE	0x0180

#define RSIMPLE_LOCK_TRY_ADDR	0xAA40
#define RSIMPLE_LOCK_TRY_SIZE	0x0180

#define RSIMPLE_UNLOCK_ADDR	0xABC0
#define RSIMPLE_UNLOCK_SIZE	0x0100

#define LOCKL_ADDR		0xACC0
#define LOCKL_SIZE		0x0200

#define FETCH_AND_OR_ADDR	0xAEC0
#define FETCH_AND_OR_SIZE	0x0060

#define FETCH_AND_NOP_ADDR	0xAF20
#define FETCH_AND_NOP_SIZE	0x0060

#define FETCH_AND_AND_ADDR	0xAF80
#define FETCH_AND_AND_SIZE	0x0060

#define FETCH_AND_ADD_H_ADDR	0xB000
#define FETCH_AND_ADD_H_SIZE	0x0100

#define UNLOCKL_ADDR		0xB100
#define UNLOCKL_SIZE		0x0200

#ifdef _POWER_MP
/* ppda/csa/curthread management
*/
#define MY_PPDA_ADDR		0xb400
#define MY_PPDA_SIZE		0x0008

#define MY_CSA_ADDR		0xb408
#define MY_CSA_SIZE		0x0020

#define GET_CURTHREAD_ADDR	0xb428
#define GET_CURTHREAD_SIZE	0x0020

#define SET_CSA_ADDR		0xb448
#define SET_CSA_SIZE		0x0018

#define SET_CURTHREAD_ADDR	0xb460
#define SET_CURTHREAD_SIZE	0x0010
#endif

#endif /* _H_OVERLAY */

