static char sccsid[] = "@(#)38  1.3.3.31  src/bos/diag/da/async/dasync_tu.c, daasync, bos41J, 9522A_all 5/27/95 18:12:32";
/*
 * COMPONENT_NAME: DAASYNC
 *
 * FUNCTIONS:   select_tu ()
 *              tu_10 ()
 *              tu_110 ()
 *              tu_20 ()
 *              tu_80 ()
 *              tu_70 ()
 *              tu_60 ()
 *              tu_50 ()
 *              tu_40 ()
 *              tu_30 ()
 *              tu_test ()
 *              init_frub ()
 *              add_fru ()
 *              chk_4_printer()
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
#include "dasync.h"
#include        "dastreams.h"

extern int     ipl_mod;
extern StreamStackStruct *StreamStack;


/*
 * NAME: select_tu
 *
 * FUNCTION:  Calls test units based on the parameters in da_input.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:
 */

select_tu ()
{
        void    add_fru();
        int     n_CuAt = 0;
        int     getall = FALSE;

        tucb_ptr.ttycb.pat_size = 0;
        tucb_ptr.ttycb.pinout_conv = 0;
        strcpy(tucb_ptr.ttycb.dev_drvr, dd_name);

        switch (Adptr_name)
        {
        case SP1:
        case SP2:
        case SP3:
                chk_4_printer();
                d_cuat = (struct CuAt *) getattr (da_input.dname, "dma_lvl",
                    getall, &n_CuAt);
                if (d_cuat == (struct CuAt *) -1) {
                        err(0x204,0,0);
                        return (-1);
                }
                arblvl = (int) strtol (d_cuat->value, (char**)NULL, 0);
                tu_10 (TEST10);

                if(tucb_ptr.ttycb.adapter != SIO10)
                        tu_20 (TEST20);

                switch(da_mode)
                {
                case ALL_TESTS_MODE:
                        select_attached_device ();
                        if (att_dev == 1 || att_dev == 3)
                                tu_80 (TEST80);
                        if (att_dev < 4 && wrap_7 == TRUE && att_cbl == MI_CABLE)
                                tu_70 (TEST70);
                        switch (tucb_ptr.ttycb.adapter)
                        {
                        case SIO:
                        case SIO9:
                        case SIO10:
                                if (wrap_7 == TRUE)
                                        tu_60 (TEST60);
                                tu_30 (TEST30);
                                break;
                        case SIO4:
                        case SIO6:
                                if (wrap_7 == TRUE) {
                                        tu_60 (TEST60);
                                        if (Adptr_name == SP1)
                                                tu_30 (TEST30);
                                        else {
                                           if ((tu60 == FALSE) && (fru_set == TRUE)) {
                                                insert_fru = FALSE;
                                                last_tu = TRUE;
                                                add_fru ();
                                           } /* endif */
                                        }  /* endelse */
                                }
                                break;
                        case SIO3:
                        case SIO5:
                                if(wrap_7 == TRUE)
                                        tu_30 (TEST30);
                                break;
                        case SIO8:
                                if (wrap_7 == TRUE)
                                        tu_60 (TEST60);
                                if (((Adptr_name == SP1) && (wrap_7 == TRUE)) ||
                                    ((Adptr_name == SP3) && (wrap_11 == TRUE)))
                                        tu_30 (TEST30);
                                else {
                                   if ((tu60 == FALSE) && (fru_set == TRUE)) {
                                        insert_fru = FALSE;
                                        last_tu = TRUE;
                                        add_fru ();
                                   } /* endif */
                                }  /* endelse */
                                break;
                        default:
                                tu_30 (TEST30);
                                break;
                        } /* end switch */
                        break;
                case LOOP_MODE_TESTS:
                        select_attached_device ();
                        switch (att_dev)
                        {
                        case 1:
                        case 3:
                                if (wrap_7 == TRUE)
                                        tu_80 (TEST80);
                                break;
                        case 2:
                                if (wrap_7 == TRUE)
                                        tu_70 (TEST70);
                                break;
                        case 4:
                        case 5:
                                switch (tucb_ptr.ttycb.adapter)
                                {
                                case SIO:
                                case SIO4:
                                case SIO6:
                                        if (wrap_7 == TRUE)
                                                tu_60 (TEST60);
                                        if((wrap_7 == FALSE && wrap_1 == TRUE)
                                            && (tucb_ptr.ttycb.adapter == SIO))

                                                tu_30 (TEST30);
                                        break;
                                case SIO2:
                                case SIO3:
                                case SIO5:
                                case SIO7:
                                case SIO10:
                                        if (wrap_7 == TRUE)
                                                tu_30 (TEST30);
                                        break;
                                case SIO8:
                                        if (wrap_7 == TRUE)
                                                tu_60 (TEST60);

                                        if((wrap_7==FALSE && wrap_11==TRUE &&
                                           Adptr_name==SP3) ||
                                          (wrap_7==TRUE && Adptr_name==SP1))
                                                tu_30 (TEST30);
                                        break;
                                case SIO9:
                                        if (wrap_7 == TRUE)
                                                tu_60 (TEST60);
                                        if(wrap_7 == FALSE && wrap_11 == TRUE)
                                                tu_30 (TEST30);
                                        break;
                                }  /* end switch (tucb_ptr.ttycb.adapter) */
                                break;
                        }  /* end switch (att_dev) */
                        break;
                }  /* end switch (da_mode) */
                break;  /* SP1 & SP2 */
        case EIA_232_64:
                chk_4_printer();
                tu_10 (TEST10);
                if (e_mode != CONC)
                        tu_20 (TEST20);
                switch (da_mode)
                {
                case ALL_TESTS_MODE:
                        select_attached_device ();
                        if (att_dev == 1 || att_dev == 3)
                                tu_80 (TEST80);
                        if (att_dev != 4 && wrap_7 == TRUE)
                        {
                                if(att_cbl == MI_CABLE)
                                        tu_70 (TEST70);
                                if (wrap_7 == TRUE)
                                        tu_60 (TEST60);
                        }  /* endif */
                        if (att_dev == 4)
                               no_dev_att = TRUE;

                        tu_50 (TEST50);
                        if (e_mode != CONC)
                        {
                                tu_40 (TEST40);
                                tu_30 (TEST30);
                        }  /* endif */
                        break;
                case LOOP_MODE_TESTS:
                        select_attached_device ();
                        switch (att_dev)
                        {
                        case 1:
                        case 3:
                                if (wrap_7 == TRUE)
                                        tu_80 (TEST80);
                                break;
                        case 2:
                                if (wrap_7 == TRUE)
                                        tu_70 (TEST70);
                                break;
                        case 4:
                                no_dev_att = TRUE;
                                if (wrap_6 == TRUE)
                                        tu_50 (TEST50);
                                break;
                        }  /* end switch (att_dev) */
                        if (wrap_6 == FALSE && wrap_7 == FALSE)
                        {
                                if (wrap_5 == TRUE)
                                        tu_40 (TEST40);
                                if (wrap_4 == TRUE && wrap_5 == FALSE)
                                        tu_30 (TEST30);
                        }
                        break;
                }  /* end switch (da_mode) */
                break;  /* EIA_232_64 */
        case EIA_232_8:
        case EIA_232_8ISA:
        case EIA_232_16:
        case M_S_188_8:
                if(Adptr_name != M_S_188_8)
                        chk_4_printer();
                tu_10 (TEST10);
                tu_20 (TEST20);
                switch (da_mode)
                {
                case ALL_TESTS_MODE:
                        select_attached_device ();
                        if (att_dev == 1 || att_dev == 3)
                                tu_80 (TEST80);
                        if (att_dev != 4 && wrap_7 == TRUE) {
                                if(att_cbl == MI_CABLE)
                                        tu_70 (TEST70);
                        }
                        else
                                no_dev_att = TRUE;

                        if (wrap_7 == TRUE)
                                tu_50 (TEST50);
                        if (e_mode != CONC)
                        {
                                if (sacasil == FALSE && lampasas == TRUE)
                                        tu_40 (TEST40);
                                if (wrap_2 == TRUE && wrap_3 == TRUE)
                                        tu_30 (TEST30);
                        }
                        break;
                case LOOP_MODE_TESTS:
                        select_attached_device ();
                        switch (att_dev)
                        {
                        case 1:
                        case 3:
                                if (wrap_7 == TRUE)
                                        tu_80 (TEST80);
                                break;
                        case 2:
                                if (wrap_7 == TRUE)
                                        tu_70 (TEST70);
                                break;
                        case 4:
                                no_dev_att = TRUE;
                                if (wrap_7 == TRUE)
                                        tu_50 (TEST50);
                                break;
                        }  /* end switch (att_dev) */
                        if ((wrap_2 == TRUE && wrap_3 == TRUE) &&
                            (wrap_7 == FALSE))
                        {
                                if (sacasil == FALSE && lampasas == TRUE)
                                        tu_40 (TEST40);
                                else
                                        tu_30 (TEST30);
                        }
                        break;
                }  /* end switch (da_mode) */
                break;  /* EIA_232_8, EIA_232_8ISA, EIA_232_16, M_S_188_8 */
        case EIA_422_8:
        case EIA_422_16:
                chk_4_printer();
                tu_10 (TEST10);
                tu_20 (TEST20);
                switch (da_mode)
                {
                case ALL_TESTS_MODE:
                        select_attached_device ();
                        if (att_dev != 3)
                                tu_70 (TEST70);
                        else
                                no_dev_att = TRUE;
                        tu_50 (TEST50);
                        if (e_mode != CONC)
                        {
                                if (sacasil == FALSE && lampasas == TRUE)
                                        tu_40 (TEST40);
                                if (wrap_2 == TRUE && wrap_3 == TRUE)
                                        tu_30 (TEST30);
                        }
                        break;
                case LOOP_MODE_TESTS:
                        select_attached_device ();
                        switch (att_dev)
                        {
                        case 1:
                        case 2:
                                if (wrap_8 == TRUE)
                                        tu_70 (TEST70);
                                if (wrap_7 == TRUE && wrap_8 == FALSE)
                                        tu_50 (TEST50);
                                break;
                        case 3:
                                no_dev_att = TRUE;
                                if (wrap_7 == TRUE)
                                        tu_50 (TEST50);
                                break;
                        }  /* end switch (att_dev) */
                        if ((wrap_7 == FALSE && wrap_8 == FALSE) &&
                            (wrap_2 == TRUE && wrap_3 == TRUE))
                        {
                                if (sacasil == FALSE && lampasas == TRUE)
                                        tu_40 (TEST40);
                                else
                                        tu_30 (TEST30);
                        }
                        break;
                }  /* end switch (da_mode) */
                break;  /* EIA_422_8 & EIA_422_16 */
        case EIA_232_128:
        case EIA_232_128ISA:
                chk_4_printer();
                set_conc_id();
                tu_110 (TEST110);
                tu_20 (TEST20);
                switch (da_mode)
                {
                case ALL_TESTS_MODE:
                        select_attached_device ();
                        if (att_dev == 1 || att_dev == 3)
                                tu_80 (TEST80);
                        if (att_dev != 4 && wrap_7 == TRUE)
                        {
                                if (att_dev != 5 && att_cbl == MI_CABLE)
                                        tu_70 (TEST70);

                                if (wrap_7 == TRUE || att_dev == 5)
                                {
                                        if (att_dev == 5)
                                                tucb_ptr.ttycb.pinout_conv=1;
                                        tu_60 (TEST60);
                                        tucb_ptr.ttycb.pinout_conv=0;
                                }
                        }  /* endif */
                        if(att_dev == 4)
                              no_dev_att = TRUE;
                        tu_50 (TEST50);
                        break;
                case LOOP_MODE_TESTS:
                        select_attached_device ();
                        if ((att_dev == 1 || att_dev == 3) && wrap_7 == TRUE)
                                tu_80 (TEST80);
                        if(att_dev == 2 && wrap_7 == TRUE && att_cbl == MI_CABLE)
                                tu_70 (TEST70);
                        if((wrap_6 == TRUE && wrap_7 == FALSE) ||
                          ((att_dev == 4 || att_dev == 5) && wrap_6 == TRUE)){
                                if(att_dev == 4)
                                       no_dev_att = TRUE;
                                tu_50 (TEST50);
                        }
                        break;
                }  /* end switch (da_mode) */
                break;  /* EIA_232_64 */
        }  /* end switch (Adptr_name) */
}  /* select_tu end */

