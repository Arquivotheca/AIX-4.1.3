static char sccsid[] = "@(#)32	1.6  src/bos/usr/lib/methods/cfgsol/cfgsol.c, cfgmethods, bos411, 9428A410j 5/12/93 14:33:48";
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: build_dds
 *		define_children
 *		download_microcode
 *		generate_minor
 *		make_special_files
 *		query_vpd
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <cf.h>		/* Error codes */
#include <errno.h>

#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/comio.h>
#include <sys/devinfo.h>
#include <sys/sysconfig.h>
#include <sys/soluser.h>
#include <soldd.h>
#include <sys/uio.h>
#include <sys/mdio.h>
#include <sys/device.h>
#include <sys/mode.h>

#include "pparms.h"
#include "cfgdebug.h"

#define S_LENGTH		256
#define CARD_PRESENT_MASK	0x0001
#define STATUS_2		0x0074
#define STATUS_2_SIZE		4

#define MKNOD_MODE	(S_IFMPX|S_IFCHR|S_IRUSR|S_IWUSR)

#define CHIP_PdDvLn	"adapter/sys/slc"
#define CARD_UTYPE	"uniquetype = 'adapter/slc/otp'"
#define PORT_UTYPE	"uniquetype = 'adapter/otp/op'"
#define PORT_PdDvLn	"adapter/otp/op"
#define CHIP0_Name	"slc0"

/*
 * NAME: build_dds
 *
 * FUNCTION: This function builds the DDS structure for the SOL system
 *
 * EXECUTION ENVIRONMENT: This function is called by the generic configure
 *                        method to build the define data structure of the SOL
 *
 * NOTES: The same DDS structure is used for both the subsystem and for each
 *        port.  A flag (dds_type) is used to distinguish a port DDS from a
 *        subsystem DDS.  Also, since this config method is used to configure
 *        the chips, cards, and ports, the DDS is built only when this config
 *        method has reached the port level since it is possible for the chips
 *        to be present but the cards and ports to be absent.  The DDS info,
 *	  however, is contained under the chip name.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int 
build_dds(lname,ddsptr,dds_len)
char	*lname;				/* logical name of the device */
char	**ddsptr;			/* pointer to DDS structure */
int	*dds_len;			/* size of DDS */
{
	struct	sol_dds	*dds_data;	/* pointer to SOL DDS structure */
	struct	CuDv	Cus_obj;	/* Customized object */
	struct	PdAt	Pre_obj;	/* Predefined object */
	char	sstring[S_LENGTH];	/* search string */
	int	rc;			/* Result code */

	/* allocate memory for DDS structure */
	dds_data = (struct sol_dds *) malloc(sizeof(struct sol_dds));
	if (dds_data == NULL ) {
		DEBUG_0("cfgsol: Malloc of DDS failed\n")
		return (E_MALLOC);
	}

	/* Get Customized Object */
	sprintf(sstring,"name = '%s'",lname);
	rc = odm_get_obj(CuDv_CLASS,sstring,&Cus_obj,TRUE);
	if (rc == 0) {
		DEBUG_0("cfgsol: no Customized Object to obtain\n")
		return (E_NOCuOBJ);
	}
	else if (rc == -1) {
		DEBUG_0("cfgsol: cannot get Customized Object\n")
		return (E_ODMGET);
	}

	/* initialize reserved attributes to default values */
	dds_data->rsvd0 = 0;
	dds_data->rsvd3 = 0;
	dds_data->rsvd4 = 0;

	dds_data->dds_type = SOL_PORT_DDS;

	/* initialize reserved attributes to default values */
	dds_data->un.sol_port.rsvd1 = 0;
	dds_data->un.sol_port.rsvd2 = 0;

	/* Get customized object that is parent of port (should be otp) */
	sprintf(sstring,"name = '%s'",Cus_obj.parent);
	rc = odm_get_obj(CuDv_CLASS,sstring,&Cus_obj,TRUE);
	if (rc == 0) {
		DEBUG_0("cfgsol: cannot find parent of port\n")
		return (E_NOCuOBJ);
	}
	else if (rc == -1) {
		DEBUG_0("cfgsol: cannot get Customized Object\n")
		return (E_ODMGET);
	}

	/* Get customized object that is parent of card (should be slc) */
	sprintf(sstring,"name = '%s'",Cus_obj.parent);
	rc = odm_get_obj(CuDv_CLASS,sstring,&Cus_obj,TRUE);
	if (rc == 0) {
		DEBUG_0("cfgsol: cannot find parent of card\n")
		return (E_NOCuOBJ);
	}
	else if (rc == -1) {
		DEBUG_0("cfgsol: cannot get Customized Object\n")
		return (E_ODMGET);
	}

	/* get buid */
	dds_data->un.sol_port.buid = (char *) strtol(Cus_obj.connwhere,
		(char **) NULL, 16);

	/* Get port Customized Object */
	sprintf(sstring,"name = '%s'",lname);
	rc = odm_get_obj(CuDv_CLASS,sstring,&Cus_obj,TRUE);
	if (rc == 0) {
		DEBUG_0("cfgsol: no Customized Object to obtain\n")
		return (E_NOCuOBJ);
	}
	else if (rc == -1) {
		DEBUG_0("cfgsol: cannot get Customized Object\n")
		return (E_ODMGET);
	}
	rc = atoi(Cus_obj.connwhere);
	if (rc == 2)   /* odd buid value for second port */
		dds_data->un.sol_port.buid++;

	*dds_len = sizeof(struct sol_dds);
	*ddsptr = (caddr_t) dds_data;

	DEBUG_0("cfgsol: DDS successfully built\n")
	return (E_OK);
}

