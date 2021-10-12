static char sccsid[] = "@(#)98	1.8  src/bos/diag/da/wga/dwga.c, dawga, bos411, 9428A410j 3/23/94 10:14:57";
/*************************************************************************
 *
 * COMPONENT_NAME: DAWGA  -  diagnostic application to test POWER Gt1x Graphics Card
 *
 * FUNCTIONS:  sys_tu_seq, interact_tu_seq,
 *            tu_test, stand_by_screen, loop_stand_by_screen, check_rc, tu_exit,
 *	       int_handler,  clean_up
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993,1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *                                                                   
 *****************************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <locale.h>

#include "diag/da.h"
#include "diag/diago.h"
#include "diag/diag.h"
#include "diag/tm_input.h"
#include "diag/tmdefs.h"
#include "diag/diag_exit.h"
#include "dwga_msg.h"
#include "exectu.h"


/**************************************************************************/
/* IS_TM is a macro that accepts two input variables VAR1 & VAR2.         */
/* VAR1 is an object of the structure tm_input.                           */
/* VAR2 is a variable or a defined value that will be compared to the     */
/*      tm_input object class variable.                                   */
/* NOTE: macro returns logical TRUE or FALSE                              */
/**************************************************************************/

#define IS_TM( VAR1, VAR2 ) ( (int) ( tm_input./**/VAR1 ) == /**/VAR2 )

/* Define all TUs to run */
int wga_tu_seq[] =
{
	/*2,3,4,5,6,7,9,10,41*/
	REGISTER_TU, VRAM_TU, PALETTE_TU, CURSOR_RAM_TU, INTERRUPT_TU,
	VIDEO_ROM_SCAN_TU, STRING_TEST_TU, FAST_COPY_TU,
	PIXEL8_STR_TST_TU
};

#define NUM_TU_SEQ 9

/* Define red, green and blue TU sequence */
int rgb_tu_seq[] =
{
	/*21,22,23*/
	RED_TU, GREEN_TU, BLUE_TU
};

/* Define TUs to be run in pretest mode */
int pretest_tu_seq[] =
{
	/*4,5,6,7*/
	PALETTE_TU, CURSOR_RAM_TU, INTERRUPT_TU, VIDEO_ROM_SCAN_TU 
};

#define NUM_IPL_TU 4

/* fru_bucket is a structure that holds information for the diagnostic 
   program to return to the diagnostic controller when a failure is found
   that needs to be reported. (FRU means Field Replacable Unit) */

