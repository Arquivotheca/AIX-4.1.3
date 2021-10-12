static char sccsid[] = "@(#)99	1.8.1.17  src/bos/usr/bin/newgrp/chk_util.c, cmdsuser, bos41J, 9515B_all 4/13/95 08:47:46";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: chgbool
 *		chglist
 *		chkaccess
 *		chkadmgroup
 *		chkadmgroups
 *		chkaud
 *		chkaudit
 *		chkauthmethod
 *		chkauthsystem
 *		chkbool
 *		chkdata
 *		chkdictionlist
 *		chkexpires
 *		chkgek
 *		chkgid
 *		chkgrp
 *		chkgrps
 *		chkhistexpire
 *		chkhistsize
 *		chkhome
 *		chkint
 *		chkmaxage
 *		chkmaxexpired
 *		chkmaxrepeats
 *		chkmgrps
 *		chkminalpha
 *		chkmindiff
 *		chkminlen
 *		chkminother
 *		chkmkadmgroups
 *		chkmkdata
 *		chkmkhistexpire
 *		chkmkhistsize
 *		chkmkmaxage
 *		chkmkmaxexpired
 *		chkmkmaxrepeats
 *		chkmkminage
 *		chkmkminalpha
 *		chkmkmindiff
 *		chkmkminlen
 *		chkmkminother
 *		chkmkstack
 *		chkmkulimit
 *		chkmkumask
 *		chknames
 *		chkparsetree
 *		chkpgrp
 *		chkprog
 *		chkpwdchecks
 *		chkregistry
 *		chkstack
 *		chktpath
 *		chkttys
 *		chkuid
 *		chkulimit
 *		chkumask
 *		chkuserid
 *		chkvalues
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *   OBJECT CODE ONLY SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/stat.h>
#include <sys/access.h>
#include <string.h>
#include <userpw.h>
#include <grp.h>
#include <pwd.h>
#include "tcbauth.h"

static	int	chkvalues(char *,char **,int (*)(),char *,int);
static	int 	chkuserid(char *);

/*
 * FUNCTION:	chkgid
 *
 * DESCRIPTION:	Make sure gid is valid number, invoker has access to
 *		make an update, and that the group does not exist.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkgid(char *val,gid_t *ret)
{
unsigned long	nid;		/* place to hold id as i long	*/
char	*id = NULL;		/* error return from strtol	*/
int		save = 0;	/* save the errno from strtol	*/
struct	group *grp;		/* struct from getgruid()	*/

	/*
	 * GID values are technically (unsigned long), but traditionally
	 * have been both signed and unsigned.  So first we try to convert
	 * to a signed value, then see if it is unsigned after the
	 * conversion fails.
	 */

	nid = (ulong) strtol(val,&id,10);
	save = errno;

	errno = 0;
	nid = strtol(val,&id,10);
	if (errno || *id) {
		errno = 0;
		nid = strtoul (val, &id, 10);
		if (errno || *id)
			return (EINVAL);
	}

	/*
	 * cannot change admin ids
	 */
	if ((nid < 200) && (!gotaccess()))
		return (EPERM);

	/*
	 * check if group exists
	 */
	set_getgrent_remote(2);	/* compat lookups only	*/
	grp = getgrgid(nid);
	set_getgrent_remote(1);	/* resume full lookups	*/
	if (grp != (struct group *)NULL)
		return(EEXIST);
	
	/*
	 *  If the number is out of range,
	 *  then return EINVAL.
	 */
	if (save == ERANGE)
		return (EINVAL);

	*ret = nid;

	return(0);
}

/*
 * FUNCTION:	chkbool
 *
 * DESCRIPTION:	check the "attr=value" attribute for value 
 *		equal true or false.
 *
 * RETURNS:	0 or error.
 *
 */
int
chkbool(char *val,char **ret)
{
	char	buf[MAXBOOL];
	int	i;
	char	*vp = val;

	/*
	 * Check the current language equivalent for yes/no.  If this fails
	 * then check all the possible English equivalents for yes/no.
	 */
	if (rpmatch(val) != -1)
	{
		*ret = val;
		return(0);
	}

	for (i = 0; *vp && (i < MAXBOOL-1); i++)
	{
		buf[i] = (isupper ((int)*vp)) ? tolower((int)*vp) : *vp;
		vp++;
	}
	buf[i] = '\0';

	if (!strcmp(buf,_TRUE)   || !strcmp(buf,_FALSE)  || 
	    !strcmp(buf,YES)     || !strcmp(buf,NO)      || 
	    !strcmp(buf,ALWAYS)  || !strcmp(buf,NEVER))
	{
		*ret = val;
		return(0);
	}
	return(EINVAL);
}

/*
 * FUNCTION:	chktpath
 *
 * DESCRIPTION:	check the "attr=value" attribute for values
 *		nosak,always,notsh,on.
 *
 * RETURNS:	0 or error.
 *
 */

int
chktpath(char *val,char **ret)
{
	*ret = val;
	if (!strcmp(val,NOSAK) || !strcmp(val,ALWAYS) ||
	    !strcmp(val,NOTSH) || !strcmp(val,ON))
		return(0);

	return(EINVAL);
}

/*
 * FUNCTION:	chkuserid
 *
 * DESCRIPTION:	adjusts the return value of getuserattr() to
 *		correspond to the chknames() routine's
 *		expected values.
 *
 * RETURNS:	0 if user exists..
 *
 */

static int
chkuserid(char *user)
{
int id;
	if (getuserattr(user,S_ID,&id,SEC_INT))
		return(1);
	else
		return(0);
}

