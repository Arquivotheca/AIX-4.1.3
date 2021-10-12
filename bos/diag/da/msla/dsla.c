static char sccsid[] = "@(#)24  1.14  src/bos/diag/da/msla/dsla.c, damsla, bos411, 9428A410j 5/17/94 08:03:10";
/*
 *   COMPONENT_NAME: DAMSLA
 *
 *   FUNCTIONS: chk_asl_stat
 *		chk_err_log
 *		clean_up
 *		main
 *		run_tests
 *		tu_test
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1990,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        <stdio.h>
#include        <fcntl.h>
#include        <nl_types.h>
#include        <locale.h>
#include        <memory.h>
#include        <sys/types.h>
#include        <sys/ioctl.h>
#include        <sys/devinfo.h>
#include        <sys/termio.h>
#include        <sys/errno.h>
#include        <sys/errids.h>

/* ...... the following are the system include files ...........................*/
#include        "diag/diago.h"            /* defines DIAG_ASL_CANCEL etc. */
#include        "diag/diag.h"             /* defines error log structure  */
#include        "diag/tm_input.h"         /* defines 'tm_input' struct    */
#include        "diag/tmdefs.h"           /* defines costants in tm_input */
#include        "diag/da.h"               /* defines 'fru_bucket' struct  */
#include        "diag/diag_exit.h"        /* defines DA_SET_RC etc        */
#include        "diag/dcda_msg.h"         /* generated from dcda.msg      */

/*....... the following are the application specific include files for msla ....*/
#include        "dsla_msg.h"          /*  generated from dsla.msg   */
#include        "dsladef.h"           /*  defines parent name etc   */
#include        "dslamacro.h"         /*  defines macros            */
#include        "dslamsgdat.h"        /*  defines msgno etc.        */
#include        "dslafrub.h"          /*  defines fru buckets used  */
#include        "dsla_tu.h"           /*defines 'mslatu' & dgsfile struct  */

/*
**  GLOBAL DATA DECLARATIONS
*/
struct {
	int	tu_num;  	/* TU number */
	int	mflg;		/* Whether TU to be executed or not */
} tmode[] = 
{
	{ MSLA_POS_TEST  , TRUE },
	{ MSLA_MEM_TEST  , TRUE },
	{ MSLA_REG_TEST  , TRUE },
	{ MSLA_HW_TEST   , TRUE },
	{ MSLA_WRAP_TEST , TRUE },
	{ MSLA_INTR_TEST , TRUE },
	{ MSLA_DMA_TEST  , TRUE },
	{ MSLA_VPD_TEST  , TRUE },
};

struct tm_input tm_input;
struct _mslatu gtu;
int	filedes = ERR_FILE_OPEN ;  /*   Pointer to the msla device address  */
nl_catd catd = CATD_ERR ;          /*   Pointer to the catalog file         */
ulong   gm_base, gio_base;         /* used for passing MSLA mem. & io. addresses */

/*
**  GLOBAL FUNCTION DECLARATIONS
*/
extern void	chk_asl_stat();
extern int	diag_asl_init();
extern long	diag_asl_msg();
extern int	init_dgodm();
extern int	diag_msg_nw();
extern nl_catd diag_catopen();
void clean_up();

/*
 * NAME:
 *	main
 *                                                                    
 * FUNCTION:
 *	This function the data structures needed to run the application
 *	test units.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This function is called by the main procedure which is forked and
 *	exec'd by the diagnostic supervisor, which runs as a process.
 *	This function runs an an application under the diagnostic subsystem.
 *                                                                   
 * (NOTES:)
 *	Initializes the tm_input structure.
 *	Initializes the tu, mfg and loop variables of the test
 *	unit control block structure, tucb_t.
 *	Checks if the correct configuration is present, if not
 *	changes the configuration.
 *
 * (RECOVERY OPERATION:)
 *	Software errors, such as failures to open, are handles by
 *	setting error flags and returning to the diagnostic
 *	supervisor.
 *
 * (DATA STRUCTURES:)
 *	This function modifies the following global structures
 *		and variables:
 *	Initializes the tm_input structure.
 *	Initializes the tmode structure.
 *	Initializes the filedes varaible. 
 *	Initialises the catd descriptor.
 *
 * RETURNS:
 *	None.
 */

