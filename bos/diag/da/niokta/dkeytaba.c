static char sccsid[] = "@(#)85  1.6  src/bos/diag/da/niokta/dkeytaba.c, dakbd, bos411, 9431A411a 8/3/94 11:15:22";
/*
 *   COMPONENT_NAME: DAKBD
 *
 *   FUNCTIONS: DISPLAY_MENU
 *		EXECUTE_TU_WITHOUT_MENU
 *		EXECUTE_TU_WITH_MENU
 *		IS_TM
 *		PUT_EXIT_MENU
 *		change_device_state
 *		chg_mn_adv
 *		chk_asl_stat
 *		chk_ela
 *		chk_terminate_key
 *		clean_up
 *		close_kbd
 *		initialize_all
 *		int_handler
 *		main
 *		open_device
 *		open_kbd
 *		report_fru
 *		select_menu_set
 *		timeout
 *		tu_test
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
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
#include	<sys/ioctl.h>
#include	<sys/devinfo.h>
#include	<sys/signal.h>
#include	<sys/errno.h>
#include	<sys/inputdd.h>

#include	"diag/class_def.h"
#include	"diag/diago.h"
#include	"diag/tm_input.h"
#include	"diag/tmdefs.h"
#include	"diag/da.h"
#include	"diag/diag_exit.h"
#include	"diag/diago.h"
#include	"diag/dcda_msg.h"
#include	"diag/atu.h"
#include	"tab_a_msg.h"
#include	"dkeytaba.h"

#define	TIMEOUT	40

#define OPEN_MODE (O_RDWR)    /* device mode: read write                  */

#define OPEN_DD (open(devname,OPEN_MODE))    /* System call to open       */
					     /* Tablet or keyboard device */
#define TAB_CHILD "tab"       /* Device name for tablet child device      */

#define KBD_CHILD "kbd"       /* Device name for keyboard child device    */

/**************************************************************************/
/*									  */
/* IS_TM is a macro takes two input variables VAR1 & VAR2.                */
/*									  */
/* VAR1 is an object of the sturcture tm_input.			          */
/*									  */
/* VAR2 is a variable or a defined value will be compared to the tm_input */
/*      object class.							  */
/*									  */
/**************************************************************************/

#define IS_TM( VAR1, VAR2 ) ( (int) ( tm_input./**/VAR1 ) == /**/VAR2 )

/**************************************************************************/
/*									  */
/* WRAP_DATA_NOT_EXEC is a macro define all the conditions where WRAP 	  */
/*		      DATA test unit will not excute.			  */
/*									  */
/**************************************************************************/

#define WRAP_DATA_NOT_EXEC	(  	IS_TM( console, CONSOLE_FALSE )    \
				   || 	IS_TM( exenv,  EXENV_CONC     )    \
				   || 	IS_TM( loopmode,  LOOPMODE_INLM  )    \
				   ||	IS_TM( advanced, ADVANCED_FALSE )  \
				   ||   IS_TM( system, SYSTEM_TRUE ) \
				)

/**************************************************************************/
/*									  */
/* WRAP_DATA_TEST is a macro define all the conditions where WRAP 	  */
/*		      DATA test unit will excute.			  */
/*									  */
/**************************************************************************/

#define WRAP_DATA_TEST ( ! ( WRAP_DATA_NOT_EXEC ) )

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
				IS_TM(loopmode, LOOPMODE_NOTLM)	           \
			   || 	IS_TM(loopmode,LOOPMODE_ENTERLM)	   \
			)						   \
			&&	(testlist[/**/TU_NUM].wrapflg == TRUE)     \
			&& 	(IS_TM( console, CONSOLE_TRUE))	           \
		)

/**************************************************************************/
/*									  */
/* EXECUTE_TU_WITHOUT_MENU is a macro define execution conditions for     */
/*			   test units without menus. The macro takes one  */
/*			   parameter TU_NUM is the test unit number.      */
/*									  */
/**************************************************************************/

#define EXECUTE_TU_WITHOUT_MENU(TU_NUM)                                    \
		( 							   \
			(						   \
			 	IS_TM(loopmode, LOOPMODE_NOTLM)	           \
			   || 	IS_TM(loopmode,LOOPMODE_ENTERLM)	   \
	  		   || 	IS_TM(loopmode,LOOPMODE_INLM)		   \
			)						   \
			&& 	(testlist[/**/TU_NUM].mflg == FALSE)	   \
		)

/**************************************************************************/
/*									  */
/* DISPLAY_MENU is a macro define when a menu should be displayed, if     */
/*		the test unit has a menu to display. This macro takes    */
/*		one parameter TU_NUM is the test unit number   		  */
/*									  */
/**************************************************************************/

#define DISPLAY_MENU( TU_NUM ) 						   \
		( 							   \
			( 						   \
				IS_TM( loopmode, LOOPMODE_NOTLM   )	   \
			   || 	IS_TM(loopmode,LOOPMODE_ENTERLM)	   \
			)						   \
			&& ( testlist[/**/TU_NUM].mflg == TRUE  	  ) 	   \
			&& ( IS_TM( console, CONSOLE_TRUE)	  )	   \
			&& ( IS_TM( system, SYSTEM_FALSE)	  )	   \
		)

