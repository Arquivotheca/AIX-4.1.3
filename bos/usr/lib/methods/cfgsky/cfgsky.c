static char sccsid[] = "@(#)88  1.10  src/bos/usr/lib/methods/cfgsky/cfgsky.c, sysxdispsky, bos411, 9428A410j 2/28/93 16:59:41";
/*
 *
 *   COMPONENT_NAME: (sysxdispsky)  Skyway configuration method
 *
 *  FUNCTIONS: build_dds
 *		define_children
 *		download_microcode
 *		generate_minor
 *		make_special_files
 *		query_vpd
 *
 *  ORIGINS: 27
 *
 *  (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *  All Rights Reserved
 *  Licensed Materials - Property of IBM
 *  US Government Users Restricted Rights - Use, duplication or
 *  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */




/*
 * Include files needed for this module follow
 */
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/sysconfig.h>
#include <sys/device.h>

/*
 * odmi interface include files
 */
#include <sys/cfgodm.h>
#include <cf.h>
#include "cfgdebug.h"

/* 
 * Local include for skyway dds structure 
 */
#include "ddsent.h"

extern char *malloc();
extern int *genminor();


struct ddsent         *dds;      /* pointer to dds structure to buiold */

/*
 * NAME: build_dds
 *
 * FUNCTION:
 *   build_dds will allocate memory for the dds structure, reporting any
 *   errors, then open the Customized Attribute Class to get the attribute
 *   objects needed for filling the dds structure.
 *
 * EXECUTION ENVIRONMENT:
 *   This routine executes under the device configuration process and
 *   is called from the device independent configuration routine.
 *
 * RETURNS:
 *   The return vales are
 *     0 - for success
 *     positive return code for failure
 */
