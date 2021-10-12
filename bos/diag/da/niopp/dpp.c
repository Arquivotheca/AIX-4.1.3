static char sccsid[] = "@(#)04  1.15.1.7  src/bos/diag/da/niopp/dpp.c, daniopp, bos41J, 9513A_all 3/24/95 16:20:10";
/*
 * COMPONENT_NAME: daniopp
 *
 * FUNCTIONS:   main ()
 *              setdamode ()
 *              open_dd ()
 *              close_dd ()
 *              exit_da ()
 *              select_tu ()
 *              pos_setup ()
 *              tu_test ()
 *              add_fru ()
 *              chk_asl_stat ()
 *              wrap_plug ()
 *              wrap_prompt ()
 *              cable_prompt ()
 *              dsply_tst_hdr ()
 *              dsply_ntf_msg ()
 *              int_handler ()
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
 *
 */

#include        "dpp.h"
#include        "diag/modid.h"

int     slot=0;
int     prev_state=0;

extern  int     get_get_cpu_model(int *);


main ()  /* begin main */
{
        int     reg,address, rc;
        int     index;
        int     mode;
        char    devlp0[NAMESIZE];
        char    bufstr[128];

        setlocale (LC_ALL, "");
        /* set up interrupt handler routine     */
        act.sa_handler = int_handler;
        sigaction (SIGINT, &act, (struct sigaction *)NULL);

        /* Initialize odm, get da input, and determine ipl mode */
        if ((init_dgodm ()) == FAIL)
                exit_da (SW_ERR);
        if ((getdainput (&da_input)) == FAIL)
                exit_da (SW_ERR);
        mode = ipl_mode (&diskette);
        strcpy (frub[0].dname, da_input.dname);

        /* Set diagnostic mode */
        setdamode (&testmode);

        /* Open catalog file, initialize asl and display first screen */
        if (c_mode == CONS_T)
        {
                diag_asl_init (NULL);
                catd = diag_catopen (MF_DPP, 0);
                menu_nmbr &= 0xFFF000;
                menu_nmbr += 0x000101;
                if ((asl_rc = dsply_tst_hdr (menu_nmbr)) != ZERO)
                        exit_da (asl_rc);
        }  /* endif */
        if (l_mode == IN_LP || l_mode == EX_LP)
                getdavar (da_input.dname, "wp7", DIAG_INT, &wrap_7);

        /* Determine adapter type */
        if ((rc = search_odm ()) != ZERO)
                exit_da (rc);

        if (strlen (da_input.child1))
                strcpy (lp_name, da_input.child1);
        else
                strcpy (lp_name, C_cudv->name);
        if (C_cudv->status == DEFINED)
                if ((prev_state = configure_device (lp_name)) == -1 )
                        exit_da (SW_ERR);
        strcpy (devppa, "/dev/");
        strcat (devppa, lp_name);
        sleep(1);
        rc = open_dd ();
        if ((c_mode == CONS_T) && (l_mode == IN_LP || l_mode == EN_LP))
        {
                asl_rc = diag_asl_read (ASL_DIAG_OUTPUT_LEAVE_SC, FALSE,
                    bufstr);
                if ((asl_rc = chk_asl_stat (asl_rc)) != ZERO)
                        exit_da (asl_rc);
        }  /* endif */
        exit_da (rc);
}  /* main end */

/*
 * NAME: setdamode
 *
 * FUNCTION:  Returns the execution environment to the DA to determine the
 *      execution mode using data defined in structure da_input and header
 *      file dpp.h.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  damode
 */

setdamode (damode)       /* begin setdamode */
int     *damode;
{
        /* Initialize exit status codes */
        DA_SETRC_STATUS (DA_STATUS_GOOD);
        DA_SETRC_USER (DA_USER_NOKEY);
        DA_SETRC_ERROR (DA_ERROR_NONE);
        DA_SETRC_TESTS (DA_TEST_NOTEST);
        DA_SETRC_MORE (DA_MORE_NOCONT);

        /* Set test mode variables */
        *damode = INVALID_TM_INPUT;
        l_mode = da_input.loopmode;    /* NT_LP, EN_LP, IN_LP, EX_LP    */
        c_mode = da_input.console;     /* CONS_T, CONS_F                */
        a_mode = da_input.advanced;    /* ADV_T, ADV_F                  */
        s_mode = da_input.system;      /* SYS_T, SYS_F                  */
        e_mode = da_input.exenv;       /* IPL, STD, RGR, CNC, EXR       */
        d_mode = da_input.dmode;       /* ELA, PD, RPR, MS1, MS2        */
        if (a_mode == ADV_T && (s_mode == SYS_F && c_mode == CONS_T))
        {
                if (l_mode == NT_LP)
                        *damode = A_T;
                else
                        *damode = L_T;
        }  /* endif */
        else if(a_mode == ADV_T && e_mode == EXR && l_mode != NT_LP)
        {
                *damode = A_T;
        }
        else
        {
                *damode = N_I;
        }  /* endelse */
}  /* setdamode end */

