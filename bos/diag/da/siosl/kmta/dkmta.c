static char sccsid[] = "@(#)19	1.10  src/bos/diag/da/siosl/kmta/dkmta.c, dasiosl, bos41J, 9515A_all 4/7/95 14:17:42";
/*
 * COMPONENT_NAME: Diagnostics application Keyboard, Mouse, and Tablet
 *  Adapter
 *
 * FUNCTIONS: main, keyboard_adapt, mouse_adapt, tablet_adapt, check_ela,
 *  select_menu_set, open_device, clean_up, unconfigure_lpp_device,
 *  determine_mach_type
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991,1994
 * All Rights Reserved
 *	
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<stdio.h>
#include	<fcntl.h>
#include	<nl_types.h>
#include	<memory.h>
#include	<locale.h>
#include	<sys/types.h>
#include	<sys/devinfo.h>
#include	<sys/cfgodm.h>
#include	<sys/signal.h>
#include	<sys/errno.h>

#include	"diag/diago.h"
#include	"diag/tm_input.h"
#include	"diag/tmdefs.h"
#include	"diag/da.h"
#include	"diag/da_rc.h"
#include	"diag/dcda_msg.h"
#include	"diag/diag_exit.h"
#include	"diag/atu.h" 
#include	"diag/modid.h"
#include	"dkbd_a_msg.h"
#include	"mouse_a_msg.h"
#include	"tab_a_msg.h"
#include	"dkmta.h"

#ifndef		TRUE
#define		TRUE	1
#endif

#ifndef		FALSE
#define		FALSE	0
#endif

/**************************************************************************/
/*									  */
/* IS_TM is a macro which takes two input variables: VAR1 & VAR2.         */
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
/* WRAP_DATA_NOT_EXEC is a macro which defines all the conditions where   */
/*		      the wrap data test unit will not excute.  	  */
/*									  */
/**************************************************************************/

#define WRAP_DATA_NOT_EXEC	( 	IS_TM( console, CONSOLE_FALSE  )   \
				   || 	IS_TM( system , SYSTEM_TRUE    )   \
				   || 	IS_TM( advanced, ADVANCED_FALSE)   \
				)

/**************************************************************************/
/*									  */
/* WRAP_DATA_TEST is a macro which defines all the conditions where       */
/*		  the wrap data test unit will excute.			  */
/*									  */
/**************************************************************************/

#define WRAP_DATA_TEST ( ! ( WRAP_DATA_NOT_EXEC ) )

/**************************************************************************/
/*									  */
/* DISPLAY_TESTING is a macro which defines when a title line will be     */
/*                 displayed.                				  */
/*									  */
/**************************************************************************/

#define DISPLAY_TESTING 						   \
		( 							   \
			( 						   \
				IS_TM( loopmode, LOOPMODE_NOTLM   )	   \
			   || 	IS_TM( loopmode, LOOPMODE_ENTERLM )	   \
			)						   \
			&&  	IS_TM( console, CONSOLE_TRUE	  )   	   \
		)

/**************************************************************************/
/*									  */
/* DISPLAY_LOOP_COUNT is a macro which defines when loop count will be    */
/*		      displayed. 					  */
/*									  */
/**************************************************************************/

#define DISPLAY_LOOP_COUNT 						   \
				(  					   \
				     (IS_TM( loopmode,LOOPMODE_INLM ) \
				    || IS_TM( loopmode,LOOPMODE_EXITLM )) \
				    && 	IS_TM( console,CONSOLE_TRUE )	   \
				)

/**************************************************************************/
/*									  */
/* PUT_EXIT_MENU is a macro which defines when there is an exit menu to   */
/*	 	 display.       				       	  */
/*									  */
/**************************************************************************/

#define PUT_EXIT_MENU		 					   \
		(							   \
 			(						   \
				IS_TM( loopmode, LOOPMODE_NOTLM     )	   \
		  	   || 	IS_TM( loopmode, LOOPMODE_EXITLM    )	   \
			) 						   \
			&& ( IS_TM( system, SYSTEM_FALSE       )   )	   \
			&& ( IS_TM( console, CONSOLE_TRUE      )   )	   \
		)

/**************************************************************************/
/*									  */
/* DISPLAY_MENU is a macro which defines when a menu should be displayed. */
/*									  */
/**************************************************************************/

#define DISPLAY_MENU	 						   \
		( 							   \
			( 						   \
				IS_TM( loopmode, LOOPMODE_NOTLM   )	   \
			   || 	IS_TM( loopmode, LOOPMODE_ENTERLM )	   \
			)						   \
			&& ( IS_TM( console, CONSOLE_TRUE)	  )	   \
			&& ( IS_TM( system, SYSTEM_FALSE)	  )	   \
		)

/**************************************************************************/
/*									  */
/* 	da_title is an array of structures which holds the message set	  */
/*	number and a message id number in the menus catalog file.	  */
/*									  */
/**************************************************************************/

struct	msglist	kda_title[]=
/*			keyboard					  */
	{
			{  DA_TITLE,	KDB_TITLE	},
			{  DA_TITLE,	KDB_STND_BY	},
			{  0,	        0		},
	};

struct	msglist	mda_title[]=
/*   			mouse					          */
	{
			{  MSE_GENERIC,	MSE_DA_TITLE	},
			{  MSE_GENERIC,	MSE_STND_BY	},
			{  0,	        0		},
	};

struct	msglist	tda_title[]=
/*			tablet						  */
	{
			{  TBL_GENERIC,	TBL_TITLE	},
			{  TBL_GENERIC,	TBL_STND_BY	},
			{  0,	        0		},
	};

struct	msglist *da_title[]=
	{ kda_title, mda_title, tda_title };

/**************************************************************************/
/*									  */
/* 	ask_mse is an array of structures which holds the message set	  */
/*	number and a message id number in the menus catalog file.	  */
/*									  */
/*	This structure will be passed to diag_display to ask the user	  */
/*	if a mouse is currently attached to the system.			  */
/*									  */
/**************************************************************************/

struct	msglist	ask_mse[]=
		{
			{  MSE_GENERIC,		MSE_DA_TITLE_A	},
			{  MSE_ASK_USR, 	MSE_1	},
			{  MSE_ASK_USR, 	MSE_2	},
			{  MSE_ASK_USR, 	MSE_IS_A_MSE	},
			{  0,		        0		},
		};

/***********************************************************************/
/*									  */
/* 	rmv_cable is an array of structures which holds the message set	  */
/*	number and a message id number in the menus catalog file.	  */
/*									  */
/*	This structure will be passed to diag_display to ask the user	  */
/*	to remove the cable if attached.				  */
/*									  */
/**************************************************************************/

