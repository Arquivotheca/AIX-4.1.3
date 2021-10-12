static char sccsid[] = "@(#)46	1.12.1.13  src/bos/usr/ccs/lib/libc/__regcomp_std.c, libcpat, bos41J, 9511A_all 2/23/95 17:04:34";
/*
 *   COMPONENT_NAME: libcpat
 *
 *   FUNCTIONS: CLEARBIT
 *		CLEARBITC
 *		OPPOSITE
 *		OVERFLOW
 *		SETBIT
 *		SETBITC
 *		__regcomp_std
 *		bracket
 *		bracketw
 *		enlarge
 *		intl_expr
 *		wgtstring
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#define _ILS_MACROS
#include <sys/types.h>
#include <sys/localedef.h>
#include <ctype.h>
#include <limits.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include "reglocal.h"
#include "patlocal.h"
#include "libc_msg.h"

/* get the weight string for the character with index "index" */
#define wgtstring(index) __OBJ_DATA(hdl)->co_subs[index].tgt_wgt_str[0]


/************************************************************************/
/* External data defined in regexec()                                   */
/************************************************************************/

extern  int     __reg_bits[];   /* bitmask for [] bitmap                */


/************************************************************************/
/* RE compilation definitions						*/
/************************************************************************/

			/* expand pattern if buffer too small		*/
#define OVERFLOW(x)	{						\
			if (pe - pp < x)				\
			  {						\
			  enlarge(x, &pp_start, &pe, &pp, &plastce);	\
			  if (pp_start == NULL)				\
			    {						\
			    preg->re_erroff =				\
			      (uchar_t *)ppat - (uchar_t *)pattern - 1;	\
			    return (REG_ESPACE);			\
			    }						\
			  }						\
			}

#define PATTERN_EXP	128	/* compiled pattern expansion (bytes)	*/

			/* remove interval, uncouple last STRING char	*/
#define REPEAT_CHECK	{						\
			if (((*plastce & CR_MASK) == CR_INTERVAL) ||    \
			    ((*plastce & CR_MASK) == CR_INTERVAL_ALL))  \
			  {						\
			  pp -= 2;					\
			  pto = plastce + 1;				\
			  pfrom = pto + 2;				\
			  do						\
			    *pto++ = *pfrom++;				\
			  while (pto > pp);				\
			  }						\
			else if (*plastce == CC_STRING)			\
			  {						\
			  plastce[1]--;					\
			  plastce = pp - 1;				\
			  *pp = pp[-1];					\
			  pp[-1] = CC_CHAR;				\
			  pp++;						\
			  }						\
			else if (*plastce == CC_I_STRING)		\
			  {						\
			  plastce[1]--;					\
			  plastce = pp - 2;				\
			  *pp = pp[-1];					\
			  pp[-1] = pp[-2];				\
			  pp[-2] = CC_I_CHAR;				\
			  pp++;						\
			  }						\
			}

			/* set character bit in C [] bitmap		*/
#define SETBITC(pp,c)	{						\
			*(pp + (c >> 3)) |= __reg_bits[c & 7];		\
			}


			/* clear character bit in C [] bitmap		*/
#define CLEARBITC(pp,c)	{						\
			*(pp + (c >> 3)) &= ~(__reg_bits[c & 7]);	\
			}

			/* set u.c.w. bit in ILS [] bitmap		*/
#define SETBIT(pp,ucoll)						\
                        {						\
                        delta = ucoll - MIN_UCOLL;			\
                        *(pp + (delta >> 3))				\
                          |= __reg_bits[delta & 7];			\
                        }

			/* clear u.c.w. bit in ILS [] bitmap		*/
#define CLEARBIT(pp,ucoll)						\
                        {						\
                        delta = ucoll - MIN_UCOLL;			\
                        *(pp + (delta >> 3))				\
                          &= ~(__reg_bits[delta & 7]);			\
                        }

			/* set opposite case u.c.w. bit in ILS bitmap	*/
#define OPPOSITE(icase,pp,ucoll,wc,wc2)					\
			{						\
			if (icase != 0)					\
			  if (((wc2 = towupper(wc)) != wc) ||		\
			      ((wc2 = towlower(wc)) != wc))		\
			    {						\
			    ucoll = __wcuniqcollwgt(wc2);		\
			    if (ucoll >= MIN_UCOLL && ucoll <= MAX_UCOLL)\
			      SETBIT(pp, ucoll);			\
			    }						\
			}


/************************************************************************/
/* Internal function prototypes						*/
/************************************************************************/

static	int	bracket(	/* convert [bracket] to bitmap		*/
			uchar_t *,
			uchar_t **,
			uchar_t *,
			int);

static	void	enlarge(	/* enlarge compiled pattern buffer	*/
			int,
			uchar_t **,
			uchar_t **,
			uchar_t **,
			uchar_t **);

static	int	bracketw(	/* convert [bracket] to bitmap		*/
			_LC_collate_objhdl_t,
			uchar_t *,
			uchar_t **,
			uchar_t *,
			regex_t *,
			uchar_t *);

static	int	intl_expr(	/* decode internationalization []	*/
			_LC_collate_objhdl_t,
			regex_t *,
			uchar_t,
			uchar_t *,
			uchar_t *,
			wchar_t *,
			wchar_t *);

/************************************************************************/
/* __regcomp_std()- Compile RE pattern					*/
/*		  - valid for all locales and any codeset 		*/
/*									*/
/*		  - hdl		ptr to __lc_collate table 		*/
/*		  - preg	ptr to structure for compiled pattern	*/
/*		  - pattern	ptr to RE pattern			*/
/*		  - cflags	regcomp() flags				*/
/************************************************************************/