main()
{
	extern int	getdainput();
	extern int	bus_mem_open();
	void	chk_err_log();
	void	run_tests();
	void    chk_chg_cfg();
	char    dd_name[20];
	int i;

setlocale(LC_ALL, "");


	/*...............INITIALIZE the ODM.............. */
	if (init_dgodm() == ERR_FILE_OPEN ) {
		DA_SETRC_ERROR ( DA_ERROR_OTHER );
		DA_SETRC_MORE  ( DA_MORE_CONT);
		DA_EXIT() ;
	}
	/*..... get Diagnostic TM input parameters....... */
	if ( getdainput( &tm_input ) != 0 ) {
		SW_ERROR;
	}
	/*............. initialise user interface if console is available .........  */
	if ( IS_TM ( console, CONSOLE_TRUE ) ) {
		chk_asl_stat ( diag_asl_init ( ASL_INIT_DEFAULT ) );
		if ( DISPLAY_TESTING ) {
			if ( ( catd = diag_catopen( MF_DSLA, 0) ) == CATD_ERR) {
				SW_ERROR;
			}
		}
	}

	/* If in concurrent mode tests cannot be done. Hence exit */
	if ( IS_TM ( exenv, EXENV_CONC ) ) {
		DA_SETRC_MORE ( DA_MORE_CONT );
		clean_up();
	}

	/* Check for correct configuration of MSLA card.....*/
	/* The function chk_chg_cfg() returns the name of the device to be tested,
	 * ie. "/dev/gsw0" or "/dev/gsw1", and the original device driver name is  
 	 * indicated through the variable 'hia_or_gsw'.
	 */
	dd_name[0] = '\0';      /* error conditions                */
	chk_chg_cfg(dd_name);
	/* If proper configuration not achieved, check error log if necessary,
         * indicate error and return.
	 */
/*        if ( dd_name == NULL ) {
		if ( ( IS_TM (dmode, DMODE_PD  ) ) || 
		    ( IS_TM (dmode, DMODE_ELA ) ) ) {
			chk_err_log(dd_name);
		}
		SW_ERROR;
	}   can't check.. don't know which
 */
	/* If in exit loop mode, do ELA if needed and then exit */
	if ( IS_TM ( loopmode, LOOPMODE_EXITLM ) ) {
		if ( ( IS_TM (dmode, DMODE_PD  ) ) || 
		    ( IS_TM (dmode, DMODE_ELA ) ) ) {
			chk_err_log(dd_name);
		}
		clean_up();
	}

	if ( IS_TM(advanced, ADVANCED_TRUE) ) {
		da_title[0].msgid++;
	}
	/*..... If not in ELA mode do all the tests .......*/
	if ( !( IS_TM ( dmode , DMODE_ELA) ) ) {
		run_tests(dd_name);
	}

	/*  Do error log analysis if needed */
	if ( ( ( IS_TM (dmode, DMODE_PD  ) ) || 
	       ( IS_TM (dmode, DMODE_ELA ) ) ) && 
	     ( ( IS_TM (loopmode, LOOPMODE_NOTLM) ) ) ) {
		chk_err_log(dd_name);
	}
	clean_up();
}


/* End of main()    */


/*
 * NAME:
 *	run_tests
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
 *	Initializes the variables of the test unit control
 *	block structure, gtu.
 * 	Displays the panels indicating that MSLA is being tested
 *	Calls the test unit entry point. 
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
 *	Initializes the gtu structure.
 *	Initializes the DA_SETRC_ERROR variable.
 *	Initializes the DA_SETRC_USER variable.
 *
 * RETURNS:
 *	There are no return values from this function.
 */

