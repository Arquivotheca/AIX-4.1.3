/* @(#)55       1.2  src/bos/diag/tu/sun/pc5xx.h, tu_sunrise, bos411, 9437A411a 5/27/94 13:41:32 */
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/**************************************************************************/
/*  File:  pc5xx.h                                                        */
/**************************************************************************/
/*
        History -
                Written by L.S.     May-June, 1992
                modified for CL5xx  February, 1993

        Description -
        This is the header file for the CL550 JPEG Still Image board.

*/

#define  TRACE          /* this installs tracing code at compile time. */
                        /* a trace file containing register loads for  */
                        /* the CL550 is produced. This is handy for    */
                        /* debugging purposes.                         */

/*  Device Types -
        Device type must be specified before using these init calls. */

#define     CL550   0
#define     CL560   1
#define     CL570   2

/*  Direction Assignments -
        Direction must be specified when building 5xx Q tables or 550 Huffman
        Tables */

#define COMPRESS    0
#define DECOMPRESS  1

#define    CODEC_TIMEOUT    400   /* Timeout of 4 seconds */

/*  Video Sync Modes -
        Specifies VSYNC, HSYNC as output (master) or input (slave) */

#define MASTER  0
#define SLAVE   1


/* CL5xx Register Index  */

#define _Codec              0x0000
#define _HPeriod            0x8000
#define _HSync              0x8004
#define _HDelay             0x8008
#define _HActive            0x800c
#define _VPeriod            0x8010
#define _VSync              0x8014
#define _VDelay             0x8018
#define _VActive            0x801c
#define _DCT                0x8800      /* DCT table */
#define _Init_5             0x8820      /* DCT sync */
#define _Init_6             0x8824      /* DCT sync */
#define _Config             0x9000
#define _Huff_Enable        0x9004
#define _S_Reset            0x9008
#define _Start              0x900c
#define _HV_Enable          0x9010
#define _Flags              0x9014
#define _NMRQ_Mask          0x9018
#define _IRQ1_Mask          0x9018      /* 560/570 only */
#define _DRQ_Mask           0x901c
#define _StartOfFrame       0x9020      /* 550 only */
#define _Version            0x9024
#define _IRQ2_Mask          0x9028      /* 560/570 only */
#define _FRMEND_Enable      0x902c      /* 560/570 only */
#define _Init_1             0x9800      /* zig-zag sync */
#define _Init_2             0x9804      /* zig-zag sync */
#define _HuffTableSequence  0xa000
#define _DPCM_SeqHigh       0xa004
#define _DPCM_SeqLow        0xa008
#define _CoderAttr          0xa00c
#define _CodingIntH         0xa010
#define _CodingIntL         0xa014
#define _CoderSync          0xa020      /* 560/570 only */
#define _CompWordCountH     0xa024      /* 560/570 only */
#define _CompWordCountL     0xa028      /* 560/570 only */
#define _CoderRCActive      0xa02c      /* 560 only */
#define _CoderRCEnable      0xa030      /* 560 only */
#define _CoderRobustness    0xa034      /* 560/570 only */
#define _CoderPadding       0xa038      /* 560/570 only */
#define _DecoderLength      0xa80c
#define _DecoderMarker      0xa810
#define _DecoderResume      0xa814      /* 550 Only */
#define _DecoderDPCM_Reset  0xa818
#define _DecoderCodeOrder   0xa81c      /* 550 Only */
#define _DecoderStart       0xa820      /* 560 Only */
#define _DecoderMismatch    0xa824      /* 560/570 only */
#define _Init_3             0xb600      /* zero packer/unpacker control */
#define _QTableBase         0xb800      /* 5xx Q Table RAM & control regs */
#define _QTable1            0xb800
#define _QTable2            0xb900
#define _QTable3            0xba00
#define _QTable4            0xbb00
#define _QuantABSelect      0xbc00
#define _QuantSync          0xbe00
#define _QuantYCSequence    0xbe08
#define _QuantABSequence    0xbe0c
#define _Matrix00           0xc000      /* Color-space converstion matrix */
#define _Matrix01           0xc004
#define _Matrix02           0xc008
#define _Matrix10           0xc00c
#define _Matrix11           0xc010
#define _Matrix12           0xc014
#define _Matrix20           0xc018
#define _Matrix21           0xc01c
#define _Matrix22           0xc020
#define _VideoLatency       0xc030      /* PBI Control Registers */
#define _HControl           0xc034
#define _VControl           0xc038
#define _LineCount          0xc03c
#define _Init_4             0xcf00      /* Block Storage Sync */
#define _Init_7             0xd400      /* DCT mem */
#define _FIFO_Mem           0xd800      /* FIFO RAM */
#define _FIFO_LevelL        0xda04      /* 560/570 Only */

                /* Huffman Table RAM */
                                                                                /* CL550 only */
