static char sccsid[] = "@(#)24	1.9.3.2  src/bos/usr/ccs/lib/libc/__regexec_std.c, libcpat, bos41J, 9511A_all 2/23/95 17:04:19";
/*
 *   COMPONENT_NAME: libcpat
 *
 *   FUNCTIONS: __regexec_std
 *		match_bit
 *		match_dot
 *		match_re
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
#include <ctype.h>
#include <limits.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include "reglocal.h"
#include "patlocal.h"
#include "libc_msg.h"


/************************************************************************/
/* External data defined in regexec()					*/
/************************************************************************/

extern	int	__reg_bits[];	/* bitmask for [] bitmap		*/


/************************************************************************
 * NOTES: The submatch and tmatch arrays in EXEC must be copied from one
 *  variable to another (r=*pe, *pe=r, rbest=r, *pe=rbest, etc.).  Our
 *  compiler can optimize memory copies of less than 100 bytes into inline
 *  copying, and the rest go through a very expensive routine.
 *  tmatch[] is an array of structs containg 8 bytes per element (each
 *  struct has a start and end pointer).  Since 8*12=96 is under the 100
 *  byte limit, if we allowed only 12 elements in tmatch, we'd have the
 *  inlining optimzation with an enormous performance boost.  In fact,
 *  the less elements it needs to copy, the faster it will be.  Since the
 *  average regular expression with subexpressions generally does not use
 *  more than 3 subexpressions, we'll use 4 for the "fast path".
 *  We obviously want to support more than 9 subexpressions.  So for the
 *  "slower path", we'll support at most 24 elements (i.e. 23 subexpressions).
 *  Any more than 24 elements proves to be a noticable performance
 *  degradation.
 *  So if the pattern has 3 or less subexpressions, we'll use the x part
 *  of the submatch_t and tmatch_t unions for best performance.  If more
 *  than 3 are used, then we'll use the y part of submatch_t and tmatch_t.
 ************************************************************************/
#define _PERF_THRESHOLD 3

/************************************************************************/
/* Macro to copy the contents of one EXEC variable into another,        */
/* including the contents of the arrays pointed to by submatch & tmatch */
/* NOTE: ints, eats, best_s, & best_t must be declared locally.		*/
/************************************************************************/
#define COPY(TO, FROM, MODE)					\
{								\
	TO = FROM;						\
	switch(MODE) {						\
		case 0: /* copying back into original EXEC */	\
			(TO).submatch = sv_submatch;		\
			(TO).tmatch   = sv_tmatch;		\
			break;					\
		case 1:	/* copy EXEC into tmp for recursion */	\
			(TO).submatch = &ints;			\
			(TO).tmatch   = &eats;			\
			break;					\
		case 2: /* copy best EXEC so far           */	\
			(TO).submatch = &best_s;		\
			(TO).tmatch   = &best_t;		\
			break;					\
		}						\
	/* Save submatch and tmatch pointers of the TO EXEC */	\
	sv_submatch = (FROM).submatch;				\
	sv_tmatch   = (FROM).tmatch;  				\
								\
	if (preg->__maxsub <= _PERF_THRESHOLD) {		\
	/* Only copy the first _PERF_THRESHOLD+1 elements.  */	\
		(TO).submatch->x = (FROM).submatch->x;		\
		(TO).tmatch->x = (FROM).tmatch->x;		\
		}						\
	else {							\
	/* Copy _REG_SUBEXP_MAX+1 elements */			\
		(TO).submatch->y = (FROM).submatch->y;		\
		(TO).tmatch->y = (FROM).tmatch->y;		\
		}						\
}


/************************************************************************/
/* Internal definitions							*/
/************************************************************************/

typedef struct {                /* subexpression addresses		*/
        uchar_t	*sa;		/* start subexpression address		*/
        uchar_t	*ea;		/* end subexpression address + 1	*/
} ea_t;

/* Long way of defining an array, but at least it allows us to copy them
 * directly instead of one element at a time (i.e. a=b, instead of a[0]=b[0],
 * a[1]=b[1], ...).  
 */

typedef union { 
	struct {int a[_PERF_THRESHOLD+1]; } x;
	struct {int a[_REG_SUBEXP_MAX+1]; } y;
	} submatch_t;

typedef union { 
	struct {ea_t a[_PERF_THRESHOLD+1]; } x;
	struct {ea_t a[_REG_SUBEXP_MAX+1]; } y;
	} tmatch_t;

#define SUBMATCH  submatch->y.a
#define TMATCH    tmatch->y.a

typedef	struct {		/* structure for data during recursion	*/
	int	flags;		/* combined cflags and eflags		*/
	unsigned wander;	/* flag: OK to wander			*/
	uchar_t	*string;	/* ptr to start of string		*/
	uchar_t	*pend_string;	/* ptr to byte beyond match in string	*/
	uchar_t *start_match;	/* ^ to beginning of match		*/
	_LC_collate_objhdl_t pcoltbl; /* ptr to collation methods table	*/
	submatch_t *submatch; 	/* # subexp matches			*/
	tmatch_t   *tmatch; 	/* subexp addresses			*/
} EXEC;

typedef	struct {		/* bitmap recursion data		*/
	int	bit_max;	/* max allowed [] matches		*/
	int	bit_count;	/* number of [] matches			*/
} BIT;


/************************************************************************/
/* Internal function prototypes						*/
/************************************************************************/

static	int	match_re(	/* match entire pattern against string	*/
			uchar_t *,
			uchar_t *,
			regex_t	*,
			EXEC	*);

static	int	match_bit(	/* match ILS [] against string		*/
			uchar_t *,
			uchar_t *,
			regex_t	*,
			EXEC	*,
			BIT	*);

static	int	match_dot(	/* match ILS . against string		*/
			uchar_t *,
			uchar_t *,
			regex_t	*,
			EXEC	*,
			BIT	*);


/************************************************************************/
/* __regexec_std() - Determine if RE pattern matches string		*/
/*		   - valid for any locale				*/
/*									*/
/*		   - hdl	ptr to __lc_collate table 		*/
/*		   - preg	ptr to structure with compiled pattern	*/
/*		   - string	string to be matched			*/
/*		   - nmatch	# of matches to report in pmatch	*/
/*		   - pmatch	reported subexpression offsets		*/
/*		   - eflags	regexec() flags				*/
/************************************************************************/

