static char sccsid[] = "@(#)21	1.9  src/bos/kernext/mpa/mpa_util.c, sysxmpa, bos41J, 9520B_all 5/17/95 14:52:03";
/*
 *   COMPONENT_NAME: (SYSXMPA) MP/A SDLC DEVICE DRIVER
 *
 *   FUNCTIONS: PioGet
 *		PioPut
 *		async_status
 *		ctrl_modem
 *		dma_request
 *		free_active_q
 *		free_dma_elem
 *		free_irpt_elem
 *		free_recv_elem
 *		free_xmit_elem
 *		mpalog
 *		que_command
 *		recv_timer
 *		ring_timer
 *		startrecv
 *		stoprecv
 *		unchain_mbuf
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/adspace.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <errno.h>
#include <stddef.h>
#include <sys/except.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/sysdma.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/errids.h>
#include <sys/timer.h>
#include <sys/xmem.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>

#include <sys/mpadd.h>

void recv_timer();
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³    Global Variable Declarations                                ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
int RI_prev=0, RI_currnt=0;
/*---------------------   Q U E _ C O M M A N D   ----------------------*/
/*                                                                      */
/*  NAME: que_command                                                   */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Issues a command to the adapter by placing it on the            */
/*      adapter's command location then sends the parms specified.      */
/*      The caller must supply a cmd_phase_t structure with the         */
/*      command and parms and number of parms specified.                */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from process or interrupt environment.            */
/*                                                                      */
/*  DATA STRUCTURES:    cmd_phase_t                                     */
/*                                                                      */
/*                                                                      */
/*  RETURNS:                                                            */
/*      EIO     PIO error occurred.                                     */
/*      EDEADLK The adapter is not responding it must be dead.          */
/*      0       If the queue succeeded.                                 */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:                                         */
/*                               i_disable,PioGet,                      */
/*                               i_enable,PioPut,mpalog                 */
/*                                                                      */
/*                                                                      */
/*----------------------------------------------------------------------*/

int que_command (
		 struct acb  *acb )     /* pointer to adapter control block */
{
	int             i;             /* for loop counter               */
	int             rc=0;          /* return code for this routine   */
	int             cnt=0;         /* counter to prevent system hang */
	int             spl;           /* used for disabeling interrupts */
	int		delay_reg;
	int		tmp;


	DISABLE_INTERRUPTS(spl);

        MPATRACE3("QcmE",acb->dev,acb->cmd_parms.cmd);


	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³  Wait for command busy = 0  in the 8273 status register.         ³
	³  This loops will exit if CBSY bit does not drop within 5 seconds ³
	³  This limit should avoid system hangs with interrupts disabled.  ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

	while(++cnt<Q_CMD_TIMEOUT) 
	{
	    if( (rc=PIO_GETC( acb,  RD_STAT_OFFSET)) == -1) 
	    {
               MPATRACE3("Qcm1",acb->dev,acb->cmd_parms.cmd);
	       mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
	       ENABLE_INTERRUPTS(spl);
	       return EIO;                       /* pio error occurred */
	    }
	    if(rc & CBSY)
	    {
	       ENABLE_INTERRUPTS(spl);
	       delay_reg = (uint)IOCC_ATT(DDS.bus_id, DL_DELAY_REG );
               tmp = BUSIO_GETC(delay_reg);  /* delay 1 microsecond */
               IOCC_DET(delay_reg);
	       DISABLE_INTERRUPTS(spl);
	    }
	    else
	       break;

	}
	if(cnt==Q_CMD_TIMEOUT) 
	{
	   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	   ³  If we waited 5 sec to see busy bit drop, assume the ³
	   ³  adapter is down and exit with EDEADLK rc.           ³
	   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
           MPATRACE3("Qcm2",acb->dev,acb->cmd_parms.cmd);
	   mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
	   ENABLE_INTERRUPTS(spl);
	   return(EDEADLK);                      /* avoid system hang */
	}

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Write the command to the 8273 command register, now that the   ³
	³ command reg is free. acb->cmd_parms.cmd..has the command value ³
	³ as set by the called of que_command().                         ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if(PIO_PUTC( acb, WR_CMD_OFFSET, acb->cmd_parms.cmd)==-1) 
	{
           MPATRACE3("Qcm3",acb->dev,acb->cmd_parms.cmd);
	   mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
	   ENABLE_INTERRUPTS(spl);
	   return EIO;                           /* pio error occurred */
	}

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ write the parameters to the 8273 parameter register.        ³
	³ NOTE: acb->cmd_parms.parm_count is set by the caller.       ³
	³       it has the number of parms to write for this commnad. ³
	³       acb->cmd_parms.parm[i].. has the value of each parm.  ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	for(i=0; i<acb->cmd_parms.parm_count; i++) 
	{
	     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	     ³  Wait for free parm reg, CPBF bit = 0 in 8273 status reg. ³
	     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

	     while(++cnt<Q_CMD_TIMEOUT) 
	     {
		if( (rc=PIO_GETC( acb, RD_STAT_OFFSET)) == -1) 
		{
           	   MPATRACE3("Qcm4",acb->dev,acb->cmd_parms.cmd);
		   mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
		   ENABLE_INTERRUPTS(spl);
		   return EIO;                   /* pio error occurred */
		}
	    	if(rc & CPBF)
	    	{
	           ENABLE_INTERRUPTS(spl);
	       	   delay_reg = (uint)IOCC_ATT(DDS.bus_id, DL_DELAY_REG );
                   tmp = BUSIO_GETC(delay_reg);  /* delay 1 microsecond */
                   IOCC_DET(delay_reg);
	           DISABLE_INTERRUPTS(spl);
	    	}
	    	else
	       	   break;
	     }
	     if(cnt==Q_CMD_TIMEOUT) 
	     {
           	MPATRACE3("Qcm5",acb->dev,acb->cmd_parms.cmd);
	   	mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
		ENABLE_INTERRUPTS(spl);
		return(EDEADLK);                 /* avoid system hang */
	     }

             MPATRACE4("Parm",acb->dev,acb->cmd_parms.cmd,acb->cmd_parms.parm[i]);
	     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	     ³ Now that the parm reg is free , write parm # i ³
	     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	     if(PIO_PUTC(acb, WR_PARM_OFFSET, acb->cmd_parms.parm[i])==-1) 
	     {
           	MPATRACE3("Qcm6",acb->dev,acb->cmd_parms.cmd);
		mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
		ENABLE_INTERRUPTS(spl);
		return EIO;                      /* pio error occurred */
	     }
	}

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³  For immediate commands a result will be returned. If the  ³
	³  caller expects a result, wait for it and return it to him ³
	³  in acb->cmd_parms.result.                                 ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if(acb->cmd_parms.flag&RETURN_RESULT) 
	{
	     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	     ³  Wait for result available bit in status reg.                ³
	     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

	     while(++cnt<Q_CMD_TIMEOUT) 
	     {
		if( (rc=PIO_GETC( acb, RD_STAT_OFFSET)) == -1) 
		{
           	   MPATRACE3("Qcm7",acb->dev,acb->cmd_parms.cmd);
		   mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
		   ENABLE_INTERRUPTS(spl);
		   return EIO;                   /* pio error occurred */
		}
	    	if(!(rc & CRBF))
	    	{
	           ENABLE_INTERRUPTS(spl);
	       	   delay_reg = (uint)IOCC_ATT(DDS.bus_id, DL_DELAY_REG );
                   tmp = BUSIO_GETC(delay_reg);  /* delay 1 microsecond */
                   IOCC_DET(delay_reg);
	           DISABLE_INTERRUPTS(spl);
	    	}
	    	else
	       	   break;
	     }

	     if(cnt==Q_CMD_TIMEOUT) 
	     {
           	MPATRACE3("Qcm8",acb->dev,acb->cmd_parms.cmd);
	   	mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
		ENABLE_INTERRUPTS(spl);
		return(EDEADLK);                 /* avoid system hang */
	     }

	     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	     ³ Now read the result reg, since the result bit came on. ³
	     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	     if( (rc=PIO_GETC( acb, RD_RES_OFFSET)) == -1) 
	     {
           	   MPATRACE3("Qcm9",acb->dev,acb->cmd_parms.cmd);
		   mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
		   ENABLE_INTERRUPTS(spl);
		   return EIO;                   /* pio error occurred */
	     }
	     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	     ³ set the result value for the caller ³
	     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	     acb->cmd_parms.result = (uchar) rc;
	}

