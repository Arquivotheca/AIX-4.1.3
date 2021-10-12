static char sccsid[] = "@(#)37  1.14.1.5  src/bos/diag/da/cdrom/disp_menu.c, dacdrom, bos41B, 9505A 1/10/95 20:38:10";
/*
 *   COMPONENT_NAME: DACDROM
 *
 *   FUNCTIONS: DIAG_NUM_ENTRIES
 *		disp_menu
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* #define PLEASE_WAIT */

#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <nl_types.h>
#include <sys/types.h>
#include <diag/scsd.h>

#include "diag/tmdefs.h"        /* diagnostic modes and variables     */
#include "diag/tm_input.h"
#include "diag/da.h"            /* FRU Bucket Database */
#include "diag/da_rc.h"
#include "diag/diago.h"
#include "diag/diag_exit.h"     /* Return codes */

#include "dcdrom_msg.h"
#include "dcdrom.h"

#define MAXTEMP 2048
#define XA_TEST_DISC    "81F8902"
#define TEST_DISC       "53F3088/81F8902"
extern int led_num;
extern is_scsd_device;
extern void    clean_up();
extern char *diag_cat_gets();
extern nl_catd catd;

struct msglist  menulist0[] =
{
 {CDROM_MENU, MENU_HEAD,},
 {CDROM_MENU, 0,},
 (int)NULL
};

struct msglist  menulist1[] =
{
 {CDROM_MENU, MENU_HEAD,},
 {CDROM_MENU, YES_OPTION,},
 {CDROM_MENU, NO_OPTION,},
 {CDROM_MENU, MENU_ONE,},
 (int)NULL
};

struct msglist  menulist2[] =
{
 {CDROM_MENU, EXER_MENU,},
 {CDROM_MENU, OPTION_EXER_1,},
 {CDROM_MENU, OPTION_EXER_2,},
 {CDROM_MENU, MOVE_CURSOR,},
 (int)NULL
};

ASL_SCR_INFO    menuonly[DIAG_NUM_ENTRIES(menulist0)];

ASL_SCR_INFO    menuinfo[DIAG_NUM_ENTRIES(menulist1)];

ASL_SCR_INFO    menuexer[DIAG_NUM_ENTRIES(menulist2)];

ASL_SCR_TYPE    menutype = DM_TYPE_DEFAULTS;


/* MENU NUMBERS
 *
 * Each menu has a unique number.  50 is added when in advanced mode.
 * A "working" menu is displayed after each menu in which a person
 * has to press a key.
 * It has the number of the proceding menu plus 100.
 */



/*
 * NAME: disp_menu
 *
 * FUNCTION: display a selected menu from the cat file
 *
 * DATA STRUCTURES: reads tm_input for location code
 *
 * RETURNS: returns return code from diag_display
 */

