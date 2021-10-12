#ifndef lint
static char sccsid[] = "@(#)01 1.4 src/bos/kernext/tty/POWER/common_open.c, sysxcommon, bos411, 9435A411a 8/20/94 11:50:40";
#endif
/*
 * COMPONENT_NAME: (sysxtty) Open disciplines for streams driver
 *
 * FUNCTIONS: local_empty,
 * openDisc_open, openDisc_close, openDisc_input,
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <sys/device.h>                     /* for config stuff */
#include <sys/errno.h>                      /* error codes */
#include <sys/intr.h>                       /* interrupt handler stuff */
#include <sys/malloc.h>                     /* malloc services */
#include <sys/pin.h>                        /* pinning stuff */
#include <sys/sleep.h>                      /* EVENT_NULL */
#include <sys/str_tty.h>
#include "ttydrv.h"
#include "common_open.h"                       /* enum openDiscType */

/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */
#define local /* static */
/*
 * ============================================================================
 * Functions declarations
 * ============================================================================
 */
/* Common functions */
local int local_empty();

/* DTRO_OPEN open discipline functions */
extern int dtro_open(caddr_t *retrieve, caddr_t driverStruct,
                    int (* ddservice) (), int status, int mode, int isFirst,
                    caddr_t eventAddr);
extern int dtro_close(caddr_t retrieve, int hupcl);
extern int dtro_input(caddr_t retrieve, caddr_t eventAddr, char receivedChar,
                     enum status receivedStatus);

/* WTO_OPEN open discipline functions */
extern int wto_open(caddr_t *retrieve, caddr_t driverStruct,
                    int (* ddservice) (), int status, int mode, int isFirst,
                    caddr_t eventAddr);
extern int wto_close(caddr_t retrieve);
extern int wto_input(caddr_t retrieve, caddr_t eventAddr, char receivedChar,
                     enum status receivedStatus);

/*
 * ============================================================================
 * Global variables
 * ============================================================================
 */

local struct openDisc_entry openDisc_sw[] = {
    /* DTRO_OPEN */     {"dtropen", dtro_open, dtro_close, dtro_input,
                                local_empty, local_empty},
    /* WTO_OPEN */      {"wtopen", wto_open, wto_close, wto_input,
                                local_empty, local_empty},
    /* OTHER_OPEN */    {"", local_empty, local_empty, local_empty,
                                local_empty, local_empty}
                };

/*
 * -----------------------------------------------------------------------------
 *                       LOCAL_EMPTY
 * -----------------------------------------------------------------------------
 * To do nothing.
 *
 * Return code: Always 0.
 *
 * -----------------------------------------------------------------------------
 */
local int local_empty()
{
    return(0);
} /* End local int local_empty() */


/*
 * =============================================================================
 *                       EXTERNAL INTERFACE OPEN DISCIPLINE FUNCTIONS
 * =============================================================================
 * External interface open discipline functions are always prefixed with "openDisc__":
 * openDisc_open, openDisc_close, openDisc_input.
 *
 * Return code: 0 if OK, errno error code otherwise.
 *
 * =============================================================================
 */
/*
 * -----------------------------------------------------------------------------
 *                       OPENDISC_OPEN
 * -----------------------------------------------------------------------------
 * First this function checks type availabity and then calls corresponding
 * open discipline open routine.
 * Space allocation and pin are made in each specific open discipline
 * open routine.
 * Each open discipline open function MUST update "retrieve" output parameter.
 *
 * Return code: 0 if OK, errno error code otherwise.
 *
 * -----------------------------------------------------------------------------
 */
int openDisc_open(caddr_t * retrieve, caddr_t driverStruct, enum openDiscType type,
                  int (* ddservice) (), int status, int mode, int isFirst, caddr_t eventAddr)
{
    switch (type) {
      case DTRO_OPEN:
        return(openDisc_sw[type].open(retrieve, driverStruct, ddservice,
                                      status, mode, isFirst, eventAddr));
      case WTO_OPEN:
        return(openDisc_sw[type].open(retrieve, driverStruct, ddservice,
                                      status, mode, isFirst, eventAddr));

      case OTHER_OPEN:
      default:
        return(EINVAL);
    }; /* End switch (type) */
} /* End int openDisc_open(...) */

/*
 * -----------------------------------------------------------------------------
 *                       OPENDISC_CLOSE
 * -----------------------------------------------------------------------------
 * Calls corresponding open discipline close routine.
 * Space desallocation and unpin are made in each specific open discipline
 * close routine.
 *
 * Return code: 0 if OK, errno error code otherwise.
 *
 * -----------------------------------------------------------------------------
 */
int openDisc_close(caddr_t retrieve, int hupcl)
{
    return(openDisc_sw[*(int *)retrieve].close(retrieve, hupcl));
} /* End int openDisc_close(...) */

/*
 * -----------------------------------------------------------------------------
 *                       OPENDISC_INPUT
 * -----------------------------------------------------------------------------
 * Calls corresponding open discipline input routine.
 *
 * Return code: 0 if OK, errno error code otherwise.
 *
 * -----------------------------------------------------------------------------
 */
int openDisc_input(caddr_t retrieve, caddr_t eventAddr, char receivedChar, enum status receivedStatus)
{
    return(openDisc_sw[*(int *)retrieve].input(retrieve, eventAddr, receivedChar, receivedStatus));
} /* End int openDisc_input(...) */