        MPATRACE4("QcmX",acb->dev,acb->cmd_parms.cmd,rc);
	ENABLE_INTERRUPTS(spl);
	return( 0 );                            /* return successful */
}


/*---------------------   M P A L O G  ---------------------------------*/
/*                                                                      */
/*  NAME: mpalog                                                        */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      This routine is used to write error log entries to the          */
/*      system error log.  It includes a text message supplied          */
/*      by the caller in acb->err_text.                                 */
/*      There is only one errid (MPA_ERROR).                            */
/*                                                                      */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from process or interrupt environment.            */
/*                                                                      */
/*  DATA STRUCTURES:    acb                                             */
/*                                                                      */
/*                                                                      */
/*  RETURNS:                                                            */
/*      VOID                                                            */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:                                         */
/*                                sprintf,strncpy, errsave              */
/*                                                                      */
/*                                                                      */
/*----------------------------------------------------------------------*/

 mpalog (struct acb     *acb,      /* pointer to adapter control block */
	 int            err_id,    /* the errid for this error         */
	 int            line,      /* line number in code, error found */
	 char           *file,     /* src file error found             */
	 uint           data1,     /* reason code, or return code      */
	 uint           data2)     /* reason code, or return code      */
 {
	 struct errormsg   msglog;   /* errormsg struct, defined in mpauser.h */
	 char         errbuf[300];   /* temp data buffer */

	 bzero(&msglog,sizeof(struct errormsg));
	 msglog.err.error_id = err_id;

         sprintf(msglog.err.resource_name, "%s",DDS.resource_name);

	 sprintf(errbuf, "line: %d file: %s ",line,file);
	 strncpy(msglog.file, errbuf, (size_t)sizeof(msglog.file));
	 msglog.data1 = data1;
	 msglog.data2 = data2;

	 errsave(&msglog, (uint)sizeof(struct errormsg));
	 return;
 }

/*--------------------- A S Y N C _ S T A T U S ------------------------*/
/*                                                                      */
/*  NAME: async_status                                                  */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Report status to the specified open structure.                  */
/*                                                                      */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from process or interrupt environment.            */
/*                                                                      */
/*  DATA STRUCTURES:    acb                                             */
/*                                                                      */
/*                                                                      */
/*  RETURNS:                                                            */
/*           0 ------ Success                                           */
/*           ENOMEM - Could not allocate a status queue element         */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:                                         */
/*                               bzero, i_disable, i_enable,            */
/*                               selnotify                              */
/*                                                                      */
/*----------------------------------------------------------------------*/

int
async_status (
	struct acb *acb,           /* pointer to adapter control block */
	int code,                  /* cio_stat_blk_t       code        */
	int opt0,                  /* cio_stat_blk_t option[0]         */
	int opt1,                  /* cio_stat_blk_t option[1]         */
	int opt2,                  /* cio_stat_blk_t option[2]         */
	int opt3)                  /* cio_stat_blk_t option[3]         */
{
	cio_stat_blk_t kstat_blk;  /* status structure from comio.h    */
	int spl;                   /* used for disable and enable irpts*/


	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³  If the device is not open return invalid command ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if( !(OPENP.op_flags & OP_OPENED) ) return EINVAL;

	bzero(&kstat_blk, sizeof(cio_stat_blk_t));
	kstat_blk.code          = (ulong)code;
	kstat_blk.option[0]     = (ulong)opt0;
	kstat_blk.option[1]     = (ulong)opt1;
	kstat_blk.option[2]     = (ulong)opt2;
	kstat_blk.option[3]     = (ulong)opt3;
	(*(OPENP.mpa_kopen.stat_fn)) (OPENP.mpa_kopen.open_id, &kstat_blk);

	return 0;

} /* async_status() */


/*--------------------- F R E E _ R E C V _ E L E M --------------------*/
/*                                                                      */
/*  NAME: free_recv_elem                                                */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Free up the given recv element. Take it off the active list     */
/*      and put it back on the free list and notify anyone sleeping     */
/*      on a free recv element event.                                   */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from process or interrupt environment.            */
/*                                                                      */
/*  DATA STRUCTURES:    acb, recv_elem_t   (defined in mpadd.h)         */
/*                                                                      */
/*                                                                      */
/*  RETURNS:                                                            */
/*           VOID                                                       */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:                                         */
/*                               startrecv, e_wakeup, i_disable,        */
/*                               i_enable, bzero                        */
/*                                                                      */
/*----------------------------------------------------------------------*/

void
free_recv_elem (
	struct acb *acb,            /* pointer to adapter control block */
	recv_elem_t *recvp)         /* pointer to recv element to free  */
{
	recv_elem_t *tmp_recvp;     /* temp recv element pointer        */
	int                spl;     /* used to disable interrupts       */

        MPATRACE3("FrRE",acb->dev,recvp);

	if(recvp == NULL)
	{
                MPATRACE2("NoRx",acb->dev);
                return;
	}

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³  It the state of this recv element is not active it has ³
	³  already been freed to just return to the caller.       ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if(!(recvp->rc_state&RC_ACTIVE)) 
	{
            MPATRACE3("FrRx",acb->dev,recvp->rc_state);
	    return;
	}

	DISABLE_INTERRUPTS(spl);

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Take the receive element off the active list.³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if (acb->act_recv_head == recvp)
		acb->act_recv_head = recvp->rc_next;
	else 
	{
	   tmp_recvp = acb->act_recv_head;
	   while( (tmp_recvp->rc_next != recvp) && (tmp_recvp != NULL) )
		  tmp_recvp = tmp_recvp->rc_next;
	   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	   ³  If tmp_recvp is NULL the recv element has not yet been      ³
	   ³  put on the active list so just put it back on the free list ³
	   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	   if(tmp_recvp) tmp_recvp->rc_next = recvp->rc_next;
	}
	if (acb->act_recv_head == NULL) acb->act_recv_tail = NULL;

	bzero(recvp,sizeof(recv_elem_t));    /* clear the element */

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Put it on the free list ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	recvp->rc_next = acb->recv_free;
	acb->recv_free = recvp;

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ If the receiver is disabled due to someone running out of  ³
	³ receive elements, start the receiver now that an element   ³
	³ is free.                                                   ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if(acb->flags & NEED_RECV_ELEM) 
	{
	   if(!(acb->flags & RECEIVER_ENABLED)) 
	   {
		startrecv(acb);
	       	if ( (acb->strt_blk.rcv_timeout != 0) &&
                       	(!(acb->flags & RCV_TIMER_ON)) )
               	{
                       	acb->flags |= RCV_TIMER_ON;
                       	timeout(recv_timer, (caddr_t)acb, (HZ/10) * (int)acb->strt_blk.rcv_timeout );
               	}
	   }
	   acb->flags &= ~NEED_RECV_ELEM;
	}

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ If anyone is sleeping waiting for recv elements wake them  ³
	³ up.                                                        ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	ENABLE_INTERRUPTS(spl);	/* Defect 140957 */
	if (acb->op_rcv_event != EVENT_NULL)
		e_wakeup(&acb->op_rcv_event);

        MPATRACE3("FrRX",acb->dev,recvp);


	return;
} /* free_recv_element() */


