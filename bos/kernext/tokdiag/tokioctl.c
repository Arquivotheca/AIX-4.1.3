static char sccsid[] = "@(#)37	1.26  src/bos/kernext/tokdiag/tokioctl.c, diagddtok, bos411, 9428A410j 10/26/93 14:05:16";
/*
 * COMPONENT_NAME: (SYSXTOK) - Token-Ring device handler
 *
 * FUNCTIONS:  tokioctl(), cio_start(), cio_halt(), cio_query(),
 *             cio_get_stat(), tokposacc(), tokinfo(), tokinitop(),
 *             tokopenop(), tokgrpaddr(), tokfuncaddr(),
 *             functional_address(), ring_info(), cio_get_fastwrt()
 *
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/cblock.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/dump.h>
#include <sys/errno.h>
#include <sys/err_rec.h>
#include "tok_comio_errids.h"
#include <sys/file.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/ioctl.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/timer.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/watchdog.h>
#include <sys/xmem.h>
#include <sys/comio.h>
#include <sys/trchkid.h>
#include <sys/tokuser.h>
#include <sys/adspace.h>


#include "tokdshi.h"
#include "tokdds.h"
#include "tokdslo.h"
#include "tokproto.h"
#include "tokprim.h"

#ifndef CIO_NETID_INV
# define CIO_NETID_INV  8
#endif

/*
*  get access to the trace table
*/
extern tracetable_t    tracetable;

/*
*  get access to the component dump table
*/
extern cdt_t   ciocdt;

/*
*  get access to the device driver control block
*/
extern dd_ctrl_t   dd_ctrl;



/*****************************************************************************/
/*
 * NAME:     tokioctl
 *
 * FUNCTION: ioctl entry point from kernel
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 *
 *     Set Adapter Initializations Options (TOK_SET_ADAP_IPARMS)
 *     Set Adapter Open Options (TOK_SET_OPEN_PARMS)
 *     Access POS Registers (TOK_ACCESS_POS)
 *     Token Driver Information (TOKINFO)
 *     Set Group Address (TOK_GRP_ADDR)
 *     Set Functional Address (TOK_FUNC_ADDR)
 *
 * RETURNS:  0 or errno
 *
 */
/*****************************************************************************/
int tokioctl (
   dev_t   devno,    /* major and minor number */
   int     cmd,      /* ioctl operation desired */
   int     arg,      /* argument for this command (usually a structure ptr) */
   ulong   devflag,  /* flags including DKERNEL */
   chan_t  chan,     /* channel number */
   int     ext)      /* optional additional argument */
{
   register dds_t   *p_dds;
   register int      adap;
   int               rc;
   open_elem_t     *open_ptr;

   DEBUGTRACE5 ("IOCb", (ulong)devno, (ulong)cmd, (ulong)arg,
      (ulong)devflag); /* tokioctl begin */
   DEBUGTRACE3 ("IOC+", (ulong)chan, (ulong)ext); /* tokioctl additional args */

   /* this shouldn't fail if kernel and device driver are working correctly */
   adap=minor(devno);
   if ((chan <= 0)                                 ||
       (chan > MAX_OPENS)                          ||
       ((p_dds = dd_ctrl.p_dds[adap]) == NULL) ||
       (CIO.chan_state[chan-1] != CHAN_OPENED)     ||
       ((open_ptr = CIO.open_ptr[chan-1]) == NULL)    )
   {
      TRACE2 ("IOC1", (ulong)ENXIO); /* tokioctl end (bad device or channel) */
      return (ENXIO);
   }

   /* handle standard ioctl's */
   switch (cmd)
   {
       case IOCINFO: /* let device-specific code handle this */
           rc = tokinfo(p_dds, arg, devflag);
           break;

       case CIO_START: /* kernel or user process */
	   TRACE2("strt", p_dds);
           rc = cio_start(p_dds, open_ptr, cmd, arg, devflag, chan, ext);
           break;

       case CIO_HALT: /* kernel or user process */
           rc = cio_halt(p_dds, open_ptr, cmd, arg, devflag, chan, ext);
           break;

       case CIO_QUERY: /* expect this to be restricted to priviledged procs */
           rc = cio_query(p_dds, open_ptr, cmd, arg, devflag, chan, ext);
           break;

       case CIO_GET_STAT: /* user process only */
           rc = cio_get_stat(p_dds, open_ptr, cmd, arg, devflag, chan, ext);
           break;

       case CIO_GET_FASTWRT: /* kernel or user process */
           rc = cio_get_fastwrt(open_ptr, arg, devflag);
           break;

       case TOK_QVPD: /* kernel or user process */
           /* return whole structure with status, length, (data) */
           rc = COPYOUT (devflag, &VPD, arg, sizeof(VPD));
           break;

       case TOK_DOWNLOAD_UCODE: /* Download microcode to adapter */
           rc = tokdnld(p_dds, arg, devflag);
           break;

       case TOK_ACCESS_POS:   /* Access POS registers */
           rc = tokposacc(p_dds, arg, devflag);
           break;

       case TOK_SET_ADAP_IPARMS:       /* Set Initialization Options: */
           rc = tokinitop(p_dds, arg, devflag);
           break;

       case TOK_SET_OPEN_PARMS:        /* Set Open Options: */
           rc = tokopenop(p_dds, arg, devflag);
           break;

       case TOK_FUNC_ADDR: /* Set functional address */
           rc = tokfuncaddr(p_dds, arg, devflag, open_ptr);
           break;

       case TOK_GRP_ADDR:  /* Set group address */
           rc = tokgrpaddr(p_dds, arg, devflag, open_ptr);
           break;

       case TOK_RING_INFO: /* Token-Ring Information */
           rc = ring_info(p_dds, arg, devflag);
           break;

       default:            /* Invalid (unsupported) op */
           rc = EINVAL;
           break;

   } /* end switch (cmd) */

   DEBUGTRACE2 ("IOCe", (ulong)rc); /* tokioctl end */
   return (rc);
} /* end tokioctl */


