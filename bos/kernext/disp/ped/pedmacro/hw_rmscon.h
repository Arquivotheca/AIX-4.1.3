/* @(#)22	1.8  src/bos/kernext/disp/ped/pedmacro/hw_rmscon.h, pedmacro, bos411, 9428A410j 6/18/93 11:54:02 */
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



/************************************************************************/
/*									*/
/*              PEDERNALES RMS HW INTERFACE CONSTANTS                   */
/*                                                                      */
/************************************************************************/

#ifndef _H_MID_HW_RMSCON
#define _H_MID_HW_RMSCON

/************************************************************************/
/* RMS FIFO COMMAND ELEMENT LENGTH CONSTANTS                            */
/*                                                                      */
/*      The following constants indicate the maximum number of items    */
/*      that can be placed in each of the variable length Pedernales    */
/*      RMS command elements.                                           */
/*                                                                      */
/*      For performance reasons, command elements are limited to 1/2    */
/*      the size of the FIFO.                                           */
/*                                                                      */
/************************************************************************/

#define MID_MAX_SE_SIZE                 4080
#define MID_MAX_PERF_SE_SIZE            (MID_MAX_SE_SIZE / 2)
#define MID_MAX_PERF_WPD_WORDS          (MID_MAX_PERF_SE_SIZE - 15)


/*
 * Select Drawing Bit Planes constants
 */

#define MID_FRAME_BUFFER	0ul
#define MID_WOG_PLANES		1ul

/*
 * Set Bit Planes Write Masks constants
 */

#define MID_UPDATE_FB_WRITE_MASK	0x00008000
#define MID_UPDATE_WOG_WRITE_MASK	0x00004000

/*
 * Set Bit Planes Display Masks constants
 */

#define MID_UPDATE_FB_DISP_MASK 	0x00008000
#define MID_UPDATE_WOG_DISP_MASK	0x00004000

/*
 * Frame Buffer Control constants
 */

#define MID_SWAP_FRAME_BUFS		0x00008000
#define MID_UPDATE_DRAW_FRAME_BUF	0x00004000
#define MID_UPDATE_DISP_FRAME_BUF	0x00002000
#define MID_DRAW_DISPLAYED_FRAME_BUF	0x00000000
#define MID_DRAW_FRAME_BUF_A		0x00002000
#define MID_DRAW_FRAME_BUF_B		0x00001000
#define MID_DRAW_FRAME_BUF_A_B		0x00003000
#define MID_DISP_FRAME_BUF_A		0x00000000
#define MID_DISP_FRAME_BUF_B		0x00000010

/*
 * Z Buffer Control constants
 */

#define MID_DISABLE_Z_BUFFER_UPDATE	( ( ulong ) (0) )
#define MID_ENABLE_Z_BUFFER_UPDATE	( ( ulong ) (1 << 4) )

#define MID_DISABLE_Z_BUFFER_TEST	( ( ulong ) (0) )
#define MID_ENABLE_Z_BUFFER_TEST	( ( ulong ) (1 << 3) )

#define MID_ALWAYS_UPDATE_Z		( ( ulong ) (0) )
#define MID_NEW_Z_LT_CURRENT_Z		( ( ulong ) (1) )
#define MID_NEW_Z_EQ_CURRENT_Z		( ( ulong ) (2) )
#define MID_NEW_Z_LE_CURRENT_Z		( ( ulong ) (3) )
#define MID_NEW_Z_GT_CURRENT_Z		( ( ulong ) (4) )
#define MID_NEW_Z_NE_CURRENT_Z		( ( ulong ) (5) )
#define MID_NEW_Z_GE_CURRENT_Z		( ( ulong ) (6) )
#define MID_NEVER_UPDATE_Z		( ( ulong ) (7) )

/*
 * Clear Bit Planes constants
 */

#define MID_FRAME_BUF_24_BIT_FILL	0x00008000
#define MID_FRAME_BUF_8_BIT_FILL	0x00004000
#define MID_OVERLAY_FILL		0x00002000
#define MID_GPM_FILL			0x00001000
#define MID_Z_BUF_FILL			0x00000800
#define MID_CLEAR_IMMED 		0x00000000
#define MID_CLEAR_NEXT_VERT_RETRACE	0x00000400
#define MID_CBP_CLIP_TO_WINDOW		0x00000200
#define MID_CBP_CLIP_TO_VIEW		0x00000000

/*
 * Clear Control Planes constants
 */

#define MID_CCP_WIDS_MASK		0xFFFFFF0F
#define MID_CCP_OVERLAY_MASK		0xFFFFFFF3
#define MID_CCP_GPM_MASK		0xFFFFFFFE
#define MID_CCP_WID_COMPARE_ON		0x8000
#define MID_CCP_WID_COMPARE_OFF		0
#define MID_CCP_GPM_COMPARE_NO_CHANGE	0
#define MID_CCP_GPM_COMPARE_ON		0x0300
#define MID_CCP_GPM_COMPARE_OFF		0x0200
#define MID_CCP_GPM_COMPARE_TO_1	0x8000
#define MID_CCP_GPM_COMPARE_TO_0	0

