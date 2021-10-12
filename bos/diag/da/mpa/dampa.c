static char sccsid[] = "@(#)97	1.1  src/bos/diag/da/mpa/dampa.c, mpada, bos411, 9428A410j 5/5/93 14:19:58";
/*
 *   COMPONENT_NAME: (MPADIAG) MP/A DIAGNOSTICS
 *
 *   FUNCTIONS: err_log
 *		exit_da
 *		int_handler
 *		main
 *		setdamode
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include        "dampa.h"

char    cmdstr[256];           /* string for system cmd calls */
char    *pat_ptr;             /* used as data pointer for tu's */
FILE    *bugfd;               /* The fildes for the debug file */
			      /* info in file /tmp/bfile   */
char    msgbuf[256];
char    *msg=msgbuf;
int     user_said_no_wrap = 0;

main ()  /* begin main */
{
	int     mode, i, odmrc = 0;
	int     aslrc = TU_GOOD;
	int     rc = TU_GOOD;
	int     exitrc = SW_ERR;
	int     maxtest;
	char    umc_msg[512], dmc_msg[512];
	char    string[512];            /* working string */
	struct  CuDv   CuDv;        /* Object structures */


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
	if (((rc = init_dgodm ()) != ZERO) ||
	    ((rc = getdainput (&da_input)) != ZERO))
		exit_da (exitrc);

	/* determine ipl mode */
	mode = ipl_mode (&diskette);

	/* set testmode variables */
	l_mode = da_input.loopmode;     /* NT_LP, EN_LP, IN_LP, EX_LP */
	c_mode = da_input.console;      /* CNT, CNF */
	a_mode = da_input.advanced;     /* ADT, ADF */
	s_mode = da_input.system;       /* SYT, SYF */
	e_mode = da_input.exenv;        /* IPL, STD, RGR, CNC, EXR */
	d_mode = da_input.dmode;        /* ELA, PD, RPR, MS1, MS2 */
	setdamode (&testmode, &maxtest);


	sprintf(msg,"\n* loops is %d *on dev %s*\n\n",da_input.lcount,da_input.dname);
	RPT_BUGGER(msg);

	if( l_mode == NT_LP || l_mode == EN_LP) {
	    sprintf(msg,"\nSet wrap_1 to FALSE in odm\n");
	    RPT_BUGGER(msg);
	    wrap_1 = FALSE;
	    putdavar (da_input.dname, "wrp1", DIAG_INT, &wrap_1);
	}
            sprintf(msg,"\nStart config_device\n");
            RPT_BUGGER(msg);
	/* Check configuration status of adapter */
	if ((made_device = configure_device (da_input.dname)) == FAIL)
	{
		sprintf(msg, "cfg mpa failed\n");
		RPT_BUGGER(msg);
		if ((exitrc = err_log (da_input.dname)) == TU_BAD)
			exit_da (exitrc);
		strcpy (frub[0].dname, da_input.dname);
		frub[0].rcode = 0x600;
		frub[0].rmsg = DMPA_ECFG;    /* msg # in vendor.cat */
		frub[0].frus[0].conf = conf4;
		frub[0].frus[1].conf = conf5;
		frub[0].frus[2].conf = conf5;
		frub[0].frus[0].fru_flag = DA_NAME;
		frub[0].frus[1].fru_flag = PARENT_NAME;
		strcpy (frub[0].frus[2].fname, fnam2);
		exitrc = add_fru (&fru_set);
		exit_da (exitrc);
	}
        sprintf(msg, "config dev done\n");
        RPT_BUGGER(msg);
	if(made_device == 0) {
	    /* made_device = 0 says that mpa was not defined so no */
	    /* driver could have been defined and when diag code   */
	    /* above made the deivce the cfgmpaa call will not     */
	    /* load any  driver so set no driver to one.           */
	    sprintf(msg, "made_device was 0 so set no_drvr=1\n");
	    RPT_BUGGER(msg);
	    no_drvr = 1;
	}
	/* check to see if a driver is loaded and if so is it the */
	/* diag driver, if not save name of current driver and    */
	/* use rmdev, mkdev to change to diag driver.             */

	/* diag driver dmpaX, X depends on the da_input.dname suffix */
	/* find and set index for driver */
	sprintf(diagdrvr,"dmpa%s",da_input.dname+(strlen(da_input.dname)-1));

	sprintf(string,"parent = '%s' AND status = 1",da_input.dname);

        sprintf(msg, "search crit %s, diagdrvr is %s\n",string,diagdrvr);
        RPT_BUGGER(msg);
	odmrc =(long)odm_get_obj(CuDv_CLASS,string,&CuDv,ODM_FIRST);
	if ( odmrc == -1 ) {
		sprintf(msg,"odm get for active driver failed\n");
		RPT_BUGGER(msg);
		if ((exitrc = err_log (da_input.dname)) == TU_BAD)
			exit_da (exitrc);
		strcpy (frub[0].dname, da_input.dname);
		frub[0].rcode = 0x700;
		frub[0].rmsg = DMPA_EOMD;   /* msg # in vendor.cat */
		frub[0].frus[0].conf = conf4;
		frub[0].frus[1].conf = conf5;
		frub[0].frus[2].conf = conf5;
		frub[0].frus[0].fru_flag = DA_NAME;
		frub[0].frus[1].fru_flag = PARENT_NAME;
		strcpy (frub[0].frus[2].fname, fnam2);
		exitrc = add_fru (&fru_set);
		exit_da (exitrc);
	}
	else if ( odmrc == 0 ) {  /* no driver found configured */
	                          /* config diag driver         */
        sprintf(msg, "no drivers found setting no_drvr to 1\n");
        RPT_BUGGER(msg);

		no_drvr = 1;      
		/* this can happen now since the mpaa cfg method does not */
		/* define or config any children. When it does I will want to */
                /* leave the adapter with no driver when I am done          */
	}
	else {                    /* change to diag driver */
		/* save name of origional driver */
		if ( strcmp(CuDv.name,diagdrvr) )  {
		     sprintf(savedrvr,"%s",CuDv.name);
		     change_drvr = 1;  /* so I know to put back the origional */
				       /* in exit_da().                  */
		     sprintf(msg, "saving current driver %s\n",savedrvr);
		     RPT_BUGGER(msg);
		}
		else {
		     sprintf(savedrvr,"%s","none");
		     sprintf(msg, "current driver was diag driver\n");
		     RPT_BUGGER(msg);
		}
	}

	/* now change to diag driver if necessary */
	if( change_drvr ) {
	   sprintf(msg, "change to diag driver\n");
	   RPT_BUGGER(msg);
	   /* remove the functional driver */
	   sprintf(cmdstr,"rmdev -l %s > /dev/null 2>&1",savedrvr);
	   system(cmdstr);
	   sprintf(cmdstr,"mkdev -l %s -c driver -s mpaa -t dmpa -p %s -w 0 "
		"> /dev/null 2>&1",diagdrvr,da_input.dname);
	   system(cmdstr);
	}
        sprintf(msg, "Start the no_driver so mkdev code\n");
        RPT_BUGGER(msg);
	if( no_drvr ) {
	   /* if there is no driver on the mpaa card then put diag on. */
           sprintf(cmdstr,"mkdev -l %s -c driver -s mpaa -t dmpa -p %s -w 0 "
                "> /dev/null 2>&1",diagdrvr,da_input.dname);
           system(cmdstr);
	}
	if( l_mode == EN_LP) {
	       sprintf(msg,"***ODM SAVE THESE \n");
	       RPT_BUGGER(msg);

	       putdavar (da_input.dname, "nodr", DIAG_INT, &no_drvr);
	       sprintf(msg,"     no_drvr = %d saved \n",no_drvr);
	       RPT_BUGGER(msg);

	       putdavar (da_input.dname, "chdr", DIAG_INT, &change_drvr);
	       sprintf(msg,"     change_drvr = %d saved \n",change_drvr);
	       RPT_BUGGER(msg);

	       putdavar (da_input.dname, "mddv", DIAG_INT, &made_device);
	       sprintf(msg,"     made_device = %d saved \n",made_device);
	       RPT_BUGGER(msg);

	       putdavar (da_input.dname, "dadr", DIAG_STRING, diagdrvr);
	       sprintf(msg,"     diagdrvr = %s saved \n",diagdrvr);
	       RPT_BUGGER(msg);

	       putdavar (da_input.dname, "sadr", DIAG_STRING, savedrvr);
	       sprintf(msg,"     savedrvr = %s saved \n",savedrvr);
	       RPT_BUGGER(msg);
	}
	else {
	       sprintf(msg,"***SAVE THESE but not in odm.\n");
	       RPT_BUGGER(msg);
	       sprintf(msg,"     no_drvr = %d saved \n",no_drvr);
	       RPT_BUGGER(msg);
	       sprintf(msg,"     change_drvr = %d saved \n",change_drvr);
	       RPT_BUGGER(msg);
	       sprintf(msg,"     made_device = %d saved \n",made_device);
	       RPT_BUGGER(msg);
	       sprintf(msg,"     diagdrvr = %s saved \n",diagdrvr);
	       RPT_BUGGER(msg);
	       sprintf(msg,"     savedrvr = %s saved \n",savedrvr);
	       RPT_BUGGER(msg);
	}


	/* Open the device driver in diagnostics mode */
	sprintf (devcat, "/dev/%s", diagdrvr);
	if ((fdes = open (devcat, O_RDWR)) == FAIL)
	{
		exitrc = FAIL_OPN;
		if (errno == EIO || errno == ENXIO)
		{
			strcpy (frub[0].dname, diagdrvr);
			frub[0].rcode = 0x500;
			frub[0].rmsg = DMPA_OPEN;  /* msg in vendor.cat */
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
	if (testmode != NO_TEST)
	{
		/* these inputs to TU's will always be the same for */
		/* all TU's */
		tucb_ptr.pr.num_oper  = 1;    /* only do one loop in tu */
		tucb_ptr.pr.fix_bytes = 1;    /* xfer fixed number of bytes */
		tucb_ptr.pr.clock     = 1;    /* use internal clock */
		tucb_ptr.header.loop  = 1;
		tucb_ptr.pr.bps       = 9600; /* default for tu 5 */
		tucb_ptr.pr.wrap_type = 0;    /* internal wrap */


		for (i = 0; i < maxtest; i++)
		{
			switch (testmode)
			{
			case A_T:
				sprintf(msg, "*************do A_T test set3[%d]..num=%02X\n",i,set3[i]);
				RPT_BUGGER(msg);
				if(set3[i] > 10)  {

				     if( (tucb_ptr.header.tu = set3[i]>>4) == TEST2) {

					  switch ( (set3[i]&0x0F) )
					  {
					     case 0:
						tucb_ptr.pr.wrap_type = 1;   /* external wrap */
						tucb_ptr.pr.recsize = 32;
						tucb_ptr.pr.bps     = 300;
						tucb_ptr.pr.nrzi    = 0;
						break;
					     case 1:
						tucb_ptr.pr.wrap_type = 1;   /* external wrap */
						tucb_ptr.pr.recsize = 23;
						tucb_ptr.pr.bps     = 50;
						tucb_ptr.pr.nrzi    = 0;
						break;
					     case 2:
						tucb_ptr.pr.wrap_type = 1;   /* external wrap */
						tucb_ptr.pr.recsize = 25;
						tucb_ptr.pr.bps     = 150;
						tucb_ptr.pr.nrzi    = 1;
						break;
					     case 3:
						tucb_ptr.pr.wrap_type = 1;   /* external wrap */
						tucb_ptr.pr.recsize = 19;
						tucb_ptr.pr.bps     = 300;
						tucb_ptr.pr.nrzi    = 0;
						break;
					     case 4:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 13;
						tucb_ptr.pr.bps     = 50;
						tucb_ptr.pr.nrzi    = 0;
						break;
					     case 5:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 46;
						tucb_ptr.pr.bps     = 300;
						tucb_ptr.pr.nrzi    = 0;
						break;
					     case 6:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 14;
						tucb_ptr.pr.bps     = 150;
						tucb_ptr.pr.nrzi    = 1;
						break;
					     case 7:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 45;
						tucb_ptr.pr.bps     = 300;
						tucb_ptr.pr.nrzi    = 0;
						break;
					     default:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 39;
						tucb_ptr.pr.bps     = 600;
						tucb_ptr.pr.nrzi    = 1;
						break;
					  }
				     }
				     else {          /* this is TU 3 set of tests */
					  switch ( (set3[i]&0x0F) )
					  {
					     case 0:
						tucb_ptr.pr.wrap_type = 1;   /* external wrap */
						tucb_ptr.pr.recsize = 699;
						tucb_ptr.pr.bps     = 9600;
						tucb_ptr.pr.nrzi    = 0;
						break;
					     case 1:
						tucb_ptr.pr.wrap_type = 1;   /* external wrap */
						tucb_ptr.pr.recsize = 342;
						tucb_ptr.pr.bps     = 9600;
						tucb_ptr.pr.nrzi    = 0;
						break;
					     case 2:
						tucb_ptr.pr.wrap_type = 1;   /* external wrap */
						tucb_ptr.pr.recsize = 4095;
						tucb_ptr.pr.bps     = 19200;
						tucb_ptr.pr.nrzi    = 1;
						break;
					     case 3:
						tucb_ptr.pr.wrap_type = 1;   /* external wrap */
						tucb_ptr.pr.recsize = 103;
						tucb_ptr.pr.bps     = 600;
						tucb_ptr.pr.nrzi    = 0;
						break;
					     case 4:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 1091;
						tucb_ptr.pr.bps     = 9600;
						tucb_ptr.pr.nrzi    = 0;
						break;
					     case 5:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 345;
						tucb_ptr.pr.bps     = 9600;
						tucb_ptr.pr.nrzi    = 0;
						break;
					     case 6:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 4045;
						tucb_ptr.pr.bps     = 19200;
						tucb_ptr.pr.nrzi    = 1;
						break;
					     case 7:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 103;
						tucb_ptr.pr.bps     = 600;
						tucb_ptr.pr.nrzi    = 0;
						break;
					     default:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 3423;
						tucb_ptr.pr.bps     = 9200;
						tucb_ptr.pr.nrzi    = 1;
						break;
					  }
				     }
				}
				else  {
				   tucb_ptr.header.tu = set3[i];
				   if(tucb_ptr.header.tu == 6) {
				      tucb_ptr.pr.wrap_type = 1;
				   }
				}
				break;
			default:
				sprintf(msg, "mode...%d  test set0[%d]..num=%02X\n",testmode,i,set0[i]);
				RPT_BUGGER(msg);
				if(set0[i] > 10)  {

				     if( (tucb_ptr.header.tu = set0[i]>>4) == TEST2) {
					  switch ( (set0[i]&0x0F) )
					  {
					     case 4:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 32;
						tucb_ptr.pr.bps     = 300;
						tucb_ptr.pr.nrzi    = 0;
						break;
					     case 5:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 13;
						tucb_ptr.pr.bps     = 50;
						tucb_ptr.pr.nrzi    = 0;
						break;
					     case 6:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 12;
						tucb_ptr.pr.bps     = 150;
						tucb_ptr.pr.nrzi    = 1;
						break;
					     case 7:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 21;
						tucb_ptr.pr.bps     = 300;
						tucb_ptr.pr.nrzi    = 0;
						break;
					     default:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 37;
						tucb_ptr.pr.bps     = 600;
						tucb_ptr.pr.nrzi    = 1;
						break;
					  }
				     }
				     else {          /* this is TU 3 set of tests */
					  switch ( (set0[i]&0x0F) )
					  {
					     case 4:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 402;
						tucb_ptr.pr.bps     = 9600;
						tucb_ptr.pr.nrzi    = 0;
						break;
					     case 5:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 185;
						tucb_ptr.pr.bps     = 9600;
						tucb_ptr.pr.nrzi    = 0;
						break;
					     case 6:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 4095;
						tucb_ptr.pr.bps     = 19200;
						tucb_ptr.pr.nrzi    = 1;
						break;
					     case 7:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 103;
						tucb_ptr.pr.bps     = 600;
						tucb_ptr.pr.nrzi    = 0;
						break;
					     default:
						tucb_ptr.pr.wrap_type = 0;   /* internal wrap */
						tucb_ptr.pr.recsize = 3423;
						tucb_ptr.pr.bps     = 19200;
						tucb_ptr.pr.nrzi    = 1;
						break;
					  }
				     }
				}
				else  tucb_ptr.header.tu = set0[i];
				break;
			}
			if ((exitrc = tu_test (tucb_ptr.header.tu)) != TU_GOOD)
				exit_da (exitrc);
		}
	}

	/* Run ELA if all test units completed without errors, testmode is
	   ELA or diagnostic microcode is not present */
	if (e_mode != RGR && e_mode != EXR)
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
 *      file dampa.h.
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
	*damode = INVALID_TM_INPUT;
	if (e_mode != RGR && e_mode != EXR)
	{
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
	}  /* endif */
	else
	{
		if (e_mode == RGR)
		{
			switch (d_mode)
			{
			case DMODE_ELA:
				*damode = R_0;
				*max_tu = SET_0;
				break;
			case DMODE_PD:
				*damode = R_1;
				*max_tu = SET_0;
				break;
			default:
				*damode = R_0;
				*max_tu = SET_0;
				break;

			}  /* end switch (d_mode) */
			if (c_mode == CNF)
			{
				*damode = R_0;
				*max_tu = SET_0;
			}
		}
		else
		{
			*damode = R_0;
			*max_tu = SET_0;
		}  /* endelse */
	}  /* endelse */
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
	long    menuid;
	int     msgid;

	getdavar (da_input.dname, "wrp1", DIAG_INT, &wrap_1);

	if(wrap_1 == TRUE) {
            sprintf(msg,"wrap_1 was true at exit\n");
            RPT_BUGGER(msg);
	    if ((l_mode == NT_LP || l_mode == EX_LP)&& c_mode == CNT)
	    {
		wrap_1 = FALSE;
		putdavar (da_input.dname, "wrp1", DIAG_INT, &wrap_1);
		menuid = 0x996108;
		msgid = DMMPA_10;

		diag_action (menuid, msgid);
	     }
	}
	if(l_mode == EX_LP) {
	     user_said_no_wrap = 0;
	     putdavar (da_input.dname, "usno", DIAG_INT, &user_said_no_wrap);
	}
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
		if (catd != MPAD_ERR)
			catclose (catd);
		diag_asl_quit (NULL);
	}  /* endif */
	if (fdes != FAIL)
		close (fdes);

	if(l_mode == EX_LP || l_mode == NT_LP ) {
	    if( l_mode == EX_LP) {
	       sprintf(msg,"***ODM GET THESE \n");
	       RPT_BUGGER(msg);

	       getdavar (da_input.dname, "nodr", DIAG_INT, &no_drvr);
	       sprintf(msg,"     no_drvr = %d saved \n",no_drvr);
	       RPT_BUGGER(msg);

	       getdavar (da_input.dname, "chdr", DIAG_INT, &change_drvr);
	       sprintf(msg,"     change_drvr = %d saved \n",change_drvr);
	       RPT_BUGGER(msg);

	       getdavar (da_input.dname, "mddv", DIAG_INT, &made_device);
	       sprintf(msg,"     made_device = %d saved \n",made_device);
	       RPT_BUGGER(msg);

	       getdavar (da_input.dname, "dadr", DIAG_STRING, diagdrvr);
	       sprintf(msg,"     diagdrvr = %s saved \n",diagdrvr);
	       RPT_BUGGER(msg);

	       getdavar (da_input.dname, "sadr", DIAG_STRING, savedrvr);
	       sprintf(msg,"     savedrvr = %s saved \n",savedrvr);
	       RPT_BUGGER(msg);
	    }
	    else {
	       sprintf(msg,"***Use saved ,but not from odm.\n");
	       RPT_BUGGER(msg);
	       sprintf(msg,"     no_drvr = %d saved \n",no_drvr);
	       RPT_BUGGER(msg);
	       sprintf(msg,"     change_drvr = %d saved \n",change_drvr);
	       RPT_BUGGER(msg);
	       sprintf(msg,"     made_device = %d saved \n",made_device);
	       RPT_BUGGER(msg);
	       sprintf(msg,"     diagdrvr = %s saved \n",diagdrvr);
	       RPT_BUGGER(msg);
	       sprintf(msg,"     savedrvr = %s saved \n",savedrvr);
	       RPT_BUGGER(msg);

	    }
	    /* now change back to origional driver if necessary */
	    if( change_drvr ) {
	       sprintf(msg,"\n**** I am done so change from %s to %s\n",diagdrvr,savedrvr);
	       RPT_BUGGER(msg);
	       sprintf(cmdstr,"rmdev -l %s -d > /dev/null 2>&1",diagdrvr);
	       system(cmdstr);
	       sprintf(cmdstr,"mkdev -l %s > /dev/null 2>&1",savedrvr);
	       system(cmdstr);
	    }
	    if(no_drvr){
	       sprintf(msg,"\n**** I am done and must rmdev %s\n",diagdrvr);
	       RPT_BUGGER(msg);
	       sprintf(cmdstr,"rmdev -l %s -d > /dev/null 2>&1",diagdrvr);
	       system(cmdstr);
	    }
	    if (made_device != FAIL) {
	       sprintf(msg,"\n**** Put card back to init state, made_device is %d\n",made_device);
	       RPT_BUGGER(msg);
	       initial_state (made_device, da_input.dname);
	    }
	}
	if( l_mode == EX_LP) {
	       sprintf(msg,"\n**** loop mode ending reset the odm vars\n");
	       RPT_BUGGER(msg);
	       made_device = FAIL;
	       change_drvr = 0;
	       no_drvr = 0;
	       putdavar (da_input.dname, "nodr", DIAG_INT, &no_drvr);
	       putdavar (da_input.dname, "chdr", DIAG_INT, &change_drvr);
	       putdavar (da_input.dname, "mddv", DIAG_INT, &made_device);
	}
	odm_terminate ();
	DA_EXIT ();
}  /* end exit_da */
