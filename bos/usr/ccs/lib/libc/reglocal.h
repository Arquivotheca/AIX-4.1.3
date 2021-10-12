/* @(#)27	1.5  src/bos/usr/ccs/lib/libc/reglocal.h, libcpat, bos411, 9428A410j 1/12/93 11:18:58 */
#ifndef _H_REGLOCAL
#define _H_REGLOCAL
/*
 * COMPONENT_NAME: (LIBCPAT) Internal Regular Expression
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/localedef.h>

/************************************************************************/
/* Regular Expressions (RE) provide a mechanism to locate substrings	*/
/* within a string from a character string template (pattern matching).	*/
/*									*/
/* The basic requirement of RE is to find the first occurance of a	*/
/* substring within a string which completely matches the RE pattern.	*/
/* RE shall also determine and report the longest possible substring.	*/
/*									*/
/* Each RE pattern is first compiled into an internal form by the	*/
/* regcomp() function and saved in a structure which is later used	*/
/* (repeatedly) by the regexec() function to determine whether the	*/
/* RE pattern is found within a given text string.			*/
/*									*/
/* RE patterns are defined by two sets of mutually exclusive syntax:	*/
/* Basic RE (BRE) and Extended RE (ERE).  By default a RE pattern is	*/
/* parsed via BRE rules unless REG_EXTENDED is set in the cflags	*/
/* argument of regcomp() function.					*/
/*									*/
/* The capabilities of each syntax are defined by POSIX 1003.2 "Shell	*/
/* and Utilities" Specification, Section 2.8 "Regular Expression	*/
/* Notation".  The following is a summary of BRE and ERE syntax:	*/
/*									*/
/*									*/
/*******************  BASIC REGULAR EXPRESSION (BRE)  *******************/
/*									*/
/*  Expression            Meaning					*/
/*  ----------	----------------------------------------------------	*/
/*									*/
/*  NUL		terminate BRE pattern and text string			*/
/*									*/
/*  c		match single non-special character			*/
/*									*/
/*  \s		match single special character				*/
/*		    \.   \[   \\   \*   \^   \$				*/
/*									*/
/*		note: non-special character preceded by \ will match	*/
/*		itself.							*/
/*									*/
/*		note: \ loses its special meaning within a bracket	*/
/*		expression.						*/
/*									*/
/*  yz		concatenation of expression y followed by expression z	*/
/*									*/
/*  .		match any single character				*/
/*									*/
/*		note: . will not match NEWLINE if REG_NEWLINE is set	*/
/*									*/
/*		note: . loses its special meaning within a bracket	*/
/*		expression.						*/
/*									*/
/*  ^		match beginning of line if first character of BRE	*/
/*									*/
/*		note: if REG_NEWLINE is set, ^ matches immediately	*/
/*		after NEWLINE and REG_NOTBOL is ignored			*/
/*									*/
/*		note: ^ loses its special meaning when not the first	*/
/*		character of BRE and not within a bracket expression.	*/
/*									*/
/*  $		match end of line if last character of BRE		*/
/*									*/
/*		note: if REG_NEWLINE is set, $ matches immediately	*/
/*		before NEWLINE and REG_NOTEOL is ignored		*/
/*									*/
/*		note: $ loses its special meaning when not the last	*/
/*		character of BRE and not within a bracket expression.	*/
/*									*/
/*  *		match zero or more successive occurances of		*/
/*		preceding expression					*/
/*									*/
/*		note: * loses its special meaning if it occurs as	*/
/*		first character of entire BRE, first character of	*/
/*		subexpression or within bracket expression.		*/
/*									*/
/*  \{m,n\}	interval expression match repeated successive		*/
/*		occurances of preceding expression, with m specifying	*/
/*		exact or minimum number and n specifying maximum 	*/
/*		number of successive occurances where:			*/
/*									*/
/*		\{m\}   match exactly m occurances			*/
/*		\{m,\}  match at least m occurances			*/
/*		\{m,n\} match between m and n occurances, inclusive	*/
/*									*/
/*		note: m and n are decimal integers from 0 through	*/
/*		RE_DUP_MAX.						*/
/*									*/
/*		note: the BRE is in error if m is greater than n or	*/
/*		\{ is not followed by a valid interval expression.	*/
/*									*/
/*		note: interval expression loses its special meaning	*/
/*		if occurs at beginning of entire BRE or after \(.	*/
/*									*/
/*									*/
/*   Bracket								*/
/*  Expression            Meaning					*/
/*  ----------	----------------------------------------------------	*/
/*									*/
/*  [...]	match any single character within braket expression	*/
/*									*/
/*		note: the BRE is in error if the bracket expression	*/
/*		set of characters in empty.				*/
/*									*/
/*		note: the BRE is in error if [ is not preceded by \	*/
/*		and not part of bracket expression.			*/
/*									*/
/*		note: special characters lose their special meaning	*/
/*		within a bracket expression				*/
/*									*/
/*		note: ] loses its special meaning if occurs first	*/
/*		in bracket expression. []...] or [^]...]		*/
/*									*/
/*  [^...]	match any single character except all characters	*/
/*		within bracket expression				*/
/*									*/
/*		note: NEWLINE is not matched if REG_NEWLINE is set	*/
/*									*/
/*		note: ^ loses its special meaning if not first		*/
/*		character of a bracket expression.			*/
/*									*/
/*  [[.cc.]]	collating symbol match all characters in order		*/
/*									*/
/*		note: the BRE is in error if the collating symbol is	*/
/*		not a valid collating element of the current locale.	*/
/*									*/
/*  [[=c=]]	equivalence class expression match any collation	*/
/*		element with same primary collation value		*/
/*									*/
/*		note: if the collating element does not belong to	*/
/*		an equivalence class (no collating value), the		*/
/*		equivalence class expression will be treated as a	*/
/*		collating symbol.					*/
/*									*/
/*  [[:y:]]	character class expression match any character of	*/
/*		class y.						*/
/*									*/
/*		note: the BRE will be in error if the class type is	*/
/*		not defined by the current locale.			*/
/*									*/
/*  [a-b]	match any single character whose unique collating	*/
/*		value falls between range starting (a) and ending (b)	*/
/*		points.							*/
/*									*/
/*		note: - loses special meaning if occurs first or last	*/
/*		in bracket expression. [-...] or [^-...] or [...-]	*/
/*		or [^...-]						*/
/*									*/
/*		note: - loses special meaning if range starting or	*/
/*		ending point in bracket expression. [--^] or [+--].	*/
/*									*/
/*		note: range starting and ending points may be		*/
/*		collating element (excludes characters with no		*/
/*		collating value), collating symbol ([. .]) or		*/
/*		equivalence class expression ([= =]).			*/
/*									*/
/*		note: if an equivalence class expression is used, the	*/
/*		starting/ ending point will be the lowest/highest	*/
/*		unique collating value of all characters represented	*/
/*		by the equivalence class expression.			*/
/*									*/
/*		note: the BRE is in error if the starting point unique	*/
/*		collating value is greater than	the unique collating	*/
/*		value of the ending point.				*/
/*									*/
/*		note: the BRE will be in error if ending point is also	*/
/*		starting point of subsequent range expression.		*/
/*									*/
/*    Sub-								*/
/*  Expression            Meaning					*/
/*  ----------	----------------------------------------------------	*/
/*									*/
/*  \(...\)	subexpression match whatever it would have matched	*/
/*		without subexpression delimiters \( and \).		*/
/*									*/
/*		note: \( \) loses its special meaning within bracket	*/
/*		expression.						*/
/*									*/
/*		note: the BRE is in error if subexpression is empty.	*/
/*									*/
/*  \n		backreference expression match the same string		*/
/*		defined by preceding nth subexpression			*/
/*									*/
/*		note: n is a single decimal digit from 1 through 9.	*/
/*									*/
/*		note: the BRE is in error if less than n subexpressions	*/
/*		precede the backreference expression.			*/
/*									*/
/* BRE Notes:								*/
/*									*/
/*  a)  Multiple duplication symbols applied to the same expression	*/
/*      are invalid (e.g. c\{3\}*).					*/
/*									*/
/*  b)  BRE Order of precedence is as follows, from high to low:	*/
/*									*/
/*	1)  bracket symbols	[= =]  [: :]  [. .]			*/
/*	2)  escaped character	\<special character>			*/
/*	3)  bracket expression	[  ]					*/
/*	4)  subexpression	\(  \)  \n				*/
/*	5)  duplication		*  \{ \}				*/
/*	6)  concatenation	ab					*/
/*	7)  anchors		 ^  $					*/
/*									*/
/*									*/
/*****************  EXTENDED REGULAR EXPRESSION (ERE)  ******************/
/*									*/
/*  Expression            Meaning					*/
/*  ----------	----------------------------------------------------	*/
/*									*/
/*  NUL		terminate ERE pattern and text string			*/
/*									*/
/*  c		match single non-special character			*/
/*									*/
/*  \s		match any single special character			*/
/*		    \.   \[   \\   \(   \)   \*   \+   \?		*/
/*		    \^   \$   \|					*/
/*									*/
/*		note: non-special character preceded by \ will match	*/
/*		itself							*/
/*									*/
/*		note: \ loses its special meaning within a bracket	*/
/*		expression.						*/
/*									*/
/*  yz		concatenation of expression y followed by expression z	*/
/*									*/
/*  .		match any single character				*/
/*									*/
/*		note: . will not match NEWLINE if REG_NEWLINE is set	*/
/*									*/
/*		note: . loses its special meaning within a bracket	*/
/*		expression.						*/
/*									*/
/*  ^		match beginning of line if first character of ERE	*/
/*									*/
/*		note: if REG_NEWLINE is set, ^ matches immediately	*/
/*		after NEWLINE and REG_NOTBOL is ignored			*/
/*									*/
/*		note: the ERE is in error if non-anchor ^ appears	*/
/*		outside a bracket expression without preceding \.	*/
/*									*/
/*		note: ^ is allowed in alternate expression or within	*/
/*		subexpression if occurs as beginning of line anchor.	*/
/*									*/
/*  $		match end of line; match NUL or NEWLINE			*/
/*									*/
/*		note: if REG_NEWLINE is set, $ matches immediately	*/
/*		before NEWLINE and REG_NOTEOL is ignored		*/
/*									*/
/*		note: the ERE is in error if non-anchor $ appears	*/
/*		outside a bracket expression without preceding \.	*/
/*									*/
/*		note: $ is allowed in alternate expression or within	*/
/*		subexpression if occurs as end of line anchor.		*/
/*									*/
/*  *		match zero or more successive occurances of		*/
/*		preceding expression					*/
/*									*/
/*		note: the ERE is in error if * occurs as the first	*/
/*		character of the entire ERE or immediately following	*/
/*		a VERTICAL-LINE, CIRCUMFLEX or OPEN-PARENTHESIS. 	*/
/*									*/
/*  +		match one or more successive occurances of		*/
/*		preceding expression					*/
/*									*/
/*		note: the ERE is in error if + occurs as the first	*/
/*		character of the entire ERE or immediately follows	*/
/*		VERTICAL-LINE, CIRCUMFLEX or OPEN-PARENTHESIS.		*/
/*									*/
/*  ?		match zero or one occurance of preceding expression	*/
/*									*/
/*		note: the ERE is in error if ? occurs as the first	*/
/*		character of the entire ERE or immediately follows	*/
/*		VERTICAL-LINE, CIRCUMFLEX or OPEN-PARENTHESIS. 		*/
/*									*/
/*  {m,n}	interval expression match repeated successive		*/
/*		occurances of preceding expression, with m specifying	*/
/*		exact or minimum number and n specifying maximum	*/
/*		number of successing occurances where:			*/
/*									*/
/*		{m}   match exactly m occurances			*/
/*		{m,}  match at least m occurances			*/
/*		{m,n} match between m and n occurances, inclusive	*/
/*									*/
/*		note: m and n are decimal integers from 0 through	*/
/*		RE_DUP_MAX.						*/
/*									*/
/*		note: interval expression is invalid if occurs at	*/
/*		beginning of entire ERE or immediately follows		*/
/*		VERTICAL-LINE, CIRCUMFLEX or OPEN-PARENTHESIS.		*/
/*									*/
/*		note: an invalid interval expression will be treated	*/
/*		as the characters themselves.				*/
/*									*/
/*  |		alternate match either expression separated by |	*/
/*									*/
/*		note: expression which matches first encountered	*/
/*		string will be chosen.					*/
/*									*/
/*		note: if more than one alternate expression matches,	*/
/*		expression which allows longest match of string		*/
/*		will be chosen.						*/
/*									*/
/*		note: | loses its special meaning within a bracket	*/
/*		expression.						*/
/*									*/
/*		note: | loses its special meaning if it is not		*/
/*		preceded and followed by valid expressions.		*/
/*									*/
/*   Bracket								*/
/*  Expression            Meaning					*/
/*  ----------	----------------------------------------------------	*/
/*									*/
/*  [...]	match any single character within braket expression	*/
/*									*/
/*		note: the ERE is in error if the bracket expression	*/
/*		set of characters in empty.				*/
/*									*/
/*		note: the BRE is in error if [ is not preceded by \	*/
/*		and not part of bracket expression.			*/
/*									*/
/*		note: special characters lose their special meaning	*/
/*		within a bracket expression.				*/
/*									*/
/*		note: ] loses its special meaning if occurs first	*/
/*		in bracket expression. []...] or [^]...]		*/
/*									*/
/*  [^...]	match any single character except all characters	*/
/*		within bracket expression				*/
/*									*/
/*		note: NEWLINE is not matched if REG_NEWLINE is set	*/
/*									*/
/*		note: ^ loses its special meaning if not first		*/
/*		character of a bracket expression.			*/
/*									*/
/*  [[.cc.]]	collating symbol match all characters in order		*/
/*									*/
/*		note: the BRE is in error if the collating symbol is	*/
/*		not a valid collating element of the current locale.	*/
/*									*/
/*  [[=c=]]	equivalence class expression match any collation	*/
/*		element with same primary collation value		*/
/*									*/
/*		note: if the collating element does not belong to	*/
/*		an equivalence class (no collating value), the		*/
/*		equivalence class expression will be treated as a	*/
/*		collating symbol.					*/
/*									*/
/*  [[:y:]]	character class expression match any character of	*/
/*		class y.						*/
/*									*/
/*		note: the ERE will be in error if the class type is	*/
/*		not defined by the current locale.			*/
/*									*/
/*  [a-b]	match any single character whose unique collating	*/
/*		value falls between range starting (a) and ending (b)	*/
/*		points.							*/
/*									*/
/*		note: - loses special meaning if occurs first or last	*/
/*		in bracket expression. [-...] or [^-...] or [...-]	*/
/*		or [^...-]						*/
/*									*/
/*		note: - loses special meaning if range starting or	*/
/*		ending point in bracket expression. [--^] or [+--].	*/
/*									*/
/*		note: range starting and ending points may be		*/
/*		collating element (excludes characters with no		*/
/*		collating value), collating symbol ([. .]) or		*/
/*		equivalence class expression ([= =]).			*/
/*									*/
/*		note: if an equivalence class expression is used, the	*/
/*		starting/ ending point will be the lowest/highest	*/
/*		unique collating value of all characters represented	*/
/*		by the equivalence class expression.			*/
/*									*/
/*		note: the BRE is in error if the starting point unique	*/
/*		collating value is greater than	the unique collating	*/
/*		value of the ending point.				*/
/*									*/
/*		note: the ERE will be in error if ending point is also	*/
/*		starting point of subsequent range expression.		*/
/*									*/
/*    Sub-								*/
/*  Expression            Meaning					*/
/*  ----------	----------------------------------------------------	*/
/*									*/
/*  (...)	subexpression match whatever it would have matched	*/
/*		without subexpression delimiters ( and )		*/
/*									*/
/*		note: ( ) loses its special meaning within a bracket	*/
/*		expression.						*/
/*									*/
/*		note: the ERE is in error if subexpression is empty.	*/
/*									*/
/* ERE Notes:								*/
/*									*/
/*  a)  Multiple duplication symbols applied to the same ERE will be	*/
/*      interpreted in the following order of precedence:		*/
/*									*/
/*	1)  *								*/
/*	2)  ?								*/
/*	3)  +								*/
/*	4)  {} 								*/
/*									*/
/*  b)  ERE Order of precedence is as follows, from high to low:	*/
/*									*/
/*	1)  bracket symbols	[= =]  [: :]  [. .]			*/
/*	2)  escaped character   \<special character>			*/
/*	3)  bracket expression	[  ]					*/
/*	4)  subexpression	(  )					*/
/*	5)  duplication		*  ?  +  {}				*/
/*	6)  concatenation	ab					*/
/*	7)  anchors		 ^  $					*/
/*	8)  alternation		|					*/
/*									*/
/************************************************************************/
/*									*/
/*	cflag options:							*/
/*									*/
/*	    REG_EXTENDED	set: use Extended RE syntax		*/
/*			      clear: use Basic RE syntax		*/
/*									*/
/*	    REG_ICASE		set: ignore case of character		*/
/*			      clear: compare characters as themselves	*/
/*									*/
/*	    REG_NEWLINE		set: use <newline> for BOL/EOL		*/
/*			      clear: ignore <newline> for BOL/EOL	*/
/*									*/
/*	    REG_NOSUB		set: regexec() will ignore pmatch[]	*/
/*			      clear: regexec() will fill pmatch[]	*/
/*				     based upon nmatch			*/
/*									*/
/*	eflag options:							*/
/*									*/
/*	    REG_NOTBOL		set: string not at beginning of line	*/
/*			    default: string at beginning of line	*/
/*									*/
/*	    REG_NOTEOL		set: string not at end of line		*/
/*			    default: string at end of line		*/
/*									*/
/************************************************************************/
/* Compiled RE pattern format with repetition interval			*/
/*									*/
/*	offset 0	Expression code (see below)			*/
/*	offset 1	Minimum repetition interval			*/
/*	offset 2	Maximum repetition interval			*/
/*	offset 3	Start of expression				*/
/*	   .								*/
/*	   .								*/
/*	offset n	End of expression				*/
/************************************************************************/
/* Compiled RE pattern format without repetition interval		*/
/*									*/
/*	offset 0	Expression code (see below)			*/
/*	offset 1	Start of expression				*/
/*	   .								*/
/*	   .								*/
/*	offset n	End of expression				*/
/************************************************************************/
/* Expression code format						*/
/*									*/
/*	bits  7 -> 5		repetition interval type (see below)	*/
/*	bits  4 -> 0		compilation code (see below)		*/
/************************************************************************/

