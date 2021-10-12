static char sccsid[] = "@(#)98  1.9  src/bos/usr/bin/pwdck/pwdbuild.c, cmdsadm, bos411, 9428A410j 7/22/91 09:35:07";
/*
 * COMPONENT_NAME: (TCBADM) Trusted System Adminstration
 *
 * FUNCTIONS: buildpw(),buildsecpw()
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/audit.h>
#include <userpw.h>
#include <usersec.h>
#include <userconf.h>
#include "pwdck.h"

#include "pwdck_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_PWDCK,n,s) 

/* 
 * global data 
 */
extern	struct	pwdck	**sptab;
extern	struct	pwfile	**ptab;
extern	int	totalusers;
extern	int	all;
extern	int	pwdreq;
extern	int	exitcode;
extern	int	modify;
extern	int	nflag;
extern 	int 	errno;
extern	char	*mp, *fxp;


/*
 * NAME: buildpw
 *
 * FUNCTION: build a table of desired entries in /etc/passwd
 *		
 *
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	read the line, parse the user name and password, and place it in 
 *	the table. 
 *
 * (DATA STRUCTURES:) 
 *	builds the globally used table structure
 *
 * RETURNS:
 *	number of entries in /etc/passwd.
 */
int
buildpw(
char **users, 	/* users to check 			*/
int fd )	/* file descriptor for /etc/passwd 	*/
{
	int 	tabdx = 0;		/* index to sptab[] 		*/
	int	tabsize = TABLESIZE;	/* initial size of sptab[]	*/
	char 	pwline[PWLINESIZE+2];	/* space to hold password line	*/
	char	*cp;			/* pointer to characters in line*/
	struct	pwfile	**pwp;		/* pointer to /etc/passwd table	*/
	FILE	*epfp;			/* file pointer to /etc/passwd	*/
	char	*strchr();

	/* get the file pointer */

	if ((epfp = fdopen(fd, nflag ? "r":"r+")) == NULL)
	{
		mp = MSGSTR(MOPENEP,DOPENEP);
		pwexit(mp, AOPENEP, (char *)NULL);
	}
	
	/* allocate initial memory for the 
	 * /etc/passwd table (ptab) 
	 */

	ptab=(struct pwfile **)pwmalloc(sizeof(struct pwfile *) * TABLESIZE);

	/* loop through /etc/passwd and 
	 * build the table 
	 */

	while (fgets(pwline,PWLINESIZE,epfp) != NULL) 
	{
		/* allocate more memory for tables if needed 
		 * Also provide for last entry and NULL 
		 * terminator.
		 */

		if (tabdx == (tabsize - 2)) 
		{
		unsigned int siz;

		     tabsize += TABLESIZE;
		     siz = tabsize * sizeof (char *);
		     ptab=(struct pwfile **)pwrealloc((char *)ptab, siz);
		}

		/* get memory for 
		 * each entry */

		ptab[tabdx]=(struct pwfile *)pwmalloc(sizeof(struct pwfile));

		/* store the 
		 * line in ptab */

		ptab[tabdx]->pwline = pwdup (pwline);

		/* initialize check and delete flags.
		 * default: check this line, don't delete it.
		 */

		ptab[tabdx]->check = 1;	
		ptab[tabdx]->delete = 0;
		
		/* check that we have 
		   at least 2 colons */

		if(((cp=strchr(pwline,':'))==NULL)||((cp=strchr(++cp,':'))==NULL))
			/* bad line, no 2 colons */
		{

			/* Worry about the bad line only if
			 * we are checking ALL users.  */

			mp = MSGSTR(MEPLINE,DEPLINE);
			fxp = MSGSTR(FEPLINE,DFEPLINE);
			/* remove the new line to print this */
			pwline[strlen(pwline)-1] = (char)NULL;
			if (all && report(mp, AEPLINE, fxp, pwline, FIX))
				/* don't save this line */
			{
				ptab[tabdx]->check = 0;
				ptab[tabdx++]->delete = 1;
				continue;
			}

			/* save the line 
			 * but don't check it */

			ptab[tabdx]->check = 0;	
			ptab[tabdx++]->delete = 0;
			continue;
		}

		/* extract the 
		 * user name */

		cp = strchr(pwline,':');	   
		*cp = '\0';		
		ptab[tabdx]->user = pwdup (pwline);

		/* null out passwd entry; 
		 * it'll be set in ckpwfield() 
		 */

		ptab[tabdx]->passwd = (char *)NULL;


		tabdx++;

	} /* end while */

	/* any entries found? */

	if (tabdx == 0)
	{
		errno = ENOENT;
		mp = MSGSTR(MEPENT,DEPENT);
		pwexit(mp, AEPENT, (char *)NULL);
	}

	/* terminate the table */

	ptab[tabdx] = (struct pwfile *)NULL;
	totalusers = tabdx;
	
	/* check if the users specified on the command line exist 
	 * in /etc/passwd.
	 */

	if (!all)
	{
		while(*users != NULL)
		{
		    for (pwp=ptab; *pwp!=NULL; pwp++)
		    {
			if (strcmp(*users,(*pwp)->user) == 0) /* found */
				break;

		    }
		    if (*pwp == NULL)	/* not found */
		    {
			mp = MSGSTR(MNENT,DNENT);
			(void) report(mp, ANENT, (char *)NULL, *users, NFIX);
			exitcode = ENOENT;
		    }
		    users++;
		}
	}

	/* success, return the number of entries */

	return(tabdx);
}

