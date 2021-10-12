static char sccsid[] = "@(#)20  1.16.1.4  src/bos/usr/bin/que/pac.c, cmdque, bos411, 9428A410j 3/30/94 10:01:50";
/*
 * COMPONENT_NAME: (CMDQUE) Queueing system
 *
 * FUNCTIONS: pac
 *
 * ORIGINS: 26, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* static char sccsid[] = "pac.c	5.2 (Berkeley) 10/30/85"; */

/*
 * Do Printer accounting summary.
 * Currently, usage is
 *	pac [-Pprinter] [-pprice] [-qfilename] [-s] [-r] [-c] [-m] [user ...]
 * to print the usage information for the named people.
 */


#define	QCNFG_FILE	"/etc/qconfig"

/* 
 * Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n"; 
 */

#define _ILS_MACROS
#include <IN/standard.h>
#include "common.h"
#include "accrec.h"
#include <stdio.h>
#include <IN/AFdefs.h>
#include <IN/FSdefs.h>
#include <sys/param.h>
#include <stdlib.h>
#include <locale.h>
#include <monetary.h>
#include <fcntl.h>
#include <sys/limits.h>
#include <sys/errno.h>
#include <ctype.h>

#include        "pac_msg.h"


nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_PAC,num,str)  /*MSG*/
#define MONSIZE  20

char	*printer;			/* printer name */
char	*acctfile;			/* accounting file (input data) */
char	sumfile[MAXPATHLEN];		/* summary file */
float	price = 0.02;			/* cost per page (or what ever) */
int	allflag = 1;			/* Get stats on everybody */
int	sort;				/* Sort by cost */
int	summarize;			/* Compress accounting file */
int	reverse;			/* Reverse sort order */
int	hcount;				/* Count of hash entries */
int	mflag = 0;			/* disregard machine names */
int	pflag = 0;			/* 1 if -p on cmd line */
int	qflag = 0;			/* 1 if -q on cmd line */
int	price100;			/* per-page cost in 100th of a cent */
char	*index();
int	pgetnum();
char	*progname = "pac";
char    *stripstr();

/*
 * Grossness follows:
 *  Names to be accumulated are hashed into the following
 *  table.
 */

#define	HSHSIZE	97			/* Number of hash buckets */

struct hent {
	struct	hent *h_link;		/* Forward hash link */
	char	*h_name;		/* Name of this user */
	float	h_feetpages;		/* Feet or pages of paper */
	int	h_count;		/* Number of runs */
};

struct	hent	*hashtab[HSHSIZE];	/* Hash table proper */
struct	hent	*enter();
struct	hent	*lookup();
struct  q	*qlist;
struct  q       *pac_config();

#define	NIL	((struct hent *) 0)	/* The big zero */

char	*pgetstr();
static void	rewrite();
static void	dumpit();
static void	account();
extern struct q *default_queue(), *get_queue();
extern void resetQCBFile();
extern struct q *rcfg();

int	jul=0;				/* Debugging flag */
struct	lconv *l_conv;

