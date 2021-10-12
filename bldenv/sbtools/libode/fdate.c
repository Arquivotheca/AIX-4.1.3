/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: casefix
 *		copyword
 *		defined
 *		dofield
 *		fdate
 *		foldc
 *		formfld
 *		percent
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 *  
 * Copyright (c) 1992 Carnegie Mellon University 
 * All Rights Reserved. 
 *  
 * Permission to use, copy, modify and distribute this software and its 
 * documentation is hereby granted, provided that both the copyright 
 * notice and this permission notice appear in all copies of the 
 * software, derivative works or modified versions, and any portions 
 * thereof, and that both notices appear in supporting documentation. 
 *  
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR 
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE. 
 *  
 * Carnegie Mellon requests users of this software to return to 
 *  
 * Software Distribution Coordinator  or  Software_Distribution@CS.CMU.EDU 
 * School of Computer Science 
 * Carnegie Mellon University 
 * Pittsburgh PA 15213-3890 
 *  
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes. 
 */
/* 
 * HISTORY
 * $Log: fdate.c,v $
 * Revision 1.7.8.1  1993/11/10  18:44:31  root
 * 	CR 463. Pedantic changes
 * 	[1993/11/10  18:42:51  root]
 *
 * Revision 1.7.6.3  1993/04/28  20:07:25  damon
 * 	CR Pedantic changes
 * 	[1993/04/28  20:07:08  damon]
 * 
 * Revision 1.7.6.2  1993/04/28  15:28:05  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  15:26:13  damon]
 * 
 * Revision 1.7.2.3  1992/12/03  17:20:45  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:05  damon]
 * 
 * Revision 1.7.2.2  1992/12/02  20:25:55  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:10  damon]
 * 
 * Revision 1.7  1991/12/05  21:04:45  devrcs
 * 	Added _FREE_ to copyright marker
 * 	[91/08/01  08:11:09  mckeen]
 * 
 * 	Added changes to support RIOS and aix
 * 	[91/01/22  13:00:23  mckeen]
 * 
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:23:28  dwm]
 * 
 * 	Declare functions static to eliminate compiler warnings.
 * 	[90/11/26  18:27:30  tom]
 * 
 * Revision 1.5  90/10/07  20:03:06  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:08:43  gm]
 * 
 * Revision 1.4  90/08/09  14:22:57  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:46:36  gm]
 * 
 * Revision 1.3  90/06/29  14:38:23  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:20:49  gm]
 * 
 * Revision 1.2  90/01/02  19:26:42  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:13:33  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * 	Adapted for 4.2 BSD UNIX:  macro file name changed.
 * 	[85/04/30            sas]
 * 
 * 	Created.
 * 	[84/03/15            lgh]
 * 
 * $EndLog$
 */
#ifndef lint
static char sccsid[] = "@(#)79  1.1  src/bldenv/sbtools/libode/fdate.c, bldprocess, bos412, GOLDA411a 1/19/94 17:40:57";
#endif /* not lint */

#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: fdate.c,v $ $Revision: 1.7.8.1 $ (OSF) $Date: 1993/11/10 18:44:31 $";
#endif
#include <stdio.h>
#include <string.h>
#include <ode/util.h>
#include <sys/types.h>
#include <sys/time.h>
#ifdef INC_TIME
# include <time.h>
#endif

#define TEXT 1
#define FAIL 2
#define OPTIONAL 3
#define SUCCEED 4

#define YES 1
#define NO 0
#define MAYBE 2

#define SMONTHS 0
#define SWDAYS 12
#define SNOON 19
#define SMIDNIGHT 20
#define SAM 21
#define SPM 22
#define STH 23
#define SST 24
#define SND 25
#define SRD 26

#define NOON 0
#define HOUR 1
#define AM 2
#define MIN 3
#define SEC 4
#define WDAY 5
#define DAY 6
#define TH 7
#define NMONTH 8
#define MONTH 9
#define YEAR 10
#define YDAY 11
#define MIDNIGHT 12
#define TIME 13

static const char *strings[] =
{ "jan*uary", "feb*ruary", "mar*ch", "apr*il", "may", "jun*e",
  "jul*y", "aug*ust", "sep*tember", "oct*ober", "nov*ember", "dec*ember",
  "sun*day", "mon*day", "tue*s*day", "wed*nesday", "thu*r*s*day",
  "fri*day", "sat*urday",
  "noon", "00:00", "am", "pm", "th", "st", "nd", "rd"
};

static const char *keys[] =
{ "noon", "hour", "am", "min", "sec",
  "wday", "day", "th", "nmonth", "month", "year", "yday",
  "midnight", "time",
  0
};

/* The following is a macro definition of %time. */

static const char *mactime =
  "[%midnight|%noon|%0hour:%min:%?sec|{%hour{:%?min}%am}]";

static const char *p;
static int resp;
static char *result;

/*
 * Prototypes
 */
static void
casefix ( char *, int );
static void
copyword ( char *, const char *, int );
static int
dofield ( char , struct tm *);
static char
foldc ( char );
static int
formfld ( struct tm *, int , int, int,
                 const char **);
