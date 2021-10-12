/* @(#)56	1.1  src/bos/diag/tu/sun/px2070.h, tu_sunrise, bos411, 9437A411a 3/28/94 17:48:31 */
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
/****************************************************************************
        File:   px2070.h
*****************************************************************************/

#define csupri 0x27c0
#define dbgpri 0x27c0
#define drdpri 0x27c0
#define ocspri 0x27c2
#define istpri 0x27c2
#define rinpri 0x27c4
#define rdtpri 0x27c6
#define mdtpri 0x27c8

#define csusec 0x0290
#define dbgsec 0x0290
#define drdsec 0x0290
#define ocssec 0x0292
#define istsec 0x0292
#define rinsec 0x0294
#define rdtsec 0x0296
#define mdtsec 0x0298


    /* Register Index Constants -- */
#define i_HIU_ISU    0x0001

#define i_VIU_MCR1   0x1000
#define i_VIU_MCR2   0x1001
#define i_VIU_DPC1   0x1002
#define i_VIU_DPC2   0x1003
#define i_VIU_WDT    0x1004
#define i_VIU_TEST   0x1006

#define i_VSU_HSW    0x1100
#define i_VSU_HAD    0x1101
#define i_VSU_HAP    0x1102
#define i_VSU_HP     0x1103
#define i_VSU_VSW    0x1104
#define i_VSU_VAD    0x1105
#define i_VSU_VAP    0x1106
#define i_VSU_VP     0x1107

#define i_IPU1_PIX   0x2100
#define i_IPU1_LIC   0x2101
#define i_IPU1_FLC   0x2102
#define i_IPU1_LIR   0x2103
#define i_IPU1_FIR   0x2104

#define i_IPU1_LRB   0x2200
#define i_IPU1_LRD   0x2201

#define i_IPU1_F1_BASE  0x3000
#define i_IPU1_F2_BASE  0x3100
#define i_MCR_offset    0
#define i_XBF_offset    1
#define i_XBI_offset    2
#define i_XEI_offset    3
#define i_XSF_offset    4
#define i_XSI_offset    5
#define i_YBF_offset    6
#define i_YBI_offset    7
#define i_YEI_offset    8
#define i_YSF_offset    9
#define i_YSI_offset    10
#define i_KFC_offset    11
#define i_MMY_offset    12
#define i_MMV_offset    13
#define i_MMU_offset    14

#define i_IPU1_MCR1  0x3000
#define i_IPU1_XBF1  0x3001
#define i_IPU1_XBI1  0x3002
#define i_IPU1_XEI1  0x3003
#define i_IPU1_XSF1  0x3004
#define i_IPU1_XSI1  0x3005
#define i_IPU1_YBF1  0x3006
#define i_IPU1_YBI1  0x3007
#define i_IPU1_YEI1  0x3008
#define i_IPU1_YSF1  0x3009
#define i_IPU1_YSI1  0x300A
#define i_IPU1_KFC1  0x300B
#define i_IPU1_MMY1  0x300C
#define i_IPU1_MMU1  0x300D
#define i_IPU1_MMV1  0x300E

#define i_IPU1_MCR2  0x3100
#define i_IPU1_XBF2  0x3101
#define i_IPU1_XBI2  0x3102
#define i_IPU1_XEI2  0x3103
#define i_IPU1_XSF2  0x3104
#define i_IPU1_XSI2  0x3105
#define i_IPU1_YBF2  0x3106
#define i_IPU1_YBI2  0x3107
#define i_IPU1_YEI2  0x3108
#define i_IPU1_YSF2  0x3109
#define i_IPU1_YSI2  0x310A
#define i_IPU1_KFC2  0x310B
#define i_IPU1_MMY2  0x310C
#define i_IPU1_MMU2  0x310D
#define i_IPU1_MMV2  0x310E

#define i_IPU2_PIX   0x2300
#define i_IPU2_LIC   0x2301
#define i_IPU2_FLC   0x2302
#define i_IPU2_LIR   0x2303
#define i_IPU2_FIR   0x2304

#define i_IPU2_F1_BASE  0x3200
#define i_IPU2_F2_BASE  0x3300
#define i_IPU2_MCR1  0x3200
#define i_IPU2_XBI1  0x3202
#define i_IPU2_XEI1  0x3203
#define i_IPU2_YBI1  0x3207
#define i_IPU2_YEI1  0x3208

