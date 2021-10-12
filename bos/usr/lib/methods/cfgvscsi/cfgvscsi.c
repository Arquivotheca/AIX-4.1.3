static char sccsid[] = "@(#)96  1.4  src/bos/usr/lib/methods/cfgvscsi/cfgvscsi.c, cfgmethods, bos412, 9446B 10/28/94 10:29:31";
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: build_dds
 *		download_microcode
 *		generate_minor
 *		make_special_files
 *		query_vpd
 *		
 *
 *   ORIGINS: 27, 83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

/* 
 * Header files needed for compilation.
 */
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/device.h>
#include <cf.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/sysconfig.h>
#include <sys/scsi.h>
#include <sys/vscsidd.h>
#include <malloc.h>
#include <stdio.h>
#include "cfghscsi.h"
#include "cfgdebug.h"


#define ASCSI_UNIQUETYPE "adapter/mca/8efc"

/*	extern declarations	*/
extern int      errno;
extern int      Dflag;
extern long    *genminor ();

int mode = 1;

/*
 * NAME   : generate_minor
 *
 * FUNCTION: This function generates minor device number for the 
 *	     virtual SCSI device.
 * 
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine 
 *	called by the generic configure method for all devices.
 *	It makes use of generic genminor() function.
 *
 * INPUTS : device logical_name,major_no.
 *
 * RETURNS: Passes minor number as a parameter.
 *          Returns 0 on success.
 *          Returns E_MINORNO if minordest parameter is NULL
 *
 */
long 
generate_minor (char *lname,     /* logical name of device      */
		long  major_no,  /* major number                */
		long *minordest) /* pointer to the minor number */
{
    long           *minorno;
    
    minorno = genminor (lname, major_no, -1, 1, 1, 1);
    if (minorno == (long *) NULL)
	return (E_MINORNO);
    *minordest = *minorno;
    return (0);
}

/*
 *
 * NAME   : make_special_files
 * 
 * FUNCTION:
 *           Creates virtual SCSI device special file.
 *	     Virtual SCSI has one character type device special file
 *           per instance of it.  There are two instances of the
 *           device per fast/wide SCSI adapter:  
 *                             1 external
 *                             0 internal
 * 
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine
 *	called by the generic configure method for all devices.
 *
 * INPUTS : logical_name,devno
 *
 * RETURNS: Returns 0 if success else -1
 *
 */
int 
make_special_files (char  *lname,  /* logical device name */
		    dev_t  devno)  /* device number       */
{
    return (mk_sp_file (devno, lname, CRF));
}


/*
 * NAME	: query_vpd
 *
 * FUNCTION :
 *	This simply returns 0 if it gets called because
 *      virtual SCSI adapters have no VPD of their own.
 *
 * EXECUTION ENVIRONMENT:
 *
 * INPUT  : All input is unused.
 *
 * RETURNS: Returns 0
 *
 */
int 
query_vpd (struct CuDv    *cusobj,  /* unused */
	   mid_t           kmid,    /* unused */
	   dev_t           devno,   /* unused */
	   char           *vpd)     /* unused */
{
    return (0);
}				/* end query_vpd */


/*
 * NAME:  build_dds
 * 
 * FUNCTION:
 *	This function builds the dds from ODM database.
 *	DDS consists of attributes of the virtual SCSI adapter.
 *	It returns the pointer to dds to the generic config method
 *	which in turn passes it on to the device driver.
 *
 *      The virtual SCSI adapter uses a few of its parent adapter
 *      attributes to set its own attributes.  The dds contains an
 *      array for the resource_name, the parent's sequence no., the parent's
 *      logical name, the parent's interrupt priority, whether or not
 *      the target mode and wide attributes are enabled, the external
 *      bus data rate, and the bus
 *      SCSI ID for this virtual SCSI device, whether it is on the
 *      internal or external bus.
 *
 *      For full FCS support, this function will need to determine
 *      who its parent is, and use a different method for building
 *      the dds.  (For instance, the FCS may have an adapter level
 *      and a device driver level of indirection.  It is at the
 *      adapter level that an interrupt priority is set.  Therefore
 *      to get the interrupt priority set at this level, the program
 *      must be smart enough to go to its "grandparent".
 *
 *
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine 
 *	called by the generic configure method for all devices.
 *	This in turn calls getatt function to the attribute value
 *	from the database.
 *
 * INPUT:  logical_name, ptr to ddsptr, ptr to dds_length
 *
 * RETURNS : 0 if success
 *           E_MALLOC couldn't allocate memory
 *           E_ODMOPEN couldn't open object in ODM
 *	     E_NOCuDv if no CuDv for this device
 *	     E_ODMGET error accessing ODM
 *           E_NOCuDvPARENT can't get parent's CuDv
 *           E_NOATTR attribute not found.
 *           E_ODMUPDATE couldn't update ODM
 *	     E_INVATTR invalid attribute to change
 *           E_ODMCLOSE error closing ODM
 */