/*
 * NAME: search_odm
 *
 * FUNCTIONS:  Search CuDv for data necessary to configure
 *
 * NOTES:  CuDv - defines configured adapter/port/device
 *
 * RETURNS:
 */

search_odm ()
{
        char    buffer[128];

        /* search CuDv for device */
        sprintf (buffer, "parent = '%s'", da_input.dname);
        C_cudv = get_CuDv_list(CuDv_CLASS, buffer, &c_info ,1 ,1);
        if (C_cudv == (struct CuDv *) FAIL)
                return (SW_ERR);
        /* search CuDv for parent */
        sprintf (buffer, "name = '%s'", da_input.parent);
        P_cudv = get_CuDv_list(CuDv_CLASS, buffer, &c_info ,1 ,1);
        if (P_cudv == (struct CuDv *) FAIL)
                return (SW_ERR);
        return (ZERO);
}  /* search_odm end */

/*
 * NAME: open_dd
 *
 * FUNCTIONS:  Open the device driver file, save pos register values, and
 *              execute test units.
 *
 * RETURNS:  none
 */

open_dd ()
{
        int     reg, address;
        int     rc = TU_GOOD;
        int     dummy, cpu_model;
        char    bufstr[128];

        if ((mddfd = OPEN_MDD) == FAIL)
                return (FAIL_OPN);
        if ((ddfd = OPEN_LPX) == FAIL)
                return (FAIL_OPN);
        drivers_open = TRUE;

        if(is_rspc_model())
            tucb_ptr.ttycb.adapter = SIO10;

        else {
            /* computing slot number from connwhere code */
            /* slot is 7 on Pegasus and Fireball */
            /* slot is 9 on Panola */
            /* slot is 15 on all others */

            slot = atoi (P_cudv->connwhere);
            slot = (slot - 1) & 0xF;

            mddRecord.md_addr = POSREG(0,slot);     /* NIO POS register  */
            mddRecord.md_size = 1;                  /* move one byte     */
            mddRecord.md_incr = MV_BYTE;            /* byte read         */
            mddRecord.md_data = (char *)&reg;       /* address of data   */
            if ((io_rc = ioctl (mddfd, MIOCCGET, &mddRecord)) != 0)
                    return (SW_ERR);
            tucb_ptr.ttycb.adapter = reg >> 24;
            if (tucb_ptr.ttycb.adapter != SIO3 && tucb_ptr.ttycb.adapter != SIO7)
            {
                    for (reg=2, address=0; reg<=maxreg; reg+=2, address++)
                    {
                            mddRecord.md_addr = POSREG(reg,slot);
                            mddRecord.md_size = 1;
                            mddRecord.md_incr = MV_BYTE;
                            mddRecord.md_data = &posreg[address];
                            if ((io_rc = ioctl (mddfd, MIOCCGET, &mddRecord)) != 0)
                                    return (SW_ERR);
                    } /* endfor */
                    posreg[2] = posreg[0] & 0xbf;   /* reset parallel port  */
                    posreg[3] = posreg[0] | 0x40;   /* enable parallel port */
                    posreg[4] = posreg[1] & 0xbf;   /* disable BIDI         */
                    posreg[5] = posreg[1] | 0x40;   /* enable BIDI          */
            }
        }

        if (d_mode == PD || d_mode == RPR)
        {
                if (tucb_ptr.ttycb.adapter != SIO3)
                {
                        if ((rc = select_tu (TEST10)) != TU_GOOD)
                                return (rc);
                }
                rc = select_tu (TEST20);
                if (prev_state == 1)
                        sleep (1);
                if ((testmode == A_T || testmode == L_T) && rc == TU_GOOD)
                {
                        if (wrap_7 == TRUE)
                                if ((rc = select_tu (TEST30)) != TU_GOOD)
                                        return (rc);
                        if ((rc = select_tu (TEST40)) != TU_GOOD)
                                return (rc);
                        if (wrap_7 == TRUE)
                        {
                                if ((rc = select_tu (TEST50)) != TU_GOOD)
                                        return (rc);
                                rc = select_tu (TEST60);
                        }  /* endif */
                }  /* endif */
                if (prev_state == 1)
                        sleep (1);
        }  /* endif */
        if ((c_mode == CONS_T) && (l_mode == IN_LP || l_mode == EN_LP))
        {
                asl_rc = diag_asl_read (ASL_DIAG_OUTPUT_LEAVE_SC, FALSE,
                    bufstr);
                if ((asl_rc = chk_asl_stat (asl_rc)) != ZERO)
                        exit_da (asl_rc);
        }  /* endif */
        return (rc);
}  /* open_dd end */

