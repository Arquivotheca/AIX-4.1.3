static char sccsid[] = "@(#)94	1.1  src/bos/diag/tu/gga/videotu.c, tu_gla, bos41J, 9515A_all 4/6/95 09:27:37";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS:
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

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <sys/types.h>
#include <sys/errno.h>

#include "ggapci.h"
#include "ggamisc.h"
#include "i2c.h"
#include "exectu.h"

/* These defines are from tu_type.h */
#ifdef LOGMSGS
extern void logmsg (char *);
extern void logerror (char *);
extern void log_syserr (char *);
extern void update_msg (void);
#define LOG_SYSERR(msg)      log_syserr (msg)
#define LOG_MSG(msg)         logmsg (msg)
#define LOG_ERROR(msg)       logerror (msg)
#define ALIVE_MSG            update_msg ()
#else
#define LOG_SYSERR(msg)
#define LOG_MSG(msg)
#define LOG_ERROR(msg)
#define ALIVE_MSG
#endif

/* Local Variables */

unsigned int Buff0Params[16];    /* parameters for the stream. */
int Buff0ParamCount;             /* number of parameters */
int i, c;
int queue_size, queue_size_code; /* queue parameters */
unsigned int queue_base;         /* base address of queue in offscreen memory */
unsigned int offscreen_base;     /* base of available offscreen memory.*/
                                 /* Points after on_screen memory and the queue */
unsigned int fill_value;         /* background color in a format           */
unsigned int swap_fill_value;    /* 9100 understands.                         */
unsigned int greyvalue;          /* background grey level. 0 to 31 allowed. */
int required_vsyncs;             /* number of vertical periods (graphics display) */
                                 /* to wait between VideoPower operations */
unsigned char palette[3 * 256];  /* used in 8-bit graphics mode to contain */
                                 /* VideoPower specific RAMDAC palette */
int config_switch = 0;
int arb_reg = 0;
int original_size = 0;
double scale = 0.0;
int source_width;
int source_height;
unsigned long capturestate;
int current_buffer;
unsigned long sticky_bits = 0x7;
int capture_overrun;
unsigned long watchdog;
int s_video = 0;
int hue_var = 0;
int lock_aspect = 0;

int Switches = 0;
int Synced = 0;
SrcSpec Buff0Input;     /* Defines the source specification for the stream  */
DstSpec Output;         /* Defines the position and size of the window     */
                        /* before any clipping. Fields are defined in diagtype.h */
ScrnSpec Screen;        /* screen parameters. Defined in diagtype.h */
SegSpec ClipWin;        /* Contains the size, offset and dimension of */
                        /* the destination window after clipping to the */
                        /* screen. Defined in diagtype.h */

static UCHAR  palette_offset = 0,
              save_palette[256*4],
              i2cstatus,
              SAA7196Init[] = {0x00,
                               0x4c,0x30,0x00,0xe8,0xb6,0xf4,0x40,0x00,
                               0xf8,0xf8,0x40,0x40,0x00,0x8a,0x38,0x50,
                               0x00,0x2c,0x40,0x40,0x34,0x0a,0xf4,0xce,
                               0xf4,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
                               0xb1,0x40,0x80,0x00,0x09,0xf0,0xf0,0x0e,
                               0x00,0x00,0x00,0x00,0x00,0xff,0x00,0x00,
                               0xa0},
              Brightness[2]  = {0x19, 0x80},
              Contrast[2]    = {0x13, 0x40},
              Hue[2]         = {0x07, 0x00},
              Saturation[2]  = {0x12, 0x40},
              Chroma_Gain[2] = {0x11, 0x2c},
              Filters[2]     = {0x06, 0x40},
              Noise[2]       = {0x10, 0x00},
              bandpass       = 0,
              coring         = 0,
              weights        = 0,
              noise          = 0,
              first_time     = 0,
              slot_found     = 0;
static USHORT output_pitch = 0,
              preempt_count = 0, sleep_count = 0, active_count = 0,
              drop = 0,
              video_source,
              input_resolution,
              output_resolution,
              brightness_setting,
              contrast_setting,
              hue_setting,
              saturation_setting,
              chroma_gain_setting;
static ULONG  output_mode = 0,
              mem_config = 0,
              arbitration = 0,
              interrupt_mode = 0,
              output_line_count = 0,
              capture_control = 0,
              buff_0_address = 0,
              queue_address = 0,
              cmdqueue_baseaddress = 0,
              cmdqueue_size = 0,
              cmdqueue_indexmask = 0,
              cmdqueue_countermask = 0,
              cmdqueue_tail = 0,
             *cmdqueueptr,
              wait_time = 0,
              camera_state = 0,
              tmp = 0;
static UINT   slot_number = 0,
              bus_id = 0;

/* External Variables */

extern UINT gga_x_max, gga_y_max;

/* Local Prototypes */

