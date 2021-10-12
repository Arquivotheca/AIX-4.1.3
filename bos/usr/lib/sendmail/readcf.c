static char sccsid[] = "@(#)20	1.21.1.5  src/bos/usr/lib/sendmail/readcf.c, cmdsend, bos41J, 9510A_all 2/15/95 19:49:49";
/* 
 * COMPONENT_NAME: CMDSEND readcf.c
 * 
 * FUNCTIONS: MSGSTR, f_process, fileclass, freezecf, makeargv, 
 *            makemailer, munchstring, printrules, readcf, scanline, 
 *            setclass, setoption, validdomain
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

# include <stdio.h>
# include <fcntl.h>
# include <ctype.h>
# include <errno.h>
# include <string.h>
# include <memory.h>
# include <sys/lockf.h>

# include "conf.h"
# include "useful.h"
# include <sys/types.h>
# include <sys/stat.h>
# include <netinet/in.h>
# include <arpa/nameser.h>

# include "sysexits.h"

# include "sendmail.h"

char **prescan();
char **copyplist();
char *fgetfolded();
char *munchstring();
char **makeargv();
long atol();
long convtime();
off_t lseek ();
char  *xalloc ();
char  *cat ();

extern int  Either;
int NextMailer;
char *DelimChar;
int QueueLA;
int RefuseLA;
extern ENVELOPE BlankEnvelope;
struct rewrite *Rwpalloc = NULL;  /* ptr to alloc()ed rewrite rule data */

struct cfrechdr {short lineno; short len; };
struct cfrec { struct cfrechdr hdr; char buf[MAXLINE]; };

static  int f_process (FILE *, int , char *, char *);
static int  scanline (char *);

/*
**  Nameserver options: user specifies in the config file which records to
**  retrieve using the mnemonic resource-record (RR) codes, and we translate
**  this into a bitmask.  We handle the special case of "ALL" in the code.
*/

struct resource_records ResourceRecords[] = {
    { "MB", T_MB },
    { "MG", T_MG },
    { "MR", T_MR },
    { "MX", T_MX },
    { "ANY", NS_ANY },  /* should be T_ANY, but needs to fit in bitmap */
    { "", 0 }  /* must be zero! */
};


/*
**  READCF -- read control file database.
**
**	This routine reads the control file database and builds the internal
**	form.
**
**	The file is formatted as a sequence of lines, each taken
**	atomically.  The first character of each line describes how
**	the line is to be interpreted.  The lines are:
**		Dxval		Define macro x to have value val.
**		Cxword		Put word into class x.
**		Fxfile [fmt]	Read file for lines to put into
**				class x.  Use scanf string 'fmt'
**				or "%s" if not present.  Fmt should
**				only produce one string-valued result.
**		Hname: value	Define header with field-name 'name'
**				and value as specified; this will be
**				macro expanded immediately before
**				use.
**		Sn		Use rewriting set n.
**		Rlhs rhs	Rewrite addresses that match lhs to
**				be rhs.
**		Mn arg=val...	Define mailer.  n is the internal name.
**				Args specify mailer parameters.
**		Oxvalue		Set option x to value.
**		Pname=value	Set precedence name to value.
**
**	Parameters:
**		cfname -- basic control file name.
**
**	Returns:
**		Std EX_xx return code.
**
**	Side Effects:
**		Builds several internal tables.
*/

