static char sccsid[] = "@(#)86	1.14  src/bos/usr/lib/sendmail/alias.c, cmdsend, bos411, 9428A410j 3/16/93 09:57:44";
/* 
 * COMPONENT_NAME: CMDSEND alias.c
 * 
 * FUNCTIONS: MSGSTR, alias, aliaslookup, forward, openaliases, 
 *            readaliases 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
**  Sendmail
**  Copyright (c) 1983  Eric P. Allman
**  Berkeley, California
**
**  Copyright (c) 1983 Regents of the University of California.
**  All rights reserved.  The Berkeley software License Agreement
**  specifies the terms and conditions for redistribution.
**
*/


#include <nl_types.h>
#include "sendmail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SENDMAIL,n,s) 

# include <sys/stat.h>
# include <errno.h>
# include <stdio.h>
# include <ctype.h>
# include <fcntl.h>
# include <sys/lockf.h>
# include "conf.h"
# include "useful.h"
# include <netinet/in.h>

# include "sysexits.h"

# include "sendmail.h"

#ifdef _SUN
char *ypDomain = NULL;
char *yellowlookup();
#endif _SUN

# include <string.h>

char *aliaslookup();
ADDRESS  *parseaddr ();
char  *cat ();

char *DelimChar;

/*
**  ALIAS -- Compute aliases.
**
**	Scans the alias file for an alias for the given address.
**	If found, it arranges to deliver to the alias list instead.
**
**	Parameters:
**		a -- address to alias.
**		sendq -- a pointer to the head of the send queue
**			to put the aliases in.
**
**	Returns:
**		none
**
**	Side Effects:
**		Aliases found are expanded.
**
**	Notes:
**		If NoAlias (the "-n" flag) is set, no aliasing is
**			done.
**
**	Deficiencies:
**		It should complain about names that are aliased to
**			nothing.
*/


typedef struct
{
	char	*dptr;
	int	dsize;
} DATUM;
DATUM fetch();

alias(a, sendq)
	register ADDRESS *a;
	ADDRESS **sendq;
{
	register char *p;

# ifdef DEBUG
	if (tTd(27, 1))
		(void) printf("alias(%s)\n", a->q_paddr);
# endif DEBUG

	/* don't realias already aliased names */
	if (bitset(QDONTSEND, a->q_flags))
		return;

	CurEnv->e_to = a->q_paddr;

	/*
	**  Look up this name
	*/

	if (NoAlias)
		p = NULL;
	else
		p = aliaslookup(a->q_user);
	
#ifdef _SUN
	if (p == NULL)
		p = yellowlookup(a);
#endif _SUN

	if (p == NULL)
		return;

	/*
	**  Match on Alias.
	**	Deliver to the target list.
	*/

# ifdef DEBUG
	if (tTd(27, 1))
		(void) printf("%s (%s, %s) aliased to %s\n",
		    a->q_paddr, a->q_host, a->q_user, p);
# endif DEBUG
	message(Arpa_Info, MSGSTR(AL_ALTO, "aliased to %s"), p); /*MSG*/
	AliasLevel++;
	sendtolist(p, a, sendq);
	AliasLevel--;
}
/*
**  ALIASLOOKUP -- look up a name in the alias file.
**
**	Parameters:
**		name -- the name to look up.
**
**	Returns:
**		the value of name.
**		NULL if unknown.
**
**	Side Effects:
**		none.
**
**	Warnings:
**		The return value will be trashed across calls.
*/

char *
aliaslookup(name)
	char *name;
{
	DATUM rhs, lhs;

	/* create a key for fetch */
	lhs.dptr = name;
	lhs.dsize = strlen(name) + 1;
	rhs = fetch(lhs);
	return (rhs.dptr);
}
#ifdef _SUN
/*
**  YELLOWLOOKUP -- look up a name in the Yellow Pages.
**
**      Parameters:
**              a -- the address to look up.
**
**      Returns:
**              the value of name.
**              NULL if unknown.
**
**      Side Effects:
**              sets
**
**      Warnings:
**              The return value will be trashed across calls.
*/