USHORT VP_Enable(UCHAR);
USHORT VP_Detect(void);
USHORT VP_Set_Up(void);
USHORT VP_Capture(void);
USHORT VP_Disable(void);
void   DumpQueue(void);
void   VCPBusyWait(void);
void   VCPQueueWait(void);
void   PrintVCPRegs(void);
void   FillQueue(int, UINT *);
void   RefreshQueueTail(void);
void   p9_rgb525_write(USHORT, UCHAR);


int video_tu(void)
  {
    ULONG  params, count;
    int    rc=SUCCESS;

  if (slot_found == 0)
    {
      rc = get_slot("gga0", &slot_number, &bus_id);
      if (rc == 0)
        {
          slot_found = 1;
        }
      else
        {
          return(GET_SLOT_ERR);
        }
    }

    ReadPalette(0, 256, save_palette);

    if (VP_Enable(HIGH_PRIORITY) != SUCCESS)
      {
        return(VP_ENABLE_ERR);
      }
    if (VP_Detect() != SUCCESS)
      {
        return(VP_DETECT_ERR);
      }
    video_source        = COMPOSITE;
    input_resolution    = INPUT_RES_320X240;
    output_resolution   = OUTPUT_RES_160X120;
    brightness_setting  = BRIGHTNESS_NOMINAL;
    contrast_setting    = CONTRAST_NOMINAL;
    hue_setting         = HUE_NOMINAL;
    saturation_setting  = SATURATION_NOMINAL;
    chroma_gain_setting = CHROMA_GAIN_NOMINAL;

    if (VP_Set_Up() != SUCCESS)
      {
        return(VP_SETUP_ERR);
      }

    color216palette(); /* Update palette with 216 linear values */
    if (VP_Capture() != SUCCESS)
      {
        set_tu_errno();
        rc = VP_CAPTURE_ERR;
        LOG_SYSERR("Failed to capture from composite input.");
      }
    video_source        = S_VIDEO;
    input_resolution    = INPUT_RES_320X240;
    output_resolution   = OUTPUT_RES_320X240;
    brightness_setting  = BRIGHTNESS_NOMINAL;
    contrast_setting    = CONTRAST_NOMINAL;
    hue_setting         = HUE_NOMINAL;
    saturation_setting  = SATURATION_NOMINAL;
    chroma_gain_setting = CHROMA_GAIN_NOMINAL;
    if (VP_Set_Up() != SUCCESS)
      {
        return(VP_SETUP_ERR);
      }
    if (VP_Capture() != SUCCESS)
      {
        set_tu_errno();
        rc = VP_CAPTURE_ERR;
        LOG_SYSERR("Failed to capture from s-video input.");
      }
    video_source        = CAMERA;
    input_resolution    = INPUT_RES_320X240;
    output_resolution   = OUTPUT_RES_480X360;
    brightness_setting  = BRIGHTNESS_NOMINAL;
    contrast_setting    = CONTRAST_NOMINAL;
    hue_setting         = HUE_NOMINAL;
    saturation_setting  = SATURATION_NOMINAL;
    chroma_gain_setting = CHROMA_GAIN_NOMINAL;
    if (VP_Set_Up() != SUCCESS)
      {
        return(VP_SETUP_ERR);
      }
    if (VP_Capture() != SUCCESS)
      {
        set_tu_errno();
        rc = VP_CAPTURE_ERR;
        LOG_SYSERR("Failed to capture from R/C camera input.");
      }
    video_source        = COMPOSITE;
    input_resolution    = INPUT_RES_320X240;
    output_resolution   = OUTPUT_RES_640X480;
    brightness_setting  = BRIGHTNESS_NOMINAL;
    contrast_setting    = CONTRAST_NOMINAL;
    hue_setting         = HUE_NOMINAL;
    saturation_setting  = SATURATION_NOMINAL;
    chroma_gain_setting = CHROMA_GAIN_NOMINAL;
    if (VP_Set_Up() != SUCCESS)
      {
        return(VP_SETUP_ERR);
      }
    if (VP_Capture() != SUCCESS)
      {
        set_tu_errno();
        rc = VP_CAPTURE_ERR;
        LOG_SYSERR("Failed to capture from composite input.");
      }
    if (VP_Disable() != SUCCESS)
      {
        return(VP_DISABLE_ERR);
      }

    WritePalette(0, 256, save_palette);

    return(rc);
  }


