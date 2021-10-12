static char sccsid[] = "@(#)40	1.17  src/bos/usr/lib/sendmail/util.c, cmdsend, bos41J, 9510A_all 2/24/95 10:47:52";
/* 
 * COMPONENT_NAME: CMDSEND util.c
 * 
 * FUNCTIONS: CAP, COPY1C, ISPG1L, ISPG2, MSGSTR, UNCAP, atobool, 
 *            bitintersect, bitzerop, buildfname, cap, capitalize, 
 *            cat, copyplist, dfopen, dfork, fgetfolded, fixcrlf, 
 *            makelower, printav, printav1, putline, qstrlen, quote, 
 *            quotestr, readtimeout, recap, safefile, sameword, 
 *            sfgets, stripquotes, uncap, uncapstr, unquote, 
 *            unquotestr, waitfor, xalloc, xisalpha, xputs, xrmdir, 
 *            xunlink 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
**  Sendmail
**  Copyright (c) 1983  Eric P. Allman
**  Berkeley, California
**
**  Copyright (c) 1983 Regents of the University of California.
**  All rights reserved.  The Berkeley software License Agreement
**  specifies the terms and conditions for redistribution.
**
*/


#include <nl_types.h>
#include "sendmail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SENDMAIL,n,s) 

# include <memory.h>
# include <stdio.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/dir.h>
# include <sys/param.h>
# include "sysexits.h"
# include <errno.h>
# include <ctype.h>
# include <string.h>
# include <pwd.h>
# include <grp.h>

# include "conf.h"
# include "useful.h"
# include <netinet/in.h>
# include <setjmp.h>

# include "sendmail.h"

long  OutputCount;
long  InputCount;

void  exit ();
char *malloc();
EVENT *setevent ();
long  time ();
#define cur_time	time ((long *) 0)


static int recap (register char *, register char *, int );
static readtimeout();
static int writetimeout();

/*
 *  QUOTE - quote a quotable character.
 *
 *	The characters ! - /, and : - @ are quoted by mapping them 
 *	into unused graphics in the top of display page 0.  All noquotable
 *	characters are returned unchanged.
 *
 *	This is different from the original sendmail implementation where
 *	ANY character could be quoted by setting the high bit in the byte.
 *	This defeats the old kloodge which disabled use of .forward files when
 *	a char in the target user name is quoted.  It won't work now!
 *
 *	Parameter: The character to quote, if possible.
 *	Returns: The result.
 */
static char quotable[] = {       0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb9, 0xba, 
			   0xbb, 0xbc, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 
			    '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  
			    '8',  '9', 0xc5, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 
			   0xcd, };

int
quote (c)
register int  c;
{
    c &= 0xff;

    if (c >= '!' && c <= '@')
        c = quotable[c - '!'];

    return (c);
}

/*
 *  UNQUOTE - unquote a quoted character.
 *
 *	Unquote by reversing the transformation in the quote function.
 *	Nonquote chars are returned unchanged.
 *
 *	Parameter: The character to unquote.
 *	Returns: The result.
 */
static char unquotable[] = {         '!',  '"',  '#',  '$',  '%', 0xb5, 0xb6, 
			      0xb7, 0xb8,  '&', '\'',  '(',  ')', 0xbd, 0xbe, 
			       '*',  '+',  ',',  '-',  '.',  '/',  ':', 0xc6, 
			      0xc7,  ';',  '<',  '=',  '>',  '?',  '@', };

int
unquote (c, pre)
register int  c, pre;
{
    c   &= 0xff;
    pre &= 0xff;

    if (!ISSSI (pre) && c >= 0xb0 && c <= 0xcd)
        c = unquotable[c - 0xb0];

    return (c);
}
/*
 *  Definitions to aid detection of (un)quoted SSi page shift bytes
 */
#define ISPG2(I) (I <= 0x1d && I >= 0x1c)
#define ISPG1L(I) (I == 0x1f)
#define COPY1C(D,S,I) { I = *S++; *D++ = I; if (ISSSI (I) && *S != '\0') *D++ = *S++; }
#define UNCAP(D,S,I) { I = uncap (D++, S++); if (I & 1) D++; if (I & 2) S++; }
#define CAP(D,S,I)   { I =   cap (D++, S++); if (I & 1) D++; if (I & 2) S++; }

/*
 *  Tables for multilanguage character utilities.
 */
#define C	0x10000			/* capital letter */
#define A	0x20000			/* letter */

/*
 *  Upper half of display page 0.
 *
 *  The numeric part is the case translation, if applicable.  In one case,
 *  (y umlaut,lower to upper case) the translation goes from a one to a two 
 *  byte string as indicated.  The first byte (0x1e) is the SSi shift 
 *  code required.
 */
static int  captable0[] = 
    {C|A|0x87,  A|0x9a,  A|0x90,  A|0xb6,  A|0x8e,  A|0xb7,  A|0x8f,  A|0x80, 
       A|0xd2,  A|0xd3,  A|0xd4,  A|0xd8,  A|0xd7,  A|0xde,C|A|0x84,C|A|0x86, 

     C|A|0x90,  A|0x92,C|A|0x91,  A|0xe2,  A|0x99,  A|0xe3,  A|0xea,  A|0xeb, 
     A|0x1eeb,C|A|0x94,C|A|0x81,  A|0x9d,    0x9c,C|A|0x9b,    0x9e,    0x9f, 

       A|0xb5,  A|0xd6,  A|0xe0,  A|0xe9,  A|0xa5,C|A|0xa4,    0xa6,    0xa7,
         0xa8,    0xa9,    0xaa,    0xab,    0xac,    0xad,    0xae,    0xaf,

         0xb0,    0xb1,    0xb2,    0xb3,    0xb4,C|A|0xa0,C|A|0x83,C|A|0xb7,
         0xb8,    0xb9,    0xba,    0xbb,    0xbc,    0xbd,    0xbe,    0xbf,

         0xc0,    0xc1,    0xc2,    0xc3,    0xc4,    0xc5,  A|0xc7,C|A|0xc6,
         0xc8,    0xc9,    0xca,    0xcb,    0xcc,    0xcd,    0xce,    0xcf,

       A|0xd1,C|A|0xd0,C|A|0x88,C|A|0x89,C|A|0x8a,  A|0x49,C|A|0xa1,C|A|0x8c,
     C|A|0x8b,    0xd9,    0xda,    0xdb,    0xdc,    0xdd,C|A|0x8d,    0xdf,

     C|A|0xa2,  A|0xe1,C|A|0x93,C|A|0x95,  A|0xe5,C|A|0xe4,    0xe6,  A|0xe8,
     C|A|0xe7,C|A|0xa3,C|A|0x96,C|A|0x97,  A|0xed,C|A|0xec };

