/* @(#)60	1.1  src/bos/diag/tu/siosl/tabletdev/tab_def.h, tu_siosl, bos411, 9428A410j 3/6/92 17:08:31 */
#ifndef _H_TAB_DEF
#define _H_TAB_DEF

/*
 * NAME:   Keyboard/Tablet/Sound/Mouse ATU -  ktsm_atu.h
 *
 * FUNCTION: This file includes defines, macros and structure templates for
 *           keyboard, tablet and mouse device application test units.
 *
 * EXECUTION ENVIRONMENT:  n/a
 *
 * RETURNS: n/a
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*      common definitions for keyboard,tablet and mouse ATUs         */

#define DEVICE_RESTORE_YES      1   /* need to restore device configuration */
#define DEVICE_RESTORE_NO       0   /* don't restore device configuration */
#define ENTER_TRUE              1   /* check whether 'Enter' key is hit */
#define ENTER_FALSE             0   /* check whether 'y' or 'n' is hit */
#define ASCII_ESC_KEY           0x01B /* ASCII character for Esc key */
#define ASCII_ESC1_KEY          0x201 /* Esc + 1 keys */
#define ASCII_ESC2_KEY          0x202 /* Esc + 2 keys */
#define ASCII_ESC3_KEY          0x203 /* Esc + 3 keys */
#define ASCII_ENTER_KEY         0xA   /* ASCII character for Enter key */
#define ATU_YES                 1   /*  user select yes choice */
#define ATU_NO                  2   /*  user select no choice */
#define POWER_OF_13             8192  /* 2 ** 13 */
#define VERTICAL                ((NLSCHAR) 0xBA) /* for curse routine */
#define HORIZONTAL              ((NLSCHAR) 0xCD) /* for curse routine */
#define TOPLEFT                 ((NLSCHAR) 0xC9) /* for curse routine */
#define TOPRIGHT                ((NLSCHAR) 0xBB) /* for curse routine */
#define BOTLEFT                 ((NLSCHAR) 0xC8) /* for curse routine */
#define BOTRIGHT                ((NLSCHAR) 0xBC) /* for curse routine */
#define TU_MIN_LINES            9  /* minimum lines to display */
#define ONE_LINE                1  /* need one line to display */
#define ONE_KEY                 1  /* one function key */
#define THREE_KEY               3  /* three function keys */
#define TWO_SPACE               2  /* display 2 blank characters */
#define SIX_SPACE               6  /* display 6 blank characters */
#define FIRST_LINE              0  /* first line on the display */
#define FIRST_COLUMN            0  /* first column on the display */
#define KBD_SERIAL_NUMBER       0x921003 /*keyboard TU starting serial number*/
#define TAB_SERIAL_NUMBER       0x922000 /*tablet TU starting serial numaber*/
#define THRESHOLD               3309 /* threshold value for tablet */
#define ONE_HUNDRED             100 /*   one hundred  */
#define TAB_M_RESOL             394   /* resolution in LPCm */
#define TAB_M_21_ENGLISH        6144 /*tablet model 21 active area in English*/
#define TAB_M_21_METRIC         1560 /*tablet model 21 active area in Metric */
#define TAB_M_22_ENGLISH        11500/*tablet model 22 active area in English*/
#define TAB_M_22_METRIC         2921 /*tablet model 22 active area in Metric */
#define TAB_MODEL_21            0  /* model 21 cursorpad */

/*      common definitions for keyboard,tablet and mouse TUs         */