struct	msglist	mrmv_cable[]=
/*   			mouse					          */
		{
			{  MSE_GENERIC,		MSE_DA_TITLE_A	},
			{  MSE_RMV_ATTACHEMENT,	MSE_UNPLUG_CBL	},
			{  MSE_GENERIC,		MSE_PRS_ENTER	},
			{  0,		        0		},
		};
struct	msglist	trmv_cable[]=
/*			tablet						  */
		{
			{  TBL_GENERIC,		TBL_TITLE_A	},
			{  TBL_RMV_ATTACHEMENT,	TBL_UNPLUG_CBL	},
			{  TBL_GENERIC,		TBL_PRS_ENTER	},
			{  0,		        0		},
		};

/**************************************************************************/
/*									  */
/* 	connect_cbl is an array of structures which holds the message set */
/*	number and a message id number in the menus catalog file.	  */
/*									  */
/*	This structure will be passed to diag_display to ask the user	  */
/*	to connect the mouse cable if it was removed.			  */
/*									  */
/**************************************************************************/

struct	msglist	m_connect_cbl[]=
/*   			mouse					          */
		{
			{  MSE_GENERIC,		MSE_DA_TITLE_A	},
			{  MSE_RECONNECT,	MSE_PLUG_CBL	},
			{  MSE_GENERIC,		MSE_PRS_ENTER	},
			{  0,		        0		},
		};
struct	msglist	t_connect_cbl[]=
/*			tablet						  */
		{
			{  TBL_GENERIC,		TBL_TITLE_A	},
			{  TBL_RECONNECT,	TBL_PLUG_CBL	},
			{  TBL_GENERIC,		TBL_PRS_ENTER	},
			{  0,		        0		},
		};

struct  msgtable
{
        struct  msglist *mlp;
        short   msgtype;
};

struct  msgtable msgtab[]=
        {
                { mrmv_cable,           0      },
                { trmv_cable,           0      },
                { m_connect_cbl,        1      },
                { t_connect_cbl,        1      },
		{ ask_mse,		2      },
        };

struct  msgtable *mtp;

ASL_SCR_TYPE	menutypes= DM_TYPE_DEFAULTS ;
/**************************************************************************/
/*									  */
/*	tmode structure holds information related to the diagnostic 	  */
/*	application.                  					  */
/*									  */
/*	fields in the structure:                                          */
/*                                                                        */
/*	tu_num : test unit number which will passed to the test unit to   */
/*		 test certain logic on the device.			  */
/*									  */
/*	rmvd   : indicates that the child device has been removed for     */
/*               testing.						  */
/*									  */
/**************************************************************************/
 
struct
{
	short	tu_num;		/* tu test number.			  */
	short	rmvd;           /* child device removed.                  */
} tmode[] =
{
/*		keyboard						  */
	{ KDB_RESET, 	FALSE },
	{ KMT_FUSE_CHK, FALSE }, 
	{ KDB_SPKR_REG, FALSE },
/*		mouse							  */ 
	{ MSE_READ_STATUS, FALSE },
	{ MSE_POS_REG,	FALSE },
	{ KMT_FUSE_CHK, FALSE },
        { MSE_NON_BLOCK, FALSE },
	{ MSE_WRAP_DATA, FALSE },
/*		tablet							  */ 
	{ TBL_INT_WRAP, FALSE },
	{ KMT_FUSE_CHK, FALSE },
/*		keyboard tu_99						  */	
	{ KBD_CLN,	FALSE },
};

/**************************************************************************/
/*									  */
/*	frub_open structure holds information related to FRU bucket       */
/*	variables. This set is based on results received after            */
/*	unsuccessfully attempting to open keyboard, mouse or tablet       */
/*	device.	      					        	  */
/*									  */
/**************************************************************************/
 
struct	fru_bucket	frub_open[] =
{
	{ "", FRUB1,  0  ,  0x332  , DKBDA_OPEN ,
			{ 
				{ 80 , "" , "" , 0, DA_NAME, EXEMPT	  },
				{ 20 ,"Software","",DKBDA_SW,NOT_IN_DB,EXEMPT },
			},
	},
	{ "", FRUB1,  0  ,  0x134  , MSE_A_OPEN ,
			{ 
				{ 80 , "" , "" , 0, DA_NAME, EXEMPT	  },
				{ 20 ,"Software","",MSE_A_SW,NOT_IN_DB,EXEMPT },
			},
	},
	{ "", FRUB1,  0  ,  0x524  , TBL_A_OPEN ,
			{ 
				{ 80 , "" , "" , 0, DA_NAME, EXEMPT	  },
				{ 20 ,"Software","",TBL_A_SW,NOT_IN_DB,EXEMPT },
			},
	},
};

/**************************************************************************/
/*									  */
/*	frub_config structure holds information related to FRU bucket     */
/*	variables. This set is based on results received after            */
/*	unsuccessfully attempting to configure keyboard, mouse or tablet  */
/*	device.	      					        	  */
/*									  */
/**************************************************************************/
 
struct	fru_bucket	frub_config[] =
{
	{ "", FRUB1,  0  ,  0x331  , DKBDA_CONF ,
			{ 
				{ 100 , "" , "" , 0, DA_NAME, EXEMPT  },
			},
	},
	{ "", FRUB1,  0  ,  0x133  , MSE_A_CONF ,
			{ 
				{ 100 , "" , "" , 0, DA_NAME, EXEMPT },
			},
	},
	{ "", FRUB1,  0  ,  0x523  , TBL_A_CONF ,
			{ 
				{ 100 , "" , "" , 0, DA_NAME, EXEMPT  },
			},
	},
};

/**************************************************************************/
/*									  */
/*	frub_kbd_reset structure holds information related to FRU bucket  */
/*	variables. This set is based on possible results received after	  */
/*	running keyboard adapter TU test #10.				  */
/*	One FRUB will be reported when reset test fails.		  */
/*									  */
/**************************************************************************/
 
struct	fru_bucket	frub_kbd_reset[]=
{
	{ "", FRUB1,  0x821  ,  0x111  , DKBDA_UNEXPECTED ,
			{ 
				{ 90 , "" , "" , 0, DA_NAME,	 EXEMPT	  },
				{10,"keyboard","",DKBDA_KBD,NOT_IN_DB,EXEMPT },
			},
	},
	{ "", FRUB1,  0x821  ,  0x310 , DKBDA_REG_TEST ,
			{ 
				{ 100 , "" , "" , 0, DA_NAME,	 EXEMPT	  },
			},
	},
	{ "", FRUB1,  0x821  ,  0x311 , DKBDA_LOGIC_ERR ,
			{ 
				{ 100 , "" , "" , 0, DA_NAME,	 EXEMPT	  },
			},
	},
	{ "", FRUB1,  0x821  ,  0x312 , DKBDA_XMIT_ERR ,
			{ 
				{ 100 , "" , "" , 0, DA_NAME,	 EXEMPT	  },
			},
	},
	{ "", FRUB1,  0x821  ,  0x313 , DKBDA_REC_ERR ,
			{ 
				{ 100 , "" , "" , 0, DA_NAME,	 EXEMPT	  },
			},
	},
};
 