/*
 * NAME: close_dd
 *
 * FUNCTIONS:  Restore the the original pos register values and close the
 *              device driver files.
 *
 * RETURNS:  none
 */

close_dd ()
{
        int     reg, address;

        if (tucb_ptr.ttycb.adapter != SIO3 && tucb_ptr.ttycb.adapter != SIO7)
        {
                for (reg=2, address=0; reg<=maxreg; reg+=2,
                    address++)
                {
                        mddRecord.md_addr = POSREG(reg,slot);
                        mddRecord.md_size = 1;
                        mddRecord.md_incr = MV_BYTE;
                        mddRecord.md_data = &posreg[address];
                        ioctl (mddfd, MIOCCPUT, &mddRecord);
                }  /* endfor */
        }  /* endif */
        if (ddfd != FAIL)
                close (ddfd);
        if (mddfd != FAIL)
                close (mddfd);
}  /* close_dd end */

/*
 * NAME: exit_da
 *
 * FUNCTION:  Performs cleanup operations prior to returning to the diagnostic
 *            controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  NONE
 */

int     exit_da (exit_code)
int     exit_code;
{
        if (exit_code != NO_TST)
                DA_SETRC_TESTS (DA_TEST_FULL);
        switch (exit_code)
        {
        case TU_BAD:
                DA_SETRC_STATUS (DA_STATUS_BAD);
                break;
        case USR_EXIT:
                DA_SETRC_USER (DA_USER_EXIT);
                break;
        case USR_QUIT:
                DA_SETRC_USER (DA_USER_QUIT);
                break;
        case FAIL_OPN:
                DA_SETRC_ERROR (DA_ERROR_OPEN);
                break;
        case SW_ERR:
                DA_SETRC_ERROR (DA_ERROR_OTHER);
                break;
        case SUB_TST:
                DA_SETRC_TESTS (DA_TEST_SUB);
                break;
        case SHR_TST:
                DA_SETRC_TESTS (DA_TEST_SHR);
                break;
        case MNU_GOAL:
                DA_SETRC_MORE (DA_MORE_CONT);
                break;
        case TU_GOOD:
        case NO_TST:
        default:
                break;
        }  /* endswitch */
        if (((l_mode == NT_LP || l_mode == EX_LP) && exit_code == TU_GOOD) &&
            (c_mode == CONS_T && s_mode == SYS_F && e_mode != EXR ))
        {
                menu_nmbr &= 0xFFF000;
                menu_nmbr += 0x000102;
                asl_rc = dsply_ntf_msg (menu_nmbr);
        }  /* endif */
        if(e_mode == EXR && c_mode == CONS_T)   clrdainput();
        close_dd ();
        sleep(1);
        initial_state(prev_state,lp_name);
        if (c_mode == CONS_T && catd != CATD_ERR)
        {
                catclose (catd);
                diag_asl_quit ();
        }  /* endif */
        term_dgodm ();
        DA_EXIT ();
}  /* exit_da end */

