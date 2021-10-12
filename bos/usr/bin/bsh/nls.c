static char sccsid[] = "@(#)11	1.21  src/bos/usr/bin/bsh/nls.c, cmdbsh, bos411, 9428A410j 4/22/94 18:21:10";
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS: NLSencode NLSencode_stk NLSndecode NLSdecodeargs NLSdecode
 *	      NLSdecode1 NLSndechr NLSfailed NLany nextwc NLpushstak
 *
 * ORIGINS: 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.13  com/cmd/sh/sh/nls.c, cmdsh, bos320, 9141320 10/4/91 10:12:10	
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */

/*   NLS conversion notes                                       DAF

The shell does an enormous amount of string handling, and its control flow is
occasionally quite befuddling.  Since many functions use the high-order bit
for quoting, something must be done to solve its quoting problem.  Rather
than attempting to convert the shell to use NLchars everywhere, which would
require either an enormous number of changes or very frequent conversions
between char strings and NLchar strings, I have come up with another
technique.

In the NLS shell, strings are either "encoded" or "decoded".  The interface
to AIX is always in terms of decoded strings.  When strings are "encoded", a
magic font-shift char (FSH0) is inserted before char in the range
128-255, if they are not already preceded by a font shift.  Also, a magic
header char (FNLS) is inserted at the front of an encoded string to
mark it encoded.   After encoding, the high order bits of all char
except those preceded by font-shifts can be used for the shell's purpose.

In this way, many functions which handle strings continue to work unchanged;
a few are called variously with both encoded and decoded strings.  Some
functions work both ways but need to know what they are working on; the
header char serves that purpose.

The following general truths hold about the shell's subsystems:

	Hash table entries are encoded.
	Estabf() arguments are decoded, except when called by macro().
	Readc(), nextc() and word() return encoded strings.
	All parsing works with encoded strings.
	Shell variables (accessed via lookup) are decoded.
	Arglists are encoded.
	Environments are decoded.
	PATH functions work both ways.
	Printing functions work both ways.
	The variable a1, used througout xec.c, is decoded.  This
	  represents "argv[1]" and used by many builtin commands.
	  Those that require encoded arguments use com[].  Those
	  that require additional decoded arguments must decode them.
	  This includes execa(), which does the central exec() call.
	The current working directory is decoded.
	Temp files created by <<X (copy(), read by subst())
	  are encoded unless terminator X is quoted, disabling substitution
	  in which case temp files are decoded.
	Filenames coming into initio() are encoded
	Filenames coming into io.c are decoded
	Pathlook() contains a kludge avoid the case where PATH is decoded
	and command name is encoded. 

A control maze occurs during macro processing.  The function "estabf()"
takes a char string and makes a pseudo-file, from which char can
be removed by readc().  In general, readc() does encoding as it processes
strings.  "Macro()" calls estabf() with a string that is already encoded (an
argument word to be macro-expanded).  All strings which are macro-expanded
therefore pass through readc() twice!  Rather than asking macro to decode its
argument string so readc() could encode it again -- which would lead to
difficult memory-management problems -- I put a flag in the file descriptor
which disables encoding in readc().

Getch() - called by macro - takes char out of the estabf() files
one by one.  But when it encounters macro strings delimited by $, it
parses the macro, finds the substitution, and inserts it into the estabf()
string.  Since macro variables are decoded, but readc() encoding is disabled,
getch() must encode them before sticking them in the estabf() file.  But it
has to strip off the leading magic char, since reac() is generating a
sequence of char, which are broken into strings down the line.  This
is all done by NLSencode_stk().  I can't believe it all works, but it seems
to.

Another minor problem in split().  This function takes a single work and
creates multiple argument words.  It needs to remove and add magic header
char carefully.

Several other functions look at the first char of a string; they had to
be changed to skip the magic header and look at the second char.  A few
routines which scan through strings had to be adjusted to skip over the
char after a font-shift char.

The debugging flag -D gives a trace of NLS conversions, carefully
displaying magic char, font shifts, etc.

*/

