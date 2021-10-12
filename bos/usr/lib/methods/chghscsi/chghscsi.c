static char sccsid[] = "@(#)45	1.13.1.2  src/bos/usr/lib/methods/chghscsi/chghscsi.c, cfgmethods, bos411, 9428A410j 5/13/93 17:21:40";
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: check_parms
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

/* header files needed for compilation */

#include <stdio.h>
#include <cf.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include "pparms.h"
#include "cfghscsi.h"
#include "cfgdebug.h"

/*
 * NAME: check_parms
 *
 * FUNCTION:
 *	It checks the device specific attributes that are dependent
 *	on other attrbutes. SCSI adapter does not have any specific
 *	attrbutes to be checked. It also tells generic change method
 *	whether changing Database alone/Device alone is allowed by
 *	this device. Scsi adapter allows changes to Database alone.
 *	It also updates the NVRAM scsi_id location for the adapter
 * 	if card_scsi_id attribute is changed in the database alone.
 * 
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine called
 *	by the generic change method for all devices.
 *
 * DATA STRUCTURES :
 *
 * INPUT : pointer to array of attribute structures,pflag,tflag,lname.
 *
 * RETURNS : 0 if success
 *	    -1 error condition
 *	    -2 pflag is set and the device does not allow it
 *	    -3 tflag is set and the device does not allow it
 *
 * RECOVERY OPERATION :
 *
 */
 
int check_parms(attrs,pflag,tflag,lname,parent,loc,badattr)
struct attr *attrs;
int	pflag,tflag;
char	*lname,*parent,*loc;
char	*badattr;
{
uchar	id,slot;
struct	attr *ap;
char	sstr[512], mcodefile[32], filename[32],*part_no;
int	rc, lvl, mcode_flag, bus_num;
struct	CuDv	CuDv;
struct	CuVPD	CuVPD;		/* Structure for VPD data */
struct	Class	*cusdev;

	/* First get the CuDv object for this device */
	sprintf(sstr,"name = '%s'",lname);
	rc = (int)odm_get_first(CuDv_CLASS,sstr,&CuDv);
	if (rc == 0) {
	    DEBUG_1("chk_parms: CuDv object not found for %s\n",sstr)
	    return(E_NOCuDv);
	} else if (rc == -1) {
	    DEBUG_1("chk_parms: Error getting object for %s\n",sstr)
	    return(E_ODMGET);
	}

	if((pflag == CHGDB_ONLY) && ((ap = att_changed(attrs,"id")) != NULL)){
	    id = atoi(ap->value);
	    if((id < 0) || (id >= MAX_ID)){
		DEBUG_1("chk_parms: wrong id%d\n",id)
		strcpy( badattr, "id");
		return(E_INVATTR);
	    }
	    if (loc != NULL) slot = atoi(loc);
	    else {
		/* Get slot  from CuDv */
		slot = atoi(CuDv.connwhere);
	    }

            /* get the parent bus object in order to determine the bus */
            /* number which is necessary for put_scsi_id               */
            if ((rc = Get_Parent_Bus(CuDv_CLASS, CuDv.parent, &CuDv)) != 0) {
               DEBUG_1("chghscsi: Unable to get parent bus object rc = %d\n",
                        rc) 
               if (rc == E_PARENT) {
                  rc = E_NOCuDvPARENT;
               }
               return (rc);
            }  
            bus_num = CuDv.location[3] - '0';
	    DEBUG_2("chk_parms: put id%d into nvram for slot%d\n",id,slot)
	    if ((rc = put_scsi_id(slot,id,bus_num)) != 0){
		DEBUG_2("chk_parms: can put scsi_id %d for slot%d\n",
		    id,slot)
                return(rc);
	    }
	}

        /* if changing tm attribute, then must validate that target mode can */
        /* be supported before allowing the value to be set to "yes"         */
	if ((ap = att_changed(attrs,"tm")) != NULL) {
		if (strcmp(ap->value,"yes") == 0) {
		   /* Get the hardware VPD for this device from the database */
		   sprintf(sstr,"name = '%s' and vpd_type = '%d'",
                           lname,HW_VPD);
		   rc = (int)odm_get_first(CuVPD_CLASS,sstr,&CuVPD);
		   if (rc == 0) {
	    	   	DEBUG_1("chk_parms: No VPD found for %s\n",lname)
			strcpy( badattr, "tm");
			return(E_INVATTR);
		   } else if (rc == -1) {
	    		DEBUG_1("chk_parms: Error getting VPD for %s\n",lname)
	    		return(E_ODMGET);
                   }
		   /* CuVPD.vpd is the VPD data */
		   lvl = (int)strtoul(read_descriptor(CuVPD.vpd,"LL"),NULL,16);
                   DEBUG_1("adap level = 0x%x\n", lvl)
                   part_no = read_descriptor(CuVPD.vpd,"PN");
                   DEBUG_1("part_no = %s\n", part_no)
                   /* adapter level must be 0x44 or greater and not part no*/
                   /* 00G2362  inorder to support target mode              */
		   if ((lvl < 0x44) || (strncmp(part_no," 00G2362",8) == 0)) {
                           DEBUG_0("adap level to low to run target mode\n")
			   strcpy( badattr, "tm");
			   return(E_INVATTR);
		   }
                   /* verify that target mode microcode is present if this */
                   /* is an adapter which requires a download for target   */
                   /* mode (scsi-1)                                        */
                   if (lvl < 0x60) {
                       sprintf(mcodefile, "8d77t.%x",lvl);
                       /* set mcode_flag to search for most recent version */ 
                       mcode_flag = VERSIONING;
                       DEBUG_1("mcodefile = %s\n", mcodefile)
                       if(!(findmcode(mcodefile, filename, mcode_flag, NULL))) {
                           DEBUG_0("could not find target mode microcode\n")
                           strcpy(badattr, "tm");
                           return(E_INVATTR);
                       }
                   }
                }
	}

	return(0);
}
