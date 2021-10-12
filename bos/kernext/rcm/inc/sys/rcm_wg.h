/* @(#)14	1.1  src/bos/kernext/rcm/inc/sys/rcm_wg.h, rcm, bos411, 9428A410j 11/9/93 08:53:41 */

/*
 * COMPONENT_NAME: (rcm) AIX Rendering Context Manager structure definitions
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991-1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
	
#ifndef _H_RCX_WG
#define _H_RCX_WG



/* -----------------------------------------------------------------
	Define the WG hash table entry here
   -------------------------------------------------------------- */

typedef struct _rcm_wg_hash
{
    ulong  *pWG  ;	 	/* Pointer to the window geom (chain) */

} rcm_wg_hash_t ;


/* ----------------------------------------------------------------
	Define the entire hash table here:

	The value RCM_WG_HASH_SIZE was pick somewhat arbitrarily.
	Although very few hash collisions were observed during 
	development, this value could be evaluated later for
	performance reasons.
   ---------------------------------------------------------------*/

#define  RCM_WG_HASH_SIZE	1024 
#define  RCM_WG_HASH_SHIFT	6 
#define  RCM_WG_HASH_MASK	((RCM_WG_HASH_SIZE-1)<<RCM_WG_HASH_SHIFT)

typedef struct _wg_hash_table 
{
	rcm_wg_hash_t 	entry[RCM_WG_HASH_SIZE] ;

} rcm_wg_hash_table_t ;



/* ----------------------------------------------------------------
	Macro to perform the hashing function

	Because of the way that the WG pointers are allocated, the
	6 low-order bits of a given WG pointer may be the same as 
	any other.  Therefore, shift out the 6 low-order bits 
	(RCM_WG_HASH_SHIFT) and use the next 10 bits as an index 
	into the hash table.  These 10 bits should be the most 
	likely to vary between WG pointers.
   ---------------------------------------------------------------*/

#define  RCM_WG_HASH(pWG) 						\
 	( ((ulong)pWG & RCM_WG_HASH_MASK) >> RCM_WG_HASH_SHIFT) 	





#endif /* _H_RCX_WG */
