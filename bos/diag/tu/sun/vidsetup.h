/* @(#)62       1.4  src/bos/diag/tu/sun/vidsetup.h, tu_sunrise, bos411, 9437A411a 8/29/94 15:49:56 */
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
/*  File: vidsetup.h                                                      */
/**************************************************************************/
/*
        History -
                Written by B. POTU     July, 1993

        Description -
                7191/7199 Initialization Tables
*/



/* Encoder/Decoder bit flags -----------------------------*/

#define PCS_PAL         0x0100  /* PAL video format */
#define PCS_NTSC_VHS    0x0200  /* NTSC VHS video format */
#define PCS_NTSC_SVHS   0x0400  /* NTSC SVHS video format */
#define PCS_411         0x0001
#define PCS_422         0x0002
#define VAL_NTSC_fsc 0x01
#define VAL_PAL_fsc 0x02
#define VAL_NTSC_std 0x0c
#define VAL_PAL_std 0x02

#define INPUTMODE       0x0000  /* Input only mode */
#define OUTPUTMODE      0x0001  /* Output only mode */
#define DUPLEXMODE      0x0002  /* Duplex only mode */
#define TESTMODE        0x0003  /* Test Loopback mode */

#define VIDPORT1        1
#define VIDPORT2        2

#define PCS_CHAN1_INPUT    0       /* Zero based */
#define PCS_CHAN2_INPUT    1
#define PCS_CHAN3_INPUT    2

#define PCS_CHAN1_OUTPUT   0       /* Zero based */
#define PCS_CHAN2_OUTPUT   1
#define PCS_CHAN3_OUTPUT   2



#define SUCCESS                  0
#define EROR1                    1
#define EROR2                    2
#define EROR3                    3
#define EROR4                    4
#define EROR5                    5

#define SUBADDRESSUSED           2
#define SUBADDRESSNOTUSED        1

/*
*       SAA7191 subaddress locations.
*/
#define idel                    0x00
#define hsyc_50_begin           0x01
#define hsyc_50_stop            0x02
#define hclp_50_begin           0x03
#define hclp_50_stop            0x04
#define hsyc_50_position        0x05
#define luminance               0x06
#define hue                     0x07
#define color_killer_qam        0x08
#define color_killer_secam      0x09
#define sensitivity_pal         0x0a
#define sensitivity_secam       0x0b
#define gain_ctl                0x0c
#define mode_ctl                0x0d
#define io_clk_ctl              0x0e
#define control3                0x0f
#define control4                0x10
#define chroma_ctl              0x11
#define dummy12                 0x12
#define dummy13                 0x13
#define hsyc_60_begin           0x14
#define hsyc_60_stop            0x15
#define hclp_60_begin           0x16
#define hclp_60_stop            0x17
#define hsyc_60_position        0x18

/*
*       7191, 7199 addresses
*/

#define SAA7191RdAddr           0x8F
#define SAA7191WrAddr           0x8E
#define SAA7199RdAddr           0xB1
#define SAA7199WrAddr           0xB0
#define SAA7199CtlAddr          0x02
#define SAA7199LutAddr          0x00

/* 7191 register values  */

#define VAL_idel 0x50
#define VAL_hsb5 0x30
#define VAL_hss5 0x00
#define VAL_hcb5 0xe8
#define VAL_hcs5 0xb6
#define VAL_hsp5 0xf4
#define VAL_lum 0x01
#define VAL_lum_SVHS 0x81
#define VAL_hue 0x00
#define VAL_cktq 0xf8
#define VAL_ckts 0xf8
#define VAL_plse 0x90
#define VAL_sese 0x90
#define VAL_gain 0x00
#define VAL_mode_VTR 0x88  /* VTR mode RTC  */
#define VAL_mode_TV 0x08  /* TV mode   RTC  */
#define VAL_iock_port0 0x78  /* j4  in svhs mode j4 is lum, j8 is chrm */
#define VAL_iock_port1 0x79  /* j8  */
#define VAL_iock_port2 0x7a  /* j7  */
#define VAL_ctl3_50Hz 0xd9
#define VAL_ctl3_422 0x99
#define VAL_ctl4_422 0x59
#define VAL_ctl3_411 0x91
#define VAL_ctl4 0x00
#define VAL_chrm_NTSC 0x2c
#define VAL_chrm_PAL 0x59
#define VAL_hsb6 0x11
#define VAL_hss6 0xfe
#define VAL_hcb6 0xe6
#define VAL_hcs6 0xce
#define VAL_hsp6 0xf4
#define VAL_noinput 0xb2
#define VAL_ctl4_50Hz 0x19

/*
*   7199 register values
*/
#define VAL_fmt1 0xae /* look up bypass and slave */
#define VAL_fmt2 0x2c  /* look up and genlock */
#define VAL_fmt3 0x2e  /* look up and slave */
#define VAL_fmt_out 0x2d
#define VAL_trer 0x00
#define VAL_treg 0x00
#define VAL_treb 0x00
#define VAL_sync_TV 0xc0   /* TV mode */
#define VAL_sync_VTR 0xd0  /* VTR mode */
#define VAL_gdc 0x3f
#define VAL_incd 0x52
#define VAL_pso 0x3f
#define VAL_ctl1 0x24
#define VAL_ctl2 0x0d  /* RTC on PAL_ID on */
#define VAL_ctl3 0x00  /* RTC off */
#define VAL_chps 0x00
#define VAL_fsc 0x01
#define VAL_std 0x0c

/* PCD8584 Register Address Assignments  */

#define _Pcd_datareg         0x18F80000
#define _Pcd_statreg         0x18F80004


#define _Bus_Busy                 0
#define _Pin_Bit_Zero             0
#define _Slave_Ack                0
#define _Loop_Brk                100

#define   ODD_MASK           0x0080        /* Mask for ODD bit check */
#define   VSYNC_MASK         0x0040        /* Mask for VSYNC bit check */
