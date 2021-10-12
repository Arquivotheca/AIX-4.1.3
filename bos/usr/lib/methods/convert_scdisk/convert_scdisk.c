static char sccsid[] = "@(#)58  1.1.1.1  src/bos/usr/lib/methods/convert_scdisk/convert_scdisk.c, cfgmethods, bos411, 9428A410j 7/19/92 17:09:41";
/*
 * COMPONENT_NAME: (CFGMETH) convert_scdisk.c - Converts SCSI disks of
 *			     of a given uniquetype to a new unique type if
 *			     it exists.
 *
 * FUNCTIONS: main(),scsi_chg_dev_type(),get_vpd_model(),det_utype(),
 *	      err_exit().
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/* header files needed for compilation */
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/sysconfig.h>
#include <sys/sysmacros.h>
#include <sys/device.h>
#include <sys/errno.h>
#include <sys/scsi.h>
#include <sys/scdisk.h>

/* Local header files */
#include "cfgdebug.h"
#include "cfghscsi.h"

#define MODEL_NAME_SIZE 	18             /* Max length of Model Name  */
#define SCD_MAX_STRING_LEN      256            /* Maximum length of string  */


char default_uniquetype[SCD_MAX_STRING_LEN];   /* Global default uniquetype */

/*
 *  Function prototypes for this program:
 */

int scsi_chg_dev_type(char *);
int get_vpd_model(char *,struct CuDv );
int det_utype(char *,char *);
int search_odm(char *,char *);
void err_exit(int);



/* 
 *
 * NAME: main
 *                  
 * FUNCTION:  To be used when new predefines for SCSI disks have been
 *	      added.  This routine should be run immediately afterwards
 *	      to update an old uniquetype for devices into the
 *	      new uniquetype, which now is for the device.
 *   
 *       
 * MOTIVATION:  To allow selective enhancements to update the
 *              ODM database for new SCSI Disks.  This program was
 *              originally written with the intention of
 *		being general enough for all SCSI devices.
 *		However during implementation 1 restriction had 
 *		to be made which limits it to SCSI disks.  The
 *		limitation is in hardcoding the uniquetype for SCSI disks.
 *
 *
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is called by a process at the process level and it
 *      can page fault.
 *
 * 
 * INPUTS
 *
 *	argv[1] 	- Uniquetype that will be replaced if found
 *			  devices in PdAt.
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *      scsi_chg_dev_type	       err_exit
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *   
 *      odm_initialize			odm_lock        
 *	odm_terminate			odm_unlock
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:     
 *       
 *	0 - If successful.
 *     -1 - If failure.
 *     >0 - If failure, then will return ODM errors found in cf.h.
 *       
 */

