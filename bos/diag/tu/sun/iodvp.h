/* @(#)51	1.1  src/bos/diag/tu/sun/iodvp.h, tu_sunrise, bos411, 9437A411a 3/28/94 17:48:10 */
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
        File:   iodvp.h
*****************************************************************************/

/**********************************************************
*       Company:    Pixel Semiconductor, Inc.
*       Project:    2070/2080 Drivers
*       File Name:  iodvp.h
*
*       Module Abstract:
*
***********************************************************
*       Author: Kyle Pratt
*       Date:   9/30/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
#ifndef IODVP
#define IODVP

#ifdef _WINDOWS
#define PTR_TYPE huge
#else
#define PTR_TYPE far
#endif

typedef enum tagPortType {MEMORY_MAPPED,IO_MAPPED,UNDEFINED} PortType;
typedef enum tagSwMode {REAL_MODE,PROTECTED_MODE} SwMode;

/* Define the Global Descriptor Table for INT 15 calls  */
typedef struct tagDescriptor
{
  unsigned short wLimit;
  unsigned short wBaseLow;
  unsigned char  bBaseMed;
  unsigned char  bAccess;
  unsigned char  bXAccessLimit;
  unsigned char  bBaseHigh;
} DESCRIPTOR;

/* Define the structure for the ports of the 2070  */
typedef struct tagPX2070Ports
{
  unsigned long PX2070Port0;
  unsigned long PX2070Port1;
  unsigned long PX2070Port2;
  unsigned long PX2070Port3;
  unsigned long PX2070Port4;
  unsigned long PX2070RamType;
} PX2070Ports;



/* define's  --------------------------------------------  */
#define PX_PORT_TYPES   2
#define MAXLINELENGTH   128
#define MAX2070COUNT    2
#define MAX2080COUNT    2
#define MAX484COUNT     1
#define MAXVIDMODCOUNT  2

#define PX2080BASE      MAX2070COUNT
#define BT484BASE       (MAX2070COUNT + MAX2080COUNT)
#define ELEMENTBT484    (MAX2070COUNT + MAX2080COUNT)
#define VIDMODBASE      (ELEMENTBT484 + MAX484COUNT)

#define MAXCONFIGARRAY  (MAX2070COUNT + MAX2080COUNT + MAX484COUNT + MAXVIDMODCOUNT)

#define FIFO_ALMOST_FULL        0x2000
#define FIFO_ALMOST_EMPTY       0x4000
/*
#define TIMEOUT         0x7fff
*/

#define DRE_SET         0x200
#define DRE_CLEAR       0xfdff
#define SRC_SET         0x800
#define SRC_CLEAR       0xf7ff
#define MDE_SET         0x400
#define MDE_CLEAR       0xfbff

#define MAX_IPU1_LUT    768
#define OPU_to_Host         0x0006
#define IPU2_from_Host      0x0030

/* Define the index values for the various registers in the 2070  */

/* HIU registers are the ports of the 2070 so index values are dont cares.  */
/*
#define HIU_CSU         NULL
#define HIU_DBG         NULL
#define HIU_DRD         NULL
#define HIU_OCS         NULL
#define HIU_IRQ         NULL
#define HIU_RIN         NULL
#define HIU_RDT         NULL
#define HIU_MDT         NULL
*/
#define HIU_IMD         0x0000
#define HIU_ISU         0x0001
/* VIU register indices of the 2070  */
#define MAX_VIU_INDX1   5
#define VIU_INDEX_1     0x1000
#define VIU_MCR1        0x1000
#define VIU_MCR2        0x1001
#define VIU_DPC1        0x1002
#define VIU_DPC2        0x1003
#define VIU_WDT         0x1004
#define VIU_TEST        0x1006

