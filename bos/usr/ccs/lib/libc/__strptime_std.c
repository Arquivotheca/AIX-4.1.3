static char sccsid[] = "@(#)68	1.8.1.11  src/bos/usr/ccs/lib/libc/__strptime_std.c, libcfmt, bos41B, 9505A 1/25/95 13:55:11";
/*
 * COMPONENT_NAME: LIBCFMT
 *
 * FUNCTIONS:  __strptime_std __strcncasecmp
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _ILS_MACROS
#include <sys/localedef.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <langinfo.h>
#include <ctype.h>

#define MONTH 	12
#define DAY_MON	31
#define HOUR_24	23
#define HOUR_12 11
#define DAY_YR	366
#define MINUTE	59
#define SECOND	61
#define WEEK_YR 53
#define DAY_WK	6
#define YEAR_99	99
#define YEAR_1900 1900
#define YEAR_9999 9999

#define BUF_SIZE 1000	/* the buffer size of the working buffer */

#define SKIP_TO_NWHITE(s)	while (*s && (isspace(*s))) s++
#define GETSTR(f)	t=f; \
			while(*subera && *subera != ':') \
				*t++ = *subera++; \
			*t = '\0'

/*
 * NOTE: must keep track of begining of str so that if str is invalid, we can
 *	 detect it and return a -1.
 */
#define STRTONUM(str,num)   {   num = 0; \
				p = (char *)str; \
				while (isdigit (*str)) { \
					num *= 10; \
					num += *str++ - '0'; \
				} \
				if (p==str) num=-1;  \
			    } \

#define RETURN(x) 	 return(am=pm=0, (char *)x)

/* Examine __OBJ_DATA(hdl)->era, one era at a time.  Eras are seperated by a 
 * semicolon.  erabuf contains one era at a time.  GETERA gets the next era's
 * info. (dir, offset, start date, end date, name, and format.)
 */
#define GETERA \
	subera = erabuf;				\
	if((p=strchr(era_s,';'))!=NULL){		\
		strncpy(subera, era_s, p-era_s);	\
		subera[p-era_s] = '\0';			\
		era_s = p+1;				\
		}					\
	else {						\
		strcpy(subera, era_s);			\
		era_s += strlen(subera);		\
		}					\
	era->dir = *subera;		 subera += 2;	\
	STRTONUM(subera, era->offset);   subera++;	\
	GETSTR(era->st_date);	        		\
	if (!(*subera++)) 	RETURN(NULL); 		\
	GETSTR(era->end_date);  			\
	if (!(*subera++)) 	RETURN(NULL);		\
	GETSTR(era->name);				\
	if (!(*subera++)) 	RETURN(NULL);		\
	GETSTR(era->form);

/*
 *
 * dysize(A) -- calculates the number of days in a year.  The year must be
 *      the current year minus 1900 (i.e. 1990 - 1900 = 90, so 'A' should
 *      be 90).
 */
#define dysize(A) (((1900+(A)) % 4 == 0 && (1900+(A)) % 100 != 0 || (1900+(A)) % 400 == 0) ? 366:365)

struct era_struct {
	char	dir;		/* direction of the current era */
	int	offset;		/* offset of the current era */ 
	char	st_date[100];	/* start date of the current era */
	char	end_date[100];	/* end date of the current era */	
	char	name[100];	/* name of the current era */
	char	form[100];	/* format string of the current era */
};
typedef struct era_struct *era_ptr;

static char *doformat(_LC_time_objhdl_t hdl, const char *buf, const char *fmt, struct tm *tm, struct globals *global_data);
static int init_altdigits(_LC_time_objhdl_t hdl, const char *fmt, struct globals *gd);
static int conv_time (era_ptr era, int year, struct tm *tm, struct globals *global_data);
static int set_day_of_year (struct tm *tm, struct globals *global_data);
static int __strncasecmp(const char *, const char *, size_t);

/* This is a constant and is never modified, so it is thread safe. */
static int day_year[]={0,31,59,90,120,151,181,212,243,273,304,334,365};

/* For thread safety, all global data has been moved into this structure
 *  and will be passed on to those routines that need it.  The defines will
 *  be used to avoid changing the actual code.
 */