/*
 * NAME: tu_10
 *
 * FUNCTION: Displays menus and messages to the user.  Calls tu_test function
 *      to run diagnostics on the selected adapter and port as defined by
 *      Adptr_name and tu_num.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

tu_10 (tu_num)  /* begin tu_10 */
int     tu_num;
{
        void    init_frub ();
        void    add_fru ();
        void    exit_da ();

        /* whenever possible, skip testing in sysx preparation pass */
        if( e_mode == SYSX && l_mode == ENTERLM
            && tucb_ptr.ttycb.adapter != UNKNOWN )
                return;

        if (l_mode != EXITLM)
        {
                tu_err = tu_test (tu_num);
                if (tu_err != 0)
                {
                        tu_err += 6;
                        init_frub ();
                        frub[0].sn = Adptr_name;
                        frub[0].rcode = tu_err + 0x0100;
                        frub[0].rmsg = RM_TU10;
                        switch (tu_err)
                        {
                        case 0x10:
                                init_frub ();
                                frub[0].sn = Adptr_name;
                                frub[0].rcode = 0x925;
                                frub[0].rmsg = RM_LM;
                                insert_fru = TRUE;
                                last_tu = TRUE;
                                add_fru ();
                                err(0x205,0,0);
                                exit_da ();
                                break;
                        case 0x11:
                        case 0x12:
                                frub[0].frus[0].conf = conf2;
                                frub[0].frus[1].conf = conf3;
                                frub[0].frus[0].fru_flag = DA_NAME;
                                frub[0].frus[1].fru_flag = PARENT_NAME;
                                break;
                        case 0x13:
                        case 0x14:
                                frub[0].frus[0].conf = conf1;
                                frub[0].frus[0].fru_flag = DA_NAME;
                                break;
                        case 0x15:
                                frub[0].frus[0].conf = conf1;
                                frub[0].frus[0].fmsg = RM_ICPN;
                                strcpy(frub[0].frus[0].fname,
                                   (char *) diag_cat_gets(catd,
                                   ASYNC_CABLE, CONCENTRATOR,
                                   NULL));
                                break;
                        default:
                                frub[0].rcode = 0x0111;
                                frub[0].frus[0].conf = conf2;
                                frub[0].frus[1].conf = conf3;
                                frub[0].frus[0].fru_flag = DA_NAME;
                                frub[0].frus[1].fru_flag = PARENT_NAME;
                                break;
                        }  /* end switch (tu_err) */
                        frub[0].frus[0].fru_exempt = EXEMPT;
                        insert_fru = TRUE;
                        last_tu = TRUE;
                        add_fru ();
                        err(0x206,0,0);
                        exit_da ();
                }  /* endif */
                putdavar(da_input.dname,"adp",DIAG_INT,&tucb_ptr.ttycb.adapter);
        }  /* endif */
}  /* tu_10 end */




/*
 * NAME: tu_110
 *
 * FUNCTION: Displays menus and messages to the user.  Calls tu_test function
 *      to run diagnostics on the selected adapter and port as defined by
 *      Adptr_name and tu_num.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */
tu_110 (tu_num)  /* begin tu_110 */
int     tu_num;
{
        void    rscmenu1(long,int);
        int msg_id;
        long menunumb=0;
         nl_catd my_catd = CATD_ERR;

        void    init_frub ();
        void    add_fru ();
        void    exit_da ();

        if (l_mode != EXITLM)
        {
             tu_err = tu_test (tu_num);


            if (tu_err != 0)
            {
                tu_err += 6;
                init_frub ();
                frub[0].sn = Adptr_name;
                frub[0].rcode = tu_err + 0x0100;
                frub[0].rmsg = RM_TU10;
                switch (tu_err) {
                case 0x10:
                    init_frub ();
                    frub[0].sn = Adptr_name;
                    frub[0].rcode = 0x938;
                    frub[0].rmsg = RM_LM;
                    insert_fru = TRUE;
                    last_tu = TRUE;
                    add_fru ();
                    err(0x207,0,0);
                    exit_da ();
                    break;
                case 0x11:
                    frub[0].rmsg = RM_CC8;
                    frub[0].frus[0].conf = conf2;
                    frub[0].frus[1].conf = conf3;
                    frub[0].frus[0].fru_flag = DA_NAME;
                    frub[0].frus[1].fru_flag = PARENT_NAME;
                    break;
                case 0x12:
                    frub[0].rmsg = RM_TU10;
                    frub[0].frus[0].conf = conf2;
                    frub[0].frus[1].conf = conf3;
                    frub[0].frus[0].fru_flag = DA_NAME;
                    frub[0].frus[1].fru_flag = PARENT_NAME;
                    break;
                case 0x13:
                    frub[0].rmsg = RM_TU10;
                    frub[0].frus[0].conf = conf1;
                    frub[0].frus[0].fru_flag = DA_NAME;
                    break;
                case 0x14:
                    frub[0].rmsg = RM_TU20;
                    frub[0].frus[0].conf = conf1;
                    frub[0].frus[0].fru_flag = DA_NAME;
                    break;
                case 0x15:
                    frub[0].rmsg = RM_TU30;
                    frub[0].frus[0].conf = conf1;
                    frub[0].frus[0].fmsg = RM_ICPN;
                    strcpy(frub[0].frus[0].fname,
                         (char *) diag_cat_gets(catd,
                         ASYNC_CABLE, REMOTE_ASYNC_NODE, NULL));
                    break;
                case 0x16:
                    frub[0].rmsg = RM_LM;
                    frub[0].frus[0].conf = conf2;
                    frub[0].frus[1].conf = conf3;
                    frub[0].frus[0].fmsg = RM_CPN6;
                    frub[0].frus[1].fru_flag = DA_NAME;
                    strcpy(frub[0].frus[0].fname,
                         (char *) diag_cat_gets(catd,
                         ASYNC_CABLE, CONTROLLER_LINE, NULL));
                    strcpy(frub[0].frus[1].fname,
                         (char *) diag_cat_gets(catd,
                         ASYNC_CABLE, ADAPTER, NULL));
                    break;
                case 0x17:
                    frub[0].rcode -= 6;
                    frub[0].rmsg = RM_CC8;
                    frub[0].frus[0].conf = conf1;
                    frub[0].frus[0].fmsg = RM_LM;
                    strcpy(frub[0].frus[0].fname,
                         (char *) diag_cat_gets(catd,
                         ASYNC_CABLE, ADAPTER, NULL));
                    break;
                case 0x18:
                    frub[0].rmsg = RM_CC9;
                    frub[0].frus[0].conf = conf1;
                    frub[0].frus[0].fmsg = RM_ICPN;
                    strcpy(frub[0].frus[0].fname,
                         (char *) diag_cat_gets(catd,
                         ASYNC_CABLE, REMOTE_ASYNC_NODE, NULL));
                    break;
                default:
                    frub[0].rcode = 0x0111;
                    frub[0].frus[0].conf = conf2;
                    frub[0].frus[1].conf = conf3;
                    frub[0].frus[0].fru_flag = DA_NAME;
                    frub[0].frus[1].fru_flag = PARENT_NAME;
                    break;
                }  /* end switch (tu_err) */
                frub[0].frus[0].fru_exempt = EXEMPT;
                insert_fru = TRUE;
                last_tu = TRUE;
                add_fru ();
                err(0x208,0,0);
                exit_da ();
            }  /* endif */
        }  /* endif */
}  /* tu_110 end */




/*
 * NAME: tu_20
 *
 * FUNCTION: Displays menus and messages to the user.  Calls tu_test function
 *      to run diagnostics on the selected adapter and port as defined by
 *      Adptr_name and tu_num.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

tu_20 (tu_num)  /* begin tu_20 */
int     tu_num;
{
        void    init_frub ();
        void    add_fru ();
        void    exit_da ();

        /* whenever possible, skip testing in sysx preparation pass */
        if( e_mode == SYSX && l_mode == ENTERLM )
                return;

        if (l_mode != EXITLM)
        {
                tu_err = tu_test (tu_num);
                if (tu_err != 0)
                {
                        tu_err += 12;
                        init_frub ();
                        frub[0].sn = Adptr_name;
                        frub[0].rcode = tu_err + 0x0100;
                        frub[0].rmsg = RM_TU20;
                        switch (tu_err)
                        {
                        case 0x20:
                                init_frub ();
                                frub[0].sn = Adptr_name;
                                frub[0].rcode = 0x926;
                                frub[0].rmsg = RM_LM;
                                insert_fru = TRUE;
                                last_tu = TRUE;
                                add_fru ();
                                err(0x209,0,0);
                                exit_da ();
                                break;
                        case 0x21:
                        case 0x22:
                        case 0x23:
                        case 0x24:
                                frub[0].frus[0].conf = conf2;
                                frub[0].frus[1].conf = conf3;
                                frub[0].frus[0].fru_flag = DA_NAME;
                                frub[0].frus[1].fru_flag = PARENT_NAME;
                                break;
                        default:
                                frub[0].rcode = 0x0121;
                                frub[0].frus[0].conf = conf2;
                                frub[0].frus[1].conf = conf3;
                                frub[0].frus[0].fru_flag = DA_NAME;
                                frub[0].frus[1].fru_flag = PARENT_NAME;
                                break;
                        }  /* end switch (tu_err) */
                        frub[0].frus[0].fru_exempt = EXEMPT;
                        insert_fru = TRUE;
                        last_tu = TRUE;
                        add_fru (tu_err);
                        err(0x210,0,0);
                        exit_da ();
                }  /* endif */
                if (Adptr_name == SP1 || Adptr_name == SP2 || Adptr_name == SP3)
                {
                        ioctl (fdes, RS_GETA, &Irs_info);
                        Ors_info = Irs_info;
                        Irs_info.rs_dma = arblvl;
                        ioctl (fdes, RS_SETA, &Irs_info);
                        tu_err = tu_test (tu_num);
                        if (tu_err != 0)
                        {
                                tu_err += 12;
                                init_frub ();
                                switch (tu_err)
                                {
                                case 0x20:
                                        init_frub ();
                                        frub[0].sn = Adptr_name;
                                        frub[0].rcode = 0x927;
                                        frub[0].rmsg = RM_LM;
                                        insert_fru = TRUE;
                                        last_tu = TRUE;
                                        add_fru ();
                                        err(0x211,0,0);
                                        exit_da ();
                                        break;
                                case 0x21:
                                case 0x22:
                                case 0x23:
                                case 0x24:
                                        frub[0].sn = Adptr_name;
                                        frub[0].rcode = tu_err + 0x0300;
                                        frub[0].rmsg = RM_TU20;
                                        frub[0].frus[0].conf = conf1;
                                        frub[0].frus[0].fru_flag = PARENT_NAME;
                                        frub[0].frus[0].fru_exempt = EXEMPT;
                                        break;
                                }  /* end switch (tu_err) */
                                insert_fru = TRUE;
                                last_tu = TRUE;
                                fru_set = TRUE;
                                add_fru ();
                                err(0x212,0,0);
                        }  /* endif */
                        ioctl (fdes, RS_SETA, &Ors_info);
                }  /* endif */
        }  /* endif */
}  /* tu_20 end */

