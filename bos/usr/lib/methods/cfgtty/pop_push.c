#ifndef lint
static char sccsid[] = "@(#)17 1.9 src/bos/usr/lib/methods/cfgtty/pop_push.c, cfgtty, bos41J, 9523B_all 6/1/95 13:14:06";
#endif
/*
 * COMPONENT_NAME: (CFGTTY) Streams facilities to autopush/pop modules
 *
 * FUNCTIONS: unconf_modules, push_modules, pop_modules, my_strtok
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>          /* standard I/O */
#include <cf.h>             /* error messages */
#include <errno.h>          /* standard error numbers */
#include <fcntl.h>          /* logical file system #define */

#include <sys/mode.h>
#include <sys/cfgdb.h>      /* config #define */
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/cfgodm.h>     /* config structures */
#include <termios.h>
#include <sys/termiox.h>

#include <sys/stropts.h>    /* Streams #define */
#include <sys/sad.h>        /* sad driver declarations */
#include <sys/str_tty.h>

#include "ttycfg.h"
#include "cfgdebug.h"

/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */
#define ADMINDEV            "/dev/sad"
#define SEPARATORS          ","
#define END_OF_STRING       '\0'

/*
 * ==============================================================================
 * External functions declarations
 * ==============================================================================
 */
extern mid_t loadext();
extern int   sptrdds();

/*
 * =============================================================================
 *                       LOCAL UTILITY ROUTINES
 * =============================================================================
 */
/*
 * -----------------------------------------------------------------------------
 *                       MY_STRTOK
 * -----------------------------------------------------------------------------
 *
 * This function is a duplicate of the "strtok_r" function.
 * This function is needed because the "attrval" function (from libcfg) uses
 * the "strtok" function (from libc) which is NOT re-entrant. Using the
 * "attrval" function, I cannot mix "attrval" and "strtok" functions.
 *
 * The right (best) fix should to remove any call to non re-entrant functions
 * in a library function => To correct the attrval function.
 * No right, no time ...
 * -----------------------------------------------------------------------------
 */
static int my_strtok(char *s1, const char *s2, char **savept)
{
    char    *p, *q, *r;

    /*first or subsequent call*/
    p = (s1 == NULL)? *savept: s1;

    if (p == 0)     /* return if no tokens remaining */
        return(NULL);

    q = p + strspn(p, s2);  /* skip leading separators */

    if (*q == '\0')     /* return if no tokens remaining */
        return(NULL);

    if ((r = strpbrk(q, s2)) == NULL)   /* move past token */
        *savept = 0; /* indicate this is last token */
    else {
        *r = '\0';
        *savept = ++r;
    }
    return (q);
}

/*
 * -----------------------------------------------------------------------------
 *                       UNCONF_MODULES
 * -----------------------------------------------------------------------------
 *
 * Unconfigures and unloads (only at last unload) modules.
 * Unused entries of the entry parameter MUST be
 * initialized to NULL.
 *
 * This routine is to be used only within this file.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * When an error occurs, the last found error code is returned.
 * -----------------------------------------------------------------------------
 */
