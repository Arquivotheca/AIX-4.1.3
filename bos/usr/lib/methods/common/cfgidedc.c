static char sccsid[] = "@(#)26  1.4  src/bos/usr/lib/methods/common/cfgidedc.c, cfgmethods, bos41J, 9516A_all 4/18/95 07:32:59";
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: 
 *		def_ide_children
 *		def_ide_dev
 *		default_utype
 *		det_utype
 *		find_dev_in_cudv
 *              chk_update_utype
 *		clean_up_cuat
 *		det_utype_updateable
 *		found_device
 *		get_from_database
 *		get_pvid_from_disk
 *		initial_ident
 *		ide_location
 *		print_pkg
 *		read_ide_pvid
 *		save_dev_info
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
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
#include	<sys/ide.h>
#include        "cfgide.h"
#include        "cfgdebug.h"
#include 	<string.h>
#include 	<math.h>


/* forward function definitions */
int	initial_ident();
char 	*det_utype();
char 	*default_utype();
void	find_dev_in_cudv();
int     chk_update_utype();
int	det_utype_updateable();
void    clean_up_cuat();
void	def_ide_dev();
void	found_device();
void	save_dev_info();
int	get_from_database();
void	get_pvid_from_disk();
int	ide_location();


/*	extern declarations	*/
extern	int	errno;
extern	int	Dflag;

extern  char    *get_ide_utype(uchar *,struct PdAt*, int,struct mna*, 
			int,int,uchar);
extern  char    *strtok(char *,char *);


/*      static declarations     */
struct  mna    *mn_attr;		/* pointer to model name attributes */
struct  PdAt   *pdobj_ptr;		/* pointer to list of model_map attr*/
struct  cust_device     *cust_dev;      /* ptr to cust device info */
struct  real_device     *real_dev;      /* ptr to real device info, these are
					   the devices actually found to be
					   attached */
int     num_mn;                         /* number of model name attributes */
int     num_cd;                         /* number of cust device objects */
int     num_rd;                         /* number of found real devices */
int	num_mma;			/* number of model_map attributes */
uchar   idmap[MAX_ID];                  /* one entry for each master/slave */
int	realdevorder[MAX_ID];         	/* one entry for each real device to */
					/* keep order correct		*/	
char    *pname;                         /* pointer to adapter name (parent) */
char    putype[UNIQUESIZE];		/* adapter's uniquetype */
char    plocation[LOCSIZE];		/* adapter's location code */
struct  Class   *cusdev;                /* for accessing CuDv */
char    device_state = AVAILABLE;       /* for determining the previous state*/
                                        /* of this device to determine if a  */
                                        /* microcode download is necessary   */
int	initial_ident_error = 0;	/* set to 1 if a device is already opened */
int	allpkg = 0;			/* packaging flag 		*/

#define	MODEL_TYPE_BITS	0x03
#define	RACK		0x02
#define	DEVPKG_PREFIX	"devices"		/* device package prefix */



/*
 * NAME   : def_ide_children
 *
 * FUNCTION : This routine detects and defines devices attached to the
 *       ide adapter. Supported devices are disks, tapes, and cdroms.
 *
 * EXECUTION ENVIRONMENT:
 *      This function operates as a device dependent subroutine
 *      called by the generic configure method for all devices.
 *
 * DATA STRUCTURES:
 *      bb_ident  : structure to hold identify_device data.
 *
 * INPUTS : logical_name,ipl_phase
 *
 * RETURNS: Returns 0 if success
 *
 * RECOVERY OPERATION:
 *
 */