/*
 * NAME: tu_80
 *
 * FUNCTION: Displays menus and messages to the user.  Calls tu_test function
 *      to run diagnostics on the selected adapter and port as defined by
 *      Adptr_name and tu_num.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

tu_80 (tu_num)  /* begin tu_80 */
int     tu_num;
{
        void    init_frub ();
        void    add_fru ();
        void    exit_da ();
        int     msg_id;

        if (l_mode == NOTLM || l_mode == ENTERLM)
        {
                wrp_plg = wp_7;
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000801;
                dsply_tst_lst (Menu_nmbr);
                if (slctn == 1)
                {
                        wrap_7 = TRUE;
                        putdavar (da_input.dname, "wp7",
                            DIAG_INT, &wrap_7);
                }  /* endif */
                else
                {
                        wrap_7 = FALSE;
                        tu80 = FALSE;
                        putdavar (da_input.dname, "wp7",
                            DIAG_INT, &wrap_7);
                        return;
                }  /* endelse */
                tu80 = TRUE;
        }
        select_attached_cabling();
        if (l_mode == NOTLM || l_mode == ENTERLM)
        {
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000802;  /* Menu_nmbr 0xnnn802        */
                switch (att_dev)
                {
                   case 1:
                        if(att_cbl == MI_CABLE)
                           msg_id = DM_44;
                        else
                           msg_id = DM_52;
                        break;
                   case 3:
                        if(att_cbl == MI_CABLE)
                           msg_id = DM_46;
                        else
                           msg_id = DM_54;
                        break;
                }  /* end switch (att_dev) */
                wrap_plug_install (Menu_nmbr, msg_id);
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000803;
                dsply_tst_hdr (Menu_nmbr);
                /* whenever possible, skip testing in sysx preparation pass */
                if( e_mode == SYSX ) {
                        return;
                }
        }  /* endif */
        if (l_mode != EXITLM)
        {
                tu_err = tu_test (tu_num);
                DA_SETRC_TESTS (DA_TEST_FULL);

                chk_screen_stat();   /* See if user hit Cancel */

                if (tu_err != 0)
                {
                        tu_err += 48;
                        init_frub ();
                        frub[0].sn = Adptr_name;
                        switch (tu_err)
                        {
                        case 0x80:
                                init_frub ();
                                frub[0].sn = Adptr_name;
                                frub[0].rcode = 0x928;
                                frub[0].rmsg = RM_LM;
                                insert_fru = TRUE;
                                last_tu = TRUE;
                                add_fru ();
                                err(0x213,0,0);
                                exit_da ();
                                break;
                        case 0x81:
                        case 0x82:
                        case 0x83:
                                switch (l_mode)
                                {
                                case NOTLM:
                                        if(att_cbl == MI_CABLE) {
                                           frub[0].rcode = tu_err + 0x0100;
                                           frub[0].rmsg = RM_TU80;
                                           frub[0].frus[0].conf = conf1;
                                           strcpy(frub[0].frus[0].fname,
                                               (char *) diag_cat_gets(catd,
                                               ASYNC_CABLE, INTERPOSER,
                                               NULL));
                                           frub[0].frus[0].fmsg = RM_CIPN;
                                        }
                                        else {
                                           frub[0].rcode = tu_err + 0x0400;
                                           frub[0].rmsg = RM_TU70;
                                           frub[0].frus[0].conf = conf1;
                                           strcpy(frub[0].frus[0].fname,
                                               (char *) diag_cat_gets(catd,
                                               ASYNC_CABLE, CABLE,
                                               NULL));
                                           frub[0].frus[0].fmsg = RM_PTCPN;
                                        }
                                        break;
                                case ENTERLM:
                                case INLM:
                                case EXITLM:
                                        frub[0].rcode = tu_err + 0x0200;
                                        frub[0].rmsg = RM_LM;
                                        frub[0].frus[0].conf = conf4;
                                        frub[0].frus[1].conf = conf5;
                                        frub[0].frus[0].fru_flag = DA_NAME;
                                        frub[0].frus[0].fru_exempt = EXEMPT;
                                        if (Adptr_name == SP1 ||
                                                Adptr_name == SP2 ||
                                                Adptr_name == SP3)
                                        {
                                                strcpy(frub[0].frus[1].fname,
                                                  (char *) diag_cat_gets(catd,
                                                  ASYNC_CABLE, CABLE, NULL));
                                                frub[0].frus[1].fmsg = RM_ACPN;

                                                if(att_cbl == PT_CABLE) {
                                                    frub[0].frus[1].fmsg =
                                                        RM_PTCPN;
                                                    frub[0].rcode = tu_err +
                                                        0x0500;
                                                }

                                        }  /* endif */
                                        else
                                        {
                                                strcpy(frub[0].frus[1].fname,
                                                    (char *) diag_cat_gets(catd,
                                                    ASYNC_CABLE, CABLE_ASSEMBLY,
                                                    NULL));
                                                frub[0].frus[1].fmsg = RM_ICPN;
                                        }  /* endelse */
                                        break;
                                }  /* end switch (l_mode) */
                        }  /* end switch (tu_err) */
                        insert_fru = TRUE;
                        if (l_mode == NOTLM)
                                last_tu = FALSE;
                        else
                                last_tu = TRUE;
                        fru_set = TRUE;
                        add_fru ();
                        err(0x214,0,0);
                }  /* endif */
                if ((Adptr_name == SP1 || Adptr_name == SP2 ||
                                        Adptr_name == SP3) && tu_err == 0)
                {
                        ioctl (fdes, RS_GETA, &Irs_info);
                        Ors_info = Irs_info;
                        Irs_info.rs_dma = arblvl;
                        ioctl (fdes, RS_SETA, &Irs_info);
                        tu_err = tu_test (tu_num);
                        if (tu_err != 0)
                        {
                                tu_err += 48;
                                init_frub ();
                                switch (tu_err)
                                {
                                case 0x80:
                                        init_frub ();
                                        frub[0].sn = Adptr_name;
                                        frub[0].rcode = 0x929;
                                        frub[0].rmsg = RM_LM;
                                        insert_fru = TRUE;
                                        last_tu = TRUE;
                                        add_fru ();
                                        err(0x215,0,0);
                                        exit_da ();
                                        break;
                                case 0x81:
                                case 0x82:
                                case 0x83:
                                        frub[0].sn = Adptr_name;
                                        frub[0].rcode = tu_err + 0x0300;
                                        frub[0].rmsg = RM_TU80;
                                        frub[0].frus[0].conf = conf1;
                                        frub[0].frus[0].fru_flag = PARENT_NAME;
                                        frub[0].frus[0].fru_exempt = EXEMPT;
                                        break;
                                }  /* end switch (tu_err) */
                                insert_fru = TRUE;
                                last_tu = TRUE;
                                fru_set = TRUE;
                                add_fru ();
                                err(0x216,0,0);
                        }  /* endif */
                        ioctl (fdes, RS_SETA, &Ors_info);
                }  /* endif */
        }  /* endif */
        if (l_mode == NOTLM || l_mode == EXITLM)
        {
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000804;
                switch (att_dev)
                {
                   case 1:
                        if(att_cbl == MI_CABLE)
                           msg_id = DM_45;
                        else
                           msg_id = DM_53;
                        break;
                   case 3:
                        if(att_cbl == MI_CABLE)
                           msg_id = DM_47;
                        else
                           msg_id = DM_55;
                        break;
                }  /* end switch (att_dev) */
                wrap_plug_remove (Menu_nmbr, msg_id);
        }  /* endif */
        if (last_tu == TRUE || (tu_err == 0 && e_mode != SYSX))
        {
                DA_SETRC_TESTS (DA_TEST_FULL);
                exit_da ();
        }  /* endif */
}  /* tu_80 end */

