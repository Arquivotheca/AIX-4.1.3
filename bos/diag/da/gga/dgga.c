static char sccsid[] = "@(#)99	1.1  src/bos/diag/da/gga/dgga.c, dagga, bos41J, 9515A_all 4/6/95 09:46:43";
/*
 * COMPONENT_NAME: DAGGA  -  diag application - POWER WAVE (S15) GRAPHICS ADAPTER
 *
 * FUNCTIONS: tu_test, clean_up, stand_by_screen, loop_stand_by_screen, check_rc
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <stdio.h>
#include <errno.h>
#include <locale.h>
#include <cf.h>
#include <fcntl.h>
#include "dgga_msg.h"
#include "exectu.h"
/*#include "tu_type.h"*/

#include <sys/cfgodm.h>
#include <sys/signal.h>
#include <sys/errids.h>

#include "diag/da.h"
#include "diag/diago.h"
#include "diag/diag.h"
#include "diag/tm_input.h"
#include "diag/tmdefs.h"
#include "diag/diag_exit.h"


/**************************************************************************/
/* IS_TM is a macro that accepts two input variables VAR1 & VAR2.         */
/* VAR1 is an object of the structure tm_input.                           */
/* VAR2 is a variable or a defined value that will be compared to the     */
/*      tm_input object class variable.                                   */
/* NOTE: macro returns logical TRUE or FALSE                              */
/**************************************************************************/

#define IS_TM( VAR1, VAR2 ) ( (int) ( tm_input./**/VAR1 ) == /**/VAR2 )

/**************************************************************************/
/* INTERACTIVE is a macro that returns TRUE if conditions exist that      */
/*              allow the running of extended tests.  Extended tests are  */
/*              those tests which will run for several minutes.           */
/* NOTE: macro returns logical TRUE or FALSE                              */
/**************************************************************************/

#define INTERACTIVE           	(                                          \
			      	      !(IS_TM( loopmode,LOOPMODE_INLM ))   \
			        &&      IS_TM( system,SYSTEM_FALSE )       \
			        &&      IS_TM( console,CONSOLE_TRUE )      \
			        )

/**************************************************************************/
/* ADVANCED is a macro that returns TRUE if the customer selected         */
/*              advanced mode of operation.                               */
/* NOTE: macro returns logical TRUE or FALSE                              */
/**************************************************************************/

#define ADVANCED                (                                          \
				        IS_TM( advanced,ADVANCED_TRUE )    \
        			)


int gga_tu_seq[7] =
{
	RED_TU,GREEN_TU,BLUE_TU,VRAM_TU,PALETTECHECK_TU,CURSOR_RAM_TU,COLOR_BAR_TU
};

int rgb_tu_seq[3] =
{
	RED_TU, GREEN_TU, BLUE_TU
};

int pretest_tu_seq[6] =
{
	RED_TU,GREEN_TU,BLUE_TU,COLOR_BAR_TU,PALETTECHECK_TU,CURSOR_RAM_TU
};



/* fru_bucket is a structure that holds information for the diagnostic 
   program to return to the diagnostic controller when a failure is found
   that needs to be reported. (FRU means Field Replacable Unit) */

struct fru_bucket frub[]=
{
	{"", FRUB1, 0x82C, 0x101, R_BASE,  /* R_BASE*/
		{
		    {90, "", "", 0, DA_NAME, NONEXEMPT},
		    {10, "", "", 0, PARENT_NAME, NONEXEMPT},
		},
	},
	
