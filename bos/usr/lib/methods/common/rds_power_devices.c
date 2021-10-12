static char sccsid[] = "@(#)76  1.8  src/bos/usr/lib/methods/common/rds_power_devices.c, cfgmethods, bos41J, 9512A_all 3/20/95 08:21:23";
/*
 * COMPONENT_NAME: (CFGMETHODS) Configuration Functions
 *
 * FUNCTIONS:rds_get_maxlun,rds_get_device_adapter, rds_find_device
 *           rds_power_off
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
extern int num_ml;
extern int num_cd;
extern struct mla *ml_attr;
extern struct cust_device *cust_dev;
extern int mode;
struct entries_list *head;

int     max_lun;

/*
 * NAME   : rds_get_maxlun
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
rds_get_maxlun(utype, sid)
char   *utype;
int     sid;
{
    int     tmaxlun;	/* temporary max lun value */
    int     i;	/* loop variable */

    tmaxlun = max_lun;

    /* if a null pointer was passed in, we don't know the device */
    /* and will not adjust the max_lun value */
    if (utype) {
	/* get max_lun attribute for this device */
	for (i = 0; i < num_ml; i++) {
	    if (!strcmp(utype, ml_attr[i].value1)) {
		tmaxlun = ml_attr[i].value2;
		break;
	    }
	}
    }

    if (tmaxlun < max_lun) {
	DEBUG_2("max_lun for sid=%d is %d\n", sid, tmaxlun);
	max_lun = tmaxlun;
    }
}

/*
 * NAME: rds_get_device_adapter
 *
 * FUNCTION: To determine if a device is present at a location, and
 *           switch on this device if it needs to be configured.
 *
 * NOTES: This function uses rds_get_maxlun. 
 *
 * RETURNS: NOT_PRESENT if the device is not present
 *          PRESENT if the device is present
 */
int
rds_get_device_adapter(lname, cab_num, bus_num, adapter, sid, lun)
char   *lname;
int	cab_num;
int     bus_num;
int     adapter;
int     sid;
int     lun;
{
    struct sc_inquiry inq_pb;
    struct inqry_data inqstr;

    char    connect[3];
    char   *utype;
    uchar  *p;
    uchar   nq;
    int     rc = NOT_PRESENT;
    int     i;

    /* start this sid/lun */
    rc = ioctl(adapter, SCIOSTART, IDLUN(sid, lun));
    if (rc != 0) {
	DEBUG_2("Can not start sid=%d, lun=%d\n", sid, lun);
	sprintf(connect, "%d%d", sid, lun);
	for (i = 0; i < num_cd; i++) {
	    if (!strcmp(lname, cust_dev[i].cudv.parent) &&
		!strcmp(connect, cust_dev[i].cudv.connwhere) &&
		(cust_dev[i].cudv.status == AVAILABLE)) {
		DEBUG_0("rds_get_device_adapter: device already available\n")
		    utype = (char *) &cust_dev[i].cudv.PdDvLn_Lvalue;

		rds_get_maxlun(utype, sid);
		break;
	    }
	}
	return (NOT_PRESENT);
    }
    else {
	DEBUG_2("rds_get_device_adapter: started sid%d lun%d\n", sid, lun);
    }

    /* clear the inq structure area */
    p = (uchar *) & inq_pb;
    nq = sizeof(struct sc_inquiry);
    while (nq--)
	*p++ = 0;

    inq_pb.flags = 0;
    for (nq = 0; nq < 8; nq++) {
	inq_pb.scsi_id = sid;
	inq_pb.lun_id = lun;
	inq_pb.inquiry_len = SCSI_INQSIZE - 1;
	inq_pb.inquiry_ptr = (uchar *) & inqstr;

	rc = ioctl(adapter, SCIOINQU, &inq_pb);
	if (rc == 0) {
	    if ( inqstr.pdevtype == SCSI_PROC ){
                DEBUG_0("rds_get_device_adapter: adapter is found\n");
                rc = NOT_PRESENT;
                break;
            }
	    else {
	    	DEBUG_0("rds_get_device_adapter: disk is found\n");
	    	utype = det_utype(adapter, sid, lun, &inq_pb);

	    	rds_get_maxlun(utype, sid);
            	rc = PRESENT;
	    	break;
	    }
	}
	else {
	    if (errno == ENOCONNECT) {
		DEBUG_2("rds_get_device_adapter: ENOCONNECT on %d, %d\n", sid, lun);
		rc = NOT_PRESENT;
	    }
	    else {
		if (errno == ENODEV) {
		    DEBUG_2("rds_get_device_adapter:Nodevice responds on id%d lun%d\n", sid, lun);
		    max_lun = 0;
		    rc = NOT_PRESENT;
		    if ((sid < 3) && (bus_num == 0) && (cab_num == 0))
			sleep(1);
		    else
			if ((sid == 0) && (cab_num > 0))
                            sleep(1);
                        else
			    if (nq > 1)
			    	break;
		}
		else {
		    DEBUG_3("rds_get_device_adapter:err=%d on %d %d\n", errno, sid, lun);
		    if (nq > 0) {
			rc = NOT_PRESENT;
			break;
		    }
		}
	    }
	}
    }

    /* do SCIOSTOP for sid/lun */
    if (ioctl(adapter, SCIOSTOP, IDLUN(sid, lun)) < 0) {
	DEBUG_2("rds_get_device_adapter: error stopping sid%d lun%d\n", sid, lun)
	rc = NOT_PRESENT;
    }

    return (rc);
}

