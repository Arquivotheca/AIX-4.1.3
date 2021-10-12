#ifndef lint
static char sccsid[] = "@(#)33 1.26 src/bos/usr/lib/methods/cfgfda/cfgfda.c, cfgmethods, bos41J, 9513A_all 3/28/95 14:28:22";
#endif
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: build_dds
 *		chg_fd_attr
 *		define_children
 *		download_microcode
 *		generate_minor
 *		make_special_files
 *		query_vpd
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 */

/*
  This file contains the device dependent code pieces for SIO diskette adapter
  config method.

  required entry points:

  generate_minor(char *logical_name,dev_t major)

  make_special_files(char *logical_name,dev_t devno)

  download_microcode(char *logical_name)

  query_vpd(char *newobj,mid_t cfg.kmid,dev_t devno, char *vpd)

  define_children(char *logical_name,int phase)

*/

#define GETATT( DEST, TYPE, ATTR, CUDV ) {                              \
		int rc;                                                 \
		rc = getatt( DEST, TYPE, CuAt_CLASS, PdAt_CLASS,        \
		        CUDV.name, CUDV.PdDvLn_Lvalue, ATTR,            \
		        (struct attr *)NULL );                          \
		if (rc) {                                               \
                	DEBUG_2("%s: failed to get %s\n", 		\
				stubnm, ATTR); 				\
		        return(rc);                                     \
		}							\
	}
#define	UNIQ_TYPE	"diskette/siofd/fd"

#include <sys/types.h>
#include <cf.h>
#include <stdio.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/ioacc.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/fd.h>
#include "cfgcommon.h"
#include "cfgdebug.h"

extern	struct	CuAt	*getattr();

char stubnm[] = "cfgfda";

/*
 * NAME: generate_minor
 *
 * FUNCTION: This function generates device minor number for the specific
 *           device.
 *
 * EXECUTION ENVIRONMENT:
 *      This device specific routine is called by the generic config method
 *      to generate device minor number for the specific device by calling
 *      lib function genminor.
 *
 *
 * (NOTES:) The adapter minor number is always FD_ADAP_MINOR.
 *
 * RETURNS: Returns 0 on SUCCESS.
 */

long
generate_minor(lname, majorno, minorno)
char *lname;
long majorno;
long *minorno;

{
	long	*minor_list;		/* address getminor stored list    */
	long	max_so_far = 0x7fff;	/* maximum minor used in range     */
					/* 0x8000 is minimum minor number  */
					/* possible for adapter */
	int	how_many;		/* Number of minor numbers in list */

	/* Get all minor numbers allocated for this major number */
	minor_list = getminor( majorno, &how_many, (char *)NULL );

	/* Find the highest minor number already assigned within the range */
	while( how_many-- ) {
		if ( *minor_list > max_so_far )
			max_so_far = *minor_list;
		minor_list++;
	}
	
	/* Allocate next available minor number */
	minor_list = genminor(lname,majorno,max_so_far+1,1,1,1);

	if(minor_list == (long *)NULL)
		return(E_MINORNO);

	DEBUG_2("genminor(..%ld..)=%d\n", majorno, *minor_list )
	*minorno = *minor_list;
	return(0);
}


/*
 * NAME: build_dds
 *
 * FUNCTION: This function builds the DDS(Defined Data Structure) for the
 *           Diskette adapter.
 *
 * EXECUTION ENVIRONMENT:
 *
 * This function operates as a device dependent subroutine called by the
 * generic configure method for all devices. It is used to build the dds
 * which describes the characteristics of an adapter to the device driver.
 *
 * NOTES: A pointer to the DDS built and its size are returned to the generic
 *        configure method.
 *
 * RETURNS: Returns 0 for SUCCESS.
 */