char *
yellowlookup(a)
	register ADDRESS *a;
{
	char *result;
	int insize, outsize;

		/*
		 * if we did not find a local alias, then
		 * try a remote alias through yellow pages.
		 */
	if (AliasMap==NULL || *AliasMap=='\0') return(NULL);

	if (ypDomain==NULL)
	{
		yp_get_default_domain(&ypDomain);
		if (ypDomain == NULL) return(NULL);
#ifdef DEBUG
		if (tTd(27, 1))
			printf("Yellow pages domain is %s\n",ypDomain);
#endif DEBUG
	}
	if (bitset(QWASLOCAL,a->q_flags)) return(NULL);
	insize = strlen(a->q_user);
	
	/* Try yp_match() with and without insize including a NULL for
	 * a NULL terminated string. The reason for this is that SUN's
	 * yp_match() looks for a NULL terminated string and AIX's
	 * doesn't. Since we don't know who the client is, try both
	 */	
	if (yp_match(ypDomain,AliasMap,a->q_user, insize+1, &result, &outsize))
	{
        	if (yp_match(ypDomain,AliasMap,a->q_user, insize, &result, &outsize))
          	{
			errno = 0;
			return(NULL);
          	}
	}

#ifdef DEBUG
	if (tTd(27, 1))
		printf("%s maps to %s\n",a->q_user, result );
#endif DEBUG
	a->q_flags |= QDOMAIN;
	return(result);
}
#endif _SUN
/*
**  OPENALIASES -- Open and validate alias data base.
**
**	Parameters:
**              aliasfile -- data base name.  Null string disables
**                           aliasing.
**
**	Returns:
**              EX_xx return code.
**
**	Side Effects:
**              None
*/

