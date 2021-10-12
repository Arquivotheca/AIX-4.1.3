static char sccsid[] = "@(#)39  1.3.2.17  src/bos/diag/da/async/dasync_asl.c, daasync, bos41J, 9521B_all 5/23/95 17:13:24";
/*
 * COMPONENT_NAME: DAASYNC
 *
 * FUNCTIONS:   chk_asl_stat ()
 *              dsply_tst_slctn ()
 *              select_attached_device ()
 *              dsply_tst_lst ()
 *              wrap_plug_install ()
 *              wrap_plug_remove ()
 *              dsply_tst_hdr ()
 *              dsply_cfg_hdr ()
 *              select_attached_cabling()
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 */
#include        "dasync.h"

/*
 * NAME: chk_asl_stat
 *
 * FUNCTION:  Checks keyboard input by user during diagnostics.  The escape
 *      or F3 keys will cause the DA to return to the controller with no
 *      further action.  The enter key will be treated as a command to continue
 *      testing.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

chk_asl_stat (asl_return)
long    asl_return;
{
        void    init_frub ();
        void    add_fru ();
        void    exit_da ();

        switch (asl_return)
        {
        case ASL_ERR_SCREEN_SIZE:
        case ASL_FAIL:
        case ASL_ERR_NO_TERM:
        case ASL_ERR_NO_SUCH_TERM:
        case ASL_ERR_INITSCR:
                init_frub ();
                frub[0].sn = Adptr_name;
                frub[0].rcode = 0x910;
                frub[0].rmsg = RM_LM;
                insert_fru = TRUE;
                last_tu = TRUE;
                add_fru ();
                err(0x145,asl_return,0);
                exit_da ();
                break;
        case ASL_EXIT:
                DA_SETRC_USER(DA_USER_EXIT);
                DA_SETRC_TESTS (DA_TEST_FULL);
                if (fru_set == TRUE)
                {
                        insert_fru = FALSE;
                        last_tu = TRUE;
                        add_fru ();
                }  /* endif */
                exit_da ();
                break;
        case ASL_CANCEL:
                DA_SETRC_USER(DA_USER_QUIT);
                DA_SETRC_TESTS (DA_TEST_FULL);
                if (fru_set == TRUE)
                {
                        insert_fru = FALSE;
                        last_tu = TRUE;
                        add_fru ();
                }  /* endif */
                exit_da ();
                break;
        default:
                break;
        }  /* end switch (asl_return) */
}  /* chk_asl_stat end */

/*
 * NAME: dsply_tst_slctn
 *
 * FUNCTION: Display diagnostic selection test menu.
 *
 * NOTES: The selection made is written into the 'selection' argument.
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      -1              : error
 *      DIAG_ASL_COMMIT
 *      DIAG_ASL_CANCEL
 *      DIAG_ASL_EXIT
 */

