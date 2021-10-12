static char sccsid[] = "@(#)99  1.4.1.2  src/bos/usr/lib/methods/cfgsga/cfgsga.c, sgadd, bos411, 9428A410j 11/3/93 13:44:18";
/*
 * COMPONENT_NAME: (SGADD) Salmon Graphics Adapter Device Driver
 *
 * FUNCTIONS: build_dds, generate_minor, download_microcode,
 *            query_vpd, get_nbr_simms, mdd_get
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
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

/* Required for IPL-control block: */
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/devinfo.h>
#include <sys/mdio.h>
#include <sys/iplcb.h>

/* 
 * Local include for sga dds structure
 */
#include <sgadds.h>

extern char *malloc();
extern int *genminor();

struct fbdds      *dds;      /* pointer to dds structure to build */

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
	DEBUG_0("\nIn CFGSGA ++++++++debug++++++++++++++++++++++++\n\n\n");
	*size = sizeof(struct fbdds);

	/*
	 * allocate the space required for the dds structure
	 */
	if( (dds = (struct fbdds *) malloc(*size)) == NULL )
	{
		DEBUG_0("build_dds : malloc failed for dds structure");
		return(E_MALLOC);
	}

	dds->reserved1 = 0;  /* does nothing at this time */

	/*
	 * Get the connection location for this device from the customized
	 * device data.
	 */
/* slot_number not being used currently 
	if((rc =  get_conn(lname, &value)) != E_OK )
		return(rc);
	dds->slot_number = value;
*/

	/*
	 * Get the proper dds attribute values from the Customized 
	 * Attributes class
	 */

	/************************************************
	***    Bert remove PdAt for these in sga.add  ***
	***    to avoid confusing the configerator    ***
	***    with microchannel attributes (buid20)  ***
	***    with a non-microchannel device (buid40)***
	*************************************************
	if( get_attrval(lname, "vram_start", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->vram_start = value;

	if( get_attrval(lname, "int_level", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->int_level = value;

	if( get_attrval(lname, "int_priority", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->int_priority = value;
	*************************************************
	***    Bert remove PdAt for these in sga.add  ***
	***    to avoid confusing the configerator    ***
	***    with microchannel attributes (buid20)  ***
	***    with a non-microchannel device (buid40)***
	************************************************/

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

	/* update odm with current number of simms */
	if((rc = get_nbr_simms(lname)) != E_OK)
		return(rc);

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
		 * and the lft it belongs to.
		 */
		if( (rc = build_depend(temp, lname)) < 0 )
		{
			DEBUG_2("Couldn't add dependency for %s -> %s", 
				temp, lname);
			return(rc);
		}
	}

/* busmask not being used currently 
	if( get_attrval(lname, "busmask", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->busmask = value;
*/

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
/* vpd info is not treated as strings so this is not necessary 
			if( vpd_data[i] == '\0' )
				vpd[i] = ' ';
			else
*/
				vpd[i] = vpd_data[i];

	return(E_OK);
}



/*
 * NAME: get_nbr_simms
 *                                                                    
 * FUNCTION: Routine for detecting number of SIMMS installed on the SGA
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
#define  SIMMSx10 1310720	/* bytes of memory in 10 SIMM configuration */
#define  SIMMSx6   655360	/* bytes of memory in 6 SIMM configuration */
#define  SIMMSx2   262144 	/* bytes of memory in 2 SIMM configuration */

int
get_nbr_simms(lname)
char  *lname;     /* logical device name */
{

SGA_DATA        iplcb_sga_data; /* SGA Post results section */
IPL_DIRECTORY   iplcb_dir;      /* IPL control block directory */
int 		rc;		/* return variable */
int		cur_size;	/* number of simms read from the IPLCB */
ulong		curval,preval;  /* variable to get value into */
struct CuAt	*cusatt;	/* customized attribute ptr and object */
int mdd_get();

        /* Read in the IPL Control Block directory */
        if( rc = mdd_get(&iplcb_dir,128,sizeof(iplcb_dir),MIOIPLCB) )
                return(rc);
        
        /* Get SGA post results section from IPL CB */
        if( rc = mdd_get(&iplcb_sga_data, iplcb_dir.sga_post_results_offset,
	                sizeof(iplcb_sga_data),MIOIPLCB) )
                return(rc);
	/* Convert number of bytes into number of SIMMS */
	switch(iplcb_sga_data.mem_size)
	{
	  case SIMMSx10: 
		cur_size = 10; break;
	  case SIMMSx2: 
		cur_size = 2; break;
	  case SIMMSx6: 
	  default:
		cur_size = 6;
	}

	/* Get the current number of simms from the ODM */
	if((cusatt = get_attrval(lname,"curr_nbr_sims",
			(char *)NULL,&curval,&rc)) == NULL)
		return(rc);

	/* Update the dds structure */
	dds->curr_nbr_sims = cur_size;
	dds->prev_nbr_sims = curval;

	/* Update the ODM if the number of SIMMS changed */
	if ( cur_size != curval )
	{
		/* Update the "curr_nbr_sims" attribute first */
		sprintf(cusatt->value, "%d", cur_size);
		if (putattr(cusatt) < 0)
		{
			DEBUG_0("build_dds: putattr failed\n");
			return(E_ODMUPDATE);
		}
		/* Update "prev_nbr_sims" attribute if needed 
		   with the old "curr_nbr_sims" attribute or if
		   first boot use the current size value.  
		*/
		if((cusatt = get_attrval(lname,"prev_nbr_sims",(char *)NULL,
				&preval,&rc)) == NULL)
			return(rc);
		if ( cur_size > preval )
		{
			sprintf(cusatt->value, "%d", cur_size);

			if (putattr(cusatt) < 0)
			{
				DEBUG_0("build_dds: putattr failed\n");
				return(E_ODMUPDATE);
			}
		}
	}
	return(E_OK);
}



/* NAME: mdd_get
 *
 * FUNCTION: Reads "num_bytes" bytes from nvram, IPL control block, or the
 *           iocc.  Bytes are read from the address "address" and stored at
 *           address "dest".
 *
 * RETURNS:  error code.  0 means no error.
 */

int
mdd_get(dest, address, num_bytes, ioctl_type)
char    *dest;
int     address;
int     num_bytes;
int     ioctl_type;
{
        int             fd;             /* file descriptor */
        MACH_DD_IO      mdd;


        if ((fd = open("/dev/nvram",0)) < 0) {
                DEBUG_0("Unable to open /dev/nvram")
                return(E_DEVACCESS);
        }

        mdd.md_addr = address;
        mdd.md_data = dest;
        mdd.md_size = num_bytes;
        mdd.md_incr = MV_BYTE;

        DEBUG_0("Calling mdd ioctl\n")
        if (ioctl(fd,ioctl_type,&mdd)) {
                DEBUG_0("Error reading IPL-Ctrl block")
                return(E_DEVACCESS);
        }

        close(fd);
        return(0);
}

