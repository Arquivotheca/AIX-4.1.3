static char sccsid[] = "@(#)70  1.2  src/bos/usr/lib/methods/cfgascsi/cfgascsi.c, cfgmethods, bos411, 9428A410j 11/17/93 15:28:27";
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: build_dds
 *		device_specific
 *		download_microcode
 *		query_vpd
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
 * Header files needed for compilation.
 */
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/device.h>
#include <cf.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/sysconfig.h>
#include <sys/ascsidd.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cfghscsi.h"
#include "cfgdebug.h"
#include "cfgtoolsx.h"
#include "cfg_ndd.h"

#define CHILD_U_TYPE "driver/scsi_scb/vscsi"

/* function prototypes */
int
getatt (struct attr_list *alist, char *aname, void *dest_addr,
	char dest_type, int *bc);

/*      extern declarations     */
extern int      errno;


/*
 * NAME:       query_vpd
 *
 * FUNCTION:
 *	This routine gets VPD data from scsi adapter using
 *	sysconfig system call and stores it in the database.
 *
 *      When the adapter is integrated, the VPD is taken
 *      from its parent, adapter/sio.
 *
 * EXECUTION ENVIRONMENT:
 *	This function is called by generic config method.
 * 
 * RETURNS: Returns 0 if success else E_SYSCONFIG.
 *
 */
int 
query_vpd (char              *lname,    /* logical name of device    */
	   struct odm_ptrs   *odm_info, /* pointers to ODM objects   */
	   struct cfg_kmod   *cfg_k,    /* sysconfig parameter       */
	   char	             *vpd)      /* Vital Product Data        */
{
    int             rc;               /* return code */
    int             len = VPDSIZE;    /* length of the VPD */
    char            tmp_vpd[VPDSIZE]; /* temporary storage for the VPD */

    /*
     * Assign kmid, devno, cmd, ddsptr, and ddslength to the VPD struct. 
     */
    cfg_k->cmd = CFG_QVPD;

    ((struct ndd_config *) cfg_k->mdiptr)->p_vpd = tmp_vpd;
    ((struct ndd_config *) cfg_k->mdiptr)->l_vpd = len;
    
    DEBUG_0 ("cfgascsi: getting MCA attached SCSI VPD\n");
    rc = sysconfig (SYS_CFGKMOD, cfg_k, sizeof (struct cfg_kmod));

    /* Handle errors accordingly.  */
    if (rc < 0)
    {
#ifdef CFGDEBUG
	switch (errno)
	{
	    /*
	     * E_SYSCONFIG is returned, no matter what, so this switch is
	     * only for DEBUG statements to help determine why -1 was the
	     * result of the sysconfig call. 
	     */
	case EINVAL:
	    DEBUG_1 ("cfgascsi: sysconfig invalid kmid = %d\n", cfg_k->kmid);
	    break;
	case EACCES:
	    DEBUG_0 ("cfgascsi: sysconfig not privileged\n");
	    break;
	case EFAULT:
	    DEBUG_0 ("cfgascsi: sysconfig I/O error\n");
	    break;
	default:
	    DEBUG_1 ("cfgascsi: sysconfig errno = 0x%x\n", errno);
	    break;
	}
#endif
	return (E_SYSCONFIG);
    }

    put_vpd (vpd, tmp_vpd, len);

    return (0);
}				/* end query_vpd */


/*
 * NAME : build_dds
 * 
 * FUNCTION :
 *	This function builds the dds from ODM database.
 *	DDS consists of attributes of the SCSI adapter, as well
 *      as other vital data.  The routine returns the pointer
 *      to dds to the generic config method
 *	which in turn passes it on to the device driver.
 *
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine 
 *	called by the generic configure method for all devices.
 *	This in turn calls getatt function to the attribute value
 *	from the database.
 *
 * RETURNS : 0 if success 
 *           E_MALLOC couldn't allocate memory
 *	     E_NOCuDv if no CuDv for this device
 *	     E_ODMGET error accessing ODM
 *           E_NOCuDvPARENT can't get parent's CuDv
 *           E_NOATTR attribute not found.
 *           E_ATTRVAL invalid attribute value.
 *
 */
