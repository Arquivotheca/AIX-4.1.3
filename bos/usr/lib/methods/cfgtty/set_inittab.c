#ifndef lint
static char sccsid[] = "@(#)20 1.6 src/bos/usr/lib/methods/cfgtty/set_inittab.c, cfgtty, bos41J, 9520A_all 4/27/95 14:22:43";
#endif
/*
 * COMPONENT_NAME: (CFGMETHODS) Functions to update the inittab file
 *
 * FUNCTIONS: update_inittab, find_console, set_inittab
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>          /* standard I/O */
#include <stdlib.h>         /* standard C library */
#include <sys/types.h>      /* standard typedef */
#include <cf.h>             /* error messages */
#include <malloc.h>         /* struct mallinfo */
#include <errno.h>          /* standard error numbers */
#include <fcntl.h>          /* logical file system #define */

#include <sys/cfgdb.h>      /* config #define */
#include <sys/sysmacros.h>
#include <sys/console.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/stat.h>
#include <sys/cfgodm.h>     /* config structures */

#include "cfgdebug.h"
#include "pparms.h"

/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */
#define CONSOLE_NAME        "cons"
#define CONSOLE_DEV         "/dev/console"

/* commands for inittab file updates */
#define LIST_INITTAB        "/usr/sbin/lsitab"
#define REMOVE_INITTAB      "/usr/sbin/rmitab"
#define CHANGE_INITTAB      "/usr/sbin/chitab"
#define MAKE_INITTAB        "/usr/sbin/mkitab"
#define PDISABLE_INITTAB    "/usr/sbin/pdisable"
#define PENABLE_INITTAB     "/usr/sbin/penable"
#define PSHARE_INITTAB      "/usr/sbin/pshare"
#define PDELAY_INITTAB      "/usr/sbin/pdelay"
#define PHOLD_INITTAB       "/usr/sbin/phold"

/* command strings for inittab file updates */
#define ENABLESTR           "\"%s:2:respawn:/usr/sbin/getty /dev/%s\""
#define DISABLESTR          "\"%s:2:off:/usr/sbin/getty /dev/%s\""
#define DLYSTRING           "\"%s:2:respawn:/usr/sbin/getty -r /dev/%s\""
#define SHRSTRING           "\"%s:2:respawn:/usr/sbin/getty -u /dev/%s\""
#define HOLDSTRING           "\"%s:2:hold:/usr/sbin/getty /dev/%s\""

/* defines for attributes search */
/* These defines MUST be in coherence with ODM database attribute names */
#define LOGIN_ATT           "login"

/* Login values */
#define ENABLE              0
#define DISABLE             1
#define SHARE               2
#define DELAY               3
#define HOLD                4

/* Used in the update_initab function */
#define DEV_NAME            cusDevPtr->name

/*
 * ==============================================================================
 * External functions declarations
 * ==============================================================================
 */

/*
 * =============================================================================
 *                       LOCAL UTILITY ROUTINES
 * =============================================================================
 */
