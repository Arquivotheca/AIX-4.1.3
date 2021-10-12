static char sccsid[] = "@(#)17  1.2  src/bos/usr/lib/methods/common/cfgcommo.c, cfgmethods, bos411, 9428A410j 2/17/94 15:00:44";
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: err_exit
 *		err_undo1
 *		err_undo2
 *		err_undo3
 *		main
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *   cfgcommo.c - Generic Configure Method Code
 *              (used for CDLI kernel module device drivers)
 *
 *   interface:
 *      cfgXXX -l <logical_name> [-<1|2>] [-D]
 *
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
#include <sys/ndd.h>

/* Local header files */
#include "cfgdebug.h"
#include "cfg_ndd.h"

/* external functions */
extern  int conferrno;          /* Config (getattr) Error Number */

/* global variables */
int	Dflag;			/* flag for defining children */


/* main function code */
main(argc, argv, envp)
int	argc;
char	*argv[];
char	*envp[];

{
    extern  int     optind;         /* for getopt function */
    extern  char    *optarg;        /* for getopt function */

    struct cfg_kmod cfg_k;          /* sysconfig command structure */

    char    *logical_name;          /* logical name to configure */
    char    sstring[256];           /* search criteria pointer */
    char    conflist[1024];         /* busresolve() configured devices */
    char    not_resolved[1024];     /* busresolve() not resolved devices */
    char    vpd[VPDSIZE];           /* vpd data */

    ndd_cfg_odm_t    dev_obj;       /* struc for odm pointers for subroutines */

    struct Class *cusvpd;           /* customized vpd class ptr */

    struct CuDv dmyobj;             /* temp customized device object storage */
    struct CuVPD vpdobj;            /* customized vpd object */
    struct CuAt *cusattobj;         /* device customized attribute storage */

    int     dev_kmid;               /* device drivers cfg_k.kmid value */
    char    addl_dvdr[128];         /* name of additional driver */
    int     how_many;               /* storage for getattr command */

    ndd_config_t    ndd_config;     /* devices cfg init struct */
    int     inst_num;                /* instance number of device */
    int     l_dds;                  /* length of user defined dds */

    ushort  devid;                  /* Device id - used at run-time */
    int     ipl_phase;              /* ipl phase: 0=run,1=phase1,2=phase2 */
    int     slot;                   /* slot of adapters */
    int     rc;                     /* return codes go here */
    int     errflg,c;               /* used in parsing parameters   */

    /*****                                                          */
    /***** Parse Parameters                                         */
    /*****                                                          */
    ipl_phase = RUNTIME_CFG;
    errflg = 0;
    logical_name = NULL;
    Dflag = FALSE;

    while ((c = getopt(argc,argv,"l:12D")) != EOF) {
	switch (c) {
	    case 'l':
		    if (logical_name != NULL)
			    errflg++;
		    logical_name = optarg;
		    break;
	    case 'D':
		    if (Dflag != FALSE)
			    errflg++;
		    Dflag = TRUE;
		    break;
	    case '1':
		    if (ipl_phase != RUNTIME_CFG)
			    errflg++;
		    ipl_phase = PHASE1;
		    break;
	    case '2':
		    if (ipl_phase != RUNTIME_CFG)
			    errflg++;
		    ipl_phase = PHASE2;
		    break;
	    default:
		    errflg++;
	}
    }
    if (errflg) {
	    /* error parsing parameters */
	    DEBUG_0("cfgcommo: command line error\n");
	    exit(E_ARGS);
    }

    /*****                                                          */
    /***** Validate Parameters                                      */
    /*****                                                          */
    /* logical name must be specified */
    if (logical_name == NULL) {
	    DEBUG_0("cfgcommo: logical name must be specified\n");
	    exit(E_LNAME);
    }

    DEBUG_1 ("cfgcommo: Configuring device: %s\n",logical_name)

    /* start up odm */
    if (odm_initialize() == -1) {
	    /* initialization failed */
	    DEBUG_0("cfgcommo: odm_initialize() failed\n")
	    exit(E_ODMINIT);
    }
    DEBUG_0 ("cfgcommo: ODM initialized\n")

    /* open customized devices object class */
    if ((int)(dev_obj.cusdev = odm_open_class(CuDv_CLASS)) == -1) {
	    DEBUG_0("cfgcommo: open class CuDv failed\n");
	    err_exit(E_ODMOPEN);
    }

    /* search for customized object with this logical name */
    sprintf(sstring, "name = '%s'", logical_name);
    rc = (int)odm_get_first(dev_obj.cusdev,sstring,&dev_obj.cusobj);
    if (rc==0) {
	/* No CuDv object with this name */
	DEBUG_1("cfgcommo: failed to find CuDv object for %s\n", logical_name);
	err_exit(E_NOCuDv);
    }
    else if (rc==-1) {
	/* ODM failure */
	DEBUG_0("cfgcommo: ODM failure getting CuDv object");
	err_exit(E_ODMGET);
    }

    /* open predefined devices object class */
    if ((int)(dev_obj.predev = odm_open_class(PdDv_CLASS)) == -1) {
	    DEBUG_0("cfgcommo: open class PdDv failed\n");
	    err_exit(E_ODMOPEN);
    }

    /* get predefined device object for this logical name */
    sprintf(sstring, "uniquetype = '%s'", dev_obj.cusobj.PdDvLn_Lvalue);
    rc = (int)odm_get_first(dev_obj.predev, sstring, &dev_obj.preobj);
    if (rc==0) {
	    /* No PdDv object for this device */
	    DEBUG_0("cfgcommo: failed to find PdDv object for this device\n");
	    err_exit(E_NOPdDv);
    }
    else if (rc==-1) {
	    /* ODM failure */
	    DEBUG_0("cfgcommo: ODM failure getting PdDv object");
	    err_exit(E_ODMGET);
    }

    /****************************************************************
      If this device is being configured during an ipl phase, then
      display this device's LED value on the system LEDs.
      ****************************************************************/
    if (ipl_phase != RUNTIME_CFG)
	    setleds(dev_obj.preobj.led);

    /****************************************************************
      Open databases and verify parent device
      ****************************************************************/
    /* open predefined attributes object class */
    if ((int)(dev_obj.preatt = odm_open_class(PdAt_CLASS)) == -1) {
	    DEBUG_0("cfgcommo: open class PdAt failed\n");
	    err_exit(E_ODMOPEN);
    }

    /* open customized attributes object class */
    if ((int)(dev_obj.cusatt = odm_open_class(CuAt_CLASS)) == -1) {
	    DEBUG_0("cfgcommo: open class CuAt failed\n");
	    err_exit(E_ODMOPEN);
    }

    /******************************************************************
      At this point, there is a CuDv entry for this device which means
      the State is either DEFINED or AVAILABLE.
      ******************************************************************/

    /* get the device's parent object */
    sprintf(sstring, "name = '%s'", dev_obj.cusobj.parent);
    rc = (int)odm_get_first(dev_obj.cusdev,sstring,&dev_obj.parobj);
    DEBUG_2("cfgcommo: name of parent device = %s,rc=%d\n",dev_obj.cusobj.parent,rc);
    if (rc==0)
    {
	    /* Parent device not in CuDv */
	    DEBUG_0("cfgcommo: no parent CuDv object\n");
	    err_exit(E_NOCuDvPARENT);
    }
    else if (rc==-1)
    {
	    /* ODM failure */
	    DEBUG_0("cfgcommo: ODM failure getting parent CuDv object\n")
	    err_exit(E_ODMGET);
    }


    /******************************************************************
      Check to see if the device is already configured (AVAILABLE).
      We actually go about the business of configuring the device
      only if the device is not configured yet. Configuring the
      device in this case refers to the process of checking parent
      and sibling status, checking for attribute consistency, build-
      ing a DDS, loading the driver, etc...
      ******************************************************************/

    if (dev_obj.cusobj.status == DEFINED)
    {

	/*******************************************************
	  The device is not available to the system yet. Now
	  check to make sure that the device's relations will
	  allow it to be configured. In particular, make sure
	  that the parent is configured (AVAILABLE),
	  and that no other devices are configured at the same
	  location.
	  *******************************************************/

	if (dev_obj.parobj.status != AVAILABLE)
	{
		DEBUG_0("cfgcommo: parent is not AVAILABLE")
		err_exit(E_PARENTSTATE);
	}

	/*******************************************************
	   Check to see if any device is selected for this parent
	   at this location.
	  *******************************************************/
	/* make sure that no other devices are configured     */
	/* at this location (This includes the following      */
	/* states: AVAILABLE, STOPPED, DIAGNOSE)              */

	sprintf(sstring, "parent = '%s' AND connwhere = '%s' AND status != %d",
		dev_obj.cusobj.parent, dev_obj.cusobj.connwhere, DEFINED);
	rc = (int)odm_get_first(dev_obj.cusdev,sstring,&dmyobj);
	if (rc == -1)
	{
		/* odm failure */
		err_exit(E_ODMGET);
	}
	else if (rc)
	{
		/* Error: device config'd at this location */
		DEBUG_0("cfgcommo: device already AVAILABLE at this connection\n")
		err_exit(E_AVAILCONNECT);
	}

	/***************************************************
	  If the device is an adapter being configured at
	  RUN TIME, then we must resolve any bus attribute
	  conflicts before configuring device to the driver.
	 ***************************************************/
	if (!strcmp(dev_obj.preobj.subclass,"mca"))
	{
	    if (ipl_phase == RUNTIME_CFG)
	    {
		DEBUG_0("cfgcommo: checking adapter for slot and bus resources\n")
		/* Make sure card is in specified slot */
		slot = atoi(dev_obj.cusobj.connwhere);
		devid = (ushort) strtol(dev_obj.preobj.devid,(char **) NULL,0);
		sprintf (sstring,"/dev/%s",dev_obj.cusobj.parent);
		rc = chkslot(sstring,slot,devid);
		if (rc != 0) {
			DEBUG_2("cfgcommo: card %s not found in slot %d\n",
				logical_name,slot);
			err_exit(rc);
		}
	       /*
		* Get the bus that this device is connected to.
		* (searches hierarchy until bus is found.
		*  then returns object information for bus )
		*/
		rc = Get_Parent_Bus(dev_obj.cusdev, dev_obj.cusobj.parent, &dmyobj) ;
		if (rc == 0)
		{
		    /* Invoke Bus Resolve  */
		    rc = busresolve(logical_name,(int)0,conflist,
			not_resolved, dmyobj.name);
		    if (rc != 0)
		    {
			DEBUG_0("cfgcommo: bus resources could not be resolved\n")
			err_exit(rc);
		    }
		}
		else if (rc == E_PARENT)
		{
		    DEBUG_0("cfgcommo: device not on bus\n")
		}
		else
		{
		    DEBUG_0("cfgcommo: ODM err getting parent bus\n")
		    err_exit(rc) ;
		}
	    }   /* end of ipl phase check */
	}   /* end of device is adapter (subclass = mca) */

	 /***************************************************
	  If device has a device driver, then need to load driver,
	  build DDS and pass to driver.
	  Then, a device dependent routine is called for
	  downloading microcode.
	  If device has an additional driver, it will
	  then be loaded.
	 ***************************************************/
	dev_kmid = 0;           /* initialize values */

	DEBUG_1("cfgcommo: checking for DvDr entry, DvDr=%s\n",dev_obj.preobj.DvDr);
	if (strcmp(dev_obj.preobj.DvDr, "") != 0)
	{

	    /* get instance number for this device */
	    inst_num = geninst(dev_obj.preobj.DvDr,logical_name);
	    if (inst_num == -1) {
		DEBUG_0("cfgcommo: error getting instance number\n")
		err_exit(E_INSTNUM);
	    }
	    DEBUG_1("cfgcommo: instance number = %d\n",inst_num);

	    DEBUG_0("cfgcommo: load a device driver\n")
	    /* call loadext to load the device driver */
	    if ((cfg_k.kmid = loadext(dev_obj.preobj.DvDr, TRUE, FALSE))
		 == NULL)
	    {
		/* error loading device driver */
		DEBUG_1("cfgcommo: error loading driver %s\n", dev_obj.preobj.DvDr)
		err_exit(E_LOADEXT);
	    }
	    dev_kmid = cfg_k.kmid;
	    DEBUG_1("cfgcommo: kernel module id = %x\n",dev_kmid)
	    ndd_config.seq_number = inst_num;
	    ndd_config.dds = (char * ) NULL;
	    cfg_k.mdiptr = (char *) &ndd_config;
	    cfg_k.mdilen = sizeof (struct ndd_config);

	    /* build the DDS  */
	    DEBUG_0("cfgcommo: Calling build_dds()\n")
	    rc = build_dds(logical_name, &dev_obj,
			   &ndd_config.dds,
			   &l_dds );
	    if (rc) {
		    /* error building dds */
		    DEBUG_1("cfgcommo: error building dds, rc=%d\n",rc)
		    err_undo1(dev_obj.preobj.DvDr);
		    if ( rc < 0 || rc > 255)
			    rc = E_DDS;
		    err_exit(rc);
	    }
	    DEBUG_0("cfgcommo: Returned from build_dds()\n")

	    /* call sysconfig to pass DDS to driver */
	    DEBUG_0("cfgcommo: Pass DDS to driver via sysconfig()\n")
	    cfg_k.cmd = CFG_INIT;
	    if (sysconfig(SYS_CFGKMOD, &cfg_k, sizeof(struct cfg_kmod )) == -1) {
		    /* error configuring device */
		    DEBUG_0("cfgcommo: error configuring device\n")
		    err_undo1(dev_obj.preobj.DvDr);
		    err_exit(E_CFGINIT);
	    }

	    /* download microcode if necessary */
	    DEBUG_0("cfgcommo: Calling download_microcode()\n")

	    ndd_config.seq_number = inst_num;
	    ndd_config.ucode = (char * ) NULL;
	    cfg_k.mdiptr = (char *) &ndd_config;
	    cfg_k.mdilen = sizeof (struct ndd_config);

	    rc = download_microcode(logical_name, &dev_obj, &cfg_k);
	    if (rc) {
		/* error downloading microcode */
		DEBUG_1("cfgcommo: error downloading microcode, rc=%d\n",rc)
		err_undo2(cfg_k.kmid,inst_num);
		err_undo1(dev_obj.preobj.DvDr);
		if ( rc < 0 || rc > 255)
			rc = E_UCODE;
		err_exit(rc);
	    }
	    DEBUG_0("cfgcommo: Returned from download_microcode()\n")

	    /***************************************************
	      If device has an additional driver, then need to
	      call the specified configure method for that driver
	      to load and configure the driver
	      Attribute addl_dvdr is the method to run
	     ***************************************************/

	    strcpy ( addl_dvdr, NULL );      /* initialize values */
	    cusattobj = getattr( logical_name, "addl_dvdr", FALSE,&how_many );
	    if (cusattobj != NULL) {
		    strcpy ( addl_dvdr, cusattobj->value);
	    }
	    else {
		/* NOTE: error to be indicated here           */
		/*       only if attribute is there but could */
		/*       not be read                          */
		if ( conferrno != E_NOPdOBJ ) {
			/* error getting addl dvdr name */
			DEBUG_0("cfgcommo: error getting addl dvdr name\n")
			err_undo2(dev_kmid,inst_num);
			err_undo1(dev_obj.preobj.DvDr);
			err_exit(E_ODMGET);
		}
	    }

	    if (strcmp(addl_dvdr, "") != 0)
	    {

		/* call specified method with parameter "ADDL_CFG"
		 * to configure the additional driver
		 */

		sprintf( sstring, " %d %s %d ",
			ADDL_CFG, logical_name, inst_num);
		DEBUG_2("cfgcommo: calling %s %s\n",addl_dvdr, sstring)

		if(odm_run_method(addl_dvdr,sstring,NULL,NULL)){
			fprintf(stderr,"cfgcommo: can't run %s\n",
				addl_dvdr);
			err_undo2(dev_kmid,inst_num);
			err_undo1(dev_obj.preobj.DvDr);
			err_exit(E_ODMRUNMETHOD);
		}

	    } /* end if (device has an additional driver) then ... */

	} /* end if (device has a driver) then ... */

	/* call device specific routine to for any special work */
	DEBUG_0("cfgcommo: Calling device specific hook()\n")

	rc = device_specific(logical_name, &dev_obj, dev_kmid);
	if (rc != 0) {
		/* error in device specific */
		DEBUG_0("cfgcommo: error in device specific\n");
		/* need to find a E_ error to return here to cover users */
		err_exit(rc);
	}
	DEBUG_0("cfgcommo: Returned from specific_work()\n");

	/* if device has VPD data then go get it */
	if (dev_obj.preobj.has_vpd == TRUE) {
	    /* get VPD for this device */
	    memset( vpd, 0, sizeof(vpd) );
	    DEBUG_0("cfgcommo: Calling query_vpd()\n")

	    ndd_config.seq_number = inst_num;
	    cfg_k.mdiptr = (char *) &ndd_config;
	    cfg_k.mdilen = sizeof (struct ndd_config);

	    rc = query_vpd(logical_name, &dev_obj, &cfg_k, vpd);
	    if (rc) {
		    /* failed to get VPD */
		    DEBUG_1("cfgcommo: error getting VPD, rc=%d\n",rc)
		    err_undo3(dev_obj.preobj.DvDr, dev_kmid, inst_num);
		    if ( rc < 0 || rc > 255)
			    rc = E_VPD;
		    err_exit(rc);
	    }
	    DEBUG_0("cfgcommo: Returned from query_vpd()\n")

	    /* open customized vpd object class */
	    if ((int)(cusvpd = odm_open_class(CuVPD_CLASS)) == -1) {
		    DEBUG_0("cfgcommo: open class CuVPD failed");
		    err_undo3(dev_obj.preobj.DvDr, dev_kmid, inst_num);
		    err_exit(E_ODMOPEN);
	    }

	    /* search for customized vpd object with this logical name */
	    sprintf(sstring, "name = '%s' and vpd_type = '%d'",
		    logical_name,HW_VPD);
	    rc = (int)odm_get_first(cusvpd,sstring,&vpdobj);
	    if (rc==-1) {
		    /* ODM failure */
		    DEBUG_0("cfgcommo: ODM failure getting CuVPD object");
		    err_undo3(dev_obj.preobj.DvDr, dev_kmid, inst_num);
		    err_exit(E_ODMGET);
	    }
	    if (rc==0) {
		/* need to add vpd object */
		DEBUG_0("cfgcommo: Adding new VPD object\n");
		strcpy(vpdobj.name,logical_name);
		vpdobj.vpd_type = HW_VPD;
		memcpy(vpdobj.vpd,vpd,sizeof(vpd));
		if (odm_add_obj(cusvpd,&vpdobj) == -1) {
			DEBUG_0("cfgcommo: ODM failure adding CuVPD object")
			err_undo3(dev_obj.preobj.DvDr, dev_kmid, inst_num);
			err_exit(E_ODMADD);
		}
		DEBUG_0("cfgcommo: Successfully added new VPD object\n");
	    } else {
		/* see if vpd object needs to be updated */
		if (memcmp(vpdobj.vpd,vpd,sizeof(vpd))) {
		    DEBUG_0("cfgcommo: Updating VPD object\n");
		    memcpy(vpdobj.vpd,vpd,sizeof(vpd));
		    if (odm_change_obj(cusvpd,&vpdobj) == -1) {
			DEBUG_0("cfgcommo: ODM failure updating CuVPD object")
			err_undo3(dev_obj.preobj.DvDr, dev_kmid, inst_num);
			err_exit(E_ODMUPDATE);
		    }
		    DEBUG_0("cfgcommo: Successfully updated VPD object\n");
		}
	    }
	    /* close customized vpd object class */
	    if (odm_close_class(CuVPD_CLASS) == -1) {
		    DEBUG_0("cfgcommo: error closing CuVPD object class\n");
		    err_undo3(dev_obj.preobj.DvDr, dev_kmid, inst_num);
		    err_exit(E_ODMCLOSE);
	    }
	}  /* end if (device has vpd) then ... */

	/*
	 * Update customized device object.  Set current
	 * status to AVAILABLE and reset change status to
	 * SAME only if it was MISSING
	 */

	dev_obj.cusobj.status = AVAILABLE;
	if (dev_obj.cusobj.chgstatus == MISSING)
	{
	    dev_obj.cusobj.chgstatus = SAME ;
	}
	if (odm_change_obj(dev_obj.cusdev, &dev_obj.cusobj) == -1)
	{
	    /* ODM failure */
	    DEBUG_0("cfgcommo: ODM failure updating CuDv object\n");
	    err_exit(E_ODMUPDATE);
	}

    } /* end if (device is not AVAILABLE) then ... */

    /* call device specific routine to detect/manage child devices */
    DEBUG_0("cfgcommo: Calling define_children()\n");
    if (define_children(logical_name, &dev_obj, ipl_phase) != 0) {
	    /* error defining children */
	    DEBUG_0("cfgcommo: error defining children\n");
	    err_exit(E_FINDCHILD);
    }
    DEBUG_0("cfgcommo: Returned from define_children()\n")

    /* close predefined device object class */
    if (odm_close_class(dev_obj.predev) == -1) {
	    DEBUG_0("cfgcommo: close object class PdDv failed");
	    err_exit(E_ODMCLOSE);
    }

    /* close customized device object class */
    if (odm_close_class(dev_obj.cusdev) == -1) {
	    DEBUG_0("cfgcommo: error closing CuDv object class\n");
	    err_exit(E_ODMCLOSE);
    }

    /* close PdAt object class */
    if (odm_close_class(dev_obj.preatt) == -1) {
	    DEBUG_0("cfgcommo: error closing PdAt object class\n");
	    err_exit(E_ODMCLOSE);
    }
    /* close customized att object class */
    if (odm_close_class(dev_obj.cusatt) == -1) {
	    DEBUG_0("cfgcommo: error closing CuAt object class\n");
	    err_exit(E_ODMCLOSE);
    }

    odm_terminate();
    exit(0);

}

