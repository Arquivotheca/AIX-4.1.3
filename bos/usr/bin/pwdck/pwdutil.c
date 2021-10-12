static char sccsid[] = "@(#)01	1.9.1.1  src/bos/usr/bin/pwdck/pwdutil.c, cmdsadm, bos411, 9428A410j 10/12/93 10:02:03";
/*
 *   COMPONENT_NAME: CMDSADM
 *
 *   FUNCTIONS: MSGSTR
 *		convflags
 *		findsystem
 *		gettime
 *		priv_get
 *		priv_put
 *		put_program
 *		pwcopy
 *		pwdup
 *		pwexit
 *		pwfree
 *		pwmalloc
 *		pwopen
 *		pwrealloc
 *		pwrequired
 *		report
 *		unlinktmp
 *		usage
 *		userfound
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/audit.h>
#include <sys/access.h>
#include <sys/flock.h>
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
extern	int	totalusers;
extern	int	yflag,nflag,pflag,tflag,ck_query();
extern	int	fixit,query;
extern	char 	*tmpep;
extern	char 	*tmpesp;
extern	FILE	*tmpepfp;
extern	FILE	*tmpespfp;
extern 	int 	errno;

extern	int	passwdrequired(char *, char **);	/* From libs.a */

static	char	*mp, *fxp;	/* pointer to message strings */

/*
 * NAME: convflags 
 *
 * FUNCTION: 
 * 	Converts the flags from strings to a numeric value and sets this 
 *	value for the calling routine.
 *
 * RETURNS: returns -1 if invalid flags were found; otherwise return 0.
 *	
 */
int
convflags(ulong *flags, char *valptr)
{
	int j=0;
	int invalid=0;	/* 0 = valid */
	char *cflag[MAXFLAGS];	/* list of flags	*/
	char *valp;		/* pointer to each flag */

	/* loop through the end of '[flags =] value' entry */
	while (*valptr != (char)NULL) 
	{
		valp = valptr;	/* mark the beginning of the flag string */

		/* skip spaces or extra commas at the front	*/
		while ((isspace((int)*valptr)) || (*valptr == ','))
			valptr++;

		/* advance to the end of the flag string (until comma or NULL)*/
		while ((*valptr != ',') && (*valptr != (char)NULL))
			valptr++;

		if (*valptr == ',')
			*valptr++ = '\0';	/* replace the comma with '\0'*/

		/* store the flag string in our list */
		cflag[j++] = valp;
	}

	cflag[j] = NULL;	/* terminate the list	*/

	/* now convert all the flags in our list */
	*flags = 0;

	for(j=0; cflag[j] != NULL; j++)
	{
		if (strcmp(cflag[j],"NOCHECK") == 0)
		{
			*flags |= PW_NOCHECK;
			continue;
		}

		if (strcmp(cflag[j],"ADMCHG") == 0) 
		{
			*flags |= PW_ADMCHG;
			continue;
		}
	
		if (strcmp(cflag[j],"ADMIN") == 0)
		{
			*flags |= PW_ADMIN;
			continue;
		}

		/* this flag is not valid */
		invalid = 1;
	}

	if (invalid)
		return(-1);
	else
		return(0);
}

/*
 * NAME: findsystem
 *
 * FUNCTION: 
 * 	this function will parse the AUTH string returned by getuserattr() and
 *	return a null separated list of principle names which are found 
 * 	with SYSTEM (or alternative) authentication methods.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *
 * RETURNS: the list principle names 
 */
char	**
findsystem(
char *authstr,		/* pointer to auth entry to parse 		*/
char *user ) 		/* user name of this stanza to use as default	*/
{
	char 	**names;		  /* list of authnames that are found */
	char	**namep;		  /* pointer to names		      */
	char	*beg,*np;
	char *strstr(); 

	/* allocate memory for our list of names */
	/* totalusers is set in buildptab */
	names = (char **) pwmalloc(sizeof(char *)*(totalusers+TABLESIZE));

	namep = names;	/* point to list */

	/* look for instances of SYSTEM or other authentication methods. */
	while (*authstr != (char)NULL)	/* loop until double-null */
	{
		beg = authstr;
		/* skip to next string (null) */
		while (*authstr++); 	/* point to char after the null */
		
		while ((*beg != *authstr))
		{
			if (*beg == ';')	
			{
				/* we have an authname. (unless it looks like ';\0') */
				np = ++beg;	/* point to name */

				/* skip to end of name */
				while (*beg != (char)NULL)
					beg++;

				/* if there's no name after the ';', use stanza name */
				if (*np == (char)NULL)
				{
					*namep++ = user;
					break;
				}

				/* now put the name into the list */
				*namep++ = np;
			}
			else
			{
				beg++;
				if (*beg == *authstr)
					*namep++ = user;
			}
		}
	}

	/* done, terminate the list */
	*namep++ = (char *)NULL;	/* terminate the list */
	names = (char **) pwrealloc ((char *) names,
		(unsigned int) (namep - names) * sizeof (char *));

	return(names);
}
/*
 * NAME: usage
 *
 * FUNCTION: Print usage message and exit
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * RETURNS: This routine does not return.
 */