int 
build_dds (char *lname,               /* logical name of device              */
	   ndd_cfg_odm_t *odm_info,   /* pointers to ODM objects             */
	   char **dds_data_ptr,       /* data pointer to device driver stuff */
	   long * dds_len)            /* length of dds                       */
{
    int             rc;              /* return code                       */
    struct CuDv     CuDv;            /* Customized device object          */
    struct CuDv     CuDv_bus;        /* parent's Customized device object */
    int             how_many;        /* number of attributes returned     */
    char            bb[4];           /* value of battery backed attribute */
    char            wide_enabled[4]; /* value of wide_enabled attribute   */
    int             bc;              /* byte count                        */
    struct Class   *cusdev;          /* customized devices                */
    struct attr_list *alist;         /* attribute list                    */
    struct attr_list *palist;        /* parent's attribute list           */
    static struct ascsi_ddi dds;     /* device driver stuff               */
    int             bus_num;         /* bus number                        */ 

    cusdev = odm_info->cusdev;
    CuDv = odm_info->cusobj;

    /* Read the attributes from the customized & predefined classes */
    alist = (struct attr_list *) get_attr_list(CuDv.name,
					       CuDv.PdDvLn_Lvalue,
					       &how_many, 13);
    if (alist == (struct attr_list *) NULL)
    {
	DEBUG_0 ("bld_dds: get_attr_list failed.\n");
	return (E_NOATTR);
    }

    /* Set slot to connwhere - 1.  */
    dds.slot = atoi (CuDv.connwhere) - 1;
    DEBUG_2 ("build_dds: conwhere=%s slot=%d\n", CuDv.connwhere, dds.slot);
    
    /*
     * Get the parent bus so that we can set some attributes based on the
     * bus to which we are attached, (either directly or indirectly). 
     */
    if ((rc = Get_Parent_Bus (cusdev, CuDv.parent, &CuDv_bus)) != 0)
    {
	DEBUG_1 ("cfgascsi: Unable to get parent bus object; rc = %d\n", rc);
	if (rc == E_PARENT)
	    return (E_NOCuDvPARENT);
    }

    /* Read the bus's attributes from the customized & predefined classes */
    palist = (struct attr_list *) get_attr_list (CuDv_bus.name,
						 CuDv_bus.PdDvLn_Lvalue,
						 &how_many, 3);
    if (palist == (struct attr_list *) NULL)
    {
	DEBUG_0 ("bld_dds: get_attr_list for parent failed.\n");
	return (E_NOATTR);
    }

    /*
     * Use the getatt function to extract the proper predefined or
     * customized attribute value from the odm databases. 
     */
    
    /*
     * Use the parent name to get bus_id.  For IOCC to use the bus, the
     * id has to be ORred with a #defined string. 
     */
    if ((rc = getatt (palist, "bus_id", (void *)&dds.bus_id, 'l', &bc)) != 0)
    {
	DEBUG_0 ("build_dds: getatt failed for bus_id\n.");
	return (rc);
    }
    dds.bus_id |= BB_BUS_ID;
    
    /* Use the parent name to get the bus_type.  */
    if ((rc = getatt (palist, "bus_type", (void *)&dds.bus_type, 
		      'h', &bc)) != 0)
	return (E_NOATTR);


    /* Get the external bus SCSI ID.  */
    if ((rc = getatt (alist, "external_id", (void *)&dds.e_card_scsi_id,
		      'c', &bc)) != 0)
	return (E_NOATTR);
    
    /* Get the internal bus SCSI ID.  */
    if ((rc = getatt (alist, "internal_id", (void *)&dds.i_card_scsi_id, 
		      'c', &bc)) != 0)
	return (E_NOATTR);
    
    /* Get the external wide_enabled attribute.  */
    if ((rc = getatt (alist, "ext_wide_enable", (void *)wide_enabled, 's',
		      &bc)) != 0)
	return (E_NOATTR);
    dds.ext_wide_ena = (strcmp (wide_enabled, "yes") == 0) ? 1 : 0;
    /*
     * If the external bus SCSI ID is greater than 7, and the
     * wide_enabled attribute is not enabled, fail the config. 
     *
     * This situation could occur if the configuration of the machine had
     * changed after the user had modified the SCSI ID of the external
     * bus. 
     */
    if (!(dds.ext_wide_ena) && (dds.e_card_scsi_id > 7))
    {
	/* fail the config */
	return (E_ATTRVAL);
    }
    
    /* Get the internal wide_enabled attribute.  */
    if ((rc = getatt (alist, "int_wide_enable", (void *)wide_enabled, 's',
		      &bc)) != 0)
	return (E_NOATTR);
    dds.int_wide_ena = (strcmp (wide_enabled, "yes") == 0) ? 1 : 0;
    /*
     * If the internal bus SCSI ID is greater than 7, and the
     * wide_enabled attribute is not enabled, fail the config. 
     *
     * This situation could occur if the configuration of the machine had
     * changed after the user had modified the SCSI ID of the external
     * bus. 
     */
    if (!(dds.int_wide_ena) && (dds.i_card_scsi_id > 7))
    {
	/* fail the config */
	return (E_ATTRVAL);
    }

    /* Get the bus_io_addr.            */
    if ((rc = getatt (alist, "bus_io_addr", (void *)&dds.base_addr, 'i',
		      &bc)) != 0)
	return (E_NOATTR);
    
    /* Get the battery_backed attribute, and set it in the dds.  */
    if ((rc = getatt (alist, "bb", (void *)bb, 's', &bc)) != 0)
	return (E_NOATTR);
    dds.battery_backed = (strcmp (bb, "yes") == 0) ? 1 : 0;
    
    /* Get the dma_lvl.   */
    if ((rc = getatt (alist, "dma_lvl", (void *)&dds.dma_lvl,
		      'i', &bc)) != 0)
	return (E_NOATTR);
    
    /* Get the bus_intr_lvl.      */
    if ((rc = getatt (alist, "bus_intr_lvl", (void *)&dds.int_lvl, 
		      'i', &bc)) != 0)
	return (E_NOATTR);
    
    /* Get the intr_priority.  */
    if ((rc = getatt (alist, "intr_priority", (void *)&dds.int_prior,
		      'i', &bc)) != 0)
	return (E_NOATTR);
    
    /* Get the dma_bus_mem.  */
    if ((rc = getatt (alist, "dma_bus_mem", (void *)&dds.tcw_start_addr,
		      'l', &bc)) != 0)
	return (E_NOATTR);
    
    /* Get the dbmw attribute to set the tcw_length.  */
    if ((rc = getatt (alist, "dbmw", (void *)&dds.tcw_length, 'l', &bc)) != 0)
	return (E_NOATTR);
    
    /* Get the tm_bus_mem attribute to set the tm_tcw_start_addr.  */
    if ((rc = getatt (alist, "tm_bus_mem", (void *)&dds.tm_tcw_start_addr,
		      'l', &bc)) != 0)
	return (E_NOATTR);

    /* Get the tm_dbmw attribute to set the tm_tcw_length.  */
    if ((rc = getatt (alist, "tm_dbmw", (void *)&dds.tm_tcw_length, 
		      'l', &bc)) != 0)
	return (E_NOATTR);

    /* Get the external_bus_data_rate.  */
    if ((rc = getatt (alist, "ext_bus_data_rt", 
		      (void *)&dds.ext_bus_data_rate, 'i', &bc)) != 0)
	return (E_NOATTR);

    /* Set the resource_name with the lname.   */
    strcpy (dds.resource_name, lname);
    
    /* Set dds values - *dds_data_ptr and dds_length.  */
    *dds_data_ptr = (char *) &dds;
    *dds_len = sizeof (struct ascsi_ddi);
    
    DEBUG_1 ("ascsi DDS length: %d\n", *dds_len);
#ifdef  CFGDEBUG
    hexdump (*dds_data_ptr, (long) *dds_len);
#endif  CFGDEBUG
    
    /*
     * IPL ROS needs to know that the boot device can reside on a device
     * attached to the internal bus, but that in such a case, the SCSI ID
     * will not be stored in NVRAM. 
     *
     * Write the external card SCSI ID to nvram. 
     */
    bus_num = CuDv.location[3] - '0';
    DEBUG_2 ("build_dds: put_scsi_id for slot %d  bus number %d\n",
	     dds.slot, bus_num);
    if (put_scsi_id ((dds.slot + 1), dds.e_card_scsi_id, bus_num) != 0)
	DEBUG_1 ("build_dds: put_scsi_id err for external card in slot%d\n",
		 dds.slot);

    return (0);
}


