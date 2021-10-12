static char sccsid[] = "@(#)78  1.3  src/bos/usr/lib/methods/cfgscdisk/rds_power_disk.c, cfgmethods, bos412, 9446B 10/28/94 10:42:14";
/*
 * COMPONENT_NAME: (CFGMETHODS) Configuration Functions
 *
 * FUNCTIONS: rds_power_on_disk, rds_power_off_disk,rds_disk_present
 *
 * ORIGINS: 83
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/sysmacros.h>
#include <sys/cfgdb.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/mdio.h>
#include <sys/scsi.h>

#include "cfghscsi.h"
#include "cfgdebug.h"
#include "rds_common.h"

/* Global variables */
extern char device_loc[];
extern struct entries_list *head;

#define CALL_FINDDISK   -1

#define INQSIZE 255
static uchar inq_data[INQSIZE];	/* Storage area for inquery data */

/*
 * NAME: rds_power_on_disk
 *
 * FUNCTION: switch on all disk children of an adapter
 *
 * NOTES: None
 *
 * RETURNS: returns 0.
 *
 */
int
rds_power_on_disk(disk, adapter, sid, lun)
struct CuDv *disk;
int     adapter;
uchar   sid;
uchar   lun;
{
    struct CuAt cusattpar;
    struct entries_list *busloc_list, *new_busloc_list, *rc_list, *p;
    int     found_flag = NOT_FOUND;
    char   *scab_num0, *sbus_num0;
    char    scab_num[2], sbus_num[2];

    int     cab_num, bus_num, rc, rc1;
    char    sstring[256];
    char    loc[16];
    char    bus_loc[4]="";

    DEBUG_0("rds_power_on_disk is running\n");
    sprintf(sstring, "name='%s' and attribute=bus_loc", disk->parent);
    DEBUG_1("rds_power_on_disk: find attribute corresponding to %s\n", sstring);

    rc = (int) odm_get_first(CuAt_CLASS, sstring, &cusattpar);
    if (rc == -1) {
	/* ODM failure */
	DEBUG_0("rds_power_on_disk: ODM failure getting CuAt object\n");
	return (E_ODMGET);
    }
    if (rc == 0) {
	/* no CuAt object found */
	DEBUG_0("rds_power_on_disk: bus_loc attribute not found\n");
    }

    if (!strncmp(cusattpar.value, "X-X", 3)) {
	busloc_list = rds_build_busloc_list(NULL);
	head = busloc_list;
	DEBUG_0("Return from rds_build_busloc_list\n");

	rds_remove_entry(&busloc_list, disk->parent);
	DEBUG_0("Return from rds_remove_entry\n");

	DEBUG_1("Enter in rds_match_entry with bus_loc value: %s\n", cusattpar.value);
	rc_list = rds_match_entry(busloc_list, cusattpar.value);
	DEBUG_0("Return from rds_match_entry\n");
	if (((int) rc_list) != 0) {
	    DEBUG_0("Enter in swap entry\n");
	    new_busloc_list = rds_swap_entry(NULL, rc_list);
	    busloc_list = new_busloc_list;
	}

	for (p = busloc_list; p != NULL && found_flag != FOUND; p = p->next) {
	    DEBUG_0("Enter in for to run the list\n");
	    scab_num[0] = scab_num[1] = '\0';
	    scab_num0 = (char *) strtok(p->value, "-");
	    scab_num[0] = *scab_num0;
	    sbus_num0 = (char *) strtok((char *) 0, "-");
	    strncpy(sbus_num, sbus_num0, 1);

	    cab_num = atoi(scab_num);
	    DEBUG_1("Cabinet number tested: %d\n", cab_num);
	    if (!strncmp(sbus_num, "A", 1))
		bus_num = 0;
	    else
		bus_num = 1;
	    DEBUG_1("Bus number tested: %d\n", bus_num);


	    rc = rds_find_device_present(cab_num, bus_num, (int) sid);
	    DEBUG_0("Returned from rds_find_device_present\n");
	    if (rc == FOUND) {
		rc = rds_switch_power("on", cab_num, bus_num, (int) sid);
		if (rc != 0) {
		    DEBUG_0("Can not switch on the device\n");
		    return (-1);
		}
		sleep(2);
		DEBUG_0("Returned from rds_switch_power on\n");
		rc = rds_get_device(disk->name, cab_num, bus_num, adapter, (int) sid, 0);
		DEBUG_0("Returned from rds_get_device\n");
		if (rc == PRESENT) {
		    found_flag = FOUND;
		    if (bus_num == 0)
			sprintf(bus_loc, "%d-A", cab_num);
		    else
			sprintf(bus_loc, "%d-B", cab_num);
		    rc1 = rds_setattr(disk->parent, "bus_loc", bus_loc);
		    if (rc1 != 0) {
			DEBUG_0("Can not set bus_loc attribute\n");
			return (-1);
		    }
		    strcpy(loc, disk->location);
		    sprintf(disk->location, "%d%c-%c%c-%c%c-%s",
			    cab_num, loc[1],
			    loc[3], loc[4],
			    sbus_num[0], loc[7],
			    disk->connwhere);
		    DEBUG_1("location code value: %s\n", disk->location);
		    sprintf(device_loc, "%1d-%1d-%1d", cab_num, bus_num, (int) sid);
		}
		else {
		    rc1 = rds_switch_power("off", cab_num, bus_num, (int) sid);
		    if (rc1 != 0) {
			DEBUG_0("Can not switch off the device\n");
			return (-1);
		    }
		    DEBUG_0("Returned from rds_switch_power off\n");

		}
	    }
	}


	if (found_flag == NOT_FOUND) {
	    rc = rds_get_device(disk->name, -1, -1, adapter, (int) sid, (int) lun);
	    if (rc == PRESENT) {
		rc1 = rds_setattr(disk->parent, "bus_loc", "X-0");
		if (rc1 != 0) {
		    DEBUG_0("Can not set bus_loc attribute\n");
		    return (-1);
		}
		strcpy(loc, disk->location);
		sprintf(disk->location, "%c%c-%c%c-%c%c-%s",
			'X', loc[1],
			loc[3], loc[4],
			'0', loc[7],
			disk->connwhere);
		DEBUG_1("location code value: %s\n", disk->location);
	    }
	}

    }

    else {
	if (strncmp(cusattpar.value, "X-0", 3)) {
	    cab_num = cusattpar.value[0] & 0x0F;
	    DEBUG_1("Cabinet number %d\n", cab_num);
	    if (cusattpar.value[2] == 'A') {
		DEBUG_0("Bus number : 0\n");
		bus_num = 0;
	    }
	    else {
		DEBUG_0("Bus number : 1\n");
		bus_num = 1;
	    }

	    rc = rds_find_device_present(cab_num, bus_num, (int) sid);
	    DEBUG_0("Returned from rds_find_device_present\n");
	    if (rc == FOUND) {
		rc = rds_switch_power("on", cab_num, bus_num, (int) sid);
		if (rc != 0) {
		    DEBUG_0("Can not switch on the device\n");
		    return (-1);
		}
		sleep(2);
		DEBUG_0("Returned from rds_switch_power on\n");
		rc = rds_get_device(disk->name, cab_num, bus_num, adapter, (int) sid, 0);
		DEBUG_0("Returned from rds_get_device\n");
		if (rc == PRESENT) {
		    sprintf(device_loc, "%1d-%1d-%1d", cab_num, bus_num, (int) sid);
		    DEBUG_0("Returned from rds_get_device and rc=0\n");
		}
		else {
		    rc1 = rds_switch_power("off", cab_num, bus_num, (int) sid);
		    if (rc1 != 0) {
			DEBUG_0("Can not switch off the device\n");
			return (-1);
		    }

		    DEBUG_0("Returned from rds_switch_power off\n");
		}

	    }

	}
    }

    DEBUG_0("End of rds_power_on_disk\n");
    return (0);
}

