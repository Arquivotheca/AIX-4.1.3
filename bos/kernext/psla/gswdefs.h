/* @(#)09  1.13  src/bos/kernext/psla/gswdefs.h, sysxpsla, bos41J, 9519A_all 5/8/95 09:55:03 */
/*
 *   COMPONENT_NAME: SYSXPSLA
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/************************************************************************/
/* PURPOSE:     Device driver defines that are used                     */
/*              when compiling the driver, not available to the user.   */
/*                                                                      */
/*;bb 022890    Using intr_priority for G_DISABLE, not INTCLASS1        */
/*;bb 030590    TEST ..... try using e_sleep instead of e_sleepl.       */
/*;bb 030590    TEST ..... null out FREE_LOCK and add UN_LOCKL.         */
/*;bb 032090    Cleanup sleep and lock macros. No longer doing sleepl,  */
/*              removed FREE_LOCK.                                      */
/*;bb 050990    Add 'setjmpx/clrjmpx' for handling PIO PARITY errors.   */
/* fp 030895    JCC marks 3.2 to 4.1 port                               */
/*                                                                      */
/************************************************************************/

/*********#define BUS_ID 0x82000020****************/
#define BUS_ID bus_id

#ifndef NULL
#define NULL       0
#endif


/*----------------------------------------------------------------------*/
/* PRINT controls the use of printf messages for normal flow of driver. */
/* PRNTE controls the use of printf messages for error conditions.      */
/* Redefine as follows if needed for debugging,			        */
/*   else, leave as null define for normal use.                         */
/* #define PRNTE(a)	printf a			                */
/* #define PRINT(a)	printf a                                        */
/*----------------------------------------------------------------------*/

#define PRNTE(a)	
#define PRINT(a)

#define DD_GSW       'W'

/* Changed - JCC */
/* #define G_ENABLE     i_enable(oldmask);
   #define G_DISABLE    oldmask = i_disable(intr_priority); */
#define G_ENABLE     unlock_enable(oldmask, &psladd_lock);
#define G_DISABLE    oldmask = disable_lock(intr_priority, &psladd_lock);

#define AWAKE_EVENTQ e_wakeup(&g->sleep_eventq);
#define AWAKE_SIO    e_wakeup(&g->sleep_sio);
#define AWAKE_OPEN(a) e_wakeup(&((mgcb_ptr + a )->sleep_open));
#define SIGAPPL(a)   pidsig(g->pid,a)

#define SET_TMR(a,b,c) timeout(gswtimer,c,a) 
#define CANCEL_TMR(a)  untimeout(gswtimer,a) 

#define END_SLEEP                                                \
   {                                                             \
	g->oflag.slp_pending = FALSE;                            \
	G_ENABLE;                                                \
	PRINT(("END_SLEEP ....return EINTR \n"));                \
	return(EINTR);                                           \
   }

#define UN_LOCK  unlockl(&g->lock);

/* Changed - JCC */
/* #define DO_SLEEP(b,c) e_sleep(b,c) */
#define DO_SLEEP(b,c) e_sleep_thread(b, &psladd_lock, c)


#define BAD_EXIT(a) { G_ENABLE; return(a); }
#define LEAVE_IT  if (found == TRUE) {  \
	 G_ENABLE;                      \
	 return(0);                     \
    }                                   \
    else {                              \
	 G_ENABLE;                      \
	 return(EINVAL);                \
    }

/*----------------------------------------------------------------------*/
/* MACROs for handling PARITY errors while doing PIO with adapter.      */
/*----------------------------------------------------------------------*/
#define SET_PARITYJMPRC(dev,bufadr,uniqcode,component,retcode,lockflag) \
  /* Set parity handler */                                              \
  parityrc = setjmpx(bufadr);                                           \
  if (parityrc != 0)                                                    \
  {                                                                     \
	PRNTE(("parity macro: returned non-zero. "));                   \
	if (parityrc == EXCEPT_IO)                                      \
	{                                                               \
	    /* Log an error and return */                               \
	    PRNTE(("parity macro: EXCEPT_IO returned."));               \
	    PRNTE(("component=%s,uniqcode=%d\n",component,uniqcode));   \
	    gerr_log(dev,NULL,component,PIOParityError,parityrc,        \
		 uniqcode);                                             \
	    clrjmpx(bufadr);                                            \
	    if (lockflag == DO_UNLOCK)                                  \
		 UN_LOCK                                                \
	    return(retcode);                                            \
	}                                                               \
	else                                                            \
	    longjmpx(parityrc);                                         \
  }
#define CLR_PARITYJMP(bf)                                               \
  clrjmpx(bf);

#define LdaLimit        31              /* max number of logical devs   */
#define RmiSize         3               /* 3 bytes of RMI data          */
#define DMA_KERNEL      0               /* kernel buffer for DMA        */
#define DMA_USER        1               /* user buffer for DMA          */
#define NO_DMA_IN_PROG  -1              /* d-complete has been issued   */

/*----------------------------------------------------------------------*/
/* Adapter i/o register offsets.                                        */
/* To use, put contents of POS reg3 in next-to-last byte.               */
/*----------------------------------------------------------------------*/

#define IntIOAdr        0x00000090      /* interrupt                    */
#define ResetIOAdr      0x00000092      /* reset                        */
#define StopIOAdr       0x00000094      /* stop                         */
#define StartIOAdr      0x00000096      /* start                        */
#define RdStatIOAdr     0x00000098      /* real MSLA interrupt status   */
#define ResetIntIOAdr   0x0000009A      /* reset interrupt              */
#define EnaIntrIOAdr    0x0000009C      /* enable MSLA interrupts       */
#define DisIntrIOAdr    0x0000009E      /* disable MSLA interrupts      */

#define RtoMCommAdrOfst 0x00001000      /* R2 to MSLA comm area offset  */
#define MtoRCommAdrOfst 0x00001040      /* MSLA to R2 comm area offset  */

/*----------------------------------------------------------------------*/
/* Bits for ddsp->hw.int_dev_type (see display.h)                       */
/*----------------------------------------------------------------------*/
#define IOBusDevice             0x80000000
#define SwitchToCoproc          0x40000000
#define SixteenBitDevice        0x20000000
#define NumMslaIntrs            0x01000000
#define UnknownSlotNumber       0x00FF0000
#define MSLADeviceNumber        0x00004900
#define NumberOfPorts           0x00000000

/* Added  - JCC */
#define PSLA_LOCK_CLASS 666   /* Random number for now */
