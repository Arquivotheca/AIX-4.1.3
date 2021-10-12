#ifndef lint
static char sccsid[] = "@(#)69 1.2 src/bos/usr/lib/methods/ucfgpty/ucfgpty.c, cfgmethods, bos411, 9428A410j 7/6/94 11:51:11";
#endif

/*
 * COMPONENT_NAME: (CFGMETHODS) Unconfigure Method for pty device
 *
 * FUNCTIONS: main, err_exit
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>
#include <stdlib.h>         /* standard C library */
#include <sys/types.h>
#include <errno.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/sysconfig.h>
#include <sys/sysmacros.h>
#include <sys/device.h>

#include <sys/str_tty.h>

#include "cfgdebug.h"
#include "ttycfg.h"
#include "ptycfg.h"

/* Header file containing pty definition */
#include "spty.h"

/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */

/*
 * ==============================================================================
 * External functions declarations
 * ==============================================================================
 */
extern mid_t loadext();

/*
 * =============================================================================
 *                       LOCAL UTILITY ROUTINES
 * =============================================================================
 */
/*
 * -----------------------------------------------------------------------------
 *                       ERR_EXIT
 * -----------------------------------------------------------------------------
 * 
 * Closes any open object classes and terminates ODM.
 * Used to back out on an error.
 *
 * This routine is to be used only within this file.
 * The device specific routines for the various device
 * specific config methods  must not call this function.
 *
 * Return code: None
 * -----------------------------------------------------------------------------
 */
static void err_exit(exitcode)
int    exitcode;
{
    /* Close any open object class */
    odm_close_class(CuDv_CLASS);
    odm_close_class(PdDv_CLASS);
    odm_close_class(CuAt_CLASS);
    odm_close_class(PdAt_CLASS);

    /* Terminate the ODM */
    odm_terminate();
    exit(exitcode);
} /* End static void err_exit(...) */