/*
 * NAME: rds_find_device
 *
 * FUNCTION: switch on all devices children of an adapter
 *
 * NOTES: None
 *
 * RETURNS: returns 0.
 *
 */
int
rds_find_device(lname, adapter, numsids, numluns)
char   *lname;
int     adapter;
int     numsids;
int     numluns;
{
    struct CuAt par_cuat;
    struct entries_list *busloc_list, *new_busloc_list, *rc_list, *p;
    int     found_flag = NOT_FOUND;
    int     i, rc, rc1;
    int     cab_num, bus_num, sid, lun;
    char    ss[256];
    char   *scab_num0, *sbus_num0;
    char    scab_num[2], sbus_num[2];
    char    bus_loc[4]="";

    max_lun = numluns - 1;
    DEBUG_0("found_flag is set to NOT_FOUND\n");

    /* get bus_loc attribute setting */
    sprintf(ss, "name = '%s' AND attribute=bus_loc", lname);
    rc = (int) odm_get_first(CuAt_CLASS, ss, &par_cuat);
    if (rc == -1) {
        /* ODM failure */
        DEBUG_0("ODM failure getting CuAt object\n");
        return (E_ODMGET);
    }
    if (rc == 0) {
        /* no CuAt object found */
        mode = NONE;
        DEBUG_0("bus_loc attribute not found\n");
    }
    else {
        if (strcmp(par_cuat.value, "X-X")) {
            DEBUG_1("Value find for bus_loc attribute: %s\n", par_cuat.value);
             sprintf(bus_loc, par_cuat.value);
            if (mode == OK)
                mode = OK;
            if (mode == NONE)
                mode = HINT;
        }
        else
            mode = NONE;
    }
    DEBUG_1("Mode active is: %d\n", mode);

    if (mode != OK) {
	for (sid = 0; sid < numsids; sid++) {
	    for (lun = 0; lun <= max_lun; lun++) {
		rc = rds_get_device_adapter(lname, -1, -1, adapter, sid, lun);
		if (rc == PRESENT) {
		    /*
		     * the adapter is associated with devices not in cabinet
		     * with RDS facility 
		     */
		    sprintf(bus_loc, "X-0");
		    rc = rds_setattr(lname, "bus_loc", bus_loc);
		    if (rc != 0) {
			DEBUG_0("Can not set bus_loc attribute\n");
			return (-1);
		    }
		    return (0);
		}
		else
		    if (rc != 0) {
			DEBUG_0("find_device: device not OK\n");
		    }
	    }
	}

	busloc_list = rds_build_busloc_list(NULL);
	head = busloc_list;
	DEBUG_0("Return from rds_build_busloc_list\n")
	    rds_remove_entry(&busloc_list, lname);
	DEBUG_0("Return from rds_remove_entry\n");

	DEBUG_1("Enter in rds_match_entry with bus_loc value: %s\n", bus_loc)
	    rc_list = rds_match_entry(busloc_list, bus_loc);
	DEBUG_0("Return from rds_match_entry\n")
	    if (((int) rc_list) != 0 && (mode == HINT)) {
	    DEBUG_0("Enter in swap entry\n");
	    new_busloc_list = rds_swap_entry(NULL, rc_list);
	    busloc_list = new_busloc_list;
	}

	if (strncmp(bus_loc, "X-0", 3)) {
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

		for (sid = 0; sid < numsids && found_flag != FOUND; sid++) {
		    DEBUG_0("Enter in for to run the sid\n");
		    rc = rds_find_device_present(cab_num, bus_num, sid);
		    DEBUG_0("Returned from rds_find_device_present\n");
		    if (rc == FOUND) {
			rc = rds_switch_power("on", cab_num, bus_num, sid);
			if (rc != 0) {
			    DEBUG_0("Can not switch on the device\n");
			    return (-1);
			}
			sleep(2);
			DEBUG_0("Returned from rds_switch_power on\n");
			rc = rds_get_device_adapter(lname, cab_num, bus_num, adapter, sid, 0);
			DEBUG_0("Returned from rds_get_device_adapter\n");
			if (rc == PRESENT) {
			    found_flag = FOUND;
			    if (bus_num == 0)
				sprintf(bus_loc, "%d-A", cab_num);
			    else
				sprintf(bus_loc, "%d-B", cab_num);
			    rc1 = rds_setattr(lname, "bus_loc", bus_loc);
			    if (rc1 != 0) {
				DEBUG_0("Can not set bus_loc attribute\n");
				return (-1);
			    }
			}
			else {
			    found_flag = NOT_FOUND;
			    rc1 = rds_switch_power("off", cab_num, bus_num, sid);
			    if (rc1 != 0) {
				DEBUG_0("Can not switch off the device\n");
				return (-1);
			    }
			    DEBUG_0("Returned from rds_switch_power off\n");
			}
		    }
		}
	    }
	}
	if (found_flag == NOT_FOUND) {
	    /* the adapter has no device connected */
	    rc = rds_setattr(lname, "bus_loc", "X-X");
	    if (rc != 0) {
		DEBUG_0("Can not set bus_loc attribute\n");
		return (-1);
	    }
	}
    }


    if ((found_flag == FOUND) || (mode == OK)) {
	sprintf(ss, "name = '%s' AND attribute=bus_loc", lname);
	rc = (int) odm_get_first(CuAt_CLASS, ss, &par_cuat);
	if (rc == -1) {
	    /* ODM failure */
	    DEBUG_0("ODM failure getting CuAt object\n");
	    return (E_ODMGET);
	}
	if (rc == 0) {
	    /* no CuAt object found */
	    DEBUG_0("bus_loc attribute not found\n");
	    return(E_NOATTR);
	}
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

	if (strncmp(par_cuat.value, "X-0", 3)) {
	    for (sid = 0; sid < numsids; sid++) {
		rc = rds_find_device_present(cab_num, bus_num, sid);
		DEBUG_0("Returned from rds_find_device_present\n");
		if (rc == NOT_FOUND)
		    continue;
		else {
		    rc = rds_switch_power("on", cab_num, bus_num, sid);
		    if (rc != 0) {
			DEBUG_0("Can not switch on the device\n");
			return (-1);
		    }
		    sleep(2);
		    DEBUG_0("Returned from rds_switch_power on\n");
		    rc = rds_get_device_adapter(lname, cab_num, bus_num, adapter, sid, 0);
		    DEBUG_1("Returned from rds_get_device_adapter with %d\n", rc);
		    if (rc == NOT_PRESENT) {
                         rc1 = rds_switch_power("off", cab_num, bus_num, sid);
                         if (rc1 != 0) {
                                DEBUG_0("Can not switch off the device\n");
                                return (-1);
                            }
                            DEBUG_0("Returned from rds_switch_power off\n");
                    }
		}
	    }
	}
    }

    return (0);
}