	{"", FRUB1, 0x82C, 0x102, R_BASE,  /* R_BASE*/
		{
		    {100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},


	{"", FRUB1, 0x82C, 0x103, R_DISP,
		{
		{100, "Display", "", 0, PARENT_NAME, NONEXEMPT},
		},
	},


	{"", FRUB1, 0x82C, 0x104, R_MON,
		{
		{65,"", "", 0, DA_NAME, NONEXEMPT},
		{35, "Display", "", F_MON, NOT_IN_DB, NONEXEMPT},
		},
	},
};

struct msglist da_title[] =
{
	{DGGA_MSGS, DGGA},
	{DGGA_MSGS, DGGA_STANDBY},
	(int)NULL
};

struct msglist l_da_title[] =
{
	{DGGA_MSGS, DGGA_L},
	/*{DGGA_MSGS, DGGA_L_STANDBY},*/
	(int)NULL
};

struct msglist havedisp[] =
{
	{DGGA_MSGS, DGGA},
	{DGGA_MSGS, DGGA_EXT_YES},
	{DGGA_MSGS, DGGA_EXT_NO},
	{DGGA_MSGS, DGGA_HAVEDISP},
	(int)NULL
};

struct msglist f3_exit[] =
{
	{DGGA_MSGS, DGGA},
	{DGGA_MSGS, F3_EXIT},
	(int)NULL
};

struct msglist redscreen[] =
{
	{DGGA_MSGS, DGGA},
	{DGGA_MSGS, DGGA_EXT_YES},
	{DGGA_MSGS, DGGA_EXT_NO},
	{DGGA_MSGS, DGGA_REDCUR},
	(int)NULL
};

struct msglist greenscreen[] =
{
	{DGGA_MSGS, DGGA},
	{DGGA_MSGS, DGGA_EXT_YES},
	{DGGA_MSGS, DGGA_EXT_NO},
	{DGGA_MSGS, DGGA_GREENCUR},
	(int)NULL
};

struct msglist bluescreen[] =
{
	{DGGA_MSGS, DGGA},
	{DGGA_MSGS, DGGA_EXT_YES},
	{DGGA_MSGS, DGGA_EXT_NO},
	{DGGA_MSGS, DGGA_BLUECUR},
	(int)NULL
};

struct msglist selfred[] =
{
	{DGGA_MSGS, DGGA},
	{DGGA_MSGS, DGGA_EXT_YES},
	{DGGA_MSGS, DGGA_EXT_NO},
	{DGGA_MSGS, DGGA_REDCUR},
	(int)NULL
};

struct msglist selfgreen[] =
{
	{DGGA_MSGS, DGGA},
	{DGGA_MSGS, DGGA_EXT_YES},
	{DGGA_MSGS, DGGA_EXT_NO},
	{DGGA_MSGS, DGGA_GREENCUR},
	(int)NULL
};

struct msglist selfblue[] =
{
	{DGGA_MSGS, DGGA},
	{DGGA_MSGS, DGGA_EXT_YES},
	{DGGA_MSGS, DGGA_EXT_NO},
	{DGGA_MSGS, DGGA_BLUECUR},
	(int)NULL
};

struct msglist color_screen[] =
{
	{DGGA_MSGS, DGGA},
	{DGGA_MSGS, DGGA_EXT_YES},
	{DGGA_MSGS, DGGA_EXT_NO},
	{DGGA_MSGS, COLOR_SCREEN},
	(int)NULL
};

struct msglist builtin[] =
{
	{DGGA_MSGS, DGGA},
	{DGGA_MSGS, DGGA_EXT_YES},
	{DGGA_MSGS, DGGA_EXT_NO},
	{DGGA_MSGS, DGGA_PDPQ},
	(int)NULL
};

struct msglist procedure[] =
{
	{DGGA_MSGS, DGGA},
	{DGGA_MSGS, DGGA_RP},
	(int)NULL
};


static ASL_SCR_TYPE menutypes = DM_TYPE_DEFAULTS;
static ASL_SCR_INFO uinfo[DIAG_NUM_ENTRIES(havedisp)];
static ASL_SCR_INFO m_uinfo[DIAG_NUM_ENTRIES(da_title)];
static ASL_SCR_INFO selfuinfo[DIAG_NUM_ENTRIES(selfred)];
static ASL_SCR_INFO final[DIAG_NUM_ENTRIES(builtin)];
static ASL_SCR_INFO procs[DIAG_NUM_ENTRIES(procedure)];
static ASL_SCR_INFO f3_uinfo[DIAG_NUM_ENTRIES(f3_exit)];


#define IS_CONSOLE  ( (int) (tm_input.console == CONSOLE_TRUE) )
#define MAXCOLORS 512
#define CHAR_TO_INT(x) x[0]<<24 | x[1]<<16 | x[2]<<8 | x[3]

TUCB          tucb_ptr;    	/* pointer to the previous structure */
struct tm_input tm_input;    	/* info. from dc to this program     */
struct listinfo	c_info;
int             rc;        	/* to hold functions return codes    */
extern  nl_catd diag_catopen(char *, int);
nl_catd         catd;     	/* file descriptor for catalog file  */
void		int_handler(int);
int             i;		/* loop counter */
int		diskette_based = DIAG_FALSE;
int		menunum = 0x82C000;
int		l_menunum = 0x82C010;
int		colornum = 0x82C020;
int		colorselfnum = 0x82C030;
int		tu_exit_flg = 0;
int		disp_attached = 0;
int		diag_asl_flg=FALSE;
char           *crit;           /* buffer */
char            DEVICE[50];     /* device name */
char	       *dname;

extern		exectu();
extern          getdainput();
extern          addfrub();
struct  sigaction       invec;  /* interrupt handler structure     */
void	ret_handler(int);
/*#define DEBUG 1*/
#if 	DEBUG
FILE	*ggabug;
#endif

void
main()
{
    
    setlocale(LC_ALL,"");
    crit = (char *) malloc(256);
    dname = (char *) malloc(256);

    (void)memset (&tucb_ptr, 0, sizeof(TUCB));
    /* initialize interrupt handler */

    invec.sa_handler =  int_handler;
    sigaction( SIGINT, &invec, (struct sigaction *) NULL );
    sigaction( SIGTERM, &invec, (struct sigaction *) NULL );


#if	DEBUG
	ggabug=fopen("/tmp/gga.debug","a+");
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

    strcpy(dname, tm_input.dname);

    if (tm_input.console == CONSOLE_TRUE) 
    {
        diag_asl_init(ASL_INIT_DEFAULT);
	diag_asl_flg=TRUE;
        catd = diag_catopen(MF_DGGA,0);
    }


    if (tm_input.loopmode == LOOPMODE_NOTLM)
    {
        stand_by_screen();
    }
    else
        loop_stand_by_screen();



    /* TU initialization */
    tucb_ptr.header.loop = 1;

    /* Check for pre-test */
    if (IS_TM (exenv,EXENV_IPL)) {
	for (i=0; i<6; ++i)
	    tu_test(pretest_tu_seq[i]);
	clean_up();
    }

    invec.sa_handler =  ret_handler;
    sigaction( SIGRETRACT, &invec, (struct sigaction *) NULL );

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

/****************
*
*  sys_tu_seq
*
*****************/

int
sys_tu_seq()
{
	for (i=0; i<7; ++i)
	    tu_test(gga_tu_seq[i]);
}
/* end sys_tu_seq */
        
/*****************
*
*  interact_tu_seq
*
******************/

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
              rc = diag_display(0x82C040, catd, NULL, DIAG_IO,
                               ASL_DIAG_KEYS_ENTER_SC, &menutypes,f3_uinfo); 
	        check_rc(rc);

	        tu_test(rgb_tu_seq[i]);

		  /*  wait 6 seconds */
		  sleep(7);

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
                    insert_frub(&tm_input, &frub[3]);
                    addfrub(&frub[3]);
        	    DA_SETRC_STATUS(DA_STATUS_BAD);
		    clean_up();
                };

        } 
	/* Ask user to run stand alone monitor test */
	insert_msg(&builtin,final,&menutypes,ASL_DIAG_LIST_CANCEL_EXIT_SC);
	menutypes.cur_index = 1;
	rc = diag_display(0x82C123, catd, NULL, DIAG_IO,
		          ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes, final);
	check_rc(rc);
	if (rc == DIAG_ASL_COMMIT)
   		switch (DIAG_ITEM_SELECTED(menutypes)) {
       		case 1 : insert_msg(&procedure,procs,&menutypes,
				    ASL_DIAG_KEYS_ENTER_SC);
			 rc = diag_display(0x82C234, catd, NULL, DIAG_IO,
		              ASL_DIAG_KEYS_ENTER_SC, &menutypes, procs);
			 check_rc(rc);
                	 break;
       		case 2 :
       		default: clean_up();
   		}
    } /* end display attached */
    return(0);
}



