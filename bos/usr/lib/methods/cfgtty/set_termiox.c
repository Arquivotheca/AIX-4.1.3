#ifndef lint
static char sccsid[] = "@(#)22 1.6 src/bos/usr/lib/methods/cfgtty/set_termiox.c, cfgtty, bos41J, 9520A_all 4/27/95 14:22:47";
#endif
/*
 * COMPONENT_NAME: (CFGTTY) Common function for DDS builds
 *
 * FUNCTIONS: set_termiox, in_termiox
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>          /* standard I/O */
#include <cf.h>             /* error messages */

#include <sys/cfgdb.h>      /* config #define */
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/cfgodm.h>     /* config structures */
#include <termios.h>        /* tcflag_t typedef */
#include <sys/termiox.h>    /* termiox structure */

#include "cfgdebug.h"
#include "ttycfg.h"

/*
 * ==============================================================================
 * defines and structures
 * ==============================================================================
 */
/*
 * -------------------
 * ODM attribute names
 * -------------------
 */
/* for termiox.x_sflag */
#define OPEN_DISP_ATT       "open_disp"

/*
 * -----------------------------------------------------------------------------
 * common structures for termiox flags
 * -----------------------------------------------------------------------------
 */
/* Structure which lists all attributes which are used */
/* to build the termiox structure. All 'XXX_ATT' defines are used */
/* Be careful: This structure must be updated if some new attributes */
/* ---------- are added or deleted in ODM database */
static char * termiox_odm_att[] = {
/* for termiox.x_sflag */
    OPEN_DISP_ATT,
/* for termiox.x_hflag */
    FLOW_DISP_ATT,
    NULL
  };

/*
 * -----------------------------------------------------------------------------
 * structure for termiox.x_sflag
 * x_sflag is only used now for open disciplines
 * -----------------------------------------------------------------------------
 */
/* Structure used to convert open discipline attribute values */
static struct convert convert_open_disp[] = {
    {"dtropen", DTR_OPEN},
    {"wtopen",  WT_OPEN},
    {"riopen",  RI_OPEN},
    {NULL,      0}
};

/* Structure used to find which sflag exist */
static struct which_attribute valid_sflag[] = {
    {OPEN_DISP_ATT, &convert_open_disp},
    {NULL,          NULL}
};

/*
 * -----------------------------------------------------------------------------
 * structure for termiox.x_hflag
 * x_hflag is only used now for flow disciplines
 * -----------------------------------------------------------------------------
 */
/* Structure used to convert flow discipline attribute values */
static struct convert convert_flow_disp[] = {
    {"dtr", DTRXOFF|CDXON},
    {"rts", RTSXOFF|CTSXON},
    {"rtsxoff", RTSXOFF},
    {"ctsxon",  CTSXON},
    {"dtrxoff", DTRXOFF},
    {"cdxon",   CDXON},
    {NULL,  0}
};

/* Structure used to find which hflag exist */
static struct which_attribute valid_hflag[] = {
    {FLOW_DISP_ATT, &convert_flow_disp},
    {NULL,          NULL}
};

/*
 * =============================================================================
 *                       SET_TERMIOX
 * =============================================================================
 * This function takes a pointer to a termiox structure
 * and uses ODM database to update it.
 *
 * Flow control discipline can be set with termiox or
 * termios structure. So, I need to drop a flag if
 * a flow control discipline (hflag) is found for termiox
 * structure.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */
int set_termiox(cusDevPtr, termioxPtr, attrList)
struct CuDv *    cusDevPtr;
struct termiox * termioxPtr;
struct attr *    attrList;
{
    extern int getatt();
    extern int set_flow_disp();

    int      att_value_found;    /* if attribute value is found */
    int      rflag_ind;          /* Only for DEBUG trace */
    int      return_code,count;
    char     current_value[ATTRVALSIZE];
    tcflag_t for_hflag;          /* To set_flow_disp call */

    struct which_attribute * current_att;   /* current attribute */
    struct convert *         current_convert;


    /* ============================== */
    /* set up initial variable values */
    /* ============================== */
    bzero((char *)termioxPtr, sizeof(struct termiox));
	for_hflag = 0;


