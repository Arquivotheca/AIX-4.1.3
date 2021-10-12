static char sccsid[] = "@(#)99  1.9  src/bos/usr/bin/pwdck/pwdckauth.c, cmdsadm, bos411, 9428A410j 7/30/91 13:28:04";
/*
 * COMPONENT_NAME: (TCBADM) Trusted System Adminstration
 *
 * FUNCTIONS: ckauth(),ckauthnames()
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
extern	int	totalusers;
extern	int 	nflag;
extern	char	*mp, *fxp;
extern 	int 	errno;

/*
 * NAME: ckauth 
 *
 * FUNCTION: 
 *	check: For each SYSTEM:authname stanza defined for a local user, 
 *	the authname must have an stanza in /etc/security/passwd.  
 *	fix: a missing stanza will be created.
 *
 * NOTES:
 *	If a user's entry and a 'default' entry are missing from 
 *	/etc/security/user, we assume:
 *		auth1 = SYSTEM;user
 *		auth2 = NONE
 *	Therefore, by default, every local user specified in /etc/passwd
 *	does have a corresponding authname.
 *
 * 	This function is just responsible for ADDING stanzas into 
 * 	/etc/security/passwd. ckauthnames() is responsible for DELETING
 *	entries. if !all, no entries will be deleted.
 *
 * EXECUTION ENVIRONMENT: 
 *                                                                   
 * RETURNS: 
 */
int
ckauth(struct	pwfile	*pwtp)
{
	int 	i;		/* auth1, auth2 loop control		     */
	char 	*s;		/* auth1, auth2 loop control		     */
	int	found;		/* flag to indicate we found the authname    */
	int	index;		/* index to sptab[]			     */
	char 	**list;		/* list of auth1 and auth2 authnames	     */
	char 	**lstmem;	/* pointer to beginning of list		     */
	char 	**authnames;	/* list of all auth names		     */
	char 	**authp;	/* pointer to authname list we loop through  */
	char 	*attrbuf;	/* to hold the attribute		     */
	struct	pwdck	**pwp;	/* pointer to sptab[] 			     */

	/* set up array 
	 * of authnames */

	authnames=(char **) pwmalloc(sizeof(char *)*(2*(totalusers+TABLESIZE)));

	authp = authnames;

	/* get auth1= and 
	 * auth2= entries */

	for (i=1; i<=2; i++)
	{
		(i==1) ? (s = S_AUTH1) : (s = S_AUTH2);

		/* get the AUTH string 
		 * for this user */

		if(getuserattr(pwtp->user,s,(void *)&attrbuf,SEC_LIST)==0) 
		{
			/* get a null-terminated list of principle 
			 * authnames from the auth entry (SYSTEM;authname).
			 */

			list = findsystem(attrbuf,pwtp->user);
		}
		else 
		{
			if ((errno != ENOATTR) && (errno != ENOENT))
			{
				/* error - no recovery */
				mp = MSGSTR(MATTR,DATTR);
			   	pwexit(mp, AATTR, pwtp->user);
			}	

			/*
			 * if the auth entry is missing for this user,
			 * then assume SYSTEM;user for auth1 and
			 * NONE for auth2
			 */

			list = (char **)pwmalloc(sizeof(char *)*2);

			if (i == 1)
			{
				*list = pwtp->user;
				*(list+1) = (char *)NULL;
			}
			else
			{
				*list = (char *)NULL;
			}
		}

		/* build the 
		 * combined list */

		lstmem = list;
		while(*list != NULL)
			*authp++ = *list++;

		pwfree (list);
	}

	/* terminate the list */

	*authp++ = (char *)NULL; 
	authnames = (char **) pwrealloc ((char *) authnames,
		    (unsigned int) (authp - authnames) * sizeof (char *));

	/*
	 * search through sptab[] to find the authnames and
	 * mark the corresponding user with 'principle=1'
	 */

	for(authp = authnames; *authp != NULL; authp++)
	{
		for (pwp = sptab;*pwp != 0;pwp++) {
			/* if name found, mark it, 
			 * and go on to next name. */

			if((*pwp)->delete == 0 &&
				strcmp(*authp,(*pwp)->upw.upw_name)==0)	
			{
				(*pwp)->principle=1;
				break;
			}
		}

		/* if we found the authname, 
		 * just continue */

		if (*pwp != NULL)
			continue;

		/* we've reached the end of the table with no match;
		 * this authname has no entry in /etc/security/passwd.
		 */

		for (i = 0;ptab[i];i++)
			if (strcmp (*authp, ptab[i]->user) == 0)
				break;

		mp = MSGSTR (MAUTH, DAUTH);
		fxp = MSGSTR(FAUTH,DFAUTH);
		if(!report(mp, AAUTH, fxp, *authp, ptab[i] ? FIX:NFIX))
		{
			if (ptab[i] == 0) {
				mp = MSGSTR (MNENT, DNENT);
				report (mp, ANENT, (char *) 0, *authp, NFIX);
			}
			pwfree((void *)authnames);
			pwfree((void *)lstmem);
			return(-1);
		}

		/*
		 * add this user to /etc/security/passwd (sptab[]). 
		 * if this user exists in /etc/passwd, transfer the 
		 * password field to /etc/security/passwd.
		 * Otherwise, force password = *.
	 	 */

		if (strcmp(*authp,pwtp->user) == 0)
			index = buildsecpw(ADD,*authp,pwtp->passwd, -1);
		else
			index = buildsecpw(ADD,*authp,(char *)NULL, -1);

		/* mark user as principle */

		sptab[index-1]->principle = 1; 
	}

	/* ok */
	pwfree((void *)authnames);
	pwfree((void *)lstmem);

	return(0);
}