void
run_tests(dev_name)
char	*dev_name;
{
	void	tu_test();
	char *sub[3];
	/*                       Display the first panel                      */


		sub[0] = diag_cat_gets(catd, MSL_TITLES, MSL_NAME);
		sub[1] = tm_input.dname;
		sub[2] = tm_input.dnameloc;


	if ( DISPLAY_LOOP_COUNT ) {
		chk_asl_stat( diag_display_menu( LOOPMODE_TESTING_MENU,
			      MENUNO_MSLA_2, sub, NULL, NULL ) );

	}
	if ( DISPLAY_ADV_TITLE ) {
		chk_asl_stat( diag_display_menu( ADVANCED_TESTING_MENU,
			      MENUNO_MSLA_1, sub, NULL, NULL ) );
	} else if ( DISPLAY_TITLE ) {
		chk_asl_stat( diag_display_menu( CUSTOMER_TESTING_MENU,
			      MENUNO_MSLA_1, sub, NULL, NULL ) );

	}
	filedes = bus_mem_open(dev_name, &gm_base, &gio_base );
	if (  filedes < 0 ) {
		DA_SETRC_ERROR ( DA_ERROR_OPEN );
		clean_up();
	}
	/* ................... set up parameters to pass to exectu ... */
	gtu.gmbase = ( unsigned int ) gm_base;
	gtu.gibase = ( unsigned int ) gio_base;
	gtu.msla_membase = gm_base;
	gtu.msla_iobase = gio_base;
	gtu.fd = filedes ;

	DA_SETRC_TESTS ( DA_TEST_FULL );
/* Call tu_test() for each of the TUs.
 * Note that the mflg parameter is always true and the third parameter in tu_test()
 * is just a dummy.
 */
	if ( tmode[PTR_MEM_TEST].mflg == TRUE ) {
		tu_test( PTR_MEM_TEST, frub_memtest, err_ram);
	}
	if ( tmode[PTR_REG_TEST].mflg == TRUE ) {
		tu_test( PTR_REG_TEST, frub_ioregtest, err_reg);
	}

       if ( tmode[PTR_POS_TEST].mflg == TRUE ) {
		tu_test( PTR_POS_TEST, frub_postest, err_pos);
	}
	if ( tmode[PTR_HW_TEST].mflg == TRUE ) {
		tu_test( PTR_HW_TEST, frub_hwtest, err_hw);
	}
	if ( tmode[PTR_WRAP_TEST].mflg == TRUE ) {
		tu_test( PTR_WRAP_TEST, frub_wraptest, err_wr);
	}
	if ( tmode[PTR_INTR_TEST].mflg == TRUE ) {
		tu_test( PTR_INTR_TEST, frub_intrtest, err_int);
	}
	  if ( tmode[PTR_DMA_TEST].mflg == TRUE ) {
		tu_test( PTR_DMA_TEST, frub_dmatest, err_dma);
	}
	  /* do vpd last */
	  if ( tmode[PTR_VPD_TEST].mflg == TRUE ) {
		tu_test( PTR_VPD_TEST, frub_vpdtest, err_ram);
	}
}

/*
 * NAME:
 *	tu_test
 *                                                                    
 * FUNCTION:
 *	This function calls the exectu function , passing the
 *	necessary parameters.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This function is called by the run_tests procedure.
 *                                                                   
 * (NOTES:)
 *	Calls the test unit entry point, exectu with a file descriptor
 *	and pointer to the gtu structure as parameters.
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
 *
 * RETURNS:
 * 	No value is returned.
 *	( If the tu fails the function does not return to the 
 *      calling procedure at all )
 */
void
tu_test( array_ptr, fru_buckets, errors )
int	array_ptr;
struct fru_bucket fru_buckets[];
{
	extern addfrub();              /* add a FRU bucket to report     */
	extern exectu();
	unsigned long	error_num = 0;    /* error number from the test unit*/
	char    cpbuf[80];
	char    *cp = cpbuf;
	int	rc, temp;
	int	indx = 0;
	int	indx2 = 0;

