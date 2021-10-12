static char sccsid[] = "@(#)73	1.5  src/bos/usr/bin/uucp/uucpadm/do_vi.c, cmduucp, bos411, 9428A410j 8/3/93 16:15:14";
/* 
 * COMPONENT_NAME: CMDUUCP do_vi.c
 * 
 * FUNCTIONS: do_vi 
 *
 * ORIGINS: 10  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <curses.h>
#include <string.h>
#include <sys/stat.h>
#include "defs.h"

extern char File[FILES][MAXFILE];
extern WINDOW *active;

int do_vi(target,lptr)
struct files *target;
char *lptr;
{
	char  sysbuf[MAXEDIT];
	char  *p, *q;
	char *getenv();
	int  k;
	int  err;
	struct stat dummy;
	resetterm();
	/*
	 *  Scan for line number.  Ugh!
	 */
	p = File[target->spot];
	k = 1;
	if (lptr != NULL) {
	while (p != lptr)
	    	    {
			k++;
			p += strlen (p) + 1;
	    	    }
		}

	 /*
	  *  Get pointer to environment EDITOR string or default
	  *  to "vi".  This may be a full path.
	  */
	p = getenv ("EDITOR");		/* get user's prefrnc */
	if (p == NULL || *p == '\0') {
/* Make sure we can find an editor. Exit if not. Why waste time. */
		if (stat("/usr/bin/vi",&dummy) != 0)
			derror(EX_USAGE,"Can't find an editor. Install vi or set EDITOR in environment.");
		p = "vi";
			}
	/*
	 *  Get pointer to the name portion.  This is used for
	 *  deciding the format of the command line.
	 */
	q = strrchr (p, '/');		/* rightmost slash */
	if (q == NULL)
		q = p;				/* no slashes */
	else
		q++;				/* move past slash */
	/*
	 *  Make sure our sprints won't overflow.
	 */
	if (strlen (p) + 25 + strlen (target->name) > MAXEDIT - 1)
	{
		derror(EX_SOFTWARE, "Sysbuf size error");
	}

	/*
	 *  Create command line, depending on the editor.
	 *  Use full path to editor in the command.
	 */
	if (!strcmp (q, "vi"))
	        (void) sprintf (sysbuf, "%s +%d %s", p, k, target->name);
	else if (!strcmp (q, "e"))
		(void) sprintf (sysbuf, "%s %s %d", p, target->name, k);
	else
		(void) sprintf (sysbuf, "%s %s", p, target->name);

/* Do a temporary chmod to eliminate read only message which confuses
*  some users. Since we are running as root we don't care about success.
*/
	(void) chmod(target->name,(size_t) 00600);
	if (err = system (sysbuf))		/* fork/exec editor */
	{
	if ((mvwprintw (active,active->_cury + 1,active->_curx, "Error executing editor:")) == ERR)
		derror(EX_SC,"Error on write to active screen!");
		    }
	if ((chmod(target->name,(size_t) 00400)) != 0) 
		if ((mvwprintw (active,active->_cury + 1,active->_curx, "Error setting permissions on %s",target->name)) == ERR)
		derror(EX_SC,"Error on write to active screen!");
	if ((err = bload(target)) != EX_OK) 
		derror(err,"Can't reload file after editor!"); /* reload file */
	initscr();
	noecho();
	cbreak();
	nonl();
	return(EX_OK);
}