/*
 *  Relevant parts of display page 1.
 *
 *  The two-byte numeric part is the case translation, if applicable.  The first
 *  byte of it is the SSi shift code (except value 0, which means no shift 
 *  code).
 */
static int  captable1[] = 
	{  A|0x1fc5,  A|0x1fc1,C|A|0x0083,C|A|0x0085,C|A|0x00a0,C|A|0x1fc0,
	   A|0x1fce,C|A|0x0088,C|A|0x0089,C|A|0x008a,C|A|0x00a1,C|A|0x008c,
	 C|A|0x008b,C|A|0x008d,C|A|0x1fc6,  A|0x1fd4,  A|0x1fd5,  A|0x1fd6,
	     0x1fd2,    0x1fd3,C|A|0x1fcf,C|A|0x1fd0,C|A|0x1fd1,    0x1fd7,

	     0x1fd8,    0x1fd9,    0x1fda,    0x1fdb,    0x1fdc,  A|0x1fe2,
	   A|0x0049,C|A|0x0093,C|A|0x0095,C|A|0x0092,C|A|0x1fdd,    0x1fe3,
	 C|A|0x0096,C|A|0x0097,C|A|0x00a3,  A|0x1fef,  A|0x1ff0,  A|0x1ff1,
	   A|0x1ff2,  A|0x1ff4,  A|0x1ff5,  A|0x1ff6,  A|0x1ff7,C|A|0x1fe7,

	 C|A|0x1fe8,C|A|0x1fe9,C|A|0x1fea,    0x1ff3,C|A|0x1feb,C|A|0x1fec,
	 C|A|0x1fed,C|A|0x1fee,  A|0x1e81,  A|0x1e82,  A|0x1ffa,  A|0x1e83,
	   A|0x1e84,    0x1ffd,  A|0x1e8d,  A|0x1e8e,  A|0x1e8f,C|A|0x1ff8,
	 C|A|0x1ff9,C|A|0x1ffb,C|A|0x1ffc,    0x1e85,  A|0x1e88,  0x1e87,

	 C|A|0x1e86,  A|0x1e8b,  A|0x1e8c,C|A|0x1e89,C|A|0x1e8a,C|A|0x1ffe,
	 C|A|0x1fff,C|A|0x1e80,  A|0x1e94,  A|0x1e95,  A|0x1e96,  A|0x1e97,
	 C|A|0x1e90,C|A|0x1e91,C|A|0x1e92,C|A|0x1e93,  A|0x1e9b,  A|0x1e9c,
	 C|A|0x1e9a,C|A|0x1e98,C|A|0x1e99,    0x1e9d,    0x1e9e,  A|0x1ea2,

	     0x1ea0,    0x1ea1,C|A|0x1e9f,    0x1ea3,  A|0x1ea5,C|A|0x1ea4,
	   A|0x1ea7,C|A|0x1ea6,  A|0x1ea9,C|A|0x1ea8,    0x1eaa,  A|0x1eac,
	 C|A|0x1eab,  A|0x1eae,C|A|0x1ead,  A|0x1eb0,C|A|0x1eaf,  A|0x1eb1,
	   A|0x1eb3,C|A|0x1eb2,  A|0x1eb5,C|A|0x1eb4,C|A|0x1eb6,  A|0x1eb8,

	 C|A|0x1eb7,  A|0x1eba,C|A|0x1eb9,  A|0x1ebc,C|A|0x1ebb,  A|0x1ebe,
	 C|A|0x1ebd,  A|0x1ec0,C|A|0x1ebf,  A|0x1ec2,C|A|0x1ec1,  A|0x1ec4,
	 C|A|0x1ec3,  A|0x1ec6,C|A|0x1ec5,  A|0x004b,  A|0x1ec9,C|A|0x1ec8,
	   A|0x1ecb,C|A|0x1eca,  A|0x1ecd,C|A|0x1ecc,  A|0x1ecf,C|A|0x1ece,

	   A|0x1ed1,C|A|0x1ed0,  A|0x1ed3,C|A|0x1ed2,  A|0x1ed5,C|A|0x1ed4,
	   A|0x1ed7,C|A|0x1ed6,  A|0x1ed9,C|A|0x1ed8,  A|0x1edb,C|A|0x1eda,
	   A|0x1edd,C|A|0x1edc,  A|0x1edf,C|A|0x1ede,  A|0x1ee1,C|A|0x1ee0,
	   A|0x1ee3,C|A|0x1ee2,  A|0x1ee5,C|A|0x1ee4,C|A|0x0098 };
/*
 *  XISALPHA (P) - return TRUE if char pointed to is alphabetic in the
 *		   multilanguage character set.
 */
int
xisalpha (p)
char  *p;
{
	int  point, page;
	int  d;

	point = *p;				/* get first byte */

	if (!ISSSI (point))			/* not SSi for another page */
	{
	    /*
	     *  Real ASCII, lower page 0.
	     */
	    if (point < 0x80)
		return (isalpha (point));

	    /*
	     *  Upper page 0.
	     */
	    d = 0;
	    if (point <= 0xed)			/* upper table limit */
	        d = captable0[point - 0x80];

	    return (d & A);
	}

	/*
	 *  point is SSi for other pages.  No xisalpha for page 2.
	 */
	if (ISPG2 (point))			/* code page 2 no cap/uncap */
	    return (0);

	page = point;
	point = *(p+1);
	if (ISPG1L (page))			/* code point been biased? */
	    point -= 0x80;			/* bias to lower half */

	d = 0;
	if (point >= 0x40 && point <= 0xe6)	/* in table range?	*/
	    d = captable1[point - 0x40];

	return (d & A);
/*NOTREACHED*/
}
/*
**	(UN)CAP - (un)capitalize in NLchar environment.
**
**	Parameter: q -- pointer to destination for output code point.
**		   p -- pointer to the code point to process.
**	Returns:   The number of chars processed.
**		   0 -> 1 in, 1 out
**		   1 -> 1 in, 2 out
**		   2 -> 2 in, 1 out
**		   3 -> 2 in, 2 out
**	
*/
int
cap (q, p)
char *q, *p;
{
    return (recap (q, p, C));
}

int
uncap (q, p)
char  *q, *p;
{
    return (recap (q, p, 0));
}

