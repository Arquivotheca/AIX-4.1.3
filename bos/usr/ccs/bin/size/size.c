static char sccsid[] = "@(#)68	1.15  src/bos/usr/ccs/bin/size/size.c, cmdaout, bos411, 9428A410j 11/18/93 17:48:58";

/*
 * COMPONENT_NAME: CMDAOUT (size command)
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27, 3
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
 */

#define RELEASE "size.c 1.15"

/* UNIX HEADERS */
#include	<stdio.h>
#include	<ar.h>

/* COMMON SGS HEADERS */
#include	"filehdr.h"
#include	"ldfcn.h"

/* SIZE HEADER */
#include	"defs.h"

/* INTERNATIONALIZATION HEADERS */
#include <locale.h>
#include <nl_types.h>
#include "size_msg.h"
nl_catd	 catd;
#define	 MSGSTR(Num, Str) catgets(catd, MS_SIZE, Num, Str)

/* EXTERNAL VARIABLES DEFINED */
#ifdef UNIX
int		numbase = DECIMAL;
#else
int		numbase = HEX;
#endif
LDFILE		*ldptr;
char		outbuf[BUFSIZ];
int		Fflag = 0;
int		Vflag = 0;
int		Exit_code = 0;

/* STATIC VARIABLES DEFINED */
static char	*dfltv[] = {"a.out", ""};

    /*
     *  main(argc, argv)
     *
     *  parses the command line
     *  opens, processes and closes each object file command line argument
     *
     *  defines:
     *      - int	numbase = HEX if the -x flag is in the command line
     *				= OCTAL if the -o flag is in the command line
     *				= DECIMAL if the -d flag is in the command line
     *      - LDFILE	*ldptr = ldopen(*filev, ldptr) for each obj file arg
     *
     *  calls:
     *      - process(filename) to print the size information in the object file
     *        filename
     *
     *  prints:
     *      - an error message if any unknown options appear on the command line
     *      - a usage message if no object file args appear on the command line
     *      - an error message if it can't open an object file
     *	      or if the object file has the wrong magic number
     *
     *  exits successfully always
     */


int
main(argc, argv)

int	argc;
char	**argv;

{
    /* UNIX FUNCTIONS CALLED */
/*    extern 		fprintf( ),
			sprintf( ),
			fflush( ),
    			exit( );
*/

    /* OBJECT FILE ACCESS ROUTINES CALLED */
    extern LDFILE	*ldopen(char *, LDFILE *);
    extern int		ldclose( ),
			ldaclose( ),
			ldahread( );

    /* SIZE FUNCTIONS CALLED */
    extern		process( );

    /* EXTERNAL VARIABLES USED */
    extern int		numbase;
    extern LDFILE	*ldptr;
    /* extern char		outbuf[ ]; */

    int		filec;
    char	**filev;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_SIZE, NL_CAT_LOCALE);

    --argc;
    filev = ++argv;
    filec = 0;

    {
	int		flagc;
	char		**flagv;

	for (flagc = argc, flagv = argv; flagc > 0; --flagc, ++flagv) {

	    if (**flagv == '-') {
		while(*++*flagv != '\0') {
		    switch (**flagv) {
			case 'o':
			    numbase = OCTAL;
			    break;

			case 'd':
			    numbase = DECIMAL;
			    break;

			case 'x':
				numbase = HEX;
				break;
			case 'f':
				Fflag++;
				break;
			case 'V':
			    Vflag++;
			    break;


			default:
			    fprintf(stderr, MSGSTR(OPTION_ERR_MSG, OPTION_ERR),
					**flagv);
			    fprintf(stderr, MSGSTR(USAGE_ERR_MSG, USAGE_ERR));
			    exit(-1);
			    break;
		    }
		}
	    } else {
		*filev++ = *flagv;
		++filec;
	    }
	}
    }

    if(Vflag)
	fprintf(stderr,SGS_STRING,SGSNAME,RELEASE);

    /*  buffer standard output, they say it's better to do so
     *  but remember to flush stdout before writing to stderr
     */
    setbuf(stdout, outbuf);

    {
	/* BLOCK LOCAL VARIABLES */
	ARCHDR	arhead;
	char	filename[MAXLEN];

	if (filec == 0) {
		filec++;
		filev = dfltv;
	} else
		filev = argv;
	
	for (; filec > 0; --filec, ++filev) {
	    ldptr = NULL;
	    do {
		if ((ldptr = ldopen(*filev, ldptr)) != NULL) {
		    if (ISMAGIC(HEADER(ldptr).f_magic)) {
		        if (ISARCHIVE(TYPE(ldptr))) {
			    if (ldahread(ldptr, &arhead) == SUCCESS) {
#ifdef PORTAR
				sprintf(filename, PORTAR_NAME,
#else
#ifdef AIXV3AR
				sprintf(filename, AIXV3AR_NAME,
#else
				sprintf(filename, OTHERAR_NAME,
#endif
#endif
				    *filev, arhead.ar_name);
#ifdef UNIX
				printf(STRING_COLON_1, filename);
#endif
				process(filename);
				fflush(stdout);
			    } else {
				fprintf(stderr, MSGSTR(AR_HDR_ERROR_MSG,
					AR_HDR_ERROR), *filev);
				Exit_code = -1;
			    }
			} else {
			    printf(STRING_COLON_1, *filev);
			    process(*filev);
			}
		    } else {
			fprintf(stderr,MSGSTR(BAD_MAGIC_MSG, BAD_MAGIC),
				*filev);
			ldaclose(ldptr);
			ldptr = NULL;
			Exit_code = -1;
		    }
		} else {
		    fprintf(stderr, MSGSTR(OPEN_ERR_MSG, OPEN_ERR), *filev);
		    Exit_code = -1;
		}
	    } while (ldclose(ldptr) == FAILURE);
	}
    }
    exit(Exit_code);
}