build_dds(logical_name,ddsptr,ddslen)
	char *logical_name;
	char **ddsptr;
	int *ddslen;
{
        struct  CuDv    bus_cudv;
        static	struct  fda_config       dds;   /* dds passed to driver */
        int     rc;

        DEBUG_0("cfgfda: in build_dds()\n");

	/* retrieve all the diskette adapter attributes from the database */
	/* these are common between the ISA driver and the bos driver     */
	GETATT (&dds.bus_int_level, 'i', "bus_intr_lvl", cudv);
	GETATT (&dds.dma_level, 'i', "dma_lvl", cudv);
	GETATT (&dds.io_address, 'i', "bus_io_addr", cudv);
	GETATT (&dds.int_class, 'c', "intr_priority", cudv);
	GETATT (&dds.adapter_type, 'c', "adapter_type", cudv);

#ifdef ISA
	/* retrieve the ISA specific attributes */
	GETATT (&dds.device_idle_time, 'i', "pm_dev_itime", cudv);
	GETATT (&dds.device_standby_time, 'i', "pm_dev_stime", cudv);
	strncpy (dds.adap_name, logical_name, 8);

	/* this attribute does not currently exist in the database  */
	/* if it needs to be added, this will change to a GETATT    */
	/* but the device driver and 'fd.h' will not have to change */
	dds.pm_attribute = 0;

	DEBUG_3("cfgfda: idle time = %d, standby time = %d, name = %s ", 
		dds.device_idle_time, dds.device_standby_time, dds.adap_name)
	DEBUG_1("pm attribute = %d\n", dds.pm_attribute)
#endif
	

        DEBUG_4("cfgfda: intlvl = %d, dma = %d, intr_pri = %d, io_addr = %x\n",
               dds.bus_int_level, dds.dma_level, dds.int_class, dds.io_address);
        DEBUG_1("cfgfda: adapter_type = %d\n", dds.adapter_type)


        /*
        now get slot number from parent SIO object
        */
        dds.slot_num = (atol(pcudv.connwhere) - 1) & 0x0f;
        DEBUG_1("slot = %d\n",dds.slot_num);

        /*
        now get bus object for it's attributes
        busid, bustype
        */
	rc = Get_Parent_Bus(CuDv_CLASS,cudv.parent,&bus_cudv);
	if (rc)
		return(rc);

        DEBUG_2("sio parent = %s utype = %s\n",
		bus_cudv.name, bus_cudv.PdDvLn_Lvalue);

        /* PCI and ISA busses do not have a bus_type attribute,
           so do not try to obtain it */
#ifdef BUS_TYPE
	GETATT (&dds.bus_type, 'h', "bus_type", bus_cudv);
        DEBUG_1("bustype = %d\n",dds.bus_type);
#endif
	GETATT (&dds.bus_id, 'l', "bus_id", bus_cudv);
        DEBUG_1("busid = %x\n",dds.bus_id);

        *ddslen = sizeof(struct fda_config);
        *ddsptr = (char *) &dds;

	return(E_OK);
}

make_special_files(logical_name,devno)
	char	*logical_name;
	dev_t	devno;
{
	return(E_OK);
}
download_microcode(logical_name)
	char	*logical_name;

{
	return(E_OK);
}
query_vpd(newobj,kmid,devno,vpd)
	char	*newobj;
	mid_t	kmid;
	dev_t	devno;
	char	*vpd;
{
	return(E_OK);
}

