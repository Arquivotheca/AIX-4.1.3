static char sccsid[] = "@(#)63	1.20.1.9  src/bos/usr/bin/mkuser/mkuser.c, cmdsuser, bos411, 9438C411a 9/23/94 15:21:06";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: addattr
 *		adduser
 *		getauthvals
 *		gethomedir
 *		main
 *		makeuser
 *		putfromargs
 *		runprog
 *		runremove
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

#include	<stdio.h>	/* for printfs		*/
#include	<limits.h>	/* for LINE_MAX		*/
#include	<sys/stat.h>	/* for mkdir		*/
#include	<sys/param.h>	/* for auditing		*/
#include	<sys/access.h>	/* for accessx		*/
#include	<sys/audit.h>	/* for auditing		*/
#include	<sys/priv.h>	/* for privilege 	*/
#include	<pwd.h>		/* for passwd struct 	*/
#include	<locale.h>	/* for setlocale() 	*/
#include	<userpw.h>	/* for PW_NAMELEN	*/
#include	<sys/wait.h>	/* for fork()		*/
#include	"tcbauth.h"

static	void	makeuser(char *,char *,int,char **);
static	void	adduser(int,char *,char *,uid_t,uid_t,char *);
static	int	addattr(char *type,char *name);
static	void	gethomedir(char **,char *,char *);
static	void	getauthvals(char *,char *);
static	int	runremove(char *);
static	int	runprog(char *);
static	void	putfromargs (int,char *,int,char **);

char	*__groupset;	/* Global variable to contain the user's GROUPS= */

/*
 *
 * NAME:     mkuser
 *                                                                    
 * FUNCTION: creates a new user account.
 *                                                                    
 * USAGE:    mkuser [-a] "attr=value" ... username
 *	     where:
 *	              -a : is an administrative user.
 *		    attr : is a valid user attribute.
 *		   value : is a valid value for that attribute.
 *		username : is the new user.
 *                                                                   
 * RETURNS: 0 if successful else the return is as follows:
 *		
 *		EINVAL 	 if the username argument is invalid (invalid chars)
 *		EACCES 	 if the invoker doesn't have write access to database
 *		 EPERM 	 if the user identification and authentication fails
 *			 if -a is specified and invoker != root.
 *		EEXIST 	 if the user already exists
 *	  ENAMETOOLONG   if the user's name is too long. 
 *		 errno	 if system error.
 */  

main(int argc,char *argv[])
{
char		*name = argv[argc - 1];	/* new user name */
char		*type = SC_SYS_USER;	/* admin or not */

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_TCBAUTH, NL_CAT_LOCALE);

	if (argc < 2 || (name && *name == '-'))
		usage(MKUSRUSAGE);

	/* suspend auditing for this process */
	auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);

	/* suspend privilege */
	privilege(PRIV_LAPSE);

	/* Open the user database */
	setuserdb( S_READ | S_WRITE );

	/* check name for restrictions */
	if (chkname(1,name))
		exitax(MKUSRAUD,errno,name,NULL,PRINT);

	/*
	 * Check for "-a" flag.  If so, set
	 * the user type to "administrative".
	 */
	if (argc > 2)
	{
		if (argv[1][0] == '-')
		{
			if (strcmp(argv[1],"-a"))
				usage(MKUSRUSAGE);
			else
				type = SC_SYS_ADMUSER;
		}

	}	

	/*
	 * Create user.  This routine does the
	 * necessary auditing and exits.
	 */
	makeuser(type,name,argc,argv);
}

/*
 * FUNCTION:	makeuser
 *
 * DESCRIPTION:	checks for invoker's access, checks for newuser's existence,
 *		gets the new user id's for updating the file, adds the user
 *		to the data base, updates the id file, cuts audit record.
 *
 * PASSED:	type = the flag of user (admin or not),
 *		username = the user to 	be added.
 *
 * RETURNS:	None.
 *
 */