USHORT VP_Enable(unsigned char priority)
  {
    UCHAR deviceID, pci_data, data, tempb;
    USHORT rc = SUCCESS;
    ULONG templ, slot1_ID, slot2_ID;
    int fd;
    char *error_string;

    /*** Enable 9130 via VCEN pin ***/

    fd = open ("/dev/bus0",  O_RDWR | O_NDELAY); /* Open PCI bus DD */
    if (fd < 0)
      {
        rc = errno = OPEN_DEV_BUS0_ERR;
        set_tu_errno();
        return(rc);
      }

    rc = rd_cfg_byte(fd, &pci_data, slot_number, CKSEL_VCEN);
    if (rc != 0)
      {
        error_string = strerror(rc);
        LOG_SYSERR(error_string);
      }

    /* Set VCEN bit in byte corresponding to config reg 66 */
    pci_data |= 0x01;

    rc = wr_cfg_byte(fd, pci_data, slot_number, CKSEL_VCEN);
    if (rc != 0)
      {
        error_string = strerror(rc);
        LOG_SYSERR(error_string);
      }

    rc = rd_cfg_byte(fd, &pci_data, slot_number, CKSEL_VCEN);
    if (rc != 0)
      {
        error_string = strerror(rc);
        LOG_SYSERR(error_string);
      }
    pci_data &= 0x01;
    if (pci_data != 0x01)
      {
        close(fd);
        rc = errno = VP_ENABLE_ERR;
        set_tu_errno();
        return(FAIL);
      }

    close(fd); /* Close DD */

    /* Set 9130 priority on 9100 */

    templ = RL(FRAME_BUFFER + VIDEO_HW_BUG);
    mem_config = RL(W9100_MEMCNFG);

    if (priority == LOW_PRIORITY)
      mem_config &= 0xFFFFFFBF;  /* Set 9130 accesses to low priority */
    else
      mem_config |= 0x00000040;  /* Set 9130 accesses to high priority */

    templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);
    WL(mem_config, W9100_MEMCNFG);
    mem_config = RL(W9100_MEMCNFG);
    mem_config = (mem_config & 0x00000040) >> 6;
    if (mem_config != priority)
      {
        rc = errno = VP_ENABLE_ERR;
        set_tu_errno();
        return(FAIL);
      }

    return(SUCCESS);
  }


USHORT VP_Detect()
  {
    USHORT rc;
    ULONG  coproc_id;
    ULONG templ;

 /* WL(0x0, W9130_ID); */ /* Write a zero to be sure */
    templ = RL(FRAME_BUFFER + W9130_HW_BUG);
    coproc_id = RL(W9130_ID);

    if ((coproc_id & 0xFFFF0000) != 0x56500000)
      {
        rc = FAIL;
      }
    else
      {
        rc = SUCCESS;
      }

    return(rc);
  }