#define i_IPU2_MCR2  0x3300
#define i_IPU2_XBI2  0x3302
#define i_IPU2_XEI2  0x3303
#define i_IPU2_YBI2  0x3307
#define i_IPU2_YEI2  0x3308

#define i_OPU_F1_BASE  0x2A00
#define i_OPU_F2_BASE  0x2B00
#define i_OPU_MCR1  0x2A00
#define i_OPU_XBI1  0x2A02
#define i_OPU_XEI1  0x2A03
#define i_OPU_YBI1  0x2A07
#define i_OPU_YEI1  0x2A08

#define i_OPU_MCR2  0x2B00
#define i_OPU_XBI2  0x2B02
#define i_OPU_XEI2  0x2B03
#define i_OPU_YBI2  0x2B07
#define i_OPU_YEI2  0x2B08
#define i_OPU_PXCNT 0x2B00
#define i_OPU_LNCNT 0x2B01

#define i_SIU_MCR    0x2800
#define i_SIU_FSC    0x2801
#define i_SIU_FOU    0x2802
#define i_SIU_SIM    0x2E00

#define i_ALU_MCR1   0x2900
#define i_ALU_MCR2   0x2901
#define i_ALU_TOP    0x2902
#define i_ALU_AV     0x2903
#define i_ALU_LOPY   0x2904
#define i_ALU_LOPU   0x2905
#define i_ALU_LOPV   0x2906
#define i_ALU_CAY    0x2907
#define i_ALU_CAU    0x2908
#define i_ALU_CAV    0x2909
#define i_ALU_CBY    0x290A
#define i_ALU_CBU    0x290B
#define i_ALU_CBV    0x290C
#define i_ALU_CCY    0x290D
#define i_ALU_CCU    0x290E
#define i_ALU_CCV    0x290F

#define i_OBU0_Base  0x4800
#define i_OBU1_Base  0x4810
#define i_OBU2_Base  0x4820
#define i_OBU3_Base  0x4830
#define i_OBU4_Base  0x4840
#define i_OBU5_Base  0x4850
#define i_OBU6_Base  0x4860
#define i_OBU7_Base  0x4870

#define i_DWU_MCR    0x4100
#define i_DWU_HAC    0x4101
#define i_DWU_MWS    0x4102

#define i_DWU0_Base  0x4400
#define i_DWU1_Base  0x4410
#define i_DWU2_Base  0x4420
#define i_DWU3_Base  0x4430
#define i_DWU_DWZ    0x0000
#define i_DWU_RFX    0x0001
#define i_DWU_LSL    0x0002
#define i_DWU_LSH    0x0003
#define i_DWU_WSX    0x0004
#define i_DWU_WSY    0x0005
#define i_DWU_DSX    0x0006
#define i_DWU_DSY    0x0007

#define i_VPU_MCR    0x2000
#define i_TestReg    0x4001
#define i_MMU_MCR     0x4000

    /* register field constants --------------------- */

    /* TestReg Fields -- */
#define VBC_TESTEN    0x01
#define OPU_TestEn    0x08
#define IP1_TestEn    0x02
#define IP2_TestEn    0x04

    /* ISU Fields -- */
#define IP2S_LC       0x0800
#define IP2S_FC       0x1000
#define IP2S_VS       0x2000

    /* MMU Fields -- */
#define FBWidth32      0x10
#define FBWidth16      0x00
#define FBType64K      0x00
#define FBType128K     0x01
#define FBType256K     0x02
#define FBType1000K    0x03

    /* VPU fields --*/
#define IP1_Disable    0x0000
#define IP1_Start_Next 0x0001
#define IP1_Start_F1   0x0003
#define IP1_F1_Only    0x0002
#define IP1_Start_F2   0x0005
#define IP1_F2_Only    0x0004

#define IP2_Disable    0x0000
#define IP2_Start_Next 0x0010
#define IP2_Start_F1   0x0030
#define IP2_F1_Only    0x0020
#define IP2_Start_F2   0x0050
#define IP2_F2_Only    0x0020

#define OP_Disable     0x0000
#define OP_Start_Next  0x0100
#define OP_Start_F1    0x0300
#define OP_F1_Only     0x0200
#define OP_Start_F2    0x0500
#define OP_F2_Only     0x0400

