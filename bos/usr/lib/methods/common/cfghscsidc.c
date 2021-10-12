static char sccsid[] = "@(#)05  1.6.1.29  src/bos/usr/lib/methods/common/cfghscsidc.c, cfgmethods, bos41J, 9523B_all 6/6/95 14:23:51";
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: bb_disk_notrdy
 *		bb_disk_spinup
 *		def_scsi_dev
 *		default_utype
 *		define_children
 *		det_utype
 *		final_inq
 *		find_dev_in_cudv
 *              chk_update_utype
 *		clean_up_cuat
 *		det_utype_updateable
 *		found_device
 *		get_from_database
 *		get_maxlun
 *		get_pvid_from_disk
 *		initial_inq
 *		print_pkg
 *		read_scsi_pvid
 *		save_dev_info
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1,  5 Years Bull Confidential Information
 */


/* header files needed for compilation */
#include	<fcntl.h>
#include 	<stdio.h>
#include	<sys/cfgodm.h>
#include	<sys/cfgdb.h>
#include        <cf.h>
#include        <odmi.h>
#include	<sys/errno.h>
#include	<sys/devinfo.h>
#include	<sys/ioctl.h>
#include	<sys/bootrecord.h>
#include	<sys/scsi.h>
#include        "cfghscsi.h"
#include        "cfgdebug.h"


/* forward function definitions */
int	initial_inq();
int	final_inq();
char 	*det_utype();
char 	*default_utype();
void	find_dev_in_cudv();
int     chk_update_utype();
int	det_utype_updateable();
void    clean_up_cuat();
void	def_scsi_dev();
void	found_device();
void    get_maxlun ();
void	save_dev_info();
int	get_from_database();
int	bb_disk_spinup();
int	bb_disk_notrdy();
void	get_pvid_from_disk();


/*	extern declarations	*/
extern	int	errno;
extern	int	Dflag;

extern  char    *get_scsi_utype(uchar *,struct PdAt*, int,struct mna*, 
			int,int,uchar, uchar);
extern  char    *strtok(char *,char *);


/*      static declarations     */
struct  mna    *mn_attr;		/* pointer to model name attributes */
struct  mla    *ml_attr;		/* pointer to max lun attributes */
struct  PdAt   *pdobj_ptr;		/* pointer to list of model_map attr*/
struct  cust_device     *cust_dev;      /* ptr to cust device info */
struct  real_device     *real_dev;      /* ptr to real device info, these are
					   the devices actually found to be
					   attached */
int     num_mn;                         /* number of model name attributes */
int     num_ml;                         /* number of max lun attributes */
int     num_cd;                         /* number of cust device objects */
int     num_rd;                         /* number of found real devices */
int	num_mma;			/* number of model_map attributes */
uchar   sidlunmap[MAX_ID][MAX_LUN];     /* one entry for each sid/lun */
uchar   maxlun[MAX_ID];                 /* the max lun value for each sid */
int	realdevorder[MAX_ID][MAX_LUN];	/* one entry for each real device to */
					/* keep order correct		*/	
int     last_start_time;                /* when spin up of last disk started*/
int     spinup_wait;			/* time between disk spin ups */
char    *pname;                         /* pointer to adapter name (parent) */
char    putype[UNIQUESIZE];		/* adapter's uniquetype */
char    plocation[LOCSIZE];		/* adapter's location code */
struct  Class   *cusdev;                /* for accessing CuDv */
int     tm_enable;                      /* for defining target mode children */
char    device_state = AVAILABLE;       /* for determining the previous state*/
                                        /* of this device to determine if a  */
                                        /* microcode download is necessary   */
int	initial_inq_error = 0;		/* set to 1 if a device is already opened */
int	allpkg = 0;			/* packaging flag 		*/

#define	MODEL_TYPE_BITS	0x03
#define	RACK		0x02
#define	DEVPKG_PREFIX	"devices"		/* device package prefix */
#define NO_DEVICE_AT_LUN 0x20



/*
 * NAME   : def_scsi_children
 *
 * FUNCTION : this routine detects and defines devices attached to the
 *       scsi adapter. Supported devices are disks, tapes, cdroms,
 *       and target mode.
 *
 * EXECUTION ENVIRONMENT:
 *      This function operates as a device dependent subroutine
 *      called by the generic configure method for all devices.
 *
 * DATA STRUCTURES:
 *      bb_inq  : structure to hold inquiry_data.
 *
 * INPUTS : logical_name,ipl_phase,num_scsi_ids
 *
 * RETURNS: Returns 0 if success
 *
 * RECOVERY OPERATION:
 *
 */

int
def_scsi_children (char *lname,     /* logical name */
		   int ipl_phase,   /* ipl phase */
		   int numsids,	    /* number of SCSI IDs */
		   int numluns)	    /* number of SCSI LUNs */
{
	char    adap_dev[32];           /* adapter device /dev name */
	int     adapter;                /* file descriptor for adapter */
	uchar   adapter_sid;            /* sid of the adapter */
	uchar   sid,lun;                /* current sid/lun being checked */
	int     done;                   /* loop control variable */
	int     rc;                     /* for return codes */
	struct  devinfo scsi_info;      /* for IOCINFO ioctl */
        struct  CuAt cuattr,sys_cuattr; /* for getting CuAt attribute */
	struct  CuAt par_cuat;          /* for getting CuAt attribute */
        struct  CuAt *cuat;		/* for getting CuAt attribute */
	int	modcode;		/* model code attribute value */
	int     cnt;                    /* used in getattr call */
	char	class[CLASSIZE];	/* class of uniquetype for pkg printing */
	char    ss[255];
	int	j;			/* loop control */

	DEBUG_0("entering def_scsi_children routine\n")

	/* Get the packaging environment variable */
   	if (!strcmp(getenv("DEV_PKGNAME"),"ALL"))
		allpkg = 1;

	/* save pointer to adapter name in static storage so the */
	/* other routines have access to it */
	pname = lname;

	/* open adapter device */
	sprintf(adap_dev,"/dev/%s",lname);
	if((adapter = open(adap_dev,O_RDWR)) < 0){
	    DEBUG_1("def_scsi_children:can not open %s \n",adap_dev)
	    return(E_DEVACCESS);
	}

	/* get configured card_scsi_id using IOCINFO ioctl cmd */
	if((ioctl(adapter,IOCINFO,&scsi_info) < 0) ||
	   (scsi_info.devtype != DD_BUS) ||
	   (scsi_info.devsubtype != DS_SCSI)){
		DEBUG_1("def_scsi_children: can not get scsi_id for %s\n",lname)
		return(E_DEVACCESS);
	}

	adapter_sid = scsi_info.un.scsi.card_scsi_id;
	DEBUG_2("def_scsi_children: scsi_id of %s = %d\n",lname,adapter_sid)


	/* open CuDv object class and keep open */
	if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1){
		DEBUG_0("def_scsi_children: can't open CuDv class\n")
		return(E_ODMOPEN);
	}

	/* get model_name and maxlun attributes from PdAt object class */
	/* get customized device information for all scsi devices in CuDv */
	/* get pvid attributes for scsi disk devices in CuDv */
	rc = get_from_database();
	if (rc) {               /* errors are returned only for fatal */
		DEBUG_0("def_scsi_children: fatal ODM error - returning\n")
		return(rc);     /* ODM errors */
	}

        /* get the value of the tme attribute from the database and use    */
        /* a static variable global to this module to reflect its state    */
        /* thus avoiding making multiple database accesses for one attribute */
        tm_enable = FALSE;		/* Initally assume no tme attr */
	if ((cuat = getattr (lname, "tme", FALSE, &cnt)) != NULL) {
	    DEBUG_1("def_scsi_children: tme = %s \n",cuat->value)
	    if (strcmp (cuat->value, "yes") == 0)
		tm_enable = TRUE;	/* tme attr exists and is yes*/
	}

	/* Set time delay to wait between disk spin ups.  If run time   */
	/* assume external disks being configured so delay=0, otherwise */
	/* set delay to 0 for rack mount models and 10 seconds for all  */
	/* other models. */
	if (ipl_phase == RUNTIME_CFG) {
		spinup_wait = 0;
	} else {
		spinup_wait = 10;	/* Initially assume 10 sec wait */
		rc = (int)odm_get_first(CuAt_CLASS,
				"name=sys0 AND attribute=modelcode", &cuattr);
		if (rc == -1) {
			DEBUG_0("ODM error getting modelcode attribute\n")
			return(E_ODMGET);
		} else if (rc != 0) {
			modcode = (int)strtoul(cuattr.value,NULL,0);
			DEBUG_1("model code = %d\n",modcode)
			if ((modcode & MODEL_TYPE_BITS) == RACK)
				spinup_wait = 0;	/* 0 sec on racks */
		}
	}
	DEBUG_1("Disk spin up delay = %d\n",spinup_wait)
