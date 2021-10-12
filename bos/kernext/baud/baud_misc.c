static char sccsid[] = "@(#)45  1.2  baud_misc.c, bos, bos320 4/14/93 16:40:52";
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
** FUNCTIONS: baud_cdt_func      baud_timer_func      bld_err_msg
**
** CONTENTS: This is where the master control structure, etc. is defined.
**
** ORIGINS: ME
*/

#include <sys/device.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/malloc.h>
#include <sys/adspace.h>
#include <sys/sleep.h>
#include <sys/ioacc.h>
#include <sys/dump.h>
#include <sys/watchdog.h>
#include <sys/m_param.h>
#include <sys/timer.h>
#include <sys/trcmacros.h>
#include <sys/err_rec.h>
#include "bauddd.h"

/*
** This is where all of the buffers, stack save areas for exception handling,
** the master control structure and dump component table are defined.
*/

char global_buf[512];
label_t stk_save;
baud_ctrl_t baud_ctrl;
struct baud_cdt_tab *baud_cdt = NULL;

/* I chose to make my own error reporting structure instead of using one
** from the file, /usr/include/sys/err_rec.h
*/

typedef struct fdderr {
    unsigned error_id;
    char     resource_name[ERR_NAMESIZE];
    ulong detail_data[4];
} fdderr_t;

/*
** This is where the dump table gets initialized.
*/

struct cdt *
baud_cdt_func(arg)
int arg;
{
    int size;

    if (arg == 1)  {
        /*
        ** Initialize the head of the dump table structure.  Rest of
        ** what gets dumped comes later.
        */

        baud_cdt->baud_cdt_head._cdt_magic = DMP_MAGIC;
        strcpy(baud_cdt->baud_cdt_head._cdt_name, "Fake DD");
        baud_cdt->baud_entry.d_segval = NULL;
        strcpy(baud_cdt->baud_entry.d_name, "FakeDD");
        baud_cdt->baud_entry.d_ptr = (char *) &baud_ctrl;
        baud_cdt->baud_entry.d_len = sizeof(baud_ctrl_t);
        size = sizeof(struct cdt_head) + sizeof(struct cdt_entry);

        baud_cdt->baud_cdt_head._cdt_len = size;
    }

    if (arg == 2)  {
        DEBUG1("Kernel has finished dumping my stuff\n");
    }
    return ((struct cdt *) baud_cdt);
}


/*
** This routine can be used to wake up the process if a finer granularity
** than 1 second is required.
*/

void
baud_timer_func(systimer)
struct trb *systimer;
{
    struct baud_control *fcp;

    /*
    ** When this timer we only know indirectly which device had its timer
    ** pop.  Remember each adapter maintains its own baud_control sturcture.
    ** Since the timer structure contains the address of where the control
    ** structure is located we need to obtain it.
    */

    fcp = (struct baud_control *) systimer->t_func_data;
    e_wakeup(&(fcp->sleep_anchor));

    return;
}


/*
** This routine is used to build the error message that will be used
** by errsave to record errors.
*/

snd_err_msg(error_id, fcp, dat1, dat2, dat3, dat4)
ulong error_id;
struct baud_control *fcp;
ulong dat1, dat2, dat3, dat4;
{

    fdderr_t errbuf;

    bzero(&errbuf, sizeof(errbuf));
    errbuf.error_id = error_id;
    bcopy(fcp->fdds.lname, errbuf.resource_name, strlen(fcp->fdds.lname));
    errbuf.detail_data[0] = dat1;
    errbuf.detail_data[1] = dat2;
    errbuf.detail_data[2] = dat3;
    errbuf.detail_data[3] = dat4;

    errsave(&errbuf, (uint)sizeof(errbuf)); /* errsave saves it in /dev/error */
}