/**************************************************************************/
/*									  */
/*	frub_kfuse structure holds information related to FRU bucket      */
/*	variables. This set is based on possible results received after   */
/*	running keyboard adapter TU test #20.				  */
/*	One FRUB will be reported when fuse test fails.	        	  */
/*									  */
/**************************************************************************/
 
struct	fru_bucket	frub_kfuse[] =
{
	{ "", FRUB1,  0  ,  0x111  , DKBDA_UNEXPECTED ,
			{ 
				{ 100 , "" , "" , 0, DA_NAME, EXEMPT  },
			},
	},
	{ "", FRUB1,  0  ,  0x220  , DKBDA_FUSE_ERR ,
			{ 
				{ 90 ,"Fuse","",DKBDA_FUSE,NOT_IN_DB,EXEMPT },
				{ 10 , ""   ,"", 0, DA_NAME, EXEMPT          },
			},
	},
};

/**************************************************************************/
/*									  */
/*	frub_spkr structure holds information related to FRU bucket       */
/*	variables. This set is based on possible results received after   */
/*	running keyboard adapter TU test #30.				  */
/*	One FRUB will be reported when speaker register test fails.	  */
/*									  */
/**************************************************************************/
 
struct	fru_bucket	frub_spkr[] =
{
	{ "", FRUB1,  0x821  ,  0x111  , DKBDA_UNEXPECTED ,
			{ 
				{ 100 , "" , "" , 0, DA_NAME, EXEMPT  },
			},
	},
	{ "", FRUB1,  0x821  ,  0x330  , DKBDA_SPKR_FAILURES ,
			{ 
				{ 100 , "" , "" , 0, DA_NAME, EXEMPT  },
			},
	},
};

/**************************************************************************/
/*									  */
/*	frub_read_status structure holds information related to	FRU bucket*/
/*	variables. This set is based on possible results received after   */
/*	running mouse adapter TU test #10.				  */
/*	One of these frus will be reported if a failure return code	  */
/*	is returned from the test unit.					  */
/*									  */
/**************************************************************************/
 
struct fru_bucket frub_read_status[]=
{
	{ "", FRUB1,  0x823  ,  0x111  , MSE_A_UNEXPECTED ,
			{
				{ 100, "", "", 0, DA_NAME, EXEMPT },
			},
	},
	{ "", FRUB1,  0x823  ,  0x112  , MSE_A_REGISTER , 
			{
				{ 100, "", "", 0, DA_NAME, EXEMPT },
			},
	},
};

/**************************************************************************/
/*                                                                        */
/*      frub_pos_reg structure holds information related to FRU bucket    */
/*      variables. This set is based on possible results received after   */
/*      running mouse adapter TU test #15.                                */
/*      One of these frus will be reported if a failure return code       */
/*      is returned from the test unit.                                   */
/*                                                                        */
/**************************************************************************/

struct fru_bucket frub_pos_reg[]=
{
        { "", FRUB1,  0x823  ,  0x111  , MSE_A_UNEXPECTED ,
                        {
                                { 100, "", "", 0, DA_NAME, EXEMPT },
                        },
        },
        { "", FRUB1,  0x823  ,  0x112  , MSE_A_REGISTER ,
                        {
                                { 100, "", "", 0, DA_NAME, EXEMPT },
                        },
        },
};

/**************************************************************************/
/*									  */
/*	frub_mfuse structure holds information related to FRU bucket      */
/*	variables. This set is based on possible results received from    */
/*	running mouse adapter TU test #20.				  */
/*	One FRUB will be reported when the fuse test fails.        	  */
/*									  */
/**************************************************************************/
 
struct	fru_bucket	frub_mfuse[] =
{
	{ "", FRUB1,  0  ,  0x111  , MSE_A_UNEXPECTED ,
			{ 
				{ 100 , "" , "" , 0, DA_NAME, EXEMPT  },
			},
	},
	{ "", FRUB1,  0  ,  0x220  , MSE_A_FUSE_ERR ,
			{ 
				{ 90 ,"Fuse","",MSE_A_FUSE, NOT_IN_DB,EXEMPT },
				{ 10 , ""   ,"", 0, DA_NAME, EXEMPT          },
			},
	},
};

/**************************************************************************/
/*									  */
/*	frub_nonb_status structure holds information related to FRU bucket*/
/*	variables. This set is based on possible results received from    */
/*	running mouse adapter TU test #25.				  */
/*	One of these frus will be reported if a failure return code is	  */
/*	returned from the test unit.					  */
/*									  */
/**************************************************************************/

struct	fru_bucket frub_nonb_status[]=
{
	{ "", FRUB1,  0x823  ,  0x111  , MSE_A_UNEXPECTED ,
			{
				{ 100 , "", "", 0, DA_NAME,     EXEMPT	 },
			},
	},
	{ "", FRUB1,  0x823    ,  0x131    , MSE_A_NONBLOCK_FAILED ,
			{
				{ 80  ,"", "", 0, DA_NAME,	EXEMPT    },
				{ 20,"Mouse","",MSE_A_B_NONB,NOT_IN_DB,EXEMPT},
			},
	},
};

/**************************************************************************/
/*									  */
/*	frub_wrap_data structure holds information related to FRU bucket  */
/*	variables. This set is based on possible results received from    */
/*	running mouse adapter TU test #30.				  */
/*	One of these frus will be reported if a failure return code is	  */
/*	returned from the test unit.					  */
/*									  */
/**************************************************************************/
 
struct	fru_bucket frub_wrap_data[]=
{
	{ "", FRUB1,  0x823  ,  0x111  , MSE_A_UNEXPECTED ,
			{
				{ 100, "", "", 0, DA_NAME, EXEMPT },
			},
	},
	{ "", FRUB1,  0x823  ,  0x122  , MSE_A_IWRAP ,
			{
				{ 100, "", "", 0, DA_NAME, EXEMPT },
			},
	},
};

/**************************************************************************/
/*									  */
/*	frub_disable structure holds information related to FRU bucket	  */
/*	variables.  This set is based on possible results received from   */
/*	running tablet adapter TU test #10.				  */
/*	One of these frus will be reported if a failure return code is	  */
/*	returned from the test unit.					  */
/*									  */
/**************************************************************************/
 
struct fru_bucket frub_disable[]=
{

	{ "", FRUB1,  0x824  ,  0x511  , TBL_A_UNEXPECTED ,
			{
				{ 100 , "" , "", 0, DA_NAME,	EXEMPT	   },
			},
	},
	{ "", FRUB1,  0x824  ,  0x512  , TBL_A_RESET ,
			{
				{ 100, "" , "", 0, DA_NAME,	EXEMPT	  },
			},
	},
};

