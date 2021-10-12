static char sccsid[] = "@(#)68	1.2  src/bos/diag/da/370pca/d370pc.c, da370pca, bos412, 9445Xdiag 11/2/94 15:29:16";
/*
 * COMPONENT_NAME: DA370PCA
 *
 * FUNCTIONS:   main ()
 *              int_handler ()
 *              setdamode ()
 *              err_log ()
 *              exit_da ()
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
#include        "d370pc.h"

main ()  /* begin main */
{
	int     mode, i;
	int     odm_flg = 0;
	int	da_flg = 0;
	int     aslrc = TU_GOOD;
	int     rc = TU_GOOD;
	int     exitrc = SW_ERR;
	char    umc_msg[512], dmc_msg[512];

	setlocale (LC_ALL, "");
	/* set up interrupt handler routine     */
	act.sa_handler = int_handler;
	sigaction (SIGINT, &act, (struct sigaction *)NULL);

	/* initialize exit status variables */
	DA_SETRC_STATUS (DA_STATUS_GOOD);
	DA_SETRC_USER (DA_USER_NOKEY);
	DA_SETRC_ERROR (DA_ERROR_NONE);
	DA_SETRC_TESTS (DA_TEST_NOTEST);
	DA_SETRC_MORE (DA_MORE_NOCONT);

	/* initialize database and get tm_input */
	odm_flg = init_dgodm ();
	da_flg = getdainput (&da_input);
	if (odm_flg == FAIL || da_flg != ZERO)
	        exit_da (exitrc);

	/* determine ipl mode */
	mode = ipl_mode (&diskette);

	/* set testmode variables */
	l_mode = da_input.loopmode;     /* NT_LP, EN_LP, IN_LP, EX_LP */
	c_mode = da_input.console;      /* CNT, CNF */
	a_mode = da_input.advanced;     /* ADT, ADF */
	s_mode = da_input.system;       /* SYT, SYF */
	e_mode = da_input.exenv;        /* IPL, STD, CNC, EXR */
	d_mode = da_input.dmode;        /* ELA, PD, RPR, MS1, MS2 */

	/* display standby screen if console TRUE */
	if (c_mode == CNT)
	{
		if (l_mode == NT_LP)
	        	aslrc = diag_asl_init ("NO_TYPE_AHEAD");
		else
	        	aslrc = diag_asl_init (ASL_INIT_DEFAULT);
		if (aslrc != DIAG_ASL_OK)
	                exit_da (exitrc);
	        catd = diag_catopen(MF_D370PC, 0);
	        Menu_nmbr = 0x862101;
	        Msg_nmbr = DM370PC_1;
	        if (a_mode == ADT)
	        {
	                if (l_mode == NT_LP || l_mode == EN_LP)
	                {
	                        Menu_nmbr = 0x862102;
	                        Msg_nmbr = DM370PC_2;
	                }  /* endif */
	                else
	                {
	                        Menu_nmbr = 0x862103;
	                        Msg_nmbr = DM370PC_3;
	                }  /* endif */
	        }  /* endif */
	        aslrc = diag_stdby (Menu_nmbr, Msg_nmbr);
	        if ((exitrc = diag_asl_stat (aslrc)) != TU_GOOD)
	                exit_da (exitrc);
	}  /* endif */
	if ((rc = setdamode (&testmode, &maxtest)) != ZERO)
		exit_da (exitrc);

	/* Check for the functional microcode file in the /etc/microcode
	   directory if diagnostics is run from the hard file.  */
	if ((l_mode == NT_LP || l_mode == EN_LP) && diskette == FALSE)
	{
	        rc = findmcode (func_ucode, func_ucode_path, VERSIONING, "");
	        if (rc != 1)
	        {
	                if (c_mode == CNT)
	                {
	                        sprintf (umc_msg, (char *)diag_cat_gets(catd,
	                                        D370PCA_DA, DM370_14));
	                	menugoal(umc_msg);
	                } /* endif */
	        } /* endif */
		else
		{
	        	strcpy (tucb_ptr.pr.funccode, func_ucode_path);
		} /* endelse */
	} /* endif */

	/* Check for the diagnostic microcode file in the /etc/microcode
	   directory, if not found display message.  */
	rc = findmcode (diag_ucode, diag_ucode_path, VERSIONING, "");
	if (rc != 1)
	{
		if (c_mode == CNT)
		{
			sprintf (dmc_msg, (char *)diag_cat_gets(catd,
				D370PCA_DA, DM370_15));
			menugoal(dmc_msg);
		} /* endif */
		diag_ucd = FALSE;
	}  /* endif */
	else
	{
		strcpy (tucb_ptr.pr.diagcode, diag_ucode_path);
	} /* endelse */

	/* Check configuration status of adapter */
	if ((made_device = configure_device (da_input.dname)) == FAIL)
	{
	        if ((exitrc = err_log (da_input.dname)) == TU_BAD)
	                exit_da (exitrc);
	        strcpy (frub[0].dname, da_input.dname);
	        frub[0].rcode = 0x600;
	        frub[0].rmsg = D370_ECFG;
	        frub[0].frus[0].conf = conf4;
	        frub[0].frus[1].conf = conf5;
	        frub[0].frus[2].conf = conf5;
	        frub[0].frus[0].fru_flag = DA_NAME;
	        frub[0].frus[1].fru_flag = PARENT_NAME;
	        strcpy (frub[0].frus[2].fname, fnam2);
	        exitrc = add_fru (&fru_set);
	        exit_da (exitrc);
	}

	/* Open the device driver in diagnostics mode */
	sprintf (devcat, "/dev/%s/D", da_input.dname);
	if ((fdes = open (devcat, O_RDWR)) == FAIL)
	{
	        exitrc = FAIL_OPN;
	        if (errno == EIO || errno == ENXIO)
	        {
	                strcpy (frub[0].dname, da_input.dname);
	                frub[0].rcode = 0x500;
	                frub[0].rmsg = D370_OPEN;
	                frub[0].frus[0].conf = conf3;
	                frub[0].frus[1].conf = conf5;
	                strncpy (frub[0].frus[0].fname, 0, NAMESIZE);
	                strncpy (frub[0].frus[1].fname, 0, NAMESIZE);
	                frub[0].frus[0].fmsg = 0;
	                frub[0].frus[1].fmsg = 0;
	                frub[0].frus[0].fru_flag = DA_NAME;
	                frub[0].frus[1].fru_flag = PARENT_NAME;
	                frub[0].frus[0].fru_exempt = 0;
	                frub[0].frus[1].fru_exempt = 0;
	                exitrc = add_fru (&fru_set);
	        }  /* endif */
	        exit_da (exitrc);
	}  /* endif */

	/* Execute test units if testmode is not ELA */
	if (testmode != NO_TEST && diag_ucd == TRUE)
	{
	        tucb_ptr.pr.dmasize = 4096;
	        tucb_ptr.pr.sram_addr = 4096;
	        for (i = 0; i < maxtest; i++)
	        {
	                tucb_ptr.header.loop = 1;
	                switch (testmode)
	                {
	                case N_I:
	                        tucb_ptr.header.tu = set0[i];
	                        break;
	                case R_1:
	                        tucb_ptr.header.tu = set1[i];
	                        break;
	                case A_T:
	                        tucb_ptr.header.tu = set3[i];
	                        break;
	                }
	                if ((exitrc = tu_test (tucb_ptr.header.tu)) != TU_GOOD)
	                        exit_da (exitrc);
	        }
	}

	/* Run ELA if all test units completed without errors, testmode is
	   ELA or diagnostic microcode is not present */
	if ((d_mode == PD || d_mode == ELA) &&
	   (l_mode == NT_LP || l_mode == EN_LP))
		exitrc = err_log (da_input.dname);

	/* Do cleanup and exit */
	exit_da (exitrc);
}  /* end main */

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

