#ifndef lint
static char sccsid[] = "@(#)06 1.14 src/bos/usr/lib/methods/cfgasync/cfgasync.c, cfgtty, bos41J, 9520A_all 4/27/95 12:49:36";
#endif
/*
 * COMPONENT_NAME: (CFGTTY) Async Adapter Configure Method for streams based driver
 *
 * FUNCTIONS: main, err_exit, err_undo1, err_undo2
 *
 * ORIGINS: 27, 83
 *
 */
/*
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>          /* standard I/O */
#include <stdlib.h>         /* standard C library */
#include <sys/types.h>      /* standard typedef */
#include <cf.h>             /* error messages */
#include <errno.h>          /* standard error numbers */

#include <sys/cfgdb.h>      /* config #define */
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/cfgodm.h>     /* config structures */

#include <sys/str_tty.h>

#include "cfgdebug.h"
#include "ttycfg.h"

#define	TTYDBG_MODULE	"ttydbg"

/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */
#define BUS_CONNKEY         "mca"

/*
 * ==============================================================================
 * External functions declarations
 * ==============================================================================
 */
extern mid_t loadext();
/*
 * =============================================================================
 * Global variable for rsadapdds : trueslot (real slot number for STD I/O)
 * =============================================================================
 */
int trueslot;

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
 * Unloads the device's device driver.  Used to back out on
 * an error.
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
    /* unload driver */
    if (loadext(DvDr, FALSE, FALSE) == (mid_t)NULL) {
        DEBUG_0("cfgasync: error unloading driver\n");
    };
} /* End static void err_undo1(...) */

/*
 * -----------------------------------------------------------------------------
 *                       ERR_UNDO2
 * -----------------------------------------------------------------------------
 *
 * Terminates the device.  Used to back out on an error.
 *
 * This routine is to be used only within this file.  The device
 * specific routines for the various device specific config methods
 * must not call this function.
 *
 * Return code: None
 * -----------------------------------------------------------------------------
 */

static void err_undo2(DvDr, cfgPtr)
char    *DvDr;                  /* pointer to device driver name */
struct cfg_dd * cfgPtr;
{
    /* terminate device */
    cfgPtr->cmd = CFG_TERM;

    if (sysconfig(SYS_CFGDD, cfgPtr, sizeof(*cfgPtr)) == -1) {
        DEBUG_0("cfgasync: error unconfiguring device\n");
    };

    err_undo1(DvDr);
} /* End static void err_undo2(...) */

/*
 * -----------------------------------------------------------------------------
 *                       LOAD_CFGTTYDBG
 * -----------------------------------------------------------------------------
 *
 * loads the module ttydbg for dump and debug.
 *
 * Return code: 0 on succcess, E_LOADEXT or E_SYSCONFIG otherwise
 * -----------------------------------------------------------------------------
 */

load_ttydbg()
{
    struct  cfg_kmod cfg;
    int	error = 0;

    /* initialization */
    cfg.kmid = loadext(TTYDBG_MODULE,FALSE,TRUE);
    if (cfg.kmid == (mid_t)NULL) {			/* not yet loaded */
        if ((cfg.kmid = loadext(TTYDBG_MODULE,TRUE,TRUE)) == (mid_t)NULL) {
            DEBUG_2("load_ttydbg: loadext of %s failed with errno %d\n",
                     TTYDBG_MODULE, errno);
            error = E_LOADEXT;
        }
        else {
            /*****  SYS_CFGKMOD *****/

            cfg.mdiptr = (caddr_t)NULL;
            cfg.mdilen = 0;
            cfg.cmd = CFG_INIT;

            /* call sysconfig() to config the driver */
            if (sysconfig(SYS_CFGKMOD, &cfg, sizeof(cfg)) == CONF_FAIL) {
                DEBUG_2("load_ttydbg: sysconfig of %s failed with errno %d\n",
                         TTYDBG_MODULE, errno);
                err_undo1(TTYDBG_MODULE);
                error = E_SYSCONFIG;
            }
        }
    }

    return(error);

} /* End of load_ttydbg() function */

