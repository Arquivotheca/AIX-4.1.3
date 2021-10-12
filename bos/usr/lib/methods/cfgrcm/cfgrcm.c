static char sccsid[] = "@(#)97	1.2  src/bos/usr/lib/methods/cfgrcm/cfgrcm.c, rcm, bos411, 9428A410j 2/28/94 13:30:03";
/*
 *   COMPONENT_NAME: RCM
 *
 *   FUNCTIONS: build_dds
 *		err_exit
 *		err_undo1
 *		err_undo2
 *		main
 *		make_special_files
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

/*
 * Include files needed for this module follow
 */
#include <sys/stat.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>

/*
 * device configuration include files
 */
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include "cfgdebug.h"

/*
 * Private include file for dds structure
 */
#include <rcmdds.h>

/* external functions */
extern int      pparse();
extern char *malloc();
extern int *genminor();
extern int mk_sp_file();


/*
 * Set permissions for special file
 */
#define FTYPE   S_IFMPX
#define FPERM   S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
#define MODE    FTYPE | FPERM

#define HFIOC   	('H'<<8)
#define RCMCOMMON	(HFIOC|17)

static rcm_dds   *dds;           /* pointer to dds structure */

/*
 * NAME: build_dds
 *
 * FUNCTION:
 *   build_dds will allocate memory for the dds structure, reporting any
 *   errors, then open the Customized Attribute Class to get the attribute
 *   objects needed for filling the dds structure.
 *
 * EXECUTION ENVIRONMENT:
 *   This function is called from the device independent module used
 *   for configuring all devices.
 *
 * RETURNS:
 *    0 on success
 *    positive return code on failure
 *
 */
