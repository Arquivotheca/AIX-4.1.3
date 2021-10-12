static char sccsid[] = "@(#)67	1.16  src/bos/usr/bin/env/env.c, cmdsh, bos41B, 412_41B_sync 12/15/94 12:11:58";
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS: env
 *
 * ORIGINS: 3, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <errno.h>

#include "env_msg.h"

extern	char **environ;

char	*nvmatch(); 
int	printvar();
void	addname(), print(), usage();

#define NENV	250		/* max # of env vars we handle	*/
static char	*newenv[NENV +1];	/* our new environment		*/
static char	*nullp = NULL;		/* a null environment		*/

/*
 * NAME:	env, printenv
 *
 * SYNTAX:	env [-i | -] [name] [name=value] ... [command [arg...]]
 *		printenv [name]
 *
 * FUNCTION:	Both env and printenv can be used to print out the
 *		current environment or a particular environment
 *		variable.  If called without any arguments, both
 *		simply print out all environment variables.  If
 *		called with 1 argument, and that argument matches
 *		an environment variable name, the environment
 *		variable name and its value is printed.
 *
 *		Env can be used to set environment variables, then
 *		execute a command with the new environment.  Arguments
 *		of the form "name=value" are added to the current
 *		environment before "command" is executed.  If "-i" is
 *		specified, env ignores the current environment and
 *		builds a new environment from scratch using the
 *		"name=value" argument.
 *
 * RETURN VALUE DESCRIPTION:
 *		0	The program completed successfully.
 *		1	An error occurred in the program.	
 *		126  	The command specified by command was found but
 *			could not be invoked.		
 *		127  	The command specified by command could not be found.
 */
int
main(int argc, char *argv[], char *envp[])
{
	register char *progname;
	register int printenv;
	int c;
	int iflg = 0;
	extern int  optind;
	
	(void) setlocale (LC_ALL, "");

	/*
	 * get the program name minus the path
	 */
	if ((progname = strrchr(argv[0], '/')) != NULL)
		progname++;
	else
		progname = argv[0];

	printenv = (strcmp(progname, "printenv") == 0);

	while ((c=getopt(argc, argv, "i")) != EOF)
	{	
		switch (c) {
			case 'i':
				iflg++;
				break;
			default:
				usage(printenv ? PUSAGE : EUSAGE);	
                                exit(1);
		}
	}

	argc -= optind;
	argv += optind;;
	
	/*
	 * check for the "-" argument.
	 * The code does not support env -i - [command [arg ...]]. Anything
 	 * following the - option are treated as operands, including --.
	 */
	if (strcmp(*argv, "-")==0 && (optind == 1))
	{
		iflg++;
		argc--;
		argv++;
	}

	if (iflg)
		/*
		 * can't call printenv with "-" or "-i".
		 */
		if (printenv)
		{
			usage(PUSAGE);	
			exit(1);
		}
		else
			/* start with a null environment */
			envp = &nullp;

	if (printenv)
	{
		/*
		 * printenv specific ...
		 */
		if (argc == 0)
			/*
			 * just print the environment if no arguments 
			 * given
			 */
			print();
			
		else if (argc == 1)                         
		{
			/*
			 * print out a particular value
			 */
			if (!printvar(argv[0]))
				exit(1);
		}
			
		else
		{
			/*
			 * can only run with 1 or no arguments
			 */
			usage(PUSAGE);
			exit(1);
		}
	}
	
	else
	{		
		/*
		 * env specific ...
		 */
			
		/*
		 * put current environment into new environment
		 */
		for (; *envp != NULL; envp++)
			if (strchr(*envp, '=') != NULL)
				addname(*envp);
		
		/*
		 * add "name=value" arguments
		 */
		while (*argv != NULL && strchr(*argv, '=') != NULL)
			addname(*argv++);
			
		environ = newenv;	/* reset environment */
			
		if (*argv == NULL)
			/*
			 * just print out environment if no arguments 
			 * given
			 */
			print();
			
		/*
		 * if there's only one argument left, try to print it.
		 * if it's not in our environment, execute it
		 */
		else if (*(argv + 1) != NULL || !printvar(*argv))
		{
			/*
			 * execute a command
			 */
			(void) execvp(*argv, argv);
					
			perror(*argv);
			if (errno == ENOENT)
				exit(127);
			else
				exit(126);
		}
	}
	
	exit(0);
}

/*
 * NAME:  addname
 *
 * FUNCTION:  adds a variable and its value to the environment. 
 */

static void
addname(register char *arg)
{
	register char **p;

	for (p = newenv; *p != NULL; p++)	/* search for variable */
	{
		if (p >= &newenv[NENV-1])	/* environment full... */
		{
			
			usage(OVERFLOW);
			print();
			exit(1);
		}

		if (nvmatch(arg, *p) != NULL)	/* match? */
			break;
	}

	*p = arg;		/* add it */
}

/*
 * NAME:  print
 *
 * FUNCTION: prints out the entire environment
 *	     (variable names and their value).
 *	     
 */

static void
print(void)
{
	register char **e;

	for (e = environ; *e != NULL; e++)
		(void) puts(*e);
}

/*
 *	s1 is either name, or name=value
 *	s2 is name=value
 *	if names match, return value of s2, else NULL
 */

static char *
nvmatch(register char *s1, register char *s2)
{
	while (*s1 == *s2++)
		if (*s1++ == '=')
			return(s2);

	if (*s1 == '\0' && *(s2-1) == '=')
		return(s2);

	return(NULL);
}

/*
 * NAME: printvar
 * 
 * FUNCTION:  searches for the value of name and then it prints out the value.
 *
 * RETURN: returns 1 if we found and printed "name=value", else 0
 */

static int
printvar(register char *name)
{
	register char **e, *value;

	for (e = environ; *e != NULL; e++)	/* search enviroment */
		if ((value = nvmatch(name, *e)) != NULL)
		{
			printf ("%s\n", value);
			return(1);
		}

	return(0);
}

/*
 * NAME:  usage
 * USAGE: usage(int) where int is the # of error message called.
 * FUNCTION: display the correct error messages for this command.
 * A good convention would be to call the message by the same number
 * as the message is called in the Message set in the env.msg file.
 * PUSAGE is for printenv EUSAGE is for env.
 */

static void
usage(int cmd)
{
	char msgstr[80];
	nl_catd cat;

	cat = catopen(MF_ENV,NL_CAT_LOCALE);

	switch(cmd)
	{
		case OVERFLOW:
			strcpy(msgstr,"too many values in environment\n");
			break;
		case PUSAGE:
			strcpy(msgstr,"Usage: printenv [name]\n");
			break;
		case EUSAGE:
			strcpy(msgstr,"Usage: env [-i | -] [name=value]... [command [argument...]]\n       env [name]\n");
			break;
	}

	(void) fputs(catgets(cat, MS_ENV, cmd, msgstr), stderr);
	catclose(cat);
}
