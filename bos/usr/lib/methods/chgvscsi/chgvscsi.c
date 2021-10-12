static char sccsid[] = "@(#)10  1.2  src/bos/usr/lib/methods/chgvscsi/chgvscsi.c, cfgmethods, bos411, 9428A410j 11/17/93 15:27:37";
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: check_parms
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* 
 *   COMPONENT_NAME: SYSXSCSI
 * Header files needed for compilation.
 */
#include <stdio.h>
#include <cf.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <stdlib.h>
#include <strings.h>
#include "pparms.h"
#include "cfghscsi.h"
#include "cfgdebug.h"

/*
 * NAME: check_parms
 *
 * FUNCTION:
 *	It checks the device specific attributes that are dependent
 *	on other attrbutes. SCSI adapter does not have any specific
 *	attrbutes to be checked. It also tells generic change method
 *	whether changing Database alone/Device alone is allowed by
 *	this device. Scsi adapter allows changes to Database alone.
 *	It also updates the NVRAM scsi_id location for the adapter
 * 	if the SCSI ID attribute is changed in the database alone.
 * 
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine called
 *	by the generic change method for all devices.
 *
 * DATA STRUCTURES :
 *
 * RETURNS : 0 if success
 *	     E_INVATTR invalid attribute to change
 *
 * RECOVERY OPERATION :
 *
 */