static	void
makeuser(char *type,char *username,int argc,char **argv)
{
uid_t		auid;
uid_t		uid;
gid_t		agid;
gid_t		gid;
gid_t		dgid;		/* default group id 			*/
int		ruid;		/* user's final id value                */
char		*dir;
char		*groupname;
register int	tflag = 0;	/* type flag - user or administrator	*/
register int	err = 0;
int		f;		/* file descriptor 			*/
int		saverrno;	/* error return for removing user       */
DBM		*pwdbm;		/* for adding to the passwd database	*/

	if(!strcmp(type,SC_SYS_ADMUSER))
		tflag=1;	/* administrative user */

	/* Only root can create an administrative user */
	if (tflag && !gotaccess())
	{
		fprintf(stderr,ERRADDADM,username);
		exitax(MKUSRAUD,EPERM,username,NULL,PRINT);
	}

	/* Get the available admin & ordinary ids */
	f = getids(username,&auid,&uid,&agid,&gid,MKUSRAUD);

	/* get default primary group from data base */
	if(getconfattr(type,SC_GROUP,&groupname,SEC_CHAR))
	{
		fprintf (stderr,NODEFGRP);
		fprintf (stderr,CHECK,MKDEF);
		exitax(MKUSRAUD,errno,username,NULL,NOPRINT);
	}

	/* make sure "groupname" exists and get id */
	if (getgroupattr(groupname,S_ID,&dgid,SEC_INT))
	{
		fprintf (stderr,GRPNONEX,*groupname);
		fprintf (stderr,CHECK,MKDEF);
		exitax(MKUSRAUD,errno,username,NULL,NOPRINT);
	}

	pwdbm = pwdbm_open();

	/*
	 * Set the user's "groups=" groupset to be
	 * only the primary group right now.
	 */
	if (addtogroup(username,groupname))
	{
		fprintf(stderr,GETUSRGRPS,username);
		fprintf(stderr,CHECK,GROUP);
		exitax(MKUSRAUD,errno,username,NULL,NOPRINT);
	}

	/*
	 * Get the user's default home directory if it exists.
	 * No need to check the return code here.
	 * 
	 * Then add the user to the database.  adduser() will
	 * exit upon error so no need for error check here.
	 *
	 * Also, let's get the AUTH1 and AUTH2 attributes from
	 * the file mkuser.default and process the $USER
	 * attributes if any exists.
	 *
	 * Then go through mkuser.default and add any default
	 * attributes to this user.  If failures, update err.
	 *
	 * Add any user supplied attributes at this time.  
	 * putfromargs() exits upon error so no error checking
	 * needs to be done here.
	 */

	gethomedir(&dir,username,type);
	adduser(tflag,username,dir,auid,uid,groupname);
	getauthvals(username,type);

	if (addattr(type,username))
	{
		err++;
		saverrno = ENOENT;
	}

	putfromargs (tflag,username,argc,argv);

	/*
	 * Now that we've finished the cache updates, let's
	 * do one big write-to-disk to COMMIT the user and
	 * group caches.
	 */
	if(putuserattr(username,(char *)NULL,(void *)NULL,SEC_COMMIT))
	{
		fprintf(stderr,COMMIT,username);
		exitax(MKUSRAUD,errno,username,NULL,PRINT);
	}

	if(putgroupattr((char *)NULL,(char *)NULL,(void *)NULL,SEC_COMMIT))
	{
		fprintf(stderr,COMMIT,groupname);
		exitax(MKUSRAUD,errno,groupname,NULL,PRINT);
	}

	/* Close the user database */
	enduserdb();

	/*
	 * update the dbm files.
	 */
	if (pwdbm != NULL) {
		if (pwdbm_add(pwdbm, username))
			fprintf (stderr, DBM_ADD_FAIL);
		(void) dbm_close(pwdbm);
	}

	/* Run mkuser.sys */
	if (runprog(username))
	{
		err++;
		fprintf (stderr,MKUSRSYS);
		fprintf (stderr,CHECK,MKSYS);
		saverrno = EACCES;
	}

	/*
	 * If any bad attrs or mkuser.sys fails, let's
	 * remove this user.
	 */
	if (err)
	{
		if(runremove(username))
			exitax(MKUSRAUD,saverrno,username,uid,NOPRINT);
		else
			exit(saverrno);
	}

	/* Get the actual UID for this user */
	getuserattr(username, S_ID, &ruid, SEC_INT);
	 
	/*
	 * If the new user has the same UID as the last UID in the
	 * .ids file, that UID has to be incremented.  This avoids
	 * needlessly incrementing the .ids file when the UID was
	 * given on the command line.  The first argument passed
	 * is 0 which tells inkids() that we've added a user.
	 */

	if (ruid == (tflag ? auid:uid))
		inkids(0,tflag,auid,uid,agid,gid,f);
		
	/*
	 * Audit success and exit.
	 */
	exitax(MKUSRAUD,0,username,NULL,NOPRINT);
}

