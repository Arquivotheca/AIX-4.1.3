static char sccsid[] = "@(#)84	1.12  src/bos/diag/da/sdisk/dh2dasd.c, dasdisk, bos411, 9428A410j 6/15/94 10:46:05";
/*
 *   COMPONENT_NAME: DASDISK
 *
 *   FUNCTIONS: check_rc
 *		clean_up
 *		create_frub
 *		diag_tst
 *		get_adapter_name
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
#include <stdio.h>
#include <signal.h>
#include <sys/buf.h>
#include <sys/cfgodm.h>
#include <sys/devinfo.h>
#include <sys/device.h>
#include <sys/errids.h>
#include <sys/ioctl.h>
#include <sys/serdasd.h>
#include <sys/sysconfig.h>

/*#define	HAR2TEST*/

/* External functions used in this file */

extern                  getdainput();
extern          long    getdamode();
extern          int     exectu();
extern          int     scsitu_init();
extern nl_catd diag_catopen(char *, int);
extern		void	db(int,...);

/* Global variables and structures used in this file.  */

nl_catd                 catd;                   /* message file catalog */
int			logical_unit;
char                    *devname=(char *)0;   /* /dev/xxx name used in openx */
static  char            dname[]="/dev/r";       /* constant /dev/ */
static  short           set_self_test_bit=TRUE;
static	int		previous_step=1;  /* Keep track of previous command */
struct  fru_bucket      frub[] =             /* Frub  used when found failure */
{
	{  "", FRUB1,-1, 0x0, DSDASD_MSG1, {
		{ 0, "", "", 0, 0,0},
		},
		}
	};

static  char            adapter_name[NAMESIZE];
static  int             fdes = -99;
static  short           i_lvmed_it = 0;    /* Flag to indicate LVM is called */
static  short           init_adapter_state= -1; /* init state of adapter      */
static  short           init_controller_state= -1;/* initial  state of parent */
static  short           init_config_state=-1;  /* initial configuration state */
struct  tm_input        tm_input;         /* DA test parameter and other info */
HAR2_TUTYPE             tucb;               /* structure for SCSI command */
uchar           tu_buffer[BUFFER_LENGTH]; /* to hold data from SCSI command */

/* Constant definition of steps used in testing of device */

#define         TOTAL_STEPS     13
#define         EXIT_STEP       0
#define		START_UNIT	1
#define         RESERVE_UNIT    2
#define         MODE_SELECT     3
#define         SEND_DIAGNOSTIC 4
#define         FORCE_POST      5
#define		TEST_UNIT_READY	6
#define		CLEAR_SENSE	7
#define         RELEASE_UNIT    8
#define         REQUEST_SENSE   9
#define         ADAPTER_FAILS   10
#define         QUIESCE         11
#define         RSVD_WARNING    12
#define         FENCED_OUT	13

/*  */
/*
 * NAME: main()
 *
 * FUNCTION: Main routine for the Serial disk DASD Diagnostic Application.
 *           This routine is invoked by the Diagnostic Controller.
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
 *          if a problem is found with the device a FRUB is generated.
 */