void
usage ()
{
	errno = EINVAL;
	mp = MSGSTR(MUSG,DUSG);
	pwexit(mp, AUSG, (char *)NULL);
}

/*
 * NAME: pwrequired
 *                                                                    
 * FUNCTION: Checks to see if the user is required to have a passwd.
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 *	Passwords are required if the password constraints specify a
 *	minimum length (via 'minalpha + minother' or minlen) for the password.
 *                                                                   
 * RETURNS: 1 if required and 0 if not required
 */  
int
pwrequired(char *uname)
{
	char	*message = (char *) NULL;
	int	rc;

	rc = passwdrequired(uname, &message);	/* returns: 1, 0, or -1 */
	if (message)
		free((void *) message);

	if (rc == -1)
	{
		mp = MSGSTR(MGETCONF,DGETCONF);
		pwexit(mp, AGETCONF, (char *)NULL);
	}
	return(rc);
}

/*
 * NAME: userfound
 *                                                                    
 * FUNCTION: looks for the specified user in the specified list of users.
 *                                                                    
 * EXECUTION ENVIRONMENT: static 
 *
 * RETURNS:
 * 		1 = user was found in list
 *		0 = user was not found in list
 *
 */  
int
userfound(
char	*user,		/* user to look for			*/
char	**list )	/* null-terminated list of users	*/
{
	while(*list != NULL)
	{
		if (strcmp(user,*list) == 0)
			return(1);	/* user was found in list */
		list++;
	}
	return(0);	/* user was not found in list */
}

/*
 * NAME: unlinktmp
 *                                                                    
 * FUNCTION: unlinks one or both passwd files.
 *	     If there's an error, process it here because we'll exit from
 *	     the calling routine. This function is called only when another
 *	     error has occured. 
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 * RETURNS: none
 *
 */  
void
unlinktmp(char *tmpep, char *tmpesp)
{
     if (tmpep)
     {
	if(unlink(tmpep) == -1)
	{
		fprintf(stderr,MSGSTR(MUNLNK,DUNLNK),tmpep);
		auditwrite(PWDERROR,AUDIT_FAIL,tmpep,strlen(tmpep),AUNLNK,strlen(AUNLNK),NULL);
		perror("");
	}
     }

     if (tmpesp)
     {
	if(unlink(tmpesp) == -1)
	{
		fprintf(stderr,MSGSTR(MUNLNK,DUNLNK),tmpesp);
		auditwrite(PWDERROR,AUDIT_FAIL,tmpesp,strlen(tmpep)+1,AUNLNK,strlen(AUNLNK)+1,NULL);
		perror("");
	}
     }
}

/*
 * NAME: pwmalloc
 *                                                                    
 * FUNCTION: frontend to malloc() with error processing
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 * RETURNS: pointer to memory
 *
 */  
char	*
pwmalloc(unsigned int size)
{
	char *mem;

	/*
	 * Zero-sized requests from malloc() return NULL, so change
	 * the request to a non-zero size ...
	 */

	if (size == 0)
		size = 1;

	/*
	 * Allocate the storage that was requested and check for any
	 * errors - I don't expect to run out of memory so treat it
	 * as fatal when it happens.
	 */

	if ((mem = (char *)malloc((size_t) size)) == NULL)
	{
		mp = MSGSTR(MMALLOC,DMALLOC);
		pwexit(mp, AMALLOC, (char *)NULL);
	}
	return(mem);
}

/*
 * NAME: pwrealloc
 *                                                                    
 * FUNCTION: frontend to realloc() with error processing
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 * RETURNS: pointer to memory
 *
 */  