/*
 * FUNCTION:	putfromargs
 *
 * DESCRIPTION:	add the attributes to the specific user database files
 *
 * RETURNS:	None.
 *
 */

static void
putfromargs (int tflag,char *user,int argc,char **argv)
{
int	i=1;		/* The command line argument to begin parsing	     */
char	*attr;		/* The attribute that we'll pass to putvalue()	     */
char	*val;		/* The value of the attribute passed to putvalue()   */
char	*members;	/* The members of the default group		     */
char	*finaluserset;  /* The final user set to be put in the default group */
char	*pgrpval;	/* The pgrp value off of the command line 	     */
char	*groupsval;	/* The groups value off of the command line 	     */
register int	siz;	/* The size of the value in pgrp or groups  	     */
int	pgrpflag = 0;	/* Flag from "pgrp" entry off of the command line    */
int	groupsflag = 0;	/* Flag from "groups" entry off of the command line  */
int	result = 0;	/* return from check routines			     */
void	*returnval;	/* return value from check routines		     */
char	*finalgrpset;	/* group set from command line, in list format 	     */

	/*
	 * want to look at second argument if
	 * tflag is set to true.
	 */
	if (tflag)
		i=2;

	/* get the passed in attribute values */
	while (argv[i] && (i != (argc - 1)))
	{
		if (getvals(argv[i],&attr,&val))
			usage(MKUSRUSAGE);
		
		/* if the invoker is root, there's no need to chk the access */
		if(!gotaccess())
		{
			if(!chkaccess(attr,val,user))
			{
				fprintf(stderr,CHGTOERR,attr,val);
				exitax(MKUSRAUD,EPERM,user,(char *)NULL,PRINT);
			}
		}

		/*
		 * Let's process all of the command line arguments with
		 * the exception of "pgrp" and "groups", since these
		 * need to be processed last.  The routine putvalue() will
		 * add all of the attributes to the user and group caches.
		 */
		if (!strcmp(attr,S_PGRP) || (!strcmp(attr,S_GROUPS)))
		{
			siz = strlen(val) + 2;
			if (!strcmp(attr,S_PGRP))
			{
				if (pgrpflag)
					free(pgrpval);
				else
					pgrpflag++;

				if ((pgrpval = (char *)malloc(siz)) == NULL)
				{
					fprintf(stderr,MALLOC);
					exitx(errno);
				}
				/*
				 * Drop all spaces and tabs in
				 * our val variable before strcpy().
				 */
				val = dropblanks(val);
				strcpy(pgrpval,val);
			}

			else if (!strcmp(attr,S_GROUPS))
			{
				groupsflag++;
				if ((groupsval = (char *)malloc(siz)) == NULL)
				{
					fprintf(stderr,MALLOC);
					exitx(errno);
				}
				/*
				 * Drop all spaces and tabs in
				 * our val variable before strcpy().
				 */
				val = dropblanks(val);
				strcpy(groupsval,val);
			}
		}
		else
		{
			if (putvalue(user,attr,val,(char *)NULL,MKUSRAUD))
			{
				fprintf(stderr,CHGTOERR,attr,val);
				exitax(MKUSRAUD,errno,user,(char *)NULL,PRINT);
			}
		}
		i++;
	}

	/*
	 * All of the command line is now parsed.  Let's handle
	 * the "pgrp" and "groups" attributes now.
	 *
	 * We have several things to look at here.  We know that
	 * the user has an initial "pgrp" in mkuser.default, and
	 * that his groupset is now set to just that group.  We
	 * also have saved the initial "groups" groupset, if any,
	 * from mkuser.default in the global variable "__groupset".
	 *
	 * Now we only need to check the flags to see if something
	 * else came off the command line.  If not, set the groups
	 * entry to be that from the mkuser.default file.  If none
	 * was supplied, it's already correct with just the pgrp.
	 * If a groups entry was supplied from mkuser.default, then
	 * make sure that it contains the current pgrp, and set the
	 * groupset accordingly.
	 */

	 if (pgrpflag)
	 {
		if ( result = (*(chkpgrp))(pgrpval,&returnval))
		{
			if ((result != EXISTS) && (result != NOEXISTS))
				fprintf(stderr,CHGTOERR,S_PGRP,pgrpval);
			exitax(CHGRPAUD,result,user,(char *)NULL,PRINT);
		}
		else
			returnval = (void *)pgrpval;

		if (addtopgrp(user,returnval))
		{
			fprintf(stderr,GETUSRGRPS,user);
			fprintf(stderr,CHECK,GROUP);
			exitax(CHGRPAUD,errno,user,NULL,
						NOPRINT);
		}
	 }

	 if (groupsflag)
	 {
		if ( result = (*(chkgrps))(groupsval,&returnval))
		{
			if ((result != EXISTS) && (result != NOEXISTS))
				fprintf(stderr,CHGTOERR,S_GROUPS,groupsval);
			exitax(CHGRPAUD,result,user,(char *)NULL,PRINT);
		}
		else
			returnval = (void *)groupsval;

		/*
	 	 * Group list passed from command line.
	 	 */
		strtolist(returnval,&finalgrpset);

		if (addtogrouplist(user,finalgrpset))
		{
			fprintf(stderr,GETUSRGRPS,user);
			fprintf(stderr,CHECK,GROUP);
			exitax(CHGRPAUD,errno,user,NULL,
						NOPRINT);
		}
	 }
	 else
	 {
		if ( result = (*(chkmgrps))(__groupset,&returnval))
		{
			if ((result != EXISTS) && (result != NOEXISTS))
				fprintf(stderr,CHGTOERR,S_GROUPS,__groupset);
			exitax(CHGRPAUD,result,user,(char *)NULL,PRINT);
		}
		else
			returnval = (void *)__groupset;

		/*
	 	 * Using grouplist from mkuser.default.
	 	 */
		if (addtogrouplist(user,returnval))
		{
			fprintf(stderr,GETUSRGRPS,user);
			fprintf(stderr,CHECK,GROUP);
			exitax(CHGRPAUD,errno,user,NULL,
						NOPRINT);
		}
	}
}

