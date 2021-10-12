static char sccsid[] = "@(#)28  1.8  src/bos/usr/bin/help/help.c, cmdman, bos41B, 9504A 12/23/94 12:47:12";
/*
 * COMPONENT_NAME: (CMDMAN) commands that allow users to read online
 * documentation 
 *
 * FUNCTIONS: none 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * help -- provides information for the new user. 
 *
 * format -- help 
 * 	It presents a one-page display of information for the new user.
 */

#include <stdio.h>
#include <locale.h>
#include "help_msg.h" 
static nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_HELP,n,s) 
#define msg_HELP \
"\n\
Look in a printed manual for general help if you can.  You should \n\
have someone show you some things and then read \"Using and Managing \n\
AIX\" manual. \n\
\n\
The commands: \n\
    man -k keyword	lists commands relevant to a keyword \n\
    man command		prints out the manual pages for a command \n\
are helpful; other basic commands are: \n\
    cat			- concatenates files (and just prints them out) \n\
    ex			- text editor \n\
    ls			- lists contents of directory \n\
    mail		- sends and receives mail \n\
    msgs		- system messages and junk mail \n\
    passwd		- changes login password \n\
    sccshelp		- views information on the Source Code Control System \n\
    smit		- system management interface tool \n\
    tset		- sets terminal modes \n\
    who			- who is on the system \n\
    write		- writes to another user \n\
You could find programs about mail by the command: 	man -k mail \n\
and print out the man command documentation via:	man mail \n\
You can log out by typing \"exit\". \n\n"

/*
 *  main
 */
main()
{
	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_HELP, NL_CAT_LOCALE);

	printf(MSGSTR(HELP, msg_HELP)); 
	exit(0);
}