#define ALU_Disable    0x0000
#define ALU_Enable     0x1000

    /* VSU Fields  --*/
#define VSU_Disable       0x0000
#define VSU_Enable        0x8000
#define VSU_SingleSweep   0x4000

/* Fifo reset fields*/
#define ALL_FIFOS       0x2aaa
#define ALU_FIFOS       0x022a
#define ALUE_FIFO       0x0200
#define ALUC_FIFO       0x0020
#define ALUB_FIFO       0x0008
#define ALUA_FIFO       0x0002
#define IPU1_FIFO       0x2000
#define IPU2_FIFO       0x0800
#define ALUE_FIFO       0x0200
#define OPU_FIFO        0x0080
#define ALUC_FIFO       0x0020
#define ALUB_FIFO       0x0008
#define ALUA_FIFO       0x0002

    /* Sequencer  Fields --*/

#define SIU_Old_ALU_Enable      0x4000
#define SIU_Halt         0
#define SIU_Start1       0x2000
#define SIU_Start2       0x3000
#define SIU_No_Toggle    0x0000
#define SIU_Toggle_Next  0x0400
#define SIU_SI1_to_Fld1  0x0800
#define SIU_SI1_to_Fld2  0x0C00
#define SIU_SI1          0x0001
#define SIU_SI2          0x0020

#define SIU_EXIT         0x100
#define IPU1_to_OB0  0
#define IPU1_to_OB1  1
#define IPU1_to_OB2  2
#define IPU1_to_OB3  3
#define IPU1_to_OB4  4
#define IPU1_to_OB5  5
#define IPU1_to_OB6  6
#define IPU1_to_OB7  7

#define IPU2_to_OB0  0x10
#define IPU2_to_OB1  0x11
#define IPU2_to_OB2  0x12
#define IPU2_to_OB3  0x13
#define IPU2_to_OB4  0x14
#define IPU2_to_OB5  0x15
#define IPU2_to_OB6  0x16
#define IPU2_to_OB7  0x17

#define OB0_to_OP    0x60
#define OB1_to_OP    0x61
#define OB2_to_OP    0x62
#define OB3_to_OP    0x63
#define OB4_to_OP    0x64
#define OB5_to_OP    0x65
#define OB6_to_OP    0x66
#define OB7_to_OP    0x67

#define OB0_to_ALUA  0x30
#define OB1_to_ALUA  0x31
#define OB2_to_ALUA  0x32
#define OB3_to_ALUA  0x33
#define OB4_to_ALUA  0x34
#define OB5_to_ALUA  0x35
#define OB6_to_ALUA  0x36
#define OB7_to_ALUA  0x37

#define OB0_to_ALUB  0x40
#define OB1_to_ALUB  0x41
#define OB2_to_ALUB  0x42
#define OB3_to_ALUB  0x43
#define OB4_to_ALUB  0x44
#define OB5_to_ALUB  0x45
#define OB6_to_ALUB  0x46
#define OB7_to_ALUB  0x47

#define OB0_to_ALUC  0x50
#define OB1_to_ALUC  0x51
#define OB2_to_ALUC  0x52
#define OB3_to_ALUC  0x53
#define OB4_to_ALUC  0x54
#define OB5_to_ALUC  0x55
#define OB6_to_ALUC  0x56
#define OB7_to_ALUC  0x57

#define ALUE_to_OB0  0x20
#define ALUE_to_OB1  0x21
#define ALUE_to_OB2  0x22
#define ALUE_to_OB3  0x23
#define ALUE_to_OB4  0x24
#define ALUE_to_OB5  0x25
#define ALUE_to_OB6  0x26
#define ALUE_to_OB7  0x27

#define offset_0     0x0000
#define offset_1     0x0200
#define offset_M1    0x3e00
#define offset_M2    0x3c00
#define offset_M3    0x3a00
#define offset_M4    0x3800
#define offset_M5    0x3600

    /* Object Buffer Fields  --*/
