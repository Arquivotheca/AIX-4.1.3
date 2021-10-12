static char sccsid[] = "@(#)36	1.4  src/bos/usr/lib/methods/cfgswpnfs/cfgswpnfs.c, cfgmethods, bos411, 9428A410j 12/3/93 14:10:54";
/*
 * COMPONENT_NAME: (CFGMETHODS) cfgswpnfs.c - Swap NFS device Config Method Code
 *
 * FUNCTIONS: main(), err_exit(), err_undo1(), 
 *            generate_minor(), make_special_files(),
 *            build_dds(), loadnfsext()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* interface:
   cfgswpnfs -l <logical_name> [-<1|2>]
*/


/* header files needed for compilation */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/sysconfig.h>
#include <sys/sysmacros.h>
#include <sys/device.h>
#include <netdb.h>
#include <netinet/in.h>
#include <nlist.h>

/* Local header files */
#include "cfgdebug.h"

/* external functions */
extern long	genmajor();
extern	struct	CuAt	*getattr();

#define RWPERMS 0600

#define NFS_KERNEL_EXT	"/etc/nfs.ext"
#define UNIQUE_TYPE	"swap/nfs/paging"
#define HOSTNAME_ATTR	"hostname"
#define SWAPFILENAME_ATTR	"swapfilename"
#define SWAPKPROCS_ATTR	"swapkprocs"

/* main function code */
main(argc, argv, envp)
int	argc;
char	*argv[];
char	*envp[];

