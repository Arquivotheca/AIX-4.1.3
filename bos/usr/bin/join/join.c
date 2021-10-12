static char sccsid[] = "@(#)27	1.19  src/bos/usr/bin/join/join.c, cmdfiles, bos412, 9446C 11/14/94 16:46:16";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: join
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<locale.h>
#include        <malloc.h>   /* D43548 */
#include "join_msg.h"

/* Function prototypes */
static void error(char*, char*);
static int  input(int);
static void output(register int, register int);
static void Usage(void);

static nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_JOIN, Num, Str)

#define F1 0
#define F2 1
#define	NFLD	20	/* initial fields per line  D43548 */
/* D43548 */
#define PPI(a,b) (*(ppi[a]+b))   
#define comp() strcoll(PPI(F1,j1),PPI(F2,j2))
/* D43548 */

static FILE	*f[2];		/* file descriptors */
static char buf[2][LINE_MAX+1];	/* input lines */
static char	**ppi[2];	/* pointers to fields in lines  D43548 */
static int	j1	= 1;	/* join of this field of file 1 */
static int	j2	= 1;	/* join of this field of file 2 */
static int	multibyte;	/* mutlibyte path flag */
static int	*olist; 	/* output these fields  D43548 */
static int	*olistf;	/* from these files  D43548 */
static unsigned nflds = NFLD;  /* #fields malloc'ed D43548 */
static int	olist_entries;  /* number of entries in olist */
static int	sep = 0;        /* -t field separator */
static char	*null	= "";	/* -e default string */
static int	aflg;		/* common and non-matching flag */
static int	vflg;		/* non-matching only flag */

/*
 * NAME: join [-afilenumber] [-v filenumber] [-[j]filenumner field] 
 * 	[-o list [-e string]] [-tseparatorchar] file1 file2
 * FUNCTION: Joins data fields of two files.
 */

