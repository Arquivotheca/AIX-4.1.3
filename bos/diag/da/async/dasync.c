static char sccsid[] = "@(#)25  1.21.3.32  src/bos/diag/da/async/dasync.c, daasync, bos41J, 9519A_all 5/2/95 17:11:55";
/*
 * COMPONENT_NAME: DAASYNC
 *
 * FUNCTIONS:   main ()
 *              setdamode ()
 *              search_odm ()
 *              chk_child ()
 *              chk_pid ()
 *              restore_attributes ()
 *              restore_stream_stack ()
 *              restore_pacing ()
 *              exit_da ()
 *              err_log ()
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
 */
 /* Global variable */

int     ipl_mod;
#include        "dasync.h"
extern  int     get_cpu_model(int *);
#include        "dastreams.h"
StreamStackStruct       *StreamStack;
int     restricted_exit=0;

main ()  /* begin main */
{
        int     index;
        int     mode;
        char    bufstr[128];
        void    exit_da ();
        int     cuat_mod;
        int     odm_rc = 0;
        char    msgstr[2048];
	struct	CuDv *cx_cudv;

 /*  StackProcessing the changed stack flag so it must be set at once */
        StreamStack = (StreamStackStruct *) calloc (1, sizeof (StreamStackStruct));
        StreamStack->StackProcessed = 0;
        StreamStack->StackDepth = 0;

        setlocale (LC_ALL, "");
        DA_SETRC_STATUS (DA_STATUS_GOOD);
        DA_SETRC_USER (DA_USER_NOKEY);
        DA_SETRC_ERROR (DA_ERROR_NONE);
        DA_SETRC_TESTS (DA_TEST_NOTEST);
        DA_SETRC_MORE (DA_MORE_NOCONT);
        init_dgodm ();
        if ((getdainput (&da_input)) == -1)
        {
                DA_SETRC_ERROR (DA_ERROR_OTHER);
                DA_SETRC_TESTS (DA_TEST_FULL);
                err(0x113,0,0);
                exit_da ();
        }  /* endif */
        mode = ipl_mode (&diskette);
        /* set up interrupt handler routine     */
        act.sa_handler = int_handler;
        sigaction (SIGINT, &act, (struct sigaction *)NULL);
        for (index = 0; index < MAXLEVELS; index++)
                strncpy (parents[index], Null, NAMESIZE);
        da_mode = setdamode ();

        if (s_mode == SYSTEM && l_mode == EXITLM)
                exit_da ();
        odm_rc = search_odm ();

        if (e_mode == SYSX && num_PdCn > 1) {
                /* (this check should not be necessary, just for safe) */
                /* At the moment, dasync can't remember parameters */
                /* for several ports because of DAVars structure */
                da_mode = NO_MENU_TEST_MODE;
        }

        if (Adptr_name == SP1)
                tucb_ptr.ttycb.sal_sio = 0;
        else
                tucb_ptr.ttycb.sal_sio = 8;
        Menu_nmbr = Adptr_name << 12;
        strcpy (dv_name, P_cudv->PdDvLn->catalog);
        Set_num = P_cudv->PdDvLn->setno;
        Msg_num = P_cudv->PdDvLn->msgno;

        ipl_mod = get_cpu_model (&cuat_mod); /* ipl_mod now used by chk_child */

        if (Adptr_name == SP1 || Adptr_name == SP2 || Adptr_name == SP3)
        {
                if ((odm_rc = chk_child ()) != 0)
                {
                        DA_SETRC_TESTS (DA_TEST_FULL);
                        err(0x114,odm_rc,0);
                        exit_da ();
                }  /* endif */
        }  /* endif */
        if (c_mode == CONSOLE)
        {
                if (l_mode == NOTLM)
                        diag_asl_init ("NO_TYPE_AHEAD");
                else
                        diag_asl_init ("DEFAULT");
                asl_init_done = 1;
        }  /* endif */
        catd = diag_catopen (MF_DASYNC, 0); /* needed even in no console mode */

	if (Adptr_name == EIA_232_128 || Adptr_name == EIA_232_128ISA) {
		cxma_adapter = TRUE;
        }  /* endif */

        /* On 128-port -- use Adapter name, not RAN name */
        if(Adptr_name == EIA_232_16CONC) {
        	sprintf (bufstr, "name = '%s'", P_cudv->parent);
        	cx_cudv = get_CuDv_list(CuDv_CLASS, bufstr, &c_info ,1 ,2);
        	if (cx_cudv == (struct CuDv *) -1)
        	{
                	DA_SETRC_ERROR (DA_ERROR_OTHER);
                	DA_SETRC_TESTS (DA_TEST_FULL);
                	err(0x115,0,0);
                	exit_da ();
        	}  /* endif */
        	Adptr_name = cx_cudv->PdDvLn->led;
                Menu_nmbr = Adptr_name << 12;   /* Set correct menu number */
        } /* endif (Adptr_name == EIA_232_16CONC) */

        /* If running off POWER platform, don't care about power status regs */
        if ( IsPowerPC(ipl_mod) )
        {
                switch (Adptr_name)
                {
                case SP1:
                case SP2:
                case SP3:
                        if((ipl_mod == RAINBOW3) || (ipl_mod == RAINBOW3P))
                                tucb_ptr.ttycb.adapter = SIO5; /*Rainbow 3&3+ */
                        else
                                tucb_ptr.ttycb.adapter = SIO6; /*Rainbow 4&5 */
                        if ( IsPowerPC_SMP(ipl_mod) ) {
                            switch (ipl_mod) {
                               case FIREBALL:
                                  tucb_ptr.ttycb.adapter = SIO8; /*Fireball*/
                                  break;
                               case PANOLA:
                                  tucb_ptr.ttycb.adapter = SIO9; /*Panola*/
                                  break;
                               default:
                                  tucb_ptr.ttycb.adapter = SIO7; /*Pegasus */
                            } /* endswitch */
                        }
                        if ( is_rspc_model() )
                                  tucb_ptr.ttycb.adapter = SIO10; /* Victory */
                        break;
                case EIA_232_8:
                        tucb_ptr.ttycb.adapter = P8RS232;
                        break;
                case EIA_232_8ISA:
                        tucb_ptr.ttycb.adapter = P8RS232ISA;
                        break;
                case EIA_422_8:
                        tucb_ptr.ttycb.adapter = P8RS422;
                        break;
                case EIA_232_16:
                        tucb_ptr.ttycb.adapter = P16RS232;
                        break;
                case EIA_422_16:
                        tucb_ptr.ttycb.adapter = P16RS422;
                        break;
                case EIA_232_64:
                        tucb_ptr.ttycb.adapter = P64RS232;
                        break;
                case M_S_188_8:
                        tucb_ptr.ttycb.adapter = P8RS188;
                        break;
                case EIA_232_128:
                        tucb_ptr.ttycb.adapter = P128RS232;
                        break;
                case EIA_232_128ISA:
                        tucb_ptr.ttycb.adapter = P128RS232ISA;
                        break;
                default:
                        tucb_ptr.ttycb.adapter = SIO5;
                        break;
                }  /* end switch Adptr_name */
        }
        else
        {
                switch (Adptr_name)
                {
                case EIA_232_128:
                        tucb_ptr.ttycb.adapter = P128RS232;
                        break;
                case EIA_232_128ISA:
                        tucb_ptr.ttycb.adapter = P128RS232ISA;
                        break;
                default:
                        tucb_ptr.ttycb.adapter = UNKNOWN;
                        break;
                }  /* end switch Adptr_name */
                chk_pid ();
        }

        if (s_mode != SYSTEM && e_mode != SYSX)
                option_checkout (num_PdCn);
        else
                system_checkout (num_PdCn);
        DA_SETRC_TESTS (DA_TEST_FULL);
        if (c_mode == CONSOLE)
        {
                asl_rc = diag_asl_read (ASL_DIAG_OUTPUT_LEAVE_SC, FALSE,
                    bufstr);
                chk_asl_stat (asl_rc);
        }  /* endif */
        exit_da ();
}  /* main end */