int
__regcomp_std(_LC_collate_objhdl_t hdl, regex_t *preg, const char *pattern, 
	      int cflags)
{
	int	altloc[_REG_SUBEXP_MAX+1]; /* offset to last alternate	*/
	int	be_size;	/* [bracket] bitmap size		*/
	int	c;		/* pattern character			*/
	int	c2;		/* opposite case pattern character	*/
	int	delta;		/* SETBIT unique collating value offset */
	int	do_all;		/* set if {m,} is used.			*/
	int	eol[_REG_SUBEXP_MAX+1]; /* EOL anchor offset in pattern	*/
	int	ere;		/* extended RE flag			*/
	int	first;		/* logical beginning of pattern		*/
	int	first_BOL;	/* set when the first ^ is found	*/
	int	i;		/* loop index				*/
	int	icase;		/* ignore case flag			*/
	int	idx;		/* current subexpression index		*/
	int	isfirst;	/* first expression flag		*/
	int	maxri;		/* mamimum repetition interval		*/
	int	mb_cur_max;	/* in memory copy of MB_CUR_MAX		*/
	int	minri;		/* minimum repetition interval		*/
	int	nsub;		/* highest subexpression index		*/
	uchar_t	*palt;		/* expand pattern ptr			*/
	uchar_t	*pe;		/* ptr to end of compiled pattern space	*/
	uchar_t	*pfrom;		/* expand pattern ptr			*/
	uchar_t	*plastce;	/* ptr to last compiled expression	*/
	uchar_t	*pmap;		/* ptr to character map table		*/
	uchar_t	*pp;		/* ptr to next compiled RE pattern slot	*/
	uchar_t	*pp_start;	/* ptr to start of compiled RE pattern	*/
	uchar_t	*ppat;		/* ptr to next RE pattern byte		*/
	uchar_t	*pri;		/* ptr to repetition interval		*/
	uchar_t	*psubidx;	/* ptr to current subidx entry		*/
	uchar_t	*pto;		/* expand pattern ptr			*/
	int	sblocale;	/* is this a single byte locale?	*/
	int	wclen;		/* length of character			*/
	uchar_t	sol[_REG_SUBEXP_MAX+1]; /* don't clear "first"		*/
	int	stat;		/* bracket() return status		*/
	uchar_t	subidx[_REG_SUBEXP_MAX+1]; /* active subexpression index*/
	wchar_t	wc;		/* a wide character			*/
	wchar_t	wc2;		/* opposite case pattern wide character	*/

/*
 * Allocate initial RE compiled pattern buffer
 * OVERFLOW(X) will expand buffer as required
 */
	pmap = (uchar_t *)malloc(256*(sizeof(uchar_t)));
	pp = (uchar_t *)malloc(PATTERN_EXP);
	if (pp == NULL || pmap == NULL)
		{
		preg->re_erroff = 0;
		return (REG_ESPACE);
		}
	pp_start = pp;
	pe = pp + PATTERN_EXP - 1;
/*
 * Other initialization
 */
	bzero(preg, sizeof(regex_t));
	preg->re_cflags = cflags;
	preg->re_ucoll[0] = MIN_UCOLL;
	preg->re_ucoll[1] = MAX_UCOLL;
	icase = cflags & REG_ICASE;
	ere = cflags & REG_EXTENDED;
	nsub = 0;
	plastce = NULL;
	preg->re_lsub[0] = 0;
	psubidx = subidx;
	*psubidx = 0;
	altloc[0] = 0;
	idx = 0;
	first = 0;
	first_BOL = 0;
	isfirst = 0;
	eol[0] = 0;
	sol[0] = 0;
	preg->re_map = pmap;
	mb_cur_max = MB_CUR_MAX;
	if (mb_cur_max == 1)
		{
		sblocale = 1;
		wclen = 1;
		}
	else
		sblocale = 0;
/*
 * BIG LOOP to process all characters in RE pattern
 * stop on NUL
 * return on any error
 * set character map for all characters which satisfy the pattern
 * expand pattern space now if large element won't fit
 */
	ppat = (char *)pattern;
	while ((c = *ppat++) != '\0')
		{
		OVERFLOW(10)
		switch(c)
			{
/*
 * match a single character
 *   error if preceeded by ERE $
 *   if multibyte locale, set wclen and get wide character
 *      otherwise wclen is always set to 1
 *   if case sensitive pattern
 *     if single byte character
 *       if no previous pattern, add CC_CHAR code to pattern
 *       if previous pattern is CC_CHAR, convert to CC_STRING
 *       if previous pattern is CC_STRING, add to end of string
 *       otherwise add CC_CHAR code to pattern
 *     if multibyte character
 *       add CC_WCHAR code to pattern
 *     
 *   if ignore case pattern
 *     if single byte character
 *       determine opposite case of pattern character
 *       if no opposite case, treat as case sensitive
 *       if no previous pattern, add CC_I_CHAR code to pattern
 *       if previous pattern is CC_I_CHAR, convert to CC_I_STRING
 *       if previous pattern is CC_I_STRING, add to end of string
 *       otherwise add CC_I_CHAR code to pattern
 *     if multibyte character
 *       determine opposite case of pattern character
 *       if no opposite case, process as case sensitive
 *         otherwise add CC_I_WCHAR code to pattern
 */

		default:
		cc_char:
			if (sblocale == 0)
				{
				wclen = mbtowc(&wc, ppat-1, mb_cur_max);
				if (wclen < 0)
					{
					preg->re_erroff = ppat - pattern - 1;
					return (REG_ECHAR);
					}
				}
			if (icase == 0)
				{
				if (wclen == 1)
					{
					if (plastce == NULL)
						{
						plastce = pp;
						*pp++ = CC_CHAR;
						*pp++ = c;
						if (isfirst++ == 0)
							pmap[c] = 1;
						}
					else if (*plastce == CC_CHAR)
						{
						*plastce = CC_STRING;
						*pp++ = plastce[1];
						plastce[1] = 2;
						*pp++ = c;
						}
					else if (*plastce == CC_STRING && plastce[1] < 255)
						{
						plastce[1]++;
						*pp++ = c;
						}
					else
						{
						plastce = pp;
						*pp++ = CC_CHAR;
						*pp++ = c;
						if (isfirst++ == 0)
							pmap[c] = 1;
						}
					} /* if wclen == 1 */
				else
					{ /* multibyte character */
					cc_wchar : 
					plastce = pp;
					*pp++ = CC_WCHAR;
					*pp++ = wclen;
					*pp++ = c;
					while (--wclen > 0)
						*pp++ = *ppat++;
					if (isfirst++ == 0)
						pmap[c] = 1;
					}
				} /* if icase == 0 */
			else
				{
				if (wclen == 1)
					{
					c2 = toupper(c);
					if (c2 == c)
						c2 = tolower(c);
					if (plastce == NULL)
						{
						plastce = pp;
						*pp++ = CC_I_CHAR;
						*pp++ = c;
						*pp++ = c2;
						if (isfirst++ == 0)
							{
							pmap[c] = 1;
							pmap[c2] = 1;
							}
						}
					else if (*plastce == CC_I_CHAR)
						{
						*plastce = CC_I_STRING;
						*pp++ = plastce[2];
						plastce[2] = plastce[1];
						plastce[1] = 2;
						*pp++ = c;
						*pp++ = c2;
						}
					else if (*plastce == CC_I_STRING && plastce[1] < 255)
						{
						plastce[1]++;
						*pp++ = c;
						*pp++ = c2;
						}
					else
						{
						plastce = pp;
						*pp++ = CC_I_CHAR;
						*pp++ = c;
						*pp++ = c2;
						if (isfirst++ == 0)
							{
							pmap[c] = 1;
							pmap[c2] = 1;
							}
						}
					} /* if single byte char */
				else  
					{ /* multibyte case */
					if (((wc2 = towupper(wc)) == wc) &&
					    ((wc2 = tolower(wc)) == wc))
						goto cc_wchar;
					plastce = pp;
					*pp++ = CC_I_WCHAR;
					*pp++ = wclen;
					*pp++ = c;
					while (--wclen > 0)
						*pp++ = *ppat++;
					wclen = wctomb(pp, wc2);
					if (wclen < 0)
						{
						wclen = 1;
						*pp = wc2;
						}
					if (isfirst++ == 0)
						{
						pmap[c] = 1;
						pmap[*pp] = 1;
						}
					pp += wclen;
					}
				} /* if case sensitive */
			first++;
			continue;
/*
 * If we can use the smaller CC_BITMAP, use it:
 *   bracket expression
 *     always use 256-bit bitmap - indexed by file code
 *     decode pattern into list of characters which satisfy the [] expression
 *     error if invalid [] expression
 *     add CC_BITMAP to pattern
 *     set character map for each bit set in bitmap
 * otherwise
 *   ILS bracket expression
 *     bitmap size is based upon min/max unique collating value
 *     zero fill bitmap - yes its big for kanji
 *     error if invalid bracket expression
 *     add CC_WBITMAP
 *     set character map for each bit set in bitmap, however must
 *       convert bits from unique collation weight to file code
 *       Note: only use first byte of multibyte languages
 */
		case '[':
			if ((strcmp(setlocale(LC_COLLATE,NULL),"C") == 0) || ((__OBJ_DATA(hdl)->co_coltbl == NULL) && (MAX_PC - MIN_PC < BITMAP_LEN * NBBY))) 
				{
				OVERFLOW(BITMAP_LEN+1)
				plastce = pp;
				*pp++ = CC_BITMAP;
				bzero(pp, BITMAP_LEN);
				stat = bracket(ppat, &pto, pp, cflags);
				if (stat != 0)
					{
					preg->re_erroff = pto - pattern - 1;
					return (stat);
					}
				ppat = pto;
				pto = pp;
				pp += BITMAP_LEN;
				if (isfirst++ == 0)
					{
					pfrom = pmap;
					do
						{
						if (*pto != 0)
							for (i=0; i<8; i++)
								if ((*pto & __reg_bits[i]) != 0)
									pfrom[i] = 1;
						pfrom += 8;
						}
					while (++pto < pp);
					}
				}
			else	/* now do CC_WBITMAP */
				{
				be_size = ((MAX_UCOLL - MIN_UCOLL) / NBBY) + 1;
				OVERFLOW(be_size+1)
				bzero(pp+1, be_size);
				stat = bracketw(hdl, ppat, &pto, pp+1, preg,
					isfirst == 0 ? pmap : NULL);
				if (stat != 0)
					{
					preg->re_erroff = (char *)pto - (char *)pattern;
					return (stat);
					}
				ppat = pto;
				plastce = pp;
				*pp++ = CC_WBITMAP;
				if (isfirst++ == 0)
					{
					wchar_t	ucoll;	/* unique collating value	*/
					wchar_t min_ucoll;	/* minimum u.c.w	*/
					wchar_t max_ucoll;	/* maximum u.c.w	*/
					uchar_t filecode[MB_LEN_MAX];	/* pc -> fc	*/

					min_ucoll = MIN_UCOLL;
					max_ucoll = MAX_UCOLL;
					for (i = MIN_PC; i <= MAX_PC; i++)
						{
						ucoll = __wcuniqcollwgt(i);
						if (ucoll >= min_ucoll && ucoll <= max_ucoll)
							{
							delta = ucoll - min_ucoll;
							if ((*(pp + (delta >> 3)) &  __reg_bits[delta & 7]) != 0)
								if (sblocale != 0)
									pmap[i] = 1;
								else if (wctomb(filecode, i) < 1)
									pmap[i & 0xff] = 1;
								else
									pmap[*filecode] = 1;
							}
						}
					}
				pp += be_size;
				}
			first++;
			continue;
/*
 * zero or more matches of previous expression
 *   error if no valid previous expression for ERE
 *   ordinary character if no valid previous expression for BRE
 *   specify CR_STAR for previous expression repeat factor
 */
		case '*':
			if (plastce == NULL)
				{
				if (ere == 0)
					goto cc_char;
				else
					{
					preg->re_erroff = ppat - pattern - 1;
					return (REG_BADRPT);
					}
				}
			REPEAT_CHECK
			isfirst = 0;
			*plastce = (*plastce & ~CR_MASK) | CR_STAR;
			continue;
/*
 * match any character except NUL
 *   error if preceeded by ERE $
 *   add CC_DOT code to pattern if REG_NEWLINE is not set & single byte locale
 *   add CC_DOTREG code to pattern if REG_NEWLINE is set & single byte locale
 *   add CC_WDOT code to pattern if multibyte locale
 *   set all map bits
 */
		case '.':
			plastce = pp;
			if (sblocale != 0)
				{
				if ((cflags & REG_NEWLINE) != 0)
					*pp++ = CC_DOTREG;
				else
					*pp++ = CC_DOT;
				}
			else
				*pp++ = CC_WDOT;
			if (isfirst++ == 0)
				memset(pmap, (int)1, (int)256);
			first++;
			continue;
/*
 * match beginning of line
 *   error if preceeded by ERE $
 *   ordinary character if not 
 *         first thing in BRE
 *         first thing is a subexpression BRE
 *   add CC_BOL to pattern
 *   set all map bits
 */
		case '^':
			if (first != 0 && ere == 0 )
				goto cc_char;
			
			if (first_BOL && ere == 0 && *(pp-1) == CC_BOL) {
				first++;
				goto cc_char;
			}

			if (isfirst++ == 0) {
				plastce = NULL;
				memset(pmap, (int)1, (int)256);
			}
			first_BOL++;
			*pp++ = CC_BOL;
			continue;
/*
 * match end of line
 *   error if preceeded by ERE $
 *   normal character if not last thing in BRE
 *   save $ offset in pattern for later testing and error reporting
 *   add CC_EOL to pattern
 */
		case '$':
			if((ere==0) && (*ppat!='\\') && (*ppat!='\0'))
				goto cc_char;
			eol[idx] = ppat - pattern;
			plastce = NULL;
			*pp++ = CC_EOL;
			if (isfirst++ == 0)
				{
				if ((cflags & REG_NEWLINE) != 0)
					pmap['\n'] = 1;
				pmap[0] = 1;
				}
			continue;
/*
 * backslash
 *   error if followed by NUL
 *   protects next ERE character
 *   introduces special BRE characters
 *     processing is based upon next character
 *       (     start subexpression
 *       )     end subexpression
 *       {     repetition interval
 *       1-9   backreference
 *       other ordinary character
 */
		case '\\':
			c = *ppat++;
			if (c == 0)
				{
				preg->re_erroff = ppat - pattern - 1;
				return (REG_EESCAPE);
				}
			if (ere != 0)
				goto cc_char;
			switch (c)
				{
/*
 * start subexpression
 *   error if too many subexpressions
 *   save start information concerning this subexpression
 *   add CC_SUBEXP to pattern
 *     subexpression data follows up to ending CC_SUBEXP_E
 */
			case '(':
			lparen:
				if (nsub++ >= _REG_SUBEXP_MAX)
					{
					preg->re_erroff = ppat - pattern - 1;
					return (REG_EPAREN);
					}
				if (nsub > preg->__maxsub)
					preg->__maxsub = nsub;
				*++psubidx = nsub;
				eol[nsub] = 0;
				altloc[nsub] = 0;
				if (first == 0)
					sol[nsub] = 0;
				else
					sol[nsub] = 1;
				idx = nsub;
				plastce = NULL;
				*pp++ = CC_SUBEXP;
				*pp++ = nsub;
				preg->re_lsub[nsub] = (void *)(pp - pp_start);
				preg->re_esub[nsub] = NULL;
				continue;
/*
 * end subexpression
 *   error if no matching start subexpression BRE
 *   regular character  if no matching start subexpression ERE
 *   save end information concerning this subexpression
 *   add CC_SUBEXP_E to pattern
 */
			case ')':
			rparen:
				if (--psubidx < subidx)
					{
					if (ere)
						{
						psubidx = subidx;
						goto cc_char;
						}
					preg->re_erroff = ppat - pattern - 1;
					return (REG_EPAREN);
					}
				preg->re_esub[idx] = (void *)(pp - pp_start);
				plastce = pp;
				*pp++ = CC_SUBEXP_E;
				*pp++ = idx;
				idx = *psubidx;
				first++;
				continue;
/*
 * repetition interval match of previous expression
 *   treat characters as themselves if no previous expression
 *   \{m\}   matches exactly m occurances
 *   \{m,\}  matches at least m occurances
 *   \{m,n\} matches m through n occurances
 *   error if invalid sequence or previous expression already has * or {}
 *   insert two bytes for min/max after pattern code
 *   specify CR_INTERVAL for previous expression repeat factor
 */
			case '{':
				do_all = 0;
				if (plastce == NULL)
					{
					c = '\\';
					ppat--;
					goto cc_char;
					}
				pri = ppat;
				minri = 0;

				while ((c2 = *pri++) >= '0' && c2 <= '9')
					minri = minri * 10 + c2 - '0';
				/**** first, lets check if we didn't convert anything ****/
				if ((pri == ppat+1) || (c2 == '\0'))
					{
					preg->re_erroff = ppat - pattern;
					return ((c2=='\0')? REG_EBRACE : REG_BADBR);
					}
				if (c2 == '\\' && *pri == '}')
					{
					pri++; 
					maxri = minri;
					}
				else if (c2 != ',')
					{
					preg->re_erroff = pri - pattern - 1;
					return (REG_BADBR);
					}
				else if (*pri == '\\' && pri[1] == '}')
					{
					pri += 2;
					do_all = 1;
					maxri = minri;
					}
				else
					{
					maxri = 0;
					while ((c2 = *pri++) >= '0' && c2 <= '9')
						maxri = maxri * 10 + c2 - '0';
					if (c2 != '\\' || *pri != '}')
						{
						preg->re_erroff = pri - pattern - 1;
						return ((c2=='\0')? REG_EBRACE : REG_BADBR); 
						}
					pri++;
					}
				if (minri > maxri || maxri > RE_DUP_MAX || *pri == '*' || (*plastce & CR_MASK) != 0)
					{
					preg->re_erroff = ppat - pattern;
					return (REG_BADBR);
					}
				maxri -= minri;
				ppat = pri;
				REPEAT_CHECK
				pp += 2;
				pto = pp - 1;
				pfrom = pto - 2;
				do
					*pto-- = *pfrom--;
				while (pfrom > plastce);
				if (do_all)
					*plastce = (*plastce & ~CR_MASK) | CR_INTERVAL_ALL;
				else
					*plastce = (*plastce & ~CR_MASK) | CR_INTERVAL;
				plastce[1] = minri;
				plastce[2] = maxri;
				if (minri == 0)
					isfirst = 0;
				continue;
/*
 * subexpression backreference
 *   error if subexpression not completed yet
 *   add CC_BACKREF to pattern if case sensitive
 *   add CC_I_BACKREF or CC_I_WBACKREF to pattern if ignore case
 */
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				c -= '0';
				if (c > nsub || preg->re_esub[c] == NULL)
					{
					preg->re_erroff = ppat - pattern - 1;
					return (REG_ESUBREG);
					}
				plastce = pp;
				if (icase == 0)
					*pp++ = CC_BACKREF;
				else
					if (sblocale != 0)
						*pp++ = CC_I_BACKREF;
					else
						*pp++ = CC_I_WBACKREF;
				*pp++ = c;
				first++;
				continue;
/*
 * not a special character
 *   treat as ordinary character
 */
			default:
				goto cc_char;
				}
/*
 * start subexpression for ERE
 *   do same as \( for BRE
 *   treat as ordinary character for BRE
 */
		case '(':
			if (ere != 0)
				goto lparen;
			goto cc_char;
/*
 * end subexpression for ERE
 *   do same as \) for BRE
 *   treat as ordinary character for BRE
 */
		case ')':
			if (ere != 0)
				goto rparen;
			goto cc_char;
/*
 * zero or one match of previous expression
 *   ordinary character for BRE
 *   error if no valid previous expression
 *   ignore if previous expression already has *
 *   specify CR_QUESTION for previous expression repeat factor
 */
		case '?':
			if (ere == 0)
				goto cc_char;
			if (plastce == NULL)
				{
				preg->re_erroff = ppat - pattern - 1;
				return (REG_BADRPT);
				}
			if ((*plastce & CR_MASK) > CR_QUESTION)
				continue;
			REPEAT_CHECK
			*plastce = (*plastce & ~CR_MASK) | CR_QUESTION;
			isfirst = 0;
			continue;
/*
 * one or more matches of previous expression
 *   ordinary character for BRE
 *   error if no valid previous expression
 *   ignore if previous expression already has * or ?
 *   specify CR_PLUS for previous expression repeat factor
 */
		case '+':
			if (ere == 0)
				goto cc_char;
			if (plastce == NULL)
				{
				preg->re_erroff = ppat - pattern - 1;
				return (REG_BADRPT);
				}
			if ((*plastce & CR_MASK) > CR_PLUS)
				continue;
			REPEAT_CHECK
			*plastce = (*plastce & ~CR_MASK) | CR_PLUS;
			continue;
/*
 * repetition interval match of previous expression
 *   ordinary character for BRE
 *   {m}   matches exactly m occurances
 *   {m,}  matches at least m occurances
 *   {m,n} matches m through n occurances
 *   treat characters as themselves if invalid sequence
 *   ignore if previous expression already has * or ? or +
 *   error if valid {} does not have previous expression
 *   insert two bytes for min/max after pattern code
 *   specify CR_INTERVAL for previous expression repeat factor
 */
		case '{':
			do_all = 0;
			if (ere == 0)
				goto cc_char;
			pri = ppat;
			minri = 0;
			while ((c2 = *pri++) >= '0' && c2 <= '9')
				minri = minri * 10 + c2 - '0';
			if (pri == ppat+1)
				goto cc_char;
/****
 XPG4 says that '{' is undefined for ERE's if it is not part of a valid 
 repetition interval, so we're going back to treating it as a normal char
 but this may change in a later release of XPG.  If XPG changes its mind
 and decides it should return an error, this is what should be done 
 (instead of "goto cc_char;") :
			{
				preg->re_erroff = ppat - pattern;
				return (REG_BADPAT);
			}
 ****/
			if (c2 == '}')
				maxri = minri;
			else if (c2 != ',')
				goto cc_char;
/****
			{
				preg->re_erroff = pri - pattern - 1;
				return (REG_BADBR);
			}
 ****/
			else if (*pri == '}')
				{
				do_all = 1;
				maxri = minri;
				pri++;
				}
			else
				{
				maxri = 0;
				while ((c2 = *pri++) >= '0' && c2 <= '9')
					maxri = maxri * 10 + c2 - '0';
				if (c2 != '}')
					goto cc_char;
/****
					{
					preg->re_erroff = pri - pattern - 1;
					return (REG_BADBR); 
					}
 ****/
				}
			if (minri > maxri || maxri > RE_DUP_MAX)
				goto cc_char;
/****
				{
				preg->re_erroff = ppat - pattern;
				return (REG_BADBR);
				}
 ****/
			maxri -= minri;
			if (plastce == NULL)
				{
				preg->re_erroff = ppat - pattern - 1;
				return (REG_BADBR);
				}
			ppat = pri;
			if ((*plastce & CR_MASK) > CR_INTERVAL_ALL)
				continue;
			REPEAT_CHECK
			pp += 2;
			pto = pp - 1;
			pfrom = pto - 2;
			do
				*pto-- = *pfrom--;
			while (pfrom > plastce);
			if (do_all)
				*plastce = (*plastce & ~CR_MASK) | CR_INTERVAL_ALL;
			else
				*plastce = (*plastce & ~CR_MASK) | CR_INTERVAL;
			plastce[1] = minri;
			plastce[2] = maxri;
			if (minri == 0)
				isfirst = 0;
			continue;
/*
 * begin alternate expression
 *   treat <vertical-line> as normal character if
 *     1) BRE
 *     2) not followed by another expression
 *     3) beginning of pattern
 *     4) no previous expression
 *   insert leading CC_ALTERNATE if this is first alternative at this level
 *      compensate affected begin/end subexpression offsets
 *   compute delta offset from last CC_ALTERNATE to this one
 *   add CC_ALTERNATE_E to pattern, terminating previous alternative
 *   add CC_ALTERNATE to pattern, starting next alternative
 *   indicate now at end-of-line position
 *   indicate now at beginning-of-line if not blocked by previous expression
 */
		case '|':
			if (ere == 0 || *ppat == ')' || *ppat == '\0' || ppat == pattern+1 ||
				(plastce == NULL && ppat[-2] != '^' && ppat[-2] != '$'))
				goto cc_char;
			palt = pp_start + (size_t)preg->re_lsub[idx];
			if (altloc[idx] == 0)
				{
				pp += 3;
				pto = pp - 1;
				pfrom = pto - 3;
				do
					*pto-- = *pfrom--;
				while (pfrom >= palt);
				*palt = CC_ALTERNATE;
				palt[1] = 0;
				palt[2] = 0;
				if (psubidx == subidx)
					{
					for (i=1; i<=nsub; i++)
						{
						preg->re_lsub[i] = (void *)((size_t)(preg->re_lsub[i]) + 3);
						preg->re_esub[i] = (void *)((size_t)(preg->re_esub[i]) + 3);
						}
					}
				else
					{
					for (i=*psubidx; i<=nsub; i++)
						if (preg->re_esub[i] != NULL)
							{
							preg->re_lsub[i] = (void *)((size_t)(preg->re_lsub[i]) + 3);
							preg->re_esub[i] = (void *)((size_t)(preg->re_esub[i]) + 3);
							}
					}
				}
			else
				palt = altloc[idx] + pp_start;
			i = pp - palt - 1;
			palt[1] = i >> 8;
			palt[2] = i & 0xff;
			*pp++ = CC_ALTERNATE_E;
			*pp++ = idx;
			altloc[idx] = pp - pp_start;
			*pp++ = CC_ALTERNATE;
			*pp++ = 0;
			*pp++ = 0;
			plastce = NULL;
			eol[idx] = 0;
			if (sol[idx] == 0)
				{
				first = 0;
				isfirst = 0;
				}
			continue;
				} /* end of switch */
			} /* end of while */
/*
 * Return error if missing ending subexpression
 */
	if (psubidx != subidx)
		{
		preg->re_erroff = ppat - pattern - 1;
		return (REG_EPAREN);
		}
/*
 * Set all map bits to prevent regexec() failure if
 * "first" expression not defined yet
 *   1) empty pattern
 *   2) last expression has *, ?, or {0,}
 */
	if (isfirst == 0)
		memset(pmap, (int)1, (int)256);
/*
 * No problems so add trailing end-of-pattern compile code
 * There is always suppose to be room for this
 */
	*pp++ = CC_EOP;
/*
 * Convert beginning/ending subexpression offsets to addresses
 * Change first subexpression expression to start of subexpression
 */
	preg->re_lsub[0] = pp_start;
	preg->re_esub[0] = pp - 1;
	for (i=1; i<=nsub; i++)
		{
		preg->re_lsub[i] = pp_start + (size_t)preg->re_lsub[i] - 2;
		preg->re_esub[i] = pp_start + (size_t)preg->re_esub[i];
		}
/*
 * Define remaining RE structure and return status
 */
	preg->re_comp = (void *)pp_start;
	preg->re_len = pp - pp_start;
	if ((cflags & REG_NOSUB) == 0)
		preg->re_nsub = nsub;
	return (0);
}		


