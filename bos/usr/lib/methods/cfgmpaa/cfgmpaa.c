static char sccsid[] = "@(#)22        1.1  src/bos/usr/lib/methods/cfgmpaa/cfgmpaa.c, mpacfg, bos411, 9428A410j 4/30/93 11:55:30";
/*
 *   COMPONENT_NAME: (MPACFG) MP/A CONFIGURATION FILES
 *
 *   FUNCTIONS: build_dds
 *		define_children
 *		download_microcode
 *		generate_minor
 *		make_special_files
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

#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include <sys/cfgdebug.h>

#define S_LENGTH  256           /* length of buffer for get commands */


/*
 *
 * NAME: build_dds
 *
 * FUNCTION: Builds a DDS (Defined Data Structure) describing a device's
 *	characteristics to it's device driver.
 *
 * EXECUTION ENVIRONMENT:
 *	Operates as a device dependent subroutine called by the generic
 *	configure method for all devices.
 *
 * NOTES: There is no DDS for the MPA adapter (DDS's are built for the child
 *        MPA drivers) this is then a NULL function.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int build_dds(lname, ddsptr, ddslen)
char  *lname;                           /* logical name of device */
uchar **ddsptr;                         /* receiving pointer for DDS address */
long  *ddslen;                          /* receiving variable for DDS length */
{
	return 0; 
}

/*
 *
 * NAME: generate_minor
 *
 * FUNCTION: To provide a unique minor number for the current device instance.
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: There is no minor number required for the MPA adapter,
 *        this function is NULL.
 *
 * RETURNS: 0 - success, in all cases
 *
 */
generate_minor( lname, majorno, minorno )
char    *lname;
long    majorno;
long    *minorno;
{
	return 0;
}

/*
 * NAME: make_special_files
 *
 * FUNCTION: To create the special character file on /dev for the current device
 *           instance.
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: For the MPA adapter, there is no special character file on /dev, this
 *        function is then NULL.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int make_special_files( lname, devno )
char    *lname;
dev_t	devno;
{ 
	return 0; 
}

/*
 *
 * NAME: download_microcode
 *
 * FUNCTION: To download the micro code for the adapter.
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 *
 * RETURNS: 0 - success, in all cases
 *
 * NOTE: There is no microcode for the MP/A adpater so this function
 *        return 0
 */

int  download_microcode( lname )
char  *lname;
{ 
	return 0; 
}

/*
 *
 * NAME: query_vpd
 *
 * FUNCTION: To query a device for VPD information (Vital Product Data)
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: For the MPA adapter, VPD information is not currently supported, this
 *        function is then NULL.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int  query_vpd( newobj, kmid, devno, vpdstr )
char   *newobj;
mid_t   kmid;
dev_t   devno;
char    *vpdstr;
{
	return 0; 
}

/*
 * NAME     : def_mpad
 *
 * FUNCTION : This function defines the children of the parent mp/a
 *      device as sdlc device drivers 
 *
 * EXECUTION ENVIRONMENT :
 *      This function returns 0 on success, < 0 on failure.
 *      Operates as a device dependent subroutine called by the generic
 *      configure method for all devices.
 *
 * NOTES :
 *      The multiprotocol adapter/a's child devices are the device drivers
 *      that may be configured to run with this adapter.  The standard
 *      device is the sdlc device driver.
 *
 */

#define MPASDLC_UNIQUETYPE "uniquetype = 'driver/mpaa/mpa'"

int def_mpad(char *lname)
{
        struct  PdDv   pddvsdlc;
        int rc;
        char string[S_LENGTH];

        if(( rc = odm_get_obj( PdDv_CLASS,MPASDLC_UNIQUETYPE, &pddvsdlc,ODM_FIRST )) == 0 ) 
	{
                DEBUG_0("cfgmpaa (def_mpad): no objects found\n")
		return(E_OK);
	}
	else if( rc == -1)
	{
                DEBUG_2("cfgmpaa (def_mpad): get failed lname=%s rc=%d\n", lname,rc)
                return(E_ODMGET);
        }

        sprintf( string, "-c driver -s mpaa -t mpa -p %s -w 0 ", lname );
        if(odm_run_method(pddvsdlc.Define,string,NULL,NULL)){
                fprintf(stderr,"cfgmpaa: can't run %s\n", pddvsdlc.Define);
                return E_ODMRUNMETHOD;
        }
       
        return 0;

}


/*
 *
 * NAME: define_children
 *
 * FUNCTION: To invoke the generic define method for each child device not
 *           already in the customized database. This will result in all
 *           children that are not AVAILABLE being created in the customized
 *           data base with status DEFINED. To then output on stdout the name
 *           of each DEFINED child device in order to cause that child's
 *           configuration.
 *
 * EXECUTION ENVIRONMENT:
 *    Operates as a device dependent subroutine called by the generic configure
 *    method for all devices.
 *
 *
 * RETURNS: 0 - Success
 *         <0 - Failure
 *
 */

int  define_children( lname, ipl_phase )
char  *lname;                          /* logical name of parent device */
int    ipl_phase;                       /* phase if ipl */
{
        struct  Class   *predev ;       /* predefined devices class ptr */
        struct  Class   *cusdev ;       /* customized devices class ptr */
        struct  PdDv    preobj ;        /* predefined device object     */
        struct  CuDv    cusobj ;        /* customized device object     */
        char    sstring[S_LENGTH];      /* search criteria              */
        int     rc;                     /* return code                  */
        char    *out_p;

        /* declarations for card query */
        char    busdev[32] ;
        int     fd ;
        struct  CuAt    *cusatt;
        ulong   ulong_val;




	DEBUG_1("cfgmpaa (def_child): ipl_phase is %d\n",ipl_phase)

        /* open customized device object class */
        if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1) {
                /* error opening class */
                DEBUG_0("cfgmpaa (def_child): open of CuDv failed\n")
                err_exit(E_ODMOPEN);
        }

        /* search CusDevices for customized object with this logical
           name as parent */
        sprintf(sstring, "parent = '%s'", lname);
        if ((rc = (int)odm_get_obj(cusdev, sstring, &cusobj, TRUE)) == 0) {
                /* odm objects not found */
                DEBUG_0("cfgmpaa (def_child): define sdlc driver as default\n")
		return(def_mpad(lname));
        }
        else if (rc == -1)
        {       DEBUG_1("cfgmpaa (def_child): couldn't get child of %s\n",lname)
                return(E_ODMGET);
        }
        DEBUG_1("cfgmpaa (def_child): name of child %s\n",cusobj.name)
        fprintf(stdout,"%s ",cusobj.name);

        rc = odm_close_class(cusdev);
        if(rc < 0){
                /* error closing object class */
                DEBUG_0("cfgmpaa (def_child): close object class CuDv failed\n")
                return(E_ODMCLOSE);
        }
        return(E_OK);
}