main(int argc, char *argv[])
{
	register int i;
	register int n1, n2;
	long top2, bot2;
	int c;
	char *optr;
	unsigned short oflag = 0, eflag = 0;

	(void)setlocale(LC_ALL, "");
	catd = catopen(MF_JOIN, NL_CAT_LOCALE);
	multibyte = MB_CUR_MAX != 1;

        /* D43548 - Dynamically allocate ppi, olist, olistf arrays */
        if ( ((ppi[0] = (char **) malloc( nflds * sizeof(char *) )) == NULL) ||
             ((ppi[1] = (char **) malloc( nflds * sizeof(char *) )) == NULL) ||
             ((olist = (int *) malloc( 2 * nflds * sizeof(int) ))   == NULL) ||
             ((olistf = (int *) malloc( 2 * nflds * sizeof(int) ))  == NULL) )
                 error(MSGSTR(MALLOC,"join: Memory allocation failure.\n"),NULL);
	/* end D43548 */

	/* Process  join  options including POSIX "obsolescent" forms of
	 * -j and -o that do not follow POSIX command syntax guidelines.
	 */
	vflg = 0;
	olist_entries = 0;
	while ((c=getopt(argc,argv,"a:v:e:t:j:1:2:o:")) != EOF) {
	  switch(c){
	  case 'a':
	  case 'v':
		if ((strlen(optarg)>1)
		    || ((optarg[0] != '1') && (optarg[0] != '2')))
			Usage();
		if (optarg[0] == '1') 
		  if (c=='a') aflg |= 1;
		  else vflg |= 1;
		else 
		  if (c=='a') aflg |= 2;
		  else vflg |= 2;
		break;
	  case 'e':
		eflag++;
		null = optarg;
		break;
	  case 't':
		if (multibyte) {
			wchar_t wc;
			if (mbtowc(&wc, optarg, MB_CUR_MAX) < 1)
				error(MSGSTR(BADSEP, "join: Invalid -t separator character\n"), NULL);
			sep = wc;
		} else
			sep = optarg[0];
		break;
	  case 'j':
		/* IF optarg is "1\0" or "2\0" immediately preceded by "-j"
		 * THEN process "-jn m"  where optarg is n
		 * ELSE process "-j m"  where optarg is m.
		 */
		if ( ((optr=optarg-2)[0]=='-') && (optr[1]=='j')
		     && ((optarg[0]=='1') || (optarg[0]=='2'))
		     && (optarg[1]=='\0')) 
		{ /* Process "-jn m" with optarg=n  as if it was "-n m"
		   * with optarg=m .
		   */
			/* Use character  n  as option-char c in case c=='1'
			 * and  case c=='2' following.
			 */
		  c = optarg[0];
			/* If NEXT argument after n begins with '-' it
			 * is syntax error: omitted  m  or negative  m.
			 * It would look like an option to getopt() .
			 */
		  if (argv[optind][0] == '-') Usage();
			/* Get  m  as optarg, and move getopt() pointer
			 * up so getopt() resumes parsing at the argument
			 * following  -jn m .
			 */
		  optarg=argv[optind++];
		}
	  case '1': /* Equivalent to pre-POSIX  -j1 m */
	  case '2': /* Equivalent to pre-POSIX  -j2 m */
		optr = optarg;
		i = strtol(optarg,&optr,10);
		/* If strtol didn't find a nonempty string ending
		 * with '\0', the  m  argument was not valid.
		 */
		if ((i<=0) && ((optr==optarg) || (optr[0]!='\0')))  
			error(MSGSTR(BADFLDNUM,"join: Invalid field number %s\n"), optarg);
		if (c != '1') j2 = i; /* if c=2 or c=j */
		if (c != '2') j1 = i; /* if c=1 or c=j */
		break;
	  case 'o':
		/* Process zero or more -o lists, each of which is a string
		 * of one or more specifiers of the form "1.m" or "2.m",
		 * separated by commas, blanks or tabs within a string, where
		 * each m is a nonnegative decimal integer.
		 */
		/* While (next argument begins with "1.m" or "2.m"
		 *        and the field table has not overflowed)
		 * {process next argument as a  -o list parameter}.
		 */
		while( ((strlen(optarg)>2) && (optarg[1]=='.')
			&& ((optarg[0]=='1') || (optarg[0]=='2'))  )
			|| optarg[0]=='0' ) {
			/* begin D43548 */
                	if ( olist_entries > nflds ) {
                  		nflds <<= 1;
                     		if(  ((ppi[0] = (char **) realloc( ppi[0], nflds * sizeof(char *) )) == NULL) ||
                          		((ppi[1] = (char **) realloc( ppi[1], nflds * sizeof(char *) )) == NULL) ||
                          		((olist  = (int *) realloc( olist, 2 * nflds * sizeof(int) ))   == NULL) ||
                          		((olistf = (int *) realloc( olistf, 2 * nflds * sizeof(int) ))  == NULL) )
                              			error(MSGSTR(MALLOC,"join: Memory allocation failure.\n"),NULL);
                     	}
			/* end D43548 */

		  	if (optarg[0]=='0') {
				olistf[olist_entries] = 3;
				olist[olist_entries++] = 0;
				optarg++;
				optr = "";
			} else {
		  		olistf[olist_entries] = ((optarg[0]=='1')? F1 : F2);
		  		olist[olist_entries++] = strtol((optr=&optarg[2]),&optarg,10) -1;
			}
			/* If m was not a nonempty string terminated by 
			 * either ',', ' ', '\t' or '\0', syntax error in -o list.
			 */
		  	if( (optr==optarg) 
			      || ((optarg[0]!='\0') && (optarg[0]!=',') && (optarg[0]!=' ') 
			      && (optarg[0]!='\t')) ) Usage();

			/* If m did not end at a NUL continue next argument in
			 * this -o list; otherwise consider the next
			 * argument as (possible) next -o list.
			 */
		  	if((optarg++)[0]=='\0')
		    		optarg = argv[optind++];
		}

		/* Leave getopt() pointer on first argument not used
		 * as a  -o list .
		 */
		optind--;
		oflag++;
		break;
	  default:
		Usage();
		break;
	  } /* switch c */
	} /* while c is next valid option character */

	/* Must have at least one list entry with -o option. */
	if (oflag && olist_entries == 0)
		Usage();

	/* The -e flag is only valid when using the -o flag */
	if (eflag != 0 && oflag == 0) {
			(void)fprintf(stderr, MSGSTR(BADDSHE, 
			"join: The -e flag is only valid with the -o flag.\n"));
			exit(1);
	}
	j1--;
	j2--;
	/* Process file parameters */
	/* Either but not both can be "-" meaning stdin */
	if (optind+2 != argc) 
	  Usage();
	if (strcmp(argv[optind],"-") == 0)
	  f[F1] = stdin;
	else if ((f[F1] = fopen(argv[optind],"r")) == NULL)
	  error(MSGSTR(CANTOPEN,"join: Cannot open file %s\n"), argv[optind]);
	if (strcmp(argv[++optind],"-") == 0) {
	  if (f[F1] == stdin)
	    error(MSGSTR(BOTHSTDIN, "join: Both files cannot be \"-\"\n"), NULL);
	  /* switch file1 and file2 - algorithm fails if file2 is stdin */
	  else {
	    f[F2] = f[F1];
	    f[F1] = stdin;
	    i = j1;
	    j1 = j2;
	    j2 = i;
	    if (aflg == 1 || aflg == 2)
	      aflg ^= 3;
	    if (vflg == 1 || vflg == 2)
	      vflg ^= 3;
	    for (i=0; i<olist_entries; i++)
		olistf[i] ^= 1;
	  }
	} else if ((f[F2] = fopen(argv[optind],"r")) == NULL)
	    error(MSGSTR(CANTOPEN,"join: Cannot open file %s\n"), argv[optind]);

#define get1() n1=input(F1)
#define get2() n2=input(F2)

	get1();
	bot2 = ftell(f[F2]);
	get2();
	while(n1>0 && n2>0 || ((aflg!=0 || vflg!=0) && n1+n2>0)) {
		if(n1==0 || n2>0 && comp()>0) {
			if((aflg&2)|(vflg&2)) output(0, n2);
			bot2 = ftell(f[F2]);
			get2();
		} else if(n2==0 || n1>0 && comp()<0) {
			if((aflg&1)|(vflg&1)) output(n1, 0);
			get1();
		} else /*(n1>0 && n2>0 && comp()==0)*/ {
			if(vflg==0) {
				while(n2>0 && comp()==0) {
					output(n1, n2);
			       		top2 = ftell(f[F2]);
					get2();
				}
				fseek(f[F2], bot2, 0);
				get2();
				get1();
				for(;;) {
					if(n1>0 && n2>0 && comp()==0) {
						output(n1, n2);
						get2();
					} else if(n2==0 || n1>0 && comp()<0) {
						fseek(f[F2], bot2, 0);
						get2();
						get1();
					} else /*(n1==0 || n2>0 && comp()>0)*/{
						fseek(f[F2], top2, 0);
						bot2 = top2;
						get2();
						break;
					}
				}
			} else {
				optr = (char *)strdup(ppi[F1][j1]);
				do
					get1();
				while (n1 > 0 && strcoll(optr,ppi[F1][j1]) == 0);
				do {
					bot2 = ftell(f[F2]);
					get2();
				} while (n2 > 0 && strcoll(optr,ppi[F2][j1]) == 0);
				free (optr);
			}
		}
	}
	exit(0);
}