/*--------------------- F R E E _ D M A _ E L E M ----------------------*/
/*                                                                      */
/*  NAME: free_dma_elem                                                 */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Free up the given dma  element. Take it off the active list     */
/*      and put it back on the free list and notify anyone sleeping     */
/*      on a free dma  element event.                                   */
/*      This routine is called to clean up after every recv or xmit     */
/*      dma. It does the d_complete and counts the recv and xmit dma's. */
/*                                                                      */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from process or interrupt environment.            */
/*                                                                      */
/*  DATA STRUCTURES:    acb, dma_elem_t  (defined in mpadd.h)           */
/*                                                                      */
/*                                                                      */
/*  RETURNS:                                                            */
/*                                                                      */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:                                         */
/*                                e_wakeup, i_disable,                  */
/*                               i_enable, bzero, mpalog, d_complete    */
/*                               startrecv                              */
/*                                                                      */
/*----------------------------------------------------------------------*/

int
free_dma_elem (
	struct acb *acb,            /* pointer to adapter control block */
	dma_elem_t *dmap)           /* pointer to dma element to free   */
{
	dma_elem_t *tmp_dmap;       /* temp dma element pointer.        */
	recv_elem_t *recvp;         /* recv element pointer.            */
	int         spl;            /* used to disable interrupts       */
	int 	    rc=0;


	MPATRACE3("FrDE",acb->dev,dmap);

	if(dmap == NULL)
	{
		rc=EIO;
                MPATRACE2("NoDm",acb->dev);
                return rc;
	}


	if(!(dmap->dm_state)) 
	{     /* don't try to free a free element */
            MPATRACE3("FrDx",acb->dev,dmap->dm_state);
	    return rc;
	}

	DISABLE_INTERRUPTS(spl);

	if(dmap->dm_state==DM_STARTED) 
	{
            MPATRACE4("FrD1",acb->dev,dmap->dm_state,dmap->dm_req_type);
	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	    ³ Cleanup after the DMA if dma is currently active ³
	    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	    rc = d_complete(acb->dma_channel, dmap->dm_flags,
		    dmap->dm_buffer, dmap->dm_length,
		    dmap->dm_xmem, NULL);

	    if( rc != DMA_SUCC )
	    {
		MPATRACE2("FrD2",rc);
                mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
	    }
	    dmap->dm_state = DM_FREE;

	    if(dmap->dm_req_type == DM_XMIT)
		++acb->stats.ds.xmit_dma_completes;   /* count xmit dma */
	    if(dmap->dm_req_type == DM_RECV)
		++acb->stats.ds.recv_dma_completes;   /* count recv dma */
	}

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Get rid of the mark that says theres an outstanding recv dma. ³
	³ being held on the hold recv dma q, if this is a recv dma elem.³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if(dmap->dm_req_type==DM_RECV) 
	{
	     recvp = (recv_elem_t *)dmap->p.recv_ptr;
	     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	     ³ Check to see if this MBUF should be freed ³
	     ³ (this was added for defect #091440).      ³
	     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	     if(recvp->rc_state & RC_MBUF)
             {
		   MPATRACE3("FrD3",acb->dev,dmap);
                   m_freem((struct mbuf *)recvp->rc_mbuf);
                   recvp->rc_state &= ~RC_MBUF;
             }
	     acb->hold_recv = NULL;

	     acb->flags &= ~RECV_DMA_ON_Q;
	}

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Take the dma element off the active list. ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if (acb->act_dma_head == dmap)
	{
		acb->act_dma_head = dmap->dm_next;
	}
	else 
	{
	   tmp_dmap = acb->act_dma_head;
	   while( (tmp_dmap->dm_next != dmap) && (tmp_dmap != NULL) )
		  tmp_dmap = tmp_dmap->dm_next;
	   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	   ³  If tmp_dmap is NULL the dma element has not yet been        ³
	   ³  put on the active list so just put it back on the free list ³
	   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	   if(tmp_dmap) tmp_dmap->dm_next = dmap->dm_next;
	}
	if (acb->act_dma_head == NULL) acb->act_dma_tail = NULL;

        MPATRACE3("FrD4",acb->dev,dmap);

	bzero(dmap,sizeof(dma_elem_t));       /* clean the element */

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Put it on the free list ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   	dmap->dm_next = acb->dma_free;
   	acb->dma_free = dmap;

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³  Was the receiver disabled waiting for dma elements? ³
	³  If so start the receiver now.                       ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if(acb->flags & NEED_DMA_ELEM) 
	{
           MPATRACE3("FrD5",acb->dev,dmap);
	   if(!(acb->flags & RECEIVER_ENABLED)) 
	   {
		startrecv(acb);
	       	if ( (acb->strt_blk.rcv_timeout != 0) &&
                       	(!(acb->flags & RCV_TIMER_ON)) )
               	{
                       	acb->flags |= RCV_TIMER_ON;
                       	timeout(recv_timer, (caddr_t)acb, (HZ/10) * (int)acb->strt_blk.rcv_timeout );
               	}
	   }
	   acb->flags &= ~NEED_DMA_ELEM;
	}

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ If anyone is sleeping waiting for dma elements wake them ³
	³ up.                                                      ³ 
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	ENABLE_INTERRUPTS(spl);	/* Defect 140957 */
	if (acb->dmabuf_event != EVENT_NULL)
		e_wakeup(&acb->dmabuf_event);

        MPATRACE3("FrDX",acb->dev,dmap);

	return rc;
} /* free_dma_elem() */


/*--------------------- F R E E _ X M I T _ E L E M --------------------*/
/*                                                                      */
/*  NAME: free_xmit_elem                                                */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Free up the given xmit element. Take it off the active list     */
/*      and put it back on the free list and notify anyone sleeping     */
/*      on a free xmit element event.                                   */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from process or interrupt environment.            */
/*                                                                      */
/*  DATA STRUCTURES:    acb, xmit_elem_t   (defined in mpadd.h)         */
/*                                                                      */
/*                                                                      */
/*  RETURNS:                                                            */
/*           VOID                                                       */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:                                         */
/*                                          e_wakeup, i_disable,        */
/*                               i_enable, bzero,                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
void
free_xmit_elem (
	struct acb *acb,       /* pointer to adapter control block      */
	xmit_elem_t *xmitp)    /* pointer to xmit element to free       */
{
	xmit_elem_t *tmp_xmitp;   /* temp xmit element pointer          */
	int         spl;          /* used to disable interrupts         */



        MPATRACE3("FrTE",acb->dev,xmitp);

	if( xmitp == NULL )
	{
                MPATRACE2("NoTx",acb->dev);
                return;
	}

	if(!(xmitp->xm_state&XM_ACTIVE)) 
	{  /* don't free a free one    */
            MPATRACE3("FrTx",acb->dev,xmitp->xm_state);
	    return;
	}

	DISABLE_INTERRUPTS(spl);

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Take the xmit element off the active list. ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if (acb->act_xmit_head==xmitp)
		acb->act_xmit_head =xmitp->xm_next;
	else 
	{
	   tmp_xmitp = acb->act_xmit_head;
	   while( (tmp_xmitp->xm_next != xmitp) && (tmp_xmitp != NULL) )
		  tmp_xmitp = tmp_xmitp->xm_next;
	   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	   ³  If tmp_xmitp is NULL the xmit element has not yet been      ³
	   ³  put on the active list so just put it back on the free list ³
	   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	   if(tmp_xmitp) tmp_xmitp->xm_next = xmitp->xm_next;
	}
	if (acb->act_xmit_head == NULL) acb->act_xmit_tail = NULL;

	bzero(xmitp,sizeof(xmit_elem_t));     /* clean the element  */

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Put it on the free list ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	xmitp->xm_next = acb->xmit_free;
	acb->xmit_free = xmitp;

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ If anyone is sleeping waiting for xmit elements wake them ³
	³ up.                                                       ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	ENABLE_INTERRUPTS(spl);	/* Defect 140957 */
	if (acb->xmitbuf_event != EVENT_NULL)
		e_wakeup(&acb->xmitbuf_event);

        MPATRACE3("FrTX",acb->dev,xmitp);


	return;
} /* free_xmit_element() */


