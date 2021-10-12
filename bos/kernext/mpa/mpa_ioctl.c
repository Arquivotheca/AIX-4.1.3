static char sccsid[] = "@(#)20	1.5  src/bos/kernext/mpa/mpa_ioctl.c, sysxmpa, bos411, 9428A410j 3/28/94 16:22:50";
/*
 *   COMPONENT_NAME: (SYSXMPA) MP/A SDLC DEVICE DRIVER
 *
 *   FUNCTIONS: change_parms
 *		cio_halt
 *		cio_start
 *		mpa_ioctl
 *		sdlc_init
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


/*******************************************************************
 *    Include Files                                                *
 *******************************************************************/

#include <sys/adspace.h>
#include <sys/errids.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/device.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/malloc.h>
#include <sys/mpadd.h>
#include <sys/param.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/sysdma.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/xmem.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>

extern void recv_timer();
/*
 * NAME: mpa_ioctl
 *
 * FUNCTION: Performs a variety of control functions to the MPA device
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Preemptable        : Yes
 *      VMM Critical Region: No
 *      Runs on Fixed Stack: Yes
 *      May Page Fault     : Yes
 *      May Backtrack      : Yes
 *
 * NOTES: More detailed description of the function, down to
 *          what bits / data structures, etc it manipulates.
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:
 */

int mpa_ioctl ( dev_t            dev,
	       int              cmd,
	       caddr_t          arg,
	       int              devflag,
	       int              chan,
	       int              ext )

{
    int                         error;         /* return value  */
    struct acb                  *acb;        /* pointer to acb struct */

    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ log a trace hook ³ 
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

    DDHKWD5(HKWD_DD_MPADD, DD_ENTRY_IOCTL, 0, dev, cmd,
            devflag, chan, ext);
    MPATRACE4("IocE",dev,cmd,chan);

    if ( ((acb = get_acb(minor(dev))) == NULL) ||
		    !(OPENP.op_flags&OP_OPENED) ) 
    {

            MPATRACE2("Ioe1",dev);
	    return ENXIO;
    }

    if (lockl(&acb->adap_lock, LOCK_SIGRET) != LOCK_SUCC) 
    {
            MPATRACE2("Ioe2",dev);
	    return EINTR;
    }

    error = 0;
    switch ( cmd )
    {
	case CIO_START:         /* Start Device Command */
	    error = cio_start(acb, arg, devflag, chan);
	    break;

	case CIO_HALT:          /* Halt Device Command */
	    error = cio_halt(acb, arg, devflag, chan);
	    break;

	case MP_CHG_PARMS:      /* Change Parameters Command */
            error = change_parms(acb, arg, devflag, chan);
	    break;

	default:
	    error = 0;
	    break;
    }

    MPATRACE3("IocX",dev,error);

    unlockl(&acb->adap_lock);

    if (error == -1)
    {
	return EIO;
    }
    
    DDHKWD5(HKWD_DD_MPADD, DD_EXIT_IOCTL, error, dev,  0, 0, 0, 0 );
    return (error);
}      /* mpa_ioctl() */

/*---------------------   SDLC_INIT      -------------------------------*/
/*                                                                      */
/*  NAME: sdlc_init                                                     */
/*                                                                      */
/*  FUNCTION:                                                           */
/*                                                  Sets up pos         */
/*      based on current info in dds, then initializes ports based      */
/*      on current info in dds, gets card ready for xfers.              */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from process or offlevel environment.             */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS:                                                            */
/*      0       If the init  succeeded.                                 */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:                                         */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*----------------------------------------------------------------------*/

