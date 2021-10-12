#ifndef lint
static char sccsid[] = "@(#)51 1.4 src/bos/usr/lib/methods/chgpty/chgpty.c, cfgmethods, bos411, 9428A410j 6/2/94 07:28:25";
#endif
/*
 * COMPONENT_NAME: (CFGMETHODS) Change Method for streams based PTY
 *
 * FUNCTIONS: check_parms
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */


#include <stdio.h>
#include <string.h>
#include <cf.h>        /* Error codes */
#include <errno.h>

#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/mode.h>
#include <sys/types.h>

#include "cfgdebug.h"
#include "ttycfg.h"
#include "ptycfg.h"
#include "pparms.h"

/* Header file containing pty definition */
#include "spty.h"

/*
 * ==============================================================================
 * Defines and extern variables
 * ==============================================================================
 */

extern int Pflag;          /* Flag to change db only */

/*
 * =============================================================================
 *                       CHECK_PARMS
 * =============================================================================
 * 
 * This function checks the attributes to be changed for PTY.
 * Needed work is done for ATTNUM_ATT and BSDNUM_ATT attributes.
 * No more work is required and we don't want to go through the
 * unconfigure/configure cycle ===> Pflag is set to 1.
 * 
 * This function operates as a device dependent subroutine called 
 * by the generic change method for all devices. It is used to 
 * check the attributes to be changed for PTY.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */
int check_parms(attList,ppflag,tflag,logicalName,parent,location,badlist)
struct attr * attList;
int    ppflag, tflag;
char * logicalName;
char * parent;
char * location;
char * badlist;
{
    extern char allattrs[];    /* attribute string for attrval() */
    extern int attrval();      /* attribute values are in range ? */
    extern int sptydds();      /* PTY DDS build */
    extern int check_files();  /* Updating of PTY files */
    extern int verify_space(); /* Verify file system space */

    struct CuDv   cus_dev;     /* customized devices object storage */
    struct PdDv   pre_dev;     /* predefined devices object storage */

    mid_t  kmid;                      /* module id from loader */
	dev_t  clone_devno;               /* Used by sptydds and check_files */
    struct spty_dds * spty_dds_ptr;   /* DDS pointer */
    struct cfg_dd cfg;                /* sysconfig command structure */

    char    sstring[50];       /* search string */
    char *  badattr;           /* ptr to list of invalid attributes */
    int     return_code;       /* return code */

    /* ================================================= */
    /* Get the Customized Devices Object for this device */
    /* ================================================= */
    sprintf(sstring, "name = '%s'", logicalName);
    return_code = (int)odm_get_obj(CuDv_CLASS, sstring, &cus_dev, ODM_FIRST);
    switch (return_code) {
      case 0: /* odm objects not found */
        DEBUG_1 ("chgpty: CuDv %s found no objects.\n",sstring);
        err_exit(E_NOCuDv);
        break;  /* To avoid bug if err_exit is suppressed */

      case -1: /* odm error occurred */
        DEBUG_1 ("chgpty: get_obj failed, %s.\n", sstring);
        err_exit(E_ODMGET);
        break;  /* To avoid bug if err_exit is suppressed */

      default: /* odm object found ==> That's OK */
        DEBUG_1 ("chgpty: CuDv %s found objects.\n",sstring);
    } /* End switch (return_code) */

    /* ================================================================= */
    /* Get the predefined object for this device. This object is located */
    /* searching the predefined devices object class based on the unique */
    /* type link descriptor in the customized device. */
    /* ================================================================= */
    /* search Predefined devices for object with this unique type */
    sprintf(sstring, "uniquetype = '%s'", cus_dev.PdDvLn_Lvalue);
    return_code = (int)odm_get_obj(PdDv_CLASS, sstring, &pre_dev, ODM_FIRST);
    switch (return_code) {
      case 0: /* odm objects not found */
        DEBUG_1("chgpty: PdDv crit=%s found no objects.\n", sstring);
        err_exit(E_NOPdDv);
        break;  /* To avoid bug if err_exit is suppressed */

      case -1: /* odm error occurred */
        DEBUG_1("chgpty: get_obj failed, crit=%s.\n", sstring);
        err_exit(E_ODMGET);
        break;  /* To avoid bug if err_exit is suppressed */

      default:  /* odm object found ==> That's OK */
        DEBUG_1("chgpty: PdDv uniquetype = %s found objects.\n",
                 cus_dev.PdDvLn_Lvalue);
    } /* End switch (return_code) */

    /* Validate attributes with attrval() routine */
    DEBUG_2 ("chgpty: Calling attrval for %s with %s.\n",
             cus_dev.PdDvLn_Lvalue, allattrs);
    if (attrval(cus_dev.PdDvLn_Lvalue, allattrs, &badattr) > 0) {
        DEBUG_0 ("chgpty: Attrval failed.\n");
        strcpy(badlist, badattr);
        return(E_ATTRVAL);
    };
    DEBUG_0("chgpty: Attrval finished OK.\n");

    /* NULL first byte of allattrs, so generic change routine */
    /* won't attempt to validate attributes */
    allattrs[0] = '\0';

    /* ============================================================ */
    /* If ATTNUM_ATT or BSDNUM_ATT is changed, we do what is needed */
    /* and, in all cases, we set Pflag to 1. It means that */
    /* ODM databse will be updated but we don't go through */
    /* the unconfigure/configure cycle. */
    /* ============================================================ */
    if ((cus_dev.status != (short) DEFINED) &&
        (att_changed(attList, ATTNUM_ATT) || att_changed(attList, BSDNUM_ATT))) {
        DEBUG_2("check_parms: '%s' or '%s' attribute is found\n",
                ATTNUM_ATT, BSDNUM_ATT);
        /* ========================================= */
        /* Call loadext() to get device driver kmid. */
        /* ========================================= */
        if ((cfg.kmid = loadext(pre_dev.DvDr, FALSE, TRUE)) == NULL) {
            DEBUG_2("chgpty: driver %s is not found, errno = %d\n",
                    pre_dev.DvDr, errno);
            return(E_LOADEXT);
        };
        /* ======================== */
        /* Build DDS for pty device */
        /* ======================== */
        DEBUG_0("chgpty: Building dds\n");
        if (return_code = sptydds(&cus_dev, &spty_dds_ptr, pre_dev.DvDr, attList,
								  &clone_devno)) {
            DEBUG_0("chgpty: error during PTY DDS building\n");
            return(return_code);
        };

		/* ====================================================== */
        /* Before configuring the driver, we need to be sure that */
		/* there is enough space on file system to create needed  */
		/* inodes.                                                */
		/* ====================================================== */
		if (verify_space(spty_dds_ptr) > 0) {
			DEBUG_0("chgpty: There is enough file system space\n");
		}
		else
		{
			DEBUG_0("chgpty: There is not enough file system space\n");
			return (E_MKSPECIAL);
		}

        /* ============================================= */
        /* Call sysconfig() to configure the driver. */
        /* The BSD master entry point is used because */
        /* the BSD master major is generated with the */
        /* effective PTY driver name */
        /* I must be sure the PTY driver agree before updating */
        /* PTY special files */
        /* ============================================= */
        cfg.ddsptr = (caddr_t)spty_dds_ptr;
        cfg.ddslen = sizeof(*spty_dds_ptr);
        cfg.devno = spty_dds_ptr->ptyp_dev;
        cfg.cmd = CFG_INIT;
        if (sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd )) == -1) {
            /* error configuring driver */
            DEBUG_1("chgpty: error configuring driver, errno = %d\n", errno);
            return(E_CFGINIT);
        };

        /* ================================================ */
        /* Make the special files that this device will */
        /* be accessed through. */
        /* ================================================ */
        if ((return_code = check_files(logicalName, &cus_dev, spty_dds_ptr,
									   clone_devno)) != 0) {
            DEBUG_1("chgpty: error making special file for %s\n",
                    logicalName);
            return(return_code);
        };
    }
    else {
        DEBUG_2("check_parms: No '%s', neither '%s' attribute is found\n",
                ATTNUM_ATT, BSDNUM_ATT);
    }
    /* Make sure dbase change only flag is set */
    /* Be careful: This is an extern variable and it can be updated somewhere else */
    Pflag = 1;

    return(0);
}
