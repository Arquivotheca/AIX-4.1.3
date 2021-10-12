static char sccsid[] = "@(#)58	1.19.1.9  src/bos/usr/bin/chuser/chuser.c, cmdsuser, bos41J, 9514A_all 3/28/95 15:36:59";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: main
 *		makechanges
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<stdio.h>	/* for printfs		*/
#include	<sys/audit.h>	/* for auditing		*/
#include	<sys/priv.h>	/* for setpriv		*/
#include	<sys/access.h>	/* for accessx		*/
#include	<userpw.h>	/* for PW_NAMELEN 	*/
#include	<locale.h>	/* for setlocale() 	*/
#include	<pwd.h>		/* for getpwnam()	*/
#include	"tcbauth.h"

/* local defines */
static	void	makechanges(char *user,int argc,char *argv[]);

/*
 *
 * NAME:     chuser
 *                                                                    
 * FUNCTION: changes the attributes of a user
 *                                                                    
 * USAGE:    chuser "attr=value" ... username
 *	     where:
 *		"attr" 	  : is a valid user attribute.
 *		"value"   : is a valid value for that attribute.
 *		"username": is the user to be changed.
 *
 * PASSED:	argc = the number of attributes entered.
 *		argv[] = the attributes entered.
 *                                                                   
 * RETURNS: 0 if successful else the return is as follows:
 *		
 *		ENOENT 	if the username or specified group doesn't exist.
 *		EINVAL 	if the attribute or attribute value is invalid
 *		EACCES 	if the attribute cannot be changed - i.e., the invoker
 *			doesn't have write access to the one or more of the user
 *			database files.
 *		EPERM 	if the user identification and authentication fails -
 *			if admin user or attribute=admin, the invoker must be 
 *			root.
 *		errno	if system error.
 */  


main(int argc,char *argv[])
{
char	name[PW_NAMELEN];	/* the username	*/
uid_t	uid;
int	id;
char	*real;
struct	passwd *pw;

	/*
	 * must be minimum of: chuser "attr=value" <username>
	 */


	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_TCBAUTH, NL_CAT_LOCALE);

	if (argc < 3)
		usage(CHUSRUSAGE);


	if (strlen(argv[argc - 1]) > PW_NAMELEN - 1)
	{
		fprintf(stderr,CHGONERR,name);
		fprintf(stderr,TOOLONG);
		exit(ENAMETOOLONG);
	}

	strcpy(name,argv[argc - 1]);

	/* 
	 * suspend auditing for this process
	 */
	auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);

	/* 
	 * suspend privilege
	 */
	privilege(PRIV_LAPSE);

	/*
	 * see if the user exists locally or via NIS
	 */
	set_getpwent_remote(2);	/* compat lookups only	*/
	pw = getpwnam(name);
	set_getpwent_remote(1);	/* resume full lookups	*/
	if (pw == (struct passwd *)NULL)
	{
		fprintf(stderr,USRNONEX,name);
		exit(ENOENT);
	}

	/* Open the user database */
	setuserdb( S_READ | S_WRITE );

	/*
	 * See if the invoker has permission to change this user
 	 * A user can change his own "gecos" field and his
	 * shell attributes. If the invoker is the user, continue.
	 */
	uid = getuid();
	if ((real = IDtouser(uid)) == NULL)
	{
		fprintf(stderr,USRNONEX,name);
		exitax(CHUSRAUD,NOEXISTS,name,(char *)NULL,PRINT);
	}

	real = strdup(real);

 	if (strcmp(real,name))
	{
 		if (!gotuaccess(name))
 		{
 			fprintf(stderr,CHGONERR,name);
 			exitax(CHUSRAUD,EPERM,name,(char *)NULL,PRINT);
 		}
	}

	/*
	 * if we're an NIS or DCE client, we cannot change the attributes
	 * we get from the server...
	 */
	if (isuserrmtnam(name))
	{
		char *equal;
		int a, r, getout = 0;     /*  getout used for exiting  */

		/* Attributes defined on the server */
		static struct 
		{
			char	*attr;		/* attribute name	*/
			int	mesgno;		/* error message number */
			char	*mesg;		/* default message	*/
		} remote_attrs[] = { S_ID,	CANT_ID, 	DEF_CANT_ID,	
				     S_PGRP,	CANT_PGRP,	DEF_CANT_PGRP,	
				     S_GROUPS, 	CANT_GROUPS,	DEF_CANT_GROUPS,
				     S_HOME,	CANT_HOME,	DEF_CANT_HOME,	
				     S_SHELL, 	CANT_SHELL,	DEF_CANT_SHELL,	
				     S_GECOS,	CANT_GECOS,	DEF_CANT_GECOS,
				     NULL,	0,		NULL	
			   };
		/*
		 * walk through and check each attribute
		 */
		for (a = 1; a < argc - 1; a++)
			if (equal = strchr(argv[a], '='))
			{
				/*
				 *  zap equal sign
				 */
				*equal = '\0';

				/*
				 * check the list of NIS or DCE attributes
				 */
				for (r = 0; remote_attrs[r].attr &&
				     strcmp(argv[a],remote_attrs[r].attr); r++)
					;
				/*
				 * found one.  error.
				 */
				if (remote_attrs[r].attr)
				{
					fprintf(stderr,
						MSGSTR(remote_attrs[r].mesgno,
						remote_attrs[r].mesg));
					getout++;
				}
				/*
				 * restore equals sign
				 */
				*equal = '=';
			}

		if (getout)
			exitax(CHUSRAUD, EPERM, name, NULL, NOPRINT);
	}

	/*
	 * Make the changes to the user specified.
	 * This routine audits and exits
	 */
	makechanges(name,argc,argv);
}

