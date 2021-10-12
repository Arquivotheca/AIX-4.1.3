#ifndef lint
static char sccsid[] = "@(#)15 1.3 src/bos/usr/lib/methods/cfgtty/ldtermdds.c, cfgtty, bos41J, 9520A_all 4/27/95 14:22:37";
#endif
/*
 * COMPONENT_NAME: (CFGTTY) Build of ldterm DDS
 *
 * FUNCTIONS: ldtermdds, decode_csmap_file
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

/* Includes for ldterm */
#include "ldtty.h"

/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */
/* defines for attributes search */
/* These defines MUST be in coherence with ODM database attribute names */
#define CSMAP_ATT                   "csmap"

#define CSMAP_FILE_DIR              "/usr/lib/nls/csmap"

#define ERR_RETURN(returnCode)      \
  free(line_dds);                   \
  line_dds = NULL;                  \
  return(returnCode);

/*
 * =============================================================================
 *                       LOCAL UTILITY ROUTINES
 * =============================================================================
 */
/*
 * -----------------------------------------------------------------------------
 *                       DECODE_CSMAP_FILE
 * -----------------------------------------------------------------------------
 * 
 * This function decodes csmap file given as
 * entry parameter:
 *     If single byte, nothing is done because the eucioc
 *     structure is not used. 
 *     If mulibyte and non-EUC code, a lower and an upper
 *     converter are pushed and the eucioc structure is updated.
 *     If multibyte and EUC code, only the eucioc stucture
 *     is updated.
 *
 * This routine is to be used only within this file.
 * The device specific routines for the various device
 * specific config methods  must not call this function.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
static int decode_csmap_file(fileName, eucPtr)
char * fileName;
struct for_euc * eucPtr;
{
    /* No decoding for the moment ==> No converter to push */
    eucPtr->existing = 0;

    return(0);
} /* End static int decode_csmap_file(...) */

/*
 * =============================================================================
 *                       LDTERMDDS
 * =============================================================================
 *
 * This function builds the DDS for the streams based ldterm module.
 *
 * This function operates as a device dependent subroutine called 
 * by the tty configure method. It is used to build the dds which 
 * will be sent to the ldterm module.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */
int ldtermdds(cusDevPtr, ddsPtr, ddsSize, eucPtr)
struct CuDv * cusDevPtr;/* Customized object pointer */
uchar ** ddsPtr;        /* ptr to ptr to dds structure */
int *    ddsSize;       /* ptr to size fo dds */
struct   for_euc * eucPtr;
{
    int    return_code;             /* return codes go here */
    int    att_count;               /* attributes count */
    struct ldterm_dds * line_dds;   /* pointer to dds structure */

    /* ODM structures declarations */
    struct CuAt *  current_att;     /* current found attribute */

    /* ============================== */
    /* set up initial variable values */
    /* ============================== */
    if ((line_dds = (struct ldterm_dds *) malloc (sizeof(struct ldterm_dds))) == NULL) {
        DEBUG_0 ("ldtermdds: Malloc of dds failed\n");
        return(E_MALLOC);
    }
    else {
        bzero((char *)line_dds, sizeof(struct ldterm_dds));
    }

    /* ============== */
    /* Set DDS fields */
    /* ============== */

    /* ======== */
    /* DDS TYPE */
    /* ======== */
    line_dds->which_dds = LDTERM_DDS;

    /* ================ */
    /* EUC CODE SET MAP */
    /* ================ */
    /* Get code set map file */
    if ((current_att = getattr(cusDevPtr->name, CSMAP_ATT, FALSE, &att_count))
        != NULL) {
        DEBUG_2("ldtermdds: '%s' attribute found = %s\n", CSMAP_ATT, current_att->value);
        /* To decode found csmap file */
        if (return_code = decode_csmap_file(current_att->value, eucPtr)) {
            DEBUG_1("ldtermdds: Error in '%s' file decoding\n", current_att->value);
            ERR_RETURN(E_INVATTR);
        }; /* End if (return_code = decode_csmap_file(...) */
    }
    else {
        DEBUG_1("ldtermdds: '%s' attribute not found\n", CSMAP_ATT);
        ERR_RETURN(E_NOATTR);
    }

    DEBUG_0("ldtermdds: line_dds:\n");
    DEBUG_1("\twhich_dds = %d\n", line_dds->which_dds);

    /* ========================== */
    /* Set argument output values */
    /* ========================== */
    *ddsPtr = (uchar *)line_dds;
    *ddsSize = (int)sizeof(struct ldterm_dds);

    /* That's OK */
    return(0);
} /* End int ldtermdds(...) */
