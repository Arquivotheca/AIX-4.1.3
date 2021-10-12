static char sccsid[] = "@(#)19	1.7  src/bos/kernext/mpa/mpa_config.c, sysxmpa, bos411, 9428A410j 3/28/94 16:22:46";
/*
 *   COMPONENT_NAME: (SYSXMPA) MP/A SDLC DEVICE DRIVER
 *
 *   FUNCTIONS: alloc_acb
 *		alloc_resources
 *		free_acb
 *		get_acb
 *		mpa_config
 *		mpa_init_dev
 *		mpa_set_pos
 *		mpa_term_dev
 *		mpa_trace
 *		shutdown_adapter
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <errno.h>
#include <sys/adspace.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/file.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/malloc.h>
#include <sys/timer.h>
#include <sys/mpadd.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/uio.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>
#include <sys/errids.h>
#include <sys/poll.h>
#include <sys/signal.h>

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³    External Function Declarations                                ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
extern int mpa_intrA();           /* SLIH function (level 3) */
extern int mpa_intrB();           /* SLIH function (level 4) */
extern int nodev();
extern int mpa_open();
extern int mpa_close();
extern int mpa_write();
extern int mpa_ioctl();
extern int mpa_mpx();
extern int mpa_offlvl();

extern void recv_timer();
extern void ring_timer();
extern void dsr_timer();
extern void xmit_timer();

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³    Global Declarations                                           ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

static struct acb *acbheader = NULL;
static int global_num_adapters = 0;
static int acbglobal_lock = LOCK_AVAIL;         /* Global Lock Variable */
static int mpaopened = 0;                       /* driver opened count */

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³ MPA Internal Trace Structure ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
static mpa_trace_t     mpatrace;

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³    Declarations for this translation unit                        ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

