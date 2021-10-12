static char sccsid[] = "@(#)24        1.1  src/bos/usr/lib/methods/cfgdmpa/cfgdmpa.c, sysxdmpa, bos411, 9428A410j 4/30/93 11:57:00";
/*
 *   COMPONENT_NAME: (MPACFG) MP/A CONFIGURATION FILES
 *
 *   FUNCTIONS: VALIDATE_ATTR
 *		build_dds
 *		define_children
 *		download_microcode
 *		generate_minor
 *		make_special_files
 *		query_vpd
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/cfgodm.h>
#include <sys/stat.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include <sys/dmpauser.h>
#include <sys/dmpadd.h>

#include <sys/limits.h>
#include <sys/cfgdebug.h>

#define VALIDATE_ATTR() \
	if (CuAt_ptr == NULL) \
		return E_NOATTR

long *genminor(char  *device_instance,
	       long  major_no,
	       int   preferred_minor,
	       int   minors_in_grp,
	       int   inc_within_grp,
	       int   inc_btwn_grp);

/*
 * NAME: build_dds
 *
 * FUNCTION: Builds a DDS (Defined Data Structure) describing a device's
 *           characteristics to it's device driver.
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: Values are accessed from records in the CuDv and CuAt object
 *        classes, first from the device and parent objects in the CuDv object
 *        class, then from the objects for the device in the CuAt object class.
 *        Values are edited and and placed in the DDS structure which is
 *        returned to the calling generic configure.
 *
 * RETURNS: 0 - success
 *         >0 - failure
 *
 */

int build_dds(lname, ddsptr, ddslen)
char  *lname;                           /* logical name of device */
uchar **ddsptr;                         /* receiving pointer for DDS address */
long  *ddslen;                          /* receiving variable for DDS length */
{
	int rc;
	struct CuDv my_CuDv;            /* has my parent */
	struct CuDv parent_CuDv;        /* has my grandparent */
	struct CuAt *CuAt_ptr;
	int num_attrs;
	struct mpadds *dds_p;
	static struct mpadds dds;
	char sstr[128];
	long value;

	/* Clear the DDS structure */
	dds_p = &dds;
	bzero(dds_p, sizeof(struct mpadds));
	*ddsptr = (uchar *)dds_p;
	*ddslen = sizeof(dds);

	/*
	** Fill in the DDS with both "hard-coded" and ODM attribute values
	*/

	/*
	** Slot Number (Note: the stamped numbers on the back of the machine
	** and the numbers in ODM start with 1, whereas, internally, slots start with 0)
	*/
	sprintf(sstr,"name=%s",lname);      /* dmpax, sdlcx, or xmpax */
	DEBUG_1(" %s\n", sstr)

	/*
	** First get my CuDv entry so I can find my parent, mpa0 or mpa1
	*/
	rc = (int)odm_get_first(CuDv_CLASS,sstr,&my_CuDv);
	if (rc == 0) {
		return(E_NOCuDv);
	}
	else if (rc == -1) {
		return(E_ODMGET);
	}

	/*
	** Now get my Parents CuDv entry so I can find my grandparent,
	** bus0 or bus1
	*/
	sprintf(sstr,"name=%s",my_CuDv.parent);   /* mpa0, mpa1 */
	rc = (int)odm_get_first(CuDv_CLASS,sstr,&parent_CuDv);
	if (rc == 0) {
		return(E_NOCuDv);
	}
	else if (rc == -1) {
		return(E_ODMGET);
	}


	dds_p->slot_num = atoi(parent_CuDv.connwhere) - 1;
	DEBUG_1( "Slot number is %d after decrement\n", dds_p->slot_num )

	/* Read attributes from CuAt, and PdAt tables */

	/* Bus ID */
	CuAt_ptr = getattr(parent_CuDv.parent, "bus_id", 0, &num_attrs);
	VALIDATE_ATTR();
	dds_p->bus_id = strtoul(CuAt_ptr->value, (char **)NULL, 0);
	dds_p->bus_id |= 0x800C0020;
	DEBUG_1("bus_id is = %08X\n", dds_p->bus_id)

	/* Bus Type */
	CuAt_ptr = getattr(parent_CuDv.parent, "bus_type", 0, &num_attrs);
	VALIDATE_ATTR();
	dds_p->bus_type = strtoul(CuAt_ptr->value, (char **)NULL, 0);
	DEBUG_1("bus_type is = %X\n", dds_p->bus_type)

	/* Bus I/O Address */
	CuAt_ptr = getattr(my_CuDv.parent, "bus_io_addr", 0, &num_attrs);
	VALIDATE_ATTR();
	dds_p->io_addr = strtoul(CuAt_ptr->value, (char **)NULL, 0);
	DEBUG_1("bus_io_addr = %X\n", dds_p->io_addr)


	/* Bus Interrupt Level */
	CuAt_ptr = getattr(my_CuDv.parent, "bus_intr_lvlA", 0, &num_attrs);
	VALIDATE_ATTR();
	dds_p->int_lvl = strtoul(CuAt_ptr->value, (char **)NULL, 0);
	if(dds_p->int_lvl != 3) {
	      /*
	      ** this diagnostics driver only operates at
	      ** interrupt level 3.  
	      */
		DEBUG_1("bus_intr_lvl = %X\n", dds_p->int_lvl)
		return(E_DDS);
	}

	DEBUG_1("bus_intr_lvl = %X\n", dds_p->int_lvl)

	/* Interrupt Priority */
	CuAt_ptr = getattr(my_CuDv.parent, "intr_priority", 0, &num_attrs);
	VALIDATE_ATTR();
	dds_p->intr_priority = strtoul(CuAt_ptr->value, (char **)NULL, 0);
	DEBUG_1("bus_intr_pri = %X\n", dds_p->intr_priority)

	/* DMA Arbitration Level */
	CuAt_ptr = getattr(my_CuDv.parent, "dma_lvl", 0, &num_attrs);
	VALIDATE_ATTR();
	dds_p->dma_lvl = strtoul(CuAt_ptr->value, (char **)NULL, 0);
	DEBUG_1("dma_lvl  = %X\n", dds_p->dma_lvl)

	/* Resource Name passed to error logging facilities */
	strncpy(dds_p->resource_name, lname, 16);
	DEBUG_1("resource_name = %s\n", dds_p->resource_name)


	return(0);
}