#ifdef _CFG_RDS
        rc = (int)odm_get_first(CuAt_CLASS, "name=sys0 AND attribute=rds_facility", &sys_cuattr);
        if (rc == -1) {
                DEBUG_0("ODM error getting rds_facility attribute\n")
                return(E_ODMGET);
	} else if (rc != 0 && sys_cuattr.value[0]=='y') {
                rc = rds_find_device(lname, adapter, numsids, numluns);
                if (rc != 0) {
                        DEBUG_0("def_scsi_children: can not find children devices\n")
                }
        }
#endif
	/* initialize the sid/lun map */
	for(sid = 0; sid < numsids; sid++) {
		maxlun[sid] = numluns - 1;
		for (lun = 0; lun <= maxlun[sid]; lun++) {
			sidlunmap[sid][lun] = UNKNOWN;
			/* initialize the real device order map */
			/* to designate no devices detected yet */
			realdevorder[sid][lun] = NOT_IN_USE;
		}	
	}

	/* adapter device is already known */
	maxlun[adapter_sid] = 0;
	sidlunmap[adapter_sid][0] = DEVICE;

	/* malloc space for keeping track of actual scsi devices we find */
	num_rd = 0;             /* number of devices actually found */
	real_dev = (struct real_device *)malloc(RD_BASE*sizeof(struct real_device));

	last_start_time = 0;    /* init first disk's spinup time at t=0 */

	/* discover devices on all sid/lun pairs */
	done = FALSE;
	while (!done) {
		done = TRUE;
		for (sid = 0; sid < numsids; sid++) {
			for (lun = 0; lun <= maxlun[sid]; lun++) {
				if (sidlunmap[sid][lun] == UNKNOWN) {

					/* NOTE: the maxlun[sid] value */
					/* can be modified by the      */
					/* initial_inq routine.        */
					rc = initial_inq(adapter,sid,lun);
					sidlunmap[sid][lun] = rc;
					if (rc == NEWDISK) {
						done = FALSE;
						break;
					}
				}
				else if (sidlunmap[sid][lun] == NEWDISK) {
					rc = final_inq(adapter,sid,lun);
					sidlunmap[sid][lun] = rc;
				}
			}
		}
	}

	/*** free up malloc'ed memory for attribute info */
	free(mn_attr);
	free(ml_attr);

	/* match up real devices with corresponding CuDv info */
	/* and define new ones if necessary */
	find_dev_in_cudv (numsids);


	if (allpkg) 
	{
		/* If the allpkg flag is set, then we must guarantee    */
		/* that the SCSI tape package is installed.  This is    */
		/* necessary for all machines with a SCSI adapter,      */
		/* because the user may later need to install additional*/
		/* packages from tape, even though no tape drive is     */
		/* currently detected.					*/
		fprintf(stdout, ":%s.scsi.tape ", DEVPKG_PREFIX);


		/* If we found any devices that were already opened, we	*/
		/* will not have correctly printed out their package	*/
		/* names.  So, at this point, if there was an error in	*/
		/* accessing a device and if the allpkg flag is set,	*/
		/* we need to loop through all customized devices and	*/
		/* for all devices that are available and that are	*/
		/* children of our device, we need to print out the	*/
		/* package name. 					*/
		if (initial_inq_error)
		{
			for (j=0; j<num_cd; j++)
			{
				if (!strcmp(cust_dev[j].cudv.parent,lname) && 
				    cust_dev[j].cudv.status == AVAILABLE)
			        {
					DEBUG_1("uniquetype for printing package is %s\n",
						cust_dev[j].cudv.PdDvLn_Lvalue );

					/* Possibly choose package based on class of uniquetype */
					sscanf(cust_dev[j].cudv.PdDvLn_Lvalue, "%[^/] ", &class);

					/* Check for 7135 using entire uniquetype */
					if (strcmp(cust_dev[j].cudv.PdDvLn_Lvalue,"array/scsi/dac7135")==0)
						fprintf(stdout, ":%s.scsi.scarray ", DEVPKG_PREFIX);

					else if ((strcmp(class, "disk")==0)   ||
						 (strcmp(class, "cdrom")==0)  ||
						 (strcmp(class, "rwopt")==0))
						fprintf(stdout, ":%s.scsi.disk ", DEVPKG_PREFIX);
					else if (strcmp(class, "tape")==0)
						fprintf(stdout, ":%s.scsi.tape ", DEVPKG_PREFIX);
					else if (strcmp(class, "tmscsi")==0)
						fprintf(stdout, ":%s.scsi.tm ", DEVPKG_PREFIX);
					else
						fprintf(stdout, ":%s.scsi ", DEVPKG_PREFIX);

				}
			}
		}

	}

	/* free up malloc'ed memory for customized database info */
	/* and real device info */
	free(cust_dev);
	free(real_dev);

	/* close adapter and return successful */
	close(adapter);

	/* close CuDv object class */
	odm_close_class(cusdev);
	return(0);
}

/*
 * NAME   : initial_inq
 *
 * FUNCTION :
 *      This function performs the initial inquiry to a scsi id and lun
 *      to determine
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 *
 * INPUT  : adapter file descriptor, scsi Id and lun.
 *
 * RETURNS:
 *
 */