disp_menu(selected_message)
        int             selected_message;       /* message number */
                                                /* in cat file     */
{
        extern  struct  tm_input tm_input;      /* input params for main */
        extern  int     non_xa_mode;
	extern  int	has_caddy;
	extern  char    scsd_media_pn[SCSD_CD_MEDIA_PN];

        int             response = 0;
        int             rc;
        int             menunum;
        char            msgstr[1024];
        char            exec_mode[MAXTEMP];
        char            menu_head[MAXTEMP];
        char            temp_buff[MAXTEMP];

        menulist0[1].msgid = selected_message;
        menulist1[3].msgid = selected_message;

        if (tm_input.advanced == ADVANCED_TRUE) {
                strncpy(exec_mode,
                        diag_cat_gets(catd, CDROM_MENU, MENU_ADVANCED_MODE, ""),
                        MAXTEMP);
                /* offset for advanced menus */
                menunum = 0x050;
        } else {
                strncpy(exec_mode,
                        diag_cat_gets(catd, CDROM_MENU, MENU_NORMAL_MODE, ""),
                        MAXTEMP);
                menunum = 0;
        }

	menunum += (led_num * 0x1000)+(selected_message - MENU_ONE);

        strncpy(temp_buff, diag_cat_gets(catd, CDROM_MENU, MENU_HEAD, ""),
                MAXTEMP);
        sprintf(menu_head, temp_buff, tm_input.dnameloc, exec_mode);

        switch (selected_message) {
        case MENU_CHECKOUT:
                /* display the selected menu */
                rc = diag_display(menunum, catd, menulist0,
                                  DIAG_MSGONLY, NULL, &menutype, menuonly);
                menuonly[0].text = menu_head;
                rc = diag_display(menunum, catd, NULL, DIAG_IO,
                                  ASL_DIAG_OUTPUT_LEAVE_SC,
                                  &menutype, menuonly);
                break;

        case MENU_ONE:
        case MENU_THREE:
        case MENU_NINE:
        case MENU_THIRTEEN:
                /* display the selected menu */
                rc = diag_display(menunum, catd, menulist1,
                                  DIAG_MSGONLY, NULL, &menutype, menuinfo);
                menuinfo[0].text = menu_head;
                if(selected_message == MENU_ONE){
			if (non_xa_mode)
				sprintf(temp_buff, menuinfo[3].text,
                                        TEST_DISC);
			else if (is_scsd_device) 
				sprintf(temp_buff, menuinfo[3].text,
                                        scsd_media_pn);
                        else
                                sprintf(temp_buff, menuinfo[3].text,
                                        XA_TEST_DISC);
                        menuinfo[3].text = temp_buff;
                }
                rc = diag_display(menunum, catd, NULL, DIAG_IO,
                                  ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                  &menutype, menuinfo);
                if ((rc == ASL_EXIT) || (rc == ASL_CANCEL))
                        break;
                response = DIAG_ITEM_SELECTED(menutype);
#ifdef PLEASE_WAIT
                /* now display the working menu */
                menunum += 0x100;
                menulist0[1].msgid = MENU_WORKING;
                rc = diag_display(menunum, catd, menulist0,
                                  DIAG_MSGONLY, NULL, &menutype, menuonly);
                menuonly[0].text = menu_head;
                rc = diag_display(menunum, catd, NULL, DIAG_IO,
                                  ASL_DIAG_OUTPUT_LEAVE_SC,
                                  &menutype, menuonly);
                if ((rc == ASL_EXIT) || (rc == ASL_CANCEL))
                        break;
#endif
                break;

        case NEW_MEDIA_MSG:
        case NEW_MEDIA_MSG_TRAY:
        case MENU_TWO:
        case MENU_FOUR:
	case MENU_FOUR_TRAY:
        case MENU_TEN:
        case MENU_ELEVEN:
                /* display the selected menu */
                rc = diag_display(menunum, catd, menulist0,
                                  DIAG_MSGONLY, NULL, &menutype, menuonly);
                menuonly[0].text = menu_head;
                if ((selected_message == NEW_MEDIA_MSG)
		    || (selected_message == NEW_MEDIA_MSG_TRAY)) {
			if (non_xa_mode)
				sprintf(temp_buff, menuonly[1].text,
                                        TEST_DISC);
			else if (is_scsd_device) 
				sprintf(temp_buff, menuonly[1].text,
                                        scsd_media_pn);
                        else
                                sprintf(temp_buff, menuonly[1].text,
                                        XA_TEST_DISC);
                        menuonly[1].text = temp_buff;
                }
                rc = diag_display(menunum, catd, NULL, DIAG_IO,
                                  ASL_DIAG_KEYS_ENTER_SC,
                                  &menutype, menuonly);
                if ((rc == ASL_EXIT) || (rc == ASL_CANCEL))
                        break;
#ifdef PLEASE_WAIT
                /* now display the working menu */
                menunum += 0x100;
                menulist0[1].msgid = MENU_WORKING;
                rc = diag_display(menunum, catd, menulist0,
                                  DIAG_MSGONLY, NULL, &menutype, menuonly);
                menuonly[0].text = menu_head;
                rc = diag_display(menunum, catd, NULL, DIAG_IO,
                                  ASL_DIAG_OUTPUT_LEAVE_SC,
                                  &menutype, menuonly);
                if ((rc == ASL_EXIT) || (rc == ASL_CANCEL))
                        break;
#endif
                break;
        case LOOP_MODE_MENU:
        case LAST_LOOP_MENU:
                /* display the selected menu */
                rc = diag_display(menunum, catd, menulist0,
                                  DIAG_MSGONLY, NULL, &menutype, menuonly);
                sprintf(temp_buff, menuonly[1].text,
                          tm_input.lcount, tm_input.lerrors);
                menuonly[0].text = menu_head;
                menuonly[1].text = temp_buff;
                rc = diag_display(menunum, catd, NULL, DIAG_IO,
                                  ASL_DIAG_OUTPUT_LEAVE_SC,
                                  &menutype, menuonly);
                break;
        case MENU_GOAL_3:
                sprintf(msgstr, diag_cat_gets(catd, CDROM_MENU,
                                              MENU_GOAL_3, NULL),
                        menunum, tm_input.dnameloc, exec_mode);
                menugoal(msgstr);
                rc = ASL_ENTER;
                break;
        case EXER_MENU:
                rc = diag_display(menunum, catd, menulist2,
                                  DIAG_MSGONLY, NULL, &menutype, menuexer);
		if (non_xa_mode)
			sprintf(temp_buff, menuexer[0].text,
                                tm_input.dnameloc,TEST_DISC);
		else if (is_scsd_device) 
				sprintf(temp_buff, menuexer[0].text,
                                tm_input.dnameloc,scsd_media_pn);
                else
                        sprintf(temp_buff, menuexer[0].text,
                                tm_input.dnameloc,XA_TEST_DISC);
                menuexer[0].text = temp_buff;
                rc = diag_display(menunum, catd, NULL, DIAG_IO,
                                  ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                  &menutype, menuexer);
                if ((rc == ASL_EXIT) || (rc == ASL_CANCEL))
                        break;
                rc = DIAG_ITEM_SELECTED(menutype);
                if (rc == 1)
                        response = 1;
                else
                        response = 0;
                break;
        case EXER_INSERT:
        case EXER_INSERT_TRAY:
                rc = diag_display(menunum, catd, menulist0,
                                  DIAG_MSGONLY, NULL, &menutype, menuonly);
                menuonly[0].text = menu_head;
		if (non_xa_mode)
			sprintf(temp_buff, menuonly[1].text,
                                TEST_DISC);
		else if (is_scsd_device) 
				sprintf(temp_buff, menuonly[1].text,
                                        scsd_media_pn);
                else
                        sprintf(temp_buff, menuonly[1].text,
                                XA_TEST_DISC);
                menuonly[1].text = temp_buff;
                rc = diag_display(menunum, catd, NULL, DIAG_IO,
                                  ASL_DIAG_KEYS_ENTER_SC,
                                  &menutype, menuonly);
                break;
        case REMOVE_THE_DISK:
	case REMOVE_THE_DISK_TRAY:
                rc = diag_display(menunum, catd, menulist0,
                                  DIAG_MSGONLY, NULL, &menutype, menuonly);
                menuonly[0].text = menu_head;
                rc = diag_display(menunum, catd, NULL, DIAG_IO,
                                  ASL_DIAG_KEYS_ENTER_SC,
                                  &menutype, menuonly);
                break;
        case CHK_ASL_READ:
                rc = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC,NULL,NULL);
                break;
	case MSG_UNKNOWN_DEVICE:
                sprintf(msgstr, diag_cat_gets(catd, CDROM_MENU,
                                              MSG_UNKNOWN_DEVICE, NULL),
                        menunum, tm_input.dname, tm_input.dnameloc);
                menugoal(msgstr);
                rc = ASL_ENTER;
                break;
        default:
                response = 9;   /* means error cond */
                break;
        }

        /* check if the user hit to exit and in case, do it. */
        if (rc == ASL_EXIT) {
                DA_SETRC_STATUS(DA_STATUS_GOOD);
                DA_SETRC_ERROR(DA_ERROR_NONE);
                DA_SETRC_TESTS(DA_TEST_NOTEST);
                DA_SETRC_USER(DA_USER_EXIT);
                DA_SETRC_MORE(DA_MORE_NOCONT);
                if (tm_input.exenv == EXENV_SYSX) {
                        if (tm_input.console == CONSOLE_TRUE)
                                diag_asl_quit();
                }
                clean_up();
        }

        if (rc == ASL_CANCEL) {
                DA_SETRC_STATUS(DA_STATUS_GOOD);
                DA_SETRC_ERROR(DA_ERROR_NONE);
                DA_SETRC_TESTS(DA_TEST_NOTEST);
                DA_SETRC_USER(DA_USER_QUIT);
                DA_SETRC_MORE(DA_MORE_NOCONT);
                if (tm_input.exenv == EXENV_SYSX) {
                        if (tm_input.console == CONSOLE_TRUE)
                                diag_asl_quit();
                }
                clean_up();
        }

        return (response);
}
