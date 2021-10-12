static char sccsid[] = "@(#)79  1.5  src/bos/usr/lib/methods/cfglvdd/cfglvdd.c, cfgmethods, bos411, 9428A410j 3/31/94 13:51:46";
/*
 * COMPONENT_NAME: (CFGMETH) LVM device driver config methods
 *
 * FUNCTIONS: main(), methexit()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/cfgdb.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>

#include "cfgdebug.h"

#define LVM_LVDDTYPE    "lvdd"
#define LVM_KMIDFILE    "/etc/vg/lvdd_kmid"
#define LVM_STREAMWR    "w"


/* main function code */

main (argc, argv, envp)
int argc;
char *argv[];
char *envp[];
{

    struct cfg_dd cfg;		/* sysconfig command structure		*/

    char sstring[256];		/* search criteria pointer		*/
    char errstring[512];	/* error string pointer			*/

    struct Class *cusdev;	/* customized devices class ptr		*/
    struct Class *predev;	/* predefined devices class ptr		*/

    struct CuDv cusobj;		/* customized device object storage	*/
    struct PdDv preobj;		/* predefined device object storage	*/

    mid_t kmid;			/* module id from loader		*/
    dev_t devno;		/* device number for config_dd		*/

    int majorno, minorno;	/* major and minor numbers		*/
    int ipl_phase;		/* ipl phase: 0=run,1=phase1,2=phase2	*/
    int rc;			/* return codes go here			*/

    FILE *stream;


	/* start up odm */
	if(odm_initialize () == -1)
		methexit( E_ODMINIT );



	/* search Custom Devices for customized object with this logical name */
	sprintf (sstring, "name = '%s'", LVM_LVDDTYPE);
	rc = (int)odm_get_first (CuDv_CLASS, sstring, &cusobj);
	if (rc == 0)
                /* No CuDv object with this name */
                exit(E_NOCuDv);
        else if (rc == -1)
                /* ODM failure */
                exit(E_ODMGET);


    /*******************************************************************
      Get the predefined object for this device. This object is located
      searching the predefined devices object class based on the unique
      type link descriptor in the customized device.
     *******************************************************************/


	/* search Predefined devices for object with this logical name */
	sprintf (sstring, "uniquetype = '%s'", cusobj.PdDvLn_Lvalue);
	rc = (int)odm_get_first (PdDv_CLASS, sstring, &preobj);
	if (rc==0)
                /* No CuDv object with this name */
                exit(E_NOPdDv);
        else if (rc==-1)
                /* ODM failure */
                exit(E_ODMGET);


    /****************************************************************
      Display this device's LED value on the system LEDs.
     ****************************************************************/

	setleds (preobj.led);

    /******************************************************************
      Check to see if the device is already configured (AVAILABLE).
      We actually go about the business of configuring the device
      only if the device is not configured yet. Configuring the
      device in this case refers to the process of checking parent
      and sibling status, checking for attribute consistency, build-
      ing a DDS, loading the driver, etc...
     ******************************************************************/

	if (cusobj.status != AVAILABLE)
	{
	/*
	 * If needed, this function will load the driver and return
	 * the driver's kernel module id (kmid). Then the device driver
	 * instance field in the device's customized object will have
	 * a key into the predefined device driver object class.
	 */

	/*
	 * Call loadext() to load device driver.
	 * loadext will load the driver only if needed,
	 * and always returns the kmid of the driver.
	 */

		cfg.kmid = loadext (preobj.DvDr,TRUE,FALSE);
		if ((int) cfg.kmid == NULL)
			methexit(E_LOADEXT);


	/**********************************************************
	  Update the customized object to reflect the device's
	  current status. The device status field should be
	  changed to AVAILABLE, and, if they were generated, the
	  device's major & minor numbers should be updated, as
	  well as any VPD data that was retrieved.
	 **********************************************************/

		cusobj.status = AVAILABLE;
		if(odm_change_obj(CuDv_CLASS, &cusobj) == -1)
			methexit( E_ODMUPDATE );
	}



	/***********************************************************
	 config method is finished at this point. Terminate the
	 ODM, and exit with a good return code.
	***********************************************************/

	odm_terminate();
	exit(E_OK);

}



/**********************
   method exit routine
 ***********************/

methexit(exitcode)
int exitcode;
{
    odm_terminate();
    exit(exitcode);
}