int
initial_inq(adapter,sid,lun)
int     adapter;                        /* file descriptor */
uchar   sid,lun;                        /* scsi id and lun */
{
	char    pvid[PVIDSIZE];		/* area to hold a pvid */
	char    *tmp_ptr;               /* local character pointer */
	char    connect[8];             /* area to hold scsi device
					   connection string */
	char    *utype;                 /* device unique type */
	uchar   state;                  /* device state of this sid/lun */
	int     current_time;           /* for holding current time */
	int     elapsed_time;           /* time since last disk spin up */
	int     rc;                     /* for return codes */
	int     i;                      /* loop variable */
	struct  inqry_data bb_inq;      /* area to hold device inquiry data */
	int	ansi_scsi_lvl;		/* for the ANSI SCSI LEVEL */
	int	pg=NO_PAGE; 		/* default page to get  */
	int	error;			/* Error from issue_scsi_inquiry */
	
	/* start this sid/lun */
	DEBUG_2("starting sid%d lun%d\n", sid, lun)
	if (ioctl(adapter,SCIOSTART,IDLUN(sid,lun))!=0) {
		DEBUG_2("can not start sid=%d, lun=%d\n",sid,lun)
		/* Can't start this sid/lun, most likely due to the device */
		/* already being configured and in opened. Look in the     */
		/* saved CuDv info to see if a device is already config'ed */
		/* on this adapter at this particular sid and lun.  If     */
		/* there is one, see if it has a maxlun attribute and      */
		/* adjust as necessary.                                    */

		/* set flag to 1 to indicate that a busy device was found  */
		/* so that the device package names will be printed	   */
		initial_inq_error = 1;

		sprintf(connect,"%d,%d",sid,lun);
		for (i=0; i<num_cd; i++) {
			if (!strcmp(pname,cust_dev[i].cudv.parent) &&
			    !strcmp(connect,cust_dev[i].cudv.connwhere) &&
			    (cust_dev[i].cudv.status == AVAILABLE)) {
				DEBUG_0("device already available\n")
				tmp_ptr = (char *)&cust_dev[i].cudv.PdDvLn_Lvalue;
				/* adjust maxlun value as necessary */
				get_maxlun (tmp_ptr, sid);
				break;
			}
		}
		return(EMPTY);
	}

	/* inquiry at this sid/lun */
	DEBUG_2("initial inquiry for sid=%d and lun=%d\n",sid,lun)

	rc = issue_scsi_inquiry(adapter,&bb_inq,sid,lun,pg,&error);
        /* the mask NO_DEVICE_AT_LUN indicates will indicate if this is a */
        /* valid lun for this id as defined in the SCSI-2 standard.  This */
        /* code will ignore the indication that there is not a device at  */
        /* this lun if the inquiry data indicates that this is an IBM 7135*/
        /* disk array so that the RAID controller that is addressed       */
        /* at lun 0, will still be configured even if there is no RAID    */
        /* device at lun 0. */
	if ((rc == 0) && (bb_inq.pdevtype & NO_DEVICE_AT_LUN) && 
            /* special case for the 7135 RAID array */
	    (!((bb_inq.product[0] == '7') && (bb_inq.product[1] == '1') &&
	    (bb_inq.product[2] == '3') && (bb_inq.product[3] == '5') &&
	    (bb_inq.reserved[0] == 0x1f))) &&
            /* special case for the Bull RAID array */
            (!((((bb_inq.product[0] == 'D') && (bb_inq.product[1] == 'I') &&
            (bb_inq.product[2] == 'S') && (bb_inq.product[3] == 'K')) ||
            ((bb_inq.product[0] == 'R') && (bb_inq.product[1] == 'A') &&
            (bb_inq.product[2] == 'I') && (bb_inq.product[3] == 'D'))) &&
            ((bb_inq.vendor[0] == 'D') && (bb_inq.vendor[1] == 'G') &&
             (bb_inq.vendor[2] == 'C'))))) {
		


		/* Determine the ANSI SCSI level of this drive */

		ansi_scsi_lvl = bb_inq.versions & SCSI_ANSI_MASK;
		if ((ansi_scsi_lvl <= 0x2) && 
		    (maxlun[sid] > (MAX_SCSI_2_LUN-1))) {
			/* If this device is SCSI-2 or lower, then the */
			/* device can support at most MAX_SCSI_2_LUN   */
			/* luns. So make  sure we are not trying to do */
			/* inquiries for luns beyond MAX_SCSI_2_LUN    */

			maxlun[sid] = MAX_SCSI_2_LUN-1;
			DEBUG_1("maxlun lowered to %d\n",maxlun[sid])

		}
		/* no device on this sid/lun */
		/* mark it empty and go to next lun */
		DEBUG_2("no device at sid=%d lun=%d\n",sid,lun)
		state = EMPTY;
			
	} 
	else if (rc == 0) {

		/* Determine the ANSI SCSI level of this drive */

		ansi_scsi_lvl = bb_inq.versions & SCSI_ANSI_MASK;
		if ((ansi_scsi_lvl <= 0x2) && 
		    (maxlun[sid] > (MAX_SCSI_2_LUN-1))) {
			/* If this device is SCSI-2 or lower, then the */
			/* device can support at most MAX_SCSI_2_LUN   */
			/* luns. So make  sure we are not trying to do */
			/* inquiries for luns beyond MAX_SCSI_2_LUN    */

			maxlun[sid] = MAX_SCSI_2_LUN-1;
			DEBUG_1("maxlun lowered to %d\n",maxlun[sid])

		}


		/* see if the device is a disk */

		if (bb_inq.pdevtype == SCSI_DISK) {
			/* see if disk is spun up */
			if (!bb_disk_notrdy(adapter,sid,lun,1)) {
				DEBUG_0("its a disk and already spinning\n")


				if (issue_scsi_inquiry(adapter,&bb_inq,
						       sid,lun,pg,
						       &error) != 0) {
					/* error--can't talk to the device! */
					DEBUG_2("inq failed sid=%d lun=%d\n",
						sid,lun)
				        /* need to indicate default disk */
				        utype = default_utype(DEFLT_DISK_TYP);
				} else if (bb_inq.pdevtype != SCSI_DISK) {
					/* error--can't talk to the device! */
					DEBUG_2("second inq failed sid=%d lun=%d\n",sid,lun)
				        /* need to indicate default disk */
				        utype = default_utype(DEFLT_DISK_TYP);
				} else {
					/* determine disk's unique type */
					utype = det_utype(adapter,sid,lun,
							  &bb_inq);
				}
				/* get pvid from disk */
				get_pvid_from_disk(adapter,sid,lun,pvid);

				/* save device info */
				save_dev_info(sid,lun,utype,pvid,TRUE);

				/* adjust maxlun value as necessary */
				get_maxlun(utype,sid);

				state = DEVICE;

			} else {
				DEBUG_0("its a disk, must spin up\n")

				/* get current time */
				time(&current_time);

				/* must have a 10 sec break between disks */
				elapsed_time = current_time-last_start_time;
				if (elapsed_time<spinup_wait)
					sleep(spinup_wait-elapsed_time);

				/* spin up disk */
				if (bb_disk_spinup(adapter,sid,lun)) {
					DEBUG_0("spin up failed\n")
					/* failed to spin up the disk. try  */
					/* to determine type of disk anyway.*/
					/* At worse, it will be treated as  */
					/* default type.                     */
					utype = det_utype(adapter,sid,lun,&bb_inq);

					/* Also, assume no pvid on disk.    */
					pvid[0] = '\0';

					/* save device info */
					save_dev_info(sid,lun,utype,pvid,TRUE);

					/* adjust maxlun value as necessary */
					get_maxlun(utype,sid);

					state = DEVICE;
				} else {
					/* get start time for this disk */
					time(&last_start_time);

					/* leave this sid/lun started and   */
					/* return to process next sid. This */
					/* sid/lun will be stopped after    */
					/* final inquiry and pvid is read.  */
					return(NEWDISK);
				}
			}

		} else { /* we've found some other type of device */
			/* determine device's unique type */
			DEBUG_0("its a non-disk device\n")

			utype = det_utype(adapter,sid,lun,&bb_inq);
                        DEBUG_1("utype = %s\n", utype)
                        /* if this is a target mode device and target mode  */
                        /* enable flag is false then don't define the child */
                        if((strncmp("tmscsi", utype, 6) == 0) && 
                           (!(tm_enable))) {
                           /* mark this location as empty */
                           state = EMPTY; 
                       
                           /* set max lun to zero so we don't bother with */
                           /* the other luns on this sid                  */
                           maxlun[sid] = 0;
                        }
                        else {
       			   pvid[0] = '\0';	/* no pvid for non-disks */
			   /*
			    * dev_is_disk is set to FALSE so find_dev_in_cudv 
			    * will not look at pvid for these types of devices.
			    */
			   /* save device info */
			   save_dev_info(sid,lun,utype,pvid,FALSE);

			   /* adjust maxlun value as necessary */
			   get_maxlun(utype,sid);

			   state = DEVICE;
                        }
		}

	} else if (error == ENODEV) {
		/* NOTE: we don't check rc here, because       */
		/* it must be non zero.  We need to check error*/
		/* instead for these cases.		       */

		DEBUG_2("no device at sid=%d lun=%d\n",sid,lun)

		/* mark this location empty */
		state = EMPTY;

		/* set max lun to zero so we don't bother with
				   the other luns on this sid */
		maxlun[sid] = 0;

	} else {
		/* i/o error on this sid/lun */
		/* mark it empty and go to next lun */
		DEBUG_2("i/o err during inq at sid=%d lun=%d\n", sid,lun)
		state = EMPTY;
	}

	/* do SCIOSTOP for sid/lun */
	DEBUG_2("stopping sid%d lun%d\n",sid,lun)
	if (ioctl(adapter,SCIOSTOP,IDLUN(sid,lun)) < 0) {
		DEBUG_2("error stopping sid%d lun%d\n",sid,lun)
	}

	return(state);
}

/*
 * NAME   : final_inq
 *
 * FUNCTION :
 *      This function performs the second inquiry for a scsi disk after
 *      it has been spun up.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 *
 * INPUT  : adapter file descriptor, scsi Id and lun.
 *
 * RETURNS:
 *
 */

int
final_inq(adapter,sid,lun)
int     adapter;                        /* file descriptor */
uchar   sid,lun;                        /* scsi id and lun */
{
	struct  inqry_data bb_inq;      /* area for disk inquiry data */
	char    *utype;                 /* disk unique type */
	char    pvid[PVIDSIZE];		/* area for disk's pvid */
	int	pg=NO_PAGE;		/* page to get   */
	int	error;			/* Error from issue_scsi_inquiry */

	/* wait for disk to spin up */
	(void) bb_disk_notrdy(adapter,sid,lun,0);

	/* do final inquiry */
	DEBUG_2("second inquiry for disk at sid=%d and lun=%d\n",sid,lun)

	if (issue_scsi_inquiry(adapter,&bb_inq,sid,lun,pg,&error) != 0) {
		/* error--can't talk to the device! */
		DEBUG_2("second inq failed sid=%d lun=%d\n",sid,lun)
		/* need to indicate disk of default type */
		utype = default_utype(DEFLT_DISK_TYP);
	} else if (bb_inq.pdevtype != SCSI_DISK) {
		/* error--can't talk to the device! */
		DEBUG_2("second inq failed sid=%d lun=%d\n",sid,lun)
		/* need to indicate disk of default type */
		utype = default_utype(DEFLT_DISK_TYP);
	} else
		/* determine disk's unique type */
		utype = det_utype(adapter,sid,lun,&bb_inq);

	/* get pvid from disk */
	get_pvid_from_disk(adapter,sid,lun,pvid);

	/* save device info */
	save_dev_info(sid,lun,utype,pvid,TRUE);

	/* adjust maxlun value as necessary */
	get_maxlun(utype,sid);

	/* do SCIOSTOP for sid/lun */
	if (ioctl(adapter,SCIOSTOP,IDLUN(sid,lun)) < 0) {
		DEBUG_2("error stopping sid%d lun%d\n",sid,lun)
	}

	return(DEVICE);
}
/*
 * NAME   : print_pkg
 *
 * FUNCTION :
 *      This function prints the appropriate package name
 *      based on the inquiry data passed in.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 *
 * INPUT  : pointer inquiry data
 *
 * RETURNS: none
 * 
 */
void
print_pkg(inqdata)
struct  inqry_data *inqdata;            /* pointer to inquiry data */
{