#define OB_rgb           0x2000
#define OB_lme           0x1000
#define OB_cme           0x0800
#define OB_Disabled      0x0000
#define OB_Locked        0x0040
#define OB_Interlaced_1  0x0100
#define OB_Interlaced_2  0x0140
#define OB_Normal        0x0300
#define OB_LineRep       0x0340
#define OB_Block         0x0380
#define OB_SSM           0x0020
#define OB_YBLT          0x0010
#define OB_XBLT          0x0008
#define OB_No_Copy       0x0000
#define OB_Copy_A        0x0001
#define OB_Copy_B        0x0002
#define OB_Copy_C        0x0003
#define OB_Copy_D        0x0004

    /* VIU WDT Fields  --*/
#define VIU_wdt_enable   0x0400
#define VIU_MField_V1i   0x0000
#define VIU_MField_V1o   0x0800
#define VIU_MField_V2i   0x1000
#define VIU_MField_V2o   0x1800
#define VIU_MField_wdt   0x2000
#define VIU_MField_vsu   0x2800
#define VIU_MField_man   0x3000
#define VIU_Man_Start    0x4000
#define VIU_mfts         0x0800


    /* IP1,IP2 and OP MCR Fields  --*/

#define FieldPol         0x8000
#define Interlaced       0x4000
#define PreScaleEn       0x2000
#define ColorConvert     0x1000
#define LutEnable        0x0800
#define OP_BLANK_OUT     0x0400
#define YScaleEnable     0x0400
#define TagPass          0x0000
#define TagFldId         0x0100
#define TagNotChroma     0x0200
#define TagChroma        0x0300
#define Out422NoTag      0x0000
#define Out422Tag        0x0010
#define OutRGBNoTag      0x0080
#define OutRGBTag        0x0090
#define Out888NoTag      0x00A0
#define Out888Tag        0x00B0
#define Out332           0x00E0
#define In422NoTag       0x0000
#define In422Tag         0x0001
#define In411            0x0002
#define InRGBNoTag       0x0008
#define InRGBTag         0x0009
#define InPseudo         0x000E
#define BlankOutMode     0x0400
#define ZoomEn           0x2000
#define FLC_Enable       0x8000

    /* VIU MCR Fields  --*/
#define VIU_IFP_HI      0x80
#define VIU_IVSP_HI     0x20

#define VIU_InOnly      0x0000
#define VIU_OutOnly     0x0001
#define VIU_iblt        0x0004
#define VIU_iblp        0x0008
#define VIU_ihsp        0x0010
#define VIU_ivsp        0x0020
#define VIU_imss        0x0040
#define VIU_ifldpol     0x0080
#define VIU_oblt        0x0100
#define VIU_oblp        0x0200
#define VIU_ohsp        0x0400
#define VIU_ovsp        0x0800
#define VIU_oss_vp      0x0000
#define VIU_oss_vp_op   0x1000
#define VIU_oss_vsu     0x2000
#define VIU_oss_vsu_op  0x3000
#define VIU_sme         0x8000
#define VIU_ofldpol     0x4000


    /* ALU  MCR Fields  --*/
#define ALU_bitmask          0x8000
#define ALU_NoTag            0x0000
#define ALU_TaggedYUV        0x2000
#define ALU_Tagged555        0x4000
#define ALU_Tagged888        0x6000

#define ALU_Alpha_Reg        0x0000
#define ALU_Alpha_OpC        0x0200
#define ALU_Add              0x0400
#define ALU_Sub              0x0600
#define ALU_Diff             0x0800
#define ALU_Reconst          0x0A00
#define ALU_FrameInterpolate 0x0C00
#define ALU_YSCale           0x1000

#define ALU_Yout_Logic       0x0000
#define ALU_Yout_Arith       0x0080
#define ALU_Yout_Tag         0x0100
#define ALU_Yout_Scale       0x0180

#define ALU_Uout_Logic       0x0000
#define ALU_Uout_Arith       0x0020
#define ALU_Uout_Tag         0x0040
#define ALU_Uout_Scale       0x0060

#define ALU_Vout_Logic       0x0000
#define ALU_Vout_Arith       0x0008
#define ALU_Vout_Tag         0x0010
#define ALU_Vout_Scale       0x0018

#define ALU_Csrc_Reg         0x0000
#define ALU_Csrc_Fifo        0x0004

#define ALU_Bsrc_Reg         0x0000
#define ALU_Bsrc_Fifo        0x0002

#define ALU_Asrc_Reg         0x0000
#define ALU_Asrc_Fifo        0x0001

