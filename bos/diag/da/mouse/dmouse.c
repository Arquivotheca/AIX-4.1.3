static char sccsid[] = "@(#)66  1.26  src/bos/diag/da/mouse/dmouse.c, damouse, bos411, 9437B411a 9/14/94 16:02:41";
/*
 *   COMPONENT_NAME: DAMOUSE
 *
 *   FUNCTIONS: EXECUTE_TU_WITHOUT_MENU
 *		EXECUTE_TU_WITH_MENU
 *		IS_TM
 *		PUT_EXIT_MENU
 *		buttons
 *		change_device_state
 *		chg_mn_adv
 *		chk_asl_stat
 *		chk_ela
 *		chk_return
 *		chk_terminate_key
 *		clean_malloc
 *		clean_up
 *		close_window
 *		get_mouse_input
 *		init_mse_device
 *		initialize_all
 *		int_handler
 *		main
 *		mouse_action
 *		open_device
 *		put_ch
 *		put_title
 *		report_fru
 *		select_menu_set
 *		timeout
 *		tu_steps
 *		tu_test
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include	<stdio.h>
#include	<cur00.h>
#include	<cur02.h>
#include	<fcntl.h>
#include	<math.h>
#include	<memory.h>
#include	<limits.h>
#include	<nl_types.h>
#include	<locale.h>
#include	<sys/types.h>
#include	<sys/ioctl.h>
#include	<sys/signal.h>
#include	<sys/termio.h>
#include	<sys/errno.h>
#include        <sys/cfgodm.h>
#include        <sys/cfgdb.h>
#include        <sys/devinfo.h>
#include	<sys/buf.h>

#include	"diag/diag.h"
#include	"diag/diago.h"
#include	"diag/tm_input.h"
#include	"diag/tmdefs.h"
#include	"diag/da.h"
#include	"diag/diag_exit.h"
#include	"diag/dcda_msg.h"
#include	"mouse_msg.h"
#include	"dmouse.h"
#include	"tu_type.h"

#define TIMEOUT	40

#define OPEN_DD (open(devmse,O_RDWR)) /* System call to open machine dd   */
					   /* or mouse device.		  */

/**************************************************************************/
/*									  */
/* IS_TM is a macro takes two input variables VAR1 & VAR2.                */
/*									  */
/* VAR1 is an object of the structure tm_input.			          */
/*									  */
/* VAR2 is a variable or a defined value will be compared to the tm_input */
/*      object class.							  */
/*									  */
/**************************************************************************/

#define IS_TM( VAR1, VAR2 ) ( (int) ( tm_input./**/VAR1 ) == /**/VAR2 )

/**************************************************************************/
/*									  */
/* READ_DATA_NOT_EXEC is a macro define all the conditions where set	  */
/*		      sampling rate and READ DATA test unit will not	  */
/*		      execute.						  */
/*									  */
/**************************************************************************/

#define READ_DATA_NOT_EXEC	( 	IS_TM( console, CONSOLE_FALSE  ))

/**************************************************************************/
/*									  */
/* READ_DATA is a macro define all the conditions where set sampling 	  */
/*	     rate and READ DATA test unit will excute.			  */
/*									  */
/**************************************************************************/

#define READ_DATA ( ! ( READ_DATA_NOT_EXEC ) )

/**************************************************************************/
/*									  */
/* SWITCH_TEST_NOT_EXEC is a macro define all the conditions where SWITCH */
/*		        TEST test unit will not execute.		  */
/*									  */
/**************************************************************************/

#define SWITCH_TEST_NOT_EXEC	( 	IS_TM( console, CONSOLE_FALSE  )   \
				   || 	IS_TM( system , SYSTEM_TRUE    )   \
				)

/**************************************************************************/
/*									  */
/* SWITCH_TEST_NOT_EXEC is a macro define all the conditions where SWITCH */
/*		        TEST test unit will execute.			  */
/**************************************************************************/

#define SWITCH_TEST ( ! ( SWITCH_TEST_NOT_EXEC ) )

/**************************************************************************/
/*									  */
/* MORE_RESOURCE is a macro define if more resource needed or further	  */
/*		 isalation needed.				   	  */
/*									  */
/**************************************************************************/

#define MORE_RESOURCE	(	       READ_DATA_NOT_EXEC		   \
				&& !( IS_TM(loopmode, LOOPMODE_EXITLM) )   \
			)

/**************************************************************************/
/*									  */
/* EXECUTE_TU_WITH_MENU is a macro define execution conditions for Test   */
/*			Units with a menu(s) to display. The macro takes  */
/* 			one parameter TU_NUM is the test unit number.     */
/*									  */
/**************************************************************************/

#define EXECUTE_TU_WITH_MENU(TU_NUM) 					   \
		( 							   \
			( 						   \
				IS_TM( loopmode, LOOPMODE_NOTLM     )	   \
			   || 	IS_TM( loopmode, LOOPMODE_ENTERLM   )	   \
			)						   \
			&& 	( IS_TM( console, CONSOLE_TRUE	)   )	   \
		)

/**************************************************************************/
/*									  */
/* EXECUTE_TU_WITHOUT_MENU is a macro define execution conditions for     */
/*			   test units without menus. The macro takes one  */
/*			   paramater TU_NUM is the test unit number.      */
/*									  */
/**************************************************************************/

#define EXECUTE_TU_WITHOUT_MENU( TU_NUM )                                  \
		( 							   \
			(						   \
			 	IS_TM( loopmode, LOOPMODE_NOTLM )	   \
			   || 	IS_TM(loopmode,LOOPMODE_ENTERLM)	   \
	  		   || 	IS_TM(loopmode,LOOPMODE_INLM)		   \
			)						   \
			&& 	( tmode[/**/TU_NUM].mflg == FALSE )	   \
		)

/**************************************************************************/
/*									  */
/* PUT_EXIT_MENU is a macro define when it has to display an exit menu if */
/*	 	 the test unit have displayed a previous menu. This macro */
/*		 takes one paramater is the Test unit number.  	  	  */
/*									  */
/**************************************************************************/

#define PUT_EXIT_MENU( TU_NUM ) 					   \
		(							   \
 			(						   \
				IS_TM( loopmode, LOOPMODE_NOTLM     )	   \
			   || 	IS_TM( loopmode, LOOPMODE_ENTERLM )	   \
			) 						   \
			&& ( tmode[/**/TU_NUM].mflg == TRUE  	  ) 	   \
			&& ( tmode[/**/TU_NUM].wrapflg == TRUE 	  ) 	   \
			&& ( IS_TM( console, CONSOLE_TRUE)	  )	   \
			&& ( IS_TM( system, SYSTEM_FALSE)	  )	   \
		)

