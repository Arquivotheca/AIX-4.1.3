/* @(#)57	1.1  src/bos/diag/tu/sun/regs.h, tu_sunrise, bos411, 9437A411a 3/28/94 17:48:36 */
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
        File:   regs.h
*****************************************************************************/

#include "pc550.h"
#include "px2070.h"

        struct regdefine
             { char *desc;
               int addr;
               int andmask;
               int ormask;
               short maskflag; };

/* Sunrise Initialized Registers */
        struct regdefine suninitregstr[] =
        {
                {"Config    ",  _Config           ,               0x000000FF,     0x0,            0x1,            },
                {"Init_1    ",  _Init_1           ,               0x0,            0x0,            0x0,            },
                {"Init_2    ",  _Init_2           ,               0x0,            0x0,            0x0,            },
                {"Init_3    ",  _Init_3           ,               0x0,            0x0,            0x0,            },
                {"Init_4    ",  _Init_4           ,               0x0,            0x0,            0x0,            },
                {"Init_5    ",  _Init_5           ,               0x0,            0x0,            0x0,            },
                {"Init_6    ",  _Init_6           ,               0x0,            0x0,            0x0,            },
                {"Init_7    ",  _Init_7           ,               0x0,            0x0,            0x0,            },
                {"QuantSync ",  _QuantSync        ,               0x0,            0x0,            0x0,            },
                {"QuantYCSeq",  _QuantYCSequence  ,               0x0,            0x0,            0x0,            },
                {"QuantABSeq",  _QuantABSequence  ,               0x0,            0x0,            0x0,            },
                {"VideoLatcy",  _VideoLatency     ,               0x0,            0x0,            0x0,            },
                {"HuffTblSeq",  _HuffTableSequence,               0x0,            0x0,            0x0,            },
                {"DPCM_SeqHi",  _DPCM_SeqHigh     ,               0x0,            0x0,            0x0,            },
                {"DPCM_SeqLo",  _DPCM_SeqLow      ,               0x0,            0x0,            0x0,            },
                {"CoderAttr ",  _CoderAttr        ,               0x0,            0x0,            0x0,            },
                {"CodingIntH",  _CodingIntH       ,               0x0,            0x0,            0x0,            },
                {"CodingIntL",  _CodingIntL       ,               0x0,            0x0,            0x0,            },
                {"DecLength ",  _DecLength        ,               0x0,            0x0,            0x0,            },
                {"DecCodeOrd",  _DecCodeOrder     ,               0x0,            0x0,            0x0,            },
        };

/* Miami Registers access from the micro-channel */
        struct regdefine miamiregstr_uc[] =
        {
                {"COMMAND   ",  _COMMAND          ,               0x0,            0x0,            0x0,            },
                {"ATTN      ",  _ATTN             ,               0x0,            0x0,            0x0,            },
                {"SCP       ",  _SCP              ,               0x0,            0x0,            0x0,            },
                {"GAID      ",  _GAID             ,               0x0,            0x0,            0x0,            },
                {"HSBR      ",  _HSBR             ,               0x0,            0x0,            0x0,            },
                {"MDATA     ",  _MDATA            ,               0x0,            0x0,            0x0,            },
                {"CONF1     ",  _CONF1            ,               0x0,            0x0,            0x0,            },
                {"CONF2     ",  _CONF2            ,               0x0,            0x0,            0x0,            },
                {"CONF3     ",  _CONF3            ,               0x0,            0x0,            0x0,            },
        };

