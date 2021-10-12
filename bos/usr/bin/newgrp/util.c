static char sccsid[] = "@(#)72	1.22.1.4  src/bos/usr/bin/newgrp/util.c, cmdsuser, bos41J, 9521B_all 5/26/95 10:02:33";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: checkfortty
 *		comtoarray
 *		dropblanks
 *		exitax
 *		exitx
 *		gotaccess
 *		gotgaccess
 *		gotiaccess
 *		gotuaccess
 *		listlen
 *		listoarray
 *		listocom
 *		onint
 *		strtolist
 *		updpasswd
 *		usage
 *		xaudit
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

#include <stdio.h>		/* for printf				*/
#include <pwd.h>		/* for passwd struct 			*/
#include <grp.h>		/* for group struct 			*/
#include <fcntl.h>		/* for O_RDWR etc. 			*/
#include <ctype.h>		/* for isalnum(),isalpha(),isdigit()	*/
#include <sys/stat.h>		/* for stat call 			*/
#include <sys/priv.h>		/* for privilege			*/
#include <sys/audit.h>		/* for AUDIT_ON, AUDIT_FAIL 		*/
#include <userpw.h>		/* for PW_NAMELEN 			*/
#include <unistd.h>		/* for environ variable 		*/
#include "tcbauth.h"		/* for local defines 			*/

/*
 * FUNCTION:	exitx
 *
 * DESCRIPTION:	if error > 0 prints perror.
 *
 * RETURNS:	None.
 *
 */

void
exitx(int error)
{
	if (error)
	{
		fflush(stderr);
		errno = error;
		perror(" ");
	}
	exit(error);
}

/*
 * FUNCTION:	listocom
 *
 * DESCRIPTION:	turns a NULL separated double NULL terminated list
 *		into a comma separated string.
 *
 * RETURNS:	the comma separated string.
 *
 */

char	*
listocom(char *val)
{
char	*ptr;	/* temporary pointer	*/

	/*
	 *  If val is the NULL pointer or points to
	 *  a zero length string, return val.
	 */
	if (!val || !*val)
		return(val);
	
	ptr = val;
	while (*ptr) 
	{
		while (*ptr)
			ptr++;
		*ptr = ',';
		ptr++;
	}
	ptr--;
	*ptr = '\0';

	return(val);
}


/*
 * FUNCTION:	listoarray
 *
 * DESCRIPTION:	turns a NULL separated double NULL terminated list
 *		to an array of strings.
 *
 * RETURNS:	the array and the number of elements in n.
 *
 */