	/* Look for 7135 Disk Array */
	if ((inqdata->product[0] == '7') && (inqdata->product[1] == '1') &&
	    (inqdata->product[2] == '3') && (inqdata->product[3] == '5') &&
	    (inqdata->reserved[0] == 0x1f)) {
	    	fprintf(stdout, ":%s.scsi.scarray ", DEVPKG_PREFIX);
		DEBUG_0("print_pkg: scarray package from inquiry data\n");
		return;
	}

	switch (inqdata->pdevtype) {
        
        case SCSI_DISK :
        case SCSICDROM :
        case SCSI_RWOPT :
	    	fprintf(stdout, ":%s.scsi.disk ", DEVPKG_PREFIX);
                break;
        case SCSI_TAPE :
		fprintf(stdout, ":%s.scsi.tape ", DEVPKG_PREFIX);
                break;
        case SCSI_PROC :
		fprintf(stdout, ":%s.scsi.tm ", DEVPKG_PREFIX);
                break;
        default :
		fprintf(stdout, ":%s.scsi ", DEVPKG_PREFIX);

        } /* end switch */

}


/*
 * NAME   : det_utype
 *
 * FUNCTION :
 *      This function determines a device's unique type from the
 *      inquiry data.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 *
 * INPUT  : pointer to inquiry data.
 *
 * RETURNS: unique type
 * 
 * The routine determines uniquetype in 1 of 3 ways:
 *
 * 1) through the model_map PdAt default string
 * 2) model_name PdAt default string
 * 3) using default for disk and tape	
 *
 */

char *
det_utype(adapter,id,lun,inqdata)
int     adapter;                        /* adapter file descriptor     */
uchar   id,lun;                         /* SCSI id and lun of device   */
struct  inqry_data *inqdata;            /* pointer to inquiry data     */
{
	char    *inq_ptr;               /* local pointer to inquiry data*/
	char    *utype;                 /* pointer to unique type       */
	char    model_name[17];         /* area to hold model name      */
	int     inq_lth;                /* length of inquiry data       */
	int     i;                      /* loop variable                */
	int     index;                  /* index into PdAt object array */
	char	class[CLASSIZE];	/* class of uniquetype 		*/
	struct inqry_data *scsd_inq;	/* struct for SCSD test         */
	int 	pg=0xc7;		/* pg for SCSD test             */
	int	rc=0;			/* return code		        */
	inq_ptr = (char *)inqdata;      /* point to inquiry data        */

	/* set uniquetype pointer to NULL in case no match */

	utype = NULL;

	utype = get_scsi_utype(inq_ptr,pdobj_ptr,num_mma,mn_attr,num_mn,
				adapter,id,lun);


	/* if allpkg set or no PdDv was found, we need to print out */
	/* the package name */
	if (allpkg || utype == NULL ) {
		print_pkg(inqdata);
        }

	return(utype);
}


/*
 * NAME   : default_utype
 *
 * FUNCTION :
 *      This function finds the default unique type for a disk, CD-ROM,
 *      tape, read/write optical, or target mode device.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 * INPUT  : default model name attribute value.
 *
 * RETURNS:
 *
 */

char *
default_utype(default_model_name)
char    *default_model_name;            /* default model name attribute val */
{
	char    *utype;                 /* ptr to unique type */
	int     i;                      /* loop variable */

	/* set unique type pointer to NULL in case no match */
	utype = NULL;

	/* look for a model name attribute matching input value */
	for(i=0; i<num_mn; i++) {
		if (!strcmp(default_model_name,mn_attr[i].value2)) {
			utype = (char *)&mn_attr[i].value1;
			break;
		}
	}
	return(utype);
}


/*
 * NAME   : find_dev_in_cudv
 *
 * FUNCTION :
 *      This function matches the scsi devices actually with their
 *      corresponding CuDv object.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 * INPUT  : Number of SCSI ids.
 *
 * RETURNS:
 *
 */

void
find_dev_in_cudv (int numsids)
{
	int     i;			/* loop variable                    */
	int     j;			/* loop variable                    */
	char    *loc_con;		/* local variable for connection    */
	char    *loc_ut;		/* local variable for unique type   */
	int     num_of_pvid_matches;	/* number of disks with same pvid   */
	int     pvid_match;		/* CuDv disk object with same pvid  */
	int	first_null_match;	/* first CuDv disk object with same */
					/* parent and same SID/LUN but with */
					/* NULL pvid                        */
	int	x;			/* to hold index into real_dev      */


	/* First make a pass through all of the real devices to see if any  */
	/* are connected where we think we already have a configured, i.e.  */
	/* available, device.  For each device for which this is true we    */
	/* will assume the CuDv object and real device go together. Someday */
	/* we may want to check to see that the uniquetypes and/or pvids    */
	/* also match and log errors if they don't match.                   */
	for(i=0; i<num_rd; i++) {
		loc_con = real_dev[i].connect;
		for(j=0; j<num_cd; j++) {
			if (!strcmp(pname,cust_dev[j].cudv.parent) &&
			    !strcmp(loc_con,cust_dev[j].cudv.connwhere) &&
			    (cust_dev[j].cudv.status == AVAILABLE)) {
				DEBUG_2("%s already available at connection %s\n",
						cust_dev[j].cudv.name,loc_con)
				found_device(i,j);
				break;
			}
		}
	}

	/* Now match up each real device with CuDv objects of same type,    */
	/* with same parent, connection address (sid/lun), and pvid.        */
	for(i=0; i<num_rd; i++) {
		/* If device already handled in previous loop, go on. */
		if (real_dev[i].status == DONE)
			continue;

		loc_con = real_dev[i].connect;
		loc_ut = real_dev[i].utype;
		for(j=0; j<num_cd; j++) {
			if (!strcmp(loc_ut,cust_dev[j].cudv.PdDvLn_Lvalue) &&
			    !strcmp(pname,cust_dev[j].cudv.parent) &&
			    !strcmp(loc_con,cust_dev[j].cudv.connwhere) &&
			    (cust_dev[j].status != USED) &&
			    (cust_dev[j].cudv.status != AVAILABLE)) {
				
				/* We have found a potential match for a real */
				/* device  based on its parent, connection,   */
				/* and uniquetype	                      */

				if ((real_dev[i].dev_is_disk == FALSE) ||
				    (!strcmp(real_dev[i].pvid,cust_dev[j].pvid))) {

					/* If this real device is not a disk */
					/* then this is sufficient for a     */
					/* match. If it is a disk then the   */
					/* PVID's must match as well. 	     */

					DEBUG_2("found %s at connection %s\n",
						cust_dev[j].cudv.name,loc_con)
				        found_device(i,j);
					break;
				}
			}


		}
	}

	/* At this point, all devices that were previously known in CuDv    */
	/* whose parent and connection addresses have not changed have been */
	/* handled.                                                         */

	/* Of all of the remaining real devices if they do not have a       */
	/* pvid (this includes tapes, cdroms, and disks with no pvid),      */
	/* then they can not be matched with a CuDv object and so are new   */
	/* devices to be defined.  However, for each remaining disk device  */
	/* having a pvid, we will attempt to match it with a CuDv disk      */
	/* object of same type with same pvid.				    */
	for(i=0; i<num_rd; i++) {
		/* If device already handled in previous loop, go on. */
		if (real_dev[i].status==DONE)
			continue;

		/* If device has a pvid, attempt to match with CuDv */
		if (real_dev[i].pvid[0]!='\0') {
			loc_con = real_dev[i].connect;
			loc_ut = real_dev[i].utype;

			num_of_pvid_matches = 0;
			first_null_match = -1;
			for(j=0; j<num_cd; j++) {
				/* skip all CuDv objects that have been used, */
				/* are available, or are of wrong type        */
				if (strcmp(loc_ut,
				    cust_dev[j].cudv.PdDvLn_Lvalue) ||
			   	    (cust_dev[j].status == USED) ||
			   	    (cust_dev[j].cudv.status == AVAILABLE))
					continue;

				/* Check for pvid match.  Don't want to       */
				/* simply use first CuDv with matching pvid.  */
				/* Must look to see if multiple pvid matches. */
			   	if (!strcmp(real_dev[i].pvid,
							cust_dev[j].pvid)) {
					num_of_pvid_matches++;
					pvid_match = j;
				}

				/* Else, if pvids don't match but CuDv has    */
				/* same parent and same SID/LUN and has a     */
				/* NULL pvid, then remember which CuDv this   */
				/* is.  The first CuDv object like this may   */
				/* be the match we are looking for.           */
				else if(!strcmp(pname,cust_dev[j].cudv.parent)&&
					!strcmp(loc_con,
					cust_dev[j].cudv.connwhere) &&
					(cust_dev[j].pvid[0] == '\0') &&
					(first_null_match == -1)) {
						first_null_match = j;
				}
			}

			/* If a pvid match was found and ONLY one match,    */
			/* then its the CuDv we're looking for.             */
			if (num_of_pvid_matches == 1) {
				DEBUG_2("found %s at connection %s\n",
					cust_dev[pvid_match].cudv.name,loc_con)
				found_device(i,pvid_match);
				continue;	/* go to next real device */
			}

			/* If no exact pvid match, or multiple pvid matches  */
			/* then still don't know which CuDv to use.  In case */
			/* of multiple matches, will NOT use any of them but */
			/* will define a new CuDv if necessary. */

			/* However, before we define a new disk having a pvid */
			/* we look to see if there is a CuDv object for same  */
			/* parent, same SID/LUN, with a NULL pvid.  If we     */
			/* find one, we will match it to the real device.     */
			/* This case occurs on a run time define/config via   */
			/* the mkdev command. */
			if (first_null_match != -1) {
				DEBUG_2("found %s at connection %s\n",
				   cust_dev[first_null_match].cudv.name,loc_con)
				found_device(i,first_null_match);
				continue;	/* go to next real device */
			}
		}

	}

	/* If we get here, then the only remaining possibility       */
	/* for finding a match is that new ODM Predefines for either */
	/* an OEM SCSI device or an SCSD SCSI device have been       */
	/* added to the system.	So we will try to see if the         */
	/* remaining devices are of this type.			     */


	chk_update_utype();
	





	/* If we get here, the device is a new device that needs to  */
	/* be defined.  However, we only define them if the Dflag    */
	/* is not set.Traverse the realdevorder array based on SID/LUN*/
	/* to ensure correct name order.			     */

	if (Dflag == FALSE) {
	    for (i=0; i < numsids; i++) {
		for (j=0; j<=maxlun[i]; j++ ) {
		    if (realdevorder[i][j] != NOT_IN_USE ) {
			x=realdevorder[i][j];
			if (real_dev[x].status == DONE)
			    continue;
			else {
			    
			    DEBUG_1("defining new device at connection %s\n",
				    real_dev[x].connect);
			    def_scsi_dev(real_dev[x].utype, pname,
					 real_dev[x].connect, real_dev[x].pvid);
			}
		    }
		}
	    }
	}  /* DFlag = false */
	return;
}
/*
 * NAME   : chk_update_utype
 *
 * FUNCTION : This function looks if the unmatched real devices
 *	      can potentially be matched with CuDv's of different
 *	      uniquetypes. This requires the CuDv uniquetype to
 *	      be more general then the real device's uniquetype
 *	      (i.e. the CuDv is an OEM device and the real
 *	      device is an SCSD or an IBM defined device.
 *
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 * INPUT  : device unique types
 *
 * RETURNS: none
 *
 */