/**************************************************************************/
/*									  */
/*	frub_tfuse structure holds information related to FRU bucket      */
/*	variables. This set is based on possible results received from	  */
/*	running tablet adapter TU test #20.				  */
/*	One FRUB will be reported when the fuse test fails.	       	  */
/*									  */
/**************************************************************************/
 
struct	fru_bucket	frub_tfuse[] =
{
	{ "", FRUB1,  0  ,  0x511  , TBL_A_UNEXPECTED ,
			{ 
				{ 100 , "" , "" , 0, DA_NAME, EXEMPT  },
			},
	},
	{ "", FRUB1,  0  ,  0x220  , TBL_A_FUSE_ERR ,
			{ 
				{ 90 ,"Fuse","",TBL_A_FUSE, NOT_IN_DB,EXEMPT },
				{ 10 , ""   ,"", 0, DA_NAME, EXEMPT          },
			},
	},
};

/**************************************************************************/

struct	tm_input	tm_input;
struct	CuDv	*cudv,*cudv_o;
struct	objlistinfo	c_info;
TUTYPE tucb_sio;
char	crit[100];

int	filedes = ERR_FILE_OPEN;	/* Pointer to the device address  */

nl_catd	catd = CATD_ERR;		/* pointer to the catalog file	  */ 
int	device_state=0;			/* previous status of device	  */
int 	dev_type = 0;			/* type of the device		  */	
int 	in_nonb = TRUE;
short	is_mouse = FALSE;
int	odm_good = FALSE;
int	asl_stat_flag = FALSE;
int 	open_err = FALSE;
int	kbd_dev = FALSE;	
unsigned long	f_code;  
unsigned short msg_inloop;
unsigned short loop_title;

char    *msgstr;
char    option[256];
char    new_out[256], new_err[256];
char    *lpp_configure_method;
char    *lpp_unconfigure_method;
int     unconfigure_lpp = FALSE;
int	rsc_type;

/*	Local functions prototyping.			    	 	   */

void	clean_up();
void	int_handler(int);
void	open_device(int);
int	chk_asl_stat(long);
int	chk_terminate_key();
int	check_ela(); 
int 	checkerrorcode(int,int);
int 	keyboard_adapt();
int 	mouse_adapt();
int	select_menu_set(struct msgtable *);
int 	report_fru(struct fru_bucket *); 
int	tu_test(int,int);
int 	tablet_adapt();
int	timeout(int);
int     unconfigure_lpp_device();
int	determine_mach_type();

/*	External functions prototyping.					   */

extern 	int 	errno;
extern	int 	addfrub();
extern	int	insert_frub();
extern  nl_catd diag_catopen(char *, int);
extern	int	kexectu(int, TUTYPE *);
extern	int	mexectu(int, TUTYPE *);
extern	int	texectu(int, TUTYPE *);
extern  int	get_cpu_model(int *);
/*
 * NAME: main()
 *
 * FUNCTION: Main driver for SALMON NIO Keyboard, Mouse, and Tablet
 *           Diagnostic applications.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described 
 *	in the Diagnostics subsystem CAS.
 *
 * RETURNS:  (NONE)
 */

main( int argc, char *argv[] ) 
{
	char *cat_type;
	struct	sigaction act;	/* interrupt handler structure	   */

	setlocale(LC_ALL,"");

	DA_SETRC_STATUS ( DA_STATUS_GOOD );
	DA_SETRC_ERROR ( DA_ERROR_NONE );
	DA_SETRC_TESTS( DA_TEST_FULL );
	DA_SETRC_MORE ( DA_MORE_NOCONT );
	DA_SETRC_USER ( DA_USER_NOKEY );

	msgstr = (char *) malloc (1200*sizeof(char));
        lpp_configure_method = (char *) malloc (256*sizeof(char));
        lpp_unconfigure_method = (char *) malloc (256*sizeof(char));

        if ((msgstr == (char *) NULL) ||
           (lpp_configure_method == (char *) NULL) ||
           (lpp_unconfigure_method == (char *) NULL))
        {
                clean_up();
        }

/*	Initialize interrupt handler structure.   		       	   */

	act.sa_handler = int_handler;
	sigaction( SIGINT, &act,( struct sigaction *)NULL);

/*      Initialize odm.		         	        	           */

	if (init_dgodm() == ERR_FILE_OPEN)	
	{
		DA_SETRC_ERROR( DA_ERROR_OTHER );
		clean_up();
	}
	else
		odm_good = TRUE;	

/*      Get diagnostic TM Input.	       	        	           */

	if(getdainput( &tm_input ) !=0 )
	{
		DA_SETRC_ERROR( DA_ERROR_OTHER );
		clean_up();
	}

/*	Determine which type of device we are supposed to test. Assign     */
/* 	an appropriate value to different variables depending on what      */
/*	is returned.							   */

	sprintf( crit, "name = %s", tm_input.dname );
	cudv = get_CuDv_list( CuDv_CLASS, crit, &c_info, 1, 2 );
	if ((cudv == (struct CuDv *) -1) || (cudv == (struct CuDv *) NULL))
	{
        	if ( IS_TM( console, CONSOLE_TRUE ) )
		DA_SETRC_ERROR(DA_ERROR_OTHER);
		clean_up();
	}
	
	f_code = cudv->PdDvLn->led;
	switch ( f_code )
	{
		case 0x821:
			dev_type = KBD_ADP;
			cat_type = MF_DKBD_A;
			msg_inloop = DKB_INLOOP;
			loop_title = DKB_INLOOP_TITLE;
			break;
		case 0x823:
			dev_type = MSE_ADP;
			cat_type = MF_MOUSE_A;
			msg_inloop = MSE_INLOOP;
			loop_title = MSE_INLOOP_TITLE;
			break;
		case 0x824:
			dev_type = TAB_ADP;
			cat_type = MF_TAB_A;
			msg_inloop = TBL_INLOOP;
			loop_title = TBL_INLOOP_TITLE;
			break;
		default:
        		if ( IS_TM( console, CONSOLE_TRUE ) )
        		{
                		chk_asl_stat( diag_asl_init("NO_TYPE_AHEAD") );
                		asl_stat_flag = TRUE;
        		}
			DA_SETRC_ERROR( DA_ERROR_OTHER );
			clean_up();
			break;
	}
/*	Shift LED value into menu ID position.				  */

	f_code <<= 12;

/*      Check if advanced mode. If so, change title number.		  */

        if ( IS_TM( advanced, ADVANCED_TRUE ) )
		da_title[dev_type]->msgid++;

/*      If running from console, initialize asl and open catalog file.     */

        if ( IS_TM( console, CONSOLE_TRUE ) )   
        {
                chk_asl_stat( diag_asl_init("NO_TYPE_AHEAD") );
		asl_stat_flag = TRUE;
	 	catd = diag_catopen( cat_type,0);
	}

/* 	Put up first screen - either loop screen or title screen.          */

        if ( DISPLAY_LOOP_COUNT )
		chk_asl_stat(diag_msg_nw(f_code + 0x110,catd,msg_inloop,
                        loop_title, tm_input.lcount,tm_input.lerrors));

        else if ( DISPLAY_TESTING )
	     chk_asl_stat( diag_display(f_code + 0x001,catd,da_title[dev_type],
                                DIAG_IO, ASL_DIAG_OUTPUT_LEAVE_SC, NULL,NULL));
	
/* 	Get out if exit loopmode and running keyboard adapter tests.	   */
/*	Mouse and tablet adapters both have possible menus to display.     */  
	
	if ((IS_TM(loopmode, LOOPMODE_EXITLM)) && ( dev_type == KBD_ADP))
		clean_up();
	
	(void) memset (&tucb_sio, 0, sizeof(tucb_sio));
	sleep(2); /* work around until asl flush stdout */
	signal(SIGALRM,timeout);
	alarm(TIMEOUT);
	open_device(dev_type);
	alarm(0);

/*      Call individual DA to be executed.                                 */ 	

	rsc_type = determine_mach_type();
	/* diag_asl_msg ("machine type is %d\n", rsc_type); */

	switch (dev_type)
	{
		case KBD_ADP:
			keyboard_adapt ();
			break;
		case MSE_ADP:
			mouse_adapt ();
			break;
		case TAB_ADP:
			tablet_adapt ();
			break;
		default:
			DA_SETRC_ERROR( DA_ERROR_OTHER );
			clean_up();
			break;
	}

/*      Do error log analysis.						    */

	if ((IS_TM( dmode, DMODE_ELA)) || (IS_TM( dmode, DMODE_PD)))
		check_ela ();        
	chk_terminate_key();

/*      Get out.							    */

	DA_SETRC_TESTS( DA_TEST_FULL );
	clean_up( );
}
/*	Main end				      			    */

