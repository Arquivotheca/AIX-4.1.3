static char sccsid[] = "@(#)00  1.8  src/bos/usr/bin/pwdck/pwdckuser.c, cmdsadm, bos411, 9428A410j 5/10/91 13:23:07";
/*
 * COMPONENT_NAME: (TCBADM) Trusted System Adminstration
 *
 * FUNCTIONS: ckusername(),ckpwfield(),ckuserpw(),ckpassword(),ckflags(),
 *	      cklastupdate()
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
#include <ctype.h>
#include <sys/types.h>
#include <sys/audit.h>
#include <userpw.h>
#include <usersec.h>
#include "pwdck.h"

#include "pwdck_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_PWDCK,n,s) 

/* 
 * global data 
 */
extern	struct	pwdck	**sptab;
extern	struct	pwfile	**ptab;
extern	int	pwdreq;
extern 	int 	errno;
extern	char	*mp, *fxp;

static int ckpassword(struct pwdck *sptabp, struct pwfile *ptabp);
static int ckflags(struct pwdck *spwent);
static int cklastupdate(struct pwdck *sptabp);


/*
 * NAME: ckusername
 *
 * FUNCTION: 
 * 	validate the logname in the /etc/password file. The
 *	restrictions on login names are:
 *	name must not start with one of these characters: '+', '-', ':', '~'
 *	name can't contain a  ':'.
 *	name can't equal 'ALL','default'
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	user process
 *                                                                   
 * RETURNS: 
 */
int
ckusername(
struct	pwfile	*ptabp )	/* pointer to /etc/passwd table */
{
	static char illegal1stch[] ={'-','+',':','~',0};
	char	*strchr();
	int	badname = 0;	/* flag 			*/
	char	*u;		/* pointer to username chars 	*/
	char	*up;		/* pointer to username*/
	char	*username;	/* pointer to user name		*/

	username = ptabp->user;

	/* if entries begin with +,-, they are NFS entries, so
	 * just ignore it */

	if ( (*username == '+') || (*username == '-') )
	{
		mp = MSGSTR(MIGN,DIGN);
		(void) report(mp, AIGN, (char *)NULL, username, NFIX);

		ptabp->check = 0;

		return(-1);
	}

	/* check for usage */
	if (username && (strchr (illegal1stch, *username)))
		badname =1;
	
	up = username;

	for(u = up; *u != '\0'; u++)
		if (*u == ':')
		{
			badname = 1;
			break;
		}

	if (badname || (strcmp(up,"default")==0) || (strcmp(up,"ALL")==0))
	{
		mp = MSGSTR(MNAM,DNAM);
		fxp = MSGSTR(FNAM,DFNAM);
		if(report(mp, ANAM, fxp, up, FIX))
			/* remove this line */
		{
			ptabp->check = 0;
			ptabp->delete = 1;
		}
		else
			/* keep this line */
		{
			ptabp->check = 0;
			ptabp->delete = 0;
		}
		
		return(-1);
	}

	return(0);
}

/*
 * NAME: ckpwfield
 *
 * FUNCTION:
 * 	validate the password field for the user in /etc/passwd. The
 *	field may only contain a '!'. If anything else it will be replaced 
 *	with a '!', and be the field transferred to /etc/security/passwd.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *  	user process
 *
 * RETURNS: changes the new /etc/passwd table if error is found
 *	    0 = success(no errors in /etc/passwd),  
 *	    1 = an error was found
 */
int
ckpwfield(struct pwfile	*tabptr)
{
	char	*pwp,*newp,*cp;
	char	newpwline[PWLINESIZE+2];
	char	*strchr();

	/* skip to the
	 * password field (past first colon)
	 */

	pwp = strchr(tabptr->pwline,':');
	pwp++;

	/* if we have ":!:" 
	 * then we're ok
	 */

	if ( (*pwp == '!') && (*(pwp+1) == ':') )
	{
		tabptr->passwd = (char *)NULL;
		return(0);	
	}

	/* password field is not ok, 
	 * fix it if necessary 
	 */

	mp = MSGSTR(MBADP,DBADP);
	fxp = MSGSTR(FBADP,DFBADP);
	if( !report(mp, ABADP, fxp, tabptr->user, FIX) )
		return(-1); 	
	
	/* now let's fix this /etc/passwd line 
	 * by replacing whatever is between the colons with a '!'.
	 * Use newpwline to hold the new information.
	 * also extract the passwd field to tranfser it later.
	 */

	pwp = tabptr->pwline;	/* point to the password line 	*/
	newp = newpwline;	/* point to new memory		*/

	while( *pwp != ':' ) 	/* first copy up to the first colon */
		*newp++ = *pwp++;

	*newp++ = *pwp++;	/* copy the colon 		*/
	*newp++ = '!';		/* add the new passwd field 	*/
	*newp = '\0';		/* that's it for newp		*/

	cp = pwp;		/* mark the beginning of the passwd field */
        pwp = strchr(pwp,':');	/* advance past the passwd field	  */

	/* save the password field in the table
	 * so ckauth() can transfer it to 
	 * /etc/security/passwd later 
	 */

	tabptr->passwd = pwmalloc (pwp - cp + 1);
	strncpy (tabptr->passwd, cp, pwp-cp);
	tabptr->passwd[pwp - cp] = '\0'; /* terminate the password */

	/* copy the rest of the pwline 
	 * (after the passwd field) 
	 */

	strcat(newp,pwp);
	strcpy(tabptr->pwline, newpwline);	/* store the new passwd line */

	return(0);
}