/*
 * NAME: tu_70
 *
 * FUNCTION: Displays menus and messages to the user.  Calls tu_test function
 *      to run diagnostics on the selected adapter and port as defined by
 *      Adptr_name and tu_num.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

tu_70 (tu_num)  /* begin tu_70 */
int     tu_num;
{
        void    init_frub ();
        void    add_fru ();
        void    exit_da ();
        int     msg_id;
        int     which_wrap;

        if (l_mode == NOTLM || l_mode == ENTERLM)
        {
                switch (Adptr_name)
                {
                case EIA_422_8:
                case EIA_422_16:
                        wrp_plg = wp_8;
                        which_wrap=8;
                        msg_id = TRUE;
                        break;
                default:
                        wrp_plg = wp_7;
                        which_wrap=7;
                        msg_id = FALSE;
                        break;
                }  /* end switch (Adptr_name */
                if (msg_id == TRUE || tu80 == FALSE)
                {
                        Menu_nmbr &= 0xFFF000;
                        Menu_nmbr += 0x000701;
                        dsply_tst_lst (Menu_nmbr);
                        if (slctn == 1)
                        {
                                switch (which_wrap) {
                                        case 7:
                                                wrap_7 = TRUE;
                                                putdavar (da_input.dname, "wp7",
                                                       DIAG_INT, &wrap_7);
                                                break;
                                         default:
                                                wrap_8 = TRUE;
                                                putdavar (da_input.dname, "wp8",
                                                       DIAG_INT, &wrap_8);
                                                break;
                                }
                        }  /* endif */
                        else
                        {
                                switch (which_wrap) {
                                        case 7:
                                                wrap_7 = FALSE;
                                                putdavar (da_input.dname, "wp7",
                                                        DIAG_INT, &wrap_7);
                                                break;
                                        default:
                                                wrap_8 = FALSE;
                                                putdavar (da_input.dname, "wp8",
                                                        DIAG_INT, &wrap_8);
                                                break;
                                } /* endswitch */
                                tu70 = FALSE;
                                return;
                        }  /* endelse */
                }  /* endif */
                tu70 = TRUE;
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000702;  /* Menu_nmbr == 0xXXX702     */
                switch (Adptr_name)
                {
                case SP1:
                case SP2:
                case SP3:
                case EIA_232_8:
                case EIA_232_8ISA:
                case M_S_188_8:
                case EIA_232_16:
                case EIA_232_64:
                case EIA_232_128:
                case EIA_232_128ISA:
                        switch (att_dev)
                        {
                        case 1:
                        case 3:
                                msg_id = DM_50;
                                break;
                        case 2:
                                msg_id = DM_48;
                                break;
                        }  /* end switch (att_dev) */
                        break;
                case EIA_422_8:
                case EIA_422_16:
                        switch (att_dev)
                        {
                        case 1:
                                msg_id = DM_52;
                                break;
                        case 2:
                                msg_id = DM_54;
                                break;
                        }  /* end switch (att_dev) */
                        break;
                }  /* end switch (Adptr_name) */
                wrap_plug_install (Menu_nmbr, msg_id);
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000703;
                dsply_tst_hdr (Menu_nmbr);
                /* whenever possible, skip testing in sysx preparation pass */
                if( e_mode == SYSX ) {
                        return;
                }
        }  /* endif */
        if (l_mode != EXITLM)
        {
                tu_err = tu_test (tu_num);
                DA_SETRC_TESTS (DA_TEST_FULL);

                chk_screen_stat();   /* See if user hit Cancel */

                if (tu_err != 0)
                {
                        tu_err += 42;
                        init_frub ();
                        frub[0].sn = Adptr_name;
                        switch (tu_err)
                        {
                        case 0x70:
                                init_frub ();
                                frub[0].sn = Adptr_name;
                                frub[0].rcode = 0x930;
                                frub[0].rmsg = RM_LM;
                                insert_fru = TRUE;
                                last_tu = TRUE;
                                add_fru ();
                                err(0x217,0,0);
                                exit_da ();
                                break;
                        case 0x71:
                        case 0x72:
                        case 0x73:
                                switch (l_mode)
                                {
                                case NOTLM:
                                        frub[0].rcode = tu_err + 0x0100;
                                        frub[0].rmsg = RM_TU70;
                                        frub[0].frus[0].conf = conf1;
                                        if (Adptr_name == EIA_232_128 ||
                                            Adptr_name == EIA_232_128ISA)
                                                strcpy(frub[0].frus[0].fname,
                                                  (char *) diag_cat_gets(catd,
                                                  ASYNC_CABLE, CABLE, NULL));
                                        else
                                                strcpy(frub[0].frus[0].fname,
                                                    (char *) diag_cat_gets(catd,
                                                    ASYNC_CABLE, CABLE_ASSEMBLY,
                                                    NULL));
                                        frub[0].frus[0].fmsg = RM_ACPN;
                                        break;
                                case ENTERLM:
                                case INLM:
                                case EXITLM:
                                        frub[0].rcode = tu_err + 0x0200;
                                        frub[0].rmsg = RM_LM;
                                        frub[0].frus[0].conf = conf4;
                                        frub[0].frus[1].conf = conf5;
                                        frub[0].frus[0].fru_flag = DA_NAME;
                                        frub[0].frus[0].fru_exempt = EXEMPT;
                                        if (Adptr_name == SP1 ||
                                                Adptr_name == SP2 ||
                                                Adptr_name == SP3)
                                        {
                                                strcpy(frub[0].frus[1].fname,
                                                  (char *) diag_cat_gets(catd,
                                                  ASYNC_CABLE, CABLE_ASSEMBLY,
                                                  NULL));
                                                frub[0].frus[1].fmsg = RM_ACPN;
                                        }  /* endif */
                                        else
                                        {
                                                strcpy(frub[0].frus[1].fname,
                                                    (char *) diag_cat_gets(catd,
                                                    ASYNC_CABLE, CABLE_ASSEMBLY,
                                                    NULL));
                                                frub[0].frus[1].fmsg = RM_ICPN;
                                        }  /* endelse */
                                        break;
                                }  /* end switch (l_mode) */
                        }  /* end switch (tu_err) */
                        insert_fru = TRUE;
                        if (l_mode == NOTLM)
                                last_tu = FALSE;
                        else
                                last_tu = TRUE;
                        fru_set = TRUE;
                        add_fru ();
                        err(0x218,0,0);
                }  /* endif */
                if ((Adptr_name == SP1 || Adptr_name == SP2 ||
                        Adptr_name == SP3) && (tu_err == 0 && tu80 == FALSE))
                {
                        ioctl (fdes, RS_GETA, &Irs_info);
                        Ors_info = Irs_info;
                        Irs_info.rs_dma = arblvl;
                        ioctl (fdes, RS_SETA, &Irs_info);
                        tu_err = tu_test (tu_num);
                        if (tu_err != 0)
                        {
                                tu_err += 42;
                                init_frub ();
                                frub[0].sn = Adptr_name;
                                frub[0].rcode = tu_err;
                                switch (tu_err)
                                {
                                case 0x70:
                                        init_frub ();
                                        frub[0].sn = Adptr_name;
                                        frub[0].rcode = 0x931;
                                        frub[0].rmsg = RM_LM;
                                        insert_fru = TRUE;
                                        last_tu = TRUE;
                                        add_fru ();
                                        err(0x219,0,0);
                                        exit_da ();
                                        break;
                                case 0x71:
                                case 0x72:
                                case 0x73:
                                        frub[0].sn = Adptr_name;
                                        frub[0].rcode = tu_err + 0x0300;
                                        frub[0].rmsg = RM_TU70;
                                        frub[0].frus[0].conf = conf1;
                                        frub[0].frus[0].fru_flag = PARENT_NAME;
                                        frub[0].frus[0].fru_exempt = EXEMPT;
                                        break;
                                }  /* end switch (tu_err) */
                                insert_fru = TRUE;
                                last_tu = TRUE;
                                fru_set = TRUE;
                                add_fru ();
                                err(0x220,0,0);
                        }  /* endif */
                        ioctl (fdes, RS_SETA, &Ors_info);
                }  /* endif */
                if (fru_set == TRUE && tu_err == 0)
                {
                        insert_fru = FALSE;
                        last_tu = TRUE;
                        add_fru ();
                }  /* endif */
        }  /* endif */
        if (l_mode == NOTLM || l_mode == EXITLM)
        {
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000704;
                switch (Adptr_name)
                {
                case SP1:
                case SP2:
                case SP3:
                case EIA_232_8:
                case EIA_232_8ISA:
                case M_S_188_8:
                case EIA_232_16:
                case EIA_232_64:
                case EIA_232_128:
                case EIA_232_128ISA:
                        switch (att_dev)
                        {
                        case 1:
                        case 3:
                                msg_id = DM_51;
                                break;
                        case 2:
                                msg_id = DM_49;
                                break;
                        }  /* end switch (att_dev) */
                        break;
                case EIA_422_8:
                case EIA_422_16:
                        switch (att_dev)
                        {
                        case 1:
                                msg_id = DM_53;
                                break;
                        case 2:
                                msg_id = DM_55;
                                break;
                        }  /* end switch (att_dev) */
                        break;
                }  /* end switch (Adptr_name) */
                wrap_plug_remove (Menu_nmbr, msg_id);
        }  /* endif */
        if (last_tu == TRUE || (tu_err == 0 && e_mode != SYSX))
        {
                DA_SETRC_TESTS (DA_TEST_FULL);
                exit_da ();
        }
}  /* tu_70 end */