main(argc, argv)
	int argc;
	char **argv;
{
	register FILE *acct;
	register char *cp;
	char str[MONSIZE];
	char *alt_price;	/* default price in qconfig file */
	char *qconfig = QCNFG_FILE;
	struct q *qptr;

	(void) setlocale (LC_ALL,"");

	catd = catopen(MF_PAC, NL_CAT_LOCALE);
	l_conv = localeconv();

	while (--argc) {
		cp = *++argv;
		if (*cp++ == '-') {
			switch(*cp++) {
			case 'P':
				/*
				 * Printer name.
				 */
				printer = cp;
				continue;

			case 'p':
				/*
				 * get the price.
				 */
			        strcpy(str,stripstr(cp));	
				price = atof(str);
				pflag = 1;
				continue;

			case 'q':
				/*
				 * new qconfig file
				 */
				qflag++;
				qconfig = cp;
				continue;

			case 's':
				/*
				 * Summarize and compress accounting file.
				 */
				summarize++;
				continue;

			case 'c':
				/*
				 * Sort by cost.
				 */
				sort++;
				continue;
#ifdef DEBUG

			case 'j':
				jul = 1;
				continue;
#endif /* DEBUG */

			case 'm':
				/*
				 * disregard machine names for each user
				 */
				mflag = 1;
				continue;

			case 'r':
				/*
				 * Reverse sorting order.
				 */
				reverse++;
				continue;

			default:
				usage();
			}
		}
		(void) enter(--cp);
		allflag = 0;
	}

	if (!qflag)
	{
		resetQCBFile();
		qlist = readconfig(0);
	}
	else 
		qlist = pac_config(qconfig);

	/* DETERMINE default printer name */
	if (printer == NULL) {
		qptr = default_queue(qlist);
		if ( qptr == NULL )
			syserr((int)EXITBAD,MSGSTR(BADENV,"Invalid PRINTER or LPDEST environment variable."));
		else printer = qptr->q_name;
	}
	else { 
		qptr = get_queue(printer,qlist);
		if (qptr == NULL)
			syserr((int)EXITBAD,MSGSTR(BADQNAM,"Invalid queue name:  %s"),printer);
	}

	acctfile = qptr->q_acctf;
	if ( *acctfile ) {
		strcpy(sumfile, acctfile);
		strcat(sumfile, "_sum");
	}
	else
		syserr((int)EXITBAD,MSGSTR(NOACCT,"No accounting file specified for queue: %s."),printer);

	if (jul)
		printf("acctfile is:%s\n",acctfile);
	
	if ((acct = fopen(acctfile, "r")) == NULL) 
		syserr((int)EXITFATAL,MSGSTR(MSGACCT,"Problem opening account file. Errno = %d"),errno);
	account(acct);
	fclose(acct);
	if ((acct = fopen(sumfile, "r")) == NULL)  {
		if (errno != ENOENT)
			syserr((int)EXITFATAL, MSGSTR(MSGSUMM,"Problem opening summary file. Errno = %d"),errno);
	} else {
		account(acct);
		fclose(acct);
	}
	if (summarize)
		rewrite();
	else
		dumpit();
	qexit((int)EXITOK);
}


/*
 * NAME: account
 *                                                                    
 * FUNCTION: 
 * 	Read the entire accounting file, accumulating statistics
 * 	for the users that we have in the hash table.  If allflag
 * 	is set, then just gather the facts on everyone.
 * 	Note that we must accomodate both the active and summary file
 * 	formats here.
 * 	Host names are ignored if the -m flag is present.
 *                                                                    
 * EXECUTION ENVIRONMENT: user level
 *                                                                   
 * RETURNS: void
 */  

static void
account(acct)
	register FILE *acct;
{
	struct acctrec	a_rec;
	register char *cp;
	register struct hent *hp;

	if (jul)
		printf ("Doing Accounting\n");
	while (fread((void *)&a_rec, (size_t)sizeof(struct acctrec),(size_t)1,acct) == 1) {
		if (jul)
		{
			printf("record read in:\n");
			printf("from: %s\nacctdate: %x	pages: %d	Jobs: %d\n",
				a_rec.from,
				a_rec.acctdate,
				a_rec.pages,
				a_rec.numjobs);
		}

		if (mflag && index(a_rec.from, '@'))
		{
		    cp = index(a_rec.from, '@');
		    *cp = '\0';
		}

		if (jul) printf ("Doing lookup\n");

		hp = lookup(a_rec.from);

		if (jul) printf ("return from lookup: %d\n",hp);

		if (hp == NIL) {
			if (!allflag)
				continue;
			hp = enter(a_rec.from);
		}

		hp->h_feetpages += a_rec.pages;
		hp->h_count 	+= a_rec.numjobs;
	}
}


/*
 * NAME: dumpit
 *                                                                    
 * FUNCTION: 
 * 		Sort the hashed entries by name or footage
 * 		and print it all out.
 *                                                                    
 * RETURNS: 	none
 */  

