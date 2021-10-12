static char sccsid[] = "@(#)02  1.9  src/bos/usr/bin/pwdck/pwdwrite.c, cmdsadm, bos411, 9428A410j 8/7/91 15:11:44";
/*
 * COMPONENT_NAME: (TCBADM) Trusted System Adminstration
 *
 * FUNCTIONS: writepw()
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
#include <sys/types.h>
#include <sys/audit.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
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
extern 	int 	errno;
extern	char 	*tmpep;
extern	char 	*tmpesp;
extern	char	*mp;		/* pointer to message strings */

/*
 * NAME: writepw 
 *
 * FUNCTION: 
 * 	write out the /etc/passwd table (ptab[]) to /etc/passwd.
 * 	write out the /etc/security/passwd table (sptab[]) to /etc/passwd.
 *                                                                    
 * NOTES:
 * 	both tables are written out to temp files first. If everything
 *	succeeds, then we'll rename the temp files to the real ones.
 *
 *	If anything fails, we'll unlink the temp files.
 *
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: 
 *	0 = files written ok.
 *	-1 = files not written.
 */
int
writepw(int epfd, 	/* fildes for /etc/passwd 		*/
int espfd)		/* fildes for /etc/security/passwd 	*/
{
	int	i;
	char	pwline[PWLINESIZE+1];	/* space to hold each pw line	    */
	struct	pwfile	**ptabp;	/* pointer to ptab[]		    */
	struct	pwdck	**pwp;		/* pointer to sptab[]		    */
	struct	secpwline *cur;		/* pointer to line in secpw stanza  */
	struct	stat	statbuf;	/* to get uid and gid		    */
	struct	pcl	*pclp;
	FILE	*tmpepfp;		/* file pointer for temp /etc/passwd*/
	FILE	*tmpespfp;		/* pointer for temp /etc/security/pw*/
	int	tmpepfd;		/* fildes for temp /etc/passwd	    */
	int	tmpespfd;		/* fildes for temp /etc/security/pw */
	char	*aclp,*acl_get();

	/* write out the /etc/passwd table first.  
	 */

	/* make a temp name in /etc */
	mktemp(tmpep);
	if (*tmpep == (int)NULL)
	{
		mp = MSGSTR(MMKTEMP,DMKTEMP);
		pwexit(mp, AMKTEMP, (char *)NULL);
	}

	/* create the temp file */
	if ((tmpepfp = fopen(tmpep,"w"))==NULL) 
	{
		mp = MSGSTR(MOPENTEP,DOPENTEP);
		pwexit(mp, AOPENTEP, (char *)NULL);
	}

	/* get the uid and gid */
	if (stat(PASSFILE, &statbuf) != 0)
	{
	       	unlinktmp(tmpep,(char *)NULL);	/* unlink temp file */
		mp = MSGSTR(MSTAT,DSTAT);
		pwexit(mp, ASTAT, PASSFILE);
	}

	privilege(PRIV_ACQUIRE);

	if (chown(tmpep, statbuf.st_uid, statbuf.st_gid) != 0)
	{
	       	unlinktmp(tmpep,(char *)NULL);	/* unlink temp file */
		mp = MSGSTR(MCHOWN,DCHOWN);
		pwexit(mp, ACHOWN, tmpep);
	}

	privilege(PRIV_LAPSE);

	/* set it with the access rights 
	 * from /etc/passwd 
	 */
	if ((aclp = acl_get(PASSFILE)) == NULL)
	{
		unlinktmp(tmpep,(char *)NULL);	/* unlink temp file */
		mp = MSGSTR(MACLGET,DACLGET);
		pwexit(mp, AACLGET, PASSFILE);
	}

	if (acl_put(tmpep,aclp,1) == -1)
	{
		unlinktmp(tmpep,(char *)NULL);	/* unlink temp file */
		mp = MSGSTR(MACLPUT,DACLPUT);
		pwexit(mp, AACLPUT, tmpep);
	}

	/* now start writing into it.
	 * If the line is marked delete, 
	 * don't write it out */

	for (ptabp = &ptab[0]; (*ptabp != NULL); ptabp++)
	{
		if ((*ptabp)->delete)
			continue;

		if(fputs((*ptabp)->pwline,tmpepfp) == EOF)
		{
	       		unlinktmp(tmpep,(char *)NULL);	/* unlink temp file */
			mp = MSGSTR(MWRITE,DWRITE);
			pwexit(mp, AWRITE, tmpep);
		}
		pwfree((void *)(*ptabp)->pwline);
		pwfree((void *)(*ptabp)->user);
		pwfree((void *)(*ptabp)->passwd);

		pwfree((void *) *ptabp);
	}
		
	if (fclose(tmpepfp) == EOF)
	{
		unlinktmp(tmpep,(char *)NULL);	/* unlink temp file */
		mp = MSGSTR(MWRITE,DWRITE);
		pwexit(mp, AWRITE, tmpep);
	}
	
	/* set it with the pcl 
	 * from /etc/passwd 
	 */
	if ((pclp = priv_get(PASSFILE)) == NULL)
	{
		unlinktmp(tmpep,(char *)NULL);	/* unlink temp file */
		mp = MSGSTR(MPCLGET,DPCLGET);
		pwexit(mp, APCLGET, PASSFILE);
	}

	if (priv_put(tmpep,pclp,1) != 0)
	{
		unlinktmp(tmpep,(char *)NULL);	/* unlink temp file */
		mp = MSGSTR(MPCLPUT,DPCLPUT);
		pwexit(mp, APCLPUT, tmpep);
	}
	
	/* now write out the /etc/security/passwd 
	 * table. */

	/* make a temp name in /etc/security */
	mktemp(tmpesp);
	if (*tmpesp == (int)NULL)
	{
	       	unlinktmp(tmpep,(char *)NULL);	/* unlink temp file */
		mp = MSGSTR(MMKTEMP,DMKTEMP);
		pwexit(mp, AMKTEMP, (char *)NULL);
	}

	/* create the temp file */
	if ((tmpespfp = fopen(tmpesp,"w"))==NULL) 
	{
	       	unlinktmp(tmpep,(char *)NULL);	/* unlink temp file */
		mp = MSGSTR(MOPENTESP,DOPENTESP);
		pwexit(mp, AOPENTESP, (char *)NULL);
	}

	/* get the uid and gid */
	if (stat(SPASSFILE, &statbuf) != 0)
	{
	       	unlinktmp(tmpep,tmpesp);	/* unlink temp files */
		mp = MSGSTR(MSTAT,DSTAT);
		pwexit(mp, ASTAT, SPASSFILE);
	}

	privilege(PRIV_ACQUIRE);

	if (chown(tmpesp, statbuf.st_uid, statbuf.st_gid) != 0)
	{
	       	unlinktmp(tmpep,tmpesp);	/* unlink temp files */
		mp = MSGSTR(MCHOWN,DCHOWN);
		pwexit(mp, ACHOWN, tmpesp);
	}

	privilege(PRIV_LAPSE);

	/* set it with the access rights 
	 * from /etc/security/passwd 
	 */
	if ((aclp = acl_get(SPASSFILE)) == NULL)
	{
	      	unlinktmp(tmpep,tmpesp);		/* unlink temp files */
		mp = MSGSTR(MACLGET,DACLGET);
		pwexit(mp, AACLGET, SPASSFILE);
	}

	if (acl_put(tmpesp,aclp,1) == -1)
	{
	      	unlinktmp(tmpep,tmpesp);		/* unlink temp files */
		mp = MSGSTR(MACLPUT,DACLPUT);
		pwexit(mp, AACLPUT, tmpesp);
	}

	/* now start writing into it */
	for(pwp = &sptab[0]; (*pwp != NULL); pwp++)
	{
		/* if this user is marked delete,
		 * then we will not store him.
		 */

		if ((*pwp)->delete)
			continue;

		for (cur = (*pwp)->lines;cur != 0;cur = cur->next) {
			if (cur->type == SPW_NAME) {
				/* store the stanza name */
				sprintf(pwline,"\n%s:\n", (*pwp)->upw.upw_name);
				if (fputs(pwline,tmpespfp) == EOF)
				{
					unlinktmp(tmpep,tmpesp);
					mp = MSGSTR(MWRITE,DWRITE);
					pwexit(mp, AWRITE, tmpesp);
				}
			} else if (cur->type == SPW_PASSWD) {
				/* store the password line */
				if ((*pwp)->password)
				{
					sprintf(pwline,"\t%s = %s\n",
					    PASSWORD,(*pwp)->upw.upw_passwd);

					if (fputs(pwline,tmpespfp) == EOF)
					{
						unlinktmp(tmpep,tmpesp);
						mp = MSGSTR(MWRITE,DWRITE);
						pwexit(mp, AWRITE, tmpesp);
					}
					pwfree((void *)(*pwp)->upw.upw_passwd); 
				}
			} else if (cur->type == SPW_LAST) {
				/* store the lastupdate line */
				if ((*pwp)->lastupdate)
				{
					sprintf(pwline,"\t%s = %u\n",
					    LASTUPDATE, (*pwp)->upw.upw_lastupdate);
					if (fputs(pwline,tmpespfp) == EOF)
					{
						unlinktmp(tmpep,tmpesp);
						mp = MSGSTR(MWRITE,DWRITE);
						pwexit(mp, AWRITE, tmpesp);
					}
				}
			} else if (cur->type == SPW_FLAGS) {
				/* store the flags line */
				if ((*pwp)->flags)
				{
					sprintf(pwline,"\t%s = ",FLAGS); 

					/* if this stanza wasn't checked,
					 * then restore the original flags 
					 * entry exactly as it first appeared */

					if (!(*pwp)->check)
					{
						strcat(pwline, (*pwp)->raw_flags);
					}
					else
						/* store the fixed entry */
					{

						i = 0;	/* to control comma placement */
						if ((*pwp)->upw.upw_flags & PW_NOCHECK)
						{
							strcat(pwline,"NOCHECK");
							i++;
						}

						if ((*pwp)->upw.upw_flags & PW_ADMCHG)
						{
							if (i++)
								strcat(pwline,",");
							strcat(pwline,"ADMCHG");
						}

						if ((*pwp)->upw.upw_flags & PW_ADMIN)
						{
							if (i)
								strcat(pwline,",");
							strcat(pwline,"ADMIN");
						}
					}

					/* finish it with the newline */
					strcat(pwline,"\n");

					if (fputs(pwline,tmpespfp) == EOF)
					{
						unlinktmp(tmpep,tmpesp);/* unlink temp files */
						mp = MSGSTR(MWRITE,DWRITE);
						pwexit(mp, AWRITE, tmpesp);
					}

					pwfree((void *)(*pwp)->raw_flags);
				}
			} else if (cur->type == SPW_COMMENT) {
				if (fputs (cur->line, tmpespfp) == EOF) {
					unlinktmp (tmpep, tmpesp);
					mp = MSGSTR (MWRITE, DWRITE);
					pwexit (mp, AWRITE, tmpesp);
				}
			}
		}
		pwfree((void *) *pwp);
	}

	/* terminate the security/passwd file
	 * with a newline
	 */
	sprintf(pwline,"\n");
	if (fputs(pwline,tmpespfp) == EOF)
	{
		unlinktmp(tmpep,tmpesp);	/* unlink temp files */
		mp = MSGSTR(MWRITE,DWRITE);
		pwexit(mp, AWRITE, tmpesp);
	}

	if (fclose(tmpespfp) == EOF)
	{
		unlinktmp(tmpep,tmpesp);	/* unlink temp files */
		mp = MSGSTR(MWRITE,DWRITE);
		pwexit(mp, AWRITE, tmpesp);
	}

	/* set it with the pcl 
	 * from /etc/security/passwd 
	 */
	if ((pclp = priv_get(SPASSFILE)) == NULL)
	{
	      	unlinktmp(tmpep,tmpesp);		/* unlink temp files */
		mp = MSGSTR(MPCLGET,DPCLGET);
		pwexit(mp, APCLGET, SPASSFILE);
	}

	if (priv_put(tmpesp,pclp,1) != 0)
	{
		unlinktmp(tmpep,tmpesp);		/* unlink temp files */
		mp = MSGSTR(MPCLPUT,DPCLPUT);
		pwexit(mp, APCLPUT, tmpesp);
	}


	/* copy the temporary password files to the
	 * real ones now */

	if ((tmpepfd = open(tmpep, O_RDONLY)) == -1)
	{
		unlinktmp(tmpep,tmpesp);	/* unlink temp files */
		mp = MSGSTR(MOPENTEP,DOPENTEP);
		pwexit(mp, AOPENTEP, (char *)NULL);
	}

	if ((tmpespfd = open(tmpesp, O_RDONLY)) == -1)
	{
		unlinktmp(tmpep,tmpesp);	/* unlink temp files */
		mp = MSGSTR(MOPENTESP,DOPENTESP);
		pwexit(mp, AOPENTESP, (char *)NULL);
	}

	if((pwcopy(tmpepfd,epfd) == -1) || (pwcopy(tmpespfd,espfd) == -1))
	{
		unlinktmp(tmpep,tmpesp);	/* unlink temp files */
		mp = MSGSTR(MCOPYTEMP,DCOPYTEMP);
		pwexit(mp, ACOPYTEMP, (char *)NULL);
	}

	close(tmpepfd);
	close(tmpespfd);

	unlinktmp(tmpep,tmpesp);	/* unlink temp files */
	
	pwfree((void *) ptab);
	pwfree((void *) sptab);

	return 0;
}