char	*
pwrealloc(char *ptr, unsigned int size)
{
	char *mem;

	if (size == 0)
		size = 1;

	if ((mem = (char *)realloc((void *)ptr,(size_t) size)) == NULL)
	{
		mp = MSGSTR(MREALLOC,DREALLOC);
		pwexit(mp, AREALLOC, (char *)NULL);
	}

	return(mem);
}

/*
 * NAME: report
 *                                                                    
 * FUNCTION: printfs every message, querys if necessary, and performs auditing.
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 * RETURNS: 
 * 	0 = don't fix it
 *	1 = fix it
 *
 */  
int
report(
char	*mp,				/* message cat string	*/
char	*ap,				/* audit string		*/
char	*fxp,				/* 'what to fix' string	*/
char	*user,				/* user name		*/
int	cmd )				/* command to perform 	*/
{
	int	ret=0;			/* return code		*/
	int	audstat;		/* audit status		*/
	char	*fixed = "not fixed";	/* zero if not, one if fixed */
	extern	int	exitcode, modify;

	if (!pflag)
		fprintf(stderr, mp, user);

	switch(cmd)
	{
		case FIX: 
			if (yflag || pflag)	/* fix it */
			{
				ret = 1;
				modify = 1;	/* to commit the fix */
				fixed = "fixed";/* fix the problem */
			}
			else if (nflag)		/* don't fix it  */
			{
				ret = 0;
			}
			else if (tflag)		/* fix it?	 */
			{
				query = 1;
				fixit = 0;
				if((ret = ck_query(fxp,NULL)) == 1) {
					modify = 1;	/* to commit the fix */
					fixed = "fixed";/* fix the problem */
				}
			}

			break;

		case NFIX: 
		default:
		/* definitely don't
		 * fix; just audit it
		 */
			break;
	}


	/* set ENOTRUST to indicate that an error
   	   was detected in the passwd files.
	   This errno should be set even if we don't fix the error,
	   and even if it's not reported (-p) but still detected. */

	exitcode = ENOTRUST;	

	audstat = AUDIT_OK;

	(void) privilege(PRIV_ACQUIRE);
	auditwrite(PWDEVENT,audstat,
		user,strlen(user)+1,
		ap,strlen(ap)+1,
		fixed,strlen(fixed)+1,NULL);
	(void) privilege(PRIV_LAPSE);

	return(ret);
}

/*
 * NAME: pwexit
 *                                                                    
 * FUNCTION: frontend to exit() with error processing and auditing
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 * RETURNS: nothing
 *
 */  
int
pwexit(
char	*mstr,		/* message cat string	*/
char	*astr,		/* audit string		*/
char	*usr )		/* user name string	*/
{
	int err = errno;
	fprintf(stderr, mstr, usr);
	perror("");
	(void) privilege(PRIV_ACQUIRE);
	auditwrite(PWDERROR,AUDIT_FAIL,usr,strlen(usr)+1,astr,strlen(astr)+1,NULL);
	(void) privilege(PRIV_DROP);
	exit(err);
}


/*
 * NAME: pwopen
 *                                                                    
 * FUNCTION: open and lock /etc/passwd and /etc/security/passwd.
 * 	     If open fails on /etc/security/passwd, try to create it, and
 * 	     set up the proper tcb attributes.
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 * RETURNS: 
 *	file descriptor from open.
 *
 */  
