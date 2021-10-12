/* @(#)80	1.1  src/bos/diag/tu/gga/p9_video.h, tu_gla, bos41J, 9515A_all 4/6/95 09:27:13 */
/*
*
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
*
*/

typedef unsigned char      bool;

/* Video Data Area - this structure is used to keep track of the video */
/* specific data that needs to be maintained for the system video      */
/* subsystem. This data is private                                     */

typedef struct {
/* subsystem specific  private */
         bool  TestModeEnabled;      /* 0 = no test at init time   */
                                     /* 1 = test at init time      */
         uint  max_x_pixel;          /* max horizontal resolution  */
         uint  max_y_pixel;          /* max vertical   resolution  */
         uint  x_min;                /* min horz res               */
         uint  y_min;                /* min vert res               */
         uint  curXpos;              /* Current cursor x position  */
         uint  curYpos;              /*    "       "   y    "      */
         uchar CurWid;               /* Cursor Width               */
         uchar CurHt;                /* Cursor Height              */
         uchar CurStyle;             /*  0 - 8x16, 1 - 9x20        */
         uchar CurFontHeight;        /*                            */
         uchar monitorID;            /*                            */
         uchar vmode;                /*  video mode requested      */
         bool  Vram2Meg;             /* 0 = 1 meg of vram          */
                                     /* 1 = 2 meg of vram          */
                                     /* 1 = test at init time      */
         ushort monSpecs[3][8];      /* TBD -  computed horizontal */
                                     /*  and vertical values based */
                                     /*  on the monitor specs      */
         int   Fcolr;                /* Current ForeGround color   */
         int   Bcolr;                /* Current BackGround color   */
         int   bits_per_pel;         /* Bit Map granularity        */
} VDA;

/* Video Request Block - this structure is used by the caller to */
/* to request a video service and the parameter variables        */
/* required to fulfill a particular service. This data is public.*/

typedef struct v{
          uint  ncurXpos;           /* New cursor x position             */
          uint  ncurYpos;           /*  "   "     y    "                 */
          short newVmode;           /* requested video mode              */
          short newCurStyle;        /* requested cursor style for        */
                                    /* 0 - 8x16 font, 1 - 9x20 font      */
          uint  fcolor;             /* requested foreground color        */
          uint  bcolor;             /*     "     background   "          */

/* Parms used for scrolling */
          int   upprLXpos;          /* Upper left window corner X pos    */
          int   upprLYpos;          /*  "      "    "      "    Y pos    */
          int   lowrRXpos;          /* Lower left window corner X pos    */
          int   lowrRYpos;          /*   "     "    "      "    Y pos    */
          int   scrollNum;          /* Number of lines to scroll         */
          int   scrollGran;         /* Granularity of scroll             */
          uchar scrollDir;          /* Scroll Direction                  */

          ulong ManchesterData;     /* 24 bit data for the ICD 2061A     */

   struct sblock *sblk;             /* pointer to a integer string block */
                                    /*    used for DBCS write string     */
   struct cblock *cblk;             /* pointer to an ascii string block  */
                                    /*    used for ASCII write string    */
   struct AFontI *ablk;             /* struct ptr for an XY font bit map */

         uint  wt;            /* Horizontal number of bits */
         uint  ht;            /* Vertical number of bits   */
         uchar *targMap;      /* ptr to array of bytes     */
} VRB;


/******** write string input parm string control block  ****/
/* the following routines must change depending on how we use */
/* the font tables: ascii write string needs a CBLK           */
/*                  dbcs  write string needs a SBLK           */
/* until we can figure out how to manage both                 */
typedef struct sblock{
         uint  *targStr;
         short len;
} SBLK;
/*         SBLK  sblk, *sptr = &sblk;*/

typedef struct cblock{
         uchar  *targStr;
         ushort len;
} CBLK;

/******** blit char input parm stucture font info ****/
/* for integer strings */
typedef struct AFontI{
         uchar x;            /* Horizontal number of bits */
         uchar y;            /* Vertical number of bits   */
         uchar bitmap[1];    /* Dummy array of bytes      */
} AFI;

/* for ascii strings */
typedef struct FontMI{
         uint  wt;            /* Horizontal number of bits */
         uint  ht;            /* Vertical number of bits   */
         uchar *targMap;      /* ptr to array of bytes     */
} FMI;


