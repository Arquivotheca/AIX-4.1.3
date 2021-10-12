static char sccsid[] = "@(#)66	1.6  src/bos/diag/tu/iop/ioptu.c, tu_iop, bos41J, 9513A_all 3/9/95 09:01:51";
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: exectu
 *              intr_handler
 *
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 *
 */



#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/devinfo.h>
#include "sys/mdio.h"

#include "address.h"
#include "tu_type.h"

extern int exectu() ;
extern int ioptu_01();
extern int ioptu_02();
extern int ioptu_04();
extern int ioptu_05();
extern int ioptu_06();
extern int ioptu_08();
extern int ioptu_09();
extern int ioptu_10();
extern int ioptu_11();
extern int ioptu_12();
extern int ioptu_13();
extern int ioptu_16();
extern int ioptu_17();
extern int ioptu_20();
extern int start();             /* todtest101.c */
extern int suspend();           /* todtest102.c */
extern int interrupt();         /* todtest103.c */
extern int osc_fail();          /* todtest104.c */
extern int periodic();          /* todtest105.c */
extern int todnvram();          /* todtest106.c */
extern int save_enable();       /* todtest107.c */
extern int leap_year();         /* todtest108.c */
extern int hour12();            /* todtest109.c */
extern int clock_stop();        /* todtest110.c */
extern int low_battery();       /* todtest111.c */
extern int tod_irq_stat();      /* todtest112.c */
extern int pf_irq();            /* todtest113.c */
extern int elapsed();           /* todtest114.c */
extern void fdebug_tod();

FILE *dbg_fd;	/* saved for future debug purposes */

#ifdef SEMAPHORE_CODE
extern int      get_sem_tod();
extern int      release_sem_tod();
#endif /* SEMAPHORE_CODE */

void intr_handler();
extern void     clear_tod_flags();
extern void     save_tod_regs();
extern void     restore_tod_regs();

 /* Flag for each TOD register */
char tod_reg_flags[NUM_TOD_FLAGS];
/* array of saved TOD registers */
uchar  tod_reg_array[NUM_TOD_REGS];

static int first_time = 1;   /* Set to true for first time. */
                             /* At the end of for loop set  */
                             /* to (0) False.               */
static int First = 1;        /* Set to true for first time, used by TU 16,17 */

