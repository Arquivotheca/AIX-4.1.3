/* @(#)82	1.4  src/bos/kernel/sys/POWER/fp_cpusync.h, sysproc, bos411, 9428A410j 4/2/93 13:52:02 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_FP_CPUSYNC
#define _H_FP_CPUSYNC

#ifdef _NO_PROTO
     int  fp_cpusync();
#else
     int  fp_cpusync( int flag );

#endif /* _NO_PROTO */


/*
 * Arguments to fp_cpusync.
 */

#define FP_SYNC_OFF   0
#define FP_SYNC_ON    1
#define FP_SYNC_QUERY 2
#define FP_SYNC_RANGE FP_SYNC_QUERY   /* upper bound for SYNC requests */
#define FP_IMP_OFF    3
#define FP_IMP_ON     4
#define FP_IMP_QUERY  5
#define FP_SYNC_ERROR -1

#endif /* _H_FP_CPUSYNC */
