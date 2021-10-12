static char sccsid[] = "@(#)67	1.12  src/bos/usr/lib/methods/chgcat/chgcat.c, sysxcat, bos411, 9433A411a 8/11/94 12:46:26";
/*
 * COMPONENT_NAME: (SYSXCAT) Change Method for Parallel Channel Adapter
 *
 * FUNCTIONS: check_parms()
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <stdio.h>
#include <cf.h>		/* Error codes */
 
#include <sys/cfgodm.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/access.h>
#include <sys/catuser.h>
#include <sys/limits.h>
 
#include <errno.h>
 
#include "cfgdebug.h"
#include "pparms.h"
 
extern int Pflag;
 
static int on_off_mode(int cmd, char *devname, struct attr *attrs);
 
#define MAX_BUFFER_MEM 208*1024
 
/*
 * NAME:
 *	check_parms
 *
 * FUNCTION:
 *	This function does PCA specific checks on the attributes.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */
int 
check_parms(
	struct attr *attrs,
	int pflag,
	int tflag,
	char *devname,
	char *parent,
	char *loc,
	char *badattr)
{
	struct attr *att_changed();	/* function to get address from 
					   parameter list */
	struct attr *attrp;
	int rc;				/* return code from odm functions */
	int xmitsz = 0;		/* PCA transmit buffer size */
	int recvsz = 0;		/* PCA receive buffer size */
	int xmitno = 0;		/* Number of PCA transmit bufs */
	int recvno = 0;		/* Number of PCA receive bufs */
	int first_sc = 0;	/* Starting valid subchannel */
	int num_sc = 0;		/* Number of subchannels avail. starting with first_sc*/
	char temp[6];		/* Used to validate clawset */
	int i;				/* Used to validate clawset */
	char *ptrb;			/* Used to validate clawset */
	long value;			/* Used to validate clawset */
	struct CuAt *CuAt_ptr;	/* Customized Attribute structure pointer */
	int num_attrs;		/* Return value holder from getattr(); */
 
	DEBUG_0("check_parms(): BEGIN check_parms()\n")
	/*
	 * If the attribute list is "check_ind indicator", 
	 * then get the current online/offline state of 
	 * the adapter with respect to the 370 host.  If
	 * any subchannels have been started, the adapter
	 * if ONLINE, else it is OFFLINE.
	 */
	if (strcmp(attrs->attribute, "check_ind") == 0) {
		rc = on_off_mode(CAT_CHECK_ON_OFF, devname, attrs);
		return rc;
	}
 
	/*
	 * If the attribute list is "switch", the set the
	 * online offline state of the adapter according to
	 * the wishes of the CE or super user by looking at
	 * the 'switch' attribute.
	 */
	if (attrp = att_changed(attrs,"switch")) {
		if (strcmp(attrp->value, "online") == 0) {
			rc = on_off_mode(CAT_SET_ONLINE, devname, attrs);
		} else if (strcmp(attrp->value, "offline") == 0) {
			rc = on_off_mode(CAT_SET_OFFLINE, devname, attrs);
		} else {
			strcpy (badattr, "switch");
			rc = E_INVATTR;
		}
		return rc;
	}
 
	/*
	 * Get the value of the 'first_sc' and 'num_sc' attributes
	 * so the clawset values can be validated.
	 */
	if ((attrp=att_changed(attrs,"first_sc"))) {
		first_sc = strtoul(attrp->value, (char **)NULL, 0);
		if (first_sc < 0 || first_sc > 255) {
			strcpy (badattr, "first_sc");
			return E_INVATTR;
		}
	} else {
		CuAt_ptr = (struct CuAt*)getattr(devname, "first_sc", 0, &num_attrs);
		if (CuAt_ptr == NULL) {
			strcpy(badattr, "first_sc");
			return E_INVATTR;
		}
		first_sc = strtoul(CuAt_ptr->value, (char **)NULL, 0);
	}
	
	if ((attrp = att_changed(attrs,"num_sc"))) {
		num_sc = strtoul(attrp->value, (char **)NULL, 0);
		if (num_sc < 0 || num_sc > 256) {
			strcpy (badattr, "num_sc");
			return E_INVATTR;
		}
	} else {
    		CuAt_ptr = (struct CuAt*)getattr(devname, "num_sc", 0, &num_attrs);
    		if (CuAt_ptr == NULL) {
			strcpy(badattr, "num_sc");
			return E_INVATTR;
		}
    		num_sc = strtoul(CuAt_ptr->value, (char **)NULL, 0);
	}
 
	if (first_sc + num_sc > 256) {
		strcpy (badattr, "first_sc num_sc");
		return E_INVATTR;
	}
 
	if (attrp = att_changed(attrs, "clawset")) {
		ptrb = attrp->value;
	} else {
    		CuAt_ptr = (struct CuAt*)getattr(devname, "clawset", 0, &num_attrs);
    		if (CuAt_ptr == NULL) {
			strcpy(badattr, "clawset");
			return E_INVATTR;
		}
		ptrb = CuAt_ptr->value;
	}

	if (strcmp(ptrb, "none")) {
		/* 
		 * First search the input value until a comma is reached, 
		 * put the string in temp[], then use strtol() to convert the 
		 * string to long integer, do it until the end of the input 
		 * value
		 */
		i = 0;
		while (i || *ptrb) {
            /*
            ** Insert a "0x" prefix if there isn't one, so
            ** strtol() knows el numero es hexidecimal.
            */
            if (i == 0) {
                temp[0] = '0';
                temp[1] = 'x';
                if (ptrb[0] == '0' && ptrb[1] == 'x') {
                    ptrb += 2;
                }
                i += 2;
            }
			if (isxdigit(*ptrb)) {
				temp[i++] = *ptrb;
			} else if (*ptrb == ',' || *ptrb == 0) {
				if (i) {
					temp[i] = 0;
					value = strtol(temp, (char **)NULL, 0);
					if (value < first_sc || value >= (first_sc+num_sc)) {
						strcpy(badattr, "clawset");
						return E_INVATTR;
					}
					/*
					 * if odd, out-of-range or dup,
					 * return error
					 */
					if ((value && (value % 2)) ||
						value > (CAT_MAX_SC - 2)) {
						strcpy(badattr, "clawset");
						return E_INVATTR;
					}
					i = 0;
				}
				if (*ptrb == 0) {
					break;
				}
			} else {
				strcpy(badattr, "clawset");
				return E_INVATTR;
			}
			ptrb++;
		}
	}
	
	/*
	 * Validate adapter receive and transmit buffer attributes.
	 */
	if ((attrp=att_changed(attrs,"xmitsz"))) {
		if (convert_att( &xmitsz,'i',attrp->value,'n') > 0) {
			strcpy (badattr, "xmitsz");
			return E_INVATTR;
		}
	} else {    /* d50723 */
		if((CuAt_ptr = (struct CuAt *) getattr(devname,"xmitsz",0,&num_attrs)) == NULL){
			strcpy(badattr,"xmitsz");
			return E_INVATTR;
		}
		xmitsz = strtoul(CuAt_ptr->value, (char **)NULL, 0);
	}
 
	if ((attrp=att_changed(attrs,"recvsz"))) {
		if (convert_att( &recvsz,'i',attrp->value,'n')  != 0){
			strcpy (badattr, "recvsz");
			return E_INVATTR;
		}
	} else { /* d50723 */
		if((CuAt_ptr = (struct CuAt *) getattr(devname,"recvsz",0,&num_attrs)) == NULL){
			strcpy(badattr,"recvsz");
			return E_INVATTR;
		}
		recvsz = strtoul(CuAt_ptr->value, (char **)NULL, 0);
	}
 
	if ((attrp=att_changed(attrs,"recvno"))) {
		if (convert_att( &recvno,'i',attrp->value,'n')  != 0){
			strcpy (badattr, "recvno");
			return E_INVATTR;
		}
	} else { /* d50723 */
		if((CuAt_ptr = (struct CuAt *) getattr(devname,"recvno",0,&num_attrs)) == NULL){
			strcpy(badattr,"recvno");
			return E_INVATTR;
		}
		recvno = strtoul(CuAt_ptr->value, (char **)NULL, 0);
	}
	
	if ((attrp=att_changed(attrs,"xmitno"))) {
		if (convert_att( &xmitno,'i',attrp->value,'n')  != 0){
			DEBUG_1("xmitno %d\n",xmitno);
			strcpy (badattr, "xmitno");
			return E_INVATTR;
		}
	} else { /* d50723 */
		if((CuAt_ptr = (struct CuAt *) getattr(devname,"xmitno",0,&num_attrs)) == NULL){
			strcpy(badattr,"xmitno");
			return E_INVATTR;
		}
		xmitno = strtoul(CuAt_ptr->value, (char **)NULL, 0);
	}
 
	if ((xmitno*xmitsz)+(recvno*recvsz) > MAX_BUFFER_MEM)  {
		DEBUG_4(" xmitno %d xmitsz %d recvno %d recvsz %d\n",
			xmitno, xmitsz, recvno, recvsz );
		strcpy (badattr, "xmitsz recvno recvsz xmitno");
		return E_INVATTR;
	}
 
	DEBUG_0("returning from check_parms\n");
	return 0;
} /* check_parms() */
 
 
/*
 * NAME:	on_off_mode
 *
 * FUNCTION:	Changes the ONLINE/OFFLINE status of the adapter
 *		and sets the 'indicator' attribute appropriately.
 *
 * RETURNS: Returns  0 on success, ~0 Error code.
 */
