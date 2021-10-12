static char sccsid[] = "@(#)11  1.2  src/bos/diag/da/isakbd/disakbd.c, daisakbd, bos41J, 9520A_all 5/16/95 16:00:59";
/*
 *   COMPONENT_NAME: DAKBD
 *
 *   FUNCTIONS: IS_TM
 *		OpenDevice
 *		checkerrorcode
 *		chk_asl_stat
 *		chk_ela
 *		chk_terminate_key
 *		cleanup
 *		findfru
 *		report_fru
 *		main
 *		putupmenu
 *		runtu
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<stdio.h>
#include	<fcntl.h>
#include	<nl_types.h>
#include	<memory.h>
#include	<locale.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/ioctl.h>
#include	<sys/devinfo.h>
#include	<sys/termio.h>
#include	<sys/errno.h>
#include	<sys/inputdd.h>

#include	"diag/class_def.h"
#include	"diag/diago.h"
#include	"diag/diag_exit.h"
#include	"diag/tm_input.h"
#include	"diag/tmdefs.h"
#include	"diag/da.h"
#include	"diag/atu.h"
#include	"diag/dcda_msg.h"
#include	"disakbd.h"
#include	"disakbd_msg.h"
 

#define OPEN_MODE (O_RDWR)	/* device mode: read/write		  */

#define OPEN_DD (open(devname,OPEN_MODE)) /* System call to open	  */
					    /* device 			  */

/**************************************************************************/
/* IS_TM is a macro that accepts two input variables VAR1 & VAR2.         */
/* VAR1 is an object of the sturcture tm_input.			          */
/* VAR2 is a variable or a defined value that will be compared to the     */
/*      tm_input object class variable.                                   */
/* NOTE: macro returns logical TRUE or FALSE				  */
/**************************************************************************/

#define IS_TM( VAR1, VAR2 ) ( (int) ( tm_input./**/VAR1 ) == /**/VAR2 )

/**************************************************************************/
/* NONI_NOT_EXECUTE is a macro that defines all the modes where the       */
/*		   non-interactive tus will not execute.                  */
/*		   Will not run if in exit loop mode.                     */
/* NOTE: macro returns logical TRUE or FALSE				  */
/**************************************************************************/

#define NONI_NOT_EXECUTE (  	IS_TM( loopmode, LOOPMODE_EXITLM  ))

#define RUN_NON_INTERACTIVE_TESTS ( !(NONI_NOT_EXECUTE ) )

/**************************************************************************/
/* INT_NOT_EXECUTE is a macro that defines all the modes where the        */
/*		   interactive tests will not execute.                    */
/*		   This TU is user interactive.				  */
/*                 Cannot be run during system test mode.                 */
/*                 Cannot be run without a console.                       */
/*		   Will not run if in exit loop mode.                     */
/*                 Will not run if in loop mode.                          */
/* NOTE: macro returns logical TRUE or FALSE				  */
/**************************************************************************/

#define INT_NOT_EXECUTE	( 	IS_TM( system, SYSTEM_TRUE  )	   \
		 	|| 	IS_TM( console, CONSOLE_FALSE  )	   \
		 	|| 	IS_TM( loopmode, LOOPMODE_EXITLM  )	   \
		 	|| 	IS_TM( loopmode, LOOPMODE_INLM  )	   \
			)

#define RUN_INTERACTIVE_TESTS ( ! ( INT_NOT_EXECUTE ) )

/**************************************************************************/
/* ALL_TUS_CAN_EXECUTE  is a macro that returns TRUE if conditions exist  */
/*		       that will allow all of the TUs to execute.         */
/* NOTE: macro returns logical TRUE or FALSE				  */
/**************************************************************************/

#define ALL_TUS_CAN_EXECUTE (    RUN_NON_INTERACTIVE_TESTS		\
			   &&    RUN_INTERACTIVE_TESTS 			\
			    )

/**************************************************************************/
/* TUS_CAN_EXECUTE      is a macro that returns TRUE if conditions exist  */
/*		       that will allow at least one of the TUs to execute.*/
/* NOTE: macro returns logical TRUE or FALSE				  */
/**************************************************************************/

#define TUS_CAN_EXECUTE     (   RUN_NON_INTERACTIVE_TESTS		\
			   ||   RUN_INTERACTIVE_TESTS			\
			    )

/**************************************************************************/
/* ADVANCED is a macro that returns TRUE if the customer selected         */
/*		advanced mode of operation.                               */
/* NOTE: macro returns logical TRUE or FALSE				  */
/**************************************************************************/

