/* @(#)36       1.1  src/bldenv/make/str.h, bldprocess, bos412, GOLDA411a 1/19/94 15:56:52
 *
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: string_arch
 *		string_hasvar
 *		string_head
 *		string_memb
 *		string_pref
 *		string_suff
 *		string_tail
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 */
/*
 * HISTORY
 * $Log: str.h,v $
 * Revision 1.1.2.3  1992/12/03  19:07:22  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:36:36  damon]
 *
 * Revision 1.1.2.2  1992/09/24  19:27:19  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:50:29  gm]
 * 
 * $EndLog$
 */

#ifndef _STR_H_
#define _STR_H_

typedef struct string *string_t;

struct string {
	unsigned hashval;
	int len;
	const char *data;
	int _flags;
#define STR_HASVAR		1
#define STR_HEADTAIL		2
#define STR_PREFSUFF		4
	int _refCount;
	string_t _head;
	string_t _tail;
	string_t _pref;
	string_t _suff;
	string_t _arch;
	string_t _memb;
};

#define string_hasvar(s)	((s)->_flags & STR_HASVAR)
#define string_head(s)	(((s)->_flags & STR_HEADTAIL) ? (s)->_head : \
			 (_string_headtail(s), (s)->_head))
#define string_tail(s)	(((s)->_flags & STR_HEADTAIL) ? (s)->_tail : \
			 (_string_headtail(s), (s)->_tail))
#define string_pref(s)	(((s)->_flags & STR_PREFSUFF) ? (s)->_pref : \
			 (_string_prefsuff(s), (s)->_pref))
#define string_suff(s)	(((s)->_flags & STR_PREFSUFF) ? (s)->_suff : \
			 (_string_prefsuff(s), (s)->_suff))
#define string_arch(s)	((s)->_arch)
#define string_memb(s)	((s)->_memb)

void string_init(void);
string_t string_create(const char *);
string_t string_ref(string_t);
void string_deref(string_t);
string_t string_concat(string_t, string_t, int);
string_t string_flatten(string_t);
void _string_headtail(string_t);
void _string_prefsuff(string_t);
void string_archmemb(string_t, string_t, string_t);

#endif /* _STR_H_ */