/*
 * FUNCTION:	chknames
 *
 * DESCRIPTION:	checks the "adms=value" for valid usernames.
 *
 * RETURNS:	0 or error.
 *
 */

int
chknames(char *val, char **ret)
{
	return(chkvalues(val, ret, chkuserid, USRNONEX, NOEXISTS));
}

/*
 * FUNCTION:	chgbool
 *
 * DESCRIPTION:	changes a 1 to true or 0 to false.
 *
 * RETURNS:	true, false or NULL.
 *
 */

char	* 
chgbool(char *name,char *val)
{
	return (val ? _TRUE : _FALSE);
}

/*
 * FUNCTION:	chglist
 *
 * DESCRIPTION:	makes a call to listocom().  needed for
 *		table-driven data.
 *
 * RETURNS:	comma separated list.
 *
 */

char	* 
chglist(char *name,char *val)
{
	return(listocom(val));
}

/*
 * FUNCTION:	chkaudit
 *
 * DESCRIPTION:	checks on S_AUDITCLASSES attribute.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkaudit(char *val, char **ret)
{
	return(chkvalues(val, ret, chkaud, NULL, EINVAL));
}

/*
 * FUNCTION:	chkaud
 *
 * DESCRIPTION:	checks to see if "auditclasses" attribute is
 *		a valid value.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkaud(char *val,char **ret)
{
char	*attr;	/* the return from getconfattr	*/

	*ret = val;
	if (!strcmp(val,ALL))
		return(0);
		
	if (getconfattr(SC_SYS_AUDIT,val,&attr,SEC_LIST)) 
	{
		fprintf (stderr,CHECK,AUDFILE);
		return(EINVAL);
	}

	return(0);
}
		
/*
 * FUNCTION:	chkprog
 *
 * DESCRIPTION:	checks to see if "shell" value is a valid shell,
 *		then, validate the shell.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkprog(char *val,char **ret)
{
register int	i = 0;		/* counter 			*/
int		n = 0;		/* counter 			*/
char		*value;		/* return from getconfattr 	*/
char		**shells;	/* array of available shells	*/
struct  stat	buffer;		/* buffer for stat of shell	*/
	
	if(getconfattr(SC_SYS_LOGIN,SC_SHELLS,&value,SEC_LIST))
	{
		fprintf (stderr,CHECK,LOGINFILE);
		return(EINVAL);
	}

	shells = listoarray(value,&n);

	for (i=0;i<n;i++)
	{
		/*
		 * check the entire list of shells, only one stat
		 * needs to be done...after the shell is validated
		 */

		if (!strcmp(val,shells[i]))
		{
			if (!stat(val, &buffer))
			{
				*ret = val;
				return(0);
			}

		}
	}
	fprintf (stderr,CHECK,LOGINFILE);
	return(EINVAL);
}

/*
 * FUNCTION:	chkuid
 *
 * DESCRIPTION:	Make sure we have a valid id, that we have access to
 * 		make updates, and that the id doesn't already exist.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkuid(char *val,uid_t *ret)
{
unsigned long	nid;		/* place to hold id as i long	*/
char		*id = '\0';	/* the error from strtol	*/
int		save = 0;	/* save the errno from strtol	*/
struct		passwd *pwd;	/* returned from getpwuid	*/

	/*
	 * UID values are technically (unsigned long), but traditionally
	 * have been both signed and unsigned.  So first we try to convert
	 * to a signed value, then see if it is unsigned after the
	 * conversion fails.
	 */

	errno = 0;
	nid = strtol(val,&id,10);
	if (errno || *id) {
		errno = 0;
		nid = strtoul (val, &id, 10);
		if (errno || *id)
			return (EINVAL);
	}

	/*
	 * cannot change admin ids
	 */
	if ((nid < 200) && !gotaccess())
		return(EPERM);

	/*
	 * check if user exists
	 */
	set_getpwent_remote(2);	/* compat lookups only	*/
	pwd = getpwuid(nid);
	set_getpwent_remote(1);	/* resume full lookups	*/
	if (pwd != (struct passwd *)NULL)
		return(EEXIST);

	/*
	 *  If the number is out of range,
	 *  then return EINVAL.
	 */
	if (save == ERANGE)
		return (EINVAL);

	*ret = nid;

	return(0);
}

/*
 * FUNCTION:	chkgrp
 *
 * DESCRIPTION:	checks group for validity.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkgrp(char *val,char **ret)
{
int	id;
struct	group *grp;

	*ret = val;

	if (!strcmp(val,ALL) || !strcmp(val,STAR))
		return(0);

	if (*val == '!')
		val++;

	set_getgrent_remote(2);	/* compat lookups only */
	grp = getgrnam(val);
	set_getgrent_remote(1);	/* resume full lookups */
	if (grp == (struct group *)NULL)
		return(EINVAL);

	return(0);
}

/*
 * FUNCTION:	chkpgrp
 *
 * DESCRIPTION:	checks the primary group.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkpgrp(char *val,char **ret)
{
int	id;

	*ret = val;

	if (getgroupattr (val,S_ID,&id,SEC_INT))
		return (EINVAL);

	return (0);
}

/*
 * FUNCTION: 	chkhome
 *
 * DESCRIPTION: checks the users login directory for illegal characters
 *
 * RETURNS:	0 or error.
 */

int
chkhome (char *val, char **ret)
{

	*ret = val;
	/*
	 * Scan through val and look for illegal characters.  Stop
	 * on the first illegal character and return EINVAL.
	 */

	for (;*val;val++)
		if (*val == ':' || *val == '\n' ||
		    *val == ' ' || *val == '\t')
			break;

	if (*val) 
		return EINVAL;

	return 0;
}

