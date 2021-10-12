static char sccsid[] = "@(#)96  1.9.2.6  src/bos/diag/da/scsitapes/disp_mn.c, datape, bos411, 9428A410j 5/31/94 17:53:31";
/*
 * COMPONENT_NAME: datape
 *
 * FUNCTIONS:  disp_menu()
 *             chk_kb_buff()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "dtape.h"

extern char *diag_cat_gets(nl_catd, int, int);
extern void clean_up(int);

int     disp_menu(int);
void    chk_kb_buff(void);

extern DA_DATA da;
extern nl_catd catd;
extern struct tm_input tm_input;
extern struct CuDv *cudv;
extern int      step;
extern int      asl_flag;         /* 1 if ASL init.                         */
extern int      ttape_flag;       /* NO if no tape,                         */
                                  /* ENABLE catalog value before ST2,       */
                                  /* PROTECT catalog value before WP test.  */
extern int       load_flag;       /* ENABLE or PROTECT                      */
extern char test_sequence[];      /* Array to hold TU sequence string.      */
extern int test_seq;

struct msglist test_reqs[] = {
        { DTAPE_SET, MSG_TEST_REQS },
        { DTAPE_SET, YES },
        { DTAPE_SET, NO },
        { DTAPE_SET, MOVE_CURSOR },
        (int)NULL
};
struct msglist sysx_reqs[] = {
        { DTAPE_SET, MSG_SYSX_REQS },
        { DTAPE_SET, MEDIA },
        { DTAPE_SET, NO_MEDIA },
        { DTAPE_SET, MOVE_CURSOR },
        (int)NULL
};
struct msglist long_stand_by[] = {
        { DTAPE_SET, MSG_TITLE },
        { DTAPE_SET, LONG_TEST },
        { DTAPE_SET, STAND_BY },
        (int)NULL
};
struct msglist long_loopmode[] = {
        { DTAPE_SET, MSG_TITLE },
        { DTAPE_SET, LONG_TEST },
        { DTAPE_SET, LOOPMODE },
        { DTAPE_SET, CANCEL_LOOPMODE },
        (int)NULL
};
struct msglist insert_tape[] = {
        { DTAPE_SET, MSG_TITLE },
        { DTAPE_SET, REMOVE_TAPE },
        { DTAPE_SET, ENSURE_TAPE },
        { DTAPE_SET, INSERT_TAPE },
        { DTAPE_SET, WF_PRESS_ENTER },
        (int)NULL
};
struct msglist check_tape[] = {
        { DTAPE_SET, MSG_TITLE },
        { DTAPE_SET, CHECK_TAPE },
        { DTAPE_SET, TC_PRESS_ENTER },
        (int)NULL
};
struct msglist no_tape[] = {
        { DTAPE_SET, MSG_TITLE },
        { DTAPE_SET, NO_TAPE },
        { DTAPE_SET, TC_PRESS_ENTER },
        (int)NULL
};
struct msglist remove_tape[] = {
        { DTAPE_SET, MSG_TITLE },
        { DTAPE_SET, REMOVE_TAPE },
        { DTAPE_SET, WF_PRESS_ENTER },
        (int)NULL
};
struct msglist complete[] = {
        { DTAPE_SET, MSG_TITLE },
        { DTAPE_SET, COMPLETE },
        { DTAPE_SET, REMOVE_TAPE },
        { DTAPE_SET, ENSURE_TAPE },
        { DTAPE_SET, WF_PRESS_ENTER },
        (int)NULL
};
struct msglist diag_boot_complete[] = {
        { DTAPE_SET, MSG_TITLE },
        { DTAPE_SET, COMPLETE },
        { DTAPE_SET, REMOVE_TAPE },
        { DTAPE_SET, ENSURE_TAPE },
        { DTAPE_SET, DIAG_BOOT_COMPLETE },
        { DTAPE_SET, WF_PRESS_ENTER },
        (int)NULL
};
struct msglist sense_3490[] = {
        { DTAPE_SET, MSG_TITLE },
        { DTAPE_SET, SUPPORT_3490 },
        { DTAPE_SET, INTERPRET_3490 },
        { DTAPE_SET, INFO_3490 },
        { DTAPE_SET, SENSE_3490_N },
        { DTAPE_SET, MORE_3490 },
        { DTAPE_SET, RETURN_TO_SEL_MENU },
        (int)NULL
};
struct msglist did_you_clean[] = {
        { DTAPE_SET, MSG_TITLE },
        { DTAPE_SET, YES },
        { DTAPE_SET, NO },
        { DTAPE_SET, DID_YOU_CLEAN_DRIVE },
        (int)NULL
};
struct msglist did_you_try_new_ttape[] = {
        { DTAPE_SET, MSG_TITLE },
        { DTAPE_SET, YES },
        { DTAPE_SET, NO },
        { DTAPE_SET, DID_YOU_TRY_NEW_TTAPE },
        (int)NULL
};
struct msglist clean_drive_now[] = {
        { DTAPE_SET, MSG_TITLE },
        { DTAPE_SET, CLEAN_DRIVE_NOW },
        (int)NULL
};
struct msglist get_new_ttape_now[] = {
        { DTAPE_SET, MSG_TITLE },
        { DTAPE_SET, GET_NEW_TTAPE_NOW },
        (int)NULL
};

ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

