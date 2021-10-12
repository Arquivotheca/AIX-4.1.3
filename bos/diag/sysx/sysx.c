static char sccsid[] = "@(#)18	1.14  src/bos/diag/sysx/sysx.c, dsysx, bos41B, bai4 1/9/95 13:57:21";
/*
 * COMPONENT_NAME: DSYSX System Exerciser
 *
 * FUNCTIONS: 	main
 *		clr_class
 *		genexit
 *		int_handler
 *		disp_mgoal
 *		disp_exit_screen
 *		wraptitle
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 */                                                                   

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mdio.h>
#include <nl_types.h>
#include "diag/diag.h"
#include "diag/diago.h"
#include "diag/tmdefs.h"
#include "diag/class_def.h"
#include "sysx_msg.h"
#include "diag/diag_define.h"
#ifdef MSG
#include <locale.h>
#endif

#define   menu_end_key   (rc==DIAG_ASL_EXIT)
#define   menu_quit_key  (rc==DIAG_ASL_CANCEL)

/* GLOBAL VARIABLES	*/
int 	lmflg = LOOPMODE_NOTLM;         /* loop mode flag               */
int	loopmode=FALSE;			/* test to be performed in loop */
short 	lcount = 0;			/* loop count   		*/
short 	lerror = 0;			/* number of errors in loop     */
int systestflg = SYSTEM_FALSE;		/* not system check-out		*/
int advancedflg = ADVANCED_FALSE;
int diskette_based = DIAG_FALSE;	/* not executing off diskette	*/
int exenvflg;				/* execution mode               */
int consoleflg = CONSOLE_FALSE;		/* no display mode		*/
int diag_mode = DMODE_REPAIR;		/* diagnostic mode selection   	*/
int num_Top;				/* number of devices in Top array    */
int num_All;				/* number of devices in All array    */
diag_dev_info_t     *Top;		/* ptr to all device data structures */
extern diag_dev_info_ptr_t *All;	/* Array containing devices in All   */
nl_catd fdes;				/* catalog file descriptor	 */
char *dadir;				/* Path to DA Directory		 */
int media_flg;


/* LOCAL FUNCTION PROTOTYPES */
int clr_class(char *);
void genexit(int);
void int_handler(int); 
int disp_mgoal(void);
void disp_exit_screen(void);

/* EXTERNAL FUNCTION DECLARATIONS */
diag_dev_info_ptr_t *generate_All();
diag_dev_info_t     *init_diag_db();
char *diag_cat_gets(nl_catd, int, int);
nl_catd diag_catopen(char *, int);
void unload_diag_kext();

/*  */
/*
 * NAME: main
 *                                                                    
 * FUNCTION: main program for the system exerciser.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *      This program executes as an application of the kernel.  It
 *      executes under a process, and can page fault.
 *                                                                   
 * NOTES:
 *
 * RETURNS:
 *       0      No trouble found.
 *      -1      A problem was found.
 */

main(int argc, char *argv[])
{
	int rc; 			/* return code				*/
	struct sigaction act;		/* interrupt handler vector structure 	*/
	static struct msglist inv_menu[] = {
			{SET_INVALID, MSG_0},
			{SET_INVALID, MSG_1},
			{SET_INVALID, MSG_E},
			{(int )NULL, (int )NULL}
	};

	setlocale(LC_ALL, "");

/* Check if invoked with loopmode option 	*/
        while((rc = getopt(argc,argv,"lL:")) != EOF) {
                switch((char) rc) {
		case 'l':
		case 'L':
			loopmode=TRUE;
			break;
		default :
			break;
		}
	}
	/* set up interrupt handler	*/
	act.sa_handler = int_handler;
	sigaction(SIGINT, &act, (struct sigaction *)NULL);

	/* determine DA Directory Path from environment variable */
	if ((dadir = (char *)getenv("DADIR")) == NULL)
		dadir = DEFAULT_DADIR;

	/* open the message catalog file	*/
	fdes = diag_catopen(MF_SYSX, 0);

	/* initialize the ODM	*/
	init_dgodm();

	/* initialize the screen */
	diag_asl_init("default");

	/* determine execution mode - the system exerciser will only run
	   in standalone mode. It is not supported from diskette. */
	exenvflg = ipl_mode( &diskette_based );
	if ((exenvflg == EXENV_CONC &&  getenv("DIAG_SYSX") == NULL)
	  || diskette_based == DIAG_TRUE) {
		diag_display(0x803001, fdes, inv_menu, DIAG_IO,
			     ASL_DIAG_NO_KEYS_ENTER_SC, NULL, NULL);
		genexit(1);			
	}

	/* start out with the object classes cleaned out */
	if (clr_class("MenuGoal"))
		Perror(fdes, ESET, ERROR6, "MenuGoal");
	if (clr_class("DAVars"))
		Perror(fdes, ESET, ERROR6, "DAVars");
	if (clr_class("TMInput"))
		Perror(fdes, ESET, ERROR6, "TMInput");

	/* initialize the data structures for the test devices	*/
	if ((Top = init_diag_db(&num_Top)) == (diag_dev_info_t *) -1 )
		genexit(1);

	/* get list of all devices to be included in diag selection menu */
	All = generate_All(num_Top, Top, &num_All);

	media_flg = FALSE;
	rc = run_sysx(); 		/* run the exerciser */
	disp_mgoal();			/* Display any MenuGoals */
	if (media_flg) disp_exit_screen(); /* Remove all media */

	genexit(0);
}

