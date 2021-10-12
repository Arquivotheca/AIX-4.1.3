static char sccsid[] = "@(#)52  1.14  src/bos/usr/lib/methods/chgtok/chgtok.c, sysxtok, bos411, 9431A411a 7/15/94 16:40:42";
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: GETATT
 *		check_parms
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <cf.h>		/* Error codes */
#include <sys/cfgodm.h>
#include "cfgdebug.h"
#include "pparms.h"

/*
 *  Macro to get attributes out of database.
 */
static struct attr_list	*alist=NULL;/* PdAt attribute list                 */
int		how_many;	/* Used by getattr routine.            */
int		byte_cnt;	/* Byte count of attributes retrieved  */

#define GETATT(A,B,C)		getatt(alist,C,A,B,&byte_cnt)

/*
 * NAME:
 *	check_parms
 *
 * FUNCTION:
 *	This function does token ring specific checks on the attributes.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int 
check_parms(attrs,pflag,tflag,devname,parent,loc,badattr)
struct attr *attrs;
int	pflag, tflag;
char	*devname;
char	*parent;
char	*loc;
char	*badattr;
{
	struct	attr    *att_changed(),     /* function to get address from 
	                                       parameter list                 */
	                *alt_attr;          /* attribute structure in list    */
	struct Class    *cuat;              /* customized attribute handle    */
	struct Class    *pdat;              /* predefined attribute handle    */
	struct Class    *cudv;              /* customized device handle       */
	struct CuDv     cusobj;             /* customized device object       */
        struct CuDv     bus_obj;            /* bus customized device object   */
	char            alt_addr[7];        /* alternate address value        */
	char            sstring[50];        /* search string                  */
	int             rc;                 /* return code from odm functions */
	int             alt_addr_ok = 0;    /* validated address flag         */
	int             check_speed,        /* Inputted ring speed            */
	                ring_speed,         /* Ring speed from NVRAM          */
	                bus_num,            /* Bus number for adapter         */
	                slot;               /* Slot adapter is in             */
                        

	DEBUG_0("check_parms(): BEGIN check_parms()\n")
	
	/* Get the customized object */
	sprintf (sstring,"name = '%s'",devname);
	if ((rc=(int)odm_get_first
			(CuDv_CLASS,sstring,&cusobj)) == 0)
		/* get object failed */
		return E_NOCuDv;
	else if (rc == -1)
		/* No object returned */
		return E_ODMGET;

	/* Validate alternate address if passed in list */
	if(alt_attr=att_changed(attrs,"alt_addr")) {
		DEBUG_4("check_parms(): %s=%s,  %s=%s\n",
			"found alt_addr.attribute",alt_attr->attribute,
			"alt_attr.value",alt_attr->value)
		convert_att(alt_addr,'b',alt_attr->value,'s',&rc);
		if(rc != 6) {
			strcpy( badattr, "alt_addr" );
			return E_INVATTR;
		}
		DEBUG_1("check_parms(): high byte of address=%x\n",alt_addr[0])
		if(alt_addr[0] & 0x80) {
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
				DEBUG_0 ("check_parms(): Alternate address\
					 not in list\n")
				/* Alternate address not in parameters */

				/*
	 		 	 *  Get attribute list
	 			 */
				if ((alist=get_attr_list(devname,
						cusobj.PdDvLn_Lvalue,
						&how_many,16)) == NULL)
					return(E_ODMGET);

				/* Get alternate address attribute */
				rc=GETATT(alt_addr,'b',"alt_addr");
				if (byte_cnt != 6) {
					if (rc > 0)
						return rc;
					else {
						DEBUG_1 ("check_parms(): bytes\
						 	in alt_addr = %d\n",rc)
						strcpy( badattr, "alt_addr" );
						return E_INVATTR;
					}
				}

				DEBUG_1("check_parms(): high byte of addr=%x\n",
					alt_addr[0])
				/* Check high order byte */
				if(alt_addr[0] & 0x80)
				{
					strcpy( badattr, "alt_addr" );
					return E_INVATTR;
				}

				DEBUG_0("check_parms(): Alternate address OK\n")

			}
		}
	}

	/*
	 *  Check ring speed.
	 */
	if (alt_attr = att_changed(attrs,"ring_speed")) {
		DEBUG_0 ("check_parms(): Checking ring speed attribute\n")
		/*
		 *  Convert the inputted ring speed to the TR ring speed.
		 *     4 = 0  and  16 = 1 and autosense = 2
		 */
		DEBUG_1 ("check_parms(): alt_attr->value is %s\n",
				alt_attr->value)
                if (strcmp(alt_attr->value,"4") == 0)
                        check_speed = 0;
                else
                        if (strcmp(alt_attr->value,"16") == 0)
                                check_speed = 1;
                        else {
                                if (strcmp(alt_attr->value,"autosense") == 0)
                                        check_speed = 2;
                                else {
                                        strcpy(badattr,"ring_speed");
                                        return(E_INVATTR);
                                }
                        }

		/*
		 *  Check to see if the TR NVRAM area is initialized.  If it
		 *  is initialized then update the ring speed.
		 */
		if (!check_magic_num()) {
			/*
			 *  Need to get the slot and bus_num 
                         *  from the database. 
			 */
			strcpy(sstring,cusobj.connwhere);
			DEBUG_1("check_parms(): connwhere=%s\n",sstring)
			slot=atoi(&sstring[strcspn(sstring,"0123456789")]);
			slot--;
			DEBUG_1("check_parms(): slot=%d\n",slot)

                        rc = Get_Parent_Bus(CuDv_CLASS,cusobj.parent,&bus_obj);
                        if (rc) {
                            if (rc == E_PARENT) rc = E_NOCuDvPARENT;
                            return (rc);
                        }
                        bus_num = bus_obj.location[3] - '0';
      
			/*
			 *  Put the ring speed value into NVRAM.
			 */
			DEBUG_1("check_parms(): check_speed=%d\n",check_speed)
			put_ring_speed(bus_num,slot,check_speed);
		}
	} DEBUGELSE
		DEBUG_0("check_parms(): no attributes need special checks\n")

	return 0;
}