/*
 * =============================================================================
 *                       MAIN
 * =============================================================================
 * 
 * This process is executed to "configure" an adapter device in streams environment.
 * 
 * This process is invoked when an adapter device instance is to be 
 * "configured". The invokation will be made by a high level
 * configuration command or by the configuration manager at IPL.
 *
 * Interface: cfgasync -l <logical_name> <phase>
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */
main(argc, argv, envp)
     int    argc;
     char    *argv[];
     char    *envp[];
{
    extern int Get_Parent_Bus();      /* Searches up for bus */
    extern int chkslot();             /* cardid is in desired slot? */
    extern int rsadapdds();           /* build RS adapter DDS */
    extern int lionadapdds();         /* build LION adapter DDS */
    extern int define_children();     /* Define children */

    char * logical_name;              /* logical name to configure */
    char   sstring[256];              /* search criteria pointer */
    char   hw_vpd[VPDSIZE];           /* vpd for parent adapter */
    char   conf_list[1024];           /* busresolve() configured devices */
    char   not_res_list[1024];        /* busresolve() not resolved devices */

    long   majorno;                   /* major number */
    int    return_code;               /* return code */
    int    current_opt, option_error; /* used in parsing parameters */
    int    ipl_phase;                 /* ipl phase */
    int    slot;                      /* slot of adapter */
    ushort devid;                     /* device identifier */

    struct cfg_dd  cfg;               /* sysconfig command structure for dd */

    /* ODM structures declarations */
    struct Class * cus_dev_class;     /* customized devices class ptr */
    struct Class * pre_dev_class;     /* predefined devices class ptr */

    struct Class * cus_vpd_class;     /* customized vpd class ptr */

    struct CuDv     cus_dev;          /* customized device object storage */
    struct PdDv     pre_dev;          /* predefined device object storage */
    struct CuDv     par_cus_dev;      /* customized device object storage */

    struct CuVPD    vpd_dev;          /* customized vpd object storage */
    struct CuDv     dmy_dev;          /* customized device object storage */

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
            if (logical_name != NULL) {
                option_error++;
            };
            logical_name = optarg;
            break;
            
          case '1':
            if (ipl_phase != RUNTIME_CFG) {
                option_error++;
            };
            ipl_phase = PHASE1;
            break;
            
          case '2':
            if (ipl_phase != RUNTIME_CFG) {
                option_error++;
            };
            ipl_phase = PHASE2;
            break;
            
          default:
            option_error++;
        }
    }
    if (option_error) {
        /* error parsing parameters */
        DEBUG_0("cfgasync: command line error\n");
        exit(E_ARGS);
    };

    /* =========================================================== */
    /* Check command-line parameters for validity */
    /* For config methods this includes making sure that a logical */
    /* name was specified, and that both phase flags were not set. */
    /* The customized devices object class must also be searched */
    /* to see if the logical name is valid. */
    /* =========================================================== */
    if (logical_name == NULL) {
        DEBUG_0("cfgasync: no logical name\n");
        exit(E_LNAME);
    };

    /* start up odm */
    if (odm_initialize() == -1) {
        DEBUG_0("cfgasync: bad odm initialization\n");
        exit(E_ODMINIT);
    };

    /* open customized devices object class (CuDv) */
    if ((int)(cus_dev_class = odm_open_class(CuDv_CLASS)) == -1) {
        DEBUG_0("cfgasync: CuDv open class error\n");
        err_exit(E_ODMOPEN);
    };

    /* ================================================= */
    /* Get the Customized Devices Object for this device */
    /* ================================================= */
    sprintf(sstring, "name = '%s'", logical_name);
    return_code = (int)odm_get_obj(cus_dev_class, sstring, &cus_dev, ODM_FIRST);
    switch (return_code) {
      case 0: /* odm objects not found */
        DEBUG_1 ("cfgasync: CuDv %s found no objects.\n",sstring);
        err_exit (E_NOCuDv);
        break;  /* To avoid bug if err_exit is suppressed */
        
      case -1: /* odm error occurred */
        DEBUG_1 ("cfgasync: get_obj failed, %s.\n", sstring);
        err_exit (E_ODMGET);
        break;  /* To avoid bug if err_exit is suppressed */
        
      default: /* odm object found ==> That's OK */
        DEBUG_1 ("cfgasync: CuDv %s found objects.\n",sstring);
    } /* End switch (return_code) */

    /* ================================================================= */
    /* Get the predefined object for this device. This object is located */
    /* searching the predefined devices object class based on the unique */
    /* type link descriptor in the customized device. */
    /* ================================================================= */

    /* open predefined devices object class (PdDv) */
    if ((int)(pre_dev_class = odm_open_class(PdDv_CLASS)) == -1) {
        DEBUG_0("cfgasync: PdDv open class error\n");
        err_exit (E_ODMOPEN);
    };

    /* search Predefined devices for object with this unique type */
    sprintf(sstring, "uniquetype = '%s'", cus_dev.PdDvLn_Lvalue);
    return_code = (int)odm_get_obj(pre_dev_class, sstring, &pre_dev, ODM_FIRST);
    switch (return_code) {
      case 0: /* odm objects not found */
        DEBUG_1 ("cfgasync: PdDv crit=%s found no objects.\n",sstring);
        err_exit (E_NOPdDv);
        break;  /* To avoid bug if err_exit is suppressed */
        
      case -1: /* odm error occurred */
        DEBUG_1 ("cfgasync: get_obj failed, crit=%s.\n",sstring);
        err_exit (E_ODMGET);
        break;  /* To avoid bug if err_exit is suppressed */
        
      default:  /* odm object found ==> That's OK */
        DEBUG_1 ("cfgasync: PdDv uniquetype = %s found objects.\n",
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
        /* The device is not available to the system yet. Now */
        /* check to make sure that the device's relations will */
        /* allow it to be configured. In particular, make sure */
        /* that the parent is configured (AVAILABLE), and that */
        /* no other devices are configured at the same location. */
        /* Get the device's parent object */
        /* ==================================================== */        
        sprintf(sstring, "name = '%s'", cus_dev.parent);
        return_code = (int)odm_get_obj(cus_dev_class,sstring,&par_cus_dev,ODM_FIRST);
        switch (return_code) {
          case 0: /* odm objects not found */
            DEBUG_1 ("cfgasync: CuDv %s found no objects.\n",
                     sstring);
            err_exit (E_NOCuDvPARENT);
            break;  /* To avoid bug if err_exit is suppressed */
            
          case -1: /* odm error occurred */
            DEBUG_1 ("cfgasync: get_obj failed, %s.\n",
                     sstring);
            err_exit (E_ODMGET);
            break;  /* To avoid bug if err_exit is suppressed */

            default : /* odm object found ==> That's OK */
            DEBUG_1 ("cfgasync: CuDv %s found objects.\n",
                     sstring);
        } /* End switch (return_code) */
        if (par_cus_dev.status != (short) AVAILABLE) {
            DEBUG_1 ("cfgasync: parent %s is not AVAILABLE\n",
                            cus_dev.parent);
            err_exit (E_PARENTSTATE);
        };
	/* trueslot = SIO slot for config native I/O
	 * used by rsadapdds
	*/
        trueslot = (atoi(par_cus_dev.connwhere) -1) & 0x0f;
        
        /* Make sure that no devices are config'd at this location */
        sprintf(sstring,
                "parent = '%s' AND connwhere = '%s' AND status = %d",
                cus_dev.parent, cus_dev.connwhere, AVAILABLE);
        return_code = (int)odm_get_obj(cus_dev_class, sstring, &dmy_dev, ODM_FIRST);
        switch (return_code) {
          case 0: /* odm objects not found ==> That's OK */
            DEBUG_1 ("cfgasync: No CuDv %s configured at this location.\n",
                     sstring);
            break;

          case -1: /* odm error occurred */
            DEBUG_1 ("cfgasync: get_obj %s failed\n", sstring);
            err_exit (E_ODMGET);
            break;  /* To avoid bug if err_exit is suppressed */

          default: /* Error: device configured at this location */
            DEBUG_1("cfgasync: %s : Device is configured at this location\n",
                    sstring);
            err_exit (E_AVAILCONNECT);
        } /* End switch (return_code) */

        /* ================================================== */
        /* If the device is an adapter being configured at */
        /* RUN TIME, then we must resolve any bus attribute */
        /* conflicts before configuring device to the driver. */
        /* ================================================== */
        if (ipl_phase == RUNTIME_CFG) {
            /* Make sure card is in specified slot */
            /* This test is done only if the adapter is directly */
            /* connected on bus. Its subclass must match */
            /* bus connkey */
            if (!strcmp(pre_dev.subclass, BUS_CONNKEY)) {
                slot = atoi(cus_dev.connwhere);
                devid = (ushort) strtol(pre_dev.devid, (char **)NULL, 0);
                sprintf(sstring,"/dev/%s", cus_dev.parent);
                return_code = chkslot(sstring, slot, devid);
                if (return_code != 0) {
                    DEBUG_2("cfgasync: card %s not found in slot %d\n",
                            logical_name, slot);
                    err_exit(return_code);
                };
            };
            /* Get the bus that this device is connected to */
            return_code = Get_Parent_Bus(cus_dev_class, cus_dev.parent, &dmy_dev);
            switch (return_code) {
              case 0:
                /* Invoke Bus Resolve  */
                return_code = busresolve(logical_name, (int)0, conf_list,
                                         not_res_list, dmy_dev.name);
                if (return_code != 0) {
                    DEBUG_0("cfgasync: bus resources could not be resolved\n");
                    err_exit(return_code);
                }
                else {
                    DEBUG_1("cfgasync: busresolve OK on bus '%s'\n", dmy_dev.name);
                }
                break;
                
              case E_PARENT:
                DEBUG_1("cfgasync: No bus found in the hierarchy above %s\n",
                        logical_name);
                err_exit(return_code);
                break;
                
              default:
                DEBUG_0("cfgasync: ODM error getting parent bus\n");
                err_exit(return_code);
            } /* End switch (return_code) */
        }; /* End if (ipl_phase == RUNTIME_CFG) */

	return_code = load_ttydbg();
	if (return_code) {
		DEBUG_1("load_ttydbg failed with %d\n", return_code);
	}
        
        /* ===================================================== */
        /* Load and configuration of the driver for this adapter */
        /* No minor is generated for adapter. Only major */
        /* is used by sysconfig routine */
        /* ===================================================== */
        /* An associated driver is needed */
        if (!strcmp(pre_dev.DvDr, "")) {
            DEBUG_0("cfgasync: No associated driver is found\n");
            err_exit(E_LNAME);
        };
        
        /* CFG.KMID */
        DEBUG_1("cfgasync: Trying to load %s driver\n", pre_dev.DvDr);
        if ((cfg.kmid = loadext(pre_dev.DvDr, TRUE, FALSE)) == NULL) {
            /* error loading device driver */
            DEBUG_1("cfgasync: loadext failed with errno = %d\n", errno);
            err_exit(E_LOADEXT);
        };
        if ((majorno = genmajor(pre_dev.DvDr)) == (long) -1) {
            /* error allocating major number */
            err_undo1 (pre_dev.DvDr);
            err_exit(E_MAJORNO);
        };
        
        /* CFG.DEVNO */
        /* minor is meaningless for adapters */
        cfg.devno = makedev(majorno, 0);
        DEBUG_1("cfgasync: devno = %x\n", cfg.devno);

        /* CFG.DDSPTR and CFG.DDSLEN */
        /* Build the DDS */
        if (!strcmp(pre_dev.DvDr, RS_DRIVER)) {
            DEBUG_0("cfgasync: Trying to build DDS\n");
            if (return_code = rsadapdds(&cus_dev, &cfg.ddsptr, &cfg.ddslen)) {
                /* error building dds */
                DEBUG_1("cfgasync: error building dds for %s\n", logical_name);
                err_undo1(pre_dev.DvDr);
                err_exit (return_code);
            };
        }
        else if (!strcmp(pre_dev.DvDr, LION_DRIVER)) {
            DEBUG_0("cfgasync: Trying to build DDS\n");
            if (return_code = lionadapdds(&cus_dev, &cfg.ddsptr, &cfg.ddslen)) {
                /* error building dds */
                DEBUG_1("cfgasync: error building dds for %s\n", logical_name);
                err_undo1(pre_dev.DvDr);
                err_exit (return_code);
            };
        }
        else {
            /* Unsupported driver */
            DEBUG_1("cfgasync: driver %s isn't supported for this configure method.\n",
                    pre_dev.DvDr);
            err_undo1(pre_dev.DvDr);
            err_exit(E_INVATTR);
        }

        /* CFG.CMD */
        cfg.cmd = CFG_INIT;

        DEBUG_1("cfgasync: Trying to configure %s driver\n", pre_dev.DvDr);
        DEBUG_1("cfgasync: cfg.kmid = 0x%x\n", cfg.kmid);
        DEBUG_1("cfgasync: cfg.devno = 0x%x\n", cfg.devno);
        DEBUG_1("cfgasync: cfg.cmd = 0x%x\n", cfg.cmd);
        DEBUG_1("cfgasync: cfg.ddsptr = 0x%x\n", cfg.ddsptr);
        DEBUG_1("cfgasync: cfg.ddslen = 0x%x\n", cfg.ddslen);
        if (sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd)) == -1) {
            /* error configuring driver */
            DEBUG_1("cfgasync: errno = %d",errno);
            err_undo1(pre_dev.DvDr);
            err_exit(E_CFGINIT);
        };
        DEBUG_0("cfgasync: Initialize Complete\n");

        /* ======================================================= */
        /* chk_lion                                                */
        /* This call is used to correct some bugs in 64 ports card.*/
        /* ========================================================*/
        if (!strcmp(pre_dev.DvDr, LION_DRIVER)) {
            if (return_code = chk_lion(&cus_dev, cfg.kmid, cfg.devno)) {
                /* failed check  */
                DEBUG_1("cfgasync: chk_lion error for %s",
                         cus_dev.name);
                err_undo2(pre_dev.DvDr, &cfg);
                err_exit(return_code);
            };
	};


        /* ======================================================= */
        /* get the vpd data                                       */
        /* ========================================================*/
    DEBUG_0("cfgasync: entering  query_vpd()\n")
        if (pre_dev.has_vpd == TRUE) {
            memset(hw_vpd, 0, sizeof(hw_vpd));
            if (return_code = query_vpd(&cus_dev, hw_vpd)) {
                /* failed to get VPD */
                DEBUG_1("cfgasync: error getting VPD for %s",
                         cus_dev.name);
                err_undo2(pre_dev.DvDr, &cfg);
                err_exit(return_code);
            };
    DEBUG_0("cfgasync: Returned from query_vpd()\n")

            /* open customized vpd object class */
            if ((int)(cus_vpd_class = odm_open_class(CuVPD_CLASS)) == -1) {
                err_undo2(pre_dev.DvDr, &cfg);
                err_exit(E_ODMOPEN);
            };

            sprintf(sstring,"name = '%s' AND vpd_type = %d", cus_dev.name, HW_VPD);
            return_code = (int)odm_get_obj(cus_vpd_class, sstring, &vpd_dev, ODM_FIRST);
            switch (return_code) {
              case 0: /* odm objects not found */
                DEBUG_1("cfgasync: CuVPD %s found no objects.\n",sstring);
                /* Need to add vpd object */
                DEBUG_0("cfgasync: Adding new VPD object\n");
                strcpy(vpd_dev.name, logical_name);
                vpd_dev.vpd_type = HW_VPD;
                memcpy(vpd_dev.vpd, hw_vpd, sizeof(hw_vpd));
                if (odm_add_obj(cus_vpd_class, &vpd_dev) == -1) {
                    DEBUG_1("cfgasync: ODM failure adding CuVPD object: %s\n",
                            vpd_dev.name);
                    /* No return code is tested for this odm_close_class */
                    /* because the first error is the interesting one */
                    odm_close_class(CuVPD_CLASS);
                    err_undo2(pre_dev.DvDr, &cfg);
                    err_exit(E_ODMADD);
                };
                DEBUG_1("cfgasync: Successfully added new VPD object: %s\n",
                        vpd_dev.name);
                break;  /* To avoid bug if err_exit is suppressed */
                
              case -1: /* odm error occurred */
                DEBUG_1("cfgasync: get_obj failed, CuVPD %s.\n",sstring);
                /* No return code is tested for this odm_close_class */
                /* because the first error is the interesting one */
                odm_close_class(CuVPD_CLASS);
                err_undo2(pre_dev.DvDr, &cfg);
                err_exit(E_ODMGET);
                break;  /* To avoid bug if err_exit is suppressed */
                
              default: /*odm objects found */
                DEBUG_1("cfgasync: CuVPD %s found objects.\n",sstring);
                /* If vpd object needs to be updated */
                if (memcmp(hw_vpd, vpd_dev.vpd ,sizeof(hw_vpd))) {
                    /* Store Hardware VPD in object */
                    memcpy (vpd_dev.vpd, hw_vpd, sizeof(hw_vpd));
                    if ((odm_change_obj(cus_vpd_class, &vpd_dev)) < 0) {
                        DEBUG_1("cfgasync: update CuVPD for %s failed", vpd_dev.name);
                        /* No return code is tested for this odm_close_class */
                        /* because the first error is the interesting one */
                        odm_close_class(CuVPD_CLASS);
                        err_undo2(pre_dev.DvDr, &cfg);
                        err_exit(E_ODMUPDATE);
                    };
                };
            } /* End switch (return_code) */
            
            if (odm_close_class(cus_vpd_class) < 0) {
                err_undo2(pre_dev.DvDr, &cfg);
                err_exit(E_ODMCLOSE);
            };
        }; /* End if (pre_dev.has_vpd == TRUE) */
        /* ====================================================== */
        /* Update the customized object to reflect the device's */
        /* current status. The device status field should be */
        /* changed to AVAILABLE. */
        /* ====================================================== */
        cus_dev.status = (short) AVAILABLE;
        if (cus_dev.chgstatus == MISSING) {
            cus_dev.chgstatus = SAME ;
        };
        if ((return_code = odm_change_obj(cus_dev_class, &cus_dev)) < 0) {
            /* change object failed */
            DEBUG_1 ("cfgasync: update %s failed.\n", logical_name);
            err_exit (E_ODMUPDATE);
        };
    }; /* End if (cus_dev.status == (short) DEFINED) */

    /* call device specific routine to detect/manage child devices */
    if (define_children(logical_name, ipl_phase)) {
        /* error defining children */
        DEBUG_0("cfgasync: error defining children\n");
        err_exit(E_FINDCHILD);
    }
    DEBUG_0("cfgasync: Returned from define_children()\n")

    /* close object classes */
    if (odm_close_class(pre_dev_class) < 0) {
        err_exit (E_ODMCLOSE);
    };
    if (odm_close_class(cus_dev_class) < 0) {
        err_exit (E_ODMCLOSE);
    };

    /* ====================================================== */
    /* Config method is finished at this point. Terminate the */
    /* ODM, and exit with a good return code. */
    /* ====================================================== */
    odm_terminate();
    return(E_OK);

} /* End main(...) */