int
chk_update_utype()
{
	int     i;			/* loop variable                    */
	int     j;			/* loop variable                    */
	char    *loc_con;		/* local variable for connection    */
	char    *loc_ut;		/* local variable for unique type   */
	int     num_of_pvid_matches;	/* number of disks with same pvid   */
	int     pvid_match;		/* CuDv disk object with same pvid  */

	int updateable_utype = FALSE;  /* If we can update CuDv to a new   */
					/* uniquetype.			    */

	
	
	for(i=0; i<num_rd; i++) {
		
		/* If device already handled in previous loop, go on. */
		if (real_dev[i].status==DONE)
			continue;
		loc_con = real_dev[i].connect;
		loc_ut = real_dev[i].utype;
		
		/* If device has a pvid, attempt to match with CuDv */
		if (real_dev[i].pvid[0]!='\0') {
			
			
			num_of_pvid_matches = 0;
			for(j=0; j<num_cd; j++) {
				/* skip all CuDv objects that have been used, */
				/* are available,                             */
				if ((cust_dev[j].status == USED) ||
			   	    (cust_dev[j].cudv.status == AVAILABLE))
					continue;
				
				/* Check for pvid match.  Don't want to       */
				/* simply use first CuDv with matching pvid.  */
				/* Must look to see if multiple pvid matches. */
			   	if (!strcmp(real_dev[i].pvid,
					    cust_dev[j].pvid)) {
					num_of_pvid_matches++;
					pvid_match = j;
				}


			}

			if (num_of_pvid_matches == 1) {

				updateable_utype = 
				det_utype_updateable(cust_dev[pvid_match].cudv.PdDvLn_Lvalue,
						      loc_ut);
			} else {
				updateable_utype = FALSE;

			}
			       

			/* If a pvid match was found and ONLY one match,    */
			/* and this is a situation where we can update the  */
			/* CuDv's PdDvLn_Lvalue, then its the CuDv we're    */
			/* looking for.             			    */
			
			if (updateable_utype) {

				DEBUG_2("found %s at connection %s\n",
					cust_dev[pvid_match].cudv.name,loc_con)

				/* Remove all non-PVID customized attributes */

				clean_up_cuat(cust_dev[pvid_match].cudv);

				strcpy(cust_dev[pvid_match].cudv.PdDvLn_Lvalue,loc_ut);

				/* Remove all non-PVID customized attributes */

				clean_up_cuat(cust_dev[pvid_match].cudv);

				/* NOTE: found_device does an */
				/* odm_change_obj, so we don't*/
				/* need to do one here.       */

				found_device(i,pvid_match);
				continue;	/* go to next real device */
			}

			/* If no exact pvid match, or multiple pvid matches  */
			/* then still don't know which CuDv to use.  In case */
			/* of multiple matches, will NOT use any of them but */
			/* will define a new CuDv if necessary.              */


		    }
		else {
			/* This device does not have a PVID  */
			
			for(j=0; j<num_cd; j++) {
				if (!strcmp(pname,cust_dev[j].cudv.parent) &&
				    !strcmp(loc_con,cust_dev[j].cudv.connwhere) &&
				    (cust_dev[j].status != USED) &&
				    (cust_dev[j].cudv.status != AVAILABLE)) {
					
					/* We have found a potential match for a real */
					/* device  based on its parent, connection.   */

					updateable_utype = 
						det_utype_updateable(cust_dev[j].cudv.PdDvLn_Lvalue,
						      loc_ut);
					
					if (updateable_utype) {

						/* If this real device is not a disk */
						/* then this is sufficient for a     */
						/* match.                	     */
						
						DEBUG_2("found %s at connection %s\n",
							cust_dev[j].cudv.name,loc_con)

						/* Remove all non-PVID   */
						/* customized attributes */

						clean_up_cuat(cust_dev[j].cudv);        
						strcpy(cust_dev[j].cudv.PdDvLn_Lvalue,
						       loc_ut);

						/* NOTE: found_device does an */
						/* odm_change_obj, so we don't*/
						/* need to do one here.       */
						found_device(i,j);
						break;

					}
				}
				
			}


		}


	}
}

/*
 * NAME   : det_utype_updateable
 *
 * FUNCTION :
 *      This function determines if the old device is of type
 *	that can be updated to a newer installed type
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 * INPUT  : device unique types
 *
 * RETURNS: 1 - Updateable
 *	    0 - Not updateable
 *
 */
int
det_utype_updateable(old_utype,new_utype)
char    *old_utype;                 /* old unique type of device */
char    *new_utype;                 /* new unique type of device */ 
{
	char    old_dev_utype[48];	/* old unique type of device        */
	char    new_dev_utype[48];	/* new unique type of device        */
	char    *old_class;		/* local variable for class	    */
	char    *old_subclass;		/* local variable for subclass	    */
	char    *old_type;		/* local variable for type	    */
	char    *new_class;		/* local variable for class	    */
	char    *new_subclass;		/* local variable for subclass	    */
	char    *new_type;		/* local variable for type	    */
	int     rc;			/* return code			    */
	int     old_type_level;         /* the higher the value the more    */
					/* specific the type is.	    */
	int     new_type_level;		/* the higher the value the more    */
					/* specific the type is.	    */

	strcpy(old_dev_utype,old_utype);
	strcpy(new_dev_utype,new_utype);

	/* First the old_dev_utype and the new_dev_utype */
	/* must have the same ODM class and subclass in  */
	/* order to be upgradeable.			 */
	
	old_class = strtok(old_dev_utype,"/");
	old_subclass = strtok((char *)NULL,"/");
	old_type = strtok((char *)NULL,"/");


	new_class = strtok(new_dev_utype,"/");
	new_subclass = strtok((char *)NULL,"/");
	new_type = strtok((char *)NULL,"/");

	if (strcmp(old_class,new_class) ||
	    strcmp(old_subclass,new_subclass)) {
		
		/* Old device can't be updated to new device */

		return (0);

	}

	if (!strcmp(old_type,new_type)) {
		/* These two are the same uniquetype so */
		/* let's say they are updateable.	*/
		return (1);

	}


	/* If we got here then only the ODM types */
	/* are different. So no lets determine    */
	/* the level (or rank) of how specific    */
	/* the types are.			  */  			

	if (!strcmp(old_type,"oscd") ||
	    !strcmp(old_type,"osdisk") ||
	    !strcmp(old_type,"osomd") ||
	    !strcmp(old_type,"ost")) {
	    

		/* If this device is an OEM CD-ROM, OEM disk, OEM */
		/* read/write optical, or an OEM tape, then give  */
		/* it the lowest level of 0.		          */
		old_type_level = 0;

	}
	else if (!strcmp(old_type,"scsd")) {
	    

		/* If this device is an SCSD then give it a level */
		/* of just one higher then OEM.			  */

		old_type_level = 1;

	}
	else  {
	    

		/* Since this device is not an OEM nor SCSD, give */
		/* it a level of 2.				  */

		old_type_level = 2;

	}	

	if (!strcmp(new_type,"oscd") ||
	    !strcmp(new_type,"osdisk") ||
	    !strcmp(new_type,"osomd") ||
	    !strcmp(new_type,"ost")) {
	    

		/* If this device is an OEM CD-ROM, OEM disk, OEM */
		/* read/write optical, or an OEM tape, then give  */
		/* it the lowest level of 0.		          */
		new_type_level = 0;

	}
	else if (!strcmp(new_type,"scsd")) {
	    

		/* If this device is an SCSD then give it a level */
		/* of just one higher then OEM.			  */

		new_type_level = 1;

	}
	else  {
	    

		/* Since this device is not an OEM nor SCSD, give */
		/* it a level of 2.				  */

		new_type_level = 2;

	}	

	 
	if (old_type_level < new_type_level) {
		
		/* This device is updateable to a higher level */
		/* device.				       */

		return (1);
	}
	else
		return (0);

}  

