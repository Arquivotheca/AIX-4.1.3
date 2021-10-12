static char sccsid[] = "@(#)07	1.5  src/bos/usr/bin/localedef/check.c, cmdnls, bos411, 9428A410j 3/10/94 10:57:26";
/*
 * COMPONENT_NAME: (CMDNLS) Locale Database Commands
 *
 * FUNCTIONS: check_upper, check_lower, check_alpha, check_space,
 *	      check_cntl, check_print, check_graph, check_punct
 *	      check_digits, check_xdigit
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/types.h>
#include <sys/localedef.h>
#include <ctype.h>
#include "semstack.h"
#include "symtab.h"
#include "err.h"

#define TERMINATOR	NULL

/* All these functions follow the same logic. For 0 through max_wchar_enc
   check the mask of the char. If the character has the characteristic
   being tested, check further to make sure it has none of the characteristic's
   that are invalid. (ie. an upper can not be a control, a punct, a digit, 
   or a space). These are all POSIX checks (from Draft 11). 

*/ 

void
check_upper()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;
	char *uppers[]={	/* upper case character symbols as	*/
	"<A>",			/* defined in XPG4 System Interface	*/
	"<B>",			/* sections 5.3.1 and 4.1, table 4-1	*/
	"<C>",
	"<D>",
	"<E>",
	"<F>",
	"<G>",
	"<H>",
	"<I>",
	"<J>",
	"<K>",
	"<L>",
	"<M>",
	"<N>",
	"<O>",
	"<P>",
	"<Q>",
	"<R>",
	"<S>",
	"<T>",
	"<U>",
	"<V>",
	"<W>",
	"<X>",
	"<Y>",
	"<Z>",
	TERMINATOR};	/* terminating value which will never occur */

	for (i = 0 ; i <= max_wchar_enc; i++){
	    if (ctype.mask[i] & _ISUPPER) 
		if ((ctype.mask[i] & _ISCNTRL) ||
		    (ctype.mask[i] & _ISDIGIT) ||
		    (ctype.mask[i] & _ISPUNCT) ||
		    (ctype.mask[i] & _ISSPACE))
		    fail=TRUE;
	}
	if (fail)
	    diag_error(ERR_INVALID_UPPER);

	fail=FALSE;
	for (i = 0; uppers[i] != TERMINATOR; i++){
	    if (!(get_ctype_mask(uppers[i]) & _ISUPPER)){
		fail=TRUE;
	    }
	}
	if (fail)
	    diag_error(ERR_MIN_UPPER);
	return;
}

void
check_lower()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;

	char *lowers[]={	/* lower case character symbols as	*/
	"<a>",			/* defined in XPG4 System Interface	*/
	"<b>",			/* sections 5.3.1 and 4.1, table 4-1	*/
	"<c>",
	"<d>",
	"<e>",
	"<f>",
	"<g>",
	"<h>",
	"<i>",
	"<j>",
	"<k>",
	"<l>",
	"<m>",
	"<n>",
	"<o>",
	"<p>",
	"<q>",
	"<r>",
	"<s>",
	"<t>",
	"<u>",
	"<v>",
	"<w>",
	"<x>",
	"<y>",
	"<z>",
	TERMINATOR};	/* terminating value which will never occur */

	for (i = 0 ; i <= max_wchar_enc; i++){
	    if (ctype.mask[i] & _ISLOWER) 
		if ((ctype.mask[i] & _ISCNTRL) ||
		    (ctype.mask[i] & _ISDIGIT) ||
		    (ctype.mask[i] & _ISPUNCT) ||
		    (ctype.mask[i] & _ISSPACE))
		    fail=TRUE;
	}
	if (fail)
 	    diag_error(ERR_INVALID_LOWER);
	fail=FALSE;
	for (i = 0; lowers[i] != TERMINATOR; i++){
	    if (!(get_ctype_mask(lowers[i]) & _ISLOWER))
		fail = TRUE;
	}
	if (fail)
	    diag_error(ERR_MIN_LOWER);
	return;
}

void
check_alpha()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;

	for (i = 0 ; i <= max_wchar_enc; i++){
	    if (ctype.mask[i] & _ISALPHA) 
		if ((ctype.mask[i] & _ISCNTRL) ||
		    (ctype.mask[i] & _ISDIGIT) ||
		    (ctype.mask[i] & _ISPUNCT) ||
		    (ctype.mask[i] & _ISSPACE))
		    fail=TRUE;
	}
	if (fail)
	    diag_error(ERR_INVALID_ALPHA);
	return;
}

void
check_space()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;

	char *spaces[]={	/* space character symbols as		*/
	"<space>",		/* defined in XPG4 System Interface	*/
	"<form-feed>",		/* sections 5.3.1 and 4.1, table 4-1, 	*/
	"<newline>",
	"<carriage-return>",
	"<tab>",
	"<vertical-tab>",
	TERMINATOR};	/* terminating value which will never occur */

	for (i = 0 ; i <= max_wchar_enc; i++){
	    if (ctype.mask[i] & _ISSPACE) 
		if ((ctype.mask[i] & _ISDIGIT) ||
		    (ctype.mask[i] & _ISUPPER) ||
		    (ctype.mask[i] & _ISLOWER) ||
		    (ctype.mask[i] & _ISALPHA) ||
		    (ctype.mask[i] & _ISXDIGIT))
		    fail=TRUE;
	}
	if (fail)
	    diag_error(ERR_INVALID_SPACE);

	fail=FALSE;
	for (i = 0; spaces[i] != TERMINATOR; i++){
	    if (!(get_ctype_mask(spaces[i]) & _ISSPACE))
		fail = TRUE;
	}

	if (fail)
	    diag_error(ERR_MIN_SPACE);

}

