static char sccsid[] = "@(#)78	1.8  src/bos/usr/ccs/lib/libc/__mcount.c, libcgen, bos411, 9428A410j 2/22/94 06:21:59";
/*
 *   COMPONENT_NAME: libcgen
 *
 *   FUNCTIONS: __mcount
 *		increment
 *		mcount
 *		qincrement
 *		qincrement1
 *
 *   ORIGINS: 10,26,27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1994
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

/***********
#define _MCOUNT_DEBUG _MCOUNT_DEBUG
************/

#include "libc_msg.h" 
#define MSG01 catgets(catd, MS_LIBC, M_MONCNT, \
                        "%s: Number of %s (%u) exceeds profiling limit (%u)\n")
/*
 *
 * NAME: __mcount
 *
 * FUNCTION: Tabulates the arc count data for function calls during
 *           -p (prof) and -pg (gprof) profiling.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      System Subroutine.  User process.  Called by compiler generated
 *      code at the beginning of each function compiled with the -p option.
 *
 * (NOTES:)
 *      Based on BSD4.3 generalized for multiple profiling ranges.
 *      Both -p and -pg code combined in single routine.
 *      For -pg operation:
 *      The link field of the first tos element is used as an allocation
 *      counter for the tos space.  The last element of the tos space is
 *      unused due to the manner of element allocation.
 *
 * (RECOVERY OPERATION:) none
 *
 * (DATA STRUCTURES:) See mon.h and global definitions in mon.c.
 *
 * RETURNS: none
 */

#include <__mcount.h>

/* Quick increment, hoping not to be interrupted                              */
static unsigned long qincrement ( unsigned int delta, unsigned int * counter ) {
  unsigned long new_value = *counter + delta ;
  *counter = new_value ;
  return new_value ;
  }

static unsigned long qincrement1 ( unsigned int delta, unsigned int * counter ) {
  return qincrement(delta, counter) ;
  }

/* Increment atomically, prevent overflows
   This function is worth building 'inline'
*/
static void increment ( int * counter ) {
  int counter_value = * counter ;
  int new_counter_value = counter_value + 1 ;
  while (-1  != counter_value
      && 0 != cs(counter,counter_value,new_counter_value) ) {
    counter_value = *counter ;
    new_counter_value = counter_value + 1 ;
  } /* endwhile */
  }

/* The stack chain is machine specific. On AIXv3, when a function is using
   full linkage, a register is used as stack pointer. Routine __stack_pointer()
   returns this register, and then the chain is chased from the 'C'.

   The compiler uses abbreviated linkage for 'leaf' functions, i.e. those which
   do not call other functions. However, all profiled functions call __mcount()
   and are thereby not leaf functions ...
*/
typedef struct __stack_chain_struct * __stack_chain_ps ;

typedef struct __stack_chain_struct {
        __stack_chain_ps __stkback ;
        int __stkcond ;
        caddr_t __stklink ;
        int __reserved_1 ;
        int __reserved_2 ;
        caddr_t __stkret ;
        caddr_t __stktoc ;
        } __stack_chains ;
__stack_chains * __stack_pointer(void) ;

extern struct monglobal _mondata;       /* global control data */


/* This is the profiling routine itself. It checks whether profiling is
   active (which would normally be set in /etc/*crt0*.o), and handles the
   two cases ...
   1) Call count profiling.
   2) Call graph profiling.

   If the entry has been seen before, all that needs to be done is to find
   the right counter and increment it. This is handled within this file,
   except for the call to kernel cs() which is used to guarantee correctness
   even when asynchronous signals are occurring.

   If the entry has not been seen before, processing is handed off to
   __mcount_construct_count_element or __mcount_construct_graph_element,
   which build the appropriate structures.

   For call count entries which have been seen before, the function under
   test will pass in the address of its counter. Actually, it is the
   address of the address of its counter; it is done that way becuase
   __mcount_construct_count_element allocates the counters.

  For call graphing, there is one counter per pair (caller's program counter,
  caller's caller's program counter). Accordingly, this pair is determined
  by looking at the stack back chain, and the counter is located efficiently
  using a hash table.
*/
void __mcount ( ulong **cntp ) {
  ulong * counter_address = *cntp ;
  if (0 == _mondata.profiling ) {
    if (_PROF_TYPE_IS_P == _mondata.prof_type) {
      /* Call count profiling */
      if (counter_address != NULL ) {
        if (0 == qincrement1(1,(unsigned int *) counter_address)) {
          *counter_address = 0xffffffff ;
        } /* endif */
      } else {
        __mcount_construct_count_element(cntp,
                                __stack_pointer() -> __stkback -> __stklink
                                ) ;
      } /* endif */
    } else {

      /* Call graph profiling */
               /* Do the easy preliminaries */
      __stack_chains * caller_stack_frame = __stack_pointer() -> __stkback ;
      __stack_chains * return_stack_frame = caller_stack_frame -> __stkback ;
      caddr_t to_address = caller_stack_frame -> __stklink ;
      caddr_t from_address = return_stack_frame -> __stklink ;
               /* Scan the hash chain */
      { graph_chain * graph_element = __mcount_graph_hash
                                       + GRAPH_HASH(to_address, from_address) ;
        caddr_t cand_to_pc = graph_element -> to_pc ;
        caddr_t cand_from_pc = graph_element -> from_pc ;
        ulong * cand_counter = graph_element -> counter_address ;
        graph_chain * scan_element = graph_element -> next ;
        if ( cand_to_pc == to_address
          && cand_from_pc == from_address
        ) {
          if (0 == qincrement1(1,(unsigned int *) cand_counter)) {
            *cand_counter = 0xffffffff ;
          } /* endif */
        } else {
          for (; ; ) {
            if (NULL == scan_element) {
              __mcount_construct_graph_element(
                                   from_address,
                                   to_address,
                                   graph_element ) ;
              return ;
            } else {
              caddr_t cand_to_pc = scan_element -> to_pc ;
              caddr_t cand_from_pc = scan_element -> from_pc ;
              ulong * cand_counter = scan_element -> counter_address ;
              scan_element = scan_element -> next ;
              if (cand_to_pc == to_address && cand_from_pc == from_address) {
                if (0 == qincrement1(1,(unsigned int *) cand_counter)) {
                  *cand_counter = 0xffffffff ;
                } /* endif */
                return ;
              } /* endif */
            } /* endif */
          } /* endfor */
        } /* endif */
      }
    } /* endif */
  } /* endif */
  }

/*
   This is an alternate name for __mcount; I don't think it is ever used. It
   only works because the compiler uses tail recursion elimination; i.e. it
   compiles into a 'branch' instruction, and doesn't make a stack frame.
*/
void mcount(
        register ulong **cntp                   /* -p addr of counter or 0 */
        )                                       /* -pg ignored */
{
        __mcount(cntp);
}
