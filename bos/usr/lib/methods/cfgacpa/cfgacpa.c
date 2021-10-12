/* @(#)06 1.7.1.1 src/bos/usr/lib/methods/cfgacpa/cfgacpa.c, sysxacpa, bos411, 9428A410j 92/03/03 18:45:25 */

/*
 * COMPONENT_NAME: SYSXACPA     Multimedia Audio Capture and Playback Adapter
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/************************************************************************/
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      The module contains functions that perform the device dependent */
/*      M-ACPA/A configuration work.                                    */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*                                                                      */
/*      This module is for the RISC System/6000 only.                   */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      rc = build_dds(lname, dds_out, size)                            */
/*      char *lname;             logical name of device                 */
/*      char **dds_out;          pointer to dds structure for return    */
/*      int  *size;              pointer to dds structure size          */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*      See above                                                       */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      N/A                                                             */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/*  RETURN VALUES:                                                      */
/*                                                                      */
/*      0 if successful                                                 */
/*      -1 if unsuccessful                                              */
/*                                                                      */
/*****  System Include Files  *******************************************/

#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/watchdog.h>

#include <sys/cfgodm.h>
#include <cf.h>

/*****  Audio Include Files  ********************************************/

#include "cfgdebug.h"
#include <sys/audio.h>
#include <sys/acpa.h>
#include "oldacpa.h"

/*****  Global Environment Structure  ***********************************/

struct acpa_dds         *dds;      /* pointer to dds structure to build */

/*****  Main Entry Point  ***********************************************/

extern int *genminor();

/*
 * Set permissions for special file
 */
#define FTYPE   S_IFCHR
#define FPERM   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
#define MODE    FTYPE | FPERM

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
 *    -1 - for failure
 */