static void
dumpit()
{
	struct hent **base;
	register struct hent *hp, **ap;
	register int hno, c, runs;
	float feet;
        int qucmp(struct hent **left, struct hent **right);
	char s[MONSIZE];

	hp = hashtab[0];
	hno = 1;
	base = (struct hent **) calloc((size_t)sizeof(hp), (size_t)hcount);
	for (ap = base, c = hcount; c--; ap++) {
		while (hp == NIL)
			hp = hashtab[hno++];
		*ap = hp;
		hp = hp->h_link;
	}
        qsort((void *)base, (size_t)hcount, (size_t)sizeof(hp), (int(*)(const void *, const void *))qucmp);
	printf(MSGSTR(HEADER,"  Login               pages/feet   runs                price\n"));
	feet = 0.0;
	runs = 0;
	for (ap = base, c = hcount; c--; ap++) {
		hp = *ap;
		runs += hp->h_count;
		feet += hp->h_feetpages;
		strfmon(s,MONSIZE,"%20a",hp->h_feetpages * price);
		printf("%-24s %7.2f %4d   %s\n", hp->h_name,
		    hp->h_feetpages, hp->h_count, s);
	}
	if (allflag) {
		strfmon(s,MONSIZE,"%20a",feet * price);
		printf("\n");
		printf("%-24s %7.2f %4d   %s\n", MSGSTR(TOTAL,"total"), feet, 
		    runs, s);
	}
}


/*
 * NAME: rewrite
 *                                                                    
 * FUNCTION: 
 * 		Rewrite the summary file with the summary
 *		information we have accumulated.
 *                                                                    
 * RETURNS: NONE
 */  

static void
rewrite()
{
	register struct hent *hp;
	register int i;
	register FILE *acctf;
	long cur_date;
	struct acctrec	a_rec;

	if ((acctf = fopen(sumfile, "w")) == NULL)
		syserr((int)EXITFATAL,MSGSTR(MSGSUMM,"Problem opening summary file. Errno = %d"),errno);
	cur_date = time(0);
	for (i = 0; i < HSHSIZE; i++) {
		hp = hashtab[i];
		while (hp != NULL) {

			strcpy (a_rec.from,hp->h_name);
			a_rec.acctdate = cur_date;
			a_rec.pages = hp->h_feetpages;
			a_rec.numjobs = hp->h_count;

			if (fwrite((void *)&a_rec, (size_t)sizeof(struct acctrec),(size_t)1,acctf) != 1)
				syserr((int)EXITFATAL,MSGSTR(MSGSUMM,"Problem writing to summary file. Errno = %d"),errno);
			hp = hp->h_link;
		}
	}
	fflush(acctf);
	if (ferror(acctf))
		syserr((int)EXITFATAL,MSGSTR(MSGWACC,"Problem writing to account file. Errno = %d"),errno);
	fclose(acctf);
	if ((acctf = fopen(acctfile, "w")) == NULL)
		syserr((int)EXITFATAL,MSGSTR(MSGACCT,"Problem opening account file. Errno = %d"),errno);
	else
		fclose(acctf);
}

/*
 * Hashing routines.
 */

/*
 * NAME: enter
 *                                                                    
 * FUNCTION: 
 * 	Enter the name into the hash table 
 *                                                                    
 * RETURNS: return the pointer allocated.
 */  


struct hent *
enter(name)
	char name[];
{
	register struct hent *hp;
	register int h;

	if (jul) printf ("entering : %s\n",name);

	if ((hp = lookup(name)) != NIL)
		return(hp);

	if (jul) printf ("hashing : %s\n",name);

	h = hash(name);
	hcount++;
	hp = (struct hent *) calloc((size_t)sizeof(*hp), (size_t)1);
	hp->h_name = (char *) calloc((size_t)sizeof(char), (size_t)(strlen(name)+1));
	strcpy(hp->h_name, name);
	hp->h_feetpages = 0.0;
	hp->h_count = 0;
	hp->h_link = hashtab[h];
	hashtab[h] = hp;
	return(hp);
}

