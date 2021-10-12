static char sccsid[] = "@(#)37  1.2.1.33  src/bos/diag/da/async/dasync_cfg.c, daasync, bos41J, 9519A_all 5/2/95 17:12:14";
/*
 * COMPONENT_NAME: DAASYNC
 *
 * FUNCTIONS:   option_checkout ()
 *              system_checkout ()
 *              config_port ()
 *              cfg_device ()
 *              unconfig_port ()
 *              ucfg_device ()
 *              open_tty ()
 *              invoke_method ()
 *              sync_test()
 *              sync_test2()
 *              chk_config()
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

/*
 * NAME: option_checkout
 *
 * FUNCTION:  Sets global variables using data from structure da_input and
 *      the predefined diagnostic devices object class that are used by the
 *      DA to build menus and messages.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:  portnum == port number to be tested on NIO planar.
 *         prt_num == port number to be tested on adapter.
 *         nports  == total number of ports on adapter to be tested.
 *         devtty  == device driver file name.
 *
 * RETURNS: NONE
 */

option_checkout (num_pdcn)      /* begin option_checkout */
int     num_pdcn;
{
        int     i, j, k;
        char    bufstr[32];     /* temporary buffer used by sprintf          */
        char    namebuf[NAMESIZE];
        void    cfg_device ();
        int     cxma_selection = TRUE;
        int     msg_id;

        void    init_frub ();
        void    add_fru ();
        void    exit_da ();

        if (Adptr_name == EIA_232_128 || Adptr_name == EIA_232_128ISA) {
                sync_test(num_pdcn);
                if (cxma_adapter == TRUE) {  /* Testing adapter w/o RANs */
                        DA_SETRC_ERROR(DA_ERROR_NONE);
                        DA_SETRC_TESTS(DA_TEST_FULL);
                        DA_SETRC_STATUS(DA_STATUS_GOOD);
                        exit_da();
                }
        }

        /* Save the device driver name */
        strcpy(dd_name, P_cudv->PdDvLn->DvDr);

        if (strlen (da_input.child1) == 0)
        {
                if (c_mode == CONSOLE && num_PdCn > 1)
                {
                        Menu_nmbr &= 0xFFF000;
                        Menu_nmbr += 0x000101;
                        i = dsply_tst_slctn (Menu_nmbr);
                }  /* endif */
                else
                {
                        i = 0;
                        cudv_ptr[i] = i;
                }  /* endelse */
                j = cudv_ptr[i];
                if ((c_mode == CONSOLE) &&
                    (strncmp (pdcn[i].connkey, pcfg, 2)))
                {
                        Menu_nmbr &= 0xFFF000;
                        Menu_nmbr += 0x000104;
                        dsply_cfg_hdr (Menu_nmbr);
                }  /* endif */
                if ((strncmp (pdcn[i].connkey, pdef, 2)) &&
                    (strncmp (pdcn[i].connkey, pcfg, 2)))
                {
                        k = config_port (i);
                        sprintf (devtty, "/dev/%s", D_cudv[k].name);
                        strcpy (namebuf, D_cudv[k].name);
                        strcpy (bufstr, D_cudv[k].location);
                }  /* endif */
                else
                {
                        if (C_cudv[j].status != Available)
                                cfg_device (C_cudv[j].name);
                        sprintf (devtty, "/dev/%s", C_cudv[j].name);
                        strcpy (namebuf, C_cudv[j].name);
                        strcpy (bufstr, C_cudv[j].location);
                }  /* endelse */
        }  /* endif */
        else
        {
                for (j = 0; j < num_CuDv; j++)
                {
                        if (!strcmp (C_cudv[j].name, da_input.child1))
                                break;
                }  /* endfor */
                if (C_cudv[j].status != Available)
                {
                        if (c_mode == CONSOLE)
                        {
                                Menu_nmbr &= 0xFFF000;
                                Menu_nmbr += 0x000104;
                                dsply_cfg_hdr (Menu_nmbr);
                        }  /* endif */
                        cfg_device (da_input.child1);
                }  /* endif */
                sprintf (devtty, "/dev/%s", da_input.child1);
                strcpy (namebuf, da_input.child1);
                strcpy (bufstr, da_input.childloc1);
        }  /* endelse */
        strcpy (lp_name, namebuf);
        if (l_mode == NOTLM || l_mode == ENTERLM)
        {
                if (e_mode != IPL && made_device != 1) {
                        dd_state = open_tty (namebuf);
                        switch(dd_state) {
                              case In_Use:
                                      /* Device other than console cannot be */
                                      /* busy except in Concurrent mode */
                                      err_chk(0);  /* 0 - only check if in Standalone mode */
                                      break;
                              case Console:
                                      dd_state = In_Use;
                                      break;
                              default:
                                      dd_state = Available;
                                      break;
                        }  /* endswitch */
                }  /* endif */
                else {
                        dd_state = Available;
                }
                putdavar (da_input.dname, "dds", DIAG_INT, &dd_state);
        }  /* endif */
        else
                getdavar (da_input.dname, "dds", DIAG_INT, &dd_state);
        if (dd_state == In_Use)
        {
                DA_SETRC_ERROR(DA_ERROR_OPEN);
                err(0x149,0,0);
                exit_da ();
        }  /* endif */
        switch (Adptr_name)
        {
        case SP1:
        case SP2:
        case SP3:
                i = 0;
                while (bufstr[i] != 'S' && i < 11)
                        i++;
                portnum[0] = bufstr[i];
                portnum[1] = bufstr[i+1];
                break;
        case EIA_232_8:
        case EIA_232_8ISA:
        case M_S_188_8:
        case EIA_422_8:
                n_ports = num_pdcn;
                break;
        case EIA_232_16:
        case EIA_422_16:
                n_ports = num_pdcn + 6;
                break;
        case EIA_232_64:
                n_ports = bufstr[7] - 0x30;
                break;
        case EIA_232_128:
        case EIA_232_128ISA:
                n_ports = 128;
                break;
        }  /* end switch (Adptr_name) */
        if (Adptr_name != SP1 && Adptr_name != SP2 && Adptr_name != SP3)
        {
                if (bufstr[9] == '0')
                        prt_num = bufstr[10] - 0x30;
                else
                        prt_num = bufstr[10] - 0x20;
        }  /* endif */
        if (c_mode == CONSOLE)
        {
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000102;
                dsply_tst_hdr (Menu_nmbr);
        }  /* endif */
        if ((fdes = open (devtty, OPEN_TTY)) == -1) {
                err(0x150,0,errno);
                err_chk (errno);
        }  /* endif */

        /*
            Until the streams system gets setup so that it will function
            without CLOCAL set, we will need to continue to set it before
            calling exectu, then clear it when exit_da() is called at the end.
            Initially save the current attributes into SaveAttributes and
            add CLOCAL via WorkingAttrs.
        */

        if ((ioctl (fdes, TCGETA, &SaveAttrs)) != 0) {
            init_frub ();
            frub[0].sn = Adptr_name;
            frub[0].rcode = 0x911;
            frub[0].rmsg = RM_LM;
            insert_fru = TRUE;
            last_tu = TRUE;
            add_fru ();
            err(0x151,0,errno);
            exit_da();
        }
        else
            attrs_saved=1;      /* Attributes were saved */

        if ((ioctl (fdes, TCGETA, &WorkingAttrs)) != 0) {
            init_frub ();
            frub[0].sn = Adptr_name;
            frub[0].rcode = 0x912;
            frub[0].rmsg = RM_LM;
            insert_fru = TRUE;
            last_tu = TRUE;
            add_fru ();
            err(0x152,0,errno);
            exit_da();
        }

        WorkingAttrs.c_cflag |= CLOCAL;
        if ((ioctl (fdes, TCSETA, &WorkingAttrs)) != 0) {
            init_frub ();
            frub[0].sn = Adptr_name;
            frub[0].rcode = 0x913;
            frub[0].rmsg = RM_LM;
            insert_fru = TRUE;
            last_tu = TRUE;
            add_fru ();
            err(0x153,0,errno);
            exit_da();
        }

        /* End of the CLOCAL update */

        /*
           If testing 128-port, force hardware pacing OFF.  Otherwise, modem
           wrap tests will fail due to DTR not dropping.
        */

        if (Adptr_name == EIA_232_128 || Adptr_name == EIA_232_128ISA) {
                if(ioctl(fdes, CXMA_GETA, &OrigPacing)) {
                        init_frub();
                        frub[0].sn = Adptr_name;
                        frub[0].rcode = 0x940;
                        frub[0].rmsg = RM_LM;
                        insert_fru = TRUE;
                        last_tu = TRUE;
                        add_fru();
                        err(0x246,0,errno);
                        exit_da();
                }

                NewPacing = OrigPacing;
                NewPacing.cxma_flags = OrigPacing.cxma_flags &
                     ~(RTSPACE|CTSPACE|DSRPACE|DCDPACE|DTRPACE|CXMA_FORCEDCD);

                if(ioctl(fdes, CXMA_SETA, &NewPacing)) {
                        init_frub();
                        frub[0].sn = Adptr_name;
                        frub[0].rcode = 0x941;
                        frub[0].rmsg = RM_LM;
                        insert_fru = TRUE;
                        last_tu = TRUE;
                        add_fru();
                        err(0x247,0,errno);
                        exit_da();
                }
                pacing_saved = 1;    /* Pacing changed & original info saved */
        }

        /* End of 128-port Pacing update */

        if (d_mode == PD || d_mode == REPAIR)
                select_tu ();
        if ((DA_CHECKRC_STATUS() == DA_STATUS_GOOD) &&
            (d_mode == PD || d_mode == ELA)) {
                err_log ();
        }  /* endif */
}  /* option_checkout end */