/**************************************************************************/
/*									  */
/* DISPLAY_TESTING is a macro define when a title line would be displayed */
/*									  */
/**************************************************************************/

#define DISPLAY_TESTING 						   \
		( 							   \
			( 						   \
				IS_TM( loopmode, LOOPMODE_NOTLM   )	   \
			   || 	IS_TM( loopmode, LOOPMODE_ENTERLM )	   \
			)						   \
			&& ( IS_TM( console, CONSOLE_TRUE	  )   )	   \
		)

/**************************************************************************/
/*									  */
/* DISPLAY_LOOP_COUNT is a macro define when loop count will be displayed */
/*									  */
/**************************************************************************/

#define DISPLAY_LOOP_COUNT 						   \
		( 		IS_TM( loopmode, LOOPMODE_INLM  )	   \
			&&	IS_TM( console, CONSOLE_TRUE	)   	   \
		)

ASL_SCR_INFO	w_uinfo[7];

/**************************************************************************/
/*									  */
/* 	mouse_button is an array of structure holds the message set	  */
/*	number and a message id number in the menus catalog file.	  */
/*									  */
/*	This structure will be passed to diag_msg to tell the user	  */
/*	to Press and release each button on the mouse device and observe  */
/*	the display changes.						  */
/*									  */
/**************************************************************************/

struct	msglist	mouse_button[]=
		{
			{  MSE_NEWTITLE,	MSE_TITLE_C	},
			{  MSE_BUTTONS_TEST,	MSE_BUT_PRESS	},
			{  MSE_GENERIC,		MSE_EXIT	},
			(int)NULL
		};

/**************************************************************************/
/*									  */
/* 	mouse_stream is an array of structure holds the message set	  */
/*	number and a message id number in the menus catalog file.	  */
/*									  */
/*	This structure will be passed to diag_msg to tell the user	  */
/*	to move the mouse in all directions and to observe the movement	  */
/*	of the cursor on the screen.					  */
/*									  */
/**************************************************************************/

struct	msglist	mouse_stream[]=
		{
			{  MSE_NEWTITLE,MSE_TITLE_C	},
			{  MSE_STREAM,	MSE_STRM_MOVE	},
			{  MSE_GENERIC,	MSE_EXIT	},
			(int)NULL
		};

/**************************************************************************/
/*									  */
/* 	mouse_response is an array of structure holds the message set	  */
/*	number and a message id number in the menus catalog file.	  */
/*									  */
/*	This structure will be passed to diag_msg to tell the user	  */
/*									  */
/**************************************************************************/

struct	msglist	mouse_response[]=
		{
			{  MSE_RETURN,  	MSE_RETN_TITLE	},
			{  MSE_GENERIC,		MSE_YES		},
			{  MSE_GENERIC,		MSE_NO		},
			{  MSE_GENERIC,		MSE_ACTION	},
			(int)NULL
		};
struct	msglist	mouse_response_other[]=
		{
			{  MSE_RETURN,  	MSE_RETN_O_T	},
			{  MSE_GENERIC,		MSE_YES		},
			{  MSE_GENERIC,		MSE_NO		},
			{  MSE_GENERIC,		MSE_ACTION	},
			(int)NULL
		};

/**************************************************************************/
/*									  */
/* 	da_title is an array of structure holds the message set		  */
/*	number and a message id number in the menus catalog file.	  */
/*									  */
/*	This structure will be passed to diag_msg to tell the user	  */
/*	Mouse device diagnostics is running.				  */
/*									  */
/**************************************************************************/

struct	msglist	da_title[]=
		{
			{  MSE_NEWTITLE,MSE_TITLE_C	},
			{  MSE_GENERIC,	MSE_STND_BY	},
			(int)NULL
		};

/**************************************************************************/
/*									  */
/* 	da_title is an array of structure holds the message set		  */
/*	number and a message id number in the menus catalog file.	  */
/*									  */
/*	This structure will be passed to diag_msg to tell the user	  */
/*	Mouse device diagnostics is running.				  */
/*									  */
/**************************************************************************/

struct	msglist	retry[]=
		{
			{  MSE_NEWTITLE,MSE_TITLE_C	},
			{  MSE_CLEAN,	MSE_CLEAN_ACT	},
			{  MSE_CLEAN,	MSE_CLEAN_INST	},
			{  MSE_GENERIC,	MSE_EXIT	},
			(int)NULL
		};

struct	msglist	mouse_window[]=
		{
			{  MSE_WINDOW,	MSE_ON		},
			{  MSE_WINDOW,	MSE_OFF		},
			{  MSE_WINDOW,	MSE_LEFT	},
			{  MSE_WINDOW,	MSE_RIGHT	},
			{  MSE_WINDOW,	MSE_MIDDLE	},
			{  MSE_WINDOW,	MSE_BLANK	},
			{  MSE_WINDOW,	MSE_PLUS	},
			(int)NULL
		};


/**************************************************************************/
/*									  */
/*	tmode structure holds information related to diagnostic 	  */
/*	application.                  					  */
/*									  */
/*	fields in the structure:                                          */
/*                                                                        */
/*	tu_num : test unit number will passed to the test unit to test    */
/*		 certain logic on the device.				  */
/*									  */
/*	mflg   : TRUE, FALSE or number of menus could be displayed in	  */
/*		 that test unit.					  */
/*									  */
/*	wrapflg: TRUE, FALSE values indicate if the user inserted a wrap  */
/*		 plug on the I/O Card.					  */
/*									  */
/*	retry  : Retry testing flag.					  */
/*									  */
/**************************************************************************/
 
struct 
{
	int	tu_num;		/* tu test number.			  */
	int	mflg;		/* display a menu to the user when this   */
                      		/* flag is greater then zero.		  */
	int	wrapflg;     	/* wrap plug flag to indicate if the user */
                      		/* inserted a wrap plug                   */
	int	retry;
} tmode[] =
{
	{ MDD_DEVTU_01, FALSE, FALSE, FALSE, },
	{ MDD_DEVTU_02, FALSE, FALSE, FALSE, },
	{ MDD_DEVTU_03, FALSE, FALSE, FALSE, },
	{ MDD_DEVTU_04, FALSE, FALSE, FALSE, },
	{ MDD_DEVTU_05, FALSE, FALSE, FALSE, },
	{ MDD_DEVTU_08, TRUE , FALSE, FALSE, },
	{ MDD_DEVTU_08, TRUE , FALSE, POS_RETRY,  },
};
 