/* Video Main case label defines           */

#define VID_INIT                 0   /* case label for initialization           */
#define VID_BLIT                 1   /* case label for blit char                */
#define VID_MODE                 2   /* case label for mode set                 */
#define VID_WTTY                 3   /* case label for write tty                */
#define VID_WSTR                 4   /* case label for write string             */
#define VID_FBC                  5   /* case label for set for/background color */
#define VID_CPOS                 6   /* case label for set cursor POS           */

/* Maximum size of a blit and the microcode. */

#define  BLIT_MAX_W         32   /* allows for a 32x64 bit font */
#define  BLIT_MAX_H         64
#define  BLIT_MAX_WORDS    128   /* allows for 128 8x16 characters across the screen */
                                 /* at 1024x768                                      */
#define  BLIT_MAX_GLYPH1   113   /* allows for 113 9x19 characters across the screen */
                                 /* at 1024x768                                      */
#define  BLIT_MAX_GLYPH2   56    /* allows for 56 18x19 characters across the screen */
                                 /* at 1024x768                                      */

/* The following describe the various addresses that we use */
/* to access the Weitek chip.                               */
/* The address is formatted into 2 parts:                   */
/*    part 1 (bits 31-22) system defined                    */
/*    part 2 (bits 21-00) Weitek defined                    */
/*                                 1    1           0       */
/*                                 432 1098 7654 3210       */
/*       e.g. ssss ssss ssww wwww wwww wwww wwww wwww       */
/*            1100 0001 0001 0000 0000 0000 0000 0100       */
/*       is the address for the system configuration reg    */
/*                                                          */
#define P9_SYSCNFG     0xc1100004  /* System configuration information reg */
#define P9_INTERRUPT   0xc1100008  /* Interrupt register used for vblank   */
#define P9_INT_EN      0xc110000C  /* Interrupt enable register            */
#define P9_ALT_WRITEB  0xc1100010  /* Alt Write Bank                       */
#define P9_ALT_READB   0xc1100014  /* Alt Read Bank                        */
#define P9_HRZT        0xc1100108  /* Horizontal length                    */
#define P9_HRZSR       0xc110010C  /* Horizontal sync rising edge          */
#define P9_HRZBR       0xc1100110  /* Horizontal blank rising edge         */
#define P9_HRZBF       0xc1100114  /* Horizontal blank falling edge        */
#define P9_PREHRZC     0xc1100118  /* Horizontal counter preload           */
#define P9_VRTT        0xc1100120  /* Vertical length.                     */
#define P9_VRTSR       0xc1100124  /* Vergical sync rising edge            */
#define P9_VRTBR       0xc1100128  /* Vertical blank rising edge           */
#define P9_VRTBF       0xc110012C  /* Vertical blank falling edge          */
#define P9_PREVRTC     0xc1100130  /* Vertical counter preload             */
#define P9_SRTCTL      0xc1100138  /* Screen repaint timing control        */

#define P9_SRADDR_INC  0xc110013C  /* Screen Address increment             */
#define P9_SRTCTL2     0xc1100140  /* Screen repaint timing control reg 2  */

#define rgb525PalRamWrite     0xc1100200  /* RGB 525 Palette Address Write */
#define rgb525PalData         0xc1100204  /* RGB 525 Palette Data          */
#define rgb525PixelMask       0xc1100208  /* RGB 525 PixelMask             */
#define rgb525PalRamRead      0xc110020C  /* RGB 525 Palette Address Read  */
#define rgb525IndexLow        0xc1100210  /* RGB 525 Index Low             */
#define rgb525IndexHigh       0xc1100214  /* RGB 525 Index High            */
#define rgb525IndexData       0xc1100218  /* RGB 525 Index Data            */
#define rgb525IndexControl    0xc110021C  /* RGB 525 Index Control         */

