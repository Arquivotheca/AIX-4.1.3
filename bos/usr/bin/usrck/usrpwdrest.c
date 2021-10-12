static char sccsid[] = "@(#)87	1.2  src/bos/usr/bin/usrck/usrpwdrest.c, cmdsadm, bos411, 9428A410j 11/3/93 15:11:19";
/*
 *   COMPONENT_NAME: CMDSADM
 *
 *   FUNCTIONS: FileAndHasAccess
 *		ck_dictionlist
 *		ck_pwdchecks
 *		ck_pwdrestrictions
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include	<stdlib.h>
#include	<userpw.h>
#include	<usersec.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<string.h>
#include	<sys/access.h>
#include	<sys/errno.h>
#include	<sys/audit.h>
#include	"usrck.h"
#include	"usrck_msg.h"


extern	int	error;
extern	int	verbose;
extern	int	accessx(char *, int, int);

enum Boolean
{
	False, True
};

typedef	struct	pwdrest
{
	char	*name;		/* getuserattr() name value.	*/
	int	minimum;	/* Minimum possible value.	*/
	int	maximum;	/* Maximum possible value.	*/
	int	value;		/* Actual value.		*/
	int	valid;		/* Is value valid?  True/False	*/
} PWDRest;


/*
 * IntRestrict - Integer based password Restrictions.
 *
 *   Note: IntRestrictOrder and IntRestrict[] must contain the same
 *         number of entries and have identical ordering.
 */
enum IntRestrictOrder
{
	IRMinAge,   IRMaxAge,   IRMaxExp, IRMinAlpha, IRMinOther,
	IRMinLen,   IRMinDiff,  IRMaxRep, IRHistSize, IRHistExpire
};

static	PWDRest	IntRestrict[] =
{ /*	Name:		Minimum:	Maximum:      Value:	Valid:	*/
 {	S_MINAGE,	MIN_MINAGE,	MAX_MINAGE,	0,	False	},
 {	S_MAXAGE,	MIN_MAXAGE,	MAX_MAXAGE,	0,	False	},
 {	S_MAXEXPIRED,	MIN_MAXEXP,	MAX_MAXEXP,	0,	False	},
 {	S_MINALPHA,	MIN_MINALPHA,	MAX_MINALPHA,	0,	False	},
 {	S_MINOTHER,	MIN_MINOTHER,	MAX_MINOTHER,	0,	False	},
 {	S_MINLEN,	MIN_MINLEN,	MAX_MINLEN,	0,	False	},
 {	S_MINDIFF,	MIN_MINDIFF,	MAX_MINDIFF,	0,	False	},
 {	S_MAXREPEAT,	MIN_MAXREP,	MAX_MAXREP,	0,	False	},
 {	S_HISTSIZE,	MIN_HISTSIZE,	MAX_HISTSIZE,	0,	False	},
 {	S_HISTEXPIRE,	MIN_HISTEXPIRE,	MAX_HISTEXPIRE,	0,	False	}
};

const	int	IntRestrictSize = sizeof(IntRestrict)/sizeof(PWDRest);


/*
 * Prototypes.
 */
static	int	ck_dictionlist(char *);
static	int	ck_pwdchecks  (char *);
static	int	FileAndHasAccess(char *, char *, char *, int, int);


/*
 * NAME:     ck_pwdrestrictions
 *
 * FUNCTION: Check all of the password attributes.
 *
 * RETURNS:  0        - No errors exist.
 *           non-zero - Errors exist.
 */
