static char sccsid[] = "@(#)06  1.1  src/bos/usr/lib/methods/common/rds_common.c, cfgmethods, bos412, 9446B 10/28/94 10:49:35";
/*
 * COMPONENT_NAME: (CFGMETHODS) Configuration Functions
 *
 * FUNCTIONS: rds_setattr,rds_add_entry,rds_match_entry,
 *            rds_swap_entry,rds_remove_element,rds_remove_entry
 *            rds_build_busloc_list,rds_find_device_present,
 *            rds_switch_power,rds_switch_off_device,
 *	      rds_get_device
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
char    device_loc[6] = "";

struct entries_list *head, *queue;

/*
 * NAME: rds_setattr
 *
 * FUNCTION: To set an attribute for a device
 *
 * NOTES: This function uses getattr, putattr
 *
 * RETURNS:  error code.  0 means no error.
 */

int
rds_setattr(lname, aname, avalue)
char   *lname;	/* name of device attribute belongs to */
char   *aname;	/* attribute name */
char   *avalue;	/* attribute value */

{
    struct CuAt *cuat;	/* customized attribute object */
    int     how_many;	/* variable needed for getattr call */

    /* routines called */
    struct CuAt *getattr();
    int     putattr();

    DEBUG_3("rds_setattr(%s,%s,%s)\n", lname, aname, avalue);

    /* get current attribute setting */
    cuat = getattr(lname, aname, 0, &how_many);
    if (cuat == (struct CuAt *) NULL) {
	DEBUG_0("ERROR: getattr() failed\n");
	return (E_NOATTR);
    }

    DEBUG_1("Value received by getattr: %s\n", cuat->value);

    /* Only update CuAt object if attr value really changed */
    if (strcmp(cuat->value, avalue)) {
	/* copy new attribute value into cuat structure */
	strcpy(cuat->value, avalue);

	if (putattr(cuat) == -1) {
	    DEBUG_0("Error: putattr() failed\n");
	    return (E_ODMUPDATE);
	}
    }

    return (0);
}

/*
 * NAME: rds_add_entry
 *
 * FUNCTION: add a new value in the list of the possible 
 *           bus_loc attribute value
 *
 * NOTES: None
 *
 * RETURNS: a pointer on the list of the bus_loc attribute value
 */
struct entries_list *
rds_add_entry(ptr, value, count)
struct entries_list *ptr;
char   *value;
int     count;
{
    char   *val;

    if (ptr == NULL) {
	ptr = (struct entries_list *) malloc(sizeof(struct entries_list));
	val = (char *) malloc(4);
	if ((ptr == NULL) || (val == NULL))
	    err_exit(E_MALLOC);
	strcpy(val, value);
	ptr->value = val;
	ptr->cnt = count;
	ptr->next = NULL;
	queue = ptr;
    }
    else
	ptr->next = rds_add_entry(ptr->next, value, count);

    return ptr;
}

/*
 * NAME: rds_match_entry
 *
 * FUNCTION: look for if a bus_loc attribute value matches a value in the
 *           list of the possible bus_loc attribute value
 *
 * NOTES: None
 *
 * RETURNS: a pointer on the list of the 
 *          bus_loc attribute value, if a match found
 *          else NULL.
 */
struct entries_list *
rds_match_entry(ptr_match, attr_value)
struct entries_list *ptr_match;
char   *attr_value;
{
    struct entries_list *ptr;
    register int diff;

    for (ptr = ptr_match; ptr != NULL && ((diff = strncmp(ptr->value, attr_value, 3)) != 0);
	 ptr = ptr->next);
    if (ptr != NULL && diff == 0)
	return ptr;
    else
	return NULL;
}

/*
 * NAME: rds_swap_entry
 *
 * FUNCTION: swap list entries so that bus_loc value is at top of list
 *
 * NOTES: None
 *
 * RETURNS: a pointer on the list of the 
 *          bus_loc attribute value, if a match found
 *
 */
