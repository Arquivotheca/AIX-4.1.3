
static char sccsid[] = "@(#) 92 1.2 src/bos/usr/lpp/bosinst/BosMenus/Help.c, bosinst, bos411, 9428A410j 94/05/23 13:31:45";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: DisplayHelp
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
 * NAME: DisplayHelp
 *
 * FUNCTION: display help screen
 *
 * EXECUTION ENVIRONMENT:
 *      can run anywhere.
 *
 * NOTES: A menu structure is filled in with the translated text passed
 *	in.  This menu is then displayed with DisplayMenu.
 *
 * RETURNS: None.
 */

#include <string.h>
#include <locale.h>
#include "BosMenus_msg.h"
#include "Menus.h"

extern nl_catd catfd;		/* catalog descriptor			*/

/* function prototypes */
struct Menu *HelpDrvr(struct Menu *, int *);
void parse(struct Menu *, int, char *, int);

int DisplayHelp(
char *str,			/* help string				*/
int num)			/* catalog message numnber		*/
{
    char *ptr;			/* catgets ptr                          */
    char *msgptr;		/* char    ptr                          */
    
    int wasitme = 0;		/* flag indicating no string found	*/
    struct Menu help;		/* The menu         			*/

    /* load the menu structure */
    bzero(&help, sizeof(struct Menu));

    help.Length = 24;
    help.driver = HelpDrvr;
    help.DefaultLine = 21;

    ptr = catgets(catfd, BH, num, str);
    msgptr = malloc(strlen(ptr)+1);
    if (!msgptr)
	    exit(2);

    strcpy(msgptr, ptr);
    
    parse(&help, 0, msgptr, 22);

    help.Text[21] = catgets(catfd, BI, BS_PREV_MENU, "    99  Previous Menu");
    help.Text[23] = catgets(catfd, BI, BS_CHOICE, ">>> Choice ");

    (void) DisplayMenu(&help);

    free(msgptr);

}
/*
 * NAME: HelpDrvr
 *
 * FUNCTION: Process user input from help screen
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Part of the bosinstall menu system, called by DisplayMenu
 *
 * NOTES: Any user input exits the screen
 *
 * RETURNS: 0.
 */

struct Menu *HelpDrvr(
struct Menu *menu,
int *input)
{
    return 0;
}