main(argc, argv, envp)
int     argc;
char    **argv;
char    **envp;
{

	int       check_rc();
	void      clean_up();
	int       get_adapter_name();
	int       create_frub();
	void      diag_tst();
	int	  get_lun();
	void	  int_handler(int);
	int       tu_test();
	int       errno_rc, envflag;
	int       rc, diskette_based=FALSE;
	int       srn=0;
	struct	  sigaction	act;

	db(1);
	setlocale( LC_ALL, "" );

			
	/* set up interrupt handler     */
	act.sa_handler = int_handler;
	sigaction(SIGINT, &act, (struct sigaction *)NULL);


	/* variables initialization */

	damode = (long)NULL;
	DA_SETRC_TESTS(DA_TEST_FULL);

	/* Open and initialize odm */

	if(init_dgodm() == -1) {
		DA_SETRC_ERROR ( DA_ERROR_OTHER );
		clean_up();
	}
	/* Get tm_input and da_mode */

	if( getdainput( &tm_input ) != FALSE) {
		DA_SETRC_ERROR ( DA_ERROR_OTHER );
		clean_up();
	} else {
		devname = (char *)malloc( NAMESIZE + strlen(dname) + 1);
		if(devname != NULL)
			strcpy( devname, dname );
		else {
			DA_SETRC_ERROR( DA_ERROR_OTHER );
			clean_up();
		}
		strcat( devname, tm_input.dname );
	}
	db(20,1,"Device name",tm_input.dname);
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
	/* set up name of adapter */
	(void)get_adapter_name(tm_input.parent);

	/* Check presence of microcode file */

	if((damode & DA_LOOPMODE_NOTLM) || (damode & DA_LOOPMODE_ENTERLM)){
		envflag = ipl_mode(&diskette_based);
		db(10,1,"diskette_base flag",diskette_based);
		if ((diskette_based == DIAG_FALSE)  &&
			(damode & DA_CONSOLE_TRUE))
		    if ((rc = check_microcode_file(adapter_name)) != 0)
		    {
			db(10,1,"check microcode retrun",rc);			
			(void)disp_menu(NO_MICROCODE_TITLE);
		    }
	}
	/* Run ELA first. If no failure detected, then run all tests. */

	if(!( damode & DA_DMODE_REPAIR ) && !( damode & DA_LOOPMODE_INLM )
	    && !(damode & DA_LOOPMODE_EXITLM) )
		srn = analyze_error_log(DISK);

	if(srn != 0) {
		create_frub(srn);
		clean_up();
	}
	if( damode & DA_DMODE_ELA )
		clean_up();
	else {   /* No error from ELA or REPAIR VERIFICATION mode */
		if( !(damode & DA_LOOPMODE_EXITLM) ) {
			(void)display_title(DISK);
			init_adapter_state = configure_device(adapter_name);
			if(init_adapter_state == -1) {
				create_frub( ADAPTER_CONFIG_ERR );
				clean_up();
			}
			db(20,1,"Device parent",tm_input.parent);
			init_controller_state = configure_device(tm_input.parent);
			if(init_controller_state == -1) {
				create_frub( CONTROLLER_CONFIG_ERR );
				clean_up();
			}
			init_config_state = configure_device(tm_input.dname);
			if(init_config_state == -1) {
				create_frub( DASD_CONFIG_ERR );
				clean_up();
			}
			fdes = openx(devname, O_RDWR, NULL, SC_DIAGNOSTIC);
			errno_rc = errno;
			db(10,3,"openx fdes",fdes,"openx errno",
			   errno_rc,"At line",__LINE__);
			/* Run all tests if device can be opened. */

			if(fdes < 0) {
				if ((errno_rc == EBUSY) ||
				    (errno_rc == EACCES)) {
					DA_SETRC_ERROR( DA_ERROR_OPEN );
					clean_up();
				} else {
					DA_SETRC_STATUS( DA_STATUS_BAD );
					create_frub( DISK_OPENX_FAIL  );
					clean_up();
				}
			} else {
				chk_terminate_key();
				diag_tst(fdes);
				chk_terminate_key();
			}
		}
	}
	clean_up();     /* No return from clean_up */
	return(0);      /* for lint */

} /* end main */

/*  */
/*
 * NAME: check_rc()
 *
 * FUNCTION: Examine return code from tu_test and determine action to be
 *           taken.
 *
 * EXECUTION ENVIRONMENT: This function can page fault.
 *
 * RETURNS: 0 end of test. non zero, next test.
 */