#define ADVANCED		(  					   \
				        IS_TM( advanced,ADVANCED_TRUE )    \
				)

/**************************************************************************/
/* DISPLAY_TESTING is a macro that returns TRUE if conditions exist that  */
/*			will allow the display of the testing device      */
/*			message to the display.                           */
/* NOTE: macro returns logical TRUE or FALSE				  */
/**************************************************************************/

#define DISPLAY_TESTING		(  					   \
					IS_TM( loopmode,LOOPMODE_NOTLM )   \
				&&	IS_TM( console,CONSOLE_TRUE )	   \
				)

/**************************************************************************/
/* DISPLAY_LOOPING is a macro that returns TRUE if conditions exist that  */
/*			will allow the display of the testing device      */
/*			in loop mode message to the display.              */
/* NOTE: macro returns logical TRUE or FALSE				  */
/**************************************************************************/

#define DISPLAY_LOOPING		(  					   \
				      (					   \
					IS_TM( system,SYSTEM_FALSE )	   \
				    &&  (				   \
					IS_TM( loopmode,LOOPMODE_INLM )    \
				     || IS_TM( loopmode,LOOPMODE_EXITLM )  \
					)				   \
				    && 	IS_TM( console,CONSOLE_TRUE )	   \
				      )					   \
				||    (					   \
					IS_TM( system,SYSTEM_TRUE )	   \
				    && !(IS_TM( loopmode,LOOPMODE_NOTLM )) \
				    && 	IS_TM( console,CONSOLE_TRUE )	   \
				      )					   \
				)

/**************************************************************************/
/* DISPLAY_ADVANCED is a macro that returns TRUE if conditions exist that */
/*			will allow the display of the testing device      */
/*			message to the display.                           */
/* NOTE: macro returns logical TRUE or FALSE				  */
/**************************************************************************/

#define DISPLAY_ADVANCED	(  					   \
			            (	IS_TM( loopmode,LOOPMODE_NOTLM )   \
				     || IS_TM( loopmode,LOOPMODE_ENTERLM ) \
				    )					   \
				&&	IS_TM( console,CONSOLE_TRUE )	   \
				&&	IS_TM( advanced,ADVANCED_TRUE )	   \
				)


/***********************************************************************/
/*	frub is an array of fru_bucket structures.  fru_bucket defines */
/*	the information related to the frus which are called out by    */
/*	the diag supervisor.                                           */
/***********************************************************************/
 
struct fru_bucket frub[]=
{

	{ "", FRUB1,  0x921  ,  0  , 0 , 
				{
					{ 100 , "" , "" , 0 , PARENT_NAME},
				},
	},

	{ "", FRUB1,  0x921  ,  0  , 0 ,
				{
					{ 70 , "" , "" , 0 , DA_NAME	},
					{ 30 , "" , "" , 0 , PARENT_NAME},
				},
	},

	{ "", FRUB1,  0x921  ,  0  , 0  , 
				{
					{ 90 , "" , "" , 0 , DA_NAME	},
					{ 10 , "" , "" , 0 , PARENT_NAME},
				},
	},

	{ "", FRUB1,  0x921  ,  0  , 0  , 
				{
					{ 90 , "" , "" , 0 , PARENT_NAME},
					{ 10 , "" , "" , 0 , DA_NAME    },
				},
	},

	{ "", FRUB2,  0x921  ,  0  , 0  ,
				{
					{ 100 , "" , "" , 0 , DA_NAME	},
				},
	},

};

struct fru_bucket frub_config[] =
{
	{ "", FRUB1,  0 , 0x701  , DKBD_CONF ,
			{
				{ 80 , "" , "" , 0 , DA_NAME, EXEMPT     },
				{ 20 , "" , "" , 0 , PARENT_NAME, EXEMPT },
			},
	},
};

/**************************************************************************/
/*	The tudata structure defines a lookup table used to map the       */
/*	valid return code from a test unit to the fru bucket information  */
/*	to return to the diag supervisor.				  */
/**************************************************************************/

struct	tudata 
{
	int	turc;
	struct	fru_bucket	*fbptr;
	int	reason_code;
	int	reason_msg;
};

struct  tudata tu0_rc[] =
{
	{ 0xF002, &frub[3],  0x103, DKBD_RESET_ERR     },
	{ 0xF003, &frub[3],  0x103, DKBD_RESET_ERR     },
        { (int)NULL, NULL, (int)NULL, (int)NULL    },
};

struct  tudata tu1_rc[] = 
{
	{ 0x1001, &frub[3],  0x105, DKBD_LON_ERR    },
	{ 0x1002, &frub[3],  0x106, DKBD_LOFF_ERR    },
	{ (int)NULL, NULL, (int)NULL, (int)NULL    },
};
	