struct fru_bucket frub[]=
{
	{"", FRUB1, 0x898, 0x110, R_BASE,  /* 100% BASE ADAPTER FAILED */
		{
		    {100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},

	{"", FRUB1, 0x898, 0x120, R_BASE,
		{
		        {85, "", "", 0, DA_NAME, NONEXEMPT},
			{10, "", "", 0, PARENT_NAME, NONEXEMPT},
			{5, "Cable", "", F_CABLE, NOT_IN_DB, EXEMPT},
		},
	},

	{"", FRUB1, 0x898, 0x880, R_BASE,
		{
		{65,"", "", 0, DA_NAME, NONEXEMPT},
		{35, "Monitor", "", F_MON, NOT_IN_DB, NONEXEMPT},
		},
	},

	{"", FRUB1, 0x898, 0x130, R_DISPLAY,
		{
		{100, "Monitor", "", F_MON, NOT_IN_DB, NONEXEMPT},
		},
	},
};

/* Set up menus to be displayed */
struct msglist da_title[] =
{
	{DWGA_MSGS, DWGA},
	{DWGA_MSGS, DWGA_STANDBY},
	(int)NULL
};

struct msglist l_da_title[] =
{
	{DWGA_MSGS, DWGA_L},
	(int)NULL
};

struct msglist havedisp[] =
{
	{DWGA_MSGS, DWGA},
	{DWGA_MSGS, DWGA_EXT_YES},
	{DWGA_MSGS, DWGA_EXT_NO},
	{DWGA_MSGS, DWGA_HAVEDISP},
	(int)NULL
};

struct msglist f3_exit[] =
{
	{DWGA_MSGS, DWGA},
	{DWGA_MSGS, F3_EXIT},
	(int)NULL
};

struct msglist color_screen[] =
{
	{DWGA_MSGS, DWGA},
	{DWGA_MSGS, DWGA_EXT_YES},
	{DWGA_MSGS, DWGA_EXT_NO},
	{DWGA_MSGS, COLOR_SCREEN},
	(int)NULL
};

struct msglist builtin[] =
{
	{DWGA_MSGS, DWGA},
	{DWGA_MSGS, DWGA_EXT_YES},
	{DWGA_MSGS, DWGA_EXT_NO},
	{DWGA_MSGS, DWGA_PDPQ},
	(int)NULL
};



static ASL_SCR_TYPE menutypes = DM_TYPE_DEFAULTS;
static ASL_SCR_INFO uinfo[DIAG_NUM_ENTRIES(havedisp)];
static ASL_SCR_INFO m_uinfo[DIAG_NUM_ENTRIES(da_title)];
static ASL_SCR_INFO l_uinfo[DIAG_NUM_ENTRIES(l_da_title)];
static ASL_SCR_INFO final[DIAG_NUM_ENTRIES(builtin)];
static ASL_SCR_INFO f3_uinfo[DIAG_NUM_ENTRIES(f3_exit)]; 


#define IS_CONSOLE  ( (int) (tm_input.console == CONSOLE_TRUE) )
#define MAXCOLORS 512
#define CHAR_TO_INT(x) x[0]<<24 | x[1]<<16 | x[2]<<8 | x[3]
int	errno_rc;

TUCB          tucb_ptr;    	/* pointer to the previous structure */
struct tm_input tm_input;    	/* info. from dc to this program     */

int             rc;        	/* to hold functions return codes    */
extern  nl_catd diag_catopen(char *, int);
nl_catd         catd;     	/* file descriptor for catalog file  */
void		int_handler(int);
int             i;		/* loop counter */
int		menunum = 0x898000;
int		l_menunum = 0x898010;
int		colornum = 0x898020;
int		colorselfnum = 0x898030;
int		cantrunrgb=FALSE;
int		tu_exit_flg = 0;
int		disp_attached = 0;
int		diag_asl_flg=FALSE;

extern		exectu();
extern          getdainput();
extern          addfrub();
struct  sigaction       invec;  /* interrupt handler structure     */
void	ret_handler(int);
/*#define DEBUG 1*/
#if 	DEBUG
FILE	*wgabug;
#endif

main()
{
    setlocale(LC_ALL,"");

    (void)memset (&tucb_ptr, 0, sizeof(TUCB));

    /* initialize interrupt handler */
    invec.sa_handler =  int_handler;
    sigaction( SIGINT, &invec, (struct sigaction *) NULL );
    sigaction( SIGTERM, &invec, (struct sigaction *) NULL );


#if	DEBUG
	wgabug=fopen("/tmp/wga.debug","a+");
#endif

    /* Initialize return codes to the controller */
    DA_SETRC_STATUS(DA_STATUS_GOOD);
    DA_SETRC_ERROR(DA_ERROR_NONE);
    DA_SETRC_USER(DA_USER_NOKEY);
    DA_SETRC_TESTS(DA_TEST_FULL);
    DA_SETRC_MORE(DA_MORE_NOCONT);
    catd = (nl_catd) -1;

    if ((rc = init_dgodm()) != 0) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
	DA_EXIT();
    }

    /* Call the external function to get the input environment. */
    if ((rc = getdainput(&tm_input)) != 0) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }


    for (i=0; i<4; ++i)
        strncpy(frub[i].dname,tm_input.dname,sizeof(frub[i].dname));


    if (IS_TM(console,CONSOLE_TRUE))
    {
        diag_asl_init(ASL_INIT_DEFAULT);
	diag_asl_flg=TRUE;
        catd = diag_catopen(MF_DWGA,0);
    }

    if (tm_input.loopmode == LOOPMODE_NOTLM)
        stand_by_screen();
    else
        loop_stand_by_screen();


    invec.sa_handler =  ret_handler;
    sigaction( SIGRETRACT, &invec, (struct sigaction *) NULL );

    /* TU initialization */
    tucb_ptr.header.mfg = 0;
    tucb_ptr.header.loop = 1;

    /* Check for pre-test */
    if (IS_TM (exenv,EXENV_IPL)) {
	for (i=0; i<NUM_IPL_TU; ++i)
	    tu_test(pretest_tu_seq[i]);
	clean_up();
    }