/**************************************************************************/
/*                                                                        */
/*      This FRU will be reported if a call to configure_device           */
/*      fails.                                                            */
/*                                                                        */
/**************************************************************************/

struct fru_bucket frub_conf[]=
{
        { "", FRUB1,  0  ,  0x110  , MSE_CONF ,
                        {
                                { 80, "", "", 0, DA_NAME,       EXEMPT  },
                                { 20, "", "", 0, PARENT_NAME,   NONEXEMPT }
                        },
	},
};

/**************************************************************************/
/*									  */
/*	This FRU will be reported when test MDD_DEV_TU_01	          */
/*	returns a failure.                                                */
/*	MDD_DEV_TU_01 is a disable/enable test.          	          */
/*									  */
/**************************************************************************/
 
struct fru_bucket frub_common[]=
{
	{ "", FRUB1,  0  ,  0x111  , MSE_UNEXPECTED ,
			{
				{ 80, "", "", 0, DA_NAME,	EXEMPT 	},
				{ 20, "", "", 0, PARENT_NAME,	NONEXEMPT }
			},
	},
	{ "", FRUB1,  0  ,  0x112  , MSE_DISABLE ,
			{
				{ 80, "", "", 0, DA_NAME,	EXEMPT 	},
				{ 20, "", "", 0, PARENT_NAME,	NONEXEMPT }
			},
	},
	{ "", FRUB1,  0  ,  0x113  , MSE_RESET ,
			{
				{ 100, "", "", 0, DA_NAME,	EXEMPT 	},
			},
	},
	{ "", FRUB1,  0  ,  0x114  , MSE_STATUS ,
			{
				{ 80, "", "", 0, DA_NAME,	EXEMPT 	},
				{ 20, "", "", 0, PARENT_NAME,	NONEXEMPT }
			},
	},
	{ "", FRUB1,  0  ,  0x115  , MSE_GENERAL ,
			{
				{ 100, "", "", 0, DA_NAME,	EXEMPT 	},
			},
	},
        { "", FRUB1,  0  ,  0x116  , MSE_UNKNOWN ,
                        {
                                { 100, "", "", 0, DA_NAME,      EXEMPT  },
                        },
	},
	{ "", FRUB1,  0  ,  0x117  , MSE_WRAP ,
			{
				{ 80, "", "", 0, DA_NAME,	EXEMPT 	},
				{ 20, "", "", 0, PARENT_NAME,	NONEXEMPT }
			},
	},
	{ "", FRUB1,  0  ,  0x118  , MSE_PARAM ,
			{
				{ 80, "", "", 0, DA_NAME,	EXEMPT 	},
				{ 20, "", "", 0, PARENT_NAME,	NONEXEMPT }
			},
	},
	{ "", FRUB1,  0  ,  0x161  , MSE_GENERAL ,
			{
				{ 100, "", "", 0, DA_NAME,	EXEMPT 	},
			},
	},
};

/**************************************************************************/
/*									  */
/*	This FRU will be reported when running error log analysis.	  */
/*									  */
/**************************************************************************/

struct fru_bucket ela_frub[]=
{
	{ "", FRUB1,  0  ,  0x300  , MSE_ELA ,
			{
				{ 100, "", "", 0, DA_NAME,	EXEMPT 	},
			},
	},
	{ "", FRUB1,  0  ,  0x301  , MSE_ELA ,
			{
				{ 80, "", "", 0, DA_NAME,	EXEMPT 	},
				{ 20, "", "", 0, PARENT_NAME,	NONEXEMPT }
			},
	},
};

/**************************************************************************/
/*									  */
/*	Define a type of data structure to be passed to tu_steps	  */
/*      to hold information about buttons pressed, mouse motion, and      */
/*	return codes.	                                                  */
/*									  */
/**************************************************************************/

struct	tu_data
{
	char	r_button;	/* right button down flag		 */
	char	m_button;	/* middle button down flag		 */
	char	l_button;	/* left button down flag		 */
	int	good_data;	/* indication of good data from MDD	 */
	int	x;		/* movement x coordinates		 */
	int	y;		/* movement y coordinates		 */
}mse_raw_data;

TUTYPE tucb_header ;

typedef struct micea {

	int	button_down;		/* Indicate which button was hit  */
	int	move_up;		/* Mouse moved up		  */
	int	move_down;		/* Mouse moved down		  */
	int	move_left;		/* Mouse moved left		  */
	int	move_right;		/* Mouse moved right		  */
	int	mvy_unit;		/* Num. of unit to move on y axis */
	int	mvx_unit;		/* Num. of unit to move on x axis */
	int	input;			/* Input character		  */
};

struct	tm_input	tm_input;

int	filedes = ERR_FILE_OPEN;	 /* Pointer to the device address */
int	fdes = ERR_FILE_OPEN; /* Pointer to machine device driver address */

extern nl_catd diag_catopen(char *, int);
extern int mktu_rc();
extern int init_dev();
extern int rd_word();
extern int wr_byte();
extern int exectu();
extern int rdata();
nl_catd	catd;		/* pointer to the catalog file			  */ 
void	int_handler(int);
void	timeout(int);
double	fmod(double, double);
int	device_state=0;
int	mse_dev_state=0;  /* initial state of mse device when diag. started */
int	mse_state_done=FALSE;/* false if exit open_device before opening mse */ 
int     two_button=0;
unsigned long    f_code;

char    *msgstr;
char    option[256];
char    new_out[256], new_err[256];
char    *lpp_configure_method;
char    *lpp_unconfigure_method;
int     unconfigure_lpp = FALSE;
struct  objlistinfo     c_info;
struct  CuDv    *cudv, *cudv_o;
struct  PdDv	*pddv_p;

/*
 * NAME: main()
 *
 * FUNCTION: Main driver for Mouse device Diagnostic applications.
 *	     This function will call several functions depends on
 *	     the environment, such as initializing and executing
 *	     several TU's depend on the environment.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described 
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS:  (NONE)
 */