/*
 * NAME: system_checkout
 *
 * FUNCTION:  Sets global variables using data from structure da_input and
 *      the predefined diagnostic devices object class that are used by the
 *      DA to build menus and messages.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:  portnum == port number to be tested on NIO planar.
 *         prt_num == port number to be tested on adapter.
 *         nports  == total number of ports on adapter to be tested.
 *         devtty  == device driver file name.
 *
 * RETURNS: NONE
 */

system_checkout (num_pdcn)      /* begin system_checkout */
int     num_pdcn;
{
        int     k;
        int     i = 0;
        int     j = 0;
        int     rc;
        char    bufstr[32];     /* temporary buffer used by sprintf          */
        char    namebuf[NAMESIZE];
        void    unconfig_port();

        void    init_frub ();
        void    add_fru ();
        void    exit_da ();
        void    restore_attributes ();
        void    restore_stream_stack ();

        while (i < num_pdcn)
        {
                if (Adptr_name == EIA_232_128 || Adptr_name == EIA_232_128ISA)
                        sync_test2(num_pdcn);

                /* Save the device driver name */
                strcpy(dd_name, P_cudv->PdDvLn->DvDr);

                if ((c_mode == CONSOLE) &&
                    (strncmp (pdcn[i].connkey, pcfg, 2)))
                {
                        Menu_nmbr &= 0xFFF000;
                        Menu_nmbr += 0x000104;
                        dsply_cfg_hdr (Menu_nmbr);
                }  /* endif */
                sighold (SIGTERM);
                if ((strncmp (pdcn[i].connkey, pdef, 2)) &&
                    (strncmp (pdcn[i].connkey, pcfg, 2)))
                {
                        k = config_port (i);
                        sprintf (devtty, "/dev/%s", D_cudv[k].name);
                        strcpy (namebuf, D_cudv[k].name);
                        strcpy (bufstr, D_cudv[k].location);
                }  /* endif */
                else
                {
                        for (j = 0; j < num_CuDv; j++)
                                if (!strcmp (C_cudv[j].connwhere,
                                    pdcn[i].connwhere))
                                        break;
                        if (C_cudv[j].status != Available)
                                cfg_device (C_cudv[j].name);
                        sprintf (devtty, "/dev/%s", C_cudv[j].name);
                        strcpy (namebuf, C_cudv[j].name);
                        strcpy (bufstr, C_cudv[j].location);
                }  /* endelse */
                strcpy (lp_name, namebuf);
                if (e_mode != IPL && made_device != 1) {
                        dd_state = open_tty (namebuf);
                        switch(dd_state) {
                              case Console:
                                      dd_state = In_Use;
                                      break;
                              case In_Use:
                                      /* Device other than console cannot be */
                                      /* busy except in Concurrent mode */
                                      err_chk(0);  /* 0 - only check if in Standalone mode */
                                      break;
                              default:
                                      dd_state = Available;
                                      break;
                        }  /* endswitch */
                }  /* endif */
                else
                        dd_state = Available;
                if (dd_state == Available)
                {
                        switch (Adptr_name)
                        {
                        case SP1:
                        case SP2:
                        case SP3:
                                k = 0;
                                while (bufstr[k] != 'S' && k < 11)
                                        k++;
                                portnum[0] = bufstr[k];
                                portnum[1] = bufstr[k+1];
                                break;
                        case EIA_232_8:
                        case EIA_232_8ISA:
                        case M_S_188_8:
                        case EIA_422_8:
                                n_ports = num_pdcn;
                                break;
                        case EIA_232_16:
                        case EIA_422_16:
                                n_ports = num_pdcn + 6;
                                break;
                        case EIA_232_64:
                                n_ports = bufstr[7] - 0x30;
                                break;
                        case EIA_232_128:
                        case EIA_232_128ISA:
                                n_ports = 128;
                                break;
                        }  /* end switch (Adptr_name) */
                        if (Adptr_name != SP1 && Adptr_name != SP2 &&
                                Adptr_name != SP3)
                        {
                                if (bufstr[9] == '0')
                                        prt_num = bufstr[10] - 0x30;
                                else
                                        prt_num = bufstr[10] - 0x20;
                        }  /* endif */
                        if (c_mode == CONSOLE &&
                            (e_mode != SYSX || l_mode != EXITLM))
                        {
                                Menu_nmbr &= 0xFFF000;
                                Menu_nmbr += 0x000102;
                                dsply_tst_hdr (Menu_nmbr);
                        }  /* endif */
                        if ((fdes = open (devtty, OPEN_TTY)) == -1) {
                                err(0x155,0,errno);
                                err_chk (errno);
                        }  /* endif */


                        /*
                            Until the streams system gets setup so that it will
                            function without CLOCAL set, we will need to
                            continue to set it before calling exectu, then clear
                            it when exit_da() is called at the end.  Initially
                            save the current attributes into SaveAttributes and
                            add CLOCAL via WorkingAttrs.
                        */

                        if ((ioctl (fdes, TCGETA, &SaveAttrs)) != 0) {
                            init_frub ();
                            frub[0].sn = Adptr_name;
                            frub[0].rcode = 0x914;
                            frub[0].rmsg = RM_LM;
                            insert_fru = TRUE;
                            last_tu = TRUE;
                            add_fru ();
                            err(0x156,0,errno);
                            exit_da();
                        }
                        else
                            attrs_saved=1;      /* Attributes were saved */

                        if ((ioctl (fdes, TCGETA, &WorkingAttrs)) != 0) {
                            init_frub ();
                            frub[0].sn = Adptr_name;
                            frub[0].rcode = 0x915;
                            frub[0].rmsg = RM_LM;
                            insert_fru = TRUE;
                            last_tu = TRUE;
                            add_fru ();
                            err(0x157,0,errno);
                            exit_da();
                        }

                        WorkingAttrs.c_cflag |= CLOCAL;
                        if ((ioctl (fdes, TCSETA, &WorkingAttrs)) != 0) {
                            init_frub ();
                            frub[0].sn = Adptr_name;
                            frub[0].rcode = 0x916;
                            frub[0].rmsg = RM_LM;
                            insert_fru = TRUE;
                            last_tu = TRUE;
                            add_fru ();
                            err(0x158,0,errno);
                            exit_da();
                        }

                        /* End of the CLOCAL update */

                        slct_port = 1;
                        if (d_mode == PD || d_mode == REPAIR)
                                select_tu ();
                        slct_port = 0;
                        if ((DA_CHECKRC_STATUS() == DA_STATUS_GOOD) &&
                            (d_mode == PD || d_mode == ELA)) {
                                err_log ();
                        }  /* endif */
                        if (fdes != -1)
                        {
                                /* Restore modem control bits if needed. */
                                restore_attributes ();
                                /* Restore user stream stack if needed. */
                                restore_stream_stack ();
                                close (fdes);
                                fdes = -1;
                        }  /* endif */
                        unconfig_port ();
                }  /* endif */
                sigrelse (SIGTERM);
                if (diskette == TRUE &&
                   (Adptr_name == EIA_232_64 || Adptr_name == EIA_232_128 ||
		    Adptr_name == EIA_232_128ISA))
                        i += 16;
                else
                        i++;
        }  /* endwhile */
}  /* system_checkout end */