USHORT VP_Set_Up()
  {
    UCHAR val;
    USHORT rc = SUCCESS, i, j;
    ULONG templ;

    /***** Compute VideoPower parameters *****/

    /* initialize structure specifying screen properties. */
    Screen.ScrnMode = 4;    /* 8-bit color */
    Screen.ScrnWidth = gga_x_max;
    Screen.ScrnHeight = gga_y_max;
    Screen.ScrnBytesPerPel = 1;
    Screen.ScrnPitch = Screen.ScrnWidth/4;

    /* set default queue_size to 16 */
    queue_size = 16;
    required_vsyncs = 2;
    original_size = 0;
    greyvalue = 0;

    /* initialize structures specifying input */

    if(video_source == S_VIDEO)
      {
        SAA7196Init[0x7] &= 0x3f;
        SAA7196Init[0x7] |= 0x80;
        SAA7196Init[0xf] &= 0xfc;
        SAA7196Init[0xf] |= 0x01;
        Filters[1] |= 0x80;
      }
    else if(video_source == CAMERA)
      {
        SAA7196Init[0x7] &= 0x3f;
        SAA7196Init[0x7] |= 0x80;
        SAA7196Init[0xf] &= 0xfc;
        SAA7196Init[0xf] |= 0x02;
        Filters[1] |= 0x80;
      }
    else /* video_source == COMPOSITE */
      {
        SAA7196Init[0x7] &= 0x3f;
        SAA7196Init[0xf] &= 0xfc;
        Filters[1] &= 0x7F;
      }

    switch (input_resolution)
      {
        case INPUT_RES_80X60:
          Buff0Input.SrcWidth = 80;
          Buff0Input.SrcHeight = 60;
          break;
        case INPUT_RES_160X120:
          Buff0Input.SrcWidth = 160;
          Buff0Input.SrcHeight = 120;
          break;
        case INPUT_RES_240X180:
          Buff0Input.SrcWidth = 240;
          Buff0Input.SrcHeight = 180;
          break;
        case INPUT_RES_320X240:
          Buff0Input.SrcWidth = 320;
          Buff0Input.SrcHeight = 240;
          break;
        default:
          /* Unexpected input_resolution in VP_Set_Up() */
          LOG_SYSERR("Unexpected input resolution.");
          return(FAIL);
          break;
      }

    SAA7196Init[0x22] = (unsigned char)(Buff0Input.SrcWidth & 0xff);
    SAA7196Init[0x25] &= 0xfc;
    SAA7196Init[0x25] |= ((unsigned char)(Buff0Input.SrcWidth  >> 8) & 0x3);
    SAA7196Init[0x25] &= 0x1f;
    if(tmp > 597)
        {
        SAA7196Init[0x25] |= 0x80;
        }
    else if(tmp > 469)
        {
        SAA7196Init[0x25] |= 0x00;
        }
    else if(tmp > 298)
        {
        SAA7196Init[0x25] |= 0xe0;
        }
    else if(tmp > 128)
        {
        SAA7196Init[0x25] |= 0x40;
        }
    else
        {
        SAA7196Init[0x25] |= 0x60;
        }

    SAA7196Init[0x26] = (unsigned char)(Buff0Input.SrcHeight & 0xff);
    SAA7196Init[0x29] &= 0xfc;
    SAA7196Init[0x29] |= ((unsigned char)(Buff0Input.SrcHeight >> 8) & 0x3);
    SAA7196Init[0x29] &= 0x9f;
    if(tmp > 416)
        {
        SAA7196Init[0x29] |= 0x00;
        }
    else if(tmp > 128)
        {
        SAA7196Init[0x29] |= 0x40;
        }
    else
        {
        SAA7196Init[0x29] |= 0x60;
        }

    /* Compute queue base address and offscreen base address */
    switch(queue_size)
        {
        case 16: queue_size_code = 0; break;
        case 32: queue_size_code = 1; break;
        case 64: queue_size_code = 2; break;
        case 128: queue_size_code = 3; break;
        case 256: queue_size_code = 4; break;
        case 512: queue_size_code = 5; break;
        case 1024: queue_size_code = 6; break;
        case 2048: queue_size_code = 7; break;
        default:  /* Invalid queue size */
          LOG_SYSERR("Unexpected queue size.");
          return(FAIL);
        }
    /* The following is based on long word addressing. */
    queue_base = Screen.ScrnPitch * Screen.ScrnHeight;
    queue_base += (queue_size - 1);
    queue_base &= ~(queue_size - 1);
    offscreen_base = queue_base + queue_size;

    /* initialize structures specifying output windows */

    switch (output_resolution)    /* set width of displayed video */
      {
        case OUTPUT_RES_160X120:
          Output.DstDx = 160;
          Output.DstDy = 120;
          break;
        case OUTPUT_RES_320X240:
          Output.DstDx = 320;
          Output.DstDy = 240;
          break;
        case OUTPUT_RES_480X360:
          Output.DstDx = 480;
          Output.DstDy = 360;
          break;
        case OUTPUT_RES_640X480:
          Output.DstDx = 640;
          Output.DstDy = 480;
          break;
        case OUTPUT_RES_800X600:
          Output.DstDx = 800;
          Output.DstDy = 600;
          break;
        case OUTPUT_RES_960X720:
          Output.DstDx = 960;
          Output.DstDy = 720;
          break;
        case OUTPUT_RES_1024X768:
          Output.DstDx = 1024;
          Output.DstDy = 768;
          break;
        case OUTPUT_RES_1280X1024:
          Output.DstDx = 1280;
          Output.DstDy = 1024;
          break;
        default:
          /* Unexpected output_resolution in VP_Set_Up() */
          LOG_SYSERR("Unexpected output resolution.");
          return(FAIL);
          break;
      }

    Output.DstX  = (Screen.ScrnWidth - Output.DstDx) / 2;   /* set origin of */
    Output.DstY  = (Screen.ScrnHeight - Output.DstDy) / 2;  /* displayed video */

    Buff0Input.SrcPitch = Buff0Input.SrcWidth * 2;
    Buff0Input.SrcStride = (((Buff0Input.SrcWidth + 15) & 0xfffffff0) + 32)/2;
    Buff0Input.SrcBytes = 2;
    Buff0Input.SrcMode = 3;
    Buff0Input.SrcOrigin = 1; /* Origin is top left corner */
    Buff0Input.SrcComps = 1;
    Buff0Input.SrcAddr0 = offscreen_base;

    /* initialize the parameters for the video stream */
    /* First clip the output window defined in "Output" structure to the */
    /* screen and put the specification for the clipped window in the    */
    /* "ClipWin" struct */

    ClipToScreen(&Output, &ClipWin, &Screen);

    Buff0ParamCount = 0;

    Buff0ParamCount = ComputeParams(&Screen, &Buff0Input, &Output, &ClipWin,
                                    &Buff0Params[0], Buff0ParamCount,
                                    (Switches & 0x7fffffff));
    /**** DEBUG ONLY - DISABLE DITHER ****/
    /* Buff0Params[12] &= 0xffffffdf;    */
    /**** DEBUG ONLY - DISABLE DITHER ****/

    /* Initialize the Philips SAA7196 */

    switch (brightness_setting)
      {
        case BRIGHTNESS_LOW:
          SAA7196Init[0x19 + 1] = 0x40;
          break;
        case BRIGHTNESS_NOMINAL:
          SAA7196Init[0x19 + 1] = 0x80;
          break;
        case BRIGHTNESS_HIGH:
          SAA7196Init[0x19 + 1] = 0xc0;
          break;
        case BRIGHTNESS_MAX:
          SAA7196Init[0x19 + 1] = 0xFF;
          break;
        default:
          /* Unexpected brightness_setting in VP_Set_Up() */
          return(FAIL);
          break;
      }
    switch (contrast_setting)
      {
        case CONTRAST_LOW:
          SAA7196Init[0x13 + 1] = 0x20;
          break;
        case CONTRAST_NOMINAL:
          SAA7196Init[0x13 + 1] = 0x40;
          break;
        case CONTRAST_HIGH:
          SAA7196Init[0x13 + 1] = 0x60;
          break;
        case CONTRAST_MAX:
          SAA7196Init[0x13 + 1] = 0x7F;
          break;
        default:
          /* Unexpected contrast_setting in VP_Set_Up() */
          return(FAIL);
          break;
      }
    switch (hue_setting)
      {
        case HUE_LOW:
          SAA7196Init[0x07 + 1] = 0x40;
          break;
        case HUE_NOMINAL:
          SAA7196Init[0x07 + 1] = 0x00;
          break;
        case HUE_HIGH:
          SAA7196Init[0x07 + 1] = 0xC0;
          break;
        case HUE_MAX:
          SAA7196Init[0x07 + 1] = 0x80;
          break;
        default:
          /* Unexpected hue_setting in VP_Set_Up() */
          return(FAIL);
          break;
      }
    switch (saturation_setting)
      {
        case SATURATION_LOW:
          SAA7196Init[0x12 + 1] = 0x20;
          break;
        case SATURATION_NOMINAL:
          SAA7196Init[0x12 + 1] = 0x40;
          break;
        case SATURATION_HIGH:
          SAA7196Init[0x12 + 1] = 0x60;
          break;
        case SATURATION_MAX:
          SAA7196Init[0x12 + 1] = 0x7F;
          break;
        default:
          /* Unexpected saturation_setting in VP_Set_Up() */
          return(FAIL);
          break;
      }
    switch (chroma_gain_setting)
      {
        case CHROMA_GAIN_LOW:
          SAA7196Init[0x11 + 1] = 0x16;
          break;
        case CHROMA_GAIN_NOMINAL:
          SAA7196Init[0x11 + 1] = 0x2c;
          break;
        case CHROMA_GAIN_HIGH:
          SAA7196Init[0x11 + 1] = 0x69;
          break;
        case CHROMA_GAIN_MAX:
          SAA7196Init[0x11 + 1] = 0xFF;
          break;
        default:
          /* Unexpected chroma_gain_setting in VP_Set_Up() */
          return(FAIL);
          break;
      }


    if (SendI2CData(RESET_ADDR, RESET_CARD, 1) != I2C_OK)
      {
        LOG_SYSERR("Failed to reset Mambo.");
        return(FAIL);
      }
    sleep(1); /* Give Mambo power a chance to cycle */

    if (SendI2CData(SAA7196, SAA7196Init, sizeof(SAA7196Init)) != I2C_OK)
      {
        LOG_SYSERR("Failed to initialize the 7196.");
        return(FAIL);
      }

    /* VideoPower Registers 2 and 5 must be cleared */
    /* to enable certain fields to be written       */
    templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);
    WL(0x1000L, W9130_MEM_CONFIG);
    templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);
    WL(0L, W9130_QUEUE_POINTER);

    palette_offset = 40; /* 216 linear colors will start at palette entry 40 */
    output_pitch = Screen.ScrnPitch;
    output_mode |= VERT_SYNC_POLARITY_0;
    output_mode |= HORIZ_SYNC_POLARITY_0;
    output_mode |= DEST_MODE_SINGLE_COMPONENT;
    output_mode |= (palette_offset << 16);
    output_mode |= output_pitch;
    templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);
    WL(output_mode, W9130_OUTPUT_MODE); /* (0x0 | mode << 24 | palette << 16) */


    mem_config = RL(W9100_MEMCNFG);
    mem_config &= 0x00000007;
    switch(mem_config)
      {
        case 1: case 2:   mem_config = (ULONG) 1; break;
        case 3:           mem_config = (ULONG) 3; break;
        case 4:           mem_config = (ULONG) 4; break;
        case 5: case 6:   mem_config = (ULONG) 5; break;
        case 7:           mem_config = (ULONG) 7; break;
        default:
          {
            return(FAIL);
          }
      }
    mem_config |= INT_MODE_PCI;        /* 0x00001000 */
    mem_config |= CONTINUE_EXECUTION;  /* 0x00000000 */
    mem_config |= BIG_ENDIAN;          /* 0x00000400 */
    mem_config |= NORMAL_SCALAR;       /* 0x00000100 */
    mem_config |= NORMAL_HOLD;         /* 0x00000000 */
    mem_config |= NORMAL_READS;        /* 0x00000000 */
    mem_config |= NORMAL_WRITES;       /* 0x00000000 */
    mem_config |= PREEMPT_MODE_1;      /* 0x00000010 */
    mem_config |= NORMAL_VRAM_SAMPLE;  /* 0x00000000 --> 0x00001510 */
    templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);
    WL(mem_config, W9130_MEM_CONFIG);  /* (mem_config | 0x00001510) */

 /* active_count = 511; sleep_count =  15; preempt_count =  60; */ /* 0x00f01fff */
 /* arbitration = (preempt_count << 18) | (sleep_count << 9) | (active_count); */
    arbitration = 0xFF;
    templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);
    WL(arbitration, W9130_ARBITRATION);

    queue_address = (queue_size_code << 20) | (queue_base >> 4);
    templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);
    WL(queue_address, W9130_QUEUE_ADDRESS);

    cmdqueue_baseaddress = queue_base << 2;
    cmdqueue_size = 0x00000010 << queue_size_code;
    cmdqueue_indexmask = cmdqueue_size - 1;
    cmdqueue_countermask = (cmdqueue_indexmask << 1) | 0x01;
    /* Set up a pointer to the command queue so we can load entries later. */
    cmdqueueptr = (ULONG *) (FRAME_BUFFER | cmdqueue_baseaddress);

    interrupt_mode |= DISABLE_SOURCE_VERT_SYNC_INT;
    interrupt_mode |= DISABLE_FRAME_CAPTURE_INT;
    interrupt_mode |= DISABLE_VP_PAUSED_INT;
    interrupt_mode |= DISABLE_QUEUE_EMPTY_INT;
    interrupt_mode |= DISABLE_VP_NOT_BUSY_INT;
    interrupt_mode |= DISABLE_P9100_VERT_SYNC_INT;
    templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);
    WL(interrupt_mode, W9130_INTERRUPT_CONTROL); /* 0x0 */

    output_line_count = 0;
    templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);
    WL(output_line_count, W9130_OUTPUT_LINE_COUNT); /* 0x0 */

    /* BUFF_0_ADDRESS is a long word offset from the start of the frame buffer */
    buff_0_address = offscreen_base;
    templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);
    WL(buff_0_address, W9130_BUFF_0_ADDRESS);

    capture_control |= FRAME_RATE_CAPTURE_30FPS;
    capture_control |= FIELD_CAPTURE_STOP;
    capture_control |= CAPTURE_SINGLE_BUFFER;
    capture_control |= CAPTURE_DISABLED;
    templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);
    WL(capture_control, W9130_CAPTURE_CONTROL); /* 0x0 */

    return(rc);
  }


