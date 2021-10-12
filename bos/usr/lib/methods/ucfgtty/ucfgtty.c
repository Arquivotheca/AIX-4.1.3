#ifndef lint
static char sccsid[] = "@(#)63 1.18 src/bos/usr/lib/methods/ucfgtty/ucfgtty.c, cfgtty, bos41J, 9512A_all 3/20/95 19:57:46";
#endif
/*
 * COMPONENT_NAME: (CFGMETHODS) Uconfigure Method for streams based tty
 *
 * FUNCTIONS: main, err_exit
 *
 * ORIGINS: 27, 83
 *
 */
/*
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>
#include <cf.h>
#include <errno.h>
#include <sys/sysmacros.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/device.h>
#include <sys/sysconfig.h> /* includes sys/types.h */

#include <sys/str_tty.h>   /* for DDS identifiers */


#include "cfgdebug.h"
#include "ttycfg.h"

/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */

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
 * This process is executed to "unconfigure" a tty device in streams environment.
 * 
 * This process is invoked when a tty device instance is to be 
 * "unconfigured".
 * This involves terminating the device to the 
 * driver and representing the unconfigured state of the device 
 * in the database. It may also unload the appropriate device 
 * driver if the device is the last configured device for the driver.
 *
 * Interface: cfgtty -l <logical_name>
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */
main(argc,argv)
int argc;
char *argv[];
{
    extern int    errno;             /* System Error Number */
    extern char * optarg;            /* argument parsed by getopt() */
    extern long * getminor();
    extern long   genmajor();
    extern mid_t  loadext();

    char * logical_name;             /* logical name to unconfigure */
    char   sstring[256];             /* search criteria */

    int    return_code;               /* return codes go here */
    int    current_opt, option_error; /* used in parsing parameters */
    int    found_minor;               /* number of minor numbers returned */
    long * minorptr;                  /* pointer to minor number "list" */
    long   majorno, minorno;          /* major and minor numbers */
    enum   dds_type which_dds;        /* used to unconfigure driver */
    
    struct cfg_dd      cfg;           /* sysconfig command structure */

    /* ODM structures declarations */
    struct Class * cus_dev_class;     /* customized devices class handle */
    struct Class * pre_dev_class;     /* predefined devices class handle */

    struct PdDv    pre_dev;           /* predefined devices object */
    struct CuDv    cus_dev;           /* cus devices object for device */
    struct PdDv    par_pre_dev;       /* predefined devices object */
    struct CuDv    par_cus_dev;       /* cus devices object for parent */

    /* ============================== */
    /* set up initial variable values */
    /* ============================== */
    option_error = 0;
    logical_name = NULL;
    found_minor = 0;
    
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
        DEBUG_0("ucfgtty: command line error\n");
        exit(E_ARGS);
    };

    /* =========================================================== */
    /* Check command-line parameters for validity */
    /* =========================================================== */
    if (logical_name == NULL) {
        DEBUG_0("ucfgtty: no logical name\n");
        exit(E_LNAME);
    };
    
    /* start up odm */
    if (odm_initialize() == -1) {
        DEBUG_0("ucfgtty: bad odm initialization\n");
        exit(E_ODMINIT);
    };

    /* open customized devices object class (CuDv) */
    if ((int)(cus_dev_class = odm_open_class(CuDv_CLASS)) == -1) {
        DEBUG_0("ucfgtty: CuDv open class error\n");
        err_exit(E_ODMOPEN);
    };
    
    /* ================================================= */
    /* Get the Customized Devices Object for this device and its parent */
    /* ================================================= */
    sprintf(sstring, "name = '%s'", logical_name);
    return_code = (int)odm_get_obj(cus_dev_class, sstring, &cus_dev, ODM_FIRST);
    switch (return_code) {
      case 0: /* odm objects not found */
        DEBUG_1("ucfgtty: CuDv %s found no objects.\n",sstring);
        err_exit(E_NOCuDv);
        break;  /* To avoid bug if err_exit is suppressed */
        
      case -1: /* odm error occurred */
        DEBUG_1("ucfgtty: get_obj failed, %s.\n", sstring);
        err_exit(E_ODMGET);
        break;  /* To avoid bug if err_exit is suppressed */
        
      default: /* odm object found ==> That's OK */
        DEBUG_1("ucfgtty: CuDv %s found objects.\n",sstring);
    } /* End switch (return_code) */
    
    /* Get parent customized device object */
    sprintf(sstring, "name = '%s'", cus_dev.parent);
    return_code = (int)odm_get_obj(cus_dev_class, sstring, &par_cus_dev,ODM_FIRST);
    switch (return_code) {
      case 0: /* odm objects not found */
        DEBUG_1("ucfgtty: CuDv %s found no objects.\n",
                 sstring);
        err_exit(E_NOCuDvPARENT);
        break;  /* To avoid bug if err_exit is suppressed */
        
      case -1: /* odm error occurred */
        DEBUG_1("ucfgtty: get_obj failed, %s.\n",
                 sstring);
        err_exit(E_ODMGET);
        break;  /* To avoid bug if err_exit is suppressed */
        
        default : /* odm object found ==> That's OK */
          DEBUG_1("ucfgtty: CuDv %s found objects.\n",
                   sstring);
    } /* End switch (return_code) */
    
    /* ===================================================== */
    /* Make sure device is configured (AVAILABLE or STOPPED) */
    /* ===================================================== */
    if (cus_dev.status == (short) DEFINED) {
        /* Device is not configured */
        DEBUG_1("ucfgtty: parent %s is notconfigured\n",
                 cus_dev.parent);
        err_exit(E_DEVSTATE);
    };
    
    /* ================================================================= */
    /* Get the predefined object for this device. This object is located */
    /* searching the predefined devices object class based on the unique */
    /* type link descriptor in the customized device. */
    /* ================================================================= */
    
    /* open predefined devices object class (PdDv) */
    if ((int)(pre_dev_class = odm_open_class(PdDv_CLASS)) == -1) {
        DEBUG_0("ucfgtty: PdDv open class error\n");
        err_exit(E_ODMOPEN);
    };
    
    /* search Predefined devices for object with this unique type */
    sprintf(sstring, "uniquetype = '%s'", cus_dev.PdDvLn_Lvalue);
    return_code = (int)odm_get_obj(pre_dev_class, sstring, &pre_dev, ODM_FIRST);
    switch (return_code) {
      case 0: /* odm objects not found */
        DEBUG_1("ucfgtty: PdDv crit=%s found no objects.\n",sstring);
        err_exit(E_NOPdDv);
        break;  /* To avoid bug if err_exit is suppressed */
        
      case -1: /* odm error occurred */
        DEBUG_1("ucfgtty: get_obj failed, crit=%s.\n",sstring);
        err_exit(E_ODMGET);
        break;  /* To avoid bug if err_exit is suppressed */
        
      default:  /* odm object found ==> That's OK */
        DEBUG_1("ucfgtty: PdDv uniquetype = %s found objects.\n",
                 cus_dev.PdDvLn_Lvalue);
    } /* End switch (return_code) */
    
    /* =================================================== */
    /* Get the driver for the tty from the parent(adapter) */
    /* predefined object class. */
    /* =================================================== */
    /* Get the parent's predefined device object */
    sprintf(sstring, "uniquetype = '%s'", par_cus_dev.PdDvLn_Lvalue);
    return_code = (int)odm_get_obj(pre_dev_class,sstring, &par_pre_dev, ODM_FIRST);
    switch (return_code) {
      case 0: /* odm objects not found */
        DEBUG_1("ucfgtty: PdDv %s found no objects.\n",
                sstring);
        err_exit(E_NOPdOBJ);
        break;  /* To avoid bug if err_exit is suppressed */
        
      case -1 : /* odm error occurred */
        DEBUG_1("ucfgtty: get_obj failed, %s.\n",
                sstring);
        err_exit(E_ODMGET);
        break;  /* To avoid bug if err_exit is suppressed */
        
      default: /* odm objects found ==> That's OK */
        DEBUG_1("ucfgtty: PdDv %s found.\n", sstring);
    }
    if (!strcmp(par_pre_dev.DvDr, "")) {
        DEBUG_1("ucfgtty: No driver found for %s\n",
                par_pre_dev.uniquetype);
        err_exit(E_INVATTR);
    };
    
    /* =========================================== */
    /* Call sysconfig() to "terminate" the device. */
    /* If fails, device instance still "open", and */
    /* device cannot be "unconfigured". */
    /* =========================================== */
    /* To retrieve devno for this device. If no major is found, */
    /* a new one is created but getminor will return no minor */
    if ((majorno = genmajor(par_pre_dev.DvDr)) == (long) -1) {
        DEBUG_1("ucfgtty: genmajor fails for driver %s\n",
                par_pre_dev.DvDr);
        err_exit(E_MAJORNO);
    };

    if (((minorptr = getminor(majorno, &found_minor, logical_name))
         == (long *) NULL) || (found_minor == 0)) {
        DEBUG_2("ucfgtty: No minor found for driver %s and major %ld\n",
                par_pre_dev.DvDr, majorno);
        err_exit(E_MINORNO);
    };
    /* For debug, I mention if more than one minor is found */
    /* In all case, only the first is used */
    if (found_minor != 1) {
        DEBUG_2("ucfgtty: More than minor has been found for driver %s and major %ld\n",
                par_pre_dev.DvDr, majorno);
    };
    minorno = *minorptr;

    cfg.devno = makedev(majorno,minorno);
    
    /* Trying to get the kmid associated with the found driver */
    if ((cfg.kmid = loadext(par_pre_dev.DvDr, FALSE, TRUE)) == (mid_t)NULL) {
        DEBUG_1("ucfgtty: Unable to get kmid for %s driver\n",
                 par_pre_dev.DvDr);
        err_exit(E_LOADEXT);
    };
    
    /* Unconfiguration of the driver */
    /* Only DDS identifier is set */
    if (!strcmp(par_pre_dev.DvDr, RS_DRIVER)) {
        which_dds = RS_LINE_DDS;
    }
    else if (!strcmp(par_pre_dev.DvDr, LION_DRIVER)) {
        which_dds = LION_LINE_DDS;
    }
    else if (!strcmp(par_pre_dev.DvDr, CXMA_DRIVER)) {
        which_dds = CXMA_LINE_DDS;
    }
    else
	which_dds = 0;

    cfg.ddsptr = (caddr_t) &which_dds;
    cfg.ddslen = sizeof(which_dds);
    cfg.cmd = CFG_TERM;    

    DEBUG_1("ucfgtty: majorno = %ld\n",majorno);
    DEBUG_1("ucfgtty: minorno = %ld\n",minorno);
    DEBUG_1("ucfgtty: cfg.devno = 0x%x\n",cfg.devno);
    DEBUG_1("ucfgtty: cfg.kmid = 0x%x\n",cfg.kmid);
    DEBUG_1("ucfgtty: cfg.ddsptr = 0x%x\n",cfg.ddsptr);
    DEBUG_1("ucfgtty: cfg.ddslen = 0x%x\n",cfg.ddslen);

    if (sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd)) == -1) {
        DEBUG_1("ucfgtty: error unconfiguring driver, errno = %d\n",
                            errno);
        if (errno == EBUSY) {
            /* device is in use and can not be unconfigured */
            DEBUG_1("ucfgtty: sysconfig fails because %s is busy\n",
                    par_pre_dev.DvDr);
            err_exit(E_BUSY);
        };
        /* Ignore other errno values because device driver */
        /* has to complete CFG_TERM operation except when  */
        /* device is busy. */
    }

    /* =========================================== */
    /* Suppress the 'autopush' of modules for this */
    /* line */
    /* It must be done after the driver is */
    /* unconfigured because it knows if the line */
    /* is used or not */
    /* =========================================== */
    if (return_code = pop_modules(majorno, minorno)) {
        DEBUG_2("ucfgtty: Unable to suppress the 'autopush' for maj %ld min %ld\n",
                majorno, minorno);
        err_exit(return_code);
    };

    DEBUG_0("ucfgtty: Device Terminated\n");
    
    /* ============================================== */
    /* Release the minor number for the tty device so */
    /* that another device can be configured at this */
    /* location and the tty being unconfigured can be */
    /* configured at a different location. */
    /* ============================================== */
    if ((reldevno(logical_name, FALSE)) == -1) {
        DEBUG_1("ucfgtty: Unable to release minor and major number for %s\n",
                logical_name);
        err_exit(E_RELDEVNO);
    };
    
    /* ============================================== */
    /* Change the status field of device to "DEFINED" */
    /* ============================================== */
    DEBUG_0("ucfgtty: Updating device state\n");
    cus_dev.status = (short)DEFINED;
    
    /* Update the Customized Object of the device */
    if ((return_code = odm_change_obj(cus_dev_class, &cus_dev)) < 0) {
        /* change object failed */
        DEBUG_1("ucfgtty: update %s failed.\n", logical_name);
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