void
check_cntl()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;

	for (i = 0 ; i <= max_wchar_enc; i++){
	    if (ctype.mask[i] & _ISCNTRL) 
		if ((ctype.mask[i] & _ISUPPER) ||
		    (ctype.mask[i] & _ISLOWER) ||
		    (ctype.mask[i] & _ISALPHA) ||
		    (ctype.mask[i] & _ISDIGIT) ||
		    (ctype.mask[i] & _ISGRAPH) ||
		    (ctype.mask[i] & _ISPUNCT) ||
		    (ctype.mask[i] & _ISPRINT) ||
		    (ctype.mask[i] & _ISXDIGIT))
		    fail=TRUE;
	}
	if (fail)
	    diag_error(ERR_INVALID_CNTRL);
	return;
}

void
check_punct()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;

	for (i = 0 ; i <= max_wchar_enc; i++){
	    if (ctype.mask[i] & _ISPUNCT) 
		if ((ctype.mask[i] & _ISUPPER) ||
		    (ctype.mask[i] & _ISLOWER) ||
		    (ctype.mask[i] & _ISALPHA) ||
		    (ctype.mask[i] & _ISDIGIT) ||
		    (ctype.mask[i] & _ISCNTRL) ||
		    (ctype.mask[i] & _ISXDIGIT) ||
		    (ctype.mask[i] & _ISSPACE))
		    fail=TRUE;
	}
	if (fail)
	    diag_error(ERR_INVALID_PUNCT);
	return;
}
void
check_graph()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;

	for (i = 0 ; i <= max_wchar_enc; i++){
	    if (ctype.mask[i] & _ISGRAPH) 
		if (ctype.mask[i] & _ISCNTRL) 
		    fail=TRUE;
	}
	if (fail)
	    diag_error(ERR_INVALID_GRAPH);
	return;
}

void
check_print()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;

	for (i = 0 ; i <= max_wchar_enc; i++){
	    if (ctype.mask[i] & _ISPRINT) 
		if (ctype.mask[i] & _ISCNTRL) 
		    fail=TRUE;
	}
	if (fail)
	    diag_error(ERR_INVALID_PRINT);
	return;
}

void
check_digits()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;
	int count_of_digit_chars=0;
	
	char *digits[]={	/* digit character symbols as		*/
	"<zero>",		/* defined in XPG4 System Interface	*/
	"<one>",		/* sections 5.3.1 and 4.1, table 4-1, 	*/
	"<two>",
	"<three>",
	"<four>",
	"<five>",
	"<six>",
	"<seven>",
	"<eight>",
	"<nine>",
	TERMINATOR};	/* terminating value which will never occur */

	for (i = 0; i <= max_wchar_enc; i++) {
		if (ctype.mask[i] & _ISDIGIT)
		    count_of_digit_chars++;
	}

	for (i = 0; digits[i] != TERMINATOR; i++){
	    if (!(get_ctype_mask(digits[i]) & _ISDIGIT))
		fail = TRUE;
	}

	if ((fail) || (count_of_digit_chars != 10))
	    diag_error(ERR_INV_DIGIT);
	return;
}

void
check_xdigit()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;
	int in_a_row = 0;
	
	char *xdigits[]={	/* xdigit character symbols as		*/
	"<zero>",		/* defined in XPG4 System Interface	*/
	"<one>",		/* sections 5.3.1 and 4.1, table 4-1, 	*/
	"<two>",
	"<three>",
	"<four>",
	"<five>",
	"<six>",
	"<seven>",
	"<eight>",
	"<nine>",
	"<A>",
	"<B>",
	"<C>",
	"<D>",
	"<E>",
	"<F>",
	"<a>",
	"<b>",
	"<c>",
	"<d>",
	"<e>",
	"<f>",
	TERMINATOR};	/* terminating value which will never occur */

	/* make sure that minimum set have been defined */
	for (i = 0; xdigits[i] != TERMINATOR; i++){
	    if (!(get_ctype_mask(xdigits[i]) & _ISXDIGIT))
		fail = TRUE;
	}


	/* each grouping of xdigit character should contain at least */
	/* six characters, and they should be consecutive.  XPG4     */
	/* Technically, there should be one set of 10 (0-9) and at   */
	/* least two sets of 6 (A-F) and (a-f), however since sets   */
	/* of six *might* overlap (yuck), I will require sets of     */
	/* at least 6 consecutive characters.                        */

	
	for (i = 0; i <= max_wchar_enc; i++) {
		if ((!(ctype.mask[i] & _ISXDIGIT)) && 
					(in_a_row > 0) && (in_a_row < 6)) {
			fail = TRUE;
			break;
		}
		if (ctype.mask[i] & _ISXDIGIT)
			in_a_row++;
		else
			in_a_row = 0;
	}

        if (fail)
	    diag_error(ERR_INV_XDIGIT);
     	return;
}	