dsply_tst_slctn (menu_num)
int     menu_num;
{
        int             index;
        int             res_rc;
        int             i, j, k;
        int             line = 0;
        int             msg_nmbr;
        char            bufstr[128];
        char            prtnum[] = "00";
        char            msgstr[512];
        char            *lctn;
        char            *dv_str;
        char            *dv_msg;
        ASL_SCR_INFO    *menu_da;
        ASL_SCR_TYPE    menutype = DM_TYPE_DEFAULTS;

        if (l_mode == NOTLM || l_mode == ENTERLM)
        {
                menu_da = (ASL_SCR_INFO *)
                    calloc(1,(num_PdCn+3)*sizeof(ASL_SCR_INFO));
                if ( menu_da == (ASL_SCR_INFO *) NULL) {
                        err(0x146,0,0);
                        return(-1);
                }
                if (a_mode == ADVANCED)
                        switch (Adptr_name)
                        {
                        case SP1:
                        case SP2:
                                msg_nmbr = DM_37B;
                                if (Adptr_name == SP1)
                                        strcpy (portnum, "S1");
                                else
                                        strcpy (portnum, "S2");
                                break;
                        case SP3:
                                msg_nmbr = DM_37B;
                                strcpy (portnum, "S3");
                                break;
                        case EIA_232_8:
                        case EIA_232_8ISA:
                        case EIA_232_16:
                                msg_nmbr = DM_38B;
                                break;
                        case EIA_422_8:
                        case EIA_422_16:
                                msg_nmbr = DM_39B;
                                break;
                        case M_S_188_8:
                                msg_nmbr = DM_40B;
                                break;
                        case EIA_232_64:
                                msg_nmbr = DM_41B;
                                break;
                        case EIA_232_128:
                        case EIA_232_128ISA:
                                msg_nmbr = DM_29;
                                break;
                        }  /* end switch (Adptr_name) */
                else
                        switch (Adptr_name)
                        {
                        case SP1:
                        case SP2:
                                msg_nmbr = DM_36A;
                                if (Adptr_name == SP1)
                                        strcpy (portnum, "S1");
                                else
                                        strcpy (portnum, "S2");
                                break;
                        case SP3:
                                msg_nmbr = DM_36A;
                                strcpy (portnum, "S3");
                                break;
                        case EIA_232_8:
                        case EIA_232_8ISA:
                        case EIA_232_16:
                                msg_nmbr = DM_38A;
                                break;
                        case EIA_422_8:
                        case EIA_422_16:
                                msg_nmbr = DM_39A;
                                break;
                        case M_S_188_8:
                                msg_nmbr = DM_40A;
                                break;
                        case EIA_232_64:
                                msg_nmbr = DM_41A;
                                break;
                        case EIA_232_128:
                        case EIA_232_128ISA:
                                msg_nmbr = DM_28;
                                break;
                        }  /* end switch (Adptr_name) */
                /* set the title line in the array      */
                sprintf (msgstr, "%s", (char *) diag_cat_gets (catd,
                    ASYNC_DIAG, msg_nmbr));
                menu_da[line].text = (char *) malloc (strlen(msgstr)+1);
                strcpy (menu_da[line].text, msgstr);
                switch (Adptr_name)
                {
                case SP1:
                case SP2:
                case SP3:
                        sprintf (msgstr, menu_da[line].text, portnum);
                        break;
                case EIA_232_8:
                case EIA_232_8ISA:
                case EIA_422_8:
                case M_S_188_8:
                        sprintf (msgstr, menu_da[line].text, num_PdCn,
                            da_input.dnameloc);
                        break;
                case EIA_232_16:
                case EIA_422_16:
                        sprintf (msgstr, menu_da[line].text, num_PdCn+6,
                            da_input.dnameloc);
                        break;
                case EIA_232_64:
                case EIA_232_128:
                case EIA_232_128ISA:
                        sprintf (msgstr, menu_da[line].text,
                            da_input.dnameloc);
                        break;
                }  /* end switch (Adptr_name) */
                free (menu_da[line].text);
                menu_da[line].text = (char *) malloc (strlen(msgstr)+1);
                strcpy (menu_da[line].text, msgstr);
                line++;
                dv_catd = diag_device_catopen (dv_name, 0);
                dv_msg = (char *) malloc (num_PdCn*132);
                if (dv_msg == (char *) NULL) {
                        err(0x147,0,0);
                        return (-1);
                }
                dv_msg = (char *) diag_device_gets (dv_catd, Set_num, Msg_num, "n/a");
                slct_port = line;
                dv_str = (char *) malloc (num_PdCn*132);
                if (dv_str == (char *) NULL) {
                        err(0x148,0,0);
                        return (-1);
                }
                lctn = (char *) malloc (NAMESIZE+1);
                j = 0;
                for(index=0; index < num_PdCn; index++)
                {
                        if ((strncmp (pdcn[index].connkey, pdef, 2)) &&
                            (strncmp (pdcn[index].connkey, pcfg, 2)))
                        {
                            if (Adptr_name == EIA_232_128 ||
                                Adptr_name == EIA_232_128ISA) {
                                strcpy (lctn, da_input.dnameloc);
                                strcat (lctn,"-");
                                i = index % 16; /* Determine port # (16 port/RAN) */
                                if (i < 10)
                                {
                                        prtnum[0] = '0';
                                        prtnum[1] = i + 0x30;
                                }  /* endif */
                                else
                                {
                                        prtnum[0] = '1';
                                        prtnum[1] = i + 0x26;
                                }  /* endif */
                                strcat (lctn, prtnum);
                                sprintf (dv_str, "%-16s %-16.16s %s",
                                    "tty*", lctn, dv_msg);
                                cudv_ptr[index] = index;
                            }
                            else {
                                /* lctn = (char *) malloc (NAMESIZE+1); */
                                strcpy (lctn, da_input.dnameloc);
                                if (index < 16)
                                {
                                        strcat (lctn, "-01-");
                                        i = index;
                                }  /* endif */
                                if (index > 15 && index < 32)
                                {
                                        strcat (lctn, "-02-");
                                        i = index - 16;
                                }  /* endif */
                                if (index > 31 && index < 48)
                                {
                                        strcat (lctn, "-03-");
                                        i = index - 32;
                                }  /* endif */
                                if (index > 47 && index < 64)
                                {
                                        strcat (lctn, "-04-");
                                        i = index - 48;
                                }  /* endif */
                                if (index > 63 && index < 80)
                                {
                                        strcat (lctn, "-05-");
                                        i = index - 64;
                                }  /* endif */
                                if (index > 79 && index < 96)
                                {
                                        strcat (lctn, "-06-");
                                        i = index - 80;
                                }  /* endif */
                                if (index > 95 && index < 112)
                                {
                                        strcat (lctn, "-07-");
                                        i = index - 96;
                                }  /* endif */
                                if (index > 111 && index < 128)
                                {
                                        strcat (lctn, "-08-");
                                        i = index - 112;
                                }  /* endif */
                                if (i < 10)
                                {
                                        prtnum[0] = '0';
                                        prtnum[1] = i + 0x30;
                                }  /* endif */
                                else
                                {
                                        prtnum[0] = '1';
                                        prtnum[1] = i + 0x26;
                                }  /* endelse */
                                strcat (lctn, prtnum);
                                sprintf (dv_str, "%-16s %-16.16s %s",
                                    "tty*", lctn, dv_msg);
                                cudv_ptr[index] = index;
                            }
                        }  /* endif */
                        else
                        {
                                sprintf (dv_str, "%-16s %-16.16s %s",
                                        C_cudv[j].name,
                                        C_cudv[j].location, dv_msg);
                                cudv_ptr[index] = j;
                                j++;
                        }  /* endelse */
                        menu_da[line].text = dv_str;
                        menu_da[line].non_select = ASL_NO;
                        dv_str = dv_str + strlen(dv_str)+1;
                        line++;
                }  /* endfor */
                free (lctn);
                /* finally add the last line */
                sprintf (msgstr, "%s", (char *) diag_cat_gets (catd,
                    ASYNC_DIAG, DM_43));
                free (menu_da[line].text);
                menu_da[line].text = (char *) malloc (strlen(msgstr)+1);
                strcpy (menu_da[line].text, msgstr);
                menutype.max_index = line;
                /* now display menu */
                asl_rc = diag_display(menu_num, catd, NULL, DIAG_IO,
                    ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, menu_da);
                slct_port = DIAG_ITEM_SELECTED(menutype);
                if (strlen (lctn))
                        free (lctn);
                if (strlen (dv_str))
                        free (dv_str);
                if (strlen (dv_msg))
                        free (dv_msg);
                port_slctd = slct_port - 1;
                catclose (dv_catd);
                putdavar (da_input.dname, "pnum", DIAG_INT, &port_slctd);
                chk_asl_stat (asl_rc);
        }  /* endif */
        else
        {
                getdavar (da_input.dname, "pnum", DIAG_INT, &port_slctd);
                asl_rc = diag_asl_read (ASL_DIAG_OUTPUT_LEAVE_SC, FALSE,
                    bufstr);
                chk_asl_stat (asl_rc);
        }  /* endelse */
        return (port_slctd);
}  /* dsply_tst_slctn end */