/*
* NAME: tu_test
*                                                                    
* FUNCTION: Executing test units and report fru(s) to the controller if a
*           failure is found.
*                                                                    
* EXECUTION ENVIRONMENT:
*                                                                   
*	Called by the main program to execute test units.
*	Call external routine exectu to actually execute the test units.
*	Call external routine diag_asl_read to get user's input to screen.
*	Call check_rc to check if user has entered the Esc or Cancel key.
*	Call external routines insert_frub and addfrub when a failure is found.
*	Call clean_up after a fru is reported to the controller.
*                                                                   
* RETURNS: NONE
*/

tu_test(testptr)
    int             testptr;		/* determines which test unit to run */
{
    int         turc;			/* return code from test unit */
    int		how_many;
    struct	CuDv *cudva;
    struct	CuAt *cuat_rrate;
    char	buff[256];


    if (tu_exit_flg == 0) {
        tucb_ptr.header.tu = TU_OPEN; 
        turc = exectu(tm_input.dname, &tucb_ptr);
#if DEBUG
    fprintf(ggabug,"after executing TU OPEN \n");
    fflush(ggabug);
    fprintf(ggabug,"turc = %x \n", turc);
    fflush(ggabug);
#endif
        tu_exit_flg = 1;
#if DEBUG
    fprintf(ggabug,"after entering monitor mode \n");
    fflush(ggabug);
#endif

    }

    tucb_ptr.header.tu = testptr;
    turc = exectu(tm_input.dname, &tucb_ptr);

#if DEBUG
    fprintf(ggabug,"after executing tu number %d\n", testptr);
    fflush(ggabug);
    fprintf(ggabug,"turc = %x \n", turc);
    fflush(ggabug);
#endif


    if (IS_CONSOLE) {
        rc = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, FALSE, NULL);
        check_rc(rc);
    }

    if (turc != 0) 
	check_common(turc);

} /* end tu_test */