/*--------------------- F R E E _ I R P T _ E L E M --------------------*/
/*                                                                      */
/*  NAME: free_irpt_elem                                                */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Free up the given irpt element. Take it off the active list     */
/*      and put it back on the free list and notify anyone sleeping     */
/*      on a free irpt element event.                                   */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from process or interrupt environment.            */
/*                                                                      */
/*  DATA STRUCTURES:    acb, xmit_elem_t   (defined in mpadd.h)         */
/*                                                                      */
/*                                                                      */
/*  RETURNS:                                                            */
/*           VOID                                                       */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:                                         */
/*                               startrecv  e_wakeup, i_disable,        */
/*                               i_enable, bzero,                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
void
free_irpt_elem (
	struct acb *acb,       /* pointer to adapter control block      */
	irpt_elem_t *irptp)    /* pointer to irpt element to free       */
{
	irpt_elem_t *tmp_irptp;   /* temp irpt element pointer          */
	int         spl;          /* used to disable interrupts         */



        MPATRACE3("FrIE",acb->dev,irptp);

	if(irptp == NULL)
	{ 
                MPATRACE2("NoIr",acb->dev);
                return;
	}

	if(!(irptp->ip_state&IP_ACTIVE)) 
	{     /* don't free a free one */
            MPATRACE3("FrIx",acb->dev,irptp->ip_state);
	    return;
	}

	DISABLE_INTERRUPTS(spl);

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Take the irpt element off the active list. ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if (acb->act_irpt_head==irptp)
		acb->act_irpt_head =irptp->ip_next;
	else 
	{
	   tmp_irptp = acb->act_irpt_head;
	   while( (tmp_irptp->ip_next != irptp) && (tmp_irptp != NULL) )
		  tmp_irptp = tmp_irptp->ip_next;
	   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	   ³  If tmp_irptp is NULL the irpt element has not yet been      ³
	   ³  put on the active list so just put it back on the free list ³
	   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	   if(tmp_irptp) tmp_irptp->ip_next = irptp->ip_next;
	}
	if (acb->act_irpt_head == NULL) acb->act_irpt_tail = NULL;

	bzero(irptp,sizeof(irpt_elem_t));    /* clean the elemnet */

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Put it on the free list ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	irptp->ip_next = acb->irpt_free;
	acb->irpt_free = irptp;

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³  Was the receiver disabled waiting for irpt elements? ³
	³  If so start the receiver now.                        ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if(acb->flags & NEED_IRPT_ELEM) 
	{
	   if(!(acb->flags & RECEIVER_ENABLED)) 
	   {
		startrecv(acb);
	       	if ( (acb->strt_blk.rcv_timeout != 0) &&
                       	(!(acb->flags & RCV_TIMER_ON)) )
               	{
                       	acb->flags |= RCV_TIMER_ON;
                       	timeout(recv_timer, (caddr_t)acb, (HZ/10) * (int)acb->strt_blk.rcv_timeout );
               	}
	   }
	   acb->flags &= ~NEED_IRPT_ELEM;
	}

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ If anyone is sleeping waiting for irpt elements wake them ³
	³ up.                                                       ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	ENABLE_INTERRUPTS(spl);	/* Defect 140957 */
	if (acb->irptbuf_event != EVENT_NULL)
		e_wakeup(&acb->irptbuf_event);

        MPATRACE3("FrIX",acb->dev,irptp);

	return;
} /* free_irpt_element() */