/*
 * NAME: setdamode
 *
 * FUNCTION:  Returns the execution environment to the DA to determine the
 *      execution mode using data defined in structure da_input and header
 *      file dasync.h.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  damode
 */

setdamode ()    /* begin setdamode */
{
        int   damode = INVALID_TM_INPUT;

        l_mode = da_input.loopmode;     /* NOTLM, ENTERLM, INLM, EXITLM      */
        c_mode = da_input.console;      /* CONSOLE_TRUE, CONSOLE_FALSE       */
        a_mode = da_input.advanced;     /* ADVANCED_TRUE, ADVANCED_FALSE     */
        s_mode = da_input.system;       /* SYSTEM_TRUE, SYSTEM_FALSE         */
        e_mode = da_input.exenv;        /* IPL, STD, MNT, CONC               */
        d_mode = da_input.dmode;        /* PD, REPAIR, ELA, MS1, MS2         */
        if (a_mode == ADVANCED &&
            (s_mode == NOT_SYSTEM && c_mode == CONSOLE))
        {
                if (l_mode == NOTLM)
                        damode = ALL_TESTS_MODE;
                if (l_mode != NOTLM && e_mode != CONC)
                        damode = LOOP_MODE_TESTS;
        }  /* endif */
        if (a_mode == ADVANCED &&
            ((s_mode == NOT_SYSTEM && c_mode == NO_CONSOLE) ||
            (s_mode == SYSTEM && c_mode == NO_CONSOLE) ||
            (s_mode == SYSTEM && c_mode == CONSOLE)))
        {
                damode = NO_MENU_TEST_MODE;
        }  /* endif */
        if(a_mode == NOT_ADVANCED && l_mode == NOTLM)
        {
                damode = NO_MENU_TEST_MODE;
        }  /* endif */
        if (e_mode == SYSX)
                damode = ( a_mode == ADVANCED ) ?
                        LOOP_MODE_TESTS : NO_MENU_TEST_MODE;
        return (damode);
}  /* setdamode end */