/*
 * NAME: disp_menu()
 *
 * FUNCTION: Display a message or a selection menu.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: ASL rc if good,
 *          NO if no console or ASL not init,
 *          ENABLE if test tape avaiable.
 */

int disp_menu(int msgno)
{

        int     i, rc;
        int     msglist_size;
        int     screen_num;
        int     scr_type;
        static  int wewp_flag = ENABLE;  /* Write enable/protect flag. */
        char    *msg_ptr;
        char    *temp_ptr;
        char    msg_buff[2048];
        struct msglist *msglist;
        ASL_SCR_INFO *menuinfo;

        if (!CONSOLE || !asl_flag)
                return(NO);

        chk_kb_buff();
        diag_asl_clear_screen();

        /* Assign the proper screen number and message structs. */
        screen_num = dev[da.dev_type].led * 0x1000;
        switch(msgno) {
        case MSG_TEST_REQS:
                screen_num += 0x001;
                msglist = test_reqs;
                msglist_size = DIAG_NUM_ENTRIES(test_reqs);
                scr_type = ASL_DIAG_LIST_CANCEL_EXIT_SC;
                break;
        case MSG_SYSX_REQS:
                screen_num += 0x002;
                msglist = sysx_reqs;
                msglist_size = DIAG_NUM_ENTRIES(sysx_reqs);
                scr_type = ASL_DIAG_LIST_CANCEL_EXIT_SC;
                break;
        case INSERT_TAPE:
                if (load_flag == ENABLE) {
                        screen_num += 0x003;
                } else {
                        wewp_flag = PROTECT;
                        screen_num += 0x023;
                }
                msglist = insert_tape;
                msglist_size = DIAG_NUM_ENTRIES(insert_tape);
                scr_type = ASL_DIAG_KEYS_ENTER_SC;
                break;
        case STAND_BY:
                screen_num += 0x004;
                msglist = long_stand_by;
                msglist_size = DIAG_NUM_ENTRIES(long_stand_by);
                scr_type = ASL_DIAG_OUTPUT_LEAVE_SC;
                break;
        case LOOPMODE:
                screen_num += 0x005;
                msglist = long_loopmode;
                msglist_size = DIAG_NUM_ENTRIES(long_loopmode);
                scr_type = ASL_DIAG_OUTPUT_LEAVE_SC;
                break;
        case CHECK_TAPE:
                wewp_flag = ENABLE;
                screen_num += 0x006;
                msglist = check_tape;
                msglist_size = DIAG_NUM_ENTRIES(check_tape);
                scr_type = ASL_DIAG_KEYS_ENTER_SC;
                break;
        case REMOVE_TAPE:
                screen_num += 0x007;
                msglist = remove_tape;
                msglist_size = DIAG_NUM_ENTRIES(remove_tape);
                scr_type = ASL_DIAG_KEYS_ENTER_SC;
                break;
        case NO_TAPE:
                screen_num += 0x008;
                msglist = no_tape;
                msglist_size = DIAG_NUM_ENTRIES(no_tape);
                scr_type = ASL_DIAG_KEYS_ENTER_SC;
                break;
        case COMPLETE:
                screen_num += 0x050;
                msglist = complete;
                msglist_size = DIAG_NUM_ENTRIES(complete);
                scr_type = ASL_DIAG_KEYS_ENTER_SC;
                break;
        case DIAG_BOOT_COMPLETE:
                screen_num += 0x051;
                msglist = diag_boot_complete;
                msglist_size = DIAG_NUM_ENTRIES(diag_boot_complete);
                scr_type = ASL_DIAG_KEYS_ENTER_SC;
                break;
        case SENSE_3490_N:
                screen_num += 0x001;
                msglist = sense_3490;
                msglist_size = DIAG_NUM_ENTRIES(sense_3490);
                scr_type = ASL_DIAG_KEYS_ENTER_SC;
                break;
	case DID_YOU_CLEAN_DRIVE:
                screen_num += 0x030;
                msglist = did_you_clean;
                msglist_size = DIAG_NUM_ENTRIES(did_you_clean);
                scr_type = ASL_DIAG_LIST_CANCEL_EXIT_SC;
                break;
	case DID_YOU_TRY_NEW_TTAPE:
                screen_num += 0x031;
                msglist =  did_you_try_new_ttape;
                msglist_size = DIAG_NUM_ENTRIES(did_you_try_new_ttape);
                scr_type = ASL_DIAG_LIST_CANCEL_EXIT_SC;
                break;
	case CLEAN_DRIVE_NOW:
                screen_num += 0x032;
                msglist = clean_drive_now;
                msglist_size = DIAG_NUM_ENTRIES(clean_drive_now);
                scr_type = ASL_DIAG_KEYS_ENTER_SC;
                break;
	case GET_NEW_TTAPE_NOW:
                screen_num += 0x033;
                msglist = get_new_ttape_now;
                msglist_size = DIAG_NUM_ENTRIES(get_new_ttape_now);
                scr_type = ASL_DIAG_KEYS_ENTER_SC;
                break;
        default:
db(10,1,"!!!! SW ERROR",10);
db(10,1,"msgno",msgno);
                clean_up(EXIT_SW_ERR);
                break;
        } /* end switch */

db(16,1,"In disp_mn(), screen num",screen_num);

        /* Allocate for ASL INFO struct. */
        menuinfo = (ASL_SCR_INFO *)calloc(msglist_size - 1,
                                          sizeof(ASL_SCR_INFO));

        menutype.cur_index = 1;
        for (i = 0; msglist[i].msgid != 0; i++) {
                msg_ptr = (char *)diag_cat_gets(catd, DTAPE_SET,
                                                msglist[i].msgid);
                switch(msglist[i].msgid) {
                case MSG_TITLE: /* 3 substitutions */
                        if (ADVANCED)
                                temp_ptr = (char *)diag_cat_gets(catd,
                                                         DTAPE_SET,
                                                         ADV_MODE);
                        else
                                temp_ptr = (char *)diag_cat_gets(catd,
                                                         DTAPE_SET,
                                                         CUST_MODE);
                        sprintf(msg_buff, msg_ptr, cudv->PdDvLn->type,
                                tm_input.dname, tm_input.dnameloc, temp_ptr);
                        break;
                case MSG_TEST_REQS: /* 6 substitutions */
                case MSG_SYSX_REQS:
                        if (ADVANCED)
                                temp_ptr = (char *)diag_cat_gets(catd,
                                                         DTAPE_SET,
                                                         ADV_MODE);
                        else
                                temp_ptr = (char *)diag_cat_gets(catd,
                                                         DTAPE_SET,
                                                         CUST_MODE);
                        sprintf(msg_buff, msg_ptr, cudv->PdDvLn->type,
                                tm_input.dname, tm_input.dnameloc, temp_ptr,
                                (char *)diag_cat_gets(catd, DTAPE_SET,
                                              dev[da.dev_type].test_tape_pn),
                                ST2_TIME);
                        break;
                case LONG_TEST: /* 1 substitution */
                        if ((load_flag == ENABLE) && (ttape_flag == YES)) {
                                sprintf(msg_buff, msg_ptr, ST2_TIME);
                        } else {
                                sprintf(msg_buff, msg_ptr, ST1_TIME);
                        }
                        break;
                case LOOPMODE: /* 2 substitutions */
                        sprintf(msg_buff, msg_ptr,
                                tm_input.lcount, tm_input.lerrors);
                        break;
                case CHECK_TAPE: 
                        if (load_flag == ENABLE) {
                		msg_ptr = (char *)diag_cat_gets(catd, 
								DTAPE_SET,
                                                           CHECK_TAPE_WE);
                        } else {
                		msg_ptr = (char *)diag_cat_gets(catd, 
								DTAPE_SET,
                                                           CHECK_TAPE_WP);
                        }
                        strcpy(msg_buff, msg_ptr);
                        break;
                case ENSURE_TAPE: /* 1 substitution */
                        if ((load_flag == ENABLE) || (msgno == COMPLETE) || 
			    (msgno == DIAG_BOOT_COMPLETE)) {
                                /* Write Enable. */
                		msg_ptr = (char *)diag_cat_gets(catd, 
								DTAPE_SET,
                                                           ENSURE_TAPE_WE);
                                sprintf(msg_buff, msg_ptr,
                                        (char *)diag_cat_gets(catd,
                                                      DTAPE_SET,
                                                      dev[da.dev_type].we_msg));
                        } else {
                                /* Write Protect. */
                		msg_ptr = (char *)diag_cat_gets(catd, 
								DTAPE_SET,
                                                           ENSURE_TAPE_WP);
                                sprintf(msg_buff, msg_ptr,
                                        (char *)diag_cat_gets(catd,
                                                      DTAPE_SET,
                                                      dev[da.dev_type].wp_msg));
                        }
                        break;
                case SENSE_3490_N:
                        strcpy(msg_buff, msg_3490_buff);
                        menutype.cur_win_offset = 0;
                        menutype.cur_win_index = 0;
                        break;
		case DID_YOU_CLEAN_DRIVE:
		case DID_YOU_TRY_NEW_TTAPE:
        		menutype.cur_index = 2;
                        strcpy(msg_buff, msg_ptr);
                	break;
                default: /* No substitutions. */
                        strcpy(msg_buff, msg_ptr);
                        break;
                } /* end switch */
                menuinfo[i].text = (char *)malloc(strlen(msg_buff) + 1);
                strcpy(menuinfo[i].text, msg_buff);
        } /* end for */

        menutype.max_index = i - 1; /* Zero based */

        /* Display the message or selection menu.  */
        rc = diag_display(screen_num, catd, NULL, DIAG_IO,
                          scr_type, &menutype, menuinfo);

        switch(rc) {
        case DIAG_ASL_ERR_SCREEN_SIZE:
        case DIAG_ASL_FAIL:
        case DIAG_ASL_ERR_NO_TERM:
        case DIAG_ASL_ERR_NO_SUCH_TERM:
        case DIAG_ASL_ERR_INITSCR:
db(10,2,"!!!! SW ERROR",11);
                clean_up(EXIT_SW_ERR);
                break;
        case DIAG_ASL_EXIT:
        case DIAG_ASL_CANCEL:
                if (rc == DIAG_ASL_CANCEL)
                        DA_SETRC_USER(DA_USER_QUIT);
                else
                        DA_SETRC_USER(DA_USER_EXIT);
                if (SYSX && ENTERLM) {
                        diag_asl_quit();
                        asl_flag = 0;
                        clrdainput();
                }
                /* Exit DA by calling clean_up().     */
                switch(msgno) {
                case DIAG_BOOT_COMPLETE:     
                case COMPLETE:      /* The COMPLETE msg's are called from */
                case SENSE_3490_N:  /* clean_up, the sense_3490 msg will  */
                        break;      /* terminate ELA, let them return.    */
                default:
                        clean_up(EXIT_NTF);
                        break;
                }
                break;
        default:
                break; /* Good or unknown status, return. */
        } /* end switch */

        if ((msgno == MSG_TEST_REQS) || (msgno == MSG_SYSX_REQS) ||
	    (msgno == DID_YOU_CLEAN_DRIVE) || (msgno == DID_YOU_TRY_NEW_TTAPE))
                rc = (DIAG_ITEM_SELECTED(menutype) == 1) ? YES : NO;

        return(rc);

} /* end disp_menu */

/*
 * NAME:  void chk_kb_buff()
 *
 * FUNCTION: If Cancel or Exit, clean up with No Trouble Found.
 *
 * NOTES: This will also check and clear the key board buffer.
 *
 * RETURNS: void
 *
 */

void chk_kb_buff(void)
{
        long rc;

        if (!CONSOLE || !asl_flag)
                return;

        rc = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, NULL, NULL);
        if (rc == DIAG_ASL_CANCEL) {
                DA_SETRC_USER(DA_USER_QUIT);
                clean_up(EXIT_NTF);
        }
        if(rc == DIAG_ASL_EXIT) {
                DA_SETRC_USER(DA_USER_EXIT);
                clean_up(EXIT_NTF);
        }

        return;

} /* end chk_kb_buff */