/*
 * NAME: ckauthnames
 *
 * FUNCTION: 
 * 	check that each /etc/security/passwd stanza corresponds to the 
 *	authentication name of a local user as a SYSTEM;authname entry
 *	in /etc/security/user. 
 *	fix: if not found, the user's entry stanza be deleted from 
 *	/etc/security/passwd.
 *
 * NOTES:
 *	if an entry has a corresponding authname, it is considered a 
 * 	principal authname, and it's 'principle' flag
 *	will be set in the pwdck structure.
 *
 * EXECUTION ENVIRONMENT: 
 *                                                                   
 * RETURNS: 
 *	0 = authname found
 *	-1 = authname not found
 */
int
ckauthnames()
{
	struct	pwdck	**pwp;	/* pointer to sptab[] table		 */

	/*
	 * loop through each /etc/security/passwd entry
	 * and check if it's been marked as having a corresponding
	 * authname. If it hasn't, then mark this user 'delete'.
	 */

	for(pwp=&sptab[0];*pwp != NULL; pwp++)
	{
		/* no authname found, delete him 
		 * by setting the delete flag 
		 * to delete this stanza 
		 */

		if ( !((*pwp)->principle) )
		{
			mp = MSGSTR(MUAUTH,DUAUTH);
			fxp = MSGSTR(FUAUTH,DFUAUTH);
			if(!report(mp, AUAUTH, fxp, (*pwp)->upw.upw_name, FIX))
				continue;

			(*pwp)->delete = 1;
			if (!nflag)
			{
				mp = MSGSTR(MDEL,DDEL);
				report(mp,ADEL,(char *)NULL,(*pwp)->upw.upw_name,NFIX);
			}
		}
	} 

	/* ok */

	return(0);
}

static int
ckauthcmp(a, b)
void **a, **b;
{
	/* comparison function for ckauthorder */

	int i, j;
	struct pwfile **pwp;
	struct pwdck *sp1 = (struct pwdck *)*a,
		     *sp2 = (struct pwdck *)*b;

	/* determine the location of the matching lines in /etc/passwd */
	if (sp1->pwfile == NULL)
		return +1;
	for (i=0, pwp=ptab; *pwp != NULL && sp1->pwfile != *pwp; pwp++)
		i++;

	if (sp2->pwfile == NULL)
		return -1;
	for (j=0, pwp=ptab; *pwp != NULL && sp2->pwfile != *pwp; pwp++)
		j++;

	/* return the difference */
	return (i-j);
}

void
ckauthorder()
{
	/* orders the stanzas in /etc/security/passwd
	 * in the same order as lines in /etc/passwd
	 */

	struct pwdck **spwp, **cpwp, **pcopy;
	int i;

	/* count number of elements */
	for (i=0, spwp=sptab; *spwp != NULL; spwp++)
		i++;

	/* make a copy of the table, then sort it */
	pcopy = (struct pwdck **) pwmalloc(sizeof(struct pwdck *) * i + 1);
	for (cpwp=pcopy, spwp=sptab; *spwp != NULL; spwp++, cpwp++)
		*cpwp = *spwp;
	*cpwp = NULL;

	/* sort it */
	qsort((void *)pcopy, (size_t)i, sizeof(struct pwdck *), ckauthcmp);

	/* compare the sorted table with the original */
	for (cpwp=pcopy, spwp=sptab; *spwp != NULL; spwp++, cpwp++)
		if (*cpwp != *spwp)
			break;
	if (*spwp != NULL) {
		/* it changed, prompt user */
		fxp = MSGSTR(FSORT,DFSORT);
		mp = MSGSTR(MSORT,DSORT);
		if (report(mp, ASORT, fxp, NULL, FIX)) {
			/* copy the sorted copy over the real table */
			for (cpwp=pcopy, spwp=sptab; *spwp != NULL; )
			    *spwp++ = *cpwp++;
			mp = MSGSTR(MORDER,DORDER);
			(void) report(mp, AORDER, (char *)0, NULL, NFIX);
		}
	}
	return;
}