/*
 * FUNCTION:	adduser
 *
 * DESCRIPTION:	adds entries to:
 *		/etc/passwd,/etc/security/passwd,/etc/security/user,
 *		/etc/security/environ,/etc/security/limits and
 *		updates /etc/security/user with "admin = whatever"
 *
 * PASSED:	flag = indicates admin or not, name = new user's name,dir = new
 *		user's directory, auid = new user's admin id, uid = new user's 
 *		id, gid = new user's group id.
 *
 * RETURNS:	None.
 *
 */

static	void
adduser(int tflag,char *name,char *dir,uid_t auid,uid_t uid,char *groupname)
{
int	id;		/* holds the new id		*/
char	*admin;		/* "true" or "false"		*/

	/* admin or ordinary user */

	id = tflag==1?auid:uid;
	admin = tflag==1?_TRUE:_FALSE;

	if (putuserattr(name,(char *)NULL,(void *)NULL,SEC_NEW))
	{
		fprintf(stderr,ERRADD,name);
		exitax(MKUSRAUD,errno,name,NULL,PRINT);
	}

	if (putuserattr(name,S_ID,(void *)id,SEC_INT))
	{
		fprintf(stderr,ERRADD,name);
		exitax(MKUSRAUD,errno,name,NULL,PRINT);
	}

	if (putuserattr(name,S_PGRP,(void *)groupname,SEC_CHAR))
	{
		fprintf(stderr,ERRADD,name);
		exitax(MKUSRAUD,errno,name,NULL,PRINT);
	}

	if (putuserattr(name,S_HOME,(void *)dir,SEC_CHAR))
	{
		fprintf(stderr,ERRADD,name);
		exitax(MKUSRAUD,errno,name,NULL,PRINT);
	}

	if (putuserattr(name,S_ADMIN,(void *)admin,SEC_BOOL))
	{
		fprintf(stderr,ERRADD,name);
		exitax(MKUSRAUD,errno,name,NULL,PRINT);
	}

}