USHORT VP_Capture()
  {
    ULONG templ;
    UINT  frames;

    /* Start capture */
    capture_control |= CAPTURE_PHILIPS_ENABLED;
    templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);
    WL(capture_control, W9130_CAPTURE_CONTROL);
    capture_control |= FIELD_CAPTURE_BOTH;
    templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);
    WL(capture_control, W9130_CAPTURE_CONTROL);

    frames = 0;
    do
      {
        /* Handle Video Input */
        watchdog = 0;
        do
          {
            capturestate = RL(W9130_CAPTURE_CONTROL);
            watchdog++;
            if (watchdog == 0x100000)
              {
                /* Stuck waiting for buffer */
                return(FAIL);
              }
          } while ((capturestate & BUFFER_0_FULL_MASK) == BUFFER_0_NOT_FULL);

     /* WaitForVSYNC(required_vsyncs); */
        FillQueue(Buff0ParamCount, Buff0Params);
        VCPQueueWait();
        VCPBusyWait();

        capturestate = RL(W9130_CAPTURE_CONTROL);
        capture_overrun = capturestate & CAPTURE_BUFFER_OVERRUN_MASK;
        capturestate = ((capturestate | sticky_bits) & ~(BUFFER_0_FULL_MASK));

        if (capture_overrun == OVERRUN)
          {
            capturestate &= ~(CAPTURE_BUFFER_OVERRUN_MASK);;
          }
        templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);
        WL(capturestate, W9130_CAPTURE_CONTROL);
        frames++;
      } while (!end_tu(get_dply_time()));

    VCPQueueWait();
    VCPBusyWait();

    return(SUCCESS);
  }