/*
 * NAME: tu_60
 *
 * FUNCTION: Displays menus and messages to the user.  Calls tu_test function
 *      to run diagnostics on the selected adapter and port as defined by
 *      Adptr_name and tu_num.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

tu_60 (tu_num)  /* begin tu_60 */
int     tu_num;
{
        void    init_frub ();
        void    add_fru ();
        void    exit_da ();
        int     msg_id;
        int     which_wrap;

        if (l_mode == NOTLM || l_mode == ENTERLM)
        {
                if (tucb_ptr.ttycb.adapter == SIO ||
                    tucb_ptr.ttycb.adapter == SIO4 ||
                    tucb_ptr.ttycb.adapter == SIO6 ||
                    tucb_ptr.ttycb.adapter == SIO8 ||
                    tucb_ptr.ttycb.adapter == SIO9 ||
                    tucb_ptr.ttycb.adapter == SIO10)
                {
                        switch(tucb_ptr.ttycb.adapter)
                        {
                        case SIO:
                                if (lampasas == TRUE)
                                        wrp_plg = cc_pn1;
                                else
                                        wrp_plg = cc_pn2;
                                break;
                        case SIO4:
                                wrp_plg = cc_pn3;
                                break;
                        case SIO6:
                                wrp_plg = cc_pn4;
                                break;
                        case SIO8:
                                if ((Adptr_name == SP1) ||
                                    (Adptr_name == SP2))
                                        wrp_plg = cc_pn4;
                                else
                                        wrp_plg = cc_pn5;
                                break;
                        case SIO9:
                        case SIO10:
                                wrp_plg = cc_pn5;
                                break;
                        }

                        Menu_nmbr &= 0xFFF000;
                        Menu_nmbr += 0x000605;
                        dsply_tst_lst (Menu_nmbr);
                        if (slctn == 1)
                        {
                                ccbl = TRUE;
                                putdavar (da_input.dname, "scc",
                                    DIAG_INT, &ccbl);
                        }  /* endif */
                        else
                        {
                                ccbl = FALSE;
                                tu60 = FALSE;
                                putdavar (da_input.dname, "scc",
                                    DIAG_INT, &ccbl);

                                if (l_mode == ENTERLM) {
                                    wrap_7 = FALSE;
                                    putdavar (da_input.dname, "wp7",
                                        DIAG_INT, &wrap_7);
                                }  /* endif */

                                return;
                        }  /* endelse */
                }  /* endif */
                if(Adptr_name == EIA_232_128 || Adptr_name == EIA_232_128ISA)
                {
                        if(att_dev == 5) {
                                which_wrap=6;
                                wrp_plg = wp_6;
                        }
                        else {
                                which_wrap=7;
                                wrp_plg = wp_7;
                        }
                }
                else {
                        which_wrap=7;
                        wrp_plg = wp_7;
                }

                switch (Adptr_name)
                {
                case EIA_232_64:
                        dm_idev = DM_35;
                        break;
                case EIA_232_128:
                case EIA_232_128ISA:
                        dm_idev = DM_27;
                        break;
                default :
                        dm_idev = DM_34;
                        break;
                } /* end switch */
                dm_c_s[0] = 0;
                dm_c_s[1] = '\0';
                if (((tu70 == FALSE) && (att_cbl == MI_CABLE)) ||
                    ((tu80 == FALSE) && (att_cbl == PT_CABLE)))
                {
                        Menu_nmbr &= 0xFFF000;
                        Menu_nmbr += 0x000601;
                        dsply_tst_lst (Menu_nmbr);
                        if (slctn == 1)
                        {
                                switch (which_wrap) {
                                        case 6:
                                                wrap_6 = TRUE;
                                                putdavar (da_input.dname, "wp6",
                                                        DIAG_INT, &wrap_6);
                                                break;
                                        default:
                                                wrap_7 = TRUE;
                                                putdavar (da_input.dname, "wp7",
                                                        DIAG_INT, &wrap_7);
                                } /* endswitch */
                        }  /* endif */
                        else
                        {
                                switch (which_wrap) {
                                        case 6:
                                                wrap_6 = FALSE;
                                                putdavar (da_input.dname, "wp6",
                                                        DIAG_INT, &wrap_6);
                                                break;
                                        default:
                                                wrap_7 = FALSE;
                                                putdavar (da_input.dname, "wp7",
                                                        DIAG_INT, &wrap_7);
                                } /* endswitch */
                                tu60 = FALSE;
                                return;
                        }  /* endelse */
                }  /* endif */
                tu60 = TRUE;
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000602;  /* Menu_nmbr == 0xXXX602     */
                msg_id = DM_56;
                wrap_plug_install (Menu_nmbr, msg_id);
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000603;  /* Menu_nmbr == 0xXXX603     */
                dsply_tst_hdr (Menu_nmbr);
                /* whenever possible, skip testing in sysx preparation pass */
                if( e_mode == SYSX ) {
                        return;
                }
        }  /* endif */
        if (l_mode != EXITLM)
        {
                tu_err = tu_test (tu_num);
                DA_SETRC_TESTS (DA_TEST_FULL);

                chk_screen_stat();   /* See if user hit Cancel */

                if (tu_err != 0)
                {
                        tu_err += 36;
                        init_frub ();
                        if (tucb_ptr.ttycb.pinout_conv == 1)
                                tu_err=0x64;
                        switch (tu_err)
                        {
                        case 0x60:
                                init_frub ();
                                frub[0].sn = Adptr_name;
                                frub[0].rcode = 0x932;
                                frub[0].rmsg = RM_LM;
                                insert_fru = TRUE;
                                last_tu = TRUE;
                                add_fru ();
                                err(0x222,0,0);
                                exit_da ();
                                break;
                        case 0x61:
                        case 0x62:
                        case 0x63:
                                frub[0].sn = Adptr_name;
                                frub[0].rcode = tu_err + 0x0100;
                                if ((tucb_ptr.ttycb.adapter != SIO4 &&
                                     tucb_ptr.ttycb.adapter != SIO6 &&
                                     tucb_ptr.ttycb.adapter != SIO8 &&
                                     tucb_ptr.ttycb.adapter != SIO9 &&
                                     tucb_ptr.ttycb.adapter != SIO10) ||
                                    Adptr_name == SP1)
                                {
                                        if (tucb_ptr.ttycb.adapter != SIO4 &&
                                            tucb_ptr.ttycb.adapter != SIO6 &&
                                            tucb_ptr.ttycb.adapter != SIO8 &&
                                            tucb_ptr.ttycb.adapter != SIO9 &&
                                            tucb_ptr.ttycb.adapter != SIO10)
                                        {
                                                frub[0].rmsg = RM_TU60;
                                                if ((lampasas == TRUE) && (Adptr_name != EIA_232_64))
                                                        frub[0].frus[0].fmsg = RM_PN20;
                                                else
                                                        frub[0].frus[0].fmsg = RM_CPN7;
                                        }
                                        else
                                        {
                                                frub[0].rmsg = RM_TU60C;
                                                switch (tucb_ptr.ttycb.adapter) {
                                                   case SIO4:
                                                        frub[0].frus[0].fmsg = RM_PN19;
                                                        break;
                                                   case SIO9:
                                                        frub[0].frus[0].fmsg = RM_PN23;
                                                        break;
                                                   case SIO8:
                                                        if (Adptr_name==SP3)
                                                                frub[0].frus[0].fmsg = RM_PN23;
                                                        else
                                                                frub[0].frus[0].fmsg = RM_PN22;
                                                        break;
                                                   default:
                                                        frub[0].frus[0].fmsg = RM_PN22;
                                                } /* endswitch */
                                        }
                                        frub[0].frus[0].conf = conf1;
                                        strcpy(frub[0].frus[0].fname,
                                            (char *) diag_cat_gets(catd,
                                            ASYNC_CABLE, CONVERTER_CABLE,
                                            NULL));

                                        if (Adptr_name == EIA_232_128 ||
                                            Adptr_name == EIA_232_128ISA) {
                                                frub[0].frus[0].fmsg = RM_CPN7;
                                                strcpy(frub[0].frus[0].fname,
                                                  (char *) diag_cat_gets(catd,
                                                  ASYNC_CABLE, CABLE, NULL));
                                        }
                                }
                                else
                                {
                                        frub[0].frus[0].conf = conf2;
                                        frub[0].frus[1].conf = conf3;
                                        frub[0].rcode += 3;
                                        frub[0].rmsg = RM_TU60C;
                                        frub[0].frus[0].fru_flag = DA_NAME;
                                        strcpy(frub[0].frus[1].fname,
                                            (char *) diag_cat_gets(catd,
                                            ASYNC_CABLE, CONVERTER_CABLE,
                                            NULL));
                                        switch (tucb_ptr.ttycb.adapter) {
                                           case SIO4:
                                                frub[0].frus[1].fmsg = RM_PN19;
                                                break;
                                           case SIO9:
                                           case SIO10:
                                                frub[0].frus[1].fmsg = RM_PN23;
                                                break;
                                           case SIO8:
                                                if(Adptr_name == SP3)
                                                   frub[0].frus[1].fmsg = RM_PN23;
                                                else
                                                   frub[0].frus[1].fmsg = RM_PN22;
                                                break;
                                           default:
                                                frub[0].frus[1].fmsg = RM_PN22;
                                        } /* endswitch */
                                        frub[0].frus[0].fru_exempt = EXEMPT;
                                        frub[0].frus[1].fru_exempt = EXEMPT;
                                }
                                break;
                        case 0x64:
                                frub[0].sn = Adptr_name;
                                frub[0].rcode = tu_err + 0x0100;
                                frub[0].rmsg = RM_TU60C;
                                frub[0].frus[0].conf = conf1;
                                strcpy(frub[0].frus[0].fname,
                                     (char *) diag_cat_gets(catd,
                                     ASYNC_CABLE, CABLE, NULL));
                                frub[0].frus[0].fmsg = RM_PN22;
                                break;
                        }  /* end switch (l_mode) */
                        insert_fru = TRUE;
                        if (((tucb_ptr.ttycb.adapter != SIO4) &&
                             (tucb_ptr.ttycb.adapter != SIO6) &&
                             (tucb_ptr.ttycb.adapter != SIO8)) ||
                            Adptr_name == SP1)
                                last_tu = FALSE;
                        else
                                last_tu = TRUE;
                        fru_set = TRUE;
                        add_fru ();
                        err(0x223,0,0);
                }  /* endif */
                if (fru_set == TRUE && tu_err == 0)
                {
                        insert_fru = FALSE;
                        last_tu = TRUE;
                        add_fru ();
                }  /* endif */
        }  /* endif */
        if (l_mode == NOTLM || l_mode == EXITLM)
        {
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000604;
                msg_id = DM_57;
                wrap_plug_remove (Menu_nmbr, msg_id);
        }  /* endif */
        if (last_tu == TRUE || (tu_err == 0 && e_mode != SYSX))
        {
                DA_SETRC_TESTS (DA_TEST_FULL);
                exit_da ();
        }  /* endif */
}  /* tu_60 end */

