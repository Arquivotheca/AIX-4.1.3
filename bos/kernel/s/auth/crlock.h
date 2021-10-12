/*
 *   COMPONENT_NAME: syssauth
 *
 *   FUNCTIONS: CRED_LOCK
 *		CRED_UNLOCK
 *		CR_LOCK
 *		CR_UNLOCK
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



/* Lock the cred lock in the u structure */
#define CR_LOCK()       simple_lock(&U.U_cr_lock)

/* Unlock the cred lock in the u structure */
#define CR_UNLOCK()     simple_unlock(&U.U_cr_lock)

/* Lock the cred_lock lock */
#define CRED_LOCK()     simple_lock(&cred_lock)

/* Unlock the cred_lock lock */
#define CRED_UNLOCK()   simple_unlock(&cred_lock)