/*
 * NAME: cio_start()
 *
 * FUNCTION:
 *      This function handles the CIO_START ioctl.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the process thread.
 *
 * NOTES:
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *      software error recovery.
 *
 * DATA STRUCTURES:
 *
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */
int
cio_start( dds_t    *p_dds,
           open_elem_t     *open_ptr,
           int             op,
           int             arg,
           unsigned long   devflag,
           chan_t  chan,     /* channel number */
           int             ext)

{
	cio_sess_blk_t    sess_blk;
	cio_stat_blk_t    stat_blk;
	int               rc=0;
	int               saved_intr_level;
	int               just_activated;

	/* get parameter block */
	if ( rc = COPYIN (devflag, arg, &sess_blk, sizeof(sess_blk) ) )
		return(rc);

	DEBUGTRACE2("snet", sess_blk.netid);
	if ( (p_dds->wrk.limbo == NO_OP_STATE) &&
		(p_dds->wrk.adap_state == NULL_STATE) )
	{
		/* we have had a fatal error
		 * reject this start
		 */
		/* 
		 * FUTURE FIX:
		 *	The following code needs to be uncommented
		 *	when the pubs are changed to indicate that
		 *	a fatal error has occured in the status field.
		 *
		 * sess_blk.status = CIO_HARD_FAIL;
		 * if ( !COPYOUT(devflag, &sess_blk, arg, sizeof(sess_blk) ) )
		 * 		rc = ENETDOWN; 
		 */
		return(ENETDOWN);
	} /* end if fatal error */

        if (sess_blk.netid >= TOK_MAX_NETIDS) {
		sess_blk.status = CIO_NETID_FULL;
		if ( !COPYOUT(devflag, &sess_blk, arg, sizeof(sess_blk) ) )
			return(EINVAL); 
        }

        if (CIO.netid_table[sess_blk.netid].inuse) {
		sess_blk.status = CIO_NETID_DUP;
            	COPYOUT(devflag, &sess_blk, arg, sizeof(sess_blk));
		return(EADDRINUSE); /* duplicate netid */
        }

        DISABLE_INTERRUPTS (saved_intr_level);
        /* at this point, the net id is acceptable and will go into table */
        CIO.netid_table[sess_blk.netid].inuse = TRUE;
        CIO.netid_table[sess_blk.netid].chan = chan;
        CIO.netid_table[sess_blk.netid].faddr = 0;
        CIO.num_netids++;
        if (RAS.cc.nid_tbl_high < CIO.num_netids) /* update hi water mark */
		RAS.cc.nid_tbl_high = CIO.num_netids;

        just_activated = FALSE; /* dsact could cause conn_done very quickly */
        if (CIO.device_state == DEVICE_NOT_CONN) {
		just_activated = TRUE;
		CIO.device_state = DEVICE_CONN_IN_PROG;

		/* activate ("open" or "start" or "connect") the adapter */
		ENABLE_INTERRUPTS (saved_intr_level);
		if (rc = ds_act (p_dds)) {
			DISABLE_INTERRUPTS (saved_intr_level);
			CIO.device_state = DEVICE_NOT_CONN;
			CIO.netid_table[sess_blk.netid].inuse = TRUE;
			CIO.num_netids--;
			ENABLE_INTERRUPTS (saved_intr_level);
			return(rc);
		}
         } else
		ENABLE_INTERRUPTS (saved_intr_level);

         /* Careful! We may have just activated the adapter and it might     */
         /* have finished very quickly.  It that is the case, then           */
         /* conn_done has already processed the START_DONE for this one. */
         if ((CIO.device_state == DEVICE_CONNECTED) && (!just_activated)) {
		TRACE1("just");
		/* let ds code build status block */
		ds_startblk (p_dds, sess_blk.netid, &stat_blk);

		/* "asynchronous" notification to caller that start complete */
		report_status (p_dds, open_ptr, &stat_blk);

                if (sess_blk.netid == TOK_MAC_FRAME_NETID)
			p_dds->wrk.mac_frame_active = TRUE;
        }
	return(rc);
}

/*
 * NAME: cio_halt()
 *
 * FUNCTION:
 *      This function handles the CIO_HALT ioctl.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the process thread.
 *
 * NOTES:
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *      software error recovery.
 *
 * DATA STRUCTURES:
 *
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */
int
cio_halt(  dds_t    *p_dds,
           open_elem_t     *open_ptr,
           int             op,
           int             arg,
           unsigned long   devflag,
           chan_t  chan,     /* channel number */
           int             ext)