build_dds(lname, dds_out, size)
char *lname;                     /* logical name of device */
char **dds_out;                  /* pointer to dds structure for return */
int  *size;                      /* pointer to dds structure size */
{
	int rc;                         /* return status */
	int i;                          /* loop control variable*/
	ulong value;                    /* temp variable to get value into */
	struct CuAt *get_attrval();     /* routine to get attribute value */
	char strval[256];               /* temporary strings */
	char s[256];                    /* temporary strings */
	struct CuDv cusobj;             /* customized device object storage */
	struct PdDv preobj;             /* customized device object storage */
	ushort devid;                   /* the device's id */
	int len;                        /* length of a string */
	char mcode[6][5] =              /* the function component of the */
	{
		"511p", "511r", "pcmp", "pcmr", "22p", "22r"
	} ;                             /* microcode names */
	int fd;                         /* file descriptor */

	/* Obtain size of device specific dds structure */
	*size = sizeof(struct acpa_dds);

	/* allocate the space required for the dds structure */
	if( (dds = (struct acpa_dds *) malloc(*size)) == NULL )
	{
                DEBUG_0("build_dds : malloc failed for dds structure");
                return(E_MALLOC);
	}

	/* The next section of code gets the adapter's id from ODM to */
	/* use it in constructing the names of the microcode modules. */
	/* The module names are of the format XXXXYYYY.LL.VV, where */
	/* XXXX is the device id, YYYY is optional, and used to indicate */
	/* the microcode functionality, LL is the level (which is always */
	/* 00 because there is no VPD on the adapter, and VV is the */
	/* version. */

	/* Get predefined device object for this logical name. */
	sprintf( strval, "name = '%s'", lname );
	rc = (int) odm_get_first( CuDv_CLASS, strval, &cusobj );
	if ( rc == 0 )
	{
		/* No CuDv object with this name */
		DEBUG_1("cfgdevice: failed to find CuDv object for %s\n", lname);
		err_exit(E_NOCuDv);
	}
	else if ( rc == -1 )
	{
		/* ODM failure */
		DEBUG_0("cfgdevice: ODM failure getting CuDv object");
		err_exit(E_ODMGET);
	}
	/* get predefined device object for this logical name */
	sprintf(strval, "uniquetype = '%s'", cusobj.PdDvLn_Lvalue);
	rc = (int) odm_get_first( PdDv_CLASS, strval, &preobj );
	if (rc==0) {
		/* No PdDv object for this device */
		DEBUG_0("cfgdevice: failed to find PdDv object for this device\n");
		err_exit(E_NOPdDv);
	}
	else if (rc==-1) {
		/* ODM failure */
		DEBUG_0("cfgdevice: ODM failure getting PdDv object");
		err_exit(E_ODMGET);
	}

	/*
	 * Get the proper dds attribute values from the Customized
	 * Attributes class
	 */
	if( get_attrval(lname, "bus_addr", (char *)NULL, &value,&rc) == NULL )
	    return(rc);
	dds->bus_addr = value;

	if( get_attrval(lname, "int_level", (char *)NULL, &value,&rc) == NULL )
		return(rc);
	dds->int_level = value;

	if( get_attrval(lname, "int_prior", (char *)NULL, &value,&rc) == NULL )
		return(rc);
	dds->priority = value;

	if( get_attrval( lname, "macpa_5r", (char *) NULL, &value, &rc ) == NULL )
		return( rc );
	dds->MACPA_5r_secs = (unsigned int) value;

	if( get_attrval( lname, "macpa_8r", (char *) NULL, &value, &rc ) == NULL )
		return( rc );
	dds->MACPA_8r_secs = (unsigned int) value;

	if( get_attrval( lname, "macpa_11r", (char *) NULL, &value, &rc ) == NULL )
		return( rc );
	dds->MACPA_11r_secs = (unsigned int) value;

	if( get_attrval( lname, "macpa_22r", (char *) NULL, &value, &rc ) == NULL )
		return( rc );
	dds->MACPA_22r_secs = (unsigned int) value;

	if( get_attrval( lname, "macpa_44r", (char *) NULL, &value, &rc ) == NULL )
		return( rc );
	dds->MACPA_44r_secs = (unsigned int) value;

	if( get_attrval( lname, "macpa_5w", (char *) NULL, &value, &rc ) == NULL )
		return( rc );
	dds->MACPA_5w_secs = (unsigned int) value;

	if( get_attrval( lname, "macpa_8w", (char *) NULL, &value, &rc ) == NULL )
		return( rc );
	dds->MACPA_8w_secs = (unsigned int) value;

	if( get_attrval( lname, "macpa_11w", (char *) NULL, &value, &rc ) == NULL )
		return( rc );
	dds->MACPA_11w_secs = (unsigned int) value;

	if( get_attrval( lname, "macpa_22w", (char *) NULL, &value, &rc ) == NULL )
		return( rc );
	dds->MACPA_22w_secs = (unsigned int) value;

	if( get_attrval( lname, "macpa_44w", (char *) NULL, &value, &rc ) == NULL )
		return( rc );
	dds->MACPA_44w_secs = (unsigned int) value;

	if( get_attrval( lname, "macpa_req", (char *) NULL, &value, &rc ) == NULL )
		return( rc );
	dds->MACPA_request_buf = (unsigned int) value;

	/* TO GET PRINTFS, YOU MUST USE DBG_ MACROS.  THE OUTPUT GOES */
	/* INTO /TMP/PHASE2.OUT */
	/* NOTE -- ANYTHING THAT GOES TO STDOUT WILL BE RUN BY THE */
	/* CONFIGURATOR! */

	/* Now get the device id to use in constructing microcode names. */
	devid = (ushort) strtol( preobj.devid, (char **) NULL, 0 );

	/* Create the path and see if the file exists. */
	strcpy( strval, "/etc/microcode/" );
	strcpy( s, preobj.devid );
	/* Now remove the 0x prefix from the device id */
	len = strlen( s );
	for ( i=0; i<len; i++ )
	    s[i] = s[i+2];
	s[i] = 0;
	strcat( strval, s );
	len = strlen( strval );
	/* There are six microcode modules (see variable mcode). */
	/* The first version number is alway 01.  Look for the highest */
	/* number available, because that is the latest version. */
	for ( rc=0,i=99; rc<6; rc++,i=99 )
	{
		/* only 99 version levels are available */
		while ( i > 0 )
		{
		    strval[len] = 0;    /* start at the right place */
		    strcat( strval, mcode[rc] );
		    /* The level number is always 00 */
		    strcat( strval, ".00." );
		    sprintf( s, "%02x", i );
		    strcat( strval, s );
		    if ( ( fd = open( strval, O_RDONLY ) ) != -1 )
		    {
		      /* The latest version of microcode has been found */
		      close( fd );
		      break;
		    }
		    else
		      i--;
		}
		if ( i > 0 )
		{
		    /* a file was found; save its name */
		    strval[len] = 0;
		    strcat( strval, mcode[rc] );
		    strcat( strval, ".00." );
		    sprintf( s, "%02x", i );
		    strcat( strval, s );
		    strcpy( &(dds->mcode[rc][0]), strval );
		}
		else
		    return( ENOENT );
	}

	if((rc =  get_conn(lname, &value)) != E_OK )
		return(rc);
	dds->slot_number = (int)value;

	/*
	  Copy logical device name to dds for error logging by device.
	*/
	*dds_out = (char *)dds;

	return(0);
}