static
int recap (register char *q, register char *p, int mode)
			/* mode  :  C -> capitalize, 0 -> uncap */
{
	int  page, point;
	int  index, d;

	point = *p;				/* get first byte */
	if (!ISSSI (point))			/* not SSi for another page */
	{
	    /*
	     *  Real ASCII, lower page 0.
	     */
	    if (point < 0x80)
	    {
	        if (mode && point >= 'a' && point <= 'z') /* to upper */
		    point -= 'a' - 'A';		/* make upper */
	        else if (!mode && point >= 'A' && point <= 'Z')
		    point += 'a' - 'A';		/* make lower */

	        *q = point;

	        return (0);			/* processed 1 char */
	    }

	    /*
	     *  Upper page 0.
	     */
	    page = 0;
	    if (point <= 0xed)			/* upper table limit */
	    {
	        d = captable0[point - 0x80];
		if ((mode ^ d) & C)		/* xor cap flags only */
		{
	    	    page = (d >> 8) & 0xff;	/* extract new page */
	    	    point = d & 0xff;		/* and page point */
		}
	    }

	    if (page == 0)			/* 1 - 1 */
	    {
	        *q = point;
		return (0);
	    }

	    *q++ = page;
	    *q   = point;

	    return (1);				/* 1 - 2 */
	}

	/*
	 *  point = SSi.
	 */
	page = point;
	point = *(p+1);

	/*
	 *  Other pages.  No cap/uncap for page 2.
	 */
	if (ISPG2 (page))			/* code page 2 no cap/uncap */
	{
	    *q++ = page;
	    *q   = point;
	    return (3);				/* 2 - 2 */
	}

	index = point;				/* real table index */
	if (ISPG1L (page))			/* code point been biased? */
	    index -= 0x80;			/* bias to lower half */

	if (index >= 0x40 && index <= 0xe6)	/* in table range?	*/
	{
	    d = captable1[index - 0x40];
	    if ((mode ^ d) & C)
	    {
		page  = d >> 8;
		point = d & 0xff;
	    }
	}

	if (page == 0)
	{
	    *q = point;				/* store xlation */
	    return (2);				/* 2 - 1 */
	}

	*q++ = page;				/* store double xlation */
	*q   = point;

	return (3);				/* 2 - 2 */
/*NOTREACHED*/
}
/*
 *  UNCAPSTR - uncap an entire string.
 *
 *	The string may be uncapped in place since its length isn't increased.
 *	This is known because the uncap tables never map 1 char to 2 when
 *	uncapping.
 */
int
uncapstr (q, p, size)
char  *q, *p;
int  size;
{
	int  n;
	int  err;

	err = 0;			/* no error */
	n = 0;				/* count put in output */
	size--;				/* max count - 1 */
	while (1)
	{
	    register char  *qs;
	    register int  i;

	    if (*p == '\0')		/* end of input */
		break;

	    if (n >= size)		/* end of output */
	    {
		err = 1;		/* error */
		break;
	    }

	    qs = q;
	    UNCAP (q, p, i);
	    n += q - qs;
	}

	*q = '\0';

	return (err);
}
/*
 *  UNQUOTESTR - unquote an entire string.
 *
 *	The string may be unquoted in place since its length isn't increased.
 */
int
unquotestr (q, p, size)
char  *q, *p;
int  size;
{
	int  n;
	int  pre;

	n = 0;				/* count put in output */
	size--;				/* max count - 1 */
	pre = '\0';
	while (*p != '\0' && n < size)
	{
	    pre = unquote (*p++, pre);
	    *q++ = pre;
	    n++;
	}
	*q = '\0';

	if (*p != '\0' && n >= size)
	    return (1);

	return (0);
}
/*
 *  QUOTESTR - quote an entire string.
 *
 *	The string may be quoted in place since its length isn't increased.
 */
int
quotestr (q, p, size)
char  *q, *p;
int  size;
{
	int  n;

	n = 0;				/* count put in output */
	size--;				/* max count - 1 */
	while (*p != '\0' && n < size)
	{
	    *q++ = quote (*p++);
	    n++;
	}
	*q = '\0';

	if (*p != '\0' && n >= size)
	    return (1);

	return (0);
}
/*
**  STRIPQUOTES -- Strip quote chars & unquote the chars in a string.
**
**	Runs through a string and optionally strips off "
**	characters and unquotes quoted characters.  This is done in place.
**
**	Parameters:
**		s -- the string to strip.
**		qf -- if true, remove actual `` " '' characters and unquote
**			all other chars.
**		      if false, unquote all chars in place.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
**
**	Called By:
**		deliver
*/

stripquotes(s, qf)
	char *s;
	int qf;
{
	register char *p;
	register char *q;
	register char c, pre;

	if (s == NULL)
		return;

	p = q = s;
	pre = '\0';
	while ((c = *p++) != '\0')
	{
	    pre = unquote (c, pre);

	    if (c != '"' || !qf)
		*q++ = pre;
	}
	*q = '\0';
}
/*
**  QSTRLEN -- give me the string length including necessary escapes (\).
**
**	Parameters:
**		s -- the string to measure.
**
**	Reurns:
**		The length of s, including space for backslash escapes.
**
**	Side Effects:
**		none.
*/

qstrlen(s)
	register char *s;
{
	register int l;
	register int c, pre;

	pre = '\0';
	l = 0;
	while ((c = *s++) != '\0')
	{
	    pre = unquote (c, pre);
	    if (c != pre)
		l++;
	    l++;
	}
	return (l);
}
/*
**  CAPITALIZE -- return a copy of a string, properly capitalized.
**
**	Parameters:
**		s -- the string to capitalize.
**
**	Returns:
**		a pointer to a properly capitalized string.
**
**	Side Effects:
**		none.
*/

char *
capitalize(s)
	register char *s;
{
	static char buf[MAXNAME];
	register char *p;
	register int  i;

	p = buf;

	while (1)
	{
	    while (!xisalpha(s) && *s != '\0') 
	    {
		COPY1C (p, s, i);
	    }

	    if (*s == '\0')
		break;

	    CAP (p, s, i);

	    while (xisalpha(s)) 
	    {
		COPY1C (p, s, i);
	    }
	}

	*p = '\0';
	return (buf);
}
/*
**  XALLOC -- Allocate memory and complain wildly on failure.
**
**	THIS IS A CLUDGE.  This should be made to give a proper
**	error -- but after all, what can we do?
**
**	Parameters:
**		sz -- size of area to allocate.
**
**	Returns:
**		pointer to data region.
**
**	Side Effects:
**		Memory is allocated.
*/

char *
xalloc(sz)
	register int sz;
{
	register char *p;

	p = malloc((unsigned) sz);
	if (p == NULL)
	{
		syserr(MSGSTR(UT_MEM, "Out of memory!!")); /*MSG*/
		exit(EX_UNAVAILABLE);
	}
	return (p);
}
/*
**  SAFESTR -- return a copy of a string, with no dangerous chars.
**
**	Parameters:
**		s -- the string to process..
**
**	Returns:
**		a pointer to a safe string.
**
**	Side Effects:
**		none.
*/