{
	cio_sess_blk_t    sess_blk;
	int               rc=0;

        if (CIO.num_netids == 0)
		return(EINVAL); /* no netid's at all */

	if ( rc = COPYIN (devflag, arg, &sess_blk, sizeof(sess_blk) ) )
		return(rc);

	if (CIO.netid_table[sess_blk.netid].inuse) {
		CIO.netid_table[sess_blk.netid].inuse = FALSE;
	
		/* let device specific code perform halt functions */
		ds_halt (p_dds, open_ptr, &sess_blk);

		if (CIO.netid_table[TOK_MAC_FRAME_NETID].inuse == TRUE)
			p_dds->wrk.mac_frame_active = TRUE;
		else
			p_dds->wrk.mac_frame_active = FALSE;
	} else
		rc = EINVAL; /* netid not in table */

	return(rc);
}

/*
 * NAME: cio_query()
 *
 * FUNCTION:
 *      This function handles the CIO_QUERY ioctl.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the process thread.
 *
 * NOTES:
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *      software error recovery.
 *
 * DATA STRUCTURES:
 *
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */
int
cio_query(  dds_t    *p_dds,
           open_elem_t     *open_ptr,
           int             op,
           int             arg,
           unsigned long   devflag,
           chan_t  chan,     /* channel number */
           int             ext)

{  /* begin function cio_query() */
   int               rc=0;
   cio_query_blk_t   qparms;
   int               saved_intr_level;

         /* get the caller's parameters */
         if ( rc = COPYIN (devflag, arg, &qparms, sizeof(qparms) ) )
            return(rc);


         qparms.status = CIO_OK;

         /* copy data to caller's buffer */
         saved_intr_level = i_disable(INTCLASS2);
         rc = COPYOUT (devflag, &p_dds->ras,
            (uchar *)qparms.bufptr, MIN(qparms.buflen, sizeof(query_stats_t) ));

         /* get and/or clear RAS area */
         if (qparms.clearall == CIO_QUERY_CLEAR)
            bzero (&p_dds->ras, sizeof(query_stats_t));
         i_enable(saved_intr_level);

         /* if the copyout failed, return */
         if (rc)
            return(rc);

         /* return parameter block to caller */
         rc = COPYOUT (devflag, &qparms, arg, sizeof(qparms));

  return(rc);
}  /* end function cio_query() */

/*
 * NAME: cio_get_stat()
 *
 * FUNCTION:
 *      This function handles the CIO_GET_STAT ioctl.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the process thread.
 *
 * NOTES:
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *      software error recovery.
 *
 * DATA STRUCTURES:
 *
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */
int
cio_get_stat(dds_t    *p_dds,
             open_elem_t     *open_ptr,
             int             op,
             int             arg,
             unsigned long   devflag,
             chan_t  chan,     /* channel number */
             int             ext)

{  /* begin function cio_get_stat() */
   int               rc=0;
   int               saved_intr_level;
   cio_stat_blk_t    stat_blk;
   sta_elem_t       *sta_ptr;



         if (devflag & DKERNEL) /* illegal call from kernel process */
         {
            rc = EACCES;
            return(rc);
         }

        /*  DISABLE_INTERRUPTS (saved_intr_level);
         if (open_ptr->sta_que_ovrflw)
         {
            open_ptr->sta_que_ovrflw = FALSE;
            stat_blk.code = (ulong)CIO_LOST_STATUS;
            ENABLE_INTERRUPTS (saved_intr_level);
         }
         else
         {
            ENABLE_INTERRUPTS (saved_intr_level);
        */
            /* get the next status que element */
            sta_ptr=(sta_elem_t *)sll_unlink_first(&(open_ptr->sta_que));
            if (sta_ptr == NULL)
            {
               stat_blk.code = (ulong)CIO_NULL_BLK;
            }
            else
            {
               /* get useful stuff for user */
               stat_blk = sta_ptr->stat_blk;

               /* free the status que element */
               sll_free_elem ((s_link_list_t *)(&(open_ptr->sta_que)),
                  (sll_elem_ptr_t)sta_ptr);
            }
        /* } */
         rc = COPYOUT (devflag, &stat_blk, arg, sizeof(stat_blk));

   return(rc);
} /* end function cio_get_stat() */


/*****************************************************************************/
/*
 * NAME:     cio_get_fastwrt
 *
 * FUNCTION: returns the fastwrite address to a kernel user along with
 *       the parameters required to access the entry point.
 *
 * EXECUTION ENVIRONMENT: called from the process environment only
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *
 */
/**********************************************************************/

static int cio_get_fastwrt(
	open_elem_t			*p_open,
	cio_get_fastwrt_t	*p_arg,
	ulong				devflag)

{
	cio_get_fastwrt_t   fastwrt;
	int					tokfastwrt();

	if (!(devflag & DKERNEL))
		return(EPERM);

	fastwrt.status = CIO_OK;
	fastwrt.fastwrt_fn = tokfastwrt;
	fastwrt.chan = 0;
	fastwrt.devno = p_open->devno;

	bcopy(&fastwrt, p_arg, sizeof(cio_get_fastwrt_t));

	return(0);
}

/*-------------------------  T O K P O S A C C  ---------------------------*/
/*                                                                         */
/*  Access Adapter POS Registers.                                          */
/*                                                                         */
/*-------------------------------------------------------------------------*/