    /* ================= */
    /* Set termiox sflag */
    /* ================= */
    /* I use valid_sflag structure to scan all valid sflag fields to set */
    /* and I use convert_table field to check available values. */
    /* It is required to find one available attribute value */
    return_code = 0;
    att_value_found = 1;
    current_att = valid_sflag;
    /* While there are attributes, they are found and available value is found */
    while ((current_att->att_name != NULL)
           && att_value_found
           && !(return_code = getatt(attrList,current_att->att_name,
                                     current_value, 's', &count))) {
            DEBUG_2("set_termiox: %s found = %s\n",
                    current_att->att_name, current_value);
            /* Check to see if some available value is found and */
            /* and use of corresponding termiox #define to set sflag */
            att_value_found = 0;
            current_convert = current_att->convert_table;
            /* While there are available values and none matches */
            while ((current_convert->att_value != NULL) && !att_value_found) {
                if (!strcmp(current_value, current_convert->att_value)) {
                    termioxPtr->x_sflag |= current_convert->define_value;
                    att_value_found = 1;
                    current_att++;
                }
                else {
                    current_convert++;
                }
            } /* End while ((... != NULL) && !att_value_found) */
    } /* End while ((current_att->att_name != NULL) && att_found) */

    /* Check for errors */
    if (!att_value_found) {
        DEBUG_1("set_termiox: Unavailable value for %s.\n", current_att->att_name);
        return(E_INVATTR);
    };
    if (return_code) {
        DEBUG_1("set_termiox: %s attribute not found.\n", current_att->att_name);
        return(return_code);
    };

    /* ================= */
    /* Set termiox hflag */
    /* ================= */
	/* I need to use 'for_hflag' variable because termios flags are 'tcflag_t' */
	/* and termiox flags are unsigned short. The 'set_flow_disp' function expects */
	/* tcflag_t pointer */
    if (return_code = set_flow_disp(cusDevPtr, &for_hflag, attrList,
                                    &valid_hflag)) {
        DEBUG_1("set_termiox: set_flow_disp failed with ODM error %d\n",
                return_code);
        return(return_code);
    }
    else {
        DEBUG_1("set_termiox: set_flow_disp succeded with hflag=%d\n",
                for_hflag);
        termioxPtr->x_hflag = (unsigned short) for_hflag;
    }

    DEBUG_0("set_termiox: Dump of the built termiox structure\n");
    DEBUG_1("\tx_hflag = 0x%x\n", termioxPtr->x_hflag);
    DEBUG_4("(RTSXOFF = 0x%x, CTSXON = 0x%x, DTRXOFF = 0x%x, CDXON = 0x%x)\n",
           RTSXOFF, CTSXON, DTRXOFF, CDXON);
    DEBUG_1("\tx_cflag = 0x%x\n", termioxPtr->x_cflag);
    for (rflag_ind = 0; rflag_ind < NFF; rflag_ind++) {
        DEBUG_2("\tx_rflag[%d] = 0x%x\n",
                rflag_ind, termioxPtr->x_rflag[rflag_ind]);
    }
    DEBUG_1("\tx_sflag = 0x%x\n", termioxPtr->x_sflag);
    DEBUG_3("(DTR_OPEN = 0x%x, WT_OPEN = 0x%x, RI_OPEN = 0x%x)\n",
           DTR_OPEN, WT_OPEN, RI_OPEN);



    return(0);
} /* End int set_termiox(...) */
/*
 * =============================================================================
 *                       IN_TERMIOX
 * =============================================================================
 * This function takes a pointer to an attribute list.
 * It searches if at least one attribute of this list concerns the termiox structure.
 *
 * Return code: 0 if no attribute is to be used the termiox structure, 1 otherwise.
 * =============================================================================
 */
int in_termiox(attrList)
struct attr *    attrList;
{
    char ** current_termiox_att;

    current_termiox_att = termiox_odm_att;
    while (*current_termiox_att != NULL) {
        if (att_changed(attrList, *current_termiox_att) != NULL) {
			DEBUG_1("in_termiox: %s attribute changed\n",
					*current_termiox_att);
            return(1);
        };
		current_termiox_att++;
    }
return(0);
} /* End int in_termiox(...) */