/* ALU logic ops*/
#define ALU_PassA            0xAAAA
#define ALU_PassB            0xCCCC
#define ALU_PassC            0xF0F0

    /* Datapath Control Fields  --*/

#define IPU1_from_Vp1       0x0000
#define IPU1_from_Vp1_Phase 0x0040
#define IPU1_from_Vp2       0x0080
#define IPU1_from_Vp2_Phase 0x00c0
#define IPU1_from_OP        0x0100
#define IPU1_from_OP_Phase  0x0140

#define IPU2_from_Vp1       0x0000
#define IPU2_from_Vp1_Phase 0x0008
#define IPU2_from_Vp2       0x0010
#define IPU2_from_Vp2_Phase 0x0018
#define IPU2_from_OP        0x0020
#define IPU2_from_OP_Phase  0x0028
#define IPU2_from_Host      0x0030

#define OPU_to_Vp1          0x0000
#define OPU_to_Vp1_Phase    0x0001
#define OPU_to_Vp2          0x0002
#define OPU_to_Vp2_Phase    0x0003
#define OPU_LoopBack        0x0004
#define OPU_LoopBack_Phase  0x0005
#define OPU_to_Host         0x0006

#define VSU_to_Vp1          0x0000
#define VSU_to_Vp1_Phase    0x0200
#define VSU_to_Vp2          0x0400
#define VSU_to_Vp2_Phase    0x0600
#define VSU_LoopBack        0x0800
#define VSU_LoopBack_Phase  0x0A00

    /* Display Window Fields  --*/
#define DW_1XCLOCK              0x8000
#define DW_VSYNC_HIGH           0x1000
#define DW_HSYNC_HIGH           0x0800
#define DW_BLANK_HIGH           0x0400
#define DW_NOT_OCCLUDED         0x0200
#define DW_INTERLACED           0x0100
#define DW_WIN3                 0x0008
#define DW_WIN2                 0x0004
#define DW_WIN1                 0x0002
#define DW_WIN0                 0x0001

#define DW_VGA                  0x0280
#define DW_ZOOM_1X              0x0000

/* 2070 Port Addresses*/
#define PRIM2070PORT0   0x27c0
#define SEC2070PORT0    0x0290

/* Miscellaneous*/
#define CSU_REGP        0x0004
#define CSU_REGS        0x0005
#define PX2070_LOCAL_IF 0x005c
#define PX2070_LOCAL_MASK 0x00de
#define SW_RESET        0x0010
#define POST_REGS       0x0380
#define POST_MASK       0xfc7f
#define OPM_MASK        0x07c0
#define FIFO_ALMOST_FULL        0x2000
#define FIFO_ALMOST_EMPTY       0x4000
#define TIMEOUT         0x7fff

#define DMAOUT 0
#define DMAIN 1
#define Gamma 0x0800

/* Type def's ------------------------------------------------------------*/
typedef enum tagWhichFifo {FIFO_A,FIFO_B,FIFO_C,FIFO_D,FIFO_E,FIFO_F,FIFO_G,FIFO_ALL} WhichFifo;
typedef enum tagWhichObject {OBJECT0,OBJECT1,OBJECT2,OBJECT3,OBJECT4,OBJECT5,OBJECT6,OBJECT7,ALL_OBJECTS} WhichObject;

/* Define the structure for the registers of the 2070 HIU*/
typedef struct tagPX2070HiuUnit
{
  unsigned int PX2070HiuCsu;
  unsigned int PX2070HiuDbg;
  unsigned int PX2070HiuDrd;
  unsigned int PX2070HiuOcs;
  unsigned int PX2070HiuIrq;
  unsigned int PX2070HiuRin;
  unsigned int PX2070HiuRdt;
  unsigned int PX2070HiuMdt;
  unsigned int PX2070HiuImd;
  unsigned int PX2070HiuIsu;
} PX2070HiuUnit;

/* Define the structure for the 2070 VIU registers*/
typedef struct tagPX2070ViuUnit
{
  unsigned int PX2070ViuMcr1;
  unsigned int PX2070ViuMcr2;
  unsigned int PX2070ViuDpc1;
  unsigned int PX2070ViuDpc2;
  unsigned int PX2070ViuWdt;
} PX2070ViuUnit;