/*
 * NAME: lookup
 *                                                                    
 * FUNCTION: 
 * 	Lookup a name in the hash table
 *                                                                    
 * RETURNS: return the pointer to the found routine, null if not found.
 */  

struct hent *
lookup(name)
	char name[];
{
	register int h;
	register struct hent *hp;

	h = hash(name);
	for (hp = hashtab[h]; hp != NIL; hp = hp->h_link)
		if (strcmp(hp->h_name, name) == 0)
			return(hp);
	return(NIL);
}


/*
 * NAME: hash
 *                                                                    
 * FUNCTION and RETURNS: 
 * 	Hash the passed name and return the index in
 * 	the hash table to begin the search.
 */  

hash(name)
	char name[];
{
	register int h;
	register char *cp;

	for (cp = name, h = 0; *cp; h = (h << 2) + *cp++)
		;
	return((h & 0x7fffffff) % HSHSIZE);
}

/*
 * NAME: qncmp
 *                                                                    
 * FUNCTION: 
 * 		The qsort comparison routine.
 * 		The comparison is ascii collating order
 * 		or by feet of typesetter film, according to sort.
 * RETURNS: 
 */  

qucmp(struct hent **left, struct hent **right)
{
	register struct hent *h1, *h2;
	register int r;

	h1 = *left;
	h2 = *right;
	if (sort)
		r = h1->h_feetpages < h2->h_feetpages ? -1 :
					h1->h_feetpages > h2->h_feetpages;
	else
		r = strcmp(h1->h_name, h2->h_name);

	return(reverse ? -r : r);
}
/*
 * NAME: usage
 *                                                                    
 * FUNCTION: 
 * 		Prints the usage message and exits;
 * RETURNS: 
 */  
usage()
{
	sysuse( TRUE,
		MSGSTR(USAGE,"[-PPrinter] [-pPrice] [-qFileName] [-s] [-c] [-r] [-m] [User ...]"),
		(char *)0
	      );
}

/*
 * Function stripstr removes all non-digit except for the decimal point
 * from the users input string.
 */

char *stripstr(cp)
char *cp;
{
	char *p = cp;
	char newstr[MONSIZE];
	int i=0;

	while( *p != '\0')
	{
	 	if (isdigit(*p))
			newstr[i++] = *p;
		else if (*p == l_conv->mon_decimal_point[0])
			newstr[i++] = l_conv->decimal_point[0];
		p++;
	}
	return(newstr);
}
/*
 * NAME: pac_config
 *                                                                    
 * FUNCTION: 
 * 		Parse user supplied qconfig file	
 * RETURNS: 
 *		qlist
 */  
struct q *pac_config(PacConfig)
	char *PacConfig;
{
	int pid, got, fd;
	int status = 0;
	static char PacBconfig[] ="/tmp/qconfig.bin.XXXXXX"; 

	mktemp(PacBconfig);
	        switch( (pid = fork()) )
        {
        case -1:
                syserr((int)EXITFATAL,MSGSTR(MSGFORK,"Cannot fork for %s."),DIGEST);

        case 0:                          /* child */
                execl(DIGEST,"digest",PacConfig,PacBconfig,0);
                syserr((int)EXITFATAL,MSGSTR(MSGEXEC,
			"Cannot exec digester %s."),DIGEST);

        default:                        /* parent */
                got=waitpid(pid,&status,0);
	        if(status) {
                	syserr((int)EXITFATAL,MSGSTR(MSGDIGE,
                         "Error from digester %s, status = %d, rv = %d."),
                       		 DIGEST,status,got);
 		}
		resetQCBFile();
		openPacQCBFile(PacBconfig);
		qlist = rcfg(PacBconfig);
		unlink(PacBconfig);

 		if (qlist == NULL)
                	syserr((int)EXITFATAL,MSGSTR(MSGNADA,
			"Nothing read from %s.  Errno = %d."),PacBconfig,errno);

        	return(qlist);
	}
}