    if (tm_input.system == SYSTEM_TRUE) {
        sys_tu_seq();
    }
    else if (tm_input.loopmode == LOOPMODE_NOTLM) {
            sys_tu_seq();
	    if (IS_CONSOLE) 
		interact_tu_seq();
    }
    else { /* must be loop mode */
	 if (tm_input.loopmode == LOOPMODE_ENTERLM) {
	     sys_tu_seq();
	     if (IS_CONSOLE) 
	         interact_tu_seq();
	 }
	 else if (tm_input.loopmode == LOOPMODE_INLM)
	     sys_tu_seq();
    };

    clean_up();


} /* end main */


/***************************************************************************
*
* NAME: sys_tu_seq
*                                                                    
* FUNCTION: Calls tu_test to run test units 1 through 9
*
* EXECUTION ENVIRONMENT:
*                                                                   
*	Called by the main procedure.
*
* NOTES: 
*                                                                   
* RETURNS: NONE
*
***************************************************************************/

int
sys_tu_seq()
{
	for (i=0; i<NUM_TU_SEQ; ++i) { 
	    tu_test(wga_tu_seq[i]);
	    }
}
/* end sys_tu_seq */
        

/***************************************************************************
*
* NAME: interact_tu_seq
*                                                                    
* FUNCTION: Check if interactive test mode is invoked. If so, run TUs to
*           to verify the red, green and blue screen. If device under test
*           is the console, display red, green and blue by calling the setcolor
*	    routine. If device is not the console, call tu_test with the 
*	    rgb_tu_seq
*
* EXECUTION ENVIRONMENT:
*                                                                   
*	Called by the main procedure.
*
* NOTES: 
*                                                                   
* RETURNS: NONE
*
***************************************************************************/

int
interact_tu_seq()
{
	int color;

	/* run TU_CLOSE so that we can display a message */
    	if (tu_exit_flg == 1)
	{
		tu_exit();
    	}

    /* Ask if there is a monitor attached to the adapter */
    insert_msg(&havedisp,uinfo,&menutypes,ASL_DIAG_LIST_CANCEL_EXIT_SC);
    menutypes.cur_index = 1;
    rc = diag_display(colornum, catd, NULL, DIAG_IO,
                      ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes, uinfo);
    check_rc(rc);
    if (rc == DIAG_ASL_COMMIT)
	switch (DIAG_ITEM_SELECTED(menutypes)) {
	    case 1 : disp_attached = 1;
		     break;
  	    case 2 : 
	    default: clean_up();
	}

    /* Run RGB if display is attached */
    if (disp_attached) {
	    for (i=0; i<3; ++i) {
		  /* run TU_CLOSE so that we can display a message */
    		  if (tu_exit_flg == 1)
		  {
			tu_exit();
    		  }

		  /* get the color that is going to be displayed */
	        switch (i) 
		  {
			case 0 : /* RED */
				color = RED;
			 	break;
			case 1 : /* GREEN */
				color = GREEN;
			 	break;
			case 2 : /* BLUE */
				color = BLUE;
				break;
	        	default: 
				break;
	        }

	        /* Put up screen telling that a color will be displayed */
              insert_color(&f3_exit,f3_uinfo,&menutypes,
						ASL_DIAG_KEYS_ENTER_SC,color);
              rc = diag_display(0x898040, catd, NULL, DIAG_IO,
                               ASL_DIAG_KEYS_ENTER_SC, &menutypes,f3_uinfo); 
	        check_rc(rc);

	        tu_test(rgb_tu_seq[i]);

		  /*  put up stand_by screen and wait 6 seconds */
		  /*  This really seems like 7 since there is a second sleep in
		   *  standbyscreen*/
	  	  stand_by_screen();
		  sleep(6);

		  /* run TU_CLOSE so that we can display a message */
    		  if (tu_exit_flg == 1)
		  {
			tu_exit();
    		  }

	        /* Put up screen asking if the screen was the right color */
              insert_color(&color_screen,uinfo,&menutypes,
						ASL_DIAG_LIST_CANCEL_EXIT_SC,color);

	        colornum += 1;
              rc = diag_display(colornum, catd, NULL, DIAG_IO,
                             ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes, uinfo);
	        check_rc(rc);
	        if (menutypes.cur_index != 1) {
	    	    /* report fru */
                    insert_frub(&tm_input, &frub[2]);
                    addfrub(&frub[2]);
        	    DA_SETRC_STATUS(DA_STATUS_BAD);
		    clean_up();
                };

        } /* end else */

	/* Ask user to run stand alone monitor test */
	insert_msg(&builtin,final,&menutypes,ASL_DIAG_LIST_CANCEL_EXIT_SC);
	menutypes.cur_index = 2;
	rc = diag_display(0x898123, catd, NULL, DIAG_IO,
		          ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes, final);
	check_rc(rc);
	if (rc == DIAG_ASL_COMMIT)
   		switch (DIAG_ITEM_SELECTED(menutypes)) {
	    	case 1 : /* report fru */
                    	insert_frub(&tm_input, &frub[3]);
                    	addfrub(&frub[3]);
        	    	DA_SETRC_STATUS(DA_STATUS_BAD);
		    	clean_up();
                	break;
       		case 2 :
       		default: clean_up();
   		}
    } /* end display attached */
    return(0);
} /* end interact_tu_seq */