/* Miami Registers access from the local bus */
        struct regdefine miamiregstr_local[] =
        {
                {"POS_Setup1",  _POS_Setup1       ,               0x0,            0x0,            0x0,            },
                {"POS_Setup2",  _POS_Setup2       ,               0x0,            0x0,            0x0,            },
                {"CRDID     ",  _CRDID            ,               0x0,            0x0,            0x0,            },
                {"PROC_CFG  ",  _PROC_CFG         ,               0x0,            0x0,            0x0,            },
                {"RSR       ",  _RSR              ,               0x0,            0x0,            0x0,            },
                {"XPOS      ",  _XPOS             ,               0x0,            0x0,            0x0,            },
                {"LBPE      ",  _LBPE             ,               0x0,            0x0,            0x0,            },
                {"CBSP      ",  _CBSP             ,               0x0,            0x0,            0x0,            },
                {"CAR1      ",  _CAR1             ,               0x0,            0x0,            0x0,            },
                {"SAR1      ",  _SAR1             ,               0x0,            0x0,            0x0,            },
                {"BCR1      ",  _BCR1             ,               0x0,            0x0,            0x0,            },
                {"CCR1      ",  _CCR1             ,               0x0,            0x0,            0x0,            },
                {"BMAR1     ",  _BMAR1            ,               0x0,            0x0,            0x0,            },
                {"LAP1      ",  _LAP1             ,               0x0,            0x0,            0x0,            },
                {"BMSTAT1   ",  _BMSTAT1          ,               0x0,            0x0,            0x0,            },
                {"BMCMD1    ",  _BMCMD1           ,               0x0,            0x0,            0x0,            },
                {"CAR2      ",  _CAR2             ,               0x0,            0x0,            0x0,            },
                {"SAR2      ",  _SAR2             ,               0x0,            0x0,            0x0,            },
                {"BCR2      ",  _BCR2             ,               0x0,            0x0,            0x0,            },
                {"CCR2      ",  _CCR2             ,               0x0,            0x0,            0x0,            },
                {"BMAR2     ",  _BMAR2            ,               0x0,            0x0,            0x0,            },
                {"LAP2      ",  _LAP2             ,               0x0,            0x0,            0x0,            },
                {"BMSTAT2   ",  _BMSTAT2          ,               0x0,            0x0,            0x0,            },
                {"BMCMD2    ",  _BMCMD2           ,               0x0,            0x0,            0x0,            },
        };

/* Local Registers */
        struct regdefine localregstr[] =
        {
                {"LBStatus  ",  _LBStatus       ,               0x0,            0x0,            0x0,            },
                {"LBControl ",  _LBControl      ,               0x0,            0x0,            0x0,            },
        };

/* Sunrise Registers */
        struct regdefine cl560regstr[] =
        {
                {"HPeriod   ",  _HPeriod        ,               0x0,            0x0,            0x0,            },
                {"HSync     ",  _HSync          ,               0x0,            0x0,            0x0,            },
                {"HDelay    ",  _HDelay         ,               0x0,            0x0,            0x0,            },
                {"HActive   ",  _HActive        ,               0x0,            0x0,            0x0,            },
                {"VPeriod   ",  _VPeriod        ,               0x0,            0x0,            0x0,            },
                {"VSync     ",  _VSync          ,               0x0,            0x0,            0x0,            },
                {"VDelay    ",  _VDelay         ,               0x0,            0x0,            0x0,            },
                {"VActive   ",  _VActive        ,               0x0,            0x0,            0x0,            },
                {"DCT01     ",  _DCT+0x00       ,               0x0,            0x0,            0x0,            },
                {"DCT11     ",  _DCT+0x04       ,               0x0,            0x0,            0x0,            },
                {"DCT21     ",  _DCT+0x08       ,               0x0,            0x0,            0x0,            },
                {"DCT31     ",  _DCT+0x0c       ,               0x0,            0x0,            0x0,            },
                {"DCT02     ",  _DCT+0x10       ,               0x0,            0x0,            0x0,            },
                {"DCT12     ",  _DCT+0x14       ,               0x0,            0x0,            0x0,            },
                {"DCT22     ",  _DCT+0x18       ,               0x0,            0x0,            0x0,            },
                {"DCT32     ",  _DCT+0x1c       ,               0x00FFFFFF,     0x0,            0x1,            },
                {"Config    ",  _Config         ,               0x000000FF,     0x0,            0x1,            },
                {"Huff_Enabl",  _Huff_Enable    ,               0x000000FF,     0x0,            0x1,            },
                {"Start     ",  _Start          ,               0x0000000F,     0x0,            0x1,            },
                {"HV_Enable ",  _HV_Enable      ,               0x000000FF,     0x0,            0x1,            },
                {"Flags     ",  _Flags          ,               0x0,            0x0,            0x0,            },
                {"IRQ1_Mask ",  _IRQ1_Mask      ,               0x0,            0x0,            0x0,            },
                {"DRQ_Mask  ",  _DRQ_Mask       ,               0x0,            0x0,            0x0,            },
                {"StartFrame",  _StartOfFrame   ,               0x0,            0x0,            0x0,            },
                {"Version   ",  _Version        ,               0x0,            0x0,            0x0,            },
                {"IRQ2_Mask ",  _IRQ2_Mask      ,               0x0,            0x0,            0x0,            },
                {"FrmEndEn  ",  _FrmEndEn       ,               0x0,            0x0,            0x0,            },
                {"DecMarker ",  _DecMarker      ,               0x0,            0x0,            0x0,            },
                {"DecResume ",  _DecResume      ,               0x0,            0x0,            0x0,            },
                {"DecDPCM_Re",  _DecDPCM_Reset  ,               0x0,            0x0,            0x0,            },
                {"VideoLaten",  _VideoLatency   ,               0x0,            0x0,            0x0,            },
                {"HControl  ",  _HControl       ,               0x0,            0x0,            0x0,            },
                {"VControl  ",  _VControl       ,               0x000000FF,     0x0,            0x1,            },
                {"VertLineCt",  _VertLineCount  ,               0x0,            0x0,            0x0,            },
        };

