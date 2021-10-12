static char sccsid[] = "@(#) 95 1.4 src/bos/usr/lpp/bosinst/BosMenus/MenuHand.c, bosinst, bos411, 9428A410j 94/01/14 11:00:28";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: DisplayMenu, ClearScreen, bibeep
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME: DisplayMenu
 *
 * FUNCTION: Displays a menu and gets user input
 *
 * EXECUTION ENVIRONMENT:
 *
 *      When running on an LFT, a clear screen is issued prior to
 *	displaying the menu.
 *
 * NOTES: 
 *      The display is assumed at least 24 lines long.  Tubes which support
 *	more than 24 lines will not line up perfectly.  LFT terminals will
 *	have the screen cleared before the menu is displayed.
 *
 *	If the '>>>' prompt is not desired, set the defaultLine to -1.
 *
 *	If a preformat is supplied in the menu structure, it is called.  This
 *	feature is usefull for translating text, paging, or changing dynamic
 *	strings.
 *
 *	If the animate flag is set, the animate process is started and the
 *	keyboard is not read.  Control is returned imediately to the caller.
 *	The animation process is killed by calling StopAnim().
 *
 *	If a driver is supplied in the menu structure, DisplayMenu will read
 *	the keyboard and pass input to the driver.
 *
 * RETURNS:  a pointer to the next menu, as determined by the driver.
 *	If there is no driver, 0 is returned. 
 */

#include <string.h>
#include <stdio.h>
#include <signal.h>
#include "Menus.h"

int anim_pid;		/* global so that it may be killed later */

char *clear;                    /* clear sequence obtained from */
								/* the CLEAR environment        */
								/* variable.                    */

struct Menu  *DisplayMenu( struct Menu *menu_in)
{
    static int firsttime = 1;           /* first time this code entered */
    int defaultvalue;			/* default value                */
    int stop; 				/* line stop point              */
    char Messages[4][89];		/* Message Box text             */
    char MsgText[180];			/* copy of MessageBox text      */
    char *ptrs[3];			/* Message Box text parsing ptr */
    int i;				/* index to menu text           */
    struct Menu *menu;			/* copy of menu ptr (debug tool)*/
    static int input;

    menu = menu_in;	/* necessary for dbx! */

    /* If first time, get the screen clear sequence from the environment  */
    /* variable "$CLEAR". */
    if ( firsttime)
    {
	firsttime = 0;
	clear = getenv("CLEAR");
    }

    /* Call menu preformat routine */
    if (menu->preformat)
    {
	if (menu->preformat(menu) == -1)
	    if (menu->driver)
	    {
		input = -1;
		return (menu->driver(menu, &input));
		
	    }
	    else
		return (struct Menu *)0;

    }

    printf("%s", clear);
    fflush(stdout);

    /* if there is a message, display it */
    if (menu->Message)
    {
	/* translate the message */


	/* copy the msaage text */
	strcpy(MsgText, menu->Message->Text);

	sprintf(Messages[0], 
	"%-23.23s+-----------------------------------------------------",
	menu->Text[19]);

	/* Scan the MessageBox text, replacing each newlinw wich a null.
	 * Save the location of the begining of each resulting new string
	 */
	ptrs[1] = strchr(MsgText, '\n');
	ptrs[2] = '\0';
	if (ptrs[1])
	{
	    *ptrs[1]++ = '\0';
	    ptrs[2] = strchr(ptrs[1], '\n');
	    if (ptrs[2])
	    {
		*ptrs[2]++ = '\0';
		ptrs[0] = strchr(ptrs[2], '\n');
		if (ptrs[0])
		    *ptrs[0] = '\0';
	    }
	    else
		ptrs[2] = " ";
	}
	else
	    ptrs[1] = " ";
	ptrs[0] = MsgText;

	for (i=0; i < 3; i++)
	{
	    sprintf(Messages[i+1], "%-23.23s|%s", menu->Text[i+20], ptrs[i]);
	}
	

    }
    /* loop through the structure and display the strings in the menu */
    if (menu->Message)
    {
	if(menu->Length == 24)
	    stop = 19;
	else
	    stop = menu->Length-1;
    }
    else
	stop = menu->Length-1;

    for (i=0, defaultvalue=-1; i<stop; i++)
    {
	/* if this line is supposed to have the ">>>" default prompt,
	 * print the prompt
	 */
	if (i == menu->DefaultLine)
	{
	    defaultvalue = atoi(menu->Text[i]+3);
	    printf(">>>%s\n", menu->Text[i]+3);
	}
	else
	    printf("%s\n", menu->Text[i]);
    
    }
    /* See if we need to print the MessageBox lines */
    if (menu->Message)
	for (; i < menu->Length-1; i++)
	    printf("%s\n", Messages[i-stop]);

    /* print last line without a newline */
    if (i == menu->DefaultLine)
	printf(">>>%s", menu->Text[i]+3);
    else
	printf("%s", menu->Text[i]);
    
    /* print default value if there is one */
	printf((defaultvalue != -1)? "[%d]: " : "[]: ", defaultvalue);
    fflush(stdout);


    /* Check the animation flag.  If set, fork and exec the animate program
     */ 
    if (menu->Animate)
    {
	/*
	signal(SIGCHLD, SIG_IGN);
	*/
	if (( anim_pid = fork()) == 0)
	{
	    execl("/usr/lpp/bosinst/Animate", "Animate", menu->AnimationString, 0);
	    exit(1);
	}
	
	/* Don't want to read the keyboard, so return now */
	return (struct Menu *)0;
    }
    else
    {
	char buf[80];
	char other;
	int *input;

	/* Read the keyboard and pass the input to the driver. 
	 * The driver will determine the next menu to call
	 */
	if (menu->MultipleSelect)
	    other = ' ';
	else
	    other = '\0';
	input = BIreadtty( other, buf, 80);

	/* Check for default key: if single select and input is -1,
	 * set input to default line
	 */
	if (*input == -1)
	{
	    if ((menu->DefaultLine >= 0) && (menu->DefaultLine < 24))
		 *input = atoi(menu->Text[menu->DefaultLine]+4);
	    if (menu->MultipleSelect)
	    {
		*(input+1) = -1;
	    }
	}
	fflush(stdin);
	if (menu->driver)
	{
	    return (menu->driver(menu, input));
	    
	}
	else
	    return (struct Menu *)0;

    }


}

/*
 * NAME: StopAnim
 *
 * FUNCTION: kills the Animate process started by DisplayMenu
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Only kills the ANimate process started by DisplayMenu.
 *
 * NOTES: Kills the pid pointed to by anim_pid, set in DisplayMenu.
 *	The pid is set to 0.
 *
 * RETURNS: NONE
 */
StopAnim()
{
    int pid, stat;

    /* only do this if it's a valid pid */
    if (anim_pid > 0)
    {
	kill(anim_pid, 2);
	pid = wait(&stat);
 	anim_pid = 0;
    }
}
/*
 * NAME: bibeep
 *
 * FUNCTION: echos a beep 
 *
 * EXECUTION ENVIRONMENT:
 *
 *      can be called by anybody
 *
 * RETURNS: NONE
 */
bibeep()
{
    printf("");
    fflush (stdout);
}