/*
 * NAME: ckuserpw
 *                                                                    
 * FUNCTION: verifies the userpw fields of /etc/security/passwd
 *	     for the user specified in userp: 
 * 	     password,flags,lastupdate
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 */  
int
ckuserpw(
struct	pwfile	*ptabp )	/* pw structure of user to check 	*/
{
	struct	pwdck	**pwp;		/* pointer to sptab entries 	*/

	/* find the user's entry in 
	 * /etc/security/passwd (sptab[]). */

	for (pwp = sptab; *pwp != NULL; pwp++)
	{
		if ( strcmp(ptabp->user,(*pwp)->upw.upw_name) == 0)
		{
			(*pwp)->check = 1; /* check this stanza */
			break;
		}
	}

	/* if user doesn't exist in 
	 * /etc/security/passwd, just continue, 
	 * no need to validate.  
	 */

	if(*pwp == NULL)
		return(-1);

	(void) ckpassword(*pwp,ptabp);

	(void) ckflags(*pwp);

	(void) cklastupdate(*pwp);
	
	return(0);
}

/*
 * NAME: ckpassword
 *                                                                    
 * FUNCTION:  checks the password entry: 
 *                                                                    
 * if there's a password in /etc/passwd,
 * 	set 'password = the /etc/passwd field' and
 *	set lastupdate.	
 *		(whether passwds are required or not)
 *	return(OK).
 *
 * if password = NULL or missing, and passwords are not required,
 * 	then return OK. 
 * else if passwords are required
 *	set 'password = *' and report it.
 *
 * NOTE: don't report it if we set the password to the field in /etc/passwd;
 *	 it's been reported in ckpwfield().
 *  
 * EXECUTION ENVIRONMENT: 
 *
 * RETURNS:
 * 		0 = password entry is now ok.
 *		-1 = password entry is still wrong.
 */  
static int
ckpassword(
struct 	pwdck	*sptabp,	/* pointer to sptab[] entry	*/
struct	pwfile	*ptabp )	/* pointer to ptab[] entry	*/
{
	int	exists;		/* 'password =' entry exists 		*/
	int	nullpasswd;	/* 'password = NULL'			*/
	int	transfer;	/* transfer passwd from /etc/passwd 	*/
	int	required;	/* passwords are required		*/

	/* set up condition flags */

	exists = sptabp->password;		 
	nullpasswd = (exists ? (sptabp->upw.upw_passwd[0] == '\0') : 0); 
	transfer = (ptabp->passwd != NULL);
	required = ( pwdreq && !((sptabp->upw.upw_flags) & PW_NOCHECK) );

	/* if there's a valid password entry, return OK.
	 * valid => password entry exists AND it is not == NULL 
	 * AND the /etc/passwd password field needs no transfer to
	 * /etc/security/passwd
	 * NOTE: if the passwd field to transfer is blank, the pointer
	 * 	 exists (non-null) but contains a '\0' as it's first character
	 */

	if (exists && !nullpasswd && !transfer)
		return(0);

	/* transfer the password from 
	 * /etc/passwd 
	 */

	if (transfer)
		/* copy over the passwd field */
	{
		sptabp->password = 1; /* the password attribute now exists */

		/* if it's blank and passwords are required, make it a *.
		 * We'll need it later for ckauth() to call buildsecpw() 
		 */

		if ( (*(ptabp->passwd) == '\0') && required)
		{
			if (ptabp->passwd)
				pwfree (ptabp->passwd);
			ptabp->passwd = pwdup ("*");

			if (sptabp->upw.upw_passwd)
				pwfree (sptabp->upw.upw_passwd);
			sptabp->upw.upw_passwd = pwdup (ptabp->passwd);
		}
		else 
			/* just copy the password from /etc/passwd */
		{
			sptabp->upw.upw_passwd = pwdup (ptabp->passwd);

			/* return if we now have 
			 * a valid password field */

			if (*(sptabp->upw.upw_passwd) != '\0') 
			{
				/* set this here so cklastupdate doesn't 
				 * report an error */

				sptabp->upw.upw_lastupdate = time((time_t *) 0);
				sptabp->lastupdate = 1; 	      
				return(0);
			}
		}

		/* reset the conditions 
		 * to validate the password from 
		 * /etc/passwd */

		exists = sptabp->password;
		nullpasswd = 1; 
	}

	/* return good if password non-existent or NULL and
	 * they aren't required
	 */

