#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)09	1.6  src/bos/usr/ccs/bin/ld/bind/util.c, cmdld, bos411, 9428A410j 5/12/94 10:47:07")
#endif

/*
 * COMPONENT_NAME: CMDLD
 *
 * FUNCTIONS:	deletemin
 *		find_item
 *		lower
 *		makeheap
 *		save_dotted_string
 *		save_string
 *		unescape_pathname
 *		upper
 *
 * STATIC FUNCTIONS:
 *		minchild
 *		siftdown
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

#include <string.h>

#include "bind.h"
#include "global.h"
#include "util.h"
#include "stats.h"
#if lint || XLC_TYPCHK_BUG
#include "strs.h"
#endif
#ifdef HASH_STATS
#include "error.h"
#endif

/* Static variables and #define for save_string and save_dotted_string */
#define STRINGS_BUMP	8100
static int	bytes_left = 0;
static char	*available_bytes;

/* Definitions for heap routines */
#define heap_size(h) (h[0].heap_index)
#define incr_heap_size(h) (++h[0].heap_index)
#define decr_heap_size(h) (--h[0].heap_index)

/* TARJAN Algorithms for maintaining heaps.  A heap is a collection of
   item and associated keys.  In the binder, items are csects and the
   keys the csect addresses.  The routine 'deletemin' finds and returns
   the item (i.e., csect) with the smallest address.  (In addition, the
   returned value is saved in the same array used to store the heap,
   allowing later access to a sorted list of csects.)  The heap could be
   replaced by sorting the csects completely and using the sorted list,
   but a heap is suppposed to have good performance characteristics.  For
   details, see ""Data Structures and Network Algorithms", Robert Tarjan,
   ISBN 0-89871-187-8 */

/************************************************************************
* Name: minchild
*									*
* Purpose: Internal routine used by deletemin
*									*
************************************************************************/
static int
minchild(int x,
	 CSECT_HEAP h)
{
    int i = heap_size(h);

    if (x == i || 2 * x > i)
	return 0;
    else if (2 * x + 1 > i)
	return 2 * x;
    else if (h[2*x].csect->c_addr < h[2*x+1].csect->c_addr)
	return 2*x;
    else
	return 2*x+1;
}
/************************************************************************
 * Name: siftdown
 *									*
 * Purpose: Internal routine used by deletemin
 *									*
 ************************************************************************/
static void
siftdown(CSECT *i,
	 int x,
	 CSECT_HEAP h)
{
    int c;

    c = minchild(x, h);
    while (c != 0 && h[c].csect->c_addr < i->c_addr) {
	h[x] = h[c];
	x = c;
	c = minchild(c,h);
    }
    h[x].csect = i;
}
/************************************************************************
 * Name: makeheap
 *									*
 * Purpose: Create a heap from an array initialized with key,value pairs.
 *	This routine performs the initialization needed by deletemin().
 ************************************************************************/
void
makeheap(CSECT_HEAP h)
{
    int x;

    for (x = heap_size(h); x > 0; x--)
	siftdown(h[x].csect, x, h);
}
/************************************************************************
 * Name: deletemin
 *									*
 * Purpose: Find the minimum element in a heap.  Save the value at the
 *	end of the array (outside the heap) in case the ordered list will
 *	be needed again.
 *									*
 ************************************************************************/
CSECT *
deletemin(CSECT_HEAP h)
{
    CSECT	*i, *j;
    int		s;

    if (heap_size(h) == 0)
	return NULL;

    s = heap_size(h);
    i = h[1].csect;
    j = h[s].csect;
    decr_heap_size(h);
    if (i != j)
	siftdown(j, 1, h);

    /* Save the returned addresses (in reverse order) so that we can
       regenerate the sequence, if necessary */
    h[s].csect = i;

    return i;
}
/* end Tarjan Algorithms */

/************************************************************************
 * Name: find_item							*
 *									*
 * Purpose: Look up an item with key <key> in array <s>, whose length is
 *	found in s[0].item_key, and whose elements are in s[1..length].
 *	The keys must be in order.
 * If the item is found, return its address.
 * if the item is not found, return NULL.
 *
#ifdef QUICK
 * The lines with QUICK around them narrow the beginning range of the
 * binary search based on the previous search result.  Testing has not yet
 * determined whether any effort is saved by using this code.
#endif
 *
 * Function:  Use a binary search to find the element.
 *
 ************************************************************************/
#ifdef QUICK
int	last_mid = -1;
#endif

