static char sccsid[] = "@(#)69	1.21.1.6  src/bos/kernext/tokdiag/tokdd.c, diagddtok, bos411, 9428A410j 11/30/93 10:31:09";
/*
 * COMPONENT_NAME: SYSXTOK - Token-Ring device handler
 *
 * FUNCTIONS:  cio_add_cdt(), cio_del_cdt(), config_init(),
 *             config_term(), config_qvpd(), tokconfig(), local_devswadd(),
 *             initdds(), cfg_pos_regs()
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

#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/dma.h>
#include <sys/errno.h>
#include <sys/err_rec.h>
#include "tok_comio_errids.h"
#include <sys/intr.h>
#include <sys/ioctl.h>
#include <sys/limits.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/pri.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/timer.h>
#include <sys/trchkid.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/watchdog.h>
#include <sys/comio.h>  /* this and following files must be in this sequence */
#include <sys/tokuser.h>
#include "tokdshi.h"
#include "tokdds.h"
#include "tokdslo.h"
#include "tokproto.h"
#include "tokprim.h"

/*****************************************************************************/
/*
 * NAME: tokdd.c
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 */

/*****************************************************************************/
/* fixed storage area                                                        */
/*****************************************************************************/
dd_ctrl_t dd_ctrl;     /* expect = {FALSE, 0, {NULL,...,NULL}, 0} */

tracetable_t tracetable;    /* this is the trace table */
volatile cdt_t ciocdt;               /* this is the component dump table */

/*****************************************************************************/
/*
 * NAME:     cio_add_cdt
 *
 * FUNCTION: add an entry to the component dump table
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  nothing
 *
 */
/*****************************************************************************/
void cio_add_cdt (
   register char *name,  /* label string for area dumped */
   register char *ptr,   /* area to be dumped */
   register int   len)   /* amount of data to be dumped */
{
   int saved_intr_level;
   struct cdt_entry temp_entry;
   int num_elems;
   int rc;

   DEBUGTRACE4 ("ACDb", (ulong)name, 
		(ulong)ptr,(ulong)len); /* cio_add_cdt begin */

   strncpy (temp_entry.d_name, name, sizeof(temp_entry.d_name));
   temp_entry.d_len = len;
   temp_entry.d_ptr = ptr;
   temp_entry.d_xmemdp = NULL;

   DISABLE_INTERRUPTS (saved_intr_level);
   num_elems = (ciocdt.header._cdt_len - sizeof(ciocdt.header)) /
               sizeof(struct cdt_entry);
   if (rc = (num_elems < MAX_CDT_ELEMS))
   {
      ciocdt.entry[num_elems] = temp_entry;
      ciocdt.header._cdt_len += sizeof(struct cdt_entry);
   }
   ENABLE_INTERRUPTS (saved_intr_level);

   DEBUGTRACE2 ("ACDe",(ulong)(!rc)); /* cio_add_cdt end */
   return;
} /* end cio_add_cdt */

/*****************************************************************************/
/*
 * NAME:     cio_del_cdt
 *
 * FUNCTION: delete an entry from the component dump table
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  nothing
 *
 */
/*****************************************************************************/
void cio_del_cdt (
   register char *name,  /* label string for area dumped */
   register char *ptr,   /* area to be dumped */
   register int   len)   /* amount of data to be dumped */
{
   int saved_intr_level;
   struct cdt_entry temp_entry;
   int num_elems;
   int rc;
   int ndx;

   DEBUGTRACE4 ("DCDb", (ulong)name, 
		(ulong)ptr,(ulong)len); /* cio_del_cdt begin */

   strncpy (temp_entry.d_name, name, sizeof(temp_entry.d_name));
   temp_entry.d_len = len;
   temp_entry.d_ptr = ptr;
   temp_entry.d_xmemdp = NULL;

   DISABLE_INTERRUPTS (saved_intr_level);
   num_elems = (ciocdt.header._cdt_len - sizeof(ciocdt.header)) /
               sizeof(struct cdt_entry);

   /* find the element in the array (match only the memory pointer) */
   for (ndx = 0;
        (ndx < num_elems) &&
        (temp_entry.d_ptr != ciocdt.entry[ndx].d_ptr);
        ndx++)
      ; /* NULL statement */

   /* re-pack the array to remove the element if it is there */
   if (ndx < num_elems)
   {
      for (ndx++ ; ndx < num_elems; ndx++)
         ciocdt.entry[ndx-1] = ciocdt.entry[ndx];
      bzero (&ciocdt.entry[ndx-1], sizeof(struct cdt_entry));
      ciocdt.header._cdt_len -= sizeof(struct cdt_entry);
      rc = 0;
   }
   else /* item not in table */
   {
      rc = 1;
   }
   ENABLE_INTERRUPTS (saved_intr_level);

   DEBUGTRACE2 ("DCDe",(ulong)(!rc)); /* cio_del_cdt end */
   return;
} /* end cio_del_cdt */