main(argc,argv) 
int	argc;
char	*argv[];
{
	int	rc;
        char    criteria[40];
	struct	listinfo	obj_info;
	struct	sigaction act;		/* interrupt handler structure	   */

	setlocale(LC_ALL,"");

        DA_SETRC_STATUS (DA_STATUS_GOOD);
        DA_SETRC_USER (DA_USER_NOKEY);
        DA_SETRC_ERROR (DA_ERROR_NONE);
        DA_SETRC_MORE (DA_MORE_NOCONT);

        msgstr = (char *) malloc (1200*sizeof(char));
        lpp_unconfigure_method = (char *) malloc (256*sizeof(char));
        lpp_configure_method = (char *) malloc (256*sizeof(char));

        if ((msgstr == (char *) NULL) ||
           (lpp_unconfigure_method == (char *) NULL) ||
           (lpp_configure_method == (char *) NULL))
        {
                DA_SETRC_ERROR ( DA_ERROR_OTHER );
                clean_up();
        }

	/* initialize interrupt handler structure	*/
	act.sa_handler = int_handler;
	sigaction( SIGINT, &act, (struct sigaction *)NULL);

	initialize_all();

        /* get the fru code from the LED value in predefined class */
	sprintf(criteria,"name = %s",tm_input.dname);
        cudv = (struct CuDv *) malloc (1200 *sizeof (char));
	cudv = get_CuDv_list(CuDv_CLASS,criteria,&obj_info,1,2);
	if ((cudv == (struct CuDv *) -1) || (cudv == (struct CuDv *) NULL))
	{ 
		DA_SETRC_ERROR(DA_ERROR_OTHER);
		clean_up();
	}

	f_code = cudv->PdDvLn->led ;
	if ( f_code == 0x925)
		two_button = FALSE;
	else
		two_button = TRUE;
        f_code <<= 12;

	if ( DISPLAY_LOOP_COUNT )
		chk_asl_stat(diag_msg_nw(f_code + 0x110,catd,MSE_LOOP,
			MSE_LOOP_TITLE, tm_input.lcount,tm_input.lerrors));

	if ( DISPLAY_TESTING )
		put_title(f_code + 0x001);

	if ( IS_TM( dmode, DMODE_ELA ) )
	{
		chk_ela();
		clean_up();
	}
	else
	{ 
		sleep(2); /* workaround until asl flush stout */
		signal(SIGALRM,timeout);
		alarm(TIMEOUT);
		open_device();
		alarm(0);

		/**********************************************************/
		/*		Run all the necessary tests		  */
		/**********************************************************/

 	 	tu_test( PTR_DISABLE, NULL );

  		tu_test( PTR_INT_WRAP, NULL );

		tu_test( PTR_RESOLUTION, NULL );

		tu_test( PTR_DEFAULT, NULL );

		if ( READ_DATA ) /* TU # 50 */
			tu_test( PTR_READ_DATA, NULL );

		if ( SWITCH_TEST ) /* TU # 60 */
		{
			chk_asl_stat(diag_display(f_code + 0x033,catd,
			mouse_window,DIAG_MSGONLY,NULL, NULL,w_uinfo));
			tu_test( PTR_SWITCH, BUTTONS );
			tu_test( PTR_SWITCH_MV, MOVEMENT );
			if(tmode[PTR_SWITCH_MV].retry == RETRY)
			{
				chk_asl_stat(diag_display(f_code + 0x035,catd,
				retry,DIAG_IO,ASL_DIAG_KEYS_ENTER_SC,NULL,
				NULL));
			       tu_test(PTR_SWITCH_MV, MOVEMENT);
			}
		}
		(void) chk_terminate_key();

		if ( IS_TM ( dmode, DMODE_PD ) )  /* Check for Error Log  */
			chk_ela();		  /* Analyze the Error Log*/
		(void) chk_terminate_key();
		
		if ( MORE_RESOURCE )	/* check more testing to be done  */
			DA_SETRC_TESTS( DA_TEST_NOTEST );
		else
			DA_SETRC_TESTS( DA_TEST_FULL );
		clean_up( );
 	}
}
 
/*
 * NAME: tu_test()
 *
 * FUNCTION: Designed to execute the test units if the test unit has
 *	     the correct environment mode. If the test unit executed
 *	     without any error returned then this function will return
 *	     to the calling function otherwise this function will report
 *	     to the diagnostic controller a FRU bucket failure indicating	
 *	     the failing problem and the program will stop execution.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * NOTE: tu_test will not return to the caller function if test unit
 *	 failed.
 *
 * RETURNS: NONE
 */
 
tu_test( array_ptr, action ) 
int 	array_ptr;
int	action;
{
	int	rc = 0;
   	int 	error_num = 0;		/* error number from the test unit */
	int	err_fru =0;             /* index into fru bucket structure */

	(void) memset(&tucb_header,0,sizeof(tucb_header));
	tucb_header.header.tu = tmode[array_ptr].tu_num;
	tucb_header.header.loop = 1;    /* in order to execute tu routines */

	(void) chk_terminate_key();

	if (    EXECUTE_TU_WITH_MENU(array_ptr) 
	     || EXECUTE_TU_WITHOUT_MENU(array_ptr) ) 
	{
		if ( tmode[array_ptr].tu_num == MDD_DEVTU_08 )
		{
			if ( error_num = init_mse_device() == 0)
			{
				error_num = select_menu_set (array_ptr, action);
			}
			else
				report_fru(&frub_common[0]);
		}
		else
			error_num = exectu (fdes, &tucb_header);
	}

	if ( PUT_EXIT_MENU( array_ptr ) ) /* check if exit menu could be  */
	{				  /* displayed.			  */
		rc = select_menu_set (array_ptr, WORKED_YES_NO);
		if ( ( rc > 1 ) && (error_num <= 0) )
		{
			if(tmode[array_ptr].retry == POS_RETRY)
			{
				tmode[array_ptr].retry = RETRY;
				error_num = 0;
			}
			else
				report_fru(&frub_common[8]);
		}
	}

	if ( error_num != 0 )
	{
		err_fru = fix_return_codes(error_num) ;
		report_fru(&frub_common[err_fru]);
	}
}
 
/*
 * NAME: chk_ela()
 *
 * FUNCTION: Designed to run error log analysis on hardware perminate
 *	     errors. If the error log contain these errors this func_
 *	     tion report a FRUB to the DC.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * NOTE: chk_ela will not return to the caller function if there is an
 *	 error in the error log.
 *
 * RETURNS: NONE
 */
 
chk_ela()
{
#if	0
	struct	errdata	err_data;
	char	*crit;
	#define Err_Options "-N KTSM -T PERM -d H  "
	int	rc=0;

	(void) memset(&err_data,0,sizeof(err_data));
	crit = (char *) calloc(*crit,sizeof(tm_input.date)+strlen(Err_Options));
	strcpy(crit,Err_Options);
	strcat(crit,tm_input.date);
	
	error_num = error_log_get(INIT,crit,&err_data);
	for (;err_data.err_id != 0xb9c0107e &&  error_num >0;)
		error_num = error_log_get(SUBSEQ,crit,&err_data);
		
	error_num = error_log_get(TERMI,crit,&err_data);
	if(err_data.err_id == 0xb9c0107e)
		report_fru(&ela_frub[0]);
#endif
}