/*
 * NAME: rds_power_off_disk
 *
 * FUNCTION: switch off a disk
 *
 * NOTES: None
 *
 * RETURNS: returns 0.
 *
 */
int
rds_power_off_disk(cudv_disk)
struct CuDv *cudv_disk;	/* disk's CuDv object */
{
    struct CuAt par_cuat;
    int     cab_num, bus_num;
    int     sid, lun;
    int     disk;
    int     status;
    int     rc;
    int     nvram;
    char    ss[256];
    MACH_DD_IO param;

    sprintf(ss, "name = '%s' AND attribute=bus_loc", cudv_disk->parent);
    rc = (int) odm_get_first(CuAt_CLASS, ss, &par_cuat);
    if (rc == -1) {
	/* ODM failure */
	DEBUG_0("ODM failure getting CuAt object\n");
	return (E_ODMGET);
    }
    if (rc == 0) {
	/* no CuAt object found */
	DEBUG_0("bus_loc attribute not found\n");
    }

    DEBUG_0("CuAt of the father is taken\n");
    cab_num = par_cuat.value[0] & 0x0F;
    DEBUG_1("Cabinet number %d\n", cab_num);
    if (par_cuat.value[2] == 'A') {
	DEBUG_0("Bus number : 0\n");
	bus_num = 0;
    }
    else {
	DEBUG_0("Bus number : 1\n");
	bus_num = 1;
    }

    sid = (int) strtoul(cudv_disk->connwhere, NULL, 10);
    lun = (int) strtoul(strchr(cudv_disk->connwhere, ',') + 1, NULL, 10);
    DEBUG_1("Sid off %d\n", sid);