/************************************************************************/
/* bracket	- convert [] expression into compiled RE pattern	*/
/*									*/
/*		- ppat		ptr to pattern				*/
/*		- pnext		ptr to pattern address following []	*/
/*		- pp		ptr to compiled RE pattern		*/
/*		- cflags	__regcomp() flags			*/
/************************************************************************/

static int
bracket(uchar_t *ppat, uchar_t **pnext, uchar_t *pp, int cflags)
{
	int	c;		/* file code of pattern character	*/
	int	c2;		/* file code of character opposite case	*/
	char	class[CLASS_SIZE+1]; /* [: :] text with terminating NUL	*/
	int	dash;		/* in the middle of a range expression	*/
	int	i;		/* loop index				*/
	int	icase;		/* ignore case flag			*/
	int	neg;		/* nonmatching bitmap			*/
	uchar_t	*pb;		/* ptr to [] expression			*/
	char	*pclass;	/* ptr to class				*/
	uchar_t	*pend;		/* ptr to end point in range expression	*/
	uchar_t	*pi;		/* ptr to [international] expression	*/
	int	prev;		/* previous character for range expr	*/
	uchar_t	*pxor;		/* nonmatching xor bitmap ptr		*/
	wctype_t wh;		/* character class handle for is_wctype	*/

/*
 * Check for nonmatching expression which has a leading <circumflex>
 */
	icase = cflags & REG_ICASE;
	pb = ppat;
	neg = 0;
	if (*pb == '^')
		{
		pb++;
		neg++;
		}
/*
 * Check for leading <hyphen> or <right-bracket> which is not the [] terminator
 */
	dash = 0;
	prev = 0;
	if (*pb == '-')
		{
		prev = *pb++;
		SETBITC(pp, prev)
		}
	else if (*pb == ']')
		{
		prev = *pb++;
		SETBITC(pp, prev)
		}
/*
 * BIG LOOP to process all characters in [] expression
 * stop on ]
 * return on any error
 * next character can begin any of the following:
 *   a) any single character (default)
 *   b) equivalence character [= =] (only mathces specified character)
 *   c) collating symbol [. .] (assumes only one single byte character)
 *   d) character class [: :]
 */
	while ((c = *pb++) != '\0')
		{
		switch(c)
			{
/*
 * single character
 *   set bitmap bit associated with character's file code
 *   if ignore case, also set bit of opposite case character
 */
		default:
		one_char:
			SETBITC(pp, c)
			if (icase != 0)
				{
				if ((c2 = toupper(c)) == c)
					c2 = tolower(c);
				SETBITC(pp, c2)
				}
			break;
/*
 * [] terminator
 *   set bit for <minus> if expression ends with -]
 *   negate bitmap if nonmatching [] expression and clear
 *     newline bit if REG_NEWLINE is set
 *   clear NUL bit to disallow match of NUL in string
 *   return ptr to next character after ]
 */
		case ']':
			if (dash != 0)
				SETBITC(pp, dash)
			if (neg != 0)
				{
				for (pxor = pp + BITMAP_LEN - 1; pxor >= pp; pxor--)
					*pxor = ~*pxor;
				*pp &= 0xfe;
				if ((cflags & REG_NEWLINE) != 0)
					pp[1] &= 0xfb;
				}
			*pnext = pb;
			return (0);
/*
 * [: :] character class
 *   move class name into NUL terminated buffer
 *   error if too short or too long
 *   determine class handle, error in undefined
 *   set bitmap bit of all characters with this class characteristic
 *   if ignore case, also set bits of opposite case characters
 */
		case '[':
			if ((c = *pb++) == ':')
				{
				pclass = class;
				while (1)
					{
					if (*pb == '\0')
						{
						*pnext = pb - 1;
						return (REG_EBRACK);
						}
					if (*pb == ':' && pb[1] == ']')
						break;
					if (pclass >= &class[CLASS_SIZE-1])
						{
						*pnext = pb;
						return (REG_ECTYPE);
						}
					*pclass++ = *pb++;
					}
				if (pclass == class)
					{
					*pnext = pb;
					return (REG_ECTYPE);
					}
				*pclass = '\0';
				if ((wh = get_wctype(class)) == -1) {
					*pnext = pb;
					return (REG_ECTYPE);
					}
				pb += 2;
				for (i=1; i<=255; i++)
					{
					if (is_wctype(i, wh) != 0)
						{
						SETBITC(pp, i)
						if (icase != 0)
							{
							if ((c2 = toupper(i)) == i)
								c2 = tolower(i);
							SETBITC(pp, c2)
							}
						}
					}
				c = 0;
				break;
				}
/*
 * [= =] equivalence class or [. .] collating element
 *   error if not a single character followed by terminating character pair
 *   set bitmap bit of character
 *   if ignore case, also set bit of opposite case character
 * set bit
 */
			else if (c == '=' || c == '.')
				{
				if (*pb == '\0' || pb[1] != c || pb[2] != ']')
					{
					*pnext = pb;
					return (REG_ECOLLATE);
					}
				c = *pb;
				pb += 3;
				SETBITC(pp, c)
				if (icase != 0)
					{
					if ((c2 = toupper(c)) == c)
						c2 = tolower(c);
					SETBITC(pp, c2)
					}
				break;
				}
			else
				{
				pb--;
				c = '[';
				goto one_char;	
				}
/*
 * <hyphen> deliniates a range expression unless it is an end point
 */
		case '-':
			if (dash == 0)
				{
				dash = c;
				pend = pb;
				continue;
				}
			else
				goto one_char;
			} /* end of switch */
/*
 * Process range expression
 *   prev is file code of previous character (start point)
 *   c is file code of character following <hyphen> (end point)
 *   error if start point is greater than end point
 *   set all bits between prev and c
 *   if ignore case, also set bits of opposite case characters
 */
		if (dash != 0)
			{
			dash = 0;
			if (prev > c || prev == 0)
				{
				*pnext = pend;
				return (REG_ERANGE);
				}
			for (i=prev+1; i<c; i++)
				{
				SETBITC(pp, i)
				if (icase != 0)
					{
					if ((c2 = toupper(i)) == i)
						c2 = tolower(i);
					SETBITC(pp, c2)
					}
				}
			prev = 0;
			}
		else
			prev = c;
		} /* end of while */
/*
 * fatal error if <right-bracket> not found
 */
	*pnext = pb - 1;
	return (REG_EBRACK);
}