/*
 * NAME: clean_up()
 *
 * FUNCTION:  exit from the DA and return an appropriate return
 *	      code to the Diagnostic controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described 
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */
 

clean_up()
 
{
        int     rc = 0;

	/* close machine device driver file descriptor */

	if (fdes != ERR_FILE_OPEN)
		close(fdes);

        /* return mouse to the original state it was in before tests */
        /* were run. */

        if ( mse_state_done == TRUE )
                change_device_state(AVAILABLE);

	clean_malloc();

	term_dgodm();

	if (catd != CATD_ERR)
		catclose( catd );
	if ( IS_TM( console, CONSOLE_TRUE ) )	/* Check if Console TRUE */
		diag_asl_quit(NULL);
	DA_EXIT(); 
}

/*
 * NAME: clean_malloc ()
 *
 * FUNCTION:  This message will free all memory previously malloc'd.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described 
 *	in Diagnostics subsystem CAS.
 *
 * NOTES:  
 *
 * RETURNS:
 *       None.
 */

clean_malloc ()
{
        if(cudv != (struct CuDv *) NULL)
                free (cudv);
        if(cudv_o != (struct CuDv *) NULL)
                free (cudv_o);
        if(pddv_p != (struct PdDv *) NULL)
                free (pddv_p);
        if(msgstr != (char *) NULL)
                free (msgstr);
        if(lpp_configure_method != (char *) NULL)
                free (lpp_configure_method);
        if(lpp_unconfigure_method != (char *) NULL)
                free (lpp_unconfigure_method);
}

/*
 * NAME: select_menu_set()
 *
 * FUNCTION: Designed to interface with the user, such as pressing
 *	     and releasing the mouse device buttons, moving mouse
 *	     device in different directions or answering questions.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described 
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

int
select_menu_set	(tst_ptr, action_selected)
int 	tst_ptr;
int	action_selected;
{
	static	int	ask_for_button = TRUE;
	int	rc = 0;

	ASL_SCR_INFO	uinfo[5];
	ASL_SCR_TYPE	menutypes= DM_TYPE_DEFAULTS; 
	WINDOW	*window;
	void	close_window();
	void	put_ch();

	(void) memset(uinfo,0,sizeof(uinfo));
	switch (action_selected)
	{
		case BUTTONS:	/* Check for pressed buttons.		    */
			chk_asl_stat(diag_display(f_code + 0x033,catd,
			mouse_button,DIAG_IO,ASL_DIAG_OUTPUT_LEAVE_SC,
		 	NULL,NULL));
			window = newwin (9,50,13,10); /* button window    */
			wcolorout(window,Bxa);
			cbox(window);
			wcolorend(window);
			put_ch(window,2,11,w_uinfo[2].text[0]); /* Put 'L' */
                        /* Put 'M' if 3-button mouse */
                        if ( ! two_button )
				put_ch(window,2,24,w_uinfo[4].text[0]);
			put_ch(window,2,35,w_uinfo[3].text[0]); /* Put 'R' */
			wrefresh(window);
			buttons(window,10,FALSE);	/* Draw 'L' Button */
			/* Draw 'M' button if 3-button mouse */
			if ( ! two_button )
				buttons(window,23,FALSE);	
			buttons(window,34,FALSE);	/* Draw 'R' Button */
			rc = get_mouse_input(window,BUTTONS);
			put_title(f_code + 0x004);

                	tmode[tst_ptr].wrapflg = TRUE;	/* Flag Q/A Menu  */
			break;
		case MOVEMENT:	/* Display the mouse device movement 	   */
			chk_asl_stat(diag_display(f_code + 0x005,catd,
			mouse_stream,DIAG_IO,ASL_DIAG_OUTPUT_LEAVE_SC,
			NULL,NULL));
			window = newwin (9,50,13,10);	/* Movement window */
			wcolorout(window,Bxa);
			cbox(window);
			wcolorend(window);
			wrefresh(window);
			/* Put '+' in the center of the sub window	  */
			put_ch(window,window->_maxy/2,window->_maxx/2,
					  w_uinfo[6].text[0]); /* Put '+' */
			wrefresh(window);
			rc = get_mouse_input(window,MOVEMENT);
			put_title(f_code + 0x105);
                	tmode[tst_ptr].wrapflg = TRUE;	/* Flag Q/A Menu  */
			break;

		case WORKED_YES_NO:	/* Ask the user if the mouse	   */
					/* operated properly.		   */
			menutypes.cur_index=1;	/* User respond	   */
			if (ask_for_button)
			{
				ask_for_button = FALSE;
				chk_asl_stat(diag_display(f_code + 0x007,
				catd,mouse_response,DIAG_IO,
				ASL_DIAG_LIST_CANCEL_EXIT_SC,&menutypes,uinfo));
			}
			else
				chk_asl_stat(diag_display(f_code + 0x008,
				catd,mouse_response_other, DIAG_IO,
				ASL_DIAG_LIST_CANCEL_EXIT_SC,&menutypes,uinfo));

                	tmode[tst_ptr].wrapflg = FALSE;	/* Flag Q/A Menu  */
			rc = menutypes.cur_index;	/* User respond	   */
			break;
	};
	return(rc);
}

/*
 * NAME: chg_mn_adv()
 *
 * FUNCTION: Designed to change the message pointer to advanced mode.
 *	     This function will be called only in advanced mode.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */
 
void
chg_mn_adv()
{
	mouse_button[0].msgid++;
	mouse_stream[0].msgid++;
	da_title[0].msgid++;
	mouse_response[0].msgid++;
	mouse_response_other[0].msgid++;

}

/*
 * NAME: mouse_action()
 *
 * FUNCTION: Designed to translate the mouse motion into a scaled
 *	     movement. Cursor movement on the screen is relative
 *	     to the defined scale value.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */
 

int
mouse_action(s)
struct micea	*s;
{
	static	int	scalex, scaley;

	scalex += mse_raw_data.x;	/* Acumulate X axis data */
	scaley += mse_raw_data.y;	/* Acumulate Y axis data */

