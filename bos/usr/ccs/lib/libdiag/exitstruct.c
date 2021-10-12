static char sccsid[] = "@(#)exitstruct.c	1.1 11/30/89 14:41:52";
/*
 * COMPONENT_NAME: (LIBDIAG) DIAGNOSTIC LIBRARY
 *
 * FUNCTIONS: da_exit_code structure 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include "diag/diag_exit.h"

da_returncode_t da_exit_code = {
		DA_STATUS_GOOD,
		DA_USER_NOKEY,
		DA_ERROR_NONE,
		DA_TEST_NOTEST,
		DA_MORE_NOCONT,
};