/*
 * get input line and split into fields
 * returns zero on EOF, otherwise returns number of fields
 */
int
input(int n) {
	register int i;
	register int c;
	register char *bp;
	register char **pp;
	int wclen;
	wchar_t wc;

	bp = buf[n];
	pp = ppi[n];
	if (fgets(bp, LINE_MAX+1, f[n]) == NULL)
		return(0);
	i = 0;
	if (sep == 0) {
		do {
			if (++i > nflds ) {  /* D43548 - realloc */
                          nflds <<= 1;
                          if (  ((ppi[0] = (char **) realloc( ppi[0], nflds * sizeof(char *) )) == NULL) ||
                                ((ppi[1] = (char **) realloc( ppi[1], nflds * sizeof(char *) )) == NULL)  )
                                       error(MSGSTR(MALLOC,"join: Memory allocation failure.\n"),NULL);
                          pp = ppi[n] + (nflds>>1); /* restore our place */
                          }
			/* end D43548 */
			

			while (*bp == ' ' || *bp == '\t')
				bp++;
			*pp++ = bp;	/* record beginning of field */
			while ((c = *bp) != ' ' && c != '\t' && c != '\n' && c != '\0')
				bp++;
			*bp++ = '\0';	/* mark end by overwriting separator */
		} while (c != '\n' && c != '\0');
	} else {
		do {
			if (++i > nflds) {  /* D43548 - realloc */
                          nflds <<= 1;
                          if (  ((ppi[0] = (char **) realloc( ppi[0], nflds * sizeof(char *) )) == NULL) ||
                                ((ppi[1] = (char **) realloc( ppi[1], nflds * sizeof(char *) )) == NULL)  )
                                       error(MSGSTR(MALLOC,"join: Memory allocation failure.\n"),NULL);
                          pp = ppi[n] + (nflds>>1); /* restore our place */
                          }
			/* end D43548 */

			*pp++ = bp;	/* record beginning of field */
			if (multibyte && sep >= 0x40) {
				while (1) {
					wclen = mbtowc(&wc, bp, MB_CUR_MAX);
					if (wclen > 0) {
						if (wc == sep || wc == '\n')
							break;
						else
							bp += wclen;
					} else if (wclen == 0) {
						wclen = 1;
						wc = '\0';
						break;
					} else
						bp++;
				}
				*bp = '\0';
				bp += wclen;
				c = wc;
			} else {
				while ((c = *bp) != sep && c != '\n' && c != '\0')
					bp++;
				*bp++ = '\0';	/* mark end by overwriting separator */
			}
		} while (c != '\n' && c != '\0');
	}
	c = i;
	while (++c <= nflds)  /* D43548 */
		*pp++ = null;
	return(i);
}