#define MID_CCP_WID_SHIFT		4
#define MID_CCP_OVERLAY_SHIFT		2
#define MID_CCP_GPM_SHIFT		0

/*
 * Set GPM Compare Value constants
 */

#define MID_DISABLE_GPM_CHECKING	0x00000000
#define MID_ENABLE_GPM_CHECKING 	0x01000000

/*
 * Set Logical Operation constants
 */

#define MID_UPDATE_BG_LOGIC_OP		0x00008000
#define MID_UPDATE_FG_LOGIC_OP		0x00004000

/*
 * Blit command (Read Pixel Data, Pixel Blit, Write Pixel Data) constants
 */

#define MID_8_BIT_BLIT			0x0000
#define MID_24_BIT_BLIT 		0x8000
#define MID_RPD_WINDOW_COORDS		0x4000
#define MID_RPD_SCREEN_COORDS		0x0000
#define MID_READ_ALL_COLORS		0x0000
#define MID_READ_RED_ONLY		0x0400
#define MID_READ_GREEN_ONLY		0x0800
#define MID_READ_BLUE_ONLY		0x0c00
#define MID_BLIT_INCREASING_Y		0x0000
#define MID_BLIT_DECREASING_Y		0x0200
#define MID_READ_FRAME_BUF_A		1ul
#define MID_READ_FRAME_BUF_B		2ul
#define MID_READ_WOG_PLANES		3ul
#define MID_READ_Z_BUF_HIGH		4ul
#define MID_READ_Z_BUF_MID		5ul
#define MID_READ_Z_BUF_LOW		6ul
#define MID_READ_Z_BUF_ALL		7ul
#define MID_READ_DISP_FRAME_BUF 	9ul
#define MID_READ_DRAW_FRAME_BUF 	10ul

#define MID_PB_WINDOW_COORDS		0x4000
#define MID_PB_SCREEN_COORDS		0x0000
#define MID_READ_PIXEL_BLIT_COLOR	8ul
#define MID_BLIT_DIR_USER_SUPPLIED	0
#define MID_BLIT_DIR_ADAPTER_SUPPLIED	1
#define MID_BLIT_INCR_X_Y		0
#define MID_BLIT_DECR_X_INCR_Y		1
#define MID_BLIT_INCR_X_DECR_Y		2
#define MID_BLIT_DECR_X_Y		3

#define MID_WPD_WINDOW_COORDS           0x4000
#define MID_WPD_SCREEN_COORDS           0x0000
#define MID_WPD_MODELLING_COORDS        0x2000
#define MID_WRITE_NO_COLOR_EXPAND       0x0000
#define MID_WRITE_COLOR_EXPAND          0x1000
#define MID_WRITE_DMA                   0x0000
#define MID_WRITE_PIO                   0x0800
#define MID_BLIT_EXEC_IMMED             0x0000
#define MID_BLIT_EXEC_NEXT_VERT_RETRACE 0x0400
#define MID_WRITE_FRAME_BUF_A           1ul
#define MID_WRITE_FRAME_BUF_B           2ul
#define MID_WRITE_BOTH_FRAME_BUFS       3ul
#define MID_WRITE_WOG_PLANES            4ul
#define MID_WRITE_Z_BUF_HIGH            5ul
#define MID_WRITE_Z_BUF_MID             6ul
#define MID_WRITE_Z_BUF_LOW             7ul
#define MID_WRITE_Z_BUF_HIGH_MID        8ul
#define MID_WRITE_Z_BUF_HIGH_LOW        9ul
#define MID_WRITE_Z_BUF_MID_LOW         10ul
#define MID_WRITE_Z_BUF_ALL_SAME        11ul
#define MID_WRITE_Z_BUF_ALL             12ul
#define MID_WRITE_DISP_FRAME_BUF        13ul
#define MID_WRITE_DRAW_FRAME_BUF        14ul

/*
 * Set Color Processing Mode constants
 */

#define MID_TRUNCATED_8_BIT_MODE	( ( ulong ) (1) )
#define MID_DITHERED_8_BIT_MODE 	( ( ulong ) (2) )
#define MID_MONOCHROME_SHADING_MODE	( ( ulong ) (3) )
#define MID_TRUE_24_BIT_MODE		( ( ulong ) (4) )

/*
 * Transparency and Color Compare constants
 */

#define MID_UPDATE_TRANSPARENCY 		0x00000002
#define MID_UPDATE_COLOR_COMPARE		0x00000001
#define MID_DISABLE_TRANSPARENCY		0x00000000
#define MID_UPDATE_SOURCE_EQUAL_TVAL		0x40000000
#define MID_UPDATE_SOURCE_NOT_EQUAL_TVAL	0xa0000000
#define MID_DISABLE_COLOR_COMPARE		0x00000000
#define MID_UPDATE_DEST_EQUAL_CVAL		0x40000000
#define MID_UPDATE_DEST_NOT_EQUAL_CVAL		0xa0000000


#endif  /* _H_MID_HW_RMSCON */