/*
 * NAME: select_tu
 *
 * FUNCTION: Displays menus and messages to the user.  Calls pos_setup
 *           function to configure the parallel port for the test units.
 *           Calls tu_test and add_fru functions to run test units, report
 *           errors and FRUs.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

select_tu (tu_num)  /* begin select_tu */
int   tu_num;
{
        int     tu_err = ZERO;

        if (tucb_ptr.ttycb.adapter != SIO3 && tucb_ptr.ttycb.adapter != SIO7 &&
            tucb_ptr.ttycb.adapter != SIO10)
        {
                if ((tu_err = pos_setup (tu_num)) != ZERO)
                        return (tu_err);
        }
        switch (tu_num)
        {
        case TEST10:
        case TEST20:
                if(e_mode == EXR && l_mode == EN_LP) /* Preparation pass */
                        break;
                if (l_mode != EX_LP)
                {
                        if ((tu_err = tu_test (tu_num)) != ZERO)
                        {
                                switch (tu_err)
                                {
                                case 0x10:
                                case 0x20:
                                        return (SW_ERR);
                                case 0x11:
                                case 0x12:
                                        frub[0].frus[0].conf = conf4;
                                        frub[0].frus[1].conf = conf5;
                                        frub[0].frus[1].fru_flag = PARENT_NAME;
                                        break;
                                case 0x21:
                                case 0x22:
                                case 0x23:
                                case 0x24:
                                case 0x25:
                                case 0x26:
                                        frub[0].frus[0].conf = conf1;
                                        frub[0].frus[1].conf = 0;
                                        frub[0].frus[1].fru_flag = 0;
                                        break;
                                }  /* end switch */
                                frub[0].rcode = 0x100 + tu_err;
                                frub[0].rmsg = SPP_2;
                                tu_err = add_fru ();
                                return (tu_err);
                        }  /* endif */
                }  /* endif */
                if (tu_err != ZERO)     return(tu_err);
                break;
        case TEST30:
                if (l_mode == NT_LP || l_mode == EN_LP)
                {
                        menu_nmbr &= 0xFFF000;
                        menu_nmbr += 0x000301;
                        if(e_mode == EXR && a_mode == ADV_F) {
                                /* SYSX without test tools */
                                asl_rc = 0;
                                slctn  = 2;
                        }
                        else    asl_rc = wrap_plug (menu_nmbr, &slctn);
                        if (asl_rc != ZERO)
                                return (asl_rc);
                        if (slctn == 1)
                        {
                                wrap_7 = TRUE;
                                putdavar (da_input.dname, "wp7",
                                    DIAG_INT, &wrap_7);
                                /* Preparation Pass */
                        }  /* endif */
                        else
                        {
                                wrap_7 = FALSE;
                                putdavar (da_input.dname, "wp7",
                                    DIAG_INT, &wrap_7);
                                return (TU_GOOD);
                        }  /* endelse */
                        menu_nmbr &= 0xFFF000;
                        menu_nmbr += 0x000302;
                        asl_rc = wrap_prompt (menu_nmbr, DM_UCB, DM_PWP);
                        if (asl_rc != ZERO)
                                return (asl_rc);
                        if(e_mode == EXR) /* Preparation pass */
                                return(TU_GOOD);
                }  /* endif */
                if (l_mode != EX_LP && wrap_7 == TRUE)
                {
                        if ((tu_err = tu_test (tu_num)) != ZERO)
                        {
                                switch (tu_err)
                                {
                                case 0x30:
                                        return (SW_ERR);
                                case 0x31:
                                case 0x32:
                                case 0x33:
                                        frub[0].frus[0].conf = conf1;
                                        frub[0].frus[1].conf = 0;
                                        frub[0].frus[1].fru_flag = 0;
                                        break;
                                }  /* end switch */
                                frub[0].rcode = 0x100 + tu_err;
                                frub[0].rmsg = SPP_2;
                                tu_err = add_fru ();
                        }  /* endif */
                }  /* endif */
                if ((l_mode == NT_LP || l_mode == EX_LP) && tu_err != ZERO)
                {
                        menu_nmbr &= 0xFFF000;
                        menu_nmbr += 0x000303;
                        asl_rc = wrap_prompt (menu_nmbr, DM_UWP, DM_PCB);
                        return (tu_err);
                }  /* endif */
                if (tu_err != ZERO)     return(tu_err);
                break;
        case TEST40:
                if ((l_mode == NT_LP || l_mode == EN_LP) && wrap_7 == FALSE)
                {
                        menu_nmbr &= 0xFFF000;
                        menu_nmbr += 0x000401;
                        asl_rc = cable_prompt (menu_nmbr, DM_UCB);
                        if (asl_rc != ZERO)
                                return (asl_rc);
                        if(e_mode == EXR) /* Preparation pass */
                                return(TU_GOOD);
                }  /* endif */
                if (l_mode != EX_LP)
                {
                        if ((tu_err = tu_test (tu_num)) != ZERO)
                        {
                                switch (tu_err)
                                {
                                case 0x40:
                                        return (SW_ERR);
                                case 0x41:
                                case 0x42:
                                        frub[0].frus[0].conf = conf1;
                                        frub[0].frus[1].conf = 0;
                                        frub[0].frus[1].fru_flag = 0;
                                        break;
                                }  /* end switch */
                                frub[0].rcode = 0x100 + tu_err;
                                frub[0].rmsg = SPP_2;
                                tu_err = add_fru ();
                        }  /* endif */
                }  /* endif */
                if ((l_mode == NT_LP || l_mode == EX_LP) && wrap_7 == FALSE)
                {
                        menu_nmbr &= 0xFFF000;
                        menu_nmbr += 0x000402;
                        asl_rc = cable_prompt (menu_nmbr, DM_PCB);
                        if (tu_err != ZERO)
                                return (tu_err);
                }  /* endif */
                if (tu_err != ZERO)     return(tu_err);
                break;
        case TEST50:
                if(e_mode == EXR && l_mode == EN_LP) /* Preparation pass */
                        break;
                if (l_mode != EX_LP)
                {
                        if ((tu_err = tu_test (tu_num)) != ZERO)
                        {
                                switch (tu_err)
                                {
                                case 0x50:
                                        return (SW_ERR);
                                case 0x51:
                                case 0x52:
                                        frub[0].frus[0].conf = conf1;
                                        frub[0].frus[1].conf = 0;
                                        frub[0].frus[1].fru_flag = 0;
                                        break;
                                }  /* end switch */
                                frub[0].rcode = 0x100 + tu_err;
                                frub[0].rmsg = SPP_3;
                                tu_err = add_fru ();
                        }  /* endif */
                }  /* endif */
                if ((l_mode == NT_LP || l_mode == EX_LP) && tu_err != ZERO)
                {
                        menu_nmbr &= 0xFFF000;
                        menu_nmbr += 0x000501;
                        asl_rc = wrap_prompt (menu_nmbr, DM_UWP, DM_PCB);
                        return (tu_err);
                }  /* endif */
                if (tu_err != ZERO)     return(tu_err);
                break;
        case TEST60:
                if(e_mode == EXR && l_mode == EN_LP) /* Preparation pass */
                        break;
                if (l_mode != EX_LP)
                {
                        if ((tu_err = tu_test (tu_num)) != ZERO)
                        {
                                switch (tu_err)
                                {
                                case 0x60:
                                        return (SW_ERR);
                                case 0x61:
                                        frub[0].frus[0].conf = conf1;
                                        frub[0].frus[1].conf = 0;
                                        frub[0].frus[1].fru_flag = 0;
                                        break;
                                case 0x62:
                                case 0x63:
                                        frub[0].frus[0].conf = conf2;
                                        frub[0].frus[1].conf = conf3;
                                        frub[0].frus[1].fru_flag = PARENT_NAME;
                                        break;
                                }  /* end switch */
                                frub[0].rcode = 0x100 + tu_err;
                                frub[0].rmsg = SPP_4;
                                tu_err = add_fru ();
                        }  /* endif */
                }  /* endif */
                if (l_mode == NT_LP || l_mode == EX_LP)
                {
                        menu_nmbr &= 0xFFF000;
                        menu_nmbr += 0x000601;
                        asl_rc = wrap_prompt (menu_nmbr, DM_UWP, DM_PCB);
                        if (tu_err != ZERO)
                                return (tu_err);
                        if (asl_rc != ZERO)
                                return (asl_rc);
                }  /* endif */
                if (tu_err != ZERO)     return(tu_err);
                break;
        }  /* end switch */
        return (TU_GOOD);
}  /* select_tu end */

