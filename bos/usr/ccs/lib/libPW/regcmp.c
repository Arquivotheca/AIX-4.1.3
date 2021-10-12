static char sccsid[] = "@(#)30	1.11  src/bos/usr/ccs/lib/libPW/regcmp.c, libPW, bos411, 9428A410j 6/3/91 10:20:29";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: regcmp
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
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

#include <varargs.h>
#include <malloc.h>
/* note for man page: collating sequence used by regex but not regcmp */

/*
 * FUNCTION: Compile patterns.
 *
 * RETURN VALUE DESCRIPTIONS:
 *		char * to compiled expression if successful
 *		NULL on ERROR
 */

#define SSIZE   25
#define	NBRA	10
#define SLOP	5
#define	EOF	0
/*				STAR PLUS RNGE   		*/
#define	NCCL	8	/* 010	 011  012  013   inverted []  	*/
#define	CKET	12	/* 014			 end group )    */
#define MINUS	16	/* 020			 dashrange foll */
#define	CCHR	20	/* 024	 025  026  027   char		*/
#define	CCL	24	/* 030	 031  032  033   [] expr	*/
#define	CDOL	28	/* 034			 trail. anchor  */
#define CIRCFL  32	/* 040			 lead. anchor	*/
#define GRP	40	/* 050			 group		*/
#define EGRP	44	/* 054			 		*/
#define TGRP	48	/* 060			 brack. group   */
#define	CEOF	52	/* 064			 expr trailer   */
#define SGRP	56	/* 070			 stargroup      */
#define	CBRA	60	/* 074			 start group (	*/
#define	CDOT	64	/* 0100  101  102  103   dot 		*/
#define PGRP	68	/* 0104			 plusgroup      */

#define	STAR	01
#define PLUS	02
#define RNGE	03

int	__i_size;

/*VARARGS*/
char *
regcmp(va_alist) va_dcl
{
	register char *ep, *sp;
	register ctwo;
	char eptyp;
	register c, i;
	register char *lastep, *eptr, *sep, **stkp;
	register cclcnt, cflg, nbra = 0, ngrp = 0;
	register char **stkmax;
	va_list ap;
	char *stack[SSIZE];

	stkp = stack;
	stkmax = &stack[SSIZE];

	va_start(ap);
	for (i = 0; sp = va_arg(ap, char *); )  i += strlen(sp);
	va_end(ap);
	if((sep = ep = (char *)malloc(2*i+SLOP)) == 0)
		return(0);
	va_start(ap);
	sp = va_arg(ap, char *);
	if (*sp == '^' ) {
		++sp;
		*ep++ = CIRCFL;
	}
	for (lastep = 0;;) {
		if ((c = *sp++) == EOF) {
			if (sp = va_arg(ap, char *)) continue;
done:
			if (nbra > NBRA || stkp > stack || ep == sep)
				goto cerror;
			*ep++ = CEOF;
			__i_size = ep - sep;
			va_end(ap);
			return(sep);
		}
		if ((c!='*') && (c!='{')  && (c!='+'))
			lastep = ep;
		else
			if (!lastep)
				goto cerror;

		switch (c) {

		case '(':
			if (stkp >= stkmax) goto cerror;
			*stkp++ = ep;
			*ep++ = CBRA;
			*ep++ = -1;
			continue;
		case ')':
			if (stkp == stack) goto cerror;
			eptr = *--stkp;
			if ((c = *sp++) == '$') {
				if ('0' > (c = *sp++) || c > '9')
					goto cerror;
				*ep++ = CKET;
				*ep++ = *++eptr = nbra++;
				*ep++ = (c-'0');
				lastep = 0;
				continue;
			}
			sp--;
			switch (c) {
			case '+':
				*eptr = PGRP;
				break;
			case '*':
				*eptr = SGRP;
				break;
			case '{':
				*eptr = TGRP;
				break;
			default:
				*eptr = GRP;
				continue;
			}
			*ep++ = EGRP;
			*ep++ = ngrp++;
			i = ep - eptr - 2;
			if (i >= (1<<8)<<2) goto cerror;
			*eptr |= i>>8;
			*++eptr = i & (1<<8)-1;
			continue;

		case '\\':
			*ep++ = CCHR;
			if ((c = *sp++) == EOF)
				goto cerror;
			*ep++ = c;
			continue;

		case '{':
			*lastep |= RNGE;
			cflg = 0;
			c = *sp++;
		nlim:
			i = 0;
			do {
				if ('0' <= c && c <= '9')
					i = (i*10+(c-'0'));
				else goto cerror;
			} while (((c = *sp++) != '}') && (c != ','));
			if (i>255) goto cerror;
			*ep++ = i;
			if (c==',') {
				if (cflg++) goto cerror;
				if((c = *sp++) == '}') {
					*ep++ = -1;
					continue;
				}
				else
					goto nlim;
			}
			if (!cflg) *ep++ = i;
			else if ((ep[-1]&0377) < (ep[-2]&0377)) goto cerror;
			continue;

		case '.':
			*ep++ = CDOT;
			continue;

		case '+':
			*lastep |= PLUS;
			continue;

		case '*':
			*lastep |= STAR;
			continue;

		case '[':
			*ep++ = CCL;
			*ep++ = 0;
			cclcnt = 1;
			if ((c = *sp++) == '^') {
				c = *sp++;
				ep[-2] = NCCL;
			}
			do {
				if (c==EOF)
					goto cerror;
				if ((c=='-') && (cclcnt>1) && (*sp!=']')) {
					*ep = ep[-1];
					ep[-1] = MINUS;
					ep++;
					cclcnt++;
					continue;
				}
				*ep++ = c;
				cclcnt++;
			} while ((c = *sp++) != ']');
			lastep[1] = cclcnt;
			continue;

		case '$':
			if (*sp == EOF && !(sp = va_arg(ap, char *))) {
				*ep++ = CDOL;
				goto done;
			}
		default:
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
   cerror:
	free((void *)sep);
	va_end(ap);
	return(0);
}