build_dds(lname, dds_out, size)
char *lname;                         /* logical name of device */
char **dds_out;                      /* pointer to dds structure for return */
int  *size;                          /* pointer to dds structure size */
{

	struct PdAt     pdat;           /* predefined attribute object */
	struct PdAt     *pdat_ptr;      /* predefined attribute object */
	struct PdAt     *pdat_ptr2;     /* predefined attribute object */
	struct objlistinfo pdat_info;   /* result of search stats */
	struct CuDv     *cudv;          /* ptr to customized device object */
	struct objlistinfo cudv_info;   /* result of search stats */
	char	crit[50];		/* tmp string for search criteria */
	int fd, rc, i, x, index;	/* temp variables */
	int nbr_displays;

	/*
	 * The first thing to do is get the total number of displays
	 * that are available to the lft. Once we have this number we can then
         * allocate the memory for the dds structure.
	 */

	/* Get a list of adapter types belonging to the lft from the PdAt */
	pdat_ptr = pdat_ptr2 = (struct PdAt *)odm_get_list(PdAt_CLASS,
	         	"deflt = 'graphics' AND attribute = 'belongs_to'",
			&pdat_info,1,1);
	if ( pdat_ptr == (struct PdAt *) -1 )
	{
                DEBUG_0("build_dds : error in retrieving displays from PdAt\n");
                return(E_ODMGET);
	}

	/* loop thru each adapter type to get total number of displays */

	for ( i = 1, nbr_displays = 0; i <= pdat_info.num; i++, pdat_ptr++ ) 
	{
		/* get number of available displays for each display type */
		sprintf(crit,"PdDvLn = '%s' AND status = '%d'",
				pdat_ptr->uniquetype, AVAILABLE);
		cudv = (struct CuDv *)
				odm_get_list(CuDv_CLASS,crit,&cudv_info,1,1);
		if ( cudv == (struct CuDv *) -1 ) 
		{
                	DEBUG_0("build_dds : error getting list from CuDv\n");
			return(E_ODMGET);
	    	}
		/* add number of displays found to the total */
		nbr_displays += cudv_info.num;
	}

        /*
         * Obtain size of device specific dds structure. The lft_dds structure
         * already has an array of 1 display structures in it so we need
         * to add the number of displays found minus one display structure.
         */

        *size = sizeof(rcm_dds) + ((nbr_displays -1) * sizeof(dds->disp_info));

        /* Allocate the rcm_dds structure */

        if( (dds = (rcm_dds *) malloc( *size ) ) == NULL )
        {
                DEBUG_0("build_dds : malloc failed for dds structure");
                return(E_MALLOC);
        }

        /* zero out the dds */
        bzero( dds, *size);

        /*
         * Search the ODM for all available displays for the lft and store
         * all the corresponding information in the array of display structures.
         * Store the total number of displays found.
         */

        /* loop thru each adapter type for a list of available adapters */
        for ( i = 1, index = 0; i <= pdat_info.num; i++, pdat_ptr2++ )
        {
                /*
                 * Get a list of available adapter device names from the list
                 * of adapter types. The names come from the 'name' field of
                 * the CuDv class. (ie.. 'ppr0'). The 'status' field having a
                 * value of 1 means the given adapter is available.
                 */
                sprintf(crit,"PdDvLn = '%s' AND status = '%d'",
                                pdat_ptr2->uniquetype, AVAILABLE);
                cudv = (struct CuDv *)
                                odm_get_list(CuDv_CLASS,crit,&cudv_info,1,1);
                if ( cudv == (struct CuDv *) -1 )
                {
                        DEBUG_0("build_dds : error getting list from CuDv\n");
                        return(E_ODMGET);
                }

                /*
                 * loop thru list of available adapters, storing into the 
		 * dds display structure the device name and devno for each
		 * available display and clearing the access flag. 
                 */
                for (x = 1; x <= cudv_info.num; x++, cudv++, index++ )
		{
                        /* store devno and devname into display structure */
                        get_devno(cudv->name, &dds->disp_info[index].devno);
                        strcpy(dds->disp_info[index].lname, cudv->name);
                } /* for all available displays */

        } /* for all adapter types */

	dds->number_of_displays = index;  /* store number of displays found */

	/* need to get the devno of the lft to support ioctl calls to it */
	get_devno( "lft0", &dds->lft_info.devno );
	dds->lft_info.fp = NULL;

	/* zero out the count of rcm opens */
	dds->open_count = 0;

	*dds_out = (char *) dds;  /* return the new dds pointer */
        DEBUG_0("build_dds: OK\n");
        return(E_OK);
}



/*
 * NAME: make_special_files
 *
 * FUNCTION: Device dependent routine creating the devices special files
 *   in /dev
 *
 * EXECUTION ENVIRONMENT:
 *   This function is called from the main module.
 *
 * RETURNS:
 *   0 - success
 *   positive return code on failure
 */
int
make_special_files(lname, devno)
char  *lname;     /* logical device name */
dev_t devno;      /* major/minor number */
{
	if ( mk_sp_file(devno, lname, MODE) != 0)
	{
		DEBUG_0("RCM make_special_files failed\n");
		return(-1);
	}

	DEBUG_0("RCM make_special_files ok\n");
        return(E_OK);
}



/*
 * NAME: main
 *
 * FUNCTION:
 *
 *   The purpose of cfgrcm is to configure the RCM pseudo device into
 *   the system and make it ready for use.  It is called with the name
 *   of the logical device representing the RCM and possibly a flag
 *   representing which phase the configuration is taking place in.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 */