struct  tudata tu2_rc[] = 
{
	{ 0x2001, &frub[1],  0x203, DKBD_KBID_ERR	},
        { 0x2002, &frub[1],  0x205, DKBD_ECHO_ERR     },
	{ 0x2003, &frub[1],  0x206, DKBD_SCAN_ERR     },
	{ (int)NULL, NULL, (int)NULL, (int)NULL    },
};

struct  tudata tu3_rc[] = 
{
	{ 0x3001, &frub[4],  0x303, DKBD_LON_ERR	},
	{ 0x3002, &frub[4],  0x304, DKBD_LOFF_ERR	},
	{ (int)NULL, NULL, (int)NULL, (int)NULL    },
};

struct  tudata tu4_rc[] = 
{
	{ 0x4001, &frub[3],  0x404, DKBD_KBID_ERR	},
	{ 0x4002, &frub[3],  0x404, DKBD_KOOO_ERR	},
	{ (int)NULL, NULL, (int)NULL, (int)NULL    },
};

struct  tudata tu7_rc[] = 		/* fru offsets for ela errors	*/
{
	{ 0x71, &frub[0],  0x901, DKBD_ELA_ADAP	}, 
	{ 0x72, &frub[3],  0x902, DKBD_ELA_DEVICE	},
	{ 0x73, &frub[2],  0x903, DKBD_ELA_AORD	},
	{ (int)NULL, NULL, (int)NULL, (int)NULL    },
};

struct	tuinfo
{
	int	tunum;
	struct	tudata	*tudptr;
};

struct	tuinfo	testlist[] =
{
	{ 0x10,	&tu1_rc[0] },		/* testlist[0] non-interactive */
	{ 0x20,	&tu2_rc[0] },		/* testlist[1] non-interactive */
	{ (int)NULL, NULL },		/* testlist[2] no test */
	{ 0x30,	&tu3_rc[0] },		/* testlist[3] interactive */
	{ 0x40,	&tu4_rc[0] },		/* testlist[4] interactive */
	{ (int)NULL, NULL },		/* testlist[5] no test */
	{ 0x10,	&tu1_rc[0] },		/* testlist[6] non-interactive */
	{ (int)NULL, &tu0_rc[0] },	/* testlist[7] no test */
	{ (int)NULL, NULL },		/* testlist[8] no test */
};

struct	err_struct
{
	int	rc;
	struct	tuinfo	*tuptr;
};
struct	err_struct	e_return;
struct	err_struct	*esptr = &e_return;

struct	CuDv	*cudv;
struct	objlistinfo	c_info;
char	crit[100];

ASL_SCR_TYPE	menutypes= DM_TYPE_DEFAULTS;

struct	msglist	da_title[]=
	{
		{ ISAKBD_MSGS, TESTING },
		{ ISAKBD_MSGS, STANDBY },
		(int)NULL
	};

extern	nl_catd diag_catopen(char *, int);
struct	tm_input tm_input;

int	fdes = ERR_FILE_OPEN;		/* Machine dd file descriptor	   */
int	filedes = ERR_FILE_OPEN;        /* keyboard device file descriptor */
nl_catd	catd = CATD_ERR;		/* Pointer to the catalog file     */
int	kbid = 921;			/* Default device id from tu	   */
int	kbtype = 0;			/* Default device type		   */
int	device_state;			/* previous device status	   */
char	kbddevname[20];
uint	arg;

/*
 * NAME:
 *	main (dkbd)
 *                                                                    
 * FUNCTION:
 *	This function determines which functions to invoke and which
 *	test units to execute.  The determinations are based on 
 *	diagnostic environment variables returned via a function call.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This function is forked and exec'd by the diagnostic supervisor
 *	which runs as a process on the system.
 *	This function runs as an application under the diagnostic subsystem.
 *                                                                   
 * (NOTES:)
 *	Initializes the tm_input structure by getting a copy of the
 *	diagnostic environment variables.
 *	Exits if running in exit loop mode.
 *	Runs chk_ela if in ela mode or if running in a mode that
 *	disallows execution of the test units.
 *	Attempts to open the device to test, returning error if
 *	that open fails.
 *	Calls function runtu to execute a tu.
 *	Runs chk_ela after the tus if in problem determination mode.
 *	Calls cleanup and returns to caller. 
 *
 * (RECOVERY OPERATION:)
 *	The tu code handles any and all hardware recovery
 *	procedures in the event of error condition.
 *	Software errors, such as failures to open, are handled by
 *	setting error flags and returning to the diagnostic
 *	supervisor.
 *
 * (DATA STRUCTURES:)
 *	This function modifies the following global structures
 *		and variables:
 *	Initializes the tm_input structure.
 *	Initializes the DA_SETRC_ERROR variable.
 *	Initializes the DA_SETRC_STATUS variable.
 *	Initializes the DA_SETRC_TESTS variable.
 *	Initializes the DA_SETRC_MORE variable.
 *	Initializes the filedes variable.
 *	Initializes the catd variable.
 *
 * RETURNS:
 *	Any values returned are done so via the addfru() and DA_EXIT()
 *	calls.
 */  