/*
 * NAME: download_microcode
 *
 * FUNCTION: This function downloads microcode when applicable
 *
 * NOTES: This is a dummy function since the Serial Optical Link does not have
 *        microcode
 *
 * RETURNS: Always returns 0
 */

int 
download_microcode(lname)
char	*lname;
{
	return (E_OK);
}

/*
 * NAME: query_vpd
 *
 * FUNCTION: This function obtains the vital product data for the SOL from the
 *           4 byte configuration register of the SLA chip.
 *
 * NOTES: This function uses the sysconfig() call which calls sol_config()
 *        of the device driver to obtain the VPD info.
 *
 * RETURNS: Returns 0 on success, >0 on failure.
 */

int
query_vpd(newobj,kmid,devno,vpdstr)
struct	CuDv	*newobj;
mid_t	kmid;				/* Device Driver Kernel module ID */
dev_t	devno;				/* Major & Minor # */
char	*vpdstr;			/* String to contain VPD */
{
	struct	cfg_dd	cfg;		/* dd config structure */
	char	vpd_area[SOL_VPDSIZE];	/* VPD storage area */
	int	rc;			/* result code */

	DEBUG_0("cfgsol: Entering query_vpd()\n")

	/* Initialize cfg structure to prep for sysconfig() call which will */
	/* then call the dd_config() of the device driver */
	cfg.kmid = kmid;
	cfg.devno = devno;
	cfg.cmd = CFG_QVPD;			/* Command to read VPD */
	cfg.ddsptr = (char *) vpd_area;		/* VPD storage area */
	cfg.ddslen = SOL_VPDSIZE;			/* VPD storage area size */

	rc = sysconfig(SYS_CFGDD,&cfg,sizeof(cfg));
	if (rc != 0) {
		DEBUG_0("cfgsol: could not obtain VPD\n")
		return (E_VPD);
	}

	/* Store VPD in database */
	put_vpd(vpdstr,vpd_area,SOL_VPDSIZE);
	return (E_OK);
}

