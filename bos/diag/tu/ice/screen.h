/* @(#)90	1.1  src/htx/usr/lpp/htx/lib/hga/screen.h, tu_hga, htx410 6/2/94 11:37:33  */
/*
 *   COMPONENT_NAME: tu_hga
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _SCREEN_H
#define _SCREEN_H

typedef struct _SCR_INFO {
	int  width, height, mode;
	int  scr_min_x, scr_max_x;
	int  scr_min_y, scr_max_y;
	int  win_min_x, win_max_x;
	int  win_min_y, win_max_y;
	int  tab;
	int  cur_x, cur_y;
} SCR_INFO;

#define CURSOR_OFF      0
#define CURSOR_NORMAL   1
#define CURSOR_HALF	2
#define CURSOR_FULL     3

#endif
