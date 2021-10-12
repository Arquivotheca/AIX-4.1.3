static char sccsid[] = "@(#)47 1.12 src/bos/usr/lib/methods/common/location.c, cfgmethods, bos41J, 9523C_all 6/6/95 14:29:56";
/*
 * COMPONENT_NAME: (CFGMETHODS) location.c - Build location code
 *
 * FUNCTIONS: location()
 *
 * ORIGINS: 27, 83
 *
 *  This module contains IBM CONFIDENTIAL code. -- (IBM
 *  Confidential Restricted when combined with the aggregated
 *  modules for this product)
 *                   SOURCE MATERIALS
 *  (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *  All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>

#include "cfgdebug.h"
/*
 * NAME: location
 * 
 * FUNCTION: This function constructs the location code for a device.
 * 
 * EXECUTION ENVIRONMENT:
 *
 *	This function is called by both the generic define method and
 *	the generic change method.
 *
 * RETURNS: Returns with 0 on completion.
 */
int
location (par_utype,par_loc,cusobj)
char *par_utype, *par_loc;
struct CuDv *cusobj;
{
	char    devcst[UNIQUESIZE],     /* space for class,subclass & type */
	        Parcst[UNIQUESIZE],     /* space for class,subclass & type */
		*devclass,		/* pionter to device class */
		*devsubc,		/* pointer to device subclass */
		*devtype,               /* pointer to device type */
	        *ParClass,		/* ptr to parent's class    */
	        *ParSubClass,           /* ptr to parent's subclass */
	        *ParType,		/* ptr to parent's types    */
		miscstr[32],		/* string space */
		*ptr1;			/* work pointer */
	int	connection;		/* connection as an integer */
	struct CuAt cusattpar, sys_cuattr;
	char sstring[256];
	int rc;

	DEBUG_0 ("Entering location(): Build location value\n")

       /*------------------------------------------------------------ 
	*  Break out the parts of the parent's unique type for easier
	*  reference later.
	*------------------------------------------------------------*/

	strcpy(Parcst,par_utype);
	ParClass   = strtok(Parcst,"/");
	ParSubClass= strtok((char *)0,"/");
	ParType    = strtok((char *)0,"/");

	DEBUG_1("location: par_utype = %s\n", par_utype) ;
	DEBUG_3("location: ParClass= %s   ParSubClass= %s   ParType= %s\n",
		ParClass, ParSubClass, ParType) ;
       /*------------------------------------------------------- 
	*  Break out the parts of the device's predefined unique 
	*  type for easier reference later.
	*-------------------------------------------------------*/

	strcpy(devcst,cusobj->PdDvLn_Lvalue);
	devclass=strtok(devcst,"/");
	devsubc=strtok((char *)0,"/");
	devtype=strtok((char *)0,"/");

	connection=atoi(&cusobj->connwhere[strcspn(cusobj->connwhere,
					           "0123456789")]);
	DEBUG_1 ("location: connection = %s\n",cusobj->connwhere)
	DEBUG_1 ("location: parent location = %s\n",par_loc)

       /*--------------------------------------------------------------
	* Now "case" through the various special cases so that the
	* location code can be set correctly.  If no special case 
	* exists, the default is to use the connwhere passed in the 
	* CuDv.
	* 
	* Order is somewhat important.  You must be careful that a
	* generic specification high in the list does not catch a 
	* more specific specification that appears later.
	* 
	* The current order attempts to follow the heirarchy of the
	* device trees (planar devices first, adapters next (including 
	* subset adapters (ie. sio)), and finally end devices 
	* (ie. scsi and tty type devices). 
	*--------------------------------------------------------------*/
        
	if(!strcmp(cusobj->PdDvLn_Lvalue,"bus/sys/mca")) 
        {/* Device is microchannel bus */
	    sprintf(cusobj->location,"00-%c0", 
		    cusobj->connwhere[strlen(cusobj->connwhere) - 1]);
	} 
        else if(!strcmp(devclass,"drawer")) 
        {
	    /* Device is drawer */
	    strcpy(cusobj->location, cusobj->connwhere) ;
	} 
       /*-------------------------------------------------------
	*  Now looking for microchannel attached devices
	*-------------------------------------------------------*/
	
