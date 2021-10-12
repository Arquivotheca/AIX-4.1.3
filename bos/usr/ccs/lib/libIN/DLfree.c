static char sccsid[] = "@(#)09	1.7  src/bos/usr/ccs/lib/libIN/DLfree.c, libIN, bos411, 9428A410j 6/10/91 10:15:22";
/*
 * LIBIN: DLfree
 *
 * ORIGIN: 9
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: Deallocate a device.
 *
 * PARAMETERS:
 *           devname         ... name of device to be free'd
 *
 *
 * RETURN VALUE DESCRIPTION: 
 *           FALSE           ... unlock failed (see cerror)
 *           TRUE            ... device deallocated successfully
 */

#include <IN/standard.h>
#include <sys/limits.h>
#include <sys/dir.h>

#define LOCKDIR "/etc/locks/"
#define LOCKLEN (MAXNAMLEN+12)

DLfree( devname )
 char *devname;
{       char lockname[ LOCKLEN ];
	extern char *CScat(), *CSsname();

	CScat(lockname, LOCKDIR, CSsname( devname ), 0 );
	if (unlink( lockname ) == 0)
		return TRUE;
	else
		return FALSE;
}