/*
 * NAME   : clean_up_cuat
 *
 * FUNCTION :
 *      This function will remove all non-PVID attributes in the CuAt
 *      for a device that is being updated to a more specific type.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 * INPUT  : device CuDv.
 *
 * RETURNS: none
 *
 */
void
clean_up_cuat(cusobj)
struct CuDv cusobj;		/* CuDv of the object that needs it CuAt */
				/* cleaned up.				 */
{
	struct  Class   *cusatt;        /* CuAt object class handle    */
        struct  CuAt    *cusatt_ptr;    /* area for CuAt object        */
        char    sstr[256];              /* area for ODM search strings */
        struct listinfo list;		/* for using odm_get_list      */
        int     rc;                     /* used for return codes       */
	int     i;			/* general counter	       */

        /* open CuAt object class */
        if ((int)(cusatt = odm_open_class(CuAt_CLASS)) == -1){
                DEBUG_0("can't open CuAt class\n")
                return;
        }

	sprintf(sstr,"name=%s",cusobj.name);


	cusatt_ptr = odm_get_list(cusatt,sstr,&list,7,1);

        if ((int)cusatt_ptr == -1)
                return;

	/* Loop until we have found all the CuAt attributes    */
	/* for this device.				       */

	for (i = 0; i < list.num; i++) {

		if (strcmp(cusatt_ptr[i].attribute,"pvid")) {

			/* Remove all non-PVID attributes   */

			sprintf(sstr,"name=%s AND attribute=%s",
				cusobj.name,cusatt_ptr[i].attribute);
			if (odm_rm_obj(cusatt,sstr) == -1) {
                                DEBUG_0("chgdevice: ODM error while restoring attributes\n")
				return;
                        }


		}

	}
	odm_free_list(cusatt_ptr,&list);

        odm_close_class(cusatt);

        DEBUG_0("Done removing CuAt objects\n")
	return;
}
/*
 * NAME   : def_scsi_dev
 *
 * FUNCTION :
 *      This function defines a new SCSI device in the Database.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 * INPUT  : device unique type, parent name, connection on parent, and
 *          pvid. pvid is a null character for non-disks.
 *
 * RETURNS: none
 *
 */

void
def_scsi_dev(utype, parent, connect, pvid)
char    *utype;                 /* unique type of device */
char    *parent;                /* parent name (name of adapter) */
char    *connect;               /* connection on adapter (sid and lun) */
char    *pvid;                  /* pvid string */
{
	struct PdDv pddv;       /* area for PdDv object */
	struct CuAt *cuat;      /* local pointer to CuDv object */
	char    sstr[256];      /* ODM search string */
	char    *dname;         /* pointer to returned device name */
	int     cnt;            /* used in getattr call */


	/* get predefined device object for this unique type */
	sprintf(sstr,"uniquetype='%s'",utype);
	if ((int)odm_get_first(PdDv_CLASS,sstr,&pddv)<=0) {
		DEBUG_1("failed to get PdDv for utype=%s\n", utype)
		return;
	}

	/* now run the define method */
	sprintf(sstr, "-c %s -s %s -t %s -p %s -w %s",
		pddv.class,pddv.subclass,pddv.type,parent,connect);
	DEBUG_2("run method:%s %s\n", pddv.Define, sstr);
	if (odm_run_method(pddv.Define, sstr, &dname, NULL)) {
		DEBUG_1("error running %s\n", pddv.Define);
		return;
	}

	/* strip off the trailing junk character */
	dname[strlen(dname)-1] = '\0';

	/* write device name to stdout so cfg mgr will invoke config meth */
	printf("%s ",dname);

	/* if we have been passed a pvid string, then store it in CuAt */
	if (*pvid) {
		/* calling getattr first fills in all necessary fields */
		if ((cuat=getattr(dname,"pvid",FALSE,&cnt))==NULL) {
			DEBUG_1("error getting pvid attr for %s\n",dname)
			return;
		}
		/* copy in actual pvid string */
		strcpy(cuat->value,pvid);
		putattr(cuat);
	}

	return;
}


/*
 * NAME   : found_device
 *
 * FUNCTION :
 *      This function updates CuDv object for a device that has been
 *      still to be present.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 * INPUT  : index into array of actual (real) device information and index
 *          into array of customized device object information for the
 *          corresponding CuDv object.
 *
 * RETURNS: none
 *
 */

void
found_device(rd,cd)
int     rd;                             /* real device index */
int     cd;                             /* customized device obj index */
{
	struct CuDv     *cudv;          /* local pointer to CuDv object */

	real_dev[rd].status = DONE;     /* done with this device */
	cust_dev[cd].status = USED;     /* this CuDv object has been used,
					   so it can't be matched to another
					   device */
	cudv = &cust_dev[cd].cudv;      /* set up local pointer */

	/* make sure connection information is correct */
	strcpy(cudv->parent,pname);
	strcpy(cudv->connwhere,real_dev[rd].connect);
	location(putype,plocation,cudv);

	/* set change status to SAME */
	if (cudv->chgstatus == MISSING )
		cudv->chgstatus = SAME;

	/* now change the object in the CuDv object class */
	if (odm_change_obj(cusdev, cudv) == -1) {
		DEBUG_1("change_obj failed for %s\n",cudv->name)
	} else
		printf("%s ", cudv->name);

	return;
}

/*
 * NAME   : get_maxlun
 *
 * FUNCTION :
 *      This function determines if a device found to be present has a
 *      limitation on checking higher numbered LUNs on same SID.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 * INPUT  : device's unique type.
 *          device's scsi id.
 *
 * RETURNS: none
 *
 */

void
get_maxlun (char *utype,                 /* unique type of device */
	    int  sid)                    /* scsi id */
{
	int     tmaxlun;        /* temporary max lun value */
	int     i;              /* loop variable */

	tmaxlun = maxlun[sid];

	/* if a null pointer was passed in, we don't know the device */
	/* and will not adjust the max lun value */
	if (utype) {
		/* get maxlun attribute for this device */
		for (i=0; i<num_ml; i++) {
			if (!strcmp (utype, ml_attr[i].value1)) {
				tmaxlun = ml_attr[i].value2;
				break;
			}
		}
	}

	/* if the max lun attribute is less than the max lun value */
	/* that is stored in the maxlun array, then store the      */
	/* attribute's value.                                      */
	if (tmaxlun < maxlun[sid]) {
	        DEBUG_2("maxlun for sid=%d is %d\n",
		        sid, tmaxlun);
		maxlun[sid] = tmaxlun;
	}
}


/*
 * NAME   : save_dev_info
 *
 * FUNCTION :
 *      This function saves information about a scsi devices that has
 *      been detected.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 * INPUT  : SCSI id, lun, device's unique type, and pvid.  The pvid will
 *          be a null character if device has no pvid.
 *
 * RETURNS: none
 *
 */