ITEM *
find_item(ITEM *s,			/* Array of items */
	  int key)			/* Key of desired item */
{
    int first, last, mid;

#ifdef HASH_STATS
    static int probes = 0;
    static int calls = 0;
#endif
#ifdef QUICK
    static ITEM	*last_item;
    static int last_i;
#endif

#ifdef HASH_STATS
    calls++;
#endif

#ifndef QUICK
    first = 1;
    last = s[0].item_key;
#else
    if (last_mid == -1) {
	first = 1;
	last = s[0].item_key;
    }
    else if (key == last_i)
	return last_item;
    else if (key < last_i) {
	first = 1;
	last = last_mid - 1;
    }
    else {
	first = last_mid + 1;
	last = s[0].item_key;
    }
#endif

    while (first <= last) {
	mid = (first + last) >> 1;
#ifdef HASH_STATS
	probes++;
#endif
	if (key == s[mid].item_key) {
#ifdef HASH_STATS
	    say(SAY_NO_NLS, "Average probes %f", (float)probes/(float)(calls));
#endif
#ifdef QUICK
	    last_mid = mid;
	    last_item = &s[mid];
	    last_i = key;
#endif
	    return &s[mid];
	}
	else if (key < s[mid].item_key)
	    last = mid - 1;
	else
	    first = mid + 1;
    }

#ifdef QUICK
    last_mid = -1;
    last_item = NULL;
#endif

    return NULL;
}
/************************************************************************
 * Name: save_string
 *									*
 * Purpose: Save a string in a string pool.
 *
 *	Note:  The string pool is allocated in chunks.  Bytes at the
 *	end of each chunk will be unused.
 *									*
 * Returns:	The address of the saved string.
 ************************************************************************/
char *
save_string(char *strng,		/* String to save--may contain nulls
					   or not be null-terminated. */
	    int str_len)		/* Length of string to save (must
					   include terminating '\0', if it
					   exists). */
{
    static char	*id = "save_string";
    char	*r;

    if (bytes_left < str_len) {
	if (str_len > STRINGS_BUMP) {
	    /* A really big string gets its own chunk */
	    r = get_memory(1, str_len, CHARS_ID, id);
	    goto copy_return;
	}
	available_bytes = get_memory(1, STRINGS_BUMP, CHARS_ID, id);
	bytes_left = STRINGS_BUMP;
    }

    r = available_bytes;		/* Save address of start of string */
    available_bytes += str_len;
    bytes_left -= str_len;

  copy_return:
    memcpy(r, strng, str_len);
    STAT_use(CHARS_ID, str_len);
    return r;
}
/************************************************************************
 * Name: save_dotted_string
 *									*
 * Purpose: Save a string, prepending a dot to the beginning of the
 *	string.  This function is the same as save_string(), except for
 *	the addition of the dot.
 *									*
 *	Note:  The string pool is allocated in chunks.  Bytes at the
 *	end of each chunk will be unused.
 *									*
 * Returns:	The address of the dot (which preceded the name) is returned.
 ************************************************************************/
char *
save_dotted_string(char *strng,		/* String to save--not necessarily
					   null-terminated. */
		   int str_len)		/* Length of string to save (must
					   include terminating '\0', if it
					   exists). Does not include 1 for
					   '.' to be added. */

{
    static char	*id = "save_dotted_string";
    char	*r;

    if (bytes_left < str_len + 1) {
	if (str_len + 1 > STRINGS_BUMP) {
	    /* A really big string gets its own chunk */
	    r = get_memory(1, str_len + 1, CHARS_ID, id);
	    goto copy_return;
	}
	available_bytes = get_memory(1, STRINGS_BUMP, CHARS_ID, id);
	bytes_left = STRINGS_BUMP;
    }

    r = available_bytes;		/* Save address of start of string */
    available_bytes += str_len + 1;
    bytes_left -= str_len + 1;

  copy_return:
    *r = '.';		/* String begins with '.' */
    memcpy(&r[1], strng, str_len);
    STAT_use(CHARS_ID, str_len + 1);
    return r;
}
/************************************************************************
 * Name: lower
 *									*
 * Purpose: Convert a string (in place) to lower case
 *									*
 * NOTE: Conversion is done in the current locale.  Characters used in
 *	command and option names must be alphabetic characters in all
 *	locales.
 *
 ************************************************************************/
void
lower(char *s)
{
    while (*s) {
	if (isupper((int) *s))
	    *s = (char)tolower((int) *s);
	s++;
    }
}
/************************************************************************
 * Name: upper
 *									*
 * Purpose: Convert a string (in place) to upper case
 *									*
 * NOTE: Conversion is done in the current locale.  Characters used in
 *	command and option names must be alphabetic characters in all
 *	locales.
 ************************************************************************/
void
upper(char *s)
{
    while (*s) {
	if (islower((int) *s))
	    *s = (char)toupper((int) *s);
	s++;
    }
}
/************************************************************************
 * Name: unescape_pathname
 *									*
 * Purpose: Convert a pathname containing escape sequences.
 *
 * Returns: The converted pathname or NULL if the pathname is too long
 ************************************************************************/
char *
unescape_pathname(char *dest,		/* Where to put converted name */
		  int maxlen,		/* Maximum strlen(dest) when complete.
					   Thus, dest must have room for
					   max_len+1 bytes. */
		  char *src)		/* Name containing escapes */
{
    char *save_dest = dest;

    src++;				/* Skip over leading \.  */
    while (maxlen-- > 0 && *src) {
	*dest++ = *src;
	if (*src++ == '\\') {
	    switch(*src++) {
	      case 's':  dest[-1] = ' ' ; break;
	      case 't':  dest[-1] = '\t'; break;
	      case 'n':  dest[-1] = '\n'; break;
	      case '\\': dest[-1] = '\\'; break;
	      default:
		src--;
	    }
	}
    }
    if (*src == '\0') {
	*dest = '\0';
	return save_dest;
    }
    /* Name is too long */
    return NULL;
}
