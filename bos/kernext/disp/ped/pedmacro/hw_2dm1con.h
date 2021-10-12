/* @(#)06	1.9  src/bos/kernext/disp/ped/pedmacro/hw_2dm1con.h, pedmacro, bos411, 9428A410j 3/17/93 19:43:31 */
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
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/****************************************************************/
/*								*/
/*          PEDERNALES 2D HW INTERFACE CONSTANTS                */
/*								*/
/****************************************************************/

#ifndef _H_MID_HW_2DM1CON
#define _H_MID_HW_2DM1CON


/************************************************************************/
/* GAI 2DM1 FIFO COMMAND ELEMENT LENGTH CONSTANTS                       */
/*                                                                      */
/*      The following constants indicate the maximum number of items    */
/*      that can be placed in each of the variable length Pedernales    */
/*      command elements.                                               */
/*                                                                      */
/*      For performance reasons, command elements are limited to 1/2    */
/*      the size of the FIFO.  The only exception to these rules are    */
/*      command elements that cannot be broken up.                      */
/*                                                                      */
/************************************************************************/

#define MID_MAX_CMD_SIZE                4080
#define MID_MAX_PERF_CMD_SIZE           (MID_MAX_CMD_SIZE / 2)
#define MID_MAX_PERF_POLYPOINT_POINTS   (MID_MAX_PERF_CMD_SIZE - 2)
#define MID_MAX_POLYLINES_POINTS        (MID_MAX_CMD_SIZE - 2)
#define MID_MAX_PERF_POLYLINES_POINTS   (MID_MAX_PERF_CMD_SIZE - 2)
#define MID_MAX_PERF_POLYSEGMENT_SEGS   (MID_MAX_PERF_CMD_SIZE - 1) / 2
#define MID_MAX_PERF_POLYRECT_RECTS     (MID_MAX_PERF_CMD_SIZE - 1) / 2
#define MID_MAX_POLYARC_ARCS            (MID_MAX_CMD_SIZE - 1) / 3
#define MID_MAX_PERF_POLYARC_ARCS       (MID_MAX_PERF_CMD_SIZE - 1) / 3
#define MID_MAX_FILLPOLYGON_POINTS      (MID_MAX_CMD_SIZE - 3)
#define MID_MAX_PERF_FILLPOLYGON_POINTS (MID_MAX_PERF_CMD_SIZE - 3)
#define MID_MAX_PERF_FILLPOLYRECT_RECTS (MID_MAX_PERF_CMD_SIZE - 1) / 2
#define MID_MAX_PERF_FILLPOLYARC_ARCS   (MID_MAX_PERF_CMD_SIZE - 1) / 3
#define MID_MAX_PERF_POLYTEXT8_CHARS    (MID_MAX_PERF_CMD_SIZE - 3) * 4
#define MID_MAX_PERF_POLYTEXT16_CHARS   (MID_MAX_PERF_CMD_SIZE - 3) * 2
#define MID_MAX_PERF_IMAGETEXT8_CHARS   (MID_MAX_PERF_CMD_SIZE - 3) * 4
#define MID_MAX_PERF_IMAGETEXT16_CHARS  (MID_MAX_PERF_CMD_SIZE - 3) * 2
#define MID_MAX_PERF_PUSHPIXELS_BITMAP_WORDS (MID_MAX_PERF_CMD_SIZE - 3)
#define MID_MAX_PERF_FILLSPANS_SPANS    (MID_MAX_PERF_CMD_SIZE - 1) / 2


/*
 * Set Line Attributes constants
 */

#define MID_UPDATE_LINE_WIDTH   0x00008000
#define MID_UPDATE_LINE_STYLE   0x00004000
#define MID_UPDATE_CAP_STYLE    0x00002000


/*
 * Set Fill Attributes constants
 */

#define MID_UPDATE_FILL_STYLE   0x00008000
#define MID_UPDATE_FILL_RULE    0x00004000
#define MID_UPDATE_ARC_MODE     0x00002000


/*
 * Set 2D Color Mode constants
 */

#define MID_MONO_SHADING_MODE   3ul
#define MID_TRUE_24BIT_MODE     4ul


#endif /* _H_MID_HW_2DM1CON */