/*
 * NAME: select_attached_device
 *
 * FUNCTION:  Builds a title line that describes the adapter/device under
 *      test.  Asks the user to select which type of device is attached to
 *      the adapter.  Wrap plug flags are retrieved when running tests in
 *      loopmode.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:  Global variable "att_dev" is set.  att_dev is used to determine
 *      which wrap plug is required and which test units to execute.
 *
 * RETURNS: NONE
 */

select_attached_device ()  /* begin select_attached_device */
{
        char    msgstr[512];
        char    bufstr[128];
        struct  msglist da_menu[] =
        {
                { ASYNC_DIAG, DM_1A, },
                { ASYNC_DIAG, DM_10, },
                { ASYNC_DIAG, DM_11, },
                { ASYNC_DIAG, DM_12, },
                { ASYNC_DIAG, DM_78, },
                { ASYNC_DIAG, DM_13, },
                { ASYNC_DIAG, DM_9,  },
                Null
        };
        ASL_SCR_TYPE menutype =
            {
                ASL_DIAG_ENTER_SC, 6, 1
        };
        ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];

        if (l_mode == NOTLM || l_mode == ENTERLM)
        {
                Menu_nmbr &= 0xFFF000;          /* Menu_nmbr == 0xXXX001     */
                Menu_nmbr += 0x000103;          /* Menu_nmbr == 0xXXX001     */
                memset (menu_da, 0, sizeof (menu_da));
                if (Adptr_name == EIA_422_8 || Adptr_name == EIA_422_16)
                {
                        da_menu[0].msgid = DM_3B;
                        da_menu[1].msgid = DM_10;
                        da_menu[2].msgid = DM_12;
                        da_menu[3].msgid = DM_13;
                        da_menu[4].msgid = DM_9;
                        da_menu[5].setid = 0;
                        da_menu[5].msgid = 0;
                        da_menu[6].setid = 0;
                        da_menu[6].msgid = 0;
                }  /* endif */
                else
                {
                        switch (Adptr_name)
                        {
                        case SP1:
                        case SP2:
                        case SP3:
                                da_menu[0].msgid = DM_1B;
                                break;
                        case EIA_232_8:
                        case EIA_232_8ISA:
                        case EIA_232_16:
                                da_menu[0].msgid = DM_2B;
                                da_menu[4].msgid = DM_13;
                                da_menu[5].msgid = DM_9;
                                da_menu[6].setid = 0;
                                da_menu[6].msgid = 0;
                                break;
                        case M_S_188_8:
                                da_menu[0].msgid = DM_4B;
                                da_menu[4].msgid = DM_13;
                                da_menu[5].msgid = DM_9;
                                da_menu[6].setid = 0;
                                da_menu[6].msgid = 0;
                                break;
                        case EIA_232_64:
                                da_menu[0].msgid = DM_5B;
                                da_menu[4].msgid = DM_13;
                                da_menu[5].msgid = DM_9;
                                da_menu[6].setid = 0;
                                da_menu[6].msgid = 0;
                                break;
                        case EIA_232_128:
                        case EIA_232_128ISA:
                                da_menu[0].msgid = DM_15B;
                                da_menu[4].msgid = DM_13;
                                da_menu[5].msgid = DM_79;
                                break;
                        }  /* end switch (Adptr_name) */
                }  /* endelse */
                diag_display (Menu_nmbr, catd, da_menu, DIAG_MSGONLY,
                    ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, menu_da);
                switch (Adptr_name)
                {
                case SP1:
                case SP2:
                case SP3:

                        sprintf(msgstr, menu_da[0].text, portnum);
                        break;
                case EIA_232_8:
                case EIA_232_8ISA:
                case EIA_232_16:
                case EIA_422_8:
                case EIA_422_16:
                case M_S_188_8:
                case EIA_232_64:
                        sprintf(msgstr, menu_da[0].text, prt_num, n_ports,
                            da_input.dnameloc);
                        break;
                case EIA_232_128:
                case EIA_232_128ISA:
                        sprintf(msgstr, menu_da[0].text, prt_num, da_input.dname,
                             da_input.dnameloc);
                        break;
                }  /* end switch (Adptr_name) */
                free (menu_da[0].text);
                menu_da[0].text = (char *) malloc (strlen(msgstr)+1);
                strcpy (menu_da[0].text, msgstr);
                asl_rc = diag_display (Menu_nmbr, catd, NULL, DIAG_IO,
                    ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, menu_da);
                att_dev = DIAG_ITEM_SELECTED (menutype);
                putdavar (da_input.dname, "dev", DIAG_INT, &att_dev);
                chk_asl_stat (asl_rc);
        }  /* endif */
        if (l_mode == INLM || l_mode == EXITLM)
        {
                getdavar (da_input.dname, "dev", DIAG_INT, &att_dev);
                getdavar (da_input.dname, "scc", DIAG_INT, &ccbl);
                getdavar (da_input.dname, "wp1", DIAG_INT, &wrap_1);
                getdavar (da_input.dname, "wp2", DIAG_INT, &wrap_2);
                getdavar (da_input.dname, "wp3", DIAG_INT, &wrap_3);
                getdavar (da_input.dname, "wp4", DIAG_INT, &wrap_4);
                getdavar (da_input.dname, "wp5", DIAG_INT, &wrap_5);
                getdavar (da_input.dname, "wp6", DIAG_INT, &wrap_6);
                getdavar (da_input.dname, "wp7", DIAG_INT, &wrap_7);
                getdavar (da_input.dname, "wp8", DIAG_INT, &wrap_8);
                getdavar (da_input.dname, "wp9", DIAG_INT, &wrap_9);
                getdavar (da_input.dname, "wp10", DIAG_INT, &wrap_10);
                getdavar (da_input.dname, "wp11", DIAG_INT, &wrap_11);
                getdavar (da_input.dname, "adp", DIAG_INT, &tucb_ptr.ttycb.adapter);
                if (c_mode == CONSOLE) {
                        asl_rc = diag_asl_read (ASL_DIAG_OUTPUT_LEAVE_SC, FALSE,
                                bufstr);
                        chk_asl_stat (asl_rc);
                }
        }  /* endif */
}  /* select_attached_device end */

