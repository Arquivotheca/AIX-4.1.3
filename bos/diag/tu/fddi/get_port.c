static char sccsid[] = "@(#)42	1.2.1.1  src/bos/diag/tu/fddi/get_port.c, tu_fddi, bos411, 9428A410j 1/24/92 16:52:52";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: get_port
 *		get_devno
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <sys/cfgodm.h>
#include "fdditst.h"

#ifdef debugg
extern void detrace();
#endif

/*****************************************************************************

get_port  using ODM

Function(s) Given the slot number, this routine will retrive the port id.

Module Name :  get_port.c
SCCS ID     :  1.8

Current Date:  5/30/91

*****************************************************************************/

int get_port(TUTYPE *tucb_ptr, char *dev)
{
    struct CuDv cusobj;		/* customized device object storage */
    char    bufp[128];		/* search string and filename buffer */
    int     rc;
    struct Class *cusdev;	/* customized vpd class ptr */

#ifdef debugg
	detrace(0,"slot = %c\n",tucb_ptr->slot);
#endif

    /* start up odm */
    if (odm_initialize () == -1)
    {
	/* initialization failed */
#ifdef debugg
	detrace (0,"odm: odm_initialize() failed\n");
#endif
	return(ODMINIT_ERR);
    }

    /*
     * Get child's Customized Object 
	 * Change to bufp should fix defect numbers 34131 and 34132.
     */
    sprintf (bufp, "name like 'fddi*' and connwhere = %c and parent = bus%c", tucb_ptr->slot,tucb_ptr->bus_no);

#ifdef debugg
	detrace (0,"odm1 string: %s\n", bufp);
#endif

    rc = (int) odm_get_first (CuDv_CLASS, bufp, &cusobj);
    if (rc == -1)
    {
#ifdef debugg
	detrace (0,"odm1: ODM fail on CuDv %s\n", bufp);
#endif
	return(ODMGET_ERR);
    }
    else if (rc == 0)
	return(NOATTR_ERR);

    strcpy (dev, cusobj.name);

    odm_terminate ();

#ifdef debugg
	detrace(0,"dev = %s\n",dev );
#endif
    return(0);
}

/*****************************************************************************

get_port  using ODM

Function(s) Given the slot number, this routine will retrive the device number
	    (i.e. major and minor number);

Module Name :  get_port.c
SCCS ID     :  1.8

Current Date:  5/30/91

*****************************************************************************/

int
get_devno (tucb_ptr,devno)
    TUTYPE  *tucb_ptr;
    ulong   *devno;
{
    char    devname[40] = "/dev/";
    char    dev[10];
    struct stat fstat;
    int    rc;

    rc = get_port (tucb_ptr, dev);
    if (rc != 0)
	return(rc);
    strcat (devname, dev);

    if ( statx(devname, &fstat, sizeof(fstat), 0 ))
    {
#ifdef debugg
	perror("statx");
#endif

	return(STATX_ERR); 
    }

    *devno = fstat.st_rdev;
    return (0);
}
