#ifndef lint
static char sccsid[] = "@(#)59 1.2 src/bos/usr/lib/methods/chgtty/set_maps.c, sysxtty, bos411, 9428A410j 6/1/94 03:31:40";
#endif
/*
 * COMPONENT_NAME: (CFGMETHODS) Use of the setmaps command
 *
 * FUNCTIONS: set_maps
 *
 * ORIGINS: 27, 83
 *
 */
/*
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */


#include <stdio.h>
#include <string.h>
#include <cf.h>        /* Error codes */
#include <errno.h>
#include <fcntl.h>

#include <sys/cfgdb.h>
#include <sys/cfgodm.h>

#include "cfgdebug.h"
#include "ttycfg.h"
#include "pparms.h"

/*
 * ==============================================================================
 * Defines and extern variables
 * ==============================================================================
 */
#define DEV_DIR             "/dev"
/*
 * -------------------
 * ODM attribute names
 * -------------------
 */
#define IMAP_ATT            "imap"
#define OMAP_ATT            "omap"

#define NONE_VALUE          "none"
#define SETMAPS_NOMAP       "NOMAP"

/*
 * =============================================================================
 *                       LOCAL UTILITY ROUTINES
 * =============================================================================
 */
/*
 * -----------------------------------------------------------------------------
 *                       CALLSETMAPS
 * -----------------------------------------------------------------------------
 * 
 * This function opens the device and issues the setmaps command.
 * 
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
static int callsetmaps(logicalName, setmapsArgv)
char * logicalName;
char * setmapsArgv[];
{
    char cmd[256];
    int file_desc;
    int ch_status;
	int ch_pid;
	int ch_stat;

    sprintf(cmd, "%s/%s", DEV_DIR, logicalName);
    if ((ch_pid = fork()) == -1) {
        DEBUG_1 ("callsetmaps: fork failed, errno = %d\n", errno)
        return E_FORK;
    }
    else if (ch_pid == 0) {
        if ((file_desc = open (cmd,O_RDONLY|O_NDELAY)) < 0) {
            DEBUG_1 ("callsetmaps: Open failed, errno = %d\n", errno)
            return E_OPEN;
        }
        DEBUG_1 ("callsetmaps: Opened %s\n", cmd)
        close(0);
        dup(file_desc);

        strcpy(cmd, "/bin/setmaps");
        setmapsArgv[0] = cmd;
        (void) execv (cmd, setmapsArgv);
    }
    else {
        while ((ch_status = wait(&ch_stat)) != ch_pid) {
            if (ch_status <0 && errno == ECHILD)
                break;
            errno = 0;
        }
    }

    DEBUG_1 ("callsetmaps: %s complete\n", cmd)
    return 0;


} /* End static int callsetmaps(...) */

/*
 * =============================================================================
 *                       SET_MAPS
 * =============================================================================
 * 
 * This function calls the setmaps command if one 'XXX_MAP' attribute is found.
 * 
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */
int set_maps(logicalName, attrList)
char *        logicalName;
struct attr * attrList;
{
	char *        cmd_argv[5];
	struct attr * current_attr;
	
	/* If IMAP_ATT is to be changed */
	if (current_attr = att_changed(attrList, IMAP_ATT)) {
		cmd_argv[1] = "-i";
		if (!strcmp(current_attr->value, NONE_VALUE)) {
			cmd_argv[2] = SETMAPS_NOMAP;
		}
		else {
			cmd_argv[2] = current_attr->value;
		}
		cmd_argv[3] = NULL;
		return(callsetmaps(logicalName,cmd_argv));
	}
	/* If OMAP_ATT is to be changed */
	else if (current_attr = att_changed(attrList, OMAP_ATT)) {
		cmd_argv[1] = "-o";
		if (!strcmp(current_attr->value, NONE_VALUE)) {
			cmd_argv[2] = SETMAPS_NOMAP;
		}
		else {
			cmd_argv[2] = current_attr->value;
		}
		cmd_argv[3] = NULL;
		return(callsetmaps(logicalName,cmd_argv));
	}
	/* If CSMAP_ATT is to be changed */
	else if (current_attr = att_changed(attrList, CSMAP_ATT)) {
		cmd_argv[1] = "-s";
		cmd_argv[1] = "-i";
		cmd_argv[3] = current_attr->value;
		cmd_argv[4] = NULL;
		return(callsetmaps(logicalName,cmd_argv));
	}
	else {
		return(0);
	}
} /* End int set_maps(...) */