{
	extern  int     optind;         /* for getopt function */
	extern  char    *optarg;        /* for getopt function */

	struct cfg_dd cfg;		/* sysconfig command structure */

	char	*logical_name;		/* logical name to configure */
	char	*phase1, *phase2;	/* ipl phase flags */
	char	sstring[256];		/* search criteria pointer */

	struct Class *cusdev;		/* customized devices class ptr */
	struct Class *predev;		/* predefined devices class ptr */
	struct Class *cusvpd;           /* customized vpd class ptr */

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
	logical_name = NULL;

	while ((c = getopt(argc,argv,"l:12")) != EOF) {
		switch (c) {
		case 'l':
			if (logical_name != NULL)
				errflg++;
			logical_name = optarg;
			break;
		case '1':
			if (ipl_phase != RUNTIME_CFG)
				errflg++;
			ipl_phase = PHASE1;
			break;
		case '2':
			if (ipl_phase != RUNTIME_CFG)
				errflg++;
			ipl_phase = PHASE2;
			break;
		default:
			errflg++;
		}
	}
	if (errflg) {
		/* error parsing parameters */
		DEBUG_0("cfgdevice: command line error\n");
		exit(E_ARGS);
	}

	/*****                                                          */
	/***** Validate Parameters                                      */
	/*****                                                          */
	/* logical name must be specified */
	if (logical_name == NULL) {
		DEBUG_0("cfgdevice: logical name must be specified\n");
		exit(E_LNAME);
	}

	DEBUG_1 ("Configuring device: %s\n",logical_name)

	/* start up odm */
	if (odm_initialize() == -1) {
		/* initialization failed */
		DEBUG_0("cfgdevice: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}
	DEBUG_0 ("ODM initialized\n")

	/* open customized devices object class */
	if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1) {
		DEBUG_0("cfgdevice: open class CuDv failed\n");
		err_exit(E_ODMOPEN);
	}

	/* search for customized object with this logical name */
	sprintf(sstring, "name = '%s'", logical_name);
	rc = (int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* No CuDv object with this name */
		DEBUG_1("cfgdevice: failed to find CuDv object for %s\n", logical_name);
		err_exit(E_NOCuDv);
	}
	else if (rc==-1) {
		/* ODM failure */
		DEBUG_0("cfgdevice: ODM failure getting CuDv object");
		err_exit(E_ODMGET);
	}

	/* open predefined devices object class */
	if ((int)(predev = odm_open_class(PdDv_CLASS)) == -1) {
		DEBUG_0("cfgdevice: open class PdDv failed\n");
		err_exit(E_ODMOPEN);
	}

	/* get predefined device object for this logical name */
	sprintf(sstring, "uniquetype = '%s'", cusobj.PdDvLn_Lvalue);
	rc = (int)odm_get_first(predev, sstring, &preobj);
	if (rc==0) {
		/* No PdDv object for this device */
		DEBUG_0("cfgdevice: failed to find PdDv object for this device\n");
		err_exit(E_NOPdDv);
	}
	else if (rc==-1) {
		/* ODM failure */
		DEBUG_0("cfgdevice: ODM failure getting PdDv object");
		err_exit(E_ODMGET);
	}

	/* close predefined device object class */
	if (odm_close_class(predev) == -1) {
		DEBUG_0("cfgdevice: close object class PdDv failed");
		err_exit(E_ODMCLOSE);
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
	  device in this case refers to the process of checking parent
	  and sibling status, checking for attribute consistency, build-
	  ing a DDS, loading the driver, etc...
	  ******************************************************************/

	if (cusobj.status == DEFINED) {

		/***************************************************
		  If device has a device driver, then need to load driver,
		  get major number, and call device dependent routines to
		  get minor number, make special files, and build DDS.
		  This code then passes the DDS to the driver.  Finally,
		  a device dependent routine is called for downloading
		  microcode.
		 ***************************************************/
		if (strcmp(preobj.DvDr, "") != 0) {

			/* call loadnfsext to make sure that the NFS
			   kernel extension has been loaded before loading
			   the device driver for swapping
			   */
			if (loadnfsext()) {
				DEBUG_0("cfgswpnfs: error loading NFS kernel extension")
				err_exit(E_LOADEXT);
			}

			/* call loadext to load the device driver */
			if ((cfg.kmid = loadext(preobj.DvDr, TRUE, FALSE)) == NULL) {
				/* error loading device driver */
				DEBUG_1("cfgdevice: error loading driver %s\n", preobj.DvDr)
				err_exit(E_LOADEXT);
			}

			/* get major number      */
			DEBUG_0("cfgdevice: Calling genmajor()\n")
			if ((majorno = genmajor(preobj.DvDr)) == -1) {
				DEBUG_0("cfgdevice: error generating major number");
				err_undo1(preobj.DvDr);
				err_exit(E_MAJORNO);
			}
			DEBUG_1("cfgdevice: Returned major number: %d\n",majorno)

			/* get minor number      */
			DEBUG_0("cfgdevice: Calling getminor()\n")
			minor_list = getminor(majorno,&how_many,logical_name);
			if (minor_list == NULL || how_many == 0) {
				DEBUG_0("cfgdevice: Calling generate_minor()\n")
				rc = generate_minor(logical_name, majorno, &minorno);
				if (rc) {
					DEBUG_1("cfgdevice: error generating minor number, rc=%d\n",rc)
					/* First make sure any minors that might have */
					/* been assigned are cleaned up */
					reldevno(logical_name, TRUE);
					err_undo1(preobj.DvDr);
					if ( rc < 0 || rc > 255)
						rc = E_MINORNO;
					err_exit(rc);
				}
				DEBUG_0("cfgdevice: Returned from generate_minor()\n")
			}
			else
				minorno = *minor_list;
			DEBUG_1("cfgdevice: minor number: %d\n",minorno)

			/* create devno for this device */
			cfg.devno = makedev(majorno, minorno);

			/* make special files      */
			DEBUG_0("cfgdevice: Calling make_special_files()\n")
			rc = make_special_files(logical_name, cfg.devno);
			if (rc) {
				/* error making special files */
				DEBUG_1("cfgdevice: error making special file(s), rc=%d\n",rc)
				err_undo1(preobj.DvDr);
				if ( rc < 0 || rc > 255)
					rc = E_MKSPECIAL;
				err_exit(rc);
			}
			DEBUG_0("cfgdevice: Returned from make_special_files()\n")

			/* build the DDS  */
			DEBUG_0("cfgdevice: Calling build_dds()\n")
			rc = build_dds(logical_name, &cfg.ddsptr, &cfg.ddslen);
			if (rc) {
				/* error building dds */
				DEBUG_1("cfgdevice: error building dds, rc=%d\n",rc)
				err_undo1(preobj.DvDr);
				if ( rc < 0 || rc > 255)
					rc = E_DDS;
				err_exit(rc);
			}
			DEBUG_0("cfgdevice: Returned from build_dds()\n")

			/* call sysconfig to pass DDS to driver */
			DEBUG_0("cfgdevice: Pass DDS to driver via sysconfig()\n")
			cfg.cmd = CFG_INIT;
			if (sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd )) == -1) {
				/* error configuring device */
				DEBUG_0("cfgdevice: error configuring device\n")
				err_undo1(preobj.DvDr);
				err_exit(E_CFGINIT);
			}


		} /* end if (device has a driver) then ... */


		/* update customized device object with a change operation */
		cusobj.status = AVAILABLE;
		if (odm_change_obj(cusdev, &cusobj) == -1) {
			/* ODM failure */
			DEBUG_0("cfgdevice: ODM failure updating CuDv object\n");
			err_exit(E_ODMUPDATE);
		}
	} /* end if (device is not AVAILABLE) then ... */

	/* close customized device object class */
	if (odm_close_class(cusdev) == -1) {
		DEBUG_0("cfgdevice: error closing CuDv object class\n");
		err_exit(E_ODMCLOSE);
	}

	odm_terminate();
	exit(0);

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

	/* Terminate the ODM */
	odm_terminate();
	exit(exitcode);
}