/* VSU register indices of the 2070  */
#define MAX_VSU_INDX1   8
#define VSU_INDEX_1     0x1100
#define VSU_HSW         0x1100
#define VSU_HAD         0x1101
#define VSU_HAP         0x1102
#define VSU_HP          0x1103
#define VSU_VSW         0x1104
#define VSU_VAD         0x1105
#define VSU_VAP         0x1106
#define VSU_VP          0x1107

/* VPU register index of the 2070  */
#define MAX_VPU_INDX1   1
#define VPU_INDEX_1     0x2000
#define VPU_MCR         0x2000

/* IPU1 register indices of the 2070  */
#define MAX_IPU1_INDX1  5
#define MAX_IPU1_INDX2  2
#define MAX_IPU1_INDX3  15
#define MAX_IPU1_INDX4  15
#define IPU1_INDEX_1    0x2100
#define IPU1_INDEX_2    0x2200
#define IPU1_INDEX_3    0x3000
#define IPU1_INDEX_4    0x3100
#define IPU1_PIX        0x2100
#define IPU1_LIC        0x2101
#define IPU1_FLC        0x2102
#define IPU1_LIR        0x2103
#define IPU1_FIR        0x2104
#define IPU1_LRB        0x2200
#define IPU1_LRD        0x2201
#define IPU1_MCR1       0x3000
#define IPU1_XBF1       0x3001
#define IPU1_XBI1       0x3002
#define IPU1_XEI1       0x3003
#define IPU1_XSF1       0x3004
#define IPU1_XSI1       0x3005
#define IPU1_YBF1       0x3006
#define IPU1_YBI1       0x3007
#define IPU1_YEI1       0x3008
#define IPU1_YSF1       0x3009
#define IPU1_YSI1       0x300a
#define IPU1_KFC1       0x300b
#define IPU1_MMY1       0x300c
#define IPU1_MMU1       0x300d
#define IPU1_MMV1       0x300e
#define IPU1_MCR2       0x3100
#define IPU1_XBF2       0x3101
#define IPU1_XBI2       0x3102
#define IPU1_XEI2       0x3103
#define IPU1_XSF2       0x3104
#define IPU1_XSI2       0x3105
#define IPU1_YBF2       0x3106
#define IPU1_YBI2       0x3107
#define IPU1_YEI2       0x3108
#define IPU1_YSF2       0x3109
#define IPU1_YSI2       0x310a
#define IPU1_KFC2       0x310b
#define IPU1_MMY2       0x310c
#define IPU1_MMU2       0x310d
#define IPU1_MMV2       0x310e

/* IPU2 register indices for the 2070  */
#define MAX_IPU2_INDX1  5
#define MAX_IPU2_INDX2  9
#define MAX_IPU2_INDX3  9
#define IPU2_INDEX_1    0x2300
#define IPU2_INDEX_2    0x3200
#define IPU2_INDEX_3    0x3300
#define IPU2_PIX        0x2300
#define IPU2_LIC        0x2301
#define IPU2_FLC        0x2302
#define IPU2_LIR        0x2303
#define IPU2_FIR        0x2304
#define IPU2_MCR1       0x3200
#define IPU2_XBI1       0x3202
#define IPU2_XEI1       0x3203
#define IPU2_YBI1       0x3207
#define IPU2_YEI1       0x3208
#define IPU2_MCR2       0x3300
#define IPU2_XBI2       0x3302
#define IPU2_XEI2       0x3303
#define IPU2_YBI2       0x3307
#define IPU2_YEI2       0x3308

