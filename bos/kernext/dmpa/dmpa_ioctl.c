static char sccsid[] = "@(#)73        1.1  src/bos/kernext/dmpa/dmpa_ioctl.c, sysxdmpa, bos411, 9428A410j 4/30/93 12:49:05";
/*
 *   COMPONENT_NAME: (SYSXDMPA) MP/A DIAGNOSTICS DEVICE DRIVER
 *
 *   FUNCTIONS: cio_get_stat
 *		cio_halt
 *		cio_query
 *		cio_start
 *		cmd_8273
 *		fetch_acb
 *		mpaioctl
 *		rw_port
 *		rw_pos
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
#include <sys/dmpauser.h>
#include <sys/dmpadd.h>
#include <sys/param.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/sysdma.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/xmem.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>


/*
 * NAME: mpaioctl
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

int mpaioctl ( dev_t            dev,
	       int              cmd,
	       caddr_t          arg,
	       int              devflag,
	       int              chan,
	       int              ext )

{
    int                         error;         /* return value  */
    int                         error1;        /* return value  */
    struct acb                  *acb;        /* pointer to acb struct */

    /* log a trace hook */

    DDHKWD5(HKWD_DD_MPADD, DD_ENTRY_IOCTL, 0, dev, cmd,
	    devflag, chan, ext);

    if ( ((acb = get_acb(minor(dev))) == NULL) ||
		    !(OPENP.op_flags&OP_OPENED) ) {

	    DDHKWD1(HKWD_DD_MPADD, DD_EXIT_IOCTL, ENXIO, dev);
	    return ENXIO;
    }

    if (lockl(&acb->adap_lock, LOCK_SIGRET) != LOCK_SUCC) {
	    DDHKWD1(HKWD_DD_MPADD, DD_EXIT_IOCTL, EINTR, dev);
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

	case CIO_QUERY: /* Query driver Statistics Command */
		error = cio_query(acb, arg, devflag, chan);
	    break;

	case CIO_GET_STAT: /* Get Status Command */
	    error = cio_get_stat(acb, arg, devflag, chan);
	    break;

	case MPA_RW_PORT:       /* Read or write adapter ports */
	    error = rw_port(acb, arg, devflag, chan);
	    break;

	case MPA_CMD_8273:      /* Send cmd to 8273 */
	    error = cmd_8273(acb, arg, devflag, chan);
	    break;

	case MPA_RW_POS:        /* Read pos regs    */
	    error = rw_pos(acb, arg, devflag, chan);
	    break;

	case MPA_GET_ACB:       /* Send cmd to 8273 */
	    error = fetch_acb(acb, arg, devflag, chan);
	    break;

	default:
	    error = 0;
	    break;
    }

    DDHKWD1(HKWD_DD_MPADD, DD_EXIT_IOCTL, error, dev);

    /* free access through segment register in bus_sr */

    unlockl(&acb->adap_lock);

    return (error);
}      /* mpaioctl() */

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


	/* I set this flag here so that if an irpt line is hung */
	/* The irpt handler can disable the card and set MPADEAD*/
	/* Then this start code can check for MPADEAD and know  */
	/* There is a problem and retrun a bad rc to application*/
	acb->flags &= ~MPADEAD;
	acb->stats.ds.irpt_fail = 0;

	/*
	**  Drop the 8273 reset bit.
	**                        The 8273 was put in a reset
	**  state by the config code or the close or halt code.
	*/
	while ( ++cntr < 2) {
	   if(PIO_PUTC(acb, PORT_B_8255, 0x07 )==-1) {
	       /* if this fails then pos2 enable may be off try */
	       /* setting it back on and try again */
	       pos = MPA_IOCC_ATT;
	       BUS_PUTC( pos + POS2, acb->pos2 );
	       BUS_PUTC( pos + POS3, acb->pos3 );
	       IOCC_DET(pos);
	   }
	   else break;
	}
	if(cntr==2) {
	    return EIO;
	}
	if(acb->flags&MPADEAD) {
	    return EIO;
	}
	/*            8255 PORT C bit Assignment
	**  | d7 | d6 | d5 | d4 | d3 | d2 | d1 | d0 |
	**    |    |    |    |    |    |    |    |
	**    |    |    |    |    |    |    |    1= Gate Internal clk
	**    |    |    |    |    |    |    1= Gate External clk
	**    |    |    |    |    |    1 = Set Electronic Wrap
	**    |    |    |    |    0=Enable irpts and DMA req
	**    |    |    |    Oscillating= Recving data
	**    |    |    Oscillating= Timer 0 output
	**    |    0=Test active in Electronic Wrap Mode
	**    1=SDLC, 0 = BSC
	*/

	/*
	**  Only the lower nibble of port C is writable.
	**  Disable irpts and DMA and set 8273 in electronic wrap
	**  and degate both internal and external clocks.  This
	**  Isolates the 8273 from both modem(DCE) and microchannel
	**  interfaces for the setup procedures that follow:
	*/
	if(PIO_PUTC( acb, PORT_C_8255, 0x0C )==-1) {
	    return EIO;
	}
	if(acb->flags&MPADEAD) {
	    return EIO;
	}

	/*
	** set up the Operating mode reg of 8273..reset all valid bits
	*/
	bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
	acb->cmd_parms.cmd=RESET_OPER_MODE_CMD;
	acb->cmd_parms.parm[0]=0xC0;
	acb->cmd_parms.parm_count=1;
	acb->state.oper_mode_8273=acb->cmd_parms.parm[0];
	if((rc=que_command(acb)) ) return(rc);
	/*
	** Now with all bits reset , set the ones specified by the user.
	*/
	if( acb->strt_blk.mode_reg_flags || (acb->strt_blk.modem_flags&GATE_INT_CLK) ) {
	   value = acb->strt_blk.mode_reg_flags;
	   bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
	   acb->cmd_parms.cmd=SET_OPER_MODE_CMD;
	   if(acb->strt_blk.modem_flags&GATE_INT_CLK) value |= SET_PRE_FRAME_MODE;
	   acb->cmd_parms.parm[0]=value;
	   acb->cmd_parms.parm_count=1;
	   acb->state.oper_mode_8273 |= acb->cmd_parms.parm[0];
	   if((rc=que_command(acb)) ) return(rc);
	}


	/*
	** Reset all bits in the Serial I/O mode reg of 8273
	*/
	bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
	acb->cmd_parms.cmd=RESET_IO_MODE_CMD;
	acb->cmd_parms.parm[0]=0xF8;
	acb->cmd_parms.parm_count=1;
	acb->state.serial_io_8273 = acb->cmd_parms.parm[0];
	if((rc=que_command(acb)) ) return(rc);
	/*
	** Set the user supplied bits in Serial I/O mode reg
	*/
	if( acb->strt_blk.data_flags ) {
	   bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
	   acb->cmd_parms.cmd=SET_IO_MODE_CMD;
	   acb->cmd_parms.parm[0]=acb->strt_blk.data_flags;
	   acb->cmd_parms.parm_count=1;
	   acb->state.serial_io_8273 |= acb->cmd_parms.parm[0];
	   if((rc=que_command(acb)) ) return(rc);
	}


	/*
	** Reset the data transfer reg bit (makes default DMA)
	*/
	bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
	acb->cmd_parms.cmd=RESET_DATA_XFER_CMD;
	acb->cmd_parms.parm[0]=0xFE;
	acb->cmd_parms.parm_count=1;
	acb->state.data_xfer_8273=acb->cmd_parms.parm[0];
	if((rc=que_command(acb)) ) return(rc);
	/*
	** Set the user supplied value for data xfer.
	*/
	if( acb->strt_blk.xfer_mode ) {    /* default is DMA */
	   bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
	   acb->cmd_parms.cmd=SET_DATA_XFER_CMD;
	   acb->cmd_parms.parm[0]= acb->strt_blk.xfer_mode;
	   acb->cmd_parms.parm_count=1;
	   acb->state.data_xfer_8273 |= acb->cmd_parms.parm[0];
	   if((rc=que_command(acb)) ) return(rc);
	   if(acb->strt_blk.xfer_mode&SET_NO_DMA) acb->flags |= PIO_MODE;
	}


	/*
	** Reset the One bit delay reg of 8273
	*/
	bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
	acb->cmd_parms.cmd=RESET_1_BIT_DELAY_CMD;
	acb->cmd_parms.parm[0]=0x7F;
	acb->cmd_parms.parm_count=1;
	acb->state.one_bit_8273=acb->cmd_parms.parm[0];
	if((rc=que_command(acb)) ) return(rc);
	/*
	** Set the user supplied value for one bit delay.
	*/
	if( acb->strt_blk.bit_delay ) {
	   bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
	   acb->cmd_parms.cmd=SET_1_BIT_DELAY_CMD;
	   acb->cmd_parms.parm[0]=acb->strt_blk.bit_delay;
	   acb->cmd_parms.parm_count=1;
	   acb->state.one_bit_8273 |= acb->cmd_parms.parm[0];
	   if((rc=que_command(acb)) ) return(rc);
	}

	/*            8273 PORT B  bit Assignment
	**  | d7 | d6 | d5 | d4 | d3 | d2 | d1 | d0 |
	**    |    |    |    |    |    |    |    |
	**    |    |    |    |    |    |    |    RTS-Request to Send
	**    |    |    |    |    |    |    Reserved
	**    |    |    |    |    |    DTR-Data Terminal Ready
	**    |    |    |    |    Reserved
	**    |    |    |    V.25 bis ENABLE
	**    |    |    Flag Detect
	**    |    NOT USED
	**    NOT USED
	*/

	/*
	** set up the Port B reg of 8273, Reset all valid bits.
	*/
	bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
	acb->cmd_parms.cmd=RESET_8273_PORT_B_CMD;
	acb->cmd_parms.parm[0]=0xCA;
	acb->state.port_b_8273=acb->cmd_parms.parm[0];
	acb->cmd_parms.parm_count=1;
	if((rc=que_command(acb)) ) return(rc);
	/*
	**  Now set data terminal ready as default no matter what
	**  User does, and include his options for port B.
	*/
	value = 0x04;
	value |= acb->strt_blk.dial_flags;
	bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
	acb->cmd_parms.cmd=SET_8273_PORT_B_CMD;
	acb->cmd_parms.parm[0]=value;
	acb->state.port_b_8273 |= acb->cmd_parms.parm[0];
	acb->cmd_parms.parm_count=1;
	if((rc=que_command(acb)) ) return(rc);



	/*
	**  The input oscillator to the 8254 runs at 1.1939 MHZ.
	**  To get a bit per sec xfer rate the specfied count divides
	**  the input clock frequency.
	**  I will allow user to pass me the bps rate and I will find
	**  The right count to load in counter 0.
	*/
	if (!(acb->strt_blk.baud_rate)) acb->strt_blk.baud_rate = 9600; /* set default */

	count = ( 1193900 / acb->strt_blk.baud_rate) ;

	acb->stats.ds.bps_rate = acb->strt_blk.baud_rate;

	/*
	** Set up counter 0 for internal clock on 8254.
	** Data 0x3E as control word sets binary counter bit 0 = 0
	** mode 3 bits 1-3 = 111b
	** read/write lsb then msb set by bits 4-5 = 11b
	** point to counter 0 by bits 6-7 = 00b
	*/
		  /*   set up mode for counter 0 */
	if(PIO_PUTC( acb, CONTROL_OFFSET,0x3E)==-1) {
	    return EIO;
	}
			    /* write lsb of count */
	if(PIO_PUTC( acb, COUNT0_OFFSET,(uchar) count)==-1) {
	    return EIO;
	}

			    /* write msb of count */
	if(PIO_PUTC( acb, COUNT0_OFFSET,(uchar) (count>>8))==-1) {
	    return EIO;
	}

	/*
	** Note: In this card design gate timer0 is tied high (always
	**       enabled so I don't need to enable it. I will just start
	**       running and be used if 8255 Port C sets gate internal clk.
	*/

	/*
	** set the station type in adapter control block.
	*/
	if (acb->strt_blk.station_type==0x00||acb->strt_blk.station_type&PRIMARY) {
	     /* set default prinary */
	     acb->station_type |= PRIMARY;
	}
	else {
	     acb->station_type |= SECONDARY;
	     acb->station_addr = acb->strt_blk.select_addr;
	}

	/*
	** Set up 8255 Port B for operation.
	*/
	if( acb->strt_blk.port_b_8255 ) {       /* set user defines */
	   if(PIO_PUTC( acb, PORT_B_8255, acb->strt_blk.port_b_8255 )==-1) {
	       return EIO;
	   }
	   acb->state.port_b_8255 = acb->strt_blk.port_b_8255;
	}
	else {                              /* set defaults */
	   /*
	   ** timers 1 and 2 not gated, no irpt on level 4, no 8273 reset,
	   ** speed select on to pick faster modem speed, not test, no
	   ** select standby and no reset of modem status changed logic.
	   */
	   value = 0x00;
	   value = SEL_STANBY_OFF|TEST_OFF|FREE_STAT_CHG;
	   if(PIO_PUTC( acb, PORT_B_8255,value)==-1) {
	       return EIO;
	   }
	   acb->state.port_b_8255 = value;

	}

	/*
	** Set up 8255 Port C for operation.
	*/
	if( acb->strt_blk.modem_flags ) {       /* set user defines */
	   if(PIO_PUTC( acb, PORT_C_8255, acb->strt_blk.modem_flags)==-1) {
	       return EIO;
	   }
	   acb->state.port_c_8255 = acb->strt_blk.modem_flags;
	}
	else {                               /* set defaults */
	   /*
	   ** Leave irpts enabled and enable external clock
	   ** Reset electronic wrap and disabel the internal clock.
	   */
	   value = 0x00;
	   value |= GATE_EXT_CLK;
	   if(PIO_PUTC( acb, PORT_C_8255, value)==-1) {
	       return EIO;
	   }
	   acb->state.port_c_8255 = value;
	}
	if(acb->flags&MPADEAD) {
	    return EIO;
	}
	return(0);
}