/*
 * NAME: err_undo1
 *
 * FUNCTION: Unloads the device's device driver.  Used to back out on an
 *           error.
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
 *   err_undo1( DvDr )
 *      DvDr = pointer to device driver name.
 *
 * RETURNS:
 *               None
 */

err_undo1(DvDr)
char    *DvDr;                  /* pointer to device driver name */
{
	/* unload driver */
	if (loadext(DvDr,FALSE,FALSE) == NULL) {
		DEBUG_0("cfgdevice: error unloading driver\n");
	}
	return;
}




/*
 * NAME: generate_minor
 *                                                                    
 * FUNCTION: Build the correct parameters to call genminor()
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	Process
 *                                                                   
 * RETURNS: 
 *	E_OK - Minorno will contain newly assigned minor number for device
 */  

int	
generate_minor(logical_name,major,minorno)
	char	*logical_name;
	long	major;
	long	*minorno;
{
	long	*list;
	int	preferred,number,skip,align;
	int	howmany,rc;

	preferred = (-1);	/* only one minor number wanted */
	number = 1;
	skip = 1;
	align = 1;
	list = genminor(logical_name,major,(-1),number,skip,align);
	if (list == NULL) {
		DEBUG_0("cfgswpnfs: genminor failed");
		return (E_MINORNO);
	}

	*minorno = *list;
	return(E_OK);

}  /* generate_minor() */


/*
 * NAME: make_special_files
 *                                                                    
 * FUNCTION: Call mk_sp_file with correct parameters
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	Process
 *
 * RETURNS:
 *	Return of mk_sp_file()
 */  
make_special_files(logical_name,devno)
	char	*logical_name;
	dev_t	devno;
{
	return(mk_sp_file(devno,logical_name,S_IFBLK | RWPERMS));
}

/*
 * NAME: build_dds
 *                                                                    
 * FUNCTION: Build the device dependent structure that will contain the
 *	specific configuration information for the SWAP NFS device.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	This routine retrieves the following information from the
 *	configuration database:
 *		hostname - hostname of the server for this NFS swap device
 *		swapfilename - path name to the file that will be used 
 *			for reading/writing to for the swap device
 *		swapkprocs - number of daemons that are to be started for
 *			this particular device.
 *
 * RETURNS: 
 *	ddsptr is given the address of the device dependent data structure.
 */  
build_dds(logical_name,ddsptr,ddslen)
	char	*logical_name;		/* logical_name */
	char	**ddsptr;
	int	*ddslen;
{
	/* Note: This data structure is shared between this configuration
	   method and the swapnfsdd device driver.  If this structure 
	   needs to change it needs to be updated in both places.
	   */
	struct	swapnfsdds {
		struct	in_addr	swapserver;
		int	swapkprocs;
		int	hostnamelen;
		int	filenamelen;
		char	swaphostandfile[1];
	} *swapnfsdds;
	struct  Class   *cusattr;
	struct  Class   *preattr;
	char    hostname[MAXHOSTNAMELEN];
	char	swapfilename[MAXPATHLEN];
	int	swapkprocs;
	char	kprocs_str[20];
	struct	hostent	*swapserver;

	/* Open needed ODM databases
	   */
	cusattr=odm_open_class(CuAt_CLASS);
	if ((int)cusattr == -1) {
		DEBUG_0("cfgfdd: build_dds: open class CuAt failed");
		return(E_ODMOPEN);
	}
	preattr=odm_open_class(PdAt_CLASS);
	if ((int)preattr == -1) {
		DEBUG_0("cfgfdd: build_dds: open class PdAt failed");
		return(E_ODMOPEN);
	}

	if (getatt(hostname, 's', cusattr, preattr,
		   logical_name, UNIQUE_TYPE, HOSTNAME_ATTR,
		   (struct attr *)NULL) > 0) {
		return(E_NOATTR);
	}
	DEBUG_1("build_dds: attribute hostname = %s\n", hostname)
	if (getatt(swapfilename, 's', cusattr, preattr,
		   logical_name, UNIQUE_TYPE, SWAPFILENAME_ATTR,
		   (struct attr *)NULL) > 0) {
		return(E_NOATTR);
	}
	DEBUG_1("build_dds: attribute swapfilename = %s\n", swapfilename)
	if (getatt(kprocs_str, 's', cusattr, preattr,
		   logical_name, UNIQUE_TYPE, SWAPKPROCS_ATTR,
		   (struct attr *)NULL) > 0) {
		return(E_NOATTR);
	}
	sscanf(kprocs_str, "%d", &swapkprocs);
	DEBUG_1("build_dds: attribute swapkprocs = %d\n", swapkprocs)


	/* Where are we going to put driver specific info and 
	   how much space do we need ?  Okay, we get an extra byte.
	   */
	*ddslen = sizeof(struct swapnfsdds) + 
		strlen(swapfilename) +
		strlen(hostname);
	if ((*ddsptr = (char *)malloc(*ddslen)) == NULL) {
		return(E_MALLOC);
	}
	swapnfsdds = (struct swapnfsdds *)*ddsptr;

	/* Get the servers address
	   */
	if ((swapserver = gethostbyname(hostname)) == NULL) {
		DEBUG_0("build_dds: gethostbyname failed")
		return(E_BADATTR);
	}
	bcopy(swapserver->h_addr, 
	      &(swapnfsdds->swapserver), 
	      swapserver->h_length);

	swapnfsdds->swapkprocs = swapkprocs;

	swapnfsdds->hostnamelen = strlen(hostname);
	swapnfsdds->filenamelen = strlen(swapfilename);

	bcopy(hostname, &(swapnfsdds->swaphostandfile[0]),
	      swapnfsdds->hostnamelen);
	bcopy(swapfilename, 
	      &(swapnfsdds->swaphostandfile[swapnfsdds->hostnamelen]),
	      swapnfsdds->filenamelen);
	return(E_OK);
}


