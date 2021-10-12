/* @(#)82	1.25  src/bos/usr/include/locale.h, libcnls, bos411, 9428A410j 1/12/93 16:59:54 */

/*
 * COMPONENT_NAME: (LIBCNLS) Locale Related Data Structures and API
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef __H_LOCALE
#define __H_LOCALE

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifdef _ANSI_C_SOURCE

#ifndef		NULL
#define		NULL	((void *)0)
#endif /* NULL */

struct lconv {
   char *decimal_point;		/* decimal point character		*/
   char *thousands_sep;		/* thousands separator		 	*/
   char *grouping;		/* digit grouping		 	*/
   char *int_curr_symbol;	/* international currency symbol	*/
   char *currency_symbol;	/* national currency symbol		*/
   char *mon_decimal_point;	/* currency decimal point		*/
   char *mon_thousands_sep;	/* currency thousands separator		*/
   char *mon_grouping;		/* currency digits grouping		*/
   char *positive_sign;		/* currency plus sign			*/
   char *negative_sign;		/* currency minus sign		 	*/
   char int_frac_digits;	/* internat currency fractional digits	*/
   char frac_digits;		/* currency fractional digits		*/
   char p_cs_precedes;		/* currency plus location		*/
   char p_sep_by_space;		/* currency plus space ind.		*/
   char n_cs_precedes;		/* currency minus location		*/
   char n_sep_by_space;		/* currency minus space ind.		*/
   char p_sign_posn;		/* currency plus position		*/
   char n_sign_posn;		/* currency minus position		*/

#ifdef _ALL_SOURCE
   char *left_parenthesis;	/* negative currency left paren         */
   char *right_parenthesis;	/* negative currency right paren        */
#else
   char *__left_parenthesis;	/* negative currency left paren         */
   char *__right_parenthesis;	/* negative currency right paren        */
#endif

} ;

#define LC_ALL		-1	/* name of locale's category name 	*/
#define LC_COLLATE	0	/* locale's collation data		*/
#define LC_CTYPE	1	/* locale's ctype handline		*/
#define LC_MONETARY	2	/* locale's monetary handling		*/
#define LC_NUMERIC	3	/* locale's decimal handling		*/
#define LC_TIME		4	/* locale's time handling		*/
#define LC_MESSAGES	5	/* locale's messages handling		*/



#ifdef _NO_PROTO
struct lconv *localeconv();
char   *setlocale();
#else
struct lconv *localeconv(void);
char   *setlocale(int, const char *);
#endif

#endif /* _ANSI_C_SOURCE */

#ifdef _ALL_SOURCE
typedef struct lconv lconv;
#endif /* _ALL_SOURCE */

#endif	/* __H_LOCALE */
