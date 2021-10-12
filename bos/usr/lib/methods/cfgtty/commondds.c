#ifndef lint
static char sccsid[] = "@(#)14 1.4 src/bos/usr/lib/methods/cfgtty/commondds.c, cfgtty, bos41J, 9520A_all 4/27/95 14:22:35";
#endif
/*
 * COMPONENT_NAME: (CFGTTY) Common functions for DDS builds
 *
 * FUNCTIONS: get_slot
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/* For performance purpose, use of ILS macros for isdigit */
#define _ILS_MACROS
#include <stdio.h>          /* standard I/O */
#include <cf.h>             /* error messages */

#include <sys/cfgdb.h>      /* config #define */
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/cfgodm.h>     /* config structures */
#include <ctype.h>          /* for ILS macros */

#include "cfgdebug.h"

/*
 * =============================================================================
 *                       COMMON FUNCTIONS USED BY DDS BUILD FUNCTIONS
 * =============================================================================
 * 
 * All these functions are used by DDS build functions for tty configuration
 * purpose.
 *
 * =============================================================================
 */

/*
 * -----------------------------------------------------------------------------
 *                       GET_SLOT
 * -----------------------------------------------------------------------------
 * This function takes a pointer to a character string 
 * representing the connection location and returns the
 * slot location number. 
 * If a SLOT number is not found in the current location,
 * this is, in fact, the native I/O which must always 
 * return ZERO. This number is used to generate the minor
 * and ibase register of the line.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
int get_slot(conn, slotNumber)
char * conn;
int  * slotNumber;
{
    /* ===================== */
    /* Get connection number */
    /* ===================== */
	if (isdigit(*conn)) {
		*slotNumber = atoi(conn);
	}
	else { /* It is for native I/O */
		*slotNumber = 0;
	} /* End if (isdigit(...)) */
	return (0);
} /* End int get_slot(...) */
