/* static char sccsid[] = "@(#)42  1.10  src/bos/usr/include/rpc/types.h, libcrpc, bos411, 9428A410j 3/11/94 11:10:42"; */
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: mem_alloc
 *		mem_free
 *		
 *
 *   ORIGINS: 24
 *
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *      @(#)types.h 1.20 88/02/08 SMI      
 */

/*	@(#)types.h	1.6 90/07/19 4.1NFSSRC SMI	*/


/*
 * Rpc additions to <sys/types.h>
 */

#ifndef	__RPC_TYPES_H
#define	__RPC_TYPES_H

#define	bool_t	int
#define	enum_t	int
#define __dontcare__	-1

#ifndef FALSE
#	define	FALSE	(0)
#endif

#ifndef TRUE
#	define	TRUE	(1)
#endif

#ifndef NULL
#	define NULL 0
#endif

#ifndef	_KERNEL
#define	mem_alloc(bsize)	malloc(bsize)
#define	mem_free(ptr, bsize)	free(ptr)
#else
#include <sys/malloc.h>
#define kmem_alloc(bsize)	xmalloc(bsize, 2, kernel_heap)
#define	kmem_free(ptr, bsize)	xmfree(ptr, kernel_heap)
#define	mem_alloc(bsize)	xmalloc(bsize, 2, kernel_heap)
#define	mem_free(ptr, bsize)	xmfree(ptr, kernel_heap)
#endif

#include <sys/types.h>

#include <sys/time.h>

#endif	/* !__RPC_TYPES_H */