tokposacc ( dds_t               *p_dds,
            caddr_t             arg,
            unsigned long       devflag)
{
        int                     rc = 0;
        int                     pio_attachment;
        tok_pos_reg_t           posdata;

        if (!arg) {
            return(EFAULT);                     /* check arg pointer */
        } else  {
            if (devflag & DKERNEL) {            /* get data from user */
                bcopy(arg, &posdata, sizeof(posdata));
            } else {
                if (copyin(arg, &posdata, sizeof(posdata)) != 0) {
                    return(EFAULT);
                }
            }
        }
        if ((p_dds->cio.mode != 'D') &&
                (p_dds->cio.mode != 'W')){ /* must be in Diag mode */
            posdata.status = TOK_NOT_DIAG_MODE;
            rc = ENOMSG;
        } else {
            pio_attachment = attach_iocc( p_dds );
            switch (posdata.opcode) {           /* read or write? */
                case TOK_READ:                  /* READ */
                    if (posdata.pos_reg > 7 || posdata.pos_reg < 0) {
                        rc = ENOMSG;               /* out of range */
                        posdata.status = TOK_BAD_RANGE;
                    } else {
                        posdata.pos_val = pio_read( p_dds, posdata.pos_reg );
                        posdata.status  = CIO_OK;
                    }
                    break;
                case TOK_WRITE:                 /* WRITE */
                    /*                                                  */
                    /*  The only legal POS registers to write to are    */
                    /*  POS 2, 3, 4, and 5.                             */
                    /*                                                  */
                        /*
                         * FUTURE FIX:
                         *      Add check for device already activated
                         *      return ENOMSG and set status to
                         *      TOK_NO_POS
                         */
                    if ((posdata.pos_reg > POS_REG_5) ||
                        (posdata.pos_reg < POS_REG_2)) {
                        rc = ENOMSG;               /* out of range */
                        posdata.status = TOK_BAD_RANGE;
                        break;
                    }
                    pio_write( p_dds, posdata.pos_reg, posdata.pos_val );
                    posdata.status = CIO_OK;
                    /*                                                  */
                    /*  If the ring speed selection bit of POS reg 3    */
                    /*  is changed, update the max packet length to     */
                    /*  accomodate the new ring speed.                  */
                    /*                                                  */
                    if ((posdata.pos_reg == POS_REG_3) &&
                        (posdata.pos_val & RING_SPEED_BIT))
                            p_dds->wrk.max_packet_len
                                    = TOK_16M_MAX_PACKET;
                    else
                            p_dds->wrk.max_packet_len
                                    = TOK_4M_MAX_PACKET;
                    break;
                default:                        /* Invalid Opcode */
                    rc = EINVAL;
                    break;
            }
            detach_iocc( p_dds, pio_attachment );
        }
    exit:
        if (devflag & DKERNEL) {                /* caller is kernel */
            bcopy(&posdata, arg, sizeof(posdata));
        } else {                                /* caller is user */
            if (copyout(&posdata, arg, sizeof(posdata)) != 0) {
                return(EFAULT);
            }
        }
        return(rc);                             /* return any error codes */
}


/*----------------------------  T O K I N F O   ---------------------------*/
/*                                                                         */
/*  Return TOKINFO Information.                                            */
/*                                                                         */
/*-------------------------------------------------------------------------*/

tokinfo (dds_t          *p_dds,
         caddr_t        arg,
         unsigned long  devflag)
{
        unsigned int           i;
        int                    pio_attachment;
        struct devinfo         info;
                                        /* check pointer */
        if (!arg) return(EFAULT);
        /*                                                                 */
        /*  The "capability" is determined by examining the ring speed     */
        /*  select bit of POS register 2;  1 = 16 Mbytes/sec, 0 = 4MB/sec. */
        /*                                                                 */

        pio_attachment = attach_iocc(p_dds);
                                        /* if ring speed select, */
        if (pio_read( p_dds, POS_REG_3 ) & RING_SPEED_BIT)
            info.un.token.speed = TOK_16M;       /* 16 MB/sec */
        else
            info.un.token.speed = TOK_4M;        /* else 4 MB/sec */
        detach_iocc( p_dds, pio_attachment );

                /* wrap of broadcast packets supported */
        info.un.token.broad_wrap = TRUE;

                /* Receive data transfer offset */
        info.un.token.rdto = p_dds->ddi.rdto;
        info.devtype = DD_NET_DH;      /* type is network device handler */
        info.flags = NULL;             /*No-op on flags field */
        info.devsubtype  = DD_TR;      /* token ring device driver */

        for (i = 0; i < 6; i++)         /* get hardware address */
        {
            info.un.token.haddr[i] = p_dds->wrk.tok_vpd_addr[i];
            if (p_dds->ddi.use_alt_addr)
                info.un.token.net_addr[i] = p_dds->ddi.alt_addr[i];
            else
                info.un.token.net_addr[i] = p_dds->wrk.tok_vpd_addr[i];
        }
        /*                                                                 */
        /*  The structure is then returned to the user by copying it to    */
        /*  the proper user or kernel space.                               */
        /*                                                                 */
        if (devflag & DKERNEL)          /* caller is kernel */
            bcopy(&info, arg, sizeof(info));
        else                            /* caller is user */
            if (copyout(&info, arg, sizeof(info)) != 0)
                return(EFAULT);
        return(0);                      /* success, return 0 */
}