/*
 * NAME: err_exit
 *
 * FUNCTION: Closes any open object classes and terminates ODM.  Used to
 *           back out on an error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.  The device
 *      specific routines for the various device specific config methods
 *      must not call this function.
 *
 * NOTES:
 *
 * void
 *   err_exit( exitcode )
 *      exitcode = The error exit code.
 *
 * RETURNS:
 *               None
 */

err_exit(exitcode)
char    exitcode;
{
	/* Close any open object class */
	odm_close_class(CuDv_CLASS);
	odm_close_class(PdDv_CLASS);
	odm_close_class(CuAt_CLASS);
	odm_close_class(PdAt_CLASS);

	/* Terminate the ODM */
	odm_terminate();
	exit(exitcode);
}

/*
 * NAME: err_undo1
 *
 * FUNCTION: Unloads the device's device driver.  Used to back out on an
 *           error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.  The device
 *      specific routines for the various device specific config methods
 *      must not call this function.
 *
 * NOTES:
 *
 * void
 *   err_undo1( DvDr )
 *      DvDr = pointer to device driver name.
 *
 * RETURNS:
 *               None
 */

err_undo1(DvDr)
char    *DvDr;                  /* pointer to device driver name */
{
	/* unload driver */
	if (loadext(DvDr,FALSE,FALSE) == NULL) {
		DEBUG_0("cfgcommo: error unloading driver\n");
	}
	return;
}