#define P9_MEMCNFG     0xc1100184  /* Memory configuration                 */
#define P9_RFPER       0xc1100188  /* Refresh period                       */
#define P9_RLMAX       0xc1100190  /* RAS low maximum                      */
#define P9_STATUS      0xc1180000  /* Status register                      */
#define P9_BitBlitcmd  0xc1180004  /* Status/cmd for bit blit              */
#define P9_BitBlit     0xc1180004  /*                                      */
#define P9_Quadcmd     0xc1180008  /* Status/cmd for drawing rectangle     */
#define P9_oor         0xc1180184  /* Status/cmd for drawing rectangle     */
#define P9_QUAD_CMD    0xc1180008  /* Quad command                         */
#define P9_PIXEL1      0xc11e0080  /* Pixel1 command register              */
#define P9_PIXEL8      0xc11e000c  /* Pixel8 command register              */
#define P9_COOR_OFFS   0xc1180190  /* coordinate offsets                   */
#define P9_FORE_GND    0xc1180200  /* Foreground color register            */
#define P9_BACK_GND    0xc1180204  /* Background color register            */
#define P9_PLANE_MASK  0xc1180208  /* Plane Mask def for Pixel1 cmd        */
#define P9_Draw_Mode   0xc118020C  /*                                      */
#define P9_PAT_ORGX    0xc1180210  /* Pattern origin x                     */
#define P9_PAT_ORGY    0xc1180214  /* Pattern origin y                     */
#define P9_RASTER      0xc1180218  /* Raster definition for Pixel1 cmd     */
#define P9_PIX8XCES    0xc118021c  /* Excess data for pixel                */
#define P9_WIN_MIN     0xc1180220  /* Window minimum register              */
#define P9_WIN_MAX     0xc1180224  /* Window maximum register              */

#define P9_COLOR2      0xc1180238  /* COLOR 2                              */
#define P9_COLOR3      0xc118023C  /* COLOR 3                              */

#define P9_PATRN0      0xc1180280  /* Pattern 0                            */
#define P9_PATRN1      0xc1180284  /* Pattern 1                            */
#define P9_PATRN2      0xc1180288  /* Pattern 2                            */
#define P9_PATRN3      0xc118028C  /* Pattern 3                            */

#define P9_X0_COOR     0xc1181008  /* Device coordinate X[0] register      */
#define P9_BitBltX0    0xc1181028  /*                                      */
#define P9_BitBltY0    0xc1181030  /*                                      */
#define P9_X1Y1_COOR   0xc1181058  /* Device coordinate X[1]/Y[1] reg      */
#define P9_BitBltX1    0xc1181068  /*                                      */
#define P9_BitBltY1    0xc1181070  /*                                      */
#define P9_X2_COOR     0xc1181088  /* Device coordinate X[2] register      */
#define P9_BitBltX2    0xc11810A8  /*                                      */
#define P9_BitBltY2    0xc11810B0  /*                                      */
#define P9_Y3_COOR     0xc11810D0  /* Device coordinate Y[3] register      */
#define P9_BitBltX3    0xc11810E8  /*                                      */
#define P9_BitBltY3    0xc11810F0  /*                                      */
#define P9_2DmetaX     0xc1181318  /*                                      */
#define P9_2DmetaY     0xc1181310  /*                                      */
#define P9_POINT_XY    0xc1181218  /* Point         16 bit x and y         */
#define P9_LINE_XY     0xc1181258  /* Line          16 bit x and y         */
#define P9_TRINGL_XY   0xc1181298  /* Triangle      16 bit x and y         */
#define P9_QUADRI_XY   0xc11812D8  /* Quadrilateral 16 bit x and y         */
#define P9_RECT_XY     0xc1181318  /* Rectangle     16 bit x and y         */
#define P9_NAT_VRAM    0xc1200000  /* Weitek Native memory area            */
#define P9000AddrBase  0x00000001  /* PCI base address value - little endian format */
#define PCIcardEnable 0x0300       /* PCI card enable mask   - little endian format */

/* for meta draw mnager case statement */
#define POINT          0x00000001
#define LINE           0x00000002
#define TRIANGLE       0x00000003
#define QUADRILATERAL  0x00000004
#define RECTANGLE      0x00000005

static int             f_color = 0x00;
static int             b_color = 0xff;
#define WEITEK_P9000 1
#define WEITEK_P9100 2
#define BT_485   1
#define RGB_525  2

/* These defines are used to display the pixels on the screen.  The         */
/* first three are used to set up the Pixel1 command, and the last          */
/* two define the two colors we use (duh).                                  */

#define PLANE_VAL        0xffffffff   /* Plane Mask enable value            */
#define P9100_PLANE_VAL  0xffffffff   /* Plane Mask enable value            */

#ifndef BLACK
#define BLACK          0x00000000   /* Black color definition               */
#endif