/*
 * FUNCTION:	chkadmgroup
 *
 * DESCRIPTION:	checks admgroups list for validity.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkadmgroup(char *val,char **ret)
{
int	groupid;

	*ret = val;

	if (*val == '!' || !strcmp(val,ALL) || !strcmp(val,STAR))
		return(EINVAL);

	if (getgroupattr(val,S_ID,&groupid,SEC_INT))
		return(EINVAL);

	return(0);
}

/*
 * FUNCTION:	chkmkadmgroups
 *
 * DESCRIPTION:	checks the group list of admgroups
 *		when creating users.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkmkadmgroups(char *val,char **ret)
{
register char	*group;	/* pointer to groupname  */
char		*ptr;	/* pointer to val string */
char		*temp;	/* temporary pointer     */

	/*
	 * save the start of val string
	 */

	group = ptr = *ret = val;

	while (ptr && *ptr)
	{
		ptr += strlen(ptr);

		if (group && *group)
		{
			if(chkadmgroup(group,&temp))
			{
				fprintf(stderr,GRPNONEX,group);
				return(NOEXISTS);
			}
		}
		ptr++;
		group = ptr;
	}

	return(0);
}

/*
 * FUNCTION:	chkadmgroups
 *
 * DESCRIPTION:	checks the group list of admgroups
 *		when changing users.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkadmgroups(char *val, char **ret)
{
	return(chkvalues(val, ret, chkadmgroup, GRPNONEX, NOEXISTS));
}

/*
 * FUNCTION:	chkmgrps
 *
 * DESCRIPTION:	checks the group list of admgroups, sugroups, groups.
 *		when creating users.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkmgrps(char *val,char **ret)
{
register char	*group;	/* pointer to groupname  */
char		*ptr;	/* pointer to val string */
char		*temp;	/* temporary pointer     */

	/*
	 * save the start of val string
	 */

	group = ptr = *ret = val;

	while (ptr && *ptr)
	{
		ptr += strlen(ptr);

		if (group && *group)
		{
			if(chkgrp(group,&temp))
			{
				fprintf(stderr,GRPNONEX,group);
				return(NOEXISTS);
			}
		}
		ptr++;
		group = ptr;
	}

	return(0);
}

/*
 * FUNCTION:	chkgrps
 *
 * DESCRIPTION:	checks the group list of admgroups, sugroups, groups.
 *		when changing users.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkgrps(char *val, char **ret)
{
	return(chkvalues(val, ret, chkgrp, GRPNONEX, NOEXISTS));
}

/*
 * FUNCTION:	chkvalues
 *
 * DESCRIPTION:	checks the attr=value for groups, users, and auditclasses.
 *
 * RETURNS:	0 or error.
 *
 */

static int
chkvalues(char *val,char **ret,int (*checker)(),char *msg,int badrc)
{
register char	*attr;	/* temporary pointer 		*/
char		*ptr;	/* temporary pointer 		*/
char		*temp;	/* temporary pointer 		*/

	if (!val || !*val)
	{
		**ret = '\0';
		return(0);
	}

	/*
	 * drop spaces and tabs from our command line args
	 * No need to check return since dropblanks exits 
	 * on fail.
	 */

	*ret = attr = ptr = dropblanks(val);

	while(*ptr)
	{
		if (*ptr == ',')
		{
			*ptr++ = '\0';
			/*
		 	 * make sure the value is valid.
		 	 */
			if (*attr)
			{
				if ((*checker)(attr,&temp))
				{
					if(msg)
						fprintf(stderr,msg,attr);
					return(badrc);
				}
			}

			attr = ptr;
		}
		else
			ptr++;
	}
	ptr++;
	*ptr = '\0';

	if ((*checker)(attr,&temp))
	{
		if(msg)
			fprintf(stderr,msg,attr);
		return(badrc);
	}

	return(0);
}

/*
 * FUNCTION:	chkmkumask
 *
 * DESCRIPTION:	checks for valid umask when creating user.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkmkumask(char *val,unsigned long *ret)
{

unsigned long	remainder;
unsigned long	number;

	number = *ret = (unsigned long)val;
	if (*ret > 777)
		return (EINVAL);

	while (number)
	{
		remainder = number % 10;
		number = number / 10;

		if (remainder > 7)
			return (EINVAL);
	}

	return(0);
}

/*
 * FUNCTION:	chkumask
 *
 * DESCRIPTION:	checks for valid value for umask when changing user.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkumask(char *val,unsigned long *ret)
{
int	err;	/* error returned from chkint */

	if (err = chkint(val,ret))
		return (err);

	return(chkmkumask(*ret,&err));
}

/*
 * FUNCTION:	chkdata
 *
 * DESCRIPTION:	checks for long value, checks for value in the
 *		range MIN_DATA <= value <= MAX_DATA.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkdata(char *val,unsigned long *ret)
{
register int	err;	/* error returned from chkint */

	if (err = chkint(val,ret))
		return (err);

	if (! (MIN_DATA <= *ret && *ret <= MAX_DATA))
		if (*ret != (unsigned long) 0 && *ret != (unsigned long) -1 )
			return(EINVAL);

	return (0);
}

