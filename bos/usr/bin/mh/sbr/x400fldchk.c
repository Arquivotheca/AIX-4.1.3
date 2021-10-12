static char sccsid[] = "@(#)13	1.7  src/bos/usr/bin/mh/sbr/x400fldchk.c, cmdmh, bos411, 9428A410j 3/27/91 17:53:58";
/*
 * COMPONENT_NAME: CMDMH x400fldchk.c
 *
 * FUNCTIONS: strsrch, cicmp, get_header, isnumbers, isblank,
 *            rmparens, intrser, askuser, arpadate, x400fldchk
 *
 * ORIGINS: 10 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 * x400fldchk.c - Do X400 extended header sanity checking.
 * Addresses are not handled (but could be).
 */

#include <stdio.h>
#include <ctype.h>
#include "../h/mh.h"
/* #ifndef SYS5 */
#include <sys/types.h>
/* #endif */
#include <time.h>
#ifdef	BSD42
#include <setjmp.h>
#endif	/* BSD42 */
#include <signal.h>
#include <stdio.h>
#include "../zotnet/tws.h"

#include "mh_msg.h"
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s)

#ifndef BSD42
static	int interrupted;
#else	/* BSD42 */
static	jmp_buf sigenv;
#endif	/* BSD42 */

char *strchr();
char *parsetime();

struct headers {
    char *value;		/* must be all lowercase */
    unsigned int flag;
#define UNIQUE	0x0001		/* Header must appear no more than once */
    unsigned int set;
};

static struct headers hdrtab[] = {
    "priority", UNIQUE, 0, 
#define PRIORITY 0
    "obsoletes", 0, 0, 
#define OBSOLETES 1
    "expiry-date", UNIQUE, 0, 
#define EXPIRY_DATE 2
    "reply-by", UNIQUE, 0, 
#define REPLY_BY 3
    "importance", UNIQUE, 0, 
#define IMPORTANCE 4
    "sensitivity", UNIQUE, 0, 
#define SENSITIVITY 5
    "to", UNIQUE, 0,
#define TO 6
    "cc", UNIQUE, 0,
#define CC 7
    "delivery-report", UNIQUE, 0,
#define DELIVERYREPORT 8
    NULLCP, 0, 0
};

static void intrser(int);

static struct swit priorityswitches[] = {
    "normal", 0, 
    "non-urgent", 0, 
    "urgent", 0, 
    NULLCP, (int)NULL
};

static struct swit importanceswitches[] = {
    "low", 0, 
    "normal", 0, 
    "high", 0, 
    NULLCP, (int)NULL
};

static struct swit sensitivityswitches[] = {
    "personal", 0, 
    "private", 0, 
    "company-confidential", 0, 
    NULLCP, (int)NULL
};

static struct swit deliveryreportswitches[] = {
    "basic", 0, 
    "none", 0, 
    "confirmed", 0, 
    NULLCP, NULL
};



/* strsrch - look for tag in buf.
 * Return pointer to first char of match in buf, or NULL, if not found.
 * This is not exactly the most clever string search...
 */
static char *
strsrch(tag, buf)
register char *tag;
register char *buf;
{
    register len;

    len = strlen(tag);
    while (*buf != (char)NULL) {
	if (strncmp(tag, buf, len) == 0)
	    return buf;
	buf++;
    }
    return NULL;
}

/* Matches lowercase c1 to anycase c2 */
static int
cicmp(c1, c2)
register char *c1, *c2;
{
    while (*c1 != '\0') {
	if (*c1 != (isupper(*c2) ? tolower(*c2) : *c2))
	    return 0;
	c1++;
	c2++;
    }
    return *c2 == '\0';
}

/* find header name in table, return index or NOTOK */
static int
get_header(header, table)
register char *header;
register struct headers *table;
{
    register struct headers *h;

    for (h = table; h->value; h++)
	if (cicmp(h->value, header))
	    return(h - table);
    return NOTOK;
}

/* isnumbers - return OK if a string contains only digits 0-9, white space */
static int
isnumbers(p)
register char *p;
{
    while (*p != '\0') {
	if (!isdigit(*p) && *p != '\n' && *p != '\040')
	    return NOTOK;
	p++;
    }
    return OK;
}