	else if (!strcmp(devsubc,  "mca"))
	{
	   /*--------------------------------------------------- 
	    * Device is a Microchannel device.  
	    * 
	    * NOTE: adapter is not used in the check so that
	    *       direct attached drives are handled correctly
	    *       (they are directly connected to the MCA bus).
	    *       
	    * Check to see if it is directly attached to the bus
	    * or in an expansion unit (these adapters still
	    * appear as  microchannel adapters).  If the last digit
	    * of the parent's location code is zero (0), then
	    * the adapter is directly connected to the bus.
	    * Otherwise it is connected to a bus extender that
	    * is connected to the bus.
	    *---------------------------------------------------*/

	    if (par_loc[strlen(par_loc) - 1] == '0')
	    { 
	       /* 
		*  Device is directly connected to the bus.
		*  Put the slot the device is in into the 
		*  last digit of the location code.
		*/
		
                if (!strcmp(cusobj->PdDvLn_Lvalue, "adapter/mca/sio_3")) {
                /*
                 *  Update the connwhere and location code of the sio
                 *  adapter for multi-processor hardware
                 */
                        sprintf(cusobj->location, "%4.4s0", par_loc);
                }
	    	else {
		    sprintf(cusobj->location, "%4.4s%1d", par_loc, connection);
		
	    	}
	    }
	    else 
	    {
	       /* 
		*  Adapter is connected to a bus extender -
		*  currently the expansion unit only.
		*  The size of the location code must be 
		*  reduced to include the extra info on the
		*  end.
		*/
		
		sprintf(cusobj->location, "%s-%02d",
			par_loc + 3, connection) ;
	    }
	} /* end micro channel adapters */

        else if (!strcmp(devsubc, "sio"))
	{
	   /*--------------------------------------------------- 
	    *  An SIO attached adapter has been detected.
	    *  All of these adapters are special cases to
	    *  match markings on the chassis of the box.
	    *  
	    *  Note that the connwhere is used to determine
	    *  which adapter is which because these are more
	    *  generic and allows easy definition of release
	    *  1 hardware and release 2 hardware (the type
	    *  fields have changed for new hardware, but they
	    *  still connect to the same place).
	    *---------------------------------------------------*/

	    if (!strcmp(cusobj->connwhere, "fda"))       /*floppy dsk adapter*/
	    {
		sprintf(cusobj->location, "%s-0D", par_loc) ;
	    } 
	    else if (!strcmp(cusobj->connwhere, "ka"))   /* keyboard adapter */
            {
		sprintf(cusobj->location, "%s-0K", par_loc) ;
            }	
	    else if (!strcmp(cusobj->connwhere, "ma"))   /* mouse adapter    */
            {
		sprintf(cusobj->location, "%s-0M", par_loc) ;
            }	
	    else if (!strcmp(cusobj->connwhere, "ta"))   /* tablet adapter   */
            {
		sprintf(cusobj->location, "%s-0T", par_loc) ;
            }	
	    else if (!strcmp(cusobj->connwhere, "ppa"))  /* parallel adapter */
            {
		sprintf(cusobj->location, "%s-0P", par_loc) ;
            }	
	    else if (!strcmp(cusobj->connwhere, "s1a"))  /* serial 1 adapter */
            {
		sprintf(cusobj->location, "%s-S1", par_loc) ;
            }	
	    else if (!strcmp(cusobj->connwhere, "s2a"))  /* serial 2 adapter */
            {
		sprintf(cusobj->location, "%s-S2", par_loc) ;
            }	
	    else if (!strcmp(cusobj->connwhere, "s3a"))  /* serial 3 adapter */
            {
		sprintf(cusobj->location, "%s-S3", par_loc) ;
            }	
	    else if (!strcmp(cusobj->connwhere, "14"))   /* SCSI adapter     */
            {
		sprintf(cusobj->location, "%s-0S", par_loc) ;
            }	
	    else if (!strcmp(cusobj->connwhere, "15"))   /* ethernet adapter */
            {
		sprintf(cusobj->location, "%s-0E", par_loc) ;
            }
        } /* end SIO based adapters */

	else if(!strcmp(cusobj->PdDvLn_Lvalue,"adapter/slc/otp")) 
        {/* Device is Serial Optical Two-Port Card */
	    sprintf(cusobj->location,"00-A%c",connection+'A');
	} 
       /*---------------------------------------------------- 
        * Now check for end devices.
	*----------------------------------------------------*/
	