/*
 * NAME: dsply_tst_lst
 *
 * FUNCTION:  Builds a title line that describes the adapter/device under test
 *            and informs the user that a wrap plug is required to run this
 *            test.  Asks user if wrap plug is available.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

dsply_tst_lst (menu_num)  /* begin dsply_tst_lst */
long    menu_num;
{
        char    msgstr[512];
        struct  msglist da_menu[] =
        {
                { ASYNC_DIAG, DM_1A, },
                { ASYNC_DIAG, DM_7,  },
                { ASYNC_DIAG, DM_8,  },
                { ASYNC_DIAG, DM_6,  },
                Null
        };
        ASL_SCR_TYPE    menutype =
            {
                ASL_DIAG_ENTER_SC, 3, 1
        };
        ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];

        memset (menu_da, 0, sizeof (menu_da));
        switch (Adptr_name)
        {
        case SP1:
        case SP2:
        case SP3:
                da_menu[0].msgid = DM_1B;
                if ((menu_num & 0x000FFF) == 0x000605)
                        da_menu[3].msgid = DM_82;
                break;
        case EIA_232_8:
        case EIA_232_8ISA:
        case EIA_232_16:
                da_menu[0].msgid = DM_2B;
                break;
        case EIA_422_8:
        case EIA_422_16:
                da_menu[0].msgid = DM_3B;
                break;
        case M_S_188_8:
                da_menu[0].msgid = DM_4B;
                break;
        case EIA_232_64:
                da_menu[0].msgid = DM_5B;
                break;
        case EIA_232_128:
        case EIA_232_128ISA:
                if(cxma_line_err == TRUE)
                    da_menu[0].msgid = DM_14E;
                else
                    da_menu[0].msgid = DM_15B;
                break;
        }  /* end switch (Adptr_name) */
        diag_display (menu_num, catd, da_menu, DIAG_MSGONLY,
            ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, menu_da);
        switch (Adptr_name)
        {
        case SP1:
        case SP2:
        case SP3:
                sprintf(msgstr, menu_da[0].text, portnum);
                break;
        case EIA_232_8:
        case EIA_232_8ISA:
        case EIA_232_16:
        case EIA_422_8:
        case EIA_422_16:
        case M_S_188_8:
        case EIA_232_64:
                sprintf(msgstr, menu_da[0].text, prt_num, n_ports,
                    da_input.dnameloc);
                break;
        case EIA_232_128:
        case EIA_232_128ISA:
                sprintf(msgstr, menu_da[0].text, prt_num, da_input.dname,
                     da_input.dnameloc);
                break;
        }  /* end switch (Adptr_name) */
        free (menu_da[0].text);
        menu_da[0].text = (char *) malloc (strlen(msgstr)+1);
        strcpy (menu_da[0].text, msgstr);
        sprintf(msgstr, menu_da[3].text, wrp_plg);
        free (menu_da[3].text);
        menu_da[3].text = (char *) malloc (strlen(msgstr)+1);
        strcpy (menu_da[3].text, msgstr);
        asl_rc = diag_display (menu_num, catd, NULL, DIAG_IO,
            ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, menu_da);
        slctn = DIAG_ITEM_SELECTED(menutype);
        chk_asl_stat (asl_rc);
}  /* dsply_tst_lst end */

