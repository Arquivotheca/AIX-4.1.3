static char sccsid[] = "@(#)53	1.14.1.6  src/bos/usr/bin/chfn/chfn.c, cmdsuser, bos411, 9428A410j 3/25/94 14:50:57";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: getgecos
 *		main
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>		/* for printfs			*/
#include <errno.h>		/* for errors			*/
#include "tcbauth.h"		/* for local defines		*/
#include <locale.h>		/* for setlocale()		*/
#include <langinfo.h>		/* for language information  	*/
#include <stdlib.h>		/* for rpmatch          	*/

static char	*getgecos(char *username);

/*
 *
 * NAME: 	chfn
 *                                                                    
 * DESCRIPTION: changes the gecos field in /etc/passwd
 *                                                                    
 * USAGE:	chfn <username>
 *	        where:
 *			username is a valid user of this system
 *                                                                   
 * RETURNS: 0 if successful else errno is set and returned.
 *		
 */  

main(int argc,char *argv[])
{
char	*gecos;		/* pointer to returned gecos value		*/
char	*name;		/* the username to be changed			*/
char	answer[BUFSIZ]; /* the input string				*/
int	rc;		/* return code from changeattr()		*/
char	*yprompt;
char	*nprompt;
char	*p;

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_TCBAUTH, NL_CAT_LOCALE);

	if (argc > 2)
		usage(CHFNUSAGE);

	if (argv[1] && argv[1][0] == '-')
		usage(CHFNUSAGE);

	/* Open the user database */
	setuserdb( S_READ | S_WRITE );

	/* see if user exists */
	if ((name = getusername(argc,argv[1])) == NULL)
	{
		if (argv[1])
		{
			fprintf(stderr,USRNONEX,argv[1]);
			exit (errno);
		}
		fprintf(stderr,USRINFO);
		fprintf (stderr, CHECK, PASSWD);
		exit (errno);
	}

	/*
	 * See if the invoker can change the gecos -- a user can change his
	 * own gecos only; root can change anyone's;group security can change
	 * only ordinary users (admin=false) gecos 
	*/

	if (!gotiaccess(name))
	{
		fprintf(stderr,CHGONERR,name);
		fprintf (stderr, PERMISSION);
		exitax(CHUSRAUD,EPERM,name,(char *)NULL,NOPRINT);
	} 

	/* get the users current gecos */
	if ((gecos = getgecos(name)) == NULL)
	{
		if (errno != ENOATTR && errno != ENOENT)
		{
			fprintf(stderr,GERRGET,name);
			fprintf (stderr,CHECK,PASSWD);
			exit (errno);
		}
	}

	/* print the current value to the screen */
	printf (CURRGEK,name,gecos);
	printf("\n");

	/* prompt for response */
	yprompt = nl_langinfo(YESSTR);
	if (*yprompt == '\0')
		yprompt = "yes";
	/* Use answer to save yesstr, since next call to nl_langinfo
	   may overwrite static area. */
	strcpy(answer, yprompt);	/* Assume answer is big enough. */

	nprompt = nl_langinfo(NOSTR);
	if (*nprompt == '\0')
		nprompt = "no";

	printf (CHANGE,answer,nprompt);

	/* read response */
	fgets (answer,BUFSIZ,stdin);

	/*
	 *  Take off the ending newline
	 *  character from the fgets.
	 */

	if ((p = strrchr (answer,'\n')) != NULL)
		*p = '\0';

	/* get response */
	if (rpmatch(answer) == 1)
	{
		/* prompt for response */
		printf(TO);
		fgets (answer,BUFSIZ,stdin);

		/*
	 	 *  Take off the ending newline
	 	 *  character from the fgets.
	 	 */

		if ((p = strrchr (answer,'\n')) != NULL)
			*p = '\0';

		/* set gecos to new value */
		rc = changeattr(name,GHEAD,answer);

		if (rc)
			exit(rc);

		/* audit this success */
		exitax(CHUSRAUD,rc,name,answer,NOPRINT);
	}
	else
	{
		printf (SETGECOS);
		exit (0);
	}
}

/*
 * FUNCTION:	getgecos
 *
 * DESCRIPTION:	Gets the current gecos information.
 *
 * PASSED:	The username to be changed (username).
 *
 * RETURNS:	The current gecos or NULL.
 *
 */

static char	*
getgecos(char *username)
{
char	*val;		/* the gecos value		*/

	if(getuserattr(username,S_GECOS,&val,SEC_CHAR))
		return(NULL);
	else
		return(val);

}

