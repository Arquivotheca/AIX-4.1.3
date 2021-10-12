static char sccsid[] = "@(#)07	1.7  src/bldenv/cmp/cmp.c, ade_build, bos412, GOLDA411a 2/7/94 11:37:32";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: cmp
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include	<stdio.h>
#include	<ctype.h>
#include	<filehdr.h>
#include	<syms.h>
#ifdef MSG
#include "cmp_msg.h"
#define	MSGSTR(Num, Str) NLgetamsg(MF_CMP,MS_CMP,Num,Str)
#else
#define NLprintf  	 printf
#define	MSGSTR(Num, Str) Str
#endif

FILE	*file1,*file2;              /* the files to be compared */

char	*arg;                       /* argument holder */

int	eflg = 0;                   /* difference flag */
int	lflg = 1;                   /* long output flag */
int	sflg = 0;                   /* short output flag */

long	line = 1;                   /* line number */
long	chr = 0;                    /* char number */

FILHDR  header;			    /* file header */
char	*hdrptr;

/*
 * NAME: cmp [-l] [-s] file1 file2 
 *                                                                    
 * FUNCTION: Compares two files
 *                                                                    
 * NOTES:   Compares two files sending the differences to standard out.
 *          If a '-' is given for file1 then cmp reads from standard in.
 *          The default is that cmp displays nothing if the files are the
 *          same.  If the files differ cmp displays the byte and line number
 *          at which the first difference occurs.
 *          -l    Displays, for each difference, the byte number in
 *                decimal and the differing bytes in octal.
 *          -s    Returns only an exit value.
 *
 * RETURN VALUE DESCRIPTION: 
 *          0  indicates identical files.
 *          1  indicates different files.
 *          2  indicates inaccessible files or a missing argument.
 */  
main(argc, argv)
int argc;
char **argv;
{
	int c1, c2;           /* current charcters from file1 and file2 */
	int sccspattern[4];
	int sccsflg = 0;
	char *string_table = NULL;
	int reading_1st_string = 0;
	int reading_2nd_string = 0;
	int save1, save2;

	sccspattern[0] = '@';
	sccspattern[1] = '(';
	sccspattern[2] = '#';
	sccspattern[3] = ')';

	if(argc < 3)                            /* not enough arguments */
		narg();
	arg = argv[1];
	if(arg[0] == '-' && arg[1] == 's') {  /* return only exit codes */
		sflg++;
		lflg--;
		argv++;
		argc--;
	}
	arg = argv[1];
	if(arg[0] == '-' && arg[1] == 'l') {             /* long output */
		lflg++;
		argv++;
		argc--;
	}
	if(argc != 3)                       /* check the number of files */
		narg();
	arg = argv[1];
	if( arg[0]=='-' && arg[1]==0 )               /* file1 from stdin */
		file1 = stdin;
	else if((file1 = fopen(arg, "r")) == NULL)
		barg();                                      /* bad file */
	arg = argv[2];
	if((file2 = fopen(arg, "r")) == NULL)
		barg();                                      /* bad file */
	hdrptr = (char *)&header;
	while(1) {                                  /* compare the files */
		c1 = getc(file1);
		c2 = getc(file2);
		if (chr < FILHSZ)
			*hdrptr = c1;
		else if (chr == FILHSZ && header.f_magic == U802TOCMAGIC)
			string_table = header.f_symptr + header.f_nsyms*SYMESZ + 4;
		else if (string_table) {
			if (chr == string_table)
				reading_1st_string = 1;
			else if (reading_1st_string) {
				if (c1 == '\0') {
					reading_1st_string = 0;
					if (save1 == '.' && save2 == 'c')
						reading_2nd_string = 2;
				}
				else {
					save1 = save2;
					save2 = c1;
				}
			}
		}
		chr++;
		hdrptr++;
		if (reading_2nd_string) {
			if (c1 == '\0' || c1 == EOF) reading_2nd_string--;
		}
		else if(c1 == c2) {
			if (sccsflg == 4) {
				if (endofsccs(c1))
					sccsflg = 0;
			}
			else
				sccsflg = (c1 == sccspattern[sccsflg] ?
					   sccsflg+1 : 0);
			
			if (c1 == '\n')
				line++;
			if(c1 == EOF) {
				if(eflg)
					exit(1);
				exit(0);
			}
		}
		else if(sccsflg == 4) {
			if (endofsccs(c1)) {
				sccsflg = 0;
				while ( !endofsccs(c2) &&
					c1 != EOF &&
					c2 != EOF) {
/* If this is an xcoff file, we want to keep the two files in sync, due to
   padding.  string_table will be set if this is true.
*/
						if (string_table)
							c1 = getc(file1);
						c2 = getc(file2);
						chr++;
				}
			}
			else if (endofsccs(c2)) {
				sccsflg = 0;
				while ( !endofsccs(c1) &&
					c1 != EOF &&
					c2 != EOF) {
						c1 = getc(file1);
						if (string_table)
							c2 = getc(file2);
						chr++;
				}
			}
					
		}

		else if(hdrptr > (char *)&header.f_timdat &&
		        hdrptr <= (char *)&header.f_timdat+sizeof(header.f_timdat) &&
			header.f_magic == U802TOCMAGIC) ;
		else {
			if(sflg > 0)
				exit(1);
			if(c1 == EOF) {
				arg = argv[1];
				earg();
			}
			if(c2 == EOF)
				earg();
			if(lflg == 1) {
				NLprintf(MSGSTR(DIFF,			/*MSG*/
				 "%s %s differ: char %d, line %d\n"),
						argv[1], arg, chr, line);/*MSG*/
			exit(1);
			}
			eflg = 1;
			printf("%6ld %3o %3o\n", chr, c1, c2);
		}
	} /* end of while */
}

/*
 * NAME: narg
 *                                                                    
 * FUNCTION:  print out usage statement and exit program.
 */  
narg()
{
	fprintf(stderr,MSGSTR(USG,"usage: cmp [-l] [-s] file1 file2\n"));/*MSG*/
	exit(2);
}

/*
 * NAME: barg
 *                                                                    
 * FUNCTION: print out error message and exit program.
 */  
barg()
{
	if (sflg == 0)
		fprintf(stderr, MSGSTR(NOTOPEN,				/*MSG*/
					"cmp: cannot open %s\n"), arg);	/*MSG*/
	exit(2);
}

/*
 * NAME: earg
 *                                                                    
 * FUNCTION: file arg was shorter than the other file, print message
 *           and exit.
 */  
earg()
{
	fprintf(stderr, MSGSTR(EOFMSG,"cmp: EOF on %s\n"), arg);	/*MSG*/
	exit(1);
}
/*
 * NAME: endofsccs
 *
 * FUNCTION: returns true if the argument is the last character of an sccsid
 */
endofsccs(c)
int c;
{
	if (c == '\0' ||
	    c == '\n' ||
	    c == '>' ||
	    c == '"' ||
	    c == '\\' )
		return(1);
	else
		return(0);
}