	if ((!exists || nullpasswd) && !required)
		return(0);

	/* if we're replacing a password from /etc/passwd, 
	 * don't report it. Fix it if necessary.
	 */

	if(!transfer)
	{
		mp = MSGSTR(MNPW,DNPW);
		fxp = MSGSTR(FNPW,DFNPW);
	 	if (!report(mp, ANPW, fxp, sptabp->upw.upw_name, FIX))
			return(-1);
	}

	/* now fix it */

	sptabp->upw.upw_passwd = pwdup ("*");

	/* the password attribute 
	 * now exists */

	sptabp->password = 1;

	/* ok */

	return(0);
}

/*
 * NAME: ckflags
 *                                                                    
 * FUNCTION: checks the flags field. It must contain only the strings
 * 		ADMIN,ADMCHG, or NOCHECK. 
 * NOTES: 
 *	The 'invalid_flags' entry in struct pwdck indicates that 
 *	an invalid flag(s) was found (by convflags()).
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 * RETURNS:
 * 		0 = flags entry is now ok.
 *		-1 = flags entry is still wrong.
 *
 */  
static int
ckflags(
struct	pwdck	*spwent )	/* pointer to sptab[] entry	*/
{
	/* if the flags attribute doesn't exist, 
	 * don't check anything.
	 * But, if the flags attribute exists, it can only be 
	 * ADMIN,ADMCHG,NOCHECK. Clear out any other flags.
	 */

	if (!(spwent->flags))
		return(0);


	/* check for invalid flags; this was set 
	 * in convflags() in buildsecpw() */

	if (spwent->invalid_flags)	
	{
		/* there are invalid flags.
		 * fix it if necessary. */

		mp = MSGSTR(MFLG,DFLG);
		fxp = MSGSTR(FFLG,DFFLG);
		if(!report(mp, AFLG, fxp, spwent->upw.upw_name, FIX))
			return(-1);

		/* let's fix it. 
		 * Delete any other flags */

		spwent->upw.upw_flags &= (PW_NOCHECK | PW_ADMCHG | PW_ADMIN);
	}

	/* ok */

	return(0);
}

/*
 * NAME: cklastupdate
 *                                                                    
 * FUNCTION: 
 *		if password = doesn't exist, or *, or blank, remove lastupdate
 *							(if lastupdate exists).
 *
 * 	  	if 'password =' field is non-blank, lastupdate is required.
 *
 *		if lastupdate exists,
 *			check that the lastupdate field is less than 
 *			the current time.
 *		else
 *			create it.
 *
 *
 * EXECUTION ENVIRONMENT: 
 *
 * RETURNS:
 * 		0  = lastupdate entry is now ok.
 *		-1 = lastupdate entry is still wrong.
 *
 */  
static int
cklastupdate(
struct	pwdck	*sptabp )	/* pointer to sptab[] entry		*/
{
	int	nullpasswd;	/* flag indicating password = NULL 	*/
	int	splat;		/* flag indicating password = *		*/

	/* set up conditions */

	nullpasswd = sptabp->password ? (sptabp->upw.upw_passwd[0] == '\0') : 0;
	splat = (strcmp(sptabp->upw.upw_passwd,"*") == 0);

	/* remove lastupdate 
	 * if necessary */

	if ( !(sptabp->password) || nullpasswd || splat )
	{
	    if (sptabp->lastupdate)
	    {
		mp = MSGSTR(MLST,DLST);
		fxp = MSGSTR(FLST1,DFLST1);
		if(!report(mp, ALST, fxp, sptabp->upw.upw_name, FIX))
			return(-1);

		/* now remove it */

	        sptabp->lastupdate = 0;
	        sptabp->upw.upw_lastupdate = 0;
	    }

	    return(0);
	}

	/* a non-blank password exists, therefore
	 * lastupdate is now required. 
	 * create it if it doesn't exist */

	if (!(sptabp->lastupdate))
	{
		mp = MSGSTR(MMIS,DMIS);
		fxp = MSGSTR(FMIS,DFMIS);
		if(!report(mp, AMIS, fxp, sptabp->upw.upw_name, FIX))
			return(-1);

		/* now add it */

		sptabp->upw.upw_lastupdate = time ((time_t *) 0);
		sptabp->lastupdate = 1;	/* mark it */

		return(0);
	}

	/* if lastupdate == 0 OR is greater than the current time,
	 * then set it to the current time.
	 */

	if( (sptabp->upw.upw_lastupdate == 0) || (sptabp->upw.upw_lastupdate > time((time_t *)0)) )
	{
		mp = MSGSTR(MLST,DLST);
		fxp = MSGSTR(FLST2,DFLST2);
		if(!report(mp, ALST, fxp, sptabp->upw.upw_name, FIX))
			return(-1);

		/* now fix it */

		sptabp->upw.upw_lastupdate = time ((time_t *) 0);
		sptabp->lastupdate = 1;	/* mark it */
	}

	/* ok */

	return(0);
}