/*
 * NAME: generate_minor
 *
 * FUNCTION: Device dependant routine for generating the device minor number
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
long    *minorno;  /* device minor number */
{
	long *minorptr;

	minorptr = genminor(lname, majno, -1, 1, 1, 1);

	if( minorptr == (long *) NULL )
		 return(E_MINORNO);

	*minorno = *minorptr;
	return(E_OK);
}


/*
 * NAME:
 *	make_special_files()
 * 
 * FUNCTION:
 *	This creates 1 special file with the same name as the logical
 *	device name.
 *
 * RETURNS:
 *		0 on Success, >0 Error code
 */
int 
make_special_files(lname,devno)
char	*lname;
dev_t	devno;
{
	long	create_flags;

	create_flags = ( S_IFMPX | S_IFCHR | S_IRUSR | S_IWUSR | \
                         S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
	return(mk_sp_file(devno,lname,create_flags));
}


/*
 * NAME: download_microcode
 * 
 * FUNCTION: Device dependant routine for downloading microcode
 *           This function is handled by the device driver
 *
 * EXECUTION ENVIRONMENT:
 *   This routine executes under the device configuration process and
 *   is called from the device independent configuration routine.
 *
 * NOTES: This routine will always return success for this device instance
 *
 * RETURNS:
 *   0 - success
 *  -1 - failure
 */
int
download_microcode(lname)
char  *lname;     /* logical device name */
{
	return(0);
}



/*
 * NAME: query_vpd
 * 
 * FUNCTION: Device dependant routine for obtaining VPD data for a device
 *                                           
 * EXECUTION ENVIRONMENT:
 *   This routine executes under the device configuration process and
 *   is called from the device independent configuration routine.
 *
 * NOTES: This routine will always return success for this device instance
 *
 * RETURNS: 
 *   0 - success
 *  -1 - failure
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

#ifdef MORE
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

	/**	Error("query_vpd: sysconfig failed\n"); **/
	else
		for(i=0; i<256; i++)
			if( vpd_data[i] == '\0' )
				vpd[i] = ' ';
			else
				vpd[i] = vpd_data[i];
#endif

	/* This needs to return 0; otherwise, the generic config would */
	/* stop. */
	return(0);
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
 *  -1 - failure
 */
int
define_children(lname, phase)
char  *lname;     /* logical device name */
int   phase;      /* phase of ipl : 0=RUN, 1=PHASE_1, 2=PHASE_2 */
{
	return(0);
}