/*---------------------   D M A _ R E Q U E S T   ----------------------*/
/*                                                                      */
/*  NAME: dma_request                                                   */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      This function takes the dma element pointer it is passed        */
/*      and puts it on the dma q. If the request is for XMIT it always  */
/*      goes on the q. If it is for RECV, it goes on only if the q      */
/*      is empty. It then starts the next dma on the q if there is      */
/*      no dma active.                                                  */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from process or interrupt environment.            */
/*                                                                      */
/*  DATA STRUCTURES:    acb, dma_elem_t    (defined in mpadd.h )        */
/*                                                                      */
/*                                                                      */
/*  RETURNS:                                                            */
/*       0 ------ queued successfully                                   */
/*       EINVAL-- if dma element has bad type                           */
/*       EIO----- PIO error occured in que_command                      */
/*       EDEADLK- Card hung occured in que_command                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:                                         */
/*                               i_disable,d_slave, mpalog              */
/*                               i_enable,que_command                   */
/*                                                                      */
/*                                                                      */
/*----------------------------------------------------------------------*/
int
dma_request (
	struct acb *acb,
	dma_elem_t  *new_dmap)
{

	dma_elem_t  *dmap;
	xmit_elem_t *xmitp;
	int         rc=0;
	int         spl;
	int         que_it=0;


        MPATRACE3("DrqE",acb->dev, new_dmap);
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ If new_dmap is NULL this is an error so shutdown             ³
	³ If new_dmap is not null                                      ³
	³ then add it to the list and start dma if one is not already  ³
	³ started.                                                     ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if( new_dmap == NULL)
	{
                MPATRACE2("Drqx",acb->dev);
         	return(EIO);
	}

	DISABLE_INTERRUPTS(spl);

	if ( new_dmap->dm_req_type == DM_XMIT ) 
	{
	     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	     ³ Set a flag to Put it on the dma q. Always q XMIT requests. ³
	     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	     que_it = 1;
	}
	else 
	{
	     /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	     ³ Put RECV on the list, only when the list is empty ³
	     ³ and set a flag to tell the other code that a recv ³
	     ³ is on the q and a recv dma request is pending.    ³
	     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	     if ( acb->act_dma_head == NULL )  
	     {
		 que_it = 1;
		 acb->flags |= RECV_DMA_ON_Q;
                 MPATRACE3("Drq1",acb->dev,acb->flags);
	     }
	}

	if(que_it)  
	{
	     if(acb->act_dma_head==NULL) 
	     {  /* its first one */
		 acb->act_dma_head=new_dmap;
		 acb->act_dma_tail=new_dmap;
	     }
	     else 
	     {   /* Theres already at least one on the chain */
		      /* so put this one on the end               */
		 acb->act_dma_tail->dm_next=new_dmap;
		 acb->act_dma_tail=new_dmap;
	     }
	}

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Now check the head of the q to see if theres one to start ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if ((dmap = acb->act_dma_head) == NULL) 
	{
		ENABLE_INTERRUPTS(spl);
		return EIO;
	}

	switch (dmap->dm_state) 
	{


	case DM_STARTED:
		break;

	case DM_READY: 
	{                /* start a DMA */
		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ Set up the DMA channel for the transfer      ³
		³ d_slave is a void function so we are working ³
		³ on faith here.                               ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		d_slave(acb->dma_channel, dmap->dm_flags,
			dmap->dm_buffer, dmap->dm_length, 
			dmap->dm_xmem);
		dmap->dm_state = DM_STARTED;

		switch (dmap->dm_req_type) 
		{
		case DM_XMIT:
                 	MPATRACE4("Drq2",acb->dev,acb->state.port_b_8273,acb->state.oper_mode_8273);

                        if( (xmitp = (xmit_elem_t *)dmap->p.xmit_ptr) == NULL )
                        {
                                MPATRACE3("Drq3",acb->dev,dmap);
				rc = EIO;
                                break;
                        }
#ifndef NO_RTS_CTRL
			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			³ drive RTS if not already ON ³
			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			if( !(acb->state.port_b_8273 & SET_RTS) )
			{		
				if( (rc = PIO_GETC( acb, PORT_A_8255)) == -1) 
				{
                			mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
					ENABLE_INTERRUPTS(spl);
                			return EIO;
        			}
				/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
				³ If Clear to Send is already on before  ³
				³ driving RTS, an error should be logged ³
				³ and the caller should be notified.     ³
				ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
				if( !(rc & CTS_OFF) &&
					!(acb->state.oper_mode_8273 & SET_FLAG_STREAM) )
				{
                			mpalog(acb,ERRID_MPA_CTSON , __LINE__,__FILE__,0,0);
					ENABLE_INTERRUPTS(spl);
                			return EIO;
				}

				bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
                        	acb->cmd_parms.cmd=SET_8273_PORT_B_CMD;
                        	acb->cmd_parms.parm[0]=SET_8273_PORT_B_RTS;
                        	acb->state.port_b_8273 |=SET_RTS;
                        	acb->cmd_parms.parm_count=1;
				if( (rc=que_command(acb)) ) break;
			}
#endif

			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			³ send MPA xmit command ³
			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/	
			bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
			acb->cmd_parms.cmd=XMIT_CMD;
			acb->cmd_parms.parm[0]=dmap->dm_length;
			acb->cmd_parms.parm[1]=(dmap->dm_length>>8);
			acb->cmd_parms.parm_count=2;
			if( (rc=que_command(acb)) ) break;
			acb->flags |= WAIT_FOR_EARLY_XMIT;

#ifndef NO_STRM_FLG_CTRL
			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			³ turn on flag stream mode if not on already ³
			³ (this was modified for defect #091439)     ³
			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			if( !(acb->state.oper_mode_8273 & SET_FLAG_STREAM) )
			{		
				bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
                        	acb->cmd_parms.cmd=SET_OPER_MODE_CMD;
                        	acb->cmd_parms.parm[0]=SET_FLAG_STREAM;     
                        	acb->state.oper_mode_8273 |=SET_FLAG_STREAM;
                        	acb->cmd_parms.parm_count=1;
				rc=que_command(acb);
			}
#endif
			break;
		case DM_RECV:
                 	MPATRACE4("Drq4",acb->dev,acb->state.port_b_8273,acb->state.oper_mode_8273);
			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			³  Now kick off a recv command.. ³
			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			if(!(acb->flags & RECEIVER_ENABLED)) 
			{
			   acb->total_bytes = 0;
			   bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
			   if(acb->station_type & GENERAL) 
			   {
			      acb->cmd_parms.cmd=GEN_RECEIVE_CMD;
			      acb->cmd_parms.parm[0]=MAX_RECV_SIZE;
			      acb->cmd_parms.parm[1]=(MAX_RECV_SIZE>>8);
			      acb->cmd_parms.parm_count=2;
			   }
			   else 
			   {
			      acb->cmd_parms.cmd=SEL_RECEIVE_CMD;
			      acb->cmd_parms.parm[0]=MAX_RECV_SIZE;
			      acb->cmd_parms.parm[1]=(MAX_RECV_SIZE>>8);
			      acb->cmd_parms.parm[2]=acb->station_addr;
			      acb->cmd_parms.parm[3]=(uchar)BROADCAST_ADDR;
			      acb->cmd_parms.parm_count=4;
			   }
			   if( (rc=que_command(acb)) ) break;

			   acb->flags |= RECEIVER_ENABLED;
			}
			break;
		default:
			mpalog(acb,ERRID_MPA_ADPERR,__LINE__,__FILE__,0,0);
			rc = EINVAL;
			break;

		} /* end switch base on dm_req_type */
	} break;

	default:
		mpalog(acb,ERRID_MPA_ADPERR,__LINE__,__FILE__,0,0);
		rc = EINVAL;
		break;

	} /* END SWITCH  based on dm_state */

	ENABLE_INTERRUPTS(spl);
	if (rc) free_dma_elem(acb, dmap);
        MPATRACE4("DrqX",acb->dev, new_dmap,rc);
	return(rc);

} /* dma_request() */
/*======================================================================*/
/*                        PIO ACCESS PRIMITIVES                         */
/*======================================================================*/

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³  All character, word, and long programmed-I/O (PIO) operations   ³
³  to/from the adapter are handled here.  These primitives are not ³
³  typically called directly -- the PIO accessor macros in mpadd.h ³
³  resolve at compile time to calls to these routines.             ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

/*---------------------------  P I O G E T  ----------------------------*/
/*                                                                      */
/*  NAME: PioGet                                                        */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Performs a PIO read of a byte, short, reversed short, long,     */
/*      or reversed long depending on the type of access specified.     */
/*      If an IO exception occurs during access, it is retried several  */
/*      times -- a -1 is returned if the retry limit is exceeded.       */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from interrupt level or on a process thread.      */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS:                                                            */
/*      -1      If pio error, value of the read otherwise.              */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: setjumpx, longjumpx, clrjumpx,          */
/*                              BUS_GETC, BUSIO_ATT, BUSIO_DET          */
/*                                                                      */
/*----------------------------------------------------------------------*/

PioGet (struct acb      *acb,
	caddr_t         addr)                  /* address to read from */
{
	int             rc;
	label_t         jump_buf;               /* jump buffer */
	volatile int    retry = PIO_RETRY_COUNT;
	ulong           iob;

	iob = MPA_BUSIO_ATT;
	while (TRUE) 
	{                          /* retry loop */
	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	    ³  Set up for context save by doing setjumpx.  If it returns ³
	    ³  zero, all is well; otherwise, an exception occurred.      ³
            ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	    
	    if ((rc = setjmpx(&jump_buf)) == 0) 
	    {
		if (retry--) 
		{                  /* retry? */
		    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		    ³  If retries have not been used up, do the read. ³
		    ³  Select the accessor according to the access    ³ 
		    ³  type.                                          ³
		    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		    rc = (int)BUS_GETC( iob + addr );
		    break;                      /* exit retry loop */
		} 
		else 
		{
		    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		    ³  Out of retries, so return an error. ³
		    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		    rc = -1;
		    break;
		}
	    } 
	    else 
	    {
		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³  An exception has occurred or reoccurred -- if it is ³
		³  a PIO error, simply retry; else, it is an exception ³
		³  not handled here so longjmpx to the next handler on ³
		³  the stack.                                          ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		if (rc != EXCEPT_IO) 
		{
		    longjmpx(rc);
		}
	    }
	}
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³  Out of retry loop -- remove jump buffer from exception ³
	³  stack and return.                                      ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	clrjmpx(&jump_buf);
	BUSIO_DET(iob);
	return (rc);
}

/*---------------------------  P I O P U T  ----------------------------*/
/*                                                                      */
/*  NAME: PioPut                                                        */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Performs a PIO write of a byte, short, reversed short, long,    */
/*      or reversed long depending on the type of access specified.     */
/*      If an IO exception occurs during access, it is retried several  */
/*      times -- a -1 is returned if the retry limit is exceeded.       */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from interrupt level or on a process thread.      */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS:                                                            */
/*      0       Pio write was successful.                               */
/*      -1      Pio error or illegal access type.                       */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: setjumpx, longjumpx, clrjumpx,          */
/*                              BUS_PUTC, BUSIO_ATT, BUSIO_DET          */
/*                                                                      */
/*----------------------------------------------------------------------*/

PioPut (struct acb      *acb,
	caddr_t         addr,                   /* address to write to */
	int             val)                    /* value to write */
{
	int             rc = 0;
	label_t         jump_buf;               /* jump buffer */
	volatile int    retry = PIO_RETRY_COUNT;
	ulong           iob;

	iob = MPA_BUSIO_ATT;
	while (TRUE) 
	{                          /* retry loop */
	    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	    ³  Set up for context save by doing setjumpx.  If it returns ³
	    ³  zero, all is well; otherwise, an exception occurred.      ³
	    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	    if ((rc = setjmpx(&jump_buf)) == 0) 
	    {
		if (retry--) 
		{                  /* retry? */
		    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		    ³  If retries have not been used up, do the write.  ³
		    ³  Select the accessor according to the access      ³
		    ³  type. If we make it out of the switch, we didn't ³
		    ³  get a PIO error.                                 ³
		    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		    BUS_PUTC( (iob + addr), (uchar) val );

		    break;                      /* exit retry loop */
		} 
   		else 
		{
		    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		    ³  Out of retries, so return an error. ³
		    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		    rc = -1;
		    break;
		}
	    } 
	    else 
	    {
		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³  An exception has occurred or reoccurred -- if it is ³
		³  a PIO error, simply retry; else, it is an exception ³
		³  not handled here so longjmpx to the next handler on ³
		³  the stack.                                          ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		if (rc != EXCEPT_IO) 
		{
		    longjmpx(rc);
		}
	    }
	}
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³  Out of retry loop -- remove jump buffer from exception ³
	³  stack and return.                                      ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	clrjmpx(&jump_buf);
	BUSIO_DET(iob);
	return (rc);
}    /* PioPut() */

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³  unchain_mbuf() - unchain the mbuf chain that is passed from the caller   ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
struct mbuf *
unchain_mbuf(struct acb *acb, 
	struct mbuf *m, 
	int length)
{
        struct mbuf *n, *m0;
        u_int num_mbufs = 1;
        u_int max_mbufs = 1;
        caddr_t p;
        struct mbuf *anchor;
	

        MPATRACE4("MbfE",acb->dev,m,length);

        n = m;
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Go down the chain and determine ³
	³ the number of mbufs to unchain  ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        while (n = n->m_next)
                num_mbufs++;

        num_mbufs -= max_mbufs; /* num_mbufs is now the number to trim */

        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ If the length is greater than MLEN  ³
	³ (256 - sizeof(struct m_hdr)), get a ³
        ³ cluster; otherwise, get a small mbuf³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/ 
	if ( length > MLEN )
	{
        	n = m_getclust(M_DONTWAIT, MT_DATA);
	}
	else
	{
		n = m_get(M_DONTWAIT, MT_DATA);
	}

        if (!n) 
	{
                return(NULL);
        }

        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Since we're using one to collect ³
        ³ have to trim an extra            ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	num_mbufs++;    
        anchor = n;     /* anchor will be the top of the chain  */
        m0 = anchor;    /* m0 will be the mbuf for gather       */

        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Follow the mbuf chain and copy data into cluster ³   
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        p = mtod(n, caddr_t);
        n->m_len = 0;
        while (num_mbufs && m) 
	{
                bcopy(mtod(m, caddr_t), p, m->m_len);
                n->m_len += m->m_len;
                p += m->m_len;
                m = m->m_next; 
                num_mbufs--;
        }
        n->m_next = m;
        if (num_mbufs)   /* couldn't collapse into max_mbufs */ 
	{        
        	MPATRACE4("Mbfx",acb->dev,anchor,num_mbufs);
                m_freem(anchor);
                return(NULL);
        } 
	else
        	MPATRACE4("MbfX",acb->dev,anchor,m);
                return(anchor);
}

int startrecv (struct acb *acb)
{

    int                         lvl,spl,rc=0;
    dma_elem_t                  *dmap;
    recv_elem_t                 *recvp;
    struct mbuf                 *mbufp=NULL;


    DISABLE_INTERRUPTS(spl);

    MPATRACE3("StrE",acb->dev,rc);

    if(acb->flags & MPADEAD) 
    {
	ENABLE_INTERRUPTS(spl);
	return EIO;
    }

    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ If there is a recv dma held in acb->hold_recv then      ³
    ³ I already have memory and other resources allocated for ³
    ³ the recv so just put the recv dma element back on the q ³
    ³ only if the active dma q is empty (no pending xmits).   ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
    if(acb->hold_recv == NULL) 
    {
    	 MPATRACE2("Str1",acb->dev);
	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³ Allocate resources for receive...         ³
	 ³ Take a recv element from the "free" list. ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 recvp = acb->recv_free;
	 if(recvp == NULL) 
	 {
	      mpalog(acb,ERRID_MPA_BFR, __LINE__,__FILE__,0,0);
	      ENABLE_INTERRUPTS(spl);
	      return ENOMEM;
	 }
	 acb->recv_free=recvp->rc_next;
	 bzero(recvp,sizeof(recv_elem_t));

	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³ Put this element on the active recv q for this adapter ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 if(acb->act_recv_head==NULL) 
	 {  /* its first one */
	     acb->act_recv_head=recvp;
	     acb->act_recv_tail=recvp;
	 }
	 else 
	 {        /* its going on the end of a chain */
	     acb->act_recv_tail->rc_next=recvp;
	     acb->act_recv_tail=recvp;
	 }

	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³ Initialize the receive element. ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 recvp->rc_mbuf = NULL;   
	 recvp->rc_state |= RC_ACTIVE;   

	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³ If we can't get an mbuf, bail out. ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 if ((mbufp = m_getclust(M_DONTWAIT, MT_DATA)) == NULL) 
	 {
	     mpalog(acb,ERRID_MPA_BFR, __LINE__,__FILE__,0,0);
	     free_recv_elem(acb,recvp);
	     ENABLE_INTERRUPTS(spl);
	     return ENOMEM;
	 }

	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³  Default receive length is 4k (CLBYTES). ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 mbufp->m_len = (CLBYTES - DDS.rdto);
	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³  Increment m_data by RDTO ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 mbufp->m_data += DDS.rdto;

	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³ Place this MBUF into the receive element. ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 recvp->rc_mbuf = mbufp;
	 recvp->rc_state |= RC_MBUF;

	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³ Take a dma element from the "free" list. ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 dmap = acb->dma_free;
	 if(dmap == NULL) 
	 {
		mpalog(acb,ERRID_MPA_BFR, __LINE__,__FILE__,0,0);
		m_freem((struct mbuf *)recvp->rc_mbuf);
		recvp->rc_state &= ~RC_MBUF;
		free_recv_elem(acb,recvp);
		ENABLE_INTERRUPTS(spl);
		return ENOMEM;
	 }
	 acb->dma_free=dmap->dm_next;
	 bzero(dmap,sizeof(dma_elem_t));

	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³ Set up the DMA element. ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 dmap->p.recv_ptr = recvp;
	 dmap->dm_req_type = DM_RECV;
	 dmap->dm_buffer = MTOD(mbufp, caddr_t);
	 dmap->dm_xmem = M_XMEMD(mbufp);
	 dmap->dm_length = mbufp->m_len;
	 dmap->dm_flags = DMA_READ | DMA_NOHIDE;
	 dmap->dm_state = DM_READY;

	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³  Set up the hold_recv pointer so I know I have     ³
	 ³  all the resources for a recv. This pointer is set ³
	 ³  back to NULL after a valid receive completes.     ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 acb->hold_recv = dmap;
    	 MPATRACE4("Str2",acb->dev,mbufp,acb->hold_recv);

    }             /* end if recv_dma element does not exits */
    else 
    {
	 /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	 ³ else there is already a recv dma elem so just go to ³
	 ³ the hold_recv variable and get the dma element ptr. ³
	 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	 dmap= acb->hold_recv;
    	 MPATRACE4("Str3",acb->dev,dmap,(struct mbuf *)(dmap->p.recv_ptr->rc_mbuf));
    }

    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³  Kick off the dma...if I just put a recv on there it will  ³
    ³  start a recv dma, else it will start the next xmit dma on ³
    ³  the q.                                                    ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
    rc=dma_request(acb,dmap);

    MPATRACE3("StrX",acb->dev,rc);
    ENABLE_INTERRUPTS(spl);
    return(rc);
}   /* startrecv() */

int stoprecv (struct acb *acb)
{

    int                         lvl,spl,rc=0;
    dma_elem_t                  *dmap;
    recv_elem_t                 *recvp;
    struct mbuf                 *mbufp;



    DISABLE_INTERRUPTS(spl);

    if(acb->flags & MPADEAD) 
    {
	ENABLE_INTERRUPTS(spl);
	return EIO;
    }
    
    MPATRACE3("SprE",acb->dev,rc);
    /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Issue a recv disable command to the adapter, to  ³
    ³ prevent any receive irpts during xmit.           ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
    if( acb->flags & RECEIVER_ENABLED )
    {
	bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
	acb->cmd_parms.cmd=DISABLE_RECV_CMD;
	acb->cmd_parms.parm_count=0;
	if( !(rc = que_command(acb)) ) 
	{
	      acb->flags &= ~RECEIVER_ENABLED;
              MPATRACE3("Spr1",acb->dev,acb->flags);
	}
    }

    if( acb->flags & RECV_DMA_ON_Q ) 
    {

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ The dma element on the head of the q will have the recv ³
	³ dma I need to put on hold, So get the dmap and check to ³
	³ make sure it is the current hold_recv element.          ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if((dmap= acb->act_dma_head) == NULL ) 
    	{
    	   MPATRACE3("Spr2",acb->dev,dmap);
	   ENABLE_INTERRUPTS(spl);
	   return ENXIO;
	}
	if( acb->hold_recv != dmap) 
	{
    	   MPATRACE4("Spr3",acb->dev,dmap,acb->hold_recv);
	   ENABLE_INTERRUPTS(spl);
	   return ENXIO;
	}


	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Take this read dma elem off the active q...but do not ³
	³ put it on the free q, it goes to hold_recv.           ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	acb->act_dma_head =dmap->dm_next; /* Null if last on q */
	acb->hold_recv = dmap;
	dmap->dm_next = NULL;

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Abort the pending recv dma ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	d_complete(acb->dma_channel, dmap->dm_flags,
		   dmap->dm_buffer, dmap->dm_length,
		   dmap->dm_xmem, NULL);

	dmap->dm_state = DM_READY;
	acb->flags &= ~RECV_DMA_ON_Q;
        MPATRACE3("Spr4",acb->dev,acb->flags);
    }

    MPATRACE3("SprX",acb->dev,rc);
    ENABLE_INTERRUPTS(spl);
    return(rc);
}   /* stoprecv() */

void free_active_q (struct acb *acb)
{
  irpt_elem_t   *irptp;
  irpt_elem_t   *next_ip;
  recv_elem_t   *recvp;
  recv_elem_t   *next_rp;
  xmit_elem_t   *xmitp;
  xmit_elem_t   *next_xp;
  dma_elem_t    *dmap;
  dma_elem_t    *next_dp;
  int           spl;



  DISABLE_INTERRUPTS(spl);

  MPATRACE2("FrAE",acb->dev);

  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ remove all dma elements from the active list ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
  if ( (dmap=acb->act_dma_head) != NULL) 
  {
     while (dmap) 
     {
        next_dp = dmap->dm_next;
        free_dma_elem(acb,dmap);
        dmap = next_dp;
     }
     acb->act_dma_head = acb->act_dma_tail = NULL;
  }
  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ remove all irpt elements from the active list ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
  if ( (irptp=acb->act_irpt_head) != NULL) 
  {
     while (irptp) 
     {
        next_ip = irptp->ip_next;
        free_irpt_elem(acb,irptp);
        irptp = next_ip;
     }
     acb->act_irpt_head = acb->act_irpt_tail = NULL;
  }

  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ remove all recv elements from the active list ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
  if ( (recvp=acb->act_recv_head) != NULL) 
  {
     while (recvp) 
     {
        next_rp = recvp->rc_next;
        free_recv_elem(acb,recvp);
        recvp = next_rp;
     }
     acb->act_recv_head = acb->act_recv_tail = NULL;
  }

  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ remove all xmit elements from the active list ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
  if ( (xmitp=acb->act_xmit_head) != NULL) 
  {
     while (xmitp) 
     {
        next_xp = xmitp->xm_next;

	/* Defect 178174 */
        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³ If the kernel user wants us to free the mbufs,³
        ³ do so now.                                    ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        MPATRACE4("FrA3",acb->dev,xmitp->xm_mbuf,xmitp->xm_ack);
        if (xmitp->xm_mbuf
                && !(xmitp->xm_ack & CIO_NOFREE_MBUF))
        {
                MPATRACE3("FrA4",acb->dev,xmitp->xm_mbuf);
                (void) m_freem(xmitp->xm_mbuf);
        }
	/* End Defect 178174 */

        free_xmit_elem(acb,xmitp);
        xmitp = next_xp;
     }
     acb->act_xmit_head = acb->act_xmit_tail = NULL;
  }

  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ Clear the acb flags associated with the freed elements. ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
  acb->flags &= ~NEED_IRPT_ELEM;
  acb->flags &= ~NEED_RECV_ELEM;
  acb->flags &= ~NEED_DMA_ELEM;
  acb->flags &= ~RECV_DMA_ON_Q;

  MPATRACE4("FrAX",irptp,recvp,dmap);
  ENABLE_INTERRUPTS(spl);
  return;
}        /* free_active_q() */

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³ recv_timer() - this routine handles receive timeouts by returning ³
³		 RCV_TIMEOUT status to the calling process          ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
void recv_timer(struct acb *acb)
{

  	MPATRACE2("RtoE",acb->dev);
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ if we ever get a receive    ³
	³ timeout, indicate timer off ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	acb->flags &= ~RCV_TIMER_ON;
	
     	if ( !(acb->flags & STARTED_CIO) )     /* and not started... */
     	{
		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ Restart the receiver. ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		startrecv(acb);
		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³  recycle timer³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/	
        	if ( (acb->strt_blk.rcv_timeout != 0) &&
                	(!(acb->flags & RCV_TIMER_ON)) )
        	{
                	acb->flags |= RCV_TIMER_ON;
			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			³ Increase timeout by 10 to reduce receive ³
			³ timer interrupts.  Once we successfully  ³
			³ complete a receive after the line is up, ³
			³ the proper timeout values will apply.    ³
			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                	timeout(recv_timer, (caddr_t)acb, HZ * (int)acb->strt_blk.rcv_timeout );
        	}
	}
	else
	{
  		MPATRACE3("Rto1",acb->dev,acb->strt_blk.data_flags);
		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ Inform user that receive timer expired ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        	async_status(acb, CIO_ASYNC_STATUS, MP_RCV_TIMEOUT, 0, 0, 0); 

		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ Should the receive timer be restarted ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		if ( acb->strt_blk.data_flags & DATA_FLG_RST_TMR ) 
		{
			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			³ Restart the receiver. ³
			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			startrecv(acb);
	        	if ( (acb->strt_blk.rcv_timeout != 0) &&
                        	(!(acb->flags & RCV_TIMER_ON)) )
                	{
                        	acb->flags |= RCV_TIMER_ON;
                        	timeout(recv_timer, (caddr_t)acb, (HZ/10) * (int)acb->strt_blk.rcv_timeout );
                	}
		}

			
	}
							       
  	MPATRACE2("RtoX",acb->dev);
        return;

} /* recv_timer() */

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³ ring_timer() - this routine polls the status of RI in 50ms intervals ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
void ring_timer(struct acb *acb)
{
	int status;


	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ read the 8255 PORT A register for status of RI ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if( (status = PIO_GETC( acb, PORT_A_8255 )) == -1) 
	{
  		MPATRACE2("RIe1",acb->dev);
               	mpalog(acb,ERRID_MPA_ADPERR , __LINE__,__FILE__,0,0);
               	return;
       	}
		
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ check status of RI ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if( !(status & RING_OFF) )
	{
		RI_currnt=1; /* RI is ON */
		if (RI_currnt!=RI_prev) /* First occurrence of RI */
		{
			RI_prev=RI_currnt; /* toggle counter */
                        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                        ³ timeout in 20ms ³
                        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                        timeout( ring_timer, (caddr_t)acb, HZ/50 );
		}
		else    /* RI is still on after 20ms */
		{
			RI_currnt=RI_prev=0; /* reset RI states */
			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
			³ drive DTR if not already on ³
			³ this will occur for CDSTL   ³
			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			if( !(acb->state.port_b_8273 & SET_DTR) )
			{
        			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        			³  Now set data terminal ready ³
        			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                		bzero(&acb->cmd_parms,sizeof(cmd_phase_t));
                		acb->cmd_parms.cmd=SET_8273_PORT_B_CMD;
                		acb->cmd_parms.parm[0]=SET_DTR;
                		acb->state.port_b_8273 |=SET_DTR;
                		acb->cmd_parms.parm_count=1;

                		if( que_command(acb) )
				{ 
  					MPATRACE2("RIe2",acb->dev);
					return;
				}
			}
		}
		
	}
	else
	{
		RI_prev=RI_currnt=0; /* reset current and previous */

                /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                ³ continuing polling every 50ms ³
                ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                timeout( ring_timer, (caddr_t)acb, HZ/20 );
	}
	return;
} /* ring_timer */

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³ ctrl_modem() - provides appropriate handling of switched communication lines³
³		as specified by EIA Standard EIA-232-D and Recommendation V.24³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
int ctrl_modem(struct acb *acb)
{
	int rc = 0;


        MPATRACE3("MdmE",acb->dev,rc);

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ get the state of DSR from 8273 ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        bzero(&acb->cmd_parms,sizeof(cmd_phase_t)); /* clear input struct */
        acb->cmd_parms.cmd = READ_8273_PORT_A_CMD;
        acb->cmd_parms.parm_count = 0;
        acb->cmd_parms.flag = RETURN_RESULT;

        if( (rc = que_command(acb)) )
	{
                MPATRACE3("Mde1",acb->dev,rc);
 		return rc;
	}

        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ check to see if DSR is active via 8273 port A reg ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        if( acb->cmd_parms.result & PORT_A_8273_PA2 )
	{
		if( acb->strt_blk.modem_flags & MF_LEASED ) 
		{
			/*ÄÄÄÄÄÄÄÄÄÄ¿
			³ drive DTR ³
			ÀÄÄÄÄÄÄÄÄÄÄ*/
			bzero(&acb->cmd_parms, sizeof(cmd_phase_t));
			acb->cmd_parms.cmd = SET_8273_PORT_B_CMD;
			acb->cmd_parms.parm[0] = SET_DTR;
			acb->state.port_b_8273 |= acb->cmd_parms.parm[0];
			acb->cmd_parms.parm_count = 1;

			if ( (rc=que_command(acb)) ) 
			{
                		MPATRACE3("Mde2",acb->dev,rc);
				return rc;
			}
			
		     	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
     			³ Inform caller that line is active. ³
     			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
			async_status(acb, CIO_START_DONE, CIO_OK, 0,0,0);
			acb->flags |= STARTED_CIO;
			acb->flags &= ~WAIT_FOR_DSR;
                	MPATRACE3("Mdmx",acb->dev,rc);
			return rc;
		}
		else
		{
        	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ If DSR is active when bringing up switched line, ³
		³ inform caller (DSR should not be on during start)³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                	MPATRACE3("Mde3",acb->dev,MP_DSR_ALRDY_ON);
			async_status(acb, CIO_START_DONE, MP_DSR_ALRDY_ON, 0,0,0);
            		mpalog(acb,ERRID_MPA_DSRON , __LINE__,__FILE__,0,0);
			return EIO;
		}
	}

	if ( !(acb->strt_blk.modem_flags & MF_CDSTL_ON) ) /* DTR Type */
	{
               	MPATRACE3("Mdm1",acb->dev,acb->strt_blk.modem_flags);
		/*ÄÄÄÄÄÄÄÄÄÄ¿
		³ drive DTR ³
		ÀÄÄÄÄÄÄÄÄÄÄ*/
		bzero(&acb->cmd_parms, sizeof(cmd_phase_t));
		acb->cmd_parms.cmd = SET_8273_PORT_B_CMD;
		acb->cmd_parms.parm[0] = SET_DTR;
		acb->state.port_b_8273 |= acb->cmd_parms.parm[0];
		acb->cmd_parms.parm_count = 1;

		if ( (rc=que_command(acb)) ) 
		{
                	MPATRACE3("Mde4",acb->dev,rc);
			return rc;
		}

		if ( (acb->strt_blk.modem_flags & MF_CALL) &&
			(!(acb->strt_blk.modem_flags & MF_LEASED)) )
		{
               		MPATRACE3("Mdm2",acb->dev,acb->strt_blk.modem_flags);
			async_status(acb, MP_RDY_FOR_MAN_DIAL, CIO_OK, 0, 0, 0);
		}
	}
	else /* CDSTL Type */
	{ 
               	MPATRACE3("Mdm3",acb->dev,acb->strt_blk.modem_flags);
		if ( !(acb->strt_blk.modem_flags & MF_LEASED) )
		{
			if (acb->strt_blk.modem_flags & MF_CALL)
			{
               			MPATRACE3("Mdm4",acb->dev,acb->strt_blk.modem_flags);
				async_status(acb, MP_RDY_FOR_MAN_DIAL, CIO_OK, 0, 0, 0);
			}
			else 
			{
				if ( !(acb->strt_blk.modem_flags & MF_MANUAL) )
				{
                			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                			³ Start the RI polling timer ³
                			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                			timeout( ring_timer, (caddr_t)acb, HZ/20 );
				}
	
			}
		}
	}
	acb->flags |= WAIT_FOR_DSR;
        MPATRACE4("MdmX",acb->dev,acb->flags,rc);
	return rc;
} /* ctrl_modem */