/*****************************************************************************/
/*
 * NAME:     config_init
 *
 * FUNCTION: process tokconfig entry with cmd of INIT
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *
 */
/*****************************************************************************/
static int config_init (
   dev_t       devno, /* major and minor number */
   struct uio *uiop)  /* pointer to uio structure */
{
   register dds_t *p_dds;
            int    adap;
            ddi_t  tempddi;
            int    dds_size;
            int    saved_intr_level;
            int    tokslih(), tokoflv();

   TRACE3 ("CFIb", (ulong)devno, (ulong)uiop); /* config_init begin */

   adap = minor(devno);
   if (adap >= TOK_MAX_MINOR)
   {
      TRACE2 ("CFI4", (ulong)EINVAL); /* config_init end (invalid minor no) */
      return (EINVAL);
   }

   /* make sure dds not already supplied */
   if (dd_ctrl.p_dds[adap] != NULL)
   {
      TRACE2 ("CFI1", (ulong)EEXIST); /* config_init end (DDS already exists)*/
      return (EEXIST);
   }
   else if (uiop->uio_resid != sizeof(ddi_t)) /* make sure length is sensical */
   {
      TRACE2 ("CFI2", (ulong)EINVAL); /* config_init end (wrong length DDI) */
      return (EINVAL);
   }
                /* get temporary copy of ddi (it's normally in user space) */
   else if (uiomove (&tempddi, sizeof(tempddi), UIO_WRITE, uiop))
   {
      TRACE2 ("CFI3",
         (ulong)EFAULT); /* config_init end (uiomove of DDI failed) */
      return (EFAULT);
   }

   /* compute size needed for entire dds (depends on ddi contents) */
   dds_size = sizeof(dds_t);
   dds_size += tempddi.xmt_que_size * sizeof(xmt_elem_t);

   /* get memory for dds */
   if ((p_dds = (dds_t *) KMALLOC (dds_size)) == NULL)
   {
      TRACE2 ("CFI4",
         (ulong)ENOMEM); /* config_init end (malloc DDS area failed) */
      return (ENOMEM);
   }
   bzero (p_dds, dds_size); /* xmalloc does NOT zero storage provided */

   /* now move the temporary copy used for checking to the right place */
   bcopy (&tempddi, &(DDI), sizeof(ddi_t));

   /* finish initialization of DDS */
   CIO.alloc_size = dds_size;   /* save this for use with cio_del_cdt */
   CIO.devno = devno;           /* save this for use with selnotify */

   CIO.xmt_fn_needed = FALSE;
   CIO.xmit_event = FALSE;

   /* set up the interrupt control structure section */
   IHS.next = (struct intr *) NULL;
   IHS.handler = (int(*)())tokslih;
   IHS.bus_type = p_dds->ddi.bus_type;
   IHS.flags = 0;
   IHS.level = p_dds->ddi.bus_int_lvl;
   IHS.priority = INTCLASS2;
   IHS.bid = p_dds->ddi.bus_id;

   /* set up the watchdog timer control structure section */
   WDT.func = (void(*)())cio_wdt_func;
   WDT.restart = 10;
   WDT.count = 0;

