#ifndef lint
static char sccsid[] = "@(#)83 1.2 src/bos/usr/lib/methods/cfgtty/rsdd/rslinedds.c, cfgtty, bos41J, 9521B_all 5/26/95 07:49:34";
#endif
/*
 * COMPONENT_NAME: (CFGTTY) Build RS lines DDS
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
#include "srs.h"

#include "cfgdebug.h"
#include "ttycfg.h"

/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */

/*
 * =============================================================================
 *                       RSLINEDDS
 * =============================================================================
 * 
 * This function builds the DDS for the streams based SRS driver which is in
 * charge of aynchronous adapters (8,16 port and SIO serial).
 * 
 * This function operates as a device dependent subroutine called 
 * by the tty configure method. It is used to build the dds which 
 * describes the characteristics of the line to the device driver.
 *
 * A pointer to the DDS built and its size are returned to the
 * configure method in order to configure the asynchronous device driver.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */
int Build_DDS(cusDevPtr, ddsPtr, ddsSize, termiosPtr, termioxPtr,
               attrList)
struct CuDv * cusDevPtr;/* Customized object pointer */
uchar ** ddsPtr;        /* ptr to ptr to dds structure */
int *    ddsSize;       /* ptr to size fo dds */
struct   termios * termiosPtr;
struct   termiox * termioxPtr;
struct attr_list *   attrList;

{
    int    rc,count,val;              
    static struct rs_line_dds line_dds;  /* static dds structure */
    char current_att[ATTRVALSIZE];


    /* ======== */
    /* DDS TYPE */
    /* ======== */
    line_dds.which_dds = RS_LINE_DDS;

    /* =========== */
    /* DEVICE NAME */
    /* =========== */
    strncpy(line_dds.rc_name, cusDevPtr->name, DEV_NAME_LN);
    line_dds.rc_name[DEV_NAME_LN] = '\0';

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
    bcopy((char *)termioxPtr, (char *)&(line_dds.disc_ctl),
                                        sizeof(line_dds.disc_ctl));

    /* === */
    /* TBC */
    /* === */
    if ((rc = getatt(attrList, TBC16_ATT, &val,
                                 'i', &count)) != 0) {
        DEBUG_1("rslinedds: '%s' attribute not found\n", TBC16_ATT);
        return(E_NOATTR);
    }
    else {
        DEBUG_2("rslinedds: '%s' attribute found = %d\n", TBC16_ATT,
                                   val);
         line_dds.tbc = (uchar)val;
    }

    /* ===== */
    /* RTRIG */
    /* ===== */
    if ((rc = getatt(attrList, RTRIG_ATT, &val,
                                 'i', &count)) != 0) {
        DEBUG_1("rslinedds: '%s' attribute not found\n", RTRIG_ATT);
        return(E_NOATTR);
    }
    else {
        DEBUG_2("rslinedds: '%s' attribute found = %d\n", RTRIG_ATT,
                                   val);
         line_dds.rtrig = (uchar)val;
    }

    DEBUG_0("rslinedds: line_dds:\n");
    DEBUG_1("\twhich_dds  = %d\n", line_dds.which_dds);
    DEBUG_1("\trc_name    = %s\n", line_dds.rc_name);
    DEBUG_1("\tadap_name  = %s\n", line_dds.adap_name);
    DEBUG_0("\ttermios and termiox not dumped\n");
    DEBUG_1("\ttbc        = 0x%x\n", line_dds.tbc);
    DEBUG_1("\trtrig      = 0x%x\n", line_dds.rtrig);
    /* ========================== */
    /* Set argument output values */
    /* ========================== */
    *ddsPtr = (uchar *)&line_dds;
    *ddsSize = (int)sizeof(struct rs_line_dds);

    /* That's OK */
    return(0);
} /* End int Build_DDS(...) */