/* Define the structure for the 2070 VSU registers*/
typedef struct tagPX2070VsuUnit
{
  unsigned int PX2070VsuHsw;
  unsigned int PX2070VsuHad;
  unsigned int PX2070VsuHap;
  unsigned int PX2070VsuHp;
  unsigned int PX2070VsuVsw;
  unsigned int PX2070VsuVad;
  unsigned int PX2070VsuVap;
  unsigned int PX2070VsuVp;
} PX2070VsuUnit;

/* Define the structure for the 2070 VPU registers*/
typedef struct tagPX2070VpuUnit
{
  unsigned int PX2070VpuMcr;
} PX2070VpuUnit;

/* Define the structure for the 2070 IPU1 registers*/
typedef struct tagPX2070Ipu1Unit
{
  unsigned int PX2070Ipu1Pix;
  unsigned int PX2070Ipu1Lic;
  unsigned int PX2070Ipu1Flc;
  unsigned int PX2070Ipu1Lir;
  unsigned int PX2070Ipu1Fir;
  unsigned int PX2070Ipu1Lrb;
  unsigned int PX2070Ipu1Lrd;
  unsigned int PX2070Ipu1Mcr1;
  unsigned int PX2070Ipu1Xbf1;
  unsigned int PX2070Ipu1Xbi1;
  unsigned int PX2070Ipu1Xei1;
  unsigned int PX2070Ipu1Xsf1;
  unsigned int PX2070Ipu1Xsi1;
  unsigned int PX2070Ipu1Ybf1;
  unsigned int PX2070Ipu1Ybi1;
  unsigned int PX2070Ipu1Yei1;
  unsigned int PX2070Ipu1Ysf1;
  unsigned int PX2070Ipu1Ysi1;
  unsigned int PX2070Ipu1Kfc1;
  unsigned int PX2070Ipu1Mmy1;
  unsigned int PX2070Ipu1Mmu1;
  unsigned int PX2070Ipu1Mmv1;
  unsigned int PX2070Ipu1Mcr2;
  unsigned int PX2070Ipu1Xbf2;
  unsigned int PX2070Ipu1Xbi2;
  unsigned int PX2070Ipu1Xei2;
  unsigned int PX2070Ipu1Xsf2;
  unsigned int PX2070Ipu1Xsi2;
  unsigned int PX2070Ipu1Ybf2;
  unsigned int PX2070Ipu1Ybi2;
  unsigned int PX2070Ipu1Yei2;
  unsigned int PX2070Ipu1Ysf2;
  unsigned int PX2070Ipu1Ysi2;
  unsigned int PX2070Ipu1Kfc2;
  unsigned int PX2070Ipu1Mmy2;
  unsigned int PX2070Ipu1Mmu2;
  unsigned int PX2070Ipu1Mmv2;
} PX2070Ipu1Unit;

/* Define the structure for the 2070 IPU2 registers*/
typedef struct tagPX2070Ipu2Unit
{
  unsigned int PX2070Ipu2Pix;
  unsigned int PX2070Ipu2Lic;
  unsigned int PX2070Ipu2Flc;
  unsigned int PX2070Ipu2Lir;
  unsigned int PX2070Ipu2Fir;
  unsigned int PX2070Ipu2Mcr1;
  unsigned int PX2070Ipu2Xbi1;
  unsigned int PX2070Ipu2Xei1;
  unsigned int PX2070Ipu2Ybi1;
  unsigned int PX2070Ipu2Yei1;
  unsigned int PX2070Ipu2Mcr2;
  unsigned int PX2070Ipu2Xbi2;
  unsigned int PX2070Ipu2Xei2;
  unsigned int PX2070Ipu2Ybi2;
  unsigned int PX2070Ipu2Yei2;
} PX2070Ipu2Unit;