/*
 * FUNCTION:	addattr
 *
 * DESCRIPTION:	adds default attributes from data base to user files
 *
 * PASSED:	type (SC_SYS_ADMUSER or SC_SYS_USER), name of newuser.
 *
 * RETURNS:	0 or error.
 *
 */

static	int
addattr(char *type,char *name)
{
void	*val;
int	result = 0;
struct	mkusattr *ptr;
char	*groups;	/* group list from mkuser.default 	*/
char	*finalpgrp;	/* pgrp passed to putuserattr()		*/
register int	siz;	/* size for malloc 			*/
char	*start;		/* start for counter pointers		*/
char	*finish;	/* finish for counter pointers		*/
char	*gptr;		/* pointer into __groupset 		*/

	/*
	 * For each attribute in the mkuser table, get the value
	 * from /usr/lib/security/mkuser.default (if any) and add it
	 * to the user data base.  The TYPE field in getconfattr()
	 * and putuserattr() is 0 since the types are declared in
	 * tables.h of libs.
	 */

	for(ptr = mkusatab; ptr < &mkusatab[mkusatabsiz]; ptr++)
	{
		if (!getconfattr(type,ptr->cattr,(void *)&(ptr->cval),0))
		{
			/* some attributes require validity checking */
			if (ptr->check)
			{
				if (result=(*(ptr->check))(ptr->cval,&val))
				{
					if ((result != EXISTS) && 
							(result != NOEXISTS))
					{
						fprintf(stderr,CHGONERR,
								ptr->cattr);
						fprintf(stderr,".\n");
					}
					fprintf (stderr,CHECK,MKDEF);
					return(1);
				}
			}
			else
				val = (void *)ptr->cval;

			/* put the data in cached memory */
			if(putuserattr(name,ptr->gattr,(void *)val,0))
			{
				fprintf(stderr,CHGTOERR,ptr->cattr,val);
				fprintf(stderr,".\n");
				return(1);
			}
		}
	}

	/*
	 * So far, the user's groupset is his PGRP. 
	 * We need to save the GROUPS= string away
	 * for later processing.  The global variable
	 * will be "__groupset".
	 */
	if (getconfattr(type,S_GROUPS,&groups,SEC_LIST))
		if (errno != ENOATTR && errno != ENOENT)
			return(1);

	if (groups && *groups)
	{
		/*
		 * Set up two pointers to count the
		 * size of the grouplist. 
		 */
		start = finish = groups;
		while (*finish)
		{
			while (*finish++);
		}
		siz = (finish - start) + 1;

		if ((__groupset = (char *)malloc(siz)) == NULL)
		{
			fprintf(stderr,MALLOC);
			exitx(errno);
		}
	
		/* 
	 	 * Set up pointer into __groupset.
	 	 */
		gptr = __groupset;

		/*
	 	 * Load up the storage space with a null-separated
	 	 * list of groups from mkuser.default.
	 	 */
		while (start <= finish)
			*gptr++ = *start++;
	}
	else
	{
		if ((__groupset = (char *)calloc(2,sizeof(char))) == NULL)
		{
			fprintf(stderr,MALLOC);
			exitx(errno);
		}

		gptr = __groupset;
	}

	return(0);
}