static int
percent ( struct tm *);

char *
fdate ( char *resb, const char *patt, struct tm *tm )
{ result = resb;
  resp = 0;
  p = patt;
  dofield (' ', tm);
  result[resp] = '\0';
  return (result);
}

static int
dofield ( char doing, struct tm *tm )
{ int ndep, s, sresp, sresp2;
  int fail;
  char ch;
  fail = MAYBE;
  sresp = resp;
  while (*p)
  { if (*p == '{' || *p == '[')
    { ch = *p++;
      s = dofield (ch, tm);
      if (s == FAIL && doing == '[')
	fail = YES;
      else if (s == SUCCEED && doing == '{')
	fail = NO;
    }
    else if (*p == '}' || *p == ']' || *p == '|')
    { if (doing == '{' && fail == MAYBE)
	fail = YES;
      if (fail == YES)
      { resp = sresp;
        if (*p == '|')
        { p++;
	  fail = MAYBE; 			     /* try next */
	  continue;
        }
	p++;
      }
      else if (*p == '|')
      { /* Scan to matching bracket/brace */
	ndep = 1;
	while (ndep > 0)
	{ if (*p == '{' || *p == '[')
	    ndep++;
	  else if (*p == '}' || *p == ']')
	    ndep--;
	  p++;
	}
      }
      else
	p++;
      return (fail == YES ? FAIL : SUCCEED);
    }
    else if (*p == '%')
    { sresp2 = resp;
      s = percent (tm);
      if (doing == '{')
      { if (s == FAIL)		     /* Discard failure results */
	  resp = sresp2;
	else if (s == SUCCEED)
	  fail = NO;
      }
      else if (doing == '[')
      { if (s == FAIL)
	  fail = YES;
      }
    }
    else
    { result[resp++] = *p;
      p++;
    }
    if (fail == YES)
    { /* Failure: scan for barline or closing bracket/brace; leave ptr there */
      resp = sresp;
      ndep = 1;
      while (ndep > 0)
      { if (*p == '|' && ndep == 1)
	  break;
	if (*p == '[' || *p == '{')
	  ndep++;
	else if (*p == ']' || *p == '}')
	{ ndep--;
	  if (ndep == 0)
	    return (FAIL);
	}
	p++;
      }
    }
  }
  return (SUCCEED);
}

static int
percent ( struct tm *tm )
{
  register char *pp;
  char *spp;
  int query, lead0, prec, upcase, sresp, s;
  const char *savep;
  register int ks;
  register const char *k;
  const char *fldp;
  pp = (char *)p + 1;
  if (*pp == '%' || *pp == '{' || *pp == '}' || *pp == '|' ||
    *pp == '[' || *pp == ']')
  { p += 2;
    result[resp++] = *pp;
    return (TEXT); 			     /* Was just text */
  }
  query = *pp == '?'; 			     /* Query? */
  if (query)
    pp++;
  lead0 = *pp == '0';	 		     /* Leading zero? */
  if (lead0)
    pp++;
  if (*pp >= '1' && *pp <= '9') 	     /* Precision? */
  { prec = *pp - '0';
    pp++;
  }
  else
    prec = 0;
  upcase = 0;	 			     /* Case? */
  if (*pp >= 'A' && *pp <= 'Z')
    upcase = 1;
  if (pp[1] >= 'A' && pp[1] <= 'Z')
    upcase = 2;

  spp = pp;
  for (ks = 0; keys[ks]; ks++) 	     /* Check for keyword */
  { for (k = keys[ks], pp = spp; *k; k++, pp++)
      if (foldc (*k) != foldc (*pp))
        break;
    if (! *k)	 			     /* Match found */
      break;
  }
  if (! keys[ks]) 			     /* No match */
  { result[resp++] = '%'; 		     /* Treat as text */
    p++;
    return (TEXT);
  }
  p = pp;
  if (ks == TIME) 			     /* Macro */
  { savep = p;
    sresp = resp;
    p = mactime + 1; 			     /* Skip leading bracket */
    s = dofield (mactime[0], tm);
    p = savep;
    return (s);
  }
  /* Match found */
  s = formfld (tm, ks, lead0, prec, &fldp);
  sresp = resp;
  if (s == OPTIONAL)
  { if (query)
      s = FAIL;
    else
      s = SUCCEED;
  }
  if (s == SUCCEED)
  { for (; *fldp; fldp++)
      result[resp++] = *fldp;
    casefix (&result[sresp], upcase);
  }
  return (s);
}

static char
foldc ( char c )
{ return (c < 'a' || c > 'z' ? c : c - 'a' + 'A');
}

static char nstr[30];
static char sfield[30];
static int thmem;