static int on_off_mode(
	int cmd,
	char *devname,
	struct attr *attrs)
{
	int fd;				/* Generic file descriptor */
	char spec_file[PATH_MAX];	/* Special file path */
	struct cat_on_offline on_off;	/* On/Offline arg to ioctl */
	int rc;
 
	sprintf(spec_file, "/dev/%s/C", devname);
	if ((fd = open(spec_file, O_RDWR)) < 0) {
		DEBUG_2("open(%s) failed, errno=%d\n",
			 spec_file, errno);
		return errno;
	}
	
	on_off.cmd = cmd;
	on_off.state = -1;	/* "undefined" */
 
	if (rc = ioctl(fd, CAT_ON_OFFLINE, &on_off)) {
		DEBUG_2("ioctl(%s, CAT_OFFLINE)\
			failed, errno=%d\n", spec_file, errno);
		return rc;
	}
 
	if (cmd == CAT_CHECK_ON_OFF) {
		if (strcmp((++attrs)->attribute, "indicator"))
			return EINVAL;
		switch (on_off.state) {
			case CAT_ONLINE:
				strcpy(attrs->value, "online");
				break;
			case CAT_OFFLINE:
				strcpy(attrs->value, "offline");
				break;
			default:
				return EINVAL;
				break;
		}
	}
 
	Pflag = 1;
 
	return 0;
} /* on_off_mode() */