char *
safestr(s)
	register char *s;
{
	register int n;
	int found = 0;
	static char *buf = NULL;
	static int len = 0;

	if ((n = strlen(s) + 1) > len)
	{
	    if (buf)
		free(buf);
	    len = n + 32;
	    buf = xalloc(len);
	}
	for(n = 0; s[n]; n++)
	{
	    if(s[n] == '\n')
	    {
		found = 1;
		buf[n] = ' ';
	    }
	    else
		buf[n] = s[n];
	}
	buf[n] = '\0';

	if(found)
	{
	    char *user = (char*)username();
	    syslog(LOG_ALERT,
		"POSSIBLE ATTACK from %s: newline in string \"%s\"",
		user == NULL ? "[UNKNOWN]" : user, buf);
	}

	return (buf);
}
/*
**  COPYPLIST -- copy list of pointers.
**
**	This routine is the equivalent of newstr for lists of
**	pointers.
**
**	Parameters:
**		list -- list of pointers to copy.
**			Must be NULL terminated.
**		copycont -- if TRUE, copy the contents of the vector
**			(which must be a string) also.
**
**	Returns:
**		a copy of 'list'.
**
**	Side Effects:
**		none.
*/

char **
copyplist(list, copycont)
	char **list;
	int copycont;
{
	char **newlist;
	int  len, lenp;
	register char  c, *p, *q, **vp;

#ifdef DEBUG
	if (tTd (70, 1))
	{
	    (void) printf ("\ncopyplist in, list head at 0x%x\n\n", list);
	    for (vp = list; *vp != NULL; vp++)
	        (void) printf ("0x%x - %s\n", *vp, *vp);
	}
#endif DEBUG

	len = lenp = 0;			/* clear length accumulators	*/

	for (vp = list; *vp != NULL; vp++)
	{
	    len += sizeof (*vp);	/* space for pointer in list	*/
	    if (copycont)
	        lenp += strlen (*vp) + 1; /* space for string		*/
	}
	len += sizeof (*vp);		/* for null pointer trailer	*/

	newlist = (char **) xalloc (len + lenp); /* get it all at once	*/

	(void) memcpy((char *) newlist, (char *) list, len);

	if (copycont)
	{
	    p = (char *) newlist + len;	/* init string dest ptr		*/
	    for (vp = newlist; (q = *vp) != NULL; vp++)
	    {
		*vp = p;		/* point to new origin		*/
		do
		{
		    c = *q; q++;
		    *p = c; p++;
		} while (c);
	    }
	}

#ifdef DEBUG
	if (p - (char *) newlist != len + lenp)
	    (void) printf ("copyplist address error, newlist 0x%x, p 0x%x\n",
					                       newlist, p);

	if (tTd (70, 1))
	{
	    (void) printf ("\ncopyplist out, list head at 0x%x\n\n", newlist);
	    for (vp = newlist; *vp != NULL; vp++)
	        (void) printf ("0x%x - %s\n", *vp, *vp);
	}
#endif DEBUG

	return (newlist);
}
/*
**  PRINTAV -- print argument vector.
**
**	Parameters:
**		av -- argument vector.
**
**	Returns:
**		none.
**
**	Side Effects:
**		prints av.
*/

printav (av)
char **av;
{
    printav1 (av, '\n');
}

printav1 (av, t)
register char **av;
register int  t;
{
    if (av == NULL)
	(void) printf ("<NULL>");

    while (av != NULL && *av != NULL)
    {
	(void) putchar (' ');
	xputs (*av++);
    }

    if (t)
	(void) putchar (t);
}
/*
**  XPUTS -- put string doing control escapes.  This is only used in debug
**		code and by printav (which, in turn, is only used for
**		debug and rules printout).
**
**	Parameters:
**		s -- string to put.
**
**	Returns:
**		none.
**
**	Side Effects:
**		output to stdout
*/

xputs(s)
	register char *s;
{
	register char c, pre;

	if (s == NULL)
	{
		(void) printf("<null>");
		(void) fflush(stdout);
		return;
	}

	(void) putchar('"');

	pre = '\0';
	while ((c = *s++) != '\0')
	{
	    pre = unquote (c, pre);

	    /*
	     *  Handle chars that were quoted.  This allows \ to be inserted.
	     */
	    if (c != pre)			/* was quoted */
		(void) putchar ('\\');		/* marker to show quoted */

	    /*
	     *  Handle high chars.
	     */
	    if (pre >= 0x7f)			/* too high for normal ASCII */
	    {
		(void) putchar('&');		/* put out high signal */
		pre &= 0x7f;			/* map to printables */
	    }

	    /*
	     *  Handle NL char stuff.
	     */
	    if (ISSSI (pre))			/* is this an SSi control? */
	    {
		(void) putchar (pre);		/* put out the control */
		pre = *s++;			/* get next char into pre */
		(void) putchar (pre);		/* last of pair */
		if (pre == 0)			/* end of line */
		    break;
		else
		    continue;
	    }

	    if (pre < ' ')			/* if control */
	    {
		(void) putchar ('^');		/* put out ctrl signal */
		pre |= 0x40;			/* map to printables */
	    }

	    (void) putchar (pre);
	}

	(void) putchar('"');
	(void) fflush(stdout);
}
/*
**  MAKELOWER -- Translate a line into lower case
**
**	Parameters:
**		p -- the string to translate.  If NULL, return is
**			immediate.
**
**	Returns:
**		none.
**
**	Side Effects:
**		String pointed to by p is translated to lower case.
**
**	Called By:
**		parse
*/

makelower(p)
	register char *p;
{
	if (p == NULL)
		return;

	(void) uncapstr (p, p, 1000000);
}
/*
**  SAMEWORD -- return TRUE if the words are the same regardless of
**		capitalization.
**
**	Parameters:
**		a, b -- the words to compare.
**
**	Returns:
**		TRUE if a & b match exactly (modulo case)
**		FALSE otherwise.
**
**	Side Effects:
**		none.
*/

int
sameword(a, b)
	register char *a, *b;
{
	char ca[2], cb[2];
	int  i;

	do
	{
	    ca[0] = ca[1] = cb[0] = cb[1] = '\0';
	    i = uncap (ca, a++);
	    if (i & 2)  a++;
	    i = uncap (cb, b++);
	    if (i & 2)  b++;

	} while (ca[0] != '\0' && ca[0] == cb[0] && ca[1] == cb[1]);

	i = (ca[0] == cb[0]) && (ca[1] == cb[1]);

	return (i);
}
/*
**  BUILDFNAME -- build full name from gecos style entry.
**
**	This routine interprets the strange entry that would appear
**	in the GECOS field of the password file.
**
**	Parameters:
**		name -- name to build.
**		login -- the login name of this user (for &).
**		buf -- place to put the result.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
*/