define_children(logical_name,phase)
	char	*logical_name;
	int	phase;
{
	int	ndx;
	int	rc;
	struct	PdDv preobj;
	struct	CuDv cusobj;
	int     d_length;
	char	d_name[128];
	char	sstring[256];
	char	pstr[128];
	char	*outp,*errp;
	struct	cfg_dd cfg;
	union	fd_config vpd; 

	DEBUG_1("in %s define_children()\n",stubnm);

	DEBUG_1("the driver's kmid is %x...\n", kmid);

	/* The correct kmid may not be set when calling "define_children" */
	/* when the adapter is already in the "available" state.          */
	/* This bit of code makes sure the kmid is set correctly	  */
        rc = get_dvdr_name();
        if (rc) {
                err_exit(rc);
        }

        /* call loadext to load the device driver */
        if ((kmid = loadext(dvdr, FALSE, TRUE)) == NULL) {
		DEBUG_0 ("error on loadext\n");
                /* error loading device driver */
                err_exit(E_LOADEXT);
        }
	DEBUG_1("the driver's kmid is %x...\n", kmid);

	cfg.kmid = kmid;
	cfg.devno = makedev(0,0);
	cfg.cmd = CFG_QVPD;
	cfg.ddsptr = (void*)&vpd;
	cfg.ddslen = sizeof(union fd_config);
	DEBUG_0("calling CFG_QVPD in config entry point...\n");

	/* does not get real vpd, actually detects presence of drives */
	if (sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd))==-1) {
		DEBUG_0("CFG_QVPD failed...\n");
		DEBUG_2("Drive 0 status is %x, drive 1 status is %x...\n",
		    vpd.drive_info[0], vpd.drive_info[1]);
		DEBUG_1("%s: sense drive presence failed", stubnm);
		return(E_SYSCONFIG);
	}
	DEBUG_0("CFG_QVPD in config entry point returned ok...\n");
	DEBUG_2("Drive 0 status is %x, drive 1 status is %x...\n",
	    vpd.drive_info[0], vpd.drive_info[1]);

	/*
	  we will use a built-in array with the essential information
	  for running children
	  */
	for (ndx = 0; ndx < 2; ndx++) {		/* max devices is 2 */
		sprintf(sstring , "parent=%s and connwhere=%d",
			logical_name,ndx);
		DEBUG_2("%s: will try \"%s\".\n", stubnm,sstring);

		/* if the drive is not present, skip it */
		if (!(vpd.drive_info[ndx] & FD_PRESENT))
			continue;
				
		if ((rc = (int)odm_get_first(CuDv_CLASS,sstring,&cusobj))==0) {
			/*
			   child not defined
			   */
			sprintf(sstring,"uniquetype=%s",
				UNIQ_TYPE);
			if ((rc=(int)odm_get_first(PdDv_CLASS,sstring,&preobj)) == 0 || rc ==-1) {
				DEBUG_2("%s: get_obj \"%s\" bad\n",
					stubnm,sstring);
				continue; /* no predefined, skip on
					    to next array cell */
			}
			/*
			  we have the predefined object in our hands now
			  */
			sprintf(pstr,"-c %s -s %s -t %s -p %s -w %d",
				preobj.class,preobj.subclass,
				preobj.type,logical_name,ndx);
			DEBUG_2("%s: will try to run \"%s\"\n",
				stubnm,pstr);
	        	if ((rc = odm_run_method(preobj.Define,pstr,&outp,
			    &errp)) == -1) {
				DEBUG_2("%s failed with message: \"%s\"\n",
				    pstr,errp);
       			} else
			 {
				fprintf(stdout, "%s\n", outp);
				if (((char)vpd.drive_info[ndx] & 0xff)
				     == D_1354H) {
					d_length = strlen(outp) - 1;
					strncpy(d_name, outp, d_length);
					d_name[d_length] = '\0';
					DEBUG_1("dname = %s\n", d_name)
					if (chg_fd_attr(d_name,"fdtype",
						"3.5inch4Mb") != 0) {
						DEBUG_1(
						    "%s: couldnt change fdtype",
						    stubnm);
						return(E_ODMUPDATE);
					}
				} else
				if (ndx == 1) {
					d_length = strlen(outp) - 1;
					strncpy(d_name, outp, d_length);
					d_name[d_length] = '\0';
					if (chg_fd_attr(d_name,"fdtype",
						"5.25inch") != 0) {
						DEBUG_1(
						    "%s: couldnt change fdtype",
						    stubnm);
						return(E_ODMUPDATE);
					}
				} /* if connected to port 1 */
			}
		} else if (rc == -1) {
			/*
			  we will try next entry if this one is bad
			  */
			DEBUG_2("%s: obj_get \"%s\" failed",
				stubnm,sstring);
                } else if (cusobj.status == DEFINED) {
                        if (cusobj.chgstatus == MISSING)
                                cusobj.chgstatus = SAME;
                        if (odm_change_obj(CuDv_CLASS,&cusobj) == -1) {
                                DEBUG_2("%s: change_obj \"%s\" failed",
                                    stubnm,sstring);
                        }
			fprintf(stdout, "%s\n", cusobj.name);
                }
	}
	return(E_OK);
}

chg_fd_attr(devname, attrname, attrval)
	char	*devname;
	char	*attrname;
	char	*attrval;
{

	int	rc=0;
	int	howmany;
	struct	CuAt	*cuatptr;

	/* first get current attr value structure */
	cuatptr = getattr(devname, attrname, FALSE, &howmany);
	if (cuatptr == NULL) {
		DEBUG_2("chg_fd_attr: failed to get %s attribute for %s.\n",
		    attrname, devname);
		return(1);
	}
	DEBUG_2("chg_fd_attr: successfully got %s attribute for %s.\n",
	    attrname, devname);

	/* now set to new value */
	strcpy(cuatptr->value, attrval);
	if (putattr(cuatptr) == -1) {
		DEBUG_2("chg_fd_attr: failed to set %s attribute to %s.\n",
		    attrname, attrval);
		rc = 2;
	}
	DEBUG_2("chg_fd_attr: successfully set %s attribute for %s.\n",
	    attrname, attrval);

	/* free memory allocated by getattr */
	free(cuatptr);

	return(rc);
} /* chg_fd_attr() */


/*
 * NAME     : device_specific
 *
 * FUNCTION : This function allows for device specific code to be
 *            executed.
 *
 * NOTES :
 *      This adapter does not have a special task to do, so this routine does
 *      nothing
 *
 * RETURNS : 0
 */

int device_specific()
{

        return 0;
}