#define TU_BREAK_KEY            0xF0  /* break key scan code */
#define TU_ESC_KEY              0x08  /* Esc key scan code */
#define TU_F1_KEY               0x07  /* F1 key scan code */
#define TU_F2_KEY               0x0F  /* F2 key scan code */
#define TU_F3_KEY               0x17  /* F3 key scan code */
#define TU_F4_KEY               0x1F  /* F4 key scan code */
#define TU_F5_KEY               0x27  /* F5 key scan code */
#define TU_PAUSE_KEY            0x62  /* Pause key scan code */
#define EXIT                    6   /* exit test mode */
#define CLEAR_SCREEN            1   /* clear the screen */
#define DRAW_MODE               2   /* keep all the points on screen */
#define POINT_MODE              3   /* only keep the current point */
#define METRIC_UNIT             4   /* set tablet to metric conversion */
#define ENGLISH_UNIT            5   /* set tablet to english conversion */
#define WRAP_MODE               4   /* display mouse motion in wrap mode */
#define UNWRAP_MODE             5   /* display mouse motion in unwrap mode */
#define TU_FAIL                 1   /* action fail */
#define TU_SUCCESS              0   /* action succeed */
#define TU_1                    0   /* test unit number 1 */
#define TU_2                    1   /* test unit number 2 */
#define TU_3                    2   /* test unit number 3 */
#define TU_4                    3   /* test unit number 4 */
#define TU_5                    4   /* test unit number 5 */
#define TU_6                    5   /* test unit number 6 */
#define TU_7                    6   /* test unit number 7 */
#define TU_8                    7   /* test unit number 8 */
#define TU_9                    8   /* test unit number 9 */
#define TU_10                   9   /* test unit number 10 */
#define TU_11                   10  /* test unit number 11 */
#define TU_12                   11  /* test unit number 12 */
#define STEP_0                  0
#define STEP_1                  1   /* first ioctl call */
#define STEP_2                  2   /* second ioctl call */
#define STEP_3                  3   /* third ioctl call */
#define STEP_4                  4   /* fourth ioctl call */
#define STEP_5                  5   /* fifth ioctl call */
#define SEND_KBD                1   /* configure keyboard command */
#define SEND_SPK                2   /* configure speaker command */
#define SEND_TAB                3   /* configure tablet command */
#define QUERY_ADAP              4   /* adapter query command */
#define SEND_ADAP               5   /* configure adapter command */
#define RECV_KBD                1   /* get data from keyboard */
#define RECV_STA_REP            2   /* get adapter status data */
#define RECV_BYTE_UART          3   /* get a byte from tablet */
#define RECV_BK_UART            4   /* get a byte from tablet in block mode*/
#define RECV_BLOCK              5   /* a tablet block is received */
#define DEVICE_RESTORE_YES      1   /* need to restore device configuration */
#define DEVICE_RESTORE_NO       0   /* don't restore device configuration */
#define ENTER_TRUE              1   /* check whether 'Enter' key is hit */
#define ENTER_FALSE             0   /* check whether 'y' or 'n' is hit */
#define ASCII_ESC_KEY           0x01B /* ASCII character for Esc key */
#define ASCII_ESC3_KEY          0x203 /* Esc + 3 keys */

/*      common definitions for keyboard and tablet TUs          */