main(argc,argv) 
int	argc;
char	*argv[];
{

	void	runtu();
	void	cleanup();
	void	OpenDevice();
	void	putupmenu();
	void	checkerrorcode();
	int	chk_ela();
	int	findfru();

	int	rc;
	unsigned long    f_code;

	setlocale(LC_ALL,"");

	if ( init_dgodm() == ERR_FILE_OPEN )
	{
		DA_SETRC_ERROR( DA_ERROR_OTHER );
		cleanup();
	}

	if ( ( rc = getdainput( &tm_input ) ) == BAD )
	{
		DA_SETRC_ERROR( DA_ERROR_OTHER );
		cleanup();
	}

/* Determine which type of keyboard we are testing. */
	sprintf (crit, "name = %s", tm_input.dname);
	cudv = get_CuDv_list (CuDv_CLASS, crit, &c_info, 1, 2);
	kbid = cudv->PdDvLn->led;

/* Assign the correct FFC to the fru bucket we might call out. */
	switch (kbid)
	{
	case SN101:
		kbtype = 0;
		break;
	case SN102:
		kbtype = 1;
		break;
	case SN106:
		kbtype = 2;
		break;
	case SNPS2:
		kbtype = 3;
		break;
	}  /* end switch (cudv->PdDvLn->led) */

	if ( IS_TM( console,CONSOLE_TRUE ) )
	{
		chk_asl_stat( diag_asl_init("NO_TYPE_AHEAD") );
		catd = diag_catopen( MF_DISAKBD,0 );

		if ( ADVANCED )
			da_title[0].msgid++;
		putupmenu();
		sleep(1);
	}
/*	
 *	Since no tests are run when in exit loop mode and this DA
 *	doesn't have any exit menus to display, just cleanup and
 *	exit.
 *	We take the defaults for the da_exit_code here because any prior
 *	errors have already been set in the object class by previous loops.
 */

	if ( IS_TM( loopmode,LOOPMODE_EXITLM ) )
		cleanup();

/*
 *	Run chk_ela if in ela mode.  
 *	Run chk_ela if we can't run anything else.
 */
 
/* REMOVE COMMENTS WHEN ELA WORKING  

	if ( ( IS_TM( dmode, DMODE_ELA ) ) || ( !( TUS_CAN_EXECUTE ) ) )
	{
		if ( ( rc = chk_ela() ) != 0 )
			findfru( ELA_PTR, rc );
			 
		else	
			cleanup();
	}
*/
	if ( ( IS_TM( dmode, DMODE_ELA ) ) || ( !( TUS_CAN_EXECUTE ) ) )
		cleanup();

	OpenDevice();

/*
 *	This section of code allows the execution of each of the
 *	test units if the following prerequisites are met:
 *
 *	That the current diagnostic environment will support the execution of
 *	the test unit.
 */

	DA_SETRC_TESTS( DA_TEST_FULL );

	if ( RUN_NON_INTERACTIVE_TESTS )
	{
 		runtu( &testlist[0] );
		if ( esptr->rc != 0 )
			checkerrorcode( esptr );
		if (IS_TM( loopmode,LOOPMODE_INLM ))
	 		sleep(3);
 	        (void) chk_terminate_key();
	}

	if ( RUN_INTERACTIVE_TESTS )
	{
 		runtu( &testlist[3] );
		putupmenu();
		if ( esptr->rc != 0 )
			checkerrorcode( esptr );
	}

/*
 *	It is necessary to run chk_ela if we have run the test units and
 *	no problem was found in problem determination mode.  The customer
 *	by selecting problem determination mode has said that they 
 *	perceive a problem with this system resource and at this point the
 *      test units were not able to find a problem.
 */

/* REMOVE COMMENTS WHEN ELA WORKING

	if ( ( IS_TM( dmode, DMODE_PD ) ) &&
		!( IS_TM( loopmode, LOOPMODE_INLM ) ) )
	{
		if ( ( rc = chk_ela() ) != 0 )
			findfru( ELA_PTR, rc );

	}
*/

	cleanup();

}  /* main end */
 