main(argc, argv)
int     argc;
char    *argv[];
{
struct cfg_dd cfg;              /* sysconfig command structure */

char    *logical_name;          /* logical name to configure */
char    sstring[256];           /* search criteria pointer */

struct CuDv cusobj;             /* customized device object storage */
struct PdDv preobj;             /* predefined device object storage */
struct CuDv parobj;             /* customized device object storage */

mid_t   kmid;                   /* module id from loader */
dev_t   devno;                  /* device number for config_dd */

long    majorno, minorno;       /* major and minor numbers */
long    *minor_list;            /* list returned by getminor */
long    *minor_ptr;             /* pointer returned by getminor */
int     how_many;               /* number of minors in list */
int     ipl_phase;              /* ipl phase: 0=run,1=phase1,2=phase2 */
int     rc;                     /* return codes go here */

extern  int     optind;         /* for getopt function */
extern  char    *optarg;        /* for getopt function */
int     errflg,c;               /* used in parsing parameters   */
int	fd;			/* File des. for /dev/lft */



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
			DEBUG_0("Setting PHASE2\n");
			ipl_phase = PHASE2;
			break;
		default:
			errflg++;
		}
	}
	if (errflg) {
		/* error parsing parameters */
		DEBUG_0("cfgrcm: command line error\n");
		exit(E_ARGS);
	}

	/*****                                                          */
	/***** Validate Parameters                                      */
	/*****                                                          */
	/* logical name must be specified */
	if (logical_name == NULL) {
		DEBUG_0("cfgrcm: logical name must be specified\n");
		exit(E_LNAME);
	}


        /*
         * Start up odm.
         */
        if ((rc = odm_initialize()) < 0)
        {
                DEBUG_0("cfgrcm: odm_initialize() failed\n");
                exit(E_ODMINIT);
        }

        /*
         * Search CusDevices for customized object with this logical name
         */

        sprintf(sstring, "name = '%s'", logical_name);
        if ((rc = (int)odm_get_first(CuDv_CLASS, sstring, &cusobj)) == 0)
        {
                DEBUG_1("cfgrcm: No CuDv object found for %s\n", logical_name);
                err_exit(E_NOCuDv);
        }
        else if (rc == -1)
        {
                DEBUG_1("cfgrcm: ODM failure getting CuDv, crit=%s\n", sstring);
                err_exit(E_ODMGET);
        }

        /*
         * Get the predefined object for this device. This object is located
         * searching the predefined devices object class based on the unique
         * type link descriptor in the customized device.
         */
        sprintf(sstring, "uniquetype = '%s'", cusobj.PdDvLn_Lvalue);
        if ((rc = (int)odm_get_first(PdDv_CLASS, sstring, &preobj)) == 0)
        {
                DEBUG_1("cfgrcm: found no PdDv object for crit=%s\n", sstring);
                err_exit(E_NOPdDv);
        }
        else if (rc == -1)
        {
                DEBUG_1("cfgrcm: ODM failure getting PdDv, crit=%s\n", sstring);
                err_exit(E_ODMGET);
        }

        /*
         * If this device is being configured during an ipl phase, then
         * display this device's LED value on the system LEDs.
         */
        if (ipl_phase != RUNTIME_CFG)
	{
		DEBUG_0("setting leds\n");
                setleds(preobj.led);
	}

        /*
         * Check to see if the device is already defined.
         * We actually go about the business of configuring the device
         * only if the device is defined. Configuring the device in this
         * case refers to the process of checking for attribute consistency,
         * building a DDS, loading the driver, etc...
         */
        if (cusobj.status == DEFINED)
        {
                if (cusobj.chgstatus == MISSING)
                {
                        DEBUG_1("cfgrcm: device %s is MISSING", logical_name);
                        err_exit(E_DEVSTATE);
                }

		/* Allocate and initialize the dds structure for the rcm */
                if ((rc = build_dds(logical_name, &cfg.ddsptr, &cfg.ddslen))
                                != E_OK)
                {
                        DEBUG_1("cfgrcm: error building dds, rc=%d\n", rc);
                        err_exit(rc);
                }


                /*
                * Call loadext() to load rcm driver.
                */
                if ((cfg.kmid = loadext(preobj.DvDr, TRUE, FALSE))
                                  == NULL)
                {
                        DEBUG_1("cfgrcm: error loading driver %s\n",
                                  preobj.DvDr);
                        err_exit(E_LOADEXT);
                }

                /*
                 * genmajor() will create a major number for this device
                 * if one has not been created yet. If one already exists,
                 * it will return that one.
                 */
                 if ((majorno = genmajor(preobj.DvDr)) == -1)
                 {
                        DEBUG_0("cfgrcm: error generating major number\n");
                        err_undo1(preobj.DvDr);
                        err_exit(E_MAJORNO);
                 }

                 /* Create minor number. */
                 minor_ptr = getminor(majorno, &how_many, logical_name);
                 if (minor_ptr == NULL || how_many == 0)
                 {
                        if((minor_ptr = genminor(logical_name, majorno,
                                         -1,1,1,1)) == NULL)
                        {
                                reldevno(logical_name, TRUE);
                                err_undo1(preobj.DvDr);
                                DEBUG_0("cfglft: error creating minor number\n")
                                err_exit(E_MINORNO);
                        }
                 }
                 minorno = *minor_ptr;

                 /* Create devno for this device */
                 cfg.devno = makedev(majorno, minorno);

                 /*
                  * Now call sysconfig() to configure the rcm.
                  */
                 cfg.cmd = CFG_INIT;
                 if (sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd ))
                                 == -1)
                 {
                         DEBUG_1("cfgrcm: error configuring driver of %s", logical_name);
                         err_undo1(preobj.DvDr);
                         err_exit(E_CFGINIT);
                 }

                 /*
                  * Now make the special files that this device will
                  * be accessed through.
                  */
                 if ((rc = make_special_files(logical_name, cfg.devno)) != E_OK)
                 {
                         DEBUG_2("cfgrcm: error making special file for %s, devno=0x%x",
                                         logical_name, cfg.devno);
                         err_undo2(cfg.devno);
                         err_undo1(preobj.DvDr);
                         err_exit(rc);
                 }


          } /* end if (device has a driver) then ... */

           /*
            * Update the customized object to reflect the device's
            * current status. The device status field should be
            * changed to AVAILABLE, and, if they were generated, the
            * device's major & minor numbers should be updated.
            */
           cusobj.status = AVAILABLE;
           if ((rc = odm_change_obj(CuDv_CLASS, &cusobj)) < 0)
           {
                  DEBUG_0("cfgrcm: ODM failure updating CuDv object\n");
                   err_exit(E_ODMUPDATE);
           }

        /*
         * Terminate the ODM and exit with success
         */
	odm_terminate();
        exit(E_OK);

}


