static char sccsid[] = "@(#)66	1.7.1.9  src/bos/usr/ccs/lib/libc/__strftime_std.c, libcfmt, bos41J, 9521B_all 5/26/95 14:57:59";
/*
 * COMPONENT_NAME: LIBCFMT
 *
 * FUNCTIONS:  __strftime_std
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _ILS_MACROS
#include <sys/localedef.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <langinfo.h>
#include <stdlib.h>
#include "ts_supp.h"

#define BADFORMAT  format=fbad; \
		   bufp = "%" ; \
		   width = 0 
#define BUFSIZE    1000
#define WIDTH(a)   (wpflag ? 0  : (a))
#define PUT(c)     (strp < strend ? *strp++  = (c) : toolong++)
#define GETSTR(f)  t=f; \
                   while(*subera && *subera != ':') \
                        *t++ = *subera++; \
                   *t = '\0'

#define STRTONUM(str,num)       num = 0; \
				while (isdigit (*str)) { \
					num *= 10; \
					num += *str++ - '0'; \
				}
/* codes used by doformat() */

#define NOYEAR     2
#define NOSECS     3
#define SKIP       for(strp=lastf; (i = *format) && i != '%'; format++ )

struct era_struct {
        char    dir;            /* direction of the current era */
        int     offset;         /* offset of the current era */
        char    st_date[100];   /* start date of the current era */
        char    end_date[100];  /* end date of the current era */
        char    name[100];      /* name of the current era */
        char    form[100];      /* format string of the current era */
};
typedef struct era_struct *era_ptr;

static char *getnum(int i, int n, struct globals *global_data);

static char *gettimezone(struct tm *timeptr);
static int init_altdigits(_LC_time_objhdl_t hdl, char *fmt, struct globals *gd);
static char *getnumber(int i, struct globals *global_data);
static int conv_time(_LC_time_objhdl_t hdl, struct tm *tm, era_ptr era, 
		     int *year);
static size_t doformat(_LC_time_objhdl_t hdl, char *s, size_t maxsize, 
		       char *format, struct tm *timeptr, int code, struct globals *);

/* For thread safety, all global data has been moved into this structure
 *  and will be passed on to those routines that need it.  The defines will
 *  be used to avoid changing the actual code.
 */
#define ALTDIGITS_MAX 101

struct globals {
	/* The following fields used to be global statics. */
	char *bufp;
	char buffer[BUFSIZE];
	struct era_struct eras;		/* the structure for current era     */
	era_ptr	eraptr;			/* pointer to the current era        */
	char *localedigits;		/* point to locale alt_digits string */
	short locoffset[ALTDIGITS_MAX];	/* store offsets into localedigits[] */
	int digitscnt;		/* number of actual alternate digit strings  */
	/* The following fields used to be static in doformat and were 
	 * maintained from one invocation of doformat to another.  */
	int era_name;		/* logical flag for detected era name   */
	int era_year;		/* logical flag for detected era year   */
				/* If timeptr info matches s_era_year,  */
	int s_era_year;		/*   s_era_mon, and s_era_day, then the */
	int s_era_mon;		/*   era struct contains the pertinent  */
	int s_era_day;		/*   era info. for timeptr.             */
	int alt_digits;		/* if alt_digits/alt_format are 1, then */
	int alt_format;		/*   alternate digits/alternate formats */
				/*   (e.g. era_d_fmt, etc.) are used.   */
	};

/*
 * FUNCTION: strftime_std()
 *	     This is the standard method to format the date and ouput to 
 *	     the output buffer s. The values returned are affected by 
 *	     the setting of the locale category LC_TIME and the time 
 *  	     information in the tm time structure.
 *
 * PARAMETERS:
 *	     _LC_TIME_objhdl_t hdl - the handle of the pointer to the LC_TIME
 *			       catagory of the specific locale.
 *           char *s - location of returned string
 *           size_t maxsize - maximum length of output string
 *           char *format - format that date is to be printed out
 *           struct tm *timeptr - date to be printed
 *
 * NOTES:  About alternate digits.
 *
 *    -If the locale does define alternate digits, then an array of at most
 *      ALTDIGITS_MAX short ints is created via malloc, and localedigits is set
 *      to point to the begining of the locale's string alt_digits.
 *    -This array is then filled with offsets relative to the begining of the
 *      string localedigits.  For example, the alternate digit string for 4
 *      begins at "localedigits+locoffset[4]" and is of length 
 *      "locoffset[5]-locoffset[4]-1".  The last offset will always point to 
 *      the null-terminator (this is the ending offset.)  XPG4 says there can
 *      be at most 100 alternate digits, so there will be be at most 101 
 *      offsets (including the ending offset.)
 *    -It's also possible to skip an alternate digit (e.g. "zero;;;three"), in
 *      which case we have to use the normal number (e.g. "2" for 2).
 *      Note that the offsets will show a length of zero.
 *    -Since it's possible to define less than 100 alternate digits as in the
 *      example of "zero;;;three", the variable digitscnt contains the actual
 *      number of alternate strings defined (note that this includes any
 *      that were skipped--e.g. "zero;;;three" contains four.), so that any
 *      numbers OTHER than 0 and 3 will use their regular numbers.
 *    
 * RETURN VALUE DESCRIPTIONS:
 *           - returns the number of bytes that comprise the return string
 *	       excluding the terminating null character.
 *           - returns 0 if s is longer than maxsize
 */