/*
 * NAME: err_undo2
 *
 * FUNCTION: Terminates the device.  Used to back out on an error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.  The device
 *      specific routines for the various device specific config methods
 *      must not call this function.
 *
 * NOTES:
 *
 * void
 *   err_undo2( kmid ,inst_num_2 )
 *      kmid = The device's kernel module id from load function.
 *      inst_num_2 = The device's instance number
 *
 * RETURNS:
 *               None
 */

err_undo2(kmid,inst_num_2)
mid_t   kmid;                  /* The device's kmid */
int     inst_num_2;             /* The device's instance number */
{
	struct  cfg_kmod cfg_k;         /* sysconfig command structure */
	ndd_config_t    ndd_config;     /* devices cfg init struct */

	/* terminate device */
	cfg_k.kmid = kmid;
	cfg_k.cmd = CFG_TERM;
	ndd_config.seq_number = inst_num_2;
	cfg_k.mdiptr = (char *) &ndd_config;
	cfg_k.mdilen = sizeof (struct ndd_config);

	if (sysconfig(SYS_CFGKMOD,&cfg_k,sizeof(struct cfg_kmod)) == -1) {
		DEBUG_0("cfgcommo: error unconfiguring device\n");
	}

	return;
}

/*
 * NAME: err_undo3
 *
 * FUNCTION: Terminates the device and unloads the driver, if the device has
 *           a driver.  If the device does not have a driver, it simply
 *           returns.  This routine is used to back out on errors that
 *           occur while processing VPD.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.  The device
 *      specific routines for the various device specific config methods
 *      must not call this function.
 *
 * NOTES:
 *
 * void
 *   err_undo3( DvDr , kmid, inst_num_3 )
 *      DvDr  = pointer to device driver name.
 *      kmid = The device's kernel module id from load function.
 *      inst_num_3 = instance number of device
 *
 * RETURNS:
 *               None
 */

err_undo3(DvDr, kmid, inst_num_3)
char    *DvDr;                  /* pointer to device driver name */
mid_t   kmid;                   /* the device's kmid */
int     inst_num_3;               /* instance number in undo3 routine */
{
	odm_close_class(CuVPD_CLASS);   /* make sure CuVPD is closed */

	if (strcmp(DvDr, "") != 0) {    /* If device has a driver then ... */
		err_undo2(kmid,inst_num_3);       /* terminate device */
		err_undo1(DvDr);        /* unload device driver */
	}
	return;
}