static struct devsw mpa_dsw;   /* the devsw entry for devswadd */


 /*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º NAME: mpa_config                                                            º
 º                                                                             º
 º FUNCTION: mpa_config performs operations necessary to the initialization of º
 º           an individual port on an adapter.  mpa_config will be called for  º
 º           each valid port during the bus configuration/device configuration º
 º           phase of the boot procedure.  mpa_config must set up various data º
 º           data structures, validate requests and so forth.                  º
 º                                                                             º
 º EXECUTION ENVIRONMENT:                                                      º
 º                                                                             º
 º      Preemptable        : Yes                                               º
 º      VMM Critical Region: No                                                º
 º      Runs on Fixed Stack: Yes                                               º
 º      May Page Fault     : Yes                                               º
 º      May Backtrack      : Yes                                               º
 º                                                                             º
 º NOTES:                                                                      º
 º                                                                             º
 º DATA STRUCTURES:                                                            º
 º                                                                             º
 º RETURN VALUE DESCRIPTION: 0 - EVT mpa_config return was successful          º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

int mpa_config ( dev_t           dev,
		int             cmd,
		struct uio      *uiop )

{
    /* mpa_config local variables */

    int                 rc=0;         /* general return code */
    struct acb          *acb;         /* pointer to the acb */
    void 		mpa_set_pos(); /* function to set pos registers */

    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ log a trace hook ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
    DDHKWD2 (HKWD_DD_MPADD, DD_ENTRY_CONFIG, 0, dev, cmd);

    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Lock Global Lock, waits for lock or returns early on signal ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
    if (lockl(&acbglobal_lock, LOCK_SIGRET) == LOCK_SIG) 
    {
	    DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,EINTR,dev,0xA0);
	    return EINTR;
    }

    switch (cmd)                /* switch based on command type */
    {

	case CFG_INIT:          /* initialize device driver and internal */
				/* data areas                            */
	{

	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	    ³ Pin all of the driver code. ³
	    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
    	    if (rc = pincode(mpa_config)) 
	    {
		unlockl(&acbglobal_lock);
		DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,rc,dev,cmd);
		return rc;
	    }

	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	    ³ first time through CFG_INIT requires special steps ³
	    ³ no active adapters means this is first time        ³
	    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

	    if (acbheader == NULL) 
    	    { /* First adapter to be added */

		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                ³ now that driver is pinned, can start in-core ³
		³ traces by setting up an in-core trace table  ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                ³ initialize   mpatrace.next = 0                              ³
                ³              mpatrace.res1 = "MPAT"                         ³
                ³              mpatrace.res2 = "RACE"                         ³
                ³              mpatrace.res3 = "TABL"                         ³
                ³              mpatrace.table[0] = "!!!!"                     ³
                ³                                                             ³
                ³ In the low-level debugger, one would do a find for          ³
                ³                                                             ³
                ³              MPATRACETABL                                   ³
                ³                                                             ³
                ³ to find the starting location of the internal trace table.  ³
                ³                                                             ³
                ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

                mpatrace.next = 0;              /* mpatrace.next */
                mpatrace.res1 = 0x4d504154;     /* "MPAT" */
                mpatrace.res2 = 0x52414345;     /* "RACE" */
                mpatrace.res3 = 0x5441424c;     /* "TABL" */
                mpatrace.table[0] = 0x21212121; /* "!!!!" */

                MPATRACE2("Cin1",dev);


		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ add our entry points to the devsw table ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		mpa_dsw.d_open     = mpa_open;
		mpa_dsw.d_close    = mpa_close;
		mpa_dsw.d_read     = nodev;
		mpa_dsw.d_write    = mpa_write;
		mpa_dsw.d_ioctl    = mpa_ioctl;
		mpa_dsw.d_strategy = nodev;
		mpa_dsw.d_ttys     = (struct tty *)NULL;
		mpa_dsw.d_select   = nodev;
		mpa_dsw.d_config   = mpa_config;
		mpa_dsw.d_print    = nodev;
		mpa_dsw.d_dump     = nodev;
		mpa_dsw.d_mpx      = mpa_mpx;
		mpa_dsw.d_revoke   = nodev;
		mpa_dsw.d_dsdptr   = NULL;
		mpa_dsw.d_selptr   = NULL;
		mpa_dsw.d_opts     = 0;

		if ((rc = devswadd(dev, &mpa_dsw)) != 0) 
		{
		    unpincode (mpa_config);
		    unlockl(&acbglobal_lock);
		    MPATRACE3("Cie1",dev,rc);
		    return(rc);
		}

		MPATRACE2("Cin2",dev);

	    } /* end of first time...pin code and load devsw entry */

            MPATRACE3("Cin3",dev,cmd);
	   
	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	    ³ Make sure this device wasn't already configured. ³
	    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	    if (acb = get_acb(minor(dev))) 
	    {
		    unlockl(&acbglobal_lock);
                    MPATRACE2("Cie2",dev);
		    return ENXIO;
	    }

	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	    ³ make sure that they don't try to exceed the ³
	    ³ maximum number of adapters                  ³
	    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	    if (global_num_adapters >= MAX_ADAPTERS) 
	    {
		    unlockl(&acbglobal_lock);
		    MPATRACE2("Cie3",dev);
		    return ENXIO;
	    }
	    global_num_adapters++; 
	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	    ³ Allocate a acb structure. ³
	    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	    if ((acb = alloc_acb()) == NULL) 
	    {
		    rc = ENOMEM;
		    MPATRACE3("Cie4",dev,rc);
		    goto release_drvr;
	    }
	    acb->adap_lock = LOCK_AVAIL;

	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	    ³ Copy cfg mthd data into device dependent structure. ³
	    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	    if (rc = uiomove(&acb->mpaddp,sizeof(acb->mpaddp),UIO_WRITE,uiop)) 
	    {
		    free_acb(acb);
		    MPATRACE3("Cie5",dev,rc);
		    goto release_drvr;
	    }

	    MPATRACE4("Cin4",sizeof(acb->mpaddp),uiop->uio_resid,rc);

	    acb->dev = dev;
	    mpa_set_pos(acb);

	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	    ³ reset the 8273 on mpa card.. NOTE: do not remove this ³
	    ³ reset it is needed to clean up the card to start. If  ³
	    ³ this reset is not done the cio_start may hang.        ³
	    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	    if ( (rc=shutdown_adapter(acb)) ) 
	    {
		    free_acb(acb);
		    MPATRACE3("Cie6",dev,rc);
		    goto release_drvr;
	    }

	    acb->flags |= MPAINIT;
	    if (mpaopened == 0)
			unpincode(mpa_config);

	    break;

	} /* end of case CFG_INIT */


	case CFG_TERM:          /* terminate the device associated with */
				/* dev parameter                      */
	{


	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	    ³ Get the acb structure. ³
	    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	    if ((acb = get_acb(minor(dev))) == NULL) 
	    {
		    unlockl(&acbglobal_lock);
		    DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,rc,dev,cmd);
		    return ENODEV;
	    }

	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	    ³ Don't terminate if there are outstanding open()s. ³
	    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	    if (acb->num_opens) 
	    {
		    unlockl(&acbglobal_lock);
		    DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,rc,dev,cmd);
		    return EBUSY;
	    }

	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	    ³ Make sure the driver is pinned ³
	    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	    if (mpaopened == 0)
	    {
		    pincode(mpa_config);
	    }

            MPATRACE3("Ctm1",dev,cmd);

	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	    ³ Shutdown the adapter puts 8273 in reset state. ³
	    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	    shutdown_adapter(acb);

	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	    ³ Free system resources. ³
	    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	    global_num_adapters--;

	    free_acb(acb);
	    if (acbheader == NULL)  
	    {
		    rc = devswdel(dev);
	    }
	    if (acbheader == NULL || mpaopened == 0)
	    {
		    rc = unpincode(mpa_config);
	    }

	    break;

	} /* end of case CFG_TERM */


	case CFG_QVPD:          /* query vital product data             */
	    break;

	default:
	    unlockl(&acbglobal_lock);
	    DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,rc,dev,cmd);
	    return(EINVAL);

    } /* end of switch statement */

    unlockl(&acbglobal_lock);
    DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,rc,dev,0);
    return(rc);

