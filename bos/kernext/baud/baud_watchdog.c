static char sccsid[] = "@(#)45  1.2  baud_watchdog.c, bos, bos320 4/14/93 16:40:52";
/*
**            NOTICE TO USERS OF THE SOURCE CODE EXAMPLES
**
** THE SOURCE CODE EXAMPLES PROVIDED BY IBM ARE ONLY INTENDED TO ASSIST IN THE
** DEVELOPMENT OF A WORKING SOFTWARE PROGRAM.
**
** INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
** EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
** WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
** LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
** PFAKEULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
** OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
** IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
** DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
** DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
** CORRECTION.
**
** IBM does not warrant that the contents of the source code examples, whether
** individually or as one or more groups, will meet your requirements or that
** the source code examples are error-free.
**
** IBM may make improvements and/or changes in the source code examples at
** any time.
**
** Changes may be made periodically to the information in the source code
** examples; these changes may be reported, for the sample device drivers
** included herein, in new editions of the examples.
**
** References in the source code examples to IBM products, programs, or
** services do not imply that IBM intends to make these available in all
** countries in which IBM operates.  Any reference to an IBM licensed
** program in the source code examples is not intended to state or imply
** that only IBM's licensed program may be used.  Any functionally equivalent
** program may be used.
**
** RISC System/6000 is a trademark of International Business Machines
** Corporation.
**
** COMPONENT_NAME: FAKE Device Driver Bottom Half Routine
**
** FUNCTIONS: baud_watchdog
**
** ORIGINS: ME
*/

#include <sys/device.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/devinfo.h>
#include <sys/uio.h>
#include <sys/malloc.h>
#include <sys/adspace.h>
#include <sys/sleep.h>
#include <sys/ioacc.h>
#include <sys/dump.h>
#include <sys/watchdog.h>
#include <sys/m_except.h>
#include <sys/m_param.h>
#include <sys/timer.h>
#include <sys/trcmacros.h>
#include "bauddd.h"

extern label_t stk_save;
extern char global_buf[512];

/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * NAME: baud_watchdog
 * This is the baud_watchdog service routine.  It is called from timer
 * interrupt context (INTTIMER).
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

void
baud_watchdog(watchdog_ptr)
struct watchdog *watchdog_ptr;    /* Pointer to PINNED watchdog structure */
{
    int rc = 0;
    struct baud_control *fcp;

    DEBUG1("============ Entering Fake Watchdog Routine ==============\n");

    /*
    ** When this timer pops we only know indirectly which device had its timer
    ** pop.  Remember each adapter maintains its own baud_control sturcture.
    ** Since the watchdog timer structure is the second element of this
    ** structure and we are passed the address of this structure we will
    ** use it to get to the base of the control structure.
    */

    fcp = (struct baud_control *) ((ulong) watchdog_ptr - sizeof(struct intr));

    FAKEHKWD1(HKWD_FAKE_DD, FAKE_ENTRY_WATCHDOG, 0, fcp->sleep_anchor);

    /*
    ** We could have an exception occur here so register an exception
    ** handler.  This xmemout routine is going to move the data for us.
    */

    if ((rc = setjmpx(&stk_save)) == 0) {             /* No exception yet!! */
        fcp->xfer_error = xmemout(global_buf, fcp->cxfer_buf.bufaddr,
                                    fcp->cxfer_buf.buflen,
                                    &(fcp->cxfer_buf.dp));
        clrjmpx(&stk_save);
    }
    else if (rc == EXCEPT_TRAP) {         /* Illegal trap instruction */
        DEBUG1("FAKEDD: baud_watchdog: SCREW UP BUDDY!!!\n");
    }
    else {
        DEBUG2("FAKEDD: baud_watchdog: Not our exception: %d\n", rc);
        longjmpx(rc);
    }

    DEBUG1("FAKEDD: baud_watchdog: Getting ready to wakeup process.\n");
    e_wakeup(&(fcp->sleep_anchor));

    DEBUG1("============ Leaving Fake Watchdog Routine ===============\n");
    FAKEHKWD1(HKWD_FAKE_DD,FAKE_EXIT_WATCHDOG,0,fcp->xfer_error);
    return;
}
