static char sccsid[] = "@(#)31	1.4  src/bos/usr/lib/methods/cfgops/cfgops.c, cfgcommo, bos41J, 9517B_all 4/17/95 15:12:04";
/*
 * COMPONENT_NAME: (CFGMETH) Configure routines for serial optical link
 *			     subsystem
 *
 * FUNCTIONS: main, build_dds, generate minor, make_special_files
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <cf.h>		/* Error codes */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/comio.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/mode.h>
#include <sys/soluser.h>
#include <soldd.h>
#include <sys/sysconfig.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/uio.h>

#include "cfgdebug.h"
#include "pparms.h"

#define S_LENGTH	256
#define UNDEFINED_PID	-1
#define OPS_UTYPE	"uniquetype = 'node/node/ops'"
#define OPS_NAME	"ops0"		/* Name is always ops0 */
#define OPS_PdDvLn	"node/node/ops"
#define PORT_PdDvLn	"adapter/otp/op"
#define MKNOD_MODE	(S_IFMPX|S_IFCHR|S_IRUSR|S_IWUSR)

/* external functions */
extern long	genmajor();

/* main function code */
main(argc, argv, envp)
int	argc;
char	*argv[];
char	*envp[];

{
	extern  int     optind;         /* for getopt function */
	extern  char    *optarg;        /* for getopt function */

	struct	cfg_dd	cfg;		/* sysconfig command structure */

	char	*lname;			/* logical name to configure */
	char	*phase1, *phase2;	/* ipl phase flags */
	char	sstring[S_LENGTH];	/* search criteria pointer */

	struct CuAt *cusatt;		/* customized attribute class ptr */
	struct CuDv cusobj;		/* customized device object storage */
	struct PdDv preobj;		/* predefined device object storage */

	int     majorno;                /* major number assigned to device */
	int     minorno;                /* minor number assigned to device */
	long    *minor_list;            /* list returned by getminor */
	int     how_many;               /* number of minors in list */

	int	ipl_phase;		/* ipl phase: 0=run,1=phase1,2=phase2 */
	int	rc;			/* return codes go here */
	int     errflg,c;               /* used in parsing parameters   */

	/*****                                                          */
	/***** Parse Parameters                                         */
	/*****                                                          */
	ipl_phase = RUNTIME_CFG;
	errflg = 0;
	lname = OPS_NAME;

	while ((c = getopt(argc,argv,"l:2")) != EOF) {
		switch (c) {
			case 'l':
				break;
			case '2':
				ipl_phase = PHASE2;
				break;
			default:
				errflg++;
		}
	}
	if (errflg) {
		/* error in parameter list */
		DEBUG_0("cfgops: command line error\n")
		exit (E_ARGS);
	}

	/* start up odm */
	rc = odm_initialize();
	if (rc == -1) {
		/* initialization failed */
		DEBUG_0("cfgops: odm_initialize() failed\n")
		exit (E_ODMINIT);
	}
	DEBUG_0 ("ODM initialized\n")

	/* get predefined device object for OPS */
	sprintf(sstring,OPS_UTYPE);
	rc = odm_get_first(PdDv_CLASS,sstring,&preobj);
	if (rc == 0) {
		/* No PdDv object for this device */
		DEBUG_0("cfgops: failed to find OPS PdDv\n")
		err_exit(E_NOPdDv);
	}
	else if (rc == -1) {
		/* ODM failure */
		DEBUG_0("cfgops: ODM failure getting PdDv object");
		err_exit(E_ODMGET);
	}

	/* make sure at least one optical port is AVAILABLE */
	sprintf(sstring,"PdDvLn = '%s'",PORT_PdDvLn);
	rc = odm_get_first(CuDv_CLASS,sstring,&cusobj);
	if (rc == 0) {
		/* No CuDv object with this name */
		DEBUG_0("cfgops: failed to find port CuDv\n")
		err_exit (E_NOCuDv);
	}
	else if (rc == -1) {
		/* ODM failure */
		DEBUG_0("cfgops: ODM failure getting CuDv object")
		err_exit (E_ODMGET);
	}
	if (cusobj.status != AVAILABLE) {
		DEBUG_0("cfgops: no port is available\n")
		err_exit (E_DEPSTATE);
	}

	/* search for OPS customized object */
	sprintf(sstring,"PdDvLn = '%s'",OPS_PdDvLn);
	rc = odm_get_first(CuDv_CLASS,sstring,&cusobj);
	if (rc == 0) {
		/* No CuDv object with this name */
		DEBUG_0("cfgops: failed to find OPS CuDv\n")
		err_exit (E_NOCuDv);
	}
	else if (rc == -1) {
		/* ODM failure */
		DEBUG_0("cfgops: ODM failure getting CuDv object")
		err_exit (E_ODMGET);
	}

	/* make sure processor_id is defined */
	cusatt = getattr(lname,"processor_id",0,&rc);
	if (cusatt == (struct CuAt *) NULL) {
		DEBUG_0("cfgops: Error getting processor_id\n")
		err_exit (E_ODMGET);
	}
	if (atoi(cusatt->value) == UNDEFINED_PID) {
		DEBUG_0("cfgops: Processor ID not defined\n")
		if (ipl_phase == PHASE2) {
			/* give user a chance to provide the processor id */
			/* in the case of this being the initial install  */
			err_exit (E_OK);
		}
		else {
			/* user should have already defined the processor id */
			/* so return an error                                */
			err_exit (E_NOATTR);
		}
	}

	/****************************************************************
	  If this device is being configured during an ipl phase, then
	  display this device's LED value on the system LEDs.
	  ****************************************************************/
	if (ipl_phase != RUNTIME_CFG)
		setleds(preobj.led);

	/******************************************************************
	  Check to see if the device is already configured (AVAILABLE).
	  We actually go about the business of configuring the device
	  only if the device is not configured yet. Configuring the
	  device in this case refers to the process of checking for
	  attribute consistency, building a DDS, loading the driver, etc...
	  ******************************************************************/

	if (cusobj.status == DEFINED) {

		/* call loadext to get kmid */
		cfg.kmid = loadext(preobj.DvDr,TRUE,FALSE);
		if (cfg.kmid == NULL) {
			/* error querying kmid */
			DEBUG_0("cfgops: cannot query kmid\n")
			err_exit (E_LOADEXT);
		}

		/* get major number */
		DEBUG_0("cfgops: Calling genmajor()\n")
		majorno = genmajor(preobj.DvDr);
		if (majorno == -1) {
			DEBUG_0("cfgops: error generating major number")
			err_exit(E_MAJORNO);
		}
		DEBUG_1("cfgops: Returned major number: %d\n",majorno)

		/* get minor number */
		DEBUG_0("cfgops: Calling getminor()\n")
		minor_list = getminor(majorno,&how_many,lname);
		if (minor_list == NULL || how_many == 0) {
			DEBUG_0("cfgops: Calling generate_minor()\n")
			rc = generate_minor(lname, majorno, &minorno);
			if (rc) {
				/* Clean up bad minor # */
				reldevno(lname, TRUE);
				if ( rc < 0 || rc > 255)
					rc = E_MINORNO;
				err_exit(rc);
			}
		}
		else
			minorno = *minor_list;
		DEBUG_1("cfgops: minor number: %d\n",minorno)

		/* create devno for this device */
		cfg.devno = makedev(majorno, minorno);

		/* make special files      */
		DEBUG_0("cfgops: Calling make_special_files()\n")
		rc = make_special_files(lname, cfg.devno);
		if (rc) {
			/* error making special files */
			if ( rc < 0 || rc > 255)
				rc = E_MKSPECIAL;
			err_exit(rc);
		}
		DEBUG_0("cfgops: Returned from make_special_files()\n")

		/* build the DDS  */
		DEBUG_0("cfgops: Calling build_dds()\n")
		rc = build_dds(lname, &cfg.ddsptr, &cfg.ddslen);
		if (rc) {
			/* error building dds */
			DEBUG_0("cfgops: error building DDS\n")
			if ( rc < 0 || rc > 255)
				rc = E_DDS;
			err_exit(rc);
		}
		DEBUG_0("cfgops: Returned from build_dds()\n")

		/* call sysconfig to pass DDS to driver */
		DEBUG_0("cfgops: Pass DDS to driver via sysconfig()\n")
		cfg.cmd = CFG_INIT;
		rc = sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd));
		if (rc == -1) {
			/* error configuring device */
			DEBUG_0("cfgops: error configuring device\n")
			err_exit(E_CFGINIT);
		}

		/* update customized device object with a change operation */
		cusobj.status = AVAILABLE;
		rc = odm_change_obj(CuDv_CLASS,&cusobj);
		if (rc == -1) {
			/* ODM failure */
			DEBUG_0("cfgops: ODM failure updating CuDv object\n");
			err_exit(E_ODMUPDATE);
		}
	} /* end if (device is not AVAILABLE) then ... */

	odm_terminate();
	exit (E_OK);
}