/* px2070 VIU & VSU Registers */
        struct regdefine VIU_VSUregsstr[] =
        {
                {"VIU_MCR1  ", i_VIU_MCR1       ,               0x0,            0x0,            0x0,            },
                {"VIU_MCR2  ", i_VIU_MCR2       ,               0x0,            0x0,            0x0,            },
                {"VIU_DPC1  ", i_VIU_DPC1       ,               0x0,            0x0,            0x0,            },
                {"VIU_DPC2  ", i_VIU_DPC2       ,               0x0,            0x0,            0x0,            },
                {"VIU_WDT   ", i_VIU_WDT        ,               0x0,            0x0,            0x0,            },
                {"VIU_TEST  ", i_VIU_TEST       ,               0x0,            0x0,            0x0,            },
                {"VSU_HSW   ", i_VSU_HSW        ,               0x0,            0x0,            0x0,            },
                {"VSU_HAD   ", i_VSU_HAD        ,               0x0,            0x0,            0x0,            },
                {"VSU_HAP   ", i_VSU_HAP        ,               0x0,            0x0,            0x0,            },
                {"VSU_HP    ", i_VSU_HP         ,               0x0,            0x0,            0x0,            },
                {"VSU_VSW   ", i_VSU_VSW        ,               0x0,            0x0,            0x0,            },
                {"VSU_VAD   ", i_VSU_VAD        ,               0x0,            0x0,            0x0,            },
                {"VSU_VAP   ", i_VSU_VAP        ,               0x0,            0x0,            0x0,            },
                {"VSU_VP    ", i_VSU_VP         ,               0x0,            0x0,            0x0,            },
        };