/*
 * NAME: search_odm
 *
 * FUNCTIONS:  Search PdCn, CuDv, and PDiagDev for data necessary to configure
 *      ports on the async adapter to be tested.
 *
 * NOTES:  CuDv - defines configured adapter/port/device
 *         PdDv - predefined description of adapter/device
 *         PdCn - predefined data related to adapters/devices
 *         PDiagDev - predefined data related to diagnostics
 *
 * RETURNS:
 */

search_odm ()
{
        char    buffer[128];
        int     i, j;
        int     so_rc = 0;

        void    init_frub ();
        void    add_fru ();
        void    exit_da ();

        sprintf (buffer, "name = '%s'", da_input.dname);
        P_cudv = get_CuDv_list(CuDv_CLASS, buffer, &c_info ,1 ,2);
        if (P_cudv == (struct CuDv *) -1)
        {
                DA_SETRC_ERROR (DA_ERROR_OTHER);
                DA_SETRC_TESTS (DA_TEST_FULL);
                err(0x115,0,0);
                exit_da ();
        }  /* endif */

        Adptr_name = P_cudv->PdDvLn->led;

        sprintf (buffer, "DType = '%s'", P_cudv->PdDvLn->type);
        pdiagdev = get_PDiagDev_list(PDiagDev_CLASS, buffer,
            &p_info ,1 ,2);
        if (pdiagdev == (struct PDiagDev *) -1)
        {
                init_frub ();
                frub[0].sn = Adptr_name;
                frub[0].rcode = 0x901;
                frub[0].rmsg = RM_LM;
                insert_fru = TRUE;
                last_tu = TRUE;
                add_fru ();
                err(0x116,0,0);
                exit_da ();
        }  /* endif */
        if (strlen(pdiagdev->AttSClass))
                sprintf(buffer, "type = %s and subclass = %s",
                    pdiagdev->AttDType, pdiagdev->AttSClass);
        else
                sprintf(buffer, "type = %s", pdiagdev->AttDType);
        pddv = get_PdDv_list(PdDv_CLASS, buffer, &p_info, 1, 2);
        if (pddv == (struct PdDv *) -1)
        {
                init_frub ();
                frub[0].sn = Adptr_name;
                frub[0].rcode = 0x902;
                frub[0].rmsg = RM_LM;
                insert_fru = TRUE;
                last_tu = TRUE;
                add_fru ();
                err(0x117,0,0);
                exit_da ();
        }  /* endif */

        /* search PdCn for all available 'connwhere' locations */
        sprintf(buffer, "uniquetype = '%s' and connkey = '%s'",
            P_cudv->PdDvLn->uniquetype,
            pdiagdev->AttSClass);
        pdcn = get_PdCn_list(PdCn_CLASS, buffer, &p_info, MAXCONN, 1);
        if (pdcn == (struct PdCn *) -1)
        {
                init_frub ();
                frub[0].sn = Adptr_name;
                frub[0].rcode = 0x903;
                frub[0].rmsg = RM_LM;
                insert_fru = TRUE;
                last_tu = TRUE;
                add_fru ();
                err(0x118,0,0);
                exit_da ();
        }  /* endif */
        num_PdCn = p_info.num;

        /* Sort the pdcn list to put all tty's in numerical order */
        /* This was added for 128-port */
        if (Adptr_name == EIA_232_128 || Adptr_name == EIA_232_128ISA) {
                for (i=0;i<num_PdCn;i++) {
                        sprintf (pdcn[i].connwhere,"%d",i);
        	} /* endfor */
	} /* endif (Adptr_name) */

        /* search CuDv for all configured ports */
        sprintf (buffer, "parent = '%s'", da_input.dname);
        C_cudv = get_CuDv_list(CuDv_CLASS, buffer, &c_info ,num_PdCn ,2);
        if (C_cudv == (struct CuDv *) -1)
        {
                init_frub ();
                frub[0].sn = Adptr_name;
                frub[0].rcode = 0x904;
                frub[0].rmsg = RM_LM;
                insert_fru = TRUE;
                last_tu = TRUE;
                add_fru ();
                err(0x119,0,0);
                exit_da ();
        }  /* endif */
        num_CuDv = c_info.num;
        for (i = 0; i < num_CuDv; i++)
        {
                for (j = 0; j < num_PdCn; j++)
                {
                        if (!strcmp (C_cudv[i].connwhere, pdcn[j].connwhere))
                        {
                                if (C_cudv[i].status == 0)
                                        strncpy (pdcn[j].connkey, pdef, 2);
                                else
                                        strncpy (pdcn[j].connkey, pcfg, 2);
                                break;
                        }  /* endif */
                }  /* endfor */
        }  /* endfor */
        return (so_rc);
}  /* search_odm end */