/*
 * NAME: pos_setup
 *
 * FUNCTION: Set POS register bits for the parallel port.  Original settings
 *      are saved first, values changed as required by test units, and then
 *      restored before exiting the DA.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

int     pos_setup (tstptr)
int     tstptr;
{
        int     reg;
        int     address;
        int     rc = ZERO;

        switch (tstptr)
        {
        case TEST10:
                for (reg=2, address=2; reg<=maxreg; reg+=2, address+=2)
                {
                        mddRecord.md_addr = POSREG(reg,slot);
                        mddRecord.md_size = 1;
                        mddRecord.md_incr = MV_BYTE;
                        mddRecord.md_data = &posreg[address];
                        if ((rc = ioctl (mddfd, MIOCCPUT, &mddRecord)) != ZERO)
                                return (SW_ERR);
                }  /* endfor */
                break;
        case TEST20:
                for (reg=2, address=3; reg<=maxreg; reg+=2, address++)
                {
                        mddRecord.md_addr = POSREG(reg,slot);
                        mddRecord.md_size = 1;
                        mddRecord.md_incr = MV_BYTE;
                        mddRecord.md_data = &posreg[address];
                        if ((rc = ioctl (mddfd, MIOCCPUT, &mddRecord)) != ZERO)
                                return (SW_ERR);
                }  /* endfor */
                break;
        case TEST30:
        case TEST40:
        case TEST50:
        case TEST60:
                for (reg=2, address=3; reg<=maxreg; reg+=2, address+=2)
                {
                        mddRecord.md_addr = POSREG(reg,slot);
                        mddRecord.md_size = 1;
                        mddRecord.md_incr = MV_BYTE;
                        mddRecord.md_data = &posreg[address];
                        if ((rc = ioctl (mddfd, MIOCCPUT, &mddRecord)) != ZERO)
                                return (SW_ERR);
                }  /* endfor */
                break;
        default:
                return (SW_ERR);
        }  /* end switch */
        return (rc);
}  /* pos_setup end */

