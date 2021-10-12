static char sccsid[] = "@(#)39  1.2.2.1  src/bos/diag/tu/fddi/exectu.c, tu_fddi, bos41J, 9512A_all 3/2/95 10:24:09";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: dma_signal_handler
 *              sr_signal_table
 *              exectu
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************

Function(s) Exec TU for HTX Exerciser and Diagnostics

Module Name :  exectu.c
SCCS ID     :  1.12

Current Date:  5/23/90, 11:17:45
Newest Delta:  2/14/90, 17:56:36

Function called by both the Hardware Exercise, Manufacturing Application,
and Diagnostic Application to invoke a Test Unit (TU).

If the "mfg" member of the struct within TUTYPE is set to INVOKED_BY_HTX,
then exectu() is being invoked by the HTX Hardware Exerciser so
test units know to look at variables in TUTYPE for values from the
rule file.  Else, the test units use pre-defined values while testing.

*****************************************************************************/
#include <stdio.h>
#include <memory.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/intr.h>
#include <sys/err_rec.h>
#include "diagddfddiuser.h"
#include "fdditst.h"    /* note that this also includes hxihtx.h */

#ifdef debugg
extern void detrace();
#endif

#define SAVE_SIG        -135
#define RESTORE_SIG     -136

/*
 * Here, we declare a 'global' jumpbuf variable local only
 * to this file for handling user generated interrupts.
 * Another global area is declared for the original POS variable values.
 */
static jmp_buf ctxt;
unsigned char global_pos[10];
int save_pos = 1;

/*****************************************************************************

dma_signal_handler

On invocation (via interrupt), function jumps back into "exectu()" to setjmp
location.  NOTE THAT SETJMP() MUST HAVE BEEN CALLED PREVIOUS TO THIS
INVOCATION (as in "exectu()")!!!

*****************************************************************************/

void dma_signal_handler (sig_type)
   int sig_type;
   {
        void longjmp();
        longjmp(ctxt, 1);
   }

/*****************************************************************************

sr_signal_table

Based on "op", function saves/restores signals.

*****************************************************************************/

int sr_signal_table (op)
   int op;
   {
        static int signal_types[] =
           {
                SIGHUP, SIGINT, SIGQUIT, SIGALRM, SIGTERM
           };
        static void (*signal_table[sizeof(signal_types) / sizeof(int)])();
        static int num_sigs = sizeof(signal_types) / sizeof(int);
        int i;

        if (op == SAVE_SIG)
           {
                for (i = 0; i < num_sigs; i++)
                   {
                        signal_table[i] = signal(signal_types[i],
                                                dma_signal_handler);
                        if (signal_table[i] == -1)
                                return(SIG_SAV_ERR);
                   }
                return(0);
           }

        if (op == RESTORE_SIG)
           {
                for (i = 0; i < num_sigs; i++)
                   {
                        if (signal(signal_types[i], signal_table[i]) < 0)
                                return(SIG_RES_ERR);
                   }
                return(0);
           }
        return(SIG_OP_ERR);
   }

/*****************************************************************************

exectu

*****************************************************************************/

int exectu (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        register i, loop, tu;
        static int rc;
        static struct session_blk sess_s;
        struct status_block stat_s;
        long status;
        extern int start_fddi();
        extern int init_pos();
        extern int mktu_rc();
        extern int tu001();
        extern int tu002();
        extern int tu003();
        extern int tu004();
        extern int tu005();
        extern int tu006();
        extern int tu007();
        extern int tu008();
        extern int tu009();
        extern int tu010();
        extern int tu011();
        extern int tu012();
        extern int tu013();
        extern int tu014();
        extern int tu015();
        extern int tu016();
        extern int tu017();
        extern int tu018();
        extern int tu019();
        extern int tu020();
        extern int tu021();
        extern int tu022();
        extern int tu023();
        extern int tu024();
        extern int tu025();
        extern int tu026();
        extern int tu027();
        extern int tu028();
        extern int tu029();
        extern int tu030();
        extern int tu031();
        extern int tu032();
        extern int tu033();
        extern int tu034();
        extern int tu035();
        extern int tu036();
        extern int tu037();

        /*
         * First, save POS registers 2 thru 7 in locations
         * global_pos[0] thru global_pos[5].  This information
         * is needed after the adapter has been reset.
         * Clear out the Reset bit (of POS 2) and save new value.
         */
        if (save_pos)
        {

                if (rc = pos_save(global_pos, &status, tucb_ptr))
                {
#ifdef debugg
                        detrace(0,"Error saving POS Registers.");
#endif
                        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
                }
                global_pos[0] = global_pos[0] & MASK_POS2_CLEAR_RESET;
                save_pos = 0;
        }

        tu = tucb_ptr->header.tu;
        loop = tucb_ptr->header.loop;
        if (tucb_ptr->header.mfg != INVOKED_BY_HTX)
                tucb_ptr->fddi_s.htx_sp = NULL;

        /*
         * Save off our context here before setting
         * up our signals. If we get a user interrupt,
         * our interrupt handler (dma_signal_handler)
         * will longjmp to here with a '1'.
         */
        if (setjmp(ctxt))
           {
                /*
                 * Return error to user letting them know
                 * that we somehow were interrupted.
                 */
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, SIG_REC_ERR));
           }
        /*
         * Save off user's signals
         */
        if (rc = sr_signal_table(SAVE_SIG))
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
#ifdef debugg
        detrace (0,"\nRUNNING TU #%2d   ",tu);