/*
 * NAME: chk_child
 *
 * FUNCTIONS:  Search PdCn, CuDv, and PDiagDev for data necessary to configure
 *      ports on the async adapter to be tested.
 *
 * NOTES:  CuDv - defines configured adapter/port/device
 *         PdDv - predefined description of adapter/device
 *         PdCn - predefined data related to adapters/devices
 *         PDiagDev - predefined data related to diagnostics
 *
 * RETURNS:
 */

chk_child ()
{
        struct  CuDv *SA;
        struct  listinfo sa_info;
        int     i;
        int     num_cucn;
        int     cc_rc = 0;
        char    buffer[16];

        void    init_frub ();
        void    add_fru ();
        void    exit_da ();

        switch(Adptr_name) {
           case SP1:
              sprintf (buffer, "parent = '%s'", "sa0");
              SA = get_CuDv_list(CuDv_CLASS, buffer, &sa_info ,1 ,2);
              break;
           case SP2:
              sprintf (buffer, "parent = '%s'", "sa1");
              SA = get_CuDv_list(CuDv_CLASS, buffer, &sa_info ,1 ,2);
              break;
           case SP3:
              if ( IsPowerPC_SMP(ipl_mod) ) {
                 sprintf (buffer, "parent = '%s'", "sa2");
                 SA = get_CuDv_list(CuDv_CLASS, buffer, &sa_info ,1 ,2);
              }
              else {
                 SA = (struct CuDv *)0;
                 sa_info.num = 0;
              }
              break;
        }

        if (SA == (struct CuDv *) -1)
        {
                init_frub ();
                frub[0].sn = Adptr_name;
                frub[0].rcode = 0x905;
                frub[0].rmsg = RM_LM;
                insert_fru = TRUE;
                last_tu = TRUE;
                add_fru ();
                err(0x120,0,0);
                exit_da ();
        }  /* endif */

        for (i = 0;((i < sa_info.num) && (cc_rc == 0)); i++) {
            if (!strncmp (SA[i].name, "dials", 5) ||
                !strncmp (SA[i].name, "lpf", 3)   ||
                !strncmp (SA[i].name, "lp",  2)) {
                    err(0x121,0,0);
                    cc_rc = M_G;
                    break;
            }
        }

        odm_free_list(SA, &sa_info);
        return (cc_rc);
}  /* chk_child end */

/*
 * NAME: chk_pid
 *
 * FUNCTIONS:  Open the machine device driver, read planar POS register 0.
 *
 * RETURNS:  none
 */