int
def_ide_children  (char *lname,     /* logical name */
		   int ipl_phase)   /* ipl phase */
{
	int	numids=MAX_ID;		/* maximum number of devices (Master and Slave) */
	char    adap_dev[32];           /* adapter device /dev name */
	int     adapter;                /* file descriptor for adapter */
	uchar   id;                     /* current master/slave being checked */
	int     done;                   /* loop control variable */
	int     rc;                     /* for return codes */
	struct  devinfo ide_info;       /* for IOCINFO ioctl */
        struct  CuAt cuattr,sys_cuattr; /* for getting CuAt attribute */
	struct  CuAt par_cuat;          /* for getting CuAt attribute */
        struct  CuAt *cuat;		/* for getting CuAt attribute */
	int	modcode;		/* model code attribute value */
	int     cnt;                    /* used in getattr call */
	char	class[CLASSIZE];	/* class of uniquetype for pkg printing */
	int	j;			/* loop control */

	DEBUG_0("entering def_ide_children routine\n")

	/* Get the packaging environment variable */
   	if (!strcmp(getenv("DEV_PKGNAME"),"ALL"))
		allpkg = 1;

	/* save pointer to adapter name in static storage so the */
	/* other routines have access to it */
	pname = lname;

	/* open adapter device */
	sprintf(adap_dev,"/dev/%s",lname);
	if((adapter = open(adap_dev,O_RDWR)) < 0){
	    DEBUG_1("def_ide_children:can not open %s \n",adap_dev)
	    return(E_DEVACCESS);
	}

	/* verify adapter supports IDE by using IOCINFO ioctl cmd */
	if((ioctl(adapter,IOCINFO,&ide_info) < 0) ||
	   (ide_info.devtype != DD_BUS) ||
	   (ide_info.devsubtype != DS_IDE)){
		DEBUG_1("def_ide_children: can not access IDE adapter for %s\n",
			lname)
		return(E_DEVACCESS);
	}

	/* open CuDv object class and keep open */
	if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1){
		DEBUG_0("def_ide_children: can't open CuDv class\n")
		return(E_ODMOPEN);
	}

	/* get model_name attributes from PdAt object class */
	/* get customized device information for all ide devices in CuDv */
	/* get pvid attributes for ide disk devices in CuDv */
	rc = get_from_database();
	if (rc) {               /* errors are returned only for fatal */
		DEBUG_0("def_ide_children: fatal ODM error - returning\n")
		return(rc);     /* ODM errors */
	}

#ifdef _CFG_RDS

/* don't have the foggiest what this is about */
        rc = (int)odm_get_first(CuAt_CLASS, "name=sys0 AND attribute=rds_facility", &sys_cuattr);
        if (rc == -1) {
                DEBUG_0("ODM error getting rds_facility attribute\n")
                return(E_ODMGET);
	} else if (rc != 0 && sys_cuattr.value[0]=='y') {
                rc = rds_find_device(lname, adapter, numids, 0);
                if (rc != 0) {
                        DEBUG_0("def_ide_children: can not find children devices\n")
                }
        }