int
check_rc( current_step, rc )
int     current_step;
int     rc;
{

	int     step;

	/* save the current step */
	previous_step = current_step;
	db(10,3,"check_rc, current_step",current_step,"rc",rc,"Line #",__LINE__);
	switch ( rc ) {
	case	SCATU_GOOD	: 
	/* case where check condition is expected, but good */
	/* status was instead returned.			    */
		step = create_frub( INVALID_SENSE_DATA );
		break;
	case    SCATU_TIMEOUT   :
		step = create_frub( ADAPTER_TIMEOUT );
		break;
	case    SCATU_RESERVATION_CONFLICT      :
		if( (current_step == RESERVE_UNIT) ||
		    (current_step == START_UNIT) )
			step = RSVD_WARNING;
		else
			if(current_step == REQUEST_SENSE)
				step = create_frub(INVALID_SCSI_STATUS);
			else
				step = create_frub(INVALID_RESERVATION);
		break;
	case    SCATU_CHECK_CONDITION   :
		if((current_step == REQUEST_SENSE) ||
		    (current_step == CLEAR_SENSE))
			step = create_frub( CHECK_READING_SENSE );
		else
			step = REQUEST_SENSE;
		break;
	case    SCATU_NONEXTENDED_SENSE :
		step = create_frub( INVALID_SENSE_DATA );
		break;
	case    SCATU_QUEUE_FULL        :
		if( (current_step == RESERVE_UNIT) || 
		    (current_step == START_UNIT) )
			step = QUIESCE;
		else
			step = create_frub( INVALID_Q_FULL );
		break;
	case    SCATU_IO_BUS_ERROR      :
		step = create_frub( BUS_ERROR   );
		break;
	case    SCATU_ADAPTER_FAILURE   :
		step = ADAPTER_FAILS;
		break;
	case    SCATU_BAD_PARAMETER     :
		step = create_frub( INVALID_PARAMS );
		break;
	default         :
		step = create_frub( INVALID_RETURN_CODE );
		break;
	}
	db(10,2,"check_rc, return step",step,"Line #",__LINE__);
	return( step );
}

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

	term_dgodm();                    /* terminate the ODM */

	db(25,"In Clean_up");
	if ( fdes > 0 ) {                /* Close device, if open */
		close(fdes);
	}
	db(10,1,"clean_up, line #",__LINE__);
	if ( init_config_state >= 0 ){
		rc = initial_state(init_config_state, tm_input.dname);
	}
	if ( init_controller_state >= 0 ){
		rc = initial_state(init_controller_state, tm_input.parent);
	}
	if ( init_adapter_state >= 0 ){
		rc = initial_state(init_adapter_state, adapter_name);
	}
	db(10,1,"clean_up, line #",__LINE__);

	if( damode & DA_CONSOLE_TRUE ){
		if (catd != CATD_ERR)        /* Close catalog files */
			catclose( catd );
		diag_asl_quit( NULL );    /* Terminate the ASl stuff */
	}

	DA_EXIT();              /* Exit to the Diagnostic controller */
} /* endfunction clean_up */

/*  */
/*
 * NAME: get_adapter_name()
 *
 * FUNCTION: Find the name of the Serial Disk Adapter.
 *
 * EXECUTION ENVIRONMENT: This function can page fault.
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: -1 unsuccessful, or 0 for successful completion
 *
 */

int
get_adapter_name(controller_name)
char    *controller_name;
{
	char    odm_search_crit[80];
	struct  CuDv    *cudv;
	struct  listinfo        obj_info;

	db(10,1,"get_adapter, line #",__LINE__);
	db(20,1,"Device controller name",controller_name);
	sprintf( odm_search_crit, "name = %s", controller_name);
	cudv = get_CuDv_list( CuDv_CLASS, odm_search_crit, &obj_info, 1, 2);
	if (cudv == (struct CuDv *) -1)
		return(-1);
	strcpy(adapter_name, cudv->parent);
	db(20,1,"Device adapter name",adapter_name);
	return( 0 );
}

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

	db(25,"In create_frub()");
	frub[0].rcode = srn;
	/* insert then add frub for Diagnostic Controller */
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
	return(EXIT_STEP);

} /* end create_frub */