/*
 * NAME: err_exit
 *
 * FUNCTION: Closes any open object classes and terminates ODM.  Used to
 *           back out on an error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.  The device
 *      specific routines for the various device specific config methods
 *      must not call this function.
 *
 * NOTES:
 *
 * void
 *   err_exit( exitcode )
 *      exitcode = The error exit code.
 *
 * RETURNS:
 *               None
 */

err_exit(exitcode)
char    exitcode;
{
	/* Close any open object class */
	odm_close_class(CuDv_CLASS);
	odm_close_class(PdDv_CLASS);
	odm_close_class(CuAt_CLASS);
	odm_close_class(PdAt_CLASS);

	/* Terminate the ODM */
	odm_terminate();
	exit(exitcode);
}

/*
 * NAME: build_dds
 *
 * FUNCTION: This function builds the DDS structure for the SOL subsystem.
 *
 * EXECUTION ENVIRONMENT: This function is called by the generic configure
 *                        method to build the define data structure of the SOL
 *
 * NOTES: There are 2 DDS structures for the SOL system: one for the subsystem
 *        and one for the ports.  The same structure is used for both DDS types
 *        and a flag in the structure determines the type of that structure.
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
	int	rc;			/* Result code */
	char	sstring[S_LENGTH];	/* Search string */
	struct utsname uname_buf;	/* Buffer for uname call */
	int	machine_id;		/* int value of uname machine id */

	/* allocate memory for DDS structure */
	dds_data = (struct sol_dds *) malloc(sizeof(struct sol_dds));
	if (dds_data == NULL ) {
		DEBUG_0("cfgops: Malloc of DDS failed\n")
		return (E_MALLOC);
	}

	/* Get Customized Object */
	sprintf(sstring,"name = '%s'",lname);
	rc = odm_get_obj(CuDv_CLASS,sstring,&Cus_obj,TRUE);
	if (rc == 0) {
		DEBUG_0("cfgops: no Customized Object to obtain\n")
		return (E_NOCuOBJ);
	}
	else if (rc == -1) {
		DEBUG_0("cfgops: cannot get Customized Object\n")
		return (E_ODMGET);
	}
 
	/* initialize reserved attributes to default values */
	dds_data->rsvd0 = 0;
	dds_data->rsvd3 = 0;
	dds_data->rsvd4 = 0;

	/* DDS is for subsystem */
	dds_data->dds_type = SOL_OPS_DDS;

	/* initialize reserved attributes to default values */
	dds_data->un.sol_ops.rsvd1 = 0;

	/* call system routine which will return the machine id of the system */
	uname (&uname_buf);
	machine_id = 0;
	/* convert character machine id to an integer */
	sscanf (uname_buf.machine, "%8X", &machine_id);
	dds_data->un.sol_ops.machine_id = machine_id;

	/* get processor id */
	rc = getatt(&dds_data->un.sol_ops.processor_id,'c',CuAt_CLASS,
		PdAt_CLASS,Cus_obj.name,Cus_obj.PdDvLn_Lvalue,"processor_id",
		(struct att *)NULL);
	if (rc != 0) {
		DEBUG_0("cfgops: Cannot get processor_id\n")
		return (E_NOATTR);
	}

	/* get status queue size */
	rc = getatt(&dds_data->un.sol_ops.sta_que_size,'i',CuAt_CLASS,
		PdAt_CLASS,Cus_obj.name,Cus_obj.PdDvLn_Lvalue,"sta_que_size",
		(struct att *)NULL);
	if (rc != 0) {
		DEBUG_0("cfgops: Cannot get sta_que_size\n")
		return (E_NOATTR);
	}
 
	/* get receive queue size */
	rc = getatt(&dds_data->un.sol_ops.rec_que_size,'i',CuAt_CLASS,
		PdAt_CLASS,Cus_obj.name,Cus_obj.PdDvLn_Lvalue,"rec_que_size",
		(struct att *)NULL);
	if (rc != 0) {
		DEBUG_0("cfgops: Cannot get rec_que_size\n")
		return (E_NOATTR);
	}
 
	*dds_len = sizeof(struct sol_dds);
	*ddsptr = (caddr_t) dds_data;

	DEBUG_0("cfgops: DDS successfully built\n")
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

	DEBUG_0("cfgops: Generating minor number\n")

	/* call genminor funcion to generate one minor number */
	/* force ops0 minor # to be 4 (required by device driver) */
	result = genminor(lname,majorno,SOL_OPS_MINOR,1,1,1);
	if (result == NULL) {
		DEBUG_0("cfgops: Error while getting minor number\n")
		return (E_MINORNO);
	}
	else {
		*minorno = *result;
		DEBUG_1("cfgops: minor number = %d\n",*minorno)
		return(E_OK);
	}
}

/*
 * NAME: make_special_files
 *
 * FUNCTION: This routine creates the device dependent files in /dev for the SOL
 *
 * EXECUTION ENVIRONMENT: This function is called from the generic config method
 *
 * NOTES: This routine checks for the validity of the device number that is
 *        passed to it, deletes any pre-existing special files, and then
 *        creates new special files.
 *
 * RETURNS: 0 on success, >0 on failure
 */

int
make_special_files(lname,devno)
char	*lname;
dev_t	devno;
{
	int	rc;			/* Result code */

	/* create SOL special files in /dev */
	DEBUG_0("cfgops: Making SOL special files for ops0\n")
	return (mk_sp_file(devno,lname,MKNOD_MODE));
}
