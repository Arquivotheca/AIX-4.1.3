static char sccsid[] = "@(#)64        1.11  src/bos/usr/lib/methods/cfgppa/cfgppa.c, cfgmethods, bos41J, 9513A_all 3/28/95 16:15:47";
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: build_dds
 *		define_children
 *		download_microcode
 *		generate_minor
 *		make_special_files
 *		query_vpd
 *              device specific
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _KERNSYS
#define _RSPC
#include <sys/systemcfg.h>
#undef _RSPC
#undef _KERNSYS

#include <stdio.h>  
#include <cf.h>		/* Error codes */
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/ioacc.h>
#include <sys/mdio.h>
#include <sys/lpio.h>
#include <sys/ppdd.h>

#include "cfgcommon.h"
#include "cfgdebug.h"


extern int Get_Parent_Bus();    /* returns parent bus */


#define POS_REG_ADDR    0x00400000      /* bus_id offset */


/*
 * NAME: define_children
 * 
 * FUNCTION: This fucntion detects and manages children of the NIO parallel 
 *           port.
 * 
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine called by the 
 *	generic configure method for all devices. The children of these adapters
 *	are Non-detectable. This implies that all information on status of
 *	attached children will be retrieved from the customized database.
 *	This subroutine should be generic for all intermediate devices with 
 *	Non-detectable children.
 *           
 * NOTES: Children that need to be configured are detected by a Status of 
 *        "Defined" and a Previous Status of "Available". The Auto Configure
 *        flag is also checked to determine the action taken. All children to
 *        be configured are returned via stdout to the Configuration Manager
 *        which in turn will eventually invoke the configure method for these
 *        child devices.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int 
define_children(lname, phase)
char *lname;
int phase;
{
	char sstr[256];			/* search criteria string */
	struct CuDv child;		/* customized device obj storage */
	int    rc;			/* return codes go here */
	int    first;			/* get_obj field flag */
	char   autoconfig[50];		/* auto config flag */


	DEBUG_0 ("Defining children ....\n")

	/* search through the cusomized object class for defined devices on
	   the parallel port
	 */

	sprintf(sstr,"parent=%s", lname);
	first = ODM_FIRST;
	while ((rc = (int)odm_get_obj(CuDv_CLASS, sstr, &child, first)) != 0) {
		if (rc == -1) {
			DEBUG_0 ("ODM get error")
			return(E_ODMGET);
		}
	
		first = ODM_NEXT;
		
		DEBUG_1("Found child %s\n",child.name)
		
		/* if chgstatus equal to MISSING, change it to SAME */

		if (child.chgstatus == MISSING) {
			child.chgstatus = (short)SAME;
			if ((rc = odm_change_obj(CuDv_CLASS,&child)) < 0) {
				/* Error changing object Change Status */
				return(E_ODMUPDATE);
			}
		}

		autoconfig[0] = '\0';
		/***********************************************
		  If child is to be "auto configured", then ...
		 ***********************************************/
		rc = getatt(autoconfig,'s',CuAt_CLASS,PdAt_CLASS,child.name,
					child.PdDvLn_Lvalue,"autoconfig",NULL);
		if ( (rc == E_NOATTR) || (!strcmp(autoconfig,"available")) )
			/***********************************************
			  Return device name of child to Configuration 
			  manager via stdout
			***********************************************/
			fprintf (stdout,"%s\n",child.name);
		else if (rc != 0)
			return(rc);

	}      /* end of while loop */

	return(0);
}


/*
 * NAME: build_dds
 * 
 * FUNCTION: This function builds the DDS(Defined Data Structure) for the 
 *           NIO Parallel Port. 
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
 * The parallel printer device driver is initially loaded by this method.
 * When a device (printer) is configured, the cfgpp method passes in a new dds
 * with printer-specific data.
 *  
 * RETURNS: Returns 0 for SUCCESS.
 */