	(void) memset (&gtu.tucb_header, 0, sizeof(gtu.tucb_header));
	gtu.tucb_header.tu = tmode[array_ptr].tu_num;
	gtu.tucb_header.loop = 1;
	error_num = exectu (filedes, &gtu);
	/* Check if user has entered cancel key, if so cleanup and exit */
	if ( IS_TM( console,CONSOLE_TRUE ) ) {
		chk_asl_stat( diag_asl_read( ASL_DIAG_KEYS_ENTER_SC,
			FALSE,cp ));
	}
	if ( (error_num) != 0 ) {
		/* ... get mapping from error code to fru bucket array index .... */
		temp = -1;
		if ( error_num > 0 ) {
			while ((indx >= 0) && (err_to_frumap[indx].testnumb != 0)) {
				if ( err_to_frumap[indx].testnumb == gtu.tucb_header.tu ) {
					while ( err_to_frumap[indx].fru_map[indx2].real_errno != 0) {
						if (err_to_frumap[indx].fru_map[indx2].real_errno == error_num ) {
							temp = err_to_frumap[indx].fru_map[indx2].fru_arrptr;
							indx = -1;
							break;
						} else
						 {
							indx2++;
						}
					}         /* end of inner 'while' */
					break;
				} else
				 {
					indx++;
				}
			}                 /* end of outer 'while' */
		}                     /* end of 'if error_num > 0 ' */
		/* ...Now  temp should have the index to the frub array ....*/
		/* if not a valid return code or if the return code indicates s/w    */
		/* failure like file open error or malloc failure exit DA           */
		if ( temp == -1 ) {
			DA_SETRC_ERROR ( DA_ERROR_OTHER );
			DA_SETRC_STATUS( DA_STATUS_BAD );
#ifndef STOP
			DA_SETRC_MORE  ( DA_MORE_CONT) ;
#endif
			clean_up( );
		}
		if ((insert_frub( &tm_input, &fru_buckets[temp] ) != 0)) {
			DA_SETRC_ERROR ( DA_ERROR_OTHER );
		} else
		 {
			strcpy ( fru_buckets[temp].dname, tm_input.dname);
			rc = addfrub( &fru_buckets[temp] );
			if ( rc != 0 ) {
				DA_SETRC_ERROR ( DA_ERROR_OTHER );
			}
		}
		DA_SETRC_STATUS( DA_STATUS_BAD );
#ifndef STOP
		DA_SETRC_MORE  ( DA_MORE_CONT) ;
#endif
		clean_up( );
	}  /* end of if (error_num != 0) */
}


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
 *	It is also called by the other functions if error is encountered.
 *	This function runs an an application under the diagnostic subsystem.
 *                                                                   
 * (NOTES:)
 *	Restores config. status if needed.
 *	Closes the device, if successfully opened.
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
void
clean_up( )
{
	int	restore_cfg();
	int	rc;

	if (filedes != ERR_FILE_OPEN) {
		if ( ( rc = closing()) != 0 ) {
			DA_SETRC_ERROR(DA_ERROR_OTHER);
			DA_SETRC_MORE(DA_MORE_CONT);
		}
	}
	rc = restore_cfg();
	if ( rc != 0 ) {
		DA_SETRC_ERROR(DA_ERROR_OTHER);
		DA_SETRC_MORE(DA_MORE_CONT);
	}
	if (catd != CATD_ERR)
		catclose( catd );
	term_dgodm();        /*      quit the ODM    */
	if ( IS_TM ( console, CONSOLE_TRUE ) ) {
		diag_asl_quit();
	}
	DA_EXIT();
}


/*
 * NAME:
 *	chk_asl_stat
 *                                                                    
 * FUNCTION:
 *	This function:
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
void
chk_asl_stat(returned_code)
long	returned_code;
{
	switch ( returned_code ) {
	case DIAG_ASL_OK:
		break;
	case DIAG_MALLOCFAILED:
		 {
			SW_ERROR;
			break;
		}
	case DIAG_ASL_ERR_SCREEN_SIZE:
	case DIAG_ASL_ERR_INITSCR:
	case DIAG_ASL_ERR_NO_TERM:
	case DIAG_ASL_ERR_TERMINFO_GET:
	case DIAG_ASL_ERR_NO_SUCH_TERM:
	case DIAG_ASL_FAIL:
		 {
			SW_ERROR;
			break;
		}
	case DIAG_ASL_CANCEL:
		 {
			DA_SETRC_USER(DA_USER_QUIT);
			DA_SETRC_MORE(DA_MORE_NOCONT);
			clean_up();
			break;
		}
	case DIAG_ASL_EXIT:
		 {
			DA_SETRC_USER(DA_USER_EXIT);
			DA_SETRC_MORE(DA_MORE_NOCONT);
			clean_up();
			break;
		}
	default:
		break;
	}
}


/*
 * NAME:
 *	chk_err_log
 *                                                                    
 * FUNCTION:
 *	This function analyses the error log maintained by the PSLA
 * 	device driver or the SSLA device driver,as requested by the caller.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This function is called by the main procedure which is forked and
 *	exec'd by the diagnostic supervisor, which runs as a process.
 *                                                                   
 * (NOTES:) 
 *	The error log entries are obtained by making calls to the
 *	error_log_get() function. If any error which indicates a
 *	permanent hardware error is found in the error log, an
 * 	appropriate FRU is added.
 *
 * (RECOVERY OPERATION:)
 *	Software error recovery is handled by setting
 *	error flags and returning these to the diagnostic
 *	supervisor.
 *
 * (DATA STRUCTURES:)
 *	The struct 'err_data' holds the error log information.
 *
 * RETURNS:
 *	The function does not return to the calling function if
 *	a FRU is added or if a software error occurs. Otherwise
 *	it returns to the caller .
 */