/*************************************************************************
** NAME: cio_get_stat()
**
** FUNCTION:
**      This function handles the CIO_GET_STAT ioctl.
**
** EXECUTION ENVIRONMENT:
**
**      This routine executes on the process thread.
**
** NOTES:
**
**      Called from:  mpaioctl
**
**  Calls: copyout, enable, disable
**
** RETURNS:
**      0 - success
**      EFAULT - copyout() failed
**      EACCESS - called by a kernel process
**************************************************************************/
int
cio_get_stat (
	struct acb *acb,
	int arg,
	ulong devflag,
	chan_t chan)
{
	cio_stat_blk_t stat_blk;
	stat_elem_t *statp;
	stat_elem_t *tmp_statp;
	int spl;
	int rc = 0;

	DDHKWD2(HKWD_DD_MPADD, DD_ENTRY_GET_STAT, 0, acb->dev, chan);

	/*
	** Calling from a kernel process is not allowed
	*/
	if (devflag & DKERNEL) {
		DDHKWD1(HKWD_DD_MPADD, DD_EXIT_GET_STAT, EACCES, acb->dev);
		return EACCES;
	}

	DISABLE_INTERRUPTS(spl);

	/*
	** Check the "active" list
	*/
	if ((statp = acb->act_stat_head) == NULL) {
		stat_blk.code = (ulong)CIO_NULL_BLK;
		if (COPYOUT(devflag, &stat_blk, (caddr_t)arg, sizeof(stat_blk))) {
			rc = EFAULT;
		}
	} else {
		if (statp->stat_blk.code == CIO_LOST_STATUS) {
			OPENP.op_flags &= ~(LOST_STATUS | SENT_LOST_STATUS);
		}
		stat_blk = statp->stat_blk;
		if (COPYOUT(devflag, &stat_blk, (caddr_t)arg, sizeof(stat_blk))) {
			rc = EFAULT;
		}
		/*
		** Take the element from the "active" list.
		*/
		if (acb->act_stat_head==statp)
			acb->act_stat_head =statp->st_next;
		else {
		   tmp_statp = acb->act_stat_head;
		   while( (tmp_statp->st_next != statp) &&
				      (tmp_statp != NULL) )
			  tmp_statp = tmp_statp->st_next;
		   ASSERT(tmp_statp);
		   tmp_statp->st_next = statp->st_next;
		}
		if (acb->act_stat_head == NULL) acb->act_stat_tail = NULL;
		/*
		** This also resets the active flag
		*/
		bzero(statp,sizeof(stat_elem_t));

		/*
		** Add this buffer to the "free" list.
		*/
		if (acb->stat_free == NULL)
		   acb->stat_free = statp;
		else {
		   tmp_statp = acb->stat_free;
		   while(tmp_statp->st_next != NULL)
			  tmp_statp=tmp_statp->st_next;
		   tmp_statp->st_next = statp;
		}

		if ((OPENP.op_flags & LOST_STATUS)
			&& ((OPENP.op_flags & SENT_LOST_STATUS) == 0)) {
			async_status(acb, CIO_LOST_STATUS, 0, -1, -1, 0);
			OPENP.op_flags |= SENT_LOST_STATUS;
		}
	}

	ENABLE_INTERRUPTS(spl);

	DDHKWD3(HKWD_DD_MPADD, DD_EXIT_GET_STAT, rc,acb->dev,stat_blk.code,stat_blk.option[0]);
	return rc;
} /* cio_get_stat() */