/*
 * NAME:
 *	runtu
 *                                                                    
 * FUNCTION:
 *	This function calls the test unit entry point, passing the
 *	necessary parameters.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This function is called by the main procedure which is forked and
 *	exec'd by the diagnostic supervisor, which runs as a process.
 *	This function runs an an application under the diagnostic subsystem.
 *                                                                   
 * (NOTES:)
 *	This function:
 *	Initializes the tu, mfg and loop variables of the test
 *	unit control block structure, tucb.
 *	Initializes additional flags in the tudata structure that
 *	are passed to the tu entry point.
 *	Calls the test unit entry point, exectu with a pointer to
 *	the tucb structure and the file descriptor as parameters.
 *	Returns to caller after setting rc and pointer information.
 *
 * (RECOVERY OPERATION:)
 *	The tu code handles any and all hardware recovery
 *	procedures in the event of error condition.
 *	Software errors, such as failures to open, are handled by
 *	setting error flags and returning to the diagnostic
 *	supervisor.
 *
 * (DATA STRUCTURES:)
 *	This function modifies the following global structures
 *		and variables:
 *	Initializes the DA_SETRC_ERROR variable.
 *	Initializes the DA_SETRC_USER variable.
 *	The structure pointed to by esptr.
 *
 * RETURNS:
 *	NONE
 */  

void	runtu( tuptr )
struct 	tuinfo	*tuptr;
{
 
	extern	exectu();
	int	findfru();
	void	cleanup();
   	int 	rc=0;
	
	struct  tucb
	{
		struct	tucb_t	header;
		struct	tucb_d	tuenv;
	}tuparams =
		{
			{ (int)NULL, (int)NULL, 1, (int)NULL, (int)NULL },
			{ NULL, (long)NULL, (int)NULL}

		};
	struct	tucb	*p=&tuparams;

	if ( IS_TM( advanced, ADVANCED_TRUE ) )
		p->tuenv.ad_mode = 1;       

	strcpy(p->tuenv.kbd_fd, kbddevname);
	p->tuenv.catd = catd;
	p->tuenv.kbtype = kbtype;

	for ( ; tuptr->tunum != (int)NULL; tuptr++ )
	{
		sleep(1);
		p->header.tu = tuptr->tunum;
		rc = exectu( fdes, p );
		if ( rc != 0 )
			break;
	}

	esptr->rc = rc & 0x8000FFFF;
	switch (esptr->rc)
	{
        	case 0xF001:
        	case 0xF00B:
		case 0xF00C:
		case 0xF00D:
                case 0xF00E:
                case 0xF00F:
                {
                	DA_SETRC_ERROR(DA_ERROR_OTHER);
                        cleanup();
                        break;
                }
                default:
                {
                	if (( esptr->rc >= 0xF002 ) && ( esptr->rc <= 0xF003 ))
				esptr->tuptr = &testlist[7];
			else
				esptr->tuptr = tuptr;
			break;
		}
	} /* switch end */	
}

/*
 * NAME:
 *	checkerrorcode
 *                                                                    
 * FUNCTION:
 *	This function switches on the return code from the execution of a
 *	tu and takes the appropriate action.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This function is called by the main procedure which is forked and
 *	exec'd by the diagnostic supervisor, which runs as a process.
 *	This function runs an an application under the diagnostic subsystem.
 *                                                                   
 * (NOTES:)
 *	This function:
 *	Checks the non-zero return code from a test unit and takes the
 *	appropriate action.
 *
 * (RECOVERY OPERATION:)
 *	The tu code handles any and all hardware recovery
 *	procedures in the event of error condition.
 *	Software errors, such as failures to open, are handles by
 *	setting error flags and returning to the diagnostic
 *	supervisor.
 *
 * (DATA STRUCTURES:)
 *	None modified.
 *
 * RETURNS:
 *	NONE
 */  

void	checkerrorcode( esptr ) 
struct	err_struct	*esptr;
{

	switch ( esptr->rc ) 
	{
		case CANCEL_KEY_ENTERED:
		{
			DA_SETRC_USER( DA_USER_QUIT );
			cleanup();
			break;
		}
		case EXIT_KEY_ENTERED:
		{
			DA_SETRC_USER( DA_USER_EXIT );
			cleanup();
			break;
		}
		default:	/* all error occurrences */
		{
			findfru( esptr->tuptr->tudptr, esptr->rc );
			break;
		}
	}  /* switch end */  
}  /* checkerrorcode end */
 

