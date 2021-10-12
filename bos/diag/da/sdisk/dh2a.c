static char sccsid[] = "@(#)83	1.9  src/bos/diag/da/sdisk/dh2a.c, dasdisk, bos411, 9428A410j 6/15/94 10:45:47";
/*
 *   COMPONENT_NAME: DASDISK
 *
 *   FUNCTIONS: clean_up
 *		create_frub
 *		diag_tst
 *		int_handler
 *		main
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* include of header files. Note: Keep include files alphabetized */

#include "dh2.h"
#include "dh2srn.h"
#include "dh2_msg.h"
#include <diag/bit_def.h>
#include <diag/da.h>
#include <diag/dcda_msg.h>
#include <diag/diag.h>
#include <diag/diag_exit.h>
#include <diag/diago.h>
#include <diag/tm_input.h>
#include <diag/tmdefs.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <nl_types.h>
#include <stdio.h>
#include <signal.h>
#include <sys/buf.h>
#include <sys/cfgodm.h>
#include <sys/devinfo.h>
#include <sys/device.h>
#include <sys/errids.h>
#include <sys/ioctl.h>
#include <sys/serdasd.h>
#include <sys/scsi.h>
#include <sys/sysconfig.h>
#include <sys/stat.h>

/* External functions used in this file */

extern			getdainput();
extern		long	getdamode();
extern nl_catd diag_catopen(char *, int);

/* Global variables and structures used in this file.  */

nl_catd			catd;			/* message file catalog */
char			*devname=(char *)0;	/* /dev/xx name used in openx */
static	char		dname[]="/dev/";	/* constant /dev/ */
struct	fru_bucket	frub[] =	/* Frub to be used when found failure */
{
	{"",FRUB1, -1, 0x0, DSDASD_MSG1, {
		{ 0, "", "", 0, 0, 0},
		},
		}
	};

static	int		init_config_state=-1;/* initial configuration state */
int			fdes=-99;
struct	tm_input	tm_input;	/* DA test param and other info. */


/*  */
/*
 * NAME: main()
 *
 * FUNCTION: Main routine for the Serial disk Adapter Diagnostic Application.
 *	     This routine is invoked by the Diagnostic Controller.
 *
 * EXECUTION ENVIRONMENT: This routine requires the ability to page fault.
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: DA_EXIT() is called to return the codes to the controller. Also
 *	    if a problem is found with the device a FRUB is generated.
 */

main(argc, argv, envp)
int	argc;
char	**argv;
char	**envp;
	{

		void	clean_up();
		int	create_frub();
		void	diag_tst();
		void	int_handler(int);
		int	errno_rc, envflag;
		int	rc,diskette_based;
		int	srn=0;
		struct	sigaction	act;

		setlocale( LC_ALL, "" );

	 	/* set up interrupt handler     */

		act.sa_handler = int_handler;
		sigaction(SIGINT, &act, (struct sigaction *)NULL);

		/* variables initialization */

		damode = (long)NULL;

		DA_SETRC_TESTS( DA_TEST_FULL );

		/* Open and initialize odm */

		if(init_dgodm() == -1) {
			DA_SETRC_ERROR ( DA_ERROR_OTHER );
			clean_up();
		}
		/* Get tm_input and da_mode */

		if( getdainput( &tm_input ) != FALSE ) {
			DA_SETRC_ERROR ( DA_ERROR_OTHER );
			clean_up();
		} else {
			devname = (char *)malloc(NAMESIZE + strlen( dname )+ 1);
			if(devname != NULL)
				strcpy( devname, dname );
			else {
				DA_SETRC_ERROR( DA_ERROR_OTHER );
				clean_up();
			}
			strcat( devname, tm_input.dname );
		}
		strcpy(frub[0].dname, tm_input.dname);
		damode = getdamode( &tm_input );
		if(damode == (long)NULL) {
			DA_SETRC_ERROR( DA_ERROR_OTHER );
			clean_up();
		}
		/* check console presence and start ASL */

		if( damode & DA_CONSOLE_TRUE ) {
			chk_asl_status( diag_asl_init( NULL ) );
			catd = diag_catopen(MF_DH2, 0);
		}
		(void)display_title(ADAPTER);

       		if( damode & DA_DMODE_MS1 ) { /* Missing flag set */
			create_frub(ADAPTER_CONFIG_ERR);
	       		clean_up();
	        }

		/* Check presence of microcode file */

		if(damode & DA_LOOPMODE_NOTLM){
        		envflag = ipl_mode(&diskette_based);
		        if((diskette_based == DIAG_FALSE) &&
				(damode & DA_CONSOLE_TRUE))
			     if((rc=check_microcode_file(tm_input.dname))== -1){
                		(void)disp_menu(NO_MICROCODE_TITLE);
				clean_up();
			     }
    		}
		/* Run ELA first. If no failure detected, then run all tests. */

		if(!( damode & DA_DMODE_REPAIR ) && !(damode & DA_LOOPMODE_INLM)
		    &&!(damode & DA_LOOPMODE_EXITLM) )
			srn = analyze_error_log(ADAPTER);

		if(srn != 0) {
			create_frub(srn);
			clean_up();
		}
		if(damode & DA_DMODE_ELA)
			clean_up();
		else {   /* No error from ELA or REPAIR VERIFICATION mode */
			if( !(damode & DA_LOOPMODE_EXITLM) ) {
				init_config_state = configure_device(tm_input.dname);
				if(init_config_state == -1) {
					create_frub(ADAPTER_CONFIG_ERR);
					clean_up();
				}
				fdes = openx( devname, O_RDWR, NULL, SC_DIAGNOSTIC );
				errno_rc = errno;
				/* Run all tests if device can be opened. */

				if(fdes < 0) {
					if ((errno_rc == EBUSY) ||
					    (errno_rc == EACCES)) {
						DA_SETRC_ERROR( DA_ERROR_OPEN );
						clean_up();
					} else {
						create_frub(ADAPTER_OPENX_FAIL);
						clean_up();
					} 
				} else {
					chk_terminate_key();
					diag_tst(fdes);
					chk_terminate_key();
				}
			}
			clean_up();     /* No return from clean_up */
		}

	} /* end main */