/*  */
/* NAME: diag_tst()
 *
 * FUNCTION: Performs the tests on the DASD as outlined in the
 *           Problem Determination Guide.
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

void
diag_tst(fdes)
int     fdes;
{
	int             errno_rc;
	int             rc;
	int             srn;
	int             stepcount[TOTAL_STEPS+1];
	int		fence=0;
	register        int     step;

	struct  sd_ioctl_parms  sd_ioctl;

	(void)memset(&sd_ioctl,0,sizeof(struct sd_ioctl_parms));
	(void)memset(&stepcount,0,(TOTAL_STEPS+1)*sizeof(int));
	sd_ioctl.time_out = 60;
	rc = 0;
	logical_unit = get_lun(tm_input.dname);
	db(10,2,"logical_unit",logical_unit,"Line #",__LINE__);
	if(logical_unit == -1) {
		DA_SETRC_ERROR( DA_ERROR_OTHER );
		return;
	}
	step = START_UNIT;
	while(step) {
		switch( step ) {
		case	START_UNIT	:
			db(10,1,"START_UNIT, Line #",__LINE__);
			++stepcount[START_UNIT];
			rc = tu_test(fdes, SCATU_START_STOP_UNIT);
			if( rc == SCATU_GOOD )
				step = RESERVE_UNIT;
			else
				step = check_rc( step, rc );
			break;
		case    RESERVE_UNIT:
			db(10,1,"RESERVE_UNIT, Line #",__LINE__);
			++stepcount[RESERVE_UNIT];
			rc = tu_test(fdes, SCATU_RESERVE_UNIT);
			if( rc == SCATU_GOOD )
				step = MODE_SELECT;
			else
				step = check_rc( step, rc );
			break;
		case    MODE_SELECT     :
			db(10,1,"SELECT_MODE, Line #",__LINE__);
			++stepcount[MODE_SELECT];
			rc = tu_test( fdes, SCATU_MODE_SELECT );
			if( rc == SCATU_GOOD )
				step = SEND_DIAGNOSTIC;
			else
				step = check_rc( step, rc );
			break;
		case    SEND_DIAGNOSTIC :
			db(10,1,"SEND_DIAGNOSTIC, Line #",__LINE__);
			++stepcount[SEND_DIAGNOSTIC];
			rc = tu_test( fdes, SCATU_SEND_DIAGNOSTIC );
			if( rc == SCATU_GOOD ){
				step = FORCE_POST;
				set_self_test_bit=FALSE;
			} else
				step = check_rc( step, rc );
			break;
		case    FORCE_POST      :
			db(10,1,"FORCE_POST, Line #",__LINE__);
			++stepcount[FORCE_POST];
			/* set tucb to have slftst = 0 and parm = 81 */
			rc = tu_test( fdes, SCATU_SEND_DIAGNOSTIC );
			if( rc == SCATU_GOOD )
				step = TEST_UNIT_READY;
			else
				step = check_rc(step, rc);
			break;
		case	TEST_UNIT_READY	:
			db(10,1,"TEST_UNIT_READY, Line #",__LINE__);
			++stepcount[TEST_UNIT_READY];
			rc = tu_test( fdes, SCATU_TEST_UNIT_READY );
			/* If first pass then expect Check condition */
			/* Else need to do request sense and analyse */
			/* sense data.				     */
			if(stepcount[TEST_UNIT_READY] == 1)
				if( rc == SCATU_CHECK_CONDITION )
					step = CLEAR_SENSE;
				else
					step = check_rc(step, rc);
			else
				if( rc == SCATU_GOOD )
					step = RELEASE_UNIT;
				else
					step = check_rc(step, rc);
			break;
		case	CLEAR_SENSE	:
			db(10,1,"CLEAR_SENSE, Line #",__LINE__);
			++stepcount[CLEAR_SENSE];
			rc = tu_test( fdes, SCATU_REQUEST_SENSE );
			if( rc == SCATU_GOOD )
				/* retest to catch error */
				step = TEST_UNIT_READY;
			else
				step = check_rc(step, rc);
			break;
		case    RELEASE_UNIT    :
			db(10,1,"RELEASE_UNIT, Line #",__LINE__);
			++stepcount[RELEASE_UNIT];
			rc = tu_test( fdes, SCATU_RELEASE_UNIT );
			if( rc == SCATU_GOOD )
				step = EXIT_STEP;
			else 
				step = check_rc( step, rc );
			break;
		case    REQUEST_SENSE   :
			db(10,1,"REQUEST_SENSE, Line #",__LINE__);
			++stepcount[REQUEST_SENSE];
			rc = tu_test( fdes, SCATU_REQUEST_SENSE );
			if( rc == SCATU_GOOD )
				/* look at HAC and QUIESCE if 1st time*/
				/* else examine UEC.                  */
				if( (tucb.scsiret.host_action_code == 8) ||
				    (tucb.scsiret.host_action_code == 0x0c) )
					if(stepcount[REQUEST_SENSE] > 1)
					/* More than 1 pass through this  */
					/* call out srn.	          */
						step = create_frub
						  (CNTRL_FAULT_QUIESCE);
					/* else do not change step so we can */
					/* retry the original command.       */
					else
						step = previous_step;
				else
					step = create_frub(tucb.scsiret.unit_error_code);
			else
				step = check_rc( step, rc );
			break;
		case    ADAPTER_FAILS   :
			db(10,1,"ADAPTER_FAILS, Line #",__LINE__);
			++stepcount[ADAPTER_FAILS];
			if( tucb.scsiret.status_validity & 0x4 )
				if(tucb.scsiret.alert_register_adap ==
					SD_CTRL_STATUS)
				    /* Controller special status */
	 			    if (stepcount[QUIESCE] == 0) 
					step = QUIESCE;
				    else
					srn = ADAPTER_FAILURE + 
					    tucb.scsiret.alert_register_adap;
				else
					srn = ADAPTER_FAILURE + 
					    tucb.scsiret.alert_register_adap;
			else
				srn = PIO_ERROR;
			if(step == ADAPTER_FAILS)
				step = create_frub( srn );
			break;
		case    QUIESCE :
			db(10,1,"QUIESCE, Line #",__LINE__);
			++stepcount[QUIESCE];
			if(stepcount[QUIESCE] == 1){
				sd_ioctl.reset_type = SD_QUIESCE_OP;
				rc = ioctl(fdes, SD_RESET, &sd_ioctl);
				errno_rc = errno;
				if((rc == 0) &&
				     (sd_ioctl.status_validity == 0x00))
					step = START_UNIT;
				else{
					if(sd_ioctl.status_validity & 0x01)
						srn=ADAPTER_FAILURE +
						    sd_ioctl.adapter_status;
					else
						srn = CNTRL_FAULT_QUIESCE;
					step = create_frub( srn );
				}
			} else
				step = create_frub( CNTRL_FAULT_RSV_UNRESET );
			break;
		case    RSVD_WARNING    :
			db(10,1,"RSVD_WARNING, Line #",__LINE__);
			++stepcount[RSVD_WARNING];
			if( stepcount [RSVD_WARNING] <= 1){
				if((!tm_input.system)
				     &&(!(damode & DA_LOOPMODE_INLM))
				     &&(damode & DA_CONSOLE_TRUE)) {
					(void)disp_menu(DO_WE_CONTINUE2);
					if(menu_return != 1){ /* Yes response */
						sd_ioctl.reset_type=SD_RESET_OP;
						sd_ioctl.status_validity = 0x00;
						db(10,1,"ioctl to reset, Line #",__LINE__);
						rc=ioctl(fdes,SD_RESET, &sd_ioctl);
						errno_rc = errno;
						fence=support_fence();
						db(10,1,"fence supported ",fence);
						if(rc==0)
							step =fence?FENCED_OUT:RESERVE_UNIT;
						else
						    step=create_frub(
						    CNTRL_FAULT_RSV_UNRESET);
					} else
						step = EXIT_STEP;
				} else
					/* No console or system checkout */
					step = create_frub(
						    CNTRL_FAULT_RSV_UNRESET);
			} else
				/* Thru this path more than once */
				step = create_frub(CNTRL_FAULT_RSV_UNRESET);
			break;
		case FENCED_OUT:
			db(25,"BREAK Fence");
			++stepcount[FENCED_OUT];
			rc = tu_test( fdes, SCATU_BREAK_FENCE );
			if( rc == SCATU_GOOD ){
				step = START_UNIT;
			} else
				step = check_rc( step, rc );
			break;
			
			
		default         :
			db(10,1,"default step, Line #",__LINE__);
			break;
		}
	}
}


