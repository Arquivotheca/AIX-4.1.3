/* @(#)19	1.2  src/bos/usr/ccs/bin/ld/bind/match.h, cmdld, bos411, 9428A410j 1/21/94 09:00:48 */
#ifndef Binder_MATCH
#define Binder_MATCH
/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

enum match_flag {
    MATCH_ADD_NEWNAME,			/* Add <pattern> as a new symbol if
					   it does not contain the '*'
					   wild card character and no
					   symbol exists with the
					   name. The ISSYMBOL flag will be
					   set for the symbol.  The function
					   parameter will be called for the
					   symbol regardless of the value of
					   match_scope. */
    MATCH_WARN_ADD_NEWNAME,		/* If <pattern> contains a '*', a
					   warning will be issued if the
					   <pattern> matches no symbols.  If
					   <pattern> does not contain a '*',
					   the behavior is the same as for
					   MATCH_ADD_NEWNAME. */
    MATCH_NO_NEWNAME,			/* Do not add a non-matching name
					   as a new symbol.  */
    MATCH_WARN_NO_NEWNAME};		/* Do not add a new name.  Furthermore,
					   print a warning if <pattern> does
					   not match any external symbols. */

#define MATCH_EXT	1		/* Match succeeds for names for which
					   there is at least one non-deleted
					   C_EXT symbol. */
#define MATCH_HID	2		/* Match succeeds for names for which
					   there is at least one non-deleted
					   C_HIDEXT symbol. */
#define MATCH_DUP	4		/* Flag must be used with MATCH_EXT or
					   MATCH_HID.  Match succeeds even if
					   all C_EXT or C_HIDEXT symbols are
					   deleted. */
#define MATCH_ER	8		/* Match succeeds for names for which
					   there is at least one ER symbol. */
#define MATCH_ANY	16		/* Match any symbol name if
					   STR_ISSYMBOL flag is set.  This flag
					   takes precedence over the other
					   flags. */

extern RETCODE match(char *,		/* pattern */
		     enum match_flag,
		     int,
		     int (*)(STR *));	/*Function to invoke on matched names*/

#endif /* Binder_MATCH */