/*
 * -----------------------------------------------------------------------------
 *                       UPDATE_INITTAB
 * -----------------------------------------------------------------------------
 * 
 * Makes/Removes inittab entry for tty based on "login" attribute.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
static int update_inittab(cusDevPtr, consDev, iplPhase, login)
struct CuDv * cusDevPtr; /* logical name of tty device */
char * consDev;          /* console device name */
int    iplPhase;         /* run phase of method: ipl phase 1,2 or run-time */
char * login;   /* attribute list - looking for login */
{
    char   cmd_str[80]; /* inittab command string */
    char * err_ptr;     /* Pointer to standard error output */
    char * out_ptr;     /* Pointer to standard output */
    char * tmp_ptr;     /* Temporary pointer */

    int    need_change;
    int    login_value; /* login value */
    int    return_code,count;

    /* ODM structures declarations */

    /* ============================== */
    /* set up initial variable values */
    /* ============================== */
    return_code = 0;
    login_value = -1;

    /* ================================================== */
    /* Get "login" customized attribute object for device */
    /* ================================================== */
    DEBUG_1("update_inittab: login = %s\n",login);

    /* =============================== */
    /* Check the login attribute value */
    /* =============================== */
    if (!strcmp(login,"enable")) {
        login_value = ENABLE;
    }
    else if (!strcmp(login,"disable")) {
        login_value = DISABLE;
    }
    else if (!strcmp(login,"share")) {
        login_value = SHARE;
    }
    else if (!strcmp(login,"delay")) {
        login_value = DELAY;
    }
    else if (!strcmp(login,"hold")) {
        login_value = HOLD;
    }
    else {
        DEBUG_2("update_inittab: Unknown %s attribute value: %s\n",
                LOGIN_ATT, login);
        return(E_INVATTR);
    }
    DEBUG_2("update_inittab: The %s attribute value is: %s\n",
            LOGIN_ATT, login);

    /* ==================================== */
    /* Attempt to get inittab entry for tty */
    /* ==================================== */
    if (odm_run_method(LIST_INITTAB, DEV_NAME, &out_ptr, &err_ptr) == -1) {
        DEBUG_1("update_inittab: %s function failed\n", LIST_INITTAB);
    };

    /* =============================== */
    /* Update the inittab file for tty */
    /* =============================== */
    /* Check for console */
    if (!strcmp(DEV_NAME, consDev)) {
        /* tty is console - remove inittab entry */
        DEBUG_0("update_inittab: TTY IS CONSOLE\n");
        if (odm_run_method(REMOVE_INITTAB, DEV_NAME, NULL, NULL) == -1) {
            DEBUG_1("update_inittab: %s function failed\n", REMOVE_INITTAB);
        };
    }
    else {
        /* Inittab entry exists for tty */
        if (!strncmp(out_ptr, DEV_NAME, strlen(DEV_NAME))) {
            DEBUG_1("update_inittab: Inittab entry: %s\n", out_ptr);
            if ((iplPhase != PHASE1) && (iplPhase != PHASE2)) {
                /* Run Time */
                switch (login_value) {
                  case DISABLE: /* Do PDISABLE_INITTAB */
                    strcpy(login, PDISABLE_INITTAB);
                    break;
                    
                  case ENABLE: /* Do PENABLE_INITTAB */
                    strcpy(login, PENABLE_INITTAB);
                    break;
                    
                  case SHARE: /* Do PSHARE_INITTAB */
                    strcpy(login, PSHARE_INITTAB);
                    break;
                    
                  case DELAY: /* Do PDELAY_INITTAB */
                    strcpy(login, PDELAY_INITTAB);
                    break;

                  case HOLD: /* Do PHOLD_INITTAB */
                    strcpy(login, PHOLD_INITTAB);
                    break;

                  default: /* Programming error ! */
                    return(E_ODMRUNMETHOD);
                } /* End switch (login_value) */
                
                /* Execute the high level command */
                if (odm_run_method(login, DEV_NAME, NULL, NULL) == -1) {
                    DEBUG_1("update_inittab: %s function failed\n", login);
                };
            }
            else {
                /* IPL Time - Check inittab entry */
                tmp_ptr = out_ptr + strlen(DEV_NAME) + 1;
                /* tmp_ptr points on the second field of the line */
                while(*tmp_ptr != ':') {
                    tmp_ptr++;
                }
                tmp_ptr++;
                /* tmp_ptr points on the third field of the line */
                /* tmp_ptr points to "off" or "respawn" */
                need_change = 0;
                if ((login_value == DISABLE) && strncmp(tmp_ptr,"off",3)) {
                    need_change = 1;
                    sprintf(cmd_str, DISABLESTR, DEV_NAME, DEV_NAME);
                }
                else {
                    if ((login_value == ENABLE)
                        && strncmp(tmp_ptr, "respawn", 7)) {
                        need_change = 1;
                        sprintf(cmd_str, ENABLESTR, DEV_NAME, DEV_NAME);
                    }
                    else {
                        if ((login_value == DELAY)
                             && strncmp(tmp_ptr, "respawn", 7)) {
                            need_change = 1;
                            sprintf(cmd_str, DLYSTRING, DEV_NAME, DEV_NAME);
                        }
                        else {
                            if ((login_value == SHARE)
                                && strncmp(tmp_ptr, "respawn", 7)) {
                                need_change = 1;
                                sprintf(cmd_str, SHRSTRING, DEV_NAME, DEV_NAME);
                            }
                            else {
                                if ((login_value == SHARE)
                                     && strncmp(tmp_ptr, "hold", 7)) {
                                     need_change = 1;
                                     sprintf(cmd_str, SHRSTRING, DEV_NAME, DEV_NAME);
                                };
                            } /* End if ((login_value == SHARE) && ...) */
                        } /* End if ((login_value == DELAY) && ...) */
                    } /* End if ((login_value == ENABLE) && ...) */
                } /* End if ((login_value == DISABLE) && ...) */
                if (need_change) {
                    /* IPL Time - Change inittab entry */
                    strcpy(login, CHANGE_INITTAB);
                    
                    /* Execute the high level command */
                    if (odm_run_method(login, cmd_str, NULL, NULL) == -1) {
                        DEBUG_1("update_inittab: %s function failed\n", login);
                    };
                };
            } /* End if ((iplPhase != PHASE1) && (iplPhase != PHASE2)) */
        }
        else {
            /* Inittab entry does not exist */
            switch (login_value) {
              case ENABLE:
                sprintf(cmd_str, ENABLESTR, DEV_NAME, DEV_NAME);
                DEBUG_2("update_inittab: command is %s %s\n",
                         MAKE_INITTAB, cmd_str);
                
                /* Execute the high level command */
                DEBUG_2("update_inittab: command is %s %s\n",
                         login, cmd_str);
                if (odm_run_method(MAKE_INITTAB, cmd_str, NULL, NULL) == -1) {
                    DEBUG_1("update_inittab: %s function failed\n", MAKE_INITTAB);
                };
                
                if ((iplPhase != PHASE1) && (iplPhase != PHASE2)) {
                    /* Run Time - Do PENABLE_INITTAB */
                    strcpy(login, PENABLE_INITTAB);
                    sprintf(cmd_str, "%s", DEV_NAME);
                    
                    /* Execute the high level command */
                    if (odm_run_method(login, cmd_str, NULL, NULL) == -1) {
                        DEBUG_1("update_inittab: %s function failed\n", login);
                    };
                };
                break;
                
              case SHARE:
                sprintf(cmd_str, SHRSTRING, DEV_NAME, DEV_NAME);
                DEBUG_2("update_inittab: command is %s %s\n",
                         MAKE_INITTAB, cmd_str);
                
                /* Execute the high level command */
                DEBUG_2("update_inittab: command is %s %s\n",
                         login, cmd_str);
                if (odm_run_method(MAKE_INITTAB, cmd_str, NULL, NULL) == -1) {
                    DEBUG_1("update_inittab: %s function failed\n", MAKE_INITTAB);
                };
                
                if ((iplPhase != PHASE1) && (iplPhase != PHASE2)) {
                    /* Run Time - Do PSHARE_INITTAB */
                    strcpy(login, PSHARE_INITTAB);
                    sprintf(cmd_str, "%s", DEV_NAME);
                    
                    /* Execute the high level command */
                    if (odm_run_method(login, cmd_str, NULL, NULL) == -1) {
                        DEBUG_1("update_inittab: %s function failed\n", login);
                    };
                };
                break;
                
              case DELAY:
                sprintf(cmd_str, DLYSTRING, DEV_NAME, DEV_NAME);
                DEBUG_2("update_inittab: command is %s %s\n",
                         MAKE_INITTAB, cmd_str);
                
                /* Execute the high level command */
                if (odm_run_method(MAKE_INITTAB, cmd_str, NULL, NULL) == -1) {
                    DEBUG_1("update_inittab: %s function failed\n", MAKE_INITTAB);
                };
                
                if ((iplPhase != PHASE1) && (iplPhase != PHASE2)) {
                    /* Run Time - Do PDELAY_INITTAB */
                    strcpy(login, PDELAY_INITTAB);
                    sprintf(cmd_str,"%s",DEV_NAME);
                    
                    /* Execute the high level command */
                    DEBUG_2("update_inittab: command is %s %s\n",
                             login, cmd_str);
                    if (odm_run_method(login, cmd_str, NULL, NULL) == -1) {
                        DEBUG_1("update_inittab: %s function failed\n", login);
                    };
                };
                break;
                
              case DISABLE:
                sprintf(cmd_str, DISABLESTR, DEV_NAME, DEV_NAME);
                DEBUG_2("update_inittab: command is %s %s\n",
                         MAKE_INITTAB,cmd_str);
                
                /* Execute the high level command */
                if (odm_run_method(MAKE_INITTAB, cmd_str, NULL, NULL) == -1) {
                    DEBUG_1("update_inittab: %s function failed\n", MAKE_INITTAB);
                };
                
                if ((iplPhase != PHASE1) && (iplPhase != PHASE2)) {
                    /* Run Time - Do PDISABLE_INITTAB */
                    strcpy(login, PDISABLE_INITTAB);
                    sprintf(cmd_str,"%s",DEV_NAME);
                    
                    /* Execute the high level command */
                    DEBUG_2("update_inittab: command is %s %s\n",
                             login, cmd_str);
                    if (odm_run_method(login, cmd_str, NULL, NULL) == -1) {
                        DEBUG_1("update_inittab: %s function failed\n", login);
                    };
                };
                break;

              case HOLD:
                sprintf(cmd_str, DISABLESTR, DEV_NAME, DEV_NAME);
                DEBUG_2("update_inittab: command is %s %s\n",
                         MAKE_INITTAB,cmd_str);
                
                /* Execute the high level command */
                if (odm_run_method(MAKE_INITTAB, cmd_str, NULL, NULL) == -1) {
                    DEBUG_1("update_inittab: %s function failed\n", MAKE_INITTAB);
                };
                
                if ((iplPhase != PHASE1) && (iplPhase != PHASE2)) {
                    /* Run Time - Do PHOLD_INITTAB */
                    strcpy(login, PHOLD_INITTAB);
                    sprintf(cmd_str,"%s",DEV_NAME);
                    
                    /* Execute the high level command */
                    DEBUG_2("update_inittab: command is %s %s\n",
                             login, cmd_str);
                    if (odm_run_method(login, cmd_str, NULL, NULL) == -1) {
                        DEBUG_1("update_inittab: %s function failed\n", login);
                    };
                };
                break;
                
              default: /* Programming error ! */
                return(E_ODMRUNMETHOD);
            } /* End switch (login_value) */
        } /* End if (!strncmp(out_ptr, DEV_NAME, strlen(DEV_NAME))) */
    } /* End if (!strcmp(DEV_NAME, consDev)) */

    return(0);
} /* End static int update_inittab(...) */

