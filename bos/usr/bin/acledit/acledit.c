static char sccsid[] = "@(#)63	1.19  src/bos/usr/bin/acledit/acledit.c, cmdsdac, bos411, 9428A410j 2/2/94 13:19:35";
/*
 * COMPONENT_NAME:  (CMDSDAC) security: access control
 *
 * FUNCTIONS:  acledit
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * acledit filename
 *
 * The acledit command:
 *	1) Invokeds "aclget" to format access control information of a file.
 *	2) Runs an editor on the formatted output.  An editor must be speci-
 *	   fied by the EDITOR environment variable.  There is no default
 *	   for the editor.
 *	3) Prompts the user asking whether the modified access control infor-
 *	   mation should be applied.
 *	4) Invokes "aclput" with the resulting file as input.  If this fails,
 *	   "acledit" returns to step 2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <nl_types.h>
#include <errno.h>
#include <regex.h>
#include <locale.h>
#include <sys/stat.h>

#define DPRINTF(args)

#define ACLGET	"/usr/bin/aclget"
#define ACLPUT	"/usr/bin/aclput"

#include "acledit_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_ACLEDIT,n,s) 
extern  int     rpmatch (const char *);

char	*getenv();	/* routine to get environment variable values */
char	*editor;	/* editor to be used (from the environment) */
char	*getvec[5];	/* argument vector for invoking aclget */
char	*putvec[5];	/* argument vector for invoking aclput */
char	*edvec[3];	/* argument vector for invoking the editor */
char	tempfile[] = "/tmp/acleXXXXXX";
char	*mktemp();

int runv(char *prog, char **av);


int
main(int	ac, 
     char	**av)
{
	char	answer[BUFSIZ];
	int	rc;
	register int    ret;    /* return from rpmatch()                */
	char    *yprompt;
	char    *nprompt;
	int     gotit = 1;   /* Message taken from message catalog */
	int 	defaultyes = 0;  /* Return on prompt implies a default yes */

	(void)setlocale(LC_ALL, "");
	catd = catopen(MF_ACLEDIT, NL_CAT_LOCALE);

	if (ac != 2)
	{
		fprintf(stderr, 
MSGSTR(USAGE, "Usage:  %s filename\n") , av[0]);
		exit(-1);
	}

	if (((editor = getenv("EDITOR")) == NULL) ||
	    (*editor == (char)NULL))
	{
		fprintf(stderr, 
MSGSTR(NOED, "%s:  EDITOR environment variable not set\n"), av[0]);
		exit(-1);
	}

	if (*editor != '/')	/* EDITOR must be absoulute path */
	{
		fprintf(stderr,
MSGSTR(EDNOTABS, 
"%s:  EDITOR environment variable must be full pathname\n"), av[0]);
		exit(-1);
	}
	
	if (mktemp(tempfile) == NULL)
	{
		fprintf(stderr, 
MSGSTR(NOTMP, "%s: Cannot create temporary file\n"), av[0]);
		exit(-1);
	}

	/* set up arg vector for aclput */
	putvec[0] = "aclput"; 
	putvec[1] = "-i";
	putvec[2] = tempfile;
	putvec[3] = av[1];
	putvec[4] = 0;

	/* set up arg vector for editor */
	edvec[0] = editor;
	edvec[1] = tempfile;
	edvec[2] = 0;

	/* set up arg vector for aclget */
	getvec[0] = "aclget"; 
	getvec[1] = "-o";
	getvec[2] = tempfile;
	getvec[3] = av[1];
	getvec[4] = 0;

	/*
	 * get the file's ACL and place it into "tempfile"
	 */
	rc = runv(ACLGET, getvec);
	if (rc)
	{
		fprintf(stderr, 
MSGSTR(NOACC, "%s: Cannot access %s\n"), av[0], av[1]);
		exit(-1);
	}

	do
	{
		struct stat	stbuf;
		time_t		start_time;

		if (stat(tempfile, &stbuf) < 0)
		{
			fprintf(stderr,
MSGSTR(NOACC, "%s: Cannot access %s\n"), av[0], tempfile);
			exit(-1);
		}
		start_time = stbuf.st_mtime;

		/* edit "tempfile" */
		if ((rc = runv(editor, edvec)) != 0)
			break;

		if (stat(tempfile, &stbuf) < 0)
		{
			fprintf(stderr,
MSGSTR(NOACC, "%s: Cannot access %s\n"), av[0], tempfile);
			exit(-1);
		}
		if (start_time == stbuf.st_mtime)
			break;
	
		/* prompt for response */
      		if ((yprompt =  catgets (catd, MS_ACLEDIT, YESSTR, (char *)NULL)) == (char *)NULL)
        	{
               		yprompt = "yes";
                	gotit = 0;
        	}

      		if ((nprompt = catgets (catd, MS_ACLEDIT, NOSTR, (char *)NULL)) == (char *)NULL)
        	{
                	nprompt = "no";
                	gotit = 0;
        	}

		printf(
MSGSTR(APPLY, "Should the modified ACL be applied? (%s) or (%s) "), yprompt, nprompt);
		fflush(stdout);
		gets(answer);

		if (*answer == (char)NULL)
			defaultyes = 1;

		/* get resposnse */
		if (defaultyes)
		   	ret = 1;
        	else if (gotit)
                	ret = rpmatch(answer);
        	else
        	{
                	ret = -1;               /* for invalid response */
                        if ( (answer[0] == 'y') || (answer[0] == 'Y') )
                                	ret = 1;
                        if ( (answer[0] == 'n') || (answer[0] == 'n') )
                                	ret = 0;
        	}

		if (ret != 1)
			break;

		/* set the file's ACL */
		rc = runv(ACLPUT, putvec);
		DPRINTF(("acledit: aclput's rc=%d\n", rc));
	} while (rc == 255); /* -1 => syntax error that can be corrected */

	unlink(tempfile);
	exit(rc);
}


/*
 * This routine simply executes a program as a child process and returns the
 * relevant portion of the return code.
 */
static int
runv(char	*prog,
     char	**av)
{
	int	status;

	if (status = fork())	/* parent */
	{
		wait(&status);
		return((status>>8)&0xff);
	}

	/* child */
	execv(prog, av);
	fprintf(stderr, 
MSGSTR(NOEXEC, "Cannot execute %s\n"), prog);
	exit(ENOENT);
}