chk_pid ()
{
        int     io_rc = 0;
        uchar   mach_data[4];

        void    init_frub ();
        void    add_fru ();
        void    exit_da ();

        if ((mddfd = OPEN_MDD) == FAIL) {
                err(0x124,mddfd,errno);
                err_chk (errno);
        }
        mddRecord = (MACH_DD_IO *) calloc (1, sizeof (MACH_DD_IO));
        mddRecord->md_addr = 0x4000e7;          /* SIO POS register  */
        mddRecord->md_size = 1;                 /* move one byte     */
        mddRecord->md_incr = MV_BYTE;           /* byte read         */
        mddRecord->md_data = mach_data;         /* address of data   */
        if ((io_rc = ioctl (mddfd, MIOCCGET, mddRecord)) != 0)
        {
                init_frub ();
                frub[0].sn = Adptr_name;
                frub[0].rcode = 0x906;
                frub[0].rmsg = RM_LM;
                insert_fru = TRUE;
                last_tu = TRUE;
                add_fru ();
                err(0x125,io_rc,errno);
                exit_da ();
        } /* endif */
        io_rc = mach_data[0];
        if ((io_rc & MACH_MASK) == LAMPASAS)
                lampasas = TRUE;
        else
                lampasas = FALSE;
        mddRecord->md_addr = POSREG(0,15);      /* SIO POS register  */
        mddRecord->md_size = 1;                 /* move one byte     */
        mddRecord->md_incr = MV_BYTE;           /* byte read         */
        mddRecord->md_data = mach_data;         /* address of data   */
        if ((io_rc = ioctl (mddfd, MIOCCGET, mddRecord)) != 0)
        {
                init_frub ();
                frub[0].sn = Adptr_name;
                frub[0].rcode = 0x907;
                frub[0].rmsg = RM_LM;
                insert_fru = TRUE;
                last_tu = TRUE;
                add_fru ();
                err(0x126,io_rc,errno);
                exit_da ();
        }  /* endif */
        if (mach_data[0] == SIO2)
                sacasil = TRUE;
        else
                sacasil = FALSE;
        free (mddRecord);
}  /* chk_pid end */

/*
 * NAME: err_chk
 *
 * FUNCTIONS:  Checks errno when an unknown failure (-1) is returned by any
 *      function call.  If unknown hardware failure fru bucket is setup.
 *
 * NOTES:
 *
 * RETURNS:
 */

err_chk (err_num)
int     err_num;

{
        if ((err_num == EIO) || (err_num == ENODEV) || (e_mode != CONC))
        {
                init_frub ();
                frub[0].sn = Adptr_name;
                frub[0].rcode = 0x0111;
                frub[0].rmsg = RM_LM;
                frub[0].frus[0].conf = conf2;
                frub[0].frus[1].conf = conf3;
                frub[0].frus[0].fru_flag = DA_NAME;
                frub[0].frus[1].fru_flag = PARENT_NAME;
                if (Adptr_name == SP1 || Adptr_name == SP2 || Adptr_name == SP3)
                        frub[0].frus[0].fru_exempt = NONEXEMPT;
                else
                        frub[0].frus[0].fru_exempt = EXEMPT;
                insert_fru = TRUE;
                last_tu = TRUE;
                add_fru ();
                if (e_mode != CONC)
                        err(0x249,0,err_num);
                else
                        err(0x127,0,err_num);
                exit_da ();
        }  /* endif */
        else
        {
                DA_SETRC_ERROR (DA_ERROR_OPEN);
                DA_SETRC_TESTS (DA_TEST_FULL);
                err(0x128,0,err_num);
                exit_da ();
        }  /* endelse */
}  /* err_chk end */

/*
 * NAME: restore_attributes
 *
 * FUNCTION:  Restore the modem control bits to original state if they were set.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  NONE
 */

void restore_attributes ()
{
        void    init_frub ();
        void    add_fru ();
        void    exit_da ();

        if (fdes != -1 && attrs_saved == 1) {
            attrs_saved = 0; /* try only one time: can be called from exit_da */
            if ((ioctl (fdes, TCSETA, &SaveAttrs)) != 0) {
                init_frub ();
                frub[0].sn = Adptr_name;
                frub[0].rcode = 0x908;
                frub[0].rmsg = RM_LM;
                insert_fru = TRUE;
                last_tu = TRUE;
                add_fru ();
                err(0x129,0,errno);
                /* if called from exit_da, exit_da must not be
                   called back to prevent setting DA_ERROR_OTHER */
                if (restricted_exit == 0) /* not called from exit_da */
                        exit_da ();
            }
        }
}
/*
 * NAME: restore_stream_stack
 *
 * FUNCTION:  Restore the streams stack to the user's configuration
 *      if needed
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  NONE
 */