/*
 * NAME: config_port
 *
 * FUNCTIONS:  Search PdCn for all possible connwheres for the current CuDv.
 *      Search CuDv for children of the current CuDv.  Configure ports for
 *      the current CuDv.
 *
 * RETURNS:
 */

config_port (conn_where)
int     conn_where;
{
        char    buffer[128];
        char    define_args[512];
        int     odm_rc = 0;
        int     index;
        char    *obuf;
        void    cfg_device();
        void    init_frub ();
        void    add_fru ();
        void    exit_da ();

        /* Define the device only. cfg_device() will configure it */
        sprintf(define_args, " -c %s -s %s -t %s -p %s -w %s",
                pddv->class,                            /* class        */
                pddv->subclass,                         /* subclass     */
                pddv->type,                             /* type         */
                P_cudv->name,                           /* parent       */
                pdcn[conn_where].connwhere);            /* connwhere    */

        odm_rc = invoke_method (pddv->Define, define_args, &obuf);
        if (odm_rc == -1)
        {
                init_frub ();
                frub[0].sn = Adptr_name;
                frub[0].rcode = 0x917;
                frub[0].rmsg = RM_LM;
                insert_fru = TRUE;
                last_tu = TRUE;
                add_fru ();
                err(0x160,0,0);
                exit_da ();
        }  /* endif */

        /* global flag indicating that a device was defined by diagnostics */
        made_device = 1;

        /* get children of async adapter */
        sprintf (buffer, "parent = '%s'", da_input.dname);
        D_cudv = get_CuDv_list(CuDv_CLASS, buffer, &c_info ,num_PdCn ,2);
        if (D_cudv == (struct CuDv *) -1)
        {
                init_frub ();
                frub[0].sn = Adptr_name;
                frub[0].rcode = 0x918;
                frub[0].rmsg = RM_LM;
                insert_fru = TRUE;
                last_tu = TRUE;
                add_fru ();
                err(0x161,0,0);
                exit_da ();
        }  /* endif */

        /* find device just defined amongst children of async adapter */
        for (index = 0; index < c_info.num; index++)
                if (!strcmp(pdcn[conn_where].connwhere,
                    D_cudv[index].connwhere))
                {
                        /* configure devices including path */
                        cfg_device(D_cudv[index].name);
                        break;
                }  /* endif */
        return (index);
}  /* config_port end */

/*
 * NAME: cfg_device
 *
 * FUNCTION: This routine configures the path to the specified device.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: This routine fills out the global array "parents".
 *
 * RETURNS:
 */