static int
formfld ( struct tm *tm, int field, int lead0, int prec,
                 const char ** fresult )
{
  int fld, ddiff;
  struct tm ctm;
  long fctime;

  fld = -1;
  sfield[0] = '\0';
  *fresult = sfield;
  if (field != TH)
    thmem = -1;
  switch (field)
  {
    case NOON:
      if (tm->tm_hour == 12 && tm->tm_min == 0 && tm->tm_sec == 0)
      { *fresult = strings[SNOON];
	return (SUCCEED);
      }
      return (FAIL);

    case HOUR:
      for (; prec > 2; prec--)
	strcat (sfield, " ");
      if (lead0)
        sprintf (nstr, "%02d", tm->tm_hour);
      else if (prec < 2)
	sprintf (nstr, "%d", (tm->tm_hour + 11) % 12 + 1);
      else
	sprintf (nstr, "%2d", (tm->tm_hour + 11) % 12 + 1);
      strcat (sfield, nstr);
      thmem = tm->tm_hour;
      return (tm->tm_hour >= 0 ? SUCCEED : FAIL);

    case MIN:
      fld = tm->tm_min;
    case SEC:
      if (field == SEC)
	fld = tm->tm_sec;
      if (prec == 0)
	prec = 2;
      for (; prec > 2; prec--)
	strcat (sfield, " ");
      if (prec < 2)
	sprintf (nstr, "%d", fld);
      else
	sprintf (nstr, "%02d", fld);
      strcat (sfield, nstr);
      thmem = fld;
      return (fld > 0 ? SUCCEED : (fld == 0 ? OPTIONAL : FAIL));

    case AM:
      if (tm->tm_hour >= 12)
	*fresult = strings[SPM];
      else if (tm->tm_hour >= 0)
	*fresult = strings[SAM];
      return (tm->tm_hour >= 0 ? SUCCEED : FAIL);

    case WDAY:
      if (tm->tm_wday >= 0 && tm->tm_wday < 7)
      { copyword (sfield, strings[SWDAYS + tm->tm_wday], prec);
	return (SUCCEED);
      }
      return (FAIL);

    case MONTH:
      if (tm->tm_mon >= 0 && tm->tm_mon < 12)
      { copyword (sfield, strings[SMONTHS + tm->tm_mon], prec);
	return (SUCCEED);
      }
      return (FAIL);

    case DAY:
      fld = tm->tm_mday;
    case NMONTH:
    case YDAY:
      if (field == NMONTH)
	fld = tm->tm_mon + 1;
      else if (field == YDAY)
	fld = tm->tm_yday;
      if (prec == 0)
	prec = 1;
      if (lead0)
      { strcpy (nstr, "%09d");
	nstr[2] = prec + '0';
      }
      else
      { strcpy (nstr, "%9d");
	nstr[1] = prec + '0';
      }
      sprintf (sfield, nstr, fld);
      thmem = fld;
      return (fld >= 0 ? SUCCEED : FAIL);

    case TH:
      if (thmem >= 0)
      { if ((thmem / 10) % 10 == 1)
	  *fresult = strings[STH];
	else if (thmem % 10 == 1)
	  *fresult = strings[SST];
	else if (thmem % 10 == 2)
	  *fresult = strings[SND];
	else if (thmem % 10 == 3)
	  *fresult = strings[SRD];
	else
	  *fresult = strings[STH];
	return (SUCCEED);
      }
      return (FAIL);

    case YEAR:
      fctime = time (0);
      ctm = *localtime (&fctime);
      ddiff = (tm->tm_year - ctm.tm_year) * 366 + tm->tm_yday - ctm.tm_yday;
      if (tm->tm_year >= -1900)
      { if (prec == 0 || prec > 3)
	  sprintf (nstr, "%04d", tm->tm_year + 1900);
	else
	  sprintf (nstr, "%02d", (tm->tm_year + 1900) % 100);
	*fresult = nstr;
	return (ddiff < 0 || ddiff > 300 ? SUCCEED : OPTIONAL);
      }
      return (FAIL);

    case MIDNIGHT:
      if (tm->tm_hour == 0 && tm->tm_min == 0 && tm->tm_sec == 0)
      { *fresult = strings[SMIDNIGHT];
	return (SUCCEED);
      }
      return (FAIL);
  }
  return (FAIL);
}

/* copyword: Copy <from> to <to> subject to precision <prec>. Asterisks in
 * <from> mark valid truncation points. The longest string no longer than
 * <prec> and broken at a valid truncation point is returned in <to>. Note
 * that the result may exceed <prec> in length only if there are no valid
 * truncation points short enough. The shortest is then taken.
 * 
 * e.g. Tue*s*day.  Precision 1-3 -> Tue,  Precision 4-6 -> Tues,
 *      Precision 0 and 7... -> Tuesday.
 */

static void
copyword ( char *to, const char *from, int prec )
{ int ast = 0, top = 0;
  if (prec == 0)
    prec = 999;
  for (; *from; from++)
  { if (*from == '*')
      ast = top;
    else
    { to[top] = *from;
      top++;
    }
    if (top > prec && ast > 0)
    { /* Required precision exceeded and asterisk found */
      to[ast] = '\0'; 			     /* Truncate at asterisk */
      return;
    }
  }
  to[top] = '\0';
  return;
}

/* casefix: select case of word. 1: first letter raised. 2: all raised. */

static void
casefix ( char *text, int upcase )
{ if (upcase == 1)
    *text = foldc (*text);
  else if (upcase > 0)
    foldup (text, text);
}