/* px2070 IPU1 & IPU2 Registers */
        struct regdefine IPU1_IPU2regsstr[] =
        {
                {"IPU1_PIX  ", i_IPU1_PIX       ,               0x0,            0x0,            0x0,            },
                {"IPU1_LIC  ", i_IPU1_LIC       ,               0x0,            0x0,            0x0,            },
                {"IPU1_FLC  ", i_IPU1_FLC       ,               0x0,            0x0,            0x0,            },
                {"IPU1_LIR  ", i_IPU1_LIR       ,               0x0,            0x0,            0x0,            },
                {"IPU1_FIR  ", i_IPU1_FIR       ,               0x0,            0x0,            0x0,            },
                {"IPU1_LRB  ", i_IPU1_LRB       ,               0x0,            0x0,            0x0,            },
                {"IPU1_LRD  ", i_IPU1_LRD       ,               0x0,            0x0,            0x0,            },
                {"IPU1_MCR1 ", i_IPU1_MCR1      ,               0x0,            0x0,            0x0,            },
                {"IPU1_XBF1 ", i_IPU1_XBF1      ,               0x0,            0x0,            0x0,            },
                {"IPU1_XBI1 ", i_IPU1_XBI1      ,               0x0,            0x0,            0x0,            },
                {"IPU1_XEI1 ", i_IPU1_XEI1      ,               0x0,            0x0,            0x0,            },
                {"IPU1_XSF1 ", i_IPU1_XSF1      ,               0x0,            0x0,            0x0,            },
                {"IPU1_XSI1 ", i_IPU1_XSI1      ,               0x0,            0x0,            0x0,            },
                {"IPU1_YBF1 ", i_IPU1_YBF1      ,               0x0,            0x0,            0x0,            },
                {"IPU1_YBI1 ", i_IPU1_YBI1      ,               0x0,            0x0,            0x0,            },
                {"IPU1_YEI1 ", i_IPU1_YEI1      ,               0x0,            0x0,            0x0,            },
                {"IPU1_YSF1 ", i_IPU1_YSF1      ,               0x0,            0x0,            0x0,            },
                {"IPU1_YSI1 ", i_IPU1_YSI1      ,               0x0,            0x0,            0x0,            },
                {"IPU1_KFC1 ", i_IPU1_KFC1      ,               0x0,            0x0,            0x0,            },
                {"IPU1_MMY1 ", i_IPU1_MMY1      ,               0x0,            0x0,            0x0,            },
                {"IPU1_MMU1 ", i_IPU1_MMU1      ,               0x0,            0x0,            0x0,            },
                {"IPU1_MMV1 ", i_IPU1_MMV1      ,               0x0,            0x0,            0x0,            },
                {"IPU1_MCR2 ", i_IPU1_MCR2      ,               0x0,            0x0,            0x0,            },
                {"IPU1_XBF2 ", i_IPU1_XBF2      ,               0x0,            0x0,            0x0,            },
                {"IPU1_XBI2 ", i_IPU1_XBI2      ,               0x0,            0x0,            0x0,            },
                {"IPU1_XEI2 ", i_IPU1_XEI2      ,               0x0,            0x0,            0x0,            },
                {"IPU1_XSF2 ", i_IPU1_XSF2      ,               0x0,            0x0,            0x0,            },
                {"IPU1_XSI2 ", i_IPU1_XSI2      ,               0x0,            0x0,            0x0,            },
                {"IPU1_YBF2 ", i_IPU1_YBF2      ,               0x0,            0x0,            0x0,            },
                {"IPU1_YBI2 ", i_IPU1_YBI2      ,               0x0,            0x0,            0x0,            },
                {"IPU1_YEI2 ", i_IPU1_YEI2      ,               0x0,            0x0,            0x0,            },
                {"IPU1_YSF2 ", i_IPU1_YSF2      ,               0x0,            0x0,            0x0,            },
                {"IPU1_YSI2 ", i_IPU1_YSI2      ,               0x0,            0x0,            0x0,            },
                {"IPU1_KFC2 ", i_IPU1_KFC2      ,               0x0,            0x0,            0x0,            },
                {"IPU1_MMY2 ", i_IPU1_MMY2      ,               0x0,            0x0,            0x0,            },
                {"IPU1_MMU2 ", i_IPU1_MMU2      ,               0x0,            0x0,            0x0,            },
                {"IPU1_MMV2 ", i_IPU1_MMV2      ,               0x0,            0x0,            0x0,            },
                {"IPU2_PIX  ", i_IPU2_PIX       ,               0x0,            0x0,            0x0,            },
                {"IPU2_LIC  ", i_IPU2_LIC       ,               0x0,            0x0,            0x0,            },
                {"IPU2_FLC  ", i_IPU2_FLC       ,               0x0,            0x0,            0x0,            },
                {"IPU2_LIR  ", i_IPU2_LIR       ,               0x0,            0x0,            0x0,            },
                {"IPU2_FIR  ", i_IPU2_FIR       ,               0x0,            0x0,            0x0,            },
                {"IPU2_MCR1 ", i_IPU2_MCR1      ,               0x0,            0x0,            0x0,            },
                {"IPU2_XBI1 ", i_IPU2_XBI1      ,               0x0,            0x0,            0x0,            },
                {"IPU2_XEI1 ", i_IPU2_XEI1      ,               0x0,            0x0,            0x0,            },
                {"IPU2_YBI1 ", i_IPU2_YBI1      ,               0x0,            0x0,            0x0,            },
                {"IPU2_YEI1 ", i_IPU2_YEI1      ,               0x0,            0x0,            0x0,            },
                {"IPU2_MCR2 ", i_IPU2_MCR2      ,               0x0,            0x0,            0x0,            },
                {"IPU2_XBI2 ", i_IPU2_XBI2      ,               0x0,            0x0,            0x0,            },
                {"IPU2_XEI2 ", i_IPU2_XEI2      ,               0x0,            0x0,            0x0,            },
                {"IPU2_YBI2 ", i_IPU2_YBI2      ,               0x0,            0x0,            0x0,            },
                {"IPU2_YEI2 ", i_IPU2_YEI2      ,               0x0,            0x0,            0x0,            },
        };