void cfg_device( name )
char    *name;
{
        int             i, rc;
        char            args[255];
        struct CuDv     *p_cudv;
        char    *obuf;
        void    init_frub ();
        void    add_fru ();
        void    exit_da ();

        /* Build array "parents".  Identifies the devices that need   */
        /* to be configured.  Stop processing if a device in the path */
        /* is configured or does not have a config method.            */

        sprintf(args, "name = %s", name);
        for (i = 0; i < MAXLEVELS+1; i++)
        {
                p_cudv = get_CuDv_list(CuDv_CLASS, args, &p_info, 1, 2);
                if (p_cudv == (struct CuDv *) -1 || p_info.num == 0)
                {
                        init_frub ();
                        frub[0].sn = Adptr_name;
                        frub[0].rcode = 0x919;
                        frub[0].rmsg = RM_LM;
                        insert_fru = TRUE;
                        last_tu = TRUE;
                        add_fru ();
                        err(0x162,0,0);
                        exit_da ();
                }  /* endif */
                if ((p_cudv->status != Available) &&
                    strlen(p_cudv->PdDvLn->Configure))
                        strcpy (parents[i], p_cudv->name);
                else
                        break;
                sprintf (args, "name = %s", p_cudv->parent);
                odm_free_list(p_cudv, &p_info);
        }  /* endfor */
        for (i = MAXLEVELS; i >= 0; i--)
        {
                if (strlen(parents[i]))
                {
                        sprintf ( args, "name = %s", parents[i] );
                        p_cudv = get_CuDv_list(CuDv_CLASS,args, &p_info, 1, 2);
                        if (p_cudv == (struct CuDv *) -1 || p_info.num == 0)
                        {
                                init_frub ();
                                frub[0].sn = Adptr_name;
                                frub[0].rcode = 0x920;
                                frub[0].rmsg = RM_LM;
                                insert_fru = TRUE;
                                last_tu = TRUE;
                                add_fru ();
                                err(0x163,0,0);
                                exit_da ();
                        }  /* endif */
                        sprintf (args, " -l %s", parents[i]);
                        rc = invoke_method (p_cudv->PdDvLn->Configure, args, &obuf);
                        if (rc != 0) {
                                err(0x164,rc,0);
                                init_frub ();
                                frub[0].sn = Adptr_name;
                                frub[0].rcode = 0x921;
                                frub[0].rmsg = RM_LM;
                                insert_fru = TRUE;
                                last_tu = TRUE;
                                add_fru ();
                                DA_SETRC_TESTS (DA_TEST_FULL);
                                exit_da ();
                        }  /* endif */

                        if (made_device != 1)
                                made_device = 2;

                }  /* endif */
        }  /* endfor */
}  /* cfg_device end */

/*
 * NAME: unconfig_port
 *
 * FUNCTIONS:  Search the customized data base for children of the current
 *      P_cudv.  Unconfigure all ports that were configured by diagnostics.
 *
 * RETURNS:  odm_rc
 */

void unconfig_port ()
{
        int     index, rc;
        void    ucfg_device();
        char    args[255];
        struct  CuDv *cudv;
        char    *obuf;
        void    init_frub ();
        void    add_fru ();
        void    exit_da ();

        sprintf (args, " -l %s", parents[0]);
        ucfg_device ();
        if (made_device == 1)
        {
                rc = invoke_method (pddv->Undefine, args, &obuf);
                if (rc != 0 )
                {
                        init_frub ();
                        frub[0].sn = Adptr_name;
                        frub[0].rcode = 0x922;
                        frub[0].rmsg = RM_LM;
                        insert_fru = TRUE;
                        last_tu = TRUE;
                        add_fru ();
                        err(0x165,rc,0);
                        made_device = 0;
                        exit_da ();
                }
        }  /* endif */
        made_device = 0;
        return;
}  /* unconfig_port end */

/*
 * NAME: ucfg_device
 *
 * FUNCTION:  Unconfigures the devices listed in the parents array.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  NONE
 */

void ucfg_device ()
{
        int     i, rc;
        char    args[255];
        struct  CuDv    *p_cudv;
        char    *obuf;
        void    init_frub ();
        void    add_fru ();
        void    exit_da ();

        /* Unconfigure parents */
        for (i = 0; i < MAXLEVELS; i++)
        {
                if (strlen(parents[i]))
                {
                        sprintf (args, "name = %s", parents[i]);
                        p_cudv = get_CuDv_list(CuDv_CLASS, args, &p_info,
                            1, 2);
                        if (p_cudv == (struct CuDv *) -1)
                        {
                                init_frub ();
                                frub[0].sn = Adptr_name;
                                frub[0].rcode = 0x923;
                                frub[0].rmsg = RM_LM;
                                insert_fru = TRUE;
                                last_tu = TRUE;
                                add_fru ();
                                err(0x166,0,0);
                                exit_da ();
                                return;
                        }  /* endif */
                        sprintf (args, " -l %s", parents[i]);
                        rc = invoke_method(p_cudv->PdDvLn->Unconfigure, args, &obuf);
                        if (rc != 0)
                        {
                                init_frub ();
                                frub[0].sn = Adptr_name;
                                frub[0].rcode = 0x924;
                                frub[0].rmsg = RM_LM;
                                insert_fru = TRUE;
                                last_tu = TRUE;
                                add_fru ();
                                err(0x167,rc,0);
                                exit_da ();
                                return;
                        }  /* endif */
                        parents[i][0] = Null;
                }  /* endif */
        }  /* endfor */
}  /* ucfg_device end */

/*
 * NAME: open_tty
 *
 * FUNCTION:  Returns tty device driver status.  If the port selected by the
 *      user is the system console or is in the enabled state tty_state will
 *      be set to zero and the user will be prompted to free additional re-.
 *      source to continue testing.  If the odm_run_method fails tty_state
 *      will be set to zero.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:  tty_state
 */

open_tty (ddname)
char    *ddname;
{
        char    *tty_outbuf, *lsc_outbuf, *pstart_buf, *ptr;
        int     i, j;
        int     tty_state = 1;
        int     odm_rc = 0;
        char    buffer[128];
        char    dvname[128];
        struct  CuAt *cuat;
        struct  listinfo p_info;

        strcpy(buffer, "attribute = syscons");
        cuat = get_CuAt_list(CuAt_CLASS, buffer, &p_info, 1, 2);
        if(cuat == (struct CuAt *) -1) {
                init_frub ();
                frub[0].sn = Adptr_name;
                frub[0].rcode = 0x939;
                frub[0].rmsg = RM_LM;
                insert_fru = TRUE;
                last_tu = TRUE;
                add_fru ();
                err(0x243,0,0);
                exit_da();
        }  /* endif */
        sprintf(dvname,"/dev/%s", ddname);
        if (!strcmp(cuat->value ,dvname)) {
                err(0x244,0,0);
                return (tty_state = Console);
        } /* endif */
        odm_free_list(cuat, &p_info);
        if (!file_present(TTY)) {
                err(0x168,0,0);
                return (tty_state = In_Use);
        }
        if (odm_run_method(TTY, "", &tty_outbuf, NULL) == -1) {
                err(0x169,0,0);
                return (tty_state = In_Use);
        }
        ptr = (char *)strtok (tty_outbuf, " /\n");
        while (ptr != NULL)
        {
                if (!strcmp(ptr, ddname)) {
                        err(0x170,0,0);
                        return (tty_state = In_Use);
                }
                ptr = (char *)strtok (NULL , " /\n");
        }  /* endwhile */
        free (tty_outbuf);
        if (!file_present(PSTART)) {
                err(0x174,0,0);
                return (tty_state = Available);
        }
        if (odm_run_method(PSTART, "", &pstart_buf, NULL) == -1) {
                err(0x175,0,0);
                return (tty_state = In_Use);
        }
        ptr = (char *)strtok (pstart_buf, " \n");
        while (ptr != NULL)
        {
                if (!strcmp(ptr, ddname))
                {
                        ptr = (char *)strtok (NULL , " /\n");
                        if ((!strcmp (ptr, "ENABLE")) &&
                            (e_mode == CONC)) {
                                err(0x176,0,0);
                                return (tty_state = In_Use);
                        }
                        return (tty_state);
                }  /* endif */
                ptr = (char *)strtok (NULL , " /\n");
        }  /* endwhile */
        free (pstart_buf);
        return (tty_state);
}  /* open_tty end */