struct entries_list *
rds_swap_entry(new_busloc_list, rc_list)
struct entries_list *new_busloc_list;
struct entries_list *rc_list;
{
    struct entries_list *p;

    for (p = rc_list; p != NULL; p = p->next)
	new_busloc_list = rds_add_entry(new_busloc_list, p->value, p->cnt);
    for (p = head; p->cnt != rc_list->cnt; p = p->next)
	new_busloc_list = rds_add_entry(new_busloc_list, p->value, p->cnt);

    return new_busloc_list;
}

/*
 * NAME: rds_remove_element
 *
 * FUNCTION: remove an element in the list of the bus_loc attribute values
 *
 * NOTES: None
 *
 * RETURNS: a pointer on the list of the 
 *          bus_loc attribute value, with the element removed
 *
 */
struct entries_list *
rds_remove_element(new_busloc_list, rc_list)
struct entries_list *new_busloc_list;
struct entries_list *rc_list;
{
    struct entries_list *p;

    for (p = rc_list->next; p != NULL; p = p->next)
	new_busloc_list = rds_add_entry(new_busloc_list, p->value, p->cnt);
    for (p = head; p->cnt != rc_list->cnt; p = p->next)
	new_busloc_list = rds_add_entry(new_busloc_list, p->value, p->cnt);

    return new_busloc_list;
}

/*
 * NAME: rds_remove_entry
 *
 * FUNCTION: remove an element in the list of the bus_loc attribute values 
 *
 * NOTES: this fonction uses rds_remove_element, rds_match_entry,
 *                           getattr, odm_get_first
 *
 * RETURNS: nothing
 *
 */
void
rds_remove_entry(busloc_list, lname)
struct entries_list **busloc_list;
char   *lname;
{
    struct entries_list *new_busloc_list, *rc_list, *p;
    struct CuAt cuat_adapter;
    struct CuDv *cudv_adapter;
    struct listinfo info_cudv_adapter;
    int     i, rc;

    char    ss[256];

    cudv_adapter = odm_get_list(CuDv_CLASS, "name like '*scsi*' and status=1", &info_cudv_adapter, 64, 1);
    if ((int) cudv_adapter == -1) {
	DEBUG_0("Error reading CuDv to look for the scsi adapter\n");
	err_exit(E_ODMGET);
    }

    if (info_cudv_adapter.num != 0) {
	for (i = 0; i < info_cudv_adapter.num; i++) {
	    if (!strcmp(cudv_adapter[i].name, lname))
		continue;
	    sprintf(ss, "name='%s' and attribute=bus_loc", cudv_adapter[i].name);
	    rc = (int) odm_get_first(CuAt_CLASS, ss, &cuat_adapter);
	    if (rc == -1) {
		DEBUG_0("Error reading CuAt to look for bus_loc attribute\n");
		err_exit(E_ODMGET);
	    }
	    if ((rc == 0) || (!strcmp(cuat_adapter.value, "X-0")))
		continue;
	    else {
		rc_list = rds_match_entry(*busloc_list, cuat_adapter.value);
		if (((int) rc_list) != 0) {
		    DEBUG_0("Match found \n");
		    new_busloc_list = rds_remove_element(NULL, rc_list);
		    head = new_busloc_list;
		    *busloc_list = new_busloc_list;
		}
		else {
		    DEBUG_0("No match found \n")
		}
	    }
	}
    }

}

/*
 * NAME: rds_build_busloc_list
 *
 * FUNCTION: build the list of the possible values of the 
 *           bus_loc attribute value.
 *
 * NOTES: None
 *
 * RETURNS: a pointer on the list of the bus_loc attribute value
 *
 */
struct entries_list *
rds_build_busloc_list(build_l)
struct entries_list *build_l;
{
    struct CuDv *cudv_cabinet;
    struct listinfo info_cudv_cabinet;

    int     num_cab;
    int     i, j, cnt;

    char    busloc_value[4];