/* SIU register indices for the 2070  */
#define MAX_SIU_INDX1   3
#define MAX_SIU_INDX2   32
#define SIU_INDEX_1     0x2800
#define SIU_INDEX_2     0x2e00
#define SIU_MCR         0x2800
#define SIU_FCS         0x2801
#define SIU_FOU         0x2802
#define SIU_SIM0        0x2e00
#define SIU_SIM1        0x2e01
#define SIU_SIM2        0x2e02
#define SIU_SIM3        0x2e03
#define SIU_SIM4        0x2e04
#define SIU_SIM5        0x2e05
#define SIU_SIM6        0x2e06
#define SIU_SIM7        0x2e07
#define SIU_SIM8        0x2e08
#define SIU_SIM9        0x2e09
#define SIU_SIM10       0x2e0a
#define SIU_SIM11       0x2e0b
#define SIU_SIM12       0x2e0c
#define SIU_SIM13       0x2e0d
#define SIU_SIM14       0x2e0e
#define SIU_SIM15       0x2e0f
#define SIU_SIM16       0x2e10
#define SIU_SIM17       0x2e11
#define SIU_SIM18       0x2e12
#define SIU_SIM19       0x2e13
#define SIU_SIM20       0x2e14
#define SIU_SIM21       0x2e15
#define SIU_SIM22       0x2e16
#define SIU_SIM23       0x2e17
#define SIU_SIM24       0x2e18
#define SIU_SIM25       0x2e19
#define SIU_SIM26       0x2e1a
#define SIU_SIM27       0x2e1b
#define SIU_SIM28       0x2e1c
#define SIU_SIM29       0x2e1d
#define SIU_SIM30       0x2e1e
#define SIU_SIM31       0x2e1f

/* ALU register indices for the 2070  */
#define MAX_ALU_INDX1   16
#define ALU_INDEX_1     0x2900
#define ALU_MCR1        0x2900
#define ALU_MCR2        0x2901
#define ALU_TOP         0x2902
#define ALU_AV          0x2903
#define ALU_LOPY        0x2904
#define ALU_LOPU        0x2905
#define ALU_LOPV        0x2906
#define ALU_CAY         0x2907
#define ALU_CAU         0x2908
#define ALU_CAV         0x2909
#define ALU_CBY         0x290a
#define ALU_CBU         0x290b
#define ALU_CBV         0x290c
#define ALU_CCY         0x290d
#define ALU_CCU         0x290e
#define ALU_CCV         0x290f

/* OPU register indices for the 2070  */
#define MAX_OPU_INDX1   9
#define MAX_OPU_INDX2   9
#define OPU_INDEX_1     0x2a00
#define OPU_INDEX_2     0x2b00
#define OPU_MCR1        0x2a00
#define OPU_XBI1        0x2a02
#define OPU_XEI1        0x2a03
#define OPU_YBI1        0x2a07
#define OPU_YEI1        0x2a08
#define OPU_MCR2        0x2b00
#define OPU_XBI2        0x2b02
#define OPU_XEI2        0x2b03
#define OPU_YBI2        0x2b07
#define OPU_YEI2        0x2b08

/* MMU register index for the 2070  */
#define MAX_MMU_INDX1   1
#define MMU_INDEX_1     0x4000
#define MMU_MCR         0x4000