/* px2070 SIU & OPU Registers */
        struct regdefine SIU_OPUregsstr[] =
        {
                {"SIU_MCR  ", i_SIU_MCR      ,               0x0,            0x0,            0x0,            },
                {"SIU_FSC  ", i_SIU_FSC      ,               0x0,            0x0,            0x0,            },
                {"SIU_FOU  ", i_SIU_FOU      ,               0x0,            0x0,            0x0,            },
                {"SIU0_SIM ", i_SIU_SIM+0    ,               0x0,            0x0,            0x0,            },
                {"SIU1_SIM ", i_SIU_SIM+1    ,               0x0,            0x0,            0x0,            },
                {"SIU2_SIM ", i_SIU_SIM+2    ,               0x0,            0x0,            0x0,            },
                {"SIU3_SIM ", i_SIU_SIM+3    ,               0x0,            0x0,            0x0,            },
                {"SIU4_SIM ", i_SIU_SIM+4    ,               0x0,            0x0,            0x0,            },
                {"SIU5_SIM ", i_SIU_SIM+5    ,               0x0,            0x0,            0x0,            },
                {"SIU6_SIM ", i_SIU_SIM+6    ,               0x0,            0x0,            0x0,            },
                {"SIU7_SIM ", i_SIU_SIM+7    ,               0x0,            0x0,            0x0,            },
                {"SIU8_SIM ", i_SIU_SIM+8    ,               0x0,            0x0,            0x0,            },
                {"SIU9_SIM ", i_SIU_SIM+9    ,               0x0,            0x0,            0x0,            },
                {"SIU10_SIM", i_SIU_SIM+10   ,               0x0,            0x0,            0x0,            },
                {"SIU11_SIM", i_SIU_SIM+11   ,               0x0,            0x0,            0x0,            },
                {"SIU12_SIM", i_SIU_SIM+12   ,               0x0,            0x0,            0x0,            },
                {"SIU13_SIM", i_SIU_SIM+13   ,               0x0,            0x0,            0x0,            },
                {"SIU14_SIM", i_SIU_SIM+14   ,               0x0,            0x0,            0x0,            },
                {"SIU15_SIM", i_SIU_SIM+15   ,               0x0,            0x0,            0x0,            },
                {"SIU16_SIM", i_SIU_SIM+16   ,               0x0,            0x0,            0x0,            },
                {"SIU17_SIM", i_SIU_SIM+17   ,               0x0,            0x0,            0x0,            },
                {"SIU18_SIM", i_SIU_SIM+18   ,               0x0,            0x0,            0x0,            },
                {"SIU19_SIM", i_SIU_SIM+19   ,               0x0,            0x0,            0x0,            },
                {"SIU20_SIM", i_SIU_SIM+20   ,               0x0,            0x0,            0x0,            },
                {"SIU21_SIM", i_SIU_SIM+21   ,               0x0,            0x0,            0x0,            },
                {"SIU22_SIM", i_SIU_SIM+22   ,               0x0,            0x0,            0x0,            },
                {"SIU23_SIM", i_SIU_SIM+23   ,               0x0,            0x0,            0x0,            },
                {"SIU24_SIM", i_SIU_SIM+24   ,               0x0,            0x0,            0x0,            },
                {"SIU25_SIM", i_SIU_SIM+25   ,               0x0,            0x0,            0x0,            },
                {"SIU26_SIM", i_SIU_SIM+26   ,               0x0,            0x0,            0x0,            },
                {"SIU27_SIM", i_SIU_SIM+27   ,               0x0,            0x0,            0x0,            },
                {"SIU28_SIM", i_SIU_SIM+28   ,               0x0,            0x0,            0x0,            },
                {"SIU29_SIM", i_SIU_SIM+29   ,               0x0,            0x0,            0x0,            },
                {"SIU30_SIM", i_SIU_SIM+30   ,               0x0,            0x0,            0x0,            },
                {"SIU31_SIM", i_SIU_SIM+31   ,               0x0,            0x0,            0x0,            },
                {"OPU_MCR1 ", i_OPU_MCR1     ,               0x0,            0x0,            0x0,            },
                {"OPU_XBI1 ", i_OPU_XBI1     ,               0x0,            0x0,            0x0,            },
                {"OPU_XEI1 ", i_OPU_XEI1     ,               0x0,            0x0,            0x0,            },
                {"OPU_YBI1 ", i_OPU_YBI1     ,               0x0,            0x0,            0x0,            },
                {"OPU_YEI1 ", i_OPU_YEI1     ,               0x0,            0x0,            0x0,            },
                {"OPU_MCR2 ", i_OPU_MCR2     ,               0x0,            0x0,            0x0,            },
                {"OPU_XBI2 ", i_OPU_XBI2     ,               0x0,            0x0,            0x0,            },
                {"OPU_XEI2 ", i_OPU_XEI2     ,               0x0,            0x0,            0x0,            },
                {"OPU_YBI2 ", i_OPU_YBI2     ,               0x0,            0x0,            0x0,            },
                {"OPU_YEI2 ", i_OPU_YEI2     ,               0x0,            0x0,            0x0,            },
                {"OPU_PXCN ", i_OPU_PXCNT    ,               0x0,            0x0,            0x0,            },
                {"OPU_LNCN ", i_OPU_LNCNT    ,               0x0,            0x0,            0x0,            },
        };