	if ( scaley < -(SCALE_UNIT ))	/* Negative Y axis movements.	 */
	{
		s-> move_down = TRUE;
		/* Number of character to be moved down on the screen	 */
		s-> mvy_unit = -(scaley/SCALE_UNIT) ;
		scaley = (int)fmod((double)scaley, (double)SCALE_UNIT);
	}
	if ( scaley > SCALE_UNIT )	/* Positive Y axis movements.	 */
	{
		s-> move_up = TRUE;
		/* Number of character to be moved up on the screen	 */
		s-> mvy_unit = scaley/SCALE_UNIT ;
		scaley = (int)fmod((double)scaley, (double)SCALE_UNIT);
	}
	if ( scalex < -(SCALE_UNIT ))	/* Negative X axis movements.	 */
	{
		s-> move_left = TRUE;
		/* Number of character to be moved left on the screen	 */
		s-> mvx_unit = -(scalex/SCALE_UNIT) ;
		scalex = (int)fmod((double)scalex, (double)SCALE_UNIT);
	}
	if ( scalex > SCALE_UNIT )	/* Positive X axis movements.	 */
	{
		s-> move_right = TRUE;
		/* Number of character to be moved right on the screen	 */
		s-> mvx_unit = scalex/SCALE_UNIT ;
		scalex = (int)fmod((double)scalex, (double)SCALE_UNIT);
	}

}

/*
 * NAME: buttons()
 *
 * FUNCTION: Designed to update mouse button status on the screen.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

int
buttons(window,x,block)
WINDOW	*window; 	/* Subscreen window pointer.		   */
int	x;	/* Button X axis positions on the screen 	   */
int	block;	/* Flag to indicate visible/invisible block	   */
{
	int	y;	/* Y coordinate on the Y axis.		   */
	int	atribute;
	static	int	flag=0;

	for (y=4;y<5;y++)
	{
		wmove(window,y,x);
		/* Select box attribute depends on block	   */
		(atribute=(block == 0)) ? NORMAL : STANDOUT;
		wchgat(window,3,atribute);
			
	}
	wmove(window,y+1,x);
	if (flag >2)
		waddstr(window,( block ? w_uinfo[0].text : w_uinfo[1].text) );
	else
		/* First time display the boxes with an OFF status */
		{
		flag++ ;
		waddstr(window, w_uinfo[1].text);
		}

	wrefresh(window);
}

/*
 * NAME: close_window()
 *
 * FUNCTION: Designed to close the subscreen window.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

void
close_window(window)
WINDOW	*window;
{
	werase(window);
	wrefresh(window);
	delwin(window);
}

/*
 * NAME: put_ch()
 *
 * FUNCTION: Designed to display a character in a specific location.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

void
put_ch(window,y,x,ch)
WINDOW	*window;	/* Pointer to work window.			  */
int	x,y;		/* X, Y Cordinates for the display char.	  */
char	ch;		/* Char to be displayed.			  */
{
	if (x >= (window->_maxx-1))	/* Force not to exceed window size*/
		x = window->_maxx -2;	/* on x axis.			  */
	if (x <= 0)
		x = 1;
	if (y >= (window->_maxy-1))	/* Force not to exceed window size*/
		y = window->_maxy -2;	/* on y axis.			  */
	if (y <= 0)
		y = 1;
	wmove(window,y,x);
	waddch(window,ch);
}

/*
 * NAME: get_mouse_input()
 *
 * FUNCTION: Designed to get the mouse motion status or buttons status
 *	     and wait for a termination key from the keyboard or a 
 *	     hardware failure.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

get_mouse_input(window,look_for)
WINDOW	*window;	/* Work window pointer				  */
int	look_for;	/* Flag for mouse motion/buttons		  */
{
	int	loop = 0;
	int	y,x;
	int	rc = 0;
	int	ex_rc = 0;
	int	key_inp;	/* Keyboard input data			  */
	int	count;
	int	input_type;

	struct	micea	mice;
	(void) memset(&mice,0,sizeof(mice));

	input_type = look_for;
	for(;loop != TRUE;)
	{
		/* Loop until good data is received from mouse device	  */
		for(; mse_raw_data.good_data == 0;)
		{
			for(count=0;count<5;count++)
			{
				key_inp = diag_asl_read
					(ASL_DIAG_KEYS_ENTER_SC,FALSE,NULL);
				if( (DIAG_ASL_CANCEL == key_inp)
				   || (DIAG_ASL_ENTER == key_inp )
				   || (DIAG_ASL_EXIT== key_inp) || (rc != 0))
				{
					close_window(window);
					chk_asl_stat(key_inp);
					return(0);
				}
			}
			rc = tu_steps (input_type);
		}
		mse_raw_data.good_data = 0;
		if(look_for == BUTTONS) 
		{
			/* Display status for each button.	     */
			buttons(window,10,mse_raw_data.l_button);
			if ( ! two_button )
				buttons(window,23,
				mse_raw_data.m_button);
			buttons(window,34,mse_raw_data.r_button);
		}
		if (look_for == MOVEMENT)
		{
			mouse_action(&mice);
			/* Display as many as number of movement on the	   */
			/* Y axis.					   */
			for(count=1;count <= mice.mvy_unit;count++)
			{
				getyx(window,y,x);	/* get x,y cordinate */
				put_ch(window,y,--x,
				     w_uinfo[5].text[0]); /* Put Blank Char  */
				put_ch(window,(mice.move_up ? --y:++y),x,
					  w_uinfo[6].text[0]); /* Put '+' */
			}
			/* Display as many as number of movement on the	   */
			/* X axis.					   */
			for(count=1;count <= mice.mvx_unit;count++)
			{
				getyx(window,y,x);	/* get x,y cordinate */
				put_ch(window,y,--x,
				     w_uinfo[5].text[0]); /* Put Blank Char  */
				put_ch(window,y,(mice.move_left ? --x:++x),
					  w_uinfo[6].text[0]); /* Put '+' */
			}
			/* Reset all movement flags.			     */
			mice.move_up = FALSE;
			mice.move_down = FALSE;
			mice.move_left = FALSE;
			mice.move_right = FALSE;
			mice.mvx_unit = FALSE;
			mice.mvy_unit = FALSE;
		}
		wrefresh(window);
	}
}

/*
 * NAME: report_fru()
 *
 * FUNCTION: Designed to report FRUB to the diagnostics controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */


report_fru( fru_buckets) 
struct	fru_bucket	*fru_buckets;
{
	int	rc = 0;
	extern	addfrub();		/* add a FRU bucket to report 	  */
	extern	insert_frub();		/* insert fru name		  */

	strncpy(fru_buckets->dname, tm_input.dname, NAMESIZE);
	rc = insert_frub( &tm_input,fru_buckets);
	if ((rc != 0) || (addfrub( fru_buckets) != 0 ))
	{
		DA_SETRC_ERROR ( DA_ERROR_OTHER );
		clean_up();
	}
	if ( IS_TM(system, SYSTEM_FALSE))
		DA_SETRC_MORE(DA_MORE_CONT);

	DA_SETRC_STATUS( DA_STATUS_BAD );
	DA_SETRC_TESTS( DA_TEST_FULL );

	clean_up( );
}