   /* set up the offlevel control structure section */
   OFL.ihs.next = (struct intr *) NULL;
   OFL.ihs.handler = ( int (*) ())tokoflv;
   OFL.ihs.bus_type = BUS_NONE;
   OFL.ihs.flags = 0;
   OFL.ihs.level = INT_OFFL1;
   OFL.ihs.priority = INTOFFL1;
   OFL.ihs.bid = p_dds->ddi.bus_id;
   OFL.scheduled = FALSE;
   OFL.running = FALSE;
   OFL.next_in = 0;
   OFL.next_out = 0;

   WRK.xmit_queue = (xmt_elem_t *)((uint)p_dds + sizeof(dds_t));

   /* perform any device-specific initialization of dds and set POS registers*/
   /* if this routine returns non-zero, then the device cannot be configured */
   if (initdds (p_dds))
   {
      /* give back the dds memory */
      if (KMFREE (p_dds) != 0)
         TRACE2 ("CFI5",
            (ulong)p_dds); /* config_init ERROR (free p_dds failed) */
      TRACE2 ("CFI6",
         (ulong)ENXIO); /* config_init end (initdds failed) */
      return (ENXIO);
   }

   /* set up system timer for QUEOFFLMS macro to use */
   CIO.systimer_ptr = (struct trb *)talloc();    /* get system timer pointer */
   CIO.systimer_ptr->flags &= ~(T_ABSOLUTE);       /* want incremental timer */
   CIO.systimer_ptr->ipri = INTCLASS2;  /* set desired priority */
   CIO.systimer_ptr->func = (void (*)())cio_queofflms_func;  /* func to call */
   CIO.systimer_ptr->func_data = (ulong)(p_dds);   /* we'll need this data */

   /* update device driver controls */
   DISABLE_INTERRUPTS (saved_intr_level);
   dd_ctrl.p_dds[adap] = p_dds;
   dd_ctrl.num_dds++;
   ENABLE_INTERRUPTS (saved_intr_level);

   /* add dds to component dump table */
   cio_add_cdt ("DDS", (char *)p_dds, CIO.alloc_size);

   TRACE2 ("CFIe", (ulong)0); /* config_init end */
   return (0);
} /* end config_init */

/*
 * NAME:     config_term
 *
 * FUNCTION: process tokconfig entry with cmd of TERM
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *
 */
static int config_term (
   int adap) /* adapter number */
{
   register dds_t *p_dds;
            int    saved_intr_level;

   TRACE2 ("CFTb", (ulong)adap); /* config_term begin */

   DISABLE_INTERRUPTS(saved_intr_level);

   p_dds = dd_ctrl.p_dds[adap];
   TRACE2 ("CFTd", (ulong)p_dds); /* config_term p_dds */

   if (p_dds == NULL) /* don't have dds to terminate */
   {
      ENABLE_INTERRUPTS(saved_intr_level);
      TRACE2 ("CFT1", (ulong)ENOENT); /* config_term end (no DDS) */
      return (ENOENT);
   }

   if (CIO.num_allocates != 0) /* adapter is in use */
   {
      ENABLE_INTERRUPTS(saved_intr_level);
      TRACE2 ("CFT2", (ulong)EBUSY); /* config_term end (DDS is busy) */
      return (EBUSY);
   }

   /* update states and counters */
   dd_ctrl.num_dds--;
   dd_ctrl.p_dds[adap] = NULL;
   ENABLE_INTERRUPTS(saved_intr_level);

   /* give back the system timer */
   tstop (CIO.systimer_ptr);
   tfree (CIO.systimer_ptr);

   /* delete dds from component dump table */
   cio_del_cdt ("DDS", (char *)p_dds, CIO.alloc_size);

   /* give back the dds memory */
   if (KMFREE (p_dds) != 0)
      TRACE2 ("CFT3",
         (ulong)p_dds); /* config_term ERROR (free p_dds failed) */

   TRACE2 ("CFTe", (ulong)0); /* config_term end */
   return (0);
} /* end config_term */

/*****************************************************************************/
/*
 * NAME:     config_qvpd
 *
 * FUNCTION: process tokconfig entry with cmd of QVPD
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *
 */