size_t 
__strftime_std(_LC_time_objhdl_t hdl, char *s, size_t maxsize, 
		      const char *format, const struct tm *timeptr)
{
    struct globals gd;	/* global data */

    gd.bufp = gd.localedigits = NULL;
    gd.eraptr = &gd.eras;
    gd.era_name = gd.era_year = gd.s_era_year = gd.s_era_mon = gd.s_era_day = 0;
    gd.digitscnt = -1;
    gd.alt_digits = gd.alt_format = 0;

    tzset();

    init_altdigits(hdl, (unsigned char *) format, &gd);

    return doformat(hdl, s, maxsize, (char *)format, (struct tm *) timeptr, 0, &gd);
}

/*
 * FUNCTION: init_altdigits() checks to see if the format passed in to it
 *	     contains any format specifiers that begin with "%O", taking into
 *	     account the possibility of a width and/or precision.  If so,
 *	     then this format will need the locale's alternate digits.  This
 * 	     routine sets this up.  See the "NOTES:  About alternate digits"
 *	     section above for a more detailed description of what is setup.
 *
 * PARAMETERS:
 *           _LC_time_objhdl_t hdl - pointer to the handle of the LC_TIME
 *			       catagory which contains all the time related
 *			       information of the specific locale.
 *	     const char *fmt - the format string which specifies the expected
 *			       format to be input from the input buf.
 *	     struct globals *gd
 *		   	     - contains all the info. needed from one function
 *		     	       to another (eg: alt_digits, localedigits, etc..
 *
 */
static int
init_altdigits(_LC_time_objhdl_t hdl, char *fmt, struct globals *gd)
{
    char *p = fmt;
    /* Since the locale may be changed from one invocation to another,
     *   setup the locale digits if there is a "%[width][.prec]O" in the fmt
     *   string.  Note that the field width and precision are optional.
     */
    while(*p && (p=strchr(p, '%'))) {
	p++;
	while(*p && (*p>='0' && *p<='9')) 	p++;
	if (*p=='.') {
		p++;
		while(*p && (*p>='0' && *p<='9'))
			p++;
		}
	if (*p=='O')
		break;		/* found an O specifier               */
	p++;			/* skip the specifier in case of "%%" */
	}
    
    if (*p) {
	gd->localedigits = __OBJ_DATA(hdl)->alt_digits;
	if (gd->localedigits != NULL && *(gd->localedigits)!='\0') {
		int i = 1;	  /* If we're here, then there's at */
		p = gd->localedigits; /*      least one alternate digit */

		do { if (*p==';') i++; } while (*p++);
	
		if (i>ALTDIGITS_MAX)
			i = ALTDIGITS_MAX; /* Only allow ALTDIGITS_MAX offsets */
	
		gd->digitscnt = i;	/* Number of alternate strings */
	
		gd->locoffset[0] = 0;
		p=gd->localedigits;
		i = 1;
		while (*p && i<ALTDIGITS_MAX) {
			while (*p && *p!=';') p++;
			gd->locoffset[i++] = p - gd->localedigits + 1;
			if (*p) p++;
			}
		}
	}
		
}

/*
 * Access to global data through the structure.  Using defines
 *   so that most of the code remains unchanged.
 */
#define bufp		(global_data->bufp)
#define buffer		(global_data->buffer)
#define eras		(global_data->eras)
#define eraptr		(global_data->eraptr)
#define localedigits	(global_data->localedigits)
#define locoffset	(global_data->locoffset)
#define digitscnt	(global_data->digitscnt)