USHORT VP_Disable()
  {
    UCHAR pci_data;
    USHORT rc = SUCCESS;
    int fd;
    char *error_string;

    /* Disable 9130 via VCEN pin */

    fd = open ("/dev/bus0",  O_RDWR | O_NDELAY);
    if (fd < 0)
      {
        rc = errno = OPEN_DEV_BUS0_ERR;
        set_tu_errno();
        return(rc);
      }

    rc = rd_cfg_byte(fd, &pci_data, slot_number, CKSEL_VCEN);
    if (rc != 0)
      {
        error_string = strerror(rc);
        LOG_SYSERR(error_string);
      }

    /* Clear VCEN bit in byte corresponding to config reg 66 */
    pci_data &= 0xFE;

    rc = wr_cfg_byte(fd, pci_data, slot_number, CKSEL_VCEN);
    if (rc != 0)
      {
        error_string = strerror(rc);
        LOG_SYSERR(error_string);
      }

    rc = rd_cfg_byte(fd, &pci_data, slot_number, CKSEL_VCEN);
    if (rc != 0)
      {
        error_string = strerror(rc);
        LOG_SYSERR(error_string);
      }
    pci_data &= 0x01;
    if (pci_data != 0x00)
      {
        close(fd);
        rc = errno = VP_DISABLE_ERR;
        set_tu_errno();
        return(FAIL);
      }

    close(fd);
    return(rc);
  }