/*****************************************************************************/
static int config_qvpd (
   register dds_t      *p_dds, /* pointer to dds structure */
            struct uio *uiop)    /* pointer to uio structure */
{
   TRACE3 ("CFQb", (ulong)p_dds, (ulong)uiop); /* config_qvpd begin */

   if (p_dds == NULL) /* don't have dds */
   {
      TRACE2 ("CFQ1", (ulong)ENOENT); /* config_qvpd end (no DDS) */
      return (ENOENT);
   }


   /* move vital product data section of DDS to user space) */
   if (uiomove (&VPD, MIN(uiop->uio_resid, sizeof(VPD) ),
                UIO_READ, uiop))
   {
      TRACE2 ("CFQ3",
         (ulong)EFAULT); /* config_qvpd end (uiomove of VPD failed) */
      return (EFAULT);
   }

   TRACE2 ("CFQe", (ulong)0); /* config_qvpd end */
   return (0);
} /* end config_qvpd */

/*****************************************************************************/
/*
 * NAME:     tokconfig
 *
 * FUNCTION: tokconfig entry point from kernel
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *
 */
/*****************************************************************************/
int tokconfig (
   dev_t       devno,  /* major and minor number */
   int         cmd,    /* operation desired (INIT, TERM, QVPD) */
   struct uio *uiop)   /* pointer to uio structure */
{
   static int local_devswadd (dev_t devno);
   int adap, first_time=FALSE;
   int rc;

   TRACE4 ("CFGb", (ulong)devno, (ulong)cmd, (ulong)uiop); /* tokconfig begin*/

   adap = minor(devno);

   /* Pin code while we are in config */
   pincode(tokconfig);

      switch (cmd)
      {
         case CFG_INIT:
            /* first config INIT must initialize driver */
            if (!dd_ctrl.initialized)
            {
                first_time=TRUE;
		/*
		 * FUTURE FIX:
		 *	Check the following logic so as
		 *	to NOT continue with the CFG_INIT logic
		 *	following the if(!dd_ctrl.initialized)
		 *	statement.
		 */
               /* try to add ourselves to switch table */
               if (rc = local_devswadd(devno))
                  break;

               /* initialize component dump table */
               ciocdt.header._cdt_magic = DMP_MAGIC;
               strncpy (ciocdt.header._cdt_name, DD_NAME_STR,
                  sizeof(ciocdt.header._cdt_name));
               ciocdt.header._cdt_len = sizeof(ciocdt.header);

               /* add trace table to component dump table */
               cio_add_cdt ("TraceTbl", (char *)(&tracetable),
                  (int)sizeof(tracetable));

               dd_ctrl.initialized = TRUE;
            }

            /* try to add a DDS */
            /* possible errors are EEXIST, EINVAL, EFAULT, ENOMEM, and ENXIO */
            if ( ( (rc = config_init (devno, uiop)) != 0 ) && first_time )
            {
                /*
                 * NOTE:
                 *      We only want to undo when this is the
                 *      first time through.
                 *
                 */
                (void)devswdel(devno);
                  /* delete trace table from component dump table */
                  cio_del_cdt ("TraceTbl", (char *)(&tracetable),
                     (int)sizeof(tracetable));

                  dd_ctrl.initialized = FALSE; /* allow re-initialization */
            }
            break;

         case CFG_TERM:
            /* we can't do this unless there has previously been a CFG_INIT */
            if (!dd_ctrl.initialized)
            {
               rc = EACCES;
               break;
            }

            /* there is at least one DDS, so try to delete the one requested */
            /* possible errors are ENOENT and EBUSY */
            if ( (rc = config_term (adap) ) != 0)
            {
                break;
            }
            /* if there are now no DDS's, then unconfigure us */
            /* note that to unconfigure, the caller must make one additional */
            /* call after terminating the last DDS */
            else if (dd_ctrl.num_dds == 0)
            {
               /* try to remove ourselves from switch table */
               /* possible errors are EEXIST, ENODEV, and EINVAL */
              /*   if (!(rc = devswdel(devno))) */

                rc = devswdel(devno);
               if ( rc == 0 )
               {
                  /* delete trace table from component dump table */
                  cio_del_cdt ("TraceTbl", (char *)(&tracetable),
                     (int)sizeof(tracetable));

                  dd_ctrl.initialized = FALSE; /* allow re-initialization */

                  rc = 0;
               }
               break;
            }

            break;

         case CFG_QVPD:
            /* we can't do this unless there has been a CFG_INIT */
            if (!dd_ctrl.initialized)
            {
               rc = EACCES;
               break;
            }

            /* try to move vpd */
            /* possible errors are ENOENT, EINVAL, and EFAULT */
            rc = config_qvpd (dd_ctrl.p_dds[adap], uiop);
            break;

         default:
            rc = EINVAL;
      } /* end switch (cmd) */

