/* @(#)19       1.1  src/bos/diag/tu/ppckbd/kbd_msg.h, tu_ppckbd, bos41J, 9520A_all 5/6/95 14:31:50 */
/*
 * COMPONENT_NAME: tu_ppckbd 
 *
 * FUNCTIONS: exectu (main), tu10, tu20, tu30, tu40,  
 *            SpeakerReadWriteTest, genkeymap, updatescreen, finish,
 *            SendKbdata, SendRxKbData, maketitleadvanced, setmenunumberbase,
 *            putmsg, chk_stat, set_raw_mode, restore_kbd_mode
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1995
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
#include "disakbd_msg.h"

ASL_SCR_TYPE    menutypes= DM_TYPE_DEFAULTS;

/*
 *      message lists for menus presented via asl during this da.
 */

struct  msglist keyboard_explain[]=
        {
                { ISAKBD_MSGS, TESTING },
                { ISAKBD_MSGS, KBD_LEDS },
                (int)NULL
        };

struct  msglist keyboard_no_enter[]=
        {
                { ISAKBD_MSGS, TESTING },
                { ISAKBD_MSGS, KBD_NOENTER },
                (int)NULL
        };

struct  msglist ledson_yes_no[]=
        {
                { ISAKBD_MSGS, TESTING },
                { ISAKBD_MSGS, SAYYES },
                { ISAKBD_MSGS, SAYNO },
		{ ISAKBD_MSGS, RUNOVER },
		{ ISAKBD_MSGS, LEDS_ON_OK },
                (int)NULL
        };

struct  msglist ledsoff_yes_no[]=
        {
                { ISAKBD_MSGS, TESTING },
                { ISAKBD_MSGS, SAYYES },
                { ISAKBD_MSGS, SAYNO },
		{ ISAKBD_MSGS, LEDS_OFF_OK },
                (int)NULL
        };

struct  msglist ask_102[]=
        {
                { ISAKBD_MSGS, TESTING },
/*
                { ISAKBD_MSGS, GERMAN_100 },
*/
                { ISAKBD_MSGS, BRAZIL_104 },
                { ISAKBD_MSGS, OTHER_102 },
                { ISAKBD_MSGS, ASK_102 },
                (int)NULL
        };

ASL_SCR_INFO    menu_102[DIAG_NUM_ENTRIES(ask_102)];

struct  msglist keypad_frame[]=
        {
                { ISAKBD_MSGS, TESTING },
                { ISAKBD_MSGS, KEYPAD },
                (int)NULL
        };

struct  msglist kbrd_yes_no[]=
        {
                { ISAKBD_MSGS, TESTING },
                { ISAKBD_MSGS, SAYYES },
                { ISAKBD_MSGS, SAYNO },
                { ISAKBD_MSGS, KEYPAD_WORKED },
                (int)NULL
        };

ASL_SCR_INFO	menu_kbdyn[DIAG_NUM_ENTRIES(kbrd_yes_no)];

struct  msgtable
{
        struct  msglist *mlp;
        long    msgnum;
        short   msgtype;
        long    err_rc;
};

struct  msgtable msgtab[]=
        {
		{ keyboard_explain,     0x736001,       1,      (int)NULL },
                { keyboard_no_enter,    0x736002,       2,      (int)NULL },
                { ledson_yes_no,        0x736003,       0,      0x3001 },
                { ledsoff_yes_no,       0x736004,       0,      0x3002 },
		{ ask_102,		0x736018,	0,	(int)NULL },
                { keypad_frame,         0x736005,       2,      (int)NULL },
                { kbrd_yes_no,          0x736006,       0,      0x4001 },
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
                        0x921006
                },
                {
                        0x922001,
                        0x922002,
                        0x922003,
                        0x922004,
			0x922018,
                        0x922005,
                        0x922006
                },
                {
                        0x923001,
                        0x923002,
                        0x923003,
                        0x923004,
			0x923018,
                        0x923005,
                        0x923006
                },
                {
                        0x736001,
                        0x736002,
                        0x736003,
                        0x736004,
			0x736018,
                        0x736005,
                        0x736006
                }
        };

