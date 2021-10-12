static char sccsid[] = "@(#)15	1.10.1.2  src/bos/usr/lbin/calprog/calprog.c, cmdmisc, bos41B, 9504A 1/4/95 14:12:25";
/*
 * COMPONENT_NAME: (CMDMISC) miscellaneous commands 
 *
 * FUNCTIONS: calprog
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	calprog.c - used by /usr/bin/calendar command.
 *
 *  NAME: calprog
 *                                                                     
 *  FUNCTION:  /usr/lbin/calprog produces an egrep -f file
 *             that will select today's and tomorrow's
 *             calendar entries, with special weekend provisions
 *             used by calendar command
 */                                                                    

#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <nl_types.h>
#include <langinfo.h>
#include <ctype.h>
static char *nlfile;
static int dayfirst=0;
static char sep[3];

#include <time.h>
#define DAY (3600*24L)

static struct mname{
	wchar_t c1;		/* First character of month name */
	wchar_t c1_alt;		/* First character of month name (2nd case) */
	wchar_t c1_ab;		/* First character of short month name */
	wchar_t c1_ab_alt;	/* First character of short month (2nd case) */
	char *rest;		/* The rest of the month name  (multi-byte) */
	char *rest_ab;		/* The rest of the abbreviated month name */
} mname;

static nl_item months[] = {
	MON_1, MON_2, MON_3, MON_4, MON_5, MON_6,
	MON_7, MON_8, MON_9, MON_10, MON_11, MON_12
};

static nl_item abmonths[] = {
	ABMON_1, ABMON_2, ABMON_3, ABMON_4, ABMON_5, ABMON_6,
	ABMON_7, ABMON_8, ABMON_9, ABMON_10, ABMON_11, ABMON_12
};

static tprint(t)
long t;
{
	struct tm *tm;
	int c1size;
	char *s;
	int mb_cur_max = MB_CUR_MAX;

	tm = localtime(&t);

	/* Get info about month name */
	s=nl_langinfo(months[tm->tm_mon]); 
	if ((mname.rest = (char *)malloc(strlen(s))) != NULL) {
		c1size = mbtowc(&mname.c1, s, mb_cur_max);
		s += c1size;	/* Skip the first character */
		strcpy(mname.rest, s);
		if (iswupper(mname.c1))
			mname.c1_alt = towlower(mname.c1);
		else
			mname.c1_alt = towupper(mname.c1);
	}

	/* Get info about abbreviated month name */
	s=nl_langinfo(abmonths[tm->tm_mon]); 
	if ((mname.rest_ab = (char *)malloc(strlen(s))) != NULL) {
		c1size = mbtowc(&mname.c1_ab, s, mb_cur_max);
		s += c1size;	/* Skip the first character */
		strcpy(mname.rest_ab, s);
		if (iswupper(mname.c1_ab))
			mname.c1_ab_alt = towlower(mname.c1_ab);
		else
			mname.c1_ab_alt = towupper(mname.c1_ab);
	}

	/*
	 * Note: The regular expression to be printed is broken up into
	 * 3 printf's because SCCS expands "<percent>C<percent>".
	 */
	if (dayfirst) {
	  printf("(^|[[:blank:][:punct:]])(0*%d(( |%s)([%C", tm->tm_mday, sep, mname.c1);
	  printf("%C]%s|[%C", mname.c1_alt, mname.rest, mname.c1_ab);
	  printf("%C]%s)[^ ]* *|%s0*%d|%s\\*))([^0123456789]|$)\n",
		mname.c1_ab_alt, mname.rest_ab, sep,tm->tm_mon + 1,sep);
	} else {
	  printf("(^|[[:blank:][:punct:]])((([%C", mname.c1);
	  printf("%C]%s|[%C", mname.c1_alt, mname.rest, mname.c1_ab);
	  printf("%C]%s)[^ ]* *|0*%d%s|\\*%s)0*%d)([^0123456789]|$)\n",
		mname.c1_ab_alt, mname.rest_ab, tm->tm_mon + 1,sep,sep,tm->tm_mday);
	}
}

main()
{
	long t;
	int i;

	(void) setlocale(LC_ALL,"");
        if ((nlfile = nl_langinfo(D_FMT)) != NULL) {
		i=0;
		while(nlfile[i++] != '%');		
		if (nlfile[i++] == 'd')
			dayfirst++;
		if (nlfile[i] == '.' || nlfile[i] == '*' || nlfile[i] == '|'
				|| nlfile[i] == '^' || nlfile[i] == '\\') {
			sep[0] = '\\';	/* must escape special character */
			sep[1] = nlfile[i];
		}
		else
			sep[0] = nlfile[i];	
	}
	time(&t);
	tprint(t);
	switch(localtime(&t)->tm_wday) {
	/* Cases 5 and 6 take care of the weekend. */
  	case 5:
  		t += DAY;
        	tprint(t);
   	case 6:
  		t += DAY;
  		tprint(t);
	default:
		t += DAY;
		tprint(t);
	}
}