setdamode (damode, max_tu)      /* begin setdamode */
int     *damode, *max_tu;
{
	int	rc = ZERO;

	*damode = INVALID_TM_INPUT;
	if (e_mode == EXR)
	{
		*damode = N_I;
		*max_tu = SET_0;
		return (rc);
	}
	switch (l_mode)
	{
	case NT_LP:
	case EN_LP:
		if (d_mode == ELA)
		{
	               	*damode = NO_TEST;
	               	*max_tu = ZERO;
	        }
	        if (a_mode == ADT && (s_mode == SYF && c_mode == CNT))
	        {
	 		*damode = A_T;
	  		*max_tu = SET_3;
	        }
	        else
	        {
	               	*damode = N_I;
	               	*max_tu = SET_0;
	        }
		if ((rc = putdavar (da_input.dname, "mode", DIAG_INT,
		    &testmode)) != ZERO)
			return (rc = SW_ERR);
		if ((rc = putdavar (da_input.dname, "max", DIAG_INT,
		    &maxtest)) != ZERO)
			return (rc = SW_ERR);
		break;
	case IN_LP:
	case EX_LP:
		if ((rc = getdavar (da_input.dname, "mode", DIAG_INT,
		    &testmode)) == FAIL)
			return (rc = SW_ERR);
		if ((rc = getdavar (da_input.dname, "max", DIAG_INT,
		    &maxtest)) == FAIL)
			return (rc = SW_ERR);
		break;
	} /* endswitch l_mode */
	return (rc);
}  /* end setdamode */

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

err_log (name)
char    *name;
{
	char    *crit;
	struct  errdata err_data;
	int     el_rc = TU_GOOD;
	int     err_logged;

	return (el_rc);
	crit = (char *) malloc (strlen(name)+NAMESIZE);
	sprintf (crit, "-N %s", name);
	err_logged = error_log_get (INIT, crit, &err_data);
	while (err_logged > 0)
	{
	        if (err_data.err_id == 0x00000000 ||
	            err_data.err_id == 0x00000000)
	        {
	                el_rc = add_fru (&fru_set);
	                return (el_rc);
	        }
	        err_logged = error_log_get (SUBSEQ, crit, &err_data);
	}  /* endwhile */
	err_logged = error_log_get (TERMI, crit, &err_data);
	return (el_rc);
}  /* err_log end */

/*
 * NAME: exit_da
 *
 * FUNCTION:    Performs cleanup operations prior to returning to the
 *              diagnostic controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  NONE
 */

exit_da (exit_code)
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
	}  /* endswitch exit_code */
	if (c_mode == CNT)
	{
	        if (catd != CATD_ERR)
	                catclose (catd);
	        diag_asl_quit (NULL);
	}  /* endif */
	if (fdes != FAIL)
	        close (fdes);
	if (made_device != FAIL)
	        initial_state (made_device, da_input.dname);
	odm_terminate ();
	DA_EXIT ();
}  /* end exit_da */
