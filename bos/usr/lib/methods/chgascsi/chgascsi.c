static char sccsid[] = "@(#)01	1.3  src/bos/usr/lib/methods/chgascsi/chgascsi.c, cfgmethods, bos411, 9428A410j 5/5/94 13:10:52";
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
 * INPUT : pointer to array of attribute structures,pflag,tflag,lname.
 *
 * RETURNS : 0 if success
 *	     E_NOCuDv if no CuDv for this device
 *	     E_ODMGET error accessing ODM
 *	     E_INVATTR invalid attribute to change
 *           E_NOCuDvPARENT parent not found for the device
 *           E_NOATTR attribute not found (getatt)
 *           E_BADATTR attribute type is invalid (getatt)
 *           E_OPEN error opening NVRAM (put_scsi_id)
 *           E_DEVACCESS error writing to NVRAM (put_scsi_id)
 *
 * RECOVERY OPERATION :
 *
 */

int 
check_parms (struct attr    *attrs,   /* attribute to change */
	     int             pflag,   /* indicates if only changing database */
	     int             tflag,   /* unused */
	     char           *lname,   /* logical name */
	     char           *parent,  /* unused */
	     char           *loc,     /* slot adapter is in */
	     char           *badattr) /* bad attribute */
{
    uchar          slot;             /* slot card is in */
    struct attr   *ap;               /* attribute to change */
    char           sstr[512];        /* search string */
    char           pname[NAMESIZE];  /* parent name */
    char           pt[UNIQUESIZE];   /* parent type */
    char           ut[UNIQUESIZE];   /* unique type */
    int            rc;               /* return code */
    int            bus_id;           /* bus id adapter card is on */
    struct CuDv    CuDv;             /* Customized Device object */
    struct CuDv    CuDv_child;       /* Customized Device object of child */
    uchar	   e_scsi_id;        /* external card SCSI ID */
    char           wide_enabled[4];  /* wide_enabled attribute */
    int            tm_dbmw;          /* target mode DMA bus memeory width */
    int            num_tm_bufs;      /* num. target mode buffers at child */
    
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
     * Check for the wide_enabled attribute being set.  Wide_enabled 
     * cannot be set to "no" when the external SCSI ID is set to 
     * greater than 7.
     */
    if (((ap = att_changed (attrs, "ext_wide_enable")) != NULL))
    {
	if (strcmp (ap->value, "no") == 0)
	{
	    /* 
	     * Assign uniquetype (ut) from CuDv attributes.
	     */
	    strcpy (ut, CuDv.PdDvLn_Lvalue);
	    
	    /* 
	     * Get the external bus SCSI ID.
	     */
	    if ((rc = getatt ((void *)&e_scsi_id, 'c', CuAt_CLASS, 
			      PdAt_CLASS, lname, ut,
			      "external_id", NULL)) > 0)
		return rc;
	    
	    if (e_scsi_id > 7)
	    {
		DEBUG_1 ("chk_parms: external id %d invalid when setting wide_enabled to no\n",
			 e_scsi_id);
		strcpy (badattr, "ext_wide_enable");
		return (E_ATTRVAL);
	    }
	}
    }

    /*
     * Check the external_id attribute being set.  This attribute can
     * be set to greater than 7 if wide_enabled is "yes".
     */
    else if (((ap = att_changed (attrs, "external_id")) != NULL))
    {
	e_scsi_id = atoi(ap->value);
	/*
	 * Wide_enabled must be yes for values over 7.
	 */
	if (e_scsi_id > 7)
	{
	    /* 
	     * Assign uniquetype (ut) from CuDv attributes.
	     */
	    strcpy (ut, CuDv.PdDvLn_Lvalue);
	    
	    /* 
	     * Get the wide_enabled attribute.
	     */
	    if ((rc = getatt ((void *)wide_enabled, 's', CuAt_CLASS, 
			      PdAt_CLASS, lname, ut,
			      "ext_wide_enable", NULL)) > 0) {
                DEBUG_1("chk_parms: getatt failed and returned %d\n", rc);
		return rc;
            }
	    if (strcmp (wide_enabled, "no") == 0)
	    {
		DEBUG_1 ("chk_parms: external id %d invalid when ext_wide_enable is no\n",
			 e_scsi_id);
		strcpy (badattr, "external_id");
		return (E_ATTRVAL);
	    }
	}
	
	if (loc != NULL)
	    slot = atoi (loc);
	else
	{
	    /* Get slot  from CuDv */
	    slot = atoi (CuDv.connwhere);
	}
	/*
	 * Get the parent bus object in order to determine the bus id 
	 * which is necessary for put_scsi_id.
	 */
	if ((rc = Get_Parent_Bus (CuDv_CLASS, CuDv.parent, &CuDv)) != 0)
	{
	    DEBUG_1 ("chgascsi: Unable to get parent bus object rc = %d\n",
		     rc);
	    if (rc == E_PARENT)
	    {
		rc = E_NOCuDvPARENT;
	    }
	    return (rc);
	}
	strcpy (pname, CuDv.name);
	strcpy (pt, CuDv.PdDvLn_Lvalue);
	/* Get the bus id attribute */
	if ((rc = getatt ((void *)&bus_id, 'l', CuAt_CLASS, PdAt_CLASS,
			  pname, pt, "bus_id", NULL)) > 0)
	    return rc;
	
	/*
	 * Put the SCSI ID into NVRAM.
	 */
	if (pflag == CHGDB_ONLY)
	{
	    DEBUG_2 ("chk_parms: put id%d into nvram for slot%d\n", 
		     e_scsi_id, slot);
	    if ((rc = put_scsi_id (slot, e_scsi_id, bus_id)) != 0)
	    {
		DEBUG_2 ("chk_parms: can put scsi_id %d for slot%d\n",
			 e_scsi_id, slot)
		    return (rc);
	    }
	}
    }

    /*
     * Check the tm_dbmw attribute being set.  The child device that's an
     * external bus must have its num_tm_bufs set to less than or equal
     * to tm_dbmw / 0x1000.
     */
    else if (((ap = att_changed (attrs, "tm_dbmw")) != NULL))
    {
	tm_dbmw = strtol (ap->value, NULL, 0);

	/*
	 * Find the CuDv for the child.
	 */
	sprintf (sstr, "parent = '%s' AND connwhere=1", lname);
	rc = (int) odm_get_first (CuDv_CLASS, sstr, &CuDv_child);
	if (rc == 0)
	{
	    DEBUG_0("chk_parms: No child found.  Any valid value is okay.\n");
	    return (0);
	}
	else if (rc == -1)
	{
	    DEBUG_1 ("chk_parms: Error getting object for %s\n", sstr);
	    return (E_ODMGET);
	}

	/* 
	 * Get the num_tm_bufs of the child.
	 */
	if ((rc = getatt ((void *)&num_tm_bufs, 'i', CuAt_CLASS, 
			  PdAt_CLASS, CuDv_child.name, 
			  CuDv_child.PdDvLn_Lvalue,
			  "num_tm_bufs", NULL)) > 0)
	{
	    DEBUG_1 ("chk_parms: Error getting object for %s\n",
		     CuDv_child.name);
	    return rc;
	}
	if (num_tm_bufs > tm_dbmw / 0x1000)
	{
	    DEBUG_2 ("chk_parms: %s's attribute num_tm_bufs must be at most %d \n",
		     CuDv_child.name, (tm_dbmw / 0x1000));
	    strcpy (badattr, "tm_dbmw");
	    return (E_ATTRVAL);
	}
    }

    return (0);
}