/*
 * NAME: tu_test
 *
 * FUNCTION: Executes the test unit and returns a test completion code to the
 *      select_tu function.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

tu_test (tstptr)  /* begin tu_test */
int     tstptr;
{
        int     tu_rc = 0;
        uchar   turc[4];

        tucb_ptr.header.tu = tstptr;
        tucb_ptr.header.mfg = 0;
        tucb_ptr.header.loop = 0;
        tu_rc = exectu (ddfd, &tucb_ptr);
        return (tu_rc);
}  /* tu_test end */

/*
 * NAME: add_fru
 *
 * FUNCTION: Executes the test unit, interprets the return code then calls
 *      the addfrub function to report the FRU number, name and confidence
 *      level when an error is detected.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

int     add_fru ()
{
        fru_set = FALSE;
        if (insert_frub (&da_input, &frub[0]) != ZERO)
                return (SW_ERR);
        if (addfrub (&frub[0]) != ZERO)
                return (SW_ERR);
        fru_set = TRUE;
        return (TU_BAD);
}  /* add_fru end */

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

int     chk_asl_stat (aslrc)
long    aslrc;
{
        switch (aslrc)
        {
        case ASL_ERR_SCREEN_SIZE:
        case ASL_FAIL:
        case ASL_ERR_NO_TERM:
        case ASL_ERR_NO_SUCH_TERM:
        case ASL_ERR_INITSCR:
                aslrc = SW_ERR;
                if (fru_set == TRUE)
                        aslrc = TU_BAD;
                break;
        case ASL_EXIT:
                aslrc = USR_EXIT;
                break;
        case ASL_CANCEL:
                aslrc = USR_QUIT;
                break;
        case ASL_OK:
        case ASL_ENTER:
        default:
                aslrc = ZERO;
                break;
        }  /* endswitch */
        return (aslrc);
}  /* chk_asl_stat end */