static void
output(register int on1, register int on2)	/* print items from olist */
{
	register int i;
	register char *temp;


/* D43548 all the ppi[a,b] 's now PPI(a,b) 's */

	if (olist_entries <= 0) {	/* default case */
		if (!*(temp = on1 ? PPI(F1,j1) : PPI(F2,j2)) )
			temp = null;

		fputs(temp,stdout);

		for (i = 0; i < on1; i++)
			if (i != j1) {
				temp = *PPI(F1,i) ? PPI(F1,i) : null;
				if (sep)
					/* avoid SCCS problem with 2 printf() */
					if (multibyte && sep > 0xff) {
						(void)printf("%C", sep);
						fputs(temp,stdout);
					} else
						(void)printf("%c%s", sep, temp);
				else
					(void)printf(" %s", temp);
			}
		for (i = 0; i < on2; i++)
			if (i != j2) {
				temp = *PPI(F2,i) ? PPI(F2,i) : null;
				if (sep)
					/* avoid SCCS problem with 2 printf() */
					if (multibyte && sep > 0xff) {
						(void)printf("%C", sep);
						fputs(temp,stdout);
					} else
						(void)printf("%c%s", sep, temp);
				else
					(void)printf(" %s", temp);
			}

		fputc('\n',stdout);
	} else {
		for (i = 0; i < olist_entries; i++) {
			if (olistf[i] != 3) {
				temp = PPI(olistf[i],olist[i]);
				if(olistf[i]==F1 && on1<=olist[i] ||
			   	   olistf[i]==F2 && on2<=olist[i] || 
				   *temp==0)
					temp = null;
			} else {
				if (!*(temp = on1 ? PPI(F1,j1) : PPI(F2,j2)) )
					temp = null;
			}

			fputs(temp,stdout);

			if (i == olist_entries - 1)
				fputc('\n',stdout);
			else if (sep)
				if (multibyte && sep > 0xff)
					(void)printf("%C", sep);
				else
					fputc(sep,stdout);
			else
				fputc(' ',stdout);
		}
	}
}


static void
error(char *s1, char *s2)
{
	(void)fprintf(stderr, s1, s2);
	exit(1);
}

static void
Usage(void)
{
	(void)fprintf(stderr,MSGSTR(USAGE,
"Usage: join [-a Num] [-v Num] [-[j] Num Field] [-t Chr] [-o Fieldlist [-e str]] File1 File2\n"));
	exit(1);
}
