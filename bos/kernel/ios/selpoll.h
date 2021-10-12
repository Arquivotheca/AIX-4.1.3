/* @(#)45	1.6  src/bos/kernel/ios/selpoll.h, sysios, bos411, 9428A410j 9/17/93 10:15:43 */
#ifndef _h_SELPOLL
#define _h_SELPOLL
/*
 * COMPONENT_NAME: (SYSIOS) Select/Poll system call header file
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
/*
 * LEVEL 1,5 Years Bull Confidential Information
 */

#define	NBITS		(NBPW * NBPB)	/* #bits per word		*/

#define	MAX_SEL_CBS	511		/* must be prime number		*/

#define	DEV_HASH_MASK	(MAX_SEL_CBS - 1)	/* used in hashing algorithm*/

/*
 * Internal structures used to hold the pointers
 * to the user's data and the pointers to the
 * kernel's copy of the data.
 */
struct	sellist_2
{
	int	*fdsmask;
	int	*msgids;
};

struct	sel_ptrs
{
	void 			*user_ptr;
	struct sellist_2	kernel_ptr;
};

/*
 * Select control block structure
 */
struct	sel_cb
{
	struct sel_cb	*dev_chain_f;	/* next blk on hash chain	*/
	struct sel_cb	*dev_chain_b;	/* prev. blk on hash chain	*/
	struct sel_cb	*thread_chain;	/* next blk on thread chain	*/
	ushort 		reqevents;	/* requested events		*/
	ushort 		rtnevents;	/* returned events		*/
	int		dev_id;		/* device id: devno, etc.	*/
	int		unique_id;	/* unique id: chan, etc.	*/
	struct thread	*threadp;	/* ptr to thread table entry	*/
	int		corl;		/* correlator: fp, etc.		*/
	void		(*notify)();    /* function ptr for nested poll */
};

/*
 * Valid flag definitions for the call to sel_hash to add or remove a
 * select control block to or from the select/poll hash table.
 */ 
#define	SEL_HASH_ADD	1		/* sel_hash routine add cb flag	*/
#define	SEL_HASH_DEL	2		/* sel_hash routine delete cb flag*/

#endif  /* _h_SELPOLL */