/*
 * NAME: wrap_plug_install
 *
 * FUNCTION:  Builds a title line that describes the adapter/device under test,
 *            prompts the user to remove any cables and or attachments and, to
 *            install the wrap plug before continuing.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

wrap_plug_install (menu_num, msg_id)  /* begin wrap_plug_install */
long    menu_num;
int     msg_id;
{
        char    msgstr[512];
        struct  msglist da_menu[] =
        {
                { ASYNC_DIAG, DM_1A, },
                { ASYNC_DIAG, DM_1A, },
                Null
        };
        ASL_SCR_TYPE    menutype =
            {
                ASL_DIAG_ENTER_SC, 2, 1
        };
        ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];

        memset (menu_da, 0, sizeof (menu_da));
        switch (Adptr_name)
        {
        case SP1:
        case SP2:
        case SP3:
                da_menu[0].msgid = DM_1B;
                break;
        case EIA_232_8:
        case EIA_232_8ISA:
        case EIA_232_16:
                da_menu[0].msgid = DM_2B;
                break;
        case EIA_422_8:
        case EIA_422_16:
                da_menu[0].msgid = DM_3B;
                break;
        case M_S_188_8:
                da_menu[0].msgid = DM_4B;
                break;
        case EIA_232_64:
                da_menu[0].msgid = DM_5B;
                break;
        case EIA_232_128:
        case EIA_232_128ISA:
                if(cxma_line_err == TRUE)
                    da_menu[0].msgid = DM_14E;
                else
                    da_menu[0].msgid = DM_15B;
                break;
        }  /* end switch (Adptr_name) */
        da_menu[1].msgid = msg_id;
        diag_display (menu_num, catd, da_menu, DIAG_MSGONLY,
            ASL_DIAG_KEYS_ENTER_SC, &menutype, menu_da);
        switch (Adptr_name)
        {
        case SP1:
        case SP2:
        case SP3:
                sprintf(msgstr, menu_da[0].text, portnum);
                break;
        case EIA_232_8:
        case EIA_232_8ISA:
        case EIA_232_16:
        case EIA_422_8:
        case EIA_422_16:
        case M_S_188_8:
        case EIA_232_64:
                sprintf(msgstr, menu_da[0].text, prt_num, n_ports,
                    da_input.dnameloc);
                break;
        case EIA_232_128:
        case EIA_232_128ISA:
                sprintf(msgstr, menu_da[0].text, prt_num, da_input.dname,
                     da_input.dnameloc);
                break;
        }  /* end switch (Adptr_name) */
        free (menu_da[0].text);
        menu_da[0].text = (char *) malloc (strlen(msgstr)+1);
        strcpy (menu_da[0].text, msgstr);

        switch (msg_id) {
           case DM_87:
              sprintf (msgstr, menu_da[1].text, prt_num, wrp_plg, prt_num);
              break;
           case DM_91:
              sprintf (msgstr, menu_da[1].text, wrp_plg, prt_num);
              break;
           default:
              sprintf (msgstr, menu_da[1].text, wrp_plg);
        } /* endswitch */

        free (menu_da[1].text);
        menu_da[1].text = (char *) malloc (strlen(msgstr)+1);
        strcpy (menu_da[1].text, msgstr);

        asl_rc = diag_display (menu_num, catd, NULL, DIAG_IO,
            ASL_DIAG_KEYS_ENTER_SC, &menutype, menu_da);
        chk_asl_stat (asl_rc);
}  /* wrap_plug_install end */

/*
 * NAME: wrap_plug_remove
 *
 * FUNCTION: Builds a title line that describes the adapter/device under test,
 *           prompts the user to remove the wrap plug and plug in any cables
 *           or attachments that were removed for the test before continuing.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

wrap_plug_remove (menu_num, msg_id)  /* begin wrap_plug_remove */
long    menu_num;
int     msg_id;
{
        char    msgstr[512];
        struct  msglist da_menu[] =
        {
                { ASYNC_DIAG, DM_1A, },
                { ASYNC_DIAG, DM_1A, },
                Null
        };
        ASL_SCR_TYPE    menutype =
            {
                ASL_DIAG_ENTER_SC, 2, 1
        };
        ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];

        memset (menu_da, 0, sizeof (menu_da));
        switch (Adptr_name)
        {
        case SP1:
        case SP2:
        case SP3:
                da_menu[0].msgid = DM_1B;
                break;
        case EIA_232_8:
        case EIA_232_8ISA:
        case EIA_232_16:
                da_menu[0].msgid = DM_2B;
                break;
        case EIA_422_8:
        case EIA_422_16:
                da_menu[0].msgid = DM_3B;
                break;
        case M_S_188_8:
                da_menu[0].msgid = DM_4B;
                break;
        case EIA_232_64:
                da_menu[0].msgid = DM_5B;
                break;
        case EIA_232_128:
        case EIA_232_128ISA:
                if(cxma_line_err == TRUE)
                    da_menu[0].msgid = DM_14E;
                else
                    da_menu[0].msgid = DM_15B;
                break;
        }  /* end switch (Adptr_name) */
        da_menu[1].msgid = msg_id;
        diag_display (menu_num, catd, da_menu, DIAG_MSGONLY,
            ASL_DIAG_KEYS_ENTER_SC, &menutype, menu_da);
        switch (Adptr_name)
        {
        case SP1:
        case SP2:
        case SP3:
                sprintf(msgstr, menu_da[0].text, portnum);
                break;
        case EIA_232_8:
        case EIA_232_8ISA:
        case EIA_232_16:
        case EIA_422_8:
        case EIA_422_16:
        case M_S_188_8:
        case EIA_232_64:
                sprintf(msgstr, menu_da[0].text, prt_num, n_ports,
                    da_input.dnameloc);
                break;
        case EIA_232_128:
        case EIA_232_128ISA:
                sprintf(msgstr, menu_da[0].text, prt_num, da_input.dname,
                     da_input.dnameloc);
                break;
        }  /* end switch (Adptr_name) */
        free (menu_da[0].text);
        menu_da[0].text = (char *) malloc (strlen(msgstr)+1);
        strcpy (menu_da[0].text, msgstr);
        asl_rc = diag_display (menu_num, catd, NULL, DIAG_IO,
            ASL_DIAG_KEYS_ENTER_SC, &menutype, menu_da);
        chk_asl_stat (asl_rc);
}  /* wrap_plug_remove end */

