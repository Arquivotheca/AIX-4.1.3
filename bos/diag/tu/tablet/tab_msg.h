/* static char sccsid[] = "@(#)16       1.3  src/bos/diag/tu/tablet/tab_msg.h, tu_tab, bos411, 9428A410j 5/12/94 15:13:26"; */
/*
 * COMPONENT_NAME:  tu_tab
 *
 * FUNCTIONS: 
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

#define YES     1
#define NO      2


#define CANCEL_KEY_ENTERED  7
#define EXIT_KEY_ENTERED    8


#include "diag/diago.h"
#include "dtablet_msg.h"

ASL_SCR_TYPE    menutypes= DM_TYPE_DEFAULTS;

/*
 *      message lists for menus presented via asl during this da.
 */
struct  msglist blink_led []=           /* tu 40 */
        {
                { TABLET_DIAG, TM_1A },
                { TABLET_DIAG, TM_5 },
		{ TABLET_DIAG, TM_6 },
                { TABLET_DIAG, TM_11 },
                (int)NULL
        };

struct  msglist rsquare_ledon []=       /* tu 50A */
        {
                { TABLET_DIAG, TM_1A },
                { TABLET_DIAG, TM_5 },
                { TABLET_DIAG, TM_6 },
                { TABLET_DIAG, TM_22 },
                (int)NULL
        };

struct  msglist rsquare_ledoff []=      /* tu 50B */
        {
                { TABLET_DIAG, TM_1A },
                { TABLET_DIAG, TM_5 },
                { TABLET_DIAG, TM_6 },
                { TABLET_DIAG, TM_23 },
                (int)NULL
        };

struct  msglist stylus_ledon []=        /* tu 60A */
        {
                { TABLET_DIAG, TM_1A },
                { TABLET_DIAG, TM_5 },
                { TABLET_DIAG, TM_6 },
                { TABLET_DIAG, TM_24 },
                (int)NULL
        };

struct  msglist stylus_ledoff []=       /* tu 60B */
        {
                { TABLET_DIAG, TM_1A },
                { TABLET_DIAG, TM_5 },
                { TABLET_DIAG, TM_6 },
                { TABLET_DIAG, TM_23A },
                (int)NULL
        };

struct  msglist motion_ledon []=        /* tu 70A */
        {
                { TABLET_DIAG, TM_1A },
                { TABLET_DIAG, TM_5 },
                { TABLET_DIAG, TM_6 },
                { TABLET_DIAG, TM_37 },
                (int)NULL
        };

struct  msglist motion_ledoff []=       /* tu 70B */
        {
                { TABLET_DIAG, TM_1A },
                { TABLET_DIAG, TM_5 },
                { TABLET_DIAG, TM_6 },
                { TABLET_DIAG, TM_38 },
                (int)NULL
        };

struct  msgtable
{
        struct  msglist *mlp;
        long    msgnum;
        long    err_rc;
};

/* N.B. -- the following numbers, 0x921003, ..., 0x921011, need to be changed
to reflect the fact that this is tablet and not keyboard code.  Polly Orr
suggests picking them up from line 107 of:

                       .../da/tablet/dtablet.c

but it is unclear how these numbers can readily be extracted from this portion
of code.
*/
struct  msgtable msgtab[]=
        {
                { blink_led,            0x926402,   0x4001 },
                { rsquare_ledon,        0x926601,   0x5001 },
                { rsquare_ledoff,       0x926602,   0x5002 },
                { stylus_ledon,         0x926702,   0x6001 },
                { stylus_ledoff,        0x926701,   0x6002 },
                { motion_ledon,         0x926121,   0x7002 },
                { motion_ledoff,        0x926122,   0x7003 },
        };

struct  msgtable *mtp;

ulong   menunums[][7] =
        {
                {
                        0x926402,
                        0x926601,
                        0x926602,
                        0x926702,
                        0x926701,
                        0x926121,
                        0x926122,
                },
                {
                        0x927402,
                        0x927601,
                        0x927602,
                        0x927702,
                        0x927701,
                        0x927121,
                        0x927122,
                }
        };


