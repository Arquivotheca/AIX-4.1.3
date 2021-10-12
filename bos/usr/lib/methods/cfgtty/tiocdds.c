#ifndef lint
static char sccsid[] = "@(#)24 1.2 src/bos/usr/lib/methods/cfgtty/tiocdds.c, cfgtty, bos41J, 9520A_all 4/27/95 14:22:50";
#endif
/*
 * COMPONENT_NAME: (CFGTTY) Build of tioc DDS
 *
 * FUNCTIONS: tiocdds
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>          /* standard I/O */
#include <malloc.h>         /* Memory allocation */
#include <cf.h>             /* error messages */

#include <sys/cfgodm.h>     /* config structures */

#include "cfgdebug.h"
#include "ttycfg.h"

/* Includes for tioc */
#include "stream_tioc.h"

/*
 * =============================================================================
 *                       TIOCDDS
 * =============================================================================
 *
 * This function builds the DDS for the streams based tioc module.
 *
 * This function operates as a device dependent subroutine called 
 * by the tty configure method. It is used to build the dds which 
 * will be sent to the tioc module.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */
int tiocdds(cusDevPtr, ddsPtr, ddsSize)
struct CuDv * cusDevPtr;/* Customized object pointer */
uchar ** ddsPtr;        /* ptr to ptr to dds structure */
int *    ddsSize;       /* ptr to size fo dds */
{
    struct tioc_dds * line_dds;   /* pointer to dds structure */

    /* ============================== */
    /* set up initial variable values */
    /* ============================== */
    if ((line_dds = (struct tioc_dds *) malloc (sizeof(struct tioc_dds))) == NULL) {
        DEBUG_0 ("tiocdds: Malloc of dds failed\n");
        return(E_MALLOC);
    }
    else {
        bzero((char *)line_dds, sizeof(struct tioc_dds));
    }

    /* ============== */
    /* Set DDS fields */
    /* ============== */

    /* ======== */
    /* DDS TYPE */
    /* ======== */
    line_dds->which_dds = TIOC_DDS;

    DEBUG_0("tiocdds: line_dds:\n");
    DEBUG_1("\twhich_dds = %d\n", line_dds->which_dds);

    /* ========================== */
    /* Set argument output values */
    /* ========================== */
    *ddsPtr = (uchar *)line_dds;
    *ddsSize = (int)sizeof(struct tioc_dds);

    /* That's OK */
    return(0);
} /* End int tiocdds(...) */