/*
 * FUNCTION:	chkstack
 *
 * DESCRIPTION:	checks for long value, checks for value in the
 *		range MIN_STACK <= value <= MAX_STACK.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkstack(char *val,unsigned long *ret)
{
register int	err;	/* error returned from chkint */

	if (err = chkint(val,ret))
		return (err);

	if (! (MIN_STACK <= *ret && *ret <= MAX_STACK))
		if (*ret != (unsigned long) 0 && *ret != (unsigned long) -1 )
			return(EINVAL);

	return (0);
}

/*
 * FUNCTION:	chkulimit
 *
 * DESCRIPTION:	checks for long value, checks for value in the
 *		range MIN_FSIZE <= value <= MAX_FSIZE when changing
 *		user attribute.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkulimit(char *val,unsigned long *ret)
{
register int	err;	/* error returned from chkint */

	if (err = chkint(val,ret))
		return (err);

	if (! (MIN_FSIZE <= *ret && *ret <= MAX_FSIZE))
		if (*ret != (unsigned long) 0 && *ret != (unsigned long) -1 )
			return(EINVAL);

	return (0);
}

/*
 * FUNCTION:	chkmkulimit
 *
 * DESCRIPTION:	checks for long value, checks for value in the
 *		range MIN_FSIZE <= value <= MAX_FSIZE when creating
 *		user attribute.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkmkulimit(char *val,unsigned long *ret)
{
	*ret = (unsigned long)val;

	if (! (MIN_FSIZE <= *ret && *ret <= MAX_FSIZE))
		if (*ret != (unsigned long) 0 && *ret != (unsigned long) -1 )
			return(EINVAL);

	return (0);
}

/*
 * FUNCTION:	chkmkstack
 *
 * DESCRIPTION:	checks for long value, checks for value in the
 *		range MIN_STACK <= value <= MAX_STACK.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkmkstack(char *val,unsigned long *ret)
{
	*ret = (unsigned long)val;

	if (! (MIN_STACK <= *ret && *ret <= MAX_STACK))
		if (*ret != (unsigned long) 0 && *ret != (unsigned long) -1 )
			return(EINVAL);

	return (0);
}

/*
 * FUNCTION:	chkmkdata
 *
 * DESCRIPTION:	checks for long value, checks for value in the
 *		range MIN_DATA <= value <= MAX_DATA.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkmkdata(char *val,unsigned long *ret)
{
	*ret = (unsigned long)val;

	if (! (MIN_DATA <= *ret && *ret <= MAX_DATA))
		if (*ret != (unsigned long) 0 && *ret != (unsigned long) -1 )
			return(EINVAL);

	return (0);
}

/*
 * FUNCTION:	chkint
 *
 * DESCRIPTION:	checks for long value.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkint(char *val,unsigned long *ret)
{
unsigned long	nid;	/* the value returned from strtol	*/
char		*id;	/* the value returned from strtol	*/

	nid = (ulong) strtol(val,&id,10);

	/*
	 *  If the number is out of range,
	 *  then return EINVAL.
	 */
	if (errno == ERANGE)
		return (EINVAL);

	if ((id != val) && (*id == NULL))
	{
		*ret = nid;
		return(0);
	}
	return(EINVAL);
}

/*
 * FUNCTION:	chkexpires
 *
 * DESCRIPTION:	checks for int values in string.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkexpires(char *val,char **ret)
{
char	*ptr;
int	month;
int	day;
int	hour;
int	min;
int	year;

	/* 
	 * Two checks should be done here:
	 *	1)  the value is 0, or
	 *	2)  the value must be 10 characters
	 *		to pass through the date 
	 *		checks below
	 */
	if (!strcmp(val,"0"))
	{
		*ret = val;
		return(0);
	}

	if (strlen(val) != 10)
		return(EINVAL);

	ptr = *ret = val;

	while (ptr && *ptr)
	{
		if (!isdigit(*ptr))
			return(EINVAL);
		ptr++;
	}
	
	sscanf(val,"%2d%2d%2d%2d%2d", &month,&day,&hour,&min,&year);
	if (month > 12 || month < 1)
		return(EINVAL);
	if (day   > 31 || day   < 1)
		return(EINVAL);
	if (hour  > 23 || hour  < 0)
		return(EINVAL);
	if (min   > 59 || min   < 0)
		return(EINVAL);
	
	return(0);
}


/*
 * FUNCTION:	chkgek
 *
 * DESCRIPTION:	check the gecos field for invalid characters.
 *
 * RETURNS:	0 or error.
 *
 */

int
chkgek(char *val,char **ret)
{
char	*ptr;	/* temporary pointer	*/


	/*
	 *  It's OK to have a NULL gecos field,
	 *  so no need to check for that case.
	 */

	ptr = val;
	while (*ptr)
	{
		if((*ptr == ':') || (*ptr == '\n'))
			return(EINVAL);
		ptr++;
	}
	*ret = val;
	return(0);
}

/*
 * FUNCTION:	chkttys
 *
 * DESCRIPTION:	check the ttys field for invalid entries
 *
 * RETURNS:	0 or error.
 *
 */