/************************************************************************/
/*									*/
/* Global symbols for regcomp_std() and regexec_std()			*/
/*									*/
/************************************************************************/

#define	BITMAP_LEN	32	/* # bytes for a CC_BITMAP array	*/

/************************************************************************/
/*									*/
/* Expression repetition interval codes					*/
/*									*/
/************************************************************************/

#define CR_MASK		0xe0	/* repetition type mask			*/
#define CR_STAR		0xe0	/* * == zero or more matches		*/
#define CR_QUESTION	0xc0	/* ? == zero or one match (ERE only)	*/
#define CR_PLUS		0xa0	/* + == one or more mathces (ERE only)	*/
#define CR_INTERVAL	0x80	/* \{m,n\} (BRE) or {m,n} (ERE)		*/
#define CR_INTERVAL_ALL	0x60	/* \{m,\} (BRE) or {m,} (ERE)		*/

/************************************************************************/
/*									*/
/* Expression compilation codes						*/
/*									*/
/************************************************************************/

				/* repetition codes			*/
#define CC_CHAR		0x01	/* a single character			*/
#define CC_DOT		0x02	/* . any single character		*/
#define CC_BITMAP	0x03	/* bracket expression bitmap		*/
#define CC_BACKREF	0x04	/* \n subexpression backreference (BRE)	*/
#define CC_SUBEXP_E	0x05	/* \) [BRE] ) [ERE] end subexpression	*/
#define CC_I_CHAR	0x06	/* ignore case CC_CHAR			*/
#define CC_I_BACKREF	0x07	/* ignore case CC_BACKREF		*/
#define CC_WCHAR	0x08	/* a single multibyte character		*/
#define CC_I_WCHAR	0x09	/* ignore case CC_WCHAR			*/
#define	CC_WDOT		0x0a	/* a single ILS character		*/
#define CC_I_WBACKREF	0x0b	/* ignore case ILS CC_BACKREF		*/
#define	CC_WBITMAP	0x0c	/* ILS bracket expression bitmap	*/
#define CC_DOTREG	0x0d	/* . any single character except \n	*/

#define CC_STRING	0x0e	/* character string			*/
#define CC_I_STRING	0x0f	/* ignore case CC_STRING		*/
#define CC_BOL		0x10	/* ^ beginning-of-line anchor		*/
#define CC_EOL		0x11	/* $ end-of-line anchor			*/
#define CC_SUBEXP	0x12	/* \( [BRE] ( [ERE] start subexpression	*/
#define CC_ALTERNATE	0x13	/* | start alternate expression		*/
#define CC_ALTERNATE_E	0x14	/* | end alternate expression		*/
/* add new codes here at bottom of the list				*/
#define CC_EOP		0x1f	/* end of compiled RE pattern		*/

#endif /* _H_REGLOCAL */