/*
 * NAME: int exectu( fildes , p )
 *
 * FUNCTION:  Execute the Test Unit Passed in _t *p.tu
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS: Error Code or Zero (0) (no error found)
 */



 int exectu( fildes , p )
 int fildes ;
 TUTYPE *p;
 {
        int err = 0 ;                       /* return code    */
        long i;
        char save_area[16];                 /* save area for tu4 and tu5 */
        char saved_data[32];                /* save area for tu16 and tu17 */

        clear_tod_flags();      /*  Clear TOD flags and reg array */


        /* Open TOD error file */


/*  This code is the loop that controls how  many times each TU
    is exectued.  The change was made to be compatible with how
    all the other exectu's are written.

    This was to insure easy reading by future programmers. */

        for (i = 0; i < p->header.loop; i++) {

                switch ( p->header.tu ) {

               /* * * * * * * *  * * * * * * * * * * * * * * * * * * * * *  */
               /*  TU 01 first reads and saves the values in NVRAM addr 300 */
               /*   and 301. It then writes values to NVRAM 300 thru 303,   */
               /*   reads them back and  tests them for correct values.     */
               /*  At the end of the test, it restores the original values. */
               /*  If a miscompare occurs, the values in NVRAM ( and in the */
               /*    leds) is unknown....                                   */
               /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

                case 1:                                  /* TU 0001  */
                        err = ioptu_01(fildes, p);
                        break;

               /* * * * * * * * * * * * * * * * * * * * * ** * * * * * * *  */
               /*  TU 02 first reads and saves the values in TOD locations  */
               /*    00DE & 00D.                                            */
               /*  It then writes values to those locations,  reads them    */
               /*  back and tests them for correct values.                  */
               /*  It then restores original values.                        */
               /*  If miscompare occurs, values in TOD RAM may be different */
               /*  from original .                                          */
               /* * * * * * * * * * * * * * * * * * * * * * * * * * * * *   */

                case 2:

#ifdef SEMAPHORE_CODE
                        /*
                         * Get TOD semaphore before entering code.
                         *
                         * First, COOPERATIVELY seek access to TOD memory
                         * space.  We'll wait FOREVER until we get it!
                         */
                        if (get_sem_tod(-1))
                        {
                                return(TOD_BUSY_ERR);
                        }
#endif /* SEMAPHORE_CODE */

                        err = ioptu_02(fildes, p);
                        break;

               /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
               /*  The first time here, TU 04 reads and saves values in     */
               /*  NVRAM 300-301                                            */
               /*  It then writes 666 to leds and returns.                  */
               /*  Subsequent times here it writes 999 to leds and returns. */
               /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

                case 4:                  /* TU 0004  */

                        err = ioptu_04(fildes, &first_time, save_area, p);
                        break;

                /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
                /*  TU 05 restores the led values if saved in TU04.        */
                /*  TU04 must have been executed before this TU or rc = 20 */
                /*  will be returned.                                      */
                /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

                case 5:                  /* TU 0005  */
                        err = ioptu_05(fildes, &first_time, save_area, p);
                        break;


                /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
                /*  TU 06 reads the EC level register and returns its value  */
                /*  in the return code.                                      */
                /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

                case 6:                  /* TU 0006  */

                        err = ioptu_06(fildes, p);
                        break;

                /* * * * * * * ** * * * * * * * * * * * * * * * * * * * * * */
                /*  TU 08 reads the first byte of power status register and */
                /*   returns its value in the return code.                  */
                /*  * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

                case 8:            /* TU 0008  */

                        err = ioptu_08(fildes, p);
                        break;

                /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
                /*  TU 09 reads the second byte of power status register and */
                /*   returns its value in the return code.                   */
                /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

                case 9:            /* TU 0009  */

                        err = ioptu_09(fildes, p);
                        break;

                /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
                /*  TU 10 reads the keylock byte of power status register    */
                /*  and returns its value in the return code.                */
                /* * * * * * * * * * * * * * * * * * * * * * * * * * * *   * */

                case 10:            /* TU 0010  */


                        err = ioptu_10(fildes, p);
                        break;

                /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
                /*  TU 11 tests for battery low signal from TOD.           */
                /*  If low, sends error = 12 to diag app.                  */
                /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

                case 11:
#ifdef SEMAPHORE_CODE
                        /*
                         * Get TOD semaphore before entering code.
                         *
                         * First, COOPERATIVELY seek access to TOD memory
                         * space.  We'll wait FOREVER until we get it!
                         */
                        if (get_sem_tod(-1))
                        {
                                return(TOD_BUSY_ERR);
                        }
#endif /* SEMAPHORE_CODE */

                        err = ioptu_11(fildes, p);
                        break;

                /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
                /*  TU 12 tests for TOD at POR state. If in that state, send */
                /*  error = 10 to diag app.                                  */
                /*  Then test for TOD running. If not, send error = 13 to    */
                /*  diag app.                                                */
                /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

                 case 12:

#ifdef SEMAPHORE_CODE
                        /*
                         * Get TOD semaphore before entering code.
                         *
                         * First, COOPERATIVELY seek access to TOD memory
                         * space.  We'll wait FOREVER until we get it!
                         */
                        if (get_sem_tod(-1))
                        {
                                return(TOD_BUSY_ERR);
                        }
#endif /* SEMAPHORE_CODE */

                        err = ioptu_12(fildes, p);
                        break;

                /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
                /*  TU 13 sets up TOD for Jan 1, 1989 8 AM . This TU may be  */
                /*  run if TU 12 sent back a return code of 10.              */
                /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

                case 13:

                        err = ioptu_13(fildes, p);
                        break;

		/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
		/*  The first time here, TU 16 reads and saves values in     */
		/*  the LCD registers                                        */
		/*  It then writes *U*U*U to LCD and returns.                */
		/*  Subsequent times here it writes VAS to LCD and returns.  */
		/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
		case 16 :
			err = ioptu_16(fildes, &First, saved_data, p);
			break;

		/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
		/*  TU 17 restores the LCD values if saved in TU16.        */
		/*  TU16 must have been executed before this TU or rc = 20 */
		/*  will be returned.                                      */
		/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
		case 17 :
			err = ioptu_17(fildes, &First, saved_data, p);
			break;

		/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
		/*  TU 20 checks the power status table of Pegasus         */
		/*  expansion cabinets                                     */
		/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
                case 20:
                        err = ioptu_20(fildes, p);
                        break;


                case START_TEST:
#ifdef SEMAPHORE_CODE
                        /*
                         * Get TOD semaphore before entering code.
                         *
                         * First, COOPERATIVELY seek access to TOD memory
                         * space.  We'll wait FOREVER until we get it!
                         */
                        if (get_sem_tod(-1))
                        {
                                return(TOD_BUSY_ERR);
                        }
#endif /* SEMAPHORE_CODE */

                        err = start(fildes, p);         /* TU 101 */
                        break;

                case SUSP_TEST:
#ifdef SEMAPHORE_CODE
                        /*
                         * Get TOD semaphore before entering code.
                         *
                         * First, COOPERATIVELY seek access to TOD memory
                         * space.  We'll wait FOREVER until we get it!
                         */
                        if (get_sem_tod(-1))
                        {
                                return(TOD_BUSY_ERR);
                        }
#endif /* SEMAPHORE_CODE */

                        err = suspend(fildes, p);       /* TU 102 */
                        break;

                case INTR_TEST:
#ifdef SEMAPHORE_CODE
                        /*
                         * Get TOD semaphore before entering code.
                         *
                         * First, COOPERATIVELY seek access to TOD memory
                         * space.  We'll wait FOREVER until we get it!
                         */
                        if (get_sem_tod(-1))
                        {
                                return(TOD_BUSY_ERR);
                        }
#endif /* SEMAPHORE_CODE */

                        err = interrupt(fildes, p)      /* TU 103 */;
                        break;

                case OSC_TEST:
#ifdef SEMAPHORE_CODE
                        /*
                         * Get TOD semaphore before entering code.
                         *
                         * First, COOPERATIVELY seek access to TOD memory
                         * space.  We'll wait FOREVER until we get it!
                         */
                        if (get_sem_tod(-1))
                        {
                                return(TOD_BUSY_ERR);
                        }
#endif /* SEMAPHORE_CODE */

                        err = osc_fail(fildes, p);      /* TU 104 */
                        break;

                case PERIODIC:
#ifdef SEMAPHORE_CODE
                        /*
                         * Get TOD semaphore before entering code.
                         *
                         * First, COOPERATIVELY seek access to TOD memory
                         * space.  We'll wait FOREVER until we get it!
                         */
                        if (get_sem_tod(-1))
                        {
                                return(TOD_BUSY_ERR);
                        }
#endif /* SEMAPHORE_CODE */

                        err = periodic(fildes, p);      /* TU 105 */
                        break;

                case TODNVRAM:
#ifdef SEMAPHORE_CODE
                        /*
                         * Get TOD semaphore before entering code.
                         *
                         * First, COOPERATIVELY seek access to TOD memory
                         * space.  We'll wait FOREVER until we get it!
                         */
                        if (get_sem_tod(-1))
                        {
                                return(TOD_BUSY_ERR);
                        }
#endif /* SEMAPHORE_CODE */

                        err = todnvram(fildes, p);      /* TU 106 */
                        break;

                case SAVE_ENABLE:
#ifdef SEMAPHORE_CODE
                        /*
                         * Get TOD semaphore before entering code.
                         *
                         * First, COOPERATIVELY seek access to TOD memory
                         * space.  We'll wait FOREVER until we get it!
                         */
                        if (get_sem_tod(-1))
                        {
                                return(TOD_BUSY_ERR);
                        }
#endif /* SEMAPHORE_CODE */

                        err = save_enable(fildes, p);   /* TU 107 */
                        break;

                case LEAP_YEAR:
#ifdef SEMAPHORE_CODE
                        /*
                         * Get TOD semaphore before entering code.
                         *
                         * First, COOPERATIVELY seek access to TOD memory
                         * space.  We'll wait FOREVER until we get it!
                         */
                        if (get_sem_tod(-1))
                        {
                                return(TOD_BUSY_ERR);
                        }
#endif /* SEMAPHORE_CODE */

                        err = leap_year(fildes, p);     /* TU 108 */
                        break;

                case HOUR12:
#ifdef SEMAPHORE_CODE
                        /*
                         * Get TOD semaphore before entering code.
                         *
                         * First, COOPERATIVELY seek access to TOD memory
                         * space.  We'll wait FOREVER until we get it!
                         */
                        if (get_sem_tod(-1))
                        {
                                return(TOD_BUSY_ERR);
                        }
#endif /* SEMAPHORE_CODE */

                        err = hour12(fildes, p);        /* TU 109 */
                        break;

                case CLOCK_STOP:
#ifdef SEMAPHORE_CODE
                        /*
                         * Get TOD semaphore before entering code.
                         *
                         * First, COOPERATIVELY seek access to TOD memory
                         * space.  We'll wait FOREVER until we get it!
                         */
                        if (get_sem_tod(-1))
                        {
                                return(TOD_BUSY_ERR);
                        }
#endif /* SEMAPHORE_CODE */

                        err = clock_stop(fildes, p);    /* TU 110 */
                        break;

                case LOW_BATTERY:
#ifdef SEMAPHORE_CODE
                        /*
                         * Get TOD semaphore before entering code.
                         *
                         * First, COOPERATIVELY seek access to TOD memory
                         * space.  We'll wait FOREVER until we get it!
                         */
                        if (get_sem_tod(-1))
                        {
                                return(TOD_BUSY_ERR);
                        }
#endif /* SEMAPHORE_CODE */

                        err = low_battery(fildes, p);   /* TU 111 */
                        break;

                case TOD_IRQ_STAT:
#ifdef SEMAPHORE_CODE
                        /*
                         * Get TOD semaphore before entering code.
                         *
                         * First, COOPERATIVELY seek access to TOD memory
                         * space.  We'll wait FOREVER until we get it!
                         */
                        if (get_sem_tod(-1))
                        {
                                return(TOD_BUSY_ERR);
                        }
#endif /* SEMAPHORE_CODE */

                        err = tod_irq_stat(fildes, p);  /* TU 112 */
                        break;

                case PF_IRQ:
#ifdef SEMAPHORE_CODE
                        /*
                         * Get TOD semaphore before entering code.
                         *
                         * First, COOPERATIVELY seek access to TOD memory
                         * space.  We'll wait FOREVER until we get it!
                         */
                        if (get_sem_tod(-1))
                        {
                                return(TOD_BUSY_ERR);
                        }
#endif /* SEMAPHORE_CODE */

                        err = pf_irq(fildes, p);        /* TU 113 */
                        break;

                case ELAPSED:
#ifdef SEMAPHORE_CODE
                        /*
                         * Get TOD semaphore before entering code.
                         *
                         * First, COOPERATIVELY seek access to TOD memory
                         * space.  We'll wait FOREVER until we get it!
                         */
                        if (get_sem_tod(-1))
                        {
                                return(TOD_BUSY_ERR);
                        }
#endif /* SEMAPHORE_CODE */

                        err = elapsed(fildes, p);       /* TU 114 */
                        break;

                case DEBUG_TOD:
#ifdef SEMAPHORE_CODE
                        /*
                         * Get TOD semaphore before entering code.
                         *
                         * First, COOPERATIVELY seek access to TOD memory
                         * space.  We'll wait FOREVER until we get it!
                         */
                        if (get_sem_tod(-1))
                        {
                                return(TOD_BUSY_ERR);
                        }
#endif /* SEMAPHORE_CODE */
                        /* This is not a TU - it's prints out the present
                           TOD register values */
                        err = debug_tod(fildes, p);       /* TU 99 */
                        break;

                case INTR_HANDLER:
                                        /* interrupt handler */
                        intr_handler(fildes, p);
                        break;

                default:                    /* invalid TU number  */
                        err = 256 ;
                        break ;
                } /* endswitch */

                /* If TOD flags have been saved, restore */
                if (check_tod_flags())
                        restore_tod_regs(fildes, p);

#ifdef SEMAPHORE_CODE
                /*
                 * Release the held TOD semaphore.
                 */
                (void) release_sem_tod();
#endif


                if (err)
                        break;          /* get out of for loop */

        }  /* For loop */

        return(err) ;
 }   /* end exectu() */


/*
 * NAME:  void intr_hansler(fdesc, tucb_ptr)
 *
 * FUNCTION:  Release the save TOD registers.
 *
 * EXECUTION:
 *
 *
 * RETURNS:  void
 */


void intr_handler(fdesc, tucb_ptr)
int fdesc;
TUTYPE *tucb_ptr;
{

        if (tod_reg_array[F_SAVE_COMPLETE]) {
                restore_tod_regs(fdesc, tucb_ptr);
        }

        /* Release the held TOD semaphore */
        (void) release_sem_tod();

}