int 
check_parms (struct attr    *attrs,   /* attribute to change */
	     int             pflag,   /* unused */
	     int             tflag,   /* unused */
	     char           *lname,   /* logical name */
	     char           *parent,  /* unused */
	     char           *loc,     /* slot adapter is in */
	     char           *badattr) /* bad attribute */
{
    int            external_bus;     /* flag - set if external bus */
    struct attr   *ap;               /* attribute to change */
    char           sstr[512];        /* search string */
    char           sibling[80];      /* name of sibling device */
    int            rc;               /* return code */
    struct CuDv    CuDv;             /* Customized Device object */
    struct CuDv    CuDv_sibling;     /* Customized Device object of sibling */
    struct CuDv    CuDv_parent;      /* Customized Device object of parent */
    struct CuAt    CuAt_sibling;     /* Customized Attribute of sibling */
    int            num_tm_bufs;      /* number of target mode buffers */
    int            cmd_elems;        /* number of command elements */
    int            cmd_elems_sibling;/* sibling's number of command elements */
    int            tm_dbmw;          /* target mode DMA bus memory width */
    
    /* 
     * First get the CuDv object for this device.
     */
    sprintf (sstr, "name = '%s'", lname);
    rc = (int) odm_get_first (CuDv_CLASS, sstr, &CuDv);
    if (rc == 0)
    {
	DEBUG_1 ("chk_parms: CuDv object not found for %s\n", sstr);
	return (E_NOCuDv);
    }
    else if (rc == -1)
    {
	DEBUG_1 ("chk_parms: Error getting object for %s\n", sstr);
	return (E_ODMGET);
    }
	
    /*
     * Check for the num_tm_bufs attribute being set.  This attribute
     * can't be changed from 0 for an internal bus and it must be at
     * least 0x10 for an external bus.  Make sure that num_tm_bufs has
     * enough bus memory width for the number of buffers requested.
     * The width must be 0x1000 bytes for each buffer.  The width is
     * stored in the parent's tm_dbmw attribute.
     */
    if (((ap = att_changed (attrs, "num_tm_bufs")) != NULL))
    {
	/*
	 * Convert the attribute to an integer.
	 */
	num_tm_bufs = strtol (ap->value, NULL, 0);
    
	/*
	 * Check to see which bus (internal or external) is
	 * being modified.
	 */
	external_bus = (strcmp (CuDv.connwhere, "1") == 0) ? 1 : 0;

	if (external_bus)
	{
	    if (num_tm_bufs < 0x10)
	    {
		DEBUG_0("chk_parms: num_tm_bufs must be > 0x10 for external bus\n");
		strcpy( badattr, "num_tm_bufs");
		return (E_ATTRVAL);
	    }

	    /* 
	     * Get the CuDv object for the parent device.
	     */
	    sprintf (sstr, "name = '%s'", CuDv.parent);
	    rc = (int) odm_get_first (CuDv_CLASS, sstr, &CuDv_parent);
	    if (rc == 0)
	    {
		DEBUG_1 ("chk_parms: CuDv object not found for %s\n", sstr);
		return (E_NOCuDv);
	    }
	    else if (rc == -1)
	    {
		DEBUG_1 ("chk_parms: Error getting object for %s\n", sstr);
		return (E_ODMGET);
	    }

	    /* 
	     * Get the tm_dbmw attribute of the parent.
	     */
	    if ((rc = getatt ((void *)&tm_dbmw, 'i', CuAt_CLASS, 
			      PdAt_CLASS, CuDv_parent.name,
			      CuDv_parent.PdDvLn_Lvalue,
			      "tm_dbmw", NULL)) > 0)
	    {
		DEBUG_1 ("chk_parms: Error getting object for %s\n",
			 CuDv_parent.name);
		return rc;
	    }
	    
	    if (num_tm_bufs * 0x1000 > tm_dbmw)
	    {
		DEBUG_1 ("chk_parms: Parent's tm_dbmw attribute must be at least 0x%X\n",
			 (num_tm_bufs * 0x1000));
		strcpy (badattr, "num_tm_bufs");
		return (E_ATTRVAL);
	    }
	} else  /* internal bus */
	{
	    if (num_tm_bufs != 0)
	    {
		DEBUG_0("chk_parms: num_tm_bufs must be = 0 for internal bus\n");
		strcpy (badattr, "num_tm_bufs");
		return (E_ATTRVAL);
	    }
	}
    }

    /*
     * Check the num_cmd_elems attribute to ensure that the total of
     * num_cmd_elems for both vscsi devices of an adapter don't exceed
     * 255.  Also the max. for a single vscsi device is 215.  This ensures
     * that each device has minimum of 40 available.
     */
    else if (((ap = att_changed (attrs, "num_cmd_elems")) != NULL))
    {
	sprintf (sstr, "parent = '%s' AND name != '%s'", CuDv.parent, lname);
	rc = (int) odm_get_first (CuDv_CLASS, sstr, &CuDv_sibling);
	if (rc == 0)
	{
	    DEBUG_0("chk_parms: No sibling found.  Any valid value is okay.\n");
	    return (0);
	}
	else if (rc == -1)
	{
	    DEBUG_1 ("chk_parms: Error getting object for %s\n", sstr);
	    return (E_ODMGET);
	}

	cmd_elems = strtol (ap->value, NULL, 0);
	/*
	 * Don't allow changes to greater than 215.
	 */
	if (cmd_elems > 215)
	{
	    DEBUG_1 ("chk_parms: num_cmd_elems for %s can't exceed 215\n",
		     lname);
	    strcpy (badattr, "num_cmd_elems");
	    return (E_ATTRVAL);
	}

	/* 
	 * Get the num_cmd_elems of the sibling.
	 */
	if ((rc = getatt ((void *)&cmd_elems_sibling, 'i', CuAt_CLASS, 
			  PdAt_CLASS, CuDv_sibling.name, 
			  CuDv_sibling.PdDvLn_Lvalue,
			  "num_cmd_elems", NULL)) > 0)
	{
	    DEBUG_1 ("chk_parms: Error getting object for %s\n",
		     CuDv_sibling.name);
	    return rc;
	}
	
	if (cmd_elems + cmd_elems_sibling > 255)
	{
	    DEBUG_2 ("chk_parms: Total num_cmd_elems for %s and %s can't exceed 255\n",
		     lname, CuDv_sibling.name);
	    strcpy (badattr, "num_cmd_elems");
	    return (E_ATTRVAL);
	}
    }

    return (0);
}
