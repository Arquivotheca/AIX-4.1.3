#ifndef lint
static char sccsid[] = "@(#)23 1.3 src/bos/usr/lib/methods/cfgtty/sptrdds.c, cfgtty, bos41J, 9520A_all 4/27/95 14:22:48";
#endif
/*
 * COMPONENT_NAME: (CFGTTY) Build of SPTR module DDS
 *
 * FUNCTIONS: get_odm_value, sptrdds
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

#include <sys/lpio.h>       /* Structures for printer */
#include <sys/cfgdb.h>      /* config #define */
#include <sys/sysmacros.h>  /* system macros */
#include <sys/cfgodm.h>     /* config structures */

#include "cfgdebug.h"
#include "ttycfg.h"

/* Includes for sptr */
#include "sptr.h"
 
/*
 * ==============================================================================
 * common extern functions
 * ==============================================================================
 */
extern int attrval();           /* attribute values are in range ? */

/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */
/* defines for attributes search */
/* These defines MUST be in coherence with ODM database attribute names */
#define PTOP_ATT                    "ptop"
#define IND_ATT                     "ind"
#define LINE_ATT                    "line"
#define COL_ATT                     "col"
#define PLOT_ATT                    "plot"
#define FORM_ATT                    "form"
#define LF_ATT                      "lf"
#define ADDCR_ATT                   "addcr"
#define TABS_ATT                    "tabs"
#define BACKSPACE_ATT               "backspace"
#define CR_ATT                      "cr"
#define CASE_ATT                    "case"
#define WRAP_ATT                    "wrap"
#define FONTINIT_ATT                "fontinit"
#define MODE_ATT                    "mode"
/* defines for attributes search */
/* These defines MUST be in coherence with ODM database attribute values */
#define YES_VALUE                   "yes"
#define NO_VALUE                    "no"

/* To return after clear */
#define ERR_RETURN(returnCode)      \
  free(printer_dds);                \
  printer_dds = NULL;               \
  return(returnCode);

/*
 * -----------------------------------------------------------------------------
 * structure for lprmod
 * -----------------------------------------------------------------------------
 */
/* Structure used to convert attribute value in a lprmode #define value */
struct convert_mode {
    char * att_name;        /* Attribute value to convert */
    int    yes_value;       /*  what to use if the value is yes */
    int    no_value;        /*  what to use if the value is no */
};

struct convert_mode valid_lprmod[] = {
    {PLOT_ATT,      PLOT,        0},
    {FORM_ATT,      0,           NOFF},
    {LF_ATT,        0,           NONL},
    {ADDCR_ATT,     0,           NOCL},
    {TABS_ATT,      NOTB,        0},
    {BACKSPACE_ATT, 0,           NOBS},
    {CR_ATT,        0,           NOCR},
    {CASE_ATT,      CAPS,        0},
    {WRAP_ATT,      WRAP,        0},
/* fontinit is not yet in ODM
    {FONTINIT_ATT,  FONTINIT,    0},
*/
    {MODE_ATT,      RPTERR,      0},
    {NULL,          0,           0}
};

/*
 * =============================================================================
 *                       LOCAL UTILITY ROUTINES
 * =============================================================================
 */
/*
 * -----------------------------------------------------------------------------
 *                       ODM_GET_VALUE
 * -----------------------------------------------------------------------------
 * 
 * This function gets the ODM value. This value is
 * checked with the attrval function.
 *
 * This routine is to be used only within this file.
 * The device specific routines for the various device
 * specific config methods  must not call this function.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
static int get_odm_value(logicalName, attName, uniqueType, returnInt)
char * logicalName;   /* logical name of device */
char * attName;       /* attribute name */
char * uniqueType;
int *  returnInt;     /* The found value */
{
    int    att_count;               /* attributes count */
    int    return_code;             /* return codes go here */
    struct CuAt * current_odm_att;  /* current found attribute */
    char   current_att_val[64];     /* Used with the attrval function */
    char   error_att_val[64];       /* Used with the attrval function */

    if ((current_odm_att = getattr(logicalName, attName,
                                   FALSE, &att_count)) != NULL) {
        DEBUG_2("get_odm_value: %s found = %s\n", attName,
                current_odm_att->value);
        /* Checking for availability */
        /* First, I build the string attr=value */
        strcpy(current_att_val, attName);
        strcat(current_att_val, "=");
        strcat(current_att_val, current_odm_att->value);
        if (attrval(uniqueType, current_att_val,
                    &error_att_val) > 0) {
            DEBUG_2("get_odm_value: %s value is invalid for %s\n",
                    current_odm_att->value, attName);
            *returnInt = 0;
            return_code = E_ATTRVAL;
        }
        else {
            *returnInt = atoi(current_odm_att->value);
            return_code = 0;
        } /* End if (attrval(...) > 0) */
    }
    else {
        DEBUG_1("get_odm_value: No %s attribute is found\n",
                attName);
        *returnInt = 0;
        return_code = E_NOATTR;
    } /* End if ((current_odm_att = getattr(...)) != NULL) */

    return(return_code);
} /* End static int get_odm_value(...) */

/*
 * =============================================================================
 *                       SPTRDDS
 * =============================================================================
 *
 * This function builds the DDS for the streams based sptr module.
 *
 * This function operates as a device dependent subroutine called 
 * by the tty configure method. It is used to build the dds which 
 * describes the characteristics of the printer to the sptr module.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */
int sptrdds(cusDevPtr, ddsPtr, ddsSize, majorNb, minorNb, onlyDevno)
struct CuDv * cusDevPtr;/* Customized object pointer */
uchar ** ddsPtr;        /* ptr to ptr to dds structure */
int *    ddsSize;       /* ptr to size fo dds */
long   majorNb, minorNb;          /* major and minor numbers */
int    onlyDevno;       /* Only devno and which_dds is updated? */
{
    int    att_count;               /* attributes count */
    int    return_code;             /* return codes go here */

    struct sptr_dds * printer_dds;  /* pointer to the dds structure */
    struct convert_mode * current_mode;

    /* ODM structures declarations */
    struct CuAt *  current_odm_att; /* current found attribute */

    /* ============================== */
    /* set up initial variable values */
    /* ============================== */
    if ((printer_dds = (struct sptr_dds *) malloc (sizeof(struct sptr_dds))) == NULL) {
        DEBUG_0 ("sptrdds: Malloc of dds failed\n");
        return(E_MALLOC);
    }
    else {
        bzero((char *)printer_dds, sizeof(struct sptr_dds));
    }

    /* ============== */
    /* Set DDS fields */
    /* ============== */

    /* ======== */
    /* DDS TYPE */
    /* ======== */
    printer_dds->which_dds = SPTR_DDS;

    /* ============= */
    /* Device number */
    /* ============= */
    printer_dds->devno = makedev(majorNb, minorNb);
	
	if (!onlyDevno) { /* If we really want the whole DDS */
		
		/* ================= */
		/* Margin parameters */
		/* ================= */
		/* IND_ATT */
		if (return_code = get_odm_value(cusDevPtr->name, IND_ATT,
										cusDevPtr->PdDvLn_Lvalue,
										&(printer_dds->plpst.ind))) {
			DEBUG_1("sptrdds: Invalid %s attribute\n", IND_ATT);
			ERR_RETURN(return_code);
		};
		/* LINE_ATT */
		if (return_code = get_odm_value(cusDevPtr->name, LINE_ATT,
										cusDevPtr->PdDvLn_Lvalue,
										&(printer_dds->plpst.line))) {
			DEBUG_1("sptrdds: Invalid %s attribute\n", LINE_ATT);
			ERR_RETURN(return_code);
		};
		/* COL_ATT */
		if (return_code = get_odm_value(cusDevPtr->name, COL_ATT,
										cusDevPtr->PdDvLn_Lvalue,
										&(printer_dds->plpst.col))) {
			DEBUG_1("sptrdds: Invalid %s attribute\n", COL_ATT);
			ERR_RETURN(return_code);
		};
		
		/* ================== */
		/* Timeout parameters */
		/* ================== */
		/* PTOP_ATT */
		if (return_code = get_odm_value(cusDevPtr->name, PTOP_ATT,
										cusDevPtr->PdDvLn_Lvalue,
										&(printer_dds->plptimer.v_timout))) {
			DEBUG_1("sptrdds: Invalid %s attribute\n", PTOP_ATT);
			ERR_RETURN(return_code);
		};
		
		/* ======================= */
		/* Printer mode parameters */
		/* ======================= */
		/* Each attribute is a switch YES_VALUE/NO_VALUE */
		/* The valid_lprmod structure (head of this file) is used */
		current_mode = valid_lprmod;
		while (current_mode->att_name != NULL) {
			if ((current_odm_att = getattr(cusDevPtr->name, current_mode->att_name,
										   FALSE, &att_count)) != NULL) {
				DEBUG_2("sptrdds: %s found = %s\n", current_mode->att_name,
						current_odm_att->value);
				if (!strcmp(current_odm_att->value, YES_VALUE)) {
					printer_dds->plpmod.modes |= current_mode->yes_value;
				}
				else if (!strcmp(current_odm_att->value, NO_VALUE)) {
					printer_dds->plpmod.modes |= current_mode->no_value;
				}
				else {
					DEBUG_2("sptrdds: %s value is unknown for %s attribut\n",
							current_odm_att->value, current_mode->att_name);
					ERR_RETURN(E_INVATTR);
				} /* End if (!strcmp(current_odm_att->value, ...)) */
				current_mode++;
			}
			else {
				DEBUG_1("sptrdds: No %s attribute is found\n",
						current_mode->att_name);
				ERR_RETURN(E_NOATTR);
			} /* End if ((current_odm_att = getattr(...)) != NULL) */
		} /* End while (current_mode->att_name != NULL) */
	}; /* End if (!onlyDevno) */

    DEBUG_0("sptrdds: printer_dds:\n");
	DEBUG_1("\twhich_dds  = %d\n", printer_dds->which_dds);
    DEBUG_1("\tdevno      = 0x%x\n", printer_dds->devno);
    DEBUG_0("\tplpst structure:\n");
    DEBUG_1("\t\tind      = 0x%x\n", printer_dds->plpst.ind);
    DEBUG_1("\t\tline     = 0x%x\n", printer_dds->plpst.line);
    DEBUG_1("\t\tcol      = 0x%x\n", printer_dds->plpst.col);
    DEBUG_0("\tplptimer structure:\n");
    DEBUG_1("\t\tvtimeout = 0x%x\n", printer_dds->plptimer.v_timout);
    DEBUG_0("\tplpmod structure:\n");
    DEBUG_1("\t\tmodes    = 0x%x\n", printer_dds->plpmod.modes);

    /* ========================== */
    /* Set argument output values */
    /* ========================== */
    *ddsPtr = (uchar *)printer_dds;
    *ddsSize = (int)sizeof(struct sptr_dds);

    /* That's OK */
    return(0);
} /* End int sptrdds(...) */