int
__regexec_std(_LC_collate_objhdl_t hdl, const regex_t *preg, const char *string,
	size_t nmatch, regmatch_t pmatch[], int eflags)
{
	EXEC	e;		/* initial data passing structure	*/
	int	i;		/* loop index				*/
	int	mb_cur_max;	/* local copy of MB_CUR_MAX		*/
	uchar_t	*pmap;		/* ptr to character map table		*/
	uchar_t	*pstr;		/* ptr to next string byte		*/
	int	stat;		/* match_re() return status		*/
	static EXEC zero;	/* faster than bzero() call		*/
	submatch_t ints;	/* Buffer.				*/
	tmatch_t   eats;	/* Buffer.				*/
	static submatch_t s_zero; /* bzero all ints.			*/
	static tmatch_t   t_zero; /* bzero all ea_t's.		*/

/*
 * Return error if RE pattern is undefined
 * or re_map is NULL
 */
	if (preg->re_comp == NULL || preg->re_map == NULL)
		return (REG_BADPAT);

/*
 * optimisation:
 *	if the pattern doesn't start with "^",
 *	trim off prefix of pstr that has no hope of matching.  If we
 *	exhaust pstr via this method, we may already be a wiener!
 */
	pmap = preg->re_map;
	pstr = (char *)string;
	mb_cur_max = MB_CUR_MAX;

	if (*(uchar_t *) preg->re_comp != CC_BOL) {
		if (mb_cur_max > 1) {
			while (*pstr && pmap[*pstr] == 0) {
				i = mblen(pstr, mb_cur_max);

				pstr += (i < 1) ? 1 : i;
			}
		} else
			while (*pstr && pmap[*pstr] == 0)
				++pstr;

		if (!*pstr && !pmap[0])
			return REG_NOMATCH;
	}

/*
 * Initialize data recursion buffer
 */
	e         = zero;
	e.flags   = preg->re_cflags | eflags;
	e.pcoltbl = hdl;
	e.string  = (char *)string;
	e.wander  = 1;
	e.start_match = pstr;
	ints = s_zero;
	eats = t_zero;
	e.submatch = &ints;
	e.tmatch   = &eats;
/*
 * Attempt to match entire compiled RE pattern starting at current
 *     position in string
 */
	stat = match_re((uchar_t *)preg->re_comp, pstr, (regex_t *)preg, &e);

/*
 * Return offsets of entire pattern match
 * Return subexpression offsets, zero-length changed to -1
 */
	if (stat == 0) {
		pstr = e.start_match;
		if (nmatch > 0 && (preg->re_cflags & REG_NOSUB) == 0)
			{
			pmatch[0].rm_so = pstr - string;
			pmatch[0].rm_eo = e.pend_string - string;
			for (i=1; i<nmatch && i<=_REG_SUBEXP_MAX; i++)
				{
				if (e.TMATCH[i].sa != NULL)
					{
					pmatch[i].rm_so = e.TMATCH[i].sa - string;
					pmatch[i].rm_eo = e.TMATCH[i].ea - string;
					}
				else
					{
					pmatch[i].rm_so = (off_t)-1;
					pmatch[i].rm_eo = (off_t)-1;
					}
				}
			}
		}
	else if (stat == REG_EBOL)
		return (REG_NOMATCH);
	return (stat);
}


/************************************************************************/
/* match_re()	- Match entire RE pattern to string			*/
/*		- Note: CC_codes are in specific order to promote	*/
/*		-       performance.  Do not change without proof	*/
/*		-       that performance is improved - fms 07/23/91	*/
/*			(Sorry `fms'. The compiler generates a jump	*/
/*			 table for the CC_codes switch, so your comments*/
/*			 about "promoting performance" are misleading)	*/
/*									*/
/*		- ppat		ptr to pattern				*/
/*		- pstr		ptr to string				*/
/*		- preg		ptr to caller's regex_t structure	*/
/*		- pe		ptr to recursion data structure		*/
/************************************************************************/