/*  */
/*
 * NAME: get_lun()
 *
 * FUNCTION: Given a logical name, determine the lun id by searching in
 *           the CuDv object class.
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
	db(10,2,"get_lun, return",cudv->connwhere[0]-'0',"Line #",__LINE__);
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
	db(25,"In int_handler()");
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
	int      blocks;
	char	 *test_unit;
	char	 *return_code;
	int	 tu;
	int      step;
	int      rc;                     /* function return code      */

	(void)memset(&tucb,0,sizeof(HAR2_TUTYPE));
	tucb.header.tu = tnum;    /* put TU number to do in TU struct */
	tucb.header.mfg = 0;     /* always set to 0, only used by mfg */

	db(16,1,"scsitu_init, tu number",tnum);
	rc = scsitu_init ( &tucb );
	db(10,2,"scsitu_init, returned number",rc,"Line #",__LINE__);
	if( rc != SCATU_GOOD )
		return ( SCATU_BAD_PARAMETER );
	tucb.scsitu.flags = 0;
	tucb.scsitu.ioctl_pass_param = PASS_PARAM;
	tucb.scsitu.data_length = 0;
	tucb.scsitu.data_buffer = NULL;
	tucb.scsitu.cmd_timeout = 50;
	tucb.scsitu.resvd5 = 0;