/*
 * NAME: addsecline
 *
 * FUNCTION: add a new /etc/security/passwd line to a stanza entry
 *
 * RETURNS: NONE
 */

void
addsecline (char *line, int type, struct pwdck *entry)
{
	struct	secpwline *cur;

	/*
	 * See if any lines are already in the linked list.  If not,
	 * you have to create the first element and update the header
	 * for this stanza.  Point to the new entry.
	 *
	 * If there are any entries, walk the linked list, looking for
	 * the end.  Add an entry and point to it.
	 */

	if (entry->lines == 0) {
		entry->lines = (struct secpwline *) pwmalloc (sizeof *cur);
		cur = entry->lines;
	} else {
		for (cur = entry->lines;cur->next != 0;cur = cur->next)
			if (type != SPW_COMMENT && type == cur->type)
				return;

		cur->next = (struct secpwline *) pwmalloc (sizeof *cur);
		cur = cur->next;
	}

	/*
	 * Fill in the fields for the new entry.  The type is the type
	 * that was provided.  The line is a copy of the line that was
	 * passed in.  The link pointer doesn't go anywheres just yet ...
	 */

	cur->type = type;
	if (line)
		cur->line = pwdup (line);
	else
		cur->line = 0;
	cur->next = 0;
}

/*
 * NAME: buildsecpw
 *
 * FUNCTION: build a table of all the entries in /etc/security/passwd
 *                                                                    
 * NOTES:
 *	if the add flag is true, then we'll add an entry to the existing
 *	table; otherwise, we'll create the whole table.
 *
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	read the stanza name and place it in the table. 
 *	If it has a colon it is a stanza name, so parse off the colon.
 *	read the password, lastupdate and flags fields, for the stanza. Parse 
 *	these and then place them in the table.
 *
 * (DATA STRUCTURES:) 
 *	builds the globally used table structure
 *
 * RETURNS:
 * 	last index of the table (NULL terminator)
 */