/*	KJI conversion notes					CEG

The modifications made for AIX-KJ used the NLS shell as a base.
The encoding/decoding mechanism is retained although the form of encoding
is different.  In the SJIS char set, single-byte katakana char
have their high bits set as do the shift bytes of 2-byte char; the second
byte of a 2-byte char may or may not have its high bit set.  For the
shell to be able to use its quoting mechanism, these non-ASCII char 
must be escaped by magic font shifts whose high order bits the shell can use for
quoting.  The high order bit of the second byte of every 2-byte char is 
also turned on during encoding to prevent any non-ASCII byte from comparing
equal to an ASCII byte.  Therefore, to perform decoding, we need font shifts
which distinguish the non-ASCII char which follow them:
(FSH21):  a 2-byte SJIS char whose second byte has the high bit on
(FSH20):  a 2-byte SJIS char whose second byte has the high bit off
(FSH0):   a 1-byte SJIS char
The third one is not strictly necessary since it cannot be confused with the 
first byte of a two-byte char, but it simplifies matters to use it.

Another divergence from the NLS shell is in the matter of char 
classification.  The NLS shell considered all non-ASCII char to be
alphanumeric and could therefore classify a char based on its
first (encoded) byte.  However, this is not a reasonable assumption for SJIS,
and code which did this had to be modified to look ahead to determine the
entire char so that it could be compared against valid SJIS ranges.

Double-wide SJIS blanks are recognized as word separators as well as ASCII
blanks and tabs.

No modifications were made to the shell for NLS to enable it to handle
non-ASCII internal field separators.  The KJI shell has been modified 
to do this.

Readvar() was changed to use encoded data since it was previously using 
the high bit quoting mechanism on decoded data, resulting in incorrect results.

Regular expression handling is also different for the AIX-KJ shell.  Within
brackets, range expressions are interpreted according to the current collation
definition; otherwise the char's NLchar value is used.  The char
class extension to the range notation (of the form [:name:]) is also supported.

Also note that some of the files used as the base for KJI had been sanitized
and no longer had NLS-specific code #ifdef'd.  I made no attempt to retrofit
#ifdef NLS in places where I had to make KJI modifications.

*/

/*    ILS conversion notes                     1991 June

The modification made for OSF/1 used the KJI shell as a base.
The encoding/decoding mechanism is retained although the form of encoding
is different.
Internal code of ILS b-shell is shown below. This can theoreticaly handle
any length code. But, at some places in b-shell, input characters are assigned
to integer (4bytes) variable. So the code length that ILS b-shell can handle
is restricted to 1 through 4 bytes.

        character               |       internal code              | byte
--------------------------------+----------------------------------+-------
font-shift-character (1 byte)   |                                  |
        FNLS(0x16), FSH0(0x1d)  | FSH0 + font-shift-character      |  2
        FSH20(0x1e),FSH21(0x1f) |                                  |
--------------------------------+----------------------------------+-------
single byte character (< 0x80)  | character (as it is)             |  1
--------------------------------+----------------------------------+-------
single byte character (> 0x7f)  | FSH0 + character                 |  2
--------------------------------+----------------------------------+-------
multi (m > 1) byte character    | FSH21 + (character size | 0x80)  | m*2+2
                                | + 0x81 + char[0] (if char[0] > 0x7f)|
				|   0x80 + (char[0]|0x80) (if < 0x80) |
                                | + 0x81 + char[1] (if char[1] > 0x7f)|
                                |   0x80 + (char[1]|0x80) (if < 0x80) |
                                | + ...                               |

*/


#include        "defs.h"
#include	<ctype.h>