/*
 * FUNCTION:	makechanges
 *
 * DESCRIPTION:	calls routines to parse input, add the info
 *		to the data base, close the data base.
 *
 * RETURNS:	audits success and exits or exits on fail.
 *
 */

static	void
makechanges(char *user,int argc,char *argv[])
{
register  int	i;			  /* counter 			   */
char		*attr;			  /* pointer to attribute 	   */
char		*val;			  /* pointer to attribute value    */
char		*arecord = (char *)NULL;  /* place to hold auditing record */
int		siz;			  /* size of audit record	   */
DBM		*pwdbm;			  /* for the database update 	   */

	/* open database */
	pwdbm = pwdbm_open();

	for (i = 1; i < (argc - 1); i++)
	{
		/*
		 * Build the audit record.  This is done
		 * first to preserve the attr=value string.
		 */
		if (i == 1)
		{
			if ((arecord = malloc(strlen(argv[i]) + 1)) == NULL)
			{
				fprintf (stderr, MALLOC);
 				exitax(CHUSRAUD,errno,user,(char *)NULL,PRINT);
			}
			strcpy(arecord,argv[i]);
		}
		else
		{
			siz = strlen(arecord) + strlen(argv[i]) + 2;
			if ((arecord = realloc(arecord,siz)) == NULL)
			{
				fprintf (stderr, MALLOC);
 				exitax(CHUSRAUD,errno,user,(char *)NULL,PRINT);
			}
			strcat(arecord,SPACE);
			strcat(arecord, argv[i]);
		}

		if (getvals(argv[i],&attr,&val))
			usage(CHUSRUSAGE);

		/* 
		 * If the invoker is root, there's no
		 * need to check the access 
		 */
		if(!gotaccess())
		{
			if(!chkaccess(attr,val,user))
			{
				fprintf(stderr,CHGTOERR,attr,val);
				exitax(CHUSRAUD,EPERM,user,arecord,PRINT);
			}
		}

		/*
	 	 * add the attribute
	 	*/
		if (putvalue(user,attr,val,arecord,CHUSRAUD))
		{
			fprintf(stderr,CHGTOERR,attr,val);
			exitax(CHUSRAUD,errno,user,arecord,PRINT);
		}
	}

	/*
	 * commit the changes to the data base
	 */
	if (putuserattr(user,(char *)NULL,((void *) 0),SEC_COMMIT))
	{
		fprintf(stderr,COMMIT,user);
		exitax(CHUSRAUD,errno,user,arecord,PRINT);
	}
	
	if(putgroupattr((char *)NULL,(char *)NULL,(void *)NULL,SEC_COMMIT))
	{
		fprintf(stderr,COMMIT,user);
		exitax(CHUSRAUD,errno,user,NULL,PRINT);
	}

	/* Close the user database */
	enduserdb();

	/*
	 * Update the dbm files.  There's no need to use
	 * pwdbm_delete() since pwdbm_add() specifies 
	 * replace.
	 */
	if (pwdbm != NULL) {
		if (pwdbm_add(pwdbm, user))
			fprintf (stderr, DBM_ADD_FAIL);
		(void) dbm_close(pwdbm);
	}

	exitax(CHUSRAUD,0,user,arecord,NOPRINT);

}