/*
  Based on TU to do, set up other parameters
*/
	switch( tnum ) {
	case SCATU_TEST_UNIT_READY  :
		tucb.scsitu.cmd_timeout = 90;
		break;
	case SCATU_START_STOP_UNIT  :
		tucb.scsitu.cmd_timeout = 90;
		break;
	case SCATU_MODE_SELECT  :
		tucb.scsitu.flags |= B_WRITE;
		/* MODE SELECT HEADER */
		tu_buffer[0] = 0x00;                /* Rsvd=0         */
		tu_buffer[1] = 0x00;                /* Medium Type=0  */
		tu_buffer[2] = 0x00;                /* Rsvd=0         */
		tu_buffer[3] = 0x08;               /* Blk Desc lgth   */
		/* Block Descriptor */
		tu_buffer[4] = 0x00;
		tu_buffer[5] = 0x00;
		tu_buffer[6] = 0x00;
		tu_buffer[7] = 0x00;
		tu_buffer[8] = 0x00;
		tu_buffer[9] = 0x00; /* Block length MSB */
		tu_buffer[10] = 0x02;
		tu_buffer[11] = 0x00;/* Block length LSB */
		/* Page 0 Descriptor */
		tu_buffer[12] = 0x00;
		tu_buffer[13] = 0x01;
		tu_buffer[14] = 0x00; /* QPE set to 1 */
		/* Page 1 Descriptor */
		tu_buffer[15] = 0x01;
		tu_buffer[16] = 0x01;
		tu_buffer[17] = 0x20; /* TB bit set */
		tucb.scsitu.data_length = 18;
		tucb.scsitu.scsi_cmd_blk[4] = 18;
		tucb.scsitu.data_buffer = tu_buffer;
		break;
	case SCATU_SEND_DIAGNOSTIC  :
		tucb.scsitu.cmd_timeout = 200;
		tucb.scsitu.flags |= B_WRITE;
		if(set_self_test_bit) {
			/* set self test bit */
			db(10,1,"self test bit, Line #",__LINE__);
			tucb.scsitu.scsi_cmd_blk[1] = 0x04;
			tucb.scsitu.scsi_cmd_blk[3] = 0x00;
			tucb.scsitu.scsi_cmd_blk[4] = 0x00;
		} else {
			/* clear self test bit */
			db(10,1,"clear self test bit, Line #",__LINE__);
			tucb.scsitu.scsi_cmd_blk[1] = 0x00;
			tucb.scsitu.scsi_cmd_blk[4] = 4;
			tu_buffer[0] = 0x81;
			tu_buffer[1] = 0x00;
			tu_buffer[2] = 0x00;
			tu_buffer[3] = 0x00;
			tucb.scsitu.data_buffer = tu_buffer;
			tucb.scsitu.data_length = 4;
		}
		break;
	case SCATU_REQUEST_SENSE    :
		tucb.scsitu.flags |= B_READ;
		tucb.scsitu.scsi_cmd_blk[4] = BUFFER_LENGTH;
		tucb.scsitu.data_length = BUFFER_LENGTH;
		tucb.scsitu.data_buffer = tu_buffer;
		memset( tu_buffer, 0x0ff, BUFFER_LENGTH );
		break;
	case SCATU_RESERVE_UNIT     :
	case SCATU_RELEASE_UNIT     :
		break;
	default :
		return ( SCATU_BAD_PARAMETER );
	}
	tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;

	db(2,&tucb);

	rc = exectu( tu_fdes, &tucb ); 
	db(16,1,"exectu returned",rc);
	if((rc == SCATU_GOOD) && (tucb.header.tu == SCATU_REQUEST_SENSE))
	{
		db(3,&tucb);
	}
	return( rc );
}  /*  end of function tu_test */

/*
 * NAME: support_fence()
 *
 * FUNCTION: This function is designed to check if the controller support
 *	     fencing.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: This function returns value found in the VPD TM9333 field.
 */

int
support_fence()
{
	char	criteria[255];
	struct	CuVPD	*cuvpd;
	struct	listinfo	obj_info;
	char	*string;
	char	tm_fields[5]={NULL};

	/* Get the VPD data and determine the	*/
	/* type of controller in the system.	*/

	sprintf(criteria,"name = %s" ,tm_input.dname);
	cuvpd = get_CuVPD_list(CuVPD_CLASS,criteria,
		&obj_info,1,2);
	if (cuvpd == (struct CuVPD *) -1 ||
		cuvpd == (struct CuVPD *) NULL)
	{
		/* No data in CuVPD or an error	*/
		/* status returned		*/

		DA_SETRC_ERROR(DA_ERROR_OTHER);
		clean_up();
	}

	/* Search the VPD for a TM Field and	*/
	/* copy the controller type into a 	*/
	/* tm_field				*/

	string=strstr(cuvpd->vpd,"9333-");
	sprintf(tm_fields,"%c%c%c",string[5],
		string[6],string[7]);

	/* If the value of tm_field == 0 then	*/
	/* we have an old controller.		*/

	return(atoi(tm_fields));
}