/* Define the structure for the 2070 SIU registers*/
typedef struct tagPX2070SiuUnit
{
  unsigned int PX2070SiuMcr;
  unsigned int PX2070SiuFcs;
  unsigned int PX2070SiuFou;
  unsigned int PX2070SiuSim0;
  unsigned int PX2070SiuSim1;
  unsigned int PX2070SiuSim2;
  unsigned int PX2070SiuSim3;
  unsigned int PX2070SiuSim4;
  unsigned int PX2070SiuSim5;
  unsigned int PX2070SiuSim6;
  unsigned int PX2070SiuSim7;
  unsigned int PX2070SiuSim8;
  unsigned int PX2070SiuSim9;
  unsigned int PX2070SiuSim10;
  unsigned int PX2070SiuSim11;
  unsigned int PX2070SiuSim12;
  unsigned int PX2070SiuSim13;
  unsigned int PX2070SiuSim14;
  unsigned int PX2070SiuSim15;
  unsigned int PX2070SiuSim16;
  unsigned int PX2070SiuSim17;
  unsigned int PX2070SiuSim18;
  unsigned int PX2070SiuSim19;
  unsigned int PX2070SiuSim20;
  unsigned int PX2070SiuSim21;
  unsigned int PX2070SiuSim22;
  unsigned int PX2070SiuSim23;
  unsigned int PX2070SiuSim24;
  unsigned int PX2070SiuSim25;
  unsigned int PX2070SiuSim26;
  unsigned int PX2070SiuSim27;
  unsigned int PX2070SiuSim28;
  unsigned int PX2070SiuSim29;
  unsigned int PX2070SiuSim30;
  unsigned int PX2070SiuSim31;
} PX2070SiuUnit;

/* Define the structure for the 2070 ALU registers*/
typedef struct tagPX2070AluUnit
{
  unsigned int PX2070AluMcr1;
  unsigned int PX2070AluMcr2;
  unsigned int PX2070AluTop;
  unsigned int PX2070AluAv;
  unsigned int PX2070AluLopy;
  unsigned int PX2070AluLopu;
  unsigned int PX2070AluLopv;
  unsigned int PX2070AluCay;
  unsigned int PX2070AluCau;
  unsigned int PX2070AluCav;
  unsigned int PX2070AluCby;
  unsigned int PX2070AluCbu;
  unsigned int PX2070AluCbv;
  unsigned int PX2070AluCcy;
  unsigned int PX2070AluCcu;
  unsigned int PX2070AluCcv;
} PX2070AluUnit;

/* Define the structure for the 2070 OPU registers*/
typedef struct tagPX2070OpuUnit
{
  unsigned int PX2070OpuMcr1;
  unsigned int PX2070OpuXbi1;
  unsigned int PX2070OpuXei1;
  unsigned int PX2070OpuYbi1;
  unsigned int PX2070OpuYei1;
  unsigned int PX2070OpuMcr2;
  unsigned int PX2070OpuXbi2;
  unsigned int PX2070OpuXei2;
  unsigned int PX2070OpuYbi2;
  unsigned int PX2070OpuYei2;
} PX2070OpuUnit;

/* Define the structure for the 2070 OBU registers*/
typedef struct tagPX2070ObuUnit
{
  unsigned int PX2070ObuMcr;
  unsigned int PX2070ObuRfx;
  unsigned int PX2070ObuLsl;
  unsigned int PX2070ObuLsh;
  unsigned int PX2070ObuBsx;
  unsigned int PX2070ObuBsy;
  unsigned int PX2070Obudec;
} PX2070ObuUnit;

/* Define the structure for the 2070 MMU register*/
typedef struct tagPX2070MmuUnit
{
  unsigned int PX2070MmuMcr;
} PX2070MmuUnit;

/* Define the structure for the 2070 DWU registers*/
typedef struct tagPX2070DwuUnit
{
  unsigned int PX2070DwuDzf;
  unsigned int PX2070DwuRfx;
  unsigned int PX2070DwuLsl;
  unsigned int PX2070DwuLsh;
  unsigned int PX2070DwuWsx;
  unsigned int PX2070DwuWsy;
  unsigned int PX2070DwuDsx;
  unsigned int PX2070DwuDsy;
} PX2070DwuUnit;