/*
 * =============================================================================
 *                       MAIN
 * =============================================================================
 * 
 *    This process is executed to "unconfigure" an adapter device in streams environment.
 * 
 *    This process is invoked when PTY device instance is to be 
 *    "unconfigured". This involves terminating the device to the 
 *    driver and representing the unconfigured state of the device 
 *    in the database. It may also unload the appropriate device driver. 
 *
 * Interface: ucfgpty -l <logical_name>
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */
main(argc, argv, envp)
     int    argc;
     char    *argv[];
     char    *envp[];
{
    extern int sptydds();

    char * logical_name;              /* logical name to configure */
    char   sstring[256];              /* search criteria pointer */
    int    return_code, how_many;     /* return code and counter */
    int    current_opt, option_error; /* used in parsing parameters */

	dev_t  clone_devno;               /* devno of clone driver */
	struct spty_dds * spty_dds_ptr;   /* DDS pointer */
    struct cfg_dd  cfg;               /* sysconfig command structure for dd */

    /* ODM structures declarations */
    struct Class * cus_dev_class;     /* customized devices class ptr */
    struct Class * pre_dev_class;     /* predefined devices class ptr */

    struct CuDv    cus_dev;        /* customized adapter device object */
    struct PdDv    pre_dev;        /* predefined device object storage */

    /* ============================== */
    /* set up initial variable values */
    /* ============================== */
    option_error = 0;
    logical_name = NULL;

    /* ================ */
    /* Parse Parameters */
    /* ================ */
    while ((current_opt = getopt(argc,argv,"l:")) != EOF) {
        switch (current_opt) {
          case 'l':
            if (logical_name != NULL) {
                option_error++;
            };
            logical_name = optarg;
            break;

          default:
            option_error++;
        }
    }
    if (option_error) {
        /* error parsing parameters */
        DEBUG_0("ucfgpty: command line error\n");
        exit(E_ARGS);
    };

    /* ========================================================= */
    /* Check command-line parameters for validity */
    /* For unconfig methods this includes making sure that a */
    /*logical name was specified. */
    /* The customized devices object class must also be searched */
    /* to see if the logical name is valid. */
    /* ========================================================= */
    if (logical_name == NULL) {
        DEBUG_0("ucfgpty: no logical name\n");
        exit(E_LNAME);
    };

    /* start up odm */
    if (odm_initialize() == -1) {
        DEBUG_0("ucfgpty: bad odm initialization\n");
        exit(E_ODMINIT);
    };

    /* open customized devices object class (CuDv) */
    if ((int)(cus_dev_class = odm_open_class(CuDv_CLASS)) == -1) {
        DEBUG_0("ucfgpty: CuDv open class error\n");
        err_exit(E_ODMOPEN);
    };

    /* ================================================= */
    /* Get the Customized Devices Object for this device */
    /* ================================================= */
    sprintf(sstring, "name = '%s'", logical_name);
    return_code = (int)odm_get_obj(cus_dev_class, sstring, &cus_dev, ODM_FIRST);
    switch (return_code) {
      case 0: /* odm objects not found */
        DEBUG_1 ("ucfgpty: CuDv %s found no objects.\n",sstring);
        err_exit (E_NOCuDv);
        break;  /* To avoid bug if err_exit is suppressed */

      case -1: /* odm error occurred */
        DEBUG_1 ("ucfgpty: get_obj failed, %s.\n", sstring);
        err_exit (E_ODMGET);
        break;  /* To avoid bug if err_exit is suppressed */

      default: /* odm object found ==> That's OK */
        DEBUG_1 ("ucfgpty: CuDv %s found objects.\n",sstring);
    } /* End switch (return_code) */

    /* ===================================================== */
    /* If device state is DEFINED, then already unconfigured */
    /* so just exit. */
    /* ===================================================== */
    if (cus_dev.status == (short) DEFINED) {
        DEBUG_0("ucfgpty: Device is already in DEFINED state\n");
        err_exit(E_OK);
    };

    /* ================================================================= */
    /* Get the predefined object for this device. This object is located */
    /* searching the predefined devices object class based on the unique */
    /* type link descriptor in the customized device. */
    /* ================================================================= */

    /* open predefined devices object class (PdDv) */
    if ((int)(pre_dev_class = odm_open_class(PdDv_CLASS)) == -1) {
        DEBUG_0("ucfgpty: PdDv open class error\n");
        err_exit (E_ODMOPEN);
    };

    /* search Predefined devices for object with this unique type */
    sprintf(sstring, "uniquetype = '%s'", cus_dev.PdDvLn_Lvalue);
    return_code = (int)odm_get_first(pre_dev_class, sstring, &pre_dev);
    switch (return_code) {
      case 0: /* odm objects not found */
        DEBUG_1 ("ucfgpty: PdDv crit=%s found no objects.\n",sstring);
        err_exit (E_NOPdDv);
        break;  /* To avoid bug if err_exit is suppressed */

      case -1: /* odm error occurred */
        DEBUG_1 ("ucfgpty: get_obj failed, crit=%s.\n",sstring);
        err_exit (E_ODMGET);
        break;  /* To avoid bug if err_exit is suppressed */

      default:  /* odm object found ==> That's OK */
        DEBUG_1 ("ucfgpty: PdDv uniquetype = %s found objects.\n",
                 cus_dev.PdDvLn_Lvalue);
    } /* End switch (return_code) */

    /* ================================= */
    /* Unconfigure the associated driver */
    /* ================================= */
    /* An associated driver is needed */
    if (!strcmp(pre_dev.DvDr, "")) {
        DEBUG_0("ucfgpty: No associated driver is found\n");
        err_exit(E_LNAME);
    };

    /* CFG.KMID */
    if ((cfg.kmid = loadext(pre_dev.DvDr, FALSE, TRUE)) == (mid_t) NULL) {
        DEBUG_1("ucfgpty: Unable to get cfg.kmid for %s\n",
                pre_dev.DvDr);
        err_exit(E_UNLOADEXT);
    } /* End if ((cfg.kmid = loadext(...)) == (mid_t) NULL) */


    /* CFG.CMD */
    cfg.cmd = CFG_TERM;

    /* Build the DDS */
	if (return_code = sptydds(&cus_dev, &spty_dds_ptr, pre_dev.DvDr, NULL,
							  &clone_devno)) {
		DEBUG_0("ucfgpty: error during PTY DDS building\n");
		err_exit(return_code);
	}
	else {
		cfg.ddsptr = (caddr_t)spty_dds_ptr;
        cfg.ddslen = sizeof(*spty_dds_ptr);
        cfg.devno = spty_dds_ptr->ptyp_dev;
	}
	

    DEBUG_1("ucfgpty: Trying to unconfigure %s driver\n", pre_dev.DvDr);
    DEBUG_1("ucfgpty: cfg.kmid = 0x%x\n", cfg.kmid);
    DEBUG_1("ucfgpty: cfg.devno = 0x%x\n", cfg.devno);
    DEBUG_1("ucfgpty: cfg.cmd = 0x%x\n", cfg.cmd);
    DEBUG_1("ucfgpty: cfg.ddsptr = 0x%x\n", cfg.ddsptr);
    DEBUG_1("ucfgpty: cfg.ddslen = 0x%x\n", cfg.ddslen);
    if (sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd)) == -1) {
        /* error configuring driver */
        DEBUG_1("ucfgpty: error configuring driver, errno = %d\n",
				errno);
		if (errno == EBUSY) {
			/* device is in use and can not be unconfigured */
			DEBUG_1("ucfgtty: sysconfig fails because %s is busy\n",
					pre_dev.DvDr);
			err_exit(E_BUSY);
		};
    };
    DEBUG_0("ucfgpty: Driver unconfiguration completed\n");

    /* =========================================== */
    /* Suppress the 'autopush' of modules for each major */
    /* It must be done after the driver is */
    /* unconfigured because it knows if lines */
    /* are used or not */
    /* =========================================== */
	/* ATT master side */
	/* Minor is set to 0 because we want to undo for all minors */
	if (pop_modules(major(spty_dds_ptr->ptc_dev), 0)) {
		DEBUG_1("cfgpty: modules pop failed for major %d\n",
				major(spty_dds_ptr->ptc_dev));
	};
	/* ATT slave side */
	/* Minor is set to 0 because we want to undo for all minors */
	if (pop_modules(major(spty_dds_ptr->pts_dev), 0)) {
		DEBUG_1("cfgpty: modules pop failed for major %d\n",
				major(spty_dds_ptr->pts_dev));
	};
	/* BSD master side */
	/* Minor is set to 0 because we want to undo for all minors */
	if (pop_modules(major(spty_dds_ptr->ptyp_dev), 0)) {
		DEBUG_1("cfgpty: modules pop failed for major %d\n",
				major(spty_dds_ptr->ptyp_dev));
	};
	/* BSD slave side */
	/* Minor is set to 0 because we want to undo for all minors */
	if (pop_modules(major(spty_dds_ptr->ttyp_dev), 0)) {
		DEBUG_1("cfgpty: modules pop failed for major %d\n",
				major(spty_dds_ptr->ttyp_dev));
	};
	
	/* ===================================================== */
	/* Call loadext() one more time to unload LDTERM_MODULE  */
	/* We need to do that because the "pre_dev.DvDr" driver  */
	/* uses LDTERM_MODULE symbols. So LDTERM_MODULE has been */
	/* loaded one more time BEFORE "pre_dev.DvDr".           */
	/* (see corresponding configuration method)              */
	/* ===================================================== */
    if (loadext(LDTERM_MODULE, FALSE, FALSE) == (mid_t) NULL) {
        DEBUG_1("ucfgpty: error unloading %s module\n",
				LDTERM_MODULE);
        err_exit(E_UNLOADEXT);
    };
    DEBUG_0("ucfgpty: Modules unconfiguration completed\n");
	
	/* ========================================= */
    /* Call loadext() to unload device driver. */
    /* loadext() will unload the driver only if */
    /* device is last instance of driver configured. */
    /* ========================================= */
    DEBUG_1("ucfgpty: Unloading the driver: %s\n", pre_dev.DvDr);
    /* Test if driver is really loaded */
    if (loadext(pre_dev.DvDr, FALSE, FALSE) == (mid_t) NULL) {
        /* error unloading device driver */
        DEBUG_0("ucfgpty: error unloading driver\n");
        err_exit(E_UNLOADEXT);
    };
    DEBUG_0("ucfgpty: Unloaded driver\n");
	
    /* ============================================== */
    /* Change the status field of device to "DEFINED" */
    /* ============================================== */
    DEBUG_0("ucfgpty: Updating device state\n");
    cus_dev.status = (short)DEFINED;

    /* Update the Customized Object of the device */
    if ((return_code = odm_change_obj(cus_dev_class, &cus_dev)) < 0) {
        /* change object failed */
        DEBUG_1("ucfgpty: update %s failed.\n", logical_name);
        err_exit(E_ODMUPDATE);
    };

    /* close object classes */
    if (odm_close_class(pre_dev_class) < 0) {
        err_exit(E_ODMCLOSE);
    };
    if (odm_close_class(cus_dev_class) < 0) {
        err_exit(E_ODMCLOSE);
    };

    /* ======================================================== */
    /* Unconfig method is finished at this point. Terminate the */
    /* ODM, and exit with a good return code. */
    /* ======================================================== */
    odm_terminate();
    return(E_OK);

} /* End main(...) */