#define TU_READ_SRAM_11         0x1100 /* read adapter SRAM 11 command */
#define TU_SPEAKER_VOLUME       0x03   /* speaker volume mask */
#define TU_UART_INTF            0x10   /* UART interface enable bit */
#define TU_INHIBIT_CLICK        0x40   /* inhibit auto_click bit */
#define TU_READ_SRAM_10         0x1000 /* read adapter SRAM 10 command */
#define TU_REP_KBD_ACK          0x01   /* report keyboard ACK bit */
#define TU_REP_TONE_COMPLETE    0x08   /* report speaker tone complete bit */
#define TU_BLOCK_ACTIVE         0x20   /* block mode active bit */
#define KEYSTROKE_ATTENTION     0x40   /* keystroke attention active bit */
#define TU_SET_FRAMING          0x8606 /* set to odd parity,6 bytes/blk */
#define TU_30HZ_HIGH            0x4008 /* high byte of 30 HZ speaker command*/
#define TU_30HZ_LOW             0xBF09 /* low byte of 30 HZ speaker command*/
#define TU_12000HZ_HIGH         0x0108 /*high byte of 12000HZ speaker command*/
#define TU_12000HZ_LOW          0x1309 /* low byte of 12000HZ speaker command*/
#define TU_2SEC_HIGH            0x0007 /* high byte of 2 sec speaker command */
#define TU_2SEC_LOW             0xFF02 /* low byte of 2 sec speaker command */
#define TU_4SEC_HIGH            0x0107 /* high byte of 4 sec speaker command */
#define TU_4SEC_LOW             0xFF02 /* low byte of 4 sec speaker command */
#define TU_6SEC_HIGH            0x0207 /* high byte of 6 sec speaker command */
#define TU_6SEC_LOW             0xFF02 /* low byte of 6 sec speaker command */
#define TU_SCAN_CODE_1          0x0101 /* set kbd to scan code set 1 command */
#define TU_SCAN_CODE_2          0x0201 /* set kbd to scan code set 2 command */
#define TU_SCAN_CODE_3          0x0301 /* set kbd to scan code set 3 command */
#define TU_SCAN_CODE_QUERY      0x0001 /* ask kbd scan code set command */
#define TURN_ALL_LEDS_ON        0x0701 /* turn on all kbd leds command */
#define TURN_ALL_LEDS_OFF       0x0001 /* turn off all kbd leds command */
#define TU_CLICK_DURATION       0xBA17 /* set click duration to BA command */
#define TU_RESTORE_CLICK_DUR    0x3617 /* set click duration to 36 command */
#define TU_508_HI_RES           0x6503 /*high byte of 508 LPIn resol. command*/
#define TU_508_LO_RES           0x9903 /* low byte of 508 LPIn resol. command*/
#define TU_1000_HI_RES          0xC803 /* high byte of 1000 LPIn res. command*/
#define TU_1000_LO_RES          0x0003 /* low byte of 1000 LPIn res. command */
#define TU_MAX_SAMPLE           0x6403 /*maximum tablet sampling rate command*/
#define TU_2MM_INC              0x3103 /*set tablet 2.5 mm increment command */
#define SEND_TAB_TEN            0x1004 /*send 0x10 tab command in wrap mode*/
#define LED_MASK                0x0700 /* mask to get kbd leds status */
#define TU_INHIBIT_CLICK_ON     0x02   /* inhibit click bit mask */
#define TU_SURPRESS_CLICK_ON    0x01   /* surpress click bit mask */
#define TU_TONE_COMPLETE        0xC0   /* tone complete mask */
#define TU_TAB_ID               0x41   /* tablet ID mask in data report*/
#define TU_TAB_RES              0x02   /* resolution mask in data report*/
#define TU_TAB_CONV             0x04   /* convertion type mask in data report*/
#define TU_TAB_DEVICE           0x20   /* tablet type mask in data report */
#define ODD_PARITY_6_BYTE       0x86   /* odd parity protocol, 6 bytes/block */
#define TU_101_KEYBOARD         0      /* 101 keyboard */
#define TU_102_KEYBOARD         1      /* 102 keyboard */
#define TU_106_KEYBOARD         2      /* 103 keyboard */
#define RIOS_ID_1               0xAB   /* first byte of read keyboard ID */
#define RIOS_ID_2               0x83   /* second byte of read keyboard ID */
#define TU_MIN_SCAN_CODE        1      /* lowest keyboard scan code */
#define TU_MAX_SCAN_CODE        132   /* highest keyboard scan code */
#define TU_MIN_POSITION_CODE    1      /* lowest keyboard position code */
#define TU_MAX_POSITION_CODE    133    /* highest keyboard position code */
#define KBD_FIRST_BYTE          0      /* first byte of 3 byte M/B key */
#define KBD_LAST_BYTE           3      /* last byte of 3 byte M/B key */
#define TAB_FIRST_BYTE          0      /* first byte of 6 byte tablet data */
#define TAB_LAST_BYTE           6      /* last byte of 6 byte tablet data */
#define ALL_LEDS_OFF            0      /* all the kbd leds are turned off */
#define TU_SPKR_VOL_OFF         0      /* speaker volume is off */
#define TU_SPKR_VOL_LOW         1      /* speaker volume is low */
#define TU_SPKR_VOL_MED         2      /* speaker volume is medium  */
#define TU_SPKR_VOL_HIGH        3      /* speaker volume is high */
#define TU_ECHO                 0xEE   /* kbe return EE after echo command */
#define SCAN_CODE_IS_1          1      /* current scan code set is 1 */
#define TU_READ                 0      /* this poll is for receiveing data */
#define TU_WRITE                1      /* this poll is for transmitting data*/
#define TU_KBD_RET_ACK          0xA0   /* keyboard ACK */
#define TU_MIN_TAB_INC          0      /* minimum tablet increment (yy) */
#define TU_MAX_TAB_INC          0xFF   /* maximum tablet increment (yy) */
#define ONE_BUTTON              1      /* input device has 1 switch */
#define FOUR_BUTTON             4      /* input device has 4 switchs */
#define SIXTEEN_BUTTON          16     /* input device has 16 switchs */
#define TEN                     0x10   /* constant 0x10 */
#define TAB_FIRST_TRY           0      /* send first test data to tablet */
#define TAB_LAST_TRY            3      /* send last test data to tablet */
#define NO_UART_BLOCK           0      /* adapter block mode is not active */
#define NO_UART_INTERF          0      /* adapter UART interface is disabled*/
#define KEYPAD_TEST             0      /* key pad test */
#define MAKE_KEY_TEST           1      /* make key type test */
#define TYPAMATIC_KEY_TEST      2      /* typamatic key type test */
#define MAKE_BREAK_KEY_TEST     3      /* make/break key type test */
#define TYPAMATIC_M_B_KEY_TEST  4      /*typamatic make/break key type test */

#endif /* _H_TAB_DEF */