static int unconf_modules(strapushPtr)
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
    while ((module_nb < MAXAPUSH) &&
           strcmp(strapushPtr->sap_list[module_nb], "")) {
        /* Set up cfg fields */
        /* mdiptr and mdilen may be updated if the DDS */
        /* is unknown */
        cfg.mdiptr = (caddr_t)&which_dds;
        cfg.mdilen = sizeof(which_dds);
        cfg.cmd = CFG_TERM;
        if ((cfg.kmid = loadext(strapushPtr->sap_list[module_nb], FALSE, TRUE))
            == (mid_t) NULL) {
            DEBUG_1("unconf_modules: %s module isn't found as kernel extension.\n",
                    strapushPtr->sap_list[module_nb]);
            return_code = E_LOADEXT;
        }
        else {
            /* Which module is it ? */
            if (!strcmp(strapushPtr->sap_list[module_nb], LDTERM_MODULE)) {
                which_dds = LDTERM_DDS;
            }
            else if (!strcmp(strapushPtr->sap_list[module_nb], SPTR_MODULE)) {
                /* Update of DDS is needed */
                /* Only which_dds and devno fields are updated => 1 as last parameter */
                if (return_code = sptrdds(NULL, &cfg.mdiptr, &cfg.mdilen,
                                          strapushPtr->sap_major,
                                          strapushPtr->sap_minor, 1)) {
                    DEBUG_1("unconf_modules: Partial build of %s DDS failed\n", SPTR_MODULE);
                    return (return_code);
                }; /* End if (return_code = sptrdds(...)) */
            }
            else if (!strcmp(strapushPtr->sap_list[module_nb], TIOC_MODULE)) {
                which_dds = TIOC_DDS;
            }
            else if (!strcmp(strapushPtr->sap_list[module_nb], NLS_MODULE)) {
                which_dds = NLS_DDS;
            }
            else {
                DEBUG_1("unconf_modules: Unknown DDS for %s module\n",
                        strapushPtr->sap_list[module_nb]);
                /* No DDS is sent */
                cfg.mdiptr = (caddr_t) NULL;
                cfg.mdilen = 0;
            }/* End if (!strcmp(...)) */
            
            /* Performs the sysconfig */
            if (sysconfig(SYS_CFGKMOD, &cfg, sizeof(cfg)) < 0) {
                DEBUG_1("unconf_modules: Unconfiguration of %s failed\n",
                        strapushPtr->sap_list[module_nb]);
                return_code = E_SYSCONFIG;
            };

            /* Performs the loadext */
            /* The kernel unloads effectively only when the */
            /* load count of the object file becomes zero */
            if (loadext(strapushPtr->sap_list[module_nb], FALSE, FALSE)
                == (mid_t) NULL) {
                DEBUG_1("unconf_modules: Unload of %s failed\n",
                        strapushPtr->sap_list[module_nb]);
                return_code = E_UNLOADEXT;
            };
        } /* End if ((cfg.kmid = loadext(...)) == (mid_t) NULL) */

        module_nb++;
    } /* End while (...) */

    return(return_code);
} /* End static int unconf_modules(...) */

/*
 * -----------------------------------------------------------------------------
 *                       PUSH_MODULES
 * -----------------------------------------------------------------------------
 * 
 * Opens SAD to autopush modules, then configures them.
 * This function makes similar action as the 'autopush' function.
 *
 * If 'minorNb' is set to -1, then a SAP_ALL request must be performed
 * instead of SAP_ONE. It is to avoid too many memory usage from the 'sad'
 * driver when configuring PTY.
 *
 * There a special handling for LDTERM module for EUC
 * codeset map. If multibyte and non-EUC code, a lower
 * and an upper converter must be autopushed
 * below and above.
 *
 * Return code: Exits with 0 on success, ODm error code otherwise.
 * -----------------------------------------------------------------------------
 */