/*
 * NAME: chk_terminate_key()
 *
 * FUNCTION: Designed to check if the user hit ESC or F3 keys between
 *	     running test units.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described 
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

chk_terminate_key( )
{
	
	if ( IS_TM( console, CONSOLE_TRUE ) )
		chk_asl_stat(diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, NULL,NULL));
}

/*
 * NAME: initialize_all()
 *
 * FUNCTION: Designed to initialize the odm, get TM input and to
 *	     initialize ASL.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */
 
initialize_all()
{
	int rcode = 0;

	if (rcode = init_dgodm() == ERR_FILE_OPEN)  /* Initialize odm.   */
                chk_return(rcode);

        if (rcode = getdainput( &tm_input ) !=0 )     /* get DA TM input    */
                chk_return(rcode);

	if ( IS_TM( advanced, ADVANCED_TRUE ) )	/* Check if Advanced mode */
		chg_mn_adv();			/* Change title number	  */

	if ( IS_TM( console, CONSOLE_TRUE ) )
	{
		chk_asl_stat( diag_asl_init(NULL) );/* initialize ASL	  */
		catd = diag_catopen( MF_MOUSE,0) ;
	}
}

/*
 * NAME: chk_asl_stat()
 *
 * FUNCTION: Designed to check ASL return code and take an appropriate action.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

chk_asl_stat(returned_code)
long	returned_code;
{
	switch ( returned_code )
	{
		case DIAG_ASL_OK:
			break;
		case DIAG_MALLOCFAILED:
		case DIAG_ASL_ERR_NO_SUCH_TERM:
		case DIAG_ASL_ERR_NO_TERM:
		case DIAG_ASL_ERR_INITSCR:
		case DIAG_ASL_ERR_SCREEN_SIZE:
		case DIAG_ASL_FAIL:
		{
			DA_SETRC_ERROR(DA_ERROR_OTHER);
			clean_up();
			break;
		}
		case DIAG_ASL_CANCEL:
		{
			DA_SETRC_USER(DA_USER_QUIT);
			clean_up();
			break;
		}
		case DIAG_ASL_EXIT:
		{
			DA_SETRC_USER(DA_USER_EXIT);
			clean_up();
			break;
		}
		default:
			break;
	}
}

/*
 * NAME: chk_return()
 *
 * FUNCTION: Designed to check for an invalid return code from a function.
 *	     Invalid return code is -1.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

chk_return(ret_code)
int	ret_code;
{
	if ( ret_code == INVALID_RETURN )
	{
		DA_SETRC_ERROR ( DA_ERROR_OTHER );
		clean_up();
	}
}


/*
 * NAME: timeout()
 *
 * FUNCTION: Desined to handle timeout interrupt handlers.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

void timeout(sig)
int sig;
{
	alarm(0);
	report_fru(&frub_common[0]);
}

/*
 * NAME: int_handler
 *                                                                    
 * FUNCTION: Perform general clean up on receipt of an interrupt
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This should describe the execution environment for this
 *	procedure. For example, does it execute under a process,
 *	interrupt handler, or both. Can it page fault. How is
 *	it serialized.
 *                                                                   
 * RETURNS: NONE
 */  

void int_handler(int sig)
{
	if ( IS_TM( console, CONSOLE_TRUE ) )
		diag_asl_clear_screen();
	clean_up();
}

/*
 * NAME: put_title()
 *
 * FUNCTION: Designed to display the diagnostics header line.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 *
 * RETURNS: NONE
 */

put_title(screen_id)
int	screen_id;
{
	chk_asl_stat( diag_display(screen_id,catd, da_title,DIAG_IO,
			ASL_DIAG_OUTPUT_LEAVE_SC, NULL,NULL) );
}

/*
 * NAME: open_device
 *
 * FUNCTION: Open mouse device to ensure sole use of the resource and then
 *	     close and unconfigure it. Then open machine device driver.
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in the Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

open_device()
{
	char	devmse[32];

	/* open machine device driver */

        fdes = open("/dev/bus0", O_RDWR);
	if (fdes == ERR_FILE_OPEN)
        {
                DA_SETRC_ERROR (DA_ERROR_OTHER);
                clean_up();
 	}       

	/* get mouse device state and save for clean_up. make device be */
	/* available, then open, close and unconfigure. */

 	if ((device_state = configure_device(tm_input.dname)) != -1)
	{
		if (device_state == AVAILABLE)
			mse_state_done = TRUE;
                sprintf (devmse, "/dev/%s", tm_input.dname);
                if ((filedes = OPEN_DD) == ERR_FILE_OPEN)
                {
                        DA_SETRC_ERROR (DA_ERROR_OPEN);
                        clean_up();
                }
                close(filedes);
		change_device_state(DEFINED);
        }
	else
		report_fru(&frub_conf[0]);
}

/*
 * NAME: change_device_state
 *
 * FUNCTION:This function is called by main() only
 *      when the device is in the available state.
 *      The device is unconfigured and its state is
 *      changed to define.
 *
 * RETURNS: 0 Good
 *          -1 Bad
 *
 */

change_device_state(status)
int status;
{
        char    temp1[128];
        int     result;

        cudv_o = (struct CuDv *) malloc (1200 *sizeof (char));
        pddv_p = (struct PdDv *) malloc (1200 *sizeof (char));

        sprintf (temp1, "name=%s AND chgstatus != 3",tm_input.dname);
        cudv_o = get_CuDv_list(CuDv_CLASS, temp1, &c_info, 1, 1);
        if (cudv_o == (struct CuDv *) NULL)
        {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }
        sprintf (temp1, "uniquetype=%s",cudv_o->PdDvLn_Lvalue);
        pddv_p = get_PdDv_list (PdDv_CLASS, temp1, &c_info, 1, 1);
        sprintf (option," -l %s", cudv_o->name);

        if (status == DEFINED)
        {
                strcpy (lpp_unconfigure_method, pddv_p->Unconfigure);
                result = odm_run_method(lpp_unconfigure_method,option,
                                &new_out,&new_err);
                if (result != 0)
                {
                        sprintf(msgstr, (char *)diag_cat_gets (catd, 1,
                         DEVICE_CANNOT_UNCONFIGURE), f_code+0x300, cudv_o->name,
                                cudv_o->location);
                        menugoal (msgstr);
			mse_state_done = FALSE;  /* to avoid infinite loop */
                        clean_up();
                }
        }
        else
        {
                strcpy (lpp_configure_method, pddv_p->Configure);
                result = odm_run_method(lpp_configure_method,option,
                                &new_out,&new_err);
                if (result != 0) 
		{
                	mse_state_done = FALSE;  /* to avoid infinite loop */
                        report_fru(&frub_conf[0]);
		}
        }
}