buildfname(name, login, buf)
	register char *name;
	char *login;
	char *buf;
{
	register char *bufp;
	register int  pre;
   	int count = 0;
	char  logbuf[MAXNAME];
	register char *logbufp;

	if (*name == '*')
		name++;				/* skip it */

	bufp = buf;
	pre = logbuf[0] = '\0';
	while (*name != '\0' && *name != ',' && *name != ';' && 
		*name != '/' && count < MAXNAME )
	{
		if (*name == '&')
		{
			name++;			/* skip it */
			count += strlen(login);
                        if (count > MAXNAME) {
                                *bufp = '\0';
                                break;
                        }


			/*
			 *  Make buffer of unquoted login name.
			 */
			if (logbuf[0] == '\0')
			    (void) unquotestr (logbuf, login, MAXNAME);

			/*
			 *  Copy the unquoted login name, capitalizing the
			 *  the first char.
			 */
			logbufp = logbuf;

			if (*logbufp != '\0')
			{
			    CAP (bufp, logbufp, pre);
			}

			while (*logbufp != '\0')
			{
			    COPY1C (bufp, logbufp, pre);
			}

			pre = '\0';		/* for next char */
		}
		else
		{
			pre = unquote (*name++, pre);
			*bufp++ = pre;
			count++;
		}
	}
	*bufp = '\0';
}
/*
**  SAFEFILE -- return true if a file exists and is safe for a user.
**
**	Parameters:
**		fn -- filename to check.
**		uid -- uid to compare against.
**		mode -- mode bits that must match.
**
**	Returns:
**		TRUE if fn exists, is owned by uid, and matches mode.
**		FALSE otherwise.
**
**	Side Effects:
**		none.
*/

int
safefile(fn, uid, mode)
	char *fn;
	uid_t uid;
	int mode;
{
	struct stat stbuf;

	if (stat(fn, &stbuf) >= 0 && stbuf.st_uid == uid &&
	    (stbuf.st_mode & mode) == mode)
		return (TRUE);
	errno = 0;
	return (FALSE);
}
/*
**  SAFEPATH -- return true if a file exists and is safe for a user.
**		For compatibility, also check that every path
**		component has execute permission.  For compatibility,
**		uid 0 is not given superuser treatment.
**
**	Parameters:
**		fn -- filename to check.
**		uid -- uid to compare against.
**		gid -- gid to compare against.
**		mode -- mode bits that must match.
**
**	Returns:
**		TRUE if fn exists and uid/gid is allowed the given
**		    mode access.
**		FALSE otherwise.
**
**	Side Effects:
**		none.
*/

int
safepath(fn, uid, gid, mode)
  char *fn;
  uid_t uid;
  gid_t gid;
  int mode;
{
    register char *c, **mem;
    struct stat stbuf;
    struct group *gr = NULL;
    struct passwd *pw;
    int mask;
    char oldc;
    static char path[MAXPATHLEN + 1];

    /* We cannot scribble on the string we are given -- it might
     * be a constant.  Make a copy.
     */
    if(strlen(fn) > MAXPATHLEN)
    {
	errno = ENAMETOOLONG;
	return (FALSE);
    }
    strcpy(path, fn);

    /* Get the password file entry for this user.
     */
    pw = getpwuid(uid);

    /* Check every path component for appropriate accessibility.
     */
    if(*(c = path) == '/')
	c++;
    for(;; c++)
	if(*c == '/' || *c == '\0')
	{
	    if((oldc = *c) == '\0')
		mask = mode;
	    else
	    {
		*c = '\0';
		mask = S_IEXEC;
	    }

	    if(stat(path, &stbuf) < 0)
		return (FALSE);
	    if(stbuf.st_uid != uid)
	    {
		mask >>= 3;
		if(stbuf.st_gid != gid)
		{
		    /* See if the user is a member of this group.
		     */
		    if(gr == NULL || gr->gr_gid != stbuf.st_gid)
			gr = getgrgid(stbuf.st_gid);
		    if(pw && gr)
		    {
			for(mem = gr->gr_mem; *mem; mem++)
			    if(strcmp(*mem, pw->pw_name) == 0)
				break;
			if(!(*mem))
			    mask >>= 3;
		    }
		    else
			mask >>= 3;
		}
	    }

	    /* For compatibility, uid 0 is NOT given superuser treatment.
	     */
	    if((stbuf.st_mode & mask) != mask)
	    {
		errno = EACCES;
		return (FALSE);
	    }

	    if((*c = oldc) == '\0')
		return (TRUE);
	}
    /*NOTREACHED*/
}
/*
**  DFORK -- Perform fork with retry if system overloaded.
**		This is only needed where not being able to fork would
**		cause loss of mail.
**
**	Parameters:
**		Retry count.
**
**	Returns:
**		Same as fork () system call.
**
**	Side Effects:
**		none.
*/

int
dfork (count)
	int count;
{
	int  i;

	i = fork ();

	while (i < 0 && --count)
	{
	    sleep (10);
	    i = fork ();
	} 

	return (i);
}
/*
**  FIXCRLF -- fix <CR><LF> in line.
**
**	Looks for the <CR><LF> combination and turns it into the
**	UNIX canonical <NL> character.  It only takes one line,
**	i.e., it is assumed that the first <NL> found is the end
**	of the line.
**
**	Parameters:
**		line -- the line to fix.
**		stripnl -- if true, strip the newline also.
**
**	Returns:
**		none.
**
**	Side Effects:
**		line is changed in place.
*/

fixcrlf(line, stripnl)
	char *line;
	int stripnl;
{
	register char *p;

	p = strchr(line, '\n');
	if (p == NULL)
		return;
	if (p > line && p[-1] == '\r')
		p--;
	if (!stripnl)
		*p++ = '\n';
	*p = '\0';
}
/*
**  DFOPEN -- determined file open
**
**	This routine has the semantics of fopen, except that it will
**	keep trying a few times to make this happen.  The idea is that
**	on very loaded systems, we may run out of resources (inodes,
**	whatever), so this tries to get around it.
*/