    cudv_cabinet = (struct CuDv *) odm_get_list(CuDv_CLASS, "name like 'cabinet*' AND status=1", &info_cudv_cabinet, NB_MAX_CABINETS, 1);
    if ((int) cudv_cabinet == -1) {
	DEBUG_0("Error reading CuDv to look for the cabinet devices\n");
	err_exit(E_ODMGET);
    }

    num_cab = info_cudv_cabinet.num;

    DEBUG_1("Number of cabinets: %d\n", num_cab);
    cnt = 1;

    for (i = 0; i < num_cab; i++) {
	sprintf(busloc_value, "%1d-A", i);
	build_l = rds_add_entry(build_l, busloc_value, cnt);
	cnt = cnt + 1;
	sprintf(busloc_value, "%1d-B", i);
	build_l = rds_add_entry(build_l, busloc_value, cnt);
	cnt = cnt + 1;
    }

    return (build_l);
}

/*
 * NAME: rds_find_device_present
 *
 * FUNCTION: detects if a device is plugged on a slot
 *
 * NOTES: None
 *
 * RETURNS: returns FOUND if a device is in this slot
 *          returns NOT_FOUND if a device is not in this slot
 *
 */
int
rds_find_device_present(cab_num, bus_num, sid_num)
int     cab_num;
int     bus_num;
int     sid_num;
{
    int     status;
    int     disk;
    int     nvram, rc;
    MACH_DD_IO param;

    /* open nvram to enable ioctls usage */
    nvram = open("/dev/nvram", O_RDWR);
    if (nvram == -1) {
	DEBUG_0("Unable to open /dev/nvram\n");
	return (E_DEVACCESS);
    }

    disk = sid_num | (bus_num << 4);

    param.md_size = cab_num;
    param.md_incr = MV_WORD;

    param.md_cbnum = cab_num;
    param.md_dknum = disk;
    param.md_data = (char *) &status;

    /* get array of the disk location */
    rc = ioctl(nvram, MRDSGET, &param);
    if (rc != 0) {
	DEBUG_0("Error reading with RDS\n");
	close(nvram);
	return (E_DEVACCESS);
    }
    else {
	if (status & 0x4) {
	    DEBUG_3("Disk on bus %d with sid %d in cabinet %d is present\n",
		    bus_num, sid_num, cab_num);
	    if (status & 0x1) {
		DEBUG_3("Disk on bus %d with sid %d in cabinet %d is power on\n",
			bus_num, sid_num, cab_num);
		rc = NOT_FOUND;
	    }
	    else
		rc = FOUND;
	}
	else
	    rc = NOT_FOUND;
    }

    close(nvram);

    return (rc);
}

/*
 * NAME: rds_switch_power
 *
 * FUNCTION: switch on/off the power on a device
 *
 * NOTES: None
 *
 * RETURNS: returns 0 if the operation succeed
 *          returns E_DEVACCESS
 *
 */
int
rds_switch_power(power, cab_num, bus_num, sid)
char   *power;
int     cab_num;
int     bus_num;
int     sid;
{
    int     disk;
    int     status;
    int     rc;
    int     nvram;
    MACH_DD_IO param;

    /* open nvram to enable ioctls usage */
    nvram = open("/dev/nvram", O_RDWR);
    if (nvram == -1) {
	DEBUG_0("rds_switch_power: unable to open /dev/nvram\n");
	return (E_DEVACCESS);
    }

    if (!strcmp(power, "on"))
	status = MRDS_ON;
    if (!strcmp(power, "off"))
	status = MRDS_OFF;
    disk = sid | (bus_num << 4);

    param.md_size = cab_num;
    param.md_incr = MV_WORD;

    param.md_cbnum = cab_num;
    param.md_dknum = disk;
    param.md_data = (char *) &status;

    /* switch disk on or off */
    rc = ioctl(nvram, MRDSSET, &param);
    if (rc == -1) {
	DEBUG_0("rds_switch_power: error reading with RDS\n");
	close(nvram);
	return (E_DEVACCESS);
    }

    close(nvram);

    return (0);
}