#define era_name	(global_data->era_name)
#define era_year	(global_data->era_year)
#define s_era_year	(global_data->s_era_year)
#define s_era_mon	(global_data->s_era_mon)
#define s_era_day	(global_data->s_era_day)
#define alt_digits	(global_data->alt_digits)
#define alt_format	(global_data->alt_format)

/*
 * FUNCTION: This function performs the actual formatting and it may
 *	     be called recursively with differernt values of code value.
 *
 * PARAMETERS:
 *           _LC_TIME_objhdl_t hdl - the handle of the pointer to the LC_TIME
 *                             catagory of the specific locale.
 *           char *s - location of returned string
 *           size_t maxsize - maximum length of output string
 *           char *format - format that date is to be printed out
 *           struct tm *timeptr - date to be printed
 *	     int code - this special attribute controls the outupt of
 *		    	certain field (eg: twelve hour form, without
 *			year or second for time and date format).
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - returns the number of bytes that comprise the return string
 *             excluding the terminating null character.
 *           - returns 0 if s is longer than maxsize
 */

static size_t 
doformat(_LC_time_objhdl_t hdl, char *s, size_t maxsize, char *format, 
	struct tm *timeptr, int code, struct globals *global_data)
{
    int i;
    int firstday; 	/* first day of the year */
    int toolong = 0;
    int weekno, wmod;
    char *strp; 	/* pointer into output buffer str */
    char *strend;	/* last available byte in output buffer str */
    char *lastf;	/* last byte of str made by a format */
    int width;          /* width in current format or 0 */
    int prec;           /* precision in current format or 0 */
    int wpflag;         /* true if width or precision specified */
    char locbuffer[BUFSIZE];	/* local temporary buffer */
    int year;           	/* %o value, year in current era */
    int	found;			/* logical flag for a valid era */
    char *fbad;        		/* points to where format start to be invalid */
    char    *era_s;		/* locale's empiror/era string */
    char    *subera;		/* era string of a multiple era */
    char    *f;             	/* era format of subera */
    char    erabuf[BUFSIZE];  	/* a work buffer for the era string */
    char    fill_char;		/* filling char which may be blank or zero */
    char    *dtfmt;		/* temporary date/time format pointer */