/*
 * NAME: invoke_method
 *
 * FUNCTION:  Invoke the specifed method and retry in a ODM LOCK occurs.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  method return code
 */

invoke_method (method, args, optr)
char    *method;
char    *args;
char    **optr;
{
        int     rc;
        int     count = 0;

        do {
                rc = odm_run_method (method, args, optr, NULL);
                if (rc == E_ODMLOCK)
                        sleep(1);
        } while (count++ != 300 && rc == E_ODMLOCK);

        return (rc);
}  /* invoke_method end */

/*
 * NAME: sync_test
 *
 * FUNCTION:  Test the sync line on 128-port adapters
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: Valid only for the 128-port adapter in option checkout mode.
 *
 * RETURNS:
 */

sync_test (num_pdcn)        /* begin sync_test */
int     num_pdcn;
{
   int     i, j, k;
   int     conf_rc = 0;
   rw_t    rw;
   char    bufstr[32];     /* temporary buffer used by sprintf          */
   char    namebuf[NAMESIZE];
   void    cfg_device ();
   void    rscmenu1(long,int);
   void    rscmenu(long);
   int     msg_id;
   nl_catd my_catd = CATD_ERR;
   int     prompted=FALSE;
   int     old_conf_rc=0;
   int     problem=FALSE;

   void    init_frub ();
   void    add_fru ();
   void    exit_da ();

   wrp_plg = wp_10;  /* 15-pin terminator */

       /*
        * If the configuration of either the port or it's
        * parent fails, we still want to do a sanity check
        * on the sync line and the concentrator's functionality.
        * 'chk_config' contains the device level methods of
        * performing these checks.
        */

       /* Add a sleep routine to allow adapter to run its sync line */
       /* termination test and set the bit in the register before */
       /* proceeding with the test, else a line terminated just prior */
       /* to running this test may fail the sync line test. */

       if (c_mode == CONSOLE) {
          Menu_nmbr &= 0xFFF000;
          Menu_nmbr += 0x000102;
          dsply_tst_hdr(Menu_nmbr);
       }
       sleep(15);


       if (!strncmp(P_cudv->name, "sa", 2)) {
           sprintf(devtty, "/dev/%s", P_cudv->parent);
           strcpy(namebuf, P_cudv->parent);
       } else {
           sprintf(devtty, "/dev/%s", P_cudv->name);
           strcpy(namebuf, P_cudv->name);
       }

       strcpy(bufstr, P_cudv->location);
       strcpy(lp_name, namebuf);
       cfg_device(namebuf);

       sync_err=0;  /* Force sync_err to 0 before running chk_config */
       putdavar(da_input.dname, "syncfail", DIAG_INT, &sync_err);
       conf_rc = chk_config(devtty);

       if (conf_rc != 0) {
           switch (conf_rc) {
            case -1: /* Adapter error */
               err(0x177,0,0);
               frub[0].sn = Adptr_name;
               frub[0].rcode = 0x0111;
               frub[0].frus[0].conf = conf2;
               frub[0].frus[1].conf = conf3;
               frub[0].frus[0].fru_flag = DA_NAME;
               frub[0].frus[1].fru_flag = PARENT_NAME;
               insert_fru = TRUE;
               last_tu = TRUE;
               add_fru ();
               DA_SETRC_STATUS(DA_STATUS_BAD);
               DA_SETRC_TESTS (DA_TEST_FULL);
               exit_da ();
               break;
            case -2: /* Line 1 error */
            case -4: /* Line 2 error */
               err(0x260,conf_rc,0);
	       cxma_line_err = TRUE;
               getdavar(da_input.dname, "syncfail", DIAG_INT, &sync_err);
               my_catd = diag_catopen (MF_DASYNC, 0);
               while(sync_err == 1) {
                  if(conf_rc == -2)
                     prt_num=1;
                  else
                     prt_num=2;

                  if(conf_rc != old_conf_rc) {
                     if(old_conf_rc == -4)
                        break;  /* Both line 1 and line 2 have passed OK */
                     else
                        old_conf_rc = conf_rc;
                  }

                  /* No way to prompt user to install terminator w/o console */
                  /* so tell user that there was an error */
                  if((c_mode != CONSOLE) || (a_mode != ADVANCED) ||
		     (l_mode == INLM) || (l_mode == EXITLM) ) {
		     err(0x261,0,0);
                     catclose(my_catd);
                     init_frub ();
                     frub[0].sn = Adptr_name;
                     frub[0].rcode = 0x119;
                     frub[0].rmsg = RM_LM;
                     frub[0].frus[0].conf = conf1;
                     frub[0].frus[0].fmsg = RM_ICPN;
                     if (cxma_adapter == FALSE ) {
                        strcpy(da_input.dname, da_input.parent);
                        strcpy(da_input.dnameloc, da_input.parentloc);
                     } /* endif */
                     strcpy(frub[0].frus[0].fname, da_input.dname);
                     strcpy(frub[0].frus[0].floc, da_input.dnameloc);
                     frub[0].frus[0].fru_flag = DA_NAME;
                     frub[0].frus[0].fru_exempt = EXEMPT;
                     insert_fru = TRUE;
                     last_tu = TRUE;
                     add_fru ();
                     DA_SETRC_TESTS (DA_TEST_FULL);
                     exit_da();
                  }

                  Menu_nmbr &= 0xFFF000;
                  Menu_nmbr += 0x000111;
                  msg_id = DM_87;
                  rscmenu2(Menu_nmbr, msg_id);

                  if((slctn == 2) &&
		    ((cxma_adapter == TRUE) || (l_mode == ENTERLM))) {
                     if(prompted == FALSE) {
                        prompted = TRUE;
                        Menu_nmbr &= 0xFFF000;
                        Menu_nmbr += 0x000112;
                        dsply_tst_lst(Menu_nmbr);
                     }
                     else
                        slctn = 1;   /* user definitely has terminator */

                     if(slctn == 1) {
                        Menu_nmbr &= 0xFFF000;
                        Menu_nmbr += 0x000113;
			if(cxma_adapter == TRUE)
                           msg_id = DM_26;
                        else
			   msg_id = DM_91;
                        wrap_plug_install(Menu_nmbr, msg_id);

                        Menu_nmbr &= 0xFFF000;
                        Menu_nmbr += 0x000114;
                        dsply_tst_hdr(Menu_nmbr);
                        sleep(3);

                        conf_rc = chk_config(devtty);

                        if((sync_err == 1) && (old_conf_rc == conf_rc))
                           problem = TRUE;
                        else
                           problem = FALSE;
                     }
                     else {   /* Can't do any tests w/o terminator */
			err(0x262,0,0);
                        DA_SETRC_STATUS(DA_STATUS_GOOD);
                        Menu_nmbr &= 0xFFF000;
                        Menu_nmbr &= 0x000111;
                        must_have_terminator(Menu_nmbr);
                        exit_da();
                     } /* endelse */
                  }
                  else
                     problem = TRUE;

                  if(problem == TRUE) {
		     err(0x263,0,0);
                     while(sync_err == 1) {
                        if((cxma_adapter == FALSE) && (l_mode == NOTLM)) {
                           Menu_nmbr &= 0xFFF000;
                           Menu_nmbr += 0x000114;
                           msg_id=DM_23;
                           rscmenu1(Menu_nmbr, msg_id);

                           chk_config(devtty);
                        }

                        if(sync_err == 1) {
                           if((cxma_adapter == FALSE) && (l_mode == NOTLM)) {
                              Menu_nmbr &= 0xFFF000;
                              Menu_nmbr += 0x000115;
                              msg_id = DM_17;
                              rscmenu2(Menu_nmbr, msg_id);
                           }
                           else
                              slctn = 1;   /* Force adapter error */

                           if (slctn == 1){  /* We're done */
			       err(0x264,0,cxma_adapter);
                               catclose(my_catd);
                               init_frub ();
                               frub[0].sn = Adptr_name;
                               frub[0].rcode = 0x114;
                               frub[0].rmsg = RM_LM;
                               frub[0].frus[0].conf = conf1;
                               frub[0].frus[0].fmsg = RM_ICPN;
                               if (cxma_adapter == FALSE ) {
                                  strcpy(da_input.dname, da_input.parent);
                                  strcpy(da_input.dnameloc, da_input.parentloc);
                               } /* endif */
                               strcpy(frub[0].frus[0].fname, da_input.dname);
                               strcpy(frub[0].frus[0].floc, da_input.dnameloc);
                               frub[0].frus[0].fru_flag = DA_NAME;
                               frub[0].frus[0].fru_exempt = EXEMPT;
                               insert_fru = TRUE;
                               last_tu = TRUE;
                               add_fru ();
                               DA_SETRC_TESTS (DA_TEST_FULL);
                               exit_da();
                               break;
                           } /* if slctn */
                        } /* if sync_err */

			/* Don't call out an error if in LOOP mode */
                        if((sync_err==0) && (l_mode == NOTLM)) {
                           err(0x179,0,0);
                           Menu_nmbr &= 0xFFF000;
                           Menu_nmbr += 0x000116;
                           msg_id=DM_24;
                           rscmenu1(Menu_nmbr, msg_id);

                           catclose(my_catd);
                           init_frub ();
                           frub[0].sn = Adptr_name;
                           frub[0].rcode = 0x116;
                           frub[0].rmsg = 18;
                           frub[0].frus[0].conf = conf8;
                           frub[0].frus[1].conf = conf9;
                           frub[0].frus[0].fmsg = 6;
                           strcpy(frub[0].frus[0].fname, "Controller Line");
                           strcpy(frub[0].frus[1].fname, da_input.dname);
                           strcpy(frub[0].frus[0].floc, da_input.dnameloc);
                           frub[0].frus[0].fru_flag = NOT_IN_DB;
                           frub[0].frus[1].fru_flag = DA_NAME;
                           frub[0].frus[0].fru_exempt = EXEMPT;
                           insert_fru = TRUE;
                           last_tu = TRUE;
                           add_fru ();
                           exit_da();
                        } /* if sync_err ==0 */
                     } /* while sync_err */
                  } /* if problem */
		  else
		     cxma_line_err=FALSE;  /*Make sure of reset if no problem*/
               } /* while sync_err */
               break;
            case -3: /* concentrator error */
               err(0x180,0,0);
               my_catd = diag_catopen (MF_DASYNC, 0);
               Menu_nmbr &= 0xFFF000;
               Menu_nmbr += 0x000118;
               msg_id=DM_25;
               rscmenu1(Menu_nmbr, msg_id);

               init_frub ();
               frub[0].sn = Adptr_name;
               frub[0].rcode = 0x118;
               frub[0].rmsg = RM_CC9;
               frub[0].frus[0].conf = conf1;
               frub[0].frus[0].fmsg = RM_ICPN;
               strcpy(frub[0].frus[0].fname, da_input.dname);
               strcpy(frub[0].frus[0].floc, da_input.dnameloc);
               frub[0].frus[0].fru_flag = DA_NAME;
               frub[0].frus[0].fru_exempt = EXEMPT;
               insert_fru = TRUE;
               last_tu = TRUE;
               add_fru ();

               catclose(my_catd);
               exit_da();
               break;
            default:
               err(0x181,0,0);
               frub[0].sn = Adptr_name;
               frub[0].rcode = 0x0111;
               frub[0].frus[0].conf = conf2;
               frub[0].frus[1].conf = conf3;
               frub[0].frus[0].fru_flag = DA_NAME;
               frub[0].frus[1].fru_flag = PARENT_NAME;
               insert_fru = TRUE;
               last_tu = TRUE;
               DA_SETRC_ERROR (DA_ERROR_OTHER);
               DA_SETRC_TESTS (DA_TEST_FULL);
               exit_da ();
               break;
           }

           if(fru_found == TRUE) {
              frub[0].frus[0].fru_exempt = EXEMPT;
              insert_fru = TRUE;
              last_tu = TRUE;
              add_fru();
              exit_da();
           }
       }
}  /* sync_test end */