int
chkttys (char *val, char **ret)
{
	char	*cp, *ccp;
	char	*ptr;
	char	*special = " \t\n,";
	int	len;

	if (!val || !*val)
	{
		**ret = '\0';
		return(0);
	}

	/*
	 * Make a copy of the TTYs list to work with.  Each
	 * entry will be copied into it and NULL separated.
	 * The entire entry is double NULL terminated.
	 */

	if ((cp = (char *)malloc(len = strlen(val) + 4)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitx(errno);
	}

	/*
	 * Copy the string, a character at a time, creating a new
	 * NULL terminated string for each non-white-space, non-comma
	 * string.
	 */

	for (ccp = cp, ptr = val;*ptr;) {
		while (*ptr && strchr (special, *ptr) == 0)
			*ccp++ = *ptr++;

		*ccp++ = '\0';

		while (*ptr && strchr (special, *ptr) != 0)
			ptr++;
	}

	/*
	 * Add the final NULL character to double NULL terminate the
	 * string.
	 */

	*ccp = '\0';

	/*
	 * Now let's go check and see what we got.  The valid strings
	 * are the word "ALL" in uppercase letters, "R" followed by anything,
	 * and any string beginning with "/".  R-cmds and /-files may be
	 * preceeded by "!" to indicate negation.
	 */

	for (ccp = cp;*ccp;ccp += strlen (ccp) + 1) {
		if (! strcmp (ccp, ALL) || ! strcmp (ccp, STAR))
			continue;

		if (ccp[0] == '!')
			ccp++;

		if (ccp[0] == '/' || ccp[0] == 'R')
			continue;

		free (cp);
		*ret = (char *) 0;
		return EINVAL;
	}

	/*
	 * Pass back a pointer to the re-arranged strings and
	 * return SUCCESS.
	 */

	*ret = cp;
	return 0;
}

/*
 * FUNCTION:	chkaccess
 *
 * DESCRIPTION:	checks to see if the invoker has access to change the
 *		admgroups, pgrp, group, or admin attributes
 *
 * PASSED:	attr=attribute name, value=attribute value, user=user name.
 *
 * RETURNS:	true or false
 *
 */

int
chkaccess(char *attr,char *value,char *user)
{
register int	i;	/* loop counter			      */
int	num;		/* num of grps in the attribute value */
char	**grps;		/* array of group attribute values    */
char	*val;

	/* Only root can change the admin attribute. */
	if (!strcmp(attr,S_ADMIN))
		if(!gotaccess())
			return(0);	
		else
			return(1);
				
	/*
	 * Only root can change the pgrp, groups, & admgroups attributes
	 * if the attribute values are administrative groups.
 	 */ 
	if (!strcmp(attr,S_PGRP))
		if(!gotgaccess(value))
			return(0);
		else
			return(1);

	if (!strcmp(attr,S_GROUPS) || !strcmp(attr,S_ADMGROUPS))
	{
		if (value && *value)
		{
			if ((val = malloc(strlen(value)+1)) == NULL)
			{
				fprintf (stderr, MALLOC);
 				exitax(CHUSRAUD,errno,user,(char *)NULL,PRINT);
			}
			strcpy(val,value);
			grps = comtoarray(val,&num);
			for (i=0;i<num;i++)
			{
				if(!gotgaccess(grps[i]))
					return(0);
			}
			free (val);
		}
	}
	return(1);
}

/*
 * NAME: chkregistry
 *
 * FUNCTION: validates the supplied variable to see if it is a well
 * 	     known or user defined administration domain.   The "registry"
 *	     variable is used to define where the user is administered.  It
 *	     is used whenever there is a possibility of resolving the user
 * 	     to other domains due to services being down or databases being
 * 	     replicated locally.
 *
 * RETURNS:  0 or error
 * 
 * ERRNO:    EINVAL - Invalid registry
 */
int
chkregistry(char *val, char **ret)
{
	int rc;

	if ((rc = chkauthmethod(val, ret)) != 0)
		return(rc);

	/*
	 * AUTH_COMPAT isn't a valid registry since it is a name resolution
	 * token that signals the getpw* and getgr* routines to follow normal
	 * resolutions through local and NIS databases.
 	 * AUTH_NONE_SEC makes no sense as a registry either since this user
	 * has to be administered somewhere.
	 */
	if (ISCOMPAT(*ret))
		return(EINVAL);
	if (ISNONE(*ret))
		return(EINVAL);

	return(rc);
}

/*
 * NAME: chkauthmethod
 * 
 * FUNCTION: validates the supplied variable to see if it is a well 
 *	     known method, such as "NIS", "NONE", "compat" or "files"
 *	     or possibly another method defined in login.cfg
 * 
 * Returns:  0 or error
 *
 * ERRNO:    EINVAL - Invalid method name
 * 	     ENOMEM - Ran out of memory
 */
int
chkauthmethod(char *val, char **ret)
{
	char *method;		/* User defined authentication method   */
	char *vp;		/* "val" variable pointer		*/
	int  i;			/* For loop counter			*/
	int  rc = EINVAL;	/* return code				*/

	static struct token	/* Well known methods and their capital */
	{			/* forms.				*/
		char 	*CAPNAME;
		char	*name;
	} *tptr, tokens[]   =	{ 	"FILES",	AUTH_FILES,
			  		"NIS",		AUTH_NIS,
			  		"COMPAT",	AUTH_COMPAT,
			  		"NONE",		AUTH_NONE_SEC,
					(char *)NULL,	(char *)NULL
				};

	*ret = val;

	/*
	 * First attempt to retrieve a method name from the database to
	 * see whether it is indeed a defined authentication domain.
 	 */
        if (!getconfattr (val, SC_AUTHPROGRAM, (void *)&method, SEC_CHAR))
	{	
		if (!accessx(method, X_OK, ACC_SELF))
			return(0);
	}

	/*
 	 * If this fails then there is one standard directory where all
	 * methods should be residing.
	 */
	if ((method = (char *)malloc(strlen(AUTH_DEFPATH) +
			strlen(val) + 1)) == (char *)NULL) return(ENOMEM);

	strcpy(method, AUTH_DEFPATH);
	strcat(method, val);
	if (!accessx(method, X_OK, ACC_SELF))
	{	
		free(method);
		return(0);
	}
	free(method);


	if ((vp = (char *)malloc(strlen(val) + 1)) == (char *)NULL)
		return(ENOMEM);

	/* 
	 * Switch to capitalized versions of the tokens, for their check 
	 */
	for (i = 0; val && *val; i++, val++)
		vp[i] = islower(*val) ? toupper(*val) : *val;

	vp[i] = (char)NULL;
	
	for (tptr = tokens; tptr->name; tptr++)
	{
		if (!strcmp(tptr->CAPNAME, vp))
		{
			if ((*ret = strdup(tptr->name)) == (char *)NULL)
				rc = ENOMEM;
			else
				rc = 0;
			break;
		}
	}
	free(vp);

	if (rc)
		*ret = (char *)NULL;

	return(rc);
}

/*
 * NAME: chkauthsystem
 * 
 * FUNCTION: Validates the supplied authentication grammar.  This routine
 *	     takes the grammar and ensures it's correctness by creating
 *	     a parse tree.  
 * 
 * RETURNS:  0 or error
 * 
 * ERRNO:    EINVAL - Invalid parse tree
 */
int
chkauthsystem(char *val, char **ret)
{
	struct secgrammar_tree *tptr;
	int    rc = 0;

	*ret = val;

	/*
	 * If the grammar supplied parses correctly, then we will consider
	 * it valid.
	 */
	if (val && (*val != (char)NULL))
	{
		if ((tptr = _create_parse_tree(val)) == 
		    (struct secgrammar_tree *)NULL)
		{
			*ret = (char *)NULL;
			rc = EINVAL;
		}
		else
		{
			rc = chkparsetree(tptr);
	        	_release_parse_tree(tptr);
		}
	}
	return(rc);
}

/*
 * NAME: chkparsetree
 * 
 * FUNCTION: Helper routine for chkauthsystem().  This routine checks each
 *	     leaf of the grammar parse tree to ensure that it is a defined
 *	     method.
 * 
 * RETURNS:  0 or error
 *
 * ERRNO:    EINVAL - Invalid parse tree element found.
 */
int
chkparsetree(struct secgrammar_tree *tptr)
{
	char *ret;
	int  rc = 0;

	switch(tptr->type)
	{
		case LEAF:
			return(chkauthmethod(tptr->name, &ret));
		case NODE:
			if (tptr->left)
				rc = chkparsetree(tptr->left);
			if (!rc && tptr->right)
				rc = chkparsetree(tptr->right);
			break;
		default:
			rc = EINVAL;
	}
	return(rc);
}


/*
 * NAME:     chkhistsize
 *
 * FUNCTION: Validates the value of the histsize attribute.
 *           It checks for long value, checks for value in the
 *              range MIN_HISTSIZE <= value <= MAX_HISTSIZE.
 *
 * RETURNS:
 *              errno if error
 *              0 if OK
 */
int
chkhistsize(char *val,int *ret)
{
register int	err;	/* error returned from chkint */

	if (err = chkint(val,ret))
		return (err);

	if (! (MIN_HISTSIZE <= *ret && *ret <= MAX_HISTSIZE))
		return(EINVAL);

	return (0);
}

/*
 * NAME:     chkmkhistsize
 *
 * FUNCTION: Validates the value of the histsize attribute.
 *           It checks for long value, checks for value in the
 *              range MIN_HISTSIZE <= value <= MAX_HISTSIZE.
 *
 * RETURNS:
 *              errno if error
 *              0 if OK
 */
int
chkmkhistsize(char *val,int *ret)
{

	*ret = (unsigned long)val;

	if (! (MIN_HISTSIZE <= *ret && *ret <= MAX_HISTSIZE))
		return(EINVAL);

	return (0);
}

/*
 * NAME:     chkhistexpire
 *
 * FUNCTION: Validates the value of the histexpire attribute.
 *           It checks for long value, checks for value in the
 *              range MIN_HISTEXPIRE <= value <= MAX_HISTEXPIRE.
 *
 * RETURNS:
 *              errno if error
 *              0 if OK
 */
int
chkhistexpire(char *val,int *ret)
{
register int	err;	/* error returned from chkint */

	if (err = chkint(val,ret))
		return (err);

        if (! (MIN_HISTEXPIRE <= *ret && *ret <= MAX_HISTEXPIRE))
                return(EINVAL);

        return (0);
}


/*
 * NAME:     chkmkhistexpire
 *
 * FUNCTION: Validates the value of the histexpire attribute.
 *           It checks for long value, checks for value in the
 *              range MIN_HISTEXPIRE <= value <= MAX_HISTEXPIRE.
 *
 * RETURNS:
 *              errno if error
 *              0 if OK
 */
int
chkmkhistexpire(char *val,int *ret)
{

	*ret = (unsigned long)val;

        if (! (MIN_HISTEXPIRE <= *ret && *ret <= MAX_HISTEXPIRE))
                return(EINVAL);

        return (0);
}

/*
 * NAME:     chkminage
 *
 * FUNCTION: Validates the value of the minage attribute.
 *
 * RETURNS:  errno - Failure.
 *               0 - Success.
 *
 */
int
chkminage(char *val, int *ret)
{
register int	err;	/* error returned from chkint */

	if (err = chkint(val, ret))
		return(err);

        if (! (MIN_MINAGE <= *ret && *ret <= MAX_MINAGE))
                return(EINVAL);

        return(0);
}

/*
 * NAME:     chkmkminage
 *
 * FUNCTION: Validates the value of the minage attribute.
 *
 * RETURNS:  errno - Failure.
 *               0 - Success.
 *
 */
int
chkmkminage(char *val, int *ret)
{

        *ret = (unsigned long)val;

        if (! (MIN_MINAGE <= *ret && *ret <= MAX_MINAGE))
                return(EINVAL);

        return(0);
}



/*
 * NAME:     chkmaxage
 *
 * FUNCTION: Validates the value of the maxage attribute.
 *
 * RETURNS:  errno - Failure.
 *               0 - Success.
 *
 */
int
chkmaxage(char *val, int *ret)
{
register int	err;	/* error returned from chkint */

	if (err = chkint(val, ret))
		return(err);

        if (! (MIN_MAXAGE <= *ret && *ret <= MAX_MAXAGE))
                return(EINVAL);

        return(0);
}

/*
 * NAME:     chkmkmaxage
 *
 * FUNCTION: Validates the value of the maxage attribute.
 *
 * RETURNS:  errno - Failure.
 *               0 - Success.
 *
 */
int
chkmkmaxage(char *val, int *ret)
{


	*ret = (unsigned long)val;

        if (! (MIN_MAXAGE <= *ret && *ret <= MAX_MAXAGE))
                return(EINVAL);

        return(0);
}



/*
 * NAME:     chkmaxexpired
 *
 * FUNCTION: Validates the value of the maxexpired attribute.
 *
 * RETURNS:  errno - Failure.
 *               0 - Success.
 *
 */
int
chkmaxexpired(char *val, int *ret)
{
register int	err;	/* error returned from chkint */

	if (err = chkint(val, ret))
		return(err);

        if (! (MIN_MAXEXP <= *ret && *ret <= MAX_MAXEXP))
                return(EINVAL);

        return(0);
}

/*
 * NAME:     chkmkmaxexpired
 *
 * FUNCTION: Validates the value of the maxexpired attribute.
 *
 * RETURNS:  errno - Failure.
 *               0 - Success.
 *
 */
int
chkmkmaxexpired(char *val, int *ret)
{

	*ret = (unsigned long)val;

        if (! (MIN_MAXEXP <= *ret && *ret <= MAX_MAXEXP))
                return(EINVAL);

        return(0);
}



/*
 * NAME:     chkminalpha
 *
 * FUNCTION: Validates the value of the minalpha attribute.
 *
 * RETURNS:  errno - Failure.
 *               0 - Success.
 *
 */
int
chkminalpha(char *val, int *ret)
{
register int	err;	/* error returned from chkint */

	if (err = chkint(val, ret))
		return(err);

        if (! (MIN_MINALPHA <= *ret && *ret <= MAX_MINALPHA))
                return(EINVAL);

        return(0);
}


/*
 * NAME:     chkmkminalpha
 *
 * FUNCTION: Validates the value of the minalpha attribute.
 *
 * RETURNS:  errno - Failure.
 *               0 - Success.
 *
 */
int
chkmkminalpha(char *val, int *ret)
{

        *ret = (unsigned long)val;

        if (! (MIN_MINALPHA <= *ret && *ret <= MAX_MINALPHA))
                return(EINVAL);

        return(0);
}


/*
 * NAME:     chkminother
 *
 * FUNCTION: Validates the value of the minother attribute.
 *
 * RETURNS:  errno - Failure.
 *               0 - Success.
 *
 */
int
chkminother(char *val, int *ret)
{
register int	err;	/* error returned from chkint */

	if (err = chkint(val, ret))
		return(err);

        if (! (MIN_MINOTHER <= *ret && *ret <= MAX_MINOTHER))
                return(EINVAL);

        return(0);
}

/*
 * NAME:     chkmkminother
 *
 * FUNCTION: Validates the value of the minother attribute.
 *
 * RETURNS:  errno - Failure.
 *               0 - Success.
 *
 */
int
chkmkminother(char *val, int *ret)
{

        *ret = (unsigned long)val;

        if (! (MIN_MINOTHER <= *ret && *ret <= MAX_MINOTHER))
                return(EINVAL);

        return(0);
}



/*
 * NAME:     chkmindiff
 *
 * FUNCTION: Validates the value of the mindiff attribute.
 *
 * RETURNS:  errno - Failure.
 *               0 - Success.
 *
 */
int
chkmindiff(char *val, int *ret)
{
register int	err;	/* error returned from chkint */

	if (err = chkint(val, ret))
		return(err);

        if (! (MIN_MINDIFF <= *ret && *ret <= MAX_MINDIFF))
                return(EINVAL);

        return(0);
}

/*
 * NAME:     chkmkmindiff
 *
 * FUNCTION: Validates the value of the mindiff attribute.
 *
 * RETURNS:  errno - Failure.
 *               0 - Success.
 *
 */
int
chkmkmindiff(char *val, int *ret)
{

        *ret = (unsigned long)val;

        if (! (MIN_MINDIFF <= *ret && *ret <= MAX_MINDIFF))
                return(EINVAL);

        return(0);
}



/*
 * NAME:     chkmaxrepeats
 *
 * FUNCTION: Validates the value of the maxrepeats attribute.
 *
 * RETURNS:  errno - Failure.
 *               0 - Success.
 *
 */
int
chkmaxrepeats(char *val, int *ret)
{
register int	err;	/* error returned from chkint */

	if (err = chkint(val, ret))
		return(err);

        if (! (MIN_MAXREP <= *ret && *ret <= MAX_MAXREP))
                return(EINVAL);

        return(0);
}

/*
 * NAME:     chkmkmaxrepeats
 *
 * FUNCTION: Validates the value of the maxrepeats attribute.
 *
 * RETURNS:  errno - Failure.
 *               0 - Success.
 *
 */
int
chkmkmaxrepeats(char *val, int *ret)
{

        *ret = (unsigned long)val;

        if (! (MIN_MAXREP <= *ret && *ret <= MAX_MAXREP))
                return(EINVAL);

        return(0);
}



/*
 * NAME:     chkminlen
 *
 * FUNCTION: Validates the value of the minlen attribute.
 *
 * RETURNS:  errno - Failure.
 *               0 - Success.
 *
 */
int
chkminlen(char *val, int *ret)
{
register int	err;	/* error returned from chkint */

	if (err = chkint(val, ret))
		return(err);

        if (! (MIN_MINLEN <= *ret && *ret <= MAX_MINLEN))
                return(EINVAL);

        return(0);
}

/*
 * NAME:     chkmkminlen
 *
 * FUNCTION: Validates the value of the minlen attribute.
 *
 * RETURNS:  errno - Failure.
 *               0 - Success.
 *
 */
int
chkmkminlen(char *val, int *ret)
{

        *ret = (unsigned long)val;

        if (! (MIN_MINLEN <= *ret && *ret <= MAX_MINLEN))
                return(EINVAL);

        return(0);
}


/*
 * NAME:     chkpwdchecks
 *
 * FUNCTION: Validates the value of the pwdchecks attribute.
 *
 * RETURNS:  errno - Failure.
 *               0 - Success.
 *
 */
int
chkpwdchecks(char *val, char **ret)
{
	char		*begin;
	char		*p, *q;
	char		*lp, *libpath;
	char		*relpath;
	char		*special = ", \t\n";
	struct	stat	sbuf;
	int		Found;
	int		len;

	*ret = (char *) NULL;

	/*
	 * Convert the comma separated list into a DNL (Double Null List).
	 */
	if (!(begin = (char *) malloc(strlen(val) + 2)))
		return(errno);

	for (q = begin, p = val; *p;) {
		while (*p && !strchr (special, *p))
			*q++ = *p++;

		*q++ = '\0';

		while (*p && strchr (special, *p))
			p++;
	}
	if (q == begin)
		*q++ = '\0';

	/* Add the final NULL character to double null terminate the list */
	*q = '\0';

	/*
	 * Validate each entry.  Must be a readable file with an absolute path
	 * name or a path name relative to PWDCHECKS_LIBPATH.
	 */
	for (q = begin; *q; q += len+1) {
		len = strlen(q);

		/* Check if an absolute path name */
        	if (*q == '/')
		{
			if (!accessx(q, X_OK, ACC_SELF) && !stat(q, &sbuf) &&
				(sbuf.st_mode & S_IFMT) == S_IFREG)
				continue;
			free((void *) begin);
			return(EINVAL);
		}

		/* Check if path name is relative to PWDCHECKS_LIBPATH */
		if (!(libpath = strdup(PWDCHECKS_LIBPATH)))
		{
			free((void *) begin);
			return(ENOMEM);
		}

		/*
		 * Parse the colon separated PWDCHECKS_LIBPATH appending the
		 * file name to each entry found in PWDCHECKS_LIBPATH.
		 */
		Found = FALSE;
		for (p=libpath; lp=strtok(p, ":"); p = (char *)NULL)
		{
			if (!(relpath = (char *) malloc(len + strlen(lp) + 2)))
			{
				free((void *) libpath);
				free((void *) begin);
				return(ENOMEM);
			}
			strcpy(relpath, lp);	/* LIBPATH  */
			strcat(relpath, "/");	/* slash    */
			strcat(relpath, q);	/* Filename */

			if (!accessx(relpath, X_OK, ACC_SELF) &&
				!stat(relpath, &sbuf) &&
				(sbuf.st_mode & S_IFMT) == S_IFREG)
			{
				Found = TRUE;
				free((void *) relpath);
				break;
			}
			free((void *) relpath);
		}
		free((void *) libpath);

		if (!Found)
		{
			free((void *) begin);
			return(EINVAL);
		}
	}

	/* Return the DNL */
	*ret = begin;
	return(0);
}


/*
 * NAME:     chkdictionlist
 *
 * FUNCTION: Validates the value of the dictionlist attribute.
 *
 * RETURNS:  errno - Failure.
 *               0 - Success.
 *
 */
int
chkdictionlist(char *val, char **ret)
{
	char		*begin;
	char		*p, *q;
	char		*special = ", \t\n";
	struct	stat	sbuf;

	*ret = (char *) NULL;

	/*
	 * Convert the comma separated list into a DNL (Double Null List).
	 */
	if (!(begin = (char *) malloc(strlen(val) + 2)))
		return(errno);

	for (q = begin, p = val; *p;) {
		while (*p && !strchr (special, *p))
			*q++ = *p++;

		*q++ = '\0';

		while (*p && strchr (special, *p))
			p++;
	}
	if (q == begin)
		*q++ = '\0';

	/* Add the final NULL character to double null terminate the list */
	*q = '\0';

	/*
	 * Validate each entry.  Must be a file with an absolute path name.
	 */
	for (q = begin; *q;) {

		/* Check if an absolute path name */
        	if (*q != '/')
		{
			free((void *) begin);
			return(EINVAL);
		}
		if (stat(q, &sbuf) || (sbuf.st_mode & S_IFMT) != S_IFREG)
		{
			free((void *) begin);
			return(EINVAL);
		}
		while (*q++);
	}

	/* Return the DNL */
	*ret = begin;
	return(0);
}