    lastf = strp = s;
    strend = s+maxsize-1;
    while (i = *format++) {
	if (i != '%')
	    PUT(i);
	else {
	    wpflag = width = prec = 0;
	    fbad = format;
	    fill_char = ' ';			/* blank is default fill char */
	    /* get field width & precision */
	    i = 0;
	    if (*format == '-') {		/* right justify */
		i++;
		format++;
		wpflag++;
	    }
	    else if (*format == '0') {		/* right justify pad with 0 */
		format++;
		fill_char = '0';
	    }
	    while(isdigit(*format)) {
		width *= 10;
		width += *format++ - '0';
		wpflag++;
	    }
	    if (i)
		width = -width;
	    if (*format == '.') {
		wpflag++;
		format++;
		while(isdigit(*format)) {
		    prec *= 10;
		    prec += *format++ - '0';
		}
	    }
	    switch(*format++) {
	    case '%':
		      bufp = "%";	/* X/Open - percent sign */
		      break;

	    case 'n': 
		      bufp = "\n";	/* X/Open - newline character */
		      break;

	    case 't': 
		      bufp = "\t";	/* X/Open - newline character */
		      break;

	    case 'm':			/* X/Open - month in decimal number */
		      bufp = getnum(timeptr->tm_mon+1,WIDTH(2), global_data);
		      break;

	    case 'd': 			/* X/Open - day of month in decimal */
		      bufp = getnum(timeptr->tm_mday,WIDTH(2), global_data);
		      break;
	    case 'e':			/* day of month with leading space */
		      bufp = getnum(timeptr->tm_mday,WIDTH(2), global_data);
		      if (*bufp == '0')
				*bufp = ' ';
		      break;
	    case 'y':			/* X/Open - year w/o century 00-99 */  
		      if (code==NOYEAR) 
			     SKIP;
		      else
                             if (alt_format) {
                                if (conv_time(hdl,timeptr,eraptr,&year))
                                        bufp = getnumber(year, global_data);
                                }
			     else
				bufp = getnum(timeptr->tm_year,WIDTH(2), global_data);
		      break;

	    case 'H':			/* X/Open - hour (0-23) in decimal */
		      bufp = getnum(timeptr->tm_hour,WIDTH(2), global_data);
		      break;

	    case 'M':			/* X/Open - minute in decimal */ 
		      bufp = getnum(timeptr->tm_min,WIDTH(2), global_data);
		      break;

	    case 'S':			/* X/Open - second in decimal */
		      if (code==NOSECS)
			     SKIP;
		      else
			     bufp = getnum(timeptr->tm_sec,WIDTH(2), global_data);
		      break;

	    case 'j': 			/* X/Open - day of year in decimal */
		      bufp = getnum(timeptr->tm_yday+1,WIDTH(3), global_data);
		      break;

	    case 'w': 			/* X/Open - weekday in decimal */
		      bufp = getnum(timeptr->tm_wday,WIDTH(1), global_data);
		      break;

	    case 'R':			/* X/Open - 24 hour notation */
		      doformat(hdl, locbuffer, BUFSIZE, "%H:%M", timeptr, 0, global_data);
		      bufp = locbuffer;
		      break;

	    case 'r': 			/* X/Open - time in British/US AM/PM */
		      if (localedigits == NULL)
				init_altdigits(hdl, __OBJ_DATA(hdl)->t_fmt_ampm, global_data);
		      doformat(hdl, locbuffer, BUFSIZE, __OBJ_DATA(hdl)->t_fmt_ampm, timeptr, 0, global_data);
		      bufp = locbuffer;
		      break;

	    case 'T': 			/* X/Open - time in %H:%M:%S notation */
		      doformat(hdl, locbuffer, BUFSIZE, "%H:%M:%S", timeptr, 0, global_data);
		      bufp = locbuffer;
		      break;
	    case 'X': 			/* X/Open - the locale time notation */
		      i = alt_format;
		      dtfmt = alt_format? __OBJ_DATA(hdl)->era_t_fmt : __OBJ_DATA(hdl)->t_fmt;

		      if (alt_format && (dtfmt==NULL || *dtfmt=='\0'))
				dtfmt = __OBJ_DATA(hdl)->t_fmt;

		      if (localedigits == NULL)
				init_altdigits(hdl, dtfmt, global_data);

		      alt_format = 0;	/* shouldn't affect anything while recursing */
		      doformat(hdl, locbuffer, BUFSIZE, dtfmt, timeptr, 0, global_data);
		      alt_format = i;
		      bufp = locbuffer;
		      break;

	    case 'l': 			/* IBM-long day name, long month name,
					   locale date representation*/
		      switch (*format++) {
		      case 'a': bufp = strcpy(locbuffer, 
					  __OBJ_DATA(hdl)->day[timeptr->tm_wday]);
				break;
		      case 'h': bufp = strcpy(locbuffer, 
					  __OBJ_DATA(hdl)->mon[timeptr->tm_mon]);
				break;
		      case 'D': doformat(hdl, locbuffer, BUFSIZE, "%b %d %Y", timeptr, 0, global_data);
				bufp = locbuffer;
				break;
		      default : BADFORMAT;
		      }
		      break;

	    case 's': 			/* IBM-hour(12 hour clock), 
					long date w/o year, long time w/o secs*/
		      switch (*format++) {
		      case 'H': i=timeptr->tm_hour;
				bufp=getnum(i>12?i-12:i,WIDTH(2), global_data);
				break;
		      case 'D': 
				{
				char *p = nl_langinfo(NLLDATE);
			        if (localedigits == NULL)
					init_altdigits(hdl, p, global_data);
				doformat(hdl, locbuffer, BUFSIZE, p, timeptr, NOYEAR, global_data);
		      		bufp = locbuffer;
				break;
				}
		      case 'T': 
			        if (localedigits == NULL)
					init_altdigits(hdl, __OBJ_DATA(hdl)->t_fmt, global_data);
				doformat (hdl, locbuffer, BUFSIZE, __OBJ_DATA(hdl)->t_fmt, timeptr, NOSECS, global_data);
		      		bufp = locbuffer;
				break;
		      default : BADFORMAT;
		      }
		      break;

	    case 'a': 			/* X/Open - locale's abv weekday name */
		      bufp = strcpy(locbuffer, 
				    __OBJ_DATA(hdl)->abday[timeptr->tm_wday]);
		      break;
	
	    case 'h':			/* X/Open - locale's abv month name */

	    case 'b': 
		      bufp = strcpy(locbuffer, 
				    __OBJ_DATA(hdl)->abmon[timeptr->tm_mon]);
		      break;

	    case 'p': 			/* X/Open - locale's equivalent AM/PM */
		      if (timeptr->tm_hour<12)
				strcpy(locbuffer, __OBJ_DATA(hdl)->am_pm[0]);
		      else
				strcpy(locbuffer, __OBJ_DATA(hdl)->am_pm[1]);
		      bufp = locbuffer;
		      break;

	    case 'C':			/* X/Open - century in decimal (truncate) */
		      if (alt_format && eraptr->name && *eraptr->name)
			      bufp = eraptr->name;
		      else
			      bufp = getnum((timeptr->tm_year+1900)/100, WIDTH(2), global_data);
		      break;

	    case 'Y':			/* X/Open - year w/century in decimal */
		      if (code==NOYEAR)
			     SKIP;
		      else
			     if (alt_format) {
				doformat(hdl, locbuffer, BUFSIZE, eraptr->form, timeptr, 0, global_data);
				bufp = locbuffer;
				}
			     else
				bufp = getnum(timeptr->tm_year+1900,WIDTH(4), global_data);
		      break;

	    case 'z':			/* IBM - timezone name if it exists */

	    case 'Z':			/* X/Open - timezone name if exists */
		      bufp = gettimezone(timeptr);
		      /* If gettimezone() returns 3 blanks, assume that
			 TZ is invalid and use a null value instead. */
		      if (strcmp(bufp, "   ") == 0)
			bufp = "";
		      break;

	    case 'A': 			/* X/Open -locale's full weekday name */
		      bufp = strcpy(locbuffer, 
				    __OBJ_DATA(hdl)->day[timeptr->tm_wday]);
		      break;

	    case 'B':			/* X/Open - locale's full month name */
		      bufp = strcpy(locbuffer, 
				    __OBJ_DATA(hdl)->mon[timeptr->tm_mon]);
		      break;


	    case 'I': 			/* X/Open - hour (1-12) in decimal */
		      i = timeptr->tm_hour;
		      bufp = getnum(i>12?i-12:i?i:12,WIDTH(2), global_data);
		      break;

	    case 'D': 			/* X/Open - date in %m/%d/%y format */
		      doformat(hdl, locbuffer, BUFSIZE, "%m/%d/%y", timeptr, 0, global_data);
		      bufp = locbuffer;
		      break;

	    case 'x': 			/* X/Open - locale's date */
		      i = alt_format;
		      dtfmt = alt_format? __OBJ_DATA(hdl)->era_d_fmt : __OBJ_DATA(hdl)->d_fmt;

		      if (alt_format && (dtfmt==NULL || *dtfmt == '\0'))
				dtfmt = __OBJ_DATA(hdl)->d_fmt;

		      if (localedigits == NULL)
				init_altdigits(hdl, dtfmt, global_data);

		      alt_format = 0;	/* shouldn't affect anything while recursing */
		      doformat(hdl, locbuffer, BUFSIZE, dtfmt, timeptr, 0, global_data);
		      alt_format = i;

		      bufp = locbuffer;
		      break;

	    case 'c': 			/* X/Open - locale's date and time */
		      i = alt_format;
		      dtfmt = alt_format? __OBJ_DATA(hdl)->era_d_t_fmt : __OBJ_DATA(hdl)->d_t_fmt;

		      if (alt_format && (dtfmt==NULL || *dtfmt=='\0'))
			      dtfmt = __OBJ_DATA(hdl)->d_t_fmt;

		      if (localedigits == NULL)
				init_altdigits(hdl, dtfmt, global_data);
		
		      alt_format=0;	/* shouldn't affect anything while recursing */
		      doformat(hdl, locbuffer, BUFSIZE, dtfmt, timeptr, 0, global_data);
		      alt_format=i;	/* turn back on if it was on before */

		      bufp = locbuffer;
		      break;

	    case 'u':			/* X/Open - decimal weekday (1-7) */
					/*   monday = 1 ... sunday = 7    */
		      bufp = getnum(timeptr->tm_wday? timeptr->tm_wday:7, WIDTH(1), global_data);
		      break;

	    case 'U': 			/* X/Open - week number of the year 
					  (Sunday as the first day) */
		      firstday=(timeptr->tm_wday-(timeptr->tm_yday % 7)+7) % 7;
		      weekno = (timeptr->tm_yday + firstday + 1) / 7;

		      wmod = (timeptr->tm_yday + firstday + 1) % 7;
		      if (wmod)
		      	  bufp = getnum(weekno,WIDTH(2), global_data);
		      else
		          bufp = getnum(weekno-1,WIDTH(2), global_data);
		      break;

	    case 'V': 		/* X/Open - week number of the year (Monday as the first day).  
					If week of Jan 1 has 4 or more days, then it is 
					week 1 otherwise, it is week 53 of previous year.  */
		      firstday=(timeptr->tm_wday-(timeptr->tm_yday % 7)+7) % 7; /* wkday of jan 1 */
		      weekno = (timeptr->tm_yday + firstday - 1) / 7;
		      if(firstday>0 && firstday<5) weekno++;  /* 4 or more days? */
		      if(firstday==0 && timeptr->tm_wday) weekno++; /* sun jan 1 && wkday=sun    */
							   /*  is special case (eg: Jan 8, 1995) */ 
		      if (weekno<1) weekno = 53;	   /* if week 0, make it week 53         */
		      bufp = getnum(weekno,WIDTH(2), global_data);
		      break;

	    case 'W': 			/* X/Open - week number of the year 
					  (Monday as the first day) */
		      firstday=(timeptr->tm_wday-(timeptr->tm_yday % 7)+7) % 7;
		      weekno = (timeptr->tm_yday + firstday) / 7;
		      wmod = (timeptr->tm_yday + firstday) % 7;
		      if (firstday == 0)
		          if (wmod)
		      	    bufp = getnum(weekno+1,WIDTH(2), global_data);
		        else
		            bufp = getnum(weekno,WIDTH(2), global_data);
		      else
		          if (wmod)
		      	    bufp = getnum(weekno,WIDTH(2), global_data);
		        else
		            bufp = getnum(weekno-1,WIDTH(2), global_data);
		      break;

		 /* This is the additional code to support non-Christian
		   eras. The formatter %Jy will display the relative
		   year from the relevant era entry in NLYEAR, %Js will
		   display the era name.
		 */

	    case 'J': 			/* IBM - era and year of the Emperor */
		     switch(*format++) {
		    	 case 'y': 
				    if (! *__OBJ_DATA(hdl)->era) {
					BADFORMAT;
				    }
				    else if (era_name) {
					bufp = getnumber(year, global_data);
					era_year = 0;
					era_name = 0;
				    }
				    else if (conv_time(hdl,timeptr,eraptr,&year)){
				    	bufp = getnumber(year, global_data);
				    	era_name = 1;
				    }
				    else {
					BADFORMAT;
				    }
				    break;
			 case 's':
			  	    if (! *__OBJ_DATA(hdl)->era){
	  	    		   	BADFORMAT;
				    }
				    else if (era_year) {
					bufp = eraptr->name;
					era_year = 0;
					era_name = 0;
				    }
				    else if (conv_time(hdl,timeptr,eraptr,&year)){
					bufp = eraptr->name;
					era_name = 1;
				    }
				    else {
					BADFORMAT;
	 			    }
				    break;
			 default:
				   BADFORMAT;
		      		   break;
		      }
		      break;

	    case 'E':	/* X/open, locale's alternative date & time info.    */
		      { 
			/* Find start of format specifier, and convert it    */
			/*  from %-5.5Ey to %-5.5y for example and setup     */
			/*  era info. if necessary/possible and then call    */
			/*  doformat() without the modifier and let the      */
			/*  flag alt_format dictate the way the conversion   */
			/*  should proceed. 				     */

			char *fmtptr=format-2, tmpfmt[10];

			/* By this point, there is a '%' a few bytes before  */
			/*  we reach this point, so there's no problem going */
			/*  too far back.				     */
			while(*fmtptr!='%') fmtptr--;
			strncpy(tmpfmt, fmtptr, format-fmtptr);
			tmpfmt[format-fmtptr-1] = *format;
			tmpfmt[format-fmtptr] = '\0';
			
			alt_format = 1;		/* make sure alternative     */
						/*  formats are used         */

			/* For EC, Ey, & EY, we need information from the era.*/
			/* if conv_time() already called with this tm struct, */
			/*   reuse era info.  Else call conv_time() to setup  */
			/*   era struct.  If it returns a 0, then there are   */
			/*   no eras in the locale, so use the regular format */

			if ((*format=='C' || *format=='y' || *format=='Y') &&
				      (s_era_year!=timeptr->tm_year ||
				       s_era_mon!=timeptr->tm_mon   ||
				       s_era_day!=timeptr->tm_mday    ))
				{
				s_era_year = s_era_mon = s_era_day = 0;
				alt_format = conv_time(hdl, timeptr, eraptr, &year); 
				if (alt_format) {
					s_era_year = timeptr->tm_year;
					s_era_mon = timeptr->tm_mon;
					s_era_day = timeptr->tm_mday;
					}
				}
	
			doformat(hdl, locbuffer, BUFSIZE, tmpfmt, timeptr, 0, global_data);

			format++; 		/* skip modified suboption.   */
			bufp = locbuffer;
			alt_format = 0;
			}
		      break;
		
	    case 'O':	/* X/open, alternative numeric symbols 		     */
		      {
			/* Find start of format specifier, and convert it    */
			/*  from %-5.5Od to %-5.5d for example and setup     */
			/*  era info. if necessary/possible and then call    */
			/*  doformat() without the modifier and let the      */
			/*  flag alt_format dictate the way the conversion   */
			/*  should proceed. 				     */

			char *fmtptr=format-2, tmpfmt[10];

			/* By this point, there is a '%' a few bytes before  */
			/*  we reach this point, so there's no problem going */
			/*  too far back.				     */
			while(*fmtptr!='%') fmtptr--;
			strncpy(tmpfmt, fmtptr, format-fmtptr);
			tmpfmt[format-fmtptr-1] = *format;
			tmpfmt[format-fmtptr] = '\0';
			
			alt_digits=1;
			doformat(hdl, locbuffer, BUFSIZE, tmpfmt, timeptr, 0, global_data);
			alt_digits=0;

			format++;
			bufp=locbuffer;
			}
		      break;

	    case 'N':			/* locale's era name */
	  		if (! *__OBJ_DATA(hdl)->era){
		   		BADFORMAT;
		    	}
		    	else if (era_year) {
				bufp = eraptr->name;
				era_year = 0;
				era_name = 0;
			}
			else if (conv_time(hdl, timeptr, eraptr, &year)){
				bufp = eraptr->name;
				era_name = 1;
			}
			else {
				BADFORMAT;
	 	    	}
			break;

	    case 'o':			/* era year */
			if (! *__OBJ_DATA(hdl)->era) {
				BADFORMAT;
			}
			else if (era_name) {
				bufp = getnumber(year, global_data);
				era_year = 0;
				era_name = 0;
			}
			else if (conv_time(hdl, timeptr, eraptr, &year)) {
				bufp = getnumber(year, global_data);
				era_name = 1;
			}
			else {
				BADFORMAT;
			}
			break;

	    default:			 /* badformat */
			BADFORMAT;
			break;
	    } /* switch */

	/* output bufp with appropriate padding */

	i = strlen(bufp);
	if (prec && prec<i) {	 /* truncate on right */
	    * (bufp + prec) = '\0' ;
	    i = prec;
	}
	if (width>0)
	    while(!toolong && i++ < width)
		PUT(fill_char);
	while(!toolong && *bufp)
		PUT(*bufp++);
	if (width<0)
	    while(!toolong && i++ < -width)
		PUT(fill_char);
	lastf = strp;
	} 			/* i == '%' */
	if (toolong)
		break;
    }
    *strp = 0;
    if(toolong)
	return(0);
    else
        return (strp - s);
}



