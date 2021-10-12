#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)84	1.8  src/bos/usr/ccs/bin/ld/bind/alloc.c, cmdld, bos411, 9428A410j 1/28/94 13:06:00")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS: emalloc
 *		erealloc
 *		efree
 *		get_mem
 *
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "global.h"
#include "error.h"
#include "stats.h"

#if lint || XLC_TYPCHK_BUG
#include "bind.h"
#include "strs.h"
#endif

/************************************************************************
 * NAME: emalloc
 *									*
 * PURPOSE: Allocate memory, with error checking.
 *
 * INPUT:	size:	Number of bytes to allocate
 *		id:	Name of routine allocating memory -or-
 *			NULL if caller will check returned value.
 *
 * RETURNS: 	Pointer to memory, if successful;
 *		NULL if memory cannot be allocated and id is NULL.
 *
 * #ifdef STATS, keeps track of memory allocated.
 *
 * NOTE:  Does not return if memory cannot be allocated and id is not NULL.
 *									*
 ************************************************************************/
void *
emalloc(size_t size,
	char *id)
{
    void	*p;

    if (size == 0) {
	size = 1;

#ifdef ALLOCZEROMSG
	bind_err(SAY_NORMAL, RC_OK,
		 NLSMSG(ALLOC_ZERO,
"%1$s: 0711-113 WARNING: Allocation of zero bytes requested in routine %2$s."),
		 Main_command_name, id ? id : "<?>");
#endif
    }

    if ((p = malloc(size)) == NULL) {
	if (id == NULL)		/* Caller will check for and handle errors */
	    return NULL;
	bind_err(SAY_NORMAL, RC_SEVERE, NLSMSG(ALLOC_FAILED,
	       "%1$s: 0711-101 FATAL ERROR: Allocation of %2$u bytes failed\n"
	       "\tin routine %3$s. There is not enough memory available.\n"
	       "\tPlease check your ulimit or paging space\n"
	       "\tor use local problem reporting procedures."),
		 Main_command_name, size, id);
	cleanup(RC_SEVERE);
    }

    STAT_allocations(BYTES_ID, size);
    STAT_use(BYTES_ID, size);

#ifdef DEBUG
    if (bind_debug & ALLOC_DEBUG)
	say(SAY_NO_NLS, "Allocate: %8u bytes at 0x%8X for %s",
	    size, (int) p, id ? id : "<?>");
#endif

    return p;
}

/************************************************************************
 * NAME: erealloc
 *									*
 * PURPOSE: Reallocate memory, with error checking.
 *
 * INPUT:	ptr:	Area to realloc
 *		size:	Number of bytes to allocate
 *		id:	Name of routine allocating memory -or-
 *			NULL if caller will check returned value.
 *
 * RETURNS: 	Pointer to memory, if successful;
 *		NULL if memory cannot be allocated and id is NULL.
 *
 * #ifdef STATS, keeps track of memory allocated.
 *
 * NOTE:  Does not return if memory cannot be allocated and id is not NULL.
 *									*
 ************************************************************************/