#ifndef WHITE
#define WHITE          0xff000000   /* Green color definition               */
#endif

/* These defines are used as mask values when testing out the p9000         */
#define BITS_7_0     0xff           /* mask out bits 31 thru 20             */
#define BITS_0_7     0x000000ff     /* mask out bits  0 thru  7             */

#define BITS_31_20   0xfff00000     /* mask out bits 31 thru 20             */
#define BITS_31_20le 0x00000fff     /* mask out bits 31 thru 20 le          */

#define BITS_25_9    0x03fffe00     /* mask out bits 25 thru  9 le          */
#define BITS_25_9le  0x00feff03     /* mask out bits 25 thru  9             */

#define BITS_17_0    0xff030000      /* mask out bits  0 thru 17            */
#define BITS_17_0le  0x000003ff      /* mask out bits  0 thru 17            */

/* The following describe the various addresss that we use  */
/* to access the Bt485 RAMDAC via the p9000                 */
/*  *** but used in this implemetation ***                  */
/*       e.g. ssss ssss ssww wwww wwww wwww wwww wwww       */
/*            0000 1001 0001 0000 0000 0010 0000 0000       */
/*            0000 1001 0001 0000 0000 0010 0000 0100       */
/*            0000 1001 0001 0000 0000 0010 0000 1000       */
/*            0000 1001 0001 0000 0000 0010 0000 1100       */

/* The following describe the addresss that are defined     */
/* to access the BT485 internal registers                   */
#define BTPalCurRamWrite        0xC1080000
#define BTPalData               0xC1080004
#define BTPixelMask             0xC1080008 /* palette regs              */
#define BTPalCurRamRead         0xC108000c
#define BTCurOverScanClrWrite   0xC1080010
#define BTCurOverScanClrData    0xC1080014
#define BTCommandReg0           0xC1080018 /* Command and Overscan regs */
#define BTCurOverScanClrRead    0xC108001c
#define BTCommandReg1           0xC1080020 /* More command regs         */
#define BTCommandReg2           0xC1080024
#define BTStatus                0xC1080028
#define BTCurRamData            0xC108002c
#define BTCurXPosLo             0xC1080030 /* Cursor regs               */
#define BTCurXPosHi             0xC1080034
#define BTCurYPosLo             0xC1080038
#define BTCurYPosHi             0xC108003c
#define BTCommandReg3           0xC1080028 /* accessed thru cr0 - SAME AS status */

/* these are for the weitek pci card */
#define RAMWRITE 0x08

/* The following describe the addresss that are defined     */
/* to access the ICD2061a internal registers                */
#define FWSense                0xC1000004L
#define FWControl              0xC1000000L

#define seq_outcntl_index     0x12

/* The following describe defines used when accessing       */
/* the ICD2061a internal registers                          */

#define ICD_Clk       0x02
#define ICD_Data      0x01
#define ICD_Clk_Data  0x03

#define DICD_Clk      0x04   /* Diamonds definition of the 3 above */
#define DICD_Data     0x08
#define DICD_Clk_Data 0x0c

#define IC_REG2       0x400000L
#define VLPolarity    0x20
#define MISCC         ICD_Clk
#define MISCD         ICD_Data
#define MCD           ICD_Clk_Data

/* Memory test defines                                                      */
#define WEI_NAT_MEM    0x001FFFFCL   /* Size of Native memory to test        */
#define WGA_LIN_MEM    0x001FFFFCL   /* Size of Linear Memory to test        */
#define WGA_XY_MEM     0x007FFFF0L   /* Size of XY Memory to test            */
#define WGA_MEM_DATA1  0x00010001L   /* Pattern written to memory            */
#define WGA_MEM_DATA2  0xAA55A5A5L   /* Pattern written to memory            */
#define WGA_MEM_DATA3  0x5A5A55AAL   /* Pattern written to memory            */
#define WGA_MEM_DATA4  0             /* Pattern written to memory            */
/* ************** Miscellaneous **************** */