/*
 * NAME: wrap_plug
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

wrap_plug (menu_num, slct)   /* begin wrap_plug_required */
long    menu_num;
int     *slct;
{
        int     asl_rtn;
        char    msgstr[256];
        struct  msglist da_menu[] =
        {
                { PARALLEL_DIAG, DM_NPP, },
                { PARALLEL_DIAG, DM_YES, },
                { PARALLEL_DIAG, DM_NO, },
                { PARALLEL_DIAG, DM_WPR, },
                (int)NULL
        };
        ASL_SCR_TYPE    menutype = {
                        ASL_DIAG_LIST_CANCEL_EXIT_SC, 3, 1
                        };
        ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];

        memset (menu_da, 0, sizeof (menu_da));
        diag_display (menu_num, catd, da_menu, DIAG_MSGONLY,
            ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, menu_da);
        sprintf (msgstr, menu_da[0].text,
            (char *) diag_cat_gets (catd, PARALLEL_DIAG, DM_ADV),"");
        menu_da[0].text = (char *) malloc (strlen(msgstr));
        sprintf (menu_da[0].text, msgstr);
        sprintf (msgstr, menu_da[3].text,
            (char *) diag_cat_gets (catd, PARALLEL_DIAG, DM_MCS));
        menu_da[3].text = (char *) malloc (strlen(msgstr));
        sprintf (menu_da[3].text, msgstr);
        asl_rtn = diag_display (menu_num, catd, NULL, DIAG_IO,
            ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, menu_da);
        *slct = DIAG_ITEM_SELECTED(menutype);
        return (asl_rtn = chk_asl_stat (asl_rtn));
}  /* wrap_plug end */

/*
 * NAME: wrap_prompt
 *
 * FUNCTION:  Builds a title line that describes the adapter/device under test.
 *            Prompts the user to remove or replace cables, attachments or
 *            wrap plugs before continuing.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

wrap_prompt (menu_num, msg_id1, msg_id2)   /* begin wrap_plug_action */
long    menu_num;
int     msg_id1, msg_id2;
{
        int     asl_rtn;
        char    msgstr[256];
        struct  msglist da_menu[] =
        {
                { PARALLEL_DIAG, DM_NPP, },
                { PARALLEL_DIAG, DM_UCB, },
                { PARALLEL_DIAG, DM_PWP, },
                { PARALLEL_DIAG, DM_ENT, },
                (int)NULL
        };
        ASL_SCR_TYPE    menutype = {
                        ASL_DIAG_KEYS_ENTER_SC, 3, 1
                        };
        ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];

        memset (menu_da, 0, sizeof (menu_da));
        da_menu[1].msgid = msg_id1;
        da_menu[2].msgid = msg_id2;
        diag_display (menu_num, catd, da_menu, DIAG_MSGONLY,
            ASL_DIAG_KEYS_ENTER_SC, &menutype, menu_da);
        sprintf (msgstr, menu_da[0].text,
            (char *) diag_cat_gets (catd, PARALLEL_DIAG, DM_ADV),"");
        menu_da[0].text = (char *) malloc (strlen(msgstr));
        sprintf (menu_da[0].text, msgstr);
        asl_rtn = diag_display (menu_num, catd, NULL, DIAG_IO,
            ASL_DIAG_KEYS_ENTER_SC, &menutype, menu_da);
        return (asl_rtn = chk_asl_stat (asl_rtn));
}  /* wrap_prompt end */

/*
 * NAME: cable_prompt
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

cable_prompt (menu_num, msg_id)    /* begin cable_prompt */
long    menu_num;
int     msg_id;
{
        int     asl_rtn;
        char    msgstr[256];
        struct  msglist da_menu[] =
        {
                { PARALLEL_DIAG, DM_NPP, },
                { PARALLEL_DIAG, DM_UCB, },
                { PARALLEL_DIAG, DM_ENT, },
                (int)NULL
        };
        ASL_SCR_TYPE    menutype = {
                        ASL_DIAG_ENTER_SC, 2, 1
                        };
        ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];

        memset (menu_da, 0, sizeof (menu_da));
        da_menu[1].msgid = msg_id;
        diag_display (menu_num, catd, da_menu, DIAG_MSGONLY,
            ASL_DIAG_KEYS_ENTER_SC, &menutype, menu_da);
        sprintf (msgstr, menu_da[0].text,
            (char *) diag_cat_gets (catd, PARALLEL_DIAG, DM_ADV),"");
        menu_da[0].text = (char *) malloc (strlen(msgstr));
        sprintf (menu_da[0].text, msgstr);
        asl_rtn = diag_display (menu_num, catd, NULL, DIAG_IO,
            ASL_DIAG_KEYS_ENTER_SC, &menutype, menu_da);
        return (asl_rtn = chk_asl_stat (asl_rtn));
}  /* cable_prompt end */