/*  */
/*
 * NAME: clean_up()
 *
 * FUNCTION: Clean_up is called whenever a fatal error occurs or after
 *           the diagnostic tests have completed. The purpose is to
 *           close files, quit ASL and provide a single point exit back
 *           to the diagnostic controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: None.
 */

void
clean_up( )
	    {
		int rc;

		term_dgodm();                        /* terminate the ODM */

		if( fdes > 0 )                      /* Close device, if open */
			close(fdes);
		/* Now return adapter to initial state */
		if ( init_config_state >= 0 ){
			rc = initial_state(init_config_state, tm_input.dname);
		}
		sleep(10);

		if( damode & DA_CONSOLE_TRUE ){
			if (catd != CATD_ERR)      /* Close catalog files */
				catclose( catd );
			diag_asl_quit( NULL );    /* Terminate the ASl stuff */
		}

		DA_EXIT();              /* Exit to the Diagnostic controller */
	} /* endfunction clean_up */

/*  */
/*
 * NAME: create_frub()
 *
 * FUNCTION: Will be called if the testing of the device indicates a
 *           problem. It sets up the FRU buckets based on what test
 *           unit failed.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: 0 which is the last step to execute in diag_tst.
 *
 */

int
create_frub( srn )
int    srn;
	{
		frub[0].rcode = srn;
		/* insert, then addfrub for Diagnostic Controller. */
		if (!( damode & DA_CONSOLE_TRUE )){
		    strcpy(frub[0].frus[0].floc, tm_input.dnameloc);
		    frub[0].frus[0].fru_flag = DA_NAME;
		}
		if( insert_frub( &tm_input, &frub[0] ) != 0 ) 
			DA_SETRC_ERROR( DA_ERROR_OTHER );
		frub[0].sn = -1;
		if( addfrub( &frub[0] ) != 0 ) 
			DA_SETRC_ERROR( DA_ERROR_OTHER );
		DA_SETRC_STATUS( DA_STATUS_BAD );
		return( 0 );

	} /* end create_frub */

/*  */
/*
 * NAME: diag_tst()
 *
 * FUNCTION: Performs the Serial disk Adapter test as outlined in the
 *           Serial disk Problem Determination Guide for RIOS. It proceeds
 *           step by step calling its respective ioctl cmd. It examines
 *           the return code from the cmd and makes a decision on which
 *           step to do next. If an error occurres it sets the return
 *           code for the controller. If a problem is found during any
 *           of the steps, it calls the create_frub function to set up the
 *           FRU bucket to be passed back to the controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: Test sequence is as follows:
 *        Send Reset to Adapter
 *	  Wait 2 seconds.
 *	  Read Card ID.
 *        If ok send Quiesce.
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: None.
 */

void
diag_tst( fdes )
int fdes;
	{

		int	rc;
		int	errno_rc;
		int	srn;
		struct	sd_ioctl_parms	sdioctl;

		sdioctl.data_length = 0;
		sdioctl.buffer = NULL;
		sdioctl.time_out = 60;
		sdioctl.status_validity = 0x00;
		sdioctl.adapter_status = 0x00;
		sdioctl.resvd1 = 0;
		sdioctl.resvd2 = 0;
		sdioctl.resvd3 = 0;
		sdioctl.resvd4 = 0;
		sdioctl.resvd5 = 0;
		sdioctl.resvd6 = 0;
		sdioctl.resvd7 = 0;
		sdioctl.reset_type = SD_RESET_OP;
		/* Do RESET, then wait 2 secs. don't care about rc */
		rc = ioctl(fdes, SD_RESET, &sdioctl);
		sleep(2);
		/* Read adapter id */
		rc = ioctl(fdes, SD_READ_ADAP_ID, &sdioctl);
		errno_rc = errno;
		if((rc != 0) || (sdioctl.resvd1 != SD_ADAP_ID)){
			rc = create_frub(ADAPTER_POST_FAIL);
			return;
		}
		/* Send QUIESCE command */
		sdioctl.reset_type = SD_QUIESCE_OP;
		sdioctl.status_validity = 0x00;
		rc = ioctl(fdes, SD_RESET, &sdioctl);
		errno_rc = errno;
		if( (rc == 0) && (sdioctl.status_validity == 0x00) )
			return;
		else
			if(sdioctl.status_validity & 0x01){
			/* Byte 2 of alert register is used here */
				srn = ADAPTER_FAILURE +
					(int)sdioctl.adapter_status;
			} else
					srn = PIO_ERROR;
			rc = create_frub( srn );

	} /* end diag_tst */

/*  */
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
void int_handler(int sig)
{
	clean_up();
}