   /* Unpin code before finishing config. */
   unpincode(tokconfig);

   TRACE2 ("CFGe", (ulong)rc); /* tokconfig end */
   return (rc);
} /* end tokconfig */

/*****************************************************************************/
/*
 * NAME:     local_devswadd
 *
 * FUNCTION: add ourselves to the device switch table (once for device driver)
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno as returned by devswadd
 *
 */
/*****************************************************************************/
static int local_devswadd (
   dev_t devno)             /* major and minor number */
{
   extern int nodev();

   struct devsw ciodevsw;
   int          rc;

   TRACE2 ("DSAb", (ulong)devno); /* local_devswadd begin */

   /* define entry points */
   ciodevsw.d_open     = (int(*)())tokopen;
   ciodevsw.d_close    = (int(*)())tokclose;
   ciodevsw.d_read     = (int(*)())tokread;
   ciodevsw.d_write    = (int(*)())tokwrite;
   ciodevsw.d_ioctl    = (int(*)())tokioctl;
   ciodevsw.d_strategy = nodev;
   ciodevsw.d_ttys     = (struct tty *)NULL;
   ciodevsw.d_select   = (int(*)())tokselect;
   ciodevsw.d_config   = (int(*)())tokconfig;
   ciodevsw.d_print    = nodev;
   ciodevsw.d_dump     = nodev;
   ciodevsw.d_mpx      = (int(*)())tokmpx;
   ciodevsw.d_revoke   = nodev;
   ciodevsw.d_dsdptr   = NULL;
   ciodevsw.d_selptr   = NULL;
   ciodevsw.d_opts     = 0;

   rc = devswadd(devno, &ciodevsw);

   TRACE2 ("DSAe", (ulong)rc); /* local_devswadd end */
   return (rc);
} /* end local_devswadd */

/*
 * initdds - perform any device-specific initialization of the dds
 */
int
initdds (dds_t *p_dds)
{
    int         i, rc=0;

   TRACE2 ("indB", (ulong)p_dds);
   p_dds->wrk.group_address   = 0;  /* init group and functional */
   p_dds->wrk.group_addr_chan = 0;  /* address variables */
   p_dds->wrk.funct_address   = 0;
                                        /*
                                        *  Set flag to indicate
                                        *  that Ring Information
                                        *  is not available yet.
                                        */
   p_dds->wrk.ri_avail = FALSE;

   p_dds->wrk.mask_int = FALSE;
   p_dds->wrk.pio_block = FALSE;
   p_dds->wrk.scb_lock = LOCK_AVAIL;
   p_dds->wrk.group_wait = FALSE;
   p_dds->wrk.group_event = EVENT_NULL;
   p_dds->wrk.funct_wait = FALSE;
   p_dds->wrk.funct_event = EVENT_NULL;
   p_dds->wrk.ring_info_wait = FALSE;
   p_dds->wrk.ring_info_event = EVENT_NULL;

   p_dds->wrk.tx_intr_in = 0;
   p_dds->wrk.tx_intr_out = 0;
   p_dds->wrk.tx_noop_cnt = 0;
   p_dds->wrk.tx_proc_limit  = 1;
   p_dds->wrk.tx_owe.int_reason = 0;
   p_dds->wrk.tx_owe.cmd = 0;
   p_dds->wrk.tx_owe.stat0 = NULL;
   p_dds->wrk.tx_owe.stat1 = NULL;
   p_dds->wrk.tx_owe.stat2 = NULL;

   p_dds->wrk.recv_intr_in = 0;
   p_dds->wrk.recv_intr_out = 0;
   p_dds->wrk.recv_noop_cnt = 0;
   p_dds->wrk.recv_owe.stat0 = NULL;
   p_dds->wrk.recv_owe.stat1 = NULL;
   p_dds->wrk.recv_owe.stat2 = NULL;

        /*
        *  Set state machine to note that there
        *  the ACA is not available.
        */
   p_dds->wrk.adap_state = DEAD_STATE;
   p_dds->wrk.limbo = PARADISE;
   p_dds->wrk.rr_entry = 0;
   p_dds->wrk.limcycle = 0;
   p_dds->wrk.entropy = 0;
   p_dds->wrk.soft_chaos = 0;
   p_dds->wrk.hard_chaos = 0;


   /* copy the bus io address and bus id, this is to localize memeory
    * access in interrupt handler
    */
   WRK.bus_id = DDI.bus_id;
   WRK.bus_io_addr = DDI.bus_io_addr;

   rc = cfg_pos_regs(p_dds);

   /*
   *   Get the vital product data from the adapter.
   */
   tokdsgetvpd(p_dds);

   (void)cfg_adap_parms(p_dds);

   TRACE1 ("indE");
   return(rc);
}

