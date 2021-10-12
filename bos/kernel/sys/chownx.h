/* @(#)26	1.8  src/bos/kernel/sys/chownx.h, syslfs, bos411, 9428A410j 12/9/92 08:12:34 */
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: chownx header
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_CHOWNX
#define _H_CHOWNX

/*
 * Values for the sva_tflag.
 * Specifies how the uid and gid values are to be set.
 */
#define	T_OWNER_AS_IS	004	/* the owner id in the inode is		*/
				/*   unaltered				*/
#define	T_GROUP_AS_IS	040	/* the group id in the inode is		*/
				/*   unaltered				*/

/* flags 00100-04000 are reserved for the other parts of setattr */

#endif	/* _H_CHOWNX */