int build_dds(lname, ddsptr, ddslen)
char *lname;
uchar **ddsptr;
int *ddslen;
{
	char sstring[50];		/* search criteria string pointer */
	struct CuDv bus_cudv;		/* CuDv object for parent bus */
	static struct ppdds dds;	/* dds structure */
	int rc;				/* return codes go here */


	sprintf(dds.name, "%s", lname);

        /*--------------------------------------------------------------*/
	/* These dds values are not really important, since they are    */
	/* printer specific and will                                    */
	/* be reset when the printer device is configured.  However,    */
	/* they are setup to sane values here for testing purposes.     */
        /*--------------------------------------------------------------*/

	dds.v_timeout = 10;
	dds.pp_lin_default = 0;
	dds.pp_col_default = 80;
	dds.pp_ind_default = 0;
	dds.interface = PPIBM_PC;
	dds.modes = 0 ;
	dds.modes |= RPTERR;
	dds.i_flags = 0;
	dds.busy_delay = 0;

        /*--------------------------------------------------------------*/
        /* Get pp device attributes from attributes DB and store in DDS */
        /*--------------------------------------------------------------*/

	/* get interrupt level, interrupt priority, and register attributes */
	/* May or may not use interrupt */
	if ((rc = getatt(&dds.level,'i',CuAt_CLASS,PdAt_CLASS,cudv.name,
			cudv.PdDvLn_Lvalue,"bus_intr_lvl",NULL)) != 0) {
		DEBUG_0("build_dds: attr bus_intr_lvl not found\n")
	} else {
		DEBUG_1("build_dds: bus_intr_lvl = %d\n", dds.level)
	}

	if ((rc = getatt(&dds.priority,'i',CuAt_CLASS,PdAt_CLASS,cudv.name,
			cudv.PdDvLn_Lvalue,"intr_priority",NULL)) != 0) {
		DEBUG_0("build_dds: attr intr_priority not found\n")
	} else {
		DEBUG_1("build_dds: intr_priority = %d\n", dds.priority)
	}

	if ((rc = getatt(&dds.rg,'i',CuAt_CLASS,PdAt_CLASS,cudv.name,
			cudv.PdDvLn_Lvalue,"bus_io_addr",NULL)) != 0) {
		DEBUG_0("build_dds: attr bus_io_addr not found\n")
		return(rc);
	}

	/* Note: slot and posadd are only used by MCA devices */
	dds.slot = (atoi(pcudv.connwhere)-1) & 0x0f ;

	/* Get parent bus device */
	rc = Get_Parent_Bus(CuDv_CLASS, cudv.parent, &bus_cudv);
	if (rc)
		return(rc);

	DEBUG_1("slot = %ld\n",dds.slot);
	DEBUG_1("bus = %s\n", bus_cudv.name);

        /* obtain bus type and BUID */ 

	/* PCI and ISA busses do not have a bus_type attribute,
	   so do not try to obtain it */
#ifdef	BUS_TYPE
	rc = getatt(&dds.bus_type,'i',CuAt_CLASS,PdAt_CLASS,bus_cudv.name,
			 bus_cudv.PdDvLn_Lvalue,"bus_type",NULL);
	if (rc) {
		DEBUG_0("build_dds: attr bus_type not found\n")
		return(rc);
	}
	DEBUG_1("bus_type = %d\n", dds.bus_type);
#endif	

	rc = getatt(&dds.bus_id,'i',CuAt_CLASS,PdAt_CLASS,bus_cudv.name,
			bus_cudv.PdDvLn_Lvalue,"bus_id",NULL);
	if (rc) {
		DEBUG_0("build_dds: attr bus_id not found\n")
		return(rc);
	}

	dds.posadd = POSREG(POS_REG_ADDR, dds.slot);

	DEBUG_1("dds bus_id :[0x%x]\n",dds.bus_id)
	DEBUG_1("dds posadd :[0x%x]\n",dds.posadd)

	/* return a pointer pointing to DDS */
	*ddsptr = (char *) &dds;

	/* return the length of DDS */
	*ddslen=sizeof(struct ppdds);

	return(0);
}


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
 * (NOTES:) The adapter minor number is always PP_ADAP_MINOR.
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


	DEBUG_0 ("generating minor\n")
/* THIS is TEMPORARY */
max_so_far = PP_ADAP_MINOR;

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
 * NAME: make_special_file 
 * 
 * FUNCTION: This function creates special file(s) for the specific device. 
 * 
 * EXECUTION ENVIRONMENT:
 *      This device specific routine is called by the generic config method 
 *      to create special file(s) for the specific device.
 *
 * (NOTES:) THIS IS A NULL FUNCTION FOR ADAPTER CONFIGURATION METHOD.
 *	    The special files are created by the device config method.
 *
 * RETURNS: Returns 0 on SUCCESS.
 */

int 
make_special_files(lname, devno)
char *lname;
dev_t devno;
{
	DEBUG_0 ("making special files\n")
	return(0);
}


/*
 * NAME: download_microcode
 * 
 * FUNCTION: This function downloads microcode of specific device 
 * 
 * EXECUTION ENVIRONMENT:
 *      This device specific routine is called by the generic config method 
 *      to download microcode for the specific device.
 *
 * (NOTES:) THIS IS A NULL FUNCTION FOR ADAPTER CONFIGURATION METHOD.
 *
 * RETURNS: Returns 0 on SUCCESS.
 */

int 
download_microcode(lname)
char *lname;
{
	DEBUG_0 ("download microcode\n")
	return(0);
}

/*
 * NAME: query_vpd
 * 
 * FUNCTION: This function querys the device via the device driver to 
 *           obtain the Vital Product Data(VPD) for the NIO Parallel Port. 
 * 
 * EXECUTION ENVIRONMENT:
 *
 * This function operates as a device dependent subroutine called by the 
 * generic configure method for all devices. It is used to obtain the vpd
 * data from the device and to update the database with the information.
 *
 * (NOTES:) THIS IS A NULL FUNCTION FOR NIO Parallel Port, because VPD
 *          can NOT be retrieved by calling service routines.
 *
 * RETURNS: Returns 0 on SUCCESS.
 */

int 
query_vpd(newobj, kmid, devno, vpd)
struct CuDv *newobj;
mid_t kmid;
dev_t  devno;
char *vpd;
{
	DEBUG_0 ("query vpd\n")
	return(0);
}
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
        return(0);
}