/*
 * NAME: rds_switch_off_device
 *
 * FUNCTION: switch off a device if something was wrong
 *           during the configuration method of this device
 *
 * NOTES: None
 *
 * RETURNS: returns 0.
 *
 */
int
rds_switch_off_device()
{
    int     cab_num, bus_num, sid;
    int     disk;
    int     status;
    int     rc;
    int     nvram;
    MACH_DD_IO param;

    if (!strcmp(device_loc, ""))
	return (0);

    cab_num = device_loc[0] - '0';
    bus_num = device_loc[2] - '0';
    sid = device_loc[4] - '0';

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
 * NAME: rds_get_device
 *
 * FUNCTION: To determine if a device is present at a location, and
 *           switch on this device if it needs to be configured.
 *
 * NOTES:
 *
 * RETURNS: return PRESENT, if the device is present
 *          return NOT_PRESENT, if the device is not present
 */
int
rds_get_device(lname, cab_num, bus_num, adapter, sid, lun)
char   *lname;
int     cab_num;
int     bus_num;
int     adapter;
int     sid;
int     lun;
{
    struct sc_inquiry inq_pb;
    struct inqry_data inqstr;

    uchar  *p;
    uchar   nq;
    int     rc = NOT_PRESENT;
    int     i;
    int     nq_count = 3;

    if (strncmp(lname, "hdisk", 5))
	nq_count = 8;

    /* start this sid/lun */
    rc = ioctl(adapter, SCIOSTART, IDLUN(sid, lun));
    if (rc != 0) {
	DEBUG_2("Can not start sid=%d, lun=%d\n", sid, lun);
	return (NOT_PRESENT);
    }
    else {
	DEBUG_2("rds_get_device: started sid%d lun%d\n", sid, lun);
    }

    /* clear the inq structure area */
    p = (uchar *) & inq_pb;
    nq = sizeof(struct sc_inquiry);
    while (nq--)
	*p++ = 0;

    inq_pb.flags = 0;
    for (nq = 0; nq < nq_count; nq++) {
	inq_pb.scsi_id = sid;
	inq_pb.lun_id = lun;
	inq_pb.inquiry_len = SCSI_INQSIZE - 1;
	inq_pb.inquiry_ptr = (uchar *) & inqstr;

	rc = ioctl(adapter, SCIOINQU, &inq_pb);
	if (rc == 0) {
	    DEBUG_0("rds_get_device: device is found\n");
	    rc = PRESENT;
	    break;
	}
	else {
	    if (errno == ENOCONNECT) {
		DEBUG_2("rds_get_device: ENOCONNECT on %d %d.\n", sid, lun);
		rc = NOT_PRESENT;
	    }
	    else {
		if (errno == ENODEV) {
		    DEBUG_2("rds_get_device: no device responds on id%d lun%d\n"
			    ,sid, lun);
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
		    DEBUG_3("rds_get_device: err=%d on %d %d\n", errno, sid, lun
			);
		    if (nq > 0) {
			rc = NOT_PRESENT;
			break;
		    }
		}
	    }
	}
    }

    if ((inqstr.pdevtype == SCSI_DISK) && (strncmp(lname, "hdisk", 5) != 0))
	rc = NOT_PRESENT;

    else
	if ((inqstr.pdevtype == SCSI_TAPE) && (strncmp(lname, "rmt", 3) != 0))
	    rc = NOT_PRESENT;

	else
	    if ((inqstr.pdevtype == SCSICDROM) && (strncmp(lname, "cd", 2) != 0))
		rc = NOT_PRESENT;

	    else
		if ((inqstr.pdevtype == SCSI_RWOPT) && (strncmp(lname, "omd", 2) != 0))
		    rc = NOT_PRESENT;

    /* do SCIOSTOP for sid/lun */
    if (ioctl(adapter, SCIOSTOP, IDLUN(sid, lun)) < 0) {
	DEBUG_2("rds_get_device: error stopping sid%d lun%d\n", sid, lun);
	rc = NOT_PRESENT;
    }

    return (rc);
}