int cio_start (struct acb        *acb,
	       mpa_start_t  *arg,
	       int          devflag,
	       int          chan)
{
     int     rc;

     if ( acb->flags & STARTED_CIO ) return EBUSY;

     /*
     ** get parameter block
     */
     if (rc = COPYIN (devflag, (caddr_t)arg, &acb->strt_blk, sizeof(mpa_start_t))) {
	     DDHKWD2(HKWD_DD_MPADD,DD_ENTRY_START,0,acb->dev,chan);
	     DDHKWD1(HKWD_DD_MPADD,DD_EXIT_START,rc,acb->dev);
	     rc= EFAULT;
	     goto start_exit;
     }
     DDHKWD5(HKWD_DD_MPADD,DD_ENTRY_START,0,acb->dev,chan,acb->strt_blk.baud_rate,
			acb->strt_blk.sb.netid,0);
     if ( acb->strt_blk.data_proto&SDLC ) {

	/*
	** make sure adapter is reset.
	*/
	if( (rc = shutdown_adapter(acb)) ) goto start_exit;

	/*
	** set the adapter up for SDLC transfers
	*/

	if( (rc = sdlc_init(acb)) ) goto start_exit;

	if (acb->strt_blk.diag_flags & XMIT_ONLY) acb->flags |= NO_RECV;
	else {
	    /*
	    **   Kick off the first recv.. There will always be a recive
	    **   outstanding with this driver design. When xmits are
	    **   required the recv is aborted until after the xmit completes
	    **   then restarted.
	    */
	    if( (rc = startrecv(acb)) ) goto start_exit;
	}
     }
     else rc = EINVAL;

start_exit:

     if(rc) {
	acb->strt_blk.sb.status = CIO_NOT_STARTED;
	COPYOUT(devflag, &acb->strt_blk, (caddr_t)arg, sizeof(mpa_start_t));
	async_status(acb,CIO_START_DONE, CIO_NOT_STARTED,rc,0, 0);
     }
     else  {
	acb->strt_blk.sb.status = CIO_OK;
	if (COPYOUT(devflag, &acb->strt_blk, (caddr_t)arg, sizeof(mpa_start_t)) != 0) {
	     DDHKWD2(HKWD_DD_MPADD,DD_EXIT_START,EFAULT,acb->dev,chan);
	     rc= EFAULT;
	     goto start_exit;
	}
	async_status(acb,CIO_START_DONE, CIO_OK,0,0, 0);
	acb->flags |= STARTED_CIO;
     }

     DDHKWD1(HKWD_DD_MPADD,DD_EXIT_START,rc,acb->dev);
     return rc;

}   /* cio_start */