/*
 * NAME:
 *	chk_ela
 *                                                                    
 * FUNCTION:
 *	Read error log looking for entries for the keyboard device.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This function is called by the main procedure which is forked and
 *	exec'd by the diagnostic supervisor, which runs as a process.
 *	This function runs an an application under the diagnostic subsystem.
 *                                                                   
 * (NOTES:)
 *	Call the error log interface passing it the search criteria.
 *	Examine any returned error log entries and make fru callouts
 *	based on the errors.
 *
 * (RECOVERY OPERATION:)
 *	Software error recovery is handled by setting
 *	error flags and returning these to the diagnostic
 *	supervisor.
 *
 * (DATA STRUCTURES:) 
 *	None.
 *
 * RETURNS:
 *	0 = good return, no errors.
 *	num = the error number found in the error log.
 */  


int	chk_ela()
{
	int	ela_rc;
	int	rc;
/*
	call error log entry retreive function with ptr
	if ( error entries )
	{
		generate a return code from this routine.
	}


*/

	return(0);

}	/* end chk_ela */
  

/*
 * NAME:
 *	cleanup
 *                                                                    
 * FUNCTION:
 *	This is the exit point for the diagnostic application.
 *	Any devices that were opened during the execution of the
 *	diagnostic application are closed here.  If it was necessary
 *	to configure the device from within the diagnostic
 *	application then that device is unconfigured here.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This function is called by the main procedure which is forked and
 *	exec'd by the diagnostic supervisor, which runs as a process.
 *	It is also called by the runtu function.
 *	This function runs an an application under the diagnostic subsystem.
 *                                                                   
 * (NOTES:)
 *	Closes the keyboard device driver if successfully opened.
 *	Closes the message catalog if successfully opened.
 *	Closes the odm via the term_dgodm() call.
 *	Issues DIAG_ASL_QUIT if message catalog was opened.
 *	Calls the DA_EXIT function to exit the code.
 *
 * (RECOVERY OPERATION:)
 *	Software error recovery is handled by setting
 *	error flags and returning these to the diagnostic
 *	supervisor.
 *
 * (DATA STRUCTURES:)
 *	No global data structures/variables effected.
 *
 * RETURNS:
 *	The DA_STATUS, DA_USER, DA_ERROR, DA_TESTS and DA_MORE flags
 *	are returned by this function.  These are initialized
 *	elsewhere and returned to the diagnostic supervisor via
 *	the DA_EXIT call.
 */  

void	cleanup()
{
	if ( filedes != ERR_FILE_OPEN )
	{
	    /*
 	     *   run test unit 10 one more time to reset the device and
             *   re-initialize it.
 	     */
        	if ( !( IS_TM( dmode, DMODE_ELA ) ) && ( TUS_CAN_EXECUTE ) )
                	runtu( &testlist[6] );
	}

	initial_state(device_state,tm_input.dname);
  	
	if ( term_dgodm() == ERR_FILE_OPEN )
		DA_SETRC_ERROR( DA_ERROR_OTHER );

	/* close /dev/bus0 */

	if (fdes != ERR_FILE_OPEN)
		close(fdes);

	if ( IS_TM( console,CONSOLE_TRUE ) )
	{
		if ( catd != CATD_ERR )
			catclose( catd );
		(void)diag_asl_quit( NULL );
	}
	DA_EXIT();
}

/*
 * NAME:
 *	findfru
 *                                                                    
 * FUNCTION:
 *	Uses the failing test unit number and error code returned 
 *	by that test unit as keys to a lookup table.  The lookup
 *	table contains array indices to the frub structure.  An
 *	index is returned by this function.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This function is called by the main procedure which is forked and
 *	exec'd by the diagnostic supervisor, which runs as a process.
 *	It is also called by the runtu function.
 *	This function runs an an application under the diagnostic subsystem.
 *                                                                   
 * (NOTES:)
 *	Extracts an array pointer from the fruinerror array, using
 *	the test unit number as an index.
 *	Scans the array pointed to by the previously extracted
 *	pointer for a match to the error return code from the     
 *	failing test unit.
 *	Initializes the local variable, offset, to an array index
 *	value associated with the error return code being matched.
 *	Calls insert_frub() to set device and parent names in the
 *	fru bucket.
 *	Calls addfru() to report that fru to the diagnostic supervisor.
 *	Calls cleanup() procedure.
 *
 * (RECOVERY OPERATION:)
 *	Software error recovery is handled by setting
 *	error flags and returning these to the diagnostic
 *	supervisor.
 *
 * (DATA STRUCTURES:)
 *	The DA_ERROR, DA_STATUS, and DA_MORE flags can be
 *	modified by this function.
 *
 * RETURNS:
 *	This function does not return to the calling routine.
 */  