/* ************************************** */
#define PASS              0
#define FAIL              1         /* null pointer passed                  */
#define  FAILnptr         2         /* bad coordinates specified            */
#define  FAILhwbnds       3
#define BAD_CMD           2         /* Bad Command error                    */
/* #define P9100Busy       0x40000000L drawing engine busy flag             */
#define P9100Busy       0x00000040L /* drawing engine busy flag             */
/* #define P9100QblitBusy  0x80000000L QuadBlit busy mask flag              */
#define P9100QblitBusy  0x00000080L /* QuadBlit busy mask flag              */
/* #define PatOvrsEnbl     0x00030000L Pattern and Oversized enable mask    */
#define PatOvrsEnbl     0x00000300L /* Pattern and Oversized enable mask    */
/* #define PatrnDisable    0x00010000L Pattern disable mask                 */
#define PatrnDisable    0x00000100L /* Pattern disable mask                 */
#define OvrsEnbl     0x00000200L    /* Oversized enable mask    */

/* minterm MASK values                                                      */
/* #define  mint_F_MASK     0x0000ff00  Foreground                           */
#define  mint_F_MASK     0x00ff0000 /* Foreground                           */
/* #define  mint_B_MASK     0x0000f0f0  Background                           */
#define  mint_B_MASK     0xf0f00000 /* Background                           */
/* #define  mint_S_MASK     0x0000cccc  Source                               */
#define  mint_S_MASK     0xcccc0000 /* Source                               */
/* #define  mint_D_MASK     0x0000aaaa  Destination                          */
#define  mint_D_MASK     0xaaaa0000 /* Destination                          */

/* Video Memory test return codes */
#define  BAD_VRAM         3         /* Bad Video memory                     */
#define  BAD_XY           4         /* Bad Video memory using XY register   */
#define  BAD_NAT          6         /* Bad Video memory in native mode      */

/* Video DAC test return codes    */
#define  BAD_DAC          5         /* Bad Video memory                     */

/* Video Modeset return codes     */
#define  GOOD_MODE        0         /* Bad Video memory                     */
#define  BAD_MODE         1         /* Bad Video memory                     */

/* Monitor ID case values         */
#define  Saturn14V        0
#define  Saturn15V        1
#define  Saturn17V        2

/* Monitor resolution case values */
#define  R1280x1024       0
#define  R1024x768        1
#define  R640x480         2

/* Monitor resolution video controller values */
#define  W1600x1280       0xff043f06L
#define  W1280x1024       0xff03ff04L
#define  W1024x768        0xff02ff03L
#define  W640x480         0xdf017f02L

/* Video Enable flags             */
#define  disableVid       0xdFFFFFfFL
#define  enableVid        0x20000000L

/* frequency mask for white oak this will change for PCI */
#define  FreqSelectMask   0x07FFFFFFffL

/* System Configuration register Horizontal resolution defines    */
/* set horizontal resolution for drawing engine in sysconfig reg  */
/*  3           2         1                                       */
/* 1098 7654 3210 9876 5432                                       */
/* 0000 0000 0xxx yyyz zz00 0000 0000 0000                        */
/* 0000 0100 0010 0010 0000 1000 0101 0110                        */
/*            000 1001 10  ---------------- renders 1280          */
/* errata     110 1000 00                                         */

/*            000 0001 10  ---------------- renders 1024 default  */
/* errata     101 0000 00  ---------------- renders 1024 default  */
/*            111 0000 00  ----------------    "    2048 = WGA    */
/* errata     000 0001 11  ----------------    "    2048 = WGA    */
/* #define  NHres2048           0x00703000L                       */
#define  NHres2048           0x00307000L
/*            101 0110 00  ----------------    "     640         */
/* eratta     000 0111 01  ----------------    "     640         */

#define  NHres1280        0x00006800
#define  P9100_NHres1280  0x00006808

#define  NHres1024        0x00306000L
#define  P9100_NHres1024  0x00306008L

#define  NHres640         0x00400700L
#define  P9100_NHres640   0x00400708L
#define  DP9100_NHres640  0x00305608L  /* 085630000 */

/* Screen Repaint Timing Control defines  - may change after spec shows up   */
#define  SCRRTdefaults    0x1C5;        /* bit 8: Select Internal VSYNC- = 1 */
                                        /* bit 7: Select Internal HSYNC- = 1 */
                                        /* bit 6: Select composite sync  = 1 */
                                        /* bit 5: Disable video is off   = 0 */
                                        /* bit 4: Screen Repaint normal  = 0 */
                                        /* bit 3: Select buffer 0        = 0 */
                                        /* bit 2-0: QSF counter          = 1 */
                                        /*                                 0 */
                                        /*                                 1 */