main(int	argc,                 	/* Number of command line arguments */
     char	**argv,			/* Command line arguments	    */
     char	**envp)		        /* environment pointers		    */
{
	int 		lock;   	/* Database lock		  */
	int 		rc =0;  	/* return code			  */



	if (argc != 2) {
		/*
		 * Must be given a uniquetype in order to run.
		 */
		printf("\nusage: convert_scdisk uniquetype\n");
		exit(-1);
	}
		
  
	/* 
	 * Start up ODM.
	 */

	if (odm_initialize() == -1) {
		/* initialization failed */
		DEBUG_0("convert_scdisk: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}

	DEBUG_0 ("ODM initialized\n")

       /* 
	* lock the ODM database 
	*/
        if ((lock = odm_lock("/etc/objrepos/config_lock",0)) == -1) {
		DEBUG_0("convert_scdisk:odm_lock() failed\n")
		err_exit(E_ODMLOCK);
                        
        }
        
        DEBUG_0("ODM locked\n")
        
	strcpy(default_uniquetype,"disk/scsi/osdisk");


	/*
	 * Find all devices with this uniquetype and change
	 * their uniquetypes if a new uniquetype is found.
	 */
	       
        rc = scsi_chg_dev_type(argv[1]);


        if (odm_unlock(lock) == -1) {
                DEBUG_0("convert_scdisk: ODM unlocked failed\n")
                err_exit(E_ODMLOCK);
        }

	err_exit(rc);


}

/* 
 *
 * NAME: scsi_chg_dev_type
 *                  
 * FUNCTION:  Finds all SCSI devices of the given uniquetype and updates
 *	      their CuDv's uniquetype pointer if another uniquetype is found.
 *	      Before this routine makes such a change it verifies the
 *	      model name via SCSI inquiry and from CuVPD.
 *   
 *       
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.
 *
 * 
 *
 * CALLED BY:
 *      
 *	main
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *      get_vpd_model			det_utype
 *	
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *   
 *      odm_get_list			odm_get_obj
 *	odm_change_obj			sprintf
 *	strcpy				strcmp
 *	strncmp
 *
 * (RECOVERY OPERATION:)  If an error occurs, return error code.
 *
 *
 * RETURNS:     
 *       0  - Successful completion.
 *      >0  - If failure return ODM error codes.
 */
int scsi_chg_dev_type(char *uniquetype)
{
	char  sstr[SCD_MAX_STRING_LEN];          /* search string for ODM     */
	char  vpd_model[MODEL_NAME_SIZE];        /* Model name from CuVPD     */
	char  new_uniquetype[SCD_MAX_STRING_LEN];/* new uniquetype found      */
	struct CuDv  *cusobj_ptr,		 /* List of customized devices*/
						 /* that match the given      */
						 /* uniquetype                */
			  cusobj;        	 /* Customized device to be   */
						 /* changed		      */
	int         i,	                         /* general counter           */
		    rc;				 /* return code               */
	struct listinfo list;                    /* for using odm_get_list    */



	

	sprintf( sstr, "PdDvLn = '%s'",uniquetype);

        DEBUG_1("PVID comparison:%s\n", sstr )

         /*
	  * Search for all devices with this uniquetype that are available.
	  * Set to initially 4 devices on this system.
	  */

	cusobj_ptr = odm_get_list(CuDv_CLASS,sstr,&list,4,1);
	if (cusobj_ptr == NULL) 
		return (0);
	if ((int)cusobj_ptr == -1)
		return(E_ODMGET);

	/*
	 * For each device found, update its CuDv entry, when a
	 * new_uniquetype is found.
	 */
	for (i = 0;i < list.num;i++) {

		
		if (get_vpd_model(vpd_model,cusobj_ptr[i])) {
			DEBUG_1("get_vpd_model failed for %s\n",
                                                            cusobj_ptr[i].name)
			continue;
		}
		
		DEBUG_1("model_name = %s\n",vpd_model)
		/*
		 * Search ODM for unique type for this model_name.
		 */

		if (det_utype(vpd_model,new_uniquetype)) {
			DEBUG_1("det_utype failed for %s\n",vpd_model)
			continue;
		}
		if (strcmp(uniquetype,new_uniquetype)) {
			/*
			 * If this model number gives us a different
			 * uniquetype then convert the CuDv to this new 
			 * uniquetype and delete all CuAt entries for
			 * this device.
			 */

			DEBUG_2("scsi_chg_dev_type: Convert from %s to %s \n",uniquetype,new_uniquetype)

			sprintf( sstr, "name = '%s'",cusobj_ptr[i].name );
			rc = (int)odm_get_obj( CuDv_CLASS, sstr, &cusobj, 
				      ODM_FIRST);
			if ((rc == 0) || (rc == -1))
				return(E_ODMGET);

			strcpy(cusobj.PdDvLn_Lvalue,new_uniquetype);
			if (rc = odm_change_obj(CuDv_CLASS,&cusobj) == -1) {
				DEBUG_2("scsi_chg_dev_type: odm_change_obj failed with odmerrno = %d for %s\n",odmerrno,cusobj.name)
			        return (E_ODMUPDATE);
			}
	      
		}
    
	}
	return (0);
} 

/* 
 *
 * NAME: get_vpd_model
 *                  
 * FUNCTION:  Gets the device's Model name from the CuVPD.
 *   
 *       
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.
 *
 * 
 *
 * CALLED BY:
 *      
 *	scsi_chg_dev_type 
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *      None
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *   
 *      odm_get_first			strncat
 *	strncpy				strstr
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:     
 *       0  - If sucessful
 *	 -1 - Fails
 */
int get_vpd_model(char *vpd_model,struct CuDv cusobj)
{
	struct CuVPD  vpdobj;               /* structure for device's VPD */
	int		  rc;               /* return code                */
	int                i;               /* general counter            */
	char  sstr[SCD_MAX_STRING_LEN];     /* Search string              */
	char  *model_name_ptr,              /* pointer to model name      */
	      *model_number_ptr,            /* pointer to model number    */
	      *next_field_ptr;              /* pointer to next VPD field  */
					    /* after model_name		  */

        int             j;                  /* second general counter     */
        int             c;                  /* holding place for asterisk */
        int             lth;                       /* length of ODM field */
        char            kw[3];                     /* keyword from ODMGET */

	/*
	 * Search CuVPD for this device
	 */

	sprintf(sstr, "name = '%s' and vpd_type = '%d'",
		cusobj.name,HW_VPD);

	rc = (int)odm_get_first(CuVPD_CLASS,sstr,&vpdobj);
	if ((rc==-1) || (rc == 0)) {

		DEBUG_0("get_vpd_model: ODM failure getting CuVPD object\n")
	        return(-1);
	}

	/*
	 * Initialize model_name and keyword.
	 */
	
	vpd_model[0] = '\0';
	kw[2]='\0';

	/*
	 * Now extract model name by stepping through the VPD, using the
	 * length field, until *TM is found.  The model name consists of
	 * a 4 character product type, a dash "-", and a model number.
	 */

	DEBUG_0("get_vpd_model: About to extract model name from VPD.\n")
        i = 0;
        while(i<VPDSIZE) {
                c = vpdobj.vpd[i++];
                if (c != '*') {
			/*
			 * Invalid VPD keyword if * is missing
			 */
                        break;
		}
		/*
		 * Get VPD keyworkd
		 */
                kw[0] = vpdobj.vpd[i++];
                kw[1] = vpdobj.vpd[i++];

		/*
		 * Compute the starting position of the next
		 * VPD keyword.  NOTE: the length field in the
		 * CuVPD includes the VPD keyword in its length.
		 * NOTE: the length found in CuVPD is half the actual
		 * length.
		 */

                lth = 2 * (vpdobj.vpd[i++] - 2);

		DEBUG_1("get_vpd_model: kw = %s.\n", kw)
                if (strcmp(kw,"TM\0")== 0) {

			DEBUG_0("get_vpd_model: found *TM keyword. \n")
		    /*
		     *  Copy the product type and model number
		     *  (i.e. the model name).
		     */
		    for(j=0; j<(lth); j++) {
                    	vpd_model[j] = vpdobj.vpd[i++];
		    }
                    vpd_model[lth] = '\0';
                    DEBUG_3("length = %d: %s = %s\n",lth,kw,vpd_model);
		    
                    break;
                }
		else
		    i+=lth;
        }
        if (vpd_model[0] == '\0'){
            return(-1);
        }

	 return (0);
}

/*
 * NAME:   det_utype
 *
 * FUNCTION :
 *      This function determines a device's unique type from the
 *      model_name.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 * CALLED BY:
 *      
 *	scsi_chg_dev_type
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *	None
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *   
 *      strcpy				sprintf
 *	odm_get_obj
 *
 * INPUTS:
 *	              model_name	- Model_name we will search for
 *		      uniquetype	- New ODM uniquetype found
 *
 * RETURNS:
 *
 *	0 		- If successful
 *     E_ODMGET		- Failure accessing ODM  (Actually can't return
 *                                                this, because of the default
 *						  uniquetype.)
 */

int det_utype(char *model_name,             /* model_name to search for */
	      char *uniquetype)             /* new found unique type    */
{
	int	i;
        char    sstr[SCD_MAX_STRING_LEN];   /* search string for ODM    */
	char    model_name2[MODEL_NAME_SIZE];/* Model name from CuVPD   */  
	



	/*
	 * There was a problem found in the AIX 3.2 and AIX 3.2.1 cfgscdisk
	 * in which it would not correctly handle blanks embedded in the
	 * model name when it built the TM field of the CuVPD.  This problem
	 * caused truncated model names to be placed in the CuVPD.  It was
	 * discovered when testing the 200mb drive which has embedded blanks.
	 * This problem was fixed with a new cfgscdisk introduced in AIX 3.2.3
	 * in 9/92.  This routine (det_utype) must be able to handle both 
	 * versions of cfgscdisk as much as is possible.  Obviously there are 
	 * some situations with the old cfgscdisk which can not be 100 % 
	 * guaranteed. It should also be noted that the AIX 3.2 and AIX 3.2.1
	 * cfgscdisk inserted a "-" as the fifth character in the TM descriptor
	 * of the CuVPD.  As a result the old cfgscdisk could have TM
	 * descriptors of 17 characters.  The new cfgscdisk does not insert
	 * a "-" so the TM descriptore it generates can be at most 16 characters
	 * long.
	 */



	/*
	 * Assume we are dealing with one of the newer cfgscdisks created for 
	 * AIX 3.2.3.  We may need to pad the ending with blanks
	 * to get a string of 16 characters.
	 */




	/*
	 * If model name length = 17 then we know we are dealing
	 * with the old cfgscdisk and so we will not try this
	 * initial search.
	 */

	if (strlen(model_name) < 17) {
		
		strcpy(model_name2,model_name);

		for (i = strlen(model_name2); i < (MODEL_NAME_SIZE - 2);i++) {
			/*
			 * Pad the ending with blanks
			 */
			model_name2[i] = ' ';
		}
		model_name2[MODEL_NAME_SIZE-2] = '\0';
		sprintf(sstr,"attribute = 'model_name' AND deflt = '%s'",
			model_name2);
	
		DEBUG_0("det_utype: invoke first search, with new cfgscdisk.\n")
		if (!search_odm(sstr,uniquetype)) {
			/*
			 * Found a match so return 
			 */
			return (0);
		}
	
	}


	
	/*
	 * We could not find an exact match when we assumed
	 * it was created by the newer cfgscdisks.  So let's
	 * assume the old cfgscdisk that was released with AIX 3.2 or 
	 * AIX 3.2.1. This will require us to 
	 * remove the "-" and add blanks on the end. We will
	 * then look for an exact match for this.  However first let's see
	 * if it is a 200mb, because this must be dealt with in a
	 * special way with regards to the old cfgscdisk due to above mentioned
	 * problem.
	 */		
	
	

	if (!(strncmp(model_name,"WDS--3200",9))) {
		/*
		 * Assume this is 200mb and build the correct model_name
		 */
		strcpy(model_name2,"WDS-3200      !J");
		model_name2[MODEL_NAME_SIZE-2] = '\0';
		sprintf(sstr,
			"attribute = 'model_name' AND deflt = '%s'",
			model_name2);
		DEBUG_0("det_utype: invoke second search, for 200mb utype.\n")
		if (!search_odm(sstr,uniquetype)) {
			
			return (0);
		}
	}
	

	
	if (model_name[4] == '-') {
		
		/*
		 * For all other non 200mb disks, We must remove the "-" 
		 * from the model name and then pad blanks on the end of 
		 * the string until its length is 16 characters.
		 */		
		strncpy(model_name2,model_name,4);
		strcpy(&model_name2[4],&model_name[5]);
		for (i = strlen(model_name2); i < (MODEL_NAME_SIZE - 2);i++) {
			/*
			 * Pad the ending with blanks
			 */
			model_name2[i] = ' ';
		}
		
		
		model_name2[MODEL_NAME_SIZE-2] = '\0';
		sprintf(sstr,
			"attribute = 'model_name' AND deflt = '%s'",
			model_name2);
		DEBUG_0("det_utype: invoke third search, for all other utypes.\n")
		if (!search_odm(sstr,uniquetype)) {
			return (0);
		}
	}	

	/*
	 * If we got here then the model name does not match anything 
	 * we are familiar with, so use the default uniquetype.
	 */

	DEBUG_0("det_utype: strcpy of osdisk to uniquetype\n")
	strcpy(uniquetype,default_uniquetype);
	return (0);

}

/*
 * NAME:   search_odm
 *
 * FUNCTION :
 *      This function searches the ODM PdAt looking for an
 *	exact match for the model_name.  If only one instance is found
 *	the search is considered successfull and the uniquetype argument
 *      is filled in from the uniquetype found in the PdAt.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 * CALLED BY:
 *      
 *	det_utype
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *	None
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *   
 *      strcpy				
 *	odm_get_obj
 *
 * INPUTS:
 *	              model_name	- Model_name we will search for
 *		      uniquetype	- New ODM uniquetype found
 *
 * RETURNS:
 *
 *	0 		- If successful
 *     E_ODMGET		- Failure accessing ODM
 */
int search_odm(char *sstr,             /* model_name to search for */
	      char *uniquetype)             /* new found unique type    */
{
	struct  PdAt  *PdAt_ptr;	    /* List of devices          */
					    /* that match the given     */
					    /* uniquetype               */
	struct listinfo list;               /* for using odm_get_list   */


	uniquetype[0] = '\0';   DEBUG_1("det_utype: calling odm_get for *%s*\n",sstr) 

	PdAt_ptr = odm_get_list(PdAt_CLASS,sstr,&list,4,1);
	

        if (PdAt_ptr == NULL) {
		/* 
		 *  No match means:
		 *    1.  The disk was configured with the pre-3.2.3
		 *	  cfgscdisk, and the search needs to be called
		 *	  again, when the model name does not have a
		 *	  dash (-) in the 5th character.
		 *    OR
		 *    2.  The predefines for that disk have been removed,
		 *	  and the disk needs to be "converted" to the
		 *	  default uniquetype.
		 *	  (In this case, it is OK to keep searching, but
		 *	   not necessary.)
		 *    OR
		 *    3.  The model name does not exist in the database.
		 */

                DEBUG_1("Use default devicetype %s\n",default_uniquetype)
                strcpy(uniquetype,default_uniquetype);
		return (1);
        }
	
        if ((int)PdAt_ptr == -1) {
                DEBUG_1("Error searching for device type using\n%s\n",sstr)
                return (E_ODMGET);
	}

        /* 
	 *  Store the uniquetype, and return if only one match found.
	 */
	if (list.num == 1) {
		strcpy( uniquetype, PdAt_ptr[0].uniquetype );
		return (0);
	}
        else
		return(1);


}

/*
 * NAME: err_exit
 *
 * FUNCTION: Closes any open object classes and terminates ODM.  Used to
 *           back out on an error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.  
 * 
 * CALLED BY:
 *      
 *	main
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *	None
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *   
 *	odm_terminate			
 *
 *
 * RETURNS:
 *               None
 */

void err_exit(int exitcode)
{

        /* 
	 * Terminate the ODM. 
	 */
        odm_terminate();
        exit(exitcode);
}