release_drvr:
	if (acbheader == NULL)
		 devswdel(dev);
	if (acbheader == NULL || mpaopened == 0)
		 unpincode(mpa_config);
	unlockl(&acbglobal_lock);
        DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,rc,dev,0xAB);
	return rc;
}  /* mpa_config() */


/*****************************************************************************
** NAME:        shutdown_adapter
**
** FUNCTION:    This code resets and enables the adapter
**              It is need during bringup to reset the card and put it in
**              the correct initial state. It can also be used to reset
**              the device to a correct state.
**
** EXECUTION ENVIRONMENT:
**
** NOTES:
**
** RETURNS:     0 if successful
**            EIO on PIO error.
**
*****************************************************************************/
int shutdown_adapter (struct acb *acb)
{
	int                spl,rc,cntr=0;
	ulong              pos_ptr;


	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ this routine is called at config time and it sets the ³
	³ mode for the 8255.                                    ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

	DISABLE_INTERRUPTS(spl);

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Use the 8255 port b to reset the 8273.³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	while ( ++cntr < 2) 
	{
	     if(PIO_PUTC( acb, MODE_OFFSET, 0x98 )== -1) 
	     {
		     pos_ptr = MPA_IOCC_ATT;
		     BUS_PUTC( pos_ptr + POS2, acb->pos2 );
		     BUS_PUTC( pos_ptr + POS3, acb->pos3 );
		     IOCC_DET(pos_ptr);
	     }
	     else break;
	}
	if(cntr==2) 
	{
		mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
		ENABLE_INTERRUPTS(spl);
		return EIO;
	}

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Save the current Pos reg values. This adds a needed delay     ³
	³ Between the two 8255 accesses without calling a delay routine ³
	³ That will trap if called in irpt environment.                 ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	pos_ptr = MPA_IOCC_ATT;

	    acb->pos0 = (uchar) BUS_GETC( pos_ptr + POS0 );
	    acb->pos1 = (uchar) BUS_GETC( pos_ptr + POS1 );
	    acb->pos2 = (uchar) BUS_GETC( pos_ptr + POS2 );
	    acb->pos3 = (uchar) BUS_GETC( pos_ptr + POS3 );

	IOCC_DET( pos_ptr );

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ This next write will disable all timers. and set the ³
	³ 8273 in a reset state. It will stay in this state    ³
	³ until the CIO_START is issued.                       ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if(PIO_PUTC( acb, PORT_B_8255, 0x17 )== -1) 
	{
	    mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
	    ENABLE_INTERRUPTS(spl);
	    return EIO;
	}

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ since shutdown adapter kills recv, 0 the flag ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	acb->flags &= ~RECEIVER_ENABLED;
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ cancel all timers which may be pending ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	acb->flags &= ~RCV_TIMER_ON;
        untimeout(recv_timer, (caddr_t)acb); /* receive timer */
        untimeout(ring_timer, (caddr_t)acb); /* ring indicator timer */
        untimeout(dsr_timer, (caddr_t)acb);  /* data set ready timer */
        untimeout(xmit_timer, (caddr_t)acb);  /* transmit failsafe timer */

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ free up any recv dma being held in acb ³
        ³ (this was added for defect #091440)    ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if( acb->hold_recv != NULL)
	{
        	free_dma_elem(acb, (dma_elem_t *)acb->hold_recv);
	}

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Mark the adapter dead - RIP  MPA is brought back to life ³
	³ by CIO_START.                                            ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	acb->flags &= ~STARTED_CIO;
	acb->flags |= MPADEAD;


	ENABLE_INTERRUPTS(spl);
	return 0;
} /* shutdown_adapter() */



/*****************************************************************************
** NAME:        mpa_set_pos
**
** FUNCTION:    Verifies the CARD ID value and sets the POS register values
**              according the information in the DDS.
**
** EXECUTION ENVIRONMENT:       Can be called from the process environent only.
**
** NOTES:
**      Inputs:
**              pointer to a channel adapter structure
**      Outputs:
**              return status
**      Called By:
**              mpa_config()
**      Calls:
**
** RETURNS:     0 - Success
**              (-1) - failed
**
*****************************************************************************/
void
mpa_set_pos (struct acb *acb)
{
    ulong                         pos_ptr;
    uchar                         t_pos;
    int                           spl;

	DISABLE_INTERRUPTS(spl);

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ get access to bus I/O space ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	pos_ptr = MPA_IOCC_ATT;

	t_pos = (unsigned char)0;

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ POS2 contains CARD ENABLE + Mode selection + DMA ENABLE   ³
	³ V.25 bis support                                          ³
	³   7     6     5     4     3     2     1     0             ³
	³ +-----+-----+-----+-----+-----+-----+-----+-----+         ³
	³ | n/a | V.25| DMA |  Mode selection code  | CD  |         ³
	³ +-----+-----+-----+-----+-----+-----+-----+-----+         ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ set the card enable and n/a bits to 1's                   ³
	³ for now default V.25 bit to 1 ( not enabled)              ³
	³ set DMA on for SDLC and set mode to 8 for SDLC            ³
	³ so this reg will be set to 0xF1                           ³
	³ later on I can use some odm or smit input to set up in    ³
	³  different mode or to enable V.25                         ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if( (DDS.io_addr&0x0a0) == 0x0a0)
	   t_pos = (P2_ENABLE|P2_ALT_SDLC );
	else
	   t_pos = (P2_ENABLE|P2_SDLC_MODE );

	t_pos |= 0x80; /* turn on the n/a bit */

	BUS_PUTC( pos_ptr + POS2, t_pos );

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ POS3 contains arb level.                           ³
	³   7     6     5     4     3     2     1     0      ³
	³ +-----+-----+-----+-----+-----+-----+-----+-----+  ³
	³ |  1  |  1  |  1  |  1  |   arbitration level   |  ³
	³ +-----+-----+-----+-----+-----+-----+-----+-----+  ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ */

	t_pos = (unsigned char)0;
	t_pos = DDS.dma_lvl;
	t_pos |= 0xF0;   /* set the one bits back */

	BUS_PUTC( pos_ptr + POS3, t_pos );

	IOCC_DET( pos_ptr );
	ENABLE_INTERRUPTS(spl);

	return;
} /* mpa_set_pos() */

/*****************************************************************************
** NAME:        mpa_init_dev
**
** FUNCTION:    Sets the POS register values according the
**                              information in the DDS.
**                              Install the interrupt handler.
**                              Install the off-level interrupt handler.
**                              Acquire a DMA channel.
**
** EXECUTION ENVIRONMENT:       Can be called from the process environent only.
**
** NOTES:
**      Inputs:
**              pointer to a channel adapter structure
**      Outputs:
**              return status
**      Called By:
**              mpa_config()
**      Calls:
**
** RETURNS:     0  - Success
**              ~0 - Failure
**
*****************************************************************************/
int
mpa_init_dev (struct acb *acb)
{
	int 		rc;
	struct intr     *p_intr;

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ pin the driver code ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if (mpaopened++ == 0) 
	{
		if (rc = pincode(mpa_config)) 
		{
		   --mpaopened;
		   return rc;
		}
                /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                ³ allocate timeout elements ³
                ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                if( (rc=timeoutcf(16)) != 0 ) 
		{
		   --mpaopened;
		   unpincode(mpa_config);
		   return ENOMEM;
		}
	}


        MPATRACE2("MinE",acb->dev);
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Initialize the adapter structure ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	acb->act_xmit_head = acb->act_xmit_tail = NULL;
	acb->act_recv_head = acb->act_recv_tail = NULL;
	acb->act_irpt_head = acb->act_irpt_tail = NULL;
	acb->act_dma_head = acb->act_dma_tail = NULL;
        acb->hold_recv = NULL;
	acb->xmit_free = NULL;
	acb->recv_free = NULL;
	acb->irpt_free = NULL;
	acb->dma_free = NULL;

	acb->mbuf_event = EVENT_NULL;
	acb->xmitbuf_event = EVENT_NULL;
	acb->irptbuf_event = EVENT_NULL;
	acb->dmabuf_event = EVENT_NULL;
	acb->op_rcv_event = EVENT_NULL;

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³  Allocate memory for the resource chains...     ³
	³  recv_elems, irpt_elems, dma_elems, xmit_elems. ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if( (rc = alloc_resources(acb)) ) 
	{
              MPATRACE3("Mie1",acb->dev,rc);
	      mpa_term_dev(acb);
	      return rc;
	}

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Install off-level interrupt handler    ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	acb->ofl.p_acb_intr = (struct acb *)acb;
        p_intr = &(acb->ofl.offl_intr);
        INIT_OFFL1(p_intr, mpa_offlvl, DDS.bus_id);

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Install interrupt handler for level 3. ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	acb->ih_structA.next            = NULL;
	acb->ih_structA.handler         = mpa_intrA;
	acb->ih_structA.bus_type        = DDS.bus_type;
	acb->ih_structA.level           = DDS.int_lvlA;
	acb->ih_structA.flags           = 0;
	acb->ih_structA.priority        = DDS.intr_priority;
	acb->ih_structA.bid             = DDS.bus_id;
	acb->ih_structA.i_count         = 0;

	if ((rc = i_init(&(acb->ih_structA))) != 0) 
	{
        	MPATRACE3("Mie2",acb->dev,rc);
		mpa_term_dev( acb );
		return EIO;
	}
	acb->flags |= MPAIINSTALL3;   /* interrupt handler 3 installed */
       
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Install interrupt handler for level 4. ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        acb->ih_structB.next            = NULL;
        acb->ih_structB.handler         = mpa_intrB;
        acb->ih_structB.bus_type        = DDS.bus_type;
        acb->ih_structB.level           = DDS.int_lvlB;
        acb->ih_structB.flags           = 0;
        acb->ih_structB.priority        = DDS.intr_priority;
        acb->ih_structB.bid             = DDS.bus_id;
        acb->ih_structB.i_count         = 0;

        if ((rc = i_init(&(acb->ih_structB))) != 0) 
	{
        	MPATRACE3("Mie3",acb->dev,rc);
                mpa_term_dev( acb );
                return EIO;
        }
        acb->flags |= MPAIINSTALL4;   /* interrupt handler 4 installed */

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Acquire a DMA channel to use ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	acb->dma_channel = d_init(DDS.dma_lvl, (DMA_SLAVE|MICRO_CHANNEL_DMA),
		DDS.bus_id);
	if (acb->dma_channel == DMA_FAIL) 
	{
        	MPATRACE3("Mie4",acb->dev,rc);
		mpa_term_dev(acb);
		return EIO;
	}
	acb->flags |= MPADMACHAN;        /* DMA channel acquired */

        MPATRACE2("MinX",acb->dev);
	return 0;
} /* mpa_init_dev() */


/*****************************************************************************
** NAME:     mpa_term_dev
**
** FUNCTION:    Disable the adapter's interrupts, remove the interrupt
**              handler, unpin the code, and release the DMA channel.
**
** EXECUTION ENVIRONMENT:       Can be called from the process
** 		environment only and with the adapter lock held.
**
** NOTES:
**      Inputs:
**              pointer to a channel adapter structure
**      Outputs:
**              return status
**      Called By:
**              free_open_struct(), and mpa_init_dev()
**      Calls:
**              free_all_resources(),  unpincode()
**
** RETURNS:     0 - Success
**
*****************************************************************************/
int
mpa_term_dev (struct acb *acb)
{
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Free all the resources. ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	free_all_resources(acb);

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ unpin the driver code ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if (mpaopened == 0 || --mpaopened == 0) 
	{
		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ deallocate the timeout elements ³
		³ and unpin the timer functions   ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		timeoutcf(-16);
		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ now unpin the driver ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		unpincode(mpa_config);
	}

	return 0;
} /* mpa_term_dev() */


/*****************************************************************************
** NAME: alloc_acb
**
** FUNCTION: Allocates memory for acb structure and puts into a linked list.
**      Also allocates sub-structures for dds and timer.
**
** EXECUTION ENVIRONMENT:       Can be called from the process environment only.
**
** NOTES: Assumes we have the global lock...
**
**    Input:
**              nothing
**    Output:
**              pointer to the allocated channel adapter structure
**    Called From:
**              mpa_config()
**    Calls:
**              xmalloc() bzero()
**
** RETURNS:     pointer to the allocated acb structure - Success
**              NULL - allocation failed
**
*****************************************************************************/
alloc_acb ()
{
	struct acb *acb;

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ allocate acb structure and put into linked list ³
	³ This will only work for two adapter structs.    ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if ((acb = KMALLOC(struct acb)) == NULL)
		return((struct acb *) NULL);

	bzero((caddr_t)acb, sizeof(struct acb ));

	if (acbheader) 
	{               /* add the second adpater struct */
		acb->next = acbheader;
	}
	else 
	{                         /* put on the first adpater struct */
		acb->next = NULL;
	}
	acbheader = acb;

	return(acb);
} /* alloc_acb() */



/*****************************************************************************
** NAME:        free_acb
**
** FUNCTION:    Finds the given channel adapter structure and frees it.
**
** EXECUTION ENVIRONMENT:       Can be called from the process environment only.
**
** NOTES:
**    Input:
**              Pointer to a channel adapter structure
**    Output:
**              status code
**    Called From:
**              mpa_config()
**    Calls:
**              xmalloc()
**
** RETURNS:
**              Nothing
**
*****************************************************************************/
int    free_acb(struct acb *acb)
{
	struct acb **nextptr;

	for (nextptr = &acbheader; *nextptr != acb; nextptr = &(*nextptr)->next) 
	{
		if (*nextptr == NULL)
			return;
	}
	*nextptr = acb->next;

	KFREE(acb);
	return; 
} /* free_acb() */


/*****************************************************************************
** NAME:        get_acb
**
** FUNCTION:    Searches for and returns the acb structure associated with the
**              minor device number.
**
** EXECUTION ENVIRONMENT:       Can be called from the process environment only.
**
** NOTES:
**    Input:
**              Major/Minor device numbers
**    Output:
**              Pointer to a channel adapter structure
**    Called From:
**
**    Calls:
**              nothing...
**
** RETURNS:     pointer to a channel adapter structure - Success
**              NULL - couldn't find the given channel adapter structure
**
*****************************************************************************/

get_acb(dev_t dev) 
{
	struct acb *acb;

	for (acb = acbheader; acb != NULL; acb = acb->next) 
	{
		if ( minor(acb->dev) == dev)
			break;
	}
	return(acb);
} /* get_acb() */


/*****************************************************************************
** NAME:     alloc_resources
**
** FUNCTION:    Allocates memory for and builds free list chain for
**              recv_elems, irpt_elems, dma_elems, xmit_elems.
**
** EXECUTION ENVIRONMENT:   Can be called from the process environment only.
**
** NOTES:
**      Inputs:
**              pointer to a channel adapter structure
**      Outputs:
**              return status
**      Called By:
**              mpa_init_dev()
**      Calls:
**
** RETURNS:     0 - Success
**		ENOMEM - KMALLOC (xmalloc()) failed to acquire memory.
**
*****************************************************************************/
int
alloc_resources (struct acb *acb)
{
	irpt_elem_t     *irpt_elem_p;
	recv_elem_t     *recv_elem_p;
	xmit_elem_t     *xmit_elem_p;
	dma_elem_t      *dma_elem_p;
	int             i;


	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Allocate memory for irpt q structures and put them on a ³
	³ free list attached to acb.                              ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

	for(i=0; i<MAX_IRPT_QSIZE; i++) 
	{
	   if ((irpt_elem_p = KMALLOC(irpt_elem_t)) == NULL) 
	   {
	      	mpalog(acb,ERRID_MPA_BFR, __LINE__,__FILE__,0,0);
	     	free_open_struct(acb);
	     	return (ENOMEM);
	   }
	   bzero(irpt_elem_p,sizeof(irpt_elem_t));
	   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	   ³ Add this element to the free list. ³
	   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	   irpt_elem_p->ip_next = acb->irpt_free;
           acb->irpt_free = irpt_elem_p;
	}
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Allocate memory for recv q structures and put them on a ³
	³ free list attached to acb.                              ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

	for(i=0; i<MAX_RECV_QSIZE; i++) 
	{
	   if ((recv_elem_p = KMALLOC(recv_elem_t)) == NULL) 
	   {
	      	mpalog(acb,ERRID_MPA_BFR, __LINE__,__FILE__,0,0);
	     	free_open_struct(acb);
	     	return (ENOMEM);
	   }
	   bzero(recv_elem_p,sizeof(recv_elem_t));
	   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	   ³ Add this element to the free list. ³
	   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	   recv_elem_p->rc_next = acb->recv_free;
           acb->recv_free = recv_elem_p;
	}
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Allocate memory for xmit q structures and put them on a ³
	³ free list attached to acb.                              ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

	for(i=0; i<MAX_XMIT_QSIZE; i++) 
	{
	   if ((xmit_elem_p = KMALLOC(xmit_elem_t)) == NULL) 
	   {
		mpalog(acb,ERRID_MPA_BFR, __LINE__,__FILE__,0,0);
		free_open_struct(acb);
		return (ENOMEM);
	   }
	   bzero(xmit_elem_p,sizeof(xmit_elem_t));
           /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
           ³ Add this element to the free list. ³
           ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
           xmit_elem_p->xm_next = acb->xmit_free;
           acb->xmit_free = xmit_elem_p;
	}
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Allocate memory for dma q structures and put them on a ³
	³ free list attached to acb.                             ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

	for(i=0; i<MAX_DMA_QSIZE; i++) 
	{
	   if ((dma_elem_p = KMALLOC(dma_elem_t)) == NULL) 
	   {
	     	mpalog(acb,ERRID_MPA_BFR, __LINE__,__FILE__,0,0);
	     	free_open_struct(acb);
	     	return (ENOMEM);
	   }
	   bzero(dma_elem_p,sizeof(dma_elem_t));
           /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
           ³ Add this element to the free list. ³
           ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
           dma_elem_p->dm_next = acb->dma_free;
           acb->dma_free = dma_elem_p;
	}

	return 0;
}  /* alloc_resources() */

/*
 * NAME: mpa_trace
 *
 * FUNCTION:
 *      This routine puts a trace entry into the internal device
 *      driver trace table.  It also calls the AIX trace service.
 *
 * EXECUTION ENVIRONMENT: process or interrupt environment
 *
 * NOTES:
 *      Each trace point is made up of 4 words (ulong).  Each entry
 *      is as follows:
 *
 *              | Footprint | data | data | data |
 *
*/
void
mpa_trace (    register uchar  str[],   /* trace data Footprint */
                register ulong  arg2,   /* trace data */
                register ulong  arg3,   /* trace data */
                register ulong  arg4)   /* trace data */

{
        register int    i,spl;
        register char   *p_str;

        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³ get the current trace point index ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        if (global_num_adapters != 0)
                DISABLE_INTERRUPTS(spl);

        i= mpatrace.next;

        p_str = &str[0];

        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³ copy the trace point data into the global trace table ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        mpatrace.table[i] = *(ulong *)p_str;
        ++i;
        mpatrace.table[i] = arg2;
        ++i;
        mpatrace.table[i] = arg3;
        ++i;
        mpatrace.table[i] = arg4;
        ++i;


        if ( i < MPA_TRACE_SIZE )
        {
                mpatrace.table[i] = 0x21212121;        /* end delimeter */
                mpatrace.next = i;
        }
        else
        {
                mpatrace.table[0] = 0x21212121;        /* end delimeter */
                mpatrace.next = 0;
        }


        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³  Make the AIX trace point call ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
         TRCHKGT(HKWD_DD_MPADD | (DD_MPA_HOOK<<8), *(ulong *)p_str, arg2,
                        arg3, arg4, 0);

        if (global_num_adapters != 0)
                ENABLE_INTERRUPTS(spl);
        return;
} /* end mpa_trace */