/*-------------------------  T O K I N I T O P  ---------------------------*/
/*                                                                         */
/*  Set Adapter Init Options.                                              */
/*                                                                         */
/*-------------------------------------------------------------------------*/

tokinitop (dds_t                *p_dds,
           caddr_t              arg,
           unsigned long        devflag)
{
        int                     rc;
        tok_set_adap_i_parms_t  op;

        if (p_dds->cio.device_state != DEVICE_NOT_CONN)
            return(EEXIST);                     /* can't if adapter started */
        if (!arg)
        {                                       /* check arg pointer */
            return(EFAULT);
        }
        else
        {
            if (devflag & DKERNEL) {            /* copy from caller */
                bcopy(arg, &op, sizeof(op));
            } else {
                if (copyin(arg, &op, sizeof(op)) != 0) {
                    return(EFAULT);
                }
            }
        }
        rc = ENOMSG;                            /* assume the worst */
                                                /* must be in Diag mode */
        if ((p_dds->cio.mode != 'D') &&
                (p_dds->cio.mode !='W'))
        {
            op.status = TOK_NOT_DIAG_MODE;
            goto exit;
        }
        if (op.xmit_burst_size & 1)
            return (EINVAL);                    /* must be even */
        if (!op.dma_abort_thresh)
            return (EINVAL);                    /* must be non-zero */


        /*                                                             */
        /*  Copy init option parameters from arg to DDS.               */
        /*                                                             */
        p_dds->wrk.diag_iparms.init_options
                = op.init_options;
        p_dds->wrk.diag_iparms.rcv_burst_size
                = op.rcv_burst_size;
        p_dds->wrk.diag_iparms.xmit_burst_size
                = op.xmit_burst_size;
        p_dds->wrk.diag_iparms.dma_abort_thresh
                = op.dma_abort_thresh;
        op.status = CIO_OK;                     /* set status */
        rc = 0;
                                                /* return arg to caller */
    exit:
        if (devflag & DKERNEL) {                /* caller is kernel */
            bcopy(&op, arg, sizeof(op));
        } else {                                /* caller is user */
            if (copyout(&op, arg, sizeof(op)) != 0) {
               return(EFAULT);
            }
        }
        return(rc);
}  /* end function tokinitop */


/*-------------------------  T O K O P E N O P  ---------------------------*/
/*                                                                         */
/*  Set Adapter Open Options.                                              */
/*                                                                         */
/*-------------------------------------------------------------------------*/

tokopenop (dds_t                *p_dds,
           caddr_t              arg,
           unsigned long        devflag)
{
        int                     rc = 0;
        tok_set_open_opts_t     op;

        if (p_dds->cio.device_state != DEVICE_NOT_CONN)
            return(EEXIST);                     /* can't if adapter started */
        if (!arg) {                             /* check arg pointer */
            return(EFAULT);
        } else {
            if (devflag & DKERNEL) {            /* copy from caller */
                bcopy(arg, &op, sizeof(op));
            } else {
                if (copyin(arg, &op, sizeof(op)) != 0) {
                    return(EFAULT);
                }
            }
        }
        /*  Check values in arg; set defaults if necessary.  Make sure */
        /*  the adapter is in Diag mode.  Each of these checks returns */
        /*  a value in status and sets the return code to EIO if there */
        /*  is a problem.                                              */
        /*                                                             */
        rc = EIO;                               /* guilty til' innocent */
                                                /* must be in Diag mode */
        if ((p_dds->cio.mode != 'D') &&
            (p_dds->cio.mode != 'W'))
        {
            op.status = TOK_NOT_DIAG_MODE;
            goto exit;
        }
        if (op.buf_size == 0)                   /* default for zero */
            op.buf_size =  112;
        if (op.buf_size < 96 || (op.buf_size & 0x07)) {
            op.status = TOK_BAD_RANGE;          /* buf size not in range */
            goto exit;
        }
        if (op.xmit_buf_max_cnt == 0)           /* default for zero */
            op.xmit_buf_max_cnt =  6;
        if (op.xmit_buf_min_cnt > op.xmit_buf_max_cnt) {
            op.status = TOK_BAD_RANGE;          /* min must be <= max */
            goto exit;
        }
        /*
        *       FUTURE FIX:
        *
        *       Also, put check if device already activated.
        *       If so, return ENOMSG and set status tok TOK_NO_PARMS.
        */

        /*                                                             */
        /*  Copy open option parameters from arg to DDS.               */
        /*                                                             */
        p_dds->wrk.diag_open_opts.options          = op.options;
        p_dds->wrk.diag_open_opts.buf_size         = op.buf_size;
        p_dds->wrk.diag_open_opts.xmit_buf_min_cnt = op.xmit_buf_min_cnt;
        p_dds->wrk.diag_open_opts.xmit_buf_max_cnt = op.xmit_buf_max_cnt;
        p_dds->wrk.diag_open_opts.i_addr1       = op.i_addr1;
        p_dds->wrk.diag_open_opts.i_addr2       = op.i_addr2;
        p_dds->wrk.diag_open_opts.i_addr3       = op.i_addr3;

        op.status = CIO_OK;                     /* set status */
        rc = 0;
                                                /* return arg to caller */
    exit:
        if (devflag & DKERNEL) {                /* caller is kernel */
            bcopy(&op, arg, sizeof(op));
        } else {                                /* caller is user */
            if (copyout(&op, arg, sizeof(op)) != 0) {
               return(EFAULT);
            }
        }
        return(rc);
}

