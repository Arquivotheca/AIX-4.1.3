/* static char sccsid[] = "@(#)94  1.5  src/bos/diag/tu/kbd/kbd_msg.h, tu_kbd, bos411, 9428A410j 6/3/94 14:27:17"; */
/*
 * COMPONENT_NAME: tu_kbd 
 *
 * FUNCTIONS: exectu (main), tu10, tu20, tu30, tu40, tu60, 
 *            SpeakerReadWriteTest, genkeymap, updatescreen, finish,
 *            SendKbdata, SendRxKbData, maketitleadvanced, setmenunumberbase,
 *            putmsg, chk_stat, set_raw_mode, restore_kbd_mode
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define ZERO    0
#define YES     1
#define NO      2
#define RETRY   3

#define RINGSIZE 200   /* Size of kbd input ring - for tu40 */

#include "diag/diago.h"
#include "dkbd_msg.h"

ASL_SCR_TYPE    menutypes= DM_TYPE_DEFAULTS;

/*
 *      message lists for menus presented via asl during this da.
 */

struct  msglist keyboard_explain[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, KBD_LEDS },
                (int)NULL
        };

struct  msglist keyboard_no_enter[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, KBD_NOENTER },
                (int)NULL
        };

struct  msglist ledson_yes_no[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, SAYYES },
                { KBRD_MSGS, SAYNO },
		{ KBRD_MSGS, RUNOVER },
		{ KBRD_MSGS, LEDS_ON_OK },
                (int)NULL
        };

struct  msglist ledsoff_yes_no[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, SAYYES },
                { KBRD_MSGS, SAYNO },
		{ KBRD_MSGS, LEDS_OFF_OK },
                (int)NULL
        };

struct  msglist ask_102[]=
        {
                { KBRD_MSGS, TESTING },
/*
                { KBRD_MSGS, GERMAN_100 },
*/
                { KBRD_MSGS, BRAZIL_104 },
                { KBRD_MSGS, OTHER_102 },
                { KBRD_MSGS, ASK_102 },
                (int)NULL
        };

ASL_SCR_INFO    menu_102[DIAG_NUM_ENTRIES(ask_102)];

struct  msglist keypad_frame[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, KEYPAD },
                (int)NULL
        };

struct  msglist kbrd_yes_no[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, SAYYES },
                { KBRD_MSGS, SAYNO },
                { KBRD_MSGS, KEYPAD_WORKED },
                (int)NULL
        };

ASL_SCR_INFO    menu_kbdyn[DIAG_NUM_ENTRIES(kbrd_yes_no)];

struct  msglist clickon_explain[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, CLICK_ON },
                (int)NULL
        };

struct  msglist clickon_yes_no[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, SAYYES },
                { KBRD_MSGS, SAYNO },
                { KBRD_MSGS, WAS_CLICK_ON },
                (int)NULL
        };

struct  msglist clickoff_explain[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, CLICK_OFF },
                (int)NULL
        };

struct  msglist clickoff_yes_no[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, SAYYES },
                { KBRD_MSGS, SAYNO },
                { KBRD_MSGS, WAS_CLICK_OFF },
                (int)NULL
        };

struct  msglist speaker_explain[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, SPEAKER },
                (int)NULL
        };

struct  msglist speaker_no_enter[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, SPENOENTER },
                (int)NULL
        };

struct  msglist speaker_yes_no[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, SAYYES },
                { KBRD_MSGS, SAYNO },
                { KBRD_MSGS, RUNOVER },
                { KBRD_MSGS, SPEAKER_WORK },
                (int)NULL
        };

struct  msgtable
{
        struct  msglist *mlp;
        long    msgnum;
        short   msgtype;
        long    err_rc;
};

struct  msgtable msgtab[]=
        {
		{ keyboard_explain,     0x921001,       1,      (int)NULL },
                { keyboard_no_enter,    0x921002,       2,      (int)NULL },
                { ledson_yes_no,        0x921003,       0,      0x3001 },
                { ledsoff_yes_no,       0x921004,       0,      0x3002 },
		{ ask_102,		0x921018,	0,	(int)NULL },
                { keypad_frame,         0x921005,       2,      (int)NULL },
                { kbrd_yes_no,          0x921006,       0,      0x4001 },
                { clickon_explain,      0x921007,       2,      (int)NULL },
                { clickon_yes_no,       0x921008,       0,      0x5006 },
                { clickoff_explain,     0x921009,       2,      (int)NULL },
                { clickoff_yes_no,      0x921010,       0,      0x5008 },
                { speaker_explain,      0x921011,       1,      (int)NULL },
                { speaker_no_enter,     0x921012,       2,      (int)NULL },
                { speaker_yes_no,       0x921013,       0,      0x6003 },
        };

struct  msgtable *mtp;

ulong   menunums[][14] =
        {
                {
			0x921001,
			0x921002,
                        0x921003,
                        0x921004,
			0x921018,
                        0x921005,
                        0x921006,
                        0x921007,
                        0x921008,
			0x921009,
			0x921010,
                        0x921011,
                        0x921012,
                        0x921013
                },
                {
                        0x922001,
                        0x922002,
                        0x922003,
                        0x922004,
			0x922018,
                        0x922005,
                        0x922006,
                        0x922007,
                        0x922008,
			0x922009,
			0x922010,
                        0x922011,
                        0x922012,
                        0x922013
                },
                {
                        0x923001,
                        0x923002,
                        0x923003,
                        0x923004,
			0x923018,
                        0x923005,
                        0x923006,
                        0x923007,
                        0x923008,
			0x923009,
			0x923010,
                        0x923011,
                        0x923012,
                        0x923013
                }
        };

