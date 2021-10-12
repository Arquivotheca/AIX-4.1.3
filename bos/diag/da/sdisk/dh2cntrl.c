static char sccsid[] = "@(#)79	1.9  src/bos/diag/da/sdisk/dh2cntrl.c, dasdisk, bos411, 9428A410j 6/15/94 10:45:56";
/*
 *   COMPONENT_NAME: DASDISK
 *
 *   FUNCTIONS: clean_up
 *		create_frub
 *		diag_tst
 *		get_lun
 *		int_handler
 *		main
 *		tu_test
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
#include <diag/har2_atu.h>
#include <diag/tm_input.h>
#include <diag/tmdefs.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <nl_types.h>
#include <signal.h>
#include <stdio.h>
#include <sys/buf.h>
#include <sys/cfgodm.h>
#include <sys/devinfo.h>
#include <sys/device.h>
#include <sys/errids.h>
#include <sys/ioctl.h>
#include <sys/serdasd.h>
#include <sys/sysconfig.h>
#include <sys/stat.h>

/* External functions used in this file */

extern			getdainput();
extern		long	getdamode();
extern		int	exectu();
extern		int	scsitu_init();
extern nl_catd diag_catopen(char *, int);

/* Global variables and structures used in this file.  */
nl_catd			catd;			/* message file catalog */
char			*devname=(char *)0;	/*  name used in openx */
static	char		dname[]="/dev/";	/* constant /dev/ */
static	int		controller_id;    /* Controller id */
struct	fru_bucket	frub[] =	/* Frub to be used when found failure */
{
	{"",FRUB1, -1, 0x0, DSDASD_MSG1, {
		{ 0, "", "", 0, 0, 0},
		},
		}
	};

int			fdes=-99;
static	int		init_adapter_state= -1;/*initial config. state of parent*/
static	int		init_config_state=-1;  /* initial configuration state */
struct	tm_input	tm_input;	/* DA test param and other info. */
HAR2_TUTYPE		tucb;			/* structure for SCSI command */
uchar			tu_buffer[BUFFER_LENGTH];	/* Buffer to hold data  */