/* px2070 OBU Registers */
        struct regdefine OBUregsstr[] =
        {
                {"OBU0_MCR  ", i_OBU0_Base+0 ,               0x0,            0x0,            0x0,            },
                {"OBU0_RFX  ", i_OBU0_Base+1 ,               0x0,            0x0,            0x0,            },
                {"OBU0_LSL  ", i_OBU0_Base+2 ,               0x0,            0x0,            0x0,            },
                {"OBU0_LSH  ", i_OBU0_Base+3 ,               0x0,            0x0,            0x0,            },
                {"OBU0_BSX  ", i_OBU0_Base+4 ,               0x0,            0x0,            0x0,            },
                {"OBU0_BSY  ", i_OBU0_Base+5 ,               0x0,            0x0,            0x0,            },
                {"OBU0_DEC  ", i_OBU0_Base+6 ,               0x0,            0x0,            0x0,            },

                {"OBU1_MCR  ", i_OBU1_Base+0 ,               0x0,            0x0,            0x0,            },
                {"OBU1_RFX  ", i_OBU1_Base+1 ,               0x0,            0x0,            0x0,            },
                {"OBU1_LSL  ", i_OBU1_Base+2 ,               0x0,            0x0,            0x0,            },
                {"OBU1_LSH  ", i_OBU1_Base+3 ,               0x0,            0x0,            0x0,            },
                {"OBU1_BSX  ", i_OBU1_Base+4 ,               0x0,            0x0,            0x0,            },
                {"OBU1_BSY  ", i_OBU1_Base+5 ,               0x0,            0x0,            0x0,            },
                {"OBU1_DEC  ", i_OBU1_Base+6 ,               0x0,            0x0,            0x0,            },

                {"OBU2_MCR  ", i_OBU2_Base+0 ,               0x0,            0x0,            0x0,            },
                {"OBU2_RFX  ", i_OBU2_Base+1 ,               0x0,            0x0,            0x0,            },
                {"OBU2_LSL  ", i_OBU2_Base+2 ,               0x0,            0x0,            0x0,            },
                {"OBU2_LSH  ", i_OBU2_Base+3 ,               0x0,            0x0,            0x0,            },
                {"OBU2_BSX  ", i_OBU2_Base+4 ,               0x0,            0x0,            0x0,            },
                {"OBU2_BSY  ", i_OBU2_Base+5 ,               0x0,            0x0,            0x0,            },
                {"OBU2_DEC  ", i_OBU2_Base+6 ,               0x0,            0x0,            0x0,            },

                {"OBU3_MCR  ", i_OBU3_Base+0 ,               0x0,            0x0,            0x0,            },
                {"OBU3_RFX  ", i_OBU3_Base+1 ,               0x0,            0x0,            0x0,            },
                {"OBU3_LSL  ", i_OBU3_Base+2 ,               0x0,            0x0,            0x0,            },
                {"OBU3_LSH  ", i_OBU3_Base+3 ,               0x0,            0x0,            0x0,            },
                {"OBU3_BSX  ", i_OBU3_Base+4 ,               0x0,            0x0,            0x0,            },
                {"OBU3_BSY  ", i_OBU3_Base+5 ,               0x0,            0x0,            0x0,            },
                {"OBU3_DEC  ", i_OBU3_Base+6 ,               0x0,            0x0,            0x0,            },

                {"OBU4_MCR  ", i_OBU4_Base+0 ,               0x0,            0x0,            0x0,            },
                {"OBU4_RFX  ", i_OBU4_Base+1 ,               0x0,            0x0,            0x0,            },
                {"OBU4_LSL  ", i_OBU4_Base+2 ,               0x0,            0x0,            0x0,            },
                {"OBU4_LSH  ", i_OBU4_Base+3 ,               0x0,            0x0,            0x0,            },
                {"OBU4_BSX  ", i_OBU4_Base+4 ,               0x0,            0x0,            0x0,            },
                {"OBU4_BSY  ", i_OBU4_Base+5 ,               0x0,            0x0,            0x0,            },
                {"OBU4_DEC  ", i_OBU4_Base+6 ,               0x0,            0x0,            0x0,            },

                {"OBU5_MCR  ", i_OBU5_Base+0 ,               0x0,            0x0,            0x0,            },
                {"OBU5_RFX  ", i_OBU5_Base+1 ,               0x0,            0x0,            0x0,            },
                {"OBU5_LSL  ", i_OBU5_Base+2 ,               0x0,            0x0,            0x0,            },
                {"OBU5_LSH  ", i_OBU5_Base+3 ,               0x0,            0x0,            0x0,            },
                {"OBU5_BSX  ", i_OBU5_Base+4 ,               0x0,            0x0,            0x0,            },
                {"OBU5_BSY  ", i_OBU5_Base+5 ,               0x0,            0x0,            0x0,            },
                {"OBU5_DEC  ", i_OBU5_Base+6 ,               0x0,            0x0,            0x0,            },

                {"OBU6_MCR  ", i_OBU6_Base+0 ,               0x0,            0x0,            0x0,            },
                {"OBU6_RFX  ", i_OBU6_Base+1 ,               0x0,            0x0,            0x0,            },
                {"OBU6_LSL  ", i_OBU6_Base+2 ,               0x0,            0x0,            0x0,            },
                {"OBU6_LSH  ", i_OBU6_Base+3 ,               0x0,            0x0,            0x0,            },
                {"OBU6_BSX  ", i_OBU6_Base+4 ,               0x0,            0x0,            0x0,            },
                {"OBU6_BSY  ", i_OBU6_Base+5 ,               0x0,            0x0,            0x0,            },
                {"OBU6_DEC  ", i_OBU6_Base+6 ,               0x0,            0x0,            0x0,            },

                {"OBU7_MCR  ", i_OBU7_Base+0 ,               0x0,            0x0,            0x0,            },
                {"OBU7_RFX  ", i_OBU7_Base+1 ,               0x0,            0x0,            0x0,            },
                {"OBU7_LSL  ", i_OBU7_Base+2 ,               0x0,            0x0,            0x0,            },
                {"OBU7_LSH  ", i_OBU7_Base+3 ,               0x0,            0x0,            0x0,            },
                {"OBU7_BSX  ", i_OBU7_Base+4 ,               0x0,            0x0,            0x0,            },
                {"OBU7_BSY  ", i_OBU7_Base+5 ,               0x0,            0x0,            0x0,            },
                {"OBU7_DEC  ", i_OBU7_Base+6 ,               0x0,            0x0,            0x0,            },
        };

/* px2070 HIU Registers */
        struct regdefine HIUregsstr[] =
        {
                {"HIU_0     ", 0              ,               0x0,            0x0,            0x0,            },
                {"HIU_1     ", 1              ,               0x0,            0x0,            0x0,            },
                {"HIU_2     ", 2              ,               0x0,            0x0,            0x0,            },
                {"HIU_3     ", 3              ,               0x0,            0x0,            0x0,            },
                {"HIU_4     ", 4              ,               0x0,            0x0,            0x0,            },
        };

/* Front-end: IIC, 7191, 7199  Registers */
        struct regdefine ftendregsstr[] =
        {
                {"IIC_DATA  ", _IIC_DATA      ,               0x0,            0x0,            0x0,            },
                {"IIC_STATUS", _IIC_STATUS    ,               0x0,            0x0,            0x0,            },
        };