int sdlc_init (struct acb  *acb)
{
	uchar           value;
	int             count,rc,cntr=0;
	ulong           pos;

	
    	MPATRACE2("SdlE",acb->dev);
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ I set this flag here so that if an irpt line is hung  ³
	³ The irpt handler can disable the card and set MPADEAD ³
	³ Then this start code can check for MPADEAD and know   ³
	³ There is a problem and return a bad rc to application ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	acb->flags &= ~MPADEAD;
	acb->stats.ds.irpt_fail = 0;

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Set up 8255 Port B for operation. ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³            8255 PORT C bit Assignment                        ³
        ³  | d7 | d6 | d5 | d4 | d3 | d2 | d1 | d0 |                   ³
        ³    |    |    |    |    |    |    |    |                      ³
        ³    |    |    |    |    |    |    |    0=Turn on Speed Select ³
        ³    |    |    |    |    |    |    0=Turn on Select Standby    ³
        ³    |    |    |    |    |    0=Turn on Test                   ³
        ³    |    |    |    |    0=Reset Modem Status Changed Logic    ³
        ³    |    |    |    1=Reset 8273                               ³
        ³    |    |    1=Gate Timer 2                                  ³
        ³    |    1=Gate Timer 1                                       ³
        ³    1=Enable Level 4 Interrupt                                ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³  Drop the 8273 reset bit.                           ³
	³                        The 8273 was put in a reset  ³
	³  state by the config code or the close or halt code.³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	value = 0x00;

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Set up default values ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	value = SEL_STANBY_OFF|TEST_OFF|FREE_STAT_CHG;

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ If DRS (Data Rate Select) is not indicated, it ³
	³ can be turned off here using SPEED_SEL_OFF     ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if ( !(acb->strt_blk.modem_flags & MF_DRS_ON) ) 
	{
		value |= SPEED_SEL_OFF; /* DRS Off */
	}
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³ We should enable level 4 interrupts so we can detect ³
        ³ changes in modem status (DSR and CTS changes)        ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        value |= ENABLE_IRPT4; 


	while ( ++cntr < 2) 
    	{
	    if(PIO_PUTC(acb, PORT_B_8255, value )==-1) 
	    {
	       	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	    	³ if this fails then pos2 enable may be off try ³
	       	³ setting it back on and try again              ³
               	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	       	pos = MPA_IOCC_ATT;
	       	BUS_PUTC( pos + POS2, acb->pos2 );
	       	BUS_PUTC( pos + POS3, acb->pos3 );
	       	IOCC_DET(pos);
	     }
	     else 
	     {
	   	acb->state.port_b_8255 = value;
		MPATRACE3("Sdl1",acb->dev,acb->state.port_b_8255);
	   	break;
	      }
	 }

	 if(cntr==2) 
	 {
	    mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
	    return EIO;
	 }
	 if(acb->flags&MPADEAD) 
	 {
	    mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
	    return EIO;
	 }

	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³            8255 PORT C bit Assignment                      ³
	 ³  | d7 | d6 | d5 | d4 | d3 | d2 | d1 | d0 |                 ³
	 ³    |    |    |    |    |    |    |    |                    ³
	 ³    |    |    |    |    |    |    |    1= Gate Internal clk ³
	 ³    |    |    |    |    |    |    1= Gate External clk      ³
	 ³    |    |    |    |    |    1 = Set Electronic Wrap        ³
	 ³    |    |    |    |    0=Enable irpts and DMA req          ³
	 ³    |    |    |    Oscillating= Recving data                ³
	 ³    |    |    Oscillating= Timer 0 output                   ³
	 ³    |    0=Test active in Electronic Wrap Mode              ³
	 ³    1=SDLC, 0 = BSC                                         ³ 
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³  Only the lower nibble of port C is writable.            ³
	 ³  Disable irpts and DMA and set 8273 in electronic wrap   ³
	 ³  and degate both internal and external clocks.  This     ³
	 ³  Isolates the 8273 from both modem(DCE) and microchannel ³
	 ³  interfaces for the setup procedures that follow:        ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 if(PIO_PUTC( acb, PORT_C_8255, 0x0C )==-1) 
	 {
	     mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
	     return EIO;
	 }
	 if(acb->flags&MPADEAD) 
	 {
	     mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
	     return EIO;
	 }

	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³ set up the Operating mode reg of 8273..reset all valid bits ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
	 acb->cmd_parms.cmd=RESET_OPER_MODE_CMD;
	 acb->cmd_parms.parm[0]=0xC0;
	 acb->cmd_parms.parm_count=1;
	 acb->state.oper_mode_8273=acb->cmd_parms.parm[0];
	 if((rc=que_command(acb)) ) return(rc);
 
	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³ turn on early interrupts on transmit to provide ³
	 ³ faster turnaround on the receive                ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 value = SET_EARLY_TX_ON;
	 bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
	 acb->cmd_parms.cmd=SET_OPER_MODE_CMD;
	 acb->cmd_parms.parm[0]=value;
	 acb->cmd_parms.parm_count=1;
	 acb->state.oper_mode_8273 |= acb->cmd_parms.parm[0];
	 if((rc=que_command(acb)) ) return(rc);
 	
	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³ Reset all bits in the Serial I/O mode reg of 8273 ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
	 acb->cmd_parms.cmd=RESET_IO_MODE_CMD;
	 acb->cmd_parms.parm[0]=0xF8;
	 acb->cmd_parms.parm_count=1;
	 acb->state.serial_io_8273 = acb->cmd_parms.parm[0];
	 if((rc=que_command(acb)) ) return(rc);
 
	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³ Set the bits in Serial I/O mode reg ³
	 ³ for NRZI if indicated by caller     ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 if( acb->strt_blk.data_flags & DATA_FLG_NRZI )
	 {
	    bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
	    acb->cmd_parms.cmd=SET_IO_MODE_CMD;
	    acb->cmd_parms.parm[0]=SET_NRZI_DATA;
	    acb->cmd_parms.parm_count=1;
	    acb->state.serial_io_8273 |= acb->cmd_parms.parm[0];
	    if((rc=que_command(acb)) ) return(rc);
	 }
 
	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³ Reset the data transfer reg bit (makes default DMA) ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
	 acb->cmd_parms.cmd=RESET_DATA_XFER_CMD;
	 acb->cmd_parms.parm[0]=0xFE;
	 acb->cmd_parms.parm_count=1;
	 acb->state.data_xfer_8273=acb->cmd_parms.parm[0];
	 if((rc=que_command(acb)) ) return(rc);


	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³ Reset the One bit delay reg of 8273 ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
	 acb->cmd_parms.cmd=RESET_1_BIT_DELAY_CMD;
	 acb->cmd_parms.parm[0]=0x7F;
	 acb->cmd_parms.parm_count=1;
	 acb->state.one_bit_8273=acb->cmd_parms.parm[0];
	 if((rc=que_command(acb)) ) return(rc);

	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³            8273 PORT B  bit Assignment                     ³
	 ³  | d7 | d6 | d5 | d4 | d3 | d2 | d1 | d0 |                 ³
	 ³    |    |    |    |    |    |    |    |                    ³
	 ³    |    |    |    |    |    |    |    RTS-Request to Send  ³
	 ³    |    |    |    |    |    |    Reserved                  ³
	 ³    |    |    |    |    |    DTR-Data Terminal Ready        ³
	 ³    |    |    |    |    Reserved                            ³
	 ³    |    |    |    V.25 bis ENABLE                          ³
	 ³    |    |    Flag Detect                                   ³
	 ³    |    NOT USED                                           ³
	 ³    NOT USED                                                ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³ set up the Port B reg of 8273, Reset all valid bits. ³
	 ³ However, don't set DTR at this time...               ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
	 acb->cmd_parms.cmd=RESET_8273_PORT_B_CMD;
	 acb->cmd_parms.parm[0]=0xCA;
	 acb->state.port_b_8273=acb->cmd_parms.parm[0];
	 acb->cmd_parms.parm_count=1;
	 if((rc=que_command(acb)) ) return(rc);
 
         /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
         ³ If being set up as Continuous Carrier, ³
	 ³ turn RTS on now so it will always be on³
         ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
         if( acb->strt_blk.data_flags & DATA_FLG_C_CARR_ON )
         {
                bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
                acb->cmd_parms.cmd=SET_8273_PORT_B_CMD;
                acb->cmd_parms.parm[0]=SET_8273_PORT_B_RTS;
                acb->state.port_b_8273 |=SET_RTS;
                acb->cmd_parms.parm_count=1;
                if( (rc=que_command(acb)) ) return (rc);
         }

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ set the station type in adapter control block. ³
	³ if no poll_addr is specified, set up in GENERAL³
	³ receive mode (passes all frames to memory).    ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if(acb->strt_blk.poll_addr) 
	{
		acb->station_type = SELECTIVE; /* setup as selective receiver */
	}
	else
	{
		 acb->station_type = GENERAL; /* setup as general receiver */
	}

	MPATRACE4("Sdl2",acb->dev,acb->station_type,acb->strt_blk.poll_addr);


	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Set up 8255 Port C for operation. ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	   ³ Enable external clock, DMA, and interrupts ³ 
	   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	   value = 0x00;
	   value |= GATE_EXT_CLK;
	   if(PIO_PUTC( acb, PORT_C_8255, value)==-1) 
	   {
	       mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
	       return EIO;
	   }
	   acb->state.port_c_8255 = value;

	if(acb->flags&MPADEAD) 
	{
	    mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
	    return EIO;
	}

	MPATRACE3("SdlX",acb->dev,acb->flags);
	return(0);
} /* sdlc_init */