/*
 * NAME: dsply_tst_hdr
 *
 * FUNCTION:  Builds a title line that describes the adapter/device under test
 *            and displays test status messages.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

dsply_tst_hdr (menu_num)        /* begin dsply_tst_hdr */
long    menu_num;
{
        int     asl_rtn;
        char    msgstr[512];
        struct  msglist da_menu[] =
        {
                { PARALLEL_DIAG, DM_NPP, },
                (int)NULL
        };
        ASL_SCR_TYPE    menutype = {
                        ASL_DIAG_OUTPUT_LEAVE_SC, 0, 1
                        };
        ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];

        if(e_mode == EXR ) {
                return(ZERO);
        }
        memset (menu_da, 0, sizeof (menu_da));
        diag_display(menu_num, catd, da_menu, DIAG_MSGONLY,
            ASL_DIAG_OUTPUT_LEAVE_SC, &menutype, menu_da);
        if (a_mode == ADV_T)
        {
                if ((l_mode == NT_LP) || (l_mode == EN_LP && s_mode != SYS_T))
                {
                        sprintf (msgstr, menu_da[0].text,
                            (char *) diag_cat_gets (catd, PARALLEL_DIAG,
                            DM_ADV),
                            (char *) diag_cat_gets (catd, PARALLEL_DIAG,
                            DM_SBY));
                }  /* endif */
                else
                {
                        sprintf (msgstr, menu_da[0].text,
                            (char *) diag_cat_gets (catd, PARALLEL_DIAG,
                            DM_ADV),
                            (char *) diag_cat_gets (catd, PARALLEL_DIAG,
                            DM_TLM));
                }  /* endelse */
        }  /* endif */
        else
        {
                sprintf(msgstr, menu_da[0].text, "",
                    (char *) diag_cat_gets (catd, PARALLEL_DIAG, DM_SBY));
        }  /* endelse */
        free (menu_da[0].text);
        menu_da[0].text = (char *) malloc (strlen(msgstr)+1);
        strcpy (menu_da[0].text, msgstr);
        if ((l_mode == IN_LP || l_mode == EX_LP) ||
            (l_mode == EN_LP && s_mode == SYS_T))
        {
                sprintf(msgstr, menu_da[0].text, da_input.lcount,
                    da_input.lerrors);
                free (menu_da[0].text);
                menu_da[0].text = (char *) malloc (strlen(msgstr)+1);
                strcpy (menu_da[0].text, msgstr);
        }  /* endif */
        asl_rtn = diag_display (menu_num, catd, NULL, DIAG_IO,
            ASL_DIAG_OUTPUT_LEAVE_SC, &menutype, menu_da);
        return (asl_rtn = chk_asl_stat (asl_rtn));
}  /* dsply_tst_hdr end */

/*
 * NAME: dsply_ntf_msg
 *
 * FUNCTION:  Builds a title line that describes the adapter/device under test
 *            and displays test status messages.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

dsply_ntf_msg (menu_num)        /* begin dsply_ntf_msg */
long    menu_num;
{
        int     asl_rtn;
        char    msgstr[256];
        struct  msglist da_menu[] =
        {
                { PARALLEL_DIAG, DM_NPP, },
                { PARALLEL_DIAG, DM_NTF, },
                { PARALLEL_DIAG, DM_CNT, },
                (int)NULL
        };
        ASL_SCR_TYPE    menutype = {
                        ASL_DIAG_KEYS_ENTER_SC, 2, 1
                        };
        ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];

        memset (menu_da, 0, sizeof (menu_da));
        diag_display(menu_nmbr, catd, da_menu, DIAG_MSGONLY,
            ASL_DIAG_KEYS_ENTER_SC, &menutype, menu_da);
        if (a_mode == ADV_F)
                sprintf(msgstr, menu_da[0].text, "", "");
        else
                sprintf (msgstr, menu_da[0].text,
                    (char *) diag_cat_gets (catd, PARALLEL_DIAG, DM_ADV), "");
        menu_da[0].text = (char *) malloc (strlen(msgstr));
        sprintf (menu_da[0].text, msgstr);
        asl_rtn = diag_display (menu_nmbr, catd, NULL, DIAG_IO,
            ASL_DIAG_KEYS_ENTER_SC, &menutype, menu_da);
        return (asl_rtn = chk_asl_stat (asl_rtn));
}  /* dsply_ntf_msg end */

/*
 * NAME:  int_handler
 *
 * FUNCTION: Perform clean up on receipt of an interrupt
 *
 * NOTES:
 *
 * RETURNS: None
 *
 */

void    int_handler (int sig)
{
        exit_da (TU_GOOD);
}  /* int_handler end */