int
ck_pwdrestrictions(char *uname)
{
	int	GenFailureRecord;	/* Generate an audit failure record */
	int	user_errors = 0;
	int	Value;
	int	i;


	/*
	 * Check if each password restriction's value contained in IntRestrict[]
	 * is in range for this user.
	 */
	for (i=0; i < IntRestrictSize; i++)
	{
		IntRestrict[i].value = 0;
		IntRestrict[i].valid = False;

		if (getuserattr(uname, IntRestrict[i].name, (void *) &Value,
								SEC_INT))
		{
			msg2(MSGSTR(M_BADGET, DEF_BADGET), uname,
							IntRestrict[i].name);
			continue;
		}

		IntRestrict[i].valid = True;
		IntRestrict[i].value = Value;

		/*
		 * Check if the value is less than the minimum.
		 */
		if (Value < IntRestrict[i].minimum)
		{
			user_errors++;
			error++;

			msg3(MSGSTR(M_PWDLOW, DEF_PWDLOW), uname,
				 IntRestrict[i].name, IntRestrict[i].minimum);

			if (ck_query(MSGSTR(M_FIXPWDLOW, DEF_FIXPWDLOW),
							IntRestrict[i].name))
			{
				Value = IntRestrict[i].minimum;
				if (!putuserattr(uname, IntRestrict[i].name,
						(void *) Value, SEC_INT))
				{
					mk_audit_rec(AUDIT_OK, uname,
						IntRestrict[i].name, Fixed);
					IntRestrict[i].value = Value;
					continue;
				}
				fprintf(stderr, MSGSTR(M_BADPUT, DEF_BADPUT),
						uname, IntRestrict[i].name);
			}
			mk_audit_rec(AUDIT_FAIL, uname, IntRestrict[i].name,
								NotFixed);
			IntRestrict[i].valid = False; /* Don't use this value */
		}
		else
		/*
		 * Check if the value is greater than the maximum.
		 */
		if (Value > IntRestrict[i].maximum)
		{
			user_errors++;
			error++;

			msg3(MSGSTR(M_PWDHIGH, DEF_PWDHIGH), uname,
				 IntRestrict[i].name, IntRestrict[i].maximum);

			if (ck_query(MSGSTR(M_FIXPWDHIGH, DEF_FIXPWDHIGH),
							IntRestrict[i].name))
			{
				Value = IntRestrict[i].maximum;
				if (!putuserattr(uname, IntRestrict[i].name,
						(void *) Value, SEC_INT))
				{
					mk_audit_rec(AUDIT_OK, uname,
						IntRestrict[i].name, Fixed);
					IntRestrict[i].value = Value;
					continue;
				}
				fprintf(stderr, MSGSTR(M_BADPUT, DEF_BADPUT),
						uname, IntRestrict[i].name);
			}
			mk_audit_rec(AUDIT_FAIL, uname, IntRestrict[i].name,
								NotFixed);
			IntRestrict[i].valid = False; /* Don't use this value */
		}
	}

	/*
	 * Check if minage is greater than maxage.
	 */
	if (IntRestrict[IRMinAge].valid && IntRestrict[IRMaxAge].valid &&
	    IntRestrict[IRMinAge].value >  IntRestrict[IRMaxAge].value)
	{
		user_errors++;
		error++;

		msg1(MSGSTR(M_MINAGE, DEF_MINAGE), uname);
		/*
		 * Reduce minage to be equal to maxage.
		 */
		GenFailureRecord = True;
		if (ck_query(MSGSTR(M_FIXMINAGE, DEF_FIXMINAGE), uname))
		{
			Value = IntRestrict[IRMaxAge].value;
			if (!putuserattr(uname, IntRestrict[IRMinAge].name,
						(void *) Value, SEC_INT))
			{
				mk_audit_rec(AUDIT_OK, uname,
					IntRestrict[IRMinAge].name, Fixed);
				IntRestrict[IRMinAge].value = Value;
				GenFailureRecord = False;
			}
			else
				fprintf(stderr, MSGSTR(M_BADPUT, DEF_BADPUT),
					uname, IntRestrict[IRMinAge].name);
		}
		if (GenFailureRecord)
		{
			mk_audit_rec(AUDIT_FAIL, uname,
					IntRestrict[IRMinAge].name, NotFixed);
		}
	}

	/*
	 * Check if minother is greater than 'PW_PASSLEN - minalpha'.
	 */
	if (IntRestrict[IRMinAlpha].valid && IntRestrict[IRMinOther].valid &&
	    IntRestrict[IRMinOther].value >
				(PW_PASSLEN - IntRestrict[IRMinAlpha].value))
	{
		user_errors++;
		error++;

		msg1(MSGSTR(M_MINOTHER, DEF_MINOTHER), uname);
		/*
		 * Reduce minother to be equal to 'PW_PASSLEN - minalpha'.
		 */
		GenFailureRecord = True;

		if (ck_query(MSGSTR(M_FIXMINOTHER, DEF_FIXMINOTHER), 0))
		{
			Value = PW_PASSLEN - IntRestrict[IRMinAlpha].value;
			if (!putuserattr(uname, IntRestrict[IRMinOther].name,
						(void *) Value, SEC_INT))
			{
				mk_audit_rec(AUDIT_OK, uname,
					IntRestrict[IRMinOther].name, Fixed);
				IntRestrict[IRMinOther].value = Value;
				GenFailureRecord = False;
			}
			else
				fprintf(stderr, MSGSTR(M_BADPUT, DEF_BADPUT),
					uname, IntRestrict[IRMinOther].name);
		}
		if (GenFailureRecord)
		{
			mk_audit_rec(AUDIT_FAIL, uname,
					IntRestrict[IRMinOther].name, NotFixed);
		}
	}

	/*
	 * Check the values contained in dictionlist.
	 */
	if (ck_dictionlist(uname))
	{
		user_errors++;
		error++;
	}

	/*
	 * Check the values contained in pwdchecks.
	 */
	if (ck_pwdchecks(uname))
	{
		user_errors++;
		error++;
	}

	return(user_errors);
}