NLSencode(s1,t1, n)
uchar_t *s1,*t1;
{
	/* encode s1 into t1; s1 is no more than n uchar_ts */

	uchar_t c; 
	register uchar_t *s=s1, *t=t1;
	register uchar_t d;
	register int    mblength;
	*t++ = FNLS;
	--n;			/* for FNLS */
	while (--n && (c = *s++)) {
	mblength = mblen (s-1, MBMAX);
	if (mblength > 1) {
		if ((n -= mblength * 2 + 1) > 0) {
			*t++ = FSH21;
			*t++ = mblength | QUOTE;
			while (mblength--) {
				*t++ = (c & QUOTE) ? (QUOTE | 1) : QUOTE;
				*t++ = c | QUOTE;
				c = *s++;
			}
			s--;
		}
	} else if (NLSneedesc (c) && --n) {
		*t++ = FSH0;
		*t++ = c;
	}
		else
			*t++ = c;
	}
	*t = 0;
#if NLSDEBUG
	debug("NLSencode",t1);
#endif
}

NLSencode_stk(v, quote)
register uchar_t *v; register uchar_t quote;
{
	/* encode v, leaving out FNLS, and push onto stack */
	/* leave stacktop at end of string */
	register uchar_t c;
	register uchar_t d;
	register int    mblength;
	uchar_t *s = staktop;

	while (c = *v++) {
	mblength = mblen(v-1, MBMAX);
	if (mblength > 1) {
		pushstak (FSH21 | quote);
		pushstak (mblength | QUOTE);
		while (mblength--) {
			pushstak ((c & QUOTE) ? (1 | QUOTE) : QUOTE);
			pushstak (c | QUOTE);
			c = *v++;
		}
		v--;
	} else if (NLSneedesc (c)) {
		pushstak (FSH0 | quote);
		pushstak (c);
	}
		else
			pushstak(c | quote);
	}
	*staktop = 0;                            /* for good luck */
#if NLSDEBUG
	debug("NLSencode_stk",s);
#endif
}

#define YUCH_BUFFER 40960 /* 10 pages */

uchar_t *
NLSndecode(s)          /* memory version - use once at a time */
uchar_t *s;
{
        /*
         * If we make the initial buffer big enough (at least
         * MIN_BUF_LEN bytes long), we'll probably never need to
         * reallocate it.
	 *
	 * The objective of this function is to dynamically 
	 * allocate memory based on the size of the buffer
	 * being passed in. If the size of buf is less than 
	 * the size of the buffer passed in we free buf and 
	 * re-malloc it. 
         */

#define MIN_BUF_LEN 4096
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static uchar_t *buf = (uchar_t *) NULL;
static int buf_len  = 0;
int len;

	if ((len = length(s)) > buf_len) {
		buf_len = len;
		if (buf == (uchar_t *) NULL)
			buf_len = MAX(buf_len, MIN_BUF_LEN);
		else
			free(buf);
		if ((buf = (uchar_t *) malloc(buf_len)) == (uchar_t *) NULL)
			error(MSGSTR(M_NOSPACE, (char *) nospace));
	}
	NLSdecode1(s,buf);
	return buf;

}

NLSdecodeargs(argv)
uchar_t **argv;
{
	while(*argv) {
		NLSdecode1(*argv, *argv);
		argv++;
	}
}

NLSdecode(str)                 /* in-place version, modifies arg */
uchar_t *str;
{
	NLSdecode1(str,str);
}

NLSdecode1(str,target)
uchar_t    *str,*target;
{

	register uchar_t   c;
	register uchar_t   *p = str;
	/*
	 * Convert sh's internal format for NLS strings to the standard
	 * system format, by turning off high order bit normally, turning
	 * it on after font shift characters, and eliminating the
	 * pseudo-font shift FSH0.  This function can replace "trim()"
	 * except for the side-effect of setting "nosubst".
	 */
	register int    mblength;

	if (p)
	{
		if (*p == FNLS)
		       NLSskiphdr(p);
#ifdef NLSDEBUG
		else debug("Decode - not encoded",p);
#endif
		while (c = *p++)
		{
			if (c==FNLS) {
#ifdef NLSDEBUG
				debug("Decode - xtra FNLS",p);
#endif
				continue;
			}
			c &= STRIP;
			if (c == FSH21) {
				mblength = *p++ & STRIP;
				while (mblength--) {
					if (*p++ & STRIP) {
						*target++ = *p++;
					} else {
						*target++ = *p++ & STRIP;
					}
				}
			} else if (c == FSH0) {
				if (*p < 128)
					*target++ = *p++ & STRIP;
				else
					*target++ = *p++;
			}
			else if (c)
				*target++ = c;
		}
		*target = 0;
	}
}

