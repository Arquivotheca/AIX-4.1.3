/* @(#)55	1.1  src/bos/usr/lib/methods/cfgktsm/cfgktsm.h, inputdd, bos41J, 9509A_all 2/14/95 12:56:25  */
/*
 *   COMPONENT_NAME: inputdd
 *
 *   FUNCTIONS: GETATT
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <cf.h>
#include <string.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/mode.h>
#include <sys/ktsmdds.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/inputdd.h>

#include "cfgcommon.h"
#include "cfgdebug.h"

#define GETATT( DEST, TYPE, CUSOBJ, ATTR, NEWATTRS ) {			\
		int rc;							\
		rc = getatt( DEST, TYPE, CuAt_CLASS, PdAt_CLASS,	\
			CUSOBJ.name, CUSOBJ.PdDvLn_Lvalue, ATTR,	\
			(struct attr *)NEWATTRS );			\
		if( rc > 0 ) {						\
			DEBUG_2("Error %x getting attr %s\n", odmerrno, ATTR )		\
			return rc;					\
        }                            \
	}


#define KEYBOARD_OFFSET  8       /* keyboard port offset from mouse    */
#define MCA_SUBCLASS  "kma"      /* PdDv subclass for MCA adapters     */
#define MOUSE_TYPE "mouse"       /* PdDv type for mouse adapters       */
#define MOUSE_LOCATION 'M'       /* mouse port in location string      */
#define KEYBOARD_TYPE "keyboard" /* PdDv type for keyboard adapters    */
#define KEYBOARD_LOCATION  'K'   /* keyboard port in location string   */
#define MKNOD_MODE (S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)


extern int ipl_phase;      /* ipl phase: 4=run,1=phase1,2=phase2 */
