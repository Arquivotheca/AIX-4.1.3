static char sccsid[] = "@(#)54	1.1  src/bos/usr/ccs/lib/libc/__mcount_construct.c, libcgen, bos411, 9428A410j 2/22/94 06:22:45";
/*
 *   COMPONENT_NAME: libcgen
 *
 *   FUNCTIONS: __mcount_construct_count_element
 *		__mcount_construct_graph_element
 *		allocate_memory
 *		chs
 *
 *   ORIGINS: 10,26,27
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
/*
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
	gmon.c      5.3 (Berkeley) 3/9/86";
*/

/*
 * Generalized to allow multiple non-contiguous program ranges to be defined
 * for profiling.  Motivated by shared libraries.  Each program range defined
 * for profiling is given a separate data structure for the from and to
 * function arc accounting like that defined with the original gprof profiling.
 * These modules do the -pg unique data allocation, initialization
 * manipulation and output conversion.  All other control operations merged
 * with similar -p operations into file mon.c
 */

/*
 * Routine to support both  -p (prof) and -pg (gprof) profiling
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/limits.h>
#include <nl_types.h>
#include <signal.h>

#include <mon.h>        /* profiling definitions */
#include <stdlib.h>

#include <__mcount.h>
extern struct monglobal _mondata;       /* global control data */

/***********
#define _MCOUNT_DEBUG _MCOUNT_DEBUG
************/

#include "libc_msg.h"
#define MSG01 catgets(catd, MS_LIBC, M_MONCNT, \
                        "%s: Number of %s (%u) exceeds profiling limit (%u)\n")
/*
 *
 * NAME: __mcount_construct_count_element
 *
 * FUNCTION: Constructs a counter for call count profiling
 *
 * EXECUTION ENVIRONMENT:
 *
 *      System Subroutine.  User process.  Called from __mcount, which
 *      is itself called by compiler generated
 *      code at the beginning of each function compiled with the -p option.
 *
 * (NOTES:)
 *      Based on BSD4.3 generalized for multiple profiling ranges.
 *      This is called the first time each profiled function is called. Its
 *      job is to allocate a counter (which will subsequently be dumped
 *      during program termination), and set up a pointer to that counter
 *      so that the second and subsequent calls to the profiled function
 *      can use the counter directly.
 *
 *      It is specially coded using cs(), so that it behaves correctly
 *      even when there are asynchronous signals, and when there is a
 *      preemptive threading environment active.
 *
 * (RECOVERY OPERATION:) none
 *
 * (DATA STRUCTURES:) See mon.h and global definitions in mon.c.
 *
 * RETURNS: none
 */

/* Compare half and swap, like compare and swap but for halfwords */
static int chs ( unsigned short * Destination
               , int Compare
               , int Value ) {

  int * BuddyPointer = (int *) (((int) Destination) ^ 2) ;
  int BuddyData = *(unsigned short *) BuddyPointer ;

  /* RIOS is big-endian */
  if ((unsigned int)Destination & 2) {
    /* CS the previous halfword || the addressed one */
    int BuddyShift = BuddyData << 16 ;
    return cs(BuddyPointer, BuddyShift+Compare, BuddyShift+Value) ;
  } else {
    /* CS the addressed halfword || the next one */
    return cs((int *)Destination,
              (Compare<<16)+BuddyData,
              (Value<<16)+BuddyData) ;
  } /* endif */
}

/* Acquire memory atomically in a take-only scheme */
static ulong allocate_memory ( ulong * base_pointer, ulong limit, ulong size ) {
  ulong old_base = * base_pointer ;
  ulong new_base =  old_base + size ;
  if (old_base >= limit) return NULL ;
  while ( 0 != cs(base_pointer,old_base,new_base) ) {
    old_base = * base_pointer ;
    new_base = old_base + size ;
    if (old_base >= limit) return NULL ;
  } /* endwhile */
  return old_base ;
  }

void __mcount_construct_count_element(ulong **cntp, caddr_t from_pc) {
  struct poutcnt * nextcounter =
   (struct poutcnt *) allocate_memory((ulong *) &(_mondata . pnextcounter),
                                       (ulong) _mondata . cl.pcountlimit,
                                       sizeof ( struct poutcnt ) ) ;
  if (NULL == nextcounter) return ;
  nextcounter -> fnpc = from_pc ;
  nextcounter -> mcnt = 1 ;
  * cntp = & (nextcounter -> mcnt) ;
}