int push_modules(cusDevPtr, modules, majorNb, minorNb)
struct CuDv * cusDevPtr;          /* Customized object pointer */
char * modules;                   /* List of modules name */
long   majorNb, minorNb;          /* major and minor numbers */
{
    extern int      ldtermdds();
    extern int      tiocdds();
    extern int      nlsdds();

    /* For sad driver access */
    int             file_desc;       /* File descriptor */
    int             modules_number;  /* Number of found modules */
    int             return_code;
    char *          next_name;       /* To scan modules list */
    struct strapush modules_list;    /* list of modules to push */

    struct cfg_kmod cfg;             /* structure for module configuration */
    struct for_euc  euc_converter;   /* For ldterm configuration */
    
	char *          save_ptr;        /* Useful for my_strtok */

    /* ============================== */
    /* set up initial variable values */
    /* ============================== */
    modules_number = 0;
    bzero((char *)&modules_list, sizeof(struct strapush));
    bzero((char *)&cfg, sizeof(cfg));
    return_code = 0;
	save_ptr = 0;
    
    /* ================ */
    /* Opens sad device */
    /* ================ */
    if ((file_desc = stream_open(ADMINDEV, O_RDWR)) == -1) {
        DEBUG_1("push_modules: stream_open error with %s\n", ADMINDEV);
        return(E_OPEN);
    };

    /* ================================================= */
    /* Sets modules_list structure and configures module */
    /* ================================================= */
    /* Retrieves modules names, builds their DDS and configures them */
    next_name = my_strtok(modules, SEPARATORS, &save_ptr);
    while ((next_name != NULL) && (modules_number < MAXAPUSH)
           && !return_code) {
        /* LDTERM ==> I need to see if converters must be autopushed */
        if (!strcmp(next_name, LDTERM_MODULE)) {
            return_code = ldtermdds(cusDevPtr, &cfg.mdiptr, &cfg.mdilen,
                                    &euc_converter);
            if (return_code) {
                DEBUG_1("push_modules: Build of %s DDS failed\n", LDTERM_MODULE);
            }
            else {
                DEBUG_1("push_modules: Build of %s DDS succeeded\n", LDTERM_MODULE);
                /* If upper and lower converter need to be autopushed */
                if (euc_converter.existing) {
                    DEBUG_3("push_modules: %s and %s need to be autopushed with %s\n",
                            euc_converter.upper, euc_converter.lower, LDTERM_MODULE);
                    /* Proceed only if not too much modules */
                    if ((modules_number + 3) < MAXAPUSH) {
                        /* Autopush upper converter */
                        strncpy(modules_list.sap_list[modules_number],
                                euc_converter.upper, FMNAMESZ);
                        modules_list.sap_list[modules_number][FMNAMESZ] = END_OF_STRING;
                        modules_number++;
                        /* Autopush ldterm */
                        strncpy(modules_list.sap_list[modules_number],
                                next_name, FMNAMESZ);
                        modules_list.sap_list[modules_number][FMNAMESZ] = END_OF_STRING;
                        modules_number++;
                        /* Autopush lower converter */
                        strncpy(modules_list.sap_list[modules_number],
                                euc_converter.lower, FMNAMESZ);
                        modules_list.sap_list[modules_number][FMNAMESZ] = END_OF_STRING;
                        modules_number++;
                    }
                    else {
                        /* Updates modules_number to return with an error */
                        modules_number += 3;
                        break;
                    } /* End if ((modules_number + 3) < MAXAPUSH) */
                }
                else { /* Only ldterm needs to be autopushed */
                    DEBUG_1("push_modules: No converter need to be autopushed with %s\n",
                            LDTERM_MODULE);
                    strncpy(modules_list.sap_list[modules_number],
                            next_name, FMNAMESZ);
                    modules_list.sap_list[modules_number][FMNAMESZ] = END_OF_STRING;
                    modules_number++;
                } /* End if (euc_converter.existing) */
            } /* End if (return_code) */
        }
        
        /* SPTR */
        else if (!strcmp(next_name, SPTR_MODULE)) {
            /* Complete DDS is needed => 0 as last parameter */
            return_code = sptrdds(cusDevPtr, &cfg.mdiptr, &cfg.mdilen,
                                  majorNb, minorNb, 0);

            if (return_code) {
                DEBUG_1("push_modules: Build of %s DDS failed\n", SPTR_MODULE);
            }
            else {
                DEBUG_1("push_modules: Build of %s DDS succeeded\n", SPTR_MODULE);
                strncpy(modules_list.sap_list[modules_number],
                        next_name, FMNAMESZ);
                modules_list.sap_list[modules_number][FMNAMESZ] = END_OF_STRING;
                modules_number++;
            }
        }

        /* TIOC */
        else if (!strcmp(next_name, TIOC_MODULE)) {
            return_code = tiocdds(cusDevPtr, &cfg.mdiptr, &cfg.mdilen);
            if (return_code) {
                DEBUG_1("push_modules: Build of %s DDS failed\n", TIOC_MODULE);
            }
            else {
                DEBUG_1("push_modules: Build of %s DDS succeeded\n", TIOC_MODULE);
                strncpy(modules_list.sap_list[modules_number],
                        next_name, FMNAMESZ);
                modules_list.sap_list[modules_number][FMNAMESZ] = END_OF_STRING;
                modules_number++;
            }
        }

        /* NLS */
        else if (!strcmp(next_name, NLS_MODULE)) {
            return_code = nlsdds(cusDevPtr, &cfg.mdiptr, &cfg.mdilen);
            if (return_code) {
                DEBUG_1("push_modules: Build of %s DDS failed\n", NLS_MODULE);
            }
            else {
                DEBUG_1("push_modules: Build of %s DDS succeeded\n", NLS_MODULE);
                strncpy(modules_list.sap_list[modules_number],
                        next_name, FMNAMESZ);
                modules_list.sap_list[modules_number][FMNAMESZ] = END_OF_STRING;
                modules_number++;
            }
        }

        /* No specific DDS is built */
        else {
            cfg.mdiptr = (caddr_t)NULL;
            cfg.mdilen = 0;
            strncpy(modules_list.sap_list[modules_number],
                    next_name, FMNAMESZ);
            modules_list.sap_list[modules_number][FMNAMESZ] = END_OF_STRING;
            modules_number++;
        }
        
        if (!return_code) {
            /* Load and configuration of found module */
            /* cfg.mdiptr and cfg.mdilen have been previously updated */
            cfg.cmd = CFG_INIT;
            if ((cfg.kmid = loadext(next_name, TRUE, FALSE)) == (mid_t)NULL) {
                DEBUG_1("push_modules: Loading of %s module failed\n",
                        next_name);
                return_code = E_LOADEXT;
            }
            else {
                if (sysconfig(SYS_CFGKMOD, &cfg, sizeof(cfg)) < 0) {
                    DEBUG_1("push_modules: Configuration of %s failed\n", next_name);
                    return_code = E_SYSCONFIG;
                }
                else {
                    /* Next module name to autopush */
                    next_name = my_strtok(NULL, SEPARATORS, &save_ptr);
                } /* End if (sysconfig(...) < 0) */
            } /* End if ((cfg.kmid = loadext(...)) == (mid_t)NULL) */
        }; /* End if (!return_code) */
    }; /* End while (...) */
    
    /* For PTY we want to perform request for any minor number */
    modules_list.sap_cmd = (minorNb == -1) ? SAP_ALL : SAP_ONE;
    modules_list.sap_major = majorNb;
    modules_list.sap_minor = minorNb;
    modules_list.sap_lastminor = 0;
    modules_list.sap_npush = modules_number;
 
    /* Checks for error */
    if (return_code || (modules_number >= MAXAPUSH)) {
        DEBUG_2("push_modules: Cannot configure modules for major = %d and minor = %d\n",
                majorNb, minorNb);
        stream_close(file_desc);
        /* To unconfigure and unload already loaded modules when an error occurs */
        if (unconf_modules(&modules_list)) {
            DEBUG_2("push_modules: Cannot recover error which occured for major = %d and minor = %d\n",
                    majorNb, minorNb);
            DEBUG_0("push_modules: Be careful, some modules are perhaps in part configured\n");
        };
        return(return_code ? return_code : E_ARGS);
    };
    DEBUG_0("push_modules: All needed modules are configured\n");

    /* Calls sad to push these modules at next open of the line */
    if (stream_ioctl(file_desc, SAD_SAP, &modules_list) < 0) {
        switch (errno) {
          case EFAULT:
            DEBUG_0("push_modules: Bad argument for stream_ioctl\n");
            break;
          case EINVAL:
            DEBUG_0("push_modules: Invalid major or some modules are unavailable\n");
            break;
          case EEXIST:
            DEBUG_2("push_modules: Device major=0x%x, minor=0x%x already configured\n",
                    majorNb, minorNb);
            break;
          case ENOSTR:
            DEBUG_1("push_modules: This major 0x%x is not a streams device\n",
                    majorNb);
            break;
          case ENODEV:
            DEBUG_2("push_modules: This device (0x%x,0x%x) is not configured for autopush\n",
                    majorNb, minorNb);
            break;
          case ENOSR:
            DEBUG_0("push_modules: No memory for data structure\n");
            break;
          default:
            DEBUG_1("push_modules: Unexpected stream_ioctl SAD_GAP; errno = %d\n",
                    errno);
        } /* End switch (errno) */
        DEBUG_0("push_modules: stream_ioctl with parameters below failed\n");
        DEBUG_1("           sap_cmd = %d\n", modules_list.sap_cmd);
        DEBUG_3("           (SAP_ONE=%d, SAP_RANGE=%d, SAP_CLEAR=%d)\n",
                SAP_ONE, SAP_RANGE, SAP_CLEAR);
        DEBUG_1("           sap_major = 0x%x\n", modules_list.sap_major);
        DEBUG_1("           sap_minor = 0x%x\n", modules_list.sap_minor);
        DEBUG_1("           sap_last_minor = 0x%x\n", modules_list.sap_lastminor);
        DEBUG_1("           sap_npush = 0x%x\n", modules_list.sap_npush);
        for (modules_number = 0; modules_number < MAXAPUSH; modules_number++) {
            DEBUG_2("           sap_list[%d] = %s\n",
                    modules_number, modules_list.sap_list[modules_number]);
        } /* End for (...) */
        stream_close(file_desc);
        return(E_ARGS);
    }
    else {
        DEBUG_0("push_modules: All needed modules are autopushed\n");
        stream_close(file_desc);
        return(0);
    } /* End if (stream_ioctl(...) <0 ) */
    
} /* End int push_modules(...) */