/* OBU register indices for the 2070  */
#define MAX_OBU_INDX1   7
#define OBU0_INDEX_1    0x4800
#define OBU1_INDEX_1    0x4810
#define OBU2_INDEX_1    0x4820
#define OBU3_INDEX_1    0x4830
#define OBU4_INDEX_1    0x4840
#define OBU5_INDEX_1    0x4850
#define OBU6_INDEX_1    0x4860
#define OBU7_INDEX_1    0x4870
#define OBU0_MCR        0x4800
#define OBU0_RFX        0x4801
#define OBU0_LSL        0x4802
#define OBU0_LSH        0x4803
#define OBU0_BSX        0x4804
#define OBU0_BSY        0x4805
#define OBU0_DEC        0x4806
#define OBU1_MCR        0x4810
#define OBU1_RFX        0x4811
#define OBU1_LSL        0x4812
#define OBU1_LSH        0x4813
#define OBU1_BSX        0x4814
#define OBU1_BSY        0x4815
#define OBU1_DEC        0x4816
#define OBU2_MCR        0x4820
#define OBU2_RFX        0x4821
#define OBU2_LSL        0x4822
#define OBU2_LSH        0x4823
#define OBU2_BSX        0x4824
#define OBU2_BSY        0x4825
#define OBU2_DEC        0x4826
#define OBU3_MCR        0x4830
#define OBU3_RFX        0x4831
#define OBU3_LSL        0x4832
#define OBU3_LSH        0x4833
#define OBU3_BSX        0x4834
#define OBU3_BSY        0x4835
#define OBU3_DEC        0x4836
#define OBU4_MCR        0x4840
#define OBU4_RFX        0x4841
#define OBU4_LSL        0x4842
#define OBU4_LSH        0x4843
#define OBU4_BSX        0x4844
#define OBU4_BSY        0x4845
#define OBU4_DEC        0x4846
#define OBU5_MCR        0x4850
#define OBU5_RFX        0x4851
#define OBU5_LSL        0x4852
#define OBU5_LSH        0x4853
#define OBU5_BSX        0x4854
#define OBU5_BSY        0x4855
#define OBU5_DEC        0x4856
#define OBU6_MCR        0x4860
#define OBU6_RFX        0x4861
#define OBU6_LSL        0x4862
#define OBU6_LSH        0x4863
#define OBU6_BSX        0x4864
#define OBU6_BSY        0x4865
#define OBU6_DEC        0x4866
#define OBU7_MCR        0x4870
#define OBU7_RFX        0x4871
#define OBU7_LSL        0x4872
#define OBU7_LSH        0x4873
#define OBU7_BSX        0x4874
#define OBU7_BSY        0x4875
#define OBU7_DEC        0x4876

/* DWU register indices for the 2070  */
#define MAX_DWU_INDX1   2
#define MAX_DWU_INDX2   8
#define DWU_INDEX_1     0x4100
#define DWU0_INDEX_2    0x4400
#define DWU1_INDEX_2    0x4410
#define DWU2_INDEX_2    0x4420
#define DWU3_INDEX_2    0x4430
#define DWU_MCR         0x4100
#define DWU_HCR         0x4101
#define DWU0_DZF        0x4400
#define DWU0_RFX        0x4401
#define DWU0_LSL        0x4402
#define DWU0_LSH        0x4403
#define DWU0_WSX        0x4404
#define DWU0_WSY        0x4405
#define DWU0_DSX        0x4406
#define DWU0_DSY        0x4407
#define DWU1_DZF        0x4410
#define DWU1_RFX        0x4411
#define DWU1_LSL        0x4412
#define DWU1_LSH        0x4413
#define DWU1_WSX        0x4414
#define DWU1_WSY        0x4415
#define DWU1_DSX        0x4416
#define DWU1_DSY        0x4417
#define DWU2_DZF        0x4420
#define DWU2_RFX        0x4421
#define DWU2_LSL        0x4422
#define DWU2_LSH        0x4423
#define DWU2_WSX        0x4424
#define DWU2_WSY        0x4425
#define DWU2_DSX        0x4426
#define DWU2_DSY        0x4427
#define DWU3_DZF        0x4430
#define DWU3_RFX        0x4431
#define DWU3_LSL        0x4432
#define DWU3_LSH        0x4433
#define DWU3_WSX        0x4434
#define DWU3_WSY        0x4435
#define DWU3_DSX        0x4436
#define DWU3_DSY        0x4437

/* Define the masks for the various registers in the 2070  */

/* HIU registers  */
#define HIU_CSU_MASK            0x0f3f
#define HIU_DBG_MASK            0x03ff
#define HIU_DRD_MASK            0xffff
#define HIU_OCS_MASK            0x04ef
#define HIU_IRQ_MASK            0x003f
#define HIU_RIN_MASK            0xffff
#define HIU_RDT_MASK            0xffff
#define HIU_MDT_MASK            0xffff
#define HIU_IMD_MASK            0xffff
#define HIU_ISU_MASK            0x3fff

/* VIU registers  */