int	findfru(tuptr, tuec)
struct	tudata	*tuptr;			/* test unit number	*/
int	tuec;				/* test unit error code */
{
	int	rc;
	extern	int	addfrub();
	extern	int	insert_frub();


	for ( ; tuptr->turc != (int)NULL ; tuptr++ )
	{
		if ( tuec == tuptr->turc )
		{
			strncpy(tuptr->fbptr->dname,tm_input.dname,NAMESIZE);
			tuptr->fbptr->sn = kbid;
			tuptr->fbptr->rcode = tuptr->reason_code;
			tuptr->fbptr->rmsg = tuptr->reason_msg;
			
			rc = insert_frub( &tm_input, tuptr->fbptr );
			if ( rc == BAD )
			{
				DA_SETRC_ERROR( DA_ERROR_OTHER );
				cleanup();
			}	
			rc = addfrub( tuptr->fbptr );
			if ( rc == BAD )
				DA_SETRC_ERROR( DA_ERROR_OTHER );
			else
				DA_SETRC_STATUS( DA_STATUS_BAD );
			break;
		}

	}

	if ( IS_TM( system, SYSTEM_FALSE  ) )
		DA_SETRC_MORE( DA_MORE_CONT );		/* set more flag*/

	if ( tuptr->turc == (int)NULL )
		DA_SETRC_ERROR( DA_ERROR_OTHER );	/* invalid rc	*/

	cleanup();
} 

/*
 * NAME: report_fru()
 *
 * FUNCTION: Designed to pass a FRU bucket and an appropriate exit code to
 *	     the controller.
 *
 * EXECUTION ENVIRONMENT:
 *	Environment must be a Diagnostics environment which is described in
 *	Diagnostics subsystem CAS.	      	
 *	  	
 * RETURNS: NONE
 */
 
int     report_fru(fru_buckets)
struct  fru_bucket  *fru_buckets;                
{
        int     rc = 0;
        extern  int     addfrub();
        extern  int     insert_frub();

	strncpy(fru_buckets->dname,tm_input.dname,NAMESIZE);
	rc = insert_frub( &tm_input, fru_buckets );
	if ( rc != 0 )
	{
        	DA_SETRC_ERROR( DA_ERROR_OTHER );
        	cleanup();
        }
	rc = addfrub( fru_buckets );
        if ( rc != 0 )
        {
		DA_SETRC_ERROR( DA_ERROR_OTHER );
 		cleanup();
	}
                               
        if ( IS_TM( system, SYSTEM_FALSE) )
                DA_SETRC_MORE( DA_MORE_CONT );          /* set more flag*/
	
	DA_SETRC_STATUS( DA_STATUS_BAD );
	DA_SETRC_TESTS( DA_TEST_FULL );
        cleanup();
}


/*
 * NAME:
 *	putupmenu
 *                                                                    
 * FUNCTION:
 *	Displays one of four possible menus to the display screen.  
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This function is called by the main procedure which is forked and
 *	exec'd by the diagnostic supervisor, which runs as a process.
 *                                                                   
 * (NOTES:)
 *	Displays either the testing device or testing device in advanced
 *	mode menus.  It is called by the da code when error log analysis
 *	is run, since that may take awhile to run.
 *
 * (RECOVERY OPERATION:)
 *	Software error recovery is handled by setting
 *	error flags and returning these to the diagnostic
 *	supervisor.
 *
 * (DATA STRUCTURES:)
 *	Does not effect global data structures/variables.
 *
 * RETURNS:
 *	This function returns to the caller if the display of the
 *	menu occurs without error.  If an error return occurs then the 
 *	exit is handled by the case statement in the chk_asl_stat function.
 */  

void	putupmenu()
{
	ulong	loopmenunum[] = { 0x921002, 0x922002, 0x923002, 0x736002 };
	ulong	dispmenunum[] = { 0x921001, 0x922001, 0x923001, 0x736001 };
	int	i;

	switch (kbid)
	{
	case SN102:
		i = 1;
		break;
	case SN106:
		i = 2;
		break;
	case SNPS2:
		i = 3;
		break;
	default:
		i = 0;
		break;
	}

	if ( DISPLAY_LOOPING )
		chk_asl_stat( diag_msg_nw( loopmenunum[i],catd,ISAKBD_MSGS,
			TESTLM,tm_input.lcount,tm_input.lerrors ) );

	else	if ( DISPLAY_ADVANCED )
		chk_asl_stat( diag_display( dispmenunum[i],catd,da_title,
			DIAG_IO, ASL_DIAG_OUTPUT_LEAVE_SC,NULL,NULL ) );

		else	if ( DISPLAY_TESTING )
			chk_asl_stat( diag_display( dispmenunum[i],catd,
				da_title, DIAG_IO, ASL_DIAG_OUTPUT_LEAVE_SC,
				NULL,NULL ) );
}

