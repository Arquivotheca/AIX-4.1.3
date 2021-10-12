/* @(#)11	1.1  src/bos/diag/tu/sun/pc550.h, tu_sunrise, bos411, 9437A411a 3/30/94 16:46:00 */
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
/*  File:  pc550.h                                                        */
/**************************************************************************/
/*  C-Cube Microsystems                                                   */
/*  1778 McCarthy Blvd.                                                   */
/*  Milpitas, CA 95035  U.S.A.                                            */
/*  Tel: (408) 944-6300  FAX: (408) 944-6314                              */
/**************************************************************************/
/*
        History -
                Written by L.S.     May-June, 1992

        Description -
                This is the header file for the CL550 JPEG image compression board.
*/

#undef  TRACE          /* this installs tracing code at compile time. */
                                                /* a trace file containing register loads for  */
                                                /* the CL550 is produced. This is handy for    */
                                                /* debugging purposes. Remove for release.     */

/* Direction assignments */

#define COMPRESS    0
#define DECOMPRESS  1

/* CL550 Register Index Assignments  */

#define _Codec              0x0000
#define _HPeriod            0x8000
#define _HSync              0x8004
#define _HDelay             0x8008
#define _HActive            0x800c
#define _VPeriod            0x8010
#define _VSync              0x8014
#define _VDelay             0x8018
#define _VActive            0x801c
#define _DCT                0x8800
#define _Init_5             0x8820
#define _Init_6             0x8824
#define _Config             0x9000
#define _Huff_Enable        0x9004
#define _S_Reset            0x9008
#define _Start              0x900c
#define _HV_Enable          0x9010
#define _Flags              0x9014
#define _IRQ1_Mask          0x9018
#define _DRQ_Mask           0x901c
#define _StartOfFrame       0x9020
#define _Version            0x9024
#define _IRQ2_Mask          0x9028
#define _FrmEndEn           0x902c
#define _Init_1             0x9800
#define _Init_2             0x9804
#define _HuffTableSequence  0xa000
#define _DPCM_SeqHigh       0xa004
#define _DPCM_SeqLow        0xa008
#define _CoderAttr          0xa00c
#define _CodingIntH         0xa010
#define _CodingIntL         0xa014
#define _DecLength          0xa80c
#define _DecMarker          0xa810
#define _DecResume          0xa814
#define _DecDPCM_Reset      0xa818
#define _DecCodeOrder       0xa81c
#define _Init_3             0xb600
#define _QTableBase         0xb800
#define _QTable1            0xb800
#define _QTable2            0xb900
#define _QTable3            0xba00
#define _QTable4            0xbb00
#define _QuantABSelect      0xbc00
#define _QuantSync          0xbe00
#define _QuantYCSequence    0xbe08
#define _QuantABSequence    0xbe0c
#define _Matrix00           0xc000
#define _Matrix01           0xc004
#define _Matrix02           0xc008
#define _Matrix10           0xc00c
#define _Matrix11           0xc010
#define _Matrix12           0xc014
#define _Matrix20           0xc018
#define _Matrix21           0xc01c
#define _Matrix22           0xc020
#define _VideoLatency       0xc030
#define _HControl           0xc034
#define _VControl           0xc038
#define _VertLineCount      0xc03c
#define _Init_4             0xcf00
#define _Init_7             0xd400
#define _FIFO_Base          0xd800
#define _HuffTables         0xe000
#define _HuffYAC            0xe000      /* address change for CL560 */
#define _HuffYDC            0xe600
#define _HuffCAC            0xe800
#define _HuffCDC            0xee00

/* #ifdef NOTREMOVE */

/*  CL550 Status Flags -
        These are masks that correspond to the CL550 status indicators.
        They are used when reading the Flags register or when writing to
        the NMRQ and DRQ mask registers.
*/

#define _Flags_FIFO_NotFull       0x8000L
#define _Flags_FIFO_NotEmpty      0x4000L
#define _Flags_Codec_NotBusy      0x2000L
#define _Flags_BusError           0x1000L
#define _Flags_MarkerCode         0x0800L
#define _Flags_VSync              0x0400L
#define _Flags_VideoNotActive     0x0200L
#define _Flags_FIFO_Empty         0x0080L
#define _Flags_FIFO_QFull         0x0040L
#define _Flags_FIFO_HalfFull      0x0020L
#define _Flags_FIFO_3QFull        0x0010L
#define _Flags_FIFO_NotQFull      0x0008L
#define _Flags_FIFO_NotHalfFull   0x0004L
#define _Flags_FIFO_Not3QFull     0x0002L
#define _Flags_FIFO_Late          0x0001L

/*  CL550 Pixel Modes -
        These are the symbols assigned to each of the CL550's pixel modes.
        Some of the CL550 initialization calculations are dependent on pixel
        mode, and this must be specified before initialization.
*/
#define _MONO         0     /* 8-bit gray scale mode */
#define _422          1     /* YUV 4:2:2 mode */
#define _444          2     /* 4:4:4 mode (24-bits, YUV or RGB) */
#define _4444         3     /* 4:4:4:4 mode (32-bits, four component */
#define _RGB_422      4     /* RGB-to-YUV4:2:2 pixel mode (24-bit/pel) */
#define _444_422      5     /* YUV4:4:4-to-YUV4:2:2 pixel */

#define NumPixModes   6

/* huffman table type assignments */

#define _AC  1
#define _DC  0

/**** Globals *******************************************************/

/* from PC550mon.c */
extern short   Error;

/* from PC550ini.c */
extern short   IOBase, PCdata, PCindex;
extern short   PCcontrol, PCstatus, PCpixel;

extern short Direction;
extern short PixelMode;
extern short ToggleOn;
extern short CL550HPeriod, CL550HDelay, CL550HActive, CL550HSync;
extern short CL550VPeriod, CL550VDelay, CL550VActive, CL550VSync;
extern short CL550ImageWidth, CL550ImageHeight;

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
extern unsigned short CL550HuffmanYAC[704]; /* Machine-specific tables */
extern unsigned short CL550HuffmanYDC[32];
extern unsigned short CL550HuffmanCAC[704];
extern unsigned short CL550HuffmanCDC[32];

extern unsigned char Bits_YDC[16];          /* Bits and Values lists */
extern unsigned char Bits_YAC[16];
extern unsigned char Bits_CDC[16];
extern unsigned char Bits_CAC[16];
extern unsigned char Values_YDC[16];
extern unsigned char Values_YAC[256];
extern unsigned char Values_CDC[16];
extern unsigned char Values_CAC[256];

extern short LengthOfValues_YDC;            /* Values list lengths */
extern short LengthOfValues_YAC;
extern short LengthOfValues_CDC;
extern short LengthOfValues_CAC;

extern unsigned short Do_Y_Tables;          /* Flags for Huffman build */
extern unsigned short Do_C_Tables;

/* from MakeQ.c */
extern unsigned char JPEG_Q_Table1[64];     /* JPEG integer lists 1-255 */
extern unsigned char JPEG_Q_Table2[64];     /* for import/export */
extern unsigned char JPEG_Q_Table3[64];
extern unsigned char JPEG_Q_Table4[64];

extern unsigned short CL550_Q_Table1[64];   /* CL550 Machine-Specific */
extern unsigned short CL550_Q_Table2[64];   /* for loading into CL550 */
extern unsigned short CL550_Q_Table3[64];
extern unsigned short CL550_Q_Table4[64];

extern unsigned short Default_Y_Visi_Table[64];   /* default Q filters */
extern unsigned short Default_C_Visi_Table[64];
extern unsigned short Zero_Visi_Table[64];


/* End of File */