/*
* NAME: clean_up
*                                                                    
* FUNCTION: Closing file descriptors and return to diagnostic controller.
*                                                                    
* EXECUTION ENVIRONMENT:
*	Called by main program and tu_test.
*                                                                   
* RETURNS: NONE
*/

clean_up()
{
    if (tu_exit_flg == 1)
	 tu_exit();
#if DEBUG
    fprintf(ggabug,"after leaving leavingmom\n");
    fflush(ggabug);
#endif
    free(crit);
#if DEBUG
    fprintf(ggabug,"after closing filedes\n");
    fflush(ggabug);
#endif
    free(dname);
    if (tm_input.console == CONSOLE_TRUE && diag_asl_flg)
        diag_asl_quit(NULL);    /* close ASL */
    if( catd != ((nl_catd) -1))
        catclose(catd);
    term_dgodm();       /* close ODM */
#if DEBUG
    fflush(ggabug);
    fclose(ggabug);
#endif
    DA_EXIT();
} /* end clean_up */




/*
* NAME: stand_by_screen
*                                                                    
* FUNCTION: Displaying screen to user indicating test units are being executed.
*                                                                    
* EXECUTION ENVIRONMENT:
*	Called by main program.
*	Call check_rc.
*                                                                   
* RETURNS: NONE
*/

int 
stand_by_screen()
{
    if (tm_input.console == CONSOLE_TRUE) {
        if (tm_input.advanced == ADVANCED_TRUE) {
	    da_title[0].msgid += 1;
	    menunum += 1;
	}
	insert_msg(&da_title,m_uinfo,&menutypes,ASL_DIAG_OUTPUT_LEAVE_SC);
	rc = diag_display(menunum, catd, NULL, DIAG_IO,
			  ASL_DIAG_OUTPUT_LEAVE_SC, &menutypes, m_uinfo);
	sleep(2);
        check_rc(rc);
    }
} /* end stand_by_screen */



/*
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
*/

int 
loop_stand_by_screen()
{
    if (tm_input.loopmode == LOOPMODE_ENTERLM)
	stand_by_screen();
    else if (IS_CONSOLE)
    {
	insert_msg(&l_da_title,m_uinfo,&menutypes,ASL_DIAG_OUTPUT_LEAVE_SC);
	rc = diag_display(menunum, catd, NULL, DIAG_IO,
			  ASL_DIAG_OUTPUT_LEAVE_SC, &menutypes, m_uinfo);
	sleep(2);
        check_rc(rc);
    }
} /* end loop_stand_by_screen */