#define VIU_MCR1_MASK           0xffff
#define VIU_MCR2_MASK           0xffff
#define VIU_DPC1_MASK           0x0fff
#define VIU_DPC2_MASK           0x0fff
#define VIU_WDT_MASK            0x7fff

/* VSU registers  */
#define VSU_HSW_MASK            0x007f
#define VSU_HAD_MASK            0x03ff
#define VSU_HAP_MASK            0x07ff
#define VSU_HP_MASK             0x03ff
#define VSU_VSW_MASK            0x007f
#define VSU_VAD_MASK            0x03ff
#define VSU_VAP_MASK            0x07ff
#define VSU_VP_MASK             0xc3ff

/* VPU registers  */
#define VPU_MCR_MASK            0x1777

/* IPU1 registers  */
#define IPU1_PIX_MASK           0x07ff
#define IPU1_LIC_MASK           0x07ff
#define IPU1_FLC_MASK           0x7fff
#define IPU1_LIR_MASK           0x07ff
#define IPU1_FIR_MASK           0xffff
#define IPU1_LRB_MASK           0x00ff
#define IPU1_LRD_MASK           0x00ff
#define IPU1_MCR1_MASK          0xffff
#define IPU1_XBF1_MASK          0xe000
#define IPU1_XBI1_MASK          0x07ff
#define IPU1_XEI1_MASK          0x07ff
#define IPU1_XSF1_MASK          0xffc0
#define IPU1_XSI1_MASK          0x003f
#define IPU1_YBF1_MASK          0xe000
#define IPU1_YBI1_MASK          0x07ff
#define IPU1_YEI1_MASK          0x07ff
#define IPU1_YSF1_MASK          0xffc0
#define IPU1_YSI1_MASK          0x003f
#define IPU1_KFC1_MASK          0x00ff
#define IPU1_MMY1_MASK          0xffff
#define IPU1_MMU1_MASK          0xffff
#define IPU1_MMV1_MASK          0xffff
#define IPU1_MCR2_MASK          0xffff
#define IPU1_XBF2_MASK          0xe000
#define IPU1_XBI2_MASK          0x07ff
#define IPU1_XEI2_MASK          0x07ff
#define IPU1_XSF2_MASK          0xffc0
#define IPU1_XSI2_MASK          0x003f
#define IPU1_YBF2_MASK          0xe000
#define IPU1_YBI2_MASK          0x07ff
#define IPU1_YEI2_MASK          0x07ff
#define IPU1_YSF2_MASK          0xffc0
#define IPU1_YSI2_MASK          0x003f
#define IPU1_KFC2_MASK          0x00ff
#define IPU1_MMY2_MASK          0xffff
#define IPU1_MMU2_MASK          0xffff
#define IPU1_MMV2_MASK          0xffff

/* IPU2 registers  */
#define IPU2_PIX_MASK           0x07ff
#define IPU2_LIC_MASK           0x07ff
#define IPU2_FLC_MASK           0x7fff
#define IPU2_LIR_MASK           0x07ff
#define IPU2_FIR_MASK           0xffff
#define IPU2_MCR1_MASK          0xe000
#define IPU2_XBI1_MASK          0x07ff
#define IPU2_XEI1_MASK          0x07ff
#define IPU2_YBI1_MASK          0x07ff
#define IPU2_YEI1_MASK          0x07ff
#define IPU2_MCR2_MASK          0xe000
#define IPU2_XBI2_MASK          0x07ff
#define IPU2_XEI2_MASK          0x07ff
#define IPU2_YBI2_MASK          0x07ff
#define IPU2_YEI2_MASK          0x07ff

