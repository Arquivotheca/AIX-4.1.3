static char sccsid[] = "@(#)77  1.3  src/bos/usr/lib/methods/common/rds_power_on_slot.c, cfgmethods, bos412, 9446B 10/28/94 10:53:37";
/*
 * COMPONENT_NAME: (CFGMETHODS) Configuration Functions
 *
 * FUNCTIONS:rds_power_on_device, rds_update_location
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

/*
 * NAME: rds_power_on_device
 *
 * FUNCTION: switch on all devices children of an adapter
 *
 * NOTES: None
 *
 * RETURNS: returns 0 and error code.
 *
 */
int
rds_power_on_device(cusobj)
struct CuDv *cusobj;
{
    struct CuAt cusattpar;
    struct entries_list *busloc_list, *new_busloc_list, *rc_list, *p;
    int     found_flag = NOT_FOUND;
    char    adap_dev[32];	/* adapter device /dev name */
    int     adapter;	/* file descriptor for adapter */
    int     sid, lun;
    char   *scab_num0, *sbus_num0;
    char    scab_num[2], sbus_num[2];
    int     cab_num, bus_num, rc, rc1;
    char    sstring[256];
    char    bus_loc[4]="";

    sid = (int) strtoul(cusobj->connwhere, NULL, 10);
    lun = (int) strtoul(strchr(cusobj->connwhere, ',') + 1, NULL, 10);

    /* open adapter device */
    sprintf(adap_dev, "/dev/%s", cusobj->parent);
    if ((adapter = open(adap_dev, O_RDWR)) < 0) {
	DEBUG_1("rds_power_on_device:can not open %s \n", adap_dev)
	    return (E_DEVACCESS);
    }

    DEBUG_0("rds_power_on_device is running\n");
    sprintf(sstring, "name='%s' and attribute=bus_loc", cusobj->parent);
    DEBUG_1("rds_power_on_device: find attribute corresponding to %s\n", sstring);

    rc = (int) odm_get_first(CuAt_CLASS, sstring, &cusattpar);
    if (rc == -1) {
	/* ODM failure */
	DEBUG_0("rds_power_on_device: ODM failure getting CuAt object\n");
	close(adapter);
	return (E_ODMGET);
    }
    if (rc == 0) {
	/* no CuAt object found */
	DEBUG_0("rds_power_on_device: bus_loc attribute not found\n");
	close(adapter);
	return(E_NOATTR);
    }

    if (!strncmp(cusattpar.value, "X-X", 3)) {
	/*
	 * It is the case of an adapter without devices previously connected
	 * with it 
	 */
	busloc_list = rds_build_busloc_list(NULL, cusobj->parent);
	head = busloc_list;
	DEBUG_0("rds_power_on_device: return from rds_build_busloc_list\n");

	rds_remove_entry(&busloc_list);
	DEBUG_0("rds_power_on_device: return from rds_remove_entry\n");

	DEBUG_1("rds_power_on_device: rds_match_entry with bus_loc value: %s\n", cusattpar.value);
	rc_list = rds_match_entry(busloc_list, cusattpar.value);
	DEBUG_0("rds_power_on_device: return from rds_match_entry\n");
	if (((int) rc_list) != 0) {
	    DEBUG_0("rds_power_on_device: swap entry\n");
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
	    DEBUG_1("rds_power_on_device: cabinet number tested: %d\n", cab_num);
	    if (!strncmp(sbus_num, "A", 1))
		bus_num = 0;
	    if (!strncmp(sbus_num, "B", 1))
		bus_num = 1;
	    DEBUG_1("rds_power_on_device: bus number tested: %d\n", bus_num);

	    rc = rds_find_device_present(cab_num, bus_num, sid);
	    DEBUG_0("rds_power_on_device: return from rds_find_device_present\n");
	    if (rc == FOUND) {
		rc = rds_switch_power("on", cab_num, bus_num, sid);
		if (rc != 0) {
		    DEBUG_0("Can not switch on the device\n");
		    close(adapter);
		    return (-1);
		}
		sleep(2);
		DEBUG_0("rds_power_on_device: returned from rds_switch_power on\n");
		rc = rds_get_device(cusobj->name, cab_num, bus_num, adapter, sid, 0);
		DEBUG_0("rds_power_on_device: returned from rds_get_device\n");
		if (rc == PRESENT) {
		    found_flag = FOUND;
		    if (bus_num == 0)
			sprintf(bus_loc, "%d-A", cab_num);
		    else
			sprintf(bus_loc, "%d-B", cab_num);
		    rc1 = rds_setattr(cusobj->parent, "bus_loc", bus_loc);
		    if (rc1 != 0) {
			DEBUG_0("Can not set bus_loc attribute\n");
			close(adapter);
			return (-1);
		    }
		}
		else {
		    rc1 = rds_switch_power("off", cab_num, bus_num, sid);
		    if (rc1 != 0) {
			DEBUG_0("Can not switch off the device\n");
			close(adapter);
			return (-1);
		    }
		    DEBUG_0("rds_power_on_device: returned from rds_switch_power off\n");

		}
	    }

	}

	if (found_flag == NOT_FOUND) {
	    rc = rds_get_device(cusobj->name, -1, -1, adapter, sid, lun);
	    if (rc == PRESENT) {
		/*
		 * the adapter pilots a device not in a cabinet with RDS
		 * facility 
		 */
		rc1 = rds_setattr(cusobj->parent, "bus_loc", "X-0");
		if (rc1 != 0) {
		    DEBUG_0("Can not set bus_loc attribute\n");
		    close(adapter);
		    return (-1);
		}
	    }
	}
    }

