/* @(#)13   1.1 src/bos/kernext/inputdd/inc/tabcmd.h, inputdd, bos411, 9433B411a 8/16/94 11:07:17 */
/*
 * COMPONENT_NAME: (INPUTDD) Keyboard/Tablet/Sound/Mouse DD - tabcmd.h
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*---------------------------------------------------------------------------*/
/* Tablet commands - RS1/RS2 platform                                        */
/*---------------------------------------------------------------------------*/

/* one frame commands                                                        */
#define  TAB_RESET_CMD          0x0103  /* reset tablet                      */
#define  ENABLE_TAB_CMD         0x0803  /* enable tablet                     */
#define  DISABLE_TAB_CMD        0x0903  /* disable tablet                    */
#define  READ_TAB_STATUS_CMD    0x0B03  /* read tablet status                */
#define  SET_WRAP_MODE_CMD      0x0E03  /* set wrap mode                     */
#define  RESET_WRAP_MODE_CMD    0x0F03  /* reset wrap mode                   */
#define  READ_TAB_CONFIG_CMD    0x0603  /* read tablet configuration (id)    */

/* two frame commands                                                        */
#define  NXT_FRAME_DATA         0x8000  /* high order bit of cmd indicates   */
                                        /*   that next frame is data         */
#define  SET_TAB_CONV           0x8303  /* set conversion command            */
#define    SET_ENGLISH          0x0003  /* set to english                    */
#define    SET_METRIC           0x0103  /* set to metric                     */

#define  SET_TAB_RES_H          0x8403  /* set resolution command            */
/*    (h)= set_value 0x00 -> 0xFF          INT(lines per in / 5)             */
#define  SET_TAB_RES_L          0x8503  /* set resolution command            */
/*    (l)= set_value 0x00,33,66,99,CC      INT(((lines per in /5) - h) << 8) */

#define  SET_TAB_ORG            0x8603  /* set origin command                */
#define    SET_LOWER_LEFT       0x0003  /* set to lower left                 */
#define    SET_CENTER           0x0203  /* set to center                     */

#define  SET_TAB_INCR           0x8903  /* set increment value               */
/*         set_value 0x00 -> 0x7F          set to value from 0x00 to 0x7f    */

#define  SET_TAB_SAM_RATE       0x8A03  /* set sample rate                   */
/*         set_value 1-100S=0x01-0x64      set to value from 0x01 to 0x64    */

#define  SET_TAB_MODE           0x8D03  /* set mode of tablet                */
#define    INCRIMENT_DATA_MODE  0x0003  /* set to increment data             */
#define    POINT_DATA_MODE      0x0103  /* set to point data                 */
#define    SWITCH_INC_DATA_MODE 0x0203  /* set to switch increment data      */
#define    REMOTE_MODE          0x0303  /* set to remote mode                */

/*---------------------------------------------------------------------------*/
/* Tablet commands - RSC Platform                                            */
/*---------------------------------------------------------------------------*/

/* tablet 1 frame commands                                                   */
#define  T_RESET_CMD            0x01    /* reset tablet                      */
#define  TAB_CONFIG_CMD         0x06    /* read tablet configuration (id)    */
#define  T_ENABLE_CMD           0x08    /* enable tablet                     */
#define  T_DISABLE_CMD          0x09    /* disable tablet                    */
#define  TAB_STATUS_CMD         0x0B    /* read tablet status                */
#define  WRAP_MODE_CMD          0x0E    /* set wrap mode                     */
#define  UNWRAP_MODE_CMD        0x0F    /* reset wrap mode                   */
/* tablet 2 frame commands                                                   */
#define  TAB_CONV               0x83    /* set conversion command            */
#define    ENGLISH              0x00    /* set to english                    */
#define    METRIC               0x01    /* set to metric                     */
#define  TAB_RES_H              0x84    /* set resolution command            */
/*         set_value 0x00 -> 0xFF          INT(lines per in / 5)             */
#define  TAB_RES_L              0x85    /* set resolution command            */
/*         set_value 0x00,33,66,99,CC      INT(((lines per in /5) - h) << 8) */

#define  TAB_ORG                0x86    /* set origin command                */
#define    LOWER_LEFT           0x00    /* set to lower left                 */
#define    CENTER               0x02    /* set to center                     */

#define  TAB_INCR               0x89    /* set increment value               */
/*         set_value 0x00 -> 0x7F          set to value from 0x00 to 0x7f    */

#define  TAB_SAM_RATE           0x8A    /* set sample rate                   */
/*         set_value 1-100S=0x01-0x64      set to value from 0x01 to 0x64    */