static int
match_re(uchar_t *ppat, uchar_t *pstr, regex_t *preg, EXEC *pe)
{
	uchar_t	*best_alt;	/* best alternative pend_string		*/
	size_t	count;		/* # bytes to backtrack each time	*/
	size_t	count2;		/* ignore case backreference counter	*/
	int	cp;		/* pattern character			*/
	int	cp2;		/* opposite case pattern character	*/
	int	cs;		/* string character			*/
	int	idx;		/* subexpression index			*/
	int	max;		/* maximum repetition count - min	*/
	int	mb_cur_max;	/* local copy of MB_CUR_MAX		*/
	int	min;		/* minimum repetition count		*/
	uchar_t	*pback;		/* ptr to subexpression backreference	*/
	uchar_t	*pea;		/* ptr to subexpression end address	*/
	uchar_t	*psa;		/* ptr to subexpression start address	*/
	uchar_t	*pstop;		/* ptr to backtracking string point	*/
	uchar_t	*ptemp;		/* ptr to string during backreference	*/
	uchar_t	*sav_pat;	/* saved pattern			*/
	uchar_t	*sav_str;	/* saved string				*/
	uchar_t	*pmap;		/* ptr to character map table		*/
	int	stat;		/* match_re() recursive status		*/
	int	wclen;		/* # bytes in character			*/
	int	wander;		/* copy of EXEC.wander			*/
	wchar_t	wc_p;		/* pattern character process code	*/
	wchar_t	wc_s;		/* string character process code	*/
	EXEC	r;		/* another copy of *pe for recursion	*/
	EXEC	rbest;		/* best alternative recursion data	*/
	submatch_t ints;	/* Buffer.				*/
	submatch_t best_s;	/* Buffer.				*/
	tmatch_t   eats;	/* Buffer (best match so far).		*/
	tmatch_t   best_t;	/* Buffer (bset match so far).		*/
	submatch_t *sv_submatch;/* Used by COPY().			*/
	tmatch_t   *sv_tmatch;  /* Used by COPY().			*/

	wander     = pe->wander;
	pmap       = preg->re_map;
	sav_pat    = ppat;
	sav_str    = pstr;
	pe->wander = 0;
	mb_cur_max = MB_CUR_MAX;

	if (0) {
	    no_match:
		/*
		 * NOTE: the only way to come here is via a goto.
		 */
		if (wander) {
			/*
			 * we come here if we fail to match, and ok to wander
			 * down the string looking for a match.
			 *	- restore the pattern to the start
			 *	- restore string to one past where we left off
			 *	  and trim unmatchables
			 */
			if (*sav_str == '\0') 
				return REG_NOMATCH;

			ppat = sav_pat;		/* restore patterm	*/
			if (mb_cur_max == 1) {
				pstr = sav_str + 1;

				while (*pstr && pmap[*pstr] == 0)
					++pstr;
			} else {
				int i = mblen(sav_str, mb_cur_max);

				pstr = sav_str + ((i < 1) ? 1 : i);

				while (*pstr && pmap[*pstr] == 0) {
					int i = mblen(pstr, mb_cur_max);

					pstr += (i < 1) ? 1 : i;
				}
			}

			/*
			 * If at end of string, and it isn't possible for '$'
			 * to start an expression, then no match.  It is possible
			 * for '$' to start an expression as in "x*$", since
			 * "x*$" is equivalent to "$" when 0 x's precede the end
			 * of line, so we have to check one more time to see if
			 * the pattern matches the empty string (i.e. empty string
			 * is equivalent to end of line).  Note that this way,
			 * "yx*$" won't match "", but "x*$" will.
			 */ 
			if (*pstr == 0 && !pmap[0])
				return REG_NOMATCH;

			pe->start_match = sav_str = pstr;
		} else
			return REG_NOMATCH;
	}

/*
 * Perform each compiled RE pattern code until end-of-pattern or non-match
 * Break to bottom of loop to match remaining pattern/string when extra
 *   expressions have been matched
 */
    while (1) {
	count = 1;
	switch (*ppat++) {
/*
 * a single character, no repetition
 *   continue if pattern character matches next string character
 *   otherwise return no match
 */

	case CC_CHAR:
		if (*ppat != *pstr)
			goto no_match;
		ppat++;
		pstr++;
		continue;
/*
 * any single character, no repetition
 *   continue if next string character is anything but <nul>
 *   otherwise return no match
 */

	case CC_DOT:
		if (*pstr++ != '\0')
			continue;
		return REG_NOMATCH;
/*
 * end-of-pattern
 *   update forward progress of matched location in string
 *   return success
 */

	case CC_EOP:
		pe->pend_string = pstr;
		return (0);
/*
 * bracket expression, no repetition
 *   continue if next string character has bit set in bitmap
 *   otherwise return no match
 */

	case CC_BITMAP:
		cs = *pstr++;
		if ((*(ppat + (cs >> 3)) & __reg_bits[cs & 7]) != 0)
			{
			ppat += BITMAP_LEN;
			continue;
			}
		goto no_match;
/*
 * character string, no repetition
 * single multibyte character, no repetition
 *   continue if next n pattern characters matches next n string characters
 *   otherwise return no match
 */

	case CC_STRING:
	case CC_WCHAR:
		count = *ppat++;
		do
			if (*ppat++ != *pstr++)
				goto no_match;
		while (--count > 0);
		continue;
/*
 * end subexpression, no repetition
 *   save subexpression ending address
 *   continue in all cases
 */

	case CC_SUBEXP_E:
		idx = *ppat++;
		pe->TMATCH[idx].ea = pstr;
		continue;
/*
 * subexpression backreference, no repetition
 *   continue if next n string characters matches what was previously
 *     matched by the referenced subexpression
 *   otherwise return no match
 */
	case CC_BACKREF:
		idx = *ppat++;
		pback = pe->TMATCH[idx].sa;
		pea = pe->TMATCH[idx].ea;
		while (pback < pea)
			if (*pback++ != *pstr++)
				goto no_match;
		continue;
/*
 * begin subexpression
 *   generate new copy of recursion data
 *   preserve subexpression starting address
 *   match remaining pattern against remaining string
 *   if remaining pattern match succeeds, update recursion data with
 *     new copy and return success
 *   if remaining pattern match fails and zero length subexpression is ok, continue
 *     with pattern immediately following CC_SUBEXP_E
 *   otherwise return fatal error
 */

	case CC_SUBEXP:
		idx = *ppat++;
		COPY(r, *pe, 1);	/* Essentially: r = *pe */
		r.TMATCH[idx].sa = pstr;
		stat = match_re(ppat, pstr, preg, &r);
		if (stat == 0)
			{
			COPY(*pe, r, 0);
			return (0);
			}
		if (((cp2 = (*(uchar_t *)preg->re_esub[idx] & CR_MASK)) == CR_QUESTION || cp2 == CR_STAR) ||
			((cp2 == CR_INTERVAL || cp2 == CR_INTERVAL_ALL) && *(((uchar_t *)preg->re_esub[idx])+1) == 0))
			{
			ppat = preg->re_esub[idx];
			if ((*ppat != (CC_SUBEXP_E | CR_INTERVAL)) && (*ppat != (CC_SUBEXP_E | CR_INTERVAL_ALL)))
				ppat += 2;
			else
				ppat += 4;
			continue;
			}
		goto no_match;
/*
 * any single ILS character, no repetition
 *   continue if next string character is anything but <nul>
 *     or <newline> and REG_NEWLINE is set
 *   otherwise return no match
 */

	case CC_WDOT:
		if (*pstr == 0 || (*pstr == '\n' && (pe->flags & REG_NEWLINE) != 0))
			goto no_match;
		wclen = mblen(pstr, mb_cur_max);
		if (wclen < 0)
			wclen = 1;
		pstr += wclen;
		continue;
/*
 * ILS bracket expression, no repetition
 *   if ignoring case, get lowercase version of collating element
 *   continue if next string collating element has bit set in bitmap
 *   otherwise return no match
 */

	case CC_WBITMAP:
		{
		wchar_t	delta;		/* character offset into bitmap		*/
		wchar_t	ucoll;		/* unique collation weight		*/

		ucoll = _mbucoll(pe->pcoltbl, pstr, &ptemp);
		if (ucoll >= preg->re_ucoll[0] && ucoll <= preg->re_ucoll[1])
			{
			delta = ucoll - preg->re_ucoll[0];
			if ((*(ppat + (delta >> 3)) & __reg_bits[delta & 7]) != 0)
				{
				pstr = ptemp;
				ppat += ((preg->re_ucoll[1] - preg->re_ucoll[0]) / NBBY) + 1;
				continue;
				}
			}
		goto no_match;
		}
/*
 * beginning-of-line anchor
 *   REG_NEWLINE allows ^ to match null string following a newline
 *   REG_NOTBOL means first character is not beginning of line
 *
 *   REG_NOTBOL   REG_NEWLINE   at BOL   ACTION
 *   ----------   -----------   ------   -------------------------
 *        N            N           Y     continue
 *        N            N           N     return REG_EBOL
 *        N            Y           Y     continue
 *        N            Y           N     continue if \n, else return REG_NOMATCH
 *        Y            N           Y     return REG_EBOL
 *        Y            N           N     return REG_EBOL
 *        Y            Y           Y     continue if \n, else return REG_NOMATCH
 *        Y            Y           N     continue if \n, else return REG_NOMATCH
 */

	case CC_BOL:
		if ((pe->flags & REG_NOTBOL) == 0)
			{
			if (pstr == pe->string)
				continue;
			else if ((pe->flags & REG_NEWLINE) == 0)
				return (REG_EBOL);
			}
		else if ((pe->flags & REG_NEWLINE) == 0)
			return (REG_EBOL);
		if (pstr > pe->string && *(pstr-1) == '\n')
			continue;
		goto no_match;
/*
 * end-of-line anchor
 *   REG_NEWLINE allows $ to match null string preceeding a newline
 *   REG_NOTEOL means last character is not end of line
 *
 *   REG_NOTEOL   REG_NEWLINE   at EOL   ACTION
 *   ----------   -----------   ------   --------------------------
 *        N            N           Y     continue
 *        N            N           N     return REG_NOMATCH
 *        N            Y           Y     continue
 *        N            Y           N     continue if \n, else return REG_NOMATCH
 *        Y            N           Y     return REG_NOMATCH
 *        Y            N           N     return REG_NOMATCH
 *        Y            Y           Y     continue if \n, else return REG_NOMATCH
 *        Y            Y           N     continue if \n, else return REG_NOMATCH
 */

	case CC_EOL:
		if ((pe->flags & REG_NOTEOL) == 0)
			{
			if (*pstr == '\0')
				continue;
			else if ((pe->flags & REG_NEWLINE) == 0)
				goto no_match;
			}
		else if ((pe->flags & REG_NEWLINE) == 0)
			goto no_match;
		if (*pstr == '\n')
			continue;
		goto no_match;
/*
 * start alternative
 *   try each alternate
 *   select best alternative or the one which gets to EOP first
 */

	case CC_ALTERNATE:
		best_alt = NULL;
		do
			{
			idx = *ppat++ << 8;
			idx += *ppat++;
			COPY(r, *pe, 1);
			stat = match_re(ppat, pstr, preg, &r);
			if (stat == 0 && best_alt < r.pend_string)
				{
				if (*r.pend_string == '\0')
					{
					COPY(*pe, r, 0);
					return (0);
					}
				best_alt = r.pend_string;
				COPY(rbest, r, 2);
				}
			if (idx == 0)
				break;
			ppat += idx + 1;
			}
		while (1);
		if (best_alt != NULL)
			{
			COPY(*pe, rbest, 0);
			return (0);
			}
		goto no_match;
/*
 * any single character except <newline>, no repetition
 *   continue if next string character is anything but <nul>
 *     or <newline> and REG_NEWLINE is set
 *   otherwise return no match
 */

	case CC_DOTREG:
		if (*pstr == 0 || (*pstr++ == '\n' && (pe->flags & REG_NEWLINE) != 0))
			goto no_match;
		continue;
/*
 * end alternative
 *   skip over any other alternative patterns and continue matching
 *     pattern to string
 */

	case CC_ALTERNATE_E:
		idx = *ppat++;
		ppat = preg->re_esub[idx];
		continue;
/*
 * invalid compiled RE code
 *   return fatal error
 */

	default:
		return (REG_BADPAT);
/*
 * ignore case single character, no repetition
 *   continue if next string character matches pattern character or
 *     opposite case of pattern character
 *   otherwise return no match
 */

	case CC_I_CHAR:
		if (*ppat++ == *pstr)
			{
			ppat++;
			pstr++;
			continue;
			}
		if (*ppat++ == *pstr++)
			continue;
		goto no_match;
/*
 * ignore case character string, no repetition
 * ignore case single multibyte character, no repetition
 *   continue if next n string characters match next n pattern characters or
 *     opposite case of next n pattern characters
 *   otherwise return no match
 */

	case CC_I_STRING:
		count = *ppat++;
		do
			{
			if (*ppat++ == *pstr)
				{
				ppat++;
				pstr++;
				}
			else if (*ppat++ != *pstr++)
				goto no_match;
			}
		while (--count > 0);
		continue;
/*
 * ignore case subexpression backreference, no repetition
 *   continue if next n string characters matches what was previously
 *     matched by the referenced subexpression
 *   otherwise return no match
 */

	case CC_I_BACKREF:
		idx = *ppat++;
		pback = pe->TMATCH[idx].sa;
		pea = pe->TMATCH[idx].ea;
		while (pback < pea)
			{
			if ((*pback != *pstr) && (tolower(*pback) != tolower(*pstr)))
				goto no_match;
			pback++;
			pstr++;
			}
		continue;
/*
 * ignore case single ILS character, no repetition
 *   continue if next n string characters match next n pattern characters or
 *     opposite case of next n pattern characters
 */

	case CC_I_WCHAR:
		count = *ppat++;
		if (strncmp(ppat, pstr, count) != 0)
			if (strncmp(ppat+count, pstr, count) != 0)
				goto no_match;
		ppat += count * 2;
		pstr += count;
		continue;
		
/*
 * ignore case ILS subexpression backreference, no repetition
 *   continue if next n string characters or their opposite case matches
 *     what was previosly matched by the referenced subexpression
 *   otherwise return no match
 */

	case CC_I_WBACKREF:
		idx = *ppat++;
		pback = pe->TMATCH[idx].sa;
		pea = pe->TMATCH[idx].ea;
		while (pback < pea)
			{
			wclen = mbtowc(&wc_p, pback, mb_cur_max);
			if (wclen < 1)
				wc_p = *pback;
			wclen = mbtowc(&wc_s, pstr, mb_cur_max);
			if (wclen < 1)
				wc_s = *pstr;
			if ((wc_p != wc_s) && (towlower(wc_p) != towlower(wc_s)))
				goto no_match;
			pback += wclen;
			pstr += wclen;
			}
		continue;
/*
 * ignore case ILS subexpression backreference, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_I_WBACKREF processing
 */

	case CC_I_WBACKREF | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		goto cc_iwbackref;
/*
 * ignore case ILS subexpression backreference, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_I_WBACKREF processing
 */

	case CC_I_WBACKREF | CR_INTERVAL_ALL:
		min = *ppat++;
		ppat++;
		max = INT_MAX - 1;
		goto cc_iwbackref;
/*
 * ignore case ILS subexpression backreference, zero or more occurances "*"
 *   define min/max and jump to common CC_I_WBACKREF processing
 */

	case CC_I_WBACKREF | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_iwbackref;
/*
 * ignore case ILS subexpression backreference - variable number of matches
 *   continue if subexpression match was zero-length
 *   match minimum number of required times
 *     return no match if cannot meet minimum count
 *   save new string position for backtracking
 *   match maximum number of required times, where max is
 *     number of remaining matches
 *   break-out to match remaining pattern/string
 */

	cc_iwbackref:
		idx = *ppat++;
		psa = pe->TMATCH[idx].sa;
		pea = pe->TMATCH[idx].ea;
		count = pea - psa;
		if (count == 0)
			continue;
		while (min-- > 0)
			{
			pback = psa;
			while (pback < pea)
				{
				wclen = mbtowc(&wc_p, pback, mb_cur_max);
				if (wclen < 1)
					wc_p = *pback;
				wclen = mbtowc(&wc_s, pstr, mb_cur_max);
				if (wclen < 1)
					wc_s = *pstr;
				if ((wc_p != wc_s) && (towlower(wc_p) != towlower(wc_s)))
					goto no_match;
				pback += wclen;
				pstr += wclen;
				}
			}
		pstop = pstr;
		while (max-- > 0)
			{
			pback = psa;
			while (pback < pea)
				{
				wclen = mbtowc(&wc_p, pback, mb_cur_max);
				if (wclen < 1)
					wc_p = *pback;
				wclen = mbtowc(&wc_s, pstr, mb_cur_max);
				if (wclen < 1)
					wc_s = *pstr;
				if ((wc_p != wc_s) && (towlower(wc_p) != towlower(wc_s)))
					break;
				pback += wclen;
				pstr += wclen;
				}
			if (pback < pea)
				break;
			}
		break;
/*
 * ignore case subexpression backreference, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_I_BACKREF processing
 */

	case CC_I_BACKREF | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		goto cc_ibackref;
/*
 * ignore case subexpression backreference, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_I_BACKREF processing
 */

	case CC_I_BACKREF | CR_INTERVAL_ALL:
		min = *ppat++;
		ppat++;
		max = INT_MAX - 1;
		goto cc_ibackref;
/*
 * ignore case subexpression backreference, zero or more occurances "*"
 *   define min/max and jump to common CC_I_BACKREF processing
 */

	case CC_I_BACKREF | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_ibackref;
/*
 * ignore case subexpression backreference - variable number of matches
 *   continue if subexpression match was zero-length
 *   match minimum number of required times
 *     return no match if cannot meet minimum count
 *   save new string position for backtracking
 *   match maximum number of required times, where max is
 *     number of remaining matches
 *   break-out to match remaining pattern/string
 */

	cc_ibackref:
		idx = *ppat++;
		psa = pe->TMATCH[idx].sa;
		pea = pe->TMATCH[idx].ea;
		count = pea - psa;
		if (count == 0)
			continue;
		while (min-- > 0)
			{
			pback = psa;
			while (pback < pea)
				{
				if ((*pback !=  *pstr) && (tolower(*pback) != tolower(*pback)))
					goto no_match;
				pback++;
				pstr++;
				}
			}
		pstop = pstr;
		while (max-- > 0)
			{
			pback = psa;
			ptemp = pstr;
			count2 = count;
			do
				{
				if ((*pback !=  *pstr) && (tolower(*pback) != tolower(*pback)))
					break;
				pback++;
				pstr++;
				}
			while (--count2 > 0);
			if (count2 == 0)
				pstr = ptemp;
			else
				break;
			}
		break;
/*
 * ignore case single multibyte character, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_I_WCHAR processing
 */

	case CC_I_WCHAR | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		goto cc_iwchar;
/*
 * ignore case single multibyte character, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_I_WCHAR processing
 */

	case CC_I_WCHAR | CR_INTERVAL_ALL:
		min = *ppat++;
		ppat++;
		max = INT_MAX - 1;
		goto cc_iwchar;
/*
 * ignore case single multibyte character, one or more occurances "+"
 *   define min/max and jump to common CC_I_WCHAR processing
 */

	case CC_I_WCHAR | CR_PLUS:
		min = 1;
		max = INT_MAX - 1;
		goto cc_iwchar;
/*
 * ignore case single multibyte character, zero or one occurances "?"
 *   define min/max and jump to common CC_I_WCHAR processing
 */

	case CC_I_WCHAR | CR_QUESTION:
		min = 0;
		max = 1;
		goto cc_iwchar;
/*
 * ignore case single multibyte character, zero or more occurances "*"
 *   define min/max and jump to common CC_I_WCHAR processing
 */

	case CC_I_WCHAR | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_iwchar;
/*
 * ignore case single multibyte character - variable number of matches
 *   match minimum number of required times
 *     return no match if cannot meet minimum count
 *   save new string position for backtracking
 *   match maximum number of required times, where max is
 *     number of remaining matches
 *   break-out to match remaining pattern/string
 */

	cc_iwchar:
		count = *ppat++;
		while (min-- > 0)
			{
			if (strncmp(ppat, pstr, count) != 0)
				if (strncmp(ppat+count, pstr, count) != 0)
					goto no_match;
			pstr += count;
			}
		pstop = pstr;
		while (max-- > 0)
			{
			if (strncmp(ppat, pstr, count) != 0)
				if (strncmp(ppat+count, pstr, count) != 0)
					break;
			pstr += count;
			}
		ppat += count * 2;
		break;
/*
 * ignore case single character, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_I_CHAR processing
 */

	case CC_I_CHAR | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		goto cc_ichar;
/*
 * ignore case single character, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_I_CHAR processing
 */

	case CC_I_CHAR | CR_INTERVAL_ALL:
		min = *ppat++;
		ppat++;
		max = INT_MAX - 1;
		goto cc_ichar;
/*
 * ignore case single character, one or more occurances "+"
 *   define min/max and jump to common CC_I_CHAR processing
 */

	case CC_I_CHAR | CR_PLUS:
		min = 1;
		max = INT_MAX - 1;
		goto cc_ichar;
/*
 * ignore case single character, zero or one occurances "?"
 *   define min/max and jump to common CC_I_CHAR processing
 */

	case CC_I_CHAR | CR_QUESTION:
		min = 0;
		max = 1;
		goto cc_ichar;
/*
 * ignore case single character, zero or more occurances "*"
 *   define min/max and jump to common CC_I_CHAR processing
 */

	case CC_I_CHAR | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_ichar;
/*
 * ignore case single character - variable number of matches
 *   match minimum number of required times
 *     return no match if cannot meet minimum count
 *   save new string position for backtracking
 *   match maximum number of required times, where max is
 *     number of remaining matches
 *   break-out to match remaining pattern/string
 */

	cc_ichar:
		cp = *ppat++;
		cp2 = *ppat++;
		while (min-- > 0)
			{
			if (cp != *pstr && cp2 != *pstr)
				goto no_match;
			pstr++;
			}
		pstop = pstr;
		while (max-- > 0 && (cp == *pstr || cp2 == *pstr))
			pstr++;
		break;
/*
 * any single character except <newline> , min/max occurances "{m,n}"
 *   define min/max and jump to common CC_DOTREG processing
 */

	case CC_DOTREG | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		goto cc_dotreg;
/*
 * any single character except <newline> , min/max occurances "{m,n}"
 *   define min/max and jump to common CC_DOTREG processing
 */

	case CC_DOTREG | CR_INTERVAL_ALL:
		min = *ppat++;
		ppat++;
		max = INT_MAX - 1;
		goto cc_dotreg;
/*
 * any single character except <newline>, one or more occurances "+"
 *   define min/max and jump to common CC_DOTREG processing
 */

	case CC_DOTREG | CR_PLUS:
		min = 1;
		max = INT_MAX - 1;
		goto cc_dotreg;
/*
 * any single character except <newline>, zero or one occurances "?"
 *   define min/max and jump to common CC_DOTREG processing
 */

	case CC_DOTREG | CR_QUESTION:
		min = 0;
		max = 1;
		goto cc_dotreg;
/*
 * any single character except <newline>, zero or more occurances "*"
 *   define min/max and jump to common CC_DOTREG processing
 */

	case CC_DOTREG | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_dotreg;
/*
 * any single character except <newline> - variable number of matches
 *   match minimum number of required times
 *     return no match if cannot meet minimum count
 *   save new string position for backtracking
 *   match maximum number of required times, where max is
 *     number of remaining matches
 *   break-out to match remaining pattern/string
 */

	cc_dotreg:
		while (min-- > 0)
			if (*pstr == '\0' || (*pstr++ == '\n' && (pe->flags & REG_NEWLINE) != 0))
				return REG_NOMATCH;
		pstop = pstr;
		while (max-- > 0)
			if (*pstr == '\0')
				break;
			else if (*pstr++ == '\n' && (pe->flags & REG_NEWLINE) != 0)
				{
				pstr--;
				break;
				}
		break;
/*
 * ILS bracket expression, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_WBITMAP processing
 */

	case CC_WBITMAP | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		goto cc_wbitmap;
/*
 * ILS bracket expression, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_WBITMAP processing
 */

	case CC_WBITMAP | CR_INTERVAL_ALL:
		min = *ppat++;
		ppat++;
		max = INT_MAX - 1;
		goto cc_wbitmap;
/*
 * ILS bracket expression, one or more occurances "+"
 *   define min/max and jump to common CC_WBITMAP processing
 */

	case CC_WBITMAP | CR_PLUS:
		min = 1;
		max = INT_MAX - 1;
		goto cc_wbitmap;
/*
 * ILS bracket expression, zero or one occurances "?"
 *   define min/max and jump to common CC_WBITMAP processing
 */

	case CC_WBITMAP | CR_QUESTION:
		min = 0;
		max = 1;
		goto cc_wbitmap;
/*
 * ILS bracket expression, zero or more occurances "*"
 *   define min/max and jump to common CC_WBITMAP processing
 */

	case CC_WBITMAP | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_wbitmap;
/*
 * ILS bracket expression - variable number of matches
 *   match minimum number of required times
 *     return no match if cannot meet minimum count
 *   call match_bit to match remaining pattern
 */

	cc_wbitmap:
		{
		BIT	b;		/* bitmap recursion data		*/

		while (min-- > 0)
			{
			wchar_t	delta;	/* character offset into bitmap		*/
			wchar_t	ucoll;	/* unique collation weight		*/

			ucoll = _mbucoll(pe->pcoltbl, pstr, &ptemp);
			if (ucoll >= preg->re_ucoll[0] && ucoll <= preg->re_ucoll[1])
				{
				delta = ucoll - preg->re_ucoll[0];
				if ((*(ppat + (delta >> 3)) & __reg_bits[delta & 7]) == 0)
					goto no_match;
				pstr = ptemp;
				}
			else
				goto no_match;
			}
		b.bit_count = 0;
		b.bit_max = max;
		if (match_bit(ppat, pstr, preg, pe, &b))
			goto no_match;
		else
			return 0;
		}
/*
 * any single ILS character, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_WDOT processing
 */

	case CC_WDOT | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		goto cc_wdot;
/*
 * any single ILS character, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_WDOT processing
 */

	case CC_WDOT | CR_INTERVAL_ALL:
		min = *ppat++;
		ppat++;
		max = INT_MAX - 1;
		goto cc_wdot;
/*
 * any single ILS character, one or more occurances "+"
 *   define min/max and jump to common CC_WDOT processing
 */

	case CC_WDOT | CR_PLUS:
		min = 1;
		max = INT_MAX - 1;
		goto cc_wdot;
/*
 * any single ILS character, zero or one occurances "?"
 *   define min/max and jump to common CC_WDOT processing
 */

	case CC_WDOT | CR_QUESTION:
		min = 0;
		max = 1;
		goto cc_wdot;
/*
 * any single ILS character, zero or more occurances "*"
 *   define min/max and jump to common CC_WDOT processing
 */

	case CC_WDOT | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_wdot;
/*
 * any single ILS character - variable number of matches
 *   match minimum number of required times
 *     return no match if cannot meet minimum count
 *   call match_dot to match remaining pattern
 */

	cc_wdot:
		{
		BIT	b;		/* period recursion data		*/

		while (min-- > 0)
			{
			if (*pstr == '\0' || (*pstr == '\n' && (pe->flags & REG_NEWLINE) != 0))
				return REG_NOMATCH;
			wclen = mblen(pstr, mb_cur_max);
			if (wclen < 0)
				wclen = 1;
			pstr += wclen;
			}
		b.bit_count = 0;
		b.bit_max = max;
		if (match_dot(ppat, pstr, preg, pe, &b))
			goto no_match;
		else
			return 0;
		}
/*
 * single multibyte character, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_WCHAR processing
 */

	case CC_WCHAR | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		goto cc_wchar;
/*
 * single multibyte character, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_WCHAR processing
 */

	case CC_WCHAR | CR_INTERVAL_ALL:
		min = *ppat++;
		ppat++;
		max = INT_MAX - 1;
		goto cc_wchar;
/*
 * single multibyte character, one or more occurances "+"
 *   define min/max and jump to common CC_WCHAR processing
 */

	case CC_WCHAR | CR_PLUS:
		min = 1;
		max = INT_MAX - 1;
		goto cc_wchar;
/*
 * single multibyte character, zero or one occurances "?"
 *   define min/max and jump to common CC_WCHAR processing
 */

	case CC_WCHAR | CR_QUESTION:
		min = 0;
		max = 1;
		goto cc_wchar;
/*
 * single multibyte character, zero or more occurances "*"
 *   define min/max and jump to common CC_WCHAR processing
 */

	case CC_WCHAR | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_wchar;
/*
 * single multibyte character - variable number of matches
 *   match minimum number of required times
 *     return no match if cannot meet minimum count
 *   save new string position for backtracking
 *   match maximum number of required times, where max is
 *     number of remaining matches
 *   break-out to match remaining pattern/string
 */

	cc_wchar:
		count = *ppat++;
		while (min-- > 0)
			for (wclen = 0, ptemp = ppat; wclen < count; wclen++)
				if (*ptemp++ != *pstr++)
					goto no_match;
		pstop = pstr;
		while (max-- > 0)
			{
			for (wclen = 0, ptemp = ppat, psa = pstr; wclen < count; wclen++)
				if (*ptemp++ != *psa++)
					break;
			if (wclen < count)
				break;
			else
				pstr += count;
			}
		ppat += count;
		break;
/*
 * end subexpression, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_SUBEXP_E processing
 */

	case CC_SUBEXP_E | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		/* if min = max = 0, then no match: "A(a){0}B" shouldn't match "AaB" */
		if (min == 0 && max == 0)
			goto no_match;
		goto cc_subexpe;
/*
 * end subexpression, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_SUBEXP_E processing
 */

	case CC_SUBEXP_E | CR_INTERVAL_ALL:
		min = *ppat++;
		ppat++;
		max = INT_MAX - min - 1;
		goto cc_subexpe;
/*
 * end subexpression, one or more occurances "+"
 *   define min/max and jump to common CC_SUBEXP_E processing
 */

	case CC_SUBEXP_E | CR_PLUS:
		min = 1;
		max = INT_MAX - 1;
		goto cc_subexpe;
/*
 * end subexpression, zero or one occurances "?"
 *   define min/max and jump to common CC_SUBEXP_E processing
 */

	case CC_SUBEXP_E | CR_QUESTION:
		min = 0;
		max = 1;
		goto cc_subexpe;
/*
 * end subexpression, zero or more occurances "*"
 *   define min/max and jump to common CC_SUBEXP_E processing
 */

	case CC_SUBEXP_E | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_subexpe;
/*
 * end subexpression - variable number of matches
 *   save subexpression ending address
 *   if zero-length match, continue with remaining pattern if
 *     at or below minimum # of required matches
 *     otherwise return an error so that the last previous string
 *     matching locations can be used
 *   increment # of subexpression matches
 *   if the maximum # of required matches have not been found,
 *     reexecute the subexpression
 *     if it succeeds or fails without reaching the minimum # of matches
 *       return with the appropriate status
 *   if maximum number of matches found or the last match_re() failed and
 *     the minimum # of matches have been found, continue matching the
 *     remaining pattern against the remaining string
 */

	cc_subexpe:
		idx = *ppat++;
		pe->TMATCH[idx].ea = pstr;
		if (pe->TMATCH[idx].ea == pe->TMATCH[idx].sa)
			if (pe->SUBMATCH[idx] < min)
				continue;
			else
				goto no_match;
		pe->SUBMATCH[idx]++;
		if (pe->SUBMATCH[idx] < min + max)
			{
			COPY(r, *pe, 1);
			stat = match_re((uchar_t *)preg->re_lsub[idx], pstr, preg, &r);
			if (stat != REG_NOMATCH || pe->SUBMATCH[idx] < min)
				{
				if (stat == 0)
					COPY(*pe, r, 0);
				return (stat);
				}
			}
		continue;
/*
 * subexpression backreference, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_BACKREF processing
 */

	case CC_BACKREF | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		goto cc_backref;
/*
 * subexpression backreference, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_BACKREF processing
 */

	case CC_BACKREF | CR_INTERVAL_ALL:
		min = *ppat++;
		ppat++;
		max = INT_MAX -1;
		goto cc_backref;
/*
 * subexpression backreference, zero or more occurances "*"
 *   define min/max and jump to common CC_BACKREF processing
 */

	case CC_BACKREF | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_backref;
/*
 * subexpression backreference - variable number of matches
 *   continue if subexpression match was zero-length
 *   match minimum number of required times
 *     return no match if cannot meet minimum count
 *   save new string position for backtracking
 *   match maximum number of required times, where max is
 *     number of remaining matches
 *   break-out to match remaining pattern/string
 */

	cc_backref:
		idx = *ppat++;
		psa = pe->TMATCH[idx].sa;
		pea = pe->TMATCH[idx].ea;
		count = pea - psa;
		if (count == 0)
			continue;
		while (min-- > 0)
			{
			pback = psa;
			while (pback < pea)
				if (*pback++ != *pstr++)
					goto no_match;
			}
		pstop = pstr;
		while (max-- > 0)
			{
			if (strncmp(psa, pstr, count) != 0)
				break;
			pstr += count;
			}
		break;
/*
 * bracket expression, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_BITMAP processing
 */

	case CC_BITMAP | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		goto cc_bitmap;
/*
 * bracket expression, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_BITMAP processing
 */

	case CC_BITMAP | CR_INTERVAL_ALL:
		min = *ppat++;
		ppat++;
		max = INT_MAX - 1;
		goto cc_bitmap;
/*
 * bracket expression, one or more occurances "+"
 *   define min/max and jump to common CC_BITMAP processing
 */

	case CC_BITMAP | CR_PLUS:
		min = 1;
		max = INT_MAX - 1;
		goto cc_bitmap;
/*
 * bracket expression, zero or one occurances "?"
 *   define min/max and jump to common CC_BITMAP processing
 */

	case CC_BITMAP | CR_QUESTION:
		min = 0;
		max = 1;
		goto cc_bitmap;
/*
 * bracket expression, zero or more occurances "*"
 *   define min/max and jump to common CC_BITMAP processing
 */

	case CC_BITMAP | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_bitmap;
/*
 * bracket expression - variable number of matches
 *   match minimum number of required times
 *     return no match if cannot meet minimum count
 *   save new string position for backtracking
 *   match maximum number of required times, where max is
 *     number of remaining matches
 *   break-out to match remaining pattern/string
 */

	cc_bitmap:
		while (min-- > 0)
			{
			cs = *pstr++;
			if ((*(ppat + (cs >> 3)) & __reg_bits[cs & 7]) == 0)
				goto no_match;
			}
		pstop = pstr;
		while (max-- > 0)
			{
			cs = *pstr;
			if ((*(ppat + (cs >> 3)) & __reg_bits[cs & 7]) != 0)
				pstr++;
			else
				break;
			}
		ppat += BITMAP_LEN;
		break;
/*
 * any single character, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_DOT processing
 */

	case CC_DOT | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		goto cc_dot;
/*
 * any single character, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_DOT processing
 */

	case CC_DOT | CR_INTERVAL_ALL:
		min = *ppat++;
		ppat++;
		max = INT_MAX - 1;
		goto cc_dot;
/*
 * any single character, one or more occurances "+"
 *   define min/max and jump to common CC_DOT processing
 */

	case CC_DOT | CR_PLUS:
		min = 1;
		max = INT_MAX - 1;
		goto cc_dot;
/*
 * any single character, zero or one occurances "?"
 *   define min/max and jump to common CC_DOT processing
 */

	case CC_DOT | CR_QUESTION:
		min = 0;
		max = 1;
		goto cc_dot;
/*
 * any single character, zero or more occurances "*"
 *   define min/max and jump to common CC_DOT processing
 */

	case CC_DOT | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_dot;
/*
 * any single character - variable number of matches
 *   match minimum number of required times
 *     return no match if cannot meet minimum count
 *   save new string position for backtracking
 *   match maximum number of required times, where max is
 *     number of remaining matches
 *   break-out to match remaining pattern/string
 */

	cc_dot:
		while (min-- > 0)
			if (*pstr++ == '\0')
				return REG_NOMATCH;
		pstop = pstr;
		while (max-- > 0)
			if (*pstr++ == '\0')
				{
				pstr--;
				break;
				}
		break;
/*
 * single character, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_CHAR processing
 */

	case CC_CHAR | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		goto cc_char;
/*
 * single character, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_CHAR processing
 */

	case CC_CHAR | CR_INTERVAL_ALL:
		min = *ppat++;
		ppat++;
		max = INT_MAX - 1;
		goto cc_char;
/*
 * single character, one or more occurances "+"
 *   define min/max and jump to common CC_CHAR processing
 */

	case CC_CHAR | CR_PLUS:
		min = 1;
		max = INT_MAX - 1;
		goto cc_char;
/*
 * single character, zero or one occurances "?"
 *   define min/max and jump to common CC_CHAR processing
 */

	case CC_CHAR | CR_QUESTION:
		min = 0;
		max = 1;
		goto cc_char;
/*
 * single character, zero or more occurances "*"
 *   define min/max and jump to common CC_CHAR processing
 */

	case CC_CHAR | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_char;
/*
 * single character - variable number of matches
 *   match minimum number of required times
 *     return no match if cannot meet minimum count
 *   save new string position for backtracking
 *   match maximum number of required times, where max is
 *     number of remaining matches
 *   break-out to match remaining pattern/string
 */

	cc_char:
		cp = *ppat++;
		while (min-- > 0)
			if (cp != *pstr++)
				goto no_match;
		pstop = pstr;
		while (max-- > 0 && cp == *pstr)
			pstr++;
		break;
		} /* switch */
	break;
	} /* while */
/*
 * surplus matched expressions end up here
 * generate new copy of recursion data
 * match remaining pattern against remaining string
 * if remaining pattern match fails, forfeit one extra matched
 *   character and try again until no spare matches are left
 * return success and new recursion data if entire remaining pattern matches
 * otherwise return no match
 */
	while (1)
		{
		COPY(r, *pe, 1);
		stat = match_re(ppat, pstr, preg, &r);
		if (stat != REG_NOMATCH)
			{
			if (stat == 0)
				COPY(*pe, r, 0);
			return (stat);
			}
		if (pstr <= pstop)
			break;
		pstr -= count;;
		}
	goto no_match;
}