void
save_dev_info(sid,lun,utype,pvid,dev_is_disk)
uchar   sid,lun;                /* device's SCSI id and lun */
char    *utype;                 /* device's unique type */
char    *pvid;                  /* device's pvid */
uchar   dev_is_disk;            /* True if device is a disk */
{
	int	i;

	/* If unique type is a null pointer, we don't know about this */
	/* device type so don't save any information about it because */
	/* won't be able to match it with a CuDv object or define it. */
	if (utype) {
		/* see if we need to allocate more memory */
		i = num_rd - RD_BASE;	/* number of devs found beyond RD_BASE*/
		if ((i >= 0) && (i%RD_INC)==0) {
			DEBUG_1("increasing memory to handle %d devices\n",
							num_rd+RD_INC)
			real_dev = (struct real_device *)realloc(real_dev,
				(num_rd+RD_INC)*sizeof(struct real_device));
		}
		real_dev[num_rd].status = 0;
		real_dev[num_rd].dev_is_disk = dev_is_disk;
		strcpy(real_dev[num_rd].utype,utype);
		sprintf(real_dev[num_rd].connect,"%d,%d",sid,lun);
		strcpy(real_dev[num_rd].pvid,pvid);

		DEBUG_3("device found, utype= %s, sid/lun= %s, pvid= %s\n",
					utype,real_dev[num_rd].connect,pvid)
		/* save the number of this real device into realdevorder   */
		/* array so that can index the array later in sid/lun order*/
		realdevorder[sid][lun] = num_rd ;

		/* increment number of real devices found so far */
		num_rd++;
	} else {
		DEBUG_0("device found, unknown utype\n")
	}
	return;
}


/*
 * NAME   : get_from_database
 *
 * FUNCTION :
 *      This function fetches needed information from the database.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 * INPUT  : none
 *
 * RETURNS: none
 *
 */

int
get_from_database()
{
	struct  Class   *preatt;        /* PdAt object class handle */
	struct  Class   *cusatt;        /* CuAt object class handle */
	struct  PdAt    PdAt;           /* area for PdAt object */
	struct  CuAt    CuAt;           /* area for CuAt object */
	struct  CuDv    CuDv;           /* area for CuDv object */
	int     i,j,k;			/* loop variables */
	struct  mna	*mn;            /* pointer to memory for model name
					   attributes */
	struct  mla	*ml;            /* pointer to memory for max lun
					   attributes */
	struct  cust_device     *devs;  /* pointer to memory for CuDv info */
	int	mna_size;		/* size of model name attribute strct */
	int	mla_size;		/* size of max lun attribute struct */
	int	cd_size;		/* size of customized device struct */
	char    sstr[256];              /* area for ODM search strings */
	int     rc;                     /* used for return codes */
	struct  listinfo 	mma_attr;/* list of model_map attributes     */	


	/* open PdAt object class */
	if ((int)(preatt = odm_open_class(PdAt_CLASS)) == -1){
		DEBUG_0("can't open PdAt class\n")
		return(E_ODMOPEN);
	}

	/* run through all PdAt objects and save information about SCSI
	   model name and maxlun attributes */

	/* allocate initial space for holding model name attributes */
	i = 0;                  /* number of model name attributes */
	mna_size = sizeof(struct mna);
	mn = (struct mna *)malloc(MN_BASE * mna_size);

	/* allocate initial space for holding max lun attributes */
	j = 0;                  /* number of max lun attributes */
	mla_size = sizeof(struct mla);
	ml = (struct mla *)malloc(ML_BASE * mla_size);

	/* set search string to look for devices in scsi subclass */
	strcpy(sstr,"uniquetype like */scsi/*");
	rc = (int)odm_get_first(preatt, sstr, &PdAt);
	while(rc != 0) {
		if (rc == -1) {
			DEBUG_0("error getting PdAt object\n")
			return(E_ODMGET);
		}

		/* if model_name attribute then save uniquetype and */
		/* attribute value (model name)		            */
		if (!strcmp(PdAt.attribute,"model_name")) {
			if ((i >= MN_BASE) && ((i-MN_BASE)%MN_INC)==0) {
				DEBUG_1("inc space for mn attrs to %d\n",
								i+MN_INC)
				mn = (struct mna *)realloc(mn,
				 			(i+MN_INC)*mna_size);
			}
			strcpy(mn[i].value1,PdAt.uniquetype);

			/* make sure model name attribute is */
			/* exactly 16 chars, and no special chars */
			strncpy(mn[i].value2,PdAt.deflt,16);
			for(k=0; k<16; k++) {
				if (mn[i].value2[k] < ' ' ||
					mn[i].value2[k] > '\177') 
					mn[i].value2[k] = ' ';
			}
			mn[i].value2[16] = '\0';
			i++;
		}

		/* if maxlun attribute then save uniquetype  */
		/* and maxlun value                          */
		else if (!strcmp(PdAt.attribute,"maxlun")) {
			if ((j >= ML_BASE) && ((j-ML_BASE)%ML_INC)==0) {
				DEBUG_1("inc space for ml attrs to %d\n",
								 j+ML_INC)
				ml = (struct mla *)realloc(ml,
				 			(j+ML_INC)*mla_size);
			}
			strcpy(ml[j].value1,PdAt.uniquetype);
			ml[j].value2 = (int)strtol(PdAt.deflt,NULL,0);
			j++;
		}
		rc = (int)odm_get_next(preatt, &PdAt);
	}

	/* get all scsi model_map attributes */
	strcpy (sstr, "uniquetype like */scsi/* AND attribute=model_map");
	pdobj_ptr = odm_get_list(PdAt_CLASS, sstr, &mma_attr,32,1);
	if ( (int)pdobj_ptr == -1) {
		DEBUG_0("error getting PdAt object list info\n")
		return(E_ODMGET);
	}
	num_mma=mma_attr.num; 		/* keep number of PdAt's returned */

	/* close PdAt object class */
	odm_close_class(preatt);

	DEBUG_0("Done getting PdAt objects\n")

	/* save pointers to arrays and number of attributes in global vars */
	num_mn = i;                     /* save number of model name attrs */
	mn_attr = mn;                   /* save pointer to model name attrs */
	num_ml = j;                     /* save number of max lun attrs */
	ml_attr = ml;                   /* save pointer to max lun attrs */



	/* run through all CuDv objects and save info about SCSI devices */
	i = 0;                  /* number of scsi CuDv objects */
	cd_size = sizeof(struct cust_device);
	devs = (struct cust_device *)malloc(CD_BASE * cd_size);

	/* set search string to look for devices in scsi subclass */
	strcpy(sstr,"PdDvLn like */scsi/*");

	rc = (int)odm_get_first(cusdev, sstr, &CuDv);
	while(rc != 0) {
		if (rc == -1) {
			DEBUG_0("error getting CuDv object\n")
			return(E_ODMGET);
		}
		if ((i >= CD_BASE) && ((i-CD_BASE)%CD_INC)==0) {
			DEBUG_1("inc space for CuDv objs to %d\n", i+CD_INC)
			devs = (struct cust_device *)
				realloc(devs, (i+CD_INC) * cd_size);
		}

		/* save entire CuDv object */
		devs[i].cudv = CuDv;
		/* Set pvid to a null character for now */
		devs[i].pvid[0] = '\0';

		i++;
		rc = (int)odm_get_next(cusdev, &CuDv);
	}

	/* save pointer to array and number of devices in global vars */
	num_cd = i;                     /* save number of CuDv objects */
	cust_dev = devs;                /* save pointer to CuDv objects */

	DEBUG_0("Done getting CuDv objects\n")

	/* open CuAt object class */
	if ((int)(cusatt = odm_open_class(CuAt_CLASS)) == -1){
		DEBUG_0("Can not open CuAt class\n")
		return(E_ODMOPEN);
	}

	/* run through all CuAt objects looking for SCSI disk pvid */
	/* attributes and add to device info already saved */
	strcpy(sstr,"attribute = pvid");

	rc = (int)odm_get_first(cusatt, sstr, &CuAt);
	while( rc != 0) {
		if (rc == -1) {
			DEBUG_0("error getting CuAt object\n")
			return(E_ODMGET);
		}

		for(i=0; i<num_cd; i++) {
			if(!strcmp(CuAt.name,devs[i].cudv.name)) {
				strcpy(devs[i].pvid,CuAt.value);
				break;
			}
		}
		rc = (int)odm_get_next(cusatt, &CuAt);
	}

	/* close CuAt object class */
	odm_close_class(cusatt);

	DEBUG_0("Done getting CuAt objects\n")

	/* set up search string to get adapter's CuDv object */
	/* NOTE: this could be done in the loop above and save a pass */
	/* through the CuDv object class. */
	sprintf(sstr,"name = %s",pname);
	rc = (int)odm_get_first(cusdev, sstr, &CuDv);
	if (rc == -1) {
		DEBUG_0("error getting adapter CuDv object\n")
		return(E_ODMGET);
	}
	else if (rc == 0) {
		return(E_NOCuDv);
	} else {
		/* found adapter object. save ite unique type and location */
		strcpy(putype,CuDv.PdDvLn_Lvalue);
		strcpy(plocation,CuDv.location);
	}

	return(0);
}