/*
 * NAME: tu_50
 *
 * FUNCTION: Displays menus and messages to the user.  Calls tu_test function
 *      to run diagnostics on the selected adapter and port as defined by
 *      Adptr_name and tu_num.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

tu_50 (tu_num)  /* begin tu_50 */
int     tu_num;
{
        void    init_frub ();
        void    add_fru ();
        void    exit_da ();
        int     msg_id;
        int     which_wrap;

        switch (Adptr_name)
        {
        case EIA_232_64:
                wrp_plg = wp_6;
                which_wrap=6;
                msg_id = TRUE;
                break;
        case EIA_232_128:
        case EIA_232_128ISA:
                wrp_plg = wp_9;
                which_wrap=9;
                msg_id = TRUE;
                break;
        case EIA_422_8:
        case EIA_422_16:
                wrp_plg = wp_7;
                which_wrap=7;
                msg_id = TRUE;
                break;
        case EIA_232_8:
        case EIA_232_8ISA:
        case EIA_232_16:
        case M_S_188_8:
        default:
                wrp_plg = wp_7;
                which_wrap=7;
                msg_id = FALSE;
                break;
        }  /* end switch (Adptr_name */
        if (l_mode == NOTLM || l_mode == ENTERLM)
        {
                if ((msg_id == TRUE) || (no_dev_att == TRUE))
                {
                        Menu_nmbr &= 0xFFF000;
                        Menu_nmbr += 0x000501;
                        dsply_tst_lst (Menu_nmbr);
                        if (slctn == 1)
                        {
                                switch (which_wrap) {
                                        case 6:
                                                wrap_6 = TRUE;
                                                putdavar (da_input.dname, "wp6",
                                                        DIAG_INT, &wrap_6);
                                                break;
                                        case 9:
                                                wrap_9 = TRUE;
                                                putdavar (da_input.dname, "wp9",
                                                        DIAG_INT, &wrap_9);
                                                break;
                                        default:
                                                wrap_7 = TRUE;
                                                putdavar (da_input.dname, "wp7",
                                                        DIAG_INT, &wrap_7);
                                } /* endswitch */
                        }  /* endif */
                        else
                        {
                                switch (which_wrap) {
                                        case 6:
                                                wrap_6 = FALSE;
                                                putdavar (da_input.dname, "wp6",
                                                        DIAG_INT, &wrap_6);
                                                break;
                                        case 9:
                                                wrap_9 = FALSE;
                                                putdavar (da_input.dname, "wp9",
                                                        DIAG_INT, &wrap_9);
                                                break;
                                        default:
                                                wrap_7 = FALSE;
                                                putdavar (da_input.dname, "wp7",
                                                DIAG_INT, &wrap_7);
                                } /* endswitch */

                                tu50 = FALSE;

                                if (Adptr_name == EIA_232_128 ||
                                    Adptr_name == EIA_232_128ISA) {
                                        insert_fru = FALSE;
                                        last_tu = TRUE;
                                        add_fru();
                                        DA_SETRC_MORE(DA_MORE_NOCONT);
                                        DA_SETRC_USER(DA_USER_NOKEY);
                                        DA_SETRC_ERROR(DA_ERROR_NONE);
                                        if ((tu60 == TRUE) || (tu70 == TRUE))
                                                DA_SETRC_TESTS(DA_TEST_FULL);
                                        else {
                                                DA_SETRC_STATUS(DA_STATUS_GOOD);
                                        } /* endelse */
                                        exit_da();
                                }
                                else {
                                        if ((fru_set == TRUE) && (e_mode == CONC))
                                        {
                                                insert_fru = FALSE;
                                                last_tu = TRUE;
                                                add_fru ();
                                        }  /* endif */
                                        return;
                                }
                        }  /* endelse */
                }  /* endif */
                tu50 = TRUE;
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000502;          /* Menu_nmbr == 0xXXX502     */
                switch (Adptr_name)
                {
                case EIA_232_8:
                case EIA_232_8ISA:
                case EIA_422_8:
                case M_S_188_8:
                case EIA_232_16:
                case EIA_422_16:
                        msg_id = DM_58;
                        break;
                case EIA_232_64:
                        msg_id = DM_60;
                        break;
                case EIA_232_128:
                case EIA_232_128ISA:
                        msg_id = DM_19;
                        break;
                }  /* end switch (Adptr_name */
                wrap_plug_install (Menu_nmbr, msg_id);
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000503;          /* Menu_nmbr == 0xXXX503     */
                dsply_tst_hdr (Menu_nmbr);
                /* whenever possible, skip testing in sysx preparation pass */
                if( e_mode == SYSX ) {
                        return;
                }
        }  /* endif */
        if (l_mode != EXITLM)
        {
                tu_err = tu_test (tu_num);
                DA_SETRC_TESTS (DA_TEST_FULL);

                chk_screen_stat();   /* See if user hit Cancel */

                if (tu_err != 0)
                {
                        tu_err += 30;
                        init_frub ();
                        frub[0].sn = Adptr_name;
                        frub[0].rcode = tu_err;
                        switch (tu_err)
                        {
                        case 0x50:
                                init_frub ();
                                frub[0].sn = Adptr_name;
                                frub[0].rcode = 0x933;
                                frub[0].rmsg = RM_LM;
                                insert_fru = TRUE;
                                last_tu = TRUE;
                                add_fru ();
                                err(0x224,0,0);
                                exit_da ();
                                break;
                        case 0x51:
                        case 0x52:
                        case 0x53:
                                switch (l_mode)
                                {
                                case NOTLM:
                                        frub[0].rcode = tu_err + 0x0100;
                                        frub[0].rmsg = RM_TU50;

                                        if (Adptr_name == EIA_232_128 ||
                                            Adptr_name == EIA_232_128ISA) {
                                                frub[0].frus[0].conf = conf2;
                                                frub[0].frus[1].conf = conf3;
                                                frub[0].frus[0].fru_flag = DA_NAME;
                                                strcpy(frub[0].frus[1].fname,
                                                   (char *) diag_cat_gets(catd,
                                                   ASYNC_CABLE,REMOTE_ASYNC_NODE,
                                                   NULL));
                                                frub[0].frus[0].fmsg = RM_ICPN;
                                                strcpy(frub[0].frus[1].fname,
                                                   da_input.parent);
                                                strcpy(frub[0].frus[1].floc,
                                                   da_input.parentloc);
                                        }
                                        else {
                                            if (e_mode != CONC)
                                            {
                                                frub[0].frus[0].conf = conf6;
                                                frub[0].frus[1].conf = conf7;
                                                frub[0].frus[0].fmsg = RM_ICPN;
                                                frub[0].frus[1].fru_flag =
                                                    DA_NAME;
                                                frub[0].frus[1].fru_exempt =
                                                    EXEMPT;
                                                frub[0].frus[0].fmsg = RM_ICPN;
                                            }  /* endif */
                                            else
                                            {
                                                frub[0].frus[0].conf = conf4;
                                                frub[0].frus[1].conf = conf5;
                                                frub[0].frus[0].fru_flag =
                                                    DA_NAME;
                                                frub[0].frus[0].fru_exempt =
                                                    EXEMPT;
                                                frub[0].frus[1].fmsg = RM_ICPN;
                                            }  /* endelse */
                                            switch (Adptr_name)
                                            {
                                            case EIA_232_8:
                                            case EIA_232_8ISA:
                                            case M_S_188_8:
                                            case EIA_232_16:
                                            case EIA_422_8:
                                            case EIA_422_16:
                                                if (e_mode != CONC)
                                                   strcpy(frub[0].frus[0].fname,
                                                    (char *) diag_cat_gets(catd,
                                                    ASYNC_CABLE, CABLE_ASSEMBLY,
                                                    NULL));
                                                else
                                                   strcpy(frub[0].frus[1].fname,
                                                    (char *) diag_cat_gets(catd,
                                                    ASYNC_CABLE, CABLE_ASSEMBLY,
                                                    NULL));
                                                break;
                                            case EIA_232_64:
                                                if (e_mode != CONC)
                                                   strcpy(frub[0].frus[0].fname,
                                                    (char *) diag_cat_gets(catd,
                                                    ASYNC_CABLE, CONCENTRATOR,
                                                    NULL));
                                                else
                                                   strcpy(frub[0].frus[1].fname,
                                                    (char *) diag_cat_gets(catd,
                                                    ASYNC_CABLE, CONCENTRATOR,
                                                    NULL));
                                                break;
                                            }  /* end switch (Adptr_name) */
                                        }
                                        break;
                                case ENTERLM:
                                case INLM:
                                case EXITLM:
                                        frub[0].rcode = tu_err + 0x0200;
                                        frub[0].rmsg = RM_LM;
                                        if (Adptr_name == EIA_232_128 ||
                                            Adptr_name == EIA_232_128ISA)
                                                frub[0].frus[0].conf = conf1;
                                        else {
                                                frub[0].frus[0].conf = conf4;
                                                frub[0].frus[1].conf = conf5;
                                        }
                                        frub[0].frus[0].fru_flag = DA_NAME;
                                        frub[0].frus[0].fru_exempt = EXEMPT;
                                        frub[0].frus[1].fmsg = RM_ICPN;
                                        switch (Adptr_name)
                                        {
                                        case EIA_232_8:
                                        case EIA_232_8ISA:
                                        case M_S_188_8:
                                        case EIA_232_16:
                                        case EIA_422_8:
                                        case EIA_422_16:
                                                strcpy(frub[0].frus[1].fname,
                                                    (char *) diag_cat_gets(catd,
                                                    ASYNC_CABLE, CABLE_ASSEMBLY,
                                                    NULL));
                                                break;
                                        case EIA_232_64:
                                                strcpy(frub[0].frus[1].fname,
                                                    (char *) diag_cat_gets(catd,
                                                    ASYNC_CABLE, CONCENTRATOR,
                                                    NULL));
                                                break;
                                        case EIA_232_128:
                                        case EIA_232_128ISA:
                                                strcpy(frub[0].frus[0].fname,
                                                    (char *) diag_cat_gets(catd,
                                                    ASYNC_CABLE,
                                                    REMOTE_ASYNC_NODE, NULL));
                                                break;
                                        }  /* end switch (Adptr_name) */
                                        break;
                                }  /* end switch (l_mode) */
                                break;
                        }  /* end switch (tu_err) */
                        insert_fru = TRUE;
                        if (l_mode == NOTLM && e_mode != CONC &&
                           (Adptr_name != EIA_232_128 &&
                            Adptr_name != EIA_232_128ISA))
                                last_tu = FALSE;
                        else
                                last_tu = TRUE;
                        fru_set = TRUE;
                        add_fru ();
                        err(0x225,0,0);
                }  /* endif */
                if (fru_set == TRUE && tu_err == 0)
                {
                        insert_fru = FALSE;
                        last_tu = TRUE;
                        add_fru ();
                }  /* endif */
        }  /* endif */
        if (l_mode == NOTLM || l_mode == EXITLM)
        {
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000504;
                switch (Adptr_name)
                {
                case EIA_232_8:
                case EIA_232_8ISA:
                case EIA_422_8:
                case M_S_188_8:
                case EIA_232_16:
                case EIA_422_16:
                        msg_id = DM_59;
                        break;
                case EIA_232_64:
                        msg_id = DM_61;
                        break;
                case EIA_232_128:
                case EIA_232_128ISA:
                        msg_id = DM_20;
                        break;
                }  /* end switch (Adptr_name */
                wrap_plug_remove (Menu_nmbr, msg_id);
        }  /* endif */
        if (last_tu == TRUE || (tu_err == 0 && e_mode != SYSX))
        {
                DA_SETRC_TESTS (DA_TEST_FULL);
                exit_da ();
        }  /* endif */
}  /* tu_50 end */

/*
 * NAME: tu_40
 *
 * FUNCTION: Displays menus and messages to the user.  Calls tu_test function
 *      to run diagnostics on the selected adapter and port as defined by
 *      Adptr_name and tu_num.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

tu_40 (tu_num)  /* begin tu_40 */
int     tu_num;
{
        void    init_frub ();
        void    add_fru ();
        void    exit_da ();
        int     msg_id;
        int     which_wrap;
        int     cable_type;

        wrp_plg = wp_5;
        switch (Adptr_name)
        {
        case EIA_232_8:
        case EIA_232_8ISA:
        case EIA_422_8:
        case M_S_188_8:
                wrp_plg = wp_2;
                which_wrap=2;
                break;
        case EIA_232_16:
        case EIA_422_16:
                wrp_plg = wp_3;
                which_wrap=3;
                break;
        case EIA_232_64:
                wrp_plg = wp_5;
                which_wrap=5;
                break;
        }  /* end switch (Adptr_name) */
        if (l_mode == NOTLM || l_mode == ENTERLM)
        {
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000401;
                dsply_tst_lst (Menu_nmbr);
                if (slctn == 1)
                {
                        switch (which_wrap) {
                                case 3:
                                        wrap_3 = TRUE;
                                        putdavar (da_input.dname, "wp3",
                                                DIAG_INT, &wrap_3);
                                        break;
                                case 5:
                                        wrap_5 = TRUE;
                                        putdavar (da_input.dname, "wp5",
                                                DIAG_INT, &wrap_5);
                                        break;
                                default:
                                        wrap_2 = TRUE;
                                        putdavar (da_input.dname, "wp2",
                                                DIAG_INT, &wrap_2);
                        } /* endswitch */
                }  /* endif */
                else
                {
                        switch (which_wrap) {
                                case 3:
                                        wrap_3 = FALSE;
                                        putdavar (da_input.dname, "wp3",
                                                DIAG_INT, &wrap_3);
                                        break;
                                case 5:
                                        wrap_5 = FALSE;
                                        putdavar (da_input.dname, "wp5",
                                                DIAG_INT, &wrap_5);
                                        break;
                                default:
                                        wrap_2 = FALSE;
                                        putdavar (da_input.dname, "wp2",
                                                DIAG_INT, &wrap_2);
                        } /* endswitch */
                        tu40 = FALSE;
                        return;
                }  /* endelse */
                tu40 = TRUE;
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000402;          /* Menu_nmbr == 0xXXX402     */
                if (Adptr_name == EIA_232_64)
                        msg_id = DM_62;
                else
                        msg_id = DM_64;
                wrap_plug_install (Menu_nmbr, msg_id);
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000403;          /* Menu_nmbr == 0xXXX403     */
                dsply_tst_hdr (Menu_nmbr);
                /* whenever possible, skip testing in sysx preparation pass */
                if( e_mode == SYSX ) {
                        return;
                }
        }  /* endif */
        if (l_mode != EXITLM)
        {
                tu_err = tu_test (tu_num);
                DA_SETRC_TESTS (DA_TEST_FULL);

                chk_screen_stat();   /* See if user hit Cancel */

                if (tu_err != 0)
                {
                        tu_err += 24;
                        init_frub ();
                        switch (tu_err)
                        {
                        case 0x40:
                                init_frub ();
                                frub[0].sn = Adptr_name;
                                frub[0].rcode = 0x934;
                                frub[0].rmsg = RM_LM;
                                insert_fru = TRUE;
                                last_tu = TRUE;
                                add_fru ();
                                err(0x226,0,0);
                                exit_da ();
                                break;
                        case 0x41:
                        case 0x42:
                        case 0x43:
                        case 0x44:
                                frub[0].sn = Adptr_name;
                                if(Adptr_name == EIA_232_64)
                                        cable_type = RM_CPN6;
                                else
                                        cable_type = RM_CPN24;
                                if (l_mode == NOTLM)
                                {
                                        frub[0].rcode = tu_err + 0x0100;
                                        frub[0].rmsg = RM_TU40;
                                        frub[0].frus[0].conf = conf1;
                                        strcpy(frub[0].frus[0].fname,
                                            (char *) diag_cat_gets(catd,
                                            ASYNC_CABLE, CONVERTER_CABLE,
                                            NULL));
                                        frub[0].frus[0].fmsg = cable_type;
                                }
                                else
                                {
                                        frub[0].rcode = tu_err + 0x0200;
                                        frub[0].rmsg = RM_LM;
                                        frub[0].frus[0].conf = conf4;
                                        frub[0].frus[1].conf = conf5;
                                        frub[0].frus[0].fru_flag = DA_NAME;
                                        frub[0].frus[0].fru_exempt = EXEMPT;
                                        strcpy(frub[0].frus[1].fname,
                                            (char *) diag_cat_gets(catd,
                                            ASYNC_CABLE, CONVERTER_CABLE,
                                            NULL));
                                        frub[0].frus[1].fmsg = cable_type;
                                }
                                break;
                        }  /* end switch (tu_err) */
                        insert_fru = TRUE;
                        if (l_mode == NOTLM)
                                last_tu = FALSE;
                        else
                                last_tu = TRUE;
                        fru_set = TRUE;
                        add_fru ();
                        err(0x227,0,0);
                }  /* endif */
                if (fru_set == TRUE && tu_err == 0)
                {
                        insert_fru = FALSE;
                        last_tu = TRUE;
                        add_fru ();
                }  /* endif */
        }  /* endif */
        if (l_mode == NOTLM || l_mode == EXITLM)
        {
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000404;
                if (Adptr_name == EIA_232_64)
                        msg_id = DM_63;
                else
                        msg_id = DM_65;
                wrap_plug_remove (Menu_nmbr, msg_id);
        }  /* endif */
        if (last_tu == TRUE || (tu_err == 0 && e_mode != SYSX))
        {
                DA_SETRC_TESTS (DA_TEST_FULL);
                exit_da ();
        }  /* endif */
}  /* tu_40 end */