/*
 * NAME:     FileAndHasAccess
 *
 * FUNCTION: Checks if a file exists and if the invoker has access.
 *
 * RETURNS:  True  - If file exists and invoker has access.
 *           False - If file doesn't exist or invoker is denied access.
 */
static	int
FileAndHasAccess(	char	*uname,
			char	*Attribute,
			char	*FileName,
			int	mode,
			int	PrintMsg)
{
	struct	stat	  sbuf;
	int	rc = True;

	if (stat(FileName, &sbuf) || (sbuf.st_mode & S_IFMT) != S_IFREG)
	{
		if (PrintMsg)
			msg3(MSGSTR(M_BADFILE, DEF_BADFILE), uname, Attribute,
								FileName);
		rc = False;
	}
	else
	if (accessx(FileName, mode, ACC_SELF))
	{
		if (PrintMsg)
			msg3(MSGSTR(M_NOACCESS, DEF_NOACCESS), uname, Attribute,
								FileName);
		rc = False;
	}
	return(rc);
}


/*
 * NAME:     ck_dictionlist
 *
 * FUNCTION: Check the "dictionlist" file names associated with this user.
 *
 * RETURNS:  0        - OK.
 *           non-zero - "dictionlist" contains invalid entries.
 */
static	int
ck_dictionlist(char *uname)
{
	char	*List,    *Entry;
	char	*NewList, *NewEntry;
	int	Changed = False;
	int	Failed;
	int	Size;
	int	Errors = 0;
	int	rc = 0;


	/*
	 * Get the dictionary list for this user.
	 */
	if (getuserattr(uname, S_DICTION, (void *) &List, SEC_LIST))
	{
		msg2(MSGSTR(M_BADGET, DEF_BADGET), uname, S_DICTION);
		return(-1);
	}
	/* If the list is empty, then don't process. */
	if (!List || !(*List))
		return(0);

	/*
	 * Determine the size of the List DNL.
	 */
	for (Entry = List, Size = 0; *Entry;)
		do {
			Size++;
		} while (*Entry++);
	Size++;

	/*
	 * Create space for a new list.
	 */
	if (!(NewList = (char *) malloc(Size)))
	{
		fprintf(stderr, MSGSTR (M_NOMEM, DEF_NOMEM));
		exit (ENOMEM);
	}
	NewList[0] = NewList[1] = '\0';		/* Initialize as an empty DNL */
	NewEntry   = NewList;

	/*
	 * Check each file name contained in List.
	 */
	for (Entry = List; *Entry;)
	{
		Failed = False;
		if (*Entry != '/')
		{
			msg3(MSGSTR(M_ABSPATH, DEF_ABSPATH), uname, S_DICTION,
									Entry);
			Failed = True;
		}
		else
		if (!FileAndHasAccess(uname, S_DICTION, Entry, R_OK, True))
			Failed = True;


		if (Failed)
		{
			rc = 1;		/* Notify the calling subroutine */

			if (ck_query(MSGSTR(M_FIXFILE, DEF_FIXFILE), S_DICTION))
			{
				while (*Entry++);	/* Ignore entry */
				Changed = True;
			}
			else
			{
				/*
				 * Keep the bad entry.  Increment Errors to
				 * note that a bad entry exists.
				 */
				while (*NewEntry++ = *Entry++);
				Errors++;
			}
		}
		else
			while (*NewEntry++ = *Entry++);	/* Keep entry */
	}
	*NewEntry = '\0';	/* Add the terminating null for the DNL. */

	if (Changed)
	{
		if (!putuserattr(uname, S_DICTION, (void *) NewList, SEC_LIST))
		{
			/*
			 * If we didn't correct the entire entry, then don't
			 * say that the new entry is fixed.
			 */
			if (Errors)
				mk_audit_rec(AUDIT_OK, uname, S_DICTION,
								PartiallyFixed);
			else
				mk_audit_rec(AUDIT_OK, uname, S_DICTION, Fixed);
		}
		else
		{
			fprintf(stderr, MSGSTR(M_BADPUT, DEF_BADPUT), uname,
								S_DICTION);
			mk_audit_rec(AUDIT_FAIL, uname, S_DICTION, CantFix);
		}
	}
	else
	if (Errors)
	{
		mk_audit_rec(AUDIT_FAIL, uname, S_DICTION, NotFixed);
	}
	free(NewList);

	return(rc);
}