#define _HuffYAC_550        0xe000      /* e000-eafc (704 words) */
#define _HuffYDC_550        0xec00      /* ec00-ec7c (32 words) */
#define _HuffCAC_550        0xf000      /* f000-fafc (704 words) */
#define _HuffCDC_550        0xfc00      /* fc00-fc7c (32 words) */

                                                                                /* CL560/570 Only */
#define _HuffYAC_560        0xe000      /* e000-e5fc (384 words) */
#define _HuffYDC_560        0xe600      /* e600-e65c (23 words) */
#define _HuffCAC_560        0xe800      /* e800-edfc (384 words) */
#define _HuffCDC_560        0xee00      /* ee00-ee5c (23 words) */

/*  CL5xx Status Flags -
        These are masks that correspond to the CL5xx status indicators.
        They are used when reading the Flags register or when writing to
        the NMRQ/IRQx and DRQ mask registers.
*/

#define _FIFO_NotFull       0x8000L
#define _FIFO_NotEmpty      0x4000L
#define _Codec_NotBusy      0x2000L
#define _BusError           0x1000L     /* 550 only */
#define _FrameEnd           0x1000L     /* 560/570 only */
#define _MarkerDetected     0x0800L
#define _VSyncActive        0x0400L
#define _VideoNotActive     0x0200L
#define _FIFO_Empty         0x0080L
#define _FIFO_QFull         0x0040L
#define _FIFO_HalfFull      0x0020L
#define _FIFO_3QFull        0x0010L
#define _FIFO_NotQFull      0x0008L
#define _FIFO_NotHalfFull   0x0004L
#define _FIFO_Not3QFull     0x0002L
#define _FIFO_Late          0x0001L

/*  CL5xx Pixel Modes -
        These are the symbols assigned to each of the CL5xx's pixel modes.
        Some of the CL5xx initialization calculations are dependent on pixel
        mode, and this must be specified before initialization.
*/
#define _MONO         0     /* 8-bit gray scale mode */
#define _422          1     /* YUV 4:2:2 mode */
#define _444          2     /* 4:4:4 mode (24-bits, YUV or RGB) */
#define _4444         3     /* 4:4:4:4 mode (32-bits, four component */
#define _RGB_422      4     /* RGB-to-YUV4:2:2 pixel mode (24-bit/pel) */
#define _444_422      5     /* YUV4:4:4-to-YUV4:2:2 pixel */

#define NumPixModes   6

/* huffman table type assignments - used in MakeHuff.c */

#define AC  1
#define DC  0

/**** Globals *******************************************************/

/* from PC550mon.c */
/*extern short   Error;*/
short   Error;

/* from PC550ini.c */
extern short   IOBase, PCdata, PCindex;
extern short   PCcontrol, PCstatus, PCpixel;

extern short DeviceType;
extern short Direction;
extern short PixelMode;
extern short MasterSlave;
extern short ToggleOn;
extern short CL5xxHPeriod, CL5xxHDelay, CL5xxHActive, CL5xxHSync;
extern short CL5xxVPeriod, CL5xxVDelay, CL5xxVActive, CL5xxVSync;
extern short CL5xxImageWidth, CL5xxImageHeight;

#ifdef TRACE
extern short   TraceRegisterLoads;
extern short   TraceHuffmanTableLoads;
extern short   TraceQTableLoads;
extern short   TraceDriverActivity;
extern short   TraceEnable;
extern short   TraceWrites;
extern short   TraceReads;
extern short   TraceCount;
extern FILE    *TraceFile;
extern char    TraceFileName[16];
#endif

/* from MakeHuff.c */
extern unsigned short CL5xxHuffmanYDC[128];   /* CL5xx machine-specific arrays */
extern unsigned short CL5xxHuffmanCDC[128];
extern unsigned short CL5xxHuffmanYAC[1024];
extern unsigned short CL5xxHuffmanCAC[1024];
extern unsigned char Bits_YDC[16];          /* Bits and Values lists */
extern unsigned char Bits_YAC[16];
extern unsigned char Bits_CDC[16];
extern unsigned char Bits_CAC[16];
extern unsigned char Values_YDC[16];
extern unsigned char Values_YAC[256];
extern unsigned char Values_CDC[16];
extern unsigned char Values_CAC[256];

/* from MakeQ.c */
extern unsigned char JPEG_Q_Table1[64];     /* JPEG integer lists 1-255 */
extern unsigned char JPEG_Q_Table2[64];     /* for import/export */
extern unsigned char JPEG_Q_Table3[64];
extern unsigned char JPEG_Q_Table4[64];

extern unsigned short CL5xx_Q_Table1[64];   /* CL5xx Machine-Specific */
extern unsigned short CL5xx_Q_Table2[64];   /* for loading into CL5xx */
extern unsigned short CL5xx_Q_Table3[64];
extern unsigned short CL5xx_Q_Table4[64];

extern unsigned short Default_Y_Visi_Table[64];   /* default Q filters */
extern unsigned short Default_C_Visi_Table[64];   /* from JPEG spec book */
extern unsigned short Zero_Visi_Table[64];        /* "kill-all" table */

/* End of File */