/************************************************************************/
/* match_dot()	- Match period to remainder of string			*/
/*		- used for all codesets except C locale			*/
/*									*/
/*		- ppat		ptr to pattern				*/
/*		- pstr		ptr to string				*/
/*		- preg		ptr to caller's regex_t structure	*/
/*		- pe		ptr to recursion data structure		*/
/*		- pb		ptr to recursion period data		*/
/************************************************************************/

static int
match_dot(uchar_t *ppat, uchar_t *pstr, regex_t *preg, EXEC *pe, BIT *pb)
{
	int	i;		/* # bytes in character			*/
	EXEC	r;		/* another copy of *pe for recursion	*/
	submatch_t ints;	/* Buffer.				*/
	tmatch_t   eats;	/* Buffer (best match so far).		*/
	submatch_t best_s;	/* Buffer.				*/
	tmatch_t   best_t;	/* Buffer (bset match so far).		*/
	submatch_t *sv_submatch;/* Used by COPY().			*/
	tmatch_t   *sv_tmatch;  /* Used by COPY().			*/

/*
 * Attempt another . match if maximum not reached yet
 * If successful, attempt next match via recursion
 */

	if (*pstr != '\0' && (*pstr != '\n' || (pe->flags & REG_NEWLINE) == 0) && pb->bit_count < pb->bit_max)
		{
		pb->bit_count++;
		i = mblen(pstr, MB_CUR_MAX);
		if (i < 0)
			i = 1;
		if (match_dot(ppat, pstr + i, preg, pe, pb) == 0)
			return (0);
		}
	COPY(r, *pe, 1);
	if ((i = match_re(ppat, pstr, preg, &r)) == 0)
		COPY(*pe, r, 0);
	return (i);
}