/*-------------------------  T O K G R P A D D R   ------------------------*/
/*                                                                         */
/*  Sets the Group Address for the Token-Ring adapter.  Only one Group     */
/*  Address may be specified at a time; the parameter block for this       */
/*  command can be found is the tok_group_addr_t found in sys/tokuser.h.   */
/*                                                                         */
/*-------------------------------------------------------------------------*/

tokgrpaddr (register dds_t      *p_dds,
            caddr_t             arg,
            unsigned long       devflag,
            open_elem_t         *open_ptr)
{
        int                     rc = 0;
	int			sil;
        tok_group_addr_t        groupaddr;

        /*                                                                 */
        /*  If the call is from a kernel process, simply copy the param-   */
        /*  eters over.  If the call is from a user, copy the parameters   */
        /*  from user space to kernel space.                               */
        /*                                                                 */
        if (!arg) {
            return(EFAULT);                     /* check arg pointer */
        } else  {
            if (devflag & DKERNEL) {            /* get data from kernel */
                bcopy(arg, &groupaddr, sizeof(groupaddr));
            } else {                            /* get it from a user */
                if (copyin(arg, &groupaddr, sizeof(groupaddr)) != 0) {
                    return(EFAULT);
                }
            }
        }
        if (p_dds->cio.device_state != DEVICE_CONNECTED) 
	{
            rc = ENETUNREACH;                   /* not started, so */
            groupaddr.status = CIO_NOT_STARTED; /* return an error */
        } 
	else if ( (p_dds->wrk.limbo == NO_OP_STATE) &&
		  (p_dds->wrk.adap_state == NULL_STATE) )
	{
		/* FUTURE FIX:
		 * 	The status field should be filled in 
		 *	with CIO_HARD_FAIL, but that error code is
		 * 	not documented in the pubs.  When the
		 *	The status code should be changed to 
		 *	CIO_HARD_FAIL when the pubs are changed.
		 */
            groupaddr.status = CIO_TIMEOUT;
            rc = ENETDOWN;
	} /* end if fatal error has occured */
	else 
	{                                /* otherwise, */
        /*                                                                */
        /*  Update the DDS group address variable with the new group      */
        /*  address.  If the group address is to be deleted, simply       */
        /*  change it to a null group address.                            */
        /*                                                                */
            switch (groupaddr.opcode) {         /* add or delete? */
                case TOK_ADD:                   /* ADD GROUP ADDRESS */
                    if (p_dds->wrk.group_address) {
                        if (open_ptr->chan      /* by someone else, */
                                != p_dds->wrk.group_addr_chan) {
                            rc = ENOMSG;        /* return an error */
                            groupaddr.status = TOK_NO_GROUP;
                            break;              /* leave switch */
                        }
                    }                           /* else, change it, */
                    p_dds->wrk.group_address   = groupaddr.group_addr;
                    p_dds->wrk.group_addr_chan = open_ptr->chan;
                    break;
                case TOK_DEL:                   /* DELETE GROUP ADDRESS */
                    if (open_ptr->chan          /* imposter deleting it? */
                            != p_dds->wrk.group_addr_chan) {
                        rc = ENOMSG;            /* return an error */
                        groupaddr.status = TOK_NO_GROUP;
                        break;                  /* and leave switch */
                    }                           /* else, delete it */
                    p_dds->wrk.group_address   = 0;
                    p_dds->wrk.group_addr_chan = 0;
                    break;
                default:                        /* Invalid Opcode */
                    groupaddr.status = TOK_INV_CMD;
                    rc = ENOMSG;
                    break;
            }
        }
        /*  If the group address was changed successfully, set the wait */
        /*  flag, issue the group address command, then wait for the    */
        /*  reply interrupt.                                            */
        /*                                                              */
        if ((rc == 0) && (p_dds->wrk.limbo == PARADISE))
        {
           /*
            *  Set timer for 1 sec
            */
            p_dds->wrk.p_group_timer->timeout.it_value.tv_sec
                        = GROUP_ADDR_TIMEOUT / 1000;
            p_dds->wrk.p_group_timer->timeout.it_value.tv_nsec
                        = (GROUP_ADDR_TIMEOUT % 1000) * 1000000;
            p_dds->wrk.group_td.owe.who_queued = OFLV_IOCTL;
            p_dds->wrk.group_td.owe.cmd = IOCTL_GROUP;

            p_dds->wrk.group_wait = TRUE;
            p_dds->wrk.group_event = EVENT_NULL;
	    sil = i_disable(INTCLASS2);
            initiate_scb_command
                (p_dds, ADAP_GROUP_CMD, p_dds->wrk.group_address);
            tstart( p_dds->wrk.p_group_timer );
            p_dds->wrk.group_td.run = TRUE;

            while (p_dds->wrk.group_wait)
                e_sleep( &p_dds->wrk.group_event, EVENT_SIGRET );

	    i_enable(sil);
            groupaddr.status = p_dds->wrk.group_status;
            if (groupaddr.status != CIO_OK)
                rc = ENOMSG;
        }
        /*  If the call was from a kernel process, simply copy the param-  */
        /*  eters back.  If the call is from a user, copy the parameters   */
        /*  from kernel to user space.                                     */
        /*                                                                 */
        if (devflag & DKERNEL) {                /* caller is kernel */
            bcopy(&groupaddr, arg, sizeof(groupaddr));
        } else {                                /* caller is user */
            if (copyout(&groupaddr, arg, sizeof(groupaddr)) != 0) {
                return(EFAULT);
            }
        }
        return(rc);                             /* return any error codes */
}