/*
 * -----------------------------------------------------------------------------
 *                       FIND_CONSOLE
 * -----------------------------------------------------------------------------
 * 
 * Finds which device is the console in the current system.
 *
 * This function talks to the console device driver
 * to get the console device name.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
static int find_console(consDev)
     char *consDev;
{
    struct stat        stat_buf;
    struct qry_devsw   query_devsw;     /* Device switch table */
    struct cfg_dd      cfg_data;        /* For sysconfig call */
    struct cons_config cons_conf;       /* For console device driver */
    char   cons_path[256];              /* Got from console driver */
    char * cons_name;                   /* Pointer on console name */

    /* ========================================================== */
    /* Stat /dev/console then get the device number and check */
    /* the device switch entry to make sure it is still accessing */
    /* the console redirector driver in the kernel. If it is then */
    /* issue the CONSOLE_CFG command to get the default console */
    /* name. */
    /* ========================================================== */
    if (stat(CONSOLE_DEV, &stat_buf) == 0) {
        if (S_ISCHR(stat_buf.st_mode)) {
            query_devsw.devno = stat_buf.st_rdev;
            if (sysconfig(SYS_QDVSW, &query_devsw, sizeof(query_devsw))) {
                DEBUG_1("find_console: sysconfig SYS_QDVSW failed, errno = %d\n",
                        errno);
                return(E_SYSCONFIG);
            };
            if (query_devsw.status & DSW_CONSOLE) {
                cfg_data.kmid   = 0;
                cfg_data.devno  = stat_buf.st_rdev;
                cfg_data.cmd    = CONSOLE_CFG;
                cfg_data.ddsptr = (char *) (&cons_conf);
                cfg_data.ddslen = sizeof(cons_conf);
                cons_conf.cmd   = CONSGETDFLT;
                cons_conf.path  = cons_path;
                if(sysconfig(SYS_CFGDD, &cfg_data, sizeof(struct cfg_dd))) {
                    DEBUG_1("find_console: sysconfig SYS_CFGDD failed, errno = %d\n",
                            errno);
                    return(E_SYSCONFIG);
                };
            }
            else {
                DEBUG_0("find_console: console not accessing driver\n");
                return(E_DEVACCESS);
            }
        }
        else {
            DEBUG_0("find_console: st_mode check failed\n");
            return(E_DEVACCESS);
        }
    }
    else {
        DEBUG_1("find_console: stat on %s failed\n", CONSOLE_DEV);
        return(E_DEVACCESS);
    } /* End if (stat(CONSOLE_DEV, &stat_buf) == 0) */

    /* ==================================== */
    /* Pull console device name out of path */
    /* ==================================== */
    cons_name = strrchr(cons_path,'/');
    if (cons_name == NULL) {
        strcpy(consDev,"");
    }
    else  {
        strcpy(consDev, ++cons_name);
    }

    return(0);
} /* End static int find_console(...) */