/*
 * NAME: dsply_tst_hdr
 *
 * FUNCTION:  Builds a title line that describes the adapter/device under test
 *      and displays a "please standby" message when a test is run one time or
 *      when entering loop mode.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

dsply_tst_hdr (menu_num)  /* begin pass_one_tst_msg */
long    menu_num;
{
        char    msgstr[512];
        int     dm_s_l;
        struct  msglist da_menu[] =
        {
                { ASYNC_DIAG, DM_1A, },
                Null
        };
        ASL_SCR_TYPE    menutype =
            {
                ASL_DIAG_OUTPUT_LEAVE_SC, 0, 1
        };
        ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];

        memset (menu_da, 0, sizeof (menu_da));
        switch (Adptr_name)
        {
        case SP1:
        case SP2:
        case SP3:
                if (a_mode == ADVANCED)
                {
                        if ((l_mode == NOTLM) ||
                        (l_mode == ENTERLM && s_mode == NOT_SYSTEM))
                                da_menu[0].msgid = DM_1C;
                        else
                                da_menu[0].msgid = DM_1D;
                }  /* endif */
                else
                        da_menu[0].msgid = DM_1A;
                break;
        case EIA_232_8:
        case EIA_232_8ISA:
        case EIA_232_16:
                if (a_mode == ADVANCED)
                {
                        if ((l_mode == NOTLM) ||
                        (l_mode == ENTERLM && s_mode == NOT_SYSTEM))
                                da_menu[0].msgid = DM_2C;
                        else
                                da_menu[0].msgid = DM_2D;
                }  /* endif */
                else
                        da_menu[0].msgid = DM_2A;
                break;
        case EIA_422_8:
        case EIA_422_16:
                if (a_mode == ADVANCED)
                {
                        if ((l_mode == NOTLM) ||
                        (l_mode == ENTERLM && s_mode == NOT_SYSTEM))
                                da_menu[0].msgid = DM_3C;
                        else
                                da_menu[0].msgid = DM_3D;
                }  /* endif */
                else
                        da_menu[0].msgid = DM_3A;
                break;
        case M_S_188_8:
                if (a_mode == ADVANCED)
                {
                        if ((l_mode == NOTLM) ||
                        (l_mode == ENTERLM && s_mode == NOT_SYSTEM))
                                da_menu[0].msgid = DM_4C;
                        else
                                da_menu[0].msgid = DM_4D;
                }  /* endif */
                else
                        da_menu[0].msgid = DM_4A;
                break;
        case EIA_232_64:
                if (a_mode == ADVANCED)
                {
                        if ((l_mode == NOTLM) ||
                        (l_mode == ENTERLM && s_mode == NOT_SYSTEM))
                                da_menu[0].msgid = DM_5C;
                        else
                                da_menu[0].msgid = DM_5D;
                }  /* endif */
                else
                        da_menu[0].msgid = DM_5A;
                break;
        case EIA_232_128:
        case EIA_232_128ISA:
                if (a_mode == ADVANCED)
                {
                        if ((l_mode == NOTLM) ||
                        (l_mode == ENTERLM && s_mode == NOT_SYSTEM)) {
                                if(cxma_adapter == TRUE)
                                        da_menu[0].msgid = DM_14C;
                                else
                                        da_menu[0].msgid = DM_15C;
                        }
                        else
                                da_menu[0].msgid = DM_15D;

                        if(cxma_line_err == TRUE)
                                da_menu[0].msgid = DM_14F;

                }  /* endif */
                else {
                        if(cxma_adapter == TRUE)
                                da_menu[0].msgid = DM_14A;
                        else
                                da_menu[0].msgid = DM_15A;
                } /* endelse */
                break;
        }  /* end switch (Adptr_name) */
        diag_display(menu_num, catd, da_menu, DIAG_MSGONLY,
            ASL_DIAG_OUTPUT_LEAVE_SC, &menutype, menu_da);
        if (l_mode == EXITLM)
                da_input.lcount -= 1;
        switch (Adptr_name)
        {
        case SP1:
        case SP2:
        case SP3:
                if ((l_mode != NOTLM && l_mode != ENTERLM) ||
                    (l_mode == ENTERLM && s_mode == SYSTEM))
                        sprintf(msgstr, menu_da[0].text, portnum,
                            da_input.lcount, da_input.lerrors);
                else
                        sprintf(msgstr, menu_da[0].text, portnum);
                break;
        case EIA_232_8:
        case EIA_232_8ISA:
        case EIA_232_16:
        case EIA_422_8:
        case EIA_422_16:
        case M_S_188_8:
        case EIA_232_64:
                if ((l_mode != NOTLM && l_mode != ENTERLM) ||
                    (l_mode == ENTERLM && s_mode == SYSTEM))
                        sprintf(msgstr, menu_da[0].text, prt_num, n_ports,
                            da_input.dnameloc, da_input.lcount,
                            da_input.lerrors);
                else
                        sprintf(msgstr, menu_da[0].text, prt_num, n_ports,
                            da_input.dnameloc);
                break;
       case EIA_232_128:
       case EIA_232_128ISA:
                if (a_mode == ADVANCED) {
                        if ((l_mode != NOTLM && l_mode != ENTERLM) ||
                            (l_mode == ENTERLM && s_mode == SYSTEM))
                                sprintf(msgstr, menu_da[0].text, prt_num,
                                    da_input.dname, da_input.dnameloc,
                                    da_input.lcount, da_input.lerrors);
                        else {
                                if(cxma_adapter == TRUE &&
                                   cxma_line_err == FALSE)
                                        sprintf(msgstr, menu_da[0].text,
                                            da_input.dname, da_input.dnameloc);
                                else
                                        sprintf(msgstr, menu_da[0].text,
                                            prt_num, da_input.dname,
                                            da_input.dnameloc);
                        }
                        if(cxma_line_err == TRUE) {
                                if(cxma_adapter == TRUE)
                                        sprintf(msgstr, menu_da[0].text,
                                                prt_num, da_input.dname,
                                                da_input.dnameloc);
                                else
                                        sprintf(msgstr, menu_da[0].text,
                                                prt_num, da_input.parent,
                                                da_input.parentloc);
                        }
                }
                else {
                        if(cxma_adapter == TRUE)
                                sprintf(msgstr, menu_da[0].text,
                                        da_input.dname, da_input.dnameloc);
                        else
                                sprintf(msgstr, menu_da[0].text, prt_num,
                                        da_input.dname, da_input.dnameloc);
                }
                break;
        }  /* end switch (Adptr_name) */
        free (menu_da[0].text);
        menu_da[0].text = (char *) malloc (strlen(msgstr)+1);
        strcpy (menu_da[0].text, msgstr);
        asl_rc = diag_display (menu_num, catd, NULL, DIAG_IO,
            ASL_DIAG_OUTPUT_LEAVE_SC, &menutype, menu_da);
        chk_asl_stat (asl_rc);
}  /* dsply_tst_hdr end */