/*-----------------------  T O K F U N C A D D R   ------------------------*/
/*                                                                         */
/*  Specifies a Functional Address to be used on the Token Ring.           */
/*                                                                         */
/*-------------------------------------------------------------------------*/

tokfuncaddr (register dds_t     *p_dds,
             caddr_t            arg,
             unsigned long      devflag,
             open_elem_t        *open_ptr)
{
        int                     i, found = FALSE, rc = 0;
	int			sil;	
        unsigned int            functional_address();
        tok_func_addr_t         funcaddr;

        /*  If the call is from a kernel process, copy the parameters   */
        /*  over.  If the call is from a user, copy the parameters      */
        /*  from user space to kernel space.                            */
        /*                                                              */
        if (!arg) {
            return(EFAULT);                     /* check arg pointer */
        } else  {
            if (devflag & DKERNEL) {            /* get data from user */
                bcopy(arg, &funcaddr, sizeof(funcaddr));
            } else {
                if (copyin(arg, &funcaddr, sizeof(funcaddr)) != 0) {
                    return(EFAULT);
                }
            }
        }
        /*  If the adapter has not been started, return an error.       */
        /*                                                              */
        if (p_dds->cio.device_state != DEVICE_CONNECTED) 
	{
            funcaddr.status = CIO_NOT_STARTED;
            rc = ENETUNREACH;
            goto exit;
        }
	else if ( (p_dds->wrk.limbo == NO_OP_STATE) &&
		  (p_dds->wrk.adap_state == NULL_STATE) )
	{
		/* FUTURE FIX:
		 * 	The status field should be filled in 
		 *	with CIO_HARD_FAIL, but that error code is
		 * 	not documented in the pubs.  When the
		 *	The status code should be changed to 
		 *	CIO_HARD_FAIL when the pubs are changed.
		 */
            funcaddr.status = CIO_TIMEOUT;
            rc = ENETDOWN;
            goto exit;
	} /* end if fatal error has occured */

        /*  Find the netid in the netid table, owned by this channel    */
        /*  number.  If not found, return an error.                     */
        /*                                                              */
        if ((p_dds->cio.netid_table[funcaddr.netid].inuse) &&
            (p_dds->cio.netid_table[funcaddr.netid].chan == open_ptr->chan))
                found = TRUE;

        if (!found) {
            funcaddr.status = CIO_NETID_INV;
            rc = ENOMSG;
            goto exit;
        }
        /*  Update the DDS functional address by ORing in the new         */
        /*  functional address bits.  If those bits are to be deleted,    */
        /*  AND in the complement.                                        */
        /*                                                                */
        switch (funcaddr.opcode) {          /* add or delete? */

            case TOK_ADD:                   /* ADD GROUP ADDRESS */
                p_dds->cio.netid_table[funcaddr.netid].faddr
                                        |= funcaddr.func_addr;
                break;

            case TOK_DEL:                   /* DELETE GROUP ADDRESS */
                p_dds->cio.netid_table[funcaddr.netid].faddr
                                        &= ~funcaddr.func_addr;
                break;

            default:                        /* Invalid Opcode */
                funcaddr.status = TOK_INV_CMD;
                rc = ENOMSG;
                break;
        }
        /*  If the functional address was chaned successfully, set      */
        /*  the wait flag, issue the functional address command, then   */
        /*  wait for the reply interrupt.                               */
        /*                                                              */
        if ((rc == 0) && (p_dds->wrk.limbo == PARADISE))
        {
           /*
            *  Set timer for 1 sec
            */
            p_dds->wrk.p_func_timer->timeout.it_value.tv_sec
                        = FUNCTIONAL_ADDR_TIMEOUT / 1000;
            p_dds->wrk.p_func_timer->timeout.it_value.tv_nsec
                        = (FUNCTIONAL_ADDR_TIMEOUT % 1000) * 1000000;
            p_dds->wrk.functional_td.owe.who_queued = OFLV_IOCTL;
            p_dds->wrk.functional_td.owe.cmd = IOCTL_FUNC;
            p_dds->wrk.funct_wait = TRUE;
            p_dds->wrk.funct_event = EVENT_NULL;
	    sil = i_disable(INTCLASS2);
            initiate_scb_command
                (p_dds, ADAP_FUNCT_CMD, functional_address(p_dds));
            tstart( p_dds->wrk.p_func_timer );
            p_dds->wrk.functional_td.run = TRUE;

            while (p_dds->wrk.funct_wait)
                e_sleep( &p_dds->wrk.funct_event, EVENT_SIGRET );
	    i_enable(sil);
            funcaddr.status = p_dds->wrk.funct_status;
            if (funcaddr.status != CIO_OK)
                rc = ENOMSG;
        }
    exit:                                       /* return status */
        if (devflag & DKERNEL) {                /* caller is kernel */
            bcopy(&funcaddr, arg, sizeof(funcaddr));
        } else {                                /* caller is user */
            if (copyout(&funcaddr, arg, sizeof(funcaddr)) != 0) {
                return(EFAULT);
            }
        }
        return(rc);                             /* return any error codes */
}