/* Miscellaneous defines */
#define  ZeroOrigin       0x00000000
#define  RedMask          0xe0
#define  GrnMask          0x1c
#define  BluMask          0x03
#define  TRUE             1
#define  FALSE            0
#define  SpecIndex        vptr->monitorID*vrb->newVmode
#define  C8x16Cursor      0
#define  C9x20Cursor      1
#define  MASK1 0x04
#define  MASK2 0x20
#define  NEGATIVE         0

/* PCI stuff */
#pragma options align=packed
typedef struct {
       short vendorID;
       short deviceID;
       short command;
       short status;
       char  revID;
       short ClassLo;
       char  ClassHi;
       char  CacheLineSize;
       char  LatencyTimer ;
       char  HdrType;
       char  BIST;
       int   BaseAddressRegs[6];
       long  reserved1;
       long  reserved2;
       long  ExpanEpromAddr;
       long  reserved3;
       long  reserved4;
       char  interruptLine;
       char  interruptPin;
       char  Min_grant;
       char  Max_latency;
} PSH;
typedef struct {
       uint  vendorID;           /* 0x00  */
       uint  command;            /* 0x04  */
       uint  revID;              /* 0x08  */
       uint  CacheLineSize;      /* 0x0c  */
       uint  BaseAddressRegs[6]; /* 0x10  */
       long  reserved1;          /* 0x28  */
       long  reserved2;          /* 0x2c  */
       long  ExpanEpromAddr;     /* 0x30  */
       long  reserved3;          /* 0x34  */
       long  reserved4;          /* 0x38  */
       uint  interruptLine;      /* 0x3c  */
       uchar Config64;           /* 0x40  */
       uchar Config65;           /* 0x41  */
       uchar Config66;           /* 0x42  */
} PSH2;

#pragma options align=reset

#define DFWControl             &(psh_ptr2->Config66)

/*** funtion prototypes ***/
uint   modeset(int);
void   setPixelClk(VRB *);
uint   init_P9100_base(void);
void   setRefClk(ulong, int);
void   SetRGB525PixelClk(void);
void   enableRGB525(void);
void   p9_rgb525_write(ushort, uchar);
uint   p9_rgb525_read(ushort);

extern char  colors[256*3];

#define CURSOR_NORMAL_DBCS 5
#define CURSOR_ARROW 7

#define P9_QUERY_FUNC_TAB              0x5a
#define P9_QUERY_PCI_SLOT              0x5b
#define P9_QUERY_VDA_PTR               0x5c
#define P9_QUERY_META_ROUTINE          0x5d
#define P9_QUERY_RESIDUAL_DATA_ROUTINE 0x5e
#define P9_QUERY_DEVICE                0x6a
#define P9_QUERY_DAC_DEVICE            0x6b
#define P9_QUERY_DEVICE_MEM            0x6c
#define P9_QUERY_RGB525_READ_ROUTINE   0x6d
#define P9_QUERY_RGB525_WRITE_ROUTINE  0x6e

#define P9_VENDOR_ID     0x00
#define P9_DEVICE1       0x00131014
#define P9_DEVICE2       0x01131014
#define P9_DEVICE3       0x9100100e

