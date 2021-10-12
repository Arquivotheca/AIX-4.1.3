#ifndef lint
static char sccsid[] = "@(#)16 1.2 src/bos/usr/lib/methods/cfgtty/nlsdds.c, cfgtty, bos41J, 9520A_all 4/27/95 14:22:39";
#endif
/*
 * COMPONENT_NAME: (CFGTTY) Build of the nls DDS
 *
 * FUNCTIONS: nlsdds
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

/* Includes for nls */
#include "snls.h"

/*
 * =============================================================================
 *                       NLSDDS
 * =============================================================================
 *
 * This function builds the DDS for the streams based nls module.
 *
 * This function operates as a device dependent subroutine called 
 * by the tty configure method. It is used to build the dds which 
 * will be sent to the nls module.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */
int nlsdds(cusDevPtr, ddsPtr, ddsSize)
struct CuDv * cusDevPtr;/* Customized object pointer */
uchar ** ddsPtr;        /* ptr to ptr to dds structure */
int *    ddsSize;       /* ptr to size fo dds */
{
    struct nls_dds * line_dds;   /* pointer to dds structure */

    /* ============================== */
    /* set up initial variable values */
    /* ============================== */
    if ((line_dds = (struct nls_dds *) malloc (sizeof(struct nls_dds))) == NULL) {
        DEBUG_0 ("nlsdds: Malloc of dds failed\n");
        return(E_MALLOC);
    }
    else {
        bzero((char *)line_dds, sizeof(struct nls_dds));
    }

    /* ============== */
    /* Set DDS fields */
    /* ============== */

    /* ======== */
    /* DDS TYPE */
    /* ======== */
    line_dds->which_dds = NLS_DDS;

    DEBUG_0("nlsdds: line_dds:\n");
    DEBUG_1("\twhich_dds = %d\n", line_dds->which_dds);

    /* ========================== */
    /* Set argument output values */
    /* ========================== */
    *ddsPtr = (uchar *)line_dds;
    *ddsSize = (int)sizeof(struct nls_dds);

    /* That's OK */
    return(0);
} /* End int nlsdds(...) */