/*
 * NAME: sync_test2
 *
 * FUNCTION:  Test the sync line on 128-port adapters
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:  Valid only for 128-port adapters in system checkout mode.
 *
 * RETURNS: NONE
 */
sync_test2 (num_pdcn)      /* begin sync_test2 */
int     num_pdcn;
{
        int     k;
        int     j = 0;
        int     rc;
        int     conf_rc = 0;
        nl_catd my_catd = CATD_ERR;
        char    bufstr[32];     /* temporary buffer used by sprintf          */
        char    namebuf[NAMESIZE];
        void    unconfig_port();
        void    cfg_device();
        int     msg_id;


        for(prt_num=0; prt_num < num_pdcn; prt_num++)
        {
	    /* Don't test 'cxma' adapter.  Only test 'sa' RANs */
            if (!strncmp(P_cudv->name, "sa", 2))
            {
               /* Add a sleep routine to allow adapter to run its sync line */
               /* termination test and set the bit in the register before */
               /* proceeding with the test, else a line terminated just prior */
               /* to running this test may fail the sync line test. */

                if (c_mode == CONSOLE) {
                   Menu_nmbr &= 0xFFF000;
                   Menu_nmbr += 0x000102;
                   dsply_tst_hdr(Menu_nmbr);
                }
                sleep(15);



                /*
                 * If the configuration of either the port or it's
                 * parent fails, we still want to do a sanity check
                 * on the sync line and the concentrator's functionality.
                 * 'chk_config' contains the device level methods of
                 * performing these checks.
                 */

                sprintf(devtty, "/dev/%s", P_cudv->parent);
                strcpy(namebuf, P_cudv->parent);
                strcpy(bufstr, P_cudv->location);
                strcpy(lp_name, namebuf);
                cfg_device(namebuf);
                if (c_mode == CONSOLE) {
                        Menu_nmbr &= 0xFFF000;
                        Menu_nmbr += 0x000102;
                        dsply_tst_hdr (Menu_nmbr);
                }  /* endif */

                sync_err=0;  /* Force sync_err to 0 before running chk_config */
                putdavar(da_input.dname, "syncfail", DIAG_INT, &sync_err);
                conf_rc = chk_config(devtty);

                if (conf_rc != 0) {
                    switch (conf_rc) {
                    case -1: /* Adapter error */
                        err(0x182,0,0);
                        frub[0].sn = Adptr_name;
                        frub[0].rcode = 0x0111;
                        frub[0].frus[0].conf = conf2;
                        frub[0].frus[1].conf = conf3;
                        frub[0].frus[0].fru_flag = DA_NAME;
                        frub[0].frus[1].fru_flag = PARENT_NAME;
                        insert_fru = TRUE;
                        last_tu = TRUE;
                        add_fru ();
                        DA_SETRC_STATUS (DA_STATUS_BAD);
                        DA_SETRC_TESTS (DA_TEST_FULL);
                        exit_da ();
                        break;
                     case -2: /* Line 1 error */
                     case -4: /* Line 2 error */
                        err(0x183,0,0);
                        catclose(my_catd);
                        init_frub ();
                        frub[0].sn = Adptr_name;
                        frub[0].rcode = 0x119;
                        frub[0].rmsg = RM_LM;
                        frub[0].frus[0].conf = conf1;
                        frub[0].frus[0].fmsg = RM_ICPN;
                        strcpy(frub[0].frus[0].fname, da_input.parent);
                        strcpy(frub[0].frus[0].floc, da_input.parentloc);
                        frub[0].frus[0].fru_flag = DA_NAME;
                        frub[0].frus[0].fru_exempt = EXEMPT;
                        insert_fru = TRUE;
                        last_tu = TRUE;
                        add_fru ();
                        DA_SETRC_TESTS (DA_TEST_FULL);
                        exit_da();
                        break;
                     case -3: /* concentrator error */
                        err(0x184,0,0);
                        my_catd = diag_catopen (MF_DASYNC, 0);
                        sprintf(menu_msg, (char *) diag_cat_gets(my_catd,
                            ASYNC_DIAG, DM_25, NULL));
                        menugoal(menu_msg);
                        catclose(my_catd);
                        DA_SETRC_STATUS(DA_STATUS_BAD);
                        DA_SETRC_ERROR(DA_ERROR_NONE);
                        DA_SETRC_MORE(DA_MORE_NOCONT);
                        DA_SETRC_USER(DA_USER_NOKEY);
                        DA_SETRC_TESTS(DA_TEST_FULL);
                        exit_da();
                        break;
                     default:
                        err(0x185,0,0);
                        frub[0].sn = Adptr_name;
                        frub[0].rcode = 0x0111;
                        frub[0].frus[0].conf = conf2;
                        frub[0].frus[1].conf = conf3;
                        frub[0].frus[0].fru_flag = DA_NAME;
                        frub[0].frus[1].fru_flag = PARENT_NAME;
                        insert_fru = TRUE;
                        last_tu = TRUE;
                        DA_SETRC_ERROR (DA_ERROR_OTHER);
                        DA_SETRC_TESTS (DA_TEST_FULL);
                        exit_da ();
                        break;
                    }
                    frub[0].frus[0].fru_exempt = EXEMPT;
                    insert_fru = TRUE;
                    last_tu = TRUE;
                    add_fru();
                    exit_da();
                } /* end if(conf_rc) */
            } /* end if(!strncmp) */
        } /* end for(prt_num) */
        if(da_input.dmode!=DMODE_MS1 )
            exit_da();
}  /* sync_test2 end */