/* Define the structure for the functional units of the 2070*/
typedef struct tagPX2070Regs
{
  PX2070HiuUnit  PX2070Hiu;
  PX2070ViuUnit  PX2070Viu;
  PX2070VsuUnit  PX2070Vsu;
  PX2070VpuUnit  PX2070Vpu;
  PX2070Ipu1Unit PX2070Ipu1;
  PX2070Ipu2Unit PX2070Ipu2;
  PX2070SiuUnit  PX2070Siu;
  PX2070AluUnit  PX2070Alu;
  PX2070OpuUnit  PX2070Opu;
  PX2070ObuUnit  PX2070Obu0;
  PX2070ObuUnit  PX2070Obu1;
  PX2070ObuUnit  PX2070Obu2;
  PX2070ObuUnit  PX2070Obu3;
  PX2070ObuUnit  PX2070Obu4;
  PX2070ObuUnit  PX2070Obu5;
  PX2070ObuUnit  PX2070Obu6;
  PX2070ObuUnit  PX2070Obu7;
  PX2070MmuUnit  PX2070Mmw;
  unsigned int PX2070DwuMcr;
  unsigned int PX2070DwuHcr;
  PX2070DwuUnit  PX2070Dwu0;
  PX2070DwuUnit  PX2070Dwu1;
  PX2070DwuUnit  PX2070Dwu2;
  PX2070DwuUnit  PX2070Dwu3;
  } PX2070Regs;


/*  prototypes*/

int Test2070MemIO(void);
int Test2070FifoIO(void);
int Test2070DWU(void);
int Test2070ALU(void);
int T2070AluPassThru(void);
int Setup_OB(unsigned int offset,unsigned int mc,unsigned int refx,
              unsigned int lah,unsigned int lal,unsigned int xsz,
              unsigned int ysz,unsigned int decm);
int Setup_DW(unsigned int offset,unsigned int refx,unsigned int lah,
              unsigned int lal,unsigned int xsz,unsigned int xst,
              unsigned int xzm,unsigned int ysz,unsigned int yst,
              unsigned int yzm);
int Setup_VSU(unsigned int hsw,unsigned int had,unsigned int hap,
              unsigned int hp,unsigned int vsw,unsigned int vad,
              unsigned int vap,unsigned int vp,unsigned int vsumode);
int Set_ALU_Constants(unsigned int cay,unsigned int cau,
              unsigned int cav,unsigned int cby,unsigned int cbu,
              unsigned int cbv,unsigned int ccy,unsigned int ccu,
              unsigned int ccv);
int Set_ALU_Logic_Function(unsigned int lopy,unsigned int lopu,
               unsigned int lopv);
int Setup_Ip1(unsigned int offset,unsigned int mcrval,
               unsigned int xbi,unsigned int xbf,unsigned int xei,
               unsigned int xsi,unsigned int xsf,
               unsigned int ybi,unsigned int ybf,unsigned int yei,
               unsigned int ysi,unsigned int ysf);
int Setup_OP(unsigned int offset,unsigned int mcrval,
              unsigned int xbi,unsigned int xei,unsigned int ybi,
              unsigned int yei);
int OBReset(WhichObject E_WhichObject);
int SoftResetDVP(void);
int ResetFifo(unsigned int wData);
int EnableFifo(unsigned int wData);
int IsFifoBlastReady(WhichFifo E_WhichFifo);
int IsFifoEmpty(WhichFifo E_WhichFifo, int *FifoStatus);
int IsFifoFull(WhichFifo E_WhichFifo, int *FifoStatus);
int GetHostPorts(void);
int UpdateGen(void);
int SeqHalt(void);
int SeqManStart(void);
int SetUpMMU(void);
int TestOPUtoIP1(void);
int DisplayBox(int StartX, int StartY, int Width, int Height,
                       unsigned char Color, unsigned char Board);
int SetGraphicsMode(void);
int OpenWindow(int iXStart,int iYStart,int iWidth,int iHeight);
int ExitGraphicsMode(void);

int SetupDMA(int, int, int, int);
int V1FieldIn(int, int, int, int, int, int, int, int);
int V1FieldOut(int, int, int, int, int);

int  ioMDTShortWriteDVP(unsigned int DATA);
int  ioMDTShortReadDVP(unsigned int *DATA);
int  ioMDTReadDVP(unsigned int *DATA);
int  ioMDTWriteDVP(unsigned int DATA);
int  ioRDTWriteDVP(unsigned int ADDR, unsigned int DATA);
int  ioOCSWriteDVP(unsigned int DATA);
int  ioOCSReadDVP(unsigned int *data);
int  ioRINWriteDVP(unsigned int ADDR);
int  ioRINReadDVP(unsigned int *DATA);
int  ioRINDataWriteDVP(unsigned int DATA);
int  ioRINDataReadDVP(unsigned int *DATA);
int  ioRDTReadDVP(unsigned int ADDR, unsigned int *DATA);