int cio_halt (struct acb        *acb,
	      mpa_start_t  *arg,
	      int          devflag,
	      int          chan)
{
     int     rc=0,cnt=0;
     mpa_start_t  sess_blk;


     if ( !(acb->flags & STARTED_CIO) ) return 0;

     DDHKWD2(HKWD_DD_MPADD,DD_ENTRY_HALT,0,acb->dev,0);

     /*
     ** get parameter block
     */
     if (COPYIN (devflag, (caddr_t)arg, &sess_blk, sizeof(sess_blk))) {
	     DDHKWD2(HKWD_DD_MPADD,DD_EXIT_HALT,EFAULT,acb->dev,0xE0);
	     return EFAULT;
     }

     /* do not shutdown if xmit pending */
     /* NOTE: irpts not disabled so xmits can complete */
     while( (acb->act_xmit_head != NULL) && ++cnt < 10000);


     if( (rc=shutdown_adapter(acb)) ) {
	  DDHKWD2(HKWD_DD_MPADD,DD_EXIT_HALT,EFAULT,acb->dev,0xE1);
	  return rc;
     }

     free_active_q(acb);

     sess_blk.sb.status = CIO_OK;
     if (COPYOUT(devflag, &sess_blk, (caddr_t)arg, sizeof(sess_blk)) ) {
	 DDHKWD2(HKWD_DD_MPADD,DD_EXIT_HALT,EFAULT,acb->dev,0xE2);
	 return EFAULT;
     }
     async_status(acb,CIO_HALT_DONE, CIO_OK,0,0, 0);

     DDHKWD2(HKWD_DD_MPADD,DD_EXIT_HALT,rc,acb->dev,0);
     return 0;

}    /* cio_halt */