/*                                                          */
/* The following describe the various addresses that we use */
/* to access the Weitek p9100.                              */
/* The address is formatted into 2 parts:                   */
/*    part 1 (bits 31-15) prefix                            */
/*    part 2 (bits 14-00) Weitek defined                    */
/*            3                   1                 0       */
/*            1                   5                 0       */
/*       e.g. pppp pppp pppp pppp pwww wwww wwww wwww       */
/*            1100 0001 0000 0000 0000 0000 0000 0100       */
/*       is the address for the system configuration reg    */
/*                                                          */
/*  Notice that the RAMDAC access is through the 9100 also  */
/*                                                          */
#define P9100_SYSCNFG     (BE_prefix | 0x000004)
#define P9100_INTERRUPT   (BE_prefix | 0x000008)
#define P9100_INT_EN      (BE_prefix | 0x00000c)
#define P9100_ALT_W       (BE_prefix | 0x000010)
#define P9100_ALT_R       (BE_prefix | 0x000014)
#define P9100_HRZC        (BE_prefix | 0x000104)
#define P9100_HRZT        (BE_prefix | 0x000108)
#define P9100_HRZSR       (BE_prefix | 0x00010c)
#define P9100_HRZBR       (BE_prefix | 0x000110)
#define P9100_HRZBF       (BE_prefix | 0x000114)
#define P9100_PREHRZC     (BE_prefix | 0x000118)
#define P9100_VRTC        (BE_prefix | 0x00011c)
#define P9100_VRTT        (BE_prefix | 0x000120)
#define P9100_VRTSR       (BE_prefix | 0x000124)
#define P9100_VRTBR       (BE_prefix | 0x000128)
#define P9100_VRTBF       (BE_prefix | 0x00012c)
#define P9100_PREVRTC     (BE_prefix | 0x000130)
#define P9100_SRADDR      (BE_prefix | 0x000134)
#define P9100_SRTCTL      (BE_prefix | 0x000138)
#define P9100_SRADDR_INC  (BE_prefix | 0x00013c)
#define P9100_SRTCTL2     (BE_prefix | 0x000140)
#define P9100_MEMCNFG     (BE_prefix | 0x000184)
#define P9100_RFPER       (BE_prefix | 0x000188)
#define P9100_RFCOUNT     (BE_prefix | 0x00018c)
#define P9100_RLMAX       (BE_prefix | 0x000190)
#define P9100_RFCUR       (BE_prefix | 0x000194)
#define P9100_PU_CONFIG   (BE_prefix | 0x000198)
#define P9100_PalCurRamW  (BE_prefix | 0x000200)
#define P9100_PalData     (BE_prefix | 0x000204)
#define P9100_PixelMask   (BE_prefix | 0x000208)
#define P9100_PalCurRamR  (BE_prefix | 0x00020c)
#define P9100_IndexLow    (BE_prefix | 0x000210)
#define P9100_IndexHigh   (BE_prefix | 0x000214)
#define P9100_IndexData   (BE_prefix | 0x000218)
#define P9100_IndexContr  (BE_prefix | 0x00021c)
#define P9100_PIXEL8      (BE_prefix | 0x06200c)
#define P9100_NEXT_PIX    (BE_prefix | 0x062014)
#define P9100_PIXEL1      (BE_prefix | 0x062080)
#define P9100_OOR         (BE_prefix | 0x002184)
#define P9100_CINDEX      (BE_prefix | 0x00218c)
#define P9100_COOR_OFFS   (BE_prefix | 0x002190)
#define P9100_P_WIN_MIN   (BE_prefix | 0x002194)
#define P9100_P_WIN_MAX   (BE_prefix | 0x002198)
#define P9100_YCLIP       (BE_prefix | 0x0021a0)
#define P9100_XCLIP       (BE_prefix | 0x0021a4)
#define P9100_XEDGE_LT    (BE_prefix | 0x0021a8)
#define P9100_XEDGE_GT    (BE_prefix | 0x0021ac)
#define P9100_YEDGE_LT    (BE_prefix | 0x0021b0)
#define P9100_YEDGE_GT    (BE_prefix | 0x0021b4)
#define P9100_BACK_GND    (BE_prefix | 0x002200)
#define P9100_FORE_GND    (BE_prefix | 0x002204)
#define P9100_PLANE_MASK  (BE_prefix | 0x002208)
#define P9100_Draw_Mode   (BE_prefix | 0x00220c)
#define P9100_PATOX       (BE_prefix | 0x002210)
#define P9100_PATOY       (BE_prefix | 0x002214)
#define P9100_RASTER      (BE_prefix | 0x002218)
#define P9100_PIX8XCES    (BE_prefix | 0x00221c)
#define P9100_WIN_MIN     (BE_prefix | 0x002220)
#define P9100_WIN_MAX     (BE_prefix | 0x002224)
#define P9100_COLOR2      (BE_prefix | 0x002238)
#define P9100_COLOR3      (BE_prefix | 0x00223c)
#define P9100_PATTERN0    (BE_prefix | 0x002280)
#define P9100_PATTERN1    (BE_prefix | 0x002284)
#define P9100_PATTERN2    (BE_prefix | 0x002288)
#define P9100_PATTERN3    (BE_prefix | 0x00228c)
#define P9100_B_WIN_MIN   (BE_prefix | 0x0022a0)
#define P9100_B_WIN_MAX   (BE_prefix | 0x0022a4)
#define P9100_X0Y0        (BE_prefix | 0x003008)
#define P9100_X1Y1        (BE_prefix | 0x003048)
#define P9100_X2Y2        (BE_prefix | 0x003088)
#define P9100_X3Y3        (BE_prefix | 0x0030c8)
#define P9100_X0Y0R       (BE_prefix | 0x003010)
#define P9100_X1Y1R       (BE_prefix | 0x003050)
#define P9100_X2Y2R       (BE_prefix | 0x003090)
#define P9100_X3Y3R       (BE_prefix | 0x0030d0)
#define P9100_STATUS      (BE_prefix | 0x002000)
#define P9100_POINT       (BE_prefix | 0x003208)
#define P9100_LINE        (BE_prefix | 0x003248)
#define P9100_TRIANGLE    (BE_prefix | 0x003288)
#define P9100_QUADR       (BE_prefix | 0x0032c8)
#define P9100_RECTANGL    (BE_prefix | 0x003308)
#define P9100_POINTR      (BE_prefix | 0x003200)
#define P9100_LINER       (BE_prefix | 0x003240)
#define P9100_TRIANGLER   (BE_prefix | 0x003280)
#define P9100_QUADRR      (BE_prefix | 0x0032c0)
#define P9100_RECTANGLR   (BE_prefix | 0x003300)
#define P9100_FRAME_BUF   (BE_prefix | 0x800000)
#define P9100_STATUS      (BE_prefix | 0x002000)
#define P9100_BitBlitcmd  (BE_prefix | 0x002004)
#define P9100_BitBlit     (BE_prefix | 0x002004)
#define P9100_Quadcmd     (BE_prefix | 0x002008)