/*
 * Decode a single character pointed to by p into a static uchar_t array.
 */
uchar_t *
NLSndechr (p)
register uchar_t *p;
{
	register uchar_t c = *p++;
	static uchar_t   ch[MBMAX];
	register uchar_t *target = ch;
	register int     mblength;

	c &= STRIP;
	if (c == FSH21) {
		mblength = *p++ & STRIP;
		while (mblength--) {
			if (*p++ & STRIP) {
				*target++ = *p++ | QUOTE;
			}else{
				*target++ = *p++;
			}
		}
	} else if (c == FSH0) {
		if (*p < 128)
			*target = *p & STRIP;
		else
			*target = *p;
	} else
		*target = c;
	return (ch);
}


#ifdef NLSDEBUG
debug(s,t)
uchar_t *s, *t;
{
	uchar_t z[512], buf[512];
	register uchar_t c;
	register uchar_t *p = t;
	register uchar_t *q = z;
	if (!(flags & debugflg)) return;
	if (*s == 0) return;
	while(c = *p++) {
		if (c < 040) {
			*q++ = '^';
			*q++ = c + 040;
		}
		else if (c==FSH0) {
			*q++ = '#';
			continue;
		}
		else *q++ = c;
	}
	*q = 0;
	sprintf(buf, "%s: %s[%d]\n", s, z, strlen(t));
	write(2,buf,strlen(buf));
}
#endif

NLSfailed(s1, s2)
uchar_t    *s1, *s2;
{
	NLSdecode(s1);
	failed(s1,s2);
}


/*	Given an encoded n-byte character c, NLany returns true if there is 
 *	an occurrence of c in the (encoded) string s.
 */
NLany (c, s)
uchar_t *c, *s;
{
    register uchar_t *p = s;
    register uchar_t *c2;
    register int   n = NLSenclen(c);
    register int   i;

    while (*p) {
        if (*p != *c) {
        	p+=NLSenclen(p);
	} else {
		for (i = 0, c2 = c; i < n; i++) 
		    if (*p++ != *c2++)
			break;
		if (i == n)
	        return (1);
    	}
    }
    return (0);
}


/*	Return next one- or two-byte encoded character. */

uchar_t *
nextwc()
{
    static   uchar_t  ch[MBMAX*2+2];
    register uchar_t *c = ch;
    register int	mblength;

    *c = nextc(0);
    if (*c == FSH0)
	*(++c) = nextc(0);
    else if (*c == FSH21) {
	*(++c) = nextc(0);
	mblength = *c & STRIP;
	while (mblength--) {
	    *(++c) = nextc(0);
	    *(++c) = nextc(0);
        }
    }
    return (ch);
}

/*	Push a 1 or 2-byte character onto the stack.	*/

NLpushstak (c)
register unsigned int c;
{
    uchar_t		uc;
    unsigned int	i, mblength;

    if (c < 0xff)
	pushstak (c);
    else {
	i = MBCDMAX >> 8;
	mblength = MBMAX;
	while (c < i) {
		i = (i >> 8);
		mblength--;
	}
	pushstak (mblength | QUOTE);
	while (mblength--) {
		uc = (c >> (8 * mblength));
		if (uc & QUOTE) {
			pushstak (1 | QUOTE);
		} else {
			pushstak (QUOTE);
	}
	pushstak (uc | QUOTE);
    }
   }
}