void restore_stream_stack ()
{
        int     temp = 0;

        void    init_frub ();
        void    add_fru ();
        void    exit_da ();

        if (StreamStack->StackProcessed != 0 &&
                StreamStack->StackDepth != 0 &&
                StreamStack->PortId != 0) {
            if( (temp = RestoreUserStack(StreamStack)) != 0 ) {
                StreamStack->StackProcessed = 0;
                StreamStack->StackDepth = 0;
    /*
        As we have an error then we must report it. Set the error and then
        exit the DA.
    */
                init_frub ();
                frub[0].sn = Adptr_name;
                frub[0].rcode = 0x909;
                frub[0].rmsg = RM_LM;
                insert_fru = TRUE;
                last_tu = TRUE;
                add_fru ();
                err(0x130,0,errno);
                /* if called from exit_da, exit_da must not be
                   called back to prevent setting DA_ERROR_OTHER */
                if (restricted_exit == 0) /* not called from exit_da */
                        exit_da ();
            } /* end of the error processing on RestoreUserStack */
        }
        StreamStack->StackProcessed = 0;
        StreamStack->StackDepth = 0;
}

/*
 * NAME: restore_pacing
 *
 * FUNCTION:  Restores the user's hardware pacing settings.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  NONE
 */

void restore_pacing ()
{
        if(ioctl(fdes, CXMA_SETA, &OrigPacing)) {
                init_frub ();
                frub[0].sn = Adptr_name;
                frub[0].rcode = 0x942;
                frub[0].rmsg = RM_LM;
                insert_fru = TRUE;
                last_tu = TRUE;
                add_fru ();
                err(0x248,0,errno);

                /* if called from exit_da, exit_da must not be
                   called back to prevent setting DA_ERROR_OTHER */

                if (restricted_exit == 0)    /* not called from exit_da */
                        exit_da ();
        }
}

/*
 * NAME: exit_da
 *
 * FUNCTION:  Calls error analysis function to check error log if no errors
 *      were found by the test units.  Performs cleanup operations prior to
 *      returning to the diagnostic controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  NONE
 */

void exit_da () /* begin exit_da */
{

        void    unconfig_port();
        int     rc = 0;
        char    args[32];
        char    *obuf;

 /*
     If restricted_exit > 0 then exit_da() processing was already being done
     when the new error occurred.  To prevent an infinite loop, just return
     to the call point so processing will continue and we can exit as
     gracefully as possible.  If restricted_exit > 5, then we have a severe
     looping problem so just bail out.
 */

        if (restricted_exit > 0) {
                if (restricted_exit > 5) {  /* Severe.  Bail out of DA */
                        err(0x245,0,0);
                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                        DA_EXIT ();
                }
                else {                /* Just return and attempt to continue */
                        err(0x242,0,0);
                        restricted_exit += 1;
                        return;
                }
        }
        else                              /* First call to exit_da() */
               restricted_exit=1;


 /*
     If the mode is ENTERLM or INLM, check to see if user pressed Cancel
 */
        if((l_mode == ENTERLM) || (l_mode == INLM)) {
                chk_screen_stat();
        }

 /*
     Restore the modem control bits to original state if they were set.
 */
        restore_attributes ();

 /*
     Before closing the port we need to restore the streams stack to the
     user's configuration so he can use it later.
 */
        restore_stream_stack ();

/*
     Before closing the port we need to restore the hardware pacing to the
     user's pacing setting so he can use it later.
*/

        if (pacing_saved) {
                restore_pacing ();
        }

        if (dtr_set)
        {
                sprintf (args, " -l %s -a dtr=yes", lp_name);
                rc = invoke_method ("chdev", args, &obuf);
        }  /* endif */
        if (mddfd != -1)
                close (mddfd);
        if (fdes != -1)
                close (fdes);
        if (made_device != 0)
                unconfig_port ();
        odm_terminate ();
        if (catd != CATD_ERR) {
                catclose (catd);
        }  /* endif */
        if (asl_init_done) {
                diag_asl_quit ("DEFAULT");
        }  /* endif */
        err(0x999,0,0);   /* Signal that we are exiting */
        err(0xfff,0,0);   /* Clear the LEDs */
        DA_EXIT ();
}  /* exit_da end */

/*
 * NAME: err_log
 *
 * FUNCTION:  Checks for device errors logged in the system error log.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  Failing fru
 */

err_log ()
{
}  /* err_log end */

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
        exit_da ();
}  /* int_handler end */