int rw_port (struct acb        *acb,
	       rw_port_t    *arg,
	       int          devflag,
	       int          chan)
{
     int             rc;
     rw_port_t       sess_blk;

     /*
     ** get parameter block
     */
     if (rc = COPYIN (devflag, (caddr_t)arg, &sess_blk, sizeof(sess_blk))) {
	     return(EFAULT);
     }


     if(sess_blk.rw_flag&MPA_READ) {
	   if( (rc = PIO_GETC( acb, sess_blk.port_addr)) == -1) {
	       return EIO;
	   }

	   sess_blk.value = rc;
	   if (COPYOUT(devflag, &sess_blk, (caddr_t)arg, sizeof(sess_blk)) != 0) {
		   return( EFAULT);
	   }
	   rc = 0;
     }
     else {
	if(PIO_PUTC( acb, sess_blk.port_addr, sess_blk.value )==-1) {
	    return EIO;
	}
     }

     return rc;

}     /* rw_port() */

int cmd_8273 (struct acb        *acb,
	       cmd_phase_t  *arg,
	       int          devflag,
	       int          chan)
{
     int     rc;
     cmd_phase_t  sess_blk;

     if ( !( acb->flags & STARTED_CIO) ) return EINVAL;

     bzero(&acb->cmd_parms,sizeof(cmd_phase_t));

     /*
     ** get parameter block
     */
     if ( COPYIN (devflag, (caddr_t)arg, &acb->cmd_parms, sizeof(cmd_phase_t))) {
	     return(EFAULT);
     }


    if( (rc = que_command(acb)) ) return rc;

    if(acb->cmd_parms.flag&RETURN_RESULT) {
	  if (COPYOUT(devflag, &acb->cmd_parms, (caddr_t)arg, sizeof(cmd_phase_t)) != 0) {
		  return( EFAULT);
	  }
    }

    return rc;

}