/********************************************************************************
*
* NAME: tu_test
*                                                                    
* FUNCTION: Execute test units and report fru(s) to the controller if a
*           failure is found.
*                                                                    
* EXECUTION ENVIRONMENT:
*                                                                   
*	Called by the main program to execute test units.
*	Call external routine exectu to actually execute the test units.
*	Call external routine diag_asl_read to get user's input to screen.
*	Call check_rc to check if user has entered the Esc or Cancel key.
*	Call check_common routine to report fru when test unit fails
*                                                                   
* RETURNS: NONE
*
********************************************************************************/

tu_test(testptr)
    int             testptr;		/* determines which test unit to run */
{
    int         turc;			/* return code from test unit */

    if (tu_exit_flg == 0) {
        tucb_ptr.header.tu = TU_OPEN;
        turc = exectu(tm_input.dname, &tucb_ptr);
	  if (turc != 0) /* Check for common return code */
		check_common(turc);

        tu_exit_flg = 1;
#if DEBUG
    fprintf(wgabug,"after entering monitor mode \n");
    fflush(wgabug);
#endif

    }

    tucb_ptr.header.tu = testptr;
    turc = exectu(tm_input.dname, &tucb_ptr);

#if DEBUG
    fprintf(wgabug,"after executing tu number %d\n", testptr);
    fflush(wgabug);
#endif

    if (IS_CONSOLE) {
        rc = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, FALSE, NULL);
        check_rc(rc);
    }

    if (turc != 0) /* Check for common return code */
	check_common(turc);

} /* end tu_test */


/*******************************************************************************
*
* NAME: clean_up
*                                                                    
* FUNCTION: Closing file descriptors and return to diagnostic controller.
*                                                                    
* EXECUTION ENVIRONMENT:
*	Called by main program and tu_test.
*                                                                   
* RETURNS: NONE
*
********************************************************************************/

clean_up()
{
    if (tu_exit_flg == 1)
	 tu_exit();
#if DEBUG
    fprintf(wgabug,"after leaving tu_exit\n");
    fflush(wgabug);
#endif
    if ((IS_TM(console,CONSOLE_TRUE)) && diag_asl_flg )
        diag_asl_quit(NULL);    /* close ASL */
    if( catd != ((nl_catd) -1))
        catclose(catd);
    term_dgodm();       /* close ODM */
#if DEBUG
    fflush(wgabug);
    fclose(wgabug);
#endif
    DA_EXIT();
} /* end clean_up */



/*******************************************************************************
*
* NAME: stand_by_screen
*                                                                    
* FUNCTION: Displaying screen to user indicating test units are being executed.
*                                                                    
* EXECUTION ENVIRONMENT:
*	Called by main program.
*	Call check_rc.
*                                                                   
* RETURNS: NONE
*
********************************************************************************/