int cio_start (struct acb        *acb,
	       t_start_dev  *arg,
	       int          devflag,
	       int          chan)
{
     int     rc=0;

     MPATRACE4("CstE",acb->dev,devflag,chan);

     if ( acb->flags & STARTED_CIO ) 
     {
          	MPATRACE4("Cse1",acb->dev,devflag,chan);
          	return EBUSY;
     }

     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
     ³ get parameter block ³
     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
     bzero(&acb->strt_blk,sizeof(t_start_dev));
     if (rc = COPYIN (devflag, (caddr_t)arg, &acb->strt_blk, 
					sizeof(t_start_dev))) 
     {
             	MPATRACE4("Cse2",acb->dev,chan,rc);
	     	rc= EFAULT;
	     	return rc;
     }
    
     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
     ³ Check to see if baud_rate is set, or if ³
     ³ the physical link is not RS232, or data ³
     ³ protocol is not SDLC Half Duplex        ³
     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
     if ( (acb->strt_blk.baud_rate != 0) || 
		(!(acb->strt_blk.phys_link & PHYS_232)) ||
		(acb->strt_blk.data_proto != DATA_PROTO_SDLC_HDX) )
     {
            	MPATRACE4("Cse3",acb->strt_blk.baud_rate,acb->strt_blk.phys_link,acb->strt_blk.data_proto);
		return (EINVAL);
     }

     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
     ³ Clear the status structure ³
     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
     bzero(&acb->stats,sizeof(mpa_query_t));

     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
     ³ Get the poll address from the start block ³
     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
     acb->station_addr = acb->strt_blk.poll_addr;

     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
     ³ make sure adapter is reset. ³
     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
     if( (rc = shutdown_adapter(acb)) )
     {
             	MPATRACE4("Cse4",acb->dev,chan,rc);
	     	return rc;
     }

     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
     ³ set the adapter up for SDLC transfers ³
     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
     if( (rc = sdlc_init(acb)) )
     {
             	MPATRACE4("Cse5",acb->dev,chan,rc);
	     	return rc;
     }

     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
     ³ handle modem logic for the connection ³
     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
     if( (rc=ctrl_modem(acb)) ) 
     {
             	MPATRACE4("Cse5",acb->dev,chan,rc);
		return rc;
     }
     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
     ³ Kick off the first recv.. There will always be a recive     ³
     ³ outstanding with this driver design. When xmits are         ³
     ³ required the recv is aborted until after the xmit completes ³
     ³ then restarted.                                             ³
     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
     if( (rc = startrecv(acb)) )
     {
             MPATRACE4("Cse6",acb->dev,chan,rc);
             return rc;
     }
     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
     ³ Start receive timer now, but it won't start reporting timeouts ³
     ³ until the line is active.                                      ³
     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
     if ( (acb->strt_blk.rcv_timeout != 0) &&
             (!(acb->flags & RCV_TIMER_ON)) )
     {
             acb->flags |= RCV_TIMER_ON;
             timeout(recv_timer, (caddr_t)acb, (HZ/10) * (int)acb->strt_blk.rcv_timeout );
     }

     MPATRACE3("CstX",acb->dev,rc);
     return rc;

}   /* cio_start */