/*
 * -----------------------------------------------------------------------------
 *                       POP_MODULES
 * -----------------------------------------------------------------------------
 * 
 * Opens SAD to pop modules.
 * We get pushed modules, pop and unconfigure them.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
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
        DEBUG_1("pop_modules: stream_open error with %s\n", ADMINDEV);
        return(E_OPEN);
    };

    /* ================== */
    /* get pushed modules */
    /* ================== */
    if ((return_code = stream_ioctl(file_desc, SAD_GAP, &sad_info)) < 0) {
        DEBUG_0("pop_modules: stream_ioctl with parameters below failed\n");
        DEBUG_1("           sap_major = 0x%x\n", sad_info.sap_major);
        DEBUG_1("           sap_minor = 0x%x\n", sad_info.sap_minor);
        switch (errno) {
          case EFAULT:
            DEBUG_0("pop_modules: Bad argument for stream_ioctl\n");
            break;
          case EINVAL:
            DEBUG_0("pop_modules: Invalid major\n");
            break;
          case ENOSTR:
            DEBUG_0("pop_modules: This major is not a streams device\n");
            break;
          case ENODEV:
            DEBUG_0("pop_modules: This device is not configured for autopush\n");
            break;
          default:
            DEBUG_1("pop_modules: Unexpected stream_ioctl SAD_GAP; errno = %d\n",
                    errno);
        } /* End switch (errno) */
        stream_close(file_desc);
        return(E_STAT);
    }; /* End if ((return_code = stream_ioctl(...)) < 0) */

    /* unconfigure pushed modules */
    if (sad_info.sap_npush > 0) {
        /* Before unconfiguring, I check if the module is really loaded */
        if (sad_info.sap_npush <= MAXAPUSH) {
            if (return_code = unconf_modules(&sad_info)) {
                DEBUG_0("pop_modules: Unconfiguration of modules failed\n");
                return(return_code);
            };
        }
        else {
            DEBUG_2("pop_modules: Number of modules %d > MAXAPUSH=%d\n",
                    sad_info.sap_npush, MAXAPUSH);
        } /* End if (sad_info.sap_npush <= MAXAPUSH) */
    } /* End if (sad_info.sap_npush > 0) */

    sad_info.sap_cmd = SAP_CLEAR;
    /* Pop all modules for the major/minor couple */
    if (stream_ioctl(file_desc, SAD_SAP, &sad_info) < 0) {
        switch (errno) {
          case EFAULT:
            DEBUG_0("push_modules: Bad argument for stream_ioctl\n");
            break;
          case EINVAL:
            DEBUG_0("push_modules: Invalid major or some modules are unavailable\n");
            break;
          case EEXIST:
            DEBUG_2("push_modules: Device major=0x%x, minor=0x%x already configured\n",
                    majorNb, minorNb);
            break;
          case ENOSTR:
            DEBUG_1("push_modules: This major 0x%x is not a streams device\n",
                    majorNb);
            break;
          case ENODEV:
            DEBUG_2("push_modules: This device (0x%x,0x%x) is not configured for autopush\n",
                    majorNb, minorNb);
            break;
          case ENOSR:
            DEBUG_0("push_modules: No memory for data structure\n");
            break;
          default:
            DEBUG_1("push_modules: Unexpected stream_ioctl SAD_GAP; errno = %d\n",
                    errno);
        } /* End switch (errno) */
        DEBUG_0("pop_modules: stream_ioctl with parameters below failed\n");
        DEBUG_1("           sap_cmd = %d\n", sad_info.sap_cmd);
        DEBUG_3("           (SAP_ONE=%d, SAP_RANGE=%d, SAP_CLEAR=%d)\n",
                SAP_ONE, SAP_RANGE, SAP_CLEAR);
        DEBUG_1("           sap_major = 0x%x\n", sad_info.sap_major);
        DEBUG_1("           sap_minor = 0x%x\n", sad_info.sap_minor);
        stream_close(file_desc);
        return(E_ARGS);
    }
    else {
        DEBUG_0("pop_modules: All modules are poped\n");
        stream_close(file_desc);
    } /* End if (stream_ioctl(...) < 0) */

    return(return_code);
} /* End int pop_modules(...) */