int
pwopen(char *file)		/* which file to open */
{

	int	n = 0;		/* the locked period		    	    */
	struct	stat	statbuf;/* to get mtime			    	    */
	int	epfd;		/* fildes to /etc/passwd		    */
	int	espfd;		/* fildes to /etc/security/passwd	    */
	int	opfd;		/* fildes for backup opasswd file	    */
	char	*cp;		/* pointer to tcb attribute		    */
	char	*acl;		/* pointer to acl			    */
	char	*pcl;		/* pointer to pcl			    */
	struct 	flock flk = { 0, 0, 0, 0, 0, 0, 0 };/* to lock the file */

	flk.l_type = nflag ? F_RDLCK:F_WRLCK;

	if (strcmp(file,PASSFILE) == 0)
	{
		/*
		 * Open the file according to the modify flags - the file
		 * is opened for writing only if it might be modified.
		 */

		if ((epfd = open(PASSFILE,nflag ? O_RDONLY:O_RDWR)) == -1) {
			mp = MSGSTR(MOPENEP,DOPENEP);
			pwexit(mp, AOPENEP, (char *)NULL);
		}

		/*
		 * Get a lock on the password file.  This will prevent
		 * anyone else from modifying the file while we have our
		 * fingers in it.
		 */

		while (fcntl (epfd, F_SETLK, &flk) < 0) {
			/* try for 2 minutes */
			n++;
			if (n == 120)
			{
				mp = MSGSTR(MLOCK,DLOCK);
				pwexit(mp, ALOCK, PASSFILE);
			}
			/* sleep and try again */
			sleep (1);
		}

		/* save a copy in opasswd just in case */

		if ((opfd = open("/etc/opasswd",
				O_WRONLY|O_TRUNC|O_CREAT)) != -1) {
			(void) pwcopy(epfd, opfd);
			close (opfd);
		}
		return(epfd);
	}

	if (strcmp(file,SPASSFILE) == 0)
	{
		/* open /etc/security/passwd */
		if ((espfd = open(SPASSFILE,nflag ? O_RDONLY:O_RDWR)) == -1) 
		{
			/* if it doesn't exist, 
			 * try to create it */

			if (errno == ENOENT)
			{
				mp = MSGSTR(MEXIST,DEXIST);
				fxp = MSGSTR(FEXIST,DFEXIST);
				if(!report(mp, AEXIST, fxp, (char *)NULL, FIX))
					pwexit(mp, AEXIST, (char *)NULL);

				espfd = open (SPASSFILE, O_CREAT|O_RDWR, 0);
				if (espfd == -1) {
					mp = MSGSTR(MCREAT,DCREAT);
					pwexit(mp, ACREAT, (char *)NULL);
				}

				/* set the right permissions */
				
				privilege (PRIV_ACQUIRE);
				stat (PASSFILE, &statbuf);
				fchown (espfd, statbuf.st_uid, statbuf.st_gid);
				fchmod (espfd, statbuf.st_mode & 0770);
				privilege (PRIV_LAPSE);

				/* get the acl */
				if (gettcbattr (SPASSFILE, "acl", &cp, 0) != 0) 
					acl = pwdup (cp);
				else
					acl = 0;

				/* get the pcl */
				if (gettcbattr (SPASSFILE, "pcl", &cp, 0) == 0) 
					pcl = pwdup (cp);
				else
					pcl = 0;

				if (acl && put_program(ACLPUT, SPASSFILE, acl))
				{
				      mp = MSGSTR(MACLPUT,DACLPUT);
				      pwexit(mp, AACLPUT, SPASSFILE);
				}

				if (pcl && put_program(PRIVPUT, SPASSFILE, pcl))
				{
				      mp = MSGSTR(MPCLPUT,DPCLPUT);
				      pwexit(mp, APCLPUT, SPASSFILE);
				}
			}
			else
			{
			      mp = MSGSTR(MOPENESP,DOPENESP);
			      pwexit(mp, AOPENESP, (char *)NULL);
			}
		}

		/*
		 * Lock the file to prevent someone else from acquiring
		 * a write lock.  There can be multiple read locks, but
		 * write locks are exclusive.
		 */

		while (fcntl(espfd,F_SETLK,&flk) < 0) {
			/* try for 2 minutes */
			n++;
			if (n == 120) {
				mp = MSGSTR(MLOCK,DLOCK);
				pwexit(mp, ALOCK, SPASSFILE);
			}
			/* sleep and try again */
			sleep (1);
		}

		/* save a copy in opasswd just in case */

		opfd = open("/etc/security/opasswd",
				O_WRONLY|O_CREAT|O_TRUNC, 0);
		privilege (PRIV_ACQUIRE);
		stat (SPASSFILE, &statbuf);
		fchown (opfd, statbuf.st_uid, statbuf.st_gid);
		fchmod (opfd, statbuf.st_mode & 0777);
		privilege (PRIV_LAPSE);

		if (opfd > 0)
		{
			(void) pwcopy(espfd, opfd);
			close(opfd);
		}

		return(espfd);
	}
}

/*
 * NAME: pwfree
 *                                                                    
 * FUNCTION: front-end to free(). This checks for a bad pointer.
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 * RETURNS: 
 *
 */  
int
pwfree(void *mem)
{
	if (! mem)
		return(-1);

	free(mem);
	return(0);
}