FILE *
dfopen(filename, mode)
	char *filename;
	char *mode;
{
	register int tries;
	register FILE *fp;

	for (tries = 0; tries < 10; tries++)
	{
		sleep((unsigned) (10 * tries));
		errno = 0;
		fp = fopen(filename, mode);
		if (fp != NULL)
			break;
		if (errno != ENFILE && errno != EINTR)
			break;
	}
	errno = 0;
	return (fp);
}
/*
**  PUTLINE -- put line(s) like fputs, optionally obeying SMTP conventions
**
**	This routine always guarantees outputing a newline (or CRLF,
**	as appropriate) at the end of the string.
**
**	Multiple input lines must be delimited by newline.  Each line is put
**	out separately.
**
**	If the M_LIMITS mailer flag is set, then all bytes have bit 2^7
**	cleared.  Also, each line is put out in sublines of maximum
**	length SMTPLINELIM with continuation marks, if necessary.
**
**	Parameters:
**		line -- line to put.
**		fp -- file to put it onto.
**		m -- the mailer used to control output.
**
**	Returns:
**		Nothing at present.  Like other things, needs work.
**
**	Side Effects:
**		output of l to fp.
*/

/* max line length, text only.  SMTP limit is 1000, including \r\n */
# define SMTPLINELIM	990
static jmp_buf jbuf;

putline(line, fp, m)
	register char *line;
	FILE *fp;
	MAILER *m;
{
	register char *p, *q, c;
	char svchar;
	int  eolk;
	EVENT *ev;
	long i;

	/*
	 *  Preprocess lines to remove 8 bit data if the M_LIMITS mailer flag
	 *  is set, and the M_LANG mailer flag is not set.  This is the normal
	 *  SMTP case without National Language support.
	 *
	 *  Clear 2^7 bits in the line(s) for normal SMTP.  These can look 
	 *  like TELNET protocol, and are illegal in SMTP messages.  
	 *  Warning:  The indiscriminate algorithm can generate nulls, newlines,
	 *  or other special characters by clearing the high bit in a byte!  
	 *  Putline replaces fake nulls with a newline character (ugh!) so as
	 *  not to truncate the output buffer.
	 *
	 *  The transmit buffer may contain one or more lines.  Lines are
	 *  SEPARATED by newline characters (if any) and the terminating null.
	 *  Each line is transmitted, followed by the mailer-defined "eol"
	 *  sequence.  Zero length lines are allowed and each will appear in the
	 *  output as the "eol" sequence only.  Exception: The only zero length
	 *  not transmitted is a possible zero length line between the last 
	 *  newline and the terminating null.  This means that an empty (null 
	 *  terminated) transmit buffer follows the normal rule, and will 
	 *  transmit a single "eol" sequence.  This also means that a set of 
	 *  newline-terminated lines will be transmitted as given, without 
	 *  appending a zero length line after them.
	 *
	 *  If the M_LIMITS mailer flag is set, each line may be broken into 
	 *  sublines of maximum text length SMTPLINELIM, if necessary.  
	 *  (The SMTP specs know nothing about this!).
	 *
	 *  SMTPLINELIM in this code is chosen to be smaller that the 1000 
	 *  char limit in the SMTP specs, since the spec'd limit includes the 
	 *  "eol" sequence.  I don't know why 990 was chosen instead of 998 
	 *  (since the eol sequence is decreed to be \r\n).
	 *
	 *  Note: The "line" variable below keeps getting moved forward.
	 */

	eolk = strlen (m->m_eol);
#ifdef DEBUG
	if (tTd (72, 30))
	    (void) printf ("putline: len(m_eol)=%d, m_eol='%s'\n",
		eolk, m->m_eol);
#endif DEBUG

	WriteErr = FALSE;
	if (strcmp(SmtpPhase, "DATA wait") == 0 && TransmitTimeout != 0)
	{
		if (setjmp(jbuf))
		{	
#ifdef LOG
			syslog(LOG_NOTICE, MSGSTR(UT_TIMEOUT, "timeout while writing to %s\n"), RealHostName);
#endif LOG
			usrerr(MSGSTR(UT_TIMEOUT2, "451 timeout while writing output"));
			WriteErr = TRUE;
			errno = 0;
			return 0;
		}

		ev = setevent(ReadTimeout, writetimeout, 0);
	}

	do
	{
		/*
		 *  Find the end of the current line.  This is delimited
		 *  by newline or null.  Two cases are provided for 
		 *  optimization.  "line" is always pointing to the start
		 *  of the next line in the buffer.  The computation below
		 *  leaves "p" pointing at the next real newline or null.
		 *
		 *  Code these loops for good register usage.
		 */
		if ( bitnset (M_LIMITS, m->m_flags) && 
		    !bitnset (M_LANG,   m->m_flags)    )
		{
		    p = line;
		    while (1)
		    {
		    	c = *p;
		    	if (c == '\0' || c == '\n')	/* end of line?	*/
			    break;		/* yes			*/

		    	c &= 0x7f;		/* clear 2^7		*/

		    	if (c == '\0' || c == '\n')	/* fake delimiters? */
			    c = ' '; 		/* how about that! (ugh!) */

		    	*p = c;			/* store it back	*/
		    	p++;			/* point to next one	*/
		    }
		}
		else
		{
			p = line;
			while (1)
			{
		    	    c = *p;
		    	    if (c == '\0' || c == '\n')
				break;

		    	    p++;
			}
		}
		    
		OutputCount += p - line;	/* incr for full text */

		/*
		 *  Put out this line in sublines of max length, if necesary 
		 *  (only if the M_LIMITS mailer flag is set).  Each continued 
		 *  subline is terminated with a ! character.  There used to be
		 *  a problem here, since the ! character used for continuation
		 *  could appear in user text as the last character of an 
		 *  unsubdivided line of exact maximum text length.  In that 
		 *  case, the line would appear to be continued, when it was 
		 *  not.
		 *
		 *  The software has been changed so that only continued sub-
		 *  lines have exact maximum text length.  The last character
		 *  position contains the !.  Any noncontinued line or trailing
		 *  subline will always have text length less than the maximum.
		 *  This way receivers can test on line length, or ! char in 
		 *  max position in order to detect that a line is continued.
		 *
		 *  None of this continuation business applies if the M_LIMITS
		 *  mailer flag is not set.
		 */
		if (bitnset (M_LIMITS, m->m_flags))
		{
		    /*
		     *  Put as continued line only if remaining length
		     *  of TEXT is actually greater than SMTPLINELIM - 1.
		     *  The actual text length in the line is will be
		     *  SMTPLINELIM-1 plus the !, which == SMTPLINELIM.
		     *  If the remaining text length <= SMTPLINELIM-1, then
		     *  that text is not transmitted as a continued line.
		     */
		    while ((p - line) > SMTPLINELIM - 1)
		    {
			/*
			 *  Mark end at SMTPLINELIM - 1 (leave room for
			 *  the bang).
			 */
			q = line + SMTPLINELIM - 1;

			svchar = *q;	/* save what's there */
			*q = '\0';	/* null for fputs */

			/*
			 *  Double the leading dot if flag M_XDOT set.
			 *  This dot isn't to be counted in SMTPLINELIM.
			 */
			if (line[0] == '.' && bitnset(M_XDOT, m->m_flags))
			{
				putc('.', fp);
				OutputCount++;
			}

#ifdef DEBUG
			if (tTd (72, 20))
			    (void) printf ("putline: fputs('%s', 0x%x)\n",
				line, fp);
#endif DEBUG
			fputs(line, fp); /* out goes the line */
			putc('!', fp);	/* nonspec continuation marker? */
			fputs(m->m_eol, fp); /* mailer eol */
			OutputCount += eolk + 1;

			*q = svchar;	/* put back what was there */
			line = q;	/* update line pointer */
		    }
		}

		/*
		 *  Output final subline, or entire line if M_LIMITS not
		 *  set.  The "while" test above prevents any SMTP line or 
		 *  subline at this point from being SMTPLINELIM in
		 *  length.
		 */
		svchar = *p;		/* save newline or null */
		*p = '\0';		/* assure it's a null */

		/*
		 *  Double the leading dot if flag M_XDOT set.
		 *  This dot is never counted for SMTP line length purposes.
		 */
		if (line[0] == '.' && bitnset(M_XDOT, m->m_flags))
		{
			putc('.', fp);
			OutputCount++;
		}

#ifdef DEBUG
		if (tTd (72, 20))
		    (void) printf ("putline: fputs(%s)\n", line);
#endif DEBUG
		fputs(line, fp);	/* send out the line */
		fputs(m->m_eol, fp);
		OutputCount += eolk;

		*p = svchar;		/* put back terminator */
		line = p;		/* update line ptr */

		if (*line == '\n')	/* skip over any newline */
			line++;

	} while (line[0] != '\0');	/* is there more? */
	
	if (strcmp(SmtpPhase, "DATA wait") == 0 && TransmitTimeout != 0)
	{
		clrevent(ev);
		return 1;
	}
}