/*
 * NAME: chk_config
 *
 * FUNCTION: In-depth check of the 128-port board and sync line.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:  Valid only for 128-port adapters
 *
 * RETURNS:  0 = No errors found
 *          -1 = Error opening device, executing an ioctl(), or comparing vpd
 *          -2 = Error executing putdavar()
 *          -3 = Error validating KME info
 */

chk_config (devtty)
char *devtty;
{
        int cxfdes;
        int busfd;
        int i;
        char *c;
        int brdnum;
        int bus;
        int slot;
        int line;
        int conc = 0;
        int id;
        int pos0;
        int j;
        char p[2];
        u_short vpd_size;
        u_short vpd_crc;
        u_short cvpd_crc;
        u_short num_reads;
        u_short line0, line1, status0, status1;
        char device[20];
        char buf[8+256];
        int bufsize = sizeof(buf);
        struct mdio md;
        struct stat sbuf;
        rw_t rw;
        char addr;
        extern crc_gen();

        if ((cxfdes = open(devtty, O_RDONLY)) == -1) {
            err(0x241,0,errno);
            return(-1);
        }

        if (fstat (cxfdes, &sbuf) == -1) {
            err(0x103,0,errno);
            return (-1);
        }

	conc = (sbuf.st_rdev >> 4) & 0x07;
	brdnum = *(devtty + strlen(devtty) - 1) - 0x30;

        sprintf(device, "/dev/bus%c", P_cudv->location[3]);
        bus = (P_cudv->location[3]-0x30);

	if (Adptr_name == EIA_232_128)
	{
        	slot = (P_cudv->location[4]-0x30);
        	slot--;
        }
	else
        	slot = 0;

        if (strlen(P_cudv->location) > 5) {
                line = P_cudv->location[6] - 0x30;
                if(line==1)
                        line=0;
                else
                        line=1;
                conc = P_cudv->location[7] - 0x30;
        }


        /* do host */
        if ((busfd = open(device ,O_RDWR)) == -1) {
            err(0x186,0,errno);
            close(cxfdes);
            return (-1);
        }

	/* 
	 * Get VPD and crc from the Host 128-port controller (MCA)
	*/
	if (Adptr_name == EIA_232_128) {
		for (i = 0; i < 2; i++) {
			md.md_addr = POSREG(i, slot);
			md.md_size = 1;
			md.md_incr = MV_BYTE;
			md.md_data = &p[i];
			if (ioctl(busfd, MIOCCGET, &md) == -1) {
				err(0x187,0,errno);
				close(busfd);
				close(cxfdes);
				return (-1);
			} /*endif*/
		} /*endfor*/
		
		id = (p[1] << 8) + p[0];
		
		/* 128-port adapter pos id */
		if (id == 0xffe1) {
			pos0 = POSREG(0, slot);
			/* 
			 * get header (CAVEAT: adapter has an Intel mind-set)
			 */
			for (j = 1; j < 8; j++) {
				addr = j;
				md.md_data = &addr;
				md.md_addr = POSREG(pos0+6, slot);
				md.md_size = 1;
				md.md_incr = MV_BYTE;
				if (ioctl(busfd, MIOCCPUT, &md) < 0) {
					err(0x188,0,errno);
					close(busfd);
					close(cxfdes);
					return(-1);
				}
				
				addr = 0;
				md.md_data = &addr;
				md.md_addr = POSREG(pos0+7, slot);
				md.md_size = 1;
				md.md_incr = MV_BYTE;
				if (ioctl(busfd, MIOCCPUT, &md) < 0) {
					err(0x189,0,errno);
					close(busfd);
					close(cxfdes);
					return(-1);
				}
				md.md_addr = POSREG(pos0+3, slot);
				md.md_size = 1;
				md.md_incr = MV_BYTE;
				md.md_data = (char *) &addr;
				if (ioctl(busfd, MIOCCGET, &md) < 0) {
					err(0x190,0,errno);
					close(busfd);
					close(cxfdes);
					return(-1);
				}
				buf[j] = addr;
			} /* End of Header loop */

			/* get data */
			vpd_size = (u_short) ((buf[4] << 8) | buf[5]);
			vpd_crc = (u_short) ((buf[6] << 8) | buf[7]);
			num_reads = vpd_size * 2;
			vpd_size *= 2;
			if (vpd_size+8 > bufsize) {
				err(0x191,0,0);
				close(busfd);
				close(cxfdes);
				return(-1);
			}
			for (j = 0; j < num_reads; j++) {
				addr = (j+8) & 0xff;
				md.md_data = &addr;
				md.md_addr = POSREG(pos0+6, slot);
				md.md_size = 1;
				md.md_incr = MV_BYTE;
				if (ioctl(busfd, MIOCCPUT, &md) < 0) {
					err(0x192,0,errno);
					close(busfd);
					close(cxfdes);
					return(-1);
				}
				addr = ((j+8) >> 8) & 0xff;
				md.md_data = &addr;
				md.md_addr = POSREG(pos0+7, slot);
				md.md_size = 1;
				md.md_incr = MV_BYTE;
				if (ioctl(busfd, MIOCCPUT, &md) < 0) {
					err(0x193,0,errno);
					close(busfd);
					close(cxfdes);
					return(-1);
				}
				md.md_addr = POSREG(pos0+3, slot);
				md.md_size = 1;
				md.md_incr = MV_BYTE;
				md.md_data = (char *) &addr;
				if (ioctl(busfd, MIOCCGET, &md) < 0) {
					err(0x194,0,errno);
					close(busfd);
					close(cxfdes);
					return(-1);
				}
				buf[j+8] = addr;
			}
			cvpd_crc = crc_gen(&buf[8], num_reads);
			if (vpd_crc != cvpd_crc) {
				err(0x195,vpd_crc,cvpd_crc);
				close(busfd);
				close(cxfdes);
				return(-1);
			}
		} else {
			err(0x196,0,0);
			close(busfd);
			close(cxfdes);
			return(-1);
		}
	} /* endif 128-port controller (MCA) */

        /* get the pointers to the fep's per line private data structures
         * line1 at 0x0D28, line2 at 0x0D2A. */
        rw.rw_board = (bus<<4)|((sbuf.st_rdev >> 8) & 0x0f);
        rw.rw_conc = 0;
        rw.rw_req = RW_READ;
        rw.rw_size = 128;
        rw.rw_addr = 0x0d28;
        if (ioctl(cxfdes, CXMA_KME, &rw) != 0) {
		err (0x197,0,errno);
		close (busfd);
		close (cxfdes);
		return (-1);
        }
        line1 = (rw.rw_data[1] << 8) | rw.rw_data[0];
        line0 = (rw.rw_data[3] << 8) | rw.rw_data[2];
        getdavar (da_input.dname, "syncfail", DIAG_INT, &sync_err);
        if (sync_err == 1) {
                Menu_nmbr &= 0xFFF000;
                Menu_nmbr += 0x000102;
                dsply_tst_hdr (Menu_nmbr);
                sleep (15);  /* Delay for disconnect on Concentrator */
        }
        rw.rw_addr = line0;
        if (ioctl(cxfdes, CXMA_KME, &rw) != 0) {
                err (0x198,0,errno);
                close (busfd);
                close (cxfdes);
                return (-1);
        }
        status1 = (rw.rw_data[1] << 8) | rw.rw_data[0];
        rw.rw_addr = line1;
        if (ioctl (cxfdes, CXMA_KME, &rw) != 0) {
                err (0x199,0,errno);
                close (busfd);
                close (cxfdes);
                return (-1);
        }
        status0 = (rw.rw_data[1] << 8) | rw.rw_data[0];
        if ((conc == 0) && ((status0 & 0x01) || (status1 & 0x01))) {
		sync_err = 1;
		putdavar (da_input.dname, "syncfail", DIAG_INT, &sync_err);
		close (busfd);
		close (cxfdes);
		if (status0 & 0x01)
			return (-2);
		else
			return (-4);
        }
        if (line == 1)
		status0 = status1;
        if (status0 & 0x01)
        {
		sync_err = 1;
		putdavar (da_input.dname, "syncfail", DIAG_INT, &sync_err);
		err (0x200,0,0);
		close (busfd);
		close (cxfdes);
		if (line == 1)
			return (-4);
		else
			return (-2);
        }
        getdavar(da_input.dname, "syncfail", DIAG_INT, &sync_err);
        if (sync_err) {
		sync_err = 0;
		putdavar (da_input.dname, "syncfail", DIAG_INT, &sync_err);
		err (0x201,0,0);
		close (busfd);
		close (cxfdes);
		if (line == 1)
			return (-4);
		else
			return (-2);
	}
	if (conc > 0) {
		rw.rw_addr = 0x0e10;
		if (ioctl(cxfdes, CXMA_KME, &rw) != 0) {
			err (0x202,0,errno);
			close (busfd);
			close (cxfdes);
			return (-1);
		}
		if ((rw.rw_data[(line * 8) + conc - 1] & 0x07) != 0x7) {
			err (0x203,0,0);
			close (busfd);
			close (cxfdes);
			return (-3);
		}
	}
	close (busfd);
	close (cxfdes);
	return (0);
}  /* chk_config end */