build_dds(lname, dds_out, size)
char *lname;                     /* logical name of device */
char **dds_out;                  /* pointer to dds structure for return */
int  *size;                      /* pointer to dds structure size */
{
int		rc;		/* return status */
int		i;		/* loop control variable*/
char		crit[80];	/* search criteria string */
ulong		value;		/* temp variable to get value into */
char		temp[256];	/* temporary string variables */
struct CuAt	*get_attrval(); /* routine to get attribute value */
struct CuAt	*cusatt;	/* customized attribute ptr and object */

	/*
	 * Obtain size of device specific dds structure
	 */
	*size = sizeof(struct ddsent);

	/*
	 * allocate the space required for the dds structure
	 */
	if( (dds = (struct ddsent *) malloc(*size)) == NULL )
	{
		DEBUG_0("build_dds : malloc failed for dds structure");
		return(E_MALLOC);
	}

	dds->reserved1 = 0;  /* does nothing at this time */

	/*
	 * Get the connection location for this device from the customized
	 * device data.
	 */
	if((rc =  get_conn(lname, &value)) != E_OK )
		return(rc);
	dds->slot_number = value;

	/*
	 * Get the proper dds attribute values from the Customized 
	 * Attributes class
	 */
	if( get_attrval(lname, "bus_addr_start", (char *)NULL, &value, &rc)
			== NULL )
		return(rc);
	dds->io_bus_addr_start = value;

	if( get_attrval(lname, "bus_mem_start", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->io_bus_mem_start = value;

	if( get_attrval(lname, "vram_start", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->vram_start = value;

	if( get_attrval(lname, "dma1_start", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->dma_range1_start = value;

	if( get_attrval(lname, "dma2_start", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->dma_range2_start = value;

	if( get_attrval(lname, "dma3_start", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->dma_range3_start = value;

	if( get_attrval(lname, "dma4_start", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->dma_range4_start = value;

	if( get_attrval(lname, "dma_channel", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->dma_channel = value;

	if( get_attrval(lname, "int_level", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->int_level = value;

	if( get_attrval(lname, "int_priority", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->int_priority = value;

	if( get_attrval(lname, "scrn_width_mm", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->screen_width_mm = value;

	if( get_attrval(lname, "scrn_height_mm", (char *)NULL, &value, &rc)
			== NULL )
		return(rc);
	dds->screen_height_mm = value;

	if((cusatt = get_attrval(lname, "display_id", (char *)NULL, &value, &rc))
							 == NULL )
		return(rc);
	dds->display_id = value;
	if((rc = get_instance(lname, &value)) != E_OK )
		return(rc);
	dds->display_id |= value;
	sprintf(cusatt->value, "0x%08x", dds->display_id);
	if (putattr(cusatt) < 0)
	{
		DEBUG_0("build_dds: putattr failed\n");
		return(E_ODMUPDATE);
	}

	/*
	 * Read in the color table.
	 */
	for( i=0; i<16; i++)
	{
		sprintf(temp, "ksr_color%d", i+1);
		if( get_attrval(lname, temp, (char *)NULL, &value, &rc) == NULL )
			return(rc);
		dds->ksr_color_table[i] = value;
	}

	if( get_attrval(lname, "belongs_to", temp, &value, &rc) == NULL )
		return(rc);
	else
	{
		/*
		 * Build a customized dependency object for this display 
		 * and the hft it belongs to.
		 */
		if( (rc = build_depend(temp, lname)) < 0 )
		{
			DEBUG_2("Couldn't add dependency for %s -> %s", 
				temp, lname);
			return(rc);
		}
	}

	if( get_attrval(lname, "busmask", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->busmask = value;

	/*
	  Copy logical device name to dds for error logging by device.
	*/
	strcpy(dds->component, lname);

	*dds_out = (char *)dds;

	return(E_OK);
}


/*
 * NAME: generate_minor
 *
 * FUNCTION: Device dependent routine for generating the device minor number
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is called from the device independent configuration
 *  method.
 *
 * RETURNS:
 *   minor number success
 *   positive return code on failure
 */
int
generate_minor(lname, majno, minorno)
char  *lname;     /* logical device name */
long   majno;      /* device major number */
long	*minorno;  /* device minor number */
{
	long *minorptr;

	minorptr = genminor(lname, majno, -1, 1, 1, 1);

	if( minorptr == (long *)NULL )
		 return(E_MINORNO);

	*minorno = *minorptr;
	return(E_OK);
}


/*
 * NAME: make_special_files
 *
 * FUNCTION: Device dependant routine creating the devices special files
 *   in /dev
 *
 * EXECUTION ENVIRONMENT:
 *   This routine executes under the device configuration process and
 *   is called from the device independent configuration routine.
 *
 * RETURNS:
 *   0 - success
 */
make_special_files(lname, devno)
char  *lname;     /* logical device name */
dev_t devno;      /* major/minor number */
{
	return(E_OK);
}




/*
 * NAME: download_microcode
 * 
 * FUNCTION: Device dependant routine for downloading microcode
 *
 * EXECUTION ENVIRONMENT:
 *   This routine executes under the device configuration process and
 *   is called from the device independent configuration routine.
 *
 * NOTES: This routine will always return success for this device instance
 *
 * RETURNS:
 *   0 - success
 */
int
download_microcode(lname)
char  *lname;     /* logical device name */
{
	return(E_OK);
}



/*
 * NAME: query_vpd
 * 
 * FUNCTION: Device dependent routine for obtaining VPD data for a device
 *                                           
 * EXECUTION ENVIRONMENT:
 *   This routine executes under the device configuration process and
 *   is called from the device independent configuration routine.
 *
 * RETURNS: 
 *   0 - success
 *   positive return code for failure
 */
int
query_vpd(cusobj, kmid, devno, vpd)
struct CuDv	*cusobj;    /* customized device object */
mid_t		kmid;       /* driver module id */
dev_t 		devno;      /* major/minor number */
char 		*vpd;		/* vpd passed back to generic piece */
{
char		vpd_data[256];	/* storage for vpd data */
struct cfg_dd	cfgdata;	/* sysconfig call structure */
int		rc;		/* return code */
int		i;		/* loop variable */

	cfgdata.kmid = kmid;
	cfgdata.devno = devno;
	cfgdata.cmd = CFG_QVPD;
	cfgdata.ddsptr = (char *)vpd_data;
	cfgdata.ddslen = 256;

	if( (rc = sysconfig( SYS_CFGDD, &cfgdata, sizeof(struct cfg_dd) )) 
		== -1 )
	{
		DEBUG_0("query_vpd: sysconfig failed\n");
		return(E_VPD);
	}
	else
		for(i=0; i<256; i++)
			if( vpd_data[i] == '\0' )
				vpd[i] = ' ';
			else
				vpd[i] = vpd_data[i];

	return(E_OK);
}




/*
 * NAME: define_children
 *                                                                    
 * FUNCTION: Routine for detecting and managing children of a logical device
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *   This routine executes under the device configuration process and
 *   is called from the device independent configuration routine.
 *                                                                   
 * NOTES: This routine will always return success for this device instance
 *
 * RETURNS: 
 *   0 - success
 */
int
define_children(lname, phase)
char  *lname;     /* logical device name */
int   phase;      /* phase of ipl : 0=RUN, 1=PHASE_1, 2=PHASE_2 */
{
	return(E_OK);
}