/*
 *
 * NAME: generate_minor
 *
 * FUNCTION: To provide a unique minor number for the current device instance.
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: Simply calls the generic generate minor number routine.
 *
 * RETURNS: 0 - success
 *         >0 - failure
 *
 */

long  generate_minor( lname, majorno, minorno )
char    *lname;
long    majorno;
long    *minorno;
{
	long   *rc;

	rc = genminor( lname, majorno, -1, 1, 1, 1 );
	if( (int)rc == NULL )
		return E_MINORNO;

	*minorno = *rc;

	return 0;
}

/*
 *
 * NAME: make_special_file
 *
 * FUNCTION: To create the special character file in /dev for the current
 *           device instance.
 *
 * EXECUTION ENVIRONMENT:
 *
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: Simply calls the generic create special file routine.
 *
 * RETURNS: 0 - success
 *         >0 - failure
 *
 */

int make_special_files( lname, devno )

char    *lname;
dev_t   devno;

{
	long   typemode;                /* type and mode of created file */
	long   rc;                      /* return code */



	typemode = S_IRUSR | S_IFCHR    /* set type for char special file */
		| S_IRGRP | S_IROTH     /* set mode for rw------- permisions */
		| S_IWUSR | S_IFMPX
		| S_IWGRP | S_IWOTH;

	/* call the general routine */

	rc = mk_sp_file( devno, lname, typemode );

	return(rc);
}

/*
 *
 * NAME: query_vpd
 *
 * FUNCTION: To query the MPA device for VPD information (Vital Product Data)
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: For the MPA adapter, VPD information is not currently supported,
 *        this function is then NULL.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int  query_vpd( newobj, kmid, devno, vpdstr )
char   *newobj;
mid_t   kmid;
dev_t   devno;
char    *vpdstr;

{
	return(0);
}

/*
 *
 * NAME: define_children
 *
 * FUNCTION: To manage the children of the device.
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: The MPA card has device drivers as children. I need to define
 *        the ones that have entries in the PdDv.
 *
 *
 * RETURNS: 0 - success, in all cases
 *
 */
int define_children(lname, children, cusdev, phase)
char  *lname;
char *children;
long cusdev;
int phase;

{
	return(0);
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* NAME:                                                                 */
/*      download_microcode()                                             */
/*                                                                       */
/* FUNCTION:                                                             */
/*                NONE   I have no microcode to load                     */
/*                                                                       */
/* RETURN CODES:                                                         */
/*      Required                                                         */
/*               0 = Successful                                          */
/*                                                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int download_microcode(
char *lname)
{
     return 0;
}