/*
 * NAME: keyboard_adapt()
 *
 * FUNCTION: Designed to execute the test units if the test unit has
 *	     the correct environment mode. If the test unit executes
 *	     without any error then this function will return to main.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in the Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */
 
int keyboard_adapt() 
{
	int 	array_ptr = 0;
   	int 	error_num = 0; 	       /* error number from the test unit */
	int	fru_index = 0;
	int 	mrc = 0;

/* 	Don't ask user to remove child device if it failed because        */
/*	would then have no input device.				  */ 

/* 	Run non-interactive TU test 10                                    */

	chk_terminate_key();

	error_num = tu_test( KBD_ADP, array_ptr = PTR_RESET_TST );  
	if (error_num != 0)
	{
		fru_index = checkerrorcode(array_ptr, error_num);
 		report_fru(&frub_kbd_reset[fru_index]);
	}
/*	Disable the keyboard */
	if (filedes != -1)       
		close(filedes);
	
	if ( DISPLAY_LOOP_COUNT )
		sleep(2);
/* 	If TU test 10 passed, run non-interactive TU test 20.              */

	chk_terminate_key();
        if ( DISPLAY_LOOP_COUNT )
		chk_asl_stat(diag_msg_nw(f_code + 0x120,catd,msg_inloop,
                        loop_title, tm_input.lcount,tm_input.lerrors));
        else if ( DISPLAY_TESTING )
	     chk_asl_stat( diag_display(f_code + 0x002,catd,da_title[dev_type],
                                DIAG_IO, ASL_DIAG_OUTPUT_LEAVE_SC, NULL,NULL));
	if (rsc_type != 3) /* can't check fuse on SMP machines */
	{
		error_num = tu_test( KBD_ADP, array_ptr = PTR_KFUSE_TST );  
		if (error_num != 0)
		{
			fru_index = checkerrorcode(array_ptr, error_num);
			report_fru(&frub_kfuse[fru_index]);
		}       
	}       
	if ( DISPLAY_LOOP_COUNT )
		sleep(2);
/*	If both TU tests 10 and 20 passed, run non-interactive TU test 30. */

	chk_terminate_key();
        if ( DISPLAY_LOOP_COUNT )
		chk_asl_stat(diag_msg_nw(f_code + 0x130,catd,msg_inloop,
                        loop_title, tm_input.lcount,tm_input.lerrors));
        else if ( DISPLAY_TESTING )
	     chk_asl_stat( diag_display(f_code + 0x003,catd,da_title[dev_type],
                                DIAG_IO, ASL_DIAG_OUTPUT_LEAVE_SC, NULL,NULL));
	error_num = tu_test( KBD_ADP, array_ptr = PTR_SPKR_REG );  
	if (error_num != 0)
	{
		fru_index = checkerrorcode(array_ptr, error_num);
		report_fru(&frub_spkr[fru_index]);
	}

	if ( DISPLAY_LOOP_COUNT )
		sleep(2);
}

/*
 * NAME: mouse_adapt()
 *
 * FUNCTION: Designed to execute the test units if the test unit has
 *	     the correct environment mode. If the test unit executes
 *	     without any error then this function will return to main.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: none
 */
 