/*
 * NAME: loadnfsext
 *                                                                    
 * FUNCTION: Load and configure the NFS kernel extension.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	Process.	Use sysconfig() to load and configure
 * RETURNS:
 *	0 - If everything went okay.
 *	1 - If something is went awry with sysconfig()
 */  
int
loadnfsext()
{
	char	*file=NFS_KERNEL_EXT;
	char	*mod_id = "nfsclnt_kmid";
	mid_t	kmid;
	struct cfg_load	cload;
	struct cfg_kmod	cmod;
	mid_t	check_load();
	int	ret;

	/* load the KDES kernel extension for all of the others */
	if ((ret = check_load("nfskdes_kmid")) < 0) {
		return(1);
	}
	if (ret == 0) {
		/* Load the gfs into memory */
		cload.path = "/usr/lib/drivers/nfs_kdes.ext";
		if (sysconfig(SYS_KLOAD, &cload, sizeof(cload)) == -1) {
			return(1);
		}
		/* Initialize the gfs */
		cmod.kmid = cload.kmid;
		cmod.cmd = CFG_INIT;
		cmod.mdiptr = (caddr_t) &cmod.kmid;
		cmod.mdilen = sizeof(cmod.kmid);
		if (sysconfig(SYS_CFGKMOD, &cmod, sizeof(cmod)) == -1) {
			return(1);
		}
	}

	/* pick up the KRPC kernel extension */
	if ((ret = check_load("nfskrpc_kmid")) < 0) {
		return(1);
	}
	if (ret == 0) {
		/* Load the gfs into memory */
		cload.path = "/usr/lib/drivers/nfs_krpc.ext";
		if (sysconfig(SYS_KLOAD, &cload, sizeof(cload)) == -1) {
			return(1);
		}
		/* Initialize the gfs */
		cmod.kmid = cload.kmid;
		cmod.cmd = CFG_INIT;
		cmod.mdiptr = (caddr_t) &cmod.kmid;
		cmod.mdilen = sizeof(cmod.kmid);
		if (sysconfig(SYS_CFGKMOD, &cmod, sizeof(cmod)) == -1) {
			return(1);
		}
	}

	/* pick up the NFS client extension */
	if ((ret = check_load("nfsclnt_kmid")) < 0) {
		return(1);
	}
	if (ret == 0) {
		/* Load the gfs into memory */
		cload.path = "/usr/lib/drivers/nfs_clnt.ext";
		if (sysconfig(SYS_KLOAD, &cload, sizeof(cload)) == -1) {
			return(1);
		}
		/* Initialize the gfs */
		cmod.kmid = cload.kmid;
		cmod.cmd = CFG_INIT;
		cmod.mdiptr = (caddr_t) &cmod.kmid;
		cmod.mdilen = sizeof(cmod.kmid);
		if (sysconfig(SYS_CFGKMOD, &cmod, sizeof(cmod)) == -1) {
			return(1);
		}
	}
	/* Everything went okay
	   */
	return(0);
}

mid_t
check_load(mod)
	char *mod;
{
struct nlist nl;
mid_t kmid;
int kmem;
int rc=0;

	nl.n_name = mod;
	if ((kmem=open("/dev/kmem", 0)) < 0) {
		return(-1);
	}
	if ((rc = knlist(&nl, 1, sizeof(struct nlist)))) {
		return(0);
	}

	if (nl.n_value == 0) {
		return(0);
	}
	if (lseek(kmem, nl.n_value, 0) == -1) {
		return(-1);
	}
	if (read(kmem, &kmid, sizeof(mid_t)) == -1) {
		return(-1);
	}
	return(kmid);
}