/* isblank - return OK if a string contains only white space */
static int
isblank(p)
register char *p;
{
    if (p == NULLCP)
	return OK;
    while (*p != '\0') {
	if (*p != '\n' && *p != '\040')
	    return NOTOK;
	p++;
    }
    return OK;
}

/* rmparens - remove parenthesis and anything in them.
 * replace with at least one space.
 * parens nest: "a(b(c)d)e" strips to "a e".
 * and		"a(b)d)e" strips to "a )e".
 */
static void
rmparens(p)
register char *p;
{
    register char *q;
    register char *r;
    register cnt;

    q = getcpy(p);
    cnt = 0;
    for (r = q; *r; r++) {
	if (*r == '(') {
	    *p++ = '\040';	/* one space */
	    cnt++;
	    r++;
	    while (*r != '\0') {
		if (*r == '(') {
		    cnt++;
		} else if (*r == ')') {
		    if (--cnt == 0) {
			r++;
		        break;
		    }
		}
		r++;
	    }
	}
	*p++ = *r;
    }
    *p++ = '\0';
    free(q);
}

static	void
intrser(int s)
{
#ifndef BSD42
    (void) signal(SIGINT, (void(*)(int))intrser);
    interrupted = 1;
#else	/* BSD42 */
    longjmp(sigenv, NOTOK);
#endif	/* BSD42 */
}

/* askuser - get an answer from the user and return the string answer.
 * The routine that interogates users for answers.
 * If 'ansp' is NULL answer is simply accumulated and returned in a
 * string.  Otherwise, help is available.  Note the help is just
 * a list of recognized strings.  Priority, etc., also allow numbers.
 * Note: Currently the NULL 'ansp' is exclusively used to
 * enquire time/date.
 */
static char *
askuser(prompt, ansp)
char *prompt;
struct swit *ansp;
{
    int i;
    char *cp;
    static char ansbuf[BUFSIZ];
    void(*istat)(int);

#ifndef BSD42
    interrupted = 0;
    istat = signal(SIGINT, (void(*)(int))intrser);
#else	/* BSD42 */
    switch(setjmp(sigenv)) {
    case OK:
	istat = signal(SIGINT, (void(*)(int))intrser);
	break;

    default:
	(void) signal(SIGINT, istat);
	return NULLCP;
    }
#endif	/* BSD42 */
    for (;;) {
	printf("%s", prompt);
	cp = ansbuf;
	while ((i = getchar()) != '\n') {
#ifndef BSD42
	    /* Return nothing if an interrupt character is received */
	    if (i == EOF || interrupted) {
		interrupted = 0;
		(void) signal(SIGINT, istat);
		return NULLCP;
	    }
#else	/* BSD42 */
	    if (i == EOF)
		longjmp(sigenv, DONE);
#endif	/* BSD42 */
	    if (cp < &ansbuf[sizeof ansbuf - 1])
		*cp++ = i;
	}
	*cp = 0;
	if (ansbuf[0] == '?' || cp == ansbuf) {
	    if (ansp != (struct swit *)NULL) {
		printf(MSGSTR(OPTIONS,"Options are:\n"));
		printsw(ALL, ansp, "");
		printf(MSGSTR(NUMBER1,"or a number\n"));
		continue;
	    } else {
		printf(MSGSTR(LIKE,"Something like: Jan 10 10:30:01 1990\n"));
		continue;
	    }
	}
	return ansbuf;
    }
}

#if 0
/* ARPADATE -- Create date in ARPANET format
 *
 *	Parameters:
 *		ud -- unix style date string.  If NULL, one is created.
 *
 *	Returns:
 *		pointer to an ARPANET date field
 *
 *	Side Effects:
 *		none
 *
 *	WARNING:
 *		date is stored in a local buffer -- subsequent
 *		calls will overwrite.
 *
 *	Bugs:
 *		Timezone is computed from local time, rather than
 *		from whereever(and whenever) the message was sent.
 *		To do better is very hard.
 *
 *		Some sites are now inserting the timezone into the
 *		local date.  This routine should figure out what
 *		the format is and work appropriately.
 */