#define ALTDIGITS_MAX 101
struct globals {
	int set_yr;	/* logical flag to see if year is detected */
	int set_mon;	/* logical flag to see if month detected */
	int set_day;	/* logical flag to see if day is detected */
	int set_wkday;	/* logical flag to detect day of week */
	int set_wk;	/* logical flag to detect week of year */
	int wk_of_yr;	/* store the week number */
	char *localedigits; /* point to locale's alt_digits string */
	short locoffset[ALTDIGITS_MAX]; /* store offsets into localedigits[] */
	signed char digitscnt; /* number of actual alternate digit strings */
	int am;		/* logical flag to show if its AM */
	int pm;		/* logical flag to show if its PM */
	int set_hour;	/* logical flag for setting the hour of tm */
	int era_name;	/* logical flag for detected era name */
	int era_year;	/* logical flag for detected era year */
	int alt_digits;	/* if 1, use alternative numeric symbols */
	int alt_format;	/* if 1; use alternative numeric symbols */
	};

/*
 * FUNCTION: This the standard method for function strptime.
 *	     It parses the input buffer according to the format string. If
 *	     time related data are recgonized, updates the tm time structure
 * 	     accordingly. 
 *
 * PARAMETERS:
 *           _LC_time_objhdl_t hdl - pointer to the handle of the LC_TIME
 *			       catagory which contains all the time related
 *			       information of the specific locale.
 *	     const char *buf - the input data buffer to be parsed for any
 *			       time related information.
 *	     const char *fmt - the format string which specifies the expected
 *			       format to be input from the input buf.
 *	     struct tm *tm   - the time structure to be filled when appropriate
 *			       time related information is found.
 *			       The fields of tm structure are:
 *
 *			       int 	tm_sec		seconds [0,61]
 *			       int	tm_min		minutes [0,61]
 *			       int	tm_hour		hour [0,23]
 *			       int	tm_mday		day of month [1,31]
 *			       int	tm_mon		month of year [0,11]
 *			       int	tm_wday		day of week [0,6] Sun=0
 *			       int	tm_yday		day of year [0,365]
 *			       int 	tm_isdst	daylight saving flag
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
 *           - if successful, it returns the pointer to the character after
 *	       the last parsed character in the input buf string.
 *           - if fail for any reason, it returns a NULL pointer. 
 */

char *
__strptime_std(_LC_time_objhdl_t hdl, const char *buf, const char *fmt, struct tm *tm)
{
	struct globals gd = {
		0, 0, 0, 0, 0, 0, NULL, {0}, -1, 0, 0, 0, 0, 0, 0, 0
		};

	init_altdigits(hdl, fmt, &gd);
	return(doformat(hdl, buf, fmt, tm, &gd));
}