static
writetimeout()
{
	longjmp(jbuf, 1);
}
/*
**  XUNLINK -- unlink a file, doing logging as appropriate.
**
**	Parameters:
**		f -- name of file to unlink.
**
**	Returns:
**		none.
**
**	Side Effects:
**		f is unlinked.
*/

xunlink(f)
	char *f;
{
	register int i;

# ifdef LOG
	if (LogLevel > 20)
		syslog(LOG_DEBUG, MSGSTR(UT_ULINK, "%s: unlink %s\n"), CurEnv->e_id, f); /*MSG*/
# endif LOG

	i = unlink(f);
# ifdef LOG
	if (i < 0 && LogLevel > 21)
		syslog(LOG_DEBUG, MSGSTR(UT_ULINK2, "%s: unlink-fail %d"), f, errno); /*MSG*/
# endif LOG
}
/*
**  SFGETS -- "safe" fgets -- times out and ignores random interrupts.
**
**	Parameters:
**		buf -- place to put the input line.
**		siz -- size of buf.
**		fp -- file to read from.
**
**	Returns:
**		NULL on error (including timeout).  This will also leave
**			buf containing a null string.
**		buf otherwise.
**
**	Side Effects:
**		none.
*/

static int	ctxreadtimeout;

char *
sfgets(buf, siz, fp)
	char *buf;
	int siz;
	FILE *fp;
{
	register char *p;
	EVENT *ev;
	MAILER *m;

	m = CurEnv->e_from.q_mailer;		/* "from" mailer */

#ifdef DEBUG
	if (tTd (71, 1))
	    (void) printf ("sfgets mailer %sdefined\n", 
						(m == NULL) ? "not " : "");
#endif DEBUG

	ctxreadtimeout = FALSE;
	if (ReadTimeout != 0)
	    ev = setevent (ReadTimeout, readtimeout, 0);

	/* try to read */
	p = NULL;
	while (p == NULL && !feof(fp) && !ferror(fp))
	{
		errno = 0;
		p = fgets(buf, siz, fp);
#ifdef DEBUG
		if (tTd (71, 20))
		    (void) printf (
	    "sfgets: after fgets: p=0x%x, errno=%d, siz=%d, ctxreadtimeout=%d,\
\n  buf='%s'\n",
			p, errno, siz, ctxreadtimeout, (p ? buf : ""));
#endif DEBUG

		if (errno == EINTR)
		{
		    if (ctxreadtimeout)
		    {
# ifdef LOG
			syslog(LOG_NOTICE, MSGSTR(UT_TIME, "timeout waiting for input from %s\n"), RealHostName ? RealHostName : "local");
# endif
			errno = 0;
			usrerr(MSGSTR(UT_TIME2, "451 timeout waiting for input"));
			buf[0] = '\0';
#ifdef DEBUG
			if (tTd (71, 1))
			    puts("sfgets: returning NULL due to timeout");
#endif DEBUG
			return (NULL);
		    }
		    clearerr(fp);
		}
	}
#ifdef DEBUG
	if (tTd (71, 5))
	    (void) printf ("sfgets: after loop: p=0x%x, feof=%d, ferror=%d\n",
		p, feof(fp), ferror(fp));
#endif DEBUG

	/* clear the event if it has not sprung */
	clrevent(ev);

	/* clean up the books and exit */
	LineNumber++;
	if (p == NULL)
	{
		buf[0] = '\0';
#ifdef DEBUG
		if (tTd (71, 1))
		    (void) puts("sfgets: returning NULL due to p==NULL");
#endif DEBUG
		return (NULL);
	}

	/*
	 *  The original Berkeley code here unconditionally cleared bit 2^7 in 
	 *  all bytes.  However, putline didn't clear 2^7 when M_LIMITS wasn't
	 *  set!  In order to allow general transmission of 8 bit data, this
	 *  code to unconditionally clear 2^7 has been made conditional.
	 *
	 *  The high bits will be cleared only when M_LIMITS mailer flag is
	 *  set and M_LANG mailer flag is NOT set.  This has the effect of
	 *  cleaning up any high bits set by defective software at the 
	 *  other end.  If the incoming SMTP were according to spec, none
	 *  of the high bits would be set anyway.  National Language SMTP
	 *  must allow 8 bit data.
	 */
	if (m != NULL && 
		 bitnset (M_LIMITS, m->m_flags) && 
		!bitnset (M_LANG,   m->m_flags)      )
	{
#ifdef DEBUG
	    if (tTd (71, 1))
		(void) printf ("sfgets clears bits 2^7\n");
#endif DEBUG

	    for (p = buf; *p != '\0'; p++)
		*p &= 0x7f;

	    InputCount += p - buf;
	}
	else
	{
	    InputCount += strlen (buf);
	}

#ifdef DEBUG
	if (tTd (71, 5))
	    (void) printf("sfgets: returning buf=0x%x\n", buf);
#endif DEBUG

	return (buf);
}

