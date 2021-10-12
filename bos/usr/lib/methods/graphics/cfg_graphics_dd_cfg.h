/* @(#)99	1.3  src/bos/usr/lib/methods/graphics/cfg_graphics_dd_cfg.h, dispcfg, bos411, 9428A410j 7/5/94 11:44:55 */

/*
 *   COMPONENT_NAME: DISPCFG
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef	_H_CFG_GRAPHICS_DD_CFG
#define _H_CFG_GRAPHICS_DD_CFG


/******************************************************************************

 PURPOSE: Defines the interfaces to the device dependent full-function
	  config methods.
 
 GENERAL INTERFACE RULES: 
  
   EXECUTION ENVIRONMENT:
     These routines execute under the device configuration process and
     are called from the device independent configuration routine.
  
   RETURN CODES:
     The return vales are defined in cf.h:
       0 - success.  
       positive return code for failure
  
 *************************************************************************/



/*-------------------------------------------------------------------------
 | NAME: generate_minor
 |
 | FUNCTION: Device dependent routine for generating the device minor number
 |
 | INPUT PARMS:
 |   The (pointer to the) device logical name.
 |   The device's major number.
 |
 | OUTPUT PARMS:
 |   The (pointer to the) device minor number.
 |
 -------------------------------------------------------------------------*/

extern int generate_minor(
   		    char *	lname,      	/* logical device name */
   		    long   	majno,      	/* device major number */
   		    long *	minorno) ;  	/* device minor number */



/*-----------------------------------------------------------------------
 | NAME: build_dds
 |
 | FUNCTION:
 |   build_dds will allocate memory for the dds structure, reporting any
 |   errors, then open the Customized Attribute Class to get the attribute
 |   objects needed to initialize the dds structure.
 |
 | INPUT PARMS:
 |   The (pointer to the) device logical name.
 |
 | OUTPUT PARMS:
 |   The (pointer to the pointer of the) device dependent structure.
 |   The (pointer to the) device dependent structure length.
 |
 -----------------------------------------------------------------------*/

extern int build_dds(
   	    char *	lname,              /* ptr to logical name of device */
   	    char **	dds_out,            /* ptr to dds structure return */
   	    int  *	size) ;             /* ptr dds structure size       */



/* -------------------------------------------------------------------------
 |
 | NAME: query_vpd
 |
 | FUNCTION: Device dependent routine for obtaining VPD data for a device
 |
 |
 | INPUT PARMS:
 |   The (pointer to the) Customized device object
 |   The driver driver module ID.
 |   The device's major/minor number.
 |
 | OUTPUT PARMS:
 |   The (pointer to the) returned VPD data.
 |
 ------------------------------------------------------------------------- */

extern int query_vpd(
   	  struct CuDv  *	cusobj,		/* customized device object */
   	  mid_t                	kmid,	       	/* driver module id */
     	  dev_t                	devno,      	/* major/minor number */
   	  char *		vpd) ;	 	/* returned vpd */


/* -------------------------------------------------------------------------
 |
 | NAME: download_microcode
 |
 | FUNCTION: Device dependant routine for downloading microcode
 |
 | INPUT PARMS:
 |   The (pointer to the) device logical name.
 |
 | OUTPUT PARMS:
 |   none 
 |
 ------------------------------------------------------------------------- */

extern int download_microcode(
   		    char *	lname) ;     /* logical device name */



/* -------------------------------------------------------------------------
 |
 | NAME: make_special_files
 |
 | FUNCTION: Device dependant routine creating the devices special files
 |   		in /dev.  Upon successful completion, /dev/lname has been 
 |		created, where lname is the logical device name.
 |
 | INPUT PARMS:
 |   The (pointer to the) device logical name.
 |   The device's major/minor number.
 |
 | OUTPUT PARMS:
 |   none
 |
 ------------------------------------------------------------------------- */

extern int make_special_files(
        		char  *lname,     	/* logical device name */
        		dev_t devno) ;      	/* major/minor number */


#endif