/*
* NAME: check_rc
*                                                                    
* FUNCTION: Checks if the user has entered the Esc or Cancel key while a screen
*           is displayed.
*                                                                    
* EXECUTION ENVIRONMENT:
*	Called by main program and some other routines.
*                                                                   
* RETURNS: rc, the input parameter
*/

int 
check_rc(rc)
    int             rc;				/* user's input */
{
    if (rc == DIAG_ASL_CANCEL) {
        tm_input.loopmode = LOOPMODE_EXITLM;	/* force microcode swap */
        DA_SETRC_USER(DA_USER_QUIT);
        clean_up();
    }
    if (rc == DIAG_ASL_EXIT) {
        tm_input.loopmode = LOOPMODE_EXITLM;	/* force microcode swap */
        DA_SETRC_USER(DA_USER_EXIT);
        clean_up();
    }
    return (rc);
} /* end check_rc */


insert_msg(msg_list,uinfo,menutypes,type)
struct  msglist msg_list[];
ASL_SCR_INFO    uinfo[];
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
	             DIAG_MSGONLY,type, menutypes,uinfo);
        if (tm_input.loopmode != LOOPMODE_NOTLM)
	    sprintf(string,uinfo[0].text,tm_input.dname,tm_input.dnameloc,
		      tm_input.lcount, tm_input.lerrors);
	 else
	    sprintf(string,uinfo[0].text,tm_input.dname,tm_input.dnameloc);
		      uinfo[0].text = (char *) malloc(strlen(string) +1);
	 strcpy(uinfo[0].text, string);
}


/*
 * NAME: int_handler
 *
 * FUNCTION: Perform general clean up on receipt of an interrupt
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This should describe the execution environment for this
 *      procedure. For example, does it execute under a process,
 *      interrupt handler, or both. Can it page fault. How is
 *      it serialized.
 *
 * RETURNS: NONE
 */

void
int_handler(int sig)
{
	invec.sa_handler =  int_handler;
        sigaction( sig, &invec, (struct sigaction *) NULL );

	if ( IS_TM( console, CONSOLE_TRUE ) )
	   diag_asl_clear_screen();
	clean_up();
}


void
ret_handler(int sig)
{
	sleep(1); /* wait for the tu to finish then clean_up */
	clean_up();
}






/*
 * NAME: tu_exit
 *
 * FUNCTION: leave monitor mode
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

int
tu_exit()
{
int   turco;
   tucb_ptr.header.tu = TU_CLOSE;
   turco = exectu(tm_input.dname, &tucb_ptr);
   tu_exit_flg = 0;
#if DEBUG
    fprintf(ggabug,"after executing TU CLOSE \n");
    fflush(ggabug);
    fprintf(ggabug,"turc = %x \n", turco);
    fflush(ggabug);
#endif
}



/*
 * NAME: check_common
 *
 * FUNCTION: check for common error return code
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

int
check_common(c_code)
int c_code;
{
        insert_frub(&tm_input, &frub[1]);
        addfrub(&frub[1]);
       	DA_SETRC_STATUS(DA_STATUS_BAD);
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
		sprintf(string,t_uinfo[1].text, diag_cat_gets(catd,DGGA_MSGS,color));
		t_uinfo[1].text = (char *) malloc(strlen(string) +1);
		strcpy(t_uinfo[1].text, string);

	}
	else
	{
		first = TRUE;
		menutypes->cur_index = 1;
		menutypes->multi_select = 'y';
		sprintf(string,t_uinfo[3].text, diag_cat_gets(catd,DGGA_MSGS,color));
		t_uinfo[3].text = (char *) malloc(strlen(string) +1);
		strcpy(t_uinfo[3].text, string); 

	}

	sprintf(string,t_uinfo[0].text,tm_input.dname,tm_input.dnameloc);
	t_uinfo[0].text = (char *) malloc(strlen(string) +1);
	strcpy(t_uinfo[0].text, string); 
}