/*
 * NAME:     ck_pwdchecks
 *
 * FUNCTION: Check the "pwdchecks" file names associated with this user.
 *
 * RETURNS:  0        - OK.
 *           non-zero - "pwdchecks" contains invalid entries.
 */
static	int
ck_pwdchecks(char *uname)
{
	char	*List,    *Entry;
	char	*NewList, *NewEntry;
	int	Changed = False;
	int	Failed;
	int	Size;
	int	Errors = 0;
	int	rc = 0;


	/*
	 * Get the pwdchecks method names for this user.
	 */
	if (getuserattr(uname, S_PWDCHECKS, (void *) &List, SEC_LIST))
	{
		msg2(MSGSTR(M_BADGET, DEF_BADGET), uname, S_PWDCHECKS);
		return(-1);
	}
	/* If the list is empty, then don't process. */
	if (!List || !(*List))
		return(0);

	/*
	 * Determine the size of the List DNL.
	 */
	for (Entry = List, Size = 0; *Entry;)
		do {
			Size++;
		} while (*Entry++);
	Size++;

	/*
	 * Create space for a new list.
	 */
	if (!(NewList = (char *) malloc(Size)))
	{
		fprintf(stderr, MSGSTR (M_NOMEM, DEF_NOMEM));
		exit (ENOMEM);
	}
	NewList[0] = NewList[1] = '\0';		/* Initialize as an empty DNL */
	NewEntry   = NewList;

	/*
	 * Check each file name contained in List.
	 */
	for (Entry = List; *Entry;)
	{
		Failed = True;
		if (*Entry == '/')
		{
			if (FileAndHasAccess(uname, S_PWDCHECKS, Entry, X_OK,
									True))
				Failed = False;
		}
		else
		{
			char	*libpath;
			char	*relpath;
			char	*lp;
			char	*q;
			int	len   = strlen(Entry);

			/* Check if path name is relative to PWDCHECKS_LIBPATH*/
			if (!(libpath = strdup(PWDCHECKS_LIBPATH)))
			{
				fprintf(stderr, MSGSTR(M_NOMEM, DEF_NOMEM));
				exit(ENOMEM);
			}

			/*
			 * Parse the colon separated PWDCHECKS_LIBPATH
			 * appending the file name to each entry found in
			 * PWDCHECKS_LIBPATH.
			 */
			for (q = libpath; lp = strtok(q, ":"); q = (char *)NULL)
			{
				if (!(relpath = (char *)
						malloc(len + strlen(lp) + 2)))
				{
					fprintf(stderr,
						MSGSTR(M_NOMEM, DEF_NOMEM));
					exit(ENOMEM);
				}
				strcpy(relpath, lp);	/* LIBPATH  */
				strcat(relpath, "/");	/* slash    */
				strcat(relpath, Entry);	/* Filename */

				if (FileAndHasAccess(uname, S_PWDCHECKS,
							relpath, X_OK, False))
				{
					Failed = False;
					free((void *) relpath);
					break;
				}
				free((void *) relpath);
			}
			free((void *) libpath);

			if (Failed)
				msg3(MSGSTR(M_NOACCESS, DEF_NOACCESS), uname, 
							S_PWDCHECKS, Entry);
		}


		if (Failed)
		{
			rc = 1;		/* Notify the calling subroutine */

			if (ck_query(MSGSTR(M_FIXFILE,DEF_FIXFILE),S_PWDCHECKS))
			{
				while (*Entry++);	/* Ignore entry */
				Changed = True;
			}
			else
			{
				/*
				 * Keep the bad entry.  Increment Errors to
				 * note that a bad entry exists.
				 */
				while (*NewEntry++ = *Entry++);
				Errors++;
			}
		}
		else
			while (*NewEntry++ = *Entry++);	/* Keep entry */
	}
	*NewEntry = '\0';	/* Add the terminating null for the DNL. */

	if (Changed)
	{
		if (!putuserattr(uname, S_PWDCHECKS, (void *)NewList,SEC_LIST))
		{
			/*
			 * If we didn't correct the entire entry, then don't
			 * say that the new entry is fixed.
			 */
			if (Errors)
				mk_audit_rec(AUDIT_OK, uname,S_PWDCHECKS,
								PartiallyFixed);
			else
				mk_audit_rec(AUDIT_OK, uname,S_PWDCHECKS,Fixed);
		}
		else
		{
			fprintf(stderr, MSGSTR(M_BADPUT, DEF_BADPUT), uname,
								S_PWDCHECKS);
			mk_audit_rec(AUDIT_FAIL, uname, S_PWDCHECKS, CantFix);
		}
	}
	else
	if (Errors)
	{
		mk_audit_rec(AUDIT_FAIL, uname, S_PWDCHECKS, NotFixed);
	}
	free(NewList);

	return(rc);
}
