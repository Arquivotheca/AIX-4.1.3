/* @(#)41	1.3  src/bos/kernel/sys/listmgr.h, libsys, bos411, 9428A410j 6/16/90 00:30:45 */
#ifndef _H_REGMGR
#define _H_REGMGR

/*
 * COMPONENT_NAME:  (LIBSYS) - Region List Manager routines
 *
 * FUNCTIONS:  listmgr.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/*
   Region management functions.  These services create a control
   structure for managing a region of memory.  Additionally, they
   provides lookup/retrieval and return services which enable a
   user to request any contiguous size of that memory region.
*/

typedef struct REG_LIST
{
        unsigned char           *_p_region;     /* Region Base Address */
        unsigned long           _rsize;         /* Region size, bytes */
        unsigned                _l2rsize;       /* Log2, Region Size */
        unsigned                _n_region;      /* Number of regions */
        unsigned char           _free [1];      /* Usage map, n_region long */
} t_reg_list;

/*
   Region management function prototypes.  Aid parameter checking of callers.
*/

extern t_reg_list       *reg_init( unsigned char *, unsigned, unsigned );
#define reg_release( p_reg )   xmfree( p_reg, pinned_heap )

extern unsigned char    *reg_alloc( t_reg_list *, unsigned );
extern int               reg_free( t_reg_list *, unsigned, unsigned char * );
extern void              reg_clear( t_reg_list * );
extern int               reg_avail( t_reg_list * );

#define REG_FREE        0xFF
#define REG_USED        0x00

#ifndef NULL                    /* Temporary definition */
#define NULL    ((void *)0)       /* Temporary definition */
#endif                          /* Temporary definition */

/* extern unsigned char    *xmalloc(),pinned_heap[]; */

#endif     /* _H_REGMGR */