/*------------------  F U N C T I O N A L _ A D D R E S S  ----------------*/
/*                                                                         */
/*  Calculates the current overall functional address from the netid       */
/*  table.                                                                 */
/*                                                                         */
/*-------------------------------------------------------------------------*/

unsigned int
functional_address (dds_t *p_dds)
{
        register int             i;
        register unsigned int    func_addr = 0;
                                                /* OR all bits together */
        for (i = 0; i < TOK_MAX_NETIDS; i++)
		if (p_dds->cio.netid_table[i].inuse)
			func_addr |= p_dds->cio.netid_table[i].faddr;
        p_dds->wrk.funct_address = func_addr;   /* save in dds */
        return(func_addr);                      /* then return result */
}



/*
*  FUNCTION:   ring_info()
*
*  INPUT:  p_dds   - pointer to DDS
*          arg     - pointer to tok_q_ring_info_t structure
*          devflag - Kernel or non-kernel caller
*
*/

int
ring_info(dds_t          *p_dds,
              caddr_t        arg,
              unsigned long  devflag)
{

unsigned int len;
int do_read, sil, rc;
tok_q_ring_info_t  ringtmp;
                                        /* check pointer */

	DEBUGTRACE4("RNGI", (ulong)p_dds, (ulong)arg, (ulong)devflag );
        /*                                                                 */
        /*  If the call is from a kernel process, simply copy the  param-  */
        /*  eters over.  If the call is from a user, copy the parameters   */
        /*  from user space to kernel space.                               */
        /*                                                                 */
        if (!arg)
        {
            return(EFAULT);                     /* check arg pointer */
        }
        else
        {
            if (devflag & DKERNEL)
            {
                bcopy(arg, &ringtmp, sizeof(ringtmp));
            }
            else
            {
                if (copyin(arg, &ringtmp, sizeof(ringtmp)) != 0)
                    return(EFAULT);
            }
        }


	if (p_dds->cio.device_state != DEVICE_CONNECTED)
	{
		ringtmp.status = CIO_NOT_STARTED;
		do_read = FALSE;
	}
	else if (p_dds->wrk.limbo != PARADISE)
	{
		ringtmp.status = CIO_OK;
		do_read = FALSE;
	}
	else
	{
		ringtmp.status = CIO_OK;
		do_read = TRUE;		/* can read info from card */
	}

	DEBUGTRACE4("RNG1", (ulong)p_dds, (ulong)arg, (ulong)ringtmp.status );

        if (do_read) 
        {
           /*
            *  Set timer for 1 sec
            */
            p_dds->wrk.p_ring_info_timer->timeout.it_value.tv_sec
                        = RING_INFO_TIMEOUT / 1000;
            p_dds->wrk.p_ring_info_timer->timeout.it_value.tv_nsec
                        = (RING_INFO_TIMEOUT % 1000) * 1000000;
            p_dds->wrk.ring_info_td.owe.who_queued = OFLV_IOCTL;
            p_dds->wrk.ring_info_td.owe.cmd = IOCTL_RING_INFO;
            p_dds->wrk.ring_info_wait = TRUE;
            p_dds->wrk.ring_info_event = EVENT_NULL;

	    DEBUGTRACE1("RNG2");
	    sil = i_disable(INTCLASS2);

		if ( rc = get_ring_info( p_dds ) )
		{
			i_enable(sil);
		}
		else
		{

			tstart( p_dds->wrk.p_ring_info_timer );
			p_dds->wrk.ring_info_td.run = TRUE;

			while (p_dds->wrk.ring_info_wait)
			e_sleep( &p_dds->wrk.ring_info_event, EVENT_SIGRET );

			i_enable(sil);
			ringtmp.status = p_dds->wrk.ring_info_status;
		}

	}
	/*
	 *   Get the length of data to transfer to user.
	 */

	len = MIN( ringtmp.l_buf,sizeof(tok_ring_info_t) );

        /*                                                                 */
        /*  The structure is then returned to the user by copying it to    */
        /*  the proper user or kernel space.                               */
        /*                                                                 */

	if (devflag & DKERNEL)          /* caller is kernel */
	{
		bcopy(&(p_dds->wrk.ring_info), ringtmp.p_info, len);
	}
	else                            /* caller is user */
	{
		if (copyout( &(p_dds->wrk.ring_info),
		    ringtmp.p_info, len) != 0)
			return(EFAULT);
	}

	if (devflag & DKERNEL)
	{                /* caller is kernel */
		bcopy(&ringtmp, arg, sizeof(ringtmp));
	}
	else
	{                                /* caller is user */
		if (copyout(&ringtmp, arg, sizeof(ringtmp)) != 0)
                	return(EFAULT);
	}

	DEBUGTRACE1("RNG3");
	return(0);
}  /* end function ring_info() */
