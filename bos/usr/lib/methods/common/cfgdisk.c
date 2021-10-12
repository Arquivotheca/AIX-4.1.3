static char sccsid[] = "@(#)50  1.17  src/bos/usr/lib/methods/common/cfgdisk.c, cfgmethods, bos412, 9446B 10/28/94 10:46:56";
/*
 * COMPONENT_NAME: (CFGMETHODS) cfgdisk.c - Disk Config Method Code
 *
 * FUNCTIONS: main(), err_exit(), err_undo1(), err_undo2(), err_undo3()
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

/* interface:
   cfgdisk -l <logical_name> [-<1|2>]
*/


/* header files needed for compilation */
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgodm.h>
#include <sys/bootrecord.h>
#include <cf.h>
#include <errno.h>
#include <sys/cfgdb.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/devinfo.h>

/* Local header files */
#include "cfgdebug.h"

/* external functions */
extern	long	genmajor();
extern	int	finddisk();
extern	int	clear_pvid();

int	get_pvidstr();

#define NULLPVID "00000000000000000000000000000000"

/* main function code */
main(argc, argv, envp)
int	argc;
char	*argv[];
char	*envp[];

{
extern  int     optind;         /* for getopt function */
extern  char    *optarg;        /* for getopt function */

	char	*logical_name;		/* logical name to configure */
	char	*phase1, *phase2;	/* ipl phase flags */
	char	sstring[256];		/* search criteria pointer */
	char    vpd[VPDSIZE];           /* vpd data */

	struct Class *cusdev;		/* customized devices class ptr */
	struct Class *cusvpd;           /* customized vpd class ptr */

	struct CuAt cuattr;		/* customized device attribute      */
					/* storage 			    */
	struct CuDv cusobj;		/* customized device object storage */
	struct PdAt pdattr;		/* customized device attribute      */
					/* storage 			    */
	struct PdDv preobj;		/* predefined device object storage */
	struct CuDv parobj;		/* customized device object storage */
	struct CuVPD vpdobj;            /* customized vpd object */
	char	pvid_attr[33];		/* pvid attribute from CuAt or Null */

	struct cfg_dd cfg;		/* sysconfig command structure */
	mid_t	kmid;			/* module id from loader */
	int     majorno;                /* major number assigned to device */
	int     minorno;                /* minor number assigned to device */
	long    *minor_list;            /* list returned by getminor */
	int     how_many;               /* number of minors in list */
	dev_t	devno;			/* device number for config_dd */
	char	devname[64];		/* Used to link to /dev/ipldevice */

	int	ipl_phase;		/* ipl phase: 0=run,1=phase1,2=phase2 */
	int	rc;			/* return codes go here */
	int     errflg,c;               /* used in parsing parameters   */
	int	led_no;			/* Displayable LED Value	*/

	/*****                                                          */
	/***** Parse Parameters                                         */
	/*****                                                          */
	ipl_phase = RUNTIME_CFG;
	errflg = 0;
	logical_name = NULL;

	while ((c = getopt(argc,argv,"l:12")) != EOF) {
		switch (c) {
		case 'l':
			if (logical_name != NULL)
				errflg++;
			logical_name = optarg;
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
		DEBUG_0("cfgdisk: command line error\n")
		exit(E_ARGS);
	}

	/*****                                                          */
	/***** Validate Parameters                                      */
	/*****                                                          */
	/* logical name must be specified */
	if (logical_name == NULL) {
		DEBUG_0("cfgdisk: logical name must be specified\n")
		exit(E_LNAME);
	}

	DEBUG_1 ("Configuring device: %s\n",logical_name)

	/* start up odm */
	if ((rc = odm_initialize())==-1) {
		/* initialization failed */
		DEBUG_0("cfgdisk: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}
	DEBUG_0 ("ODM initialized\n")

	/* open customized devices object class */
	if ((cusdev = odm_open_class(CuDv_CLASS)) == NULL) {
		DEBUG_0("cfgdisk: open class CuDv failed\n")
		err_exit(E_ODMOPEN);
	}

	/* search for customized object with this logical name */
	sprintf(sstring, "name = '%s'", logical_name);
	rc = (int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* No CuDv object with this name */
		DEBUG_1("cfgdisk: failed to find CuDv object for %s\n", logical_name)
		err_exit(E_NOCuDv);
	}
	else if (rc==-1) {
		/* ODM failure */
		DEBUG_0("cfgdisk: ODM failure getting CuDv object")
		err_exit(E_ODMGET);
	}

	/* get predefined device object for this logical name */
	sprintf(sstring, "uniquetype = '%s'", cusobj.PdDvLn_Lvalue);
	rc = (int)odm_get_first(PdDv_CLASS, sstring, &preobj);
	if (rc==0) {
		/* No PdDv object for this device */
		DEBUG_0("cfgdisk: failed to find PdDv object for this device\n")
		err_exit(E_NOPdDv);
	}
	else if (rc==-1) {
		/* ODM failure */
		DEBUG_0("cfgdisk: ODM failure getting PdDv object")
		err_exit(E_ODMGET);
	}

	/****************************************************************
	  If this device is being configured during an ipl phase, then
	  display this device's LED value on the system LEDs.
	  ****************************************************************/
	if (ipl_phase != RUNTIME_CFG) {
		led_no = (int)preobj.led;
		if(led_no == 0) {
			/* No LED number in PdDv so check for type Z */
			/* attribute 				     */

			sprintf(sstring,
			   "name=%s AND type=Z AND attribute=led",
			    cusobj.name);

			rc = (int)odm_get_first(CuAt_CLASS,sstring,&cuattr);
			if(rc != 0 && rc != -1)
				led_no = (int)strtoul(cuattr.value,NULL,0);


			/* if PdDv class of led_no is STILL 0, look for */
			/* PdAt attr 					*/

			if(led_no == 0) {
				sprintf(sstring,
			       "uniquetype=%s AND type=Z AND attribute=led",
				preobj.uniquetype);

				rc = (int)odm_get_first(PdAt_CLASS,sstring,
							&pdattr);
				if(rc != 0 && rc != -1)
					led_no = (int)strtoul(pdattr.deflt,
							     NULL,0);
			}

		
		} 
		setleds(led_no);
	}
	/******************************************************************
	  Check to see if the device is already configured (AVAILABLE).
	  We actually go about the business of configuring the device
	  only if the device is not configured yet. Configuring the
	  device in this case refers to the process of checking parent
	  and sibling status, checking for attribute consistency, build-
	  ing a DDS, loading the driver, etc...
	  ******************************************************************/

	if (cusobj.status == DEFINED) {

		/* get the device's parent object */
		sprintf(sstring, "name = '%s'", cusobj.parent);
		rc = (int)odm_get_first(cusdev,sstring,&parobj);
		if (rc==0) {
			/* Parent device not in CuDv */
			DEBUG_0("cfgdisk: no parent CuDv object\n")
			err_exit(E_NOCuDvPARENT);
		} else if (rc==-1) {
			/* ODM failure */
			DEBUG_0("ODM error getting parent CuDv object\n")
			err_exit(E_ODMGET);
		}

		/* Get pvid attribute from CuAt database.  Will use it to
		   find disk at runtime and later in check_pvid(). */
		rc = get_pvid_attr(&cusobj, pvid_attr);
		if (rc) {
			err_exit(rc);
		}

		/* if this is a run-time configuration we want to call
		   finddisk to track down the current location of the disk. */
		if (ipl_phase == RUNTIME_CFG) {

			/* see if disk is where the database says it is */
			rc = disk_present(&cusobj,&preobj,&parobj,pvid_attr);
			if (rc > 0) {
				/* Had an error of a type for which it does not
				   do any good to call finddisk, i.e. E_ODMGET.
				   For badisk, it could also be becaulse
				   parent is not available. */
				err_exit(rc);
			}

			if (rc == -1) {
				/* Call finddisk to try and find the disk */
				DEBUG_0("cfgdisk: calling finddisk()\n")
				if ((rc=finddisk(&cusobj,&parobj))==-1) {
					/* can't find the disk. */
					DEBUG_0("finddisk() did not find it\n")
					err_exit(E_NODETECT);
				} else if (rc != 0) {
					/* had an error while trying to find
					   the disk in the system */
					DEBUG_0("finddisk() failed\n")
					err_exit(rc);
				}

				/* We found the disk! Refetch this disk's CuDv
				   and its parent's CuDv so we have correct
				   copies, since finddisk may have corrected
				   connwhere and even parent name. */

				sprintf(sstring, "name = '%s'", logical_name);
				rc = (int)odm_get_first(cusdev,sstring,&cusobj);
				if (rc==0) {
					/* No CuDv object with this name */
					err_exit(E_NOCuDv);
				}
				else if (rc==-1) {
					/* ODM failure */
					err_exit(E_ODMGET);
				}

				sprintf(sstring, "name = '%s'", cusobj.parent);
				rc = (int)odm_get_first(cusdev,sstring,&parobj);
				if (rc==0) {
					/* Parent device not in CuDv */
					err_exit(E_NOCuDvPARENT);
				} else if (rc==-1) {
					/* ODM failure */
					err_exit(E_ODMGET);
				}
			}
		}

		/* OK to go ahead and configure the device */

		/* call loadext to load the device driver */
		if ((cfg.kmid = loadext(preobj.DvDr, TRUE, FALSE)) == NULL) {
			/* error loading device driver */
			DEBUG_1("cfgdisk: error loading driver %s\n", preobj.DvDr)
			err_exit(E_LOADEXT);
		}

		/* get major number      */
		DEBUG_0("cfgdisk: Calling genmajor()\n")
		if ((majorno = genmajor(preobj.DvDr)) == -1) {
			DEBUG_0("cfgdisk: error generating major number")
			err_undo1(preobj.DvDr);
			err_exit(E_MAJORNO);
		}
		DEBUG_1("cfgdisk: Returned major number: %d\n",majorno)

		/* get minor number      */
		DEBUG_0("cfgdisk: Calling getminor()\n")
		minor_list = getminor(majorno,&how_many,logical_name);
		if (minor_list == NULL || how_many == 0) {
			DEBUG_0("cfgdisk: Calling generate_minor()\n")
			rc = generate_minor(logical_name, majorno, &minorno);
			if (rc) {
				DEBUG_1("cfgdisk: error generating minor number, rc=%d\n",rc)
				/* First make sure any minors that might have */
				/* been assigned are cleaned up */
				reldevno(logical_name, TRUE);
				err_undo1(preobj.DvDr);
				if ( rc < 0 || rc > 255)
					rc = E_MINORNO;
				err_exit(rc);
			}
			DEBUG_0("cfgdisk: Returned from generate_minor()\n")
		}
		else
			minorno = *minor_list;
		DEBUG_1("cfgdisk: minor number: %d\n",minorno)

		/* create devno for this device */
		cfg.devno = makedev(majorno, minorno);

		/* make special files      */
		DEBUG_0("cfgdisk: Calling make_special_files()\n")
		rc = make_special_files(logical_name, cfg.devno);
		if (rc) {
			/* error making special files */
			DEBUG_1("cfgdisk: error making special file(s), rc=%d\n",rc)
			err_undo1(preobj.DvDr);
			if ( rc < 0 || rc > 255)
				rc = E_MKSPECIAL;
			err_exit(rc);
		}
		DEBUG_0("cfgdisk: Returned from make_special_files()\n")

		/* build the DDS  */
		DEBUG_0("cfgdisk: Calling build_dds()\n")
		rc = build_dds(&cusobj, &parobj, &cfg.ddsptr, &cfg.ddslen);
		if (rc) {
			/* error building dds */
			DEBUG_1("cfgdisk: error building dds, rc=%d\n",rc)
			err_undo1(preobj.DvDr);
			if ( rc < 0 || rc > 255)
				rc = E_DDS;
			err_exit(rc);
		}
		DEBUG_0("cfgdisk: Returned from build_dds()\n")

		/* call sysconfig to pass DDS to driver */
		DEBUG_0("cfgdisk: Pass DDS to driver via sysconfig()\n")
		cfg.cmd = CFG_INIT;
		if (sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd )) == -1) {
			/* error configuring device */
			DEBUG_0("cfgdisk: error configuring device\n")
			err_undo1(preobj.DvDr);
			err_exit(E_CFGINIT);
		}

		/* if device has VPD data then go get it */
		if (preobj.has_vpd == TRUE) {
			/* get VPD for this device */
			vpd[0] = '\0';
			DEBUG_0("cfgdisk: Calling query_vpd()\n")
			rc = query_vpd(&cusobj, cfg.kmid, cfg.devno, vpd);
			if (rc) {
				/* failed to get VPD */
				DEBUG_1("cfgdisk: error getting VPD, rc=%d\n",rc)
				err_undo3(preobj.DvDr, cfg.devno);
				if ( rc < 0 || rc > 255)
					rc = E_VPD;
				err_exit(rc);
			}
			DEBUG_0("cfgdisk: Returned from query_vpd()\n")

			/* open customized vpd object class */
			if ((cusvpd = odm_open_class(CuVPD_CLASS)) == NULL) {
				DEBUG_0("cfgdisk: open class CuVPD failed")
				err_undo3(preobj.DvDr, cfg.devno);
				err_exit(E_ODMOPEN);
			}

			/* search for customized vpd object with this
			   logical name */
			sprintf(sstring, "name = '%s' and vpd_type = '%d'",
				logical_name,HW_VPD);
			rc = (int)odm_get_first(cusvpd,sstring,&vpdobj);
			if (rc==-1) {
				/* ODM failure */
				DEBUG_0(" ODM failure getting CuVPD object")
				err_undo3(preobj.DvDr, cfg.devno);
				err_exit(E_ODMGET);
			}
			if (rc==0) {
				/* need to add vpd object */
				DEBUG_0("Adding new VPD object\n")
				strcpy(vpdobj.name,logical_name);
				vpdobj.vpd_type = HW_VPD;
				memcpy(vpdobj.vpd,vpd,VPDSIZE);
				if (odm_add_obj(cusvpd,&vpdobj) ==-1) {
					DEBUG_0("Failure adding CuVPD obj\n")
					err_undo3(preobj.DvDr, cfg.devno);
					err_exit(E_ODMADD);
				}
				DEBUG_0("Successfully added new VPD object\n")
			} else {
				/* see if vpd object needs to be updated */
				if (memcmp(vpdobj.vpd,vpd, VPDSIZE)) {
					DEBUG_0("Updating VPD object\n")
					memcpy(vpdobj.vpd,vpd,VPDSIZE);
					if (odm_change_obj(cusvpd,&vpdobj)==-1){
						DEBUG_0("ODM failure updating CuVPD object")
						err_undo3(preobj.DvDr, cfg.devno);
						err_exit(E_ODMUPDATE);
					}
					DEBUG_0("Updated CuVPD object\n")
				}
			}
			/* close customized vpd object class */
			if (odm_close_class(CuVPD_CLASS) ==-1) {
				DEBUG_0("cfgdisk: error closing CuVPD object class\n")
				err_undo3(preobj.DvDr, cfg.devno);
				err_exit(E_ODMCLOSE);
			}
		}

		/* check PVID information */
		DEBUG_1("calling check_pvid(%s)\n", logical_name)
		if ((rc=check_pvid(&cusobj,pvid_attr,ipl_phase))!=0) {
			/* error doing this pvid stuff */
			DEBUG_2("check_pvid(%s) failed,rc=%d\n",
							logical_name,rc)
			err_undo3(preobj.DvDr, cfg.devno);
			err_exit(rc);
		}

	       /* 
		* Update customized device object to indicate
		* it is available and it is the SAME.  Only 
		* indicate SAME if it existed before.
		*/

		cusobj.status = AVAILABLE;
		if (cusobj.chgstatus == MISSING)
		{
		    cusobj.chgstatus = SAME ;
		}
		if (odm_change_obj(cusdev, &cusobj) ==-1) {
			/* ODM failure */
			DEBUG_0("cfgdisk: ODM failure updating CuDv object\n")
			err_undo3(preobj.DvDr, cfg.devno);
			err_exit(E_ODMUPDATE);
		}

	} /* end if (device is DEFINED) then ... */

	/* close customized device object class */
	if (odm_close_class(cusdev) ==-1) {
		DEBUG_0("cfgdisk: error closing CuDv object class\n")
		err_exit(E_ODMCLOSE);
	}

	odm_terminate();
	exit(0);

}

int
check_pvid(disk, pvid_attr, ipl_phase)
struct	CuDv *disk;		/* this disk's CuDv object */
char	*pvid_attr;		/* pvid attr value, else NULL */
int	ipl_phase;		/* ipl phase: 0=run,1=phase1,2=phase2 */
{
/*
 *************************************************************************
 * NAME     : check PV Id
 * FUNCTION : keep the ODM database and the disk in sync with respect to
 *            the PV IDs.  Will generate or clear PVIDs on disks based
 *            on the "pv" attribute in the ODM database.
 * NOTES    :
 *   -  Upon entry to this routine, 1 of the following conditions is
 *      true :
 *        a) There is a PV ID on the disk AND it matches the one in the
 *           database - this condition is usually the case; finddisk
 *           matches disk names based on PV IDs.
 *        b) There is no PV ID on the disk AND there is not one in the
 *           database - this condition occurs for new disks being
 *           configured into the system for the 1st time.
 *        c) PV ID exists on the disk BUT there is not one in the
 *           database - this condition occurs for disks that have 
 *           been configured into some system, but are being newly
 *           added to this system.
 *      One of these conditions being TRUE is VITAL to the checks that
 *      are made below.
 *      NOTE that only in condition "a" does the ODM database have
 *           a PVID stored.
 *      NOTE that the condition of a PVID in the database and not one
 *           on disk CANNOT exist at this point.  The reason that it
 *           cannot exist is due to the function of finddisk.  If 
 *           there is a PVID in the database but not one on disk, 
 *           it is assumed that the disk is a different disk and a
 *           new hdisk name/number is assigned to it; thereby breaking
 *           the association with the PVID in the database.
 *   -  Due to a timing problem found via HANFS, there are situations
 *      when finddisk was able to read the PVID from the disk and 
 *      match it with the database to get the name, but when this
 *      routine attempts to read the PVID from the disk, the read
 *      fails.  This situation occurs when the other system has the
 *      disk reserved.  Condition "a" is relied upon to handle this
 *      situation and NOT null out the PVID string in the database
 *      as it used to.
 ***********************************************************************
 */  
struct	CuAt *pv_attr;
char	pvidstr[33];
char	sstr[256];
int	rc;

	/* see what the "pv" attribute is for this disk */
	if ((pv_attr = getattr(disk->name,"pv",FALSE,&rc)) == NULL) {
		/* error or didn't find it */
		DEBUG_1("check_pvid: couldn't find pv attr for %s\n",
			disk->name)
		return(E_NOATTR);
	}

	DEBUG_1("check_pvid: pv = %s\n",pv_attr->value)
	DEBUG_2("check_pvid: calling get_pvidstr(%s,%x)\n",disk->name,pvidstr)
	rc = get_pvidstr(disk,pvidstr);
	if (rc) {
		/* problem reading PVID */
		DEBUG_1("check_pvid: error reading PVID for %s\n",disk->name)
		DEBUG_0("assuming NULL pvid\n")
		pvidstr[0] = '\0';
	}

	if (!strcmp(pv_attr->value,"yes")) 
	{
	    rc = 0 ;
            /*
	     *-------------------------------------------------------
	     *  There is no need to write a new PVID if one already
	     *  exists.  If there is one in the database, given
	     *  condition "a" above, there is one on the disk 
	     *  already.
	     *-------------------------------------------------------
	     */
	    if (pvid_attr[0] == '\0')
	    {
		/* Need to make sure disk has a pvid if it doesn't already */
		if (pvidstr[0] == '\0') 
		{
			/* doesn't have one - so generate one */
			DEBUG_0("generating a new pvid\n")
			rc = pvid_to_disk(disk->name,pvidstr,0);
			DEBUG_1("chk_pvid: rtn from pvid_to_disk; rc = %x\n", 
				rc) ;
		}
		DEBUG_1("disk pvid = %s\n",pvidstr)

		/*
		 *------------------------------------------------------------
		 * If the putting of the PVID to disk failed above, then do
		 * NOT update the database.  The PVID should be resynced on
		 * the next configure (reboot).  Names will be matched by 
                 * finddisk based on scsi Id and LUN.
		 *------------------------------------------------------------
                 */
		if (rc == 0)
		{
			DEBUG_0("writing pvid attr into CuAt\n")
			(void)putpvidattr(disk->name,pvidstr);
		}
	    }

	} 
	else if (!strcmp(pv_attr->value,"clear")) 
	{
		/* Dont clear the pvid unless in runtime */
		if (ipl_phase == RUNTIME_CFG) 
		{
		    /* clear pvid on disk ; don't clear database unless
                       disk is successfully cleared.			*/
		    DEBUG_1("check_pvid: calling clear_pvid(%s)\n", disk->name)
		    
		    if (clear_pvid(disk->name,0) == 0)
		    {
			/* delete CuAt object for pvid */
			DEBUG_0("deleting CuAt pvid attribute\n")
			sprintf(sstr, "name = '%s' AND attribute = 'pvid'",
				 disk->name);
			rc = (int)odm_rm_obj(CuAt_CLASS,sstr);
			if (rc == -1) 
			{
				/* error removing object */
				DEBUG_1("check_pvid: error deleting %s\n",sstr)
				return(E_ODMDELETE);
			}
		    }
		}
	
	} 
	else if (!strcmp(pv_attr->value,"no")) 
	{
	    /*
	     *------------------------------------------------------------
	     *  This is the default value for the "pv" attribute.
	     *  We only need to update the database if the database has
	     *  no PVID stored AND there is one on the disk.
	     *------------------------------------------------------------
	     */
	    if (pvid_attr[0] == '\0' && pvidstr[0] != '\0')
	    {
		DEBUG_0("chk_pvid: updating ODM w/ PVID for 'pv=no' case\n") ;
		(void)putpvidattr(disk->name,pvidstr);
	    }
	    /* don't need to delete the pv attribute */
	    return(0);
	}

	/* remove pv attr from the database */
	sprintf(sstr,"name = '%s' AND attribute = 'pv'", disk->name);
	if (odm_rm_obj(CuAt_CLASS,sstr)==-1) {
		/* error removing object */
		DEBUG_1("check_pvid: error deleting %s\n",sstr)
		return(E_ODMDELETE);
	}

	return(0);

}


int
get_pvid_attr(cusobj,pvid_attr)
struct	CuDv *cusobj;
char	*pvid_attr;
{
	struct	CuDv *dmyobj;
	char	sstring[256];		/* search criteria pointer */
	struct	CuAt attrobj;		/* customized attribute object     */
	int	rc;

	/* see if disk object has a pvid attribute */
	sprintf(sstring,"name = '%s' AND attribute = 'pvid'", cusobj->name);
	rc = (int)odm_get_first(CuAt_CLASS,sstring,&attrobj);
	if (rc == -1) {
		DEBUG_0("get_pvid_attr: odmget err\n")
		return(E_ODMGET);
	} else if (rc == 0) {
		/* it does not have a PVID attribute so cannot find the disk
		   if it has moved. See if a disk is already AVAILABLE at this
		   at this connection location on this parent.  If there is,
		   then can not configure this disk. */
		sprintf(sstring,
			"parent = '%s' AND connwhere = '%s' AND status = %d",
			cusobj->parent,cusobj->connwhere,
			AVAILABLE);
		rc = (int)odm_get_first(CuDv_CLASS,sstring,&dmyobj);
		if (rc!=0) {
			if (rc==-1) {
				DEBUG_0("get_pvid_attr: ODM error\n")
				return(E_ODMGET);
			} else {
				/* device already here! */
				return(E_AVAILCONNECT);
			}
		}
		pvid_attr[0] = '\0';
	} else {
		strcpy(pvid_attr,attrobj.value);
	}
	return(0);
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
	struct   CuAt  sys_cuattr;
	int rc;

	/* Close any open object class */
	odm_close_class(CuDv_CLASS);
	odm_close_class(PdDv_CLASS);
	odm_close_class(CuAt_CLASS);

#ifdef _CFG_RDS
	rc = (int)odm_get_first(CuAt_CLASS, "name=sys0 AND attribute=rds_facility",
				 &sys_cuattr);
	if (rc == -1) {
		DEBUG_0("ODM error getting rds_facility attribute\n")
		return(E_ODMGET);
	} else if (rc != 0 && sys_cuattr.value[0]=='y') {
		rds_switch_off_device();
	}
#endif

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
		DEBUG_0("cfgdisk: error unloading driver\n")
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
 *   err_undo2( devno )
 *      devno = The device's devno.
 *
 * RETURNS:
 *               None
 */

err_undo2(devno)
dev_t   devno;                  /* The device's devno */
{
	struct  cfg_dd cfg;             /* sysconfig command structure */

	/* terminate device */
	cfg.devno = devno;
	cfg.kmid = (mid_t)0;
	cfg.ddsptr = (caddr_t) NULL;
	cfg.ddslen = (int)0;
	cfg.cmd = CFG_TERM;

	if (sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd)) == -1) {
		DEBUG_0("cfgdisk: error unconfiguring device\n")
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
 *   err_undo3( DvDr , devno )
 *      DvDr  = pointer to device driver name.
 *      devno = The device's devno.
 *
 * RETURNS:
 *               None
 */

err_undo3(DvDr, devno)
char    *DvDr;                  /* pointer to device driver name */
dev_t   devno;                  /* the device's devno */
{
	odm_close_class(CuVPD_CLASS);   /* make sure CuVPD is closed */
	err_undo2(devno);       /* terminate device */
	err_undo1(DvDr);        /* unload device driver */
	return;
}
