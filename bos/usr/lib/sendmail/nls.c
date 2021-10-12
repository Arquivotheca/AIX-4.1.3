static char sccsid[] = "@(#)42	1.10.1.1  src/bos/usr/lib/sendmail/nls.c, cmdsend, bos411, 9428A410j 3/16/93 09:58:07";
/* 
 * COMPONENT_NAME: CMDSEND nls.c
 * 
 * FUNCTIONS: MSGSTR, freezenl, get_btype, nl_process, readnl, freenl
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
**
**  Created: 03/07/89, INTERACTIVE Systems Corporation
**
**  These routines were added to the sendmail program to
**  encode the body of a mail item for either NLS (AIX) or
**  ISO 8859 compatible systems.  The sendmail.nl file contains lists
**  of the sytems which are NLS or ISO compatible.
**
*/


# include <stdio.h>
# include <fcntl.h>
# include <errno.h>
# include <memory.h>
# include <string.h>
# include <sys/lockf.h>
# include <sys/types.h>
# include <sys/stat.h>
# include "sysexits.h"
# include <netinet/in.h>
# include "conf.h"
# include "sendmail.h"

#define MAXNLLIST 10
#define NLMAGIC 0x20e51c6

#include <nl_types.h>
#include "sendmail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SENDMAIL,n,s) 

char  *xalloc ();
char  *cat ();

static int  nlists = 0;		/* number of lists in sendmail.nl */
static char *re_array[MAXNLLIST]; /* ptrs to alloc'ed memory for RE's */
static  int nl_process (FILE *, int, char *, char *);

struct nltable {
    int	body_type;		/* set to BT_ESC or BT_ISO */
    int list_size;		/* number of RE's in the list */
    int table_size;		/* size of the complete array of RE's */ 
    char **regp;		/* array of pointers to all the RE's */
} tablelist[MAXNLLIST];


/*
**  READNL -- read the sendmail.nlDB database
**
**	This routine read the lists of system names which determine how
**	the body of an outgoing mail item is to be encoded.
**
**	The file consists of lists of compiled regular expressions.
**	Each individual list consists of a character array which contains
**	all of the compiled strings and an array of pointers which point to
**	the individual compiled strings.  There are only two kinds of lists
**	currently defined: NLS and ISO-8859.  This routine allows for any
**	number of lists.  This allows for future expansion and also allows
**	the system administrator to keep multiple lists of the same type
**	which may be more convenient for administrative reasons.
**
**	Parameters:
**		nlname -- path name of database
**
**	Returns:
**		Std EX_xx return code.
**
**	Side Effects:
**		Builds internal system name tables.
*/