readcf(cfname)
	char *cfname;
{
	int errors;
	int cf;
	int ruleset = 0;
	char *q;
	char **pv;
	struct rewrite *rwp = NULL;
	struct rewrite *rwpalloc;
	struct cfrec rec;
	register char *p;
	char exbuf[MAXLINE];
	char pvpbuf[PSBUFSIZE];
	char  dbname [MAXNAME];
	char lockfile[MAXNAME];
	char errname [MAXNAME];
	int  err, next_l;
	int  first;
	char  *afn;
	int  rules;
	int  fdl;

	errno = 0;			/* clear out junk		*/

	/*
	 *  Create required file names.
	 */
	if (cat (  dbname, sizeof   dbname,      cfname, "DB"  ) == NULL || 
	    cat (lockfile, sizeof lockfile,      cfname, "DBl" ) == NULL ||
	    cat ( errname, sizeof  errname, "Original ", cfname) == NULL)
	{
	    syserr (MSGSTR(CF_ELONG, "Configuration file path \"%s\", too long"), cfname); /*MSG*/
	    return (EX_DB);
	}

	/*
	 *  Isolate file name portion, and assure that it's distinguishable.
	 */
	afn = strrchr (cfname, '/');
	if (afn == NULL)  afn = cfname;
	else              afn++;

	if (strlen (afn) > MAXFNAME - strlen ("DBl"))
	{
	    syserr (MSGSTR(CF_ELONG2, "Configuration file name \"%s\", too long"), afn); /*MSG*/
	    return (EX_DB);
	}

	/*
	 *  Serialize opens with updates.
	 */
	fdl = open (lockfile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if (fdl < 0)
	{
	    syserr (MSGSTR(CF_ELOCK, "Unable to open configuration lock file \"%s\""), lockfile); /*MSG*/
	    return (EX_DB);
	}

	if (lockf (fdl, F_LOCK, 0) < 0)		/* pause for exclusive use */
	{
	    syserr (MSGSTR(DF_ELOCK2, "Unable to lock configuration lock file \"%s\""), lockfile); /*MSG*/
	    (void) close (fdl);
	    return (EX_OSERR);
	}

	/*
	 *  Open the database.
	 */
	cf = open (dbname, O_RDONLY);
	if (cf < 0)
	{
	    if (errno == ENOENT)
	    {
		/* try freezing it and reopening */

		syslog (LOG_NOTICE, MSGSTR(MN_CFFORCE,
		    "Attempting to freeze configuration data base \"%sDB\""),
		    cfname);

		if (freezecf(cfname) || (cf = open (dbname, O_RDONLY)) < 0) {
		    errno = 0;
		    syserr (MSGSTR(CF_EFROZEN,
			"No frozen configuration file \"%s\""), dbname);
		    usrerr (MSGSTR(DF_EFROZE2,
			"Use \"/usr/lib/sendmail -bz\" to freeze \"%s\""),
			cfname);

		} else  /* log success */
		    syslog (LOG_NOTICE, MSGSTR(MN_CF,
			"Configuration data base \"%sDB\" created"), cfname);
	    }
	    else
	        syserr (MSGSTR(CF_EFROZE3, "Cannot open frozen configuration file \"%s\""), dbname); /*MSG*/
	    if (cf < 0) {
		(void) close (fdl);
		return(EX_DB);
	    }
	}

	/*
	 *  Release lock to allow update.  The update process won't touch
	 *  our open file.
	 */
	(void) close (fdl);

	/*
	 *  Process file content.
	 */
	FileName = errname;
	LineNumber = 0;
	errors = 0;
	first = TRUE;			/* set when after first record	*/
	next_l = sizeof (struct cfrechdr) + 1; /* first rec has 1 byte data */
	while (1)
	{
		/*
		 *  next_l is next rec length if nonzero; else EOF.
		 */
		if (!next_l)			/* zero means no more	*/
		    break;

		/*
		 *  Read in the next record.
		 */
#ifdef DEBUG
		if (tTd (39, 1))
		    printf("readcf: reading %d chars into 0x%lx from fd %d\n",
			next_l, &rec, cf);
#endif DEBUG

		err = read (cf, (char *) &rec, (unsigned) next_l);

		if (err < 0)			/* read error?	*/
		{
		    FileName = NULL;		/* don't need line # */
		    syserr (MSGSTR(CF_EREAD, "Read error, \"%s\""), dbname); /*MSG*/
		    return (EX_DB);
		}

		if (err != next_l || rec.hdr.len > MAXLINE) /* weird? */
		{
		    FileName = NULL;		/* don't need line # */
		    syserr (MSGSTR(CF_EFROZ4, "Invalid freeze file \"%s\""), dbname); /*MSG*/
		    return (EX_DB);
		}

#ifdef DEBUG
		if (tTd (37, 1))
		   (void) printf ("%s%4d: size %3d  next-size %3d  buf <%s>\n", 
					first ? "rule count " : "",
					rec.hdr.lineno,
					next_l,		/* current length */
					rec.hdr.len,
					rec.buf);
#endif DEBUG

		next_l = rec.hdr.len;		/* for next time	*/

		/*
		 *  The first record contains the rule count in the buffer.
		 *  Allocate all memory space for rulesets with one xalloc
		 *  call.
		 */
		if (first)
		{
		    first = FALSE;
		    rules = rec.hdr.lineno;	/* get max rule count	*/
		    Rwpalloc = rwpalloc = (struct rewrite *)
				       xalloc (rules * (int) sizeof (*rwp));
		    continue;			/* no line here		*/
		}

		/*
		 *  Set up global for any error messages that occur.
		 */
		LineNumber = rec.hdr.lineno;

		/*
		 *  Process the config line.
		 */
		switch (rec.buf[0])
		{
		  /*
		   *  This takes a lot of time because there are maybe
		   *  170 of them in a typical case.
		   */
		  case 'R':		/* rewriting rule */
#ifdef DEBUG
			if (tTd (39, 1))
			    printf("readcf: rewrite rule='%s'\n", rec.buf);
#endif DEBUG

			if (!Ined) {for (p = &rec.buf[1]; *p != '\0' && *p != '\t'; p++)
				continue;
			}
			if (Ined) {for (p = &rec.buf[1]; *p != '\0' && *p != '\t' && *p != ' '; p++)
				continue;
			}

			if (*p == '\0')
			{
				syserr(MSGSTR(CF_EREWRITE, "Invalid rewrite line \"%s\""), rec.buf); /*MSG*/
				errors++;
				break;
			}

			/* allocate space for the rule header */
			if (rwp == NULL)
			{
				RewriteRules[ruleset] = rwp = rwpalloc++;
			}
			else
			{
				rwp->r_next = rwpalloc++;
				rwp = rwp->r_next;
			}
			rwp->r_next = NULL;

			/* expand and save the LHS */
			*p = '\0';
			expand(&rec.buf[1], exbuf, &exbuf[sizeof exbuf],CurEnv);
			rwp->r_lhs = prescan(exbuf, '\t', pvpbuf);
			if (rwp->r_lhs != NULL)
				rwp->r_lhs = copyplist(rwp->r_lhs, TRUE);
#ifdef DEBUG
			if (tTd (39, 1))
			    printav1 (rwp->r_lhs, '\t');
#endif DEBUG

			/* expand and save the RHS */
			if (!Ined) {while (*++p == '\t')
				continue;
			}
			if (Ined) {for (++p; *p == '\t' || *p == ' '; p++)
				continue;
			}
			q = p;
			if (!Ined) {while (*p != '\0' && *p != '\t')
				p++;
			}
			if (Ined) {while (*p != '\0' && *p != '\t' && *p != ' ')
				p++;
			}
			*p = '\0';
			expand(q, exbuf, &exbuf[sizeof exbuf], CurEnv);
			rwp->r_rhs = prescan(exbuf, '\t', pvpbuf);
			if (rwp->r_rhs != NULL)
				rwp->r_rhs = copyplist(rwp->r_rhs, TRUE);
#ifdef DEBUG
			if (tTd (39, 1))
			    printav1 (rwp->r_rhs, '\n');
#endif DEBUG
			break;

		  case 'S':		/* select rewriting set */
			ruleset = atoi(&rec.buf[1]);
#ifdef DEBUG
			if (tTd (39, 1))
			    printf ("ruleset %d\n", ruleset);
#endif DEBUG
			if (ruleset >= MAXRWSETS || ruleset < 0)
			{
				syserr(MSGSTR(CF_ERULE, "Bad ruleset %d (%d max)"), ruleset, MAXRWSETS); /*MSG*/
				errors++;
				break;
			}
			rwp = NULL;
			break;

		  case 'D':		/* macro definition */
			if ((rec.buf[1] == 'w' || rec.buf[1] == 'D')  &&
			    !validdomain(munchstring(&rec.buf[2], 
			     rec.buf[1]=='D'))) {
				syserr(MSGSTR(CF_EDOMAIN,
				"Illegal character in host/domain name \"%s\""),
				    &rec.buf[2]);
				errors++;
			} else {
			    freemac(rec.buf[1], CurEnv);  /* free if defined */
			    define(rec.buf[1], 
				newstr(munchstring(&rec.buf[2])), CurEnv); 
			}
			break;

		  case 'H':		/* required header line */
			(void) chompheader(&rec.buf[1], TRUE);
			break;

		  case 'C':		/* word class */
		  case 'F':		/* word class from file */
			/* read list of words from argument or file */
			if (rec.buf[0] == 'F')
			{
			    /* read from file */
			    for (p = &rec.buf[2]; *p != '\0' && !isspace(*p); p++)
			    	continue;
			    if (*p == '\0')
			    	p = "%s";
			    else
			    {
			    	*p = '\0';
			    	while (isspace(*++p))
			    		continue;
			    }
			    fileclass(rec.buf[1], &rec.buf[2], p);
			    break;
			}

			/* scan the list of words and set class for all */
			for (p = &rec.buf[2]; *p != '\0'; )
			{
				register char *wd;
				char delim;

				while (*p != '\0' && isspace(*p))
					p++;
				wd = p;
				while (*p != '\0' && !isspace(*p))
					p++;
				delim = *p;
				*p = '\0';
				if (wd[0] != '\0')
					setclass(rec.buf[1], wd);
				*p = delim;
			}
			break;

		  case 'M':		/* define mailer */
# ifdef DEBUG
			if (tTd(39, 1))
			      (void) printf("mailer path %s\n", &rec.buf[1]);
# endif DEBUG
			makemailer(&rec.buf[1]);
			break;

		  case 'O':		/* set option */
			/*
			 *  "safe" flag is TRUE because we are coming from a
			 *  configuration file.  Access to config files is
			 *  protected elsewhere.  Options from the config
			 *  file are not sticky.
			 */
			if (setoption(rec.buf[1], &rec.buf[2], TRUE, FALSE))
			    errors++;

			break;

		  case 'P':		/* set precedence */
			if (NumPriorities >= MAXPRIORITIES)
			{
				syserr(MSGSTR(CF_EPLINE, "Too many 'P'lines, %d max"),  MAXPRIORITIES); /*MSG*/
				errors++;
				break;
			}
			for(p = &rec.buf[1]; *p != '\0' && *p != '=' && *p != '\t'; p++)
				continue;
			if (*p == '\0')
			{
				syserr (MSGSTR(CF_EPREC, "Bad precedence line")); /*MSG*/
				errors++;
				break;
			}

			*p = '\0';
			Priorities[NumPriorities].pri_name =newstr(&rec.buf[1]);
			Priorities[NumPriorities].pri_val = atoi(++p);
			NumPriorities++;
			break;

		  case 'T':		/* trusted user(s) */
			p = &rec.buf[1];
			while (*p != '\0')
			{
				while (isspace(*p))
					p++;
				q = p;
				while (*p != '\0' && !isspace(*p))
					p++;
				if (*p != '\0')
					*p++ = '\0';
				if (*q == '\0')
					continue;
				for (pv = TrustedUsers; *pv != NULL; pv++)
					continue;
				if (pv >= &TrustedUsers[MAXTRUST])
				{
					syserr(MSGSTR(CF_ETLI, "Too many 'T'lines, %d max"),  MAXTRUST); /*MSG*/
					errors++;
					break;
				}
				*pv = newstr(q);
			}
			break;

		  default:
			syserr(MSGSTR(CF_ECONTR, "Unknown control line \"%s\""), rec.buf); /*MSG*/
			errors++;
		}
	}

	FileName = NULL;		/* disable diag prtout of name */

	if (errors)			/* syntax errs in lines? */
	    return (EX_DB);

	return (EX_OK);
}
/*
**  FILECLASS -- read members of a class from a file
**
**	Parameters:
**		class -- class to define.
**		filename -- name of file to read.
**		fmt -- scanf string to use for match.
**
**	Returns:
**		none
**
**	Side Effects:
**
**		puts all lines in filename that match a scanf into
**			the named class.
*/

fileclass(class, filename, fmt)
	int class;
	char *filename;
	char *fmt;
{
	FILE *f;
	char buf[MAXLINE];

	f = fopen(filename, "r");
	if (f == NULL)
	{
		syserr(MSGSTR(CF_EOPEN, "Cannot open fileclass file \"%s\""), filename); /*MSG*/
		return;
	}

	while (fgets(buf, sizeof buf, f) != NULL)
	{
		register STAB *s;
		register char *p;
		char wordbuf[MAXNAME+1];

		if (sscanf(buf, fmt, wordbuf) != 1)
			continue;
		p = wordbuf;

		/*
		**  Break up the match into words.
		*/

		while (*p != '\0')
		{
			register char *q;

			/* strip leading spaces */
			while (isspace(*p))
				p++;
			if (*p == '\0')
				break;

			/* find the end of the word */
			q = p;
			while (*p != '\0' && !isspace(*p))
				p++;
			if (*p != '\0')
				*p++ = '\0';

			/* enter the word in the symbol table */
			s = stab(q, ST_CLASS, ST_ENTER);
			setbitn(class, s->s_class);
		}
	}

	(void) fclose(f);
}
/*
**  MAKEMAILER -- define a new mailer.
**
**	Parameters:
**		line -- description of mailer.  This is in labeled
**			fields.  The fields are:
**			   P -- the path to the mailer
**			   F -- the flags associated with the mailer
**			   A -- the argv for this mailer
**			   S -- the sender rewriting set
**			   R -- the recipient rewriting set
**			   E -- the eol string
**			The first word is the canonical name of the mailer.
**
**	Returns:
**		none.
**
**	Side Effects:
**		enters the mailer into the mailer table.
*/

makemailer(line)
	char *line;
{
	register char *p;
	register struct mailer *m;
	register STAB *s;
	int i;
	char fcode;

	/* allocate a mailer and set up defaults */
	m = (struct mailer *) xalloc(sizeof (*m));
	(void) memset((char *) m, 0, sizeof (*m));
	m->m_mno = NextMailer;
	m->m_eol = newstr("\n");

	/* collect the mailer name */
	for (p = line; *p != '\0' && *p != ',' && !isspace(*p); p++)
		continue;
	if (*p != '\0')
		*p++ = '\0';
	m->m_name = newstr(line);

	/* now scan through and assign info from the fields */
	while (*p != '\0')
	{
		while (*p != '\0' && (*p == ',' || isspace(*p)))
			p++;

		/* p now points to field code */
		fcode = *p;
		while (*p != '\0' && *p != '=' && *p != ',')
			p++;
		if (*p++ != '=')
		{
			syserr(MSGSTR(CF_EQU, "`=' expected")); /*MSG*/
			return;
		}
		while (isspace(*p))
			p++;

		/* p now points to the field body */
		p = munchstring(p);

		/* install the field into the mailer struct */
		switch (fcode)
		{
		  case 'P':		/* pathname */
			m->m_mailer = newstr(p);
			break;

		  case 'F':		/* flags */
			for (; *p != '\0'; p++)
				setbitn(*p, m->m_flags);
			break;

		  case 'S':		/* sender rewriting ruleset */
		  case 'R':		/* recipient rewriting ruleset */
			i = atoi(p);
			if (i < 0 || i >= MAXRWSETS)
			{
				syserr(MSGSTR(CF_EREW, "Invalid rewrite set, %d max"), MAXRWSETS); /*MSG*/
				return;
			}
			if (fcode == 'S')
				m->m_s_rwset = i;
			else
				m->m_r_rwset = i;
			break;

		  case 'E':		/* end of line string */
			free(m->m_eol);  /* free the default */
			m->m_eol = newstr(p);
			break;

		  case 'A':		/* argument vector */
			m->m_argv = makeargv(p);
			break;

		  case 'M':		/* maximum message size */
			m->m_maxsize = atol(p);
			break;
		}

		p = DelimChar;
	}

	/* now store the mailer away */
	if (NextMailer >= MAXMAILERS)
	{
		syserr(MSGSTR(CF_EMAILER, "Too many mailers defined (%d max)"), MAXMAILERS); /*MSG*/
		return;
	}
	Mailer[NextMailer++] = m;
	s = stab(m->m_name, ST_MAILER, ST_ENTER);
	s->s_mailer = m;
}
/*
**  MUNCHSTRING -- translate a string into internal form.
**
**	Parameters:
**		p -- the string to munch.
**
**	Returns:
**		the munched string.
**
**	Side Effects:
**		Sets "DelimChar" to point to the string that caused us
**		to stop.
*/

char *
munchstring(p)
	register char *p;
{
	register char *q;
	int backslash = FALSE;
	int quotemode = FALSE;
	static char buf[MAXLINE];

	for (q = buf; *p != '\0'; p++)
	{
		if (backslash)
		{
			/* everything is roughly literal */
			backslash = FALSE;
			switch (*p)
			{
			  case 'r':		/* carriage return */
				*q++ = '\r';
				continue;

			  case 'n':		/* newline */
				*q++ = '\n';
				continue;

			  case 'f':		/* form feed */
				*q++ = '\f';
				continue;

			  case 'b':		/* backspace */
				*q++ = '\b';
				continue;
			}
			*q++ = *p;
		}
		else
		{
			if (*p == '\\')
				backslash = TRUE;
			else if (*p == '"')
				quotemode = !quotemode;
			else if (quotemode || *p != ',')
				*q++ = *p;
			else
				break;
		}
	}

	DelimChar = p;
	*q++ = '\0';
	return (buf);
}
/*
**  MAKEARGV -- break up a string into words
**
**	Parameters:
**		p -- the string to break up.
**
**	Returns:
**		a char **argv (dynamically allocated)
**
**	Side Effects:
**		munges p.
*/

char **
makeargv(p)
	register char *p;
{
	char *q;
	int i;
	char **avp;
	char *argv[MAXPV + 1];

	/* take apart the words */
	i = 0;
	while (*p != '\0' && i < MAXPV)
	{
		q = p;
		while (*p != '\0' && !isspace(*p))
			p++;
		while (isspace(*p))
			*p++ = '\0';
		argv[i++] = newstr(q);
	}
	argv[i++] = NULL;

	/* now make a copy of the argv */
	avp = (char **) xalloc((int) sizeof (*avp) * i);
	(void) memcpy((char *) avp, (char *) argv, (int) sizeof (*avp) * i);

	return (avp);
}
/*
**  PRINTRULES -- print rewrite rules (for debugging)
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		prints rewrite rules.
*/

# ifdef DEBUG

printrules()
{
	register struct rewrite *rwp;
	register int ruleset;

	for (ruleset = 0; ruleset < MAXRWSETS; ruleset++)
	    if (RewriteRules[ruleset]) {
		(void) printf("Rule Set %d:\n", ruleset);

		for (rwp = RewriteRules[ruleset]; rwp; rwp = rwp->r_next) {
		    (void) fputs("  LHS:", stdout);
		    printav(rwp->r_lhs);
		    (void) fputs("  RHS:", stdout);
		    printav(rwp->r_rhs);
		}
	    }
}

# endif DEBUG
/*
**  SETOPTION -- set global processing option
**
**	Parameters:
**		opt -- option name.
**		val -- option value (as a text string).
**		safe -- set if this came from a configuration file or internal
**			operations of sendmail.  unset if coming from sendmail
**			invocation flags.
**		sticky -- if set, subsequent setoptions for this option will
**			be ignored (except for M, macros).
**
**	Returns:
**		EX_xx return code.
**
**	Side Effects:
**		Sets options as implied by the arguments.
*/

static BITMAP	StickyOpt;		/* set if option is stuck */

setoption(opt, val, safe, sticky)
	char opt;
	char *val;
	int safe;
	int sticky;
{

register char *cp;
register struct resource_records *rrp;
static char ws[] = " \t";  /* whitespace delimiters for strtok */

# ifdef DEBUG
	if (tTd(38, 1))
		(void) printf("setoption %c=%s", opt, val);
# endif DEBUG

	/*
	**  See if this option is preset for us.
	*/

	if (bitnset(opt, StickyOpt))
	{
# ifdef DEBUG
		if (tTd(38, 1))
			(void) printf(" (ignored)\n");
# endif DEBUG
		return (EX_OK);
	}

# ifdef DEBUG
	if (tTd(38, 1))
		(void) printf("\n");
# endif DEBUG

	/*
	 *  If real uid is root or real gid is system, then anything is safe.
	 */
	if (Either)
	    safe = TRUE;

	/*
	 *  If real uid/gid is other, then certain options aren't OK.
	 */
	if (!safe && (int) strchr("defimorsv", opt) == (int)NULL)
	{
	    syserr (MSGSTR(CF_INV, "Option '%c' invalid unless user=root or group=system"),  opt); /*MSG*/
	    return (EX_NOPERM);
	}

	switch (opt)
	{
	  case 'A':                        /* set alias file */
		/*
		 *  This path must be absolute, since sendmail may not
		 *  be in its home directory when the database is opened.
		 *  This option is only available if "safe".
		 */
		if (val[0] != '/')
		{
		    syserr (MSGSTR(CF_ABS, "The 'A' option requires an absolute path")); /*MSG*/
		    return (EX_USAGE);
		}
		AliasFile = newstr(val);
		break;

	  case 'B':		/* substitution for blank character */
		/*
		 *  This option is only available if "safe".
		 */
		SpaceSub = val[0];
		if (SpaceSub == '\0')
			SpaceSub = ' ';
		break;

	  case 'b':		/* NetCode for Kanji Environment */
		/*
		 *  This option is only available if "safe".
		 */
		NetCode = newstr(val);
		break;

	  case 'c':		/* don't connect to "expensive" mailers */
		/*
		 *  This option is only available if "safe".
		 */
		NoConnect = atobool(val);
		break;

	  case 'd':		/* delivery mode */
		switch (*val)
		{
		  case SM_QUEUE:	/* queue only */
		  case SM_DELIVER:	/* do everything */
		  case SM_FORK:		/* fork after verification */
			SendMode = *val;
			break;

		  default:
			if (*val != '\0')
			    syserr (MSGSTR(CF_EMODE, "Unknown delivery mode '%c'"), *val); /*MSG*/
			else
			    syserr (MSGSTR(CF_EMODE2, "Unspecified delivery mode")); /*MSG*/
		        return (EX_USAGE);
		}
		break;

	  case 'e':		/* set error processing mode */
		switch (*val)
		{
		  case EM_QUIET:	/* be silent about it */
		  case EM_MAIL:		/* mail back */
		  case EM_BERKNET:	/* do berknet error processing */
		  case EM_WRITE:	/* write back (or mail) */
			HoldErrs = TRUE;
			/* fall through... */

		  case EM_PRINT:	/* print errors normally (default) */
			ErrorMode = *val;
			break;

		  default:
			if (*val != '\0')
			    syserr (MSGSTR(CF_EMODE3, "Unknown error mode '%c'"), *val); /*MSG*/
			else
			    syserr (MSGSTR(CF_EMODE4, "Unspecified error mode")); /*MSG*/
		        return (EX_USAGE);
		}
		break;

	  case 'E':		/* read timeout awaiting reply to RCPT */
		RcptTimeout = convtime(val);
		break;

	  case 'F':             /* file mode */
		FileMode = atooct(val) & 0777;
		break;

	  case 'f':		/* save Unix-style From lines on front */

		SaveFrom = atobool(val);
		break;

	  case 'g':		/* default gid */
		/*
		 *  This option is only available if "safe".
		 */
		DefGid = atoi(val);
		break;

	  case 'G':		/* write timeout for smtp */
		TransmitTimeout = convtime(val);
		break;

	  case 'h':	/* alternate address limit for multihomed hosts */
		/*
		 *  This option is only available if "safe".
		 */
		AltAddrLimit = atoi(val);
		break;

	  case 'i':		/* ignore dot lines in message */
		IgnrDot = atobool(val);
		break;

	  case 'I':		/* use internet domain name server */
		UseNameServer = atobool(val);
		break;

	  case 'J':		/* Ined editor in use. Destroys tabs.*/
		Ined = TRUE;
		break;

	  case 'k':		/* do not transform the body of any mail */
		/*
		 * This option allows AIX to operate exactly like versions
		 * prior to Version 3.0.  It prevents any changes to the
		 * body of mail items.
		 *  This option is only available if "safe".
		 */
		NlEsc = FALSE;
		break;

	  case 'K':		/* name server options */
		/*
		 *  The value is a list of nameserver resource record (RR)
		 *  mnemonic names; we parse these and turn on the corresponding
		 *  bits in the global bitmask.  We handle the special case of
		 *  "ALL" to mean turn on all supported RRs.
		 */

		for (cp = val; *cp = toupper(*cp); ++cp);  /* uppercase val */

		/* check each word in val; handle ALL and supported RRs */

		for (cp = strtok(val, ws); cp; cp = strtok(NULL, ws)) {
		    if (! strcmp(cp, "ALL")) {  /* set all supported RRs */
			for (rrp = ResourceRecords; rrp->value; ++rrp)
			    setbitn(rrp->value, NameServOpt);
			break;  /* all done */

		    } else {  /* check if it's supported */
			for (rrp = ResourceRecords; rrp->value; ++rrp)
			    if (! strcmp(cp, rrp->name)) {  /* found it */
				setbitn(rrp->value, NameServOpt);
				break;
			    }
			if (! rrp->value) {  /* didn't find RR type */
			    syserr(MSGSTR(CF_BADRR,
			    "Unsupported nameserver resource record \"%s\""),
				cp);
			    return(EX_USAGE);
			}
		    }
		}
		if (! NameServOpt) {  /* nothing found */
		    syserr(MSGSTR(CF_NORR,
			"Unspecified nameserver resource record(s)"));
		    return(EX_USAGE);
		}
		break;

	  case 'l':                        /* set sendmail.nl file */
		/*
		 *  This path must be absolute, since sendmail may not
		 *  be in its home directory when the database is opened.
		 *  This option is only available if "safe".
		 */
		if (val[0] != '/')
		{
		    syserr (MSGSTR(CF_KOPT, "The 'l' option requires an absolute path")); /*MSG*/
		    return (EX_USAGE);
		}
		NlFile = newstr(val);
		break;

	  case 'L':		/* log level */
		/*
		 *  This option is only available if "safe".
		 */
		LogLevel = atoi(val);
		break;

	  case 'M':		/* define macro */
		/*
		 *  This option is only available if "safe".
		 *  This option never sticks.
		 */
		freemac(val[0], CurEnv);  /* free it if defined */
		define(val[0], newstr(&val[1]), CurEnv);
		sticky = FALSE;
		break;

	  case 'm':		/* send to me too */
		MeToo = atobool(val);
		break;

	  case 'n':		/* validate RHS in newaliases */
		/*
		 *  This option is only available if "safe".
		 */
		CheckAliases = atobool(val);
		break;

	  case 'o':		/* assume old style headers */
		if (atobool(val))
			CurEnv->e_flags |= EF_OLDSTYLE;
		else
			CurEnv->e_flags &= ~EF_OLDSTYLE;
		break;
	
	  case 'O':		/* MailCode for Kanji Environment */
		/*
		 *  This option is only available if "safe".
		 */
		MailCode = newstr(val);
		break;

	  case 'p':	/* set alias map name for yellow pages */
		AliasMap = newstr(val);
		/*
                 * Trim all of the trailing spaces off of the yp map name
                 * This gets confusing otherwise.
                 */
                while (isspace(AliasMap[strlen(AliasMap) - 1]))
                        AliasMap[strlen(AliasMap) - 1] = NULL;
		break;

	  case 'P':		/* postmaster copy address for returned mail */
		/*
		 *  This option is only available if "safe".
		 */
		PostMasterCopy = newstr(val);
		break;

	  case 'q':		/* slope of queue only function */
		/*
		 *  This option is only available if "safe".
		 */
		QueueFactor = atoi(val);
		break;

	  case 'Q':		/* queue directory */
		/*
		 *  It is best that this path be absolute. 
		 *  This option is only available if "safe".
		 */
		if (val[0] != '/')
		{
		    syserr (MSGSTR(CF_EQOPT, "The 'Q' option requires an absolute path")); /*MSG*/
		    return (EX_USAGE);
		}
		QueueDir = newstr(val);
		break;

	  case 'R':		/* read timeout awaiting reply to DATA */
		DataTimeout = convtime(val);
		break;

	  case 'r':		/* read timeout */
		ReadTimeout = convtime(val);
		break;

	  case 'S':		/* status file */
		/*
		 *  This option is only available if "safe".
		 */
		if (val[0] != '/')
		{
		    syserr (MSGSTR(CF_ESOPT, "The 'S' option requires an absolute path")); /*MSG*/
		    return (EX_USAGE);
		}
		StatFile = newstr(val);
		break;

	  case 's':		/* be super safe, even if expensive */
		SuperSafe = atobool(val);
		break;

	  case 'T':		/* queue timeout */
		/*
		 *  This option is only available if "safe".
		 */
		TimeOut = convtime(val);
		break;

	  case 'U':		/* read timeout while awaiting greeting */
		GreetTimeout = convtime(val);
		break;

	  case 'u':		/* set default uid */
		/*
		 *  This option is only available if "safe".
		 */
		DefUid = atoi(val);
		setdefuser();
		break;

	  case 'V':		/* read timeout awaiting reply to MAIL */
		MailTimeout = convtime(val);
		break;

	  case 'v':		/* run in verbose mode */
		Verbose = atobool(val);
		break;

	  case 'W':		/* read timeout awaiting reply to '.' */
		PeriodTimeout = convtime(val);
		break;

	  case 'w':		/* ISO-8859/1 option */
		/* With this option set, all incoming mail which does not
		 * have an "X-NLesc" header but has 8-bit data in the body
		 * will be assumed to be encoded in the ISO-8859/1 character 
		 * set.  If the mail is to be delivered locally, the body will
		 * be encoded in the NLS character set.
		 *  This option is only available if "safe".
		 */
		IsoIn = TRUE;
		break;

	  case 'x':		/* load avg at which to auto-queue msgs */
		/*
		 *  This option is only available if "safe".
		 */
		QueueLA = atoi(val);
		break;

	  case 'X':	/* load avg at which to auto-reject connections */
		/*
		 *  This option is only available if "safe".
		 */
		RefuseLA = atoi(val);
		break;

	  case 'y':		/* work recipient factor */
		/*
		 *  This option is only available if "safe".
		 */
		WkRecipFact = atoi(val);
		break;

	  case 'Y':		/* fork jobs during queue runs */
		/*
		 *  This option is only available if "safe".
		 */
		ForkQueueRuns = atobool(val);
		break;

	  case 'z':		/* work message class factor */
		/*
		 *  This option is only available if "safe".
		 */
		WkClassFact = atoi(val);
		break;

	  case 'Z':		/* work time factor */
		/*
		 *  This option is only available if "safe".
		 */
		WkTimeFact = atoi(val);
		break;

	  case '+':		/* secure SMTP */
		/*
		 *  This option is only available if "safe".
		 */
		SecureSMTP = atobool(val);
		break;

	  case '-':		/* log SMTP user queries */
		/*
		 *  This option is only available if "safe".
		 */
		LogSMTP = atobool(val);
		break;

	  case 'C':		/* checkpoint after N connections */
	  case 'D':		/* rebuild alias database as needed */
	  case 'H':		/* help file */
	  case 'N':		/* home (local?) network name */
	  case 'a':		/* look N minutes for "@:@" in alias file */
	  case 't':		/* time zone name */
		/*
		 *  These are currently unsupported, but must be accepted
		 *  to maintain compatibility.
		 */
		if (OpMode == MD_FREEZE)  /* only warn them on freeze */
		    syslog(LOG_WARNING, MSGSTR(CF_IGN,
			"Configuration option '%c' is not supported (ignored)"),
			opt);
		break;

	  default:
		syserr (MSGSTR(CF_ECF, "Unknown configuration option '%c'"), opt); /*MSG*/
		return (EX_USAGE);
	}

	if (sticky)
		setbitn(opt, StickyOpt);

	return (EX_OK);
}
/*
**  SETCLASS -- set a word into a class
**	The word is passed through expand () to handle embedded macros.
**	(Remember these start with ctrl-A, so real $ signs that come
**	here won't cause macro expansion.  This is important for the
**	setclass () calls in main () defining class 'w'.)
**
**	Parameters:
**		class -- the class to put the word in.
**		word -- the word to enter
**
**	Returns:
**		none.
**
**	Side Effects:
**		puts the word into the symbol table as a class item.
*/

setclass(class, word)
	int class;
	char *word;
{
	register STAB *s;
	char  buf[MAXNAME];

	/*
	 *  Allow for definition of class word from predefined macros in
	 *  config file.
	 */
	expand (word, buf, &buf[sizeof buf - 1], CurEnv);

	/*
	 *  Enter the word as a generic class item in the symbol table, or 
	 *  locate it if already there.  A word is entered only once, no 
	 *  matter how many different classes it may be defined in.
	 */
	s = stab (buf, ST_CLASS, ST_ENTER);

	/*
	 *  Set the proper class bit in the symbol table entry.
	 */
	setbitn (class, s->s_class);
}

/*
**  FREEZECF -- read control file and create corresponding database.
**
**	This routine reads the control file and builds the database
**	for optimized loading.
**
**	The file is formatted as a sequence of lines.  Continuation lines
**	are allowed and are provided for by using the "fgetfolded" routine.
**	The first character of each line describes how the line is to be
**	interpreted.  Nonempty lines are transferred as-is to the output
**	freeze file, except for lines beginning with '#' (comment).
**	The scan for replacement of $ by SOH is also performed.
**
**	Parameters:
**		cfname -- control file name (input).
**
**	Returns:
**		EX_xx completion code.
**
**	Side Effects:
**		Creates database under <cfname>DB.
*/

freezecf (cfname)
	char *cfname;
{
	FILE *cf;
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
	if (cfname == NULL)
	{
	    syserr (MSGSTR(CF_EPATH, "No valid config file path; data base not initialized")); /*MSG*/
	    return (EX_DB);
	}

	/*
	 *  There is no way to protect against the INPUT config file being
	 *  changed (say by a text editor) while it is being read for
	 *  purposes of rebuilding the corresponding data base.  AIX does
	 *  not provide the necessary exclusive open operation.  Lockf
	 *  operations are normally voluntary and are not performed by
	 *  text editors.
	 *
	 *  This problem is overcome by the operational stipulation that
	 *  the system manager, only, be permitted to update the a config
	 *  text file; and furthermore that no one but the system manager
	 *  be allowed to perform the necessary "sendmail -bz" 
	 *  operations.  He will perform the text update and data base
	 *  rebuild serially for any given config file.  Existing 
	 *  processes will continue to use the old data base until the data
	 *  base reconstruction operation finishes.
	 *
	 *  There is currently no protection against the system manager
	 *  performing two or more simultaneous data base runs for the
	 *  same data base.  Again, this is solved operationally by
	 *  correct procedures on the part of the system manager.
	 *  However, if necessary, interlock can be programmed by locking
	 *  on the input config file.
	 *
	 *  ANOTHER OPERATIONAL PROBLEM:  If the daemon is using the data
	 *  base being updated, it will have to be killed and restarted
	 *  after the update is complete.
	 *
	 *  The sendmail program, configuration file, and instructions
	 *  for use are modified to reflect the fact that only the system
	 *  manager can perform the freeze operation.
	 *
	 *  SPACE PROBLEM:  This update procedure requires enough additional
	 *  space in the minidisk containing /usr/lib to allow for 
	 *  creation of the temporary subdirectory for the new data base.
	 *  This may be a problem if there is not enough free space customarily 
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
	 *  Create required file names.
	 */
	if (cat ( tdbname, sizeof  tdbname, cfname, "DBt") == NULL ||
	    cat (  dbname, sizeof   dbname, cfname, "DB" ) == NULL ||
	    cat (lockfile, sizeof lockfile, cfname, "DBl") == NULL)
	{
	    syserr (MSGSTR(CF_ELONG3, "Config path \"%s\", too long"), cfname); /*MSG*/
	    return (EX_DB);
	}

	/*
	 *  Isolate file name portion and assure that names will be 
	 *  distinguishable.
	 */
	afn = strrchr (cfname, '/');
	if (afn == NULL)  afn = cfname;
	else		  afn++;

	if (strlen (afn) > MAXFNAME - strlen ("DBt"))
	{
	    syserr (MSGSTR(CF_ELONG4, "Config file name \"%s\", too long"), afn); /*MSG*/
	    return (EX_DB);
	}

	/*
	 *  Create a new work file at named "<cfname>DBt".
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
	if ((fc = open (tdbname, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0)
	{
		syserr(MSGSTR(CF_ETEMP, "Cannot create temporary config data base \"%s\""), tdbname); /*MSG*/
		return (EX_DB);
	}
	/*
	 *  From this point on, close output file before any exit.
	 */

	/*
	 *  Get access to input configuration file.
	 */
	cf = fopen(cfname, "r");
	if (cf == NULL)
	{
		syserr(MSGSTR(CF_EOPEN2, "Cannot open configuration file \"%s\""), cfname); /*MSG*/
		(void) close (fc);
		return(EX_DB);
	}
	/*
	 *  From this point on, close input file before any exit.
	 */

	FileName = cfname;              /* for error reporting          */
	LineNumber = 0;			/* gets incr'd in fgetfolded	*/

	err = f_process (cf, fc, cfname, tdbname);  /* do file processing */

	(void) fclose (cf);             /* always close all the files   */
	(void) close (fc);

	FileName = NULL;		/* turn off file name diags */

	if (err)                        /* if error state, just exit now */
	    return (err);

	/*
	 *  Serialize database update "mv" operation via lock file.
	 */
	fdl = open (lockfile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if (fdl < 0)
	{
	    syserr (MSGSTR(CF_ELOCK, "Unable to open configuration lock file \"%s\""), lockfile); /*MSG*/
	    return (EX_DB);
	}

	if (lockf (fdl, F_LOCK, 0) < 0)
	{
	    syserr (MSGSTR(DF_ELOCK2, "Unable to lock configuration lock file \"%s\""), lockfile); /*MSG*/
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

	return (EX_OK);
}

/*
 *  Perform processing loop for creating freeze file.
 *
 *  The format of a generated record in general is:
 *
 *  field 1) short: line number of this line in the original config file.
 *  field 2) short: length in bytes of the next complete record, header and all.
 *  field 3)  char: array of characters of size >= 1 and <= MAXLINE.
 *		    the array is always terminated by a null character.
 *
 *  In the first record, the field 1 is the total rule count instead, and the
 *  buffer length is one (containing a null).  In the last record, field 2 == 0.
 */
static  int f_process (FILE *cf, int fc, char *cfname, char *tdbname)
{
	int  prev_l, il, rules, errors, lineno;
	struct cfrec rec;
	char c, ibuf[MAXLINE];

	errors = 0;			/* clear error counter		*/
	rules = 0;			/* clear rule counter		*/
	prev_l = sizeof (struct cfrechdr) + 1; /* first rec has header, null */
	rec.hdr.lineno = 0;		/* clear the rule count		*/
	rec.buf[0] = '\0';		/* contains null only		*/

	while (1)
	{
		/*
		 *  Get next possibly folded text line, if any.
		 */
		if (fgetfolded (ibuf, MAXLINE, cf) == NULL)
		    break;

		lineno = LineNumber;	/* save lineno of current line */

		/*
		 *  Process the line.  Return zero to ignore, else, length.
		 */
		if (!(il = scanline (ibuf)))
		    continue;

		/*
		 *  Check lines for acceptable types.  Accumulate rule count.
		 */
		c = *ibuf;
		switch (c)
		{
		  case 'R':		/* rewriting rule */
			rules++;	/* keep count */
		  case 'S':		/* select rewriting set */
		  case 'D':		/* macro definition */
		  case 'H':		/* required header line */
		  case 'C':		/* word class */
		  case 'F':		/* word class from file */
		  case 'M':		/* define mailer */
		  case 'O':		/* set option */
		  case 'P':		/* set precedence */
		  case 'T':		/* trusted user(s) */
			break;		/* these are OK */

		  default:
			syserr(MSGSTR(CF_ECONTR, "Unknown control line \"%s\""), ibuf); /*MSG*/
			errors++;
			continue;	/* don't write anything */
		}

		/*
		 *  Write out the previous line.  It is holding for the
		 *  length of the current line.
		 */
		il += sizeof (struct cfrechdr); /* form total record size */

		rec.hdr.len = il;	/* fill it in previous record */
		if (write (fc, (char *) &rec, (unsigned) prev_l) != prev_l)
		{
		    syserr (MSGSTR(CF_EWRITE, "Error writing \"%s\""), tdbname); /*MSG*/
		    return (EX_DB);
		}
#ifdef DEBUG
		if (tTd (37, 1))
		    (void) printf ("%4d: size %3d  next-size %3d  buf <%s>\n", 
			rec.hdr.lineno, prev_l, rec.hdr.len, rec.buf);
#endif DEBUG

		/*
		 *  Move current line to previous line.
		 */
		prev_l = rec.hdr.len;
		rec.hdr.lineno = lineno;
		(void) memcpy (rec.buf, ibuf, il);
	}

	/*
	 *  Check for any errors on reading config file.
	 */
	if (ferror (cf))
	{
	    syserr (MSGSTR(CF_EREAD2, "Error reading \"%s\""), cfname); /*MSG*/
	    return (EX_DB);
	}

	/*
	 *  Write out previous line.  There is no current line.
	 */
	rec.hdr.len = 0;		/* eof flag			*/
	if (write (fc, (char *) &rec, (unsigned) prev_l) != prev_l)
	{
	    syserr (MSGSTR(CF_EWRITE, "Error writing \"%s\""), tdbname); /*MSG*/
	    return (EX_DB);
	}

#ifdef DEBUG
	if (tTd (37, 1))
	    (void) printf ("%4d: size %3d  next-size %3d  buf <%s>\n", 
			rec.hdr.lineno, prev_l, rec.hdr.len, rec.buf);
#endif DEBUG

	/*
	 *  Update the rules count in the first record.
	 */
	prev_l = sizeof (struct cfrechdr) + 1;	/* rec size */

	if (lseek (fc, (off_t) 0, 0) != 0)
	{
	    syserr (MSGSTR(CF_ESEEK, "Error seeking to 0 in \"%s\""), tdbname); /*MSG*/
	    return (EX_DB);
	}

	if (read (fc, (char *) &rec, (unsigned) prev_l) != prev_l)
	{
	    syserr (MSGSTR(CF_EREAD2, "Error reading \"%s\""), tdbname); /*MSG*/
	    return (EX_DB);
	}

	rec.hdr.lineno = rules;		/* update the rule count */

	if (lseek (fc, (off_t) 0, 0) != 0)
	{
	    syserr (MSGSTR(CF_ESEEK, "Error seeking to 0 in \"%s\""), tdbname); /*MSG*/
	    return (EX_DB);
	}

	if (write (fc, (char *) &rec, (unsigned) prev_l) != prev_l)
	{
	    syserr (MSGSTR(CF_EWRITE, "Error writing \"%s\""), tdbname); /*MSG*/
	    return (EX_DB);
	}

#ifdef DEBUG
	if (tTd (37, 1))
	  (void) printf ("rule count: %d: size %3d  next-size %3d  buf <%s>\n", 
			rec.hdr.lineno, prev_l, rec.hdr.len, rec.buf);
#endif DEBUG

	if (errors)
	    return (EX_DB);

	return (EX_OK);
}

/*
 *  scanline - get line length.
 */
static int  scanline (char *ibuf)
{
	register char *p, *q, c;
	int  len;

	c = *ibuf;		/* first char of line		*/

	/*
	 *  Eliminate empty lines and comments right now.
	 */
	if (c == '\0' || c == '#')
	    return (0);

	/*
	 *  Scan everything else.  Cause $$ -> $.  Cause $x, where
	 *  x != $, to go to SOHx.
	 *
	 * DOES THIS REALLY APPLY TO EACH KIND OF LINE?
	 *
	 *  Presumably, no lines from here on out will be empty, but it
	 *  should work if they are, anyway.  The processing program will
	 *  just throw them away.
	 */
	p = q = ibuf;
	for (; *p != '\0'; p++)		/* go till we see the null */
	{
		c = *p;

		if (c != '$')		/* normally TRUE	*/
		{
		    *q = c; q++;	/* just put it out and go on */
		    continue;
		}

		p++;			/* if $ look at next one */
		c = *p;
		if (c == '$')
		{
		    *q = '$'; q++;  /* if also $, put single $, go on */
		}
		else
		{
		    p--;		/* back up to $		*/
		    *q = '\001'; q++;  /* put out SOH, instead	*/
		}
	}
	*q = '\0'; q++;			/* put null terminator	*/
	len = (int) (q - ibuf);		/* size			*/

	return (len);
}

#ifdef DEBUG

/*
 *  dumpcf - dump config data to stdout
 */
void dumpcf(e, s)
register ENVELOPE *e;
char *s;
{
int i, j;
register char *fmt;
register struct mailer *m;
register HDR *h, **hp;

    /* macros */
    printf("\ndumpcf(%#x, %s) (Cur=%#x): macros:\n", e, s, CurEnv);
    for (i = 0; i < sizeof(e->e_macro) / sizeof(e->e_macro[0]); ++i)
	if (e->e_macro[i]) {  /* it's defined */
	    fmt = isprint(i) ? "  '%c' = " : "  %02x = ";
	    printf(fmt, i);
	    xputs(e->e_macro[i]);
	    putchar('\n');
	}

    /* classes */
    puts("\nclasses:");
    dumpstab(ST_CLASS);

    /* rewrite rules */
    putchar('\n');
    printrules();

    /* mailers */
    puts("\nmailers:");
    for (i = 0; i < MAXMAILERS; i++)
	if (m = Mailer[i]) {  /* defined */
	    printf("  %d (%s): P=%s S=%d R=%d M=%ld F=", 
		i, m->m_name, m->m_mailer, m->m_s_rwset, 
		m->m_r_rwset, m->m_maxsize);
	    for (j = '\0'; j <= '\177'; j++)
		if (bitnset(j, m->m_flags))
		    putchar(j);
	    fputs(" E=", stdout);
	    xputs(m->m_eol);
	    fputs(" A=", stdout);
	    printav(m->m_argv);
	}
    
    /* headers */
    puts("\nheaders:");
    for (hp = &e->e_header; h = *hp; hp = &h->h_link) {
	printf("  field='%s', value=", h->h_field);
	xputs(h->h_value);
	printf(", flags=%#x,\n    map=%#08x %08x %08x %08x\n",
	    h->h_flags, h->h_mflags[0], h->h_mflags[1],
	    h->h_mflags[2], h->h_mflags[3]);
    }

    /* priorities */
    puts("\npriorities:");
    for (i = 0; i < NumPriorities; ++i)
	printf("  %d: name='%s', val=%d\n", i, Priorities[i].pri_name,
	    Priorities[i].pri_val);
    
    /* trusted users */
    puts("\ntrusted users:");
    for (i = 0; TrustedUsers[i]; ++i)
	printf("  '%s'\n", TrustedUsers[i]);

    /* options */
    puts("\noptions:");
    printf("\
  AltAddrLimit=%d\n\
  AliasFile='%s'\n\
  SpaceSub='%c'\n\
  NoConnect=%d\n\
  GreetTimeout=%d\n\
  SendMode='%c'\n\
  MailTimeout=%d\n\
  ErrorMode='%c'\n\
  HoldErrs=%d\n\
  RcptTimeout=%d\n\
  SaveFrom=%d\n\
  DataTimeout=%d\n\
  DefGid=%d\n\
  TransmitTimeout=%d\n\
  PeriodTimeout=%d\n\
  IgnrDot=%d\n\
  UseNameServer=%d\n\
  Ined=%d\n\
  NlEsc=%d\n\
  NameServOpt=%#08x %08x %08x %08x\n\
  NlFile='%s'\n\
  LogLevel=%d\n\
  MeToo=%d\n\
  CheckAliases=%d\n\
  NetCode='%s'\n\
  MailCode='%s'\n\
  AliasMap='%s'\n\
  PostMasterCopy='%s'\n\
  QueueFactor=%d\n\
  QueueDir='%s'\n\
  ReadTimeout=%d\n\
  StatFile='%s'\n\
  SuperSafe=%d\n\
  TimeOut=%d\n\
  DefUid=%d\n\
  Verbose=%d\n\
  IsoIn=%d\n\
  QueueLA=%d\n\
  RefuseLA=%d\n\
  WkRecipFact=%d\n\
  ForkQueueRuns=%d\n\
  WkClassFact=%d\n\
  WkTimeFact=%d\n",
    AltAddrLimit, (AliasFile ? AliasFile : ""), SpaceSub, NoConnect, 
    GreetTimeout, SendMode, MailTimeout, ErrorMode, HoldErrs,
    RcptTimeout, SaveFrom, DataTimeout, DefGid, TransmitTimeout, 
    PeriodTimeout,IgnrDot, UseNameServer, Ined, NlEsc, NameServOpt[0],
    NameServOpt[1], NameServOpt[2], NameServOpt[3], (NlFile ? NlFile : ""), 
    LogLevel, MeToo, CheckAliases, (NetCode ? NetCode : ""), (MailCode ? 
    MailCode : ""), (AliasMap ? AliasMap : ""), (PostMasterCopy ? 
    PostMasterCopy : ""), QueueFactor, (QueueDir ? QueueDir : ""), ReadTimeout, 
    (StatFile ? StatFile : ""), SuperSafe, TimeOut, DefUid, Verbose, IsoIn, 
    QueueLA, RefuseLA, WkRecipFact, ForkQueueRuns, WkClassFact, WkTimeFact);
}

#endif DEBUG

/*
 *  freecf - free up the current config data to allow for a re-init and
 *  re-reading of the config file (sequence should be freecf(), initcf(),
 *  readcf()).
 *
 *  NOTE that this frees and clears all the configurable data structures,
 *  so BE VERY CAREFUL when and where you call this!!!
 */
void freecf()
{
/* addresses of global strings that are alloc()ed by readcf() */
static char **options[] = {
    &AliasFile, &NlFile, &NetCode, &MailCode, &AliasMap, &PostMasterCopy, &QueueDir, &StatFile, NULL };
int i;
register struct rewrite *rwp;
register HDR *h, *h_next;
register MAILER *m;
register char **ap, ***op;


    /* rewrite rules: step through set array, looking for defined sets;
       step through these liked lists, freeing merrily as we go */

    for (i = 0; i < MAXRWSETS; ++i) {
	rwp = RewriteRules[i];
# ifdef DEBUG
	if (tTd(0, 25) && rwp)
	    printf("freecf: freeing RewriteRules[%d]\n", i);
# endif DEBUG
	while (rwp) {  /* trace linked list */
	    if (rwp->r_rhs)  /* these are allocated by copyplist() */
		free(rwp->r_rhs);
	    if (rwp->r_lhs)
		free(rwp->r_lhs);
	    rwp = rwp->r_next;
	}
	RewriteRules[i] = NULL;
    }
    /* now free all the structs that were alloc()ed by readcf() */
    free(Rwpalloc);

    /* macros: clear the ptrs in the BlankEnvelope, which is the
       template envelope; NOTE that this assumes that all macros
       defined in the BlankEnvelope have been alloc()ed */
    
    for (i = 0; i < sizeof(BlankEnvelope.e_macro) /
	sizeof(BlankEnvelope.e_macro[0]); ++i)
	    if (BlankEnvelope.e_macro[i]) {  /* it's defined */
# ifdef DEBUG
		if (tTd(0, 25))
		    printf("freecf: freeing BlankEnvelope.e_macro[%d]\n", i);
# endif DEBUG
		free(BlankEnvelope.e_macro[i]);
		BlankEnvelope.e_macro[i] = NULL;
	    }
    
    /* headers: step through header list in BlankEnvelope and free them */

    h = BlankEnvelope.e_header;
    while (h) {
# ifdef DEBUG
	if (tTd(0, 25))
	    printf("freecf: freeing header '%s'\n", h->h_field);
# endif DEBUG
	free(h->h_value);  /* these alloc()ed by chompheader() */
	free(h->h_field);
	h_next = h->h_link;
	free(h);
	h = h_next;
    }
    BlankEnvelope.e_header = NULL;  /* clear list */

    /* classes: clear them from the symbol table */

    freestab(ST_CLASS);

    /* mailers: free and clear defined mailers, as well as their symbol
       table entries */
    
    for (i = 0; i < NextMailer; ++i) {
# ifdef DEBUG
	if (tTd(0, 25))
	    printf("freecf: freeing Mailer[%d]\n", i);
# endif DEBUG
	m = Mailer[i];
	free(m->m_name);  /* these alloc()ed by makemailer() */
	free(m->m_eol);
	if (m->m_mailer)
	    free(m->m_mailer);
	if (ap = m->m_argv) {  /* argv array */
	    while (*ap) {
# ifdef DEBUG
		if (tTd(0, 25)) {
		    printf("  freeing m_argv: ");
		    xputs(*ap);
		    putchar('\n');
		}
# endif DEBUG
		free(*ap++);
	    }
	    free(m->m_argv);
	}
	free(m);
    }
    NextMailer = 0;  /* clear array index */
    freestab(ST_MAILER);  /* clear mailers from symbol table */

    /* priorities: free and clear each struct */

    for (i = 0; i < NumPriorities; ++i) {
# ifdef DEBUG
	if (tTd(0, 25))
	    printf("freecf: freeing Priorities[%d]\n", i);
# endif DEBUG
	free(Priorities[i].pri_name);  /* alloc()ed by readcf() */
    }
    NumPriorities = 0;

    /* trusted users: free and clear each string */

    for (ap = TrustedUsers; *ap; ++ap) {
# ifdef DEBUG
	if (tTd(0, 25))
	    printf("freecf: freeing trusted user '%s'\n", *ap);
# endif DEBUG
	free(*ap);  /* alloc()ed by readcf() */
	*ap = NULL;
    }

    /* options: free and clear the strings alloc()ed by setoption() */

    for (op = options; *op; ++op)
	if (**op) {  /* has been alloc()ed */
# ifdef DEBUG
	    if (tTd(0, 25))
		printf("freecf: freeing option '%s'\n", **op);
# endif DEBUG
	    free(**op);
	    **op = NULL;
	}
}

/*
 *  initcf - init the globals that can be re-configured from the config file
 */
void initcf()
{
int i;

    QueueLA = 8;		/* shouldqueue says no below this load */	
    QueueFactor = 10000;	/* else shouldq compares arg to func of this */
    RefuseLA = 12;		/* daemon refuses connections over this load */
    SpaceSub = ' ';
    WkRecipFact = 1000;
    WkClassFact = 1800;
    WkTimeFact = 9000;
    FileMode = 0660;
    DefUid = 1;
    DefGid = 1;
    NlEsc = TRUE;
    setdefuser();

    /* these are reset to their initial state of zero, since they are static */

    for (i = BITMAPINTS - 1; i >= 0; --i)
	NameServOpt[i] = 0;
    AliasFile = NlFile = NetCode = MailCode = AliasMap = PostMasterCopy =
	QueueDir = StatFile = NULL;
    NoConnect = GreetTimeout = SendMode = MailTimeout = ErrorMode = 
        HoldErrs = RcptTimeout = SaveFrom = DataTimeout = TransmitTimeout = 
	PeriodTimeout = IgnrDot = UseNameServer = Ined = LogLevel = 
	MeToo = CheckAliases = ReadTimeout = SuperSafe = TimeOut =
	Verbose = IsoIn = ForkQueueRuns = AltAddrLimit = 0;
}

/*
 * validdomain() - verifies that a domain name conforms to RFC 822
 *                 standards, which specifies that the name have no 
 *                 control characters, spaces or any of ()<>@,;:".[] 
 *                 in the string.
 * 
 * Input
 *    name - domain name or component.
 *    dotok - TRUE if a dot (.) is allowed in the name.
 *
 * Returns 
 *    False (0) - if domain name is illegal.
 *    True (1) - if domain name is legal.
 */

#define BADDOMCH " ()<>@,;:\".[]"
#define BADDOMCH2 " ()<>@,;:\"[]"

int validdomain(name, dotok)
    char *name;
    int dotok;
{
    char *p;

    for (p=name; *p!='\0'; p++)
	if (strchr(dotok ? BADDOMCH2 : BADDOMCH, *p) != NULL || iscntrl(*p))
	    return(FALSE);

    return(TRUE);
}