/**************************************************************************/
/*									  */
/* PUT_EXIT_MENU is a macro define when it has to display an exit menu if */
/*	 	 the test unit have displayed a previous menu. This macro */
/*		 takes one parameter is the Test unit number.  	  	  */
/*									  */
/**************************************************************************/

#define PUT_EXIT_MENU( TU_NUM ) 					   \
		(							   \
 			(						   \
				IS_TM( loopmode, LOOPMODE_NOTLM     )	   \
			   || 	IS_TM(loopmode,LOOPMODE_ENTERLM)	   \
			) 						   \
			&& ( testlist[/**/TU_NUM].mflg == TRUE  	  ) 	   \
			&& ( testlist[/**/TU_NUM].wrapflg == TRUE 	  ) 	   \
			&& ( IS_TM( console, CONSOLE_TRUE)	  )	   \
			&& ( IS_TM( system, SYSTEM_FALSE)	  )	   \
		)

/**************************************************************************/
/*									  */
/* DISPLAY_WARN_TESTING defines when a title line would be displayed      */
/*									  */
/**************************************************************************/

#define DISPLAY_WARN_TESTING 						   \
		( 							   \
			( 						   \
				IS_TM( loopmode, LOOPMODE_NOTLM   )	   \
			||	IS_TM( loopmode, LOOPMODE_ENTERLM )	   \
			)						   \
			&& ( IS_TM( system, SYSTEM_FALSE)	  )	   \
			&& ( IS_TM( console, CONSOLE_TRUE	  )   )	   \
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
			||	IS_TM( loopmode, LOOPMODE_ENTERLM )	   \
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

/**************************************************************************/
/*									  */
/* 	rmv_wrap is an array of structure holds the message set 	  */
/*	number and a message id number in the menus catalog file.	  */
/*									  */
/*	This structure will be passed to diag_display to ask the user	  */
/*	to remove the tablet wrap plug if attached.	                  */
/*									  */
/**************************************************************************/

struct	msglist	rmv_wrap[]=
		{
			{  TBL_GENERIC,		KBT_TITLE	},
			{  TBL_WRAPWARN,	TBL_WARN_WRP	},
			{  TBL_GENERIC,		TBL_PRS_ENTER	},
			(int)NULL
		};

/**************************************************************************/
/*									  */
/* 	wrp_yes_no is an array of structure which holds the message set	  */
/*	number and a message id number in the menus catalog file.	  */
/*									  */
/*	This structure will be passed to diag_display to ask the user	  */
/*	for a wrap plug. 						  */
/*									  */
/**************************************************************************/

struct	msglist	wrp_yes_no[]=
		{
			{  TBL_WRP_NUM, KBT_WRP_TITLE	},
			{  TBL_WRP_NUM, TBL_WRP_YES	},
			{  TBL_WRP_NUM, TBL_WRP_NO	},
			{  TBL_WRP_NUM, TBL_WRP_ACTION	},
			(int)NULL
		};

/**************************************************************************/
/*									  */
/* 	rmv_cable is a structure which holds the message set	          */
/*	number and a message id number in the menus catalog file.	  */
/*									  */
/*	This structure will be passed to diag_display to ask the user	  */
/*	to remove the tablet cable if attached and to insert a wrap	  */
/*	plug on the adapter card.					  */
/*									  */
/**************************************************************************/

struct	msglist	rmv_cable[]=
		{
			{  TBL_GENERIC,		KBT_TITLE	},
			{  TBL_RMV_ATTACHEMENT,	TBL_UNPLUG_CBL	},
			{  TBL_RMV_ATTACHEMENT,	TBL_PLUG_WRP	},
			{  TBL_GENERIC,		TBL_PRS_ENTER	},
			(int)NULL
		};


/**************************************************************************/
/*									  */
/* 	connect_cbl is an array of structure holds the message set	  */
/*	number and a message id number in the menus catalog file.	  */
/*									  */
/*	This structure will be passed to diag_display to ask the user	  */
/*	to remove the wrap plug and to connect the tablet cable if	  */
/*	it was removed.							  */
/*									  */
/**************************************************************************/

struct	msglist	connect_cbl[]=
		{
			{  TBL_GENERIC,		KBT_TITLE	},
			{  TBL_RECONNECT,	TBL_UNPLUG_WRP	},
			{  TBL_RECONNECT,	TBL_PLUG_CBL	},
			{  TBL_GENERIC,		TBL_PRS_ENTER	},
			(int)NULL
		};

/**************************************************************************/
/*									  */
/* 	da_title is an array of structure holds the message set		  */
/*	number and a message id number in the menus catalog file.	  */
/*									  */
/**************************************************************************/

struct	msglist	da_title[]=
		{
			{  TBL_GENERIC,	KBT_TITLE	},
			{  TBL_GENERIC,	TBL_STND_BY	},
			(int)NULL
		};

ASL_SCR_TYPE	menutypes= DM_TYPE_DEFAULTS ;

/***********************************************************************/
/*      frub is an array of fru_bucket structures.  fru_bucket defines */
/*      the information related to the frus which are called out by    */
/*      the diag supervisor.                                           */
/***********************************************************************/

struct fru_bucket frub[]=
{
        { "", FRUB1,  0x821  ,  0  , 0 ,
			{
                                { 100 , "" , "" , 0 , DA_NAME   },
                        },
        },
        { "", FRUB1,  0x821  ,  0  , 0 ,
                        {
                                { 90 ,"Fuse","", DKBDA_FUSE, NOT_IN_DB, EXEMPT},
                                { 10 , "" , "" , 0 , DA_NAME},
                        },
        },
};

/**************************************************************************/
/*                                                                        */
/*      frub_timeout structure holds information related to FRU bucket    */
/*      variables. this is called out when a timeout occurs.              */
/*                                                                        */
/**************************************************************************/

struct  fru_bucket      frub_timeout[] =
{
        { "", FRUB1, 0x821  ,  0x111  , DKBDA_UNEXPECTED ,
                        {
                                { 80 , "" , "" , 0, DA_NAME,     EXEMPT   },
                                { 20 ,"Software","",DKBDA_SW,NOT_IN_DB,EXEMPT },
                        },
        },
};

/**************************************************************************/
/*                                                                        */
/*      frub_open structure holds information related to FRU bucket       */
/*      variables. This set is based on results received after            */
/*      unsuccessfully attempting to open keyboard, mouse or tablet       */
/*      device.                                                           */
/*                                                                        */
/**************************************************************************/

struct  fru_bucket      frub_open[] =
{
        { "", FRUB1,  0x821  ,  0x338  , DKBDA_OPEN ,
                        {
                                { 80 ,"Tablet","",DKBDA_TAB,NOT_IN_DB,EXEMPT  },
                                { 20 ,"Software","",DKBDA_SW,NOT_IN_DB,EXEMPT },
                        },
        },
        { "", FRUB1,  0x821  ,  0x339  , DKBDA_OPEN ,
                        {
                                { 80,"Keyboard","",DKBDA_KBD,NOT_IN_DB,EXEMPT },
                                { 20 ,"Software","",DKBDA_SW,NOT_IN_DB,EXEMPT },
                        },
        },
};
/**************************************************************************/
/*                                                                        */
/*      frub_conf structure holds information related to FRU bucket     */
/*      variables. This set is based on results received after            */
/*      unsuccessfully attempting to configure keyboard, mouse or tablet  */
/*      device.                                                           */
/*                                                                        */
/**************************************************************************/

struct  fru_bucket      frub_conf[] =
{
        { "", FRUB1,  0x821  ,  0x336  , DKBDA_CONF ,
                        {
                                { 20 , "" , "" , 0, DA_NAME, EXEMPT       },
                                { 60 ,"Tablet","",DKBDA_TAB,NOT_IN_DB,EXEMPT  },
                                { 20 ,"Software","",DKBDA_SW,NOT_IN_DB,EXEMPT },
                        },
        },
        { "", FRUB1,  0x821  ,  0x337  , DKBDA_CONF ,
                        {
                                { 20 , "" , "" , 0, DA_NAME, EXEMPT       },
                                { 60,"Keyboard","",DKBDA_KBD,NOT_IN_DB,EXEMPT },
                                { 20 ,"Software","",DKBDA_SW,NOT_IN_DB,EXEMPT },
                        },
        },
        { "", FRUB1,  0x821  ,  0x331  , DKBDA_CONF ,
                        {
                                { 80 , "" , "" , 0, DA_NAME, EXEMPT       },
                                { 20 ,"Software","",DKBDA_SW,NOT_IN_DB,EXEMPT },
                        },
        },
};
/**************************************************************************/
/*      The tudata structure defines a lookup table used to map the       */
/*      valid return code from a test unit to the fru bucket information  */
/*      to return to the diag supervisor.                                 */
/**************************************************************************/

struct  tudata
{
        int     turc;
        struct  fru_bucket      *fbptr;
        int     reason_code;
        int     reason_msg;
};

/* one of these frus will be reported if the TU fails with a common return */
/* code of the form 0xF0xx */

struct  tudata tu0_rc[] =
{
        { 0xF001, &frub[0],  0x333, DKBDA_RESET        },
        { 0xF002, &frub[0],  0x311, DKBDA_LOGIC_ERR    },
        { 0xF003, &frub[0],  0x311, DKBDA_LOGIC_ERR    },
        { (int)NULL, NULL, (int)NULL, (int)NULL    },
};

/* one of these frus will be reported if TU 10 fails */

struct  tudata tu1_rc[] =
{
        { 0x1001, &frub[1],  0x220, DKBDA_FUSE_ERR    },
        { 0x1002, &frub[1],  0x220, DKBDA_FUSE_ERR    },
        { 0x1003, &frub[0],  0x311, DKBDA_LOGIC_ERR   },
        { 0x1004, &frub[0],  0x311, DKBDA_LOGIC_ERR   },
        { 0x1005, &frub[0],  0x311, DKBDA_LOGIC_ERR   },
        { 0x1006, &frub[0],  0x311, DKBDA_LOGIC_ERR   },
        { 0x1007, &frub[0],  0x313, DKBDA_REC_ERR     },
        { 0x1008, &frub[0],  0x312, DKBDA_XMIT_ERR    },
        { 0x1009, &frub[0],  0x312, DKBDA_XMIT_ERR    },
        { 0x100a, &frub[0],  0x221, DKBDA_FAILURES    },
        { (int)NULL, NULL, (int)NULL, (int)NULL    },
};

/* one of these frus will be reported if TU 20 fails */

struct  tudata tu2_rc[] =
{
        { 0x2001, &frub[0],  0x313, DKBDA_REC_ERR     },
        { 0x2002, &frub[0],  0x313, DKBDA_REC_ERR     },
        { 0x2003, &frub[0],  0x311, DKBDA_LOGIC_ERR   },
        { 0x2004, &frub[0],  0x311, DKBDA_LOGIC_ERR   },
        { 0x2005, &frub[0],  0x311, DKBDA_LOGIC_ERR   },
        { (int)NULL, NULL, (int)NULL, (int)NULL    },
};

/* one of these frus will be reported if TU 30 fails */

struct  tudata tu3_rc[] =
{
        { 0x3001, &frub[0],  0x311, DKBDA_LOGIC_ERR      },
        { 0x3002, &frub[0],  0x313, DKBDA_REC_ERR        },
        { 0x3003, &frub[0],  0x313, DKBDA_REC_ERR        },
        { 0x3004, &frub[0],  0x310, DKBDA_REG_TEST       },
        { (int)NULL, NULL, (int)NULL, (int)NULL    },
};

/* one of these frus will be reported if TU 40 fails */

struct  tudata tu4_rc[] =
{
        { 0x4001, &frub[0],  0x310, DKBDA_REG_TEST       },
        { 0x4002, &frub[0],  0x334, DKBDA_XWRAP          },
        { 0x4003, &frub[0],  0x334, DKBDA_XWRAP          },
        { 0x4004, &frub[0],  0x310, DKBDA_REG_TEST       },
        { (int)NULL, NULL, (int)NULL, (int)NULL    },
};

/**************************************************************************/
/*									  */
/*	testlist structure holds information related to the diagnostic 	  */
/*	application.                  					  */
/*									  */
/*	fields in the structure:                                          */
/*                                                                        */
/*	tu_num : test unit number passed to the test unit to test         */
/*		 certain logic on the device.				  */
/*									  */
/*	mflg   : TRUE, FALSE or number of menus could be displayed in	  */
/*		 that test unit.					  */
/*									  */
/*	wrapflg: TRUE, FALSE values indicate if the user inserted a wrap  */
/*		 plug on the I/O Card.					  */
/*									  */
/*	fru_ptr: is a pointer to the fru structure for that TU	 	  */
/*									  */
/**************************************************************************/
 
struct tuinfo
{
	short	tu_num;		/* tu test number.			  */
	short	mflg;		/* display a menu to the user when this   */
                      		/* flag is greater then zero.		  */
	short	wrapflg;     	/* wrap plug flag to indicate if the user */
                      		/* inserted a wrap plug                   */
	struct tudata *tudptr;	/* structure of frus for each tu          */
};

struct tuinfo testlist[] =
{
	{ TEST1  , FALSE, FALSE, &tu1_rc[0] },
	{ TEST2  , FALSE, FALSE, &tu2_rc[0] },
	{ TEST3  , FALSE, FALSE, &tu3_rc[0] },
	{ TEST4  , TRUE , FALSE, &tu4_rc[0] },
	{ (int)NULL, NULL, NULL, NULL	    },
};
struct  err_struct
{
        int     rc;
        struct  tuinfo  *tuptr;
};
struct	err_struct	e_return;
struct  err_struct      *esptr = &e_return;

extern nl_catd diag_catopen(char *, int);
extern  int     addfrub();
extern  int     insert_frub();

struct	tm_input	tm_input;

struct  CuDv    *cudv_t,*cudv_k,*cudv_kta;
struct  objlistinfo     t_info, k_info, kta_info;

char	crit[100];

int	fdes = ERR_FILE_OPEN;    /* file descriptor of mach device driver */
				 /* passed to TU in exectu call */
int	filedes = ERR_FILE_OPEN; /* file descriptor of kbd device  */
int	filed = ERR_FILE_OPEN;   /* file descriptor of tab device  */

nl_catd	catd = CATD_ERR;       	 /* pointer to the catalog file		  */ 
void	int_handler(int);
void	timeout(int);
int	device_state=0;

char    *msgstr;
char    option[256];
char	option_kta[256];
char    new_out[256], new_err[256];
char    *lpp_unconfigure_method;
char    *lpp_configure_method;
int     unconfigure_lpp = FALSE;
int	tab_dev_state = 0;
int	kbd_dev_state = 0;
int	tab_state_done = FALSE; /* TRUE = tab attached & initial state saved */
int	kbd_state_done = FALSE; /* TRUE = kbd attached & initial state saved */
int	kbd_opened = FALSE; /* TRUE = kbd attached & opened sucessfully */
uint    arg;

/*
 * NAME: main()
 *
 * FUNCTION: Main driver for NIO Keyboard/Tablet Adapter DA.
 *	     This function will call several functions depending on
 *	     the environment, such as initialization and execution
 *	     of TU's.
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
	struct	sigaction act;		/* interrupt handler structure	   */
	extern	int	errno ;

	setlocale(LC_ALL,"");

        DA_SETRC_STATUS ( DA_STATUS_GOOD );
        DA_SETRC_ERROR ( DA_ERROR_NONE );
        DA_SETRC_TESTS( DA_TEST_FULL );
        DA_SETRC_MORE ( DA_MORE_NOCONT );
        DA_SETRC_USER ( DA_USER_NOKEY );

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

	if ( DISPLAY_WARN_TESTING )
		chk_asl_stat ( diag_display(0x821111,catd,rmv_wrap,
			DIAG_IO,ASL_DIAG_KEYS_ENTER_SC, NULL,NULL) );

	if ( DISPLAY_TESTING )
		chk_asl_stat( diag_display(0x821001,catd,da_title,DIAG_IO,
				  ASL_DIAG_OUTPUT_LEAVE_SC, NULL,NULL) );

	if ( DISPLAY_LOOP_COUNT ) /* In loop mode display loop count	  */
		chk_asl_stat(diag_msg_nw(0x821110,catd,TBL_INLOOP,
			KBT_INLOOP_TITLE, tm_input.lcount,tm_input.lerrors));

	if ( IS_TM( loopmode, LOOPMODE_EXITLM ) )
	{	
		DA_SETRC_TESTS( DA_TEST_FULL );
		clean_up( );
	}

	if ( IS_TM( dmode, DMODE_ELA ) )
	{
		chk_ela();
		clean_up();
	}
	else
	{ 
		sleep(2); /* work around until asl flush stdout */
		signal(SIGALRM,timeout);
		alarm(TIMEOUT);
		open_device();
		alarm(0);

		/**********************************************************/
		/*		Run all the necessary tests		  */
		/**********************************************************/

 	 	tu_test( PTR_TST_1, tu1_rc);

 	 	tu_test( PTR_TST_2, tu2_rc);

 	 	tu_test( PTR_TST_3, tu3_rc);

		if ( WRAP_DATA_TEST )		  /* Port Wrap data	  */
  			tu_test( PTR_TST_4, tu4_rc);

		if ( IS_TM ( dmode, DMODE_PD ) )  /* Check for Error Log  */
		{
			chk_ela();		  /* Analyze the Error Log*/
			clean_up();
		}

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
 *	     to the calling function. Otherwise this function will report
 *	     to the diagnostic controller a FRU bucket failure indicating	
 *	     the failing problem and the program will stop the execution.
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
 
tu_test( array_ptr,fru_buckets ) 
int 	array_ptr;
struct	fru_bucket	fru_buckets[];
{
	struct	tucb_t	tucb_header ;
	extern  exectu();
   	int 	error_num = 0;		/* error number from the test unit*/

	(void) memset (&tucb_header, 0, sizeof(tucb_header));

	tucb_header.tu = testlist[array_ptr].tu_num;
	tucb_header.loop = 1; /* required to force tu routines to run */

	if ( DISPLAY_MENU(array_ptr) )	/* check if a menu to display  */
		select_menu_set ( array_ptr, INSERT_WRAP );

	if ((EXECUTE_TU_WITH_MENU(array_ptr)) 
	     || (EXECUTE_TU_WITHOUT_MENU(array_ptr))) 
	{
		error_num = exectu(fdes,&tucb_header);
		if (IS_TM (loopmode, LOOPMODE_INLM))
		{
			/* enable kbd so F3 or F10 can be captured */
			close_kbd();
			sleep(3);
			(void) chk_terminate_key();
			/* return kbd to state where TUs can be run */
			open_kbd();
		}
	}

	if ( PUT_EXIT_MENU( array_ptr ) ) /* check if exit menu could be  */
				  /* displayed.			  */
		select_menu_set (array_ptr, REMOVE_WRAP);

	/*	check the TU return code for a valid error	  */

	if ( error_num != 0 )
	{
	        esptr->rc = error_num;
		esptr->tuptr = &testlist[array_ptr];
		fix_return_code(esptr->tuptr->tudptr, esptr->rc);
	}
}
 
/*
 * NAME: fix_return_code()
 *
 * FUNCTION:  Exit from the DA and return an appropriate return
 *            code to the Diagnostic controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

fix_return_code(tuptr, tuec)
struct  tudata  *tuptr;                 /* pointer to fru bucket structure */
int     tuec;                           /* test unit error code */
{
	int rc;

	/*     mask off high bits      */

	tuec = ( tuec & 0x8000FFFF );

	/*     if TU semaphore problem, report SW error and get out */

        if (( tuec >= 0xF004 ) && ( tuec <= 0xF008 ))
	{
                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                        clean_up();
	}
        if (( tuec >= 0xF001 ) && ( tuec <= 0xF003 ))
	{
                        esptr->rc = tuec;
                        esptr->tuptr->tudptr = (struct tudata *)&testlist[0];
	}
        for ( ; tuptr->turc != (int)NULL ; tuptr++ )
        {
                if ( tuec == tuptr->turc )
                {
                        strncpy(tuptr->fbptr->dname,tm_input.dname,NAMESIZE);
                        tuptr->fbptr->sn = KEYTABA_LED;
                        tuptr->fbptr->rcode = tuptr->reason_code;
                        tuptr->fbptr->rmsg = tuptr->reason_msg;

                        rc = insert_frub( &tm_input, tuptr->fbptr );
                        if ( rc == BAD )
                        {
                                DA_SETRC_ERROR( DA_ERROR_OTHER );
                                clean_up();
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
                DA_SETRC_MORE( DA_MORE_CONT );          /* set more flag*/

        if ( tuptr->turc == (int)NULL ) 
                DA_SETRC_ERROR( DA_ERROR_OTHER );       /* invalid rc   */

        clean_up();
}
	
/*
 * NAME: clean_up()
 *
 * FUNCTION:  Exit from the DA and return an appropriate return
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
	/* if didn't get out because of file open error, close the kbd.    */
	/* close_kbd checks whether keyboard exists and is currently open. */
	/* NOTE: tablet cannot be closed when keyboard is in diag mode.    */

	if (filedes != ERR_FILE_OPEN) 
		close_kbd();

        /* close machine device driver file descriptor */

        if (fdes != ERR_FILE_OPEN)
                close(fdes);

	/* return tablet to the original state it was in before tests */
	/* were run. */

	if ( tab_state_done == TRUE )
		change_device_state(AVAILABLE);

	/* return keyboard to the original state it was in before tests */
	/* were run. */

        if ( kbd_state_done == TRUE )
                initial_state(kbd_dev_state,cudv_k->name);

	term_dgodm();
	if (catd != CATD_ERR)
		catclose( catd );
	if ( IS_TM( console, CONSOLE_TRUE ) )	/* Check if Console TRUE */
		diag_asl_quit(NULL);
	DA_EXIT(); 
}

/*
 * NAME: select_menu_set()
 *
 * FUNCTION: Designed to ask the user on the first pass if a wrap plug is
 *	     available. If not, then the program terminates and returns to
 *	     the controller. If the user selects yes, the program will
 *	     continue with more menus.
 *	     The first menu asks the user to remove the tablet cable
 *	     if attached and insert a wrap plug. The second menu 
 *	     asks the user to remove the wrap plug and insert the cable 
 *	     if it had been removed.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described 
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */
 
select_menu_set	(tst_ptr, action_selected)
int 	tst_ptr;
int	action_selected;
{
	short	wait=TRUE;
   	static	short	wrap_available = FALSE;

	close_kbd();
	if ( ( !(wrap_available) ) && ( ( IS_TM( loopmode,LOOPMODE_NOTLM ) )
		|| ( IS_TM( loopmode,LOOPMODE_ENTERLM ) ) ) ) 
	{
		menutypes.cur_index=1;/* Position cursor on yes selection */

		chk_asl_stat ( diag_display(0x821003,catd,wrp_yes_no,DIAG_IO,
			ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,NULL) );

		if ( menutypes.cur_index == TRUE )
			wrap_available = TRUE; 	/* user has a wrap plug   */
		else
		{
			DA_SETRC_TESTS( DA_TEST_FULL );  
			clean_up();
		}
	}

	switch (action_selected)
	{
		case INSERT_WRAP:
			chk_asl_stat ( diag_display(0x821004,catd,rmv_cable,
				DIAG_IO,ASL_DIAG_KEYS_ENTER_SC, NULL,NULL) );
			chk_asl_stat( diag_display(0x821005,catd,da_title,
					DIAG_IO,ASL_DIAG_OUTPUT_LEAVE_SC,
					NULL,NULL) );
                	testlist[tst_ptr].wrapflg = TRUE;
			break;
		case REMOVE_WRAP:
			chk_asl_stat( diag_display(0x821006,catd,connect_cbl,
				DIAG_IO,ASL_DIAG_KEYS_ENTER_SC, NULL,NULL) );
			chk_asl_stat( diag_display(0x821007,catd,da_title,
					DIAG_IO,ASL_DIAG_OUTPUT_LEAVE_SC,
					NULL,NULL) );
                	testlist[tst_ptr].wrapflg = FALSE;
			break;
	};
	open_kbd();
}

/*
 * NAME: report_fru()
 *
 * FUNCTION: Designed to insert a FRU bucket to the controller and set
 *	     the appropriate exit code to the controller.
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
	extern	insert_frub();

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
}

/*
 * NAME: timeout()
 *
 * FUNCTION: Designed to handle timeout interrupt handlers.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

void timeout(int sig)
{
	alarm(0);
	report_fru(&frub_timeout[0]);
	clean_up();
}

/*
 * NAME: int_handler
 *                                                                    
 * FUNCTION: Perform general clean up on receipt of an interrupt.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	Cleans up and exits DA when an interrupt signal is received.
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
 

chg_mn_adv()
{
	rmv_wrap[0].msgid++;
	wrp_yes_no[0].msgid++;
	rmv_cable[0].msgid++;
	connect_cbl[0].msgid++;
	da_title[0].msgid++;
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
 
chk_terminate_key()
{
	if ( IS_TM( console, CONSOLE_TRUE ) )	/* Check if Console TRUE */
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
	if (init_dgodm() == ERR_FILE_OPEN)	/* Initialize odm.	  */
	{
		DA_SETRC_ERROR( DA_ERROR_OTHER );
		clean_up();
	}

	if(getdainput( &tm_input ) !=0 ) /* get Diagnostic TM input 	  */
	{
		DA_SETRC_ERROR( DA_ERROR_OTHER );
		clean_up();
	}

	if ( IS_TM( advanced, ADVANCED_TRUE ) )	/* Check if Advanced mode */
		chg_mn_adv();			/* Change titles number	  */

	if ( IS_TM( console, CONSOLE_TRUE ) )
	{
		chk_asl_stat( diag_asl_init(NULL) );/* initialize ASL	  */
		catd = diag_catopen( MF_TAB_A,0); 
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
 * NAME: open_device
 *
 * FUNCTION: Open machine device driver. See if tablet device attached. If
 *	     so, open it then unconfigure it. See if keyboard attached. If
 *	     so, open it. Make sure keyboard/tablet adapter is configured.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

open_device()
{
	int	rc, err_code = 0;
	char	devname[32];

	/* open machine device driver */

        fdes = open("/dev/bus0",O_RDWR | O_NDELAY);

        if (fdes == ERR_FILE_OPEN)
        {
                alarm(0);
                DA_SETRC_ERROR (DA_ERROR_OTHER);
                clean_up();
        }

	/* see if tablet attached and not missing */

        sprintf (crit, "parent=%s AND name like %s* AND chgstatus != 3", 
		tm_input.dname, TAB_CHILD);
        cudv_t = get_CuDv_list( CuDv_CLASS, crit, &t_info, 1, 2 );

        /* if database not available, get out */

        if ( cudv_t == -1 )
        {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }

        /* if tablet device attached, configure, open, close and then */
	/* call unconfigure_lpp_device. */

        if (t_info.num != 0)
        {
                if((tab_dev_state = configure_device(cudv_t->name))!=-1)
        	{
			if (tab_dev_state == AVAILABLE)
				tab_state_done = TRUE;
                	sprintf (devname,"/dev/%s",cudv_t->name);
                	if ((filed = OPEN_DD) == ERR_FILE_OPEN)
                	{
                        	err_code = errno;
                        	alarm(0);
                        	if ( err_code == EIO || err_code == ENODEV ||
                          		err_code == ENXIO )
                               		report_fru(&frub_open[0]);
		                sprintf(msgstr, (char *)diag_cat_gets (catd, 1,
       		                   KTA_DEVICE_CANNOT_OPEN), cudv_t->name);
                		menugoal (msgstr);
                        	clean_up ();
                 	}
			else
			{
                 		close(filed);
                 		change_device_state(DEFINED);
			}
        	}
                else
		{
                        report_fru(&frub_conf[0]);
			clean_up;
		}
	}

        /* see if keyboard attached and not missing */

        sprintf (crit, "parent=%s AND name like %s* AND chgstatus != 3", 
		tm_input.dname, KBD_CHILD);
        cudv_k = get_CuDv_list( CuDv_CLASS, crit, &k_info, 1, 1 );

	/* if database not available, get out */

        if ( cudv_k == -1 )
        {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }

        /* if keyboard device attached, configure, open and then */
	/* enable device for diagnostics mode. */

	if (k_info.num != 0)
	{
		kbd_dev_state = get_device_status(cudv_k->name);
		kbd_state_done = TRUE;
		open_kbd();
	}
	else if (t_info.num != 0)
	{
		/* if tablet and no keyboard, set cudv for key/tab adapter    */
		/* because we'll have to configure it in change_device_state. */

        	sprintf (crit,"name like %s AND chgstatus != 3",tm_input.dname);
	        cudv_kta = get_CuDv_list( CuDv_CLASS, crit, &kta_info, 1, 1 );
	}

	/* if keyboard/tablet adapter not available, can't run tests */
	/* so get out. */

        if((device_state = configure_device(tm_input.dname)) == -1)
	{
		report_fru(&frub_conf[2]);
		clean_up();
	}
}

/*
 * NAME: open_kbd
 *
 * FUNCTION:This function is called by open_device(), before calling TUS 
 *      each time when in loop mode, and after user input has been obtained
 *      from menus. The keyboard will be disabled but TU tests will be able
 *      to run.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in the Diagnostics subsystem CAS.
 */

open_kbd()
{ 
       int ernum = 0;
       char kbdname[32];

	if ((kbd_state_done == TRUE) && (kbd_opened == FALSE))
	{
       		if((device_state = configure_device(cudv_k->name))!=-1)
       		{
               		sprintf(kbdname,"/dev/%s",cudv_k->name);
               		if ((filedes = open(kbdname, O_RDWR)) != ERR_FILE_OPEN)
               		{
				kbd_opened = TRUE;
                        	arg = KSDENABLE;
                        	if (ioctl(filedes, KSDIAGMODE, &arg) != 0)
                        	{
                                	DA_SETRC_ERROR(DA_ERROR_OTHER);
                                	clean_up();
                        	}
               		}
               		else
               		{
                        	ernum = errno;
                        	alarm(0);
                        	if ( ernum == EIO || ernum == ENODEV ||
                                	ernum == ENXIO )
                                	report_fru(&frub_open[1]);
                        	sprintf(msgstr, (char *)diag_cat_gets (catd, 1,
                            	   KTA_DEVICE_CANNOT_OPEN), cudv_k->name);
                        	menugoal (msgstr);
                        	clean_up ();
                	}
       		}
       		else
       		{
                	report_fru(&frub_conf[1]);
                	clean_up;
       		}
	}
}

/*
 * NAME: close_kbd
 *
 * FUNCTION:This function is called before putting menus on the screen
 *      which require a user's response and between TUs in loop mode
 *      so that the user has an opportunity to exit. The keyboard is
 *      disabled otherwise and no user input can be captured.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in the Diagnostics subsystem CAS.
 *
 * RETURNS:
 *       
 */

int close_kbd()
{
	/* if keyboard attached and previously opened */
        if ((kbd_state_done == TRUE) && (kbd_opened == TRUE))
	{
        	close(filedes);
		kbd_opened = FALSE;
	}
}

/*
 * NAME: change_device_state
 *
 * FUNCTION:This function is called by open_device() only
 *      when the tablet device is in the available state.
 *      The tablet is unconfigured and its state is
 *      changed to define.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in the Diagnostics subsystem CAS.
 *
 * RETURNS: 0 Good
 *          -1 BAD
 */

change_device_state(status)
int status;
{
        struct PdDv     *pddv_t;    /* for tablet device */
	struct PdDv	*pddv_kta;  /* for keyboard/tablet adapter */
        char    criteria[128];
        int     result=0;

	/* the structure, cudv_t, holds the tablet device name. this was */
	/* set in the open_device routine. */

        sprintf (criteria, "uniquetype=%s",cudv_t->PdDvLn_Lvalue);
        pddv_t = get_PdDv_list (PdDv_CLASS, criteria, &t_info, 1, 1);
        sprintf (option," -l %s", cudv_t->name);

	if (status == DEFINED)
	{
        	strcpy (lpp_unconfigure_method, pddv_t->Unconfigure);
        	result = odm_run_method(lpp_unconfigure_method,option,
                                &new_out,&new_err);
		if (result != 0)
        	{
                	sprintf(msgstr, (char *)diag_cat_gets (catd, 1,
                        	  DEVICE_CANNOT_UNCONFIGURE2), cudv_t->name,
                          	cudv_t->location);
                	menugoal (msgstr);
                	clean_up();
        	}
	}
	else
	{
		/* The structure, cudv_kta, holds the keyboard/tablet	  */
		/* device info.  This was set in the open_device routine. */
		/* We only configure the keyboard/tablet adapter if the	  */
		/* keyboard is not there.				  */
		if (k_info.num == 0) 
		{
                	sprintf (criteria, "uniquetype=%s",
				 cudv_kta->PdDvLn_Lvalue);
                	pddv_kta = get_PdDv_list(PdDv_CLASS, criteria, 
						 &kta_info, 1, 1);
	                sprintf (option_kta," -l %s", cudv_kta->name);

			strcpy (lpp_configure_method, pddv_kta->Configure);
			result = odm_run_method(lpp_configure_method,option_kta,
				&new_out,&new_err);
		}
		if (result == 0)  /* try to config tab iff success with kta */
		{
        		strcpy (lpp_configure_method, pddv_t->Configure);
        		result = odm_run_method(lpp_configure_method,option,
                                &new_out,&new_err);
		}
		if (result != 0) 
			report_fru(&frub_conf[0]);
		clean_up;
	}
}

/*
 * NAME: chk_ela
 *
 * FUNCTION:This function is not currently implemented because error logging
 *      of this device is not done.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in the Diagnostics subsystem CAS.
 *
 * RETURNS: 
 */

chk_ela()
{
return(0);
}