/*
 *
 * NAME: init_mse_device 
 *
 * FUNCTION: This subroutine is used in place of a TU to get low level data
 *     from the mouse about buttons pressed or mouse movement.
 *
 * EXECUTION ENVIRONMENT:
 *      Exclusive use of mouse resource (device and adapter).
 *      MOUSE cable is attached.
 *      User interaction is required.
 *
 * RETURNS:
 *      0x00 - successful operation  (data report is passed to the user)
 *      0x61 - unexpected device or adapter error
 *
 */

int init_mse_device()
{
        unsigned char cdata;
        unsigned int mou;
        int count;
        int rc = 0;
        union regs {
                unsigned int lreg;
                unsigned char reg[4];
                } mouse;

/* initialize variables */

mouse.lreg = 0;
count = 0;

	/* initialize mouse device */
        /* disable adapter interrupts to system */
        /* read from mouse receive/stat register - make sure data
           buffer is empty */

        if ((rc = init_dev(fdes)) == 0)
        {
                cdata = ADP_CMD_DISABLE_INT;
                if ((rc = wr_byte(fdes,&cdata,MOUSE_ADP_CMD_REG)) == 0)
                {
                        usleep(25 * 1000);
                        for (count = 0; count < READ_COUNT; count++)
                               rc=rd_word(fdes, &mouse.lreg, MOUSE_RX_STAT_REG);
                }
        }
        return(rc);
}


/*
 *
 * NAME:  tu_steps
 *
 * FUNCTION: This subroutine is used in place of a TU to get low level data 
 *     from the mouse about buttons pressed or mouse movement.
 *
 * EXECUTION ENVIRONMENT:
 *      Exclusive use of mouse resource (device and adapter).
 *      MOUSE cable is attached.
 *      User interaction is required.
 *
 * RETURNS:
 *      0x00 - successful operation  (data report is passed to the user)
 *      0x61 - unexpected device or adapter error
 *
 */

int tu_steps(int action_type)
{
	int rc = 0;
	int info = 0;
	char char_delta;
	int xint_delta, yint_delta;
	union regs {
		unsigned int lreg;
		unsigned char reg[4];
		} mouse;

/* initialize variables */

mouse.lreg = 0;

	if ((rc = rd_word(fdes, &mouse.lreg, MOUSE_RX_STAT_REG)) == 0);
	{
		/* look for data received */
		if ((mouse.lreg & 0xff000000) == 0x39000000)
		{
			switch (action_type)
			{
			case BUTTONS:
			/* mask off bit which distinguishes 2-button from */
			/* 3-button mouse, also highest values */
			info = (mouse.lreg & 0x00070000);

				/* left button pressed */
			mse_raw_data.l_button = 
				((info & LEFT_BUTTON) == LEFT_BUTTON) ? 1 : 0;

    				/* right button pressed */
			mse_raw_data.r_button = 
				((info & RIGHT_BUTTON) == RIGHT_BUTTON) ? 1 : 0;

		       		/* middle button pressed */
			mse_raw_data.m_button = 
				((info & MIDDLE_BUTTON)==MIDDLE_BUTTON) ? 1 : 0;

			break;
			
			case MOVEMENT:
				/* process x data */
				char_delta = (char)mouse.reg[2];
				  /* is x movement negative ? */
				if (mouse.reg[1] & 0x10)
					char_delta = -char_delta;
				xint_delta = (int) char_delta;
				  /* is x movement negative ? */
				if (mouse.reg[1] & 0x10)
               				xint_delta = -xint_delta;
				mse_raw_data.x = xint_delta;

				/* process y data */
               			char_delta = (char)mouse.reg[3];
				  /* is y movement negative ? */
               			if (mouse.reg[1] & 0x20)
                       			char_delta = -char_delta;
              	 		yint_delta = (int) char_delta;
				  /* is y movement negative ? */
               			if (mouse.reg[1] & 0x20)
                       			yint_delta = -yint_delta;
				mse_raw_data.y = yint_delta;
				break;
			}

			/* let function know that data arrived */
			mse_raw_data.good_data = 1;
		}
		else
			{
			/* if no data received, reset values */
			mse_raw_data.l_button = 0;
			mse_raw_data.m_button = 0;
			mse_raw_data.r_button = 0;
			mse_raw_data.x = 0;
			mse_raw_data.y = 0;
			}
	}
	return (rc);
}


int fix_return_codes(err_num)
int err_num;
{
	int ret_code = 0;
	
	/* set to unexpected adapter or device error if rc is 0x48 or 0x49 */

	if (( err_num == ADAP_DTX_REG ) || ( err_num == ADAP_CMD_REG ))
		ret_code = 0;
	else
	{ 
		/* mask off all but last 4 digits. don't care about others */

		err_num = (err_num & 0x8000FFFF);

		/* call out SW error if TU semaphore problem */

		if ((err_num >= 0xA00C) || (err_num <= 0xA010))
	        {
        	        DA_SETRC_ERROR( DA_ERROR_OTHER );
                	clean_up();
                }

		/* ret_code will be used as an index into the array, */
		/* frub_common */

		switch(err_num) 
		{
			/* mouse unexpected */
			case 0xA001:
				ret_code = 0;
				break;

			/* mouse disable */
			case 0xA008:
			case 0x1000:
			case 0x1300:
				ret_code = 1;
				break;

			/* mouse reset */
			case 0xA00B:
			case 0x1100:
				ret_code = 2;
				break;

			/* mouse status */
			case 0xA003:
			case 0xA004:
			case 0xA005:
			case 0xA006:
			case 0xA007:
			case 0x1200:
			case 0x3500:
			case 0x4200:
			case 0x5100:
			case 0x5200:
				ret_code = 3;
				break;

			/* mouse general */
				ret_code = 4;
				break;

			/* mouse unknown */
			case 0x1800:
				ret_code = 5;
				break;

			/* mouse wrap */
			case 0x2100:
			case 0x2200:
			case 0x2300:
			case 0x2400:
			case 0x2500:
				ret_code = 6;
				break;

			/* mouse param */     
			case 0xA002:
			case 0xA009:
			case 0xA00A:
			case 0x1400:
			case 0x1500:
			case 0x1600:
			case 0x1700:
			case 0x1900:
			case 0x3100:
			case 0x3200:
			case 0x3300:
			case 0x3400:
			case 0x4100:
				ret_code = 7;
				break;

			/* any other errors will be reported as unexpected */
			/* mouse or adapter error */
			default:
				ret_code = 0;
				break;
		}
	}
	return(ret_code);
}