int mouse_adapt()
{ 
	int     wrap_fail = FALSE;
	int	array_ptr = 3;
   	int 	error_num = 0;		/* error number from the test unit*/
	int 	fru_index = 0;
	int	mrc = 0;

/*	Ask user to remove child device if it failed.			   */

	if ((DISPLAY_MENU) && IS_TM( state1, STATE_BAD ) 
          &&  IS_TM( advanced, ADVANCED_TRUE ) )
	{
		mtp = &msgtab[0];
		mrc = select_menu_set( mtp );
	}

/*	Run non-interactive TU test 10					  */
	
	chk_terminate_key();
	error_num = tu_test( MSE_ADP, array_ptr = PTR_READ_TST);  
	if (error_num != 0)
	{
		fru_index = checkerrorcode(array_ptr, error_num);
		report_fru(&frub_read_status[fru_index]);
	}
       
/*	If TU 10 passes,then run other non-interactive TUs.		  */

	chk_terminate_key();
        if ( DISPLAY_LOOP_COUNT )
		chk_asl_stat(diag_msg_nw(f_code + 0x120,catd,msg_inloop,
                        loop_title, tm_input.lcount,tm_input.lerrors));
        else if ( DISPLAY_TESTING )
	     chk_asl_stat( diag_display(f_code + 0x002,catd,da_title[dev_type],
                                DIAG_IO, ASL_DIAG_OUTPUT_LEAVE_SC, NULL,NULL));

/*	Determine the type of machine we are running. If it a RSC/POWER-PC,*/
/*	run TU 15.							   */
 	
	if (rsc_type != 0)
	{
        	error_num = tu_test( MSE_ADP, array_ptr = PTR_POS_TST);
        	if (error_num != 0)
        	{
                	fru_index = checkerrorcode(array_ptr, error_num);
                	report_fru(&frub_pos_reg[fru_index]);
        	}
	}
	if (rsc_type != 3) /* can't check fuse on SMP machines */
	{
		error_num = tu_test( MSE_ADP, array_ptr = PTR_MFUSE_TST);  
		if (error_num != 0)
		{
			fru_index = checkerrorcode(array_ptr, error_num);
			report_fru(&frub_mfuse[fru_index]);
		}
	}

/*	Have user replace previously removed child device, if necessary.   */

	if ((PUT_EXIT_MENU) && IS_TM( state1, STATE_BAD )
          &&  IS_TM( advanced, ADVANCED_TRUE ) )
	{
		mtp = &msgtab[2];
		mrc = select_menu_set( mtp );
	}

/*	Query user to find out if a mouse is attached.			   */
/*	Save value for later.						   */
		 
	if ((DISPLAY_MENU) && IS_TM( advanced, ADVANCED_TRUE )
	  && !(IS_TM( state1, STATE_BAD)))
	{
		mtp = &msgtab[4];
		mrc = select_menu_set ( mtp );
		if (mrc == YES)
		{
			is_mouse = TRUE;
                        putdavar(tm_input.dname,"ismou",DIAG_SHORT,
                                                &is_mouse );
		}
	}

/*	See if mouse originally was on system. This is for subsequent	   */
/*	passes in loopmode.						   */

        getdavar(tm_input.dname,"ismou",DIAG_SHORT, &is_mouse);

/*	Stop here if running system checkout or with no console.	   */

/*      Run TU test 25 to put mouse adapter in non-block mode on first     */
/*	pass if a mouse is present.					   */

	if (( WRAP_DATA_TEST )&&(is_mouse )) /*  Port Wrap data	  */
	{
		if ((IS_TM(loopmode, LOOPMODE_NOTLM ))
		|| (IS_TM(loopmode, LOOPMODE_ENTERLM)))
		{
		      chk_terminate_key();
		      error_num = tu_test( MSE_ADP, array_ptr = PTR_NBLK_TST);  
	              if (error_num != 0)
		      {
			      fru_index = checkerrorcode(array_ptr, error_num);
			 	report_fru(&frub_nonb_status[fru_index]);
		      }

/*	Ask user to remove the mouse.   			           */

		      	mtp = &msgtab[0];	
		      	mrc = select_menu_set ( mtp );
		}

/*	Run TU 30 - wrap data test - only if adapter is in non-block mode.  */

	      wrap_fail = FALSE; 	
 	      error_num = tu_test( MSE_ADP, array_ptr = PTR_MWRP_TST);
	      if (error_num != 0)
			wrap_fail = TRUE;		  

/*	Change database value to no mouse, so that next time DA is run    */
/*	it won't automatically assume a mouse is present.	          */    
/*	Tell user to replug the mouse if appropriate.			  */

		if (PUT_EXIT_MENU)
		{
			is_mouse = FALSE;
                        putdavar(tm_input.dname,"ismou",DIAG_SHORT,
                                                &is_mouse );
			mtp = &msgtab[2];
			mrc = select_menu_set ( mtp );
		}

/*	Wait until user has replaced mouse, if appropriate, before        */
/*	reporting FRU and exiting.					  */
 
		if (wrap_fail)
		{
		        fru_index = checkerrorcode(array_ptr, error_num);
			report_fru(&frub_wrap_data[fru_index]);
		}

	}
}

/*
 * NAME: select_menu_set()
 *
 * FUNCTION: Designed to ask the user if a mouse is attached, to tell the
 *           user to remove a cable from the NIO adapter or to 
 *           tell the user to insert a cable into the NIO
 *           adapter depending on the selection parameter.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described 
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: Integer (yes or no) for case ASK_MOUSE.
 */

int select_menu_set( struct msgtable *mtp )
{
	int rc = 0;

	menutypes.cur_index=YES;

	switch ( mtp->msgtype )
	{
		case 0:   /*   REMOVE_CABLE  */
		       chk_asl_stat(diag_display(f_code+0x003,catd,
				mtp->mlp,DIAG_IO,
			 	ASL_DIAG_KEYS_ENTER_SC,NULL,NULL));
			break;
		case 1:   /*   CONNECT_CABLE  */
	         	chk_asl_stat(diag_display(f_code+0x005,catd,
				mtp->mlp,DIAG_IO,
			        ASL_DIAG_KEYS_ENTER_SC,NULL,NULL));
			break;
		case 2:    /*   ASK_MOUSE   */
		        rc = chk_asl_stat(diag_display(f_code+0x008,catd,
			     mtp->mlp, DIAG_IO,
	                     ASL_DIAG_LIST_CANCEL_EXIT_SC,&menutypes,NULL));
			if (rc != 0)
				return( rc );
			else
			if (TRUE)   /* added to fool lint */
				return(menutypes.cur_index);
			else
				break;
	 }
}

/*
 * NAME: tablet_adapt()
 *
 * FUNCTION: Designed to execute the tablet adapter test units if the test
 *           unit has the correct environment mode.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */
 
int tablet_adapt() 
{
	int 	array_ptr = 8;
   	int 	error_num = 0;		/* error number from the test unit*/
	int 	fru_index = 0;
	int	mrc = 0;

/*	Ask user to remove child device if it failed.			   */

	if ((DISPLAY_MENU) && IS_TM( state1, STATE_BAD ) 
          &&  IS_TM( advanced, ADVANCED_TRUE ) )
	{
		mtp = &msgtab[1];
		mrc = select_menu_set( mtp );
	}

/*	Run non-interactive TU test 10.				          */

	chk_terminate_key();
	error_num = tu_test( TAB_ADP, array_ptr = PTR_TWRP_TST);  
	if (error_num != 0)
	{
		fru_index = checkerrorcode(array_ptr, error_num);
 		report_fru(&frub_disable[fru_index]);
	}

/*	If TU 10 passes, run non-interactive TU test 20.		  */
 
	chk_terminate_key();
        if ( DISPLAY_LOOP_COUNT )
		chk_asl_stat(diag_msg_nw(f_code + 0x120,catd,msg_inloop,
                        loop_title, tm_input.lcount,tm_input.lerrors));
        else if ( DISPLAY_TESTING )
	     chk_asl_stat( diag_display(f_code + 0x002,catd,da_title[dev_type],
                                DIAG_IO, ASL_DIAG_OUTPUT_LEAVE_SC, NULL,NULL));
	error_num = tu_test( TAB_ADP, array_ptr = PTR_TFUSE_TST);  
	if (error_num != 0)
	{
		fru_index = checkerrorcode(array_ptr, error_num);
		report_fru(&frub_tfuse[fru_index]);
	}

/*	Have user replace previously removed child device, if appropriate.  */

	if ((PUT_EXIT_MENU) && IS_TM( state1, STATE_BAD )
       	  &&  IS_TM( advanced, ADVANCED_TRUE ) )
	{
		mtp = &msgtab[dev_type+1];
		mrc = select_menu_set( mtp );
	}
}