int 
build_dds (char  *lname,              /* logical name of device              */
	   char **dds_data_ptr,       /* data pointer to device driver stuff */
	   long  *dds_len)            /* length of dds                       */
{
    int             rc;              /* return code                        */
    struct CuDv     CuDv;            /* CuDv of this device 		   */
    struct CuAt    *CuAt;            /* CuAt of this device                */
    struct CuDv     CuDv_parent;     /* CuDv of parent device 		   */
    char            sstr[512];       /* ODM search string                  */
    int             how_many;        /* number of attributes returned      */
    int             bc;              /* byte count                         */
    char            wide_enabled[4]; /* wide enabled attribute             */
    long            max_data_rate;   /* maximum data rate attribute        */
    long            p_max_data_rate; /* parent maximum data rate attribute */
    char            pname[NAMESIZE]; /* parent logical name                */
    char            pt[UNIQUESIZE];  /* parent device unique type          */
    struct Class   *cusatt;          /* customized attributes              */
    struct Class   *preatt;          /* predefined attributes              */
    struct Class   *cusdev;          /* customized devices                 */
    int             num_attrs;       /* number of attributes               */
    uchar           scsi_id;         /* SCSI ID of this device             */
    uchar           p_scsi_id;       /* SCSI ID of parent device           */
    char            istr[80];        /* character representation of an int */
    static struct vscsi_ddi dds;     /* device driver stuff                */
    struct attr_list *alist;         /* attribute list                     */
    struct attr_list *palist;        /* parent's attribute list            */

        mode = 0;
	DEBUG_1("mode attribute value for this adapter: %d\n", mode);
    
    /*
     *  Get the device's CuDv 
     */
    sprintf (sstr, "name = '%s'", lname);
    rc = (int) odm_get_first (CuDv_CLASS, sstr, &CuDv);
    if (rc == 0)
    {
	DEBUG_1 ("bld_dds: No entry in CuDv for %s\n", sstr);
	return (E_NOCuDv);
    }
    if (rc == -1)
    {
	DEBUG_1 ("bld_dds: err in getobj from CuDv for %s\n", sstr);
	return (E_ODMGET);
    }

    /* Read the attributes from the customized & predefined classes */
    alist = (struct attr_list *) get_attr_list(CuDv.name,
					       CuDv.PdDvLn_Lvalue,
					       &how_many, 4);
    if (alist == (struct attr_list *) NULL)
    {
	DEBUG_0 ("bld_dds: get_attr_list failed.\n");
	return (E_NOATTR);
    }

    /*
     * dds.location is one if the CuDv.location represents an external
     * bus.  Connwhere is used since location also contains bus
     * and slot info.
     */
    dds.location = (strcmp (CuDv.connwhere, "1") == 0) ? 1 : 0;
    DEBUG_1 ("bld_dds: location =  %s\n", CuDv.location);
    DEBUG_1 ("bld_dds: connwhere = %s\n", CuDv.connwhere);

    /*
     *  Get the parent's (ascsi) CuDv 
     */
    sprintf (sstr, "name = '%s'", CuDv.parent);
    rc = (int) odm_get_first (CuDv_CLASS, sstr, &CuDv_parent);
    if (rc == 0)
    {
	DEBUG_1 ("bld_dds: No entry in CuDv for %s\n", sstr);
	return (E_NOCuDv);
    }
    if (rc == -1)
    {
	DEBUG_1 ("bld_dds: err in getobj from CuDv for %s\n", sstr);
	return (E_ODMGET);
    }

    /* Read the attributes from the customized & predefined classes */
    palist = (struct attr_list *) get_attr_list(CuDv_parent.name,
					        CuDv_parent.PdDvLn_Lvalue,
					        &how_many, 13);
    if (palist == (struct attr_list *) NULL)
    {
	DEBUG_1 ("bld_dds: get_attr_list for parent %s failed.\n", pname);
	return (E_NOATTR);
    }

    (void) strcpy (dds.resource_name, lname);

    (void) strcpy (dds.parent_lname, CuDv_parent.name);
    (void) strcpy (pname, CuDv_parent.name);
    (void) strcpy (pt, CuDv_parent.PdDvLn_Lvalue);
    
    DEBUG_2 ("bld_dds: parent = %s for %s\n", pname, lname);

    if ((dds.parent_unit_no = lsinst (pname)) == -1)
    {
	DEBUG_1 ("bld_dds: lsinst failed for %s\n", pname);
	return (E_LAST_ERROR);
    }
    DEBUG_1 ("bld_dds: dds.parent_unit_no = %d\n", dds.parent_unit_no);
    
    if ((int) (cusatt = odm_open_class (CuAt_CLASS)) == -1)
    {
	DEBUG_0 ("bld_dds: can not open CuAt\n");
	return (E_ODMOPEN);
    }
    if ((int) (preatt = odm_open_class (PdAt_CLASS)) == -1)
    {
	DEBUG_0 ("bld_dds: can not open PdAt\n");
	return (E_ODMOPEN);
    }
    
    /*
     * Use the location attribute that was set during the adapter define
     * children phase to determine if this is the internal or external (0
     * or 1, respectively) bus getting configured. 
     *
     * If this is the external bus, set the bus_scsi_id to be the parent's
     * CuAt external_id.  Check the wide_enabled attribute.  If it is
     * "no" and the external_bus_scsi_id is > 7, fail the config. 
     *
     * If it is the internal bus, neither the bus_scsi_id nor the
     * wide_enabled values are changeable.  
     */
    if (dds.location == 1)  /* This is an external bus */
    {
	/* Get and set num_tm_bufs.  */
	if ((rc = getatt (alist, "num_tm_bufs", (void *)&dds.num_tm_bufs,
			  'i', &bc)) != 0)
	    return (E_NOATTR);

	/* Get parent's external_id.  */
	if ((rc = getatt (palist, "external_id", (void *)&p_scsi_id, 
			  'c', &bc)) != 0)
	    return (E_NOATTR);
	dds.bus_scsi_id = p_scsi_id;

	/* Get the wide_enabled attribute from the parent.  */
	if ((rc = getatt (palist, "ext_wide_enable", (void *)wide_enabled,
			  's', &bc)) != 0)
	    return (E_NOATTR);
	dds.wide_enabled = (strcmp (wide_enabled, "yes") == 0) ? 1 : 0;
	
	/*
	 * If the external bus SCSI ID is greater than 7, and the
	 * wide_enabled attribute is not enabled, fail the config. 
	 *
	 * This situation could occur if the configuration of the
	 * machine had changed after the user had modified the SCSI
	 * ID of the external bus. 
	 */
	DEBUG_0 ("Testing wide_enabled\n");
	if (!(dds.wide_enabled) && (dds.bus_scsi_id > 7))
	    return (E_INVATTR);

	/*
	 * Get the max_data_rate.  It is only changeable for the
	 * external bus. 
	 */
	if ((rc = getatt (palist, "ext_bus_data_rt", (void *)&p_max_data_rate,
			  'l', &bc)) != 0)
	    return (E_NOATTR);

	/*
	 * The parent's rate is in MHz.  Convert this to kbytes/sec.
	 */
	p_max_data_rate *= 1000;

	/*
	 * The rate is doubled if wide_enabled is true.
	 */
	if (dds.wide_enabled)
	    p_max_data_rate *= 2;

	if ((rc = getatt (alist, "max_data_rate", (void *)&max_data_rate,
			  'l', &bc)) != 0)
	    return (E_NOATTR);
	/*
	 * If the max_data_rate of this device doesn't match the max_data_rate
	 * for the parent.  Then update the rate to that of the parent.
	 */
	if (max_data_rate != p_max_data_rate)
	{
	    /*
	     * Get a copy of the customized attribute "max_data_rate" so 
	     * it can be changed.
	     */
	    if ((CuAt = getattr(lname, "max_data_rate", 0, &num_attrs))
		== (struct CuAt *) NULL) 
	    {
		DEBUG_0("cfgvscsi: getattr for max_data_rate failed\n");
		return(E_NOATTR);
	    }

	    /*
	     * Convert the integer to a string.
	     */
	    sprintf (istr, "%d", p_max_data_rate);

	    (void) strcpy(CuAt->value, istr);
	    /* 
	     * Call putattr to update the data base 
	     */
	    if (putattr (CuAt) < 0) 
	    {
		DEBUG_0("cfgvscsi: putattr failed\n");
		return(E_ODMUPDATE);
	    }
	}
    } else
    {			/* It is an internal bus. */
	/* Get parent's internal_id.  */
	if ((rc = getatt (palist, "internal_id", (void *)&p_scsi_id, 
			  'c', &bc)) != 0)
	    return (E_NOATTR);
	dds.bus_scsi_id = p_scsi_id;

	DEBUG_0 ("Setting num_tm_bufs to 0\n");
	/* 
	 * Set the number of target mode buffers to 0 since target mode
	 * is not available on the internal bus.  Set this value in the
	 * ODM.
	 */
	dds.num_tm_bufs = 0;
	/*
	 * Get a copy of the customized attribute "num_tm_bufs" so 
	 * it can be changed.
	 */
	if ((CuAt = getattr(lname, "num_tm_bufs", 0, &num_attrs))
	    == (struct CuAt *) NULL) 
	{
	    DEBUG_0("cfgvscsi: getattr for num_tm_bufs failed\n");
	    return(E_NOATTR);
	}

	/*
	 * Convert the integer to a string.
	 */
	sprintf (istr, "%d", dds.num_tm_bufs);

	(void) strcpy(CuAt->value, istr);
	/* 
	 * Call putattr to update the data base 
	 */
	if (putattr (CuAt) < 0) 
	{
	    DEBUG_0("cfgvscsi: putattr failed\n");
	    return(E_ODMUPDATE);
	}

	/* Get the wide_enabled attribute from the parent.  */
	if ((rc = getatt (palist, "int_wide_enable", (void *)wide_enabled,
			  's', &bc)) != 0)
	    return (E_NOATTR);
	dds.wide_enabled = (strcmp (wide_enabled, "yes") == 0) ? 1 : 0;
    }

    /* Get this device's id.  */
    if ((rc = getatt (alist, "id", (void *)&scsi_id, 'c', &bc)) != 0)
	return (E_NOATTR);

    /*
     * Make sure the SCSI ID in the ODM is the same as the parent's
     */
    if (scsi_id != p_scsi_id)
    {
	/*
	 * Get a copy of the customized attribute "id" so changes can
	 * be made to it.
	 */
	if ((CuAt = getattr(lname, "id", 0, &num_attrs))
	    == (struct CuAt *) NULL) 
	{
	    DEBUG_0("cfgvscsi: getattr for attribute id failed\n");
	    return (E_NOATTR);
	}
	
	/*
	 * Convert the integer to a string.
	 */
	sprintf (istr, "%d", p_scsi_id);
	
	(void) strcpy(CuAt->value, istr);
	/* 
	 * Call putattr to update the data base 
	 */
	if (putattr (CuAt) < 0) 
	{
	    DEBUG_0("cfgvscsi: putattr failed\n");
	    return (E_ODMUPDATE);
	}
    }

    /*
     * Get parent's int_prior, and set ours to be the same. With FCS,
     * this may have to be a function call, because it will end up being
     * the grandparent's interrupt priority. 
     */
    if ((rc = getatt (palist, "intr_priority", (void *)&dds.intr_priority,
		      'i', &bc)) != 0)
	return (E_NOATTR);

    if ((rc = getatt (alist, "num_cmd_elems", (void *)&dds.num_cmd_elems,
		      'c', &bc)) != 0)
	return (E_NOATTR);
    
    /*
     * Close the ODM databases, and return good. 
     */
    if (odm_close_class (cusatt) < 0)
	return (E_ODMCLOSE);
    if (odm_close_class (preatt))
	return (E_ODMCLOSE);
    odm_close_class (cusdev);
    
    /* Hard code the command delay after reset attribute.  */
    dds.cmd_delay = 7;
    
    /* Use #defines to set the entity id's for FCS.  */
    dds.sc_im_entity_id = SCB_SCSI_INITIATOR;
    dds.sc_tm_entity_id = SCB_SCSI_TARGET;
    
    (void) strcpy (dds.resource_name, lname);
    
    *dds_data_ptr = (char *) &dds;
    *dds_len = sizeof (struct vscsi_ddi);
    
    return (0);
}