int cio_halt (struct acb        *acb,
	      cio_sess_blk_t  *arg,
	      int          devflag,
	      int          chan)
{
     int     rc=0,cnt=0;
     cio_sess_blk_t  sess_blk;


     MPATRACE2("ChlE",acb->dev);
     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
     ³ get parameter block ³
     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
     if (COPYIN (devflag, (caddr_t)arg, &sess_blk, sizeof(sess_blk))) 
     {
             MPATRACE2("Che1",acb->dev);
	     return EFAULT;
     }

     if ( !(acb->flags & STARTED_CIO) )
     {
     	sess_blk.status = CIO_OK;
     	async_status(acb,CIO_HALT_DONE, CIO_OK,0,0, 0);
        MPATRACE3("Chlx",acb->dev,acb->flags);
 	return 0; /* Device already halted */
      }

     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
     ³ do not shutdown if xmit pending   ³ 
     ³ NOTE: irpts not disabled so xmits ³
     ³ can complete                      ³
     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
     while( (acb->act_xmit_head != NULL) && ++cnt < 49 )
     {
	delay(1);
     }

     if( (rc=shutdown_adapter(acb)) ) 
     {
          MPATRACE2("Che2",acb->dev);
	  return rc;
     }

     free_active_q(acb);

     sess_blk.status = CIO_OK;

     async_status(acb,CIO_HALT_DONE, CIO_OK,0,0, 0);

     MPATRACE3("ChlX",acb->dev,rc);
     return 0;

}    /* cio_halt */