/*   */
/* NAME: clr_class
 *
 * FUNCTION: This function clears Object Classes used by the DA's. 
 * 
 * RETURNS:
 *	0  - clear successful
 *	-1 - clear unsuccessful
 */

int clr_class(char *classname)
{
	int rc;

	if( !strcmp(classname,"FRUB") )
		rc = diag_rm_obj(FRUB_CLASS, "");
	else if( !strcmp(classname,"FRUs") )
		rc = diag_rm_obj(FRUs_CLASS, "");
	else if( !strcmp(classname,"MenuGoal") )
		rc = diag_rm_obj(MenuGoal_CLASS, "");
	else if( !strcmp(classname,"DAVars") )
		rc = diag_rm_obj(DAVars_CLASS, "");
	else if( !strcmp(classname,"TMInput") )
		rc = diag_rm_obj(TMInput_CLASS, "");
	else
		rc = -1;

	return( (rc >= 0) ? 0: -1 );

}
/*  */
/*
 * NAME:  genexit
 *
 * FUNCTION: general exit point for sysx.
 *
 * NOTES:
 *
 * RETURNS: None
 *
 */

void genexit(int exitcode)
{
	char cmd[256];

      /* Unload diagnostic kernel extension if needed */

        unload_diag_kext();
	sprintf(cmd,"%s 1>/dev/null 2>&1",SLIBCLEAN);
	(void) system(cmd);


	/* save all changes to the Customized Diag Object Class */
	if (exitcode == 0)
	  {
	  if (save_cdiag (Top, num_Top)) 
	    Perror(fdes, ESET, ERROR6, "CDiagDev");
	  }

	/* remove any devices that were defined just for diagnostics */
	remove_new_device();

	/* clean up asl & ODM */
	diag_asl_quit();
	term_dgodm();
	catclose(fdes);
	exit(exitcode);
}

/*  */
/****************************************************************
* NAME: int_handler 
*
* FUNCTION: In case of an interrupt, this routine is called.
*	    (NOTE:  It was unclear whether this routine would
*	    be called with 1 argument, as indicated by the
*	    sigaction structure, or 3 arguments, as indicated
*	    in the documentation of system call sigaction().)
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	genexit()
*
* RETURNS:
*	None
****************************************************************/
void int_handler(int sig)
{
	diag_asl_clear_screen();

	genexit(1);
}

/*  */
/*
 * NAME: disp_mgoal
 *
 * FUNCTION: Display a menu goal if one is present. 
 *
 * NOTES:
 *
 * RETURNS:
 * 	0 - if no menu goal was displayed 
 * 	1 - if menu goal was displayed 
 *     -1 - if error occurred          
 */