/*
 * NAME: tu_test()
 *
 * FUNCTION: Designed to execute the test units if the test unit has
 *           the correct environment mode. If the test unit executed
 *           without any error returned then this function will return
 *           to the calling function. Otherwise this function will report
 *           to the diagnostic controller a FRU bucket failure indicating
 *           the failing problem and the program will stop the execution.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: index to fru bucket structure
 */

int tu_test( int test_type, int ary_ptr )
{
        int     e_num = 0;          /* error number from the test unit*/

        tucb_sio.header.tu = tmode[ary_ptr].tu_num;

/*	make (k,m,t)exectu call depending on which adapter is being tested */
	if (test_type == KBD_ADP)
        	e_num = kexectu (filedes, &tucb_sio);
	else if (test_type == MSE_ADP)
		e_num = mexectu (filedes, &tucb_sio);
	else if (test_type == TAB_ADP)
		e_num = texectu (filedes, &tucb_sio);
	/* diag_asl_msg ("tu#%d rc=%d\n", tucb_sio.header.tu, e_num); */
	return(e_num);
}

/*
 * NAME:
 *      checkerrorcode
 *
 * FUNCTION:
 *      This function switches on the return code from the execution of a
 *      tu and takes the appropriate action.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is called by the main procedure which is forked and
 *      exec'd by the diagnostic supervisor, which runs as a process.
 *      This function runs as an application under the diagnostic subsystem.
 *
 * (NOTES:)
 *      This function:
 *      Handles all non-zero return codes from keyboard, tablet, and mouse  
 *      adapter test units and takes the appropriate action.
 *
 * (RECOVERY OPERATION:)
 *      The tu code handles any and all hardware recovery
 *      procedures in the event of error condition.
 *      Software errors, such as failures to open, are handled by
 *      setting error flags and returning to the diagnostic
 *      supervisor.
 *
 * (DATA STRUCTURES:)
 *      None modified.
 *
 * RETURNS:
 *      NONE
 */ 

int    checkerrorcode( int index_num, int rtn_cd )
{
	int ret_code = 0;
	switch (rtn_cd)
	{	
		case -1:
		case 255: ret_code = 0;
			 break;
		case 12: if (index_num == PTR_RESET_TST) 
			{	
				 rtn_cd -= 9;
				 ret_code = rtn_cd;
			}
			 else if ((index_num == PTR_READ_TST) || 
				  (index_num == PTR_TWRP_TST) ||
				  (index_num == PTR_POS_TST))	
				 ret_code = 1;
			 else
			 {
				 DA_SETRC_ERROR(DA_ERROR_OTHER);
				 clean_up();
			 }
			 break;
		case 10:
		case 11:
		case 13: if (index_num == PTR_RESET_TST)
			{
				 rtn_cd -= 9;
				 ret_code = rtn_cd;
			}
			 else
			 {
				 DA_SETRC_ERROR(DA_ERROR_OTHER);
				 clean_up();
			 }
			 break;	
		case 20: if ((index_num == PTR_KFUSE_TST) ||
			     (index_num == PTR_MFUSE_TST) ||
			     (index_num == PTR_TFUSE_TST)) 	
				 ret_code = 1;
			 else
			 {
				 DA_SETRC_ERROR(DA_ERROR_OTHER);
				 clean_up();
			 }
			 break;
		case 22: if (index_num == PTR_MWRP_TST) 
				 ret_code = 1;
			 else
			 {
				 DA_SETRC_ERROR(DA_ERROR_OTHER);
				 clean_up();
			 }
			 break;
		case 25: if (index_num == PTR_NBLK_TST)
				 ret_code = 1;
			 else
			 {
				 DA_SETRC_ERROR(DA_ERROR_OTHER);
				 clean_up();
			 }
			 break;
		case 30: if (index_num == PTR_SPKR_REG) 
				 ret_code = 1;
			 else
			 {
				 DA_SETRC_ERROR(DA_ERROR_OTHER);
				 clean_up();
			 }
			 break;
		default: DA_SETRC_ERROR(DA_ERROR_OTHER);
			 clean_up();
	}
	return(ret_code);
}

/*
 * NAME: report_fru()
 *
 * FUNCTION: Designed to pass a FRU bucket and an appropriate exit code 
 *	     to the controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */
 
int report_fru( struct fru_bucket *fru_buckets) 
{
	int	rc = 0;

	strncpy(fru_buckets->dname,tm_input.dname,NAMESIZE);
	rc = insert_frub( &tm_input,fru_buckets);
	if (rc != 0) 
	{
		DA_SETRC_ERROR ( DA_ERROR_OTHER );
		clean_up();
	}
	rc = addfrub(fru_buckets);
	if (rc != 0)
	{
		DA_SETRC_ERROR ( DA_ERROR_OTHER );
		clean_up();
	}
	if ( IS_TM(system, SYSTEM_FALSE))
		DA_SETRC_MORE(DA_MORE_CONT);

	DA_SETRC_STATUS( DA_STATUS_BAD );
	DA_SETRC_TESTS( DA_TEST_FULL );

	clean_up();
}

/*
 * NAME: check_ela()
 *
 * FUNCTION: Will check the error log to see if any errors have been logged
 *	     by the device driver. This function is not currently implemented.	
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

int check_ela()
{
	return(0);
} 

/*
 * NAME: clean_up()
 *
 * FUNCTION: Exits from the DA and returns an appropriate return
 *	     code to the Diagnostic controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described 
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */
 
void clean_up()
{

/*	Close the device. If device was previously opened, return it to     */
/*	it's initial state and terminate ODM. If running from console       */
/*	quit asl. Exit from the DA and return to controller.                */
 	
	int 	rc = 0;
	int array_ptr = 0;

	if (tucb_sio.mach_fd != ERR_FILE_OPEN)
	{
        	if (dev_type == KBD_ADP)     /* call keyboard test unit #99 */
                	tu_test( KBD_ADP, array_ptr = PTR_KCLN_TST );
		close(tucb_sio.mach_fd);
	}

	if (kbd_dev == FALSE)
	{	
	    if (device_state == AVAILABLE)
	    {
		if (open_err == FALSE)
		{
			if (unconfigure_lpp == TRUE)
			{
				sprintf (option," -l %s",cudv_o->name);
				rc = odm_run_method(lpp_configure_method,option,
					&new_out,&new_err);
				if (rc != NO_ERROR)
				{
				   sprintf(msgstr, (char *)diag_cat_gets (catd, 
				      1, DEVICE_CANNOT_CONFIGURE),
				      cudv_o->name,cudv_o->location);
				   menugoal (msgstr);
				}
			}
		}
	    }
	
	}
	
	if (odm_good)	
		term_dgodm();
	if (asl_stat_flag)
		catclose( catd );
	if ( IS_TM( console, CONSOLE_TRUE ) )	/* Check if Console TRUE */
		diag_asl_quit(NULL);
	DA_EXIT(); 
}

