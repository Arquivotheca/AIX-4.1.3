static char sccsid[] = "@(#)35  1.19  src/bos/usr/bin/cal/cal.c, cmdmisc, bos41J, 9513A_all 3/28/95 18:12:32";
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

#define MOWIDTH		30		/* dimensions of printed calendars */ 
#define MODEPTH		6
#define TTLWIDTH	26
#define LNWIDTH		MOWIDTH * 2
#define STRSIZE		LNWIDTH * MODEPTH
#define COLUMN_WIDTH	4
#define MAXYR		9999
void cal(), pstr();

static char	*dayw[] = {
	" S  ",
	" M  ",
	" Tu ",
	" W  ",
	" Th ",
	" F  ",
	" S  "
};

static char    *lmon[12]= {
	"January", "February", "March", "April",
	"May", "June", "July", "August",
	"September", "October", "November", "December"
};

static char	mon[13] = {               /* month ending days */
	0,
	31, 29, 31, 30,
	31, 30, 31, 31,
	30, 31, 30, 31,
};

#include <locale.h> 
#include "cal_msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <langinfo.h>
char *center();
static char *nlfile;

static char *days[] = {
	"Sun","Mon","Tue","Wed",
	"Thu","Fri","Sat"
};


#define MSGSTR(Num, Str) catgets(catd, MS_CAL, Num, Str)
static nl_catd catd;


static char	string[STRSIZE];  /* days of the calendar(s) */
static char    mhdr[MOWIDTH+1];  /* month title for single month calendar */

/*
 * NAME: main
 *                                                                    
 * FUNCTION: The cal command does one of four things when invoked:
 *	     - prints current month of current year by default
 *	     - prints a full year calendar if called with one argument
 *	     - prints a single month calendar if called with two 
 *	       arguments
 *
 * RETURN VALUE DESCRIPTION:  0  successful completion of any of the above 
 *			      1  bad input detected 
 */  
main(argc, argv)
int argc;
char *argv[];
{
	register int y, m, c, i, j;
	long today;
	struct tm *ts;
	
        (void)setlocale(LC_ALL, ""); 
        catd = catopen(MF_CAL, NL_CAT_LOCALE);
   
	initdaysmonths();

	while ((c=getopt(argc,argv,"")) != EOF) {
		switch (c) {
			default:
				usage();
		}
	}

	switch (c=(argc - optind)) {
		case 0: /* print out just month */
			time(&today);
			ts = localtime(&today);
		case 2: /* print out just month */
			m = c ? number(argv[optind]) : ts->tm_mon+1;
			y = c ? number(argv[optind+1]) : 1900 + ts->tm_year;
			if ((m<1 || m>12) || (y<1 || y>MAXYR))
				badarg();
			sprintf(mhdr, "%s %u",lmon[m-1], y);
			printf("%s\n", center(mhdr, TTLWIDTH));
			prn_days_hdr();
			printf("\n");
			cal(m, y, string, MOWIDTH);
			for(i=0; i<MODEPTH*MOWIDTH; i+=MOWIDTH)
				pstr(string+i, MOWIDTH);
			exit(0);
		case 1: /* print out just complete year */
			y = number(argv[optind]);
		   	if(y<1 || y>MAXYR)
				badarg();
	   		printf("\n\n\n");
          	 	printf("				%u\n", y);
           		printf("\n");
	   		for(i=0; i<12; i+=2) {
				for(j=0; j<STRSIZE; j++)
					string[j] = '\0';

				printf("%s   ", center(lmon[i],TTLWIDTH));
				printf("%s\n", center(lmon[i+1],TTLWIDTH));
				prn_days_hdr(); prn_days_hdr();
				printf("\n");

				cal(i+1, y, string, LNWIDTH);
				cal(i+2, y, string+MOWIDTH-1, LNWIDTH);

			/* print out the rows and columns of month days */
				for(j=0; j<STRSIZE; j+=LNWIDTH)
					pstr(string+j, LNWIDTH);
	   		}
           		printf("\n\n\n");
	   		exit(0);
		default:
			fprintf(stderr,MSGSTR(BADNOARGS, "Too many arguments.\n"));
			exit(1);
	}
}

/*
 * NAME: number
 *                                                                    
 * FUNCTION: converts a string of numbers to its numeric value
 *
 * RETURN VALUE DESCRIPTION:  the numeric value of the string if its characters
 *			      are 0 - 9, zero otherwise
 */  
static int number(str)
char *str;
{
	register int n, c;
	register char *s;

	n = 0;
	s = str;
	while(c = *s++) {
		if(c<'0' || c>'9')
			return(0);
		n = n*10 + c-'0';
	}
	return(n);
}

/*
 * NAME: pstr
 *                                                                    
 * FUNCTION:  prints the given string to stdout, after replacing nulls with
 *            blanks
 *                                                                    
 * RETURN VALUE DESCRIPTION: NONE
 *
 */  
static void  pstr(str, n)           /* void * declaration causes clash with str */
char *str;
int n;
{
	register int i;
	register char *s;

	s = str;
	i = n;
	while(i--)
		if(*s++ == '\0')
			s[-1] = ' ';         /* replace the null */
	i = n+1;
	while(i--)
		if(*--s != ' ')
			break;
	s[1] = '\0';
	printf("%s\n", str);
}