int change_parms ( struct acb   *acb,
                  t_chg_parms   *arg,
                  int           devflag,
                  int           chan)
{    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
     ³ start of change parameters ³
     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

     int                     chg_mask;       /* mask for parameters */
     int                     parm1, parm2;   /* parameters          */
     int                     rc = 0;         /* return code         */
     int                     lvl;            /* saved irpt level    */
     t_chg_parms             new_parm;       /* chg parms struct    */


     MPATRACE2("CpmE",acb->dev);

     bzero(&new_parm,sizeof(t_chg_parms));
     if (rc = COPYIN (devflag, (caddr_t)arg, &new_parm, sizeof(t_chg_parms)))
     {
             return EFAULT;
     }

        parm1 = (int) new_parm.rcv_timer;
        parm2 = (int) new_parm.poll_addr;

        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³  Set the change mask value to determine which parameter   ³
        ³   needs to be changed (rcv_timer, poll_addr, or both).    ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

        chg_mask = new_parm.chg_mask;

        MPATRACE4("Cpm1",acb->dev,parm1,parm2);

        if (chg_mask & FS_RCV_TMR) /* receive timer needs to be changed */
        {
        	MPATRACE2("Cpm2",acb->dev);
                acb->strt_blk.rcv_timeout = new_parm.rcv_timer;

                /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                ³ If timer is on and new rcv_timeout is zero, stop timer ³
                ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                if ( acb->strt_blk.rcv_timeout == 0 )
                {
                        untimeout(recv_timer,(caddr_t)acb);
			acb->flags &= ~RCV_TIMER_ON;
                } 
        }

        if (chg_mask & FS_POLL_ADDR) /* poll address needs to be changed */
        {
        	MPATRACE2("Cpm3",acb->dev);

		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ Disable the receiver before ³
		³ changing the poll address.  ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                if ( acb->strt_blk.rcv_timeout != 0 )
                {
                        untimeout(recv_timer,(caddr_t)acb);
			acb->flags &= ~RCV_TIMER_ON;
                } 
		if ( (rc = stoprecv(acb)) )
		{
                      MPATRACE2("Cpe1",rc);
		      return rc;
		} 

                /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                ³ Setup the adapter to perform selective ³
                ³ receives on the poll address passed in.³
                ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                acb->station_addr = new_parm.poll_addr;
                acb->station_type = SELECTIVE;

		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ Enable the receiver again once ³
		³ the poll address is changed.   ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		if ( (rc = startrecv(acb)) )
     		{
                      MPATRACE2("Cpe2",rc);
		      return rc;
		}
		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ start the receive timer if indicated ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                if ( (acb->strt_blk.rcv_timeout != 0 ) && 
			(!(acb->flags & RCV_TIMER_ON)) )
                {
			acb->flags |= RCV_TIMER_ON;
			timeout(recv_timer, (caddr_t)acb, (HZ/10) * (int)acb->strt_blk.rcv_timeout );
                }  
         }

        MPATRACE2("CpmX",rc);
        return rc;

} /* change_parms */