int disp_mgoal(void)
{
	int	cnt1, cnt2, rc;
	long	menu_number, *num_list;
	char	buffer[4096], *bufptr, *titleptr;
	char    title_b[1028];
	char    *line;
	char    *last_p; /* Where title ended */
	struct MenuGoal	*T_MenuGoal;
	struct listinfo f_info;
	ASL_SCR_INFO    *resinfo;
	static ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;


        /* allocate space for 3 entries */
        resinfo = (ASL_SCR_INFO *) calloc(3, sizeof(ASL_SCR_INFO));
        if (resinfo == (ASL_SCR_INFO *) NULL) return(-1);

        /* read all entries from MenuGoal Object Class */
	T_MenuGoal = (struct MenuGoal *)diag_get_list(MenuGoal_CLASS,"",
			&f_info,MAX_EXPECT,1);
	if ((T_MenuGoal != (struct MenuGoal *) -1) && (f_info.num > 0))
	  {
	  num_list = (long *)calloc(f_info.num, sizeof(long)); 
          for (cnt1=0; cnt1 < f_info.num; cnt1++)
	    {
	    restype.max_index = 2;
	    /* extract menu number from text */
	    sscanf( T_MenuGoal[cnt1].tbuffer1, "%X", &menu_number);

	    *(num_list + cnt1) = menu_number;
	    for (cnt2=0; cnt2 < cnt1; cnt2++)
	      if (menu_number == *(num_list + cnt2)) break;

	    if (cnt2 == cnt1)
	      {
	      /* next extract the title */
	      strtok(T_MenuGoal[cnt1].tbuffer1," ");
	      titleptr = T_MenuGoal[cnt1].tbuffer1 + 
			 strlen(T_MenuGoal[cnt1].tbuffer1) + 1;
	      line = (char *) strtok(titleptr, "\n");
	      last_p = line;
	      wraptitle(0, title_b, line);

	      resinfo[0].text = title_b;

	      /* everything else is message text */
	      bufptr = titleptr + strlen(resinfo[0].text) + 1;
	      strcpy(buffer, bufptr);
	      strcat(buffer, T_MenuGoal[cnt1].tbuffer2);
	      resinfo[1].text = buffer;
	      resinfo[2].text = "";
	      rc = diag_display(menu_number, fdes, NULL, DIAG_IO,
				ASL_DIAG_ENTER_SC,
				&restype, resinfo);
	      }
	    }
	  }

        free( resinfo );
	clr_class("MenuGoal");
	return((f_info.num == 0) ? 0 : 1);
}

/*  */
/* NAME: disp_exit_screen
 *
 * FUNCTION: Instruct user to remove media 
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 *
 */
void disp_exit_screen(void)
{
  int selection;
  int rc;
  ASL_SCR_INFO *resinfo;
  static ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;


        resinfo = (ASL_SCR_INFO *) calloc(3, sizeof(ASL_SCR_INFO));
        if (resinfo == (ASL_SCR_INFO *) -1) return;

        /* set up the menu */
	resinfo[0].text = diag_cat_gets(fdes, EXIT_SYSX, EXIT_TITLE);
	resinfo[1].text = diag_cat_gets(fdes, EXIT_SYSX, EXIT_INSTRUCT);
	resinfo[2].text = diag_cat_gets(fdes, EXIT_SYSX, EXIT_LAST);

        restype.max_index = 2;

        /* now display screen */
        rc = diag_display(0x803008, fdes, NULL, DIAG_IO,
			  ASL_DIAG_ENTER_SC, &restype, resinfo);

        free(resinfo);
	return;
}

/*   */
/*
 * NAME: wraptitle
 *
 * FUNCTION: Adjust wraparound of text on screen
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0
 */

/* Title length is 80 chars less 6 digits menu numbers and 1 space */

#define	TITLE_LENGTH	73

wraptitle( string_length, buffer, text )
int     string_length;  /* current length of text in buffer     */
char    *buffer;        /* buffer to append text to             */
char    *text;          /* text to be appended                  */
{
        int     i;
        int     char_positions;

        /* determine if length of text string will fit on one line */
        char_positions = TITLE_LENGTH - string_length;
      	if ( char_positions < strlen(text))  {

        /* dont break the line in the middle of a word */
            	 if( text[char_positions] != ' ')
                      	 while ( --char_positions )
                              	 if( text[char_positions] == ' ')
                                      	 break;
             	 if ( char_positions == 0 )
                       	char_positions = TITLE_LENGTH - string_length;

                 for (i = 0; i < char_positions; i++, buffer++, text++ )
                       	*buffer = ( *text == '\n' ) ? ' ' : *text ;
	         *buffer++ = '\n';
                 while ( *text == ' ' ) /* remove any leading blanks */
                       	text++;
	         wraptitle( string_length, buffer, text);
        }
        else
              	sprintf(buffer, "%s", text);

        return(0);
}