#endif
        for (i = 0; i < loop; i++)
           {
                switch(tu)
                   {
                        case  1:
                                rc = tu001(fdes, tucb_ptr);
                                break;

                        case  2:
                                rc = tu002(fdes, tucb_ptr);
                                break;

                        case  3:
                                rc = tu003(fdes, tucb_ptr);
                                break;

                        case  4:
                                rc = tu004(fdes, tucb_ptr);
                                break;

                        case  5:
                                rc = tu005(fdes, tucb_ptr);
                                break;

                        case  6:
                                rc = tu006(fdes, tucb_ptr);
                                break;

                        case  7:
                                rc = tu007(fdes,tucb_ptr);
                                break;

                        case  8:
                                rc = tu008(fdes, tucb_ptr);
                                break;

                        case  9:
                                rc = tu009(fdes, tucb_ptr);
                                break;

                        case 10:
                                rc = tu010(fdes, tucb_ptr);
                                break;

                        case 11:
                                rc = tu011(fdes, tucb_ptr);
                                break;

                        case 12:
                                rc = tu012(fdes, tucb_ptr);
                                break;

                        case 13:
                                rc = tu013(fdes, tucb_ptr);
                                break;

                        case 14:
                                rc = tu014(fdes, tucb_ptr);
                                break;

                        case 15:
                                rc = tu015(fdes, tucb_ptr);
                                break;

                        case 16:
                                rc = tu016(fdes, tucb_ptr);
                                break;

                        case 17:
                                rc = tu017(fdes, tucb_ptr);
                                break;

                        case 18:
                                rc = tu018(fdes, tucb_ptr);
                                break;

                        case 19:
                                rc = tu019(fdes, tucb_ptr);
                                break;

                        case 20:
                                rc = tu020(fdes, tucb_ptr);
                                break;

                        case 21:
                                rc = tu021(fdes, tucb_ptr);
                                break;

                        case 22:
                                rc = tu022(fdes, tucb_ptr);
                                break;

                        case 23:
                                rc = tu023(fdes, tucb_ptr);
                                break;

                        case 24:
                                rc = tu024(fdes, tucb_ptr);
                                break;

                        case 25:
                                rc = tu025(fdes, tucb_ptr);
                                break;

                        case 26:
                                rc = tu026(fdes, tucb_ptr);
                                break;

                        case 27:
                                rc = tu027(fdes, tucb_ptr);
                                break;

                        case 28:
                                rc = tu028(fdes, tucb_ptr);
                                break;

                        case 29:
                                rc = tu029(fdes, tucb_ptr);
                                break;

                        case 30:
                                rc = tu030(fdes, tucb_ptr);
                                break;

                        case 31:
                                rc = tu031(fdes, tucb_ptr);
                                break;

                        case 32:
                                rc = tu032(fdes, tucb_ptr);
                                break;

                        case 33:
                                rc = tu033(fdes, tucb_ptr);
                                break;

                        case 34:
                                rc = tu034(fdes, tucb_ptr);
                                break;

                        case 35:
                                rc = tu035(fdes, tucb_ptr);
                                break;

                        case 36:
                                rc = tu036(fdes, tucb_ptr);
                                break;

                        case 37:
                                rc = tu037(&fdes, tucb_ptr);
                                break;

                        default:
                                (void) sr_signal_table(RESTORE_SIG);
                                return(ILLEGAL_TU_ERR);
                   };

                if (rc)
                   {
                        /*
                         * if running manuf. diagnostic and a tu returns
                         * an error, then break out of for-loop.
                         */
                        if (tucb_ptr->header.mfg != INVOKED_BY_HTX)
                                break;
                        /*
                         * check on retries keyword.  If set
                         * from rule file stanza, then
                         * continue loop to retry tu, else
                         * break out and return.
                         */
                        if (!(*tucb_ptr->fddi_s.retries))
                                break;
                   }
           } /* end for */

        /*
         * An error occurred in a TU or something,
         * return the original error code.
         */
        if (rc)
           {
                (void) sr_signal_table(RESTORE_SIG);
                return(rc);
           }

        if (rc = sr_signal_table(RESTORE_SIG))
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));

        return(rc);
   }