/************************************************************************/
/* enlarge	- enlarge compiled pattern buffer                       */
/*									*/
/*		- x		# of new bytes needed in pattern buf	*/
/*		- pp_start	ptr to starting address of pattern buf	*/
/*		- pe		ptr to ending address of pattern buf	*/
/*		- plastce	ptr to last compiled pattern code	*/
/************************************************************************/

static void
enlarge(int x, uchar_t **pp_start, uchar_t **pe, uchar_t **pp, uchar_t **plastce)
{
        size_t  old_len;                /* previous length (bytes)	*/
        size_t  new_len;                /* new length (bytes)		*/
        uchar_t *old_start;             /* previous pp_start            */
        uchar_t *new_start;             /* new pp_start                 */

        old_start = *pp_start;
        old_len = *pe - old_start + 1;
        new_len = old_len + PATTERN_EXP;
        while (new_len < old_len + x)
                new_len += PATTERN_EXP;
        new_start = (uchar_t *)malloc(new_len);
        *pp_start = new_start;
        if (new_start != NULL)
                {
                memcpy(new_start, old_start, old_len);
                *pe = new_start + new_len - 1;
                *pp = (*pp - old_start) + new_start;
                if (*plastce != NULL)
                        *plastce = (*plastce - old_start) + new_start;
                free(old_start);
                }
	return;
}