/*
 * NAME : download_microcode
 * 
 * FUNCTION :
 *	This device does no microcode downloads.  This routine is provided
 *      as a stub to the generic configuration routine.
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
 * NAME : device_specific
 * 
 * FUNCTION :
 *      This function is provided as a hook into the configuration method.
 *	This device needs no special processing for configuration. This is
 *      provided as a stub to the generic configuration routine.
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
device_specific (char    	   *logical_name, /* unused */
		 struct odm_ptrs   *odm_info,     /* unused */
		 struct cfg_kmod   *cfg_k)        /* unused */
{
    return (0);
}

/*
 * NAME:    define_children
 *
 * FUNCTION:
 *       This routine defines the two busses associated with the
 *       fast, half-wide scsi adapter.  The two busses are subsequently
 *       known as /dev/vscsi# and /dev/vscsi#+n.
 *
 *	Search CuDv to see if two vscsi's already exist.
 *   	   Loop through them to see if 
 *              -  parent names match this adapter's logical name
 *              -  CuDv locations are set (internal or external)
 *    	If the devices need to be defined,
 *          -  call routine that creates the device, using
 *    	       the -c flag to insert the connection value
 *    	    -  set the CuDv.location attribute as follows
 *    	       0 for internal or 1 for external
 *          -  Set the parent name
 *   	Write the name to standard out.  The devices must be made available
 *   	by their own cfg method.
 *
 *
 * EXECUTION ENVIRONMENT:
 *      This function operates as a device dependent subroutine
 *      called by the generic configure method for all devices.
 *
 * INPUTS : logical_name, odm_objects, ipl_phase
 *
 * RETURNS : Returns 0 when successful.
 *           E_ODMOPEN couldn't open object in ODM
 *	     E_ODMGET error accessing ODM
 *           E_ODMUPDATE couldn't update ODM
 *           E_ODMRUNMETHOD odm_run_method failed
 *
 */