void *
erealloc(void *ptr,
	 size_t size,
	 char *id)
{
    void	*p;
#if CENTERLINE == 0 && (STATS == 1 || DEBUG == 1)
    int		bytes = ((long *)ptr)[-1];
#endif

#if CENTERLINE == 0 && (STATS == 1 || DEBUG == 1)
    /* Portability problem--assumes length precedes area */
    STAT_free(BYTES_ID, bytes);
#endif

    if (size == 0) {
	size = 1;

#ifdef ALLOCZEROMSG
	bind_err(SAY_NORMAL, RC_OK,
		 NLSMSG(ALLOC_ZERO,
"%1$s: 0711-113 WARNING: Allocation of zero bytes requested in routine %2$s."),
		 Main_command_name, id ? id : "<?>");
#endif
    }

    if ((p = realloc(ptr, size)) == NULL) {
	if (id == NULL)		/* Caller will check for and handle errors */
	    return NULL;
	bind_err(SAY_NORMAL, RC_SEVERE, NLSMSG(ALLOC_FAILED2,
       "%1$s: 0711-104 FATAL ERROR: Reallocation of %2$u bytes failed\n"
       "\tin routine %3$s. There is not enough memory available.\n"
       "\tPlease check your ulimit or paging space\n"
       "\tor use local problem reporting procedures."),
		 Main_command_name, size, id);
	cleanup(RC_SEVERE);
    }

    STAT_allocations(BYTES_ID, size);
    STAT_use(BYTES_ID, size);

#ifdef DEBUG
    if (bind_debug & ALLOC_DEBUG)
#if CENTERLINE == 0
	say(SAY_NO_NLS,
	    "ReAllocate: %8u bytes at 0x%8X (old size %u) for %s",
	    size, (int) p, bytes, id ? id : "<?>");
#else
	say(SAY_NO_NLS, "ReAllocate: %8u bytes at 0x%8X for %s",
	    size, (int) p, id ? id : "<?>");
#endif
#endif

    return p;
}
#ifdef DEBUG
/************************************************************************
 * Name: efree
 *
 * Purpose: Free memory, printing diagnostic information, if requested.
 *
 * Input:	ptr:	Memory to be freed.
 *		Note:	This function assumes that the length of an allocated
 *			area is stored in the word preceding the area.  This
 *			function is not portable.
 *
 * RETURNS: 	Nothing
 *
 * Note: If DEBUG is not defined, efree is defined as a macro in global.h.
 *
 * #ifdef STATS and #ifndef CENTERLINE, keeps track of memory allocated
 *
 ************************************************************************/
void
efree(void *ptr)
{
#ifndef CENTERLINE
    /* Portability problem--assumes length precedes area */
    size_t bytes = ((long *)ptr)[-1];
    STAT_free(BYTES_ID, bytes);
#endif

    free(ptr);

    if (bind_debug & ALLOC_DEBUG)
#ifdef CENTERLINE
	say(SAY_NO_NLS, "Free: 0x%8X", (int) ptr);
#else
	say(SAY_NO_NLS, "Free: %8u bytes at 0x%8X", bytes, (int) ptr);
#endif
}
#endif

/************************************************************************
 * Name: get_mem
 *
 * Purpose: Allocate memory, with error checking.
 *
 * Input:	size:	Size of element being allocated
 *		count:	Number of elements to allocate
 *		mem_id:	Type of memory unit being allocated.
 *		id:	Name of routine allocating memory -or-
 *			NULL if caller will check returned value.
 *
 * RETURNS: 	Pointer to memory, if successful;
 *		NULL if memory cannot be allocated and id is NULL.
 *
 * #ifdef STATS, keeps track of memory allocated
 *
 * NOTE: Does not return if memory cannot be allocated and id is not NULL.
 *
 ************************************************************************/
void *
get_mem(size_t size,
	unsigned int count,
#ifdef STATS
	int mem_id,
#endif
	char *id)
{
    void *p;

    size_t bytes = size * count;

    if (bytes == 0) {
	bytes = 1;

#ifdef ALLOCZEROMSG
	bind_err(SAY_NORMAL, RC_OK,
		 NLSMSG(ALLOC_ZERO,
"%1$s: 0711-113 WARNING: Allocation of zero bytes requested in routine %2$s."),
		 Main_command_name, id ? id : "<?>");
#endif
    }

    if ((p = malloc(bytes)) == NULL) {
	if (id == NULL)		/* Caller will check for and handle errors */
	    return NULL;
	bind_err(SAY_NORMAL, RC_SEVERE, NLSMSG(ALLOC_FAILED,
	       "%1$s: 0711-101 FATAL ERROR: Allocation of %2$u bytes failed\n"
	       "\tin routine %3$s. There is not enough memory available.\n"
	       "\tPlease check your ulimit or paging space\n"
	       "\tor use local problem reporting procedures."),
		 Main_command_name, bytes, id);
	cleanup(RC_SEVERE);
    }

    STAT_allocations(mem_id, count);

#ifdef DEBUG
    if (bind_debug & ALLOC_DEBUG)
#ifdef STATS
	say(SAY_NO_NLS,
	    "Allocate: %8u bytes at 0x%8X (%d %s) for %s",
	    bytes, (int) p, count, mem_name[mem_id], id ? id : "<?>");
#else
	say(SAY_NO_NLS,
	    "Allocate: %8u bytes at 0x%8X (%d items) for %s",
	    bytes, (int) p, count, id ? id : "<?>");
#endif
#endif

    return p;
}
