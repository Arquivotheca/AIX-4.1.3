static char sccsid[] = "@(#)50	1.7.1.1  src/bos/usr/lib/methods/chgent/chgent.c, cfgmethods, bos411, 9428A410j 9/24/93 10:50:33";
/*
 * COMPONENT_NAME: (CFGMETHODS) Change Method for ethernet adpaters
 *
 * FUNCTIONS:   check_parms
 *              handle_nvram
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
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

#ifdef CFGDEBUG
#include <errno.h>
#endif

#include "cfgdebug.h"
#include "pparms.h"

static  struct    attr_list *alist=NULL;   /* PdAt attribute list      */
int     how_many;               /* Used by getattr routine.            */
int     byte_cnt;               /* Byte count of attributes retrieved  */

#define GETATT(A,B,C)           getatt(alist,C,A,B,&byte_cnt)

/*
 * NAME:
 *	check_parms
 *
 * FUNCTION:
 *	This function does ethernet specific checks on the attributes.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int 
check_parms(attrs,pflag,tflag,devname,parent,loc,badattr)
struct attr *attrs;
int	pflag, tflag;
char	*devname,*parent,*loc,*badattr;
{
	struct	attr	*att_changed(),	/* function to get address from 
					   parameter list */
			*alt_attr;	/* attribute structure in list */
	struct Class	*cudv;		/* customized device handle */
	struct CuDv	cusobj;		/* customized device object */
	char    attr_str[8];            /* temp string to read attrs */
	char	sstring[50];		/* search string */
	int	rc;			/* return code from odm functions */
	int	alt_addr_ok = 0;	/* validated address flag */


	DEBUG_0("check_parms(): BEGIN check_parms()\n")

	/*
	 * Get customized object for later use.
	 */

	/* Open customized devices object class */
	if ((int)(cudv=odm_open_class(CuDv_CLASS))==-1)
		return E_ODMOPEN;

	/* Get the customized object */
	sprintf (sstring,"name = '%s'",devname);
	if ((rc=(int)odm_get_obj(cudv,sstring,&cusobj,
				ODM_FIRST)) == 0)
		/* get object failed */
		return E_NOCuDv;
	else if (rc == -1)
		/* No object returned */
		return E_ODMGET;

	if (odm_close_class(cudv) <0)
		return E_ODMCLOSE;

	/* get list of attributes just in case */
	if ((alist=get_attr_list(devname,cusobj.PdDvLn_Lvalue,&how_many,16)) == NULL)
		return(E_ODMGET);

	/*
	 * Validate attributes
	 */
	
	/* Validate alternate address if passed in list */
	if(alt_attr=att_changed(attrs,"alt_addr")) {
		DEBUG_4("check_parms(): %s=%s,  %s=%s\n",
			"found alt_addr.attribute",alt_attr->attribute,
			"alt_attr.value",alt_attr->value)
		convert_att(attr_str,'b',alt_attr->value,'s',&byte_cnt);
		if(byte_cnt != 6) {
			strcpy( badattr, "alt_addr" );
			return E_INVATTR;
		}
		DEBUG_1("check_parms(): high byte of address=%c\n",attr_str[0])
		if(attr_str[0] & 0x01) {
			strcpy( badattr, "alt_addr" );
			return E_INVATTR;
		}
		DEBUG_0("check_parms(): address verification successful\n")
		alt_addr_ok = 1;
	}

	/* Check use alternate address flag */
	if (alt_attr = att_changed(attrs,"use_alt_addr")) {
		DEBUG_1 ("check_parms(): use_alt_addr = %s\n",alt_attr->value)
		if (alt_attr->value[0] == 'y') {
			/* Use alternate address */
			DEBUG_0 ("check_parms(): Using alternate address\n")
			if (!alt_addr_ok) {
				DEBUG_0 ("check_parms(): Alternate address not in list\n")
				/* Alternate address not in parameters */

				/* Get alternate address attribute */
				rc=GETATT(attr_str,'b',"alt_addr");
				if (rc == 0) {
					if(byte_cnt != 6) {
						DEBUG_1 ("check_parms(): bytes in alt_addr = %d\n",byte_cnt)
						strcpy( badattr, "alt_addr" );
						return E_INVATTR;
					}
				}
				else {
					DEBUG_1 ("check_parms(): getatt of alt_addr = %d\n",rc)
					strcpy( badattr, "alt_addr" );
					return rc;
				}
				DEBUG_1 ("check_parms(): alt_addr in db = %s\n",attr_str)
				/* Check high order byte */
				if(attr_str[0] & 0x01) {
					strcpy( badattr, "alt_addr" );
					return E_INVATTR;
				}

				DEBUG_0 ("check_parms(): Alternate address OK\n")
			}
		}
	} DEBUGELSE
		DEBUG_0("check_parms(): no attributes need special checks\n")

	/* Handle NVRAM if 3-com device and bnc_select value was changed */
	/* if uniquetype is for 3-COM adapter */
	DEBUG_1("check_parms(): checking unqiuetype of %s\n",cusobj.PdDvLn_Lvalue)
	if (!strncmp(cusobj.PdDvLn_Lvalue, "adapter/mca/ethernet", 20)) {

	    if(alt_attr=att_changed(attrs,"bnc_select")) {
		DEBUG_4("check_parms(): %s=%s,  %s=%s\n",
			"found alt_attr.attribute",alt_attr->attribute,
			"alt_attr.value",alt_attr->value)
		rc = handle_nvram(&cusobj, ((alt_attr->value[0] == 'd') ? 0 : 1));
		if (rc != 0 ) {
		    return (rc);
		}

	    }
	}

	return 0;
}


/*
 *  Name:  handle_nvram()
 *
 *  Function:
 *    Performs any 3-COM Ethernet attribute checking.
 *    This subroutine checks to see if NVRAM has been initialized
 *    for ethernet and writes the changed value for bnc_select
 *    to the wire type NVRAM space.
 *
 *    Called by change method to update NVRAM due to user modifying
 *    odm attribute value.
 *
 *  Returns:
 *    Returns a 0 upon success and one of the following error
 *    codes:  E_OPEN or E_DEVACCESS
 */
int
handle_nvram(cust_obj,bnc_select)
struct  CuDv    *cust_obj;
int     bnc_select;
{

	char    sstring[256];   /* temp string */
	int     slot;           /* device slot on bus */
	struct  CuDv    bus_obj;
	int     bus_num;        /* Bus number                       */
	int     rc;             /* temp return code */

    /*
     *  Check to see if the Ethernet NVRAM area is initialized.
     *  If it is initialized then update the wire type.
     */
    if (!check_magic_num()) {
	    /*
	     *  Need to get the slot and bus_num
	     *  from the database.
	     */
	    strcpy(sstring,cust_obj->connwhere);
	    DEBUG_1("handle_nvram(): connwhere=%s\n",sstring)
	    slot=atoi(&sstring[strcspn(sstring,"0123456789")]);
	    slot--;
	    DEBUG_1("handle_nvram(): slot=%d\n",slot)

	    rc = Get_Parent_Bus(CuDv_CLASS,cust_obj->parent,&bus_obj);
	    if (rc) {
		if (rc == E_PARENT) rc = E_NOCuDvPARENT;
		return (rc);
	    }
	    bus_num = bus_obj.location[3] - '0';

	    /*
	     *  Put the wire type value into NVRAM.
	     */
	    DEBUG_1("handle_nvram(): bnc_select=%d\n",bnc_select)
	    put_wire_type(bus_num,slot,bnc_select);
    }
    return(0);
}