/*
 * NAME: generate_minor
 *
 * FUNCTION: This function generates a minor number for an optic port.
 *
 * NOTES:
 *
 * RETURNS: Returns 0 on success, >0 on failure
 *
 */

int
generate_minor(lname,majorno,minorno)
char	*lname;				/* device logical name */
long	majorno;			/* major # of the device */
long	*minorno;			/* minor # of the device */
{
	long	*result;

	DEBUG_0("cfgsol: Generating minor number\n")

	/* call genminor funcion to generate one minor number */
	result = genminor(lname,majorno,-1,1,1,1);
	if (result == NULL) {
		DEBUG_0("cfgsol: Error while getting minor number\n")
		return (E_MINORNO);
	}
	else {
		*minorno = *result;
		DEBUG_1("cfgsol: minor number = %d\n",*minorno)
		return (E_OK);
	}
}

/*
 * NAME: define_children
 *
 * FUNCTION: This function detects and manages the children of the SOL system
 *           (in this case, the ports).  The ports must be defined if they do
 *           not already exist in the Customized database.  The names of all
 *           defined children are written to stdout so that they can also be
 *           configured.
 *
 * EXECUTION ENVIRONMENT: This routine is called from the generic config method
 *
 * RETURNS: 0 on success, >0 on failure
 */