int rw_pos (struct acb        *acb,
	       rw_port_t    *arg,
	       int          devflag,
	       int          chan)
{
     int          cntr = 0;
     rw_port_t    sess_blk;
     ulong        pos;
     uchar        value;

     /*
     ** get parameter block
     */
     if (COPYIN (devflag, (caddr_t)arg, &sess_blk, sizeof(sess_blk))) {
	     return(EFAULT);
     }

    pos = MPA_IOCC_ATT;

    if(sess_blk.rw_flag&MPA_READ) {
	  sess_blk.value = BUS_GETC( pos + sess_blk.port_addr);
	  if (COPYOUT(devflag, &sess_blk, (caddr_t)arg, sizeof(sess_blk)) != 0) {
		  IOCC_DET(pos);
		  return( EFAULT);
	  }
    }
    else {
	  /* put in a loop to try 10 times to get it right, if it */
	  /* fails return error                                   */
	  while (++cntr < 10) {
	     BUS_PUTC( pos + sess_blk.port_addr, sess_blk.value );
	     value = BUS_GETC( pos + sess_blk.port_addr);
	     if( (value&0x1F) == (sess_blk.value&0x1F) ) break;
	  }
	  if(cntr==10) {
	    IOCC_DET(pos);
	    return(EFAULT);
	 }
    }

    IOCC_DET(pos);
    return 0;

}    /* rw_pos() */
/*********************************************************************
** NAME: cio_query()
**
** FUNCTION:
** This function handles the CIO_QUERY ioctl.  The statistics data
** collected by this driver is returned to the user process.
**
** EXECUTION ENVIRONMENT: process thread only
**
** INPUT:       pointer to the adapter structure
**                      address of the user process' argument structure
**                      the mode flags for this open
**                      the channel number or the minor device's minor number
**
** OUTPUT:      see notes for mpaioctl
**
** CALLERS:     mpaioctl
**
** CALLEES:     COPYIN(), COPYOUT(), bzero(), ENABLE_INTERRUPTS(),
**                      DISABLE_INTERRUPTS()
**
** RETURNS: 0 ------ successful completion
**                      EFAULT - COPYIN() or COPYOUT failed
**********************************************************************/
int
cio_query (
	struct acb *acb,
	int arg,
	ulong devflag,
	chan_t chan)
{
	cio_query_blk_t qparms;
	int spl;

	DDHKWD2(HKWD_DD_MPADD, DD_ENTRY_QUERY, 0, acb->dev, chan);

	/*
	** get the caller's parameters
	*/
	if (COPYIN (devflag, (caddr_t)arg, &qparms, sizeof(qparms))) {
		DDHKWD2(HKWD_DD_MPADD,DD_EXIT_QUERY,EFAULT,acb->dev,0xD0);
		return EFAULT;
	}

	/*
	** Check for data truncation.
	*/
	if (qparms.buflen < sizeof(mpa_query_t)) {
		qparms.status = CIO_BUF_OVFLW;
		DDHKWD2(HKWD_DD_MPADD,DD_EXIT_QUERY,EINVAL,acb->dev,0xD1);
		COPYOUT (devflag, &qparms, (caddr_t)arg, sizeof(qparms));
		return EINVAL;
	}
	qparms.status = CIO_OK;

	/*
	** Copy data to caller's buffer and clear the
	** RAS area if indicated.
	*/
	DISABLE_INTERRUPTS(spl);
	if (COPYOUT(devflag, &acb->stats, qparms.bufptr,
		MIN(qparms.buflen, sizeof(mpa_query_t))))  {
		DDHKWD2(HKWD_DD_MPADD,DD_EXIT_QUERY,EFAULT,acb->dev,0xD2);
		ENABLE_INTERRUPTS(spl);
		return EFAULT;
	}
	if (qparms.clearall == CIO_QUERY_CLEAR)
		bzero(&acb->stats, sizeof(mpa_query_t));
	ENABLE_INTERRUPTS(spl);

	/*
	** return parameter block to caller
	*/
	if (COPYOUT(devflag, &qparms, (caddr_t)arg, sizeof(qparms))) {
		DDHKWD2(HKWD_DD_MPADD,DD_EXIT_QUERY,EFAULT,acb->dev,0xD3);
		return EFAULT;
	}

	DDHKWD2(HKWD_DD_MPADD,DD_EXIT_QUERY,0,acb->dev,0);
	return 0;
} /* cio_query() */
/*********************************************************************
** NAME: fetch_acb()
**
** FUNCTION:
** This function returns the adapter control block to an application.
**
** EXECUTION ENVIRONMENT: process thread only
**
** INPUT:       pointer to the adapter structure
**                      address of the user process' argument structure
**                      the mode flags for this open
**                      the channel number or the minor device's minor number
**
** OUTPUT:      see notes for mpaioctl
**
** CALLERS:     mpaioctl
**
** CALLEES:     COPYIN(), COPYOUT(), bzero(), ENABLE_INTERRUPTS(),
**                      DISABLE_INTERRUPTS()
**
** RETURNS: 0 ------ successful completion
**                      EFAULT - COPYIN() or COPYOUT failed
**********************************************************************/
int
fetch_acb (
	struct acb *acb,
	int arg,
	ulong devflag,
	chan_t chan)
{

	cio_query_blk_t qparms;
	int spl;

	/*
	** get the caller's parameters
	*/
	if (COPYIN (devflag, (caddr_t)arg, &qparms, sizeof(qparms))) {
		return EFAULT;
	}

	/*
	** Check for data truncation.
	*/
	if (qparms.buflen < sizeof(struct acb)) {
		qparms.status = CIO_BUF_OVFLW;
		COPYOUT (devflag, &qparms, (caddr_t)arg, sizeof(qparms));
		return EINVAL;
	}
	qparms.status = CIO_OK;

	/*
	** Copy data to caller's buffer and clear the
	** RAS area if indicated.
	*/
	DISABLE_INTERRUPTS(spl);
	if (COPYOUT(devflag, acb, qparms.bufptr,
		MIN(qparms.buflen, sizeof(struct acb))))  {
		ENABLE_INTERRUPTS(spl);
		return EFAULT;
	}
	ENABLE_INTERRUPTS(spl);

	/*
	** return parameter block to caller
	*/
	if (COPYOUT(devflag, &qparms, (caddr_t)arg, sizeof(qparms))) {
		return EFAULT;
	}

	return 0;
} /* fetch_acb() */


