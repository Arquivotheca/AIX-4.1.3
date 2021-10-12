static char sccsid[] = "@(#)56  1.5  src/bos/kernext/prnt/prnformat.c, sysxprnt, bos411, 9428A410j 2/4/94 12:52:01";
/*
 *   COMPONENT_NAME: (SYSXPRNT) Parallel Port Printer Device Driver
 *
 *   FUNCTIONS: prnformat
 *		stuffc
 *
 *   ORIGINS: 27,3
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/ppdd.h>
#include <sys/lpio.h>
/*
 * NAME: prnformat
 *
 * FUNCTION:
 *         Converts output according to modes before putting on
 *         output queue (EX. Form Feed-Line Feed, tabs, col width, etc.)
 *         Only called if not in PLOT mode.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function will execute in process state and can page fault
 *
 * (NOTES:)
 *
 * RETURNS: none
 */

void prnformat(
register struct prtinfo *page ,
register uint mode,
register int c)

{
	register char *mark = "\n...";

	if (mode&CAPS) {
		if (c >= 'a' && c <= 'z')
			c += 'A'-'a';
		else {
			switch (c) {
			case '{':
				c = '(';
				prnformat(page, mode,c);
				page->ccc--;
				c = '-';
				break;

			case '}':
				c = ')';
				prnformat(page, mode,c);
				page->ccc--;
				c = '-';
				break;

			case '`':
				c = '\'';
				prnformat(page, mode,c);
				page->ccc--;
				c = '-';
				break;

			case  '|':
				c = '!';
				prnformat(page, mode,c);
				page->ccc--;
				c = '-';
				break;

			case '~':
				c = '^';
				prnformat(page, mode,c);
				page->ccc--;
				c = '-';
				break;
			}
		}
	}

	switch (c) {
	case '\r':
		if (mode&NOCR)
			c = '\n';
		else {
			page->mcc = 0;
			page->ccc = page->ind;
			stuffc(page, c);
			return;
		}

	case '\n':
		if (mode&NONL)
			c = '\r';
		if (++page->mlc >= page->line) {
			if (page->line && ((mode&NOFF) == 0))
				c = FF;
			page->mlc = 0;
		}
		page->mcc = 0;
		page->ccc = page->ind;
		if( ! ( mode & NOCL) ) {
			stuffc(page,(int)'\r');
		}
		stuffc(page, c);
		return;

	case FF:
		if (page->mlc == 0 && page->mcc == 0) {
			page->ccc = page->ind;
			return;
		}
		if (mode&NOFF) {
			while (page->mlc)
				prnformat(page, mode,(int)'\n');
			return;
		}
		page->mlc = 0;

		page->mcc = 0;
		page->ccc = page->ind;
		stuffc(page, c);
		return;

	case '\t':
		while (page->ccc > page->mcc) {         /* Bring mcc to ccc  */
			stuffc(page, (int)' ');
			page->mcc++;
		}
		page->ccc += (8 -(page->ccc & 7)) ;     /* step to next tab  */
		if (mode&NOTB) {                        /* IF NOTB set       */
			page->mcc = page->ccc ;         /* set mcc to ccc    */
			stuffc(page,c) ;                /* send tab          */
		}
		return;                                 /* return            */

	case ' ':
		page->ccc++;
		return;

	case '\b':
		if (mode&NOBS) {
			if (page->ccc > page->ind)
				page->ccc--;
			return;
		}
	}

	if (page->ccc < page->mcc) {
		if (mode&NOCR) {
			stuffc(page, (int)'\n');
			++page->mlc;
		}
		else
			stuffc(page, (int)'\r');
		page->mcc = 0;
	}
	if (page->ccc < page->col) {
		while (page->ccc > page->mcc) {
			stuffc(page, (int)' ');
			page->mcc++;
		}
		stuffc(page, c);
		page->mcc++;
	}
	else {
		if (mode&WRAP) {
			while (*mark)
				prnformat(page, mode,(int)*mark++);
			stuffc(page, c);
			page->mcc++;
		}
	}
	page->ccc++;
} /* end prnformat */


/*
 * NAME: stuffc:
 *
 * FUNCTION: put a character in the out queuet queue
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function will execute in process state and can page fault
 *
 * RETURNS: none
 */

void stuffc(
struct prtinfo *pp,
int c)
{
	while( putc(c, &pp->outq) == -1 )
		waitcfree() ;


} /* end stuffc */
