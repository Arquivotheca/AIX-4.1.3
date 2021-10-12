static char sccsid[] = "@(#)91	1.17  src/bos/usr/bin/sum/sum.c, cmdfiles, bos412, 9446C 11/14/94 16:49:37";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: sum
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
/*
 * Sum bytes in file mod 2^16
 */

#include <stdio.h>
#include <locale.h>
#include <filehdr.h>
#include "sum_msg.h"

static nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_SUM,Num,Str) 

#define WDMSK 0177777L
#define BLOCK_SIZE	1024


static int	errflg = 0;
static int 	oflag = 0;
static int 	rflag = 0;
static int 	iflag = 0;
	
static FILE	*f;
static int	i;

/*
 * NAME: sum [-i][-r|-o] file
 *                                                                    
 * FUNCTION: Displays the checksum and block count of a file.
 *   FLAGS: 
 *    -r     Default algorithm computes the checksum (rigorous byte-by-byte).
 *    -o     Algorithm computes the checksum using word by word computation.
 *    -i     Skip file  header information
 */  
main(argc, argv)
int   argc;
char *argv[];
{
	int c;

	setlocale(LC_ALL,"");
	catd = catopen(MF_SUM, NL_CAT_LOCALE);

	while((c = getopt(argc, argv, "iro")) != EOF)
			switch (c) {
			case 'i':
				iflag++;
				break;
			case 'o':
				if (rflag) err_msg();
				oflag++;
				break;
			case 'r':
				if (oflag) err_msg();
				rflag++;
				break;
			default:
				err_msg();
			}
	i=optind;
	do {
		if (i < argc) {
			if ((f = fopen(argv[i], "r")) == NULL) {
				(void) fprintf(stderr, MSGSTR(OPENERR , 
					"sum: Can't open %s\n"), argv[i]);
				errflg += 10;
				continue;
			}
		}
		else
			f = stdin;
		if (iflag) 
			if (!skip_header(argc, argv)) continue; 
		if (oflag)
			sysV_sum(argc, argv);
		else
			bsd_sum(argc, argv);

		fclose(f);

	} while (++i < argc);

	exit(errflg);
}

static int skip_header(argc, argv)
int argc;
char *argv[];
{
	short magic;

	if(fread((void *) &magic, sizeof(magic), 1, f) ) {
		/* (RS6000) readonly text segments and TOC	*/
		if (magic == U802TOCMAGIC)  
			fseek(f, FILHSZ, 0);
		else
			rewind(f);
	}				
	else  
		if (ferror(f)) {
			errflg++;
			(void) fprintf(stderr, MSGSTR(READERR, 
				"sum: read error on %s\n"), argv[i]?argv[i]:"-");
			fclose(f);
			return(0);
		}
	return(1);
}		

static err_msg()
{
	(void) fprintf(stderr, MSGSTR(USAGE1,
	"usage: sum [-i][-r|-o] file ...\n"));
	exit(1);
}
/*
 * Execute a byte-by-byte computation (BSD4.3).
 */
static bsd_sum(argc, argv)
int  argc;
char *argv[];
{
	register unsigned sum;
	register int c;
	register long nbytes;

	sum = 0;
	nbytes = 0;

	while ((c = getc(f)) != EOF) {
		nbytes++;
		if (sum&01)
			sum = (sum>>1) + 0x8000;
		else
			sum >>= 1;
		sum += c;
		sum &= 0xFFFF;
	}

	if (ferror(f)) {
		errflg++;
		(void) fprintf(stderr, MSGSTR(READERR, 
			"sum: read error on %s\n"), argv[i]?argv[i]:"-");
	}

	printf("%05u %5ld %s\n", sum, (nbytes + BLOCK_SIZE - 1) / BLOCK_SIZE, argv[i]?argv[i]:"" );
}



struct part {
	short unsigned hi,lo;
};
static union hilo { /* this only works right in case short is 1/2 of long */
	struct part hl;
	long	lg;
} tempa, suma;


/*
 * Execute a word-by-word computation (AIX/SYS V).
 */
static sysV_sum(argc, argv)
int  argc;
char *argv[];
{
	register long nbytes;
	register int ca;
	unsigned lsavhi, lsavlo;

	suma.lg = 0;
	nbytes = 0;

	while ((ca = getc(f)) != EOF) {
		nbytes++;
		suma.lg += ca & WDMSK;
	}

	if (ferror(f)) {
		errflg++;
		fprintf(stderr, MSGSTR(READERR,"sum: read error on %s\n"),
				argc > 1 ? argv[i] : "-");
	}

	tempa.lg = (suma.hl.lo & WDMSK) + (suma.hl.hi & WDMSK);
	lsavhi = (unsigned) tempa.hl.hi;
	lsavlo = (unsigned) tempa.hl.lo;
	printf("%u %ld", (unsigned) (lsavhi + lsavlo), 
				(nbytes+BLOCK_SIZE-1)/BLOCK_SIZE);

	if ((argc-oflag) > 1)
		printf(" %s", argv[i] == (char *) 0 ? "" : argv[i]);

	fputc('\n',stdout);
}
