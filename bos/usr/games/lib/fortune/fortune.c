static char sccsid[] = "@(#)06	1.7  src/bos/usr/games/lib/fortune/fortune.c, cmdgames, bos411, 9428A410j 4/26/94 14:20:36";
/*
 * COMPONENT_NAME: (CMDGAMES) unix games
 *
 * FUNCTIONS: fortune
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/* $Header: fortune.c,v 1.10 85/11/01 15:19:49 arnold Exp $ */

# include	<sys/types.h>
# include	<stdio.h>
# include	<sys/file.h>
# include	"strfile.h"

# define	TRUE	1
# define	FALSE	0
# define	bool	short

# define	MINW	6		/* minimum wait if desired */
# define	CPERS	20		/* # of chars for each sec */
# define	SLEN	160		/* # of chars in short fortune */

# define	FORTFILE	"/usr/games/lib/fortune/fortunes.dat"

bool	Wflag		= FALSE,	/* wait desired after fortune */
	Sflag		= FALSE,	/* short fortune desired */
	Lflag		= FALSE,	/* long fortune desired */
/*	Oflag		= FALSE,*/	
	Aflag		= FALSE,	/* any fortune allowed */
	ROflag		= FALSE;	/* fortune file opened read-only */

char	*Fortfile	= FORTFILE,	/* fortune database */
	*Usage[]	= {
       "usage:  fortune [ - ] [ -wsla ] [ file ]",
       "	- - give this summary of usage",
       "	w - have program wait after printing message in order",
       "	    to give time to read",
       "	s - short fortune only",
       "	l - long fortune only",
/*	   "    o - offensive fortunes only",    */
       "	a - any fortune",
 /*      "		Mail suggested fortunes to \"fortune\"",*/
	NULL
	};

long	Seekpts[2];			/* seek pointers to fortunes */

FILE	*Inf;				/* input file */

STRFILE	Tbl;				/* input table */

time_t	time();

main(ac, av)
int	ac;
char	*av[];
{
	register char	c;
	register int	nchar = 0;

	getargs(ac, av);
	/* open the fortunes file read only */
	if ((Inf = fopen(Fortfile, "r+")) == NULL) {
		if ((Inf = fopen (Fortfile, "r")) == NULL) {
			perror(Fortfile);
			exit(-1);
		}
		Aflag = TRUE;
		ROflag = TRUE;
	}
	fread((void *)&Tbl, (size_t)sizeof(Tbl), (size_t)1, Inf); /* NOSTRICT */
	if (Tbl.str_longlen <= SLEN && Lflag) {
		puts("Sorry, no long strings in this file");
		exit(0);
	}
	if (Tbl.str_shortlen > SLEN && Sflag) {
		puts("Sorry, no short strings in this file");
		exit(0);
	}

	/*
	 * initialize the pointer to the first -o fortune if need be.
	 */
	if (Tbl.str_delims[2] == 0)
		Tbl.str_delims[2] = Tbl.str_delims[0];

	do {
		getfort();
	} while ((Sflag && !is_short()) || (Lflag && !is_long()));

	fseek(Inf, Seekpts[0], 0);
	while (c = getc(Inf)) {
		nchar++;
		putchar(c);
	}
	fflush(stdout);

	if (! ROflag) {
		fseek(Inf, 0L, 0);
#ifdef	LOCK_EX
		/*
		 * if we can, we exclusive lock, but since it isn't very
		 * important, we just punt if we don't have easy locking
		 * available.
		 */
		flock(fileno(Inf), LOCK_EX);
#endif	LOCK_EX
		fwrite((void *)&Tbl, (size_t)1, (size_t)sizeof(Tbl), Inf);
#ifdef	LOCK_EX
		flock(fileno(Inf), LOCK_UN);
#endif	LOCK_EX
	}
	if (Wflag)
		sleep(max((int) nchar / CPERS, MINW));
	exit(0);
}

/*
 * is_short:
 *	Return TRUE if fortune is "short".
 */
is_short()
{
	register int	nchar;

	if (!(Tbl.str_flags & (STR_RANDOM | STR_ORDERED)))
		return (Seekpts[1] - Seekpts[0] <= SLEN);
	fseek(Inf, Seekpts[0], 0);
	nchar = 0;
	while (getc(Inf))
		nchar++;
	return (nchar <= SLEN);
}

/*
 * is_long:
 *	Return TRUE if fortune is "long".
 */
is_long()
{
	register int	nchar;

	if (!(Tbl.str_flags & (STR_RANDOM | STR_ORDERED)))
		return (Seekpts[1] - Seekpts[0] > SLEN);
	fseek(Inf, Seekpts[0], 0);
	nchar = 0;
	while (getc(Inf))
		nchar++;
	return (nchar > SLEN);
}

/*
 *	This routine evaluates the arguments on the command line
 */
getargs(ac, av)
register int	ac;
register char	*av[];
{
	register int	i;
	register char	*sp;
	register int	j;
	register short	bad = 0;

	for (i = 1; i < ac; i++)  {
		if (av[i][0] != '-') {
			setuid(getuid());
			setgid(getgid());
			Fortfile = av[i];
		}
		else if (av[i][1] == '\0') {
			j = 0;
			while (Usage[j] != NULL)
				puts(Usage[j++]);
			exit(0);
			/* NOTREACHED */
		}
		else
			for (sp = &av[i][1]; *sp != '\0'; sp++)
				switch (*sp) {
				  case 'w':	/* give time to read */
					Wflag++;
					break;
				  case 's':	/* short ones only */
					Sflag++;
					break;
				  case 'l':	/* long ones only */
					Lflag++;
					break;
		/*		  case 'o':	
					Oflag++;
					break;   */
				  case 'a':	/* any fortune */
					Aflag++;
					/*
					 * initialize the random number
					 * generator; throw away the first
					 * few numbers to avoid any non-
					 * randomness in startup
					 */
					srand(time(NULL) + getpid());
					for (j = 0; j < 20; j++)
						(void) (rand() % 100);
					break;
				  default:
					printf("unknown flag: '%c'\n", *sp);
					bad++;
					break;
				}
	}
	if (bad) {
		printf("use \"%s -\" to get usage\n", av[0]);
		exit(-1);
	}
}

/*
 * getfort:
 *	Get the fortune data file's seek pointer for the next fortune.
 */
getfort()
{
	register int	fortune;

	/*
	 * Make sure all values are in range.
	 */

	if (Tbl.str_delims[1] >= Tbl.str_delims[0])
		Tbl.str_delims[1] = 0;
	if (Tbl.str_delims[2] >= Tbl.str_numstr)
		Tbl.str_delims[2] = Tbl.str_delims[0];

	if (Aflag) {
		if (rand() % (Tbl.str_numstr) < Tbl.str_delims[0])
			fortune = Tbl.str_delims[1]++;
		else
			fortune = Tbl.str_delims[2]++;
	}
/*	else if (Oflag)
		fortune = Tbl.str_delims[2]++;  */
	else
		fortune = Tbl.str_delims[1]++;  

	fseek((FILE *)Inf, (long)(sizeof(Seekpts[0])*fortune+sizeof(Tbl)), 0);
	fread((void *)Seekpts, (size_t)sizeof(Seekpts[0]), (size_t)2, Inf);
}

max(i, j)
register int	i, j;
{
	return (i >= j ? i : j);
}