/*
 * NAME: chk_terminate_key()
 *
 * FUNCTION: Designed to check if the user has asked to terminate the 
 * application by hitting the ESC or CANCEL function key.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described 
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

int chk_terminate_key( )
{
	if ( IS_TM( console, CONSOLE_TRUE ) )	/* Check if Console TRUE */
		chk_asl_stat(diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, NULL,NULL));
	return(0);
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

int chk_asl_stat(long returned_code)
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
	return(0);
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

int timeout(int sig)
{
	alarm(0);
	switch (dev_type)
	{	
		case KBD_ADP:	
			report_fru(&frub_kbd_reset[0]);
			break;
		case MSE_ADP:
			report_fru(&frub_read_status[0]);
			break;
		case TAB_ADP:
			report_fru(&frub_disable[0]);
			break;
		default:
			DA_SETRC_ERROR(DA_ERROR_OTHER);
			clean_up();
			break;
	}
}

/*
 * NAME: int_handler
 *                                                                    
 * FUNCTION: Perform general clean up on receipt of an interrupt
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	Executes in a diagnostics environment when an interrupt
 *      has been received.
 *                                                                   
 * RETURNS: NONE
 */  

void int_handler(int sig)
{
	int	array_ptr = 0;

	if ( IS_TM( console, CONSOLE_TRUE ) )
		diag_asl_clear_screen();
	clean_up();
}

/*
 * NAME: open_device
 *
 * FUNCTION: Opens the machine device driver and the actual device.
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in the Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

void open_device(int type_adap)
{
	int	err_code = 0;
	int	rc = 0; 
	char	devname[32];

        tucb_sio.mach_fd = open("/dev/bus0",O_RDWR | O_NDELAY);

        if (tucb_sio.mach_fd == ERR_FILE_OPEN)
	{
		alarm(0);
                DA_SETRC_ERROR (DA_ERROR_OPEN);
		clean_up();
        }
	sprintf( crit,"parent = %s AND chgstatus != 3",tm_input.dname );
	cudv_o = get_CuDv_list( CuDv_CLASS, crit, &c_info, 1, 2 );
	if (cudv_o == (struct CuDv *) -1) 
        {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }
	
	device_state = cudv_o->status;
	if ( strncmp(cudv_o->name,"kbd",3) )
	{
	  /* 
	     if the device is mouse or tablet and is available, open and 
	     unconfigure the device.
	  */ 
		if (device_state == AVAILABLE)
		{
			sprintf (devname,"/dev/%s",cudv_o->name);
			if ((filedes = OPEN_DD) == ERR_FILE_OPEN)
			{
				open_err = TRUE;
				err_code = errno;
				alarm(0);
				if ( err_code == EIO || err_code == ENODEV ||
			    	   err_code == ENXIO )
					report_fru(&frub_open[type_adap]);
				DA_SETRC_ERROR( DA_ERROR_OPEN );
				clean_up ();
			}
			close(filedes);
			rc = unconfigure_lpp_device();			
		}
	}
	else
	{
		kbd_dev = TRUE;	        	/* flag for Keyboard device */
		if (device_state == AVAILABLE)
		{
			sprintf(devname,"/dev/%s",cudv_o->name);
			if ((filedes = OPEN_DD) == ERR_FILE_OPEN)
			{
				DA_SETRC_ERROR( DA_ERROR_OPEN );
				clean_up();
			}
		}
		else
			filedes = -1;	
	}
}

/*
 * NAME: unconfigure_lpp_device 					
 * 
 * FUNCTION:This function is called by open_device() only   
 *	when the device is in the available state.	
 *      Here,the device is unconfigured and its state is 	
 *	changed to define.					
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in the Diagnostics subsystem CAS.
 *
 * RETURNS: 0 Good         	                       
 *          -1 BAD 
 */
                	              
int     unconfigure_lpp_device()
{
        struct PdDv     *pddv_p;
        char    criteria[128];
        int     result;

        sprintf (criteria, "uniquetype=%s",cudv_o->PdDvLn_Lvalue);
        pddv_p = get_PdDv_list (PdDv_CLASS, criteria, &c_info, 1, 1);
        strcpy (lpp_unconfigure_method, pddv_p->Unconfigure);
        strcpy (lpp_configure_method, pddv_p->Configure);

        sprintf (option," -l %s", cudv_o->name);
        result = odm_run_method(lpp_unconfigure_method,option,
                                &new_out,&new_err);
        
	if (result != -1)
        	unconfigure_lpp = TRUE;
        else
        {
		sprintf(msgstr, (char *)diag_cat_gets (catd, 1,
                          DEVICE_CANNOT_UNCONFIGURE), cudv_o->name,
                          cudv_o->location);
                menugoal (msgstr);
                clean_up();
        }
	return(0);
}			

/*
 * NAME: determine_mach_type
 *
 * FUNCTION: This function determines the type of machine we are testing.
 *
 * EXECUTION ENVIRONMENT:
 *      Environment must be a diagnostics environment which is described
 *      in the Diagnostics subsystem CAS.
 *
 * RETURNS: 0 - for RS1/RS2
 *	    1 - for RSC
 *	    2 - for PowerPC
 * 	    3 - for SMP (multiprocessor systems)
 */

int 	determine_mach_type()
{
	int	cuat_mod;
	int	ipl_mod;

	ipl_mod = get_cpu_model(&cuat_mod);
	/* diag_asl_msg ("mod-attribute %x-%x\n", ipl_mod, cuat_mod); */

/* 	
	02010041 and 02010045 - salmon (45 is in the field,41 early internals)
   	02010043        - cabeza
   	08010046        - rainbow 3 - POWER PC
   	02010047        - chaparral 
*/

/* !!!WARNING!!! The check for a multiprocessor system (IsPowerPC_SMP) must
		 be done before the check for PowerPC (IsPowerPC) or RSC */

	if ((IsPowerPC_SMP(ipl_mod)) || (IsPowerPC_SMP_A(ipl_mod)))
    		return (3); /* SMP system */
	if (IsPowerPC(ipl_mod))
    		return (2); /* PowerPC system */
	if (IS_RSC(cuat_mod))
    		return (1); /* RSC system */
    	return (0); /* RS1, RS2 systems */
}