/*
 * NAME: chk_terminate_key()
 *
 * FUNCTION: Designed to check if the user hit ESC of F3 keys between
 *           running test units.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

chk_terminate_key()
{
        if ( IS_TM( console, CONSOLE_TRUE ) )
                chk_asl_stat(diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, NULL,NULL));
}

/*
 * NAME:
 *	chk_asl_stat
 *                                                                    
 * FUNCTION:
 *	Handles the return code from an asl or diag_asl procedure call.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This function is called by the main procedure which is forked and
 *	exec'd by the diagnostic supervisor, which runs as a process.
 *	It is also called by the putupmenu() function.
 *                                                                   
 * (NOTES:) 
 *	This function acts on the return code from an asl or diag_asl
 *	procedure call.  It either returns to the calling routine or
 *	sets error flags and calls the cleanup() routine.
 *
 * (RECOVERY OPERATION:)
 *	Software error recovery is handled by setting
 *	error flags and returning these to the diagnostic
 *	supervisor.
 *
 * (DATA STRUCTURES:)
 *	Global DA_ERROR flag can be set in this function.
 *
 * RETURNS:
 *	This function returns to the calling procedure only if the 
 *	asl_okay flag or an undefined flag are set in the error 
 *	return code passed as an input parameter.
 */  

chk_asl_stat(returned_code)
long	returned_code;
{
	void	cleanup();

	switch ( returned_code )
	{
		case DIAG_ASL_OK:
			break;
		case DIAG_ASL_FAIL:
		{
			DA_SETRC_ERROR(DA_ERROR_OTHER);
			cleanup();
			break;
		}
		case DIAG_ASL_CANCEL:	
		{
			DA_SETRC_USER(DA_USER_QUIT);
			cleanup();
			break;
		}
		case DIAG_ASL_EXIT:
		{
			DA_SETRC_USER(DA_USER_EXIT);
			cleanup();
			break;
		}
		default:
			break;
	}
}

/*
 * NAME:
 *	OpenDevice
 *                                                                    
 * FUNCTION: 
 *	Opens machine device driver. Here the keyboard
 *	diagnostics mode is enabled via the ioctl call.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This function is called by the main procedure which is forked and
 *	exec'd by the diagnostic supervisor, which runs as a process.
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:)
 *	Software error recovery is handled by setting
 *	error flags and returning these to the diagnostic supervisor.
 *
 * (DATA STRUCTURES:) 
 *	Global variables set in this function are:
 *	device_state
 *
 * RETURNS:
 *	This function returns no return code to the caller.
 */  

void	OpenDevice()
{
	void	checkerrorcode();
	char	*devname;
	char	*msgstr;
	int	err_code = 0;
	long	menu_num = 0;

	msgstr = (char *) malloc(1200 * sizeof(char));

	fdes = open("/dev/bus0", O_RDWR); 
	if (fdes == ERR_FILE_OPEN)
	{
		DA_SETRC_ERROR (DA_ERROR_OTHER);
		cleanup();
	}
	
	/* check if the device is configured, if not then configure and	   */
	/* save the previous device state for clean_up.			   */

	strcpy(kbddevname,"/dev/");
	strncat(kbddevname,cudv->name,NAMESIZE);
	if((device_state = configure_device(tm_input.dname)) != -1)
	{
		if ((filedes = open(kbddevname, O_RDWR)) != ERR_FILE_OPEN) 
		{
			arg = KSDENABLE;
			if (ioctl(filedes, KSDIAGMODE, &arg) == 0) 
			{
				sleep(2);
				arg = KSDDISABLE;
 				ioctl(filedes, KSDIAGMODE, &arg);
			}
			close(filedes);     /* close success or failure */
		}
		else
		{
			err_code = errno;
			if ((err_code == EBUSY) ||
		   		(err_code == ENXIO)  || (err_code == ENODEV))
			{
				esptr->rc = 0x11;
				esptr->tuptr = &testlist[0];
				checkerrorcode( esptr );
			}
			DA_SETRC_ERROR( DA_ERROR_OPEN );
			cleanup();
		}
	}
	else	/* Error on configuration.			   */
	{
		alarm(0);
		report_fru(&frub_config[0]);
		cleanup();
	}
}