/*
 * NAME: cal
 *                                                                    
 * FUNCTION: fills an array with the days of the given month 
 *                                                                    
 * RETURN VALUE DESCRIPTION: NONE 
 *			     
 */  
static void cal(m, y, p, w)        
int m,y,w;      /* month, year, offset to the next week of current month */
char *p;	/* start addr of the array to be loaded with calendar days */ 
{
	register int d, i; /* day of the week, counter */
	register char *s;  /* current pointer position in the array of days */

	s = p;
	d = jan1(y);
	mon[2] = 29;
	mon[9] = 30;

	switch((jan1(y+1)+7-d)%7) {

	/*
	 *	non-leap year
	 */
	case 1:
		mon[2] = 28;
		break;

	/*
	 *	1752
	 */
	default:
		mon[9] = 19;
		break;

	/*
	 *	leap year
	 */
	case 2:
		;
	}
	for(i=1; i<m; i++) {
		d += mon[i];    
		}
	d %= 7;		
	s += COLUMN_WIDTH*d;
	for(i=1; i<=mon[m]; i++) {
		if(i==3 && mon[m]==19) {
			i += 11;
			mon[m] += 11;
		}
		if(i > 9)
			*s = i/10+'0';   /* load the 10's digit */
		s++;
		*s++ = i%10+'0';         /* load the  1's digit */
		s++; s++;
		if(++d == 7) {
			d = 0;
			s = p+w;   /* advance to load the next week */         
			p = s;
		}
	}
}


/*
 * NAME: jan1
 *                                                                    
 * FUNCTION: returns the day of the week of Jan 1 of the given year
 *                                                                    
 * RETURN VALUE DESCRIPTION:  0 through 6 correspond to Sunday through Saturday 
 *			     
 */  

static int jan1(yr)
int yr;
{
	register int y, d;

/*
 *	normal gregorian calendar has
 *	one extra day per four years
 */

	y = yr;
	d = 4+y+(y+3)/4;

/*
 *	julian calendar is the
 *	regular gregorian
 *	less three days per 400
 */

	if(y > 1800) {
		d -= (y-1701)/100;
		d += (y-1601)/400;
	}

/*
 *	great calendar changeover instant
 */

	if(y > 1752)
		d += 3;

	return(d%7);
}

/*
 * NAME: center
 *                                                                    
 * FUNCTION: centers string s in a field of length len
 *                                                                    
 * RETURN VALUE DESCRIPTION: NONE 
 *			     
 */  
static char *center(s, len)
   char *s; int len; 
{
	int i, slen;
	static char buf[4 * MOWIDTH + 1];

	slen = mbslen(s);
	if (slen > len)
		mbsncpy(buf, s, len);
	else {
		for(i=0;i<((len-slen)/2)+((len-slen)%2);i++) buf[i]=' ';
		strcpy(&buf[i], s);
		for(i=strlen(buf);i<len;i++) buf[i]=' ';
	}
	buf[strlen(buf)]='\0';
	return(buf);
}



/*
 * NAME: initdaysmonths
 * FUNCTION: initialize the arrays of days and months.
 */

static nl_item dayitems[7] = {
	DAY_1, DAY_2, DAY_3, DAY_4,
	DAY_5, DAY_6, DAY_7
};

static nl_item abdayitems[7] = {
	ABDAY_1, ABDAY_2, ABDAY_3, ABDAY_4,
	ABDAY_5, ABDAY_6, ABDAY_7
};

static nl_item monthitems[12] = {
	MON_1, MON_2, MON_3, MON_4, MON_5, MON_6,
	MON_7, MON_8, MON_9, MON_10, MON_11, MON_12
};

static initdaysmonths()
{
	char *ptr, *cp;
	int i;

	for(i = 0; i < 7; i++) {
		ptr = nl_langinfo(dayitems[i]);
		if((days[i] = malloc(strlen(ptr) + 1)) != NULL) {
			strcpy(days[i], ptr);
		}
	}

	for(i = 0; i < 7; i++) {
		ptr = nl_langinfo(abdayitems[i]);
		if((dayw[i] = malloc(strlen(ptr) + 1)) != NULL) {
			strcpy(dayw[i], ptr);
		}
	}

	for(i = 0; i < 12; i++) {
		ptr = nl_langinfo(monthitems[i]);
		if((lmon[i] = malloc(strlen(ptr) + 1)) != NULL) {
			strcpy(lmon[i], ptr);
		}
	}

}

/*
 * NAME: prn_days_hdr
 * FUNCTION: print the days header above the calendar
 */

static prn_days_hdr()
{
	int i;

	for(i = 0; i < 7; i++) {
		printf("%-4s", dayw[i]);
	}
	printf(" ");
}

static usage()
{
	fprintf(stderr,MSGSTR(USAGE,"usage: cal [[month] year]\n"));
	exit(1);
}

static badarg()
{
	fprintf(stderr,MSGSTR(BADARG, "Bad argument.\n"));
	exit(1);
}