int
define_children(lname,ipl_phase)
char	*lname;			/* logical name of device */
int	ipl_phase;		/* ipl phase */
{
	struct	CuDv	Cus_obj;	/* Customized object */
	struct	PdDv	Pre_obj;	/* Predefined object */
	struct	CuAt	*Cus_att;	/* Customized attribute */
	char	sstring[S_LENGTH];	/* Search criteria string */
	char	*outp;			/* output from odm_run_method */
	int	rc,rc1;			/* Result codes */
	MACH_DD_IO	mdd;		/* Command structure for ioctl */
	int	md_fd;
	ulong	mdd_result;
	int	connect_pt;
	uchar	slot;

	DEBUG_0("cfgsol: Entered define_children()\n")

	/* Get Customized Object */
	sprintf(sstring,"name = '%s'",lname);
	rc = odm_get_obj(CuDv_CLASS,sstring,&Cus_obj,TRUE);
	if (rc == 0) {
		DEBUG_0("cfgsol: no Customized Object to obtain\n")
		return (E_NOCuOBJ);
	}
	else if (rc == -1) {
		DEBUG_0("cfgsol: cannot get Customized Object\n")
		return (E_ODMGET);
	}

	/* if object is port, return since ports have no children */
	rc = strcmp(Cus_obj.PdDvLn_Lvalue,PORT_PdDvLn);
	if (rc == 0)
		return (E_OK);

	rc = strcmp(Cus_obj.PdDvLn_Lvalue,CHIP_PdDvLn);
	if (rc == 0) {
		/* must ensure card is present before continuing */
		md_fd = open("/dev/bus0",O_RDONLY,0);
		if (md_fd == -1) {
			DEBUG_0("cfgsol: bus0 open failed\n")
			return (E_FINDCHILD);
		}
		mdd.md_incr = MV_WORD;
		mdd.md_size = 1;	/* 1 word (4 bytes) output */
		mdd.md_data = &mdd_result;
		mdd.md_addr = STATUS_2;

		/* get buid */
		mdd.md_buid = (char *)strtol(Cus_obj.connwhere,
			(char **) NULL, 16);

		rc = ioctl(md_fd,MSLAGET,(caddr_t)&mdd);
		if (rc) {
			DEBUG_0("cfgsol: STATUS2 ioctl failed\n")
			close(md_fd);
			return (E_FINDCHILD);
		}
		close(md_fd);
		rc = mdd_result & CARD_PRESENT_MASK;
		if (rc)
			return (E_OK);		/* Return if no card present */

		/* get predefined object for cards */
		rc = odm_get_obj(PdDv_CLASS,CARD_UTYPE,&Pre_obj,TRUE);
		if (rc == 0) {
			DEBUG_0("cfgsol: no card predefine\n")
			return (E_NOPdDv);
		}
		else if (rc == -1) {
			DEBUG_0("cfgsol: cannot get PdDv\n")
			return (E_ODMGET);
		}

		if (strcmp(Cus_obj.name,CHIP0_Name))
			slot=0;
		else slot=1;

		sprintf(sstring,"parent = '%s'",lname);
		rc = odm_get_obj(CuDv_CLASS,sstring,&Cus_obj,TRUE);
		if (rc == 0) {
			sprintf(sstring,"-c adapter -s slc -t otp -p %s -w %d",
				lname,slot);
			rc1 = odm_run_method(Pre_obj.Define,sstring,&outp,NULL);
			if (rc1) {
				DEBUG_0("cfgsol: cannot run define method\n")
				return (E_ODMRUNMETHOD);
			}
			fprintf(stdout,"%s\n",outp);
		}
		else if (rc > 0) {
			if ( Cus_obj.chgstatus==MISSING){
				Cus_obj.chgstatus = SAME;
				rc1 = odm_change_obj(CuDv_CLASS,&Cus_obj);
				if (rc1 < 0) {
					DEBUG_0("cfgsol: cannot change CuDv\n")
					return (E_ODMUPDATE);
				}
			}
			fprintf(stdout,"%s\n",Cus_obj.name);
		}
		else /* rc == -1 */
			return (E_ODMGET);
	}
	else {	/* going to define children of card */
		/* get predefined object for ports */
		rc = odm_get_obj(PdDv_CLASS,PORT_UTYPE,&Pre_obj,TRUE);
		if (rc == 0) {
			DEBUG_0("cfgsol: no port predefine\n")
			return (E_NOPdDv);
		}
		else if (rc == -1) {
			DEBUG_0("cfgsol: cannot get PdDv\n")
			return (E_ODMGET);
		}

		for (connect_pt=1;connect_pt<3;connect_pt++) {
			/* get object with lname as its parent and correct */
			/* connection point */
			sprintf(sstring,"parent = '%s' AND connwhere = '%d'",
				lname,connect_pt);
			rc = odm_get_obj(CuDv_CLASS,sstring,&Cus_obj,TRUE);
			if (rc == 0) {
				sprintf(sstring,
					"-c adapter -s otp -t op -p %s -w %d",
					lname,connect_pt);
				rc1 = odm_run_method(Pre_obj.Define,sstring,
					&outp,NULL);
				if (rc1) {
					DEBUG_0("cfgsol: fail define method\n")
					return (E_ODMRUNMETHOD);
				}
				fprintf(stdout,"%s\n",outp);
			}
			else if (rc > 0) {
				if( Cus_obj.chgstatus==MISSING){
					Cus_obj.chgstatus = SAME;
					rc1 = odm_change_obj(CuDv_CLASS,&Cus_obj);
					if (rc1 < 0) {
						DEBUG_0("cfgsol: cannot change CuDv\n")
						return (E_ODMUPDATE);
					}
				}
				fprintf(stdout,"%s\n",Cus_obj.name);
			}
			else /* rc == -1 */
				return (E_ODMGET);
		}
	}

	return (E_OK);
}

/*
 * NAME: make_special_files
 *
 * FUNCTION: This routine creates the device dependent files in /dev for the SOL
 *
 * EXECUTION ENVIRONMENT: This function is called from the generic config method
 *
 * NOTES: This routine creates the special files for the SOL ports by calling
 *	  mk_sp_file() which is located in "cfgtools.c".
 *
 * RETURNS: 0 on success, >0 on failure
 */

int
make_special_files(lname,devno)
char	*lname;
dev_t	devno;
{
	/* create SOL special files in /dev */
	DEBUG_0("cfgsol: Making SOL special files\n")
	return (mk_sp_file(devno,lname,MKNOD_MODE));
}