#define P9100_X0_COOR     (BE_prefix | 0x003008) /* Device coordinate X[0] register */
#define P9100_BitBltX0    (BE_prefix | 0x003028) /*                                 */
#define P9100_BitBltY0    (BE_prefix | 0x003030) /*                                 */
#define P9100_X1Y1_COOR   (BE_prefix | 0x003058) /* Device coordinate X[1]/Y[1] reg */
#define P9100_BitBltX1    (BE_prefix | 0x003068) /*                                 */
#define P9100_BitBltY1    (BE_prefix | 0x003070) /*                                 */
#define P9100_X2_COOR     (BE_prefix | 0x003088) /* Device coordinate X[2] register */
#define P9100_BitBltX2    (BE_prefix | 0x0030A8) /*                                 */
#define P9100_BitBltY2    (BE_prefix | 0x0030B0) /*                                 */
#define P9100_Y3_COOR     (BE_prefix | 0x0030D0) /* Device coordinate Y[3] register */
#define P9100_BitBltX3    (BE_prefix | 0x0030E8) /*                                 */
#define P9100_BitBltY3    (BE_prefix | 0x0030F0) /*                                 */

#define P9100_POINT_XY    (BE_prefix | 0x003218) /* Point         16 bit x and y    */
#define P9100_LINE_XY     (BE_prefix | 0x003258) /* Line          16 bit x and y    */
#define P9100_TRINGL_XY   (BE_prefix | 0x003298) /* Triangle      16 bit x and y    */
#define P9100_QUADRI_XY   (BE_prefix | 0x0032D8) /* Quadrilateral 16 bit x and y    */
#define P9100_RECT_XY     (BE_prefix | 0x003318) /* Rectangle     16 bit x and y    */

#define FRAME_BUFFER         (BE_prefix | 0x800000)

/* PCI configuration register offsets for 9100 */

#define PCIslot1      0x80802000L
#define PCIslot2      0x80804000L

#define VEND_ID_LOW          0
#define VEND_ID_HIGH         1
#define DEVICE_ID_LOW        2
#define DEVICE_ID_HIGH       3
#define DAC_MEM_IO           4
#define STATUS               7
#define REVISION_ID          8
#define VGA_PRESENT         10
#define CLASS_CODE          11
#define WBASE               19
#define ROM_ENABLE          48
#define ROM_BASE_0          49
#define ROM_BASE_8_1        50
#define ROM_BASE_16_9       51
#define BUS_CFGBA_EEDAIN    64
#define MODE_NATIVE_bBH     65
#define CKSEL_VCEN          66