readnl(nlname)
	char *nlname;
{
	register char *cp;
	char  dbname [MAXNAME];
	char lockfile[MAXNAME];
	char errname [MAXNAME];
	char *afn;
	char **ptr_array;
	int  ret;
	int  i;
	int  offset;
	int  list;
	int  magic;
	int last;
	int  fdl;
	int  nlfile;
	unsigned  nread;

	/* Return if no file.  This file is not required. */
	if (nlname == NULL || ! *nlname)
	    return(0);
	errno = 0;			/* clear out junk		*/

	/*
	 *  Create required file names.
	 */
	if (cat (  dbname, sizeof   dbname,      nlname, "DB"  ) == NULL || 
	    cat (lockfile, sizeof lockfile,      nlname, "DBl" ) == NULL ||
	    cat ( errname, sizeof  errname, "Original ", nlname) == NULL)
	{
	    syserr (MSGSTR(NL_LONGPATH, "Sendmail.nl file path \"%s\", too long"), nlname); /*MSG*/
	    return (EX_DB);
	}

	/*
	 *  Isolate file name portion, and assure that it's distinguishable.
	 */
	afn = strrchr (nlname, '/');
	if (afn == NULL)  afn = nlname;
	else              afn++;

	if (strlen (afn) > MAXFNAME - strlen ("DBl"))
	{
	    syserr (MSGSTR(NL_LONGNAME, "Sendmail.nl file name \"%s\", too long"), afn); /*MSG*/
	    return (EX_DB);
	}

	/*
	 *  Serialize opens with updates.
	 */
	fdl = open (lockfile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if (fdl < 0)
	{
	    syserr (MSGSTR(NL_ELOCKFILE, "Unable to open sendmail.nl lock file \"%s\""), lockfile); /*MSG*/
	    return (EX_DB);
	}

	if (lockf (fdl, F_LOCK, 0) < 0)		/* pause for exclusive use */
	{
	    syserr (MSGSTR(NL_ELLOCKFILE, "Unable to lock sendmail.nl lock file \"%s\""), lockfile); /*MSG*/
	    (void) close (fdl);
	    return (EX_OSERR);
	}

	/*
	 *  Open the database.
	 */
	nlfile = open (dbname, O_RDONLY);
	if (nlfile < 0)
	{
	    if (errno == ENOENT)	/* guide the user */
	    {
		errno = 0;
		syserr (MSGSTR(NL_NOFRZFILE, "No frozen sendmail.nl file \"%s\""), dbname); /*MSG*/
		usrerr (MSGSTR(NL_TOFRZ, "Use \"/usr/lib/sendmail -bn\" to freeze \"%s\""), nlname); /*MSG*/
	    }
	    else
	        syserr (MSGSTR(NL_EOFRZFILE, "Cannot open frozen sendmail.nl file \"%s\""), dbname); /*MSG*/
	    (void) close (fdl);
	    return(EX_DB);
	}

	/*
	 *  Release lock to allow update.  The update process won't touch
	 *  our open file.
	 */
	(void) close (fdl);

	/*
	 *  Process file content.
	 */
	
	ret = read(nlfile, (char *) &magic,  sizeof(int));
	if (ret != sizeof(int)) {
	    syserr (MSGSTR(CF_EREAD, "Read error, \"%s\""), dbname); /*MSG*/
	    return (EX_DB);
	}
	if (magic != NLMAGIC)  {
	    syserr (MSGSTR(NL_EFORMAT, "Format error - bad magic, \"%s\""), dbname); /*MSG*/
	    return (EX_DB);
	}
	ret = read(nlfile, (char *)&nlists,  sizeof(int));
	if (ret != sizeof(int)) {
	    syserr (MSGSTR(CF_EREAD, "Read error, \"%s\""), dbname); /*MSG*/
	    return (EX_DB);
	}
	if (nlists > MAXNLLIST)  {
	    syserr (MSGSTR(NL_EFORMAT2, "Format error or too many lists, \"%s\""), dbname); /*MSG*/
	    return (EX_DB);
	}

	/*
	 *  read in each list
	 */
	for (list = 0; list < nlists; list++)  {
	    /* get the table structure */
	    ret = read(nlfile, (char *)(&tablelist[list]), sizeof(struct nltable));
	    if (ret != sizeof(struct nltable)) {
		syserr (MSGSTR(CF_EREAD, "Read error, \"%s\""), dbname); /*MSG*/
		return (EX_DB);
	    }

	    /* read in the array of pointers */
	    nread = (tablelist[list].list_size) * sizeof(char *);
	    cp = xalloc(nread);
	    tablelist[list].regp = (char **) cp;
	    ret = read(nlfile, cp, nread);
	    if (ret != nread) {
		syserr (MSGSTR(CF_EREAD, "Read error, \"%s\""), dbname); /*MSG*/
		return (EX_DB);
	    }

	    /* read in the array of RE's */
	    nread = tablelist[list].table_size;
	    re_array[list] = xalloc(nread);  /* save it for freenl() */
	    ret = read(nlfile, re_array[list], nread);
	    if (ret != nread) {
		syserr (MSGSTR(CF_EREAD, "Read error, \"%s\""), dbname); /*MSG*/
		return (EX_DB);
	    }

	    /* Go through and adjust each pointer so that it is an actual
	     * pointer into the array rather than the offset which was
	     * read from the file.  Also check the values for sanity.
	     */
	    last = 0;
	    ptr_array = tablelist[list].regp;
	    for (i = 0; i < tablelist[list].list_size; i++)  {
		offset = (int)(ptr_array[i]);
		if (offset < last || offset > nread)  {
		    syserr (MSGSTR(NL_EFORMAT3, "Format error, \"%s\" offset = %d"), dbname, offset); /*MSG*/
		    return (EX_DB);
		}
		last = offset;
		ptr_array[i] = offset + re_array[list];
	    }
	}
	return (EX_OK);
}


/*
**  GET_BTYPE -- get the body type for the specified address
**
**	This routine looks up the address in the lists from the sendmail.nl
**	file.  The address is compared sequentially with every regular
**	expression in the list until a match occurs.  The return value
**	indicates the ISO or NLS list in which the match occured.
**
**
**	Parameters:
**		a -- address to look up
**
**	Returns:
**		BT_ESC, BT_ISO, or BT_FLAT
**
**	Side Effects:
**		None
*/

get_btype(a)
char *a;
{
    register int size, re, list, btype;

    btype = BT_FLAT;
    for (list = 0; list < nlists; list ++)  {	    /* go through each list */
	size = tablelist[list].list_size;	    /* number of re's in list */
	for (re = 0; re < size; re++)  {	    /* go through each re */
	    if (regex(tablelist[list].regp[re], a)) {  /* got a match? */
		btype = tablelist[list].body_type;  /* if so, return type */
		break;
	    }
	}
	if (btype != BT_FLAT)
	    break;
    }
#ifdef DEBUG
    if (tTd(66, 9))
	    (void) printf("getbtype: address=%s bodytype=%d\n", a, btype);
#endif DEBUG
    return(btype);				    /* no match, or no lists */
}

#define TSIZE 1000

char *fgetfolded();
char *regcmp();

struct tmptable {
    int	body_type;		/* set to BT_ESC or BT_ISO */
    int list_size;		/* number of RE's in the list */
    int table_size;		/* size of the complete array of RE's */ 
    char *regp;			/* array of pointers to all the RE's */
    char *cmpptr[TSIZE];	/* temporary storage of regp pointers */
    int cmpptr_size[TSIZE];	/* size of each RE used in final writes */
} *writetable[MAXNLLIST];

/*
**  FREEZENL -- read control file and create corresponding database.
**
**	This routine reads the control file and builds the database
**	for optimized loading.
**
**	The file is formatted as a sequence of lines.  Continuation lines
**	are allowed and are provided for by using the "fgetfolded" routine.
**
**	Parameters:
**		nlname -- control file name (input).
**
**	Returns:
**		EX_xx completion code.
**
**	Side Effects:
**		Creates database under <nlname>DB.
*/

freezenl (nlname)
	char *nlname;
{
	FILE *nl;
	int   fc;
	char  *afn;
	char  tdbname [MAXNAME];
	char   dbname [MAXNAME];
	char lockfile [MAXNAME];
	int  err;
	int  fdl;

	errno = 0;			/* clear out junk		*/

	/*
	 *  Null path name is wrong.
	 */
	if (nlname == NULL || ! *nlname)
	{
	    syserr (MSGSTR(NL_NOPATH, "No valid sendmail.nl file path; data base not initialized"));  /*MSG*/
	    return (EX_DB);
	}

	/*
	 *  Create required file names.
	 */
	if (cat ( tdbname, sizeof  tdbname, nlname, "DBt") == NULL ||
	    cat (  dbname, sizeof   dbname, nlname, "DB" ) == NULL ||
	    cat (lockfile, sizeof lockfile, nlname, "DBl") == NULL)
	{
	    syserr (MSGSTR(NL_LONGPATH2, "Sendmail.nl path \"%s\", too long"), nlname); /*MSG*/
	    return (EX_DB);
	}

	/*
	 *  Isolate file name portion and assure that names will be 
	 *  distinguishable.
	 */
	afn = strrchr (nlname, '/');
	if (afn == NULL)  afn = nlname;
	else		  afn++;

	if (strlen (afn) > MAXFNAME - strlen ("DBt"))
	{
	    syserr (MSGSTR(NL_LONGNAME, "Sendmail.nl file name \"%s\", too long"), afn); /*MSG*/
	    return (EX_DB);
	}

	/*
	 *  Create a new work file at named "<nlname>DBt".
	 *  Delete any previous version.
	 *  This step is necessary since it is conceivable that the
	 *  scratch name is pointing to something real due to a previous
	 *  failure.
	 */
	if (unlink (tdbname) < 0)
	{
	    if (errno != ENOENT)        /* must just not exist          */
	    {
		syserr (MSGSTR(AL_EULINK, "Can't unlink \"%s\""), tdbname); /*MSG*/
		return (EX_DB);
	    }
	}
	errno = 0;

	/*
	 *  Create new data base file.
	 *
	 *  The creat call can't be used because it opens the file O_WRONLY.
	 */
#ifdef DEBUG
	if (tTd(67, 1))
		(void) printf("freezenl: opening db file '%s'\n", tdbname);
#endif DEBUG

	if ((fc = open (tdbname, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0)
	{
		syserr(MSGSTR(NL_ECREATE, "Cannot create temporary sendmail.nl data base \"%s\""), tdbname); /*MSG*/
		return (EX_DB);
	}
	/*
	 *  From this point on, close output file before any exit.
	 */

	/*
	 *  Get access to input configuration file.
	 */
	nl = fopen(nlname, "r");
	if (nl == NULL)
	{
		syserr(MSGSTR(NL_EOPEN, "Cannot open sendmail.nl file \"%s\""), nlname); /*MSG*/
		(void) close (fc);
		return(EX_DB);
	}
	/*
	 *  From this point on, close input file before any exit.
	 */

	err = nl_process (nl, fc, nlname, tdbname);  /* do file processing */

	(void) fclose (nl);             /* always close all the files   */
	(void) close (fc);

	if (err)                        /* if error state, just exit now */
	    return (err);

	/*
	 *  Serialize database update "mv" operation via lock file.
	 */
	fdl = open (lockfile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if (fdl < 0)
	{
	    syserr (MSGSTR(NL_ELOCKFILE, "Unable to open sendmail.nl lock file \"%s\""), lockfile); /*MSG*/
	    return (EX_DB);
	}

	if (lockf (fdl, F_LOCK, 0) < 0)
	{
	    syserr (MSGSTR(NL_ELLOCKFILE, "Unable to lock sendmail.nl lock file \"%s\""), lockfile); /*MSG*/
	    (void) close (fdl);
	    return (EX_OSERR);
	}

	/*
	 *  Move work file name to production name.
	 *  First, remove production database name, if it exists.
	 *  This temporarily causes the production name to not exist.
	 *  However, this interval is covered by usage of the lock file.
	 *  All users of the old database file continue to have access to
	 *  it as long as they had it open before the lock.
	 */
	if (unlink (dbname) < 0)
	{
	    if (errno != ENOENT)
	    {
		syserr (MSGSTR(AL_EULINK, "Can't unlink \"%s\""), dbname); /*MSG*/
	        (void) close (fdl);
		return (EX_DB);
	    }
	}
	errno = 0;

	/*
	 *  Attach production name to newly completed data base.
	 *  If this fails, we are left with no database.  We code this
	 *  all up professionally even though it might be possible to
	 *  prove that a failure here cannot occur.
	 */
	if (link (tdbname, dbname) < 0) /* point to new direct.*/
	{
	    syserr (MSGSTR(AL_ELINK, "Can't link \"%s\" to \"%s\""), dbname, tdbname); /*MSG*/
	    (void) close (fdl);
	    return (EX_DB);
	}

	/*
	 *  Release lock file and allow access to revised database.
	 */
	(void) close (fdl);

	/*
	 *  Remove scratch database name from its parent directory.
	 *
	 *  An unlink failure means that the scratch name and production
	 *  names are left pointing to the same place.  This would imply
	 *  that the next rebuild would be into a working data base thus
	 *  defeating our protection scheme.  This is overcome by assuring 
	 *  that the scratch database name is always unlinked (above) before 
	 *  being recreated.
	 *
	 *  Since the database is good, we ignore any failure here.
	 *  If tdbname can't be unlinked, then it will fail later when the
	 *  rebuild is attempted again.
	 */
	(void) unlink (tdbname);

# ifdef LOG
	syslog (LOG_NOTICE, MSGSTR(MN_NLSCF, 
	    "NLS configuration data base \"%s\" created"), dbname);
# endif LOG
	return (EX_OK);
}

#define DELIM ':'

/*
 *  Perform processing loop for creating freeze file.
 *  Wiggle through the file, extracting information and
 *  building the structure.
 *
 */
static  int nl_process (FILE *nl, int fc, char *nlname, char *tdbname)
{
	struct tmptable *table;
	int  magic, numlists, nitems, i, j, k, cmp_errors;
	int  cmpsize, tablesize;
	char ibuf[MAXLINE];
	char b_type[MAXLINE];
	char next_item[MAXLINE];
	char *p, *q, *s, *ptr, *type, *offset;
	static char nolist_msg[] = "No lists in \"%s\"";
	static char cf_ewrite[] = "Error writing \"%s\"";
	numlists = 0;
	cmp_errors = 0;

	while (1)
	{
	    /*
	     *  Get next possibly folded text line, if any.
	     */
	    if (fgetfolded (ibuf, MAXLINE, nl) == NULL)
		break;

	    if ((*ibuf == '\0') || (*ibuf == '#'))
		continue;

#ifdef DEBUG
	    if (tTd(67, 1))
		    (void) printf("nl_process: got line '%s'\n", ibuf);
#endif DEBUG

	    if ((p = strchr (ibuf, DELIM)) == NULL) {
		syserr (MSGSTR(NL_EFORMAT4, "Format error in \"%s\""), nlname); /*MSG*/
		return (EX_DB);
	    }

	    table = (struct tmptable *) xalloc(sizeof(struct tmptable));
#ifdef DEBUG
	    if (tTd(67, 1))
		    (void) printf("nl_process: xalloc'ed 0x%x\n", table);
#endif DEBUG

	    writetable[numlists] = table;

	    strncpy (b_type, ibuf, p - ibuf);
	    if ((strncmp(b_type, "NLS", 3)) == 0)
		table->body_type = BT_ESC;
	    else if ((strncmp(b_type, "8859", 4)) == 0)
		table->body_type = BT_ISO;
	    else {
		syserr (MSGSTR(NL_ENAME, "Wrong body type name in \"%s\""), nlname); /*MSG*/
		return (EX_DB);
	    }

	    p++;  /* point to start of REs for this body type */

	    tablesize = 0;
	    nitems = 0;
	    q = (char *) 1;  /* force first test ok */

	    for (;;) {
		/* strip leading ws */
		while (*p == ' ' || *p == '\t' || *p == '\n')
		    p++;
		if (! q || *p == '\0')  /* all done */
		    break;
		if (q = (char *) strchr (p, ','))  /* parse at next comma */
		    *q++ = '\0';
		strcpy (next_item, p);

#ifdef DEBUG
		if (tTd(67, 1))
		    (void) printf("nl_process: processing item '%s'\n",
			next_item);
#endif DEBUG

		s = regcmp(next_item, 0);
		if (s == NULL)  {		/* correct compile? */
		    syserr(MSGSTR(NL_EEXP, "Incorrect regular expression \"%s\"."), next_item); /*MSG*/
		    cmp_errors++;		/* count errs, we will go on */
		    continue;
		}
		table->cmpptr_size[nitems] = cmpsize = strlen(s) + 1;
		tablesize += cmpsize;
		table->cmpptr[nitems] = s;
#ifdef DEBUG
		if (tTd(67, 1))
		    (void) printf("nl_process: expanded to '%s', size = %d\n\
  nitems = %d, q=0x%x\n",
			table->cmpptr[nitems], table->cmpptr_size[nitems],
			nitems, q);
#endif DEBUG

		p = q;
		nitems++;
	    }
	    table->list_size = nitems;
	    table->table_size = tablesize;
	    table->regp = (char *) 0;


	    /* if valid bodytype with no items, silently complain, and
	     * don't add to list. */

	    if (table->list_size == 0)
		continue;
	    numlists++;

	    if (numlists > MAXNLLIST) {
		syserr (MSGSTR(NL_EFORMAT5, "Format error or too many lists.")); /*MSG*/
		return (EX_DB);
	    }
	}
	if (cmp_errors) {
	    syserr (MSGSTR(NL_EXPERRS, "%d Regular Expression errors."), cmp_errors); /*MSG*/
	    return (EX_DB);
	}

	/*
	 *  Check for any errors on reading config file.
	 */
	if (ferror (nl))
	{
	    syserr (MSGSTR(CF_EREAD2, "Error reading \"%s\""), nlname); /*MSG*/
	    return (EX_DB);
	}

	/*
	 * Write out the magic number, the number of lists.
	 * Then, for each list, write out the structure elements.
	 */

	magic = NLMAGIC;
	if (write (fc, (char *) &magic, (unsigned) sizeof(int))
				!= sizeof(int)) {
	    syserr (MSGSTR(CF_EWRITE, cf_ewrite), tdbname); /*MSG*/
	    return (EX_DB);
	}
	if (write (fc, (char *) &numlists, (unsigned) sizeof(int))
				!= sizeof(int)) {
	    syserr (MSGSTR(CF_EWRITE, cf_ewrite), tdbname); /*MSG*/
	    return (EX_DB);
	}

	if (numlists == 0) {  /* warn them and return ok */
	    message(Arpa_Info, MSGSTR(NL_NOLISTS, nolist_msg), nlname);
# ifdef LOG
	    syslog(LOG_WARNING, MSGSTR(NL_NOLISTS, nolist_msg), nlname);
# endif LOG
	    return (EX_OK);
	}

	message(Arpa_Info, "System Type      RE's");
	for (i = 0; i <numlists; i++) {
	    switch(writetable[i]->body_type)  {
	    case BT_ESC:	type = "NLS: ";	break;
	    case BT_ISO:	type = "8859:";	break;
	    default:		type = "     ";	break;
	    }
	    message(Arpa_Info, "%s            %d", type, 
					writetable[i]->list_size);
	    if (write (fc, (char *) &writetable[i]->body_type,
			(unsigned) sizeof(int)) != sizeof(int)) {
		syserr (MSGSTR(CF_EWRITE, cf_ewrite), tdbname); /*MSG*/
		return (EX_DB);
	    }
	    if (write (fc, (char *) &writetable[i]->list_size,
			(unsigned) sizeof(int)) != sizeof(int)) {
		syserr (MSGSTR(CF_EWRITE, cf_ewrite), tdbname); /*MSG*/
		return (EX_DB);
	    }
	    if (write (fc, (char *) &writetable[i]->table_size,
			(unsigned) sizeof(int)) != sizeof(int)) {
		syserr (MSGSTR(CF_EWRITE, cf_ewrite), tdbname); /*MSG*/
		return (EX_DB);
	    }
	    if (write (fc, (char *) &writetable[i]->regp,
				sizeof(char *)) != sizeof(char *)) {
		syserr (MSGSTR(CF_EWRITE, cf_ewrite), tdbname); /*MSG*/
		return (EX_DB);
	    }
	    offset = 0;
	    for (j = 0; j < writetable[i]->list_size; j++)  {
		if (write (fc, (char *) &offset,
			(unsigned ) sizeof(char *)) != sizeof(char *)) {
		    syserr (MSGSTR(CF_EWRITE, cf_ewrite), tdbname); /*MSG*/
		    return (EX_DB);
		}
		offset += writetable[i]->cmpptr_size[j];
	    }

	    for (j = 0; j < writetable[i]->list_size; j++) {
		cmpsize = writetable[i]->cmpptr_size[j];
		if (write (fc, writetable[i]->cmpptr[j], cmpsize) != cmpsize) {
			syserr (MSGSTR(CF_EWRITE, cf_ewrite), tdbname); /*MSG*/
			return (EX_DB);
		}
		free (writetable[i]->cmpptr[j]);
	    }
	}
	    
	return (EX_OK);
}

#ifdef DEBUG

/*
 *
 *  Dump the contents of the nls config data to stdout
 *
 */

void dumpnl()
{

int i, j;
char **cp, *type;

	fputs("\nNLS configuration:", stdout);
	for (i = 0; i < nlists; i++) {
	    switch(tablelist[i].body_type)  {
		case BT_ESC:
		    type = "NLS";
		    break;
		case BT_ISO:
		    type = "8859";
		    break;
		default:
		    type = "<invalid>";
		    break;
	    }
	    printf("\n  body type %s: %d regular expressions:",
		type, j = tablelist[i].list_size);
	    for (cp = tablelist[i].regp; j; --j) {
		fputs("\n    ", stdout);
		xputs(*cp++);
	    }
	}
	putchar('\n');
}

#endif DEBUG

/*
 *
 *  Free up the nls config data structures and clear them.  This should
 *  only be called immediately before re-reading the config file!
 *
 */

void freenl()
{

int list;

	/* step through table, free()ing the allocated data */

	for (list = 0; list < nlists; list++)  {
# ifdef DEBUG
	    if (tTd(0, 25))
		printf("freecf: freeing tablelist[%d].regp\n", list);
# endif DEBUG
	    free(tablelist[list].regp);
	    free(re_array[list]);
	}

	/* zero the list count to clear the data */

	nlists = 0;
}