/*
 * FUNCTION: conv_time()
 *	     This function converts the current Christian year into year
 * 	     of the appropriate era. The era chosen such that the current 
 *	     Chirstian year should fall between the start and end date of 
 * 	     the first matched era in the hdl->era string. All the era 
 *	     associated information of a matched era will be stored in the era 
 *	     structure and the era year will be stored in the year 
 *	     variable.
 *
 * PARAMETERS:
 *           _LC_TIME_objhdl_t hdl - the handle of the pointer to the LC_TIME
 *                             catagory of the specific locale.
 *           struct tm *timeptr - date to be printed
 *	     era_ptr era - pointer to the current era.
 *	     int *year - year of the current era.
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - returns 1 if the current Christian year fall into an
 *	       valid era of the locale.
 *           - returns 0 if not.
 */

static int 
conv_time (_LC_time_objhdl_t hdl, struct tm *tm, era_ptr era, 
		      int *year)
{
	char *subera;
	char erabuf[BUFSIZE];
	char *era_s;
        char *str;
	char *t;
        int start_year = 0;
	int start_month = 0;
	int start_day = 0;
        int end_year = 0;
	int end_month = 0;
	int end_day = 0;
	int cur_year = 0;
	int cur_month = 0;
	int cur_day = 0;
        int no_limit = 0;
	int found = 0;
	int extra = 0;		/* extra = 1 when current date is less than
			           the start date, otherwise 0. This is the 
				   adjustimet for correct counting up to the
				   month and day of the start date */

	cur_year = tm->tm_year + 1900;
	cur_month = tm->tm_mon + 1;
	cur_day = tm->tm_mday;
	era_s = __OBJ_DATA(hdl)->era;
	while (*era_s && !found) {
		subera = erabuf;
		while (*era_s && *era_s != ';')
			*subera++ = *era_s++;	/* copy the subera up to ';' */
		*subera = '\0';
		subera = erabuf;
		if (*era_s)
			era_s++;	/* skip the semicolon */
		era->dir = *subera++;	/* set the direction */
		subera++;		/* skip the colon */
		STRTONUM(subera,era->offset);		/* set the offset */
		subera++;		/*skip the colon */
		GETSTR (era->st_date);
		subera++;		/*skip the colon */
		GETSTR (era->end_date);
		subera++;		/*skip the colon */
		GETSTR (era->name);
		subera++;		/*skip the colon */
		GETSTR (era->form);

 	       	str = era->st_date;
       		if (*str == '-') {
                	str++;
			STRTONUM(str,start_year);
                	start_year = -start_year;
        	}
       		else
			STRTONUM(str,start_year);
		str++;			/* skip the slash */
		STRTONUM(str,start_month);
		str++;			/* skip the slash */
		STRTONUM(str,start_day);

        	str = era->end_date;
		if ((*str=='+' || *str=='-') && *(str+1)=='*')
                	no_limit = 1;
        	else {
                	no_limit = 0;
			if (*str == '-') {
                		str++;
				STRTONUM(str,end_year);
                		end_year = -end_year;
 	      		}
        		else
				STRTONUM(str,end_year);
			str++;		/* skip the slash */
			STRTONUM(str,end_month);
			str++;		/* skip the slash */
			STRTONUM(str,end_day);
		}
		if (no_limit && cur_year >= start_year)
			found = 1;
		else if (((cur_year > start_year) ||
			  (cur_year == start_year && cur_month > start_month) ||
			  (cur_year == start_year && cur_month == start_month &&
			   cur_day >= start_day)) &&
			 ((cur_year < end_year) ||
			  (cur_year == end_year && cur_month < end_month) ||
			  (cur_year == end_year && cur_month == end_month &&
			   cur_day <= end_day)))
			found = 1;
		else 
			continue; 

		if ((cur_month < start_month) || 
		   (cur_month == start_month && cur_day < start_day))
			extra = 1;
        	if (era->dir == '+')
                	*year = cur_year - start_year + era->offset - extra;
		else
			*year = end_year - cur_year - extra;
	}
	if (found)
		return (1);
	else
		return (0);
}