/************************************************************************/
/* bracketw - convert [bracket expression] into compiled RE bitmap	*/
/************************************************************************/

static int
bracketw(_LC_collate_objhdl_t hdl, uchar_t *ppat, uchar_t **pnext, uchar_t *pp,
	 regex_t *preg, uchar_t *pmap)
{
	int	dashflag;	/* in the middle of a range expression	*/
	int	delta;		/* SETBIT unique collating value offset	*/
	int	i;		/* loop index for range of bits		*/
	int	icase;		/* ignore case flag			*/
	wchar_t	max_ucoll;	/* maximum unique collating value	*/
	wchar_t	min_ucoll;	/* minimum unique collating value	*/
	int	mb_cur_max;	/* local copy of MB_CUR_MAX		*/
	int	neg;		/* nonmatching bitmap flag		*/
	uchar_t	*pb;		/* ptr to [bracket expression]		*/
	uchar_t	*pclass;	/* class[] ptr				*/
	uchar_t	*pdash;		/* ptr to <hyphen> in range expression	*/
	wchar_t	prev_min_ucoll;	/* previous character min_ucoll		*/
	wchar_t	sv_wc;		/* save wc of previous character	*/
	uchar_t	*pxor;		/* pattern ptr to xor nonmatching []	*/
	int	stat;		/* intl_expr return status		*/
	wchar_t	ucoll;		/* unique collating value of lowercase	*/
	wchar_t	wc;		/* character process code		*/
	wchar_t	wc2;		/* OPPOSITE character process code	*/
	int	wclen;		/* # bytes in next character		*/
	uchar_t	class[CLASS_SIZE]; /* [ ] text with terminating <NUL>	*/

	pb = ppat;
	dashflag = 0;
	mb_cur_max = MB_CUR_MAX;
	icase = preg->re_cflags & REG_ICASE;
/*
 * <circumflex> defines a nonmatching bracket expression if it is
 *   the first [bracket expression] character
 */
	if (*pb == '^')
		{
		pb++;
		neg++;
		}
	else
		neg = 0;
/*
 * determine process code of next character
 * leading <circumflex> means nonmatching []
 *
 * use next byte if invalid multibyte character detected
 * determine min/max unique collating value of next character
 *
 * next character can be one of the following
 *	a) single collating element (any single character)
 *	b) equivalence character ([= =])
 *	c) character class ([: :])
 *	d) collating symbol ([. .])
 */
	while ((wclen = mbtowc(&wc, pb, mb_cur_max)) > 0)
	{
		pb += wclen;
		min_ucoll = __wcuniqcollwgt(wc);
		max_ucoll = min_ucoll;
		switch (wc)
			{
/*
 * single character collating element
 * invalid if has an out-of-range unique collating value (meaning
 *   it is not considered for collation)
 * set bitmap associated with character's unique collating value
 */
		default:
		coll_ele:
			if (min_ucoll < MIN_UCOLL || min_ucoll > MAX_UCOLL)
				{
				*pnext = pb - wclen;
				return (REG_ECOLLATE);
				}
			SETBIT(pp, min_ucoll);
			OPPOSITE(icase, pp, ucoll, wc, wc2);
			break;
/*
 * <hyphen> defines a range expression a-z if it is surrounded by a
 *   valid range expression
 * it is treated as itself if the first or last character within
 *   the [bracket expression]
 */
		case '-':
			if ((dashflag != 0) ||
				((neg == 0 && pb == ppat + 1) || (neg != 0 && pb == ppat + 2)) ||
				(*pb == ']'))
				goto coll_ele;
			dashflag++;
			pdash = pb - 1;
			continue;
/*
 * <open-bracket> initiates one of the following internationalization
 *   character expressions:
 *	a) [= =] equivalence character class
 *	b) [. .] collation symbol
 *	c) [: :] character class
 *
 * it is treated as itself if not followed by one of the three
 *   special characters <equal-sign>, <period>, or <colon>
 *
 * move contents of [ ] to a <NUL> terminated string
 * set bitmap bits by calling intl_expr
 * min/max will return with valid values if not character class
 *
 * set pmap bit for first byte of collation symbol because loop
 * in bracketw() does not know about collation symbols in v3.2
 */
		case '[':
			if (*pb != '=' && *pb != '.' && *pb != ':')
				goto coll_ele;
			*pnext = pb++;
			pclass = class;
			while (1)
				{
				if (*pb == '\0')
					{
					*pnext = pb;
					return (REG_EBRACK);
					}
				if (*pb == **pnext && pb[1] == ']')
					break;
				if (pclass >= &class[CLASS_SIZE])
					return (REG_ECTYPE);
				*pclass++ = *pb++;
				}
			if (pclass == class)
				return (REG_ECTYPE);
			*pclass = '\0';
			pb += 2;
			stat = intl_expr(hdl, preg, (char)**pnext, class, pp, 
					 &min_ucoll, &max_ucoll);
			if (stat != 0)
				return (stat);
			if (pmap != NULL && **pnext == '.')
				pmap[*class] = 1;
			break;
/*
 * <close-bracket> is treated as itself if it is the first character
 *   within the [bracket expression]
 * otherwise it correctly ends this [bracket expression]
 * set 
 * complement the final bitmap if nonmatching [bracket expression]
 *   making sure <NUL> is not allowed to match, and <newline> does
 *   not match if REG_NEWLINE is set
 */
		case ']':
			if ((neg == 0 && pb == ppat + 1) || (neg != 0 && pb == ppat + 2))
				goto coll_ele;
			if (neg != 0)
				{
				pxor = pp + ((MAX_UCOLL - MIN_UCOLL) / NBBY);
				for (; pxor >= pp; pxor--)
					*pxor = ~*pxor;
				ucoll = __wcuniqcollwgt('\0');
				if (ucoll >= MIN_UCOLL && ucoll <= MAX_UCOLL)
					CLEARBIT(pp, ucoll) 
				if ((preg->re_cflags & REG_NEWLINE) != 0)
					{
					ucoll = __wcuniqcollwgt('\n');
					if (ucoll >= MIN_UCOLL && ucoll <= MAX_UCOLL)
						CLEARBIT(pp, ucoll)
					}
				}
			*pnext = pb;
			return (0);
			} /* end of switch */
/*
 * a range expression a-z sets all of the bitmap bits between the starting
 * and ending point of the range.  The range is invalid if
 *	a) either the starting or ending point has a non-collating collating value
 *	b) the starting point collating value is greater than the ending point
 *	c) either end point is a character class
 *	d) the starting point is a previous range expression
 * if ignoring case, set bit associated with opposite case of each character
 *   and multicharacter collating symbol
 */
		if (dashflag != 0)
			{
			dashflag = 0;
			if (prev_min_ucoll < MIN_UCOLL ||
				max_ucoll > MAX_UCOLL ||
				prev_min_ucoll > max_ucoll)
				{
				*pnext = pdash;
				return (REG_ERANGE);
				}
			/* Loop thru all process codes looking for wgts between the two endpoints */
			for (i=MIN_PC; i<=MAX_PC; i++) {
				ucoll = __wcuniqcollwgt(i);
				if (ucoll>=prev_min_ucoll && ucoll<=max_ucoll){
					SETBIT(pp, ucoll);
					OPPOSITE(icase, pp, ucoll, i, wc2);
				}
			}
			if (icase != 0)
				{
				}
			min_ucoll = 0;
			}
		prev_min_ucoll = min_ucoll;
		sv_wc = wc;
	} /* end of while */
/*
 * return with error when <NUL> or invalid character detected
 */
	*pnext = pb;
	if (wclen < 0)
		return (REG_ECHAR);
	return (REG_EBRACK);
}