/* SIU registers  */
#define SIU_MCR_MASK            0x3fff
#define SIU_FCS_MASK            0x3fff
#define SIU_FOU_MASK            0x3fff
#define SIU_SIM0_MASK           0x3fff
#define SIU_SIM1_MASK           0x3fff
#define SIU_SIM2_MASK           0x3fff
#define SIU_SIM3_MASK           0x3fff
#define SIU_SIM4_MASK           0x3fff
#define SIU_SIM5_MASK           0x3fff
#define SIU_SIM6_MASK           0x3fff
#define SIU_SIM7_MASK           0x3fff
#define SIU_SIM8_MASK           0x3fff
#define SIU_SIM9_MASK           0x3fff
#define SIU_SIM10_MASK          0x3fff
#define SIU_SIM11_MASK          0x3fff
#define SIU_SIM12_MASK          0x3fff
#define SIU_SIM13_MASK          0x3fff
#define SIU_SIM14_MASK          0x3fff
#define SIU_SIM15_MASK          0x3fff
#define SIU_SIM16_MASK          0x3fff
#define SIU_SIM17_MASK          0x3fff
#define SIU_SIM18_MASK          0x3fff
#define SIU_SIM19_MASK          0x3fff
#define SIU_SIM20_MASK          0x3fff
#define SIU_SIM21_MASK          0x3fff
#define SIU_SIM22_MASK          0x3fff
#define SIU_SIM23_MASK          0x3fff
#define SIU_SIM24_MASK          0x3fff
#define SIU_SIM25_MASK          0x3fff
#define SIU_SIM26_MASK          0x3fff
#define SIU_SIM27_MASK          0x3fff
#define SIU_SIM28_MASK          0x3fff
#define SIU_SIM29_MASK          0x3fff
#define SIU_SIM30_MASK          0x3fff
#define SIU_SIM31_MASK          0x3fff

/* ALU registers  */
#define ALU_MCR1_MASK           0xffff
#define ALU_MCR2_MASK           0xffff
#define ALU_TOP_MASK            0xffff
#define ALU_AV_MASK             0x00ff
#define ALU_LOPY_MASK           0xffff
#define ALU_LOPU_MASK           0xffff
#define ALU_LOPV_MASK           0xffff
#define ALU_CAY_MASK            0x00ff
#define ALU_CAU_MASK            0x01ff
#define ALU_CAV_MASK            0x01ff
#define ALU_CBY_MASK            0x00ff
#define ALU_CBU_MASK            0x01ff
#define ALU_CBV_MASK            0x01ff
#define ALU_CCY_MASK            0x00ff
#define ALU_CCU_MASK            0x01ff
#define ALU_CCV_MASK            0x01ff

/* OPU registers  */
#define OPU_MCR1_MASK           0xe40f
#define OPU_XBI1_MASK           0x07ff
#define OPU_XEI1_MASK           0x07ff
#define OPU_YBI1_MASK           0x07ff
#define OPU_YEI1_MASK           0x07ff
#define OPU_MCR2_MASK           0xe40f
#define OPU_XBI2_MASK           0x07ff
#define OPU_XEI2_MASK           0x07ff
#define OPU_YBI2_MASK           0x07ff
#define OPU_YEI2_MASK           0x07ff

/* MMU register  */
#define MMU_MCR_MASK            0x001f

/* OBU registers  */
#define OBU_MCR_MASK            0x1fff
#define OBU_RFX_MASK            0x07fe
#define OBU_LSL_MASK            0xfffe
#define OBU_LSH_MASK            0x003f
#define OBU_BSX_MASK            0x07fe
#define OBU_BSY_MASK            0x07ff
#define OBU_DEC_MASK            0x00ff

/* DWU registers  */
#define DWU_MCR_MASK            0xfd0f
#define DWU_HCR_MASK            0x07fe
#define DWU_DZF_MASK            0xffff
#define DWU_RFX_MASK            0x07fe
#define DWU_LSL_MASK            0xfffe
#define DWU_LSH_MASK            0x003f
#define DWU_WSX_MASK            0x07fe
#define DWU_WSY_MASK            0x07ff
#define DWU_DSX_MASK            0x07fe
#define DWU_DSY_MASK            0x07ff

#endif