/*
 * NAME: gettime
 *                                                                    
 * FUNCTION: get the last-modified time of a file
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 * RETURNS: 
 *
 */  
time_t
gettime(char *filename)
{
	struct	stat	stbuf;

	/* get the last-modified time */
	if(stat(filename,&stbuf) != 0)
	{
		mp = MSGSTR(MTIME,DTIME);
		pwexit(mp, ATIME, filename);
	}
	return (stbuf.st_mtime);
}

/*
 * NAME: priv_get
 *                                                                    
 * FUNCTION: Return a pointer to a PCL for a file.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Performs similiar function to acl_get, except priv_get isn't
 *	in the C library.
 *
 * RETURNS: Pointer to malloc()'d PCL or NULL on error.
 */  

struct pcl *
priv_get (char *file)
{
	int	length;
	struct	pcl	*pcl;

	for (length = sizeof *pcl;;) {
		if (! (pcl = (struct pcl *) pwmalloc ((unsigned int)length)))
			return 0;

		if (statpriv (file, 0, pcl, length)) {
			if (errno != ENOSPC) {
				(void) pwfree ((void *)pcl);
				return 0;
			}
			length = pcl->pcl_len;
			(void) pwfree ((void *)pcl);
			continue;
		} else
			break;
	}
	return pcl;
}

/*
 * NAME: priv_put
 *                                                                    
 * FUNCTION: Apply a PCL to a named file.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Performs similiar function to acl_put, except priv_put isn't
 *	in the C library.
 *
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
priv_put (char *file, struct pcl *pcl, int flag)
{
	int	rc;

	rc = chpriv (file, pcl, pcl->pcl_len);

	if (flag)
		(void) pwfree ((void *)pcl);

	return rc;
}

/*
 * NAME:	put_program
 *
 * FUNCTION:	Execute a program piping it's input from a buffer
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS: Exit status from executed command.
 */
int
put_program (char *prog, char *arg, char *input)
{
	FILE	*fp;
	int	i;
	int	rc;
	char	buf[BUFSIZ];

	/*
	 * Sanity check the program for being an absolute pathname.
	 * Create the command line by appending the argument to
	 * the pathname which was provided in prog.
	 */

	if (prog[0] != '/') {
		return -1;
	}
	if (! arg)
		strcpy (buf, prog);
	else
		sprintf (buf, "%s %s", prog, arg);

	/*
	 * Execute the program, and pipe the input from the user's
	 * buffer.  Report write or execution errors.
	 */

	if (! (fp = popen (buf, "w"))) {
		mp = MSGSTR(MPOPEN,DPOPEN);
		pwexit(mp, APOPEN, prog);
	}
	while (*input && ! ferror (fp))
		putc (*input++, fp);

	if (! ferror (fp)) {
		putc ('\n', fp);
		fflush (fp);
		return pclose (fp);
	} else {
		(void) pclose (fp);
		return -1;
	}
}

/*
 * NAME: pwdup
 *                                                                    
 * FUNCTION: Duplicate a string into a new buffer with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Pointer to string
 */  
char *
pwdup (char *s)
{
	char	*cp;

	cp = pwmalloc ((unsigned int)(strlen (s) + 1));
	return ((char *)strcpy (cp, s));
}

/*
 * NAME: pwcopy
 *                                                                    
 * FUNCTION: copy source file to target file
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: 0 = success		-1 = failure
 */  
int
pwcopy(int sourcefd, int targetfd)
{
	struct	stat	srcstat;	/* to get size of source file	*/
	char	*buffer;		/* to hold the data		*/

	if (lseek(sourcefd, 0 , 0) == -1)
		return(-1);

	if (fstat(sourcefd, &srcstat))
		return(-1);

	if ((buffer = pwmalloc((unsigned int) srcstat.st_size)) == NULL)
		return(-1);

	if (read(sourcefd, buffer, srcstat.st_size) == -1)
		return(-1);

	if (lseek(targetfd, 0 , 0) == -1)
		return(-1);

	if (write(targetfd, buffer, srcstat.st_size) != srcstat.st_size)
		return(-1);

	if (ftruncate(targetfd,srcstat.st_size) == -1)
		return(-1);

	(void) pwfree((void *)buffer);

	if (lseek(sourcefd, 0 , 0) == -1)
		return(-1);

	if (lseek(targetfd, 0 , 0) == -1)
		return(-1);

	return(0);
}
