/* @(#)70       1.4  src/bos/diag/tu/wga/exectu.h, tu_wga, bos411, 9428A410j 1/3/94 17:18:07 */
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS:
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


#include "diag/atu.h"
#include "wgareg.h"


/* Next structure used to return memory test parameters when test fail. (If  */
/* test was successful all members of the str. will be set to 0.             */

typedef struct
{
  ulong_t addr;                          /* Address at which test failed     */
  ulong_t write;                         /* data written                     */
  ulong_t read;                          /* data read                        */

} MEM;

/* next struct. used to draw a rectangle */
typedef struct
{
  MEM mem_test;                          /* output: additional mem info.     */
  int errno;                             /* global errno after a system call */

} WGAPARM;


typedef struct
{
  struct  tucb_t header;
  WGAPARM wga;

} TUCB;



/* ------------------------------------------------------------------------- */
/* TUs supported :                                                           */
/* ------------------------------------------------------------------------- */

#define VPD_TU                      1
#define REGISTER_TU                 2
#define VRAM_TU                     3
#define PALETTE_TU                  4
#define CURSOR_RAM_TU               5
#define INTERRUPT_TU                6
#define VIDEO_ROM_SCAN_TU           7
#define ADVANCED_DPLY_TU            8
#define STRING_TEST_TU              9
#define FAST_COPY_TU                10

#define BLACK_TU                    20
#define RED_TU                      21
#define GREEN_TU                    22
#define BLUE_TU                     23
#define WHITE_TU                    24
#define CURSOR_TU                   25
#define SCROLL_H_GRAPH_TU0          27
#define COLOR_BAR_TU                28
#define BW_X_64_TU                  29
#define WB_9X9_TU                   30
#define WB_9X11_TU                  31
#define RGB_TU                      32
#define SCROLL_H_GRAPH_TU1          33
#define SQUARE_BOX_50MM_TU          34
#define DISPLAY_AT_TU               35
#define BW9X11_DOTS_TU              36
#define WB9X11_DOTS_TU              37
#define BLIT_TU                     38
#define PIXEL1_TU                   39
#define PIXEL8_TU                   40
#define PIXEL8_STR_TST_TU           41


/* --------------------------------- */
/* for CEL : from 100 - 200          */

#define LUMINANCE_TU                100
#define LUMINANCE_TU_1              201
#define LUMINANCE_TU_2              202
#define LUMINANCE_TU_3              203
#define LUMINANCE_TU_4              204
#define LUMINANCE_TU_5              205
#define LUMINANCE_TU_6              206
#define LUMINANCE_TU_7              207
#define LUMINANCE_TU_8              208
#define LUMINANCE_TU_9              209
#define LUMINANCE_TU_10             210
#define LUMINANCE_TU_11             211
#define LUMINANCE_TU_12             212

/* --------------------------------- */


#define TU_CLOSE                    300
#define TU_OPEN                     301

/* ------------------------------------------------------------------------- */
/* TUs :                                                                     */
/* ------------------------------------------------------------------------- */

extern int exectu(char *, TUCB *);


/*------- ERROR  RETURN  CODES  ---------------------------------------------*/

#define OPEN_RCM_ERROR       1         /* ioctl RCM_SET_DIAG_OWNER failed     */
#define DEVICE_BUSY_ERROR    2         /* HFSHANDLE ioctl failed              */
#define MAKE_GP_ERR          3         /* aixgsc() with MAKE_GP call failed   */
#define UNMAKE_GP_ERR        4         /* aixgsc() with UNMAKE_GP call failed */
#define WRITE_ERR            5         /* write() error                       */
#define TU_NUMBER_ERR        7         /* bad TU number                       */
#define BAD_OP_MODE_ERR      8         /* attmp. to set an op. mode NOT supported */
#define IOCTL_ERR            9         /* ioctl command failed                */