int 
stand_by_screen()
{
	/* flag means lets me know that this has already gone through this loop*/
	static flag=0;

    if (tm_input.console == CONSOLE_TRUE) {
        if (tm_input.advanced == ADVANCED_TRUE) {
	    if(flag == 0)
	    {
		flag = 1;
	    	da_title[0].msgid += 1;
	    	menunum += 1;
	     }
	}
	insert_msg(&da_title,m_uinfo,&menutypes,ASL_DIAG_OUTPUT_LEAVE_SC);
	rc = diag_display(menunum, catd, NULL, DIAG_IO,
			  ASL_DIAG_OUTPUT_LEAVE_SC, &menutypes, m_uinfo);
	sleep(1);
      check_rc(rc);
    }
} /* end stand_by_screen */


/*******************************************************************************
*
* NAME: loop_stand_by_screen
*                                                                    
* FUNCTION: Displaying screen to user indicating test units are being executed
*           in loop mode.
*                                                                    
* EXECUTION ENVIRONMENT:
*	Called by main program.
*	Call check_rc.
*                                                                   
* RETURNS: NONE
*
********************************************************************************/

int 
loop_stand_by_screen()
{
    if (tm_input.loopmode == LOOPMODE_ENTERLM)
	stand_by_screen();
    else if (IS_CONSOLE)
    {
	insert_msg(&l_da_title,l_uinfo,&menutypes,ASL_DIAG_OUTPUT_LEAVE_SC);
	rc = diag_display(l_menunum, catd, NULL, DIAG_IO,
			  ASL_DIAG_OUTPUT_LEAVE_SC, &menutypes, l_uinfo);
	sleep(2);
        check_rc(rc);
    }
} /* end loop_stand_by_screen */



/*******************************************************************************
*
* NAME: check_rc
*                                                                    
* FUNCTION: Checks if the user has entered the Esc or Cancel key while a screen
*           is displayed.
*                                                                    
* EXECUTION ENVIRONMENT:
*	Called by main program and some other routines.
*                                                                   
* RETURNS: rc, the input parameter
*
********************************************************************************/

int 
check_rc(rc)
    int             rc;				/* user's input */
{
    if (rc == DIAG_ASL_CANCEL) {
        DA_SETRC_USER(DA_USER_QUIT);
        clean_up();
    }
    if (rc == DIAG_ASL_EXIT) {
        DA_SETRC_USER(DA_USER_EXIT);
        clean_up();
    }
    return (rc);
} /* end check_rc */



/*******************************************************************************
*
* NAME: insert_msg
*                                                                    
* FUNCTION: initialize menutypes structure and call diag_display routine to
*           display menus
*                                                                    
* EXECUTION ENVIRONMENT:
*	Called by main program and some other routines.
*                                                                   
* RETURNS: NONE
*
********************************************************************************/

insert_msg(msg_list,uinfo_msg,menutypes,type)
struct  msglist msg_list[];
ASL_SCR_INFO    uinfo_msg[];
ASL_SCR_TYPE    *menutypes;
long    type;
{
	char    string[1024];
	menutypes->max_index = 0;
	menutypes->cur_index = 1;
	menutypes->cur_win_offset = 0;
	menutypes->cur_win_index = 0;
	menutypes->multi_select = 'y';
	menutypes->text_size = 0;
	menutypes->ask = '\0';

        diag_display(0x0,catd, msg_list,
	             DIAG_MSGONLY,type, menutypes,uinfo_msg);
        if (tm_input.loopmode != LOOPMODE_NOTLM)
	    sprintf(string,uinfo_msg[0].text,tm_input.dname,tm_input.dnameloc,
		      tm_input.lcount, tm_input.lerrors);
	 else
	    sprintf(string,uinfo_msg[0].text,tm_input.dname,tm_input.dnameloc);
		      uinfo_msg[0].text = (char *) malloc(strlen(string) +1);
	 strcpy(uinfo_msg[0].text, string);
}


/*******************************************************************************
 *
 * NAME: int_handler
 *
 * FUNCTION: Perform general clean up on receipt of an interrupt
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This is invoked when an interrupt occurs
 *
 * RETURNS: NONE
 *
 ********************************************************************************/

void
int_handler(int sig)
{
	invec.sa_handler =  int_handler;
        sigaction( sig, &invec, (struct sigaction *) NULL );

	if ( IS_TM( console, CONSOLE_TRUE ) )
	   diag_asl_clear_screen();
	clean_up();
}

