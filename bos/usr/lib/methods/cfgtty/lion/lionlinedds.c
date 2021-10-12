#ifndef lint
static char sccsid[] = "@(#)81 1.1 src/bos/usr/lib/methods/cfgtty/lion/lionlinedds.c, cfgtty, bos41J, 9520A_all 4/27/95 14:41:18";
#endif
/*
 * COMPONENT_NAME: (CFGTTY) Build LION lines DDS.
 *
 * FUNCTIONS: Build_DDS  
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>          /* standard I/O */
#include <string.h>         /* string manipulation */
#include <malloc.h>         /* Memory allocation */
#include <errno.h>          /* standard error numbers */
#include <ctype.h>
#include <math.h>
#include <cf.h>             /* error messages */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/cfgdb.h>      /* config #define */
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/cfgodm.h>     /* config structures */
#include <termios.h>
#include <sys/termiox.h>

/* Includes for srs DDS structure*/
#include "slion.h"

#include "cfgdebug.h"
#include "ttycfg.h"

/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */

/*
 * =============================================================================
 *                       Build_DDS
 * =============================================================================
 * 
 * This function builds the DDS for the streams based LION driver which is in
 * charge of the 64 ports aynchronous adapters.
 * 
 * This function operates as a device dependent subroutine called 
 * by the tty configure method. It is used to build the dds which 
 * describes the characteristics of the line to the device driver.
 *
 * A pointer to the DDS built and its size are returned to the
 * configure method in order to configure the asynchronous device driver.
 * The configure method is in charge of freeing DDS structure.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */
int Build_DDS(cusDevPtr, ddsPtr, ddsSize, termiosPtr, termioxPtr, attrList)
struct CuDv * cusDevPtr;/* Customized object pointer */
uchar ** ddsPtr;        /* ptr to ptr to dds structure */
int *    ddsSize;       /* ptr to size fo dds */
struct   termios * termiosPtr;
struct   termiox * termioxPtr;
struct attr_list *   attrList;
{
    int    att_count;               		/* attributes count */
    int    rc;
    char   attr_val[32];            		/* To validate input attribute value */
    char * inv_attr;                		/* Names of invalid attributes */
    static struct lion_line_dds line_dds;  	/* static dds structure */

    /* ODM structures declarations */
    struct CuAt *  current_att;     		/* current found attribute */



    /* ======== */
    /* DDS TYPE */
    /* ======== */
    line_dds.which_dds = LION_LINE_DDS;

    /* =========== */
    /* DEVICE NAME */
    /* =========== */
    strncpy(line_dds.line_name, cusDevPtr->name, DEV_NAME_LN);
    line_dds.line_name[DEV_NAME_LN] = '\0';

    /* ===================== */
    /* ADAPTER (PARENT) NAME */
    /* ===================== */
    strncpy(line_dds.adap_name, cusDevPtr->parent, DEV_NAME_LN);
    line_dds.adap_name[DEV_NAME_LN] = '\0';

    /* ============= */
    /* CONTROL MODES */
    /* ============= */
    bcopy((char *)termiosPtr, (char *)&(line_dds.ctl_modes), sizeof(line_dds.ctl_modes));

    /* ================================ */
    /* OPEN AND FLOW CONTROL DISCIPLINE */
    /* ================================ */
    bcopy((char *)termioxPtr, (char *)&(line_dds.disc_ctl), sizeof(line_dds.disc_ctl));

    /* === */
    /* TBC */
    /* === */
    if ((rc = getatt(attrList, TBC64_ATT, &current_att,
                                's', &att_count)) != NULL) {
        DEBUG_1("Build_DDS -lion : '%s' attribute not found\n", TBC64_ATT);
        return(E_NOATTR);
    } 
    line_dds.tbc = (uchar) atoi(current_att->value);
    DEBUG_2("Build_DDS -lion : '%s' found attribute = %s\n",
                                       TBC64_ATT, current_att->value);

    /* ======= */
    /* IN_XPAR */
    /* ======= */
    if ((rc = getatt(attrList, IN_XPAR_ATT, &current_att,
                                's', &att_count)) != NULL) {
        DEBUG_1("Build_DDS -lion : '%s' attribute not found\n", IN_XPAR_ATT);
        /* No error because it should be for a serial printer */
        /* This field should not be used in this case */
    }
    else {
        strcpy(line_dds.in_xpar, current_att->value);
        DEBUG_2("Build_DDS -lion : '%s' found attribute = %s\n",
                                            IN_XPAR_ATT, current_att->value);
    }

    /* ======= */
    /* LV_XPAR */
    /* ======= */
    if ((rc = getatt(attrList, LV_XPAR_ATT, &current_att,
                                's', &att_count)) != NULL) {
        DEBUG_1("Build_DDS -lion : '%s' attribute not found\n", LV_XPAR_ATT);
        /* No error because it should be for a serial printer */
        /* This field should not be used in this case */
    }
    else {
        strcpy(line_dds.lv_xpar, current_att->value);
        DEBUG_2("Build_DDS -lion : '%s' found attribute = %s\n",
                                            LV_XPAR_ATT, current_att->value);
    } 

    /* ======== */
    /* PRIORITY */
    /* ======== */
    if ((rc = getatt(attrList, PRIORITY_ATT, &current_att,
                                's', &att_count)) != NULL) {
        DEBUG_1("Build_DDS -lion : '%s' attribute not found\n", PRIORITY_ATT);
        /* No error because it should be for a serial printer */
        /* This field should not be used in this case */
    }
    else {
        line_dds.priority = (uchar) atoi(current_att->value);
        DEBUG_2("Build_DDS -lion : '%s' found attribute = %s\n", 
                                            PRIORITY_ATT, current_att->value);
    } 

    DEBUG_0("Build_DDS -lion : line_dds:\n");
    DEBUG_1("\twhich_dds  = %d\n", line_dds.which_dds);
    DEBUG_1("\tline_name  = %s\n", line_dds.line_name);
    DEBUG_1("\tadap_name  = %s\n", line_dds.adap_name);
    DEBUG_0("\ttermios and termiox not dumped\n");
    DEBUG_1("\ttbc        = 0x%x\n", line_dds.tbc);
    DEBUG_1("\tin_xpar    = %s\n", line_dds.in_xpar);
    DEBUG_1("\tlv_xpar    = %s\n", line_dds.lv_xpar);
    DEBUG_1("\tpriority   = 0x%x\n", line_dds.priority);

    /* ========================== */
    /* Set argument output values */
    /* ========================== */
    *ddsPtr = (uchar *)&line_dds;
    *ddsSize = (int)sizeof(struct lion_line_dds);

    /* That's OK */
    return(0);
} /* End int Build_DDS (...) */