/*
 * NAME: tu_30
 *
 * FUNCTION: Displays menus and messages to the user.  Calls tu_test function
 *      to run diagnostics on the selected adapter and port as defined by
 *      Adptr_name and tu_num.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

tu_30 (tu_num)  /* begin tu_30 */
int     tu_num;
{
        void    init_frub ();
        void    add_fru ();
        void    exit_da ();
        int     msg_id;
        int     which_wrap;

        switch (Adptr_name)
        {
        case SP1:
        case SP2:
        case SP3:
                switch (tucb_ptr.ttycb.adapter) {
                   case SIO:
                      wrp_plg = wp_1;
                      which_wrap=1;
                      break;
                   case SIO9:
                   case SIO10:
                      wrp_plg = wp_11;
                      which_wrap=11;
                      break;
                   case SIO8:
                      if(Adptr_name == SP3) {
                         wrp_plg = wp_11;
                         which_wrap=11;
                      }
                      else {
                         wrp_plg = wp_7;
                         which_wrap=7;
                      } /* endelse */
                      break;
                   default:
                      wrp_plg = wp_7;
                      which_wrap=7;
                } /* endswitch */
                break;
        case EIA_232_8:
        case EIA_232_8ISA:
        case EIA_422_8:
        case M_S_188_8:
                wrp_plg = wp_2;
                which_wrap=2;
                break;
        case EIA_232_16:
        case EIA_422_16:
                wrp_plg = wp_3;
                which_wrap=3;
                break;
        case EIA_232_64:
                wrp_plg = wp_4;
                which_wrap=4;
                break;
        }  /* end switch (Adptr_name) */
        if (l_mode == NOTLM || l_mode == ENTERLM)
        {
                if ((!strcmp(wrp_plg,wp_2)) ||
                    (!strcmp(wrp_plg,wp_3)) ||
                    (!strcmp(wrp_plg,wp_4)) ||
                    (att_cbl == MI_CABLE &&
                      ( (tucb_ptr.ttycb.adapter == SIO) ||
                        (tucb_ptr.ttycb.adapter == SIO4 && tu60 == FALSE) ||
                        (tucb_ptr.ttycb.adapter == SIO6 && tu60 == FALSE) ||
                        (tucb_ptr.ttycb.adapter == SIO2 && tu70 == FALSE) ||
                        (tucb_ptr.ttycb.adapter == SIO3 && tu70 == FALSE) ||
                        (tucb_ptr.ttycb.adapter == SIO5 && tu70 == FALSE) ||
                        (tucb_ptr.ttycb.adapter == SIO7 && tu70 == FALSE) ||
                        (tucb_ptr.ttycb.adapter == SIO8 && tu60 == FALSE) ||
                        (tucb_ptr.ttycb.adapter == SIO9) ||
                        (tucb_ptr.ttycb.adapter == SIO10) ||
                        (tucb_ptr.ttycb.adapter != SIO2 && sacasil == TRUE))))
                {
                        Menu_nmbr &= 0xFFF000;
                        Menu_nmbr += 0x000301;
                        dsply_tst_lst (Menu_nmbr);
                        if (slctn == 1)
                        {
                                switch (which_wrap) {
                                        case 1:
                                                wrap_1 = TRUE;
                                                putdavar (da_input.dname, "wp1",
                                                        DIAG_INT, &wrap_1);
                                                break;
                                        case 2:
                                                wrap_2 = TRUE;
                                                putdavar (da_input.dname, "wp2",
                                                        DIAG_INT, &wrap_2);
                                                break;
                                        case 3:
                                                wrap_3 = TRUE;
                                                putdavar (da_input.dname, "wp3",
                                                        DIAG_INT, &wrap_3);
                                                break;
                                        case 4:
                                                wrap_4 = TRUE;
                                                putdavar (da_input.dname, "wp4",
                                                        DIAG_INT, &wrap_4);
                                                break;
                                        case 11:
                                                wrap_11 = TRUE;
                                                putdavar (da_input.dname, "wp11",
                                                        DIAG_INT, &wrap_11);
                                                break;
                                        default:
                                                wrap_7 = TRUE;
                                                putdavar (da_input.dname, "wp7",
                                                        DIAG_INT, &wrap_7);
                                } /* endswitch */

                                tu30 = TRUE;
                        }  /* endif */
                        else
                        {
                                switch (which_wrap) {
                                        case 1:
                                                wrap_1 = FALSE;
                                                putdavar (da_input.dname, "wp1",
                                                        DIAG_INT, &wrap_1);
                                                break;
                                        case 2:
                                                wrap_2 = FALSE;
                                                putdavar (da_input.dname, "wp2",
                                                        DIAG_INT, &wrap_2);
                                                break;
                                        case 3:
                                                wrap_3 = FALSE;
                                                putdavar (da_input.dname, "wp3",
                                                        DIAG_INT, &wrap_3);
                                                break;
                                        case 4:
                                                wrap_4 = FALSE;
                                                putdavar (da_input.dname, "wp4",
                                                        DIAG_INT, &wrap_4);
                                                break;
                                        case 11:
                                                wrap_11 = FALSE;
                                                putdavar (da_input.dname, "wp11",
                                                        DIAG_INT, &wrap_11);
                                                break;
                                        default:
                                                wrap_7 = FALSE;
                                                putdavar (da_input.dname, "wp7",
                                                        DIAG_INT, &wrap_7);
                                } /* endswitch */

                                tu30 = FALSE;
                                if (fru_set == TRUE)
                                {
                                        insert_fru = FALSE;
                                        last_tu = TRUE;
                                        add_fru ();
                                }  /* endif */
                                return;
                        }  /* endelse */
                }  /* endif */
                tu30 = TRUE;
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000302;          /* Menu_nmbr == 0xXXX302     */
                switch (Adptr_name)
                {
                case SP1:
                case SP2:
                case SP3:
                        if ((tucb_ptr.ttycb.adapter == SIO ||
                            tucb_ptr.ttycb.adapter == SIO4 ||
                            tucb_ptr.ttycb.adapter == SIO6 ||
                            tucb_ptr.ttycb.adapter == SIO8 ||
                            tucb_ptr.ttycb.adapter == SIO9 ||
                            tucb_ptr.ttycb.adapter == SIO10) && ccbl == TRUE)
                                msg_id = DM_66;
                        else
                                msg_id = DM_80;
                        break;
                case EIA_232_8:
                case EIA_232_8ISA:
                case EIA_422_8:
                case M_S_188_8:
                        if (lampasas == TRUE && sacasil == FALSE)
                                msg_id = DM_70;
                        else
                                msg_id = DM_68;
                        break;
                case EIA_232_16:
                case EIA_422_16:
                        if (lampasas == TRUE && sacasil == FALSE)
                                msg_id = DM_74;
                        else
                                msg_id = DM_72;
                        break;
                case EIA_232_64:
                        msg_id = DM_76;
                        break;
                }  /* end switch (Adptr_name) */
                wrap_plug_install (Menu_nmbr, msg_id);
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000303;          /* Menu_nmbr == 0xXXX303     */
                dsply_tst_hdr (Menu_nmbr);
                /* whenever possible, skip testing in sysx preparation pass */
                if( e_mode == SYSX ) {
                        return;
                }
        }  /* endif */
        if (l_mode != EXITLM)
        {
                tu_err = tu_test (tu_num);
                DA_SETRC_TESTS (DA_TEST_FULL);

                chk_screen_stat();   /* See if user hit Cancel */

                if (tu_err != 0)
                {
                        tu_err += 18;
                        init_frub ();
                        switch (tu_err)
                        {
                        case 0x30:
                                init_frub ();
                                frub[0].sn = Adptr_name;
                                frub[0].rcode = 0x935;
                                frub[0].rmsg = RM_LM;
                                insert_fru = TRUE;
                                last_tu = TRUE;
                                add_fru ();
                                err(0x228,0,0);
                                exit_da ();
                                break;
                        case 0x31:
                        case 0x32:
                        case 0x33:
                        case 0x34:
                                frub[0].sn = Adptr_name;
                                frub[0].rcode = tu_err + 0x0100;
                                frub[0].rmsg = RM_TU30;
                                if (tucb_ptr.ttycb.adapter == SIO)
                                {
                                        frub[0].frus[0].conf = conf2;
                                        frub[0].frus[1].conf = conf3;
                                        frub[0].frus[0].fru_flag = DA_NAME;
                                        frub[0].frus[1].fru_flag = PARENT_NAME;
                                        frub[0].frus[0].fru_exempt = EXEMPT;
                                        frub[0].frus[1].fru_exempt = EXEMPT;
                                }  /* endif */
                                else
                                {
                                        frub[0].frus[0].conf = conf1;
                                        frub[0].frus[0].fru_flag = DA_NAME;
                                        frub[0].frus[1].fru_exempt = EXEMPT;
                                }  /* endelse */
                        }  /* end switch (tu_err) */
                        insert_fru = TRUE;
                        last_tu = TRUE;
                        add_fru ();
                        err(0x229,0,0);
                }  /* endif */
                if ((Adptr_name == SP1 || Adptr_name == SP2 ||
                                        Adptr_name == SP3) && tu_err == 0)
                {
                        ioctl (fdes, RS_GETA, &Irs_info);
                        Ors_info = Irs_info;
                        Irs_info.rs_dma = arblvl;
                        ioctl (fdes, RS_SETA, &Irs_info);
                        tu_err = tu_test (tu_num);
                        if (tu_err != 0)
                        {
                                tu_err += 18;
                                init_frub ();
                                switch (tu_err)
                                {
                                case 0x30:
                                        init_frub ();
                                        frub[0].sn = Adptr_name;
                                        frub[0].rcode = 0x936;
                                        frub[0].rmsg = RM_LM;
                                        insert_fru = TRUE;
                                        last_tu = TRUE;
                                        add_fru ();
                                        err(0x230,0,0);
                                        exit_da ();
                                        break;
                                case 0x31:
                                case 0x32:
                                case 0x33:
                                        frub[0].sn = Adptr_name;
                                        frub[0].rcode = tu_err + 0x0300;
                                        frub[0].rmsg = RM_TU30;
                                        frub[0].frus[0].conf = conf1;
                                        frub[0].frus[0].fru_flag = PARENT_NAME;
                                        frub[0].frus[0].fru_exempt = EXEMPT;
                                        break;
                                }  /* end switch (tu_err) */
                                insert_fru = TRUE;
                                last_tu = TRUE;
                                add_fru ();
                                err(0x231,0,0);
                        }  /* endif */
                        ioctl (fdes, RS_SETA, &Ors_info);
                }  /* endif */
                if (fru_set == TRUE && tu_err == 0)
                {
                        insert_fru = FALSE;
                        last_tu = TRUE;
                        add_fru ();
                }  /* endif */
        }  /* endif */
        if (l_mode == NOTLM || l_mode == EXITLM)
        {
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000304;
                switch (Adptr_name)
                {
                case SP1:
                case SP2:
                case SP3:
                        if ((tucb_ptr.ttycb.adapter == SIO ||
                            tucb_ptr.ttycb.adapter == SIO4 ||
                            tucb_ptr.ttycb.adapter == SIO6 ||
                            tucb_ptr.ttycb.adapter == SIO8 ||
                            tucb_ptr.ttycb.adapter == SIO9 ||
                            tucb_ptr.ttycb.adapter == SIO10) && ccbl == TRUE)
                                msg_id = DM_67;
                        else
                                msg_id = DM_81;
                        break;
                case EIA_232_8:
                case EIA_232_8ISA:
                case EIA_422_8:
                case M_S_188_8:
                        if (lampasas == TRUE && sacasil == FALSE)
                                msg_id = DM_71;
                        else
                                msg_id = DM_69;
                        break;
                case EIA_232_16:
                case EIA_422_16:
                        if (lampasas == TRUE && sacasil == FALSE)
                                msg_id = DM_75;
                        else
                                msg_id = DM_73;
                        break;
                case EIA_232_64:
                        msg_id = DM_77;
                        break;
                }  /* end switch (Adptr_name) */
                wrap_plug_remove (Menu_nmbr, msg_id);
        }  /* endif */
}  /* tu_30 end */