char *
arpadate(ud)
register char *ud;
{
    register char *p;
    register char *q;
    register int off;
    register int i;
    register struct tm *lt;
    time_t t;
    struct tm gmt;
    static char b[40];
    extern struct tm *localtime(), *gmtime();
    extern char *ctime();
    extern time_t time();

/*	Get current time.
 *	This will be used if a null argument is passed and
 *	to resolve the timezone.
 */
    (void) time(&t);
    if (ud == NULL)
	ud = ctime(&t);

/* Crack the UNIX date line in a singularly unoriginal way. */
    q = b;

    p = &ud[0];		/* Mon */
    *q++ = *p++;
    *q++ = *p++;
    *q++ = *p++;
    *q++ = ',';
    *q++ = ' ';

    p = &ud[8];		/* 16 */
    if (*p == ' ')
	p++;
    else
	*q++ = *p++;
    *q++ = *p++;
    *q++ = ' ';

    p = &ud[4];		/* Sep */
    *q++ = *p++;
    *q++ = *p++;
    *q++ = *p++;
    *q++ = ' ';

    p = &ud[22];	/* 79 */
    *q++ = *p++;
    *q++ = *p++;
    *q++ = ' ';

    p = &ud[11];	/* 01:03:52 */
    for (i = 8; i > 0; i--)
	*q++ = *p++;

/* should really get the timezone from the time in "ud"(which
 * is only different if a non-null arg was passed which is different
 * from the current time), but for all practical purposes, returning
 * the current local zone will do(its all that is ever needed).
 */
    gmt = *gmtime(&t);
    lt = localtime(&t);

    off =(lt->tm_hour - gmt.tm_hour) * 60 + lt->tm_min - gmt.tm_min;

    /* assume that offset isn't more than a day ... */
    if (lt->tm_year < gmt.tm_year)
	off -= 24 * 60;
    else if (lt->tm_year > gmt.tm_year)
	off += 24 * 60;
    else if (lt->tm_yday < gmt.tm_yday)
	off -= 24 * 60;
    else if (lt->tm_yday > gmt.tm_yday)
	off += 24 * 60;

    *q++ = ' ';
    if (off == 0) {
	*q++ = 'G';
	*q++ = 'M';
	*q++ = 'T';
    } else {
	if (off < 0) {
	    off = -off;
	    *q++ = '-';
	} else {
	    *q++ = '+';
	}
	if (off >= 24*60)		/* should be impossible */
	    off = 23*60+59;		/* if not, insert silly value */

	*q++ =(off / 600) + '0';
	*q++ =(off / 60) % 10 + '0';
	off %= 60;
	*q++ =(off / 10) + '0';
	*q++ =(off % 10) + '0';
    }
    *q = '\0';

    return(b);
}
#endif

/* x400fldchk - Do X400 extended header sanity checking.
 * Addresses are not handled(but could be).
 */