/*******************************************************************************
 *
 * NAME: ret_handler
 *
 * FUNCTION: Put in delay before actual clean up
 *
 * EXECUTION ENVIRONMENT:
 *	     This is invoked when an interrupt occurs
 *
 * RETURNS: NONE
 *
 ********************************************************************************/

void
ret_handler(int sig)
{
        invec.sa_handler =  ret_handler;
	sigaction( SIGRETRACT, &invec, (struct sigaction *) NULL );

	sleep(1); /* wait for the tu to finish then clean_up */
	clean_up();
}


/*******************************************************************************
 *
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
 *
 ********************************************************************************/

int
chk_return(ret_code)
int	ret_code;
{
	if ( ret_code == -1 )
	{
		DA_SETRC_ERROR ( DA_ERROR_OTHER );
		clean_up();
	}
}


/*******************************************************************************
 *
 * NAME: tu_exit
 *
 * FUNCTION:  runs the TU_CLOSE TU
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 *
 *******************************************************************************/

int
tu_exit()
{
   int   turco;
   tucb_ptr.header.tu = TU_CLOSE;
   turco = exectu(tm_input.dname, &tucb_ptr);
   tu_exit_flg = 0;

}


/*******************************************************************************
 *
 * NAME: check_common
 *
 * FUNCTION: check for common error return code
 *
 *	Call external routines insert_frub and addfrub when a failure is found.
 *	Call clean_up after a fru is reported to the controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 *
 *******************************************************************************/

int
check_common(c_code)
int c_code;
{
	if(c_code == DEVICE_BUSY_ERROR)
      {
	   DA_SETRC_ERROR( DA_ERROR_OPEN );
	   clean_up();
    	}
		
    
    if (((c_code>=12)&&(c_code<=17)) ||
        ((c_code>=20)&&(c_code<=22)) ||
        ((c_code>=31)&&(c_code<=39)) ||
        ((c_code>=42)&&(c_code<=48)) ||
        ((c_code>=53)&&(c_code<=54)) ||
        ((c_code>=60)&&(c_code<=61)) ||
        ((c_code>=70)&&(c_code<=72)) ||
         (c_code==56)||(c_code==57))
    {
        insert_frub(&tm_input, &frub[0]);
        addfrub(&frub[0]);
       	DA_SETRC_STATUS(DA_STATUS_BAD);
	clean_up();
    }

    if (c_code == 30)
    {
        insert_frub(&tm_input, &frub[1]);
        addfrub(&frub[1]);
       	DA_SETRC_STATUS(DA_STATUS_BAD);
	clean_up();
    }

/* if return code does not match any of the above, return software error*/
   DA_SETRC_ERROR( DA_ERROR_OTHER );
   clean_up();

}
/*******************************************************************************
*
* NAME: insert_color
*                                                                    
* FUNCTION: initialize menutypes structure and call diag_display routine to
*           display menus
*                                                                    
* EXECUTION ENVIRONMENT:
*	Called by main program and some other routines.
*                                                                   
* RETURNS: NONE
*
********************************************************************************/

insert_color(msg_list,t_uinfo,menutypes,type,color)
struct  msglist msg_list[];
ASL_SCR_INFO    t_uinfo[];
ASL_SCR_TYPE    *menutypes;
long    type;
int color;
{
	char    string[1024];
	static int first = TRUE;

	diag_display(0x0,catd, msg_list, DIAG_MSGONLY,type, menutypes,t_uinfo);
	if(first == TRUE)
	{
		first = FALSE;
		sprintf(string,t_uinfo[1].text, diag_cat_gets(catd,DWGA_MSGS,color));
		t_uinfo[1].text = (char *) malloc(strlen(string) +1);
		strcpy(t_uinfo[1].text, string);

	}
	else
	{
		first = TRUE;
		menutypes->cur_index = 1;
		menutypes->multi_select = 'y';
		sprintf(string,t_uinfo[3].text, diag_cat_gets(catd,DWGA_MSGS,color));
		t_uinfo[3].text = (char *) malloc(strlen(string) +1);
		strcpy(t_uinfo[3].text, string); 

	}

	sprintf(string,t_uinfo[0].text,tm_input.dname,tm_input.dnameloc);
	t_uinfo[0].text = (char *) malloc(strlen(string) +1);
	strcpy(t_uinfo[0].text, string); 
}