static
readtimeout()
{
	ctxreadtimeout = TRUE;
}
/*
**  FGETFOLDED -- like fgets, but know about folded lines.
**
**	Parameters:
**		buf -- place to put result.
**		n -- bytes available.
**		f -- file to read from.
**
**	Returns:
**		buf on success, NULL on error or EOF.
**
**	Side Effects:
**		buf gets lines from f, with continuation lines (lines
**		with leading white space) appended.  CRLF's are mapped
**		into single newlines.  Any trailing NL is stripped.
*/

char *
fgetfolded(buf, n, f)
	char *buf;
	register int n;
	FILE *f;
{
	register char *p = buf;
	register int i;

	n--;
	while ((i = getc(f)) != EOF)
	{
		if (i == '\r')
		{
			i = getc(f);
			if (i != '\n')
			{
				if (i != EOF)
					(void) ungetc(i, f);
				i = '\r';
			}
		}
		if (--n > 0)
			*p++ = i;
		if (i == '\n')
		{
			LineNumber++;
			i = getc(f);
			if (i != EOF)
				(void) ungetc(i, f);
			if (i != ' ' && i != '\t')
			{
				*--p = '\0';
				return (buf);
			}
		}
	}
	return (NULL);
}
/*
**  ATOBOOL -- convert a string representation to boolean (integer).
**
**	Defaults to "TRUE"
**
**	Parameters:
**		s -- string to convert.  Takes "tTyY" as true,
**			others as false.
**
**	Returns:
**		A boolean representation of the string (integer 0 or 1).
**
**	Side Effects:
**		none.
*/

int
atobool(s)
	register char *s;
{
	if (*s == '\0' || strchr("tTyY", *s) != NULL)
		return (TRUE);
	return (FALSE);
}
/*
**  ATOOCT -- convert a string representation to octal.
**
**	Parameters:
**		s -- string to convert.
**
**	Returns:
**		An integer representing the string interpreted as an
**		octal number.
**
**	Side Effects:
**		none.
*/

atooct(s)
	register char *s;
{
	register int i = 0;

	while (*s >= '0' && *s <= '7')
		i = (i << 3) | (*s++ - '0');
	return (i);
}
/*
**  WAITFOR -- wait for a particular process id if zombies are allowed.
**		Else, wait for all children to finish.  In the latter case,
**		a dummy positive pid should be given as argument.
**		EINTR is always ignored.
**
**	Parameters:
**		pid -- process id to wait for.
**
**	Returns:
**		status of pid.
**		-1 if pid never shows up.
**
**	Side Effects:
**		none.
*/

waitfor(pid)
	int pid;
{
	int st;
	int i;

	/*
	 *  Repeat a wait call if it fails due to signal, or picks up an
	 *  unwanted child.
	 */
	do
	{
		errno = 0;
		i = wait(&st);
	} while ((i >= 0 || errno == EINTR) && i != pid);
	if (i < 0)
		st = -1;
	return (st);
}
/*
**  BITINTERSECT -- tell if two bitmaps intersect
**
**	Parameters:
**		a, b -- the bitmaps in question
**
**	Returns:
**		TRUE if they have a non-null intersection
**		FALSE otherwise
**
**	Side Effects:
**		none.
*/

int
bitintersect(a, b)
	BITMAP a;
	BITMAP b;
{
	int i;

	for (i = BITMAPBYTES / sizeof (int); --i >= 0; )
		if ((a[i] & b[i]) != 0)
			return (TRUE);
	return (FALSE);
}
/*
**  BITZEROP -- tell if a bitmap is all zero
**
**	Parameters:
**		map -- the bit map to check
**
**	Returns:
**		TRUE if map is all zero.
**		FALSE if there are any bits set in map.
**
**	Side Effects:
**		none.
*/

int
bitzerop(map)
	BITMAP map;
{
	int i;

	for (i = BITMAPBYTES / sizeof (int); --i >= 0; )
		if (map[i] != 0)
			return (FALSE);
	return (TRUE);
}

/*
 *  cat - Concatenate two strings into a buffer, while checking for overflow.
 *	  Return pointer to target if OK; else, NULL.
 */
char *
cat (target, size, s1, s2)
char  *target;
register char  *s1, *s2;
int  size;
{
    register char  *p, *q;

    p = target;				/* moving target */
    q = target + size - 1;		/* last place available for storage */

    while (p <= q && *s1 != '\0')	/* copy s1 */
    {
	*p = *s1; p++; s1++;
    }

    while (p <= q && *s2 != '\0')	/* copy s2 */
    {
	*p = *s2; p++; s2++;
    }

    if (p > q)				/* filled entirely */
	return (NULL);

    *p = '\0';				/* terminate the output */

    return (target);
}

/*
**  XRMDIR -- remove directory and its files
**
**	Parameters:
**		dir -- the directory pathname to remove
**		ignore -- non-zero iff we should ignore ENOENT errors on
**			  the directory
**
**	Returns:
**		-1 if error, errno set to value
**		0 if success
**
**	Side Effects:
**		none.
*/

int
xrmdir(dir, ignore)
char *dir;
int ignore;

{

register int num, err = 0;
struct dirent *((*list)[]);
char fname[MAXNAME * 2];


    /* try to run scandir() to build dir list; ignore non-existent dir
       error if we are told to */

    if ((num = scandir(dir, &list, NULL, NULL)) == -1)
	return((ignore && errno == ENOENT) ? 0 : -1);

    /* step through files in dir, unlinking them; if we get an error,
       don't try to unlink the others, but continue to free the structs;
       don't unlink the ".." and "." links */

    if (num)
	do {
	    --num;
	    if (! err && num > 1) {  /* ".." and "." are entries 0 and 1 */
		sprintf(fname, "%s/%s", dir, ((*list)[num])->d_name);
		if (unlink(fname))
		    err = errno;  /* save error */
#ifdef DEBUG
		if (tTd (74, 5))
		    (void) printf(
			"xrmdir: unlink(%s) (dir entry %d) returned %d\n",
			((*list)[num])->d_name, num, err);
#endif DEBUG
	    }

	    free((char *) (*list)[num]);  /* free structs allocd by scandir() */

	} while (num);

    free((char *) list);  /* free the array allocated by scandir() */

    /* if there were no errors unlinking files, we now remove the dir */

    errno = err;  /* restore error */
    if (! err && rmdir(dir))
	return((ignore && errno == ENOENT) ? 0 : -1);
    return(err ? -1 : 0);
}