/*
 * NAME: tu_test
 *
 * FUNCTION: Executes the test unit and returns the completion code to the
 *      calling function.
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
        int     i;

        void    init_frub ();
        void    add_fru ();
        void    exit_da ();

        tucb_ptr.header.tu = tstptr;
        tucb_ptr.header.mfg = 0;
        tucb_ptr.header.loop = 1;
        if (e_mode != IPL && c_mode == CONSOLE)
        {
        tucb_ptr.ttycb.baud[0] = 1200;
        tucb_ptr.ttycb.baud[1] = 9600;
        /*
                As a workaround for a hardware problem we will not test
                the async ports at 38400 if the port is native and the
                mode is loop. We will use 19200 instead.
        */
        if (l_mode != NOTLM && (tucb_ptr.ttycb.adapter == SIO  ||
                                tucb_ptr.ttycb.adapter == SIO2 ||
                                tucb_ptr.ttycb.adapter == SIO3 ||
                                tucb_ptr.ttycb.adapter == SIO4 ||
                                tucb_ptr.ttycb.adapter == SIO5 ||
                                tucb_ptr.ttycb.adapter == SIO6 ||
                                tucb_ptr.ttycb.adapter == SIO7 ||
                                tucb_ptr.ttycb.adapter == SIO8 ||
                                tucb_ptr.ttycb.adapter == SIO9 ) )
        {

            tucb_ptr.ttycb.baud[2] = 19200;

        } else {

            tucb_ptr.ttycb.baud[2] = 38400;
        }
        tucb_ptr.ttycb.baud[3] = STOP;
        tucb_ptr.ttycb.chars[0] = 8;
        tucb_ptr.ttycb.chars[1] = STOP;
        tucb_ptr.ttycb.sbits[0] = 1;
        tucb_ptr.ttycb.sbits[1] = 2;
        tucb_ptr.ttycb.sbits[2] = STOP;
        tucb_ptr.ttycb.parity[0] = 1;
        tucb_ptr.ttycb.parity[1] = 2;
        tucb_ptr.ttycb.parity[2] = 3;
        tucb_ptr.ttycb.parity[3] = STOP;
        }  /* endif */
        else
        {
        tucb_ptr.ttycb.baud[0] = 9600;
        tucb_ptr.ttycb.baud[1] = STOP;
        tucb_ptr.ttycb.chars[0] = 8;
        tucb_ptr.ttycb.chars[1] = STOP;
        tucb_ptr.ttycb.sbits[0] = 1;
        tucb_ptr.ttycb.sbits[1] = STOP;
        tucb_ptr.ttycb.parity[0] = 1;
        tucb_ptr.ttycb.parity[1] = STOP;
        }  /* endelse */
        if (tucb_ptr.ttycb.pat_size < 225)
        {
                tucb_ptr.ttycb.pat_size = 225;
                for (i = 0; i < tucb_ptr.ttycb.pat_size; i++)
                    tucb_ptr.ttycb.pattern[i] = 0x1f + i;
        }
   /*
      Test to see if we need to rework the streams stack. If we do then
      we call SetDAStack, otherwise, insure the StackDepth is zero (0).
      The zero (0) value indicates the stack used was ok. If the stack is
      not changed then we need to flush the stack to remove any remaining
      data. If the stack is changed then no flush is needed as the queues
      are removed and replaced. The StackProcessed flag prevents us from
      testing the stack more than once during a test sequence.
   */
        StreamStack->PortId = fdes;
        if (StreamStack->StackProcessed == 0) {

            int temp = 0;

            if((StreamStack->StackDepth = streamio(StreamStack->PortId,
                                I_LIST, NULL)) == DEFAULT_STACK_DEPTH &&
                (streamio(StreamStack->PortId, I_FIND, "ldterm")) &&
                (streamio(StreamStack->PortId, I_FIND, "tioc"))) {

                    StreamStack->StackProcessed = 1;
                    StreamStack->StackDepth = 0;
                    streamio(StreamStack->PortId, I_FLUSH, FLUSHRW);
            }
            else {
                if((temp = SetDAStack(StreamStack)) != 0) {
                    if ((temp == STACK_CALL_FAILED) ||
                        (temp == INVALID_CALL_PARAMETER)) {
                            free(StreamStack->StackList);
                    }

    /*
        As we have an error then we must report it. Set the error and then
        exit the DA.
    */
                    init_frub ();
                    frub[0].sn = Adptr_name;
                    frub[0].rcode = 0x937;
                    frub[0].rmsg = RM_LM;
                    insert_fru = TRUE;
                    last_tu = TRUE;
                    add_fru ();
                    err(0x232,temp,0);
                    exit_da ();

                } /* end of the SetDAStack error return processing */
                StreamStack->StackProcessed = 1;
            } /* end of Change the stack test */
        } /* end of StackProcessed if */

        tu_rc = exectu (fdes, &tucb_ptr);
        return (tu_rc);
}  /* tu_test end */

/*
 * NAME: init_frub
 *
 * FUNCTION:  Initializes the fru bucket structure to zero for the test units.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

void    init_frub ()
{
        strcpy (frub[0].dname, da_input.dname);
        frub[0].sn = 0;
        frub[0].rcode = 0;
        frub[0].rmsg = 0;
        frub[0].frus[0].conf = 0;
        frub[0].frus[1].conf = 0;
        if (Adptr_name == EIA_232_128 || Adptr_name == EIA_232_128ISA) {
              strncpy(frub[0].frus[0].fname,
                  (char *) diag_cat_gets(catd, ASYNC_CABLE, NULL_MSG, NULL),
                  sizeof(frub[0].frus[0].fname));
              strncpy(frub[0].frus[1].fname,
                  (char *) diag_cat_gets(catd, ASYNC_CABLE, NULL_MSG, NULL),
                  sizeof(frub[0].frus[1].fname));
        }
        else {
              strncpy(frub[0].frus[0].fname,
                  (char *) diag_cat_gets(catd, ASYNC_CABLE, NULL_MSG, NULL),
                  14);
              strncpy(frub[0].frus[1].fname,
                  (char *) diag_cat_gets(catd, ASYNC_CABLE, NULL_MSG, NULL),
                  14);
        }
        frub[0].frus[0].fmsg = 0;
        frub[0].frus[1].fmsg = 0;
        frub[0].frus[0].fru_flag = 0;
        frub[0].frus[1].fru_flag = 0;
        frub[0].frus[0].fru_exempt = 0;
        frub[0].frus[1].fru_exempt = 0;
}  /* init_frub end */

/*
 * NAME: add_fru
 *
 * FUNCTION: Sets the rmsg number and then calls the insert_frub[0] and
 *       addfrub[0] functions to report the FRU number, name and confidence
 *       level when an an error is detected.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

void    add_fru ()
{
        void    exit_da ();

        if (insert_fru == TRUE)
        {
                fru_found = TRUE;
                if (insert_frub (&da_input, &frub[0]) != 0)
                {
                        DA_SETRC_ERROR (DA_ERROR_OTHER);
                        DA_SETRC_TESTS (DA_TEST_FULL);
                        err(0x233,0,0);
                        exit_da ();
                }  /* endif */
        }  /* endif */

        if (Adptr_name == EIA_232_128 || Adptr_name == EIA_232_128ISA)
                frub[0].sn = Adptr_name;  /* Force SRN to match publications */

        if (last_tu == TRUE)
        {
                if (addfrub (&frub[0]) != 0)
                {
                        DA_SETRC_ERROR (DA_ERROR_OTHER);
                        DA_SETRC_TESTS (DA_TEST_FULL);
                        err(0x234,0,0);
                        exit_da ();
                }  /* endif */
        }  /* endif */
        DA_SETRC_STATUS (DA_STATUS_BAD);
        DA_SETRC_USER (DA_USER_NOKEY);
        DA_SETRC_ERROR (DA_ERROR_NONE);
        DA_SETRC_TESTS (DA_TEST_FULL);
        DA_SETRC_MORE (DA_MORE_NOCONT);
}  /* add_fru end */

/*
 * NAME: chk_4_printer
 *
 * FUNCTION: Checks if there is a printer configured on the port, and if so
 *       it will disable dtr while the DA is running.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:  Not valid for MIL-STD 188 8-port adapter
 *
 * RETURNS: NONE
 */

chk_4_printer()
{
        char    args[32];
        int     rc = 0;
        int     getall = FALSE;
        int     n_CuAt = 0;
        char    *obuf;

        tucb_ptr.ttycb.prtr_att = FALSE;
        if (!strncmp (lp_name, "lp", 2)) {
                tucb_ptr.ttycb.prtr_att = TRUE;
                t_cuat = (struct CuAt *) getattr (lp_name, "dtr", getall,
                    &n_CuAt);
                if (t_cuat == (struct CuAt *) -1) {
                        err(0x235,0,0);
                        return (-1);
                }
                if (!strcmp (t_cuat->value, "yes"))
                {
                        sprintf (args, " -l %s -a dtr=no", lp_name);
                        rc = invoke_method ("chdev", args, &obuf);
                        dtr_set = TRUE;
                }  /* endif */
        }  /* endif */
}  /* chk_4_printer end */

/*
 * NAME: set_conc_id
 *
 * FUNCTION: Sets the brd_conc_id field of tucb_ptr.ttycb
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:  Only valid for 128-port adapter
 *
 * RETURNS: NONE
 */

set_conc_id()
{
        int brd = 0;
        char *c;

        if (!strncmp(da_input.parent, "cxma", strlen("cxma"))) {
            c = da_input.parent;
            c += 4;
            brd |= (*c - 0x30) << 4;
        } else {
            /* the default conc # will be '0' */
            if (!strncmp(da_input.dname, "cxma", strlen("cxma"))) {
                c = da_input.dname;
                c += 4;
                /* max of 7 boards */
                brd |= (*c - 0x30) << 4;
            }
        }
        tucb_ptr.ttycb.brd_conc_id = brd;

}