#define LAST_SW_ERR          SIGNAL_ERR   /* Last software error              */
#define RESV_BIT_ERR         12  /* Attempt to Access Reserved Register Error */
#define REG_BIT_ERR          13  /* Invalid Value put in Control Register     */
#define STRING_BIT_ERR       14  /* Illegal String Operation Error            */
#define PARITY_BIT_ERR       15  /* RSC Parity bit set in status register     */
#define CMD_BIT_ERR          16  /* Invalid RSC cmd bit set in stat register  */
#define STATUS_CLEAR_ERR     17  /* Stat reg. err bits where not clr after rd */
#define OUT_OFF_MEMORY_ERR   18  /* Can not allocate memory                   */

#define SCREEN_RES_ERR       20  /* could not set/get screen resolution       */
#define SET_WORIG_ERR        21  /* could not set window origin               */
#define SET_CURSOR_ERR       22  /* could not set cursor                      */
#define SET_WCLIP_ERR        23  /* could not set clipping registers          */

#define REG_COMPARE_ERR      30  /* Register compare failed                   */
#define VRAM_RW_TEST_ERR     31  /* VRAM R/W test failed                      */
#define VRAM_W_XY_R_LIN_ERR  32  /* VRAM write XY read LIN test failed        */
#define VRAM_W_XY_R_NAT_ERR  33  /* VRAM write XY read NATIVE test failed     */
#define VRAM_W_LIN_R_XY_ERR  34  /* VRAM write LINEAR read XY test failed     */
#define VRAM_W_NAT_R_XY_ERR  35  /* VRAM write NATIVE read XY test failed     */
#define VRAM_NATIVE_CLR_ERR  36  /* Could not clear VRAM in NATIVE mode       */
#define VRAM_DECAY_ERR       37  /* VRAM decay test failed                    */
#define WORIG_RANDOM_ERR     38  /* Random test of WORIG function failed      */
#define WORIG_SEQ_ERR        39  /* Sequential test of WORIG failed           */
#define WCLIP_RANDOM_ERR     40  /* Random test of WCLIP function failed      */
#define WCLIP_SEQ_ERR        41  /* Sequential test of WCLIP failed           */
#define PRAM_RW_ERR          42  /* Write/read test of palette RAM failed     */
#define PRAM_DECAY_ERR       43  /* Decay test of pelette RAM failed          */
#define CRAM_RW_ERR          44  /* Write/read test of cursor RAM failed      */
#define CRAM_DECAY_ERR       45  /* Decay test of cursor RAM failed           */
#define BAD_ERR_ST_ERR       46  /* Status register failed to report attempt  */
                                 /* to write to a reserved memory location.   */
#define BAD_ERRADDR_ERR      47  /* ERRADDR_REG didn't show address of        */
                                 /* illegal memory location.                  */
#define VPD_ERR              48  /* VPD TU failed                             */
#define BAD_VBLANKED_INT_ERR 50  /* Interrupt register failed to rep. VBLANK  */
#define BAD_DE_IDLE_INT_ERR  51  /* Interrupt register failed to rep. DE_IDLE */
#define VIDEO_ROM_ERR        53  /* Video ROM Scan error                      */
#define VIDEO_ROM_CRC_ERR    54  /* Video ROM Scan CRC check                  */
#define VRAM_STRING_TEST_ERR 56  /* RSC string function test failed           */
#define VRAM_FAST_COPY_ERR   57  /* Fast copy R/W test failed (bad WIZARD PAL)*/

#define QUAD_CMD_FAILED      60  /* Drawing engine failed (can't draw)        */
#define BLIT_CMD_FAILED      61  /* BitBlt operation failed (can't copy image)*/
#define CLEAR_BKGROUND_ERR   70  /* could not paint the background color      */
#define SET_FOREGROUND_ERR   71  /* could not paint the foreground color      */
#define CLEAR_IMAGE_ERR      72  /* could not clear the image                 */