int
define_children (char *lname,               /* logical name of device  */
        	 ndd_cfg_odm_t *odm_info,   /* pointers to ODM objects */
		 int ipl_phase)             /* which phase of ipl      */
{
    struct CuDv   CuDv_child;       /* Customized Device object            */
    struct PdDv   PdDv_child;       /* Predefined Device object            */
    char          sstr[256];	    /* ODM search strings                  */
    int           rc;	            /* return code                         */
    struct Class *cusdev;           /* customized devices                  */
    int           connection;       /* 0 - internal, 1 - external          */
    char	 *dname;            /* pointer to returned device name     */
    
    /* Get CuDv object class.    */
    cusdev = odm_info->cusdev;

    for (connection = 0; connection < 2; connection++)
    {
	/* 
	 * Get the CuDv for devices with parent lname and
	 * current connection.
	 */
	sprintf (sstr, "parent=%s AND connwhere=%1d", lname, connection);
	DEBUG_1 ("cfgascsidc: Retrieving CuDv for %s\n", sstr);
	
	rc = (int) odm_get_first (cusdev, sstr, (void *)&CuDv_child);
	if (rc == -1) 
	{
	    DEBUG_1 ("cfgascsidc: odm_get_first fail on %s\n", sstr);
	    return (E_ODMGET);
	}
	if (rc == 0) 
	{
	    /*
	     * The child needs to be defined.
	     */
	    sprintf (sstr, "uniquetype=%s", CHILD_U_TYPE);
	    rc = (int)odm_get_first (PdDv_CLASS, sstr, (void *)&PdDv_child);
	    if (rc == -1)
	    {
		DEBUG_1("cfgascsidc: Failed to get PdDv for %s\n", sstr);
		return (E_ODMGET);
	    }
	    else if (rc == 0)
	    {
		DEBUG_1("cfgascsidc: Failed to find PdDv entry for %s\n", sstr);
		return (E_NOPdDv);
	    }
	    else
	    {
		/*
		 * The PdDv was found successfully.  Now run the
		 * define method for this object.
		 */
		sprintf (sstr, "-c %s -s %s -t %s -p %s -w %1d",
			 PdDv_child.class, PdDv_child.subclass,
			 PdDv_child.type, lname, connection);
		DEBUG_2 ("run method:%s %s\n", PdDv_child.Define, sstr);
		if (odm_run_method (PdDv_child.Define, sstr, &dname, NULL))
		{
		    DEBUG_1 ("error running %s\n", PdDv_child.Define);
		    return (E_ODMRUNMETHOD);
		}
		printf ("%s", dname);
	    }
	}
	else
	{
	    /*
	     * This device is defined already.
	     */
	    if (CuDv_child.chgstatus != DONT_CARE)
	    {
		CuDv_child.chgstatus = SAME;
		/* 
		 * Now change the object in the CuDv object class.
		 */
		if (odm_change_obj (cusdev, &CuDv_child) == -1)
		{
		    DEBUG_1 ("cfgascsidc: odm_change_obj failed for %s\n",
			     CuDv_child.name);
		    return (E_ODMUPDATE);
		}
	    } 
	    printf ("%s ", CuDv_child.name);
	}
    }
    return (0);
}