/*
 * FUNCTION: getnumber()
 *	     This function convert a integral numeric value i into
 *	     character string.
 *
 * PARAMETERS:
 *	     int i - a numeric itegral value.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     It returns the character string of the integral value.
 */

static char *
getnumber(int i, struct globals *global_data)
{
        char temp[BUFSIZE];
        char *s = buffer;
        char *t = temp;
        div_t result;
        int q;

        *t++ = '\0';
        if (i < 0) {
                i = -i;
                *t++ = '-';
        }
        q = i;
        while (q > 0) {
                result = div (q,10);
                *t++ = result.rem + '0';
                q = result.quot;
        }
        while (*--t)
                *s++ = *t;
	*s = '\0';
        return (buffer);
}
	

/*
 * FUNCTION: getnum()
 *	     This function convert a integral numeric value i into
 *	     character string with a fixed field.
 *
 * PARAMETERS:
 *	     int i - a numeric itegral value.
 *	     int n - output field width.
 *	     struct globals *global_data 
 *		   - contains all the info. needed from one function
 *		     to another (eg: alt_digits, localedigits, etc..
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     It returns the character string of the integral value.
 */
	
#define ALTDIGIT(i)  (localedigits + locoffset[i])

static char * 
getnum(int i, int n, struct globals *global_data)
{
	char *s = buffer;

	/* Do we want and have the alternate digit? */
	if (alt_digits && i >= 0 && i < digitscnt) {
		int len = locoffset[i+1] - locoffset[i] - 1;
		if (len) {
			buffer[len] = '\0';
			return(strncpy(buffer, ALTDIGIT(i), len));
			}
		}	/* if len==0 or i>=digitscnt, use regualar digits. */

	s += n ? n : 19;
	*s = 0;
	while(s>buffer) {
	    if (i==0 && n==0) break;
	    *--s = (i%10)+'0';
	    i /= 10;
	}
	return s;
}


/*
 * FUNCTION: gettimezone()
 *	     This function calls tzset() first and then retuns the 
 *	     name of the current time zone.
 *
 * PARAMETERS:
 *	     struct tm *timeptr - time structure.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     It returns the current time zone.
 */

static char * 
gettimezone(struct tm *timeptr) 
{
	tzset();
	if (daylight && timeptr->tm_isdst)
		return tzname[1];
	else
		return tzname[0];
}
