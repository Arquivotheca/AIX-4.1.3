/* @(#)89       1.2  src/bldenv/pkgtools/backup_io.h, pkgtools, bos412, GOLDA411a 1/29/93 16:05:07 */
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: buf_append
 *		buf_empty
 *		buf_full
 *		
 *
 *   ORIGINS: 3,27,9
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "adepackage.h"
#include <sys/ino.h>
#include <sys/dir.h>
#include <dumprestor.h>

dword      *buf_start = NULL;        /* where the data goes */
dword      *buf_save  = NULL;        /* place to save fragments */
dword      *buf_ptr   = NULL;        /* current position */

unsigned   buf_len;                  /* length of buffer in dwords */

#define buf_full() (buf_ptr >= buf_start + buf_len)
#define buf_empty() (buf_ptr == buf_start)
#define buf_append(dwp) (*buf_ptr++ = *(dwp))

#define NOT_PACKED   0
#define PACKED       1