openaliases(aliasfile)
	char *aliasfile;
{
	int err, fd;
	char  lockfile[MAXNAME];        /* data base lock file name */
	char  aliasdb [MAXNAME];        /* data base name                */
	static int initialized = FALSE;
	char  *afn;

	errno = 0;			/* clean up any junk		*/

	if (initialized)
		return (EX_OK);
	initialized = TRUE;

	if (aliasfile == NULL)          /* no, or improper, A option    */
	{
	    NoAlias = TRUE;
	    return (EX_OK);
	}

	/*
	 *  <aliasfile> is the path of a text input file created by the system
	 *  manager.  The system manager uses the "sendmail -bi" function
	 *  to transform this text input into a corresponding data base which
	 *  can be used by sendmail.  There is a separate database for each
	 *  differently named alias file.  Date base files reside in a
	 *  subdirectory with respect to the input alias file.  The sub-
	 *  directory is named "<aliasfile>DB".  Inside the subdirectory
	 *  are two files named DB.dir and DB.pag.
	 *
	 *  When rebuilding a database, the new version is temporarily
	 *  placed in a subdirectory named "<aliasfile>DBt".  After the
	 *  new data base files are built, "<aliasfile>DBt" is "mv"d
	 *  to "<aliasfile>DB".  Only one rebuild on each input alias file
	 *  should be active simultaneously.  This is enforced by system
	 *  management procedures.
	 *
	 *  The directory "mv" operation must be unitary with respect to
	 *  other processes wanting to open the data base for reading only.
	 *  However, since the date base open is composed of two file open
	 *  operations, the "mv" could take place between them.  Therefore
	 *  a "semaphore" must be used to serialize data base open and "mv".
	 *  The semaphore effect is gained by locking the file at relative
	 *  path "<aliasfile>DBl".  Read-only users lock this file,
	 *  open and verify data base content, then (keeping the files open)
	 *  release the lock.  An update process locks the semaphore file,
	 *  perform the "mv" renaming the directory, then unlocks the file.
	 *  Current users of old versions are unaffected.
	 *
	 *  Why is a subdirectory used is to make the data base update
	 *  into a single "mv" operation?  Else, two "mv"s would be required.
	 *  In that case, the update could fail after the first "mv",
	 *  leaving the data bases inconsistent.
	 *
	 *  Dbminit is not smart enough to detect this inconsistency, and
	 *  neither are we since we don't compare to the text alias file
	 *  modification date anymore.
	 *
	 *  We do not look at the alias file itself because it may be updated
	 *  at any time without affecting sendmail (until the sendmail
	 *  -bi is performed for it).  We don't want sendmail having an error
	 *  or even issuing a warning if the alias file being used has been
	 *  updated.  It is the system manager's responsibility to see that
	 *  the sendmail alias file data base update function is performed
	 *  for each alias file that he updates.
	 */

	/*
	 *  Create required path strings.
	 */
	if (cat ( aliasdb, sizeof  aliasdb, aliasfile, "DB/DB") == NULL ||
	    cat (lockfile, sizeof lockfile, aliasfile, "DBl"  ) == NULL)
	{
	    syserr (MSGSTR(AL_ELONG, "Alias path \"%s\" too long"), aliasfile); /*MSG*/
	    return (EX_DB);
	}

	/*
	 *  Isolate file name portion, and assure distinguishability.
	 */
	afn = strrchr (aliasfile, '/');
	if (afn == NULL)  afn = aliasfile;
	else		  afn++;

	if (strlen (afn) > MAXFNAME - strlen ("DBl"))
	{
	    syserr (MSGSTR(AL_LONG, "Alias file name \"%s\", too long"), afn); /*MSG*/
	    return (EX_DB);
	}

	/*
	 *  Get access to "<aliasfile>DBl".
	 */
	fd = open (lockfile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if (fd < 0)
	{
	    syserr (MSGSTR(AL_ELOCK, "Unable to open alias lock file \"%s\""), lockfile); /*MSG*/
	    return (EX_DB);
	}

	/*
	 *  Serialize open/update accesses at this point.
	 *
	 *  This unnecessarily causes all sendmail openaliases
	 *  operations to serialize execution at this point.
	 *  However, this is not a problem since data base validation
	 *  operations are rapid.
	 */
	if (lockf(fd, F_LOCK, 0) < 0)   /* lock the whole thing         */
	{
	    syserr (MSGSTR(AL_ELOCK2, "Error locking alias lock file \"%s\""), lockfile); /*MSG*/
	    (void) close (fd);
	    return (EX_OSERR);
	}
	/*
	 *  From this point onwards, release lock before exiting.
	 *  This is done with a close.
	 */

	/*
	 *  Gain access to data base files (open them with dbminit).
	 *
	 *  ASSUMPTION:  It is assumed that the dbm package keeps the data-
	 *  base files open continuously.  If not, then this synchronization
	 *  logic fails.
	 *
	 *  If we fail to open because it deoesn't exist, try building it
	 *  automatically and re-opening.
	 */
	if ((err = dbminit(aliasdb)) && (errno == ENOENT)) {

	    /* try reading it and reopening */

	    syslog (LOG_NOTICE, MSGSTR(MN_ALFORCE,
		"Attempting to build aliases data base \"%s\""), aliasdb);

	    if (! (err = readaliases(AliasFile) || dbminit(aliasdb)))
		syslog (LOG_NOTICE, MSGSTR(MN_ALOK,
		    "Aliases data base \"%s\" created"), aliasdb);
	}

	if (err) {
	    syserr(MSGSTR(AL_EINIT,
		"Cannot open aliases data base \"%s\""), aliasdb);
	    (void) close (fd);          /* release locks                */
	    return (EX_DB);
	}

	/*
	 *  Verify content of the data base.  Check for the last entry
	 *  written to assure that data base update run didn't somehow
	 *  fail.  This is a relic and possibly can't happen under
	 *  the current scheme, since the "mv" to update the data base
	 *  won't occur unless the data base is completely written to
	 *  begin with.  Still, leave it in for warm fuzzies.
	 */
	if (aliaslookup("@") == NULL)       /* file complete?       */
	{
	    syserr (MSGSTR(AL_INC, "\"%s\" incomplete"), aliasdb); /*MSG*/
	    (void) close (fd);          /* release locks                */
	    return (EX_DB);
	}

	/*
	 *  We now have both db files open.  Release the lock and we
	 *  can't lose them or see them overwritten before our eyes.
	 */
	(void) close (fd);              /* release locks                */

	return (EX_OK);
}

/*
**  READALIASES -- Builds the aliasfile database.
**
**	Parameters:
**		aliasfile -- the pathname of the alias file master.
**                           Null path is an error.
**
**	Returns:
**              -1 on failure.  Absence of alias file is an error.
**
*/

readaliases(aliasfile)
	char *aliasfile;
{
	register char *p;
	char *rhs, *afn;
	int skipping;
	int naliases, bytes, longest;
	int  err, fd;
	FILE *af;
	ADDRESS al, bl;
	DATUM  key, content;
	char line[BUFSIZ];
	char  lockfile [MAXNAME];
	char   dirname [MAXNAME];
	char  tdirname [MAXNAME];
	char   tdbname [MAXNAME];
	char  tfiledir [MAXNAME];
	char  tfilepag [MAXNAME];

	errno = 0;			/* clean up any junk		*/

	/*
	 *  Null path name is wrong.
	 */
	if (aliasfile == NULL)
	{
	    syserr (MSGSTR(AL_NOPATH, "No valid alias file path; data base not initialized")); /*MSG*/
	    return (EX_DB);
	}

	/*
	 *  SEE OPENALIASES FOR DATABASE UPDATE PHILOSOPHY.
	 */

	/*
	 *  There is no way to protect against the INPUT alias file being
	 *  changed (say by a text editor) while it is being read for
	 *  purposes of rebuilding the corresponding data base.  AIX does
	 *  not provide the necessary exclusive open operation.  Lockf
	 *  operations are normally voluntary and are not performed by
	 *  text editors.
	 *
	 *  This problem is overcome by the operational stipulation that
	 *  the system manager, only, be permitted to update the an alias
	 *  text file; and furthermore that no one but the system manager
	 *  be allowed to perform the necessary "sendmail -bi" function.
	 *  He will perform the text update and data base rebuild serially
	 *  for each alias file.  Other active sendmail processes will
         *  continue to use the old data base until the data base reconstruction
	 *  operation finishes.
	 *
	 *  There is currently no protection against the system manager
	 *  performing two or more simultaneous data base runs for the
	 *  same data base.  Again, this is solved operationally by
	 *  correct procedures on the part of the system manager.
	 *  However, if necessary, interlock can be programmed by locking
	 *  on the input aliasfile.
	 *
	 *  ANOTHER OPERATIONAL PROBLEM:  If the daemon is using the data
	 *  base being updated, it will have to be killed and restarted
	 *  after the update is complete.
	 *
	 *  The sendmail program, configuration file, and instructions
	 *  for use are modified to reflect the fact that only the system
	 *  manager can perform "sendmail -bi".
	 *
	 *  SPACE PROBLEM:  This update procedure requires enough additional
	 *  space in the minidisk containing /usr/lib to allow for
	 *  creation of the temporary subdirectory for the new data base.  This
	 *  may be a problem if there is not enough free space customarily
	 *  available.  If this is intolerable, then some other semaphore 
	 *  concept will have to be implemented to allow the data base files 
	 *  to be updated in place.  The update will have to stall until all 
	 *  read-only users of the data base finish.  Furthermore, new
	 *  read-only users will also have to be stalled until the update
	 *  operation is finished.  (This could be easily provided by
	 *  an exclusive open operation that UNIX doesn't have!)
	 *  One unfortunate side affect of this alternate idea is that
	 *  even read-only (ordinary) sendmail operations could wait a long
	 *  time before finishing processing.  This is probably intolerable
	 *  in practice.
	 */

	/*
	 *  Create required path names.
	 */
	if (cat ( dirname, sizeof  dirname, aliasfile, "DB"  ) == NULL ||
	    cat (tdirname, sizeof tdirname, aliasfile, "DBt" ) == NULL ||
	    cat ( tdbname, sizeof  tdbname,  tdirname, "/DB" ) == NULL ||
	    cat (lockfile, sizeof lockfile, aliasfile, "DBl" ) == NULL ||
	    cat (tfiledir, sizeof tfiledir,   tdbname, ".dir") == NULL ||
	    cat (tfilepag, sizeof tfilepag,   tdbname, ".pag") == NULL)
	{
	    syserr (MSGSTR(AL_ELONG, "Alias path \"%s\" too long"), aliasfile); /*MSG*/
	    return (EX_DB);
	}

	/*
	 *  Isolate file name portion and assure distinguishability.
	 */
	afn = strrchr (aliasfile, '/');
	if (afn == NULL)  afn = aliasfile;
	else		  afn++;

	if (strlen (afn) > MAXFNAME - strlen ("DBt"))
	{
	    syserr (MSGSTR(AL_LONG, "Alias file name \"%s\", too long"), afn); /*MSG*/
	    return (EX_DB);
	}

	/*
	 *  Create all path names required.
	 */


	/*
	 *  Create a new work subdirectory at relative path "<aliasfile>DBt".
	 *  Delete any previous version, using xrmdir() to remove dir and
	 *  any files in it.
	 *  This step is necessary since it is conceivable that the
	 *  scratch name is pointing to something real due to a previous
	 *  failure.
	 */
	if (xrmdir(tdirname, TRUE))
	{
	    syserr (MSGSTR(AL_EULINK, "Can't remove \"%s\""), tdirname); /*MSG*/
	    return (EX_DB);
	}
	errno = 0;			/* clean up any junk		*/

	/*
	 *  Create fresh directory.
	 */
	if (mkdir(tdirname, 0770))
	{
		syserr(MSGSTR(AL_EMAKE, "Cannot make \"%s\""), tdirname);
		return (EX_DB);
	}

	/*
	 *  Create empty data base files.
	 */
	if ((err = creat(tfiledir, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0)
	{
		syserr(MSGSTR(AL_EMAKE, "Cannot make \"%s\""), tfiledir); /*MSG*/
		return (EX_DB);
	}
	(void) close (err);

	if ((err = creat(tfilepag, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0)
	{
		syserr(MSGSTR(AL_EMAKE, "Cannot make \"%s\""), tfilepag); /*MSG*/
		return (EX_DB);
	}
	(void) close (err);

	/*
	 *  Crank up dbm package.
	 */
	if ((err = dbminit(tdbname)) < 0)
	{
	    syserr (MSGSTR(AL_EINIT,
		"Cannot open aliases data base \"%s\""), tdbname);
	    return (EX_DB);
	}

	/*
	 *  Get access to aliasfile.
	 */
	if ((af = fopen(aliasfile, "r")) == NULL)
	{
	    syserr (MSGSTR(AL_EAVAIL, "Alias file \"%s\", not available"), aliasfile); /*MSG*/
	    return (EX_DB);
	}

	/*
	**  Read and interpret lines
	*/

	FileName = aliasfile;
	LineNumber = 0;
	naliases = bytes = longest = 0;
	skipping = FALSE;
	while (fgets(line, sizeof (line), af) != NULL)
	{
	    int lhssize, rhssize;

	    LineNumber++;
	    p = strchr(line, '\n');
	    if (p != NULL)
		    *p = '\0';
	    switch (line[0])
	    {
	      case '#':
	      case '\0':
		    skipping = FALSE;
		    continue;

	      case ' ':
	      case '\t':
		    if (!skipping)
			    syserr(MSGSTR(AL_ESPACE, "Non-continuation line starts with space")); /*MSG*/
		    skipping = TRUE;
		    continue;
	    }
	    skipping = FALSE;

	    /*
	    **  Process the LHS
	    **      Find the final colon, and parse the address.
	    **      It should resolve to a local name -- this will
	    **      be checked later (we want to optionally do
	    **      parsing of the RHS first to maximize error
	    **      detection).
	    */

	    for (p = line; *p != '\0' && *p != ':' && *p != '\n'; p++)
		    continue;
	    if (*p++ != ':')
	    {
		    syserr(MSGSTR(AL_ECOLON, "Missing colon")); /*MSG*/
		    continue;
	    }
	    if (parseaddr(line, &al, 1, ':') == NULL)
	    {
		    syserr(MSGSTR(AL_ENAME, "Illegal alias name")); /*MSG*/
		    continue;
	    }
	    loweraddr(&al);

	    /*
	    **  Process the RHS.
	    **      'al' is the internal form of the LHS address.
	    **      'p' points to the text of the RHS.
	    */

	    rhs = p;
	    while (1)
	    {
		register char c;

		if (CheckAliases)
		{
			/* do parsing & compression of addresses */
			while (*p != '\0')
			{
			    while (isspace(*p) || *p == ',')
				    p++;
			    if (*p == '\0')
				    break;
			    if (parseaddr(p, &bl, -1, ',') == NULL)
				    usrerr(MSGSTR(AL_EADDR, "%s... bad address"), p); /*MSG*/
			    p = DelimChar;
			}
		}
		else
		{
			p = &p[strlen(p)];
			if (p[-1] == '\n')
				*--p = '\0';
		}

		/* see if there should be a continuation line */
		c = fgetc(af);
		if (ferror (af))
		{
		    syserr (MSGSTR(AL_EREAD, "Error reading alias file \"%s\""), aliasfile); /*MSG*/
		    (void) fclose (af);
		    return (EX_DB);
		}

		if (!feof(af))
			(void) ungetc(c, af);
		if (c != ' ' && c != '\t')
			break;

		/* read continuation line */
		if (fgets(p, (int) sizeof (line) - (int) (p - line), af) ==
								       NULL)
		    break;
		LineNumber++;
	    }
	    if (ferror (af))
	    {
		syserr (MSGSTR(AL_EREAD, "Error reading alias file \"%s\""), aliasfile); /*MSG*/
		(void) fclose (af);
		return (EX_DB);
	    }

	    if (al.q_mailer != LocalMailer)
	    {
		    syserr(MSGSTR(AL_ELOCAL, "Cannot alias non-local names")); /*MSG*/
		    continue;
	    }

	    /*
	    **  Insert alias into DBM file
	    */

	    lhssize = strlen(al.q_user) + 1;
	    rhssize = strlen(rhs) + 1;

	    key.dsize = lhssize;
	    key.dptr = al.q_user;
	    content.dsize = rhssize;
	    content.dptr = rhs;
	    if ((err = store(key, content)) < 0)
	    {
		syserr (MSGSTR(AL_EDBM, "Dbm store error")); /*MSG*/
		(void) fclose (af);
		return (EX_DB);
	    }

	    /* statistics */
	    naliases++;
	    bytes += lhssize + rhssize;
	    if (rhssize > longest)
		    longest = rhssize;
	}

	/*
	 *  Make sure we didn't have a read error.
	 */
	if (ferror (af))
	{
	    syserr (MSGSTR(AL_EREAD, "Error reading alias file \"%s\""), aliasfile); /*MSG*/
	    (void) fclose (af);
	    return (EX_DB);
	}

	/* add the distinquished alias "@" */
	key.dsize = 2;
	key.dptr = "@";
	if ((err = store(key, key)) < 0)
	{
	    syserr (MSGSTR(AL_EDBM, "Dbm store error")); /*MSG*/
	    (void) fclose (af);
	    return (EX_DB);
	}

	(void) fclose (af);             /* close input file             */

	CurEnv->e_to = NULL;
	FileName = NULL;

	/*
	 *  The complete database is now available under the scratch directory
	 *  name.
	 */

	/*
	 *  Prevent other openaliases operations from accessing the
	 *  data base until we are through.  Open and lock the lock file
	 *  Read-only users only perform their double data
	 *  base open after successfully locking this file.  They release
	 *  the lock after they are either successfully opened, or failed.
	 */
	fd = open (lockfile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if (fd < 0)
	{
	    syserr (MSGSTR(AL_ELOCK3, "Error opening alias lock file \"%s\""), lockfile); /*MSG*/
	    return (EX_DB);
	}

	if (lockf (fd, F_LOCK, 0) < 0)
	{
	    syserr (MSGSTR(AL_ELOCK2, "Error locking alias lock file \"%s\""), lockfile); /*MSG*/
	    (void) close (fd);
	    return (EX_OSERR);
	}
	/*
	 *  From this point onward, the semaphore file must be unlocked
	 *  before exiting.  This is done with a close operation.
	 */

	/*
	 *  Move work directory name to production name.
	 *
	 *  First, remove production directory name, if it exists.  This
	 *  unavoidably causes the directory name to temporarily not exist.
	 *  However, no opens will be attempted since folks who do will be
	 *  pended on the lock file.  Those who have the data base files
	 *  already fully opened are OK.  In the latter case, they can't
	 *  lose access to their open files even though the file names
	 *  disappear from the directory world.
	 */
	if (xrmdir(dirname, TRUE))
	{
	    syserr (MSGSTR(AL_EULINK, "Can't remove \"%s\""), dirname); /*MSG*/
	    (void) close (fd);   /* close and rel locks          */
	    return (EX_DB);
	}
	errno = 0;			/* clean up junk		*/

	/*
	 *  The production directory name is no longer available to anyone.
	 *  Attach the production name to the new directory before releasing
	 *  the lock file.
	 *
	 *  If this fails, we are left with no database.  We code this
	 *  all up professionally even though it might be possible to
	 *  prove that a failure here cannot occur.
	 */
	if (link (tdirname, dirname) < 0) /* point to new direct.*/
	{
	    syserr (MSGSTR(AL_ELINK, "Can't link \"%s\" to \"%s\""), dirname, tdirname); /*MSG*/
	    (void) close (fd);		/* release lock file		*/
	    return (EX_DB);
	}

	/*
	 *  Now allow readers access to the data base under production name.
	 */
	(void) close (fd);		/* release lock file		*/

	/*
	 *  Remove scratch directory name from its parent directory.
	 *
	 *  An unlink failure means that the scratch names and production
	 *  names are left pointing to the same place.  This would imply
	 *  that the next rebuild would be into a working data base, thus
	 *  defeating our protection scheme.  This is overcome by assuring 
	 *  that the scratch directory name is unlinked (above) before being 
	 *  recreated.
	 *
	 *  Since the database is good, we ignore any failure here.
	 *  If tdirname can't be unlinked, then it will fail later when the
	 *  rebuild is attempted again.
	 */
	(void) unlink (tdirname);

	/*
	 *  Log situation for users info.
	 */
	message(Arpa_Info, MSGSTR(AL_NBYTES, "%d aliases, longest %d bytes, %d bytes total"), naliases, longest, bytes); /*MSG*/
# ifdef LOG
	syslog(LOG_INFO, MSGSTR(AL_NBYTES, "%d aliases, longest %d bytes, %d bytes total"), naliases, longest, bytes); /*MSG*/
# endif LOG

	return (EX_OK);
}
/*
**  FORWARD -- Try to forward mail
**
**	This is similar but not identical to aliasing.
**
**	Parameters:
**		user -- the name of the user who's mail we would like
**			to forward to.  It must have been verified --
**			i.e., the q_home field must have been filled
**			in.
**		sendq -- a pointer to the head of the send queue to
**			put this user's aliases in.
**
**	Returns:
**		none.
**
**	Side Effects:
**		New names are added to send queues.
*/

forward(user, sendq)
	ADDRESS *user;
	ADDRESS **sendq;
{
	char buf[60];

# ifdef DEBUG
	if (tTd(27, 1))
		(void) printf("forward(%s)\n", user->q_paddr);
# endif DEBUG

	if (user->q_mailer != LocalMailer || bitset(QBADADDR, user->q_flags))
		return;
	if (user->q_home == NULL)
		syserr (MSGSTR(AL_FORW, "Forward: no home"));  /*MSG*/

	/* good address -- look for .forward file in home */
	define('z', user->q_home, CurEnv);
	expand("\001z/.forward", buf, &buf[sizeof buf - 1], CurEnv);
	if (!bitset (QGOODUID, user->q_flags) || !safefile(buf, user->q_uid, S_IREAD))
		return;

	/* we do have an address to forward to -- do it */
	include(buf, MSGSTR(AL_FRWRD, "forwarding"), user, sendq); /*MSG*/
}

#ifdef DEBUG

/*
**  dump contents of alias file to stdout
**/

dumpal()
{

extern DATUM firstkey(), nextkey();
DATUM key, al;

    puts("\naliases:");
    for (key = firstkey(); key.dptr; key = nextkey(key)) {
	al = fetch(key);
	if (! al.dptr) {
	    printf("\nerror on fetch(%s)\n", key.dptr);
	    continue;
	}
	printf("  key='%s', alias='%s'\n", key.dptr, al.dptr);
    }
}

#endif DEBUG