/*
 * NAME: dsply_cfg_hdr
 *
 * FUNCTION:  Builds a title line that describes the adapter/device under test
 *      and displays a "please standby" message when an adapter is being con-
 *      figured while in loop mode.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

dsply_cfg_hdr (menu_num)
long    menu_num;
{
        struct  msglist da_menu[] =
        {
                { ASYNC_DIAG, DM_42, },
                Null
        };
        ASL_SCR_TYPE    menutype =
            {
                ASL_DIAG_OUTPUT_LEAVE_SC, 0, 1
        };
        ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];

        memset (menu_da, 0, sizeof (menu_da));
        asl_rc = diag_display(menu_num, catd, da_menu, DIAG_IO,
            ASL_DIAG_OUTPUT_LEAVE_SC, NULL, NULL);
        chk_asl_stat (asl_rc);
}  /* dsply_cfg_hdr end */


/*
 * NAME: rscmenu1
 *
 * FUNCTION:  Prompts user to install terminator on the previous RAN or
 *      controller cable, or notifies the user that the test has succeeded.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */
rscmenu1(menu_num, msg_id)  /* begin */
long menu_num;
int  msg_id;
{
        char    msgstr[2048];
        struct  msglist da_menu[] = {
            { ASYNC_DIAG, DM_15A, },
            { ASYNC_DIAG, DM_23, },
            Null
        };
        ASL_SCR_TYPE    menutype = {
            ASL_DIAG_ENTER_SC, 2, 1
        };

        ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];

        memset (menu_da, 0, sizeof (menu_da));

        if (a_mode == ADVANCED)
        {
                if(cxma_line_err == TRUE)
                        da_menu[0].msgid = DM_14E;
                else
                        da_menu[0].msgid = DM_15B;
        }  /* endif */
        else {
                if(cxma_line_err == TRUE)
                        da_menu[0].msgid = DM_14B;
                else
                        da_menu[0].msgid = DM_15A;
        } /* endelse */

        da_menu[1].msgid = msg_id;

        diag_display (menu_num, catd, da_menu, DIAG_MSGONLY,
            ASL_DIAG_KEYS_ENTER_SC, &menutype, menu_da);

        if(cxma_adapter == TRUE)
            sprintf(msgstr, menu_da[0].text, prt_num, da_input.dname,
                da_input.dnameloc);
        else
            sprintf(msgstr, menu_da[0].text, prt_num, da_input.parent,
                da_input.parentloc);
        free (menu_da[0].text);
        menu_da[0].text = (char *) malloc (strlen(msgstr)+1);
        strcpy (menu_da[0].text, msgstr);

        if(sync_err == 1) {
            sprintf(msgstr, menu_da[1].text, wrp_plg);
            free (menu_da[1].text);
            menu_da[1].text = (char *) malloc (strlen(msgstr)+1);
            strcpy (menu_da[1].text, msgstr);
        }

        asl_rc = diag_display (menu_num, catd, NULL, DIAG_IO,
            ASL_DIAG_KEYS_ENTER_SC, &menutype, menu_da);
        chk_asl_stat (asl_rc);
}  /* rscmenu1 end */


/*
 * NAME: rscmenu2
 *
 * FUNCTION:  Asks the user if the terminator was installed on the adapter or
 *      not.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */
rscmenu2(menu_num, msg_id)  /* begin */
long    menu_num;
int     msg_id;
{
        char    msgstr[1024];
        struct  msglist da_menu[] = {
            { ASYNC_DIAG, DM_15A, },
            { ASYNC_DIAG, DM_7,  },
            { ASYNC_DIAG, DM_8,  },
            { ASYNC_DIAG, DM_17,  },
            Null
        };
        ASL_SCR_TYPE    menutype = {
            ASL_DIAG_ENTER_SC, 3, 1
        };
        ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];

        memset (menu_da, 0, sizeof (menu_da));

        if (a_mode == ADVANCED)
        {
                if(cxma_line_err == TRUE)
                        da_menu[0].msgid = DM_14E;
                else
                        da_menu[0].msgid = DM_15B;
        }  /* endif */
        else {
                if(cxma_line_err == TRUE)
                        da_menu[0].msgid = DM_14B;
                else
                        da_menu[0].msgid = DM_15A;
        } /* endelse */

        da_menu[3].msgid = msg_id;

        diag_display (menu_num, catd, da_menu, DIAG_MSGONLY,
            ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, menu_da);

        if(cxma_adapter == TRUE)
            sprintf(msgstr, menu_da[0].text, prt_num, da_input.dname,
                  da_input.dnameloc);
        else
            sprintf(msgstr, menu_da[0].text, prt_num, da_input.parent,
                  da_input.parentloc);
        free (menu_da[0].text);
        menu_da[0].text = (char *) malloc (strlen(msgstr)+1);
        strcpy (menu_da[0].text, msgstr);

        if(msg_id == DM_87)
            sprintf(msgstr, menu_da[3].text, prt_num, wrp_plg);
        else
            sprintf(msgstr, menu_da[3].text, wrp_plg);
        free (menu_da[3].text);
        menu_da[3].text = (char *) malloc (strlen(msgstr)+1);
        strcpy (menu_da[3].text, msgstr);


        asl_rc = diag_display (menu_num, catd, NULL, DIAG_IO,
            ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, menu_da);
        slctn = DIAG_ITEM_SELECTED(menutype);
        chk_asl_stat (asl_rc);
}  /* rscmenu2 end */