/*
 * FUNCTION:	gethomedir
 *
 * DESCRIPTION:	gets the user's home dir if it exists.
 *
 * PASSED:	dir = place to put the directory, name = the user's name,
 *		type = the type of user (admin or ordinary).
 *
 * RETURNS:	None.
 *
 */

static	void
gethomedir(char **dir,char *name,char *type)
{
char		*ptr;
char		*home = NULL;
int		siz;

	/* get default home directory from mkuser.default */
	if (getconfattr(type,SC_HOME,(void *)&home,SEC_CHAR))
	{
		fprintf (stderr,NODEFHOME);
		fprintf (stderr,CHECK,MKDEF);
		exitax(MKUSRAUD,errno,name,NULL,NOPRINT);
	}

	/* allocate space for new directory */
	siz = strlen(home) + strlen(name) + 1;
	if ((*dir = (char *)malloc (siz)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitax(MKUSRAUD,errno,name,NULL,PRINT);
	}

	/* substitute user's name for wildcard $USER value */
	ptr = home;
	while (ptr && *ptr)
	{
		if (*ptr == '$')
		{
			*ptr++ = '\0';
			if(!(strcmp(ptr,"USER")))
			{
				strcpy(*dir,home);
				strcat(*dir,name);
				return;
			}
			else
			{
				*(--ptr) = '$';
				break;
			}
		}
		ptr++;
	}

	/* if no substitution copy in directory name */
	strcpy(*dir,home);
}

/*
 * FUNCTION:	getauthvals
 *
 * DESCRIPTION:	gets the user's AUTH1 and AUTH2 values and convert's
 *		the $USER string into the user name.
 *
 * PASSED:	name = the user's name, type = the type of
 *		user (admin or ordinary).
 *
 * RETURNS:	None.
 *
 */

static	void
getauthvals(char *name, char *type)
{

char	*subptr;			/* substitution pointer for auth     */
char	*startptr;			/* starting pointer of original auth */
char	*authname;			/* work area for AUTH string	     */
char	*auth;				/* original AUTH string		     */	
char	*set[] = {SC_AUTH1,SC_AUTH2};	/* set of strings for getconfattr()  */	
int	ctr;				/* counter			     */


	/*
	 * Get default AUTH values from mkuser.default
	 * if one exists.  It's OK if there's no attribute.
	 * We must turn this list into a comma-separated 
	 * string for manipulation.
	 */
	for (ctr=0;ctr<2;ctr++)
	{
		if (getconfattr(type,set[ctr],(void *)&auth,SEC_LIST))
			if (errno != ENOATTR && errno != ENOENT)
				return;

		/*
		 * Turn the list into a
		 * comma-separated string.
		 */
		listocom(auth);

		/*
		 * If we got something from getconfattr(),
		 * let's check for a $USER in the string.
		 */
		if (auth && *auth)
		{
			startptr = auth;

			/*
			 * If there is no $USER in the sting, then
			 * send the info to putuserattr() and go on
			 * to next "auth" string.
			 */
			if ((subptr = (char *)strstr(startptr,"$USER")) == NULL)
			{
				if (putuserattr(name,set[ctr],(void *)auth,SEC_LIST))
				{
					fprintf(stderr,ERRADD,name);
					exitax(MKUSRAUD,errno,name,NULL,PRINT);
				}
			}
			else
			{
				/*
				 * We have at least one $USER, so let's
				 * process the string and replace all
				 * $USER's with the username.  The call
				 * to calloc() zero's out the memory
				 * locations so that we can strcat()
				 * straight to it.
				 */
				if ((authname = (char *)calloc (LINE_MAX,sizeof(char))) == NULL)
				{
					fprintf(stderr,MALLOC);
					exitax(MKUSRAUD,errno,name,NULL,PRINT);
				}

				/*
				 * Check for $USER, then check the
				 * character after the 'R' in $USER
				 * for delimiters.
				 */
				do
				{
					if(!(strncmp(subptr,"$USER",5)))
					{
						if (*(subptr+5) == ',' || *(subptr+5) == '\0'
						||  *(subptr+5) == ' ' || *(subptr+5) == '\t'
						||  *(subptr+5) == ';' || *(subptr+5) == '$')
						{
							/*
							 * Now we have a $USER, so copy in everything
							 * up to the specified delimiter, and then
							 * strcat() the name onto that.  Then bump
							 * pointer to original string past the "R"
							 * to in $USER so that we get the delimiter
							 * on the next pass, if any.
							 */
							strncat(authname,startptr,(subptr - startptr));
							strcat(authname,name);
							startptr = subptr+5;
							subptr = startptr;
						}
						else
						{
							/*
							 * Copy everything since this is $USER(something),
							 * so go just past the $ sign to restart the strstr().
							 */
							strncat(authname,startptr,(subptr - startptr + 1));
							startptr = subptr += 1;
						}
					}
				} while ((subptr = (char *)strstr(startptr,"$USER")) != NULL);

				/*
				 * Copy the remainder of the auth string
				 * to the cache for later processing.
				 */
				strcat(authname,startptr);

				if (putuserattr(name,set[ctr],(void *)authname,SEC_LIST))
				{
					fprintf(stderr,ERRADD,name);
					exitax(MKUSRAUD,errno,name,NULL,PRINT);
				}
				free (authname);
			}
		}
	}
}

/*
 * FUNCTION:	runremove
 *
 * DESCRIPTION:	removes the user if problems with the initialization routine.
 *
 * PASSED:	name = username to be removed.
 *
 * RETURNS:	Returns exit status of fork.
 *
 */

static	int
runremove(char *name)
{
int	status;
pid_t	pid;

	/* run /usr/bin/rmuser to make the changes */
	if ((pid = fork()) == 0)
	{
		execl(RMUSER,"rmuser","-p",name,0);
		fprintf(stderr,EXECL,RMUSER);
		fprintf (stderr,CHECK,RMUSER);
		exit (EACCES);
	}
	else if (pid == -1)
	{
		fprintf(stderr,EXECL,RMUSER);
		fprintf (stderr,CHECK,RMUSER);
		exit (errno);
	}
	else
	{
		/* get the return code */
		while (waitpid(pid, &status, 0) == -1)
		{
			if (errno != EINTR)
			{
				status = errno;
				break;
			}
		}
	}
	return (WEXITSTATUS(status));
}

/*
 * FUNCTION:	runprog
 *
 * DESCRIPTION:	runs /usr/lib/security/mkuser.sys.
 *
 * RETURNS:	Returns exit status of fork or -1 on getuserattr
 *		failures.
 *
 */

static	int
runprog(char *username)
{
int	status;
pid_t	pid;
char	*groupname;
char	*dir;
char	*shell;

	if (getuserattr (username,S_HOME,&dir,SEC_CHAR))
		return (-1);

	if (getuserattr (username,S_PGRP,&groupname,SEC_CHAR))
		return (-1);

	if (getuserattr (username,S_SHELL,&shell,SEC_CHAR))
		return (-1);

	chdir(dir);

	/* run /usr/lib/security/mkuser.sys to make the changes */
	if ((pid = fork()) == 0)
	{
		execlp(MKSYS,"mksys",dir,username,groupname,shell,0);
		fprintf(stderr,EXECL,MKSYS);
		xaudit(MKUSRAUD,errno,username,NULL);
	}
	else if (pid == -1)
	{
		fprintf(stderr,EXECL,MKSYS);
		xaudit(MKUSRAUD,errno,username,NULL);
	}
	else
	{
		/* get the return code */
		while (waitpid(pid, &status, 0) == -1)
		{
			if (errno != EINTR)
			{
				status = errno;
				break;
			}
		}
	}
	return (WEXITSTATUS(status));
}
