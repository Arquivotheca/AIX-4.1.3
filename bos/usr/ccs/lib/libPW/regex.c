static char sccsid[] = "@(#)41	1.17  src/bos/usr/ccs/lib/libPW/regex.c, libPW, bos411, 9428A410j 3/30/94 16:13:49";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: regex
 *
 * ORIGINS: 3 27
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
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */

#define _ILS_MACROS
#include <ctype.h>
#include <nl_types.h>
#include<sys/localedef.h>
#include "pw_msg.h"
#include <setjmp.h>


/*
 *                                                                    
 * FUNCTION: compare regular expressions
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     - NULL on failure
 */

static jmp_buf  jmpbuf;

#define SSIZE   50
#define NBRA    10
#ifndef EOF
#define EOF     -1
#endif
/*              dec     octal   STAR PLUS RNGE                  */
#define NCCL    8       /* 010   011  012  013   inverted []    */
#define CKET    12      /* 014                   end group )    */
#define MINUS   16      /* 020                   dashrange foll */
#define CHCL    17      /* 021                   charclass foll */
#define CCHR    20      /* 024   025  026  027   char           */
#define CCL     24      /* 030   031  032  033   [] expr        */
#define CDOL    28      /* 034                   trail. anchor  */
#define CIRCFL  32      /* 040                   lead. anchor   */
#define GRP     40      /* 050                   group          */
#define EGRP    44      /* 054                                  */
#define TGRP    48      /* 060                   brack. group   */
#define CEOF    52      /* 064                   expr trailer   */
#define SGRP    56      /* 070                   stargroup      */
#define CBRA    60      /* 074                   start group (  */
#define CDOT    64      /* 0100  101  102  103   dot            */
#define PGRP    68      /* 0104                  plusgroup      */

#define STAR    01
#define PLUS    02
#define RNGE    03
#define A256    01
#define A512    02
#define A768    03

#define U(c)    (unsigned char)(c)

/* As the "is" functions are macros, not functions, we cannot put       */
/* the "function" in the array below; thus another layer of indirection */

ALPHA(c) {return(isascii(c) && isalpha(c));}
UPPER(c) {return(isascii(c) && isupper(c));}
LOWER(c) {return(isascii(c) && islower(c));}
DIGIT(c) {return(isascii(c) && isdigit(c));}
ALNUM(c) {return(isascii(c) && isalnum(c));}
SPACE(c) {return(isascii(c) && isspace(c));}
PRINT(c) {return(isascii(c) && isprint(c));}
PUNCT(c) {return(isascii(c) && ispunct(c));}
XDIGIT(c) {return(isascii(c) && isxdigit(c));}

struct isarray {
	char *isstr;
	int (*isfunc)();
} istab[] = {
	{ "alpha", ALPHA },
	{ "upper", UPPER },
	{ "lower", LOWER },
	{ "digit", DIGIT },
	{ "alnum", ALNUM },
	{ "space", SPACE },
	{ "print", PRINT },
	{ "punct", PUNCT },
	{ "xdigi", XDIGIT }

#define NISTAB (sizeof(istab) / sizeof(struct isarray))
};

char    *__loc1;

static  char *braslist[NBRA];
static  char *braelist[NBRA];
static  int  bravar[NBRA];
static  char **stkp, **stkmax;

static char *execute(), *advance();
static int cclass();

static push(p) char *p;
{
	int len;
	char *msg;

	nl_catd catd;
	catd=catopen(MF_PW, NL_CAT_LOCALE);
	if (stkp >= stkmax) {
		len=strlen(msg = catgets(catd,MS_PW,STOVFL,"regex: stack overflow\n"));
		write(2,msg, len);
		longjmp(jmpbuf,1);
	}
	catclose(catd);

	*stkp++ = p;
}
#define pop() *--stkp

char *
regex(addrc,addrl,a1) char *addrc, *addrl, *a1;
{
	register char *p1, *p2;
	register in;
	register char **adx;
	register char *cur;

	if (setjmp(jmpbuf))
		return 0;

	for(in=0;in<NBRA;in++)
		bravar[in] = -1;
	cur = execute(addrc,addrl);
	adx = &a1;
	for(in=0;in<NBRA;in++) {
		if (bravar[in] >= 0) {
			p1 = braslist[in];
			p2 = adx[bravar[in]];
			while(p1 < braelist[in]) *p2++ = *p1++;
			*p2 = '\0';
		}
	}
	return cur;
}

static char *
execute(addrc,addrl)
char *addrc,*addrl;
{
	register char *p1, *p2, c, *ret;
	char *stack[SSIZE];

	stkmax = &stack[SSIZE];
	p1 = addrl;
	p2 = addrc;
	if (*p2==CIRCFL) {
		stkp = stack;
		return advance(__loc1 = p1, ++p2);
	}
	/* fast check for first character */
	if (*p2==CCHR) {
		c = p2[1];
		do {
			if (*p1==c) {
				stkp = stack;
				if (ret=advance(p1, p2))  {
				/* duplicate effort but it's much simpler */
					__loc1 = p1;
					return(ret);
				}
			}
		} while (*p1++);
		return(0);
	}
	/* regular algorithm */
	do {
		stkp = stack;
		if (ret=advance(p1, p2))  {
			__loc1 = p1;
			return(ret);
		}
	} while (*p1++);
	return(0);
}