void FillQueue(int paramcount, UINT *params)
{
ULONG cmdqueue_current;
ULONG free_entries;
ULONG register5;
ULONG templ;
UCHAR phh, phl, plh, pll, z;

/* Outer loop runs until we've run out of parameters */
while (paramcount != 0)
        {
        /* Wait until there are some free entries in the command queue */
        do
                {
                register5 = RL(W9130_QUEUE_POINTER);
                cmdqueue_tail = register5 & 0x00000fff; /* read queue tail */

                /* NOTE: The VideoPower internal queue pointer is not        */
                /*       constrained in any way when incremented, so we must */
                /*       mask off the upper bits that we don't care about.   */

                cmdqueue_current = (register5 >> 16) & cmdqueue_countermask;

                /* Since the queue is circular there are two cases that must */
                /* be considered when computing the number of free entires.  */
                /* NOTE: This matches the queue addressing scheme used in    */
                /* VideoPower which should be well understood.               */

                if (cmdqueue_current > cmdqueue_tail)
                  free_entries = cmdqueue_current - cmdqueue_tail - cmdqueue_size;
                else
                  free_entries = cmdqueue_size + cmdqueue_current - cmdqueue_tail;

                /* May want to add a delay here so that we don't read from */
                /* VideoPower too often while we're waiting for some queue */
                /* entries to free up.                                     */

                for (z=0; z<100; z++)
                  templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);

                } while (free_entries == 0);

        /* We now need to fill min(free_entries, paramcount) slots */
        /* in the command queue.                                   */
        while ((free_entries != 0) && (paramcount != 0))
                {
                phh = (*params & 0xff000000) >> 24;
                phl = (*params & 0x00ff0000) >> 16;
                plh = (*params & 0x0000ff00) >>  8;
                pll = (*params & 0x000000ff);
                templ = ((pll << 24) | (plh << 16) | (phl << 8) | phh);
                *(cmdqueueptr + (cmdqueue_tail & cmdqueue_indexmask)) = templ;
                cmdqueue_tail = (cmdqueue_tail + 1) & cmdqueue_countermask;
                free_entries--;
                params++;
                paramcount--;
                }
        /* We've either filled the queue or run out of parameters. */
        /* Update the queue tail register.                         */
        templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);
        WL(cmdqueue_tail, W9130_QUEUE_POINTER);
        cmdqueue_tail &= 0x00000fff; /* preserve queue tail */
        }

}


void RefreshQueueTail()
{
  ULONG cmdqueue_pointer_buffer[512];
  ULONG tail;
  ULONG templ;

  templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);
  WL(cmdqueue_tail, W9130_QUEUE_POINTER);

}


int WaitForVSYNC(n)
int n;
  {
    int i, value, watchdog;

    for (i = 0 ; i < n ; i++)
      {
        /* Wait for the vertical counter to reach zero. */
        watchdog = 0;
        do
          {
            value = RL(W9100_VRTC);
            watchdog++;
            if (watchdog > 0x100000)
              /* Stuck waiting for VRTC to reach 0 */
              return(FAIL);
          } while (value != 0);
        /* Then wait for it to increment past zero. */
        watchdog = 0;
        do
          {
            value = RL(W9100_VRTC);
            watchdog++;
            if (watchdog > 0x100000)
              /* Stuck waiting for VRTC to become non-zero */
              return(FAIL);
          } while (value == 0);
      }
    return(0);
  }


