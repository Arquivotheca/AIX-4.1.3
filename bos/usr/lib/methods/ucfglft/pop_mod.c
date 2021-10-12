static char sccsid[] = "@(#)69	1.2  src/bos/usr/lib/methods/ucfglft/pop_mod.c, lftdd, bos411, 9428A410j 12/7/93 10:28:25";
/*
 * COMPONENT_NAME: (LFTDD)	LFT unconfiguration routine
 *
 * FUNCTIONS:
 *
 *	pop_modules, unconf_modules
 *
 * ORIGIN: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>          /* standard I/O (usr/include) */
#include <cf.h>             /* error messages (usr/include) */
#include <errno.h>          /* standard error numbers (kernel/sys) */
#include <fcntl.h>          /* logical file system #define (usr/include) */

#include <sys/mode.h>
#include <sys/cfgdb.h>      /* config #define (kernel/sys) */
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/cfgodm.h>     /* config structures (???) */
#include <termios.h>
#include <sys/termiox.h>

#include <sys/stropts.h>    /* Streams #define */
#include <sys/sad.h>        /* sad driver declarations */
#include <sys/tty.h>
#include <sys/str_tty.h>

#include "ttycfg.h"
#include "cfgdebug.h"
#include <lftras.h>

/*
 * ===========================================================================
 * defines and strutures
 * ===========================================================================
 */
#define LOCAL               static

#define ADMINDEV            "/dev/sad"
#define SEPARATORS          " \t,"
#define END_OF_STRING       '\0'

/*
 * ===========================================================================
 * External functions declarations
 * ===========================================================================
 */
extern mid_t loadext();

/*
 * ---------------------------------------------------------------------------
 *                       UNCONF_MODULES
 * ---------------------------------------------------------------------------
 *
 * Unconfigures and unloads (only at last unload) modules.
 * Unused entries of the entry parameter MUST be
 * initialized to NULL.
 *
 * This routine is to be used only within this file.
 *
 * Return code: Exits with 0 on success, errno error code otherwise.
 * When an error occurs, the last found error code is returned.
 * ---------------------------------------------------------------------------
 */
LOCAL int unconf_modules(strapushPtr)
struct strapush * strapushPtr;
{
    int    module_nb;                /* module number */
    int    return_code;
    enum   dds_type which_dds;       /* dds identifier */
    struct cfg_kmod cfg;             /* structure for module unconfiguration */

    /* ============================== */
    /* set up initial variable values */
    /* ============================== */
    module_nb = 0;
    return_code = 0;
    
    /* While we have available names */
    while ((module_nb < strapushPtr->sap_npush) &&
           strcmp(strapushPtr->sap_list[module_nb], "")) {
        /* Set up cfg fields */
        /* mdiptr and mdilen may be updated if the DDS */
        /* is unknown */
        cfg.mdiptr = (caddr_t)&which_dds;
	cfg.mdilen = sizeof(which_dds);
        cfg.cmd = CFG_TERM;
        if ((cfg.kmid = loadext(strapushPtr->sap_list[module_nb], FALSE, TRUE))
            == (mid_t) NULL) {
            DEBUG_1("pop_mod: %s module isn't found as kernel extension.\n",
                    strapushPtr->sap_list[module_nb]);
            cfg_lfterr(NULL,"UCFGLFT","pop_mod","load_ext",NULL,
                                UCFG_LOADEXT, UNIQUE_1);

            return_code = E_LOADEXT;
        }
        else {
            /* Which module is it ? */
            if (!strcmp(strapushPtr->sap_list[module_nb], LDTERM_MODULE)) {
                which_dds = LDTERM_DDS;
            }
            else if (!strcmp(strapushPtr->sap_list[module_nb], TIOC_MODULE)) {
                which_dds = TIOC_DDS;
            }
            else {
                DEBUG_1("pop_mod: Unknown DDS for %s module\n",
                        strapushPtr->sap_list[module_nb]);
		cfg_lfterr(NULL,"UCFGLFT","pop_mod",NULL,NULL,
                                UCFG_UNKNOWN_MOD, UNIQUE_2);
                /* No DDS is sent */
                cfg.mdiptr = (caddr_t) NULL;
                cfg.mdilen = 0;
            }/* End if (!strcmp(...)) */
            
            /* Performs the sysconfig */
            if (sysconfig(SYS_CFGKMOD, &cfg, sizeof(cfg)) < 0) {
                DEBUG_1("pop_mod: Unconfiguration of %s failed\n",
                        strapushPtr->sap_list[module_nb]);
		cfg_lfterr(NULL,"UCFGLFT","pop_mod","sysconfig",errno,
                                UCFG_SYSCONFIG, UNIQUE_3);
                return_code = errno;
            };

            /* Performs the loadext */
            /* The kernel unloads effectively only when the */
            /* load count of the object file becomes zero */
            if (loadext(strapushPtr->sap_list[module_nb], FALSE, FALSE)
                == (mid_t) NULL) {
                DEBUG_1("pop_mod: Unload of %s failed\n",
                        strapushPtr->sap_list[module_nb]);
		cfg_lfterr(NULL,"UCFGLFT","pop_mod","loadext",NULL,
                                UCFG_LOADEXT, UNIQUE_4);
                return_code = E_UNLOADEXT;
            };
        } /* End if ((cfg.kmid = loadext(...)) == (mid_t) NULL) */

        module_nb++;
    } /* End while (...) */

