#ifndef lint
static char sccsid[] = "@(#)10 1.25 src/bos/usr/lib/methods/cfgpty/cfgpty.c, cfgmethods, bos411, 9428A410j 4/8/94 07:49:41";
#endif
/*
 * COMPONENT_NAME: (CFGMETHODS) - Config Method Code for pty devices
 *
 * FUNCTIONS: main, err_exit, err_undo1
 *
 * ORIGINS: 27, 83
 *
 */
/*
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>
#include <cf.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/mode.h>
#include <sys/types.h>

#include "cfgdebug.h"
#include "pparms.h"
#include "ttycfg.h"

/* Header file containing pty definition */
#include "spty.h"

/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */

/* defines for autopush function, slave part */
#define SLAVE_MODULES       LDTERM_MODULE "," TIOC_MODULE

static char masterModules[] = TIOC_MODULE;
static char slaveModules[16];

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
 * -----------------------------------------------------------------------------
 *                       ERR_UNDO1
 * -----------------------------------------------------------------------------
 *
 * Unloads the device's device driver or module.  Used to back out on an
 * error.
 *
 * This routine is to be used only within this file.  The device
 * specific routines for the various device specific config methods
 * must not call this function.
 *
 * Return code: None
 * -----------------------------------------------------------------------------
 */

static void err_undo1(DvDr)
char    *DvDr;                  /* pointer to device driver name */
{
    DEBUG_1("cfgpty: Error %s is going to be unloaded\n", DvDr);
    if (loadext(DvDr, FALSE, FALSE) == NULL) {
        DEBUG_0("cfgpty: error unloading driver or module\n");
    };
} /* End static void err_undo1(...) */

/*
 * =============================================================================
 *                       MAIN
 * =============================================================================
 * 
 * This process is executed to "configure" a pty device in streams environment.
 *
 * This process is invoked when a pty device instance is to be 
 * "configured". The invokation will be made by a high level
 * configuration command or by the configuration manager at IPL.
 *
 * Interface: cfgpty -l <logical_name> <phase>
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */

main(argc, argv, envp)
int    argc;
char    *argv[];
char    *envp[];
{
    extern char * optarg;             /* for getopt function */
    extern int push_modules();        /* similar action as autopush command */
    extern int pop_modules();         /* reverse action as autopush command */
    extern int sptydds();             /* PTY DDS building */
    extern int check_files();         /* Update needed files */
 
    char * logical_name;              /* logical name to configure */
    char   sstring[256];              /* search criteria pointer */

    mid_t  kmid;                      /* module id from loader */
    dev_t  clone_devno;               /* devno of clone driver */
    struct spty_dds * spty_dds_ptr;   /* DDS pointer */

    int    ipl_phase;                 /* ipl phase: 0=run,1=phase1,2=phase2 */
    int    current_opt, option_error; /* used in parsing parameters */
    int    return_code;               /* return codes go here */
        
    struct cfg_dd cfg;                /* sysconfig command structure */

   /* ODM structures declarations */
    struct Class * cus_dev_class;     /* customized devices class ptr */
    struct Class * pre_dev_class;     /* predefined devices class ptr */
        
    struct CuDv    cus_dev;           /* customized device object storage */
    struct PdDv    pre_dev;           /* predefined device object storage */

    /* ============================== */
    /* set up initial variable values */
    /* ============================== */
    ipl_phase = RUNTIME_CFG;
    option_error = 0;
    logical_name = NULL;

    /* ================ */
    /* Parse Parameters */
    /* ================ */
    while ((current_opt = getopt(argc,argv,"l:12")) != EOF) {
        switch (current_opt) {
        case 'l':
            if (logical_name != NULL)
                option_error++;
            logical_name = optarg;
            break;
        case '1':
            if (ipl_phase != RUNTIME_CFG)
                option_error++;
            ipl_phase = PHASE1;
            break;
        case '2':
            if (ipl_phase != RUNTIME_CFG)
                option_error++;
            ipl_phase = PHASE2;
            break;
        default:
            option_error++;
        }
    }
    if (option_error) {
        /* error parsing parameters */
        DEBUG_0("cfgpty: command line error\n");
        exit(E_ARGS);
    };

    /* logical name must be specified */
    if (logical_name == NULL) {
        DEBUG_0("cfgpty: no logical name\n");
        exit(E_LNAME);
    };

    /* Start up odm */
    if (odm_initialize() == -1) {
        DEBUG_0("cfgpty: bad odm initialization\n");
        exit(E_ODMINIT);
    };

    /* Open customized devices object class (CuDv) */
    if ((int)(cus_dev_class = odm_open_class(CuDv_CLASS)) == -1) {
        DEBUG_0("cfgpty: CuDv open class error\n");
        err_exit(E_ODMOPEN);
    };

    /* ================================================= */
    /* Get the Customized Devices Object for this device */
    /* ================================================= */
    sprintf(sstring, "name = '%s'", logical_name);
    return_code = (int)odm_get_obj(cus_dev_class, sstring, &cus_dev, ODM_FIRST);
    switch (return_code) {
      case 0: /* odm objects not found */
        DEBUG_1("cfgpty: CuDv %s found no objects.\n", sstring);
        err_exit(E_NOCuDv);
        break;  /* To avoid bug if err_exit is suppressed */
        
      case -1: /* odm error occurred */
        DEBUG_1("cfgpty: get_obj failed, %s.\n", sstring);
        err_exit(E_ODMGET);
        break;  /* To avoid bug if err_exit is suppressed */
        
      default: /* odm object found ==> That's OK */
        DEBUG_1("cfgpty: CuDv %s found objects.\n", sstring);
    } /* End switch (return_code) */

    /* ================================================================= */
    /* Get the predefined object for this device. This object is located */
    /* searching the predefined devices object class based on the unique */
    /* type link descriptor in the customized device. */
    /* ================================================================= */

    /* Open predefined devices object class (PdDv) */
    if ((int)(pre_dev_class = odm_open_class(PdDv_CLASS)) == -1) {
        DEBUG_0("cfgpty: PdDv open class error\n");
        err_exit(E_ODMOPEN);
    };

    /* search Predefined devices for object with this unique type */
    sprintf(sstring, "uniquetype = '%s'", cus_dev.PdDvLn_Lvalue);
    return_code = (int)odm_get_obj(pre_dev_class, sstring, &pre_dev, ODM_FIRST);
    switch (return_code) {
      case 0: /* odm objects not found */
        DEBUG_1("cfgpty: PdDv crit=%s found no objects.\n", sstring);
        err_exit(E_NOPdDv);
        break;  /* To avoid bug if err_exit is suppressed */
        
      case -1: /* odm error occurred */
        DEBUG_1("cfgpty: get_obj failed, crit=%s.\n", sstring);
        err_exit(E_ODMGET);
        break;  /* To avoid bug if err_exit is suppressed */
        
      default:  /* odm object found ==> That's OK */
        DEBUG_1("cfgpty: PdDv uniquetype = %s found objects.\n",
                 cus_dev.PdDvLn_Lvalue);
    } /* End switch (return_code) */

    /* ============================================================ */
    /* If this device is being configured during an ipl phase, then */
    /* display this device's LED value on the system LEDs. */
    /* ============================================================ */
    if (ipl_phase != RUNTIME_CFG) {
        /* display LED value */
        setleds(pre_dev.led);
    };

    /* ============================================================ */    
    /* Check to see if the device is already configured. */
    /*  We actually go about the business of configuring the device */
    /*  only if the device is not configured yet i.e. DEFINED. */
    /* ============================================================ */    
    if (cus_dev.status == (short) DEFINED) {

        /* ==================================================== */
        /* Call loadext() to load LDTERM_MODULE                 */
        /* We need to do that because the "pre_dev.DvDr" driver */
        /* uses LDTERM_MODULE symbols. So LDTERM_MODULE must be */
        /* loaded BEFORE "pre_dev.DvDr".                        */
        /* Really needed load and configuration are done        */
        /* further within the 'push_modules' function.          */
        /* ==================================================== */
        if ((cfg.kmid = loadext(LDTERM_MODULE, TRUE, FALSE)) == NULL) {
            DEBUG_2("cfgpty: error loading %s module, errno = %d\n",
                    LDTERM_MODULE, errno);
            err_exit(E_LOADEXT);
        };

        /* ===================================== */
        /* Call loadext() to load device driver. */
        /* ===================================== */
        if ((cfg.kmid = loadext(pre_dev.DvDr, TRUE, FALSE)) == NULL) {
            DEBUG_2("cfgpty: error loading driver %s, errno = %d\n",
                    pre_dev.DvDr, errno);
            err_undo1(LDTERM_MODULE);
            err_exit(E_LOADEXT);
        };

        /* ======================== */
        /* Build DDS for pty device */
        /* ======================== */
        DEBUG_0("cfgpty: Building dds\n");
        if (return_code = sptydds(&cus_dev, &spty_dds_ptr, pre_dev.DvDr, NULL,
                                  &clone_devno)) {
            DEBUG_0("cfgpty: error during PTY DDS building\n");
            err_undo1(LDTERM_MODULE);
            err_undo1(pre_dev.DvDr);
            err_exit(return_code);
        };

        /* ================================================ */
        /* Make the special files that this device will */
        /* be accessed through. */
        /* ================================================ */
        if ((return_code = check_files(logical_name, &cus_dev, spty_dds_ptr, clone_devno)) != 0) {
            DEBUG_1("cfgpty: error making special file for %s\n",
                    logical_name);
            err_undo1(LDTERM_MODULE);
            err_undo1(pre_dev.DvDr);
            err_exit(return_code);
        };

        /* ============================================= */
        /* Call sysconfig() to configure the driver. */
        /* The BSD master entry point is used because */
        /* the BSD master major is generated with the */
        /* effective PTY driver name */
        /* ============================================= */
        cfg.ddsptr = (caddr_t)spty_dds_ptr;
        cfg.ddslen = sizeof(*spty_dds_ptr);
        cfg.devno = spty_dds_ptr->ptyp_dev;
        cfg.cmd = CFG_INIT;
        if (sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd )) == -1) {
            /* error configuring driver */
            DEBUG_1("cfgpty: error configuring driver, errno = %d\n", errno);
            err_undo1(LDTERM_MODULE);
            err_undo1(pre_dev.DvDr);
            err_exit(E_CFGINIT);
        };

        /* ==================================================== */
        /* Autopush and configuration of useful streams modules */
        /* ==================================================== */
        /* Modules are pushed for any minors per major */
        /* --------------- */
        /* ATT master side */
        /* --------------- */
        DEBUG_1("cfgpty: Trying to push streams modules for ATT master side: %s\n",
                masterModules);
        if (return_code = push_modules(&cus_dev, masterModules,
                                       (long) major(spty_dds_ptr->ptc_dev), -1)) {
            DEBUG_0("cfgpty: Push of streams modules for ATT master failed\n");
            err_undo1(LDTERM_MODULE);
            err_undo1(pre_dev.DvDr);
            err_exit(return_code);
        };
        DEBUG_0("cfgpty: Push of streams modules for ATT master side succeeded\n");
        
        /* --------------- */
        /* BSD master side */
        /* --------------- */
        DEBUG_1("cfgpty: Trying to push streams modules for BSD master side: %s\n",
                masterModules);
        if (return_code = push_modules(&cus_dev, masterModules,
                                       (long) major(spty_dds_ptr->ptyp_dev), -1)) {
            DEBUG_0("cfgpty: Push of streams modules for BSD master failed\n");
            /* Pop all modules which have already been pushed */
            /* ATT master side */
            /* Minor is set to 0 because we want to undo for all minors */
            if (pop_modules(major(spty_dds_ptr->ptc_dev), 0)) {
                DEBUG_1("cfgpty: modules pop failed for major %d\n",
                        major(spty_dds_ptr->ptc_dev));
            };
            err_undo1(LDTERM_MODULE);
            err_undo1(pre_dev.DvDr);
            err_exit(return_code);
        };
        DEBUG_0("cfgpty: Push of streams modules for BSD master side succeeded\n");
        
        /* -------------- */
        /* ATT slave side */
        /* -------------- */
        DEBUG_1("cfgpty: Trying to push streams modules for ATT slave side: %s\n",
                SLAVE_MODULES);
        strcpy(slaveModules, SLAVE_MODULES);
        if (return_code = push_modules(&cus_dev, slaveModules,
                                       (long) major(spty_dds_ptr->pts_dev), -1)) {
            DEBUG_0("cfgpty: Push of streams modules for ATT slave failed\n");
            /* Pop all modules which have already been pushed */
            /* ATT master side */
            /* Minor is set to 0 because we want to undo for all minors */
            if (pop_modules(major(spty_dds_ptr->ptc_dev), 0)) {
                DEBUG_1("cfgpty: modules pop failed for major %d\n",
                        major(spty_dds_ptr->ptc_dev));
            };
            /* BSD master side */
            /* Minor is set to 0 because we want to undo for all minors */
            if (pop_modules(major(spty_dds_ptr->ptyp_dev), 0)) {
                DEBUG_1("cfgpty: modules pop failed for major %d\n",
                        major(spty_dds_ptr->ptyp_dev));
            };
            err_undo1(LDTERM_MODULE);
            err_undo1(pre_dev.DvDr);
            err_exit(return_code);
        };
        DEBUG_0("cfgpty: Push of streams modules for ATT slave side succeeded\n");
        
        /* -------------- */
        /* BSD slave side */
        /* -------------- */
        DEBUG_1("cfgpty: Trying to push streams modules for BSD slave side: %s\n",
                SLAVE_MODULES);
        strcpy(slaveModules, SLAVE_MODULES);
        if (return_code = push_modules(&cus_dev, slaveModules,
                                       (long) major(spty_dds_ptr->ttyp_dev), -1)) {
            DEBUG_0("cfgpty: Push of streams modules for BSD slave failed\n");
            /* Pop all modules which have already been pushed */
            /* ATT slave side */
            /* Minor is set to 0 because we want to undo for all minors */
            if (pop_modules(major(spty_dds_ptr->pts_dev), 0)) {
                DEBUG_1("cfgpty: modules pop failed for major %d\n",
                        major(spty_dds_ptr->pts_dev));
            };
            /* ATT master side */
            /* Minor is set to 0 because we want to undo for all minors */
            if (pop_modules(major(spty_dds_ptr->ptc_dev), 0)) {
                DEBUG_1("cfgpty: modules pop failed for major %d\n",
                        major(spty_dds_ptr->ptc_dev));
            };
            /* BSD master side */
            /* Minor is set to 0 because we want to undo for all minors */
            if (pop_modules(major(spty_dds_ptr->ptyp_dev), 0)) {
                DEBUG_1("cfgpty: modules pop failed for major %d\n",
                        major(spty_dds_ptr->ptyp_dev));
            };
            err_undo1(LDTERM_MODULE);
            err_undo1(pre_dev.DvDr);
            err_exit(return_code);
        };
        DEBUG_0("cfgpty: Push of streams modules for BSD slave side succeeded\n");
        
        
        
        /* ====================================================== */
        /* Update the customized object to reflect the device's */
        /* current status. The device status field should be */
        /* changed to AVAILABLE. */
        /* ====================================================== */
        cus_dev.status = AVAILABLE;
        if ((return_code = odm_change_obj(cus_dev_class, &cus_dev)) < 0) {
            DEBUG_1("cfgpty: update %s failed\n", logical_name);
            err_exit(E_ODMUPDATE);
        };

    }; /* End if (cus_dev.status == (short) DEFINED) */

    /* Close ODM object classes */
    if (odm_close_class(cus_dev_class) < 0) {
        DEBUG_0("cfgpty: close object class CuDv failed\n");
        err_exit(E_ODMCLOSE);
    };
    if (odm_close_class(pre_dev_class) < 0) {
        DEBUG_0("cfgpty: close object class PdDv failed\n");
        err_exit(E_ODMCLOSE);
    };

    /* ====================================================== */
    /* Config method is finished at this point. Terminate the */
    /* ODM, and exit with a good return code. */
    /* ====================================================== */
    odm_terminate();
    return(E_OK);
} /* End main(...) */