#define getrnge(p) (lcnt = U((p)[0]), \
	(dcnt = U((p)[1])) == U(-1)? (dcnt = 20000) : (dcnt -= lcnt))

static char *
advance(lp, ep)
register char *lp, *ep;
{
	register char *curlp;
	register i, lcnt, dcnt;
	register int c;
	int c2;
	int crc;
	register char *ret;
	register gflg;

#define CCLASS(ep,lp) (crc=cclass(ep,lp), lp += 1, crc)

	for (;;) {
		switch(*ep++) {

	case CCHR:

		c = *ep++;
		if (c == *lp++)
				continue;
		return(0);


	case EGRP|PLUS:
	case EGRP|STAR:
	case EGRP|RNGE:
	case CEOF:
		return(lp);

	case EGRP:
	case GRP:
		ep++;
		continue;

	case CDOT:
		if (*lp == 0)
			return(0);
			lp++;
		continue;

	case CDOL:
		if (*lp==0)
			continue;
		return(0);

	case TGRP:
	case TGRP|A768:
	case TGRP|A512:
	case TGRP|A256:
		i = (ep[-1]&03)<<8;
		i += U(*ep++);
		getrnge(ep+i);
		while(--lcnt >= 0)
			if (!(lp=advance(lp,ep)))
				return(0);
		for(push(curlp=lp); --dcnt >= 0 && (ret=advance(lp,ep));)
			push(lp=ret);
		ep += i + 2;
		gflg = 1;
		goto star;

	case CCHR|RNGE:
		i = *ep++;
		c = 0;
		getrnge(ep);
		while(--lcnt >= 0) {
			if (*lp++ != i) return(0);
			if (c && *lp++ != c) return(0);
		}
		for(curlp = lp; --dcnt >= 0 && *lp == i;) {
			if (c && *(++lp) != c) break;
			lp++;
		}
		ep += 2;
		lp += 1;
		gflg = 0;
		goto star;

	case CDOT|RNGE:
		getrnge(ep);
		while(--lcnt >= 0)
			if(*lp++ == '\0') return(0);
		for(curlp = lp; --dcnt >= 0 && *lp; lp++)
			;
		ep += 2;
		lp += 1;
		gflg = 0;
		goto star;

	case CCL|RNGE:
	case NCCL|RNGE:
		i = U(*ep);
		getrnge(ep+i);
		while(--lcnt >= 0)
			if(!CCLASS(ep,lp)) return(0);
		for(curlp = lp; --dcnt >= 0 && CCLASS(ep,lp);) ;
		ep += i + 2;
		if (dcnt<0) lp++;
		gflg = 0;
		goto star;

	case CCL:
	case NCCL:
		if (CCLASS(ep, lp)) {
			ep += U(*ep);
			continue;
		}
		return(0);

	case CBRA:
		braslist[*ep++] = lp;
		continue;

	case CKET:
		braelist[*ep] = lp;
		bravar[*ep] = ep[1];
		ep += 2;
		continue;

	case CDOT|PLUS:
		if (*lp++ == '\0') return(0);
	case CDOT|STAR:
		curlp = lp;
		while (*lp++);
		gflg = 0;
		goto star;

	case CCHR|PLUS:
		if (*lp++ != *ep) return(0);
	case CCHR|STAR:
		curlp = lp;
		c = *ep++;
		c2 = 0;
		while (*lp++ == c && (!c2 || *lp++ == c2));
		gflg = 0;
		goto star;

	case PGRP:
	case PGRP|A256:
	case PGRP|A512:
	case PGRP|A768:
		if (!(lp=advance(lp,ep+1))) return(0);
	case SGRP|A768:
	case SGRP|A512:
	case SGRP|A256:
	case SGRP:
		i = (ep[-1]&03) << 8;
		i += U(*ep++);
		for(push(curlp=lp); ret=advance(lp,ep);)
			push(lp=ret);
		ep += i;
		gflg = 1;
		goto star;

	case CCL|PLUS:
	case NCCL|PLUS:
		if (!CCLASS(ep,lp)) return(0);
	case CCL|STAR:
	case NCCL|STAR:
		curlp = lp;
		while (CCLASS(ep, lp));
		ep += U(*ep);
		gflg = 0;
		goto star;

	star:
		do {
			if(!gflg) lp--;
			else lp = pop();
			if (ret=advance(lp, ep))
				return(ret);
		} while (lp > curlp);
		return(0);

	default:
		return(0);
	}
	}
}

static cclass(set, ac)
register char *set;
char *ac;
{
	register int c;
	register char *n;
	register f;
	register int s1, s2;
	if (*ac==0) return(0);
	c = *ac++;
	f = set[-1] & (CCL & ~NCCL);
	n = set + U(*set++);
	while (set<n) {
		if (*set == MINUS) {
			++set;
			s1 = *set++;
			s2 = *set++;
			if (s2 - s1 < 0) return(0);
				/* [c-a] matches nothing, so does [^c-a] */
			if (s1 <= c && c <= s2)
				return(f);
			continue;
		}
	s1 = *set++;
	if (s1 == c)
		return(f);
	}
	return(!f);
}

