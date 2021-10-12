/* @(#)83	1.2  src/bos/kernext/lft/inc/lft.h, lftdd, bos411, 9428A410j 5/20/94 11:23:48 */
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

/* ------------------------------------------------------------------------ *
 * The lft_struct anchors all of the other lft data structures.  A pointer  *
 * to this data structure is stored as the 'dsd_ptr' in the device switch   *
 * table entry for the lft.						    *
 * ------------------------------------------------------------------------ */

#define	SUCCESS 0
#define FILE_NAME_LEN 255
#define LFTNUMFONTS 8
#define APP_IS_DIAG 0x1
#define SCR_HEIGHT    25
#define SCR_WIDTH     80


#include <fkproc.h>
#include <lft_swkbd.h>
#include <vt.h>
#include <lft_dds.h>
#include <lftras.h>
typedef struct  strlft *strlft_ptr_t;
typedef struct  strlft strlft_t;

typedef struct lft {
        lft_dds_t	*dds_ptr;	/* Pointer to the lft DDS 	    */
        uint		initialized;	/* Boolean: TRUE if lft initialized */
        uint		open_count;	/* Number of opens 		    */
        uint		default_cursor; /* Pointer to default cursor 	    */
        struct font_data  *fonts;	/* Points to all of the fonts 	    */
        lft_swkbd_t	*swkbd;		/* Points to the swkbd information  */
	lft_fkp_t	lft_fkp;	/* Font kernel process info 	    */
	strlft_ptr_t	strlft;		/* Streams information		    */
	long		spares[20];     /* not use - for future development */
} lft_t, *lft_ptr_t;
