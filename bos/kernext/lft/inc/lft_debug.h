/* @(#)81	1.4  src/bos/kernext/lft/inc/lft_debug.h, lftdd, bos411, 9435D411a 9/1/94 19:22:43 */
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: none
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

/*
    Module ID's
*/

#define hkwd_GS_lftconfig	0x100
#define hkwd_GS_lftinit		0x120
#define hkwd_GS_lftterm		0x140
#define hkwd_GS_lftfonts	0x160
#define hkwd_GS_lft_fonts_init	0x161
#define hkwd_GS_load_font	0x162
#define hkwd_GS_lft_fonts_term	0x163
#define hkwd_GS_lftswkbd	0x170

/*
   Streams
*/

#define hkwd_GS_lftopen         0x200
#define hkwd_GS_lftclose        0x201
#define hkwd_GS_lft_streams_init 0x202
#define hkwd_GS_lft_streams_term 0x203
#define hkwd_GS_lftout          0x204
#define hkwd_GS_vtmupd          0x205
#define hkwd_GS_vtmupd3         0x206
#define hkwd_GS_lftKiSak        0x207
#define hkwd_GS_lftKiCb         0x208
#define hkwd_GS_lftKiOffl       0x209
#define hkwd_GS_lftKiInit       0x210
#define hkwd_GS_lftKiTerm       0x211
#define hkwd_GS_lftst           0x212

/*
   Font kernel Process (fkproc)
*/
#define hkwd_GS_fkproc		0x300
#define hkwd_GS_create_fkproc	0x301
#define hkwd_GS_kill_fkproc	0x302
#define hkwd_GS_pin_font	0x303
#define hkwd_GS_get_font_addr	0x304
#define hkwd_GS_fkproc_attach_shm 0x305
#define hkwd_GS_fkproc_detach_shm 0x306
#define hkwd_GS_pin_part_font 	0x307
#define hkwd_GS_fkproc_dev_dep_fun 0x308

#define hkwd_GS_fsqueue		0x320
#define hkwd_GS_fsp_enq		0x321
#define hkwd_GS_fsp_deq		0x322

#define hkwd_GS_kernel_ftok	0x340


/*
 * Display Power Management - routine id's
 */

#define hkwd_GS_lft_pwrmgr	0x400