/************************************************************************/
/* match_bit()	- Match bracket expression [] to remainder of string	*/
/*		- used for all codesets except C locale			*/
/*									*/
/*		- ppat		ptr to pattern				*/
/*		- pstr		ptr to string				*/
/*		- preg		ptr to caller's regex_t structure	*/
/*		- pe		ptr to recursion data structure		*/
/*		- pb		ptr to recursion bitmap data		*/
/************************************************************************/

static int
match_bit(uchar_t *ppat, uchar_t *pstr, regex_t *preg, EXEC *pe, BIT *pb)
{
	wchar_t	delta;		/* character offset into bitmap		*/
	wchar_t	ucoll;		/* unique collation weight		*/
	uchar_t	*pnext;		/* ptr to next collation symbol		*/
	int	stat;		/* match_re() return status		*/
	EXEC	r;		/* another copy of *pe for recursion	*/
	submatch_t ints;	/* Buffer.				*/
	tmatch_t   eats;	/* Buffer (best match so far).		*/
	submatch_t best_s;	/* Buffer.				*/
	tmatch_t   best_t;	/* Buffer (bset match so far).		*/
	submatch_t *sv_submatch;/* Used by COPY().			*/
	tmatch_t   *sv_tmatch;  /* Used by COPY().			*/

/*
 * Attempt another [] match if maximum not reached yet
 * If successful, attempt next match via recursion
 */
	if (*pstr != '\0' && pb->bit_count < pb->bit_max)
		{
		ucoll = _mbucoll(pe->pcoltbl, pstr, &pnext);
		if (ucoll >= preg->re_ucoll[0] && ucoll <= preg->re_ucoll[1])
			{
			delta = ucoll - preg->re_ucoll[0];
			if ((*(ppat + (delta >> 3)) & __reg_bits[delta & 7]) != 0)
				{
				pb->bit_count++;
				if (match_bit(ppat, pnext, preg, pe, pb) == 0)
					return (0);
				}
			}
		}
	COPY(r, *pe, 1);
	ppat += ((preg->re_ucoll[1] - preg->re_ucoll[0]) / NBBY) + 1;
	if ((stat = match_re(ppat, pstr, preg, &r)) == 0)
		COPY(*pe, r, 0);
	return (stat);
}