        else if (!strcmp(ParSubClass, "sio")) {
           /* -----------------------------------------------
	    *  Device is connected to an SIO adapter.
	    *  All of these location codes end in "-00"
	    *  except floppy and scsi which use their
	    *  connwhere.
	    *  
	    *  NOTE THAT THIS CHECK MUST PRECEDE THE
	    *  CHECK FOR SCSI!! 
	    *       This is required so that devices on 
	    *       the integrated SCSI will preserve
	    *       the location code field ID'ing the
	    *       integrated SCSI.
	    *------------------------------------------------*/
	   		
	    if (!strcmp(devsubc, "siofd"))  
	    {
		sprintf(cusobj->location, "%s-%02d", 
			par_loc, connection) ;
	    }
	    else if (!strcmp(devsubc, "scsi"))
	    {
		sprintf(cusobj->location, "%s-%s", 
			par_loc, cusobj->connwhere) ;
	    }
	    else if (!strcmp (devsubc, "scsi_scb"))
	    {
		sprintf(cusobj->location,"%c%c-%c%c-0%d",par_loc[0],par_loc[1], 
			par_loc[3], par_loc[4], connection); 
	    }
	    else
	    {
		sprintf(cusobj->location, "%s-00", par_loc) ;
	    }
	} /* end sio attached end devices */
	
	else if (!strcmp(ParSubClass, "isa_sio")) {
        	/*------------------------------------------------
	 	* Its an integrated ISA adapter
	 	*------------------------------------------------*/
		if (!strcmp(devsubc, "siofd")) {
			sprintf(cusobj->location, "%s-00-%02d", 
					par_loc, connection);
		}
		else {
			sprintf(cusobj->location, "%s-00-00", par_loc);
		}
	}

	else if (!strcmp(devsubc,"scsi"))                /* SCSI Device      */
	  {
		rc = (int)odm_get_first(CuAt_CLASS, "name=sys0 AND attribute=rds_facility",
				 &sys_cuattr);
		if (rc == -1) {
			DEBUG_0("ODM error getting rds_facility attribute\n")
			return(rc);
		} else if (rc != 0 && sys_cuattr.value[0]=='y') {
		  sprintf(sstring,"name='%s' and attribute=bus_loc", cusobj->parent);
		  rc = (int) odm_get_first(CuAt_CLASS, sstring, &cusattpar);
		  if (rc == -1) {
			/* ODM failure */
			DEBUG_0("update_location: ODM failure getting CuAt object\n");
			return(rc);
		  }
		  if (rc == 0) {
			/* no CuAt object found */
			DEBUG_0("update_location: bus_loc attribute not found\n");
			if (!strcmp(ParSubClass, "scsi_scb"))
				sprintf(cusobj->location,"%c%c-%c%c-%c%c-%s",
					'X', par_loc[1], par_loc[3],  par_loc[4],
					'0',  par_loc[7], cusobj->connwhere);
			else
				sprintf(cusobj->location,"%c%c-%c%c-%c%c-%s",
					'X', par_loc[1], par_loc[3],  par_loc[4],
					'0', '0', cusobj->connwhere);
		  }
		  else {
			if (!strcmp(ParSubClass, "scsi_scb")) {
				 sprintf(cusobj->location,"%c%c-%c%c-%c%c-%s",
					 cusattpar.value[0], par_loc[1],
					 par_loc[3],  par_loc[4],
					 cusattpar.value[2],  par_loc[7],
					 cusobj->connwhere);
			 	 DEBUG_1("location code value: %s\n", cusobj->location);
		  	}
		  	else {
				  sprintf(cusobj->location,"%c%c-%c%c-%c%c-%s",
					  cusattpar.value[0], par_loc[1],
					  par_loc[3],  par_loc[4],
					  cusattpar.value[2], '0',
					  cusobj->connwhere);
			 	 DEBUG_1("location code value: %s\n", cusobj->location);
		  	}
		  }
		}
		else {
		  /* Check to see if parent is a SCSI protocol device */
		  if (!strcmp(ParSubClass, "scsi_scb"))
			sprintf(cusobj->location, "%s-%s", 
					par_loc, cusobj->connwhere);
		  else
			sprintf(cusobj->location, "%s-00-%s", 
					par_loc, cusobj->connwhere);
		} 
	  }
	else if ((!strcmp(devsubc,"rs232")) ||		 /* serial tty dvc   */
		 (!strcmp(devsubc,"rs422")))
	{
		/* fixed for 128 port */
                if ( !strcmp(ParClass,"concentrator") ) {
                        sprintf(cusobj->location, "%s-%02d", par_loc, connection);
                } else {
	    sprintf(cusobj->location, "%s-%02d-%02d", par_loc,
		    (connection/16)+1, connection%16);
		} 
	}
        else if(!strcmp(cusobj->PdDvLn_Lvalue,"adapter/otp/op")) 
        {/* Device is Serial Optical Port */
	    if (!strcmp(par_loc,"00-AA"))
		sprintf(cusobj->location,"%s-%dA",par_loc,connection);
	    else
	       	sprintf(cusobj->location,"%s-%dB",par_loc,connection);
	} 

	else /* no special processing required */ 
        {
	    sprintf(cusobj->location,"%s-%02d",par_loc,connection);
	}
return(0);
}