    return(return_code);
} /* End LOCAL int unconf_modules(...) */

/*
 * ---------------------------------------------------------------------------
 *                       POP_MODULES
 * ---------------------------------------------------------------------------
 * 
 * Opens SAD to pop modules.
 * We get pushed modules, pop and unconfigure them.
 *
 * Return code: Exits with 0 on success, errno error code otherwise.
 * ---------------------------------------------------------------------------
 */
int pop_modules(majorNb, minorNb)
long   majorNb, minorNb;          /* major and minor numbers */
{
    /* For sad driver access */
    int             file_desc;       /* File descriptor */
    int             return_code;
    struct strapush sad_info;        /* list of modules to push */
    
    /* ============================== */
    /* set up initial variable values */
    /* ============================== */
    bzero((char *)&sad_info, sizeof(struct strapush));
    sad_info.sap_major = majorNb;
    sad_info.sap_minor = minorNb;

    /* ================ */
    /* Opens sad device */
    /* ================ */
    if ((file_desc = stream_open(ADMINDEV, O_RDWR)) == -1) {
        DEBUG_1("pop_mod: stream_open error with %s\n", ADMINDEV);
        return(EACCES);
    };

    /* ================== */
    /* get pushed modules */
    /* ================== */
    if ((return_code = stream_ioctl(file_desc, SAD_GAP, &sad_info)) < 0) {
        DEBUG_0("pop_mod: stream_ioctl with parameters below failed\n");
        DEBUG_1("           sap_major = 0x%x\n", sad_info.sap_major);
        DEBUG_1("           sap_minor = 0x%x\n", sad_info.sap_minor);
        switch (errno) {
          case EFAULT:
            DEBUG_0("pop_mod: Bad argument for stream_ioctl\n");
            break;
          case EINVAL:
            DEBUG_0("pop_mod: Invalid major\n");
            break;
          case ENOSTR:
            DEBUG_0("pop_mod: This major is not a streams device\n");
            break;
          case ENODEV:
            DEBUG_0("pop_mod: This device is not configured for autopush\n");
            break;
          default:
            DEBUG_1("pop_mod: Unexpected stream_ioctl SAD_GAP; errno = %d\n",
                    errno);
        } /* End switch (errno) */
        stream_close(file_desc);
        return(return_code);
    }; /* End if ((return_code = stream_ioctl(...)) < 0) */

    /* unconfigure pushed modules */
    /* Before unconfiguring, I check if the module is really loaded */
    if ((sad_info.sap_npush > 0) && (sad_info.sap_npush <= MAXAPUSH)) {
        if (return_code = unconf_modules(&sad_info)) {
            DEBUG_0("pop_mod: Unconfiguration of modules failed\n");
            return(return_code);
        };
    }
    else {
        DEBUG_2("pop_mod: Number of modules %d > MAXAPUSH=%d\n",
                sad_info.sap_npush, MAXAPUSH);
    } /* End if (sad_info.sap_npush <= MAXAPUSH) */

    sad_info.sap_cmd = SAP_CLEAR;
    /* Pop all modules for the major/minor couple */
    if (stream_ioctl(file_desc, SAD_SAP, &sad_info) < 0) {
        DEBUG_0("pop_mod: stream_ioctl with parameters below failed\n");
        DEBUG_1("           sap_cmd = %d\n", sad_info.sap_cmd);
        DEBUG_3("           (SAP_ONE=%d, SAP_RANGE=%d, SAP_CLEAR=%d)\n",
                SAP_ONE, SAP_RANGE, SAP_CLEAR);
        DEBUG_1("           sap_major = 0x%x\n", sad_info.sap_major);
        DEBUG_1("           sap_minor = 0x%x\n", sad_info.sap_minor);
        stream_close(file_desc);
        return(EINVAL);
    }
    else {
        DEBUG_0("pop_mod: All modules are poped\n");
        stream_close(file_desc);
    } /* End if (stream_ioctl(...) < 0) */

    return(return_code);
} /* End int pop_modules(...) */