    /* open nvram to enable ioctls usage */
    nvram = open("/dev/nvram", O_RDWR);
    if (nvram == -1) {
	DEBUG_0("Unable to open /dev/nvram\n");
	return (E_DEVACCESS);
    }

    status = MRDS_OFF;

    disk = sid | (bus_num << 4);

    param.md_size = 1;
    param.md_incr = MV_WORD;

    param.md_cbnum = cab_num;
    param.md_dknum = disk;
    param.md_data = (char *) &status;

    /* switch disk off */
    rc = ioctl(nvram, MRDSSET, &param);
    if (rc == -1) {
	DEBUG_0("Error reading with RDS\n");
	close(nvram);
	return (E_DEVACCESS);
    }

    close(nvram);

    return (0);
}

/*
 * NAME: rds_disk_present
 *
 * FUNCTION: Device dependent routine that checks to see that the disk device
 *           is still at the same connection location on the same parent as
 *           is indicated in the disk's CuDv object.
 *
 * EXECUTION ENVIRONMENT:
 *    This code is designed to be linked with cfgdisk.
 *
 * RETURNS:     0 if succeed, or errno if fail
 */

int
rds_disk_present(disk, preobj, parent, pvidattr)
struct CuDv *disk;	/* disk's CuDv object */
struct PdDv *preobj;	/* disk's PdDv object */
struct CuDv *parent;	/* disk's parents CuDv object */
char   *pvidattr;	/* disk's CuAt pvid attribute value */
{
    struct CuAt dpvidattr;
    struct CuDv tmpcudv;
    int     dpvid, adap;
    int     rc;
    char    pvidstr[33];
    char    adapname[64];
    char    sstr[256];
    uchar   sid, lun;	/* SCSI Id and LUN of disk */
    int     error;

    /* make sure parent is configured */
    if (parent->status != AVAILABLE) {
	DEBUG_1("disk_present: parent status=%d\n", parent->status);
	return (CALL_FINDDISK);
    }

    /* Check to see if there is another disk already AVAILABLE at the */
    /* same parent/parent connection.  If there is, then will need to */
    /* call finddidsk() to see if this disk was moved. */
    sprintf(sstr, "parent = '%s' AND connwhere = '%s' AND status = '%d'",
	    disk->parent, disk->connwhere, AVAILABLE);
    rc = (int) odm_get_first(CuDv_CLASS, sstr, &tmpcudv);
    if (rc == -1) {
	DEBUG_0("disk_present: ODM error.\n")
	    return (E_ODMGET);
    }
    else
	if (rc) {
	    DEBUG_1("disk_present: %s already cfged at this location.\n", tmpcudv.name)
		return (CALL_FINDDISK);
	}

    /* get scsi id and lun from connwhere field */
    if (get_sid_lun(disk->connwhere, &sid, &lun))
	return (E_INVCONNECT);

    /* get inquiry data from drive */
    sprintf(adapname, "/dev/%s", disk->parent);
    if ((adap = open(adapname, O_RDWR)) < 0) {
	/* error opening adapter */
	DEBUG_2("disk_present: error opening %s, errno=%d\n", adapname, errno)
	    return (CALL_FINDDISK);
    }


    /* switch on the disk */
    rc = rds_power_on_disk(disk, adap, sid, lun);
    if (rc != 0)
	DEBUG_0("Problem in rds_power_on_disk\n");

    /* issue SCIOSTART for this id/lun */
    if (ioctl(adap, SCIOSTART, IDLUN((int) sid, (int) lun)) < 0) {
	DEBUG_3("disk_present: Failed to SCIOSTART device at sid %d lun %d errno=%d\n", sid, lun, errno)
	    close(adap);
	rc = rds_power_off_disk(disk);
	if (rc != 0)
	    DEBUG_0("Problem in rds_power_off_disk\n");
	return (CALL_FINDDISK);
    }

    /* get inquiry data to make sure this is a disk */
    DEBUG_3("disk_present: calling issue_scsi_inq adap=%s sid=%d lun=%d\n", adapname, sid, lun)
	if ((rc = issue_scsi_inquiry(adap, inq_data, sid, lun, (int) NO_PAGE,&error)) != 0) {
	/* error getting inquiry data from drive */
	DEBUG_4("disk_present: SCIOINQU error rc=%d adap=%s sid=%d lun=%d\n", rc, adapname, sid, lun)
	    ioctl(adap, SCIOSTOP, IDLUN((int) sid, (int) lun));
	close(adap);
	rc = rds_power_off_disk(disk);
	if (rc != 0)
	    DEBUG_0("Problem in rds_power_off_disk\n");
	return (CALL_FINDDISK);
    }

    /* make sure inquiry data is for a disk */
    if ((inq_data[0] & '\037') != '\000') {
	DEBUG_0("Device is of wrong class entirely\n")
	    ioctl(adap, SCIOSTOP, IDLUN((int) sid, (int) lun));
	close(adap);
	rc = rds_power_off_disk(disk);
	if (rc != 0)
	    DEBUG_0("Problem in rds_power_off_disk\n");
	return (CALL_FINDDISK);	/* Device class is incorrect */
    }

    /* make sure disk is spun up */
    DEBUG_2("disk_present: calling spin_up_disk sid=%d lun=%d\n", sid, lun)
	if (spin_up_disk(adap, sid, lun) != 0) {
	DEBUG_2("disk_present: Failed to spin up disks sid=%d lun=%d\n", sid, lun);
	ioctl(adap, SCIOSTOP, IDLUN((int) sid, (int) lun));
	close(adap);
	rc = rds_power_off_disk(disk);
	if (rc != 0)
	    DEBUG_0("Problem in rds_power_off_disk\n");
	return (-1);
    }

    /* now get final inquiry data */
    DEBUG_3("disk_present: calling issue_scsi_inq adap=%s sid=%d lun=%d\n",
	    adapname, sid, lun)
	if ((rc = issue_scsi_inquiry(adap, inq_data, sid, lun, (int) NO_PAGE,&error)) != 0) {
	/* error getting inquiry data from drive */
	DEBUG_4("disk_present: SCIOINQU error rc=%d adap=%s sid=%d lun=%d\n", rc, adapname, sid, lun)
	    ioctl(adap, SCIOSTOP, IDLUN((int) sid, (int) lun));
	close(adap);
	rc = rds_power_off_disk(disk);
	if (rc != 0)
	    DEBUG_0("Problem in rds_power_off_disk\n");
	return (CALL_FINDDISK);
    }

#ifdef CFGDEBUG
    /* look at inquiry data */
    hexdump(inq_data, (long) (inq_data[4] + 5));
#endif

    /* get pvid from the disk */
    get_pvid(adap, sid, lun, pvidstr);

    /* issue SCIOSTOP to sid/lun */
    if (ioctl(adap, SCIOSTOP, IDLUN((int) sid, (int) lun)) < 0) {
	DEBUG_2("disk_present: Failed to SCIOSTOP device at scsi %d lun %d\n", (int) sid, (int) lun)
	    close(adap);
	rc = rds_power_off_disk(disk);
	if (rc != 0)
	    DEBUG_0("Problem in rds_power_off_disk\n");
	return (CALL_FINDDISK);
    }

    close(adap);

    /* make sure drive is the right type */
    DEBUG_0("disk_present: calling chktype\n")
	rc = chktype(inq_data, disk->PdDvLn_Lvalue, SCSI_DISK,disk->parent,sid,lun);
    if (rc) {
	if (rc == E_ODMGET) {
	    /* error checking on drive types */
	    DEBUG_0("ODM error\n")
		rc = rds_power_off_disk(disk);
	    if (rc != 0)
		DEBUG_0("Problem in rds_power_off_disk\n");
	    return (E_ODMGET);
	}
	else {
	    DEBUG_0("Wrong type of disk\n")
		rc = rds_power_off_disk(disk);
	    if (rc != 0)
		DEBUG_0("Problem in rds_power_off_disk\n");
	    return (CALL_FINDDISK);
	}
    }

/* SHOULDNT WE CHECK FOR SAME DRIVE TYPE IN FOLLOWING TEST? */
    /*
     * if the drive has a pvid then see if there is an object for it in the
     * database 
     */
    dpvid = 0;	/* initialize to no pvid attr */
    if (*pvidstr) {
	sprintf(sstr, "attribute = 'pvid' AND value = '%s'", pvidstr);
	dpvid = (int) odm_get_first(CuAt_CLASS, sstr, &dpvidattr);
	if (dpvid == -1) {
	    rc = rds_power_off_disk(disk);
	    if (rc != 0)
		DEBUG_0("Problem in rds_power_off_disk\n");
	    return (E_ODMGET);
	}
    }

    /*
     * If the CuDv disk object we want to configure has NO pvid attribute but
     * the disk has a pvid on it and there's a matching pvid attribute in the
     * database 
     *
     * OR 
     * If the CuDv disk object we want to configure HAS a pvid attribute and the
     * disk has a pvid attribute on it but they don't match 
     *
     * THEN the disk is the wrong device 
     */
    DEBUG_1("disk pvid = %s\n", pvidstr)
	DEBUG_1("CuAt pvid = %s\n", pvidattr)
	if (((pvidattr[0] == '\0') && *pvidstr && dpvid) ||
	    ((pvidattr[0] != '\0') && strcmp(pvidattr, pvidstr))) {
	rc = CALL_FINDDISK;
	rds_power_off_disk(disk);
    }
    else {
	rc = E_OK;
    }
    DEBUG_1("disk_present: returning %d\n", rc);
    return (rc);
}