int
buildsecpw(
int 	op,   		/* add an entry or create the whole table 	*/
char	*user,		/* user to add 		   			*/
char	*passwd,	/* password of user to add 			*/
int	fd ) 		/* fildes for /etc/security/passwd		*/
{
	int	i;
static	int 	tabdx = 0;		/* index to sptab[] 		*/
	int	validstanza = 0;	/* flag for first stanza	*/
static	int	tabsize = TABLESIZE;	/* initial size of sptab[]	*/
	ulong 	date;			/* holds date for lastupdate	*/	
	char 	*col;			/* pointer for ':'		*/
	char 	*pwlinep;		/* pointer to password line	*/
	char 	pwline[PWLINESIZE+1];	/* space to hold password line	*/
	char 	attribute[200];		/* attribute string		*/
	char 	value[200];		/* value string			*/
	char	*cp;			/* pointer to tcb attribute	*/
	char	*acl;			/* pointer to acl		*/
	char	*pcl;			/* pointer to pcl		*/
	struct	pwdck	*spwent = 0;	/* pointer to a sptab[] entry 	*/
	FILE	*espfp;			/* file pointer for security/pw */


	/* ADD AN ENTRY TO EXISTING TABLE 
	 * (THE TABLE IS INTIALLY CREATED BELOW)
	 * if 'add' set, we want to add an entry to the end of the table, 
	 * and then return. We'll use tabsize to make sure we have 
	 * enough space allocated.
	 * note: tabsize and tabdx are static.
	 */

	if (op == ADD)
	{
		/* Allocate more memory for the table (if needed).
		 * Note: we must provide for last entry and 
		 * the NULL terminator 
		 */

		if (tabdx == (tabsize -2)) 
		{
		    tabsize += TABLESIZE;
		    sptab = (struct pwdck **) pwrealloc ((char *) sptab,
			    sizeof(struct pwdck *)*tabsize);
		}

		/* just report that we added a 
		 * new stanza.
		 */

		mp = MSGSTR(MNEW,DNEW);
		(void)report(mp, ANEW, (char *)NULL, user, NFIX);

		/* allocate space for 
		 * the new table entry */

		spwent = (struct pwdck *)pwmalloc(sizeof(struct pwdck));

		spwent->lines = 0;
		for (i = 0;ptab[i] != 0;i++) {
			if (strcmp (ptab[i]->user, user) == 0) {
				spwent->pwfile = ptab[i];
				break;
			}
		}
		if (ptab[i] == 0)
			spwent->pwfile = 0;

		addsecline ((char *) 0, SPW_NAME, spwent);

		/*
		 * set up the stanza as follows:
		 * authname:
		 *
		 *		password = * or passwd field in /etc/passwd
		 *		lastupdate = present time (if password is
		 *					   non-blank).
		 */

	       	strcpy(spwent->upw.upw_name,user);

		/* default to no 
		 * lastupdate field */

		spwent->lastupdate = 0; /* no lastupdate */
		spwent->upw.upw_lastupdate = 0;

		/* copy the /etc/passwd password if it 
		 * exists and is non-null */

		if (passwd == NULL)
		{
			spwent->upw.upw_passwd = pwdup ("*");
		}
		else 
		{
			if((*passwd == '\0') && pwdreq)
			{
				spwent->upw.upw_passwd = pwdup ("*");
			}
			else
			{
				spwent->upw.upw_passwd = pwdup (passwd);
			}

			/* if passwd exists and non-blank, 
			 * set lastupdate */

			if ((*passwd != '\0') && (strcmp(passwd,"*") != 0))
			{
	       			spwent->lastupdate = 1; /* lastupdate exists*/
	       			spwent->upw.upw_lastupdate = time((time_t *) 0);
				addsecline ((char *) 0, SPW_LAST, spwent);
			}
		}

	       	spwent->upw.upw_flags = 0;	/* flag = blank		*/
		spwent->raw_flags = NULL;/* attribute doesn't exist 	*/
	       	spwent->password = 1; 	/* attribute exists		*/
		addsecline ((char *) 0, SPW_PASSWD, spwent);
	       	spwent->flags = 0; 	/* attribute doesn't exist	*/
	       	spwent->invalid_flags = 0;/* flags are not invalid	*/
	       	spwent->principle = 0;	/* this entry is not a principle*/

		if (all && spwent->pwfile == 0) {
			mp = MSGSTR(MNENT,DNENT);
			fxp = MSGSTR(FUAUTH,DFUAUTH);
			if(report(mp, AUAUTH, fxp, spwent->upw.upw_name, FIX))
			{
				spwent->delete = 1;
				spwent->check = 0;
				mp = MSGSTR(MDEL,DDEL);
				report(mp,ADEL,(char *) 0,
					spwent->upw.upw_name,NFIX);
			}
		} else {
			spwent->delete = 0;
			spwent->check = 1;
		}
		/* store the new entry, and terminate the table again	*/
		sptab[tabdx++] = spwent;
		sptab[tabdx] = 0;

		return(tabdx);
	} /* if add */

	/*
	 * CREATE THE TABLE HERE 
	 */

	/* get the file pointer */

	if ((espfp = fdopen(fd, nflag ? "r":"r+")) == NULL)
	{
		mp = MSGSTR(MOPENESP,DOPENESP);
		pwexit(mp, AOPENESP, (char *)NULL);
	}

	/* allocate initial memory for 
	 * the /etc/security/passwd table */

	sptab = (struct pwdck **)pwmalloc(sizeof(struct pwdck *)*TABLESIZE);

	/* loop through /etc/security/passwd 
	 * and build the table */

	while (fgets(pwline,PWLINESIZE,espfp) != NULL) 
	{
		/* Allocate more memory for the table (if needed).
		 * Note: we must provide for last entry and 
		 * the NULL terminator 
		 */

		if (tabdx == (tabsize - 2)) 
		{
		    tabsize += TABLESIZE;
		    sptab = (struct pwdck **)pwrealloc((char *)sptab,
				sizeof(struct pwdck *)*tabsize);
		}

		/* parse the line; 
		 * use pwlinep */

		pwlinep = pwline;

		while ((isspace((int)*pwlinep)) && (*pwlinep!='\n'))
			pwlinep++;

		if (*pwlinep == '\n') {	/* end of line */
			if (validstanza) {
				sptab[tabdx++] = spwent;
				sptab[tabdx] = 0;
				spwent = 0;
				validstanza = 0;
			}
			continue;
		}

		/*
		 * See if this is a comment line.  Comments begin with
		 * '*'.
		 */

		if (*pwline == '*') {
			if (spwent)
				addsecline (pwline, SPW_COMMENT, spwent);

			continue;
		}

		/* look for stanza; stanza name must terminated by a ':'
		 * once we start a stanza, set up  it's upw structure
		 */

		if ((col = strchr(pwlinep,':')) != NULL) 
		{
			*col = '\0';	 /* get rid of the ':'  */

			/* we got a new stanza, write out the 
			 * previous entry.
			 * NOTE: this passes if we have a previous spwent 
			 * to write out.
			 * 'validstanza' gets set below */
			if (validstanza) 	
			{
				sptab[tabdx++] = spwent;
				sptab[tabdx] = 0;
			}

			/* allocate space for 
			 * each table entry */
			spwent=(struct pwdck *)pwmalloc(sizeof(struct pwdck));
                     	spwent->lines = 0;

			/* store the stanza name */
			strcpy(spwent->upw.upw_name, pwlinep);
			for (i = 0;ptab[i] != 0;i++) {
				if (strcmp (ptab[i]->user, pwlinep) == 0) {
					spwent->pwfile = ptab[i];
					break;
				}
			}
			if (ptab[i] == 0)
				spwent->pwfile = 0;

			addsecline ((char *) 0, SPW_NAME, spwent);

			/* initial (default) state of the pwdck fields.
			 * default: entry doesn't exist in /etc/security/passwd
			 */
			spwent->upw.upw_passwd = (char *)NULL;	/* blank      */
			spwent->upw.upw_flags = 0;		/* blank      */
			spwent->upw.upw_lastupdate = 0;		/* blank      */
			spwent->raw_flags = (char *)NULL;	/* blank      */
			spwent->password = 0; /* attribute non-existent       */
			spwent->flags = 0;    /* attribute non-existent	      */
			spwent->lastupdate = 0;/* attribute non-existent      */
			spwent->invalid_flags = 0;  /* flags are not invalid  */
			spwent->principle=0; /* this entry is not an authname */

			if (all && spwent->pwfile == 0) {
				mp = MSGSTR(MNENT,DNENT);
				fxp = MSGSTR(FUAUTH,DFUAUTH);
				if(report(mp, AUAUTH, fxp,
					spwent->upw.upw_name, FIX))
				{
					spwent->delete = 1;
					spwent->check = 0;
					mp = MSGSTR(MDEL,DDEL);
					report(mp,ADEL,(char *) 0,
						spwent->upw.upw_name,NFIX);
				}
			} else {
				spwent->delete = 0;
				spwent->check = 1;
			}

			/* 'validate' this stanza so that 
			 * it gets stored later*/
			validstanza = 1;

			continue; 	/* go get the next line */
		}

		/* continue in case we have junk at the 
		 * beginning of /etc/security/passwd */

		if (! validstanza) {
			/* remove the new line to print this */
			pwline[strlen(pwline)-1] = (char)NULL;
			mp = MSGSTR(MLINE,DLINE);
			(void) report(mp, ALINE, (char *)NULL, pwline, NFIX);
			exitcode = ENOTRUST;
			modify = 1;
			continue;
		}

		/* 
		 * get attributes: password, lastupdate, flags
		 */

		/* loop until we get the 
		 * whole attribute name */

		while (*pwlinep == ' ' || *pwlinep == '\t')
			pwlinep++;

		for (i = 0;'a' <= *pwlinep && *pwlinep <= 'z';)
			attribute[i++] = *pwlinep++;

		attribute[i] = '\0';

		while (*pwlinep == ' ' || *pwlinep == '\t')
			pwlinep++;

		if (*pwlinep != '=')
		{
		   	/* no '=' found; 
			 * let's ignore this line */
			/* remove the new line to print this */
			pwline[strlen(pwline)-1] = (char)NULL;
			mp = MSGSTR(MLINE,DLINE);
			(void) report(mp, ALINE, (char *)NULL, pwline, NFIX);
			exitcode = ENOTRUST;
			modify = 1;
		   	continue;
		}

		/* get past = sign and then 
		 * get the value of the attribute 
		 */
		pwlinep++;	/* skip over '=' */

		/* skip the spaces */
		while (*pwlinep == ' ')
			pwlinep++;

		/* loop until we get the 
		 * attribute value 
		 */
		i = 0;

		while ((*pwlinep != '\n') && (*pwlinep != ' ')) 
		{
			value[i++] = *pwlinep++;
		}

		value[i] = '\0';

		/* store 'password' attribute 
		 * in the table */
		if (strcmp(PASSWORD,attribute) == 0) 
		{
			/* found 'passwd = value' */

			spwent->upw.upw_passwd=pwmalloc(strlen(value)+1);
			strcpy(spwent->upw.upw_passwd,value);
			spwent->password = 1;	/* attribute exists	*/
			addsecline ((char *) 0, SPW_PASSWD, spwent);
		}
		
		/* store 'flags' attribute 
		 * in the table */
		else if (strcmp(FLAGS,attribute) == 0) 
		{
			/* convert the flags from strings to ulong and store 
			 * them for us.
			 * a return of -1 indicates that invalid flags also
			 * exist. (used for ckflags()).
			 * Also store the whole attribute in case we don't
			 * validate it (we can write it back).
			 */

			spwent->raw_flags = pwdup (value);
			if (convflags(&(spwent->upw.upw_flags),value) == -1)
			{
				spwent->invalid_flags = 1;
			}
			spwent->flags = 1;	/* attribute exists	*/
			addsecline ((char *) 0, SPW_FLAGS, spwent);
		}

		/* store 'lastupdate' attribute 
		 * in the table */
		else if (strcmp(LASTUPDATE,attribute) == 0) 
		{
			date = atoi(value);
			spwent->upw.upw_lastupdate = date;
			spwent->lastupdate = 1;	/* attribute exists	*/
			addsecline ((char *) 0, SPW_LAST, spwent);
		}
		else 
		{
			/* we found an unknown attribute;
			 * let's ignore this line
			 * NOTE: if more attributes are added to the 
			 * 	/etc/security/passwd file, then a new
			 * 	member should be added to the pwdck structure
			 * 	declared in pwdck.h; Also, the new structure
			 * 	member should be set here, and written out in
			 *	writepw().
			 */
			/* remove the new line to print this */
			pwline[strlen(pwline)-1] = (char)NULL;
			mp = MSGSTR(MLINE,DLINE);
			(void) report(mp, ALINE, (char *)NULL, pwline, NFIX);
			exitcode = ENOTRUST;
			modify = 1;
		}
		continue;

	} /* end while */

	/* add the last entry 
	 * to the table */

	if (validstanza)
		sptab[tabdx++] = spwent;

	/* terminate it */

	sptab[tabdx] = (struct pwdck *)NULL;

	return(tabdx);
}