char *
x400fldchk(name, buf, pushflg, x400st)
register char *name;
register char *buf;
register int pushflg;
register char *x400st;
{
    register i, nsw;
    time_t tp;
    char *ctimep, *swp, *errp;
    char **cpp;
    struct swit *switp;
    char *namep;

    /* remove leading whitespace */
    while (*buf == '\040')
	buf++;

    /* return NULLCP if the buffer contents are empty */
    if (isblank(buf) == OK)
	return NULLCP;
#if 0
    printf(MSGSTR(HAEDBUF,"header buffer='%s'\n"), buf);
#endif
    i = get_header(name, hdrtab);
    if (i != NOTOK) {

        /* Remove '\n' from the 'buf' string if exist */
        if (buf != NULLCP)
	    if ((swp = strrchr(buf, '\n')) != NULLCP)
	        *swp = '\0';

	if (hdrtab[i].flag & UNIQUE) {
	    if (hdrtab[i].set & UNIQUE) {
		return NULLCP;
	    } else {
		hdrtab[i].set |= UNIQUE;
	    }
	}
    }
retry:
    switch(i) {
    case TO:
    case CC:
	if ((strsrch("/C=", buf) == NULL) || (strsrch("/ADMD=", buf) == NULL))
	    break;	/* must not be X400 */
	swp = getcpy(buf);
	goto done;
    case NOTOK:
	break;
    case PRIORITY:
    case IMPORTANCE:
    case SENSITIVITY:
    case DELIVERYREPORT:
	switch (i) {
	case PRIORITY:
	    switp = priorityswitches;
	    namep = "Priority";
	    break;
	case IMPORTANCE:
	    switp = importanceswitches;
	    namep = "Importance";
	    break;
	case SENSITIVITY:
	    switp = sensitivityswitches;
	    namep  = "Sensitivity";
	    break;
	case DELIVERYREPORT:
	    switp = deliveryreportswitches;
	    namep  = "Delivery-Report";
	    break;
	}
	rmparens(buf);
	if (isblank(buf) == OK)
	    return NULLCP;
	cpp = brkstring(buf, " ", NULLCP);
	nsw = smatch(*cpp, switp);
	switch(nsw) {
	case AMBIGSW:
	    advise(NULLCP, MSGSTR(AMBIG,"%s ambiguous.  It matches"), *cpp);
	    printsw(*cpp, switp, "");
	    advise(NULLCP, MSGSTR(NUMBER2,"or a number."));
	    break;
	case UNKWNSW:
	    if (cpp == NULL || *cpp == NULL || **cpp == '\0' ||
		    isblank(*cpp) == OK)
		return NULLCP;
	    if (isnumbers(*cpp) == NOTOK) {	/* numbers are OK */
		printf(MSGSTR(NOKNOWN,"%s '%s' unknown.\n"), namep, *cpp);
		if (pushflg == 0)
		    printf(MSGSTR(HELPX400,"Hit <CR> for help.\n"));
		break;
	    } else {
		strcpy(buf, *cpp);
		goto done;
	    }
	default:
	    swp = switp[nsw].sw;
	    if (sizeof(buf) >= strlen(swp)) {
		(void) strcpy(buf, swp);
	    } else {
		buf = getcpy(swp);
	    }
	    goto done;
	}
	/* Field contents are inapropriate */
	if (pushflg == 0) { /* Is it called through 'push' ? */
	    /* - No */
	    char b[80];
	    sprintf(b, MSGSTR(REENTER1,"Please re-enter the %s field: "), namep);
	    if ((buf = askuser(b, switp)) == NULLCP)
		return NULLCP;
	    goto retry;
	} else {		/* - Yes */
	    *x400st = 1;	/* Set the status to bad */
#if 0	/* modify the original draft for push - just wrong */
	    strcpy(buf, namep);
	    strcpy(buf, MSGSTR(ISBAD," is bad\n"));	/* Return the bad string */
#endif
	    goto done;
	}
    case OBSOLETES:
	break;
    case EXPIRY_DATE:
    case REPLY_BY:
	rmparens(buf);
	if (isblank(buf) == OK)
	    return NULLCP;
	/* Try to make sense out of the given time */
	errp = parsetime(buf, &tp);
	if (errp != NULL) {	/* non-NULL is error, with error message */
	    printf(MSGSTR(PROBLEM,"Problem with %s '%s': %s\n"),
		(i == EXPIRY_DATE) ? "Expiry-Date" : "Reply-By",
		buf, errp);
	    if (pushflg == 0) { /* Is it called through 'push' ? */
		/* - No: interogate user */
		if ((buf = askuser(MSGSTR(REENTER2,"Please re-enter the date: "), NULLCP))
			== NULLCP) {
		    return NULLCP;
		}
		goto retry;
	    } else {		/* - Yes */
		*x400st = 1;	/* Set the status to bad */
#if 0	/* modify the original draft for push - just wrong */
		sprintf(&buf[strlen(buf)], "(%s)", errp);
#endif
		goto done;
	    }
	} else {		/* parsetime ; NULL = good */
#if 1
	    /* Make ARPA style time from time. */
	    buf = dtime(&tp);
#else
	    /* Convert the time to UNIX time format */
	    if ((ctimep = ctime(&tp)) == NULLCP) {
		perror(MSGSTR(CTIME,"ctime() failed, this header is ignored"));
		return NULLCP;
	    } else {		/* Convert the time to ARPA format */
		buf = arpadate(ctimep);
	    }
#endif
	} /* parsetime ; NULL = good */
	break;
    default:
	break;
    }
done:
    if (buf != NULLCP)
	if ((swp = strchr(buf, '\n')) == NULLCP) {
	    strcat(buf, "\n");
	}
    return buf;
}