/*
 * FUNCTION: init_altdigits() checks to see if the format passed in to it
 *	     contains any format specifiers that begin with "%O".  If so,
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
init_altdigits(_LC_time_objhdl_t hdl, const char *fmt, struct globals *gd)
{
	char *p = (char *)fmt;
	/* Since the locale may be changed from one invocation to another,
	 *   setup the locale digits if there is a "%O" in the fmt string.
	 */
	if (p=strstr(p, "%O")) {
		gd->localedigits = __OBJ_DATA(hdl)->alt_digits;
		if (gd->localedigits != NULL && *(gd->localedigits)!='\0') {
			int i=1;      /* There's at least one alternate digit */
			p = gd->localedigits;

			do { if (*p==';') i++; } while (*p++);

			if (i>ALTDIGITS_MAX)
				i=ALTDIGITS_MAX; /* Only allow ALTDIGITS_MAX offsets */

			gd->digitscnt = i;  /* Number of alternate strings */

			gd->locoffset[0] = 0;
			p = gd->localedigits;
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
#define set_yr		(global_data->set_yr)
#define set_mon		(global_data->set_mon)
#define set_day		(global_data->set_day)
#define set_wkday	(global_data->set_wkday)
#define set_wk		(global_data->set_wk)
#define wk_of_yr	(global_data->wk_of_yr)
#define localedigits	(global_data->localedigits)
#define locoffset	(global_data->locoffset)
#define digitscnt	(global_data->digitscnt)
#define am		(global_data->am)
#define pm		(global_data->pm)
#define set_hour	(global_data->set_hour)
#define era_name	(global_data->era_name)
#define era_year	(global_data->era_year)
#define alt_digits	(global_data->alt_digits)
#define alt_format	(global_data->alt_format)

char *
doformat(_LC_time_objhdl_t hdl, const char *buf, const char *fmt, struct tm *tm, 
	struct globals *global_data)
{
	char	bufchr;		/* current char in buf string */
	char	fmtchr;		/* current char in fmt string */
	int	found;		/* boolean flag for a match of buf and fmt */
	int	width;		/* the field width of an locale entry */ 
	int 	lwidth;		/* the field width of an locale entry */
	char 	*era_s;		/* locale's emperor/era string */
	char	*subera;	/* era string of a multiple era */
	char	*f;		/* era format of subera */
	char	erabuf[BUF_SIZE];/* a work buffer for the era string */
	struct era_struct eras; /* a structure for current era */
	era_ptr era = &eras;	/* pointer to the current era struct */

	int	year;		/* %o value, year in current era */
	char	*t,*p;		/* a temp string */
	int	i,j;

	SKIP_TO_NWHITE(fmt);
	while ((fmtchr = *fmt++) && (bufchr = *buf)) {
						/* stops when buf or fmt ends */
		if (fmtchr != '%') {		/* ordinary character */
			SKIP_TO_NWHITE(buf);
			bufchr = *buf;
			if (bufchr == fmtchr) {
				buf++;
				SKIP_TO_NWHITE(fmt);
				continue;	/* ordinary char, skip */
			}
			else
				RETURN(NULL); 	/* error, ordinary char in fmt 
						   unmatch char in buf */
		}
		else {
			switch (fmtchr = *fmt++) {
			case 'a':
			case 'A': 
			/* locale's full or abbreviate weekday name */
				SKIP_TO_NWHITE(buf);
				found = 0;
				for (i=0; i < 7 && !found; i++) {
					width = strlen(__OBJ_DATA(hdl)->abday[i]);
					lwidth = strlen(__OBJ_DATA(hdl)->day[i]);
					if (!__strncasecmp(buf,__OBJ_DATA(hdl)->day[i],
						     lwidth)){
						found = 1;
						buf += lwidth;
					}
					else
					if (!__strncasecmp(buf,
						     __OBJ_DATA(hdl)->abday[i],
						     width)){
						found = 1;
						buf += width;
					}
				}
				if (found)
					tm->tm_wday = i-1;
				else
					RETURN(NULL);
				break;

                        case 'b': 
			case 'B': 
			case 'h':
			/* locale's full or abbreviate month name */
				SKIP_TO_NWHITE(buf);
                                found = 0;
                                for (i=0; i < 12 && !found; i++) {
                                        width = strlen(__OBJ_DATA(hdl)->abmon[i]);
                                        lwidth = strlen(__OBJ_DATA(hdl)->mon[i]);
                                        if (!__strncasecmp(buf,
						     __OBJ_DATA(hdl)->mon[i],
						     lwidth)){
                                                found = 1;
                                                buf += lwidth;
                                        }
                                        else
                                        if (!__strncasecmp(buf,
					             __OBJ_DATA(hdl)->abmon[i],
						     width)){
                                                found = 1;
                                                buf += width;
                                        }
                                }
                                if (found) {
                                        tm->tm_mon = i-1;
					set_mon = 1;
					(void)set_day_of_year(tm, global_data);
				}
                                else
                                        RETURN(NULL);
				break;

			case 'c': 		/* locale's date and time */
				SKIP_TO_NWHITE(buf);
				if (alt_format) {
					alt_format=0;
					if (localedigits == NULL)
						init_altdigits(hdl, __OBJ_DATA(hdl)->d_t_fmt, global_data);
					if ((buf=doformat(hdl, buf, __OBJ_DATA(hdl)->d_t_fmt, tm, global_data)) 
					     == NULL)
						RETURN(NULL);
					alt_format=1;
					}
				else {
					if (localedigits == NULL)
						init_altdigits(hdl, __OBJ_DATA(hdl)->d_t_fmt, global_data);
					if ((buf=doformat(hdl, buf, __OBJ_DATA(hdl)->d_t_fmt, tm, global_data)) 
					     == NULL)
						RETURN(NULL);
					}
				break;

			case 'C':     /* century 0-99 */
				SKIP_TO_NWHITE(buf);
				STRTONUM(buf,i);
                                if (i >= 0 && i <= 99)
                                        ;
                                else
                                        RETURN(NULL);
				break;

			case 'd':		/* day of month, 1-31 */
			case 'e':
				SKIP_TO_NWHITE(buf);
				i = altstrtonum(&buf, global_data);
				if (i > 0 && i <= DAY_MON) {
					tm->tm_mday = i;
					set_day = 1;
					(void)set_day_of_year(tm, global_data);
				}
				else
					RETURN(NULL);
				break;

			case 'D':		/* %m/%d/%y */
				SKIP_TO_NWHITE(buf);
				if ((buf = doformat(hdl,buf, "%m/%d/%y", tm, global_data)) == NULL)
					RETURN(NULL);
				break;

			case 'E': /* X/open, locale's alternative date & time */
			      { int alt=1;      /* assume alternative format */

				SKIP_TO_NWHITE(buf);
				switch(*fmt) {
				case 'c':
				case 'x':
				case 'X':
				      { /* Format is like "%Od". */
					/*  Just remove the 'E'  */
					char tmpf[3]={'%', *fmt, '\0'};

					alt_format=1;
					buf = doformat(hdl, buf, tmpf, tm, global_data);
					alt_format=0;
					if (buf==NULL)
						RETURN(NULL);
				      } /* release local variables */
				    break;
				case 'y':
				    STRTONUM(buf,year);
				    if (year >= 0) {
					era_year = 1;
					if (era_name) {
						era_year = 0;
						era_name = 0;
						if (!conv_time (era,year,tm,global_data))
							RETURN(NULL);
					}
				    }
				    break;
				case 'C':
				    if (*__OBJ_DATA(hdl)->era) {
					era_s = __OBJ_DATA(hdl)->era;
					era_name = 0;
					while (*era_s && !era_name) {
						subera = erabuf;
						while (*era_s && *era_s != ';')
							*subera++ = *era_s++;
						*subera = '\0';
						subera = erabuf;
						if (*era_s)
							era_s++;
						era->dir = *subera++;
						subera++;
						STRTONUM(subera,era->offset);
						subera++;
						GETSTR (era->st_date);
						if (!(*subera++))
							RETURN(NULL);
						GETSTR (era->end_date);
						if (!(*subera++))
							RETURN(NULL);
						GETSTR (era->name);
						if (!(*subera++))
							RETURN(NULL);
						GETSTR (era->form);
						i = strlen(era->name);
						if (!__strncasecmp(buf,era->name,i)){
							buf += i;
							era_name = 1;
							}
						} /* while */
					}	  /* if */
				    else
					RETURN(NULL); 
				    if (era_name) {
					if (era_year) {
					    era_name = 0;
					    era_year = 0;
					    if(!conv_time (era,year,tm,global_data))
						RETURN(NULL);
						}
				            }
					else
					    RETURN(NULL);

				    break;
				case 'Y':
					if (*__OBJ_DATA(hdl)->era) {
					    found = 1;
					    era_s = __OBJ_DATA(hdl)->era;
					    found = 0;
					    while (*era_s && !found) {
						/* offset/name/format of era */
						GETERA;
						if (matchformat(era->form, buf, era->name)) {
						    alt_format = 1;
						    if ((buf=doformat(hdl, buf, era->form,tm, global_data))!=NULL)
							found = 1;
						    else {
							alt_format = 0;
							RETURN(NULL);
							}
						    alt_format = 0;
						    }
						}  /* end of while loop */
					} /* if current locale has eras */
					break;
				}   /* end of switch */
				fmt++;
			      }   /* release local variables */

			      break;

			case 'H':		/* hour 0-23 */
				am = pm = 0; /* in case of %p */
				SKIP_TO_NWHITE (buf);
				i = altstrtonum(&buf, global_data);
				if (i >= 0 && i <= HOUR_24) {
					tm->tm_hour = i;
				}
				else
					RETURN(NULL);
				break;

			case 'I':		/* hour 1-12 */
				SKIP_TO_NWHITE (buf);
				i = altstrtonum(&buf, global_data);
                                if (i > 0 && i <= HOUR_12 + 1) {
					if (am) {
						if (i == 12 )
							i=0;
					}
					else if (pm) {
						if ( i != 12 )
							i += 12;
					}
					else
						set_hour = 1;

                                        tm->tm_hour = i;
				}
                                else
                                        RETURN(NULL);
				break;

                        case 'j':		/* day of year, 1-366 */
				SKIP_TO_NWHITE(buf);
				STRTONUM(buf,i);
                                if (i > 0 && i <= DAY_YR)
                                        tm->tm_yday = i - 1;
                                else
                                        RETURN(NULL);
				break;

                        case 'm':		/* month of year, 1-12 */
				SKIP_TO_NWHITE(buf);
				i = altstrtonum(&buf, global_data);
                                if (i > 0 && i <= MONTH) {
                                        tm->tm_mon = i-1;
					set_mon = 1;
					(void)set_day_of_year(tm, global_data);
				}
                                else
                                        RETURN(NULL);
				break;

			case 'M':		/* minute 0-59 */
				SKIP_TO_NWHITE(buf);
				i = altstrtonum(&buf, global_data);
                                if (i >= 0 && i <= MINUTE)
                                        tm->tm_min = i;
                                else
                                        RETURN(NULL);
				break;

			case 'N':
				SKIP_TO_NWHITE(buf);
                                if (*__OBJ_DATA(hdl)->era) {
                                        era_s = __OBJ_DATA(hdl)->era;
        	                        era_name = 0;
                                        while (*era_s && !era_name) {
                                                subera = erabuf;
                                                while (*era_s && *era_s != ';')
                                                        *subera++ = *era_s++;
                                                *subera = '\0';
                                                subera = erabuf;
                                                if (*era_s)
                                                        era_s++;
						era->dir = *subera++;
						subera++;
						STRTONUM(subera,era->offset);
						subera++;
						GETSTR (era->st_date);
						if (!(*subera++))
							RETURN(NULL);
					 	GETSTR (era->end_date);
						if (!(*subera++))
							RETURN(NULL);
						GETSTR (era->name);
						if (!(*subera++))
							RETURN(NULL);
						GETSTR (era->form);
						i = strlen(era->name);
						if (!__strncasecmp(buf,era->name,i)){
							buf += i;
							era_name = 1;
						}
					} /* while */
				}	  /* if */
				else
					RETURN(NULL); 
				if (era_name) {
					if (era_year) {
						era_name = 0;
						era_year = 0;
						if(!conv_time (era,year,tm,global_data))
							RETURN(NULL);
					}
				}
				else
					RETURN(NULL);
				break;

			case 'n':		/* new line character */
				SKIP_TO_NWHITE(buf);
				break;

			case 'o':		/* year of era */
				SKIP_TO_NWHITE(buf);
				STRTONUM(buf,year);
				if (year >= 0) {
					era_year = 1;
					if (era_name) {
						era_year = 0;
						era_name = 0;
						if (!conv_time (era,year,tm,global_data))
							RETURN(NULL);
					}
				}
				break;


			case 'O':		/* use alternative numeric symbols */
			      { /* Format is like "%Od". Just remove the 'O' */
				char tmpf[3]={'%', *fmt, '\0'};

				alt_digits=1;
				buf = doformat(hdl, buf, tmpf, tm, global_data);
				alt_digits=0;
				if (buf==NULL)
					RETURN(NULL);
				fmt++;
				}

				break;

			case 'p':		/* locale's AM or PM */
				SKIP_TO_NWHITE(buf);
				width = strlen(__OBJ_DATA(hdl)->am_pm[0]);
				lwidth = strlen(__OBJ_DATA(hdl)->am_pm[1]);
				if (!__strncasecmp(buf, __OBJ_DATA(hdl)->am_pm[0],width)) {
					am = 1;
					if (set_hour) {
						if ( tm->tm_hour == 12 )
							tm->tm_hour = 0;
						set_hour = 0;
					}
					buf += width;
				}
				else if (!__strncasecmp(buf,__OBJ_DATA(hdl)->am_pm[1],lwidth)) {
					pm = 1;
					if (set_hour) {
						if (tm->tm_hour != 12) {
							tm->tm_hour += 12;
							set_hour = 0;
						}
					}
					buf += lwidth;
				}
				else
					RETURN(NULL);
				break;

			case 'r':		/* %I:%M:%S [AM|PM] */
				SKIP_TO_NWHITE(buf);
				if (localedigits == NULL)
					init_altdigits(hdl, __OBJ_DATA(hdl)->t_fmt_ampm, global_data);
                                if ((buf = doformat(hdl,buf, __OBJ_DATA(hdl)->t_fmt_ampm, tm, global_data)) == NULL)
                                        RETURN(NULL);
				break;

			case 'R':		/* %H:%M */
				SKIP_TO_NWHITE(buf);
                                if ((buf = doformat(hdl,buf, "%H:%M ", tm, global_data)) == NULL)
                                        RETURN(NULL);
				break;

                        case 'S':               /* second 0-61 */
				SKIP_TO_NWHITE(buf);
				i = altstrtonum(&buf, global_data);
                                if (i >= 0 && i <= SECOND)
                                        tm->tm_sec = i;
                                else
                                        RETURN(NULL);
				break;

                        case 't':               /* tab character */
				SKIP_TO_NWHITE(buf);
				break;

			case 'T':		/* %H:%M:%S */
				SKIP_TO_NWHITE(buf);
                                if ((buf = doformat(hdl, buf, "%H:%M:%S", tm, global_data)) == NULL)
                                        RETURN(NULL);
				break;

                        case 'U': 
			case 'W':     /* week of year, 0-53 */
				SKIP_TO_NWHITE(buf);
				i = altstrtonum(&buf, global_data);
                                if (i >= 0 && i <= WEEK_YR) {
					set_wk = (fmtchr=='U')? 1:2;
					wk_of_yr = i;
					if (set_day_of_year(tm, global_data)==0)
						RETURN(NULL);
					}
                                else
                                        RETURN(NULL);
				break;

                        case 'w':               /* day of week, 0-6 */
				SKIP_TO_NWHITE(buf);
				i = altstrtonum(&buf, global_data);
                                if (i >= 0 && i <= DAY_WK) {
                                        tm->tm_wday = i;
					set_wkday=1;
					if (set_day_of_year(tm, global_data)==0)
						RETURN(NULL);
					}
                                else
                                        RETURN(NULL);
				break;

			case 'x':		/* locale's date format */
				SKIP_TO_NWHITE(buf);
				if (alt_format) {
					alt_format=0;
					if (localedigits == NULL)
						init_altdigits(hdl, __OBJ_DATA(hdl)->d_fmt, global_data);
					if ((buf = doformat(hdl,buf, __OBJ_DATA(hdl)->d_fmt, tm, global_data))
					     == NULL)
						RETURN(NULL);
					alt_format=1;
					}
				else {
					if (localedigits == NULL)
						init_altdigits(hdl, __OBJ_DATA(hdl)->d_fmt, global_data);
					if ((buf = doformat(hdl,buf, __OBJ_DATA(hdl)->d_fmt, tm, global_data)) 
					     == NULL)
						RETURN(NULL);
					}
				break;

			case 'X':		/* locale's time format */
				SKIP_TO_NWHITE(buf);
				if (alt_format) {
					alt_format=0;
					if (localedigits == NULL)
						init_altdigits(hdl, __OBJ_DATA(hdl)->t_fmt, global_data);
					if ((buf = doformat(hdl, buf, __OBJ_DATA(hdl)->t_fmt, tm, global_data)) 
					     == NULL)
						RETURN(NULL);
					alt_format=1;
					}
				else {
					if (localedigits == NULL)
						init_altdigits(hdl, __OBJ_DATA(hdl)->t_fmt, global_data);
					if ((buf = doformat(hdl, buf, __OBJ_DATA(hdl)->t_fmt, tm, global_data)) 
					     == NULL) 
						RETURN(NULL);
					}
				break;

                        case 'y':               /* year of century, 0-99 */
				SKIP_TO_NWHITE(buf);
				i = altstrtonum(&buf, global_data);
                                if (i >= 0 && i <= YEAR_99) {
					tm->tm_year = i;
					set_yr = 1;
					if (set_day_of_year(tm, global_data)==0)
						RETURN(NULL);
				}
                                else
                                        RETURN(NULL);
				break;

                        case 'Y':               /* year with century, dddd */
				SKIP_TO_NWHITE(buf);
				STRTONUM(buf,i);
                                if (i >= 0 && i <= YEAR_9999) {
					tm->tm_year = i-YEAR_1900;
					set_yr = 1;
					if (set_day_of_year (tm, global_data)==0)
						RETURN(NULL);
				}
                                else
                                        RETURN(NULL);
				break;
	
			case 'Z':		/* time zone name */
				SKIP_TO_NWHITE(buf);
				tzset();
				width = strlen(tzname[0]);
				lwidth = strlen(tzname[1]);
				if (!__strncasecmp(buf,tzname[0],width)) {
					tm->tm_isdst = 0;
					buf += width;
				}
				else if (!__strncasecmp(buf,tzname[1],lwidth)) {
					tm->tm_isdst = 1;
					buf += lwidth;
				}
				else
					RETURN(NULL);
				break;

			case '%' :		/* double % character */
				SKIP_TO_NWHITE(buf);
				bufchr = *buf;
				if (bufchr == '%')
					buf++;
				else
					RETURN(NULL);
				break;

			default:
				RETURN(NULL);
			} /* switch */
		} /* else */
		SKIP_TO_NWHITE(fmt);
	} /* while */
	if (fmtchr)
		RETURN(NULL); 		/* buf string ends before fmt string */
	else 
		RETURN((char *)buf);	/* successful completion */
}

/*
 * FUNCTION: altstrtonum(char **str, struct globals *global_data)
 *	     Convert the string given by *str into an int.  If alt_digits
 *	     TRYALTDIGITS are both true, then string may be in the form of
 *	     an alternate digit (e.g. "zero", "three", "ninety-nine"...)
 *	     Otherwise, it is a simple ascii to int conversion.
 *	     Note that the string pointer is updated as a side effect of this
 *	     subroutine as is in the STRTONUM macro.
 *
 * PARAMETERS:
 *	     char **str -- Pointer to the string pointer.
 *	     struct globals *global_data 
 *		   - contains all the info. needed from one function
 *		     to another (eg: alt_digits, localedigits, etc..
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - returns the converted value.
 *           - returns -1 if it fails.
 */

#define ALTDIGIT(i)  (localedigits + locoffset[i])

int
altstrtonum(char **str, struct globals *global_data)
{
	char *p;	/* used by the STRTONUM macro. */
	int num=0;
	size_t len;
	/* Do we want and have the alternate digits? */
	if (alt_digits && digitscnt > 0)	{
		for (; num<digitscnt; num++) { 
			len = locoffset[num+1] - locoffset[num] - 1;
			if (!len)
				continue;	/* skip */
			if (__strncasecmp(*str, ALTDIGIT(num), len) == 0)
				break; 		/* found it */
			}
		if (num < digitscnt) {
			*str += len;
			return(num);
			}
		} 
	/* Try the normal behaviour then. */ 
	STRTONUM((*str), num)  
	return(num);
}

/*
 * FUNCTION: conv_time (era_ptr era, int year, struct tm *tm)
 *	     By supplying the current era and year of the era, the fuction
 *	     converts this era/year combination into Christian ear and 
 *	     set the tm->tm_year field of the tm time structure.
 *
 * PARAMETERS:
 *           era_ptr era - the era structure provides the related information
 *			   of the current era.
 *           int year - year of the specific era.
 *           struct tm *tm - a pointer to the time structure where the     
 *			     tm->tm_year field will be set.
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - returns 1 if the conversion is valid and successful. 
 *           - returns 0 if fails.
 */

static int 
conv_time (era_ptr era, int year, struct tm *tm, struct globals *global_data)
{
	char *str, *p;
	int start_year = 0;
	int end_year = 0;
	int no_limit = 0;
	int i;

	str = era->st_date;
	if (*str == '-') {
		str++;
		STRTONUM(str,start_year);
		start_year = -start_year;
	}
	else
		STRTONUM(str,start_year);

	str = era->end_date;
	if ((*str=='+' && *(str+1)=='*' ) || (*str=='-' && *(str+1)=='*'))
		no_limit = 1;
	else if (*str == '-') {
		str++;
		STRTONUM(str,end_year);
		end_year = -end_year;
	}
	else
		STRTONUM(str,end_year);

	if (era->dir == '+') {
		i = year - era->offset + start_year;
		if (no_limit){
			tm->tm_year = i - YEAR_1900;
			set_yr = 1;
			if (set_day_of_year (tm, global_data)==0)
				return(0);
			return (1);
		}
		else if (i <= end_year) {
			tm->tm_year = i - YEAR_1900;
			set_yr = 1;
			if (set_day_of_year (tm, global_data)==0)
				return(0);
			return (1);
		}
		else 
			return (0);
	}
	else
		if ((i = end_year - year) <= start_year) {
			tm->tm_year = i - YEAR_1900;
			set_yr = 1;
			if (set_day_of_year (tm, global_data)==0)
				return(0);
			return (1);
		}
		else
			return (0);
}


/*
 * FUNCTION: set_day_of_year (struct tm *tm, struct globals* global_data)
 *	If the month, day, and year have been determine. It should be able
 * 	to calculate the day-of-year field tm->tm_yday of the tm structure.
 *	It calculates if its leap year by calling the dysize() which 
 *	returns 366 for leap year.
 *
 * PARAMETERS:
 *           struct tm *tm - a pointer to the time structure where the     
 *			     tm->tm_yday field will be set.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	void.
 */

static int 
set_day_of_year (struct tm *tm, struct globals *global_data)
{
    struct tm tmp_tm_ptr;
    time_t    t_secs;
    int i, j;
    int leapyear;

    leapyear = dysize(tm->tm_year)==366;
    if (set_day && set_mon && set_yr) {
	if (tm->tm_mon >= 2 && leapyear) 
		tm->tm_yday = day_year[tm->tm_mon] + tm->tm_mday;
	else 
		tm->tm_yday = day_year[tm->tm_mon] + tm->tm_mday - 1;
	set_day = set_mon = set_yr = 0;
    }
    else if (set_yr && set_wk && set_wkday) {
	tmp_tm_ptr.tm_sec   = 0;  tmp_tm_ptr.tm_min = 0;    tmp_tm_ptr.tm_hour = 0;
	tmp_tm_ptr.tm_mday  = 1;  tmp_tm_ptr.tm_mon = 0;    tmp_tm_ptr.tm_year = tm->tm_year;
	tmp_tm_ptr.tm_isdst = 0;   	/* begin year: day light savings time */
	
        t_secs = mktime(&tmp_tm_ptr);	/* get tmp_tm_ptr.tm_wday = week day of 1st day of */
	i = tm->tm_wday;		/*   year with 0 = sunday  			   */

	if (set_wk==2) {		/* '%W' == week starts on mon, so mon=0, tue=1,    */
	   i = (tm->tm_wday==0)? 6:tm->tm_wday-1;  		/* etc., and sun=6 	   */
	   tmp_tm_ptr.tm_wday = (tmp_tm_ptr.tm_wday==0)? 6:tmp_tm_ptr.tm_wday-1;
	   }
		
	set_wk = set_wkday = set_yr = 0;

	j = (wk_of_yr)*7 + i - tmp_tm_ptr.tm_wday;	/* number of days in given # of weeks */

	if(j < 0 || j > 364 + leapyear)
		return(0);			/* out of range */
	tm->tm_yday = j;
				 /* day_year:{0,31,59,90,120,151,181,212,243,273,304,334,365} */
	for(i=0; i<11 && j > day_year[i+1]; i++);	/* find approximate month */
	if (j==day_year[i+1] && (i==0 || !leapyear))	/* find exact month */
		i++;
		
	if (i<12) {			 	/* if valid month then */
		tm->tm_mon = i;		 	/*    get day of month which is 0-based so */
		i = (j<60 || !leapyear);    	/* if before feb 29 or not leap year, add 1 */
		tm->tm_mday = j-day_year[tm->tm_mon]+i;
		}
	}
	return(1);
}

/* 
 * FUNCTION: matchformat (char *fmt, char *inb, char *eraname)
 *	Check if the input buffer can match the the format of an era.
 *
 * PARAMETERS:
 *	char *fmt - format of an era.
 *	char *inb - input buffer 
 *	char *eraname - name of era
 *
 * RETURN VALUES:
 *	0 - input doesn't match given era format
 *	x - match found.  number of bytes matched is returned
 */

static int
matchformat(char *fmt, char *inb, char *eraname)
{	int i=0, cnt=0;

	while(*inb && *fmt) {
		while(*fmt && *inb && *fmt == *inb) {	/* plain text match */
		    fmt++;
		    inb++;
		    cnt++;
		}
		if (*fmt=='\0') return(cnt);              /* buffer matched format */
		if (*fmt != '%' || ((*(fmt+1)!='N'&&*(fmt+1)!='o') && (*(fmt+1)!='E' || *(fmt+2)!='C' && *(fmt+2)!='y')))
			return (0);  /* allow only: "%EC, %Ey, %N, %o" */
		fmt++;	/* skip the % sign */
		if (*fmt == 'E')
			fmt++;	/* skip the E */
		if (*fmt=='N' || *fmt=='C') {
			fmt++;
			i = strlen(eraname);
			if (!__strncasecmp(eraname, inb, i)) {
				inb += i;
				cnt += i;
				continue;
				}
			else
				return (0);	/* don't match */
		    }
		else 
			fmt++;
			while(isdigit(*inb)) {
			    inb++;
			    cnt++;
			}
		}
	return (cnt);
}


/**********
  This function only determines equality.  If the strings are not equal, a -1 is
  returned
**********/
static
int
__strncasecmp(const char *s1, const char *s2, size_t n)
{
	int	i;		/* loop counter */
	int	len1;		/* Length of converted multi-byte character for s1 */
	int	len2;		/* Length of converted multi-byte character for s2 */
	char	*base_s1=(char *)s1;	/* save pointer for s1 */
	char	*base_s2=(char *)s2;	/* save pointer for s1 */
	wchar_t	wc1;		/* process code for s1  */
	wchar_t	wc2;		/* process code for s2  */

	/**********
	  if MB_CUR_MAX is 1, then we can just loop thru the 
	  string, and compare each value, 
	**********/

	if ( MB_CUR_MAX == 1 ) {
		for (i=0; i<n; i++) {
			/**********
			  s1 is not equal to s2
			**********/
			if (tolower(s1[i]) != tolower(s2[i]))
				return(-1);
		}
		/**********
		  s1 is equal to s2
		**********/
		return(0);
	}

	/**********
	  Multibyte Case
		Only convert n bytes to multibyte characters
	**********/
	while(1) {
		/**********
		  convert each character to process code
		  if either on fails, return -1
		**********/
		if ((len1 = mbtowc(&wc1, s1, MB_CUR_MAX)) == -1)
			return(-1);
		if ((len2 = mbtowc(&wc2, s2, MB_CUR_MAX)) == -1)
			return(-1);

		/**********
		  if both of the strings have exceeded the length, then the 
		  last thing we knew the were equal, return equal
		**********/
		s1 += len1;
		s2 += len2;
		if ( ((s1 - base_s1) >= n) && ((s2 - base_s2) >= n) )
			return(0);

		/**********
		  check if the lower for the process codes match, if they don't
		  return not equal
		**********/
		if ( towlower(wc1) != towlower(wc2) )
			return(-1);

		/**********
		  if both are null, then return a match
		  if only 1 is null, return nomatch
		**********/
		if (*s1 == NULL && *s2 == NULL)
			return(0);
		if (*s1 == NULL || *s2 == NULL)
			return(-1);
	}
	/**********
	  If we are here, they must match
	**********/
	return(0);
}