/*
 * NAME: chk_screen_stat
 *
 * FUNCTION:  Performs an ASL_READ to see if the user has pressed the
 *      Cancel or Exit keys to exit from loop mode.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */
chk_screen_stat ()   /* begin chk_screen_stat */
{
        char bufstr[128];

        if(asl_init_done) {  /* If ASL has been started */
            asl_rc = diag_asl_read (ASL_DIAG_OUTPUT_LEAVE_SC, FALSE, bufstr);
            chk_asl_stat (asl_rc);
        }
}  /* dsply_cfg_hdr end */



/*
 * NAME: must_have_terminator
 *
 * FUNCTION:  Creates a menugoal telling the user that they must have a
 *            terminator for the 128-port adapter and RAN to work.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */
must_have_terminator (menu_num)   /* begin must_have_terminator */
long    menu_num;
{
        char msgstr[1024];
        char tmpmsg[1024];

        sprintf(msgstr, "%x ", menu_num);
        sprintf(tmpmsg, "%s", (char *) diag_cat_gets (catd,
            ASYNC_MENUGOAL, MG2));
        sprintf(tmpmsg, tmpmsg, wrp_plg);
        strcat(msgstr, tmpmsg);

        menugoal(msgstr);
}  /* must_have_terminator end */

/*
 * NAME: select_attached_cabling
 *
 * FUNCTION:  Builds a title line that describes the adapter/device under
 *      test.  Asks the user to select which type of cabling is attached to
 *      the adapter.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:  Global variable "att_cbl" is set.  att_cbl is used to determine
 *      which cabling is installed during loop mode testing.
 *
 * RETURNS: NONE
 */

select_attached_cabling ()  /* begin select_attached_cabling */
{
        char    msgstr[512];
        char    bufstr[128];
        struct  msglist da_menu[] =
        {
                { ASYNC_DIAG, DM_1A, },
                { ASYNC_DIAG, DM_88, },
                { ASYNC_DIAG, DM_89, },
                { ASYNC_DIAG, DM_90,  },
                Null
        };
        ASL_SCR_TYPE menutype =
            {
                ASL_DIAG_ENTER_SC, 4, 1
        };
        ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];

        if (l_mode == NOTLM || l_mode == ENTERLM)
        {
                Menu_nmbr &= 0xFFF000;          /* Menu_nmbr == 0xXXX001     */
                Menu_nmbr += 0x000806;          /* Menu_nmbr == 0xXXX001     */
                memset (menu_da, 0, sizeof (menu_da));
                switch (Adptr_name)
                {
                case SP1:
                case SP2:
                case SP3:
                        da_menu[0].msgid = DM_1B;
                        break;
                case EIA_232_8:
                case EIA_232_8ISA:
                case EIA_232_16:
                        da_menu[0].msgid = DM_2B;
                        break;
                case M_S_188_8:
                        da_menu[0].msgid = DM_4B;
                        break;
                case EIA_232_64:
                        da_menu[0].msgid = DM_5B;
                        break;
                case EIA_232_128:
                case EIA_232_128ISA:
                        da_menu[0].msgid = DM_15B;
                        break;
                }  /* end switch (Adptr_name) */
                diag_display (Menu_nmbr, catd, da_menu, DIAG_MSGONLY,
                    ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, menu_da);
                switch (Adptr_name)
                {
                case SP1:
                case SP2:
                case SP3:

                        sprintf(msgstr, menu_da[0].text, portnum);
                        break;
                case EIA_232_8:
                case EIA_232_8ISA:
                case EIA_232_16:
                case M_S_188_8:
                case EIA_232_64:
                        sprintf(msgstr, menu_da[0].text, prt_num, n_ports,
                            da_input.dnameloc);
                        break;
                case EIA_232_128:
                case EIA_232_128ISA:
                        sprintf(msgstr, menu_da[0].text, prt_num, da_input.dname,
                             da_input.dnameloc);
                        break;
                }  /* end switch (Adptr_name) */
                free (menu_da[0].text);
                menu_da[0].text = (char *) malloc (strlen(msgstr)+1);
                strcpy (menu_da[0].text, msgstr);
                asl_rc = diag_display (Menu_nmbr, catd, NULL, DIAG_IO,
                    ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, menu_da);
                att_cbl = DIAG_ITEM_SELECTED (menutype)-1;
                putdavar (da_input.dname, "cbl", DIAG_INT, &att_cbl);
                chk_asl_stat (asl_rc);
        }  /* endif */
        if (l_mode == INLM || l_mode == EXITLM)
        {
                getdavar (da_input.dname, "cbl", DIAG_INT, &att_cbl);
                if (c_mode == CONSOLE) {
                        asl_rc = diag_asl_read (ASL_DIAG_OUTPUT_LEAVE_SC, FALSE,
                                bufstr);
                        chk_asl_stat (asl_rc);
                }
        }  /* endif */
}  /* select_attached_cabling end */