/*  */
/*
 * NAME: main()
 *
 * FUNCTION: Main routine for the Serial disk controller Diagnostic Application.
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

		void	  clean_up();
		int	  create_frub();
		void	  diag_tst();
		void	  int_handler(int);
		int	  tu_test();
		int	  errno_rc, envflag;
		int	  rc,diskette_based;
		int	  srn=0;
		struct 	  sigaction act;

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
			devname = (char *)malloc(NAMESIZE + strlen(dname) + 1 );
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

		/* Check presence of microcode file */

		if((damode & DA_LOOPMODE_NOTLM) || (damode & DA_LOOPMODE_ENTERLM)){
        		envflag = ipl_mode(&diskette_based);
		        if ((diskette_based == DIAG_FALSE) &&
				(damode & DA_CONSOLE_TRUE))
			     if((rc=check_microcode_file(tm_input.dname))== -1){
                		(void)disp_menu(NO_MICROCODE_TITLE);
				clean_up();
			     }
    		}

		/* Run ELA first. If no failure detected, then run all tests. */

		if(!( damode & DA_DMODE_REPAIR ) && !(damode & DA_LOOPMODE_INLM)
		    && !(damode & DA_LOOPMODE_EXITLM) )
			srn  = analyze_error_log(CONTROLLER);

		if(srn != 0) {
			create_frub( srn );
			clean_up();
		}
		if(damode & DA_DMODE_ELA)
			clean_up();
		else {   /* No error from ELA or REPAIR VERIFICATION mode */
			if( !(damode & DA_LOOPMODE_EXITLM) ) {
				(void)display_title(CONTROLLER);
				init_adapter_state = configure_device(tm_input.parent);
				if(init_adapter_state == -1) {
					create_frub( ADAPTER_CONFIG_ERR );
					clean_up();
				}
				init_config_state = configure_device(tm_input.dname);
				if(init_config_state == -1) {
					create_frub( CONTROLLER_CONFIG_ERR );
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
						create_frub( CONTROLLER_OPENX_FAIL );
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

		term_dgodm();                         /* terminate the ODM */

		if( fdes > 0 )                       /* Close device, if open */
			close(fdes);

		if ( init_config_state >= 0 ){
			rc = initial_state(init_config_state, tm_input.dname);
		}
		if ( init_adapter_state >= 0 ){
			rc = initial_state(init_adapter_state, tm_input.parent);
		}

		sleep(10);
		if( damode & DA_CONSOLE_TRUE ){
			if (catd != CATD_ERR)         /* Close catalog files */
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
create_frub( controller_err )
int    controller_err;
	{

		frub[0].rcode = controller_err;
		/* insert and add frub */
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
		return ( 0 );
	} /* end create_frub */

/*  */
/* NAME: diag_tst()
 *
 * FUNCTION: Performs the tests on the controller as outlined in the
 *	     Problem Determination Guide.
 * 
 * EXECUTION ENVIRONMENT: This function can page fault.
 *
 * NOTES:
 * 
 * RECOVERY OPERATION:
 *
 * NOTES:
 * 
 * RETURNS: None.
 */

#define		TOTAL_STEPS	6
#define		EXIT_STEP	0
#define		INQUIRY 	1
#define		FORCE_POST	2
#define		REQUEST_SENSE	3
#define		QUIESCE		4
#define		ADAPTER_FAILS	5
#define		BAD_RETURN	6

void
diag_tst(fdes)
int	fdes;
	{
		int		errno_rc;
		int		rc=0;
		int		stepcount[TOTAL_STEPS+1];
		struct	sd_ioctl_parms	sd_ioctl;
		int		srn;
		register	int	step;

		/* Clear all steps counter */
		for( step = 0; step <= TOTAL_STEPS; ++step )
			stepcount[step] = 0;
		/* setup lun id */

		controller_id = get_lun(tm_input.dname);
		if(controller_id == -1) {
			DA_SETRC_ERROR( DA_ERROR_OTHER );
			return;
		}
		sd_ioctl.data_length = 0;
		sd_ioctl.buffer = NULL;
		sd_ioctl.time_out = 60;
		sd_ioctl.status_validity = 0x00;
		sd_ioctl.resvd1 = 0;
		sd_ioctl.resvd2 = 0;
		sd_ioctl.resvd3 = 0;
		sd_ioctl.resvd4 = 0;
		sd_ioctl.resvd5 = 0;
		sd_ioctl.resvd6 = 0;
		sd_ioctl.resvd7 = 0;
		step = INQUIRY;
		while(step) {
			switch( step ) {
			case	INQUIRY:
				++stepcount[INQUIRY];
				rc = tu_test(fdes, SCATU_INQUIRY);
				switch( rc ) {
				case	SCATU_TIMEOUT	:
					step = create_frub( ADAPTER_TIMEOUT );
					break;
				case	SCATU_RESERVATION_CONFLICT	:
					step = create_frub(INVALID_RESERVATION);
					break;
				case 	SCATU_CHECK_CONDITION	:
					step = REQUEST_SENSE;
					break;
				case	SCATU_IO_BUS_ERROR	:
					step = create_frub( BUS_ERROR	);
					break;
				case	SCATU_ADAPTER_FAILURE	:
					step = ADAPTER_FAILS;
					break;
				case	SCATU_QUEUE_FULL	:
					step = create_frub( INVALID_Q_FULL );
					break;
				case 	SCATU_GOOD	:
					step = FORCE_POST;
					break;
				case	SCATU_BAD_PARAMETER	:
					step = create_frub( INVALID_PARAMS );
					break;
				default:
					step = create_frub(INVALID_RETURN_CODE);
					break;
				}
				break;
			case	FORCE_POST	:
				++stepcount[FORCE_POST];
				sd_ioctl.reset_type = SD_RESET_OP;
				rc = ioctl(fdes, SD_RESET, &sd_ioctl);
				errno_rc = errno;
				if( rc != 0 )
					if(errno_rc == ETIMEDOUT)
						step = create_frub
						    ( CONTROLLER_POST_FAIL );
					else
						step = BAD_RETURN;
				else
					if(sd_ioctl.status_validity == 0x00)
						step = EXIT_STEP;
					else
						step = BAD_RETURN;
				break;
			case	REQUEST_SENSE	:
				++stepcount[REQUEST_SENSE];
				rc = tu_test( fdes, SCATU_REQUEST_SENSE );
				switch( rc ) {
				case	SCATU_TIMEOUT	:
					step = create_frub( ADAPTER_TIMEOUT );
					break;
				case	SCATU_QUEUE_FULL	:
				case	SCATU_RESERVATION_CONFLICT	:
					step = create_frub(INVALID_SCSI_STATUS);
					break;
				case	SCATU_CHECK_CONDITION	:
					step = create_frub(CHECK_READING_SENSE);
					break;
				case	SCATU_NONEXTENDED_SENSE	:
					step = create_frub(INVALID_SENSE_DATA);
					break;
				case	SCATU_ADAPTER_FAILURE 	:
					step = ADAPTER_FAILS;
					break;
				case 	SCATU_BAD_PARAMETER	:
					step = create_frub( INVALID_PARAMS );
					break;
				case 	SCATU_GOOD	:
					if( tucb.scsiret.host_action_code == 0 )
						if(stepcount[REQUEST_SENSE]==1)
							step = QUIESCE;
						else
							step = create_frub
							  (CNTRL_QUIESCE_FAILS);
					else
						step = create_frub
							(tucb.scsiret.unit_error_code);
					break;
				default 	:
					step = create_frub(INVALID_RETURN_CODE);
					break;
				}
				break;
			case	QUIESCE	:
				++stepcount[QUIESCE];
				sd_ioctl.reset_type = SD_QUIESCE_OP;
				sd_ioctl.status_validity = 0x00;
				rc = ioctl(fdes, SD_RESET, &sd_ioctl);
				errno_rc = errno;
				if((rc == 0) && 
				      (sd_ioctl.status_validity == 0x00))
						step = INQUIRY;
				else
					step = BAD_RETURN;
				break;
			case	ADAPTER_FAILS	:
				++stepcount[ADAPTER_FAILS];
				if(tucb.scsiret.status_validity & 0x04)
					if(tucb.scsiret.alert_register_adap ==
						SD_CTRL_STATUS)
						if(stepcount[ADAPTER_FAILS]==1)
							step = QUIESCE;
						else
	 						srn = CNTRL_QUIESCE_FAILS;
					else  /* byte 2 of alert reg. */
						srn = ADAPTER_FAILURE +
						   tucb.scsiret.alert_register_adap;
				else
					if(tucb.scsiret.status_validity & 0x02) 
						srn = PIO_ERROR;
				if(step == ADAPTER_FAILS)
					step = create_frub(srn);
				break;
			case	BAD_RETURN	:
				++stepcount[BAD_RETURN];
				if(sd_ioctl.status_validity & 0x01)
					srn = ADAPTER_FAILURE +
						sd_ioctl.adapter_status;
				else
						srn = PIO_ERROR;
				step = create_frub( srn );
				break;
			default 	:
				break;
			}
		}
	}

/*  */
/*
 * NAME: get_lun()
 *
 * FUNCTION: given the logical name of a device, determine the lun id.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: lun id of device.
 */
int
get_lun(dev_name)
uchar	*dev_name;
	    {
		char    sstring[256];           /* search criteria pointer */
		struct CuDv *cudv;
		struct  listinfo        obj_info;

		sprintf(sstring,"name = %s",dev_name);
		cudv = get_CuDv_list( CuDv_CLASS, sstring, &obj_info, 1, 2);
		if (cudv == (struct CuDv *) -1)
			return(-1);
		return(cudv->connwhere[0]-'0');
	}

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
/*  */
/*
 * NAME: tu_test()
 *
 * FUNCTION: tu_test sets some parameters to be used in the execution
 *           of each test unit. It then calls the exectu function to
 *           actually perform the test.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: This function returns the return code from the test unit
 *          called by the exectu function.
 */

int
tu_test( tu_fdes, tnum )
int   tu_fdes;
int   tnum;
	{
		int	 step;
		int      rc;                        /* function return code    */
		int	 logical_unit = 0;	/* default lun to 0	     */

		tucb.header.tu = tnum;    /* put TU number to do in TU struct */
		tucb.header.mfg = 0;      /* always set to 0, only used by mfg */
		rc = scsitu_init ( &tucb );
		if( rc != SCATU_GOOD )
		{
			return ( SCATU_BAD_PARAMETER );
		}
		tucb.scsitu.flags = 0;
		tucb.scsitu.ioctl_pass_param = PASS_PARAM;
		tucb.scsitu.data_length = 0;
		tucb.scsitu.data_buffer = NULL;
		tucb.scsitu.cmd_timeout = 30;
		tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;
		tucb.scsitu.resvd5 = 0x10;
		tucb.scsitu.resvd5 |= controller_id << 5;
	/*
	  Based on TU to do, set up other parameters
	*/
		switch( tnum ) {
		case SCATU_REQUEST_SENSE    :
			tucb.scsitu.cmd_timeout = 30;
			tucb.scsitu.flags |= B_READ;
			tucb.scsitu.scsi_cmd_blk[4] = BUFFER_LENGTH;
			tucb.scsitu.data_length = BUFFER_LENGTH;
			tucb.scsitu.data_buffer = tu_buffer;
			memset( tu_buffer, 0x0ff, BUFFER_LENGTH );
			break;
		case SCATU_INQUIRY          :
			tucb.scsitu.flags |= B_READ;
			tucb.scsitu.cmd_timeout = 30;
			tucb.scsitu.scsi_cmd_blk[4] = BUFFER_LENGTH;
			tucb.scsitu.data_length = BUFFER_LENGTH;
			tucb.scsitu.data_buffer = tu_buffer;
			break;
		default :
			return ( SCATU_BAD_PARAMETER );
		}
		rc = exectu( tu_fdes, &tucb );
		return( rc );
	}  /*  end of function tu_test */