#endif
	/* initialize the master/slave map */
	for(id = 0; id < numids; id++) {
		idmap[id] = UNKNOWN;
		/* initialize the real device order map */
		/* to designate no devices detected yet */
		realdevorder[id] = NOT_IN_USE;
	}

	/* malloc space for keeping track of actual ide devices we find */
	num_rd = 0;             /* number of devices actually found */
	real_dev = (struct real_device *)
			malloc(RD_BASE*sizeof(struct real_device));

	/* discover devices on master/slave */
	done = FALSE;
	for (id = 0; id < numids; id++) {
		rc = identify_device(adapter,id);
		idmap[id] = rc;
	}

	/*** free up malloc'ed memory for attribute info */
	free(mn_attr);

	/* match up real devices with corresponding CuDv info */
	/* and define new ones if necessary */
	find_dev_in_cudv (numids);

	/* If we found any devices that were already opened, we	*/
	/* will not have correctly printed out their package	*/
	/* names.  So, at this point, if there was an error in	*/
	/* accessing a device and if the allpkg flag is set,	*/
	/* we need to loop through all customized devices and	*/
	/* for all devices that are available and that are	*/
	/* children of our device, we need to print out the	*/
	/* package name. 					*/
	if (initial_ident_error && allpkg)
	{
		for (j=0; j<num_cd; j++)
		{
			if (!strcmp(cust_dev[j].cudv.parent,lname) && 
				cust_dev[j].cudv.status == AVAILABLE)
			{
				DEBUG_1("uniquetype for printing package is %s",
					cust_dev[j].cudv.PdDvLn_Lvalue );

				/* choose package based on class of uniquetype */
				sscanf(cust_dev[j].cudv.PdDvLn_Lvalue, "%[^/] ", &class);

				if (strcmp(class, "disk")==0)
				    	fprintf(stdout, ":%s.ide.disk ", DEVPKG_PREFIX);
				else if (strcmp(class, "cdrom")==0)
					fprintf(stdout, ":%s.ide.cdrom ", DEVPKG_PREFIX);
				else if (strcmp(class, "tape")==0)
					fprintf(stdout, ":%s.ide.tape ", DEVPKG_PREFIX);
				else
					fprintf(stdout, ":%s.ide ", DEVPKG_PREFIX);

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
 * NAME   : identify_device
 *
 * FUNCTION :
 *      This function performs the identify device to a master or slave
 *      to determine what type device.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 *
 * INPUT  : adapter file descriptor, master/slave indicator
 *
 * RETURNS:
 *
 */

int
identify_device(adapter,id)
int     adapter;                        /* file descriptor */
uchar   id;                             /* master/slave indicator */
{
	char    pvid[PVIDSIZE];		/* area to hold a pvid */
	char    *tmp_ptr;               /* local character pointer */
	char    connect[8];             /* area to hold ide device
					   connection string */
	char    *utype;                 /* device unique type */
	uchar   state;                  /* device state of this id */
	int     current_time;           /* for holding current time */
	int     elapsed_time;           /* time since last disk spin up */
	int     rc;                     /* for return codes */
	int     i;                      /* loop variable */
	struct  identify_device bb_ident;  /* area to hold identify device data */
	int	error;			/* Error from issue_ide_identify_dev */
	uchar	dev_type;		/* indicates what type of device, if any */
	
	/* start this id */
	DEBUG_1("starting id %d\n", id)
	if (ioctl(adapter,IDEIOSTART,id)!=0) {
		DEBUG_1("can not start id=%d\n",id)
		/* Can't start this id, most likely due to the device */
		/* already being configured and in opened. Look in the     */
		/* saved CuDv info to see if a device is already config'ed */
		/* on this adapter at this particular id. */

		/* set flag to 1 to indicate that a busy device was found  */
		/* so that the device package names will be printed	   */
		initial_ident_error = 1;

		sprintf(connect,"%d",id);
		for (i=0; i<num_cd; i++) {
			if (!strcmp(pname,cust_dev[i].cudv.parent) &&
			    !strcmp(connect,cust_dev[i].cudv.connwhere) &&
			    (cust_dev[i].cudv.status == AVAILABLE)) {
				DEBUG_0("device already available\n")
				tmp_ptr = (char *)&cust_dev[i].cudv.PdDvLn_Lvalue;
				break;
			}
		}
		return(EMPTY);
	}

	/* identify device for the id */
	DEBUG_1("identify device for id=%d\n",id)

	rc = issue_ide_identify_device(adapter,&bb_ident,id,&error,&dev_type);
	if ((rc == 0) && (dev_type == IDE_ID_NODEV)) {
		
		/* no device on this id */
		/* mark it empty and go to next id */
		DEBUG_1("no device at id=%d\n",id)
		state = EMPTY;
			
	} 
	else if (rc == 0) {


		/* see if the device is a disk */

		if (!(bb_ident.gen_config & ID_ATAPI)) {
			DEBUG_0("its a disk\n")

			/* determine disk's unique type */
			utype = det_utype(adapter, id, &bb_ident);

			/* get pvid from disk */
			get_pvid_from_disk(adapter, id, pvid);

			/* save device info */
			save_dev_info(id, utype, pvid, TRUE);

			state = DEVICE;

		} else { /* we've found some other type of device */
			/* determine device's unique type */
			DEBUG_0("its a non-disk device\n")

			utype = det_utype(adapter, id, &bb_ident);
                        DEBUG_1("utype = %s\n", utype)
                        
       			pvid[0] = '\0';	/* no pvid for non-disks */
			/*
			 * dev_is_disk is set to FALSE so find_dev_in_cudv 
			 * will not look at pvid for these types of devices.
			 */
			/* save device info */
			save_dev_info(id, utype, pvid, FALSE);


			state = DEVICE;
		}

	} else if (error == ENODEV) {
		/* NOTE: we don't check rc here, because       */
		/* it must be non zero.  We need to check error*/
		/* instead for these cases.		       */

		DEBUG_1("no device at id=%d\n",id)

		/* mark this location empty */
		state = EMPTY;

	} else {
		/* i/o error on this id */
		/* mark it empty and go to next device */
		DEBUG_1("i/o err during ident at id=%d\n", id)
		state = EMPTY;
	}

	/* do IDEIOSTOP for id */
	DEBUG_1("stopping id%d\n",id)
	if (ioctl(adapter,IDEIOSTOP,id) < 0) {
		DEBUG_1("error stopping id%d\n",id)
	}

	return(state);
}

/*
 * NAME   : print_pkg
 *
 * FUNCTION :
 *      This function prints the appropriate package name
 *      based on the ident data passed in.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 *
 * INPUT  : pointer ident data
 *
 * RETURNS: none
 * 
 */
void
print_pkg(identdata)
struct  identify_device *identdata;            /* pointer to identify data */
{

	if ( identdata->gen_config & ID_ATAPI ) {
		if ( ((identdata->gen_config & ID_DEVICE_TYPE_MASK) 
		      >> ID_DEVICE_TYPE_SHIFT)  == IDECDROM ) {
	    		fprintf(stdout, ":%s.ide.cdrom ", DEVPKG_PREFIX);
		} else {
			fprintf(stdout, ":%s.ide.tape ", DEVPKG_PREFIX);
		}	
	} else {
	    	fprintf(stdout, ":%s.ide.disk ", DEVPKG_PREFIX);
	}
}


/*
 * NAME   : det_utype
 *
 * FUNCTION :
 *      This function determines a device's unique type from the
 *      identify device data.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 *
 * INPUT  : pointer to identify device data.
 *
 * RETURNS: unique type
 * 
 * The routine determines uniquetype in 1 of 3 ways:
 *
 * 1) through the model_map PdAt default string
 * 2) model_name PdAt default string
 * 3) using default for disk, cdrom and tape	
 *
 */

char *
det_utype(adapter,id,identdata)
int     adapter;                        /* adapter file descriptor     */
uchar   id;                             /* IDE id of device            */
struct  identify_device *identdata;     /* pointer to ident data       */
{
	char    *ident_ptr;             /* local pointer to ident data  */
	char    *utype;                 /* pointer to unique type       */
	char    model_name[41];         /* area to hold model name      */
	int     ident_lth;              /* length of ident data         */
	int     i;                      /* loop variable                */
	int     index;                  /* index into PdAt object array */
	char	class[CLASSIZE];	/* class of uniquetype 		*/
	int	rc=0;			/* return code		        */
	ident_ptr = (char *)identdata;  /* point to ident data          */

	/* set uniquetype pointer to NULL in case no match */
	utype = NULL;

	utype = get_ide_utype(ident_ptr,pdobj_ptr,num_mma,mn_attr,num_mn,
				adapter,id);


	/* if allpkg set or no PdDv was found, we need to print out */
	/* the package name */
	if (allpkg || utype == NULL ) {
		print_pkg(identdata);
        }

	return(utype);
}


/*
 * NAME   : default_utype
 *
 * FUNCTION :
 *      This function finds the default unique type for a disk, CD-ROM,
 *      or tape device.
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
 *      This function matches the IDE devices actually with their
 *      corresponding CuDv object.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 * INPUT  : Number of IDE ids.
 *
 * RETURNS:
 *
 */

void
find_dev_in_cudv (int numids)
{
	int     i;			/* loop variable                    */
	int     j;			/* loop variable                    */
	char    *loc_con;		/* local variable for connection    */
	char    *loc_ut;		/* local variable for unique type   */
	int     num_of_pvid_matches;	/* number of disks with same pvid   */
	int     pvid_match;		/* CuDv disk object with same pvid  */
	int	first_null_match;	/* first CuDv disk object with same */
					/* parent and same ID but with      */
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
	/* with same parent, connection address (id), and pvid.             */
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
				/* same parent and same ID and has a          */
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
			/* parent, same ID, with a NULL pvid.  If we          */
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
	/* an OEM IDE device.  So we will try to see if the          */
	/* remaining devices are of this type.			     */


	chk_update_utype();
	


	/* If we get here, the device is a new device that needs to  */
	/* be defined.  However, we only define them if the Dflag    */
	/* is not set.  Traverse the realdevorder array based on ID  */
	/* to ensure correct name order.			     */

	if (Dflag == FALSE) {
	    for (i=0; i < numids; i++) {
		if (realdevorder[i] != NOT_IN_USE ) {
		    x=realdevorder[i];
		    if (real_dev[x].status == DONE)
		        continue;
		    else {
			    
		        DEBUG_1("defining new device at connection %s\n",
			        real_dev[x].connect);
		        def_ide_dev(real_dev[x].utype, pname,
			         real_dev[x].connect, real_dev[x].pvid);
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
 *	      device is an IBM defined device).
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

	if (!strcmp(old_type,"oicdrom") ||
	    !strcmp(old_type,"oidisk") ||
	    !strcmp(old_type,"oit")) {
	    

		/* If this device is an OEM CD-ROM, OEM disk,          */
		/* or an OEM tape, then give it the lowest level of 0. */
		old_type_level = 0;

	}
	else  {
	    

		/* Since this device is not an OEM, give */
		/* it a level of 2.				  */

		old_type_level = 2;

	}	

	if (!strcmp(new_type,"oicdrom") ||
	    !strcmp(new_type,"oidisk") ||
	    !strcmp(new_type,"oit")) {
	    

		/* If this device is an OEM CD-ROM, OEM disk,          */
		/* or an OEM tape, then give it the lowest level of 0. */
		new_type_level = 0;

	}
	else  {
	    

		/* Since this device is not an OEM, give */
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
 * NAME   : def_ide_dev
 *
 * FUNCTION :
 *      This function defines a new IDE device in the Database.
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
def_ide_dev(utype, parent, connect, pvid)
char    *utype;                 /* unique type of device */
char    *parent;                /* parent name (name of adapter) */
char    *connect;               /* connection on adapter (id) */
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
 *      This function updates CuDv object location for a device that has been
 *      found and is still present.
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
	ide_location(putype,plocation,cudv);

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
 * NAME   : save_dev_info
 *
 * FUNCTION :
 *      This function saves information about a IDE devices that has
 *      been detected.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 * INPUT  : IDE id, device's unique type, and pvid.  The pvid will
 *          be a null character if device has no pvid.
 *
 * RETURNS: none
 *
 */

void
save_dev_info(id,utype,pvid,dev_is_disk)
uchar   id;                     /* device's IDE id */
char    *utype;                 /* device's unique type */
char    *pvid;                  /* device's pvid */
uchar   dev_is_disk;            /* True if device is a disk */
{
	int	i;

	/* If unique type is a null pointer, we don't know about this */
	/* device type so don't save any information about it because */
	/* won't be able to match it with a CuDv object or define it. */
	if (utype) {
		i = num_rd - RD_BASE;	/* number of devs found beyond RD_BASE*/
		real_dev[num_rd].status = 0;
		real_dev[num_rd].dev_is_disk = dev_is_disk;
		strcpy(real_dev[num_rd].utype,utype);
		sprintf(real_dev[num_rd].connect,"%d",id);
		strcpy(real_dev[num_rd].pvid,pvid);

		DEBUG_3("device found, utype= %s, id= %s, pvid= %s\n",
					utype,real_dev[num_rd].connect,pvid)
		/* save the number of this real device into realdevorder   */
		/* array so that can index the array later in id order*/
		realdevorder[id] = num_rd ;

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
	struct  cust_device     *devs;  /* pointer to memory for CuDv info */
	int	mna_size;		/* size of model name attribute strct */
	int	cd_size;		/* size of customized device struct */
	char    sstr[256];              /* area for ODM search strings */
	int     rc;                     /* used for return codes */
	struct  listinfo 	mma_attr;/* list of model_map attributes     */	


	/* open PdAt object class */
	if ((int)(preatt = odm_open_class(PdAt_CLASS)) == -1){
		DEBUG_0("can't open PdAt class\n")
		return(E_ODMOPEN);
	}

	/* run through all PdAt objects and save information about IDE
	   model name */

	/* allocate initial space for holding model name attributes */
	i = 0;                  /* number of model name attributes */
	mna_size = sizeof(struct mna);
	mn = (struct mna *)malloc(MN_BASE * mna_size);

	/* set search string to look for devices in IDE subclass */
	strcpy(sstr,"uniquetype like */ide/*");
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
			/* exactly 40 chars, and no special chars */
			strncpy(mn[i].value2,PdAt.deflt,40);
			for(k=0; k<40; k++) {
				if (mn[i].value2[k] < ' ' ||
					mn[i].value2[k] > '\177') 
					mn[i].value2[k] = ' ';
			}
			mn[i].value2[40] = '\0';
			i++;
		}

		rc = (int)odm_get_next(preatt, &PdAt);
	}

	/* get all IDE model_map attributes */
	strcpy (sstr, "uniquetype like */ide/* AND attribute=model_map");
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


	/* run through all CuDv objects and save info about IDE devices */
	i = 0;                  /* number of IDE CuDv objects */
	cd_size = sizeof(struct cust_device);
	devs = (struct cust_device *)malloc(CD_BASE * cd_size);

	/* set search string to look for devices in IDE subclass */
	strcpy(sstr,"PdDvLn like */ide/*");

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

	/* run through all CuAt objects looking for IDE disk pvid */
	/* attributes and add to device info already saved */
	strcpy(sstr,"attribute = pvid");  /* scsi & ide will be listed */

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
 * INPUT  : adapter file descriptor, IDE id, pointer to
 *          area of memory which is to hold the pvid string.
 *
 * RETURNS: none
 *
 */

void
get_pvid_from_disk(adapter,id,pvidstr)
int     adapter;                        /* adapter file descriptor */
uchar   id;                             /* IDE id of disk */
char    *pvidstr;                       /* pointer to area to hold pvid */
{
	int     rc;
	char    pvidbuf[4096];          /* area for boot record, must be 4K */
	IPL_REC *iplrec;

	/* issue ioctl to adapter to get this drive's boot record */
	rc = read_ide_pvid(adapter, id, pvidbuf);
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
 * NAME	: read_ide_pvid
 *
 * FUNCTION :  This function tries to read the disk block containing
 *	the PVID information.
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
 *	Read is retried once if error is ETIMEDOUT or EIO.
 *
 **********************************************************************/

int read_ide_pvid(adapter, id, ptr)
int	adapter;
uchar	id;
uchar	 *ptr;
{
	struct ide_readblk pvid_data;
	int	blksiz;
	uchar	tried_async;
	int	rc;
	int	tries;

	DEBUG_3("read_ide_pvid: adapter_fd=%d, id=%d, ptr=%x\n",
		adapter,id,ptr);
	
	pvid_data.ide_device = id;
	pvid_data.blkno   = 0;
	pvid_data.timeout_value = 30;
	pvid_data.data_ptr = ptr;
	pvid_data.rsv1  = 0;
	pvid_data.flags = 0;
	
	for (blksiz = 512; blksiz <= 4096; blksiz <<= 1) {
		
		pvid_data.blksize  = blksiz;
		
		for (tries = 0; tries < 3; tries++) {
			rc = ioctl(adapter, IDEIOREAD, &pvid_data);
			if (rc == 0) {
				return(rc);
			}
			DEBUG_2("read_ide_pvid: blksiz=%d,tries = %d\n",
				blksiz,tries);
			/* else error condition */
			if (tries > 0) {
				
				switch (errno) {
				      case ENODEV:
					DEBUG_1("read_ide_pvid: ENODEV on %d.\n",
						id);
					break;
				      case ENOCONNECT:
					DEBUG_1("read_ide_pvid: ENOCONNECT @ %d.\n",
						id);
					break;
				      case ETIMEDOUT:
					pvid_data.timeout_value = 5;
					break;
				      case EIO:
					DEBUG_1("read_ide_pvid: EIO @ %d. do new blocksize\n",
						id);
					break;
				      default:
					DEBUG_2("read_ide_pvid: err=%d on %d\n",
						errno, id);
					rc = -1;
				}
			}
		}
	}
	return(rc);
}


/*
 * NAME: ide_location
 * 
 * FUNCTION: This function constructs the location code for an IDE device.
 * 
 * EXECUTION ENVIRONMENT:
 *
 *	This function is called by IDE configure methods.
 *
 * RETURNS: Returns with 0 on completion.
 */
int
ide_location (par_utype,par_loc,cusobj)
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
	        *ParType;		/* ptr to parent's types    */
	int	connection;		/* connection as an integer */

	DEBUG_0 ("Entering ide_location(): Build location value\n")

       /*------------------------------------------------------------ 
	*  Break out the parts of the parent's unique type for easier
	*  reference later.
	*------------------------------------------------------------*/

	strcpy(Parcst,par_utype);
	ParClass   = strtok(Parcst,"/");
	ParSubClass= strtok((char *)0,"/");
	ParType    = strtok((char *)0,"/");

	DEBUG_1("ide_location: par_utype = %s\n", par_utype) ;
	DEBUG_3("ide_location: ParClass= %s   ParSubClass= %s   ParType= %s\n",
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
	DEBUG_1 ("ide_location: connection = %s\n",cusobj->connwhere)
	DEBUG_1 ("ide_location: parent location = %s\n",par_loc)

	sprintf(cusobj->location,"%s-%02d",par_loc,connection);

	return(0);
}