    else
	if (strncmp(cusattpar.value, "X-0", 3)) {
	    cab_num = cusattpar.value[0] & 0x0F;
	    DEBUG_1("rds_power_on_device: cabinet number %d\n", cab_num);
	    if (cusattpar.value[2] == 'A') {
		DEBUG_0("rds_power_on_device: bus number=0\n");
		bus_num = 0;
	    }
	    else {
		DEBUG_0("rds_power_on_device: bus number=1\n");
		bus_num = 1;
	    }

	    rc = rds_find_device_present(cab_num, bus_num, sid);
	    DEBUG_0("rds_power_on_device: returned from rds_find_device_present\n");
	    if (rc == FOUND) {
		rc1 = rds_switch_power("on", cab_num, bus_num, sid);
		if (rc1 != 0) {
		    DEBUG_0("Can not switch on the device\n");
		    close(adapter);
		    return (-1);
		}
		sleep(2);
		DEBUG_0("rds_power_on_device: returned from rds_switch_power on\n");
		rc = rds_get_device(cusobj->name, cab_num, bus_num, adapter, sid, 0);
		DEBUG_0("rds_power_on_device: returned from rds_get_device\n");
		if (rc == PRESENT) {
		    sprintf(device_loc, "%1d-%1d-%1d", cab_num, bus_num, sid);
		    DEBUG_0("rds_power_on_device: returned from rds_get_device and rc=0\n");
		}
		else {
		    rc1 = rds_switch_power("off", cab_num, bus_num, sid);
		    if (rc1 != 0) {
			DEBUG_0("Can not switch off the device\n");
			close(adapter);
			return (-1);
		    }
		    DEBUG_0("rds_power_on_device: returned from rds_switch_power off\n");
		}

	    }

	}

    DEBUG_0("End of rds_power_on_device\n");
    close(adapter);
    return (0);
}

/*
 * NAME: rds_update_location
 *
 * FUNCTION: update the location code of a device 
 *
 * NOTES: Called by query_vpd
 *
 * RETURNS: returns 0 and error code.
 *
 */
int
rds_update_location(cusobj)
struct CuDv *cusobj;
{
    struct CuAt cusattpar;
    int     rc;
    char    sstring[256];
    char    loc[16];

    sprintf(sstring, "name='%s' and attribute=bus_loc", cusobj->parent);

    rc = (int) odm_get_first(CuAt_CLASS, sstring, &cusattpar);
    if (rc == -1) {
	/* ODM failure */
	DEBUG_0("rds_update_location: ODM failure getting CuAt object\n");
	return (E_ODMGET);
    }
    if (rc == 0) {
	/* no CuAt object found */
	DEBUG_0("rds_update_location: bus_loc attribute not found\n");
	return(E_NOATTR);
    }

    strcpy(loc, cusobj->location);
    sprintf(cusobj->location, "%c%c-%c%c-%c%c-%s",
	    cusattpar.value[0], loc[1],
	    loc[3], loc[4],
	    cusattpar.value[2], loc[7],
	    cusobj->connwhere);
    DEBUG_1("location code value: %s\n", cusobj->location);

    return (0);
}