/************************************************************************/
/* intl_expr - decode [ ] internationalization character expression	*/
/************************************************************************/

static int
intl_expr(_LC_collate_objhdl_t hdl, regex_t *preg, uchar_t type, 
	  uchar_t *pexpr, uchar_t *pp, wchar_t *pmin, wchar_t *pmax)
{
	int	delta;		/* SETBIT variable			*/
	wint_t	i;		/* loop index				*/
	int	icase;		/* ignore case flag			*/
	wchar_t	ocoll;		/* opposite case collating weight	*/
	wchar_t	pcoll;		/* primary collating weight		*/
	uchar_t	*pend;		/* ptr to end of collating element + 1	*/
	wchar_t	*pwgt;		/* ptr to collation weight table	*/
	wchar_t *tmpwgt;	/* temp collating weight table ptr	*/
	wchar_t	ucoll;		/* unique collating weight		*/
	wchar_t	wc;		/* process code of pexpr character	*/
	wchar_t	wc2;		/* OPPOSITE character process code	*/
	int	wclen;		/* # bytes in pexpr character		*/
	wctype_t wctype;	/* character class handle for is_wctype	*/
	uchar_t	lcexpr[NL_NMAX*MB_LEN_MAX]; /* lowercase [. .]		*/
	

	icase = preg->re_cflags & REG_ICASE;
	switch (type)
		{
/*
 * equivalence class [= =]
 * treat invalid collating element as collating symbol
 * set bitmap bits for all characters in the equivalence class
 * if ignoring case, set bit associated with opposite case version of [= =]
 * define min/max unique collating values for the equivalence class
 */
			case '=':
				wclen = mbtowc(&wc, pexpr, MB_CUR_MAX);
				if (wclen < 0)
					return (REG_ECHAR);
				if (pexpr[wclen] != '\0')
					goto co_symbol;
				pwgt = __wccollwgt(wc);
				if (pwgt == NULL)
					return (REG_ECOLLATE);
				*pmax = pwgt[_UCW_ORDER];
				pcoll = *pwgt;
				if ((*pmax < MIN_UCOLL || *pmax > MAX_UCOLL) && (pcoll != 0))
					return (REG_ECOLLATE);
				*pmin = *pmax;
				{
						/* These indexes are for 1 to many mappings. 	    */
				int idx_ec;	/* index to wgt string for equivalence class sought */
				int idx_chk;	/* same, but for character being checked 	    */
				char *wstr_ec;  /* weight string for equivalence class sought       */
				char *wstr_chk; /* same, but for string for character being checked */
				int  substring;
				substring = (pcoll == _SUB_STRING);
				if (substring) {
					idx_ec = pwgt[0];
					wstr_ec = wgtstring(idx_ec);	/* We'll need this later. */
					}
				for (i = MIN_PC; i <= MAX_PC; i++) {
					tmpwgt = __wccollwgt(i);	/* weight table */
					ucoll = tmpwgt[_UCW_ORDER];	/* unique weight */
					if (substring && tmpwgt[0] == _SUB_STRING) {
						idx_chk = tmpwgt[0];		/* weight string index */
						wstr_chk = wgtstring(idx_chk);	/* weight string */
						}

					/* do primary wgts match (even if they're _SUB_STRING) */
					if (tmpwgt[0] == pcoll && (ucoll >= MIN_UCOLL && ucoll <= MAX_UCOLL)) {
						if (substring && strcmp(wstr_ec, wstr_chk) != 0)
							continue;	/* substrings don't match */
						/* same equivalence class */
						SETBIT(pp, ucoll);
						OPPOSITE(icase, pp, ocoll, i, wc2);
						if (ucoll < *pmin)
							*pmin = ucoll;
						if (ucoll > *pmax)
							*pmax = ucoll;
						}
					}
				}

		break;
/*
 * collating symbol [. .]
 * set single bitmap bit for the collation symbol
 * define min/max as collation symbol unique collating value
 * if ignoring case, set bit associated with opposite case version of [. .]
 *
 * return error if invalid collation symbol
 */
	case '.':
	co_symbol:
		ucoll = _mbucoll(hdl, pexpr, &pend);
		if (ucoll < MIN_UCOLL || ucoll > MAX_UCOLL)
			return (REG_ECOLLATE);
		if (*pend != '\0')
			return (REG_ECOLLATE);
		SETBIT(pp, ucoll);
		if (icase != 0)
			{
			}
		*pmin = ucoll;
		*pmax = ucoll;
		break;
/*
 * character class [: :]
 * return error if undefined in current locale
 * set bitmap bit for each process code with the class characteristic
 * if ignoring case, set bit associated with opposite case version of [: :]
 * define min unique collating value as zero so character class
 *   cannot be used with a range expression
 */
	case ':':
		wctype = get_wctype(pexpr);
		for (i = 1; i <= MAX_PC; i++)
			if (is_wctype(i, wctype) != 0)
				{
				ucoll = __wcuniqcollwgt(i);
				if (ucoll >= MIN_UCOLL && ucoll <= MAX_UCOLL)
					{
					SETBIT(pp, ucoll);
					OPPOSITE(icase, pp, ocoll, i, wc2);
					}
				}
		*pmin = 0;
		break;
		}
	return (0);
}