/*
 *
 * NAME: __mcount_construct_graph_element
 *
 * FUNCTION: Constructs a counter for call graph profiling
 *
 * EXECUTION ENVIRONMENT:
 *
 *      System Subroutine.  User process.  Called from __mcount, which
 *      is itself called by compiler generated
 *      code at the beginning of each function compiled with the -pg option.
 *
 * (NOTES:)
 *      Based on BSD4.3 generalized for multiple profiling ranges.
 *      This is called the first time each profiled function is called. Its
 *      job is to allocate a counter (which will subsequently be dumped
 *      during program termination), and set up that counter in a hash table
 *      so that it can be located efficiently during the second and subsequent
 *      calls to the profiled function.
 *
 *      It is specially coded using cs(), so that it behaves correctly
 *      even when there are asynchronous signals, and when there is a
 *      preemptive threading environment active.
 *
 *      The counter is addressable in 2 ways
 *      1) 'old' way, by scanning the shared library profiling chain
 *      2) 'new' way, via the hash table
 *      If the profiling initialisation and termination routines were
 *      updated, then this routine could be simplified because it would
 *      no longer need to support the 'old' way of doing this.
 *
 * (RECOVERY OPERATION:) none
 *
 * (DATA STRUCTURES:) See mon.h and global definitions in mon.c.
 *
 * RETURNS: none
 */
#define GRAPH_OVERFLOW_LIMIT 65536
static graph_chain overflow_chain[GRAPH_OVERFLOW_LIMIT] ;
static graph_chain * current_overflow = &overflow_chain[0] ;

graph_chain __mcount_graph_hash[GRAPH_HASH_COUNT] ;

void __mcount_construct_graph_element (
        caddr_t from_address,
        caddr_t to_address,
        graph_chain * this_chain
        ) {

     /* Find profiling range with 'frompc'                                    */
     struct gfctl * p = _mondata . cb.pgcountbase ;
     uint fromindex ;
     while ( (from_address < p->fromlowpc)
          || ( ( fromindex=(uint)(FROM_INDEX(
                                  ((uint)from_address),((uint)(p->fromlowpc))
                                            )
                                 )
               ) >= p->fromlimit
             )
           ) {
       if ((++p) >= _mondata . cl.pgcountlimit) return ; /* Not in any region */
     } /* endwhile */

     /* Get a new counter */
     /* First, old-style processing to link it in the 'gprof' table */
     { HASH_LINK *pold_link = &(p -> tos[0] . link) ;
       int link_limit = p -> tolimit ;
       int old_link = *pold_link ;
       int new_link = old_link + 1 ;
       if (new_link >= link_limit) return ;
       while (0 != chs(pold_link,old_link,new_link)) {
         old_link = *pold_link ;
         new_link = old_link + 1 ;
         if (new_link >= link_limit) return ;
       } /* endwhile */
       { struct tostruct * top = & ( p -> tos [ new_link ] ) ;
         ulong * counter_address = (ulong *) & ( top -> count ) ;
         top -> selfpc = to_address ;
         top -> count = 1 ;
         { HASH_LINK * fromchainhead = & ( p -> froms [ fromindex ] ) ;
           int old_head = * fromchainhead ;
           top -> link = old_head ;
           while (0 != chs(fromchainhead,old_head,new_link)) {
             old_head = *fromchainhead ;
             top -> link = old_head ;
           } /* endwhile */
         }
         /* Next, new-style processing to link it in the hash table               */
         {
           if (NULL == this_chain -> from_pc
               && 0 == cs((int *) &(this_chain -> from_pc), (int) NULL, (int) from_address)
           ) {
             /* First entry in this hash bucket, easy ! */
             this_chain -> to_pc = to_address ;
             this_chain -> counter_address = counter_address ;
           } else {
             /* Hash collision, take store from the overflow list */
             graph_chain * graph_element = ( graph_chain * )
                      allocate_memory((ulong *) & current_overflow,
                                      (ulong) & overflow_chain[GRAPH_OVERFLOW_LIMIT],
                                      sizeof(graph_chain) ) ;
             graph_chain * apparent_next = this_chain -> next ;
             if (NULL == graph_element) return ;
             graph_element -> from_pc = from_address ;
             graph_element -> to_pc = to_address ;
             graph_element -> counter_address = counter_address ;
             graph_element -> next = apparent_next ;
             while (0 != cs((int *) &(this_chain -> next) ,
                            (int) apparent_next,
                            (int)(graph_element) )) {
               apparent_next = this_chain -> next ;
               graph_element -> next = apparent_next ;
             } /* endwhile */
           } /* endif */

         }
       }
     }
   }
