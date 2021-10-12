/* @(#)68	1.1  src/bos/diag/tu/gga/exectu.h, tu_gla, bos41J, 9515A_all 4/6/95 09:26:53 */
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "diag/atu.h"
#include "ggareg.h"


/* Next structure used to return memory test parameters when test fail. (If  */
/* test was successful all members of the str. will be set to 0.             */

typedef struct
{
  unsigned long addr;                    /* Address at which test failed     */
  unsigned long write;                   /* data written                     */
  unsigned long read;                    /* data read                        */

} MEM;

/* next struct. used to draw a rectangle */
typedef struct
{
  MEM mem_test;                          /* output: additional mem info.     */
  int errno;                             /* global errno after a system call */

} GGAPARM;


typedef struct
{
  struct  tucb_t header;
  GGAPARM gga;

} TUCB;



/* ------------------------------------------------------------------------- */
/* TUs supported :                                                           */
/* ------------------------------------------------------------------------- */

#define REGISTER_TU                 2
#define VRAM_TU                     3
#define PALETTE_TU                  4
#define PALETTECHECK_TU             50
#define CURSOR_RAM_TU               5
#define INTERRUPT_TU                6
#define ADVANCED_DPLY_TU            8
#define STRING_TEST_TU              9
#define TU_OPEN                     11
#define TU_CLOSE                    12

#define BLACK_TU                    20
#define RED_TU                      21
#define GREEN_TU                    22
#define BLUE_TU                     23
#define WHITE_TU                    24
#define CURSOR_TU                   25
#define SCROLL_H_DRAW               26
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
#define SCROLL_H_PIXEL1_EMC_TU      42
#define VIDEO_TU                    43
#define SCROLL_H_PIXEL1_TU          44


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


/* ------------------------------------------------------------------------- */
/* TUs :                                                                     */
/* ------------------------------------------------------------------------- */

extern int exectu (char *, TUCB *);


/*------- ERROR  RETURN  CODES  ----------------------------------------------*/

#define OPEN_RCM0_ERR        1   /* Attempt to open RCM0 failed               */
#define GET_GSC_ERR          2   /* GSC_HANDLE ioctl failed                   */
#define MAKE_GP_ERR          3   /* aixgsc() with MAKE_GP call failed         */
#define UNMAKE_GP_ERR        4   /* aixgsc() with UNMAKE_GP call failed       */
#define WRITE_ERR            5   /* write() error                             */
#define TU_NUMBER_ERR        7   /* bad TU number                             */
#define BAD_OP_MODE_ERR      8   /* attmp. to set an op. mode NOT supported   */
#define IOCTL_ERR            9   /* ioctl command failed                      */

#define SIGNAL_ERR           11        /* could not set error signal          */
#define LAST_SW_ERR          SIGNAL_ERR   /* Last software error              */
#define RESV_BIT_ERR         12  /* Attempt to Access Reserved Register Error */
#define REG_BIT_ERR          13  /* Invalid Value put in Control Register     */
#define STRING_BIT_ERR       14  /* Illegal String Operation Error            */
#define PARITY_BIT_ERR       15  /* RSC Parity bit set in status register     */
#define CMD_BIT_ERR          16  /* Invalid RSC cmd bit set in stat register  */
#define STATUS_CLEAR_ERR     17  /* Stat reg. err bits where not clr after rd */
#define OUT_OFF_MEMORY_ERR   18  /* Can not allocate memory                   */

#define SCREEN_RES_ERR       20  /* could not set/get screen resolution       */
#define SET_CURSOR_ERR       22  /* could not set cursor                      */
#define SET_WCLIP_ERR        23  /* could not set clipping registers          */

#define REG_COMPARE_ERR      30  /* Register compare failed                   */
#define VRAM_RW_TEST_ERR     31  /* VRAM R/W test failed                      */
#define PCI_REG_COMPARE_ERR  32  /* PCI configuration register compare failed */
#define VRAM_W_NAT_R_NAT_ERR 35  /* VRAM write NATIVE read NATIVE test failed */
#define VRAM_NATIVE_CLR_ERR  36  /* Could not clear VRAM in NATIVE mode       */

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
#define FAIRWAY_NOT_FOUND    73  /* could not locate Fairway adapter          */

#define VP_ENABLE_ERR        80  /* Could not enable 9130                     */
#define VP_DETECT_ERR        81  /* Could not find the 9130 ID                */
#define VP_SETUP_ERR         82  /* Could not set up the 9130 correctly       */
#define VP_CAPTURE_ERR       83  /* No video stream detected                  */
#define VP_DISABLE_ERR       84  /* Could not disable the 9130                */

#define SET_MODE_ERR         90  /* Could not set display mode                */
#define OPEN_DEV_BUS0_ERR    91  /* Could not open /dev/bus0                  */
#define GET_SLOT_ERR         92  /* Could not get slot info from ODM          */