#define PSLA_ADAPTER_ERR 	0x20	  /* Reason code for PSLA adapter error */
#define FATAL_IO_ERR   		0x54	  /* Error type for PSLA fatal error    */
#define FATAL_ERR   		0x53	  /* Error type for PSLA fatal io error */
#define FILENAME_OFFSET		16	  /* Offset for filename in detail_data of SSLA */
#define REASON_OFFSET		27        /* offset for reason code in detail_data of PSLA */
#define ERRTYPE_OFFSET		23        /* offset for error type in detail_data of PSLA */

#define PIO_ERR_FILENAME        "mslamisc.c"      /* File names which are stored in the   */
#define ERR_FILENAME		"mslaofflv.c"     /* detail_data of SSLA error log        */
#define DETAIL 			err_data.detail_data

void
chk_err_log(dd_name)
char	*dd_name; 	/* Name of the device driver, ie. SSLA or PSLA */
{
	char	crit[200];	 /* array to hold search criterion */
	int	rc;
	struct  errdata err_data;
	struct  fru_bucket *fru;

	fru = NULL;
	if ( dd_name[0] == '\0' ) {
		return;
	}
/*--------------------------
	Search the error log for the named device driver.For PSLA
  the detailed reason of the error is contained in the 'reason' field
  and in the 'error type' field. For SSLA the filename in which the
  error occured is stored in the error log. Only some the entries
  are to be treated as fatal hardware errors and a FRU is to added for 
  these errors. 
  --------------------------------*/
	sprintf(crit, "-N %s %s", dd_name, tm_input.date);
	rc = error_log_get(INIT, crit, &err_data);
	while (rc != 0) {
		if (rc == -1) {
			DA_SETRC_ERROR(DA_ERROR_OTHER);
			clean_up();
		}
                /* else if there is an entry */
		else if (rc > 0) { 
			/* if hardware error from PSLA device driver... */
			if ( err_data.err_id == ERRID_PSLA002) {
				if ( DETAIL[REASON_OFFSET] == PSLA_ADAPTER_ERR ) { 
					if ( DETAIL[ERRTYPE_OFFSET] == FATAL_ERR ) { 
						fru = &frub_ela[1];
					}
					else if ( DETAIL[ERRTYPE_OFFSET] == FATAL_IO_ERR) {
						fru = &frub_ela[0];
					}
				} 
			}
			/* if hardware error from SSLA device driver... */
			else if ( err_data.err_id == ERRID_MSLA_ADAPTER) {
				if ( strcmp( &DETAIL[FILENAME_OFFSET],PIO_ERR_FILENAME ) == 0 ) {
						fru = &frub_ela[2];
				}
				else if ( strcmp( &DETAIL[FILENAME_OFFSET],ERR_FILENAME ) == 0 ) {
						fru = &frub_ela[3];
				}
			}
			if ( fru != NULL ) {
				rc = insert_frub(&tm_input,fru);
				if (rc != 0) {
					DA_SETRC_ERROR(DA_ERROR_OTHER);
					clean_up();
				}
			   	strcpy ( fru->dname, tm_input.dname);
				addfrub(fru);
				DA_SETRC_STATUS(DA_STATUS_BAD);
				clean_up();
			}
		}
		/* Get next entry from error log */
		rc = error_log_get(SUBSEQ, crit, &err_data);
	}
	/* Close error log */
	rc = error_log_get(TERMI, crit, &err_data);
	if (rc == -1) {
		DA_SETRC_ERROR(DA_ERROR_OTHER);
		clean_up();
	}
}