/*
 * NAME: err_exit
 *
 * FUNCTION: Terminates ODM.  Used to back out on an error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.
 *
 * NOTES:
 *
 *   err_exit( exitcode )
 *      exitcode = The error exit code.
 *
 * RETURNS:
 *               None
 */

err_exit(exitcode)
char    exitcode;
{
        /*
         * Terminate the ODM 
         */
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
 *      This routine is to be used only within this file.
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
                DEBUG_0("cfgrcm: error unloading driver\n");
        }
        return;
}

/*
 * NAME: err_undo2
 *
 * FUNCTION: Terminates the device.  Used to back out on an error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.
 *
 * NOTES:
 *
 * void
 *   err_undo2( devno )
 *      devno = The device's devno.
 *
 * RETURNS:
 *               None
 */

err_undo2(devno)
dev_t   devno;                  /* The device's devno */
{
        struct  cfg_dd cfg;             /* sysconfig command structure */

	DEBUG_0("cfgrcm: in err_undo2, unconfiguring device\n");
        /* terminate device */
        cfg.devno = devno;
        cfg.kmid = (mid_t)0;
        cfg.ddsptr = (caddr_t) NULL;
        cfg.ddslen = (int)0;
        cfg.cmd = CFG_TERM;

/*        if (sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd)) == -1) {
                DEBUG_0("cfgrcm: error unconfiguring device\n");
        } */
        return;
}