#define  TAB_MODE               0x8D    /* set mode of tablet                */
#define    INCRIMENT            0x00    /* set to increment data             */
#define    POINT                0x01    /* set to point data                 */
#define    SWITCH_INC           0x02    /* set to switch increment data      */
#define    REMOTE               0x03    /* set to remote mode                */

/*---------------------------------------------------------------------------*/
/* Tablet command responses                                                  */
/*---------------------------------------------------------------------------*/

/* tablet types (from  read tablet configuration command)                    */
#define  TAB_MODEL_11          0x15     /* tablet is a model 11              */
#define  TAB_MODEL_12          0x14     /* tablet is a model 12              */

/* input device query (from read tablet status command)                      */
#define  INPUT_DEVICE_TYPE     0x18     /* input device type mask            */
#define  NO_INPUT_DEVICE       0x00
#define  STYLUS_INPUT_DEVICE   0x08
#define  PUCK_INPUT_DEVICE     0x10

/*---------------------------------------------------------------------------*/
/* Tablet status report format                                               */
/*---------------------------------------------------------------------------*/

#define TAB_REPORT_SIZE     6          /* number of frames in tablet report  */

/* frame 0                                                                   */
#define TAB_PRESENCE        0x80       /* 0 = out , 1 = in                   */
#define TAB_IDENTIFIER      0x40       /* always 1 = tablet                  */
#define TAB_DEV_ATACHED     0x20       /* 0 = M21 , 1 = M22                  */
#define TAB_INPUT_DEVICE    0x18       /* 0= none, 1= pen, 2=4button 3=16 but*/
#define TAB_CONVERT_FLAG    0x04       /* 0 = english , 1 = metric           */
#define TAB_RES_CMD_RCVD    0x02       /* 1 = set resolution command recieved*/

/* frame 1                                                                   */
#define TAB_BUTTON_PRESSED  0xF8      /* actual number of button depressed   */
#define  TAB_BUTTON_1_DOWN  0x08
#define  TAB_BUTTON_2_DOWN  0x10
#define  TAB_BUTTON_3_DOWN  0x18
#define  TAB_BUTTON_4_DOWN  0x20
#define  TAB_BUTTON_5_DOWN  0x28
#define  TAB_BUTTON_6_DOWN  0x30
#define  NO_TAB_BUTTON_DOWN 0x00

#define TAB_X_DATA_SIGN     0x04      /* sign bit for x data                 */
#define TAB_Y_DATA_SIGN     0x02      /* sign bit for y data                 */

/* frame 2                                                                   */
#define X_TAB_DATA_HI       0xFE      /* x position bits 12 - 6              */

/* frame 3                                                                   */
#define X_TAB_DATA_LO       0xFC      /* x position bits 5 - 0               */
#define X_TAB_DATA_MSB      0x02      /* x position bit 13                   */

/* frame 4                                                                   */
#define Y_TAB_DATA_HI       0xFE      /* y position bits 12 - 6              */

/* frame 5                                                                   */
#define Y_TAB_DATA_LO       0xFC      /* y position bits 5 - 0               */
#define Y_TAB_DATA_MSB      0x02      /* y position bit 13                   */

/*---------------------------------------------------------------------------*/
/* Miscellaneous definitions                                                 */
/*---------------------------------------------------------------------------*/

#define  TAB_MOD_11_SIZE       6144     /* 6.144 inches, * 1000              */
#define  TAB_MOD_12_SIZE       11500    /* 11.5  inches, * 1000              */
#define  TAB_MOD_11_MET_SIZE   15600    /* 15.6 cms      * 1000              */
#define  TAB_MOD_12_MET_SIZE   29210    /* 29.21 cms,    * 1000              */

#define  MIN_TAB_RESOLUTION        0 
#define  MAX_TAB_RESOLUTION_INCH   1279   /* max in lines/inch               */
#define  MAX_TAB_RESOLUTION_CM     500    /* max in lines/cm                 */
#define  DFT_TAB_RESOLUTION        500    /* default in lines/inch           */
#define  TAB_FLICKER_VALUE         150    /* resolution where flicker starts */

#define  MIN_TAB_SAMPLE_RATE       1      /* minimum samples per second      */
#define  MAX_TAB_SAMPLE_RATE       100    /* maximum samples per second      */
#define  DFT_TAB_SAMPLE_RATE       1      /* default sample rate             */

#define  MIN_TAB_DEADZONE          0
#define  MAX_TAB_DEADZONE          32767
