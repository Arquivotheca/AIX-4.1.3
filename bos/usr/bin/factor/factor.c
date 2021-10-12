static char sccsid[] = "@(#)46	1.11  src/bos/usr/bin/factor/factor.c, cmdmisc, bos41B, 9504A 1/4/95 14:11:47";
/*
 * COMPONENT_NAME: (CMDMISC) miscellaneous commands
 *
 * FUNCTIONS:
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
*       NOTES:
*	    To COMPILE:	cc -O factor.c -s -i -lm -o factor
*           works up to 14 digit numbers
*           runumbering time is proportional to sqrt(n)
*           accepts arguments either as input or on command line
*           0 input terminates processing
*/

double modf(), sqrt();
static double number, bigfact;

/* This is the largest number which can be factored. */
static double huge = 1.0e14;

/* This array is used to compute the prime numbers. */
static double sq[] = {
	10, 2, 4, 2, 4, 6, 2, 6,
	 4, 2, 4, 6, 6, 2, 6, 4,
	 2, 6, 4, 6, 8, 4, 2, 4,
	 2, 4, 8, 6, 4, 6, 2, 4,
	 6, 2, 6, 6, 4, 2, 4, 6,
	 2, 6, 4, 2, 4, 2,10, 2,
};

#include <stdlib.h>
#include <locale.h>
#include "factor_msg.h"
#define MSGSTR(Num, Str) catgets(catd, MS_FACTOR, Num, Str)
static nl_catd catd;

main(argc, argv)
int argc;
char *argv[];
{
	int test = 1;
	int ret;
	register j;
	double junk;
	double fr;
	double facter;
	char   *cp;

        (void) setlocale (LC_ALL, "");
        catd = catopen(MF_FACTOR, NL_CAT_LOCALE);

        /* There can be no more than two arguments. */
	if(argc > 2){
		fprintf(stderr,MSGSTR(USAGE, "Usage: factor [number]\n"));
		exit(1);
	}
	while(test == 1){
		/* If a number is given as an argument, then find its          
		 * factors, otherwise read the numbers from stdin.          */
		if(argc == 2){
			/* First arg read as flt-pt and placed in number.   */
			cp = argv[1];
			number = strtod(cp, &cp);
			fr = modf(number, &junk);
			if (fr>0 || *cp!='\0' || number<=0.0 || number>huge) {
				fprintf(stderr,MSGSTR(NOTINT,"factor: specify an integer ranging from 1 to 1.0e14.\n"));
				fprintf(stderr,MSGSTR(USAGE, "Usage: factor [number]\n"));
				exit(1);
			}
			test = 0;
			printf("%.0f\n", number);
		} else {
			ret = scanf("%lf", &number);
			if ((ret<1) || (number == 0.0)) {
				exit(0);
			}
			fr = modf(number, &junk);
			/* check if the number is an integer or inside the
			 * the range from 1 to 1.0e14, if not, print out
			 * error message.
			 */
			if((fr != 0.0) || (number < 0.0) || (number > huge)) {
				fprintf(stderr,MSGSTR(NOTINT,
				"factor: specify an integer ranging from 1 to 1.0e14.\n"));
				continue;
			}
		}
		/* Use the number 2,3,5 and 7 as factors first.              */
		try(2.0);
		try(3.0);
		try(5.0);
		try(7.0);

		/* The factor cannot be greater than this number(bigfact).   */
	        bigfact = 1 + sqrt(number);

		/* facter holds the numbers which are being tried as factors */
		facter = 1.0;

                /* Process the loop until we're trying factors which are 
		 * impossibly large.                                         */
		while(facter <= bigfact){
			/* This loop computes all the prime numbers and uses
			 * them as factors into the number.                  */
			for(j=0; j<48; j++){
				facter += sq[j];
				try(facter);
			}
		}
                /* If the number as been factored as much as possible, then
		 * print the final factor.                                   */
		if(number > 1.0){
			printf("     %.0f\n", number);
		}
		printf("\n");
	}
	exit(0);
}

/* 
 * NAME: try 
 *
 * FUNCTION: This function is passed a number and it determines 
 *	     if that number is a factor.  If it is, then it's 
 *	     divided out, printed and tried again.  This function
 *	     stops after it's determined that the number is no
 *           longer a factor. 
 *
 * RETURN VALUE: none
 *
 */
static try(arg)
double arg;
{
	double temp;
	int    divisor = 1;

        while (divisor == 1) {
		modf(number/arg, &temp);
		if(number == temp*arg) {
			printf("     %.0f\n", arg);
			number = number/arg;
			bigfact = 1 + sqrt(number);
		}
		else divisor = 0;
	}
	return;
}