/*
* NAME: cfg_pos_regs
*
* FUNCTION:
*
*      Configures the POS registers for the Token-Ring adapter.
*
* EXECUTION ENVIRONMENT:
*
*      This routine executes on the process thread.
*
* NOTES:
*
* RECOVERY OPERATION:
*      None.
*
* DATA STRUCTURES:
*
* RETURNS:
*      None.
*/

int
cfg_pos_regs(dds_t *p_dds)

{  /* begin function cfg_pos_regs() */

    unsigned short pio_addr;   /* PIO address */
    unsigned short pos_intr;   /* POS interrupt level setting */
    unsigned char  adidl, adidh; /* low and high cardid bytes */
    unsigned char  adtp4, adtp5; /* temp POS4 and POS5 values */
    int         pio_attachment;


        pio_attachment = attach_iocc( p_dds );

            adidl = pio_read( p_dds, POS_REG_0 );
            adidh = pio_read( p_dds, POS_REG_1 );
            if ((adidl == TOKEN_L) && (adidh == TOKEN_H))
            { 	/*  we found a token adapter */

                p_dds->wrk.cfg_pos[0] = adidl;
                p_dds->wrk.cfg_pos[1] = adidh;

                /*
                 * set PIO addr, interrupt level, ring speed and card
                 * enable by setting value in POS2
                 * +-----+-----+-----+-----+-----+-----+-----+-----+
                 * |user |      PIO        |      INTR       |card |
                 * |def. |      addr       |      level      |enabl|
                 * +-----+-----+-----+-----+-----+-----+-----+-----+
                 *    7     6     5     4     3     2     1     0
                 */
               switch( (unsigned int)p_dds->ddi.bus_io_addr)
               {   /* get the PIO registers base address */
                   case PIO_86A0:
                       pio_addr = 0;
                       break;
                   case PIO_96A0:
                       pio_addr = 1;
                       break;
                   case PIO_A6A0:
                       pio_addr = 2;
                       break;
                   case PIO_B6A0:
                       pio_addr = 3;
                       break;
                   case PIO_C6A0:
                       pio_addr = 4;
                       break;
                   case PIO_D6A0:
                       pio_addr = 5;
                       break;
                   case PIO_E6A0:
                       pio_addr = 6;
                       break;
                   case PIO_F6A0:
                       pio_addr = 7;
                       break;
               }   /* end switch */


               switch(p_dds->ddi.bus_int_lvl)
               {   /* get the bus interrupt level */
                   case POS_INT_9:             /*  Interrupt Level 9 */
                       pos_intr = 0;
                       break;
                   case POS_INT_3:             /*  Interrupt Level 3 */
                       pos_intr = 1;
                       break;
                   case POS_INT_4:             /*  Interrupt Level 4 */
                       pos_intr = 2;
                       break;
                   case POS_INT_5:             /*  Interrupt Level 5 */
                       pos_intr = 3;
                       break;
                   case POS_INT_7:             /*  Interrupt Level 7 */
                       pos_intr = 4;
                       break;
                   case POS_INT_10:            /*  Interrupt Level 10 */
                       pos_intr = 5;
                       break;
                   case POS_INT_11:            /*  Interrupt Level 11 */
                       pos_intr = 6;
                       break;
                   case POS_INT_12:            /*  Interrupt Level 12 */
                       pos_intr = 7;
                       break;
               }   /* end switch */

		/*
		 * we turn off the card
		 * enable bit in pos reg 2.
		 * The card will be enabled on the
		 * first open to the device.
		 */

                pio_write( p_dds, POS_REG_2,
                          ((pio_addr << 4) |
                           ( (pos_intr << 1) & ~(CARD_ENABLE) ) ));

                p_dds->wrk.cfg_pos[2] = pio_read( p_dds, POS_REG_2 );

       /*
        *      For POS 3, we can set the Ring Speed (4/16 Mb), the
        *      Disable Early Token Release, and Include System
        *      in DMA Recovery.
        *
        *      Ring Speed:
        *              0 = 4Mb
        *              1 = 16Mb
        *      Disable Early Token Release:
        *              0 = ETR at 16Mb
        *              1 = no ETR
        *
        *      DMA Recovery
        *              0 = Do not participate
        *              1 = participate
        *
        * +-----+-----+-----+-----+-----+-----+-----+-----+
        * |          Reserved           |DMA  |Ring |Disab|
        * |                             |Rcvry|Speed|Etr  |
        * +-----+-----+-----+-----+-----+-----+-----+-----+
        *    7     6     5     4     3     2     1     0
        */


                pio_write( p_dds, POS_REG_6, 0 );
                pio_write( p_dds, POS_REG_7, 0 );

                pio_write( p_dds, POS_REG_3,
                        ( (DMA_RCVRY) | (p_dds->ddi.ring_speed << 1)
                          | (EARLY_TOKEN_REL) ) );

                p_dds->wrk.cfg_pos[3] = pio_read( p_dds, POS_REG_3);

           /*
            * set arbitration level, fairness enable, time to
            * free MC after preempt, MC Parity enable,
            * SFDBKRTN (1=Monitor SFDBKRTN)
            *  note: fairness is enabled if bit is 0
            *  +---------+-----+-----+-----+-----+-----+-----+-----+
            *  |Monitor  |Parit|MC fr|Fairn|     MC Bus Arbit-     |
            *  |SFDBKRTN |enabl|delay|disab|      ration level     |
            *  +---------+-----+-----+-----+-----+-----+-----+-----+
            *      7        6     5     4     3     2     1     0
            */


                adtp4 = ((0x0f & (p_dds->ddi.dma_lvl)) |
                        (SFDBKRTN | MC_PARITY_ON | MC_PREEMPT_TIME));

                pio_write( p_dds, POS_REG_4, adtp4 );
                p_dds->wrk.cfg_pos[4] = pio_read( p_dds, POS_REG_4);


               /*
               *   POS5:
               *  note: DMA arbitration is allowed if bit is 0
               * +-----+-----+-----+-----+-----+-----+-----+-----+
               * |Chan check |Strea|Block|   Reserved for        |
               * |& stat. bit|data |Arbit|      future use       |
               * +-----+-----+-----+-----+-----+-----+-----+-----+
               *    7     6     5     4     3     2     1     0
               *
               */

               adtp5 = (STREAM_DATA | ~(MC_ARBITRATION) );

                pio_write( p_dds, POS_REG_5, adtp5 );
                p_dds->wrk.cfg_pos[5] = pio_read( p_dds, POS_REG_5);
                p_dds->wrk.cfg_pos[6] = pio_read( p_dds, POS_REG_6);
                p_dds->wrk.cfg_pos[7] = pio_read( p_dds, POS_REG_7);
             }

        detach_iocc( p_dds, pio_attachment );
        return(0);
}  /* end cfg_pos_regs */
