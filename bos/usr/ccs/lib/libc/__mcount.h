/* @(#)10	1.1  src/bos/usr/ccs/lib/libc/__mcount.h, libcgen, bos411, 9428A410j 10/20/93 14:19:45 */
/*
 *   COMPONENT_NAME: LIBC
 *
 *   FUNCTIONS: GRAPH_HASH
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
typedef struct graph_chain_t {
  caddr_t from_pc ;
  caddr_t to_pc ;
  ulong * counter_address ;
  struct graph_chain_t * next ;
  } graph_chain ;

void __mcount_construct_graph_element (
        caddr_t from_address,
        caddr_t to_address,
        graph_chain * graph_element
        ) ;

void __mcount_construct_count_element(ulong **cntp, caddr_t from_pc) ;

#define GRAPH_HASH_COUNT 16384
#define GRAPH_HASH(x,y) ((((int)(x)+(int)(y)) & ((GRAPH_HASH_COUNT-1) << 2)) >> 2 ) 

extern graph_chain __mcount_graph_hash[GRAPH_HASH_COUNT] ;