/*
 * NAME : download_microcode
 * 
 * FUNCTION :
 *	This device has no microcode.  This routine is provided as a
 *      stub to the generic configuration routine.
 *
 * EXECUTION ENVIRONMENT:
 *
 * INPUT  : All input is unused.
 *
 * RETURNS: Returns 0 
 *
 * NOTE :
 */
int
download_microcode (char  	      *logical_name, /* unused */
		    struct odm_ptrs   *odm_info,     /* unused */
		    struct cfg_kmod   *cfg_k)        /* unused */
{
    return (0);
}


/*
 * NAME   : define_children
 *
 * FUNCTION : this routine detects and defines devices attached to the
 *       scsi adapter. Supported devices are disks, tapes, cdroms,
 *       and target mode.
 *
 * EXECUTION ENVIRONMENT:
 *      This function operates as a device dependent subroutine
 *      called by the generic configure method for all devices.
 *
 * DATA STRUCTURES:
 *
 * INPUTS : logical_name,ipl_phase
 *
 * RETURNS: Returns 0 if success
 *
 * RECOVERY OPERATION:
 *
 */
int
define_children (char    *lname,
		 int      ipl_phase)
{
    int         rc;                     /* for return codes                 */
    struct CuDv CuDv;                   /* for getting CuDv attribute       */
    char	sstr[256];		/* for getting CuAt attribute       */
    int	        numsids;		/* number of SCSI Ids               */
    char	wide_enabled[4];	/* wide_enabled attribute           */
    int         external;
    int         how_many;               /* number of attributes returned    */
    int         bc;                     /* byte count                       */
    struct attr_list *palist;           /* parent's attribute list          */
    struct CuDv CuDv_parent;            /* CuDv of parent device            */
    
    DEBUG_0 ("cfgvscsidc: entering define children routine\n");

    /* get CuDv object for device */
    sprintf (sstr, "name = '%s'",lname);
    rc = (int)odm_get_first (CuDv_CLASS, sstr, &CuDv);
    if (rc == 0)
    {
	DEBUG_1 ("cfgvscsidc: No entry in CuDv for %s\n", sstr);
	return (E_NOCuDv);
    }
    if (rc == -1)
    {
	DEBUG_0 ("cfgvscsidc : ");
	DEBUG_1 ("error in getobj from CuDv for %s\n", sstr);
	return (E_ODMGET);
    }
    DEBUG_1 ("cfgvscsidc : uniquetype = %s\n", CuDv.PdDvLn_Lvalue);

    /*
     * Determine whether or not this is an external bus.
     */
    external = (strcmp (CuDv.connwhere, "1") == 0) ? 1 : 0;

    if (external)
    {
	/*
	 *  Get the parent's (ascsi) CuDv 
	 */
	sprintf (sstr, "name = '%s'", CuDv.parent);
	rc = (int) odm_get_first (CuDv_CLASS, sstr, &CuDv_parent);
	if (rc == 0)
	{
	    DEBUG_1 ("bld_dds: No entry in CuDv for %s\n", sstr);
	    return (E_NOCuDv);
	}
	if (rc == -1)
	{
	    DEBUG_1 ("bld_dds: err in getobj from CuDv for %s\n", sstr);
	    return (E_ODMGET);
	}
	
	/* Read the attributes from the customized & predefined classes */
	palist = (struct attr_list *) get_attr_list(CuDv_parent.name,
						    CuDv_parent.PdDvLn_Lvalue,
						    &how_many, 13);
	if (palist == (struct attr_list *) NULL)
	{
	    DEBUG_0 ("bld_dds: get_attr_list for parent failed.\n");
	    return (E_NOATTR);
	}
	/*
	 * Check attribute "wide_enabled" so the number 
	 * of SCSI IDs to check is known.
	 */
	if ((rc = getatt (palist, "ext_wide_enable", (void *)wide_enabled,
			  's', &bc)) != 0)
	    return rc;
	if (strcmp (wide_enabled, "no") == 0)
	    numsids = 8;
	else
	    numsids = MAX_ID;
    }
    else
	numsids = MAX_ID;
    
    rc = def_scsi_children (lname, ipl_phase, numsids, MAX_LUN);
    return (rc);
}