/*
 * =============================================================================
 *                       SET_INITTAB
 * =============================================================================
 * 
 * This function sets some initab lines.
 * 
 * This function is used for the tty configuration.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */
int set_inittab(cusDev, loginval, iplPhase)
struct CuDv * cusDev; /* device customised object */
char * loginval;
int    iplPhase;      /* run phase of method: ipl phase 1,2 or run-time */
{
    char console[20];        /* To store console identifier */
    char * out_ptr;          /* Pointer to standard output */
    char * err_ptr;          /* Pointer to standard error output */
    int  return_code;

    /* ============================== */
    /* set up initial variable values */
    /* ============================== */
    console[0] = '\0';
    return_code = 0;

    if ((iplPhase != PHASE1) && (iplPhase != PHASE2)) {
        DEBUG_0("set_inittab: Not in phase1, neither in phase2\n");
        /* Check to see if there is an inittab entry for console */
        if (odm_run_method(LIST_INITTAB, CONSOLE_NAME, &out_ptr, &err_ptr) == -1) {
            DEBUG_1("set_inittab: %s function failed\n", LIST_INITTAB);
        };

        /* If CONSOLE_NAME is found */
        if (!strncmp(out_ptr, CONSOLE_NAME, 4)) {
            DEBUG_1("set_inittab: %s is found in the inittab file.\n",
                    CONSOLE_NAME);
            if (return_code = find_console(console)) {
                return(return_code);
            };
        }; /* End if (!strncmp(out_ptr, CONSOLE_NAME, 4)) */
    }; /* End if ((iplPhase != PHASE1) && (iplPhase != PHASE2)) */

    /* ================================= */
    /* Make/Remove inittab entry for tty */
    /* ================================= */ 
    DEBUG_1("update_inittab: loginval = %s\n",loginval);
    return_code = update_inittab(cusDev, console, iplPhase, loginval);

    return(return_code);
} /* End int set_inittab(...) */