void p9_rgb525_write( USHORT index, UCHAR data)
  {
  UINT  RepdData;

  RepdData  = index & 0x00ff;
  RepdData |= (RepdData << 8) | (RepdData << 16) | (RepdData << 24);
  RL(FRAME_BUFFER + RAMDAC_HW_BUG);          /* hw bug  */
  WL(W9100_INDEXLOW, RepdData);
  RL(W9100_PU_CONFIG); /* ensure 5 clocks between DAC accesses */

  RepdData  = ( index & 0xff00 ) >> 8;
  RepdData |= (RepdData << 8) | (RepdData << 16) | (RepdData << 24);
  RL(FRAME_BUFFER + RAMDAC_HW_BUG);          /* hw bug  */
  WL(W9100_INDEXHIGH, RepdData);
  RL(W9100_PU_CONFIG); /* ensure 5 clocks between DAC accesses */

  RepdData  = data;
  RepdData |= (RepdData << 8) | (RepdData << 16) | (RepdData << 24);
  RL(FRAME_BUFFER + RAMDAC_HW_BUG);          /* hw bug  */
  WL(W9100_INDEXDATA, RepdData);
  RL(W9100_PU_CONFIG); /* ensure 5 clocks between DAC accesses */

  return;
}


void VCPBusyWait(void)
        {
        unsigned long busycount;
        unsigned long temp, templ, qptr;

        busycount = 0;
        while(RL(W9130_INTERRUPT_CONTROL) & 0x20000)
                {
                busycount++;
                if (busycount == 0x100000)
                        {
                        busycount = 0;
                        /* Busycount warning from VCPBusyWait */
                        temp = RL(W9130_MEM_CONFIG);
                        temp &= 0xfffffeff;
                        templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);
                        WL(temp, W9130_MEM_CONFIG);
                        templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);
                        WL(0, W9130_QUEUE_POINTER);
                        temp |= 0x100;
                        templ = RL(FRAME_BUFFER+VIDEO_HW_BUG);
                        WL(temp, W9130_MEM_CONFIG);
                        }
                }

        }


void VCPQueueWait(void)
        {
        unsigned int temp;
        unsigned long busycount;

        busycount = 0;
        temp = RL(W9130_INTERRUPT_CONTROL);
        while(!(temp & 0x40000))
                {
                temp = RL(W9130_INTERRUPT_CONTROL);
                if (busycount == 0x100000)
                        {
                        busycount = 0;
                        /* Busycount warning from VCPQueueWait */
                        }
                }

        }


void PrintVCPRegs(void)
  {
    int i;
    ULONG temp, reg_addr;

    /* print out registers */

    reg_addr = W9130_BASE_ADDR;

    for(i = 0; i < 12; i++)
      {
        temp = RL(reg_addr++);
        printf("%08x\n", temp);
      }
    printf("\n");

  }

/***************************************************************************
 *    ClipToScreen() clips the coordinates of the output windows to the    *
 *    screen edges.                                                        *
 ***************************************************************************/

ClipToScreen(DstSpec *Output, SegSpec *ClipWin, ScrnSpec *Screen)
  {
    int i;
    int win_right;      /* window right - non inclusive */
    int win_bottom;     /* window bottom - non inclusive */

    win_right = Output->DstX + Output->DstDx;
    win_bottom = Output->DstY + Output->DstDy;
    if((Output->DstX >= Screen->ScrnWidth) || (win_right <= 0))
        {
        ClipWin->Status = INVALID;   /* Window is completely off screen */
        }
    else
        {
        ClipWin->Status = VALID;
        if(Output->DstX < 0)          /* left edge of window is off screen */
                {
                /* visible part starts at left edge of screen */
                        ClipWin->X0 = 0;
                        /* Compute horizontal offset of visible part */
                        /* with respect to original window */
                        ClipWin->XOff = -Output->DstX;
                }
        else                                    /* left edge of window is on screen */
                {
                ClipWin->X0 = Output->DstX;
                ClipWin->XOff = 0;
                }
        if(win_right >= Screen->ScrnWidth) /* right edge is off screen */
                {
                ClipWin->X1 = Screen->ScrnWidth - 1;
                }
        else                                        /* right edge is on screen */
                {
                ClipWin->X1 = win_right - 1;
                }
        }
    if((Output->DstY >= Screen->ScrnHeight) || (win_bottom <= 0))
        {
        ClipWin->Status = INVALID;      /* Window is completely off screen */
        }
    else
        {
        ClipWin->Status = VALID;
        if(Output->DstY < 0)              /* top edge is off screen */
                {
                /* visible part starts at top edge of screen */
                ClipWin->Y0 = 0;
                        /* Compute vertical offset of visible part */
                ClipWin->YOff = -Output->DstY;
                }
        else                                        /* top edge is on screen */
                {
                ClipWin->Y0 = Output->DstY;
                ClipWin->YOff = 0;
                }
        if(win_bottom >= Screen->ScrnHeight)        /* bottom edge is off screen */
                {
                ClipWin->Y1 = Screen->ScrnHeight - 1;
                }
        else                                                /* bottom edge is on screen */
                {
                ClipWin->Y1 = win_bottom - 1;
                }
        }
  }