char	**
listoarray(char *list,int *n)
{
register int	i = 0;		/* a counter		*/
char		*val;		/* temporary pointer	*/
char		**args;
register int	count = 0;	/* a counter		*/
register int	siz;		/* how much space to allocate */

	/* allocate enough for MAXID users */
	if ((args = (char **)malloc(sizeof(char *) * MAXID)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitx(errno);
	}

	val = list;

	/* go through list and convert it to an array */
	while (list && *list)
	{
		list += strlen(list) + 1;

		/* if there is more than MAXID realloc */
		count++;
		if (count == MAXID)
		{
			siz=(((i+1)/MAXID)+1) * sizeof(char *) * MAXID;
			if ((args = (char **)realloc(args,siz))==NULL)
			{
				fprintf(stderr,MALLOC);
				exitx(errno);
			}
			/* reset counter */
			count = 0;
		}
		args[i++] = val;
		val = list;
	}

 	/* set last entry to NULL */
	args[i] = NULL;

	*n = i;
	return(args);
}


/*
 * FUNCTION:	comtoarray
 *
 * DESCRIPTION:	turns a comma separated string into an array of chars.
 *
 * RETURNS:	the array of pointers and the number in n.
 *
 */

char **
comtoarray(char *string,int *n)
{
register int	i = 0;		/* a counter			*/
char		*val;		/* a temporary pointer		*/
register int	count = 0;	/* count of # of users		*/
register int	siz;		/* size of memory to allocate 	*/
char		**args;		/* where array goes 		*/

	/* allocate enough for MAXID users */
	if ((args = (char **)malloc(sizeof(char *) * MAXID)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitx(errno);
	}

	val = string = dropblanks(string);

	while (*string)
	{
		if (*string == ',')
		{
			*string++ = '\0';
			/* if there is more than MAXID realloc */
			count++;
			if (count == MAXID)
			{
				siz=(((i+1)/MAXID)+1) * sizeof(char *) * MAXID;
				if ((args = (char **)realloc(args,siz))==NULL)
				{
					fprintf(stderr,MALLOC);
					exitx(errno);
				}
				count = 0;
			}

			args [i++] = val;
			val = string;
		}
		else
			string++;
		
	}
	count += 2;
	/* if there is more than MAXID realloc */
	if (count >= MAXID)
	{
		/* just add space for two more pointers */
		siz=((i+2) * sizeof(char *));
		if ((args = (char **)realloc(args,siz))==NULL)
		{
			fprintf(stderr,MALLOC);
			exitx(errno);
		}
	}

	args [i++] = val;
	args [i] = NULL;


	*n = i;
	return(args);
}

/*
 * FUNCTION:	gotaccess
 *
 * DESCRIPTION:	checks for root.
 *
 * RETURNS:	true or false.
 *
 */

int
gotaccess(void)
{
uid_t	id;	/* the invoker's id	*/

	id = getuid();

	if (id == 0)
		return(1);
	else
		return(0);
}

/*
 * FUNCTION:	xaudit
 *
 * DESCRIPTION:	reacquires privilege and cuts an audit record.
 *
 * PASSED:	the audit event name, the result of the operation 
 *		(usually errno), the name added, and the attributes
 *
 * RETURNS:	None.
 *
 * PASSED:
 *
 *	char	*type;		 the audit event name
 *	int	error;		 error value usually errno
 *	char	*name;		 the group or user name effected
 *	char	*buf;		 the values changed
 *
 */

void
xaudit (char *type,int error,char *name,char *buf)
{
int	result = 0;	/* place to hold result flag */

	/* pickup audit privilege */

	privilege(PRIV_ACQUIRE);

	/* set success or fail */
	if (error)
		result |= AUDIT_FAIL;
	else
		result |= AUDIT_OK;

	auditwrite(type,result,name,strlen(name) + 1, buf,strlen(buf) + 1,0);

	privilege(PRIV_DROP);
	exit(error);
}

/*
 *
 * FUNCTION:	exitax
 *
 * DESCRIPTION:	cuts audit record, exits with error.
 *
 * RETURNS:	No return.
 *
 * PASSED:
 *	char	*type;		 the audit event name
 *	int	result;		 error value usually errno
 *	char	*name;		 the group or user name effected
 *	char	*attrs;		 the values changed
 */

void
exitax(char *type,int result,char *name,char *attrs,int pflag)
{

	if (result && pflag)
	{
		fflush(stderr);
		switch (result)
		{
			case EPERM : fprintf (stderr, PERMISSION);
					break;

			case EINVAL : fprintf (stderr, ERBADVAL);
					break;

			case EEXIST : fprintf (stderr, ACCEXIST);
					break;

			case EXISTS : result = EEXIST;
					break;

			case NOEXISTS : result = ENOENT;
					break;

			case ESAD : 	break;

			case ENAMETOOLONG : fprintf (stderr, TOOLONG);
					break;

			case ENOMEM : fprintf (stderr, NOMEM);
					break;

			case ENOTTY : fprintf (stderr, ERNOTERM);
					break;

			case BADATTR : fprintf (stderr, ERBADATTR);
					result = EINVAL;
					break;

			case BADVALUE : fprintf (stderr, ERBADVAL);
					result = EINVAL;
					break;

			default : fprintf (stderr, ".\n");
		}
	}

	else
	{
		if (pflag)
			fprintf (stderr, ".\n");
	}

	xaudit(type,result,name,attrs);
}

/*
 * FUNCTION:	usage
 *
 * DESCRIPTION:	prints usage.
 *
 * RETURNS:	No return.
 */

void
usage(char *program)
{
	fprintf(stderr,program);
	exit(EINVAL);
}

/*
 * FUNCTION:	strtolist
 *
 * DESCRIPTION:	turns a char string to a list.
 *
 * RETURNS:	pointer to the value.
 *
 */

int
strtolist(char *val,char **ret)
{
char		*ptr;	/* temporary pointer	*/

	if ((*ret = (char *)malloc(strlen(val) + 2)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitx(errno);
	}

	strcpy(*ret,val);

	ptr = *ret;
	
	while (*ptr)
	{
		if (*ptr == ',')
			*ptr++ = '\0';
		else
			ptr++;
	}
	ptr++;
	*ptr = '\0';
	return(0);
}

/*
 * FUNCTION:	checkfortty
 *
 * DESCRIPTION:	checks for execution from a tty.
 *
 * PASSED:	none.
 *
 * RETURNS:	returns error.	
 */

int
checkfortty(void)
{
register int	err = 0;	/* flag */

	if (!isatty(0)) 
		err++;
	if (!isatty(1)) 
		err++;
	if (!isatty(2)) 
		err++;

	return(err);
}

/*
 * FUNCTION:	gotgaccess
 *
 * DESCRIPTION:	checks if the group is administrative & if the invoker has 
 *		access
 *
 * RETURNS:	true or false.
 *
 */

int
gotgaccess(char *groupname)
{
int	adm=0;		/* flag indicating whether this group is an */
			/* administrative group or not		    */

	/*
	 * see if this is an administrative group
	 */
	if (getgroupattr(groupname,S_ADMIN,&adm,SEC_BOOL) == -1)
	{
		if (errno == ENOATTR || errno == ENOENT)
			return (1);
		return(0);
	}

	if (adm)
	{
		/*
		 * only root can change an administrative group
		 */
		if (gotaccess())
			return(1);
		else
			return(0);

	}
	return(1);
}

/*
 * FUNCTION:	gotuaccess
 *
 * DESCRIPTION:	checks if the user is an administrative user & if the invoker
 *		has access to change the user
 *
 * RETURNS:	true or false.
 *
 */

int
gotuaccess(char *username)
{
int	adm=0;		/* flag indicating whether this user is an */
			/* administrative user or not    	   */

	/*
	 * see if this is an administrative user
	 */
	if (getuserattr(username,S_ADMIN,&adm,SEC_BOOL))
	{
		if (errno == ENOATTR || errno == ENOENT)
			return (1);
		return(0);
	}

	/* if the user is administrative, check the access */
	if (adm)
	{
		/*
		 * only root can change an administrative user
		 */
		if (gotaccess())
			return(1);
		else
			return(0);

	}
	return(1);
}


/*
 * FUNCTION:	gotiaccess
 *
 * DESCRIPTION:	checks if the invoker is the user being changed; if not,
 *		the invoker must be root or in group security.
 *
 * RETURNS:	true or false.
 *
 */

int
gotiaccess(char *username)
{
uid_t	id;			/* invoker's real user id   */
gid_t	gid;			/* invoker's real group id  */
register char	*group;		/* invoker's real groupname */
register char	*user;		/* invoker's real username  */
char	**groups;		/* process group set	    */
char	*pointer;		/* temporary pointer	    */
int	n;			/* number of groups	    */
int	i;			/* counter		    */
	/*
	 * if invoker = username --> ok
	 * if uid = 0 --> ok
	 * if group = security --> ok
	 * else --> fail
	 */

	id = getuid();

	if (!id)
		return (1);

	if ((user = IDtouser (id)) == NULL)
		return (0);

	if (strcmp(user,username))
	{
		/*
		 * If we're trying to change someone
		 * else, and we're not root, then get
		 * the process groupset, strip off 
		 * header (GROUPS=), and check for
		 * group "security".  If we've gone
		 * through the whole list and haven't
		 * found "security", then return false.
		 */
		if ((groups = getpcred(CRED_GROUPS)) == NULL) 
			return (0);
		
		if ((pointer = strchr(*groups,'=')) == NULL)
			return (0);

		pointer++;

		groups = comtoarray (pointer,&n);
		
		for (i=0;i<n;i++)
			if (!strcmp(SECURITY,groups[i]))
				return (1);

		return (0);
	}
	return (1);
}

/*
 *
 * FUNCTION:	updpasswd
 *
 * DESCRIPTION:	puts a "!" in /etc/passwd.
 *
 * RETURNS:	0 or -1.
 *
 */

int
updpasswd(char *name)
{
	/* Make changes on the /etc/passwd file */
	if (putuserattr(name,S_PWD,"!",SEC_CHAR))
		return (-1);

	if (putuserattr(name,NULL,NULL,SEC_COMMIT))
		return (-1);

	return (0);
}

/*
 * FUNCTION:    listlen
 *
 * DESCRIPTION: returns number of items in a comma separated list
 *
 * RETURNS:     Number of items in the list
 *
 */

int
listlen(char *list)
{
	char *p = list;
	int  num;

	/*
	 * If list is a null pointer or a
	 * null string, return 0.
	 */
	if (!list || !*list)
		return(0);

	/* Count first item */
	num = 1;

	while (*p)
	{   
      		if (*p==',')
			num++;
		p++;
	}
	return(num);
}

/*
 * NAME: 	onint()
 *                                                                    
 * FUNCTION: 	signal catching routine.
 *                                                                    
 * RETURNS: 	none
 */  

void
onint(void)
{
	fprintf (stderr,GOTINT);
	exit (EINTR); 
}

/*
 * FUNCTION:	dropblanks
 *
 * DESCRIPTION:	removes all spaces and tabs from the string passed in.
 *
 * RETURNS:	newstrings
 *
 */

char *
dropblanks(char *orig)
{
char 	*newstring;
char 	*ret;

	if ((ret = newstring = malloc(strlen(orig) + 2)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitx(errno);
	}

	while (*orig)
	{
		if (*orig == ' ' || *orig == '\t')
			*orig++;
		else
			*newstring++ = *orig++;
	}
	*newstring = *orig;

	return (ret);
}