/*
 * NAME : bb_disk_spinup
 *
 * FUNCTION :
 *      This functions spins up a disk connected on scsiaddr id,lun.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *      su_pb : parameter block for spin_up command.
 *
 * INPUT  : filedescriptor, id, lun
 *
 * RETURNS: Returns 0 if success else -1
 *
 * RECOVERY OPERATION:
 *      Retrys once if error is EIO or ETIMEDOUT
 *
 */

int bb_disk_spinup(adapter,id,lun)
int     adapter;
uchar   id,lun;
{
uchar   *p;
struct  sc_startunit su_pb;
int     nsup,tried_async,rc;

	/* clear startunit structure */
	p = (uchar *)&su_pb;
	nsup = sizeof(struct sc_startunit);
	while(nsup--)*p++ = 0;

	tried_async = 0;
	su_pb.flags = 0;
	su_pb.scsi_id = id;
	su_pb.lun_id = lun;
	su_pb.start_flag = TRUE;        /* start the unit */
	su_pb.immed_flag = TRUE;        /* immediate return */
	su_pb.timeout_value = SUP_TIMEOUT;
	for(nsup = 0; nsup < 3; nsup++){

	    rc = ioctl(adapter,SCIOSTUNIT,&su_pb);
	    if(rc == 0) break;  /* else error condition */
	    if(nsup > 0){
		if((errno == ENOCONNECT) && (tried_async == 0)){
		    tried_async = 1;
		    su_pb.flags = SC_ASYNC;
		}

		else {
		    DEBUG_3("bb_spinup:err=%d on id%d lun%d.\n",errno,id,lun)
		    rc = -1;
		}

	    }
	}       /* end for nsup loop */
	return(rc);
}

/*
 * NAME : bb_disk_notrdy
 *
 * FUNCTION :
 *      Checks if the disk on id, lun is spinning and ready.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *      tr_pb : parameter block for test_unit_rdy command.
 *
 * INPUT  : filedescriptor, id, lun, immed flag
 *             immed flag, when set, means that the routine
 *             should not wait for the disk to spin-up
 *             before returning success or failure.
 *
 * RETURNS: Returns 0 if success else -1
 *
 * RECOVERY OPERATION:
 *      Retrys once for all errors.
 *      Retrys on valid SCSI status other than Reservation Conflict.
 *
 */

int bb_disk_notrdy(adapter,id,lun,immed_flag)
int     adapter;
uchar   id,lun;
int     immed_flag;
{
uchar   *p;
struct  sc_ready  tr_pb;
int     rc,ntur,try_async,total_time;

	/* clear sc_ready structure */
	p = (uchar *)&tr_pb;
	ntur = sizeof(struct sc_ready);
	while(ntur--)*p++ = 0;

	tr_pb.flags = 0;
	tr_pb.scsi_id = id;
	tr_pb.lun_id = lun;

	total_time = 0;
	try_async = 0;

	for(ntur = 0;; ntur++){
	    tr_pb.status_validity = 0;
	    tr_pb.scsi_status = 0;
	    rc = ioctl(adapter,SCIOTUR,&tr_pb);
	    DEBUG_2("bb_turdy: SCIOTUR   rc=%d   errno=%d\n", 
		    rc, errno);
	    if(rc == 0) {
		DEBUG_2("bb_turdy: validity=%d status=0x%x\n",
		    tr_pb.status_validity,tr_pb.scsi_status)
		break;
	    }                           /* else error condition */
	    if(ntur == 0)continue;              /* retry once anyway */
	    /* the following allows retry, but does not wait until
	       device comes ready */
	    if (immed_flag)
	    {
		DEBUG_2("bb_turdy: ntur = %d  immed_flag = %d\n", 
			ntur, immed_flag);
		break;
	    }
	    /* clear vendor unique and reserved bits from scsi status */
	    tr_pb.scsi_status &= SCSI_STATUS_MASK;

	    if(errno == ENOCONNECT){
		DEBUG_0("bb_turdy: errno = ENOCONNECT \n");
		try_async = 1;
		break;
	    }
	    else {
		if((errno == EIO) &&
		   (tr_pb.status_validity == SC_SCSI_ERROR) &&
		   (tr_pb.scsi_status != SC_RESERVATION_CONFLICT)) {
		    DEBUG_2("bb_turdy: validity=%d status=0x%x\n",
			tr_pb.status_validity,tr_pb.scsi_status)
		    if (total_time >= SUP_TIMEOUT) {
			break;          /* timeout condition met */
		    }
		    else {
			sleep(TUR_WAIT);
			total_time += TUR_WAIT;
			/* command is now retried */
		    }
		}
		else {
		    DEBUG_2("bb_turdy:status_validity=%d scsi_status=%d\n",
			    tr_pb.status_validity,tr_pb.scsi_status)
		    DEBUG_3("bb_turdy:err=%d on id%d lun%d\n",errno,id,lun)
		    break;
		}
	    }
	}       /* end for ntur loop */

	if(rc && (try_async == 1)){     /* on enoconnect try async once */
	    tr_pb.flags = SC_ASYNC;
	    rc = ioctl(adapter,SCIOTUR,&tr_pb);
	}
	return(rc);
}


/*
 * NAME   : get_pvid_from_disk
 *
 * FUNCTION :
 *      This function reads the pvid from a disk.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 * INPUT  : adapter file descriptor, SCSI id and lun of disk, pointer to
 *          area of memory which is to hold the pvid string.
 *
 * RETURNS: none
 *
 */

void
get_pvid_from_disk(adapter,sid,lun,pvidstr)
int     adapter;                        /* adapter file descriptor */
uchar   sid,lun;                        /* scsi id and lun of disk */
char    *pvidstr;                       /* pointer to area to hold pvid */
{
	int     rc;
	char    pvidbuf[4096];          /* area for boot record, must be 4K */
	IPL_REC *iplrec;

	/* issue ioctl to adapter to get this drive's boot record */
	rc = read_scsi_pvid(adapter, sid, lun, pvidbuf);
	if (rc==0) {
		iplrec = (IPL_REC *)pvidbuf;
		if (iplrec->IPL_record_id == IPLRECID) {
			/* copy pvid string */
			strcpy(pvidstr,pvidtoa(&iplrec->pv_id));
			if (strcmp(pvidstr,NULLPVID)==0)
				*pvidstr = '\0';
		} else
			*pvidstr = '\0';
	} else
		*pvidstr = '\0';
	return;
}

/****************************************************************
 * NAME	: read_scsi_pvid
 *
 * FUNCTION :
 *
 * EXECUTION ENVIRONMENT:
 *	This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 * INPUT  :
 *
 * RETURNS: Returns 0 if success else -1
 *
 * RECOVERY OPERATION:
 *	Inquiry is retried once if error is ETIMEDOUT or EIO.
 *
 **********************************************************************/

int read_scsi_pvid(adapter, id, lun, ptr)
int	adapter;
uchar	id, lun;
uchar	 *ptr;
{
	struct sc_readblk pvid_data;
	int	blksiz;
	uchar	tried_async;
	int	rc;
	int	tries;

	DEBUG_4("read_scsi_pvid: adapter_fd=%d, id=%d,lun=%d,ptr=%x\n",
		adapter,id,lun,ptr);
	
	pvid_data.scsi_id = id;
	pvid_data.lun_id  = lun;
	pvid_data.blkno   = 0;
	pvid_data.timeout_value = 30;
	pvid_data.data_ptr = ptr;
	pvid_data.rsv1  = 0;
	pvid_data.rsv2  = 0;
	pvid_data.rsv3  = 0;
	pvid_data.rsv4  = 0;
	pvid_data.flags = 0;
	
	for (blksiz = 512; blksiz <= 4096; blksiz <<= 1) {
		
		pvid_data.blklen  = blksiz;
		
		for (tries = 0; tries < 3; tries++) {
			rc = ioctl(adapter, SCIOREAD, &pvid_data);
			if (rc == 0) {
				return(rc);
			}
			DEBUG_2("read_scsi_pvid: blksiz=%d,tries = %d\n",
				blksiz,tries);
			/* else error condition */
			if (tries > 0) {
				
				switch (errno) {
				      case ENODEV:
					DEBUG_2("read_scsi_pvid: ENODEV on %d %d. try ASYNC\n",
						id, lun);
					tried_async=1;
					pvid_data.flags |= SC_ASYNC;
				      case ENOCONNECT:
					tried_async=1;
					pvid_data.flags |= SC_ASYNC;
					DEBUG_2("read_scsi_pvid: ENOCONNECT @ %d %d. try ASYNC\n",
						id, lun);
					break;
				      case ETIMEDOUT:
					pvid_data.timeout_value = 5;
					break;
				      case EIO:
					DEBUG_2("read_scsi_pvid: EIO @ %d %d. do new blocksize\n",
						id, lun);
					break;
				      default:
					DEBUG_3("bb_inqry:err=%d on %d %d\n",
						errno, id, lun);
					rc = -1;
				}
			}
		}
	}
	return(rc);
}
