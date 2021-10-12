static char sccsid[] = "@(#)07  1.14 src/bos/kernext/c327/c327npn.c, sysxc327, bos41J, 9524A_all 6/10/95 09:35:21";

/*
 * COMPONENT_NAME: (SYSXC327) c327 non-pinned device driver entry points
 *
 * FUNCTIONS: c327InitDDS(), c327_add_dds(), c327close(), c327config(),
 *            c327ioctl(), c327mpx(), c327open(), c327read(), c327select(),
 *            c327write()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/ddtrace.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/intr.h>
#include <sys/device.h>
#include <sys/param.h>
#include <sys/pin.h>
#include <sys/timer.h>
#include <sys/trchkid.h>
#include <sys/sleep.h>
#include <sys/types.h>
#include <sys/io3270.h>
#include "c327dd.h"
#include "dftnsSubr.h"
#include "tcaexterns.h"


#ifdef _POWER_MP
#include <sys/lock_alloc.h>
#include <sys/lock_def.h> 
extern Simple_lock c327_intr_lock;
#else
#include <sys/lockl.h>
#endif

extern DDS_CONTROL dev_control[];
extern TIMR_QUE timr_que;
extern INTR_QUE intr_que;
extern int c327_dev_major;
extern int  c327_intr_level;
extern int  c327_first_time;
extern struct bus_struct global_bus_id[];

#ifdef _POWER_MP
extern Complex_lock c327_lock;
#else
extern lock_t c327_lock;
#endif

extern struct intr   c327offlstruct;             /* off level structure */
extern struct intr   c327intrstruct1;             /* interrupt structure */
extern struct intr   c327intrstruct2;             /* interrupt structure */

#define FIRST_OPEN    1
/* copied from c327pn.c */
struct bus_struct {
   ulong bus_id;   /* microchannel bus identifier */
   short count;    /* Reference count for each bus. Equals number of adapters
                      configured for each bus */
   boolean intr_added;  /* i_init completed - defect 180148 */
   };

void dftnsProcessInterrupt(DDS_DATA *);
/*PAGE*/
/*
 * NAME: c327InitDDS()
 *
 * FUNCTION: initialize a DDS
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:)
 *
 *
 * (RECOVERY OPERATION:)
 *
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *
 */

void c327InitDDS (DDS_DATA *dds_ptr)
{
    int ndx;

    bzero((void *)&(DEV.cu_info.cu_array[0]), 
          (uint)sizeof(DEV.cu_info.cu_array));

    /* bzero (&(RAS), sizeof(DDS_RAS_SECTION))             Zero the DDS RAS  */

    bzero((void *)&(WRK), (uint)sizeof(DDS_WRK_SECTION));/* Zero the DDS WRK */

    for (ndx = 0; ndx < DEV.netid_table_entries; ndx++)
       bzero ((void *)&(TBL(ndx)), sizeof (NETID_TBL_ENTRY));/* 0 netid tbls */

    return;
}
/*PAGE*/
/*
 * NAME: c327_add_dds()
 *
 * FUNCTION: add and verify a dds
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 *
 * (RECOVERY OPERATION:)
 *
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *
 */

void c327_add_dds (c327_dds *ddi_ptr, int adapter_number)
{
    DDS_DATA *dds_ptr;
    int      saved_intr_level;  
    int      i, dds_size;

    dds_size = sizeof(DDS_HDR_SECTION) + sizeof(DDS_HDW_SECTION) +
               sizeof(DDS_DEV_SECTION) + sizeof(DDS_WRK_SECTION) +
               ( sizeof(NETID_TBL_ENTRY) * ddi_ptr->num_sessions);

    dds_ptr = (DDS_DATA *)xmalloc((uint)dds_size, (uint)3, pinned_heap);

    bzero((void *)dds_ptr, (uint)dds_size);

    /* copy stuff from config */
    HDR.minor_number         = adapter_number;
    HDW.bus_mem_beg          = ddi_ptr->bus_mem_beg;
    HDW.bus_mem_size         = ddi_ptr->bus_mem_size;
    HDW.io_port              = ddi_ptr->io_port;
    HDW.slot_number          = ddi_ptr->slot_number;
    HDW.PIO_error            = FALSE;

    DEV.buffer_size          = ddi_ptr->buffer_size;
    DEV.netid_table_entries  = ddi_ptr->num_sessions;

    for (i=0; i<sizeof(HDR.dev_name); ++i)
       HDR.dev_name[i] = ddi_ptr->dev_name[i];

    for (i=0; i<MAX_CULTA; ++i)
    {
       if (ddi_ptr->printer_addr[i] == -1)
          DEV.culta_prt[i] = (char)0xff;
       else
          DEV.culta_prt[i] = (char)(0x1f & (int)ddi_ptr->printer_addr[i]);
    }

    C327TRACE2("DP@0", dds_ptr);
    C327TRACE5("DP@1", DEV.culta_prt[0], DEV.culta_prt[1],
                       DEV.culta_prt[2], DEV.culta_prt[3]);
    C327TRACE5("DP@2", DEV.culta_prt[4], DEV.culta_prt[5],
                       DEV.culta_prt[6], DEV.culta_prt[7]);
    C327TRACE5("DDS1", HDW.bus_mem_beg, HDW.bus_mem_size,
                       HDW.io_port, HDW.slot_number);
    C327TRACE4("DDS2", DEV.netid_table_entries, DEV.buffer_size,
                       HDR.minor_number);

    DEV.dev_info.dev.machine_type_number[0] = 0xf6;
    DEV.dev_info.dev.machine_type_number[1] = 0xf0;
    DEV.dev_info.dev.machine_type_number[2] = 0xf0;
    DEV.dev_info.dev.machine_type_number[3] = 0xf0;

    DEV.dev_info.dev.customer_id            = 0xe1;

    DEV.dev_info.dev.model_number[0]        = 0xf0;
    DEV.dev_info.dev.model_number[1]        = 0xf0;
    DEV.dev_info.dev.model_number[2]        = 0xf0;

    DEV.dev_info.dev.serial_no[0]           = 0xf0;
    DEV.dev_info.dev.serial_no[1]           = 0xf0;
    DEV.dev_info.dev.serial_no[2]           = 0xf0;
    DEV.dev_info.dev.serial_no[3]           = 0xf0;
    DEV.dev_info.dev.serial_no[4]           = 0xf0;
    DEV.dev_info.dev.serial_no[5]           = 0xf0;
    DEV.dev_info.dev.serial_no[6]           = 0xf0;

    DEV.dev_info.dev.software_release_level[0] = 0xc1;
    DEV.dev_info.dev.software_release_level[1] = 0xc9;
    DEV.dev_info.dev.software_release_level[2] = 0xe7;
/*
 ** for (i=0; i<4; ++i)
 **    DEV.dev_info.dev.machine_type_number[i]
 **          = ddi_ptr->machine_type_number[i];
 **  DEV.dev_info.dev.customer_id
 **       = ddi_ptr->customer_id;
 **  for (i=0; i<3; ++i)
 **    DEV.dev_info.dev.model_number[i]
 **          = ddi_ptr->model_number[i];
 **  for (i=0; i<2; ++i)
 **    DEV.dev_info.dev.plant_manufactured[i]
 **          = ddi_ptr->plant_manufactured[i];
 **  for (i=0; i<7; ++i)
 **    DEV.dev_info.dev.serial_no[i]
 **          = ddi_ptr->serial_no[i];
 **  for (i=0; i<3; ++i)
 **    DEV.dev_info.dev.software_release_level[i]
 **          = ddi_ptr->software_release_level[i];
 **  for (i=0; i<16; ++i)
 **    DEV.dev_info.dev.EC_level[i]
 **          = ddi_ptr->ec_level[i];
 */
    DISABLE_INTERRUPTS(saved_intr_level);
    dev_control[adapter_number].dds_state = DDS_AVAILABLE;
    dev_control[adapter_number].dds_ptr   = dds_ptr;
    RESTORE_INTERRUPTS(saved_intr_level);

    return;

}                                               /* end c327_add_dds */
/*PAGE*/
/*
 * NAME: c327config()
 *
 * FUNCTION: init call from kernel for /dev/3270cn
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 *
 * (RECOVERY OPERATION:)
 *
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *
 */

int c327config(dev_t dev, int cmd, struct uio *uiop)
{
    int rc, adapter_number;
    int cfg_cnt, i;

    DDHKWD5 (HKWD_DD_C327DD, DD_ENTRY_CONFIG, 0, dev, cmd, 0, 0, 0);

    adapter_number = minor(dev);

    if (!c327_first_time)
    {
       for (i = cfg_cnt = 0; i < MAX_MINORS && cfg_cnt <= MAX_ADAPTERS; i++)
       {
          if (dev_control[i].dds_state != DDS_NOT_EXIST)
             cfg_cnt++;
       }
       if ((cfg_cnt > MAX_ADAPTERS) || (adapter_number >= MAX_MINORS))
       {
          DDHKWD1 (HKWD_DD_C327DD, DD_EXIT_CONFIG, ENXIO, (int)dev);
          return(ENXIO);
       }
    }
    else
       if (adapter_number >= MAX_MINORS)
       {
          DDHKWD1 (HKWD_DD_C327DD, DD_EXIT_CONFIG, ENXIO, (int)dev);
          return(ENXIO);
       }

    switch (cmd)
    {
       case CFG_INIT:                             /* config */
          rc = c327_cfg_init( dev, uiop );
          break;
       case CFG_TERM:
          rc = c327_cfg_term( dev );
          break;
       case CFG_QVPD:                            /* query vital product data */
          rc = EIO;                              /* (not available) */
          break;
       default:                                  /* all unrecognized comands */
          rc = EINVAL;                           /* (not available) */
    }
    DDHKWD1 (HKWD_DD_C327DD, DD_EXIT_CONFIG, rc, (int)dev);
    return( rc );
}
/*PAGE*/
/*
 * NAME: c327mpx()
 *
 * FUNCTION: mpx entry point for 3270 device driver
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 *
 * (RECOVERY OPERATION:)
 *
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *
 */

int c327mpx(dev_t devno, int *chanp, char *channame)
{
   int      rc, i, open_cnt, adapter_number;
   DDS_DATA *dds_ptr;
   boolean first_open;

#ifdef _POWER_MP
   lock_write(&c327_lock);
#else
   lockl(&c327_lock, LOCK_SHORT); 
#endif

   DDHKWD5 ( HKWD_DD_C327DD, DD_ENTRY_MPX, 0, devno, (int)*chanp,
             (int)*channame, 0, 0 );

   adapter_number =  minor(devno);

   if (adapter_number >= MAX_MINORS)
   {
      DDHKWD5 ( HKWD_DD_C327DD, DD_EXIT_MPX,
                ENXIO, devno, (int)*channame, (int)*chanp, 0, 0 );

#ifdef _POWER_MP
      lock_done(&c327_lock);
#else
      unlockl(&c327_lock);
#endif

      return(ENXIO);
   }

   if (dev_control[adapter_number].dds_state <= DDS_NOT_EXIST)
   {
      DDHKWD5 ( HKWD_DD_C327DD, DD_EXIT_MPX, ENXIO, devno, (int)*channame,
               (int)*chanp, 0, 0 );

#ifdef _POWER_MP
      lock_done(&c327_lock);
#else
      unlockl(&c327_lock);
#endif

      return(ENXIO);                        /* dds does not exist for device */
   }

   if ( channame == NULL )
   {  /* let whip deallocate the session (OK even if not whip session) */
      rc = tcampx (devno,chanp,channame);
      DDHKWD5 ( HKWD_DD_C327DD, DD_EXIT_MPX, rc, devno,
               (int)*channame, (int)*chanp, 0, 0);

#ifdef _POWER_MP
      lock_done(&c327_lock);
#else
      unlockl(&c327_lock);
#endif

      return(rc);
   }

   *chanp = (int)*channame;                     /* passes channame to open */

   dds_ptr = dev_control[adapter_number].dds_ptr;  /* make dds_ptr good     */

   if (HDW.PIO_error)                              /* permanent pio error */
   {
      DDHKWD5 ( HKWD_DD_C327DD, DD_EXIT_MPX, EIO, devno, (int)*channame,
               (int)*chanp, 0, 0 );

#ifdef _POWER_MP
      lock_done(&c327_lock);
#else
      unlockl(&c327_lock);
#endif

      return(EIO);
   }

/*
** check to see if all adapters are avail if they are then it's
** possible this will be the first open and code will need to be
** pinned...we'll check later after the open
*/
   for (i=0,first_open = TRUE; i < MAX_MINORS && first_open == TRUE; i++)
   {
      if ((dev_control[i].dds_state != DDS_AVAILABLE) &&
          (dev_control[i].dds_state != DDS_NOT_EXIST))
      {
         first_open = FALSE;
      }
   }
   
   rc = 0;

   if ( ( *channame == '\0') ||
        ( *channame == 'P' ) ||
        ( *channame == '0' ) )
   {
       /* open of dftns device */
       if ( (dev_control[adapter_number].dds_state != DDS_AVAILABLE)   &&
            (dev_control[adapter_number].dds_state != DDS_OPENED_DFTNS) )
       {
          rc = EBUSY;
       }
       else
       {
          /* have whip do its setup stuff */
          if( ( rc = tcampx (devno,chanp,channame)) != 0 )
             dev_control[adapter_number].dds_state = DDS_AVAILABLE;
          else
             dev_control[adapter_number].dds_state = DDS_OPENED_DFTNS;
       }
    }
    else if ( *channame  == 'D' )            /* check for diagnostic open */
    {
       if (dev_control[adapter_number].dds_state != DDS_AVAILABLE)
       {
          rc = EBUSY;
       }
       else
       {
          dev_control[adapter_number].dds_state = DDS_OPENED_DIAG;
       }
    }
    else if ( *channame  == 'C' )               /* check for CUT mode open */
    {
       if (dev_control[adapter_number].dds_state != DDS_AVAILABLE)
       {
          rc = EBUSY;
       }
       else
       {
          dev_control[adapter_number].dds_state = DDS_OPENED_CUT;
       }
    }
    else if ( *channame  == 'F' )        /* check for CUT file transfer open */
    {
       if (dev_control[adapter_number].dds_state != DDS_OPENED_CUT)
       {
          rc = EBUSY;
       }
       else
       {
          dev_control[adapter_number].dds_state = DDS_OPENED_CUTFT;
       }
    }
    else
    {
       rc = ENXIO;
    }
    if (rc)
    {
       DDHKWD5 ( HKWD_DD_C327DD, DD_EXIT_MPX, rc, devno, (int)*channame, 
                 (int)*chanp, 0, 0 );

#ifdef _POWER_MP
      lock_done(&c327_lock);
#else
      unlockl(&c327_lock);
#endif

       return(rc);
    }

 /*
 ** Pincode if it is the first open - start defect 180148 **
 */
    if (first_open)
    {
        for (i=open_cnt=0; i < MAX_MINORS && open_cnt < 2; i++)
        {
           if ((dev_control[i].dds_state != DDS_AVAILABLE) &&
               (dev_control[i].dds_state != DDS_NOT_EXIST))
           {
              open_cnt++;
           }
        }
        if (open_cnt == FIRST_OPEN)
        {
           rc = pincode((int (*)(void *))dftnsProcessInterrupt);
           C327TRACE3("pnmx",1, rc);

	   if (rc)
              return(rc);
        }
    }

    /* register interrupt handler on first open */
    if ((global_bus_id[0].count) && (global_bus_id[0].intr_added == FALSE))
       {
          rc = i_init(&c327intrstruct1);
	  if (rc)
          {
             if (open_cnt == FIRST_OPEN)
                (void) unpincode((int (*)(void *))dftnsProcessInterrupt);
             return(rc);
          }
          global_bus_id[0].intr_added = TRUE;
       }

    if ((global_bus_id[1].count) && (global_bus_id[1].intr_added == FALSE))
       {
          rc = i_init(&c327intrstruct2);
	  if (rc)
          {
             if (open_cnt == FIRST_OPEN)
                (void) unpincode((int (*)(void *))dftnsProcessInterrupt);
             return(rc);
          }
          global_bus_id[1].intr_added = TRUE;
       }
/* end defect 180148 */

 /*
 ** Enable interrupts from adapter card (may be already enabled) which was
 ** just opened.
 */
    PUTC_IOCC (dds_ptr, 2, 0x01);     /* turn on global enable bit */

    DDHKWD5 ( HKWD_DD_C327DD, DD_EXIT_MPX, rc, devno, (int)*channame,
              (int)*chanp, 0, 0 );

#ifdef _POWER_MP
      lock_done(&c327_lock);
#else
      unlockl(&c327_lock);
#endif

    return(rc);
}                                           /* end of mpx routine */
/*PAGE*/
/*
 * NAME: c327open()
 *
 * FUNCTION: open entry point for 3270 device driver
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 *
 * (RECOVERY OPERATION:)
 *
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *
 */

int c327open (dev_t dev, uint flag, int chan, int ext)
{
    int adapter_number, rc, xrc, i;
    DDS_DATA *dds_ptr;
    boolean all_closed;

#ifdef _POWER_MP
   lock_write(&c327_lock);
#else
   lockl(&c327_lock, LOCK_SHORT); 
#endif

    DDHKWD5 ( HKWD_DD_C327DD, DD_ENTRY_OPEN, 0, 
              dev, (int)flag, chan, ext, 0 );
    rc = 0;

    adapter_number =  minor(dev);

    if ( chan == -1 )
    {
       DDHKWD1 ( HKWD_DD_C327DD, DD_EXIT_OPEN, ENXIO, (int)dev );

#ifdef _POWER_MP
      lock_done(&c327_lock);
#else
      unlockl(&c327_lock);
#endif
       return(ENXIO);
    }

    if (dev_control[adapter_number].dds_state <= DDS_NOT_EXIST)
    {
       DDHKWD1 ( HKWD_DD_C327DD, DD_EXIT_OPEN, ENXIO, (int)dev );

#ifdef _POWER_MP
      lock_done(&c327_lock);
#else
      unlockl(&c327_lock);
#endif

       return(ENXIO);                       /* dds does not exist for device */
    }

    dds_ptr = dev_control[adapter_number].dds_ptr;  /* make dds_ptr good     */

    switch (dev_control[adapter_number].dds_state)
    {
       case DDS_OPENED_DFTNS:
          rc = tcaopen (dev, (int)flag, chan, dds_ptr);
          if (WRK.num_opens == 0)
             dev_control[adapter_number].dds_state = DDS_AVAILABLE;

    C327TRACE3("copn",1, rc);
          if (rc != SUCCESS) {
	     /*
             ** Check to see if the FIRST open failed.  If so, the interrupt 
             ** handler needs to be unpinned and cleared and the dft timer
             ** needs to be stopped.
             */

             /*
             ** See if the open failed and it was the FIRST open
             **/
             all_closed = TRUE;
             for (i=0; i < MAX_MINORS; i++) {
    C327TRACE4("copn",2, i, dev_control[i].dds_state);
                if ((dev_control[i].dds_state != DDS_AVAILABLE) &&
                (dev_control[i].dds_state != DDS_NOT_EXIST)) {
                      all_closed = FALSE;
                      break;
                }
             }

             /*
             ** unpincode and stop DFT timer if the FIRST open failed
             */
             if (all_closed)
             {
    C327TRACE3("copn",3, dftnsProcessInterrupt);
                untimeout((int (*)(void))dftnsTimer);

		/* start defect 180148 */
                if ((global_bus_id[0].count)&&(global_bus_id[0].intr_added == TRUE))
		{
     	           i_clear(&c327intrstruct1);
		   global_bus_id[0].intr_added = FALSE;
		}
                if ((global_bus_id[1].count)&&(global_bus_id[1].intr_added == TRUE))
		{
                   i_clear(&c327intrstruct2);
		   global_bus_id[1].intr_added = FALSE;
		}
		/* end defect 180148 */

                if ((xrc = unpincode((int (*)(void *))dftnsProcessInterrupt))
                != 0)
                    rc = xrc;
    C327TRACE3("upop",3, dftnsProcessInterrupt);
             }
          }
          break;
       case DDS_OPENED_CUT:
          /* this is all there is to do for open of CUT device */
          c327InitDDS (dds_ptr);

          WRK.CUT_read_without_wait = FALSE;

          WRK.CUT_read_sleeping = FALSE;

          WRK.CUT_read_sleep_event = EVENT_NULL;

          WRK.CUT_timer_started = FALSE;

          WRK.CUT_intr_mask = 0xFFFF;

          WRK.seek_displacement = 0;

          c327cut_connect (dds_ptr, 5);

          break;

       case DDS_OPENED_CUTFT:
          /* there's nothing to do for file transfer mode */
          break;

       case DDS_OPENED_DIAG:
          c327InitDDS (dds_ptr);
          break;

       default:
          rc = ENOMSG;
          break;

    } /* end switch */

    DDHKWD1 ( HKWD_DD_C327DD, DD_EXIT_OPEN, rc, (int)dev );

#ifdef _POWER_MP
      lock_done(&c327_lock);
#else
      unlockl(&c327_lock);
#endif

    return(rc);
}                                                           /* end c327open */
/*PAGE*/
/*
 * NAME: c327close()
 *
 * FUNCTION: close entry point for 3270 device driver
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 *
 * (RECOVERY OPERATION:)
 *
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *
 */

int c327close(dev_t dev, int chan)
{
    int adapter_number, rc, xrc, i;
    DDS_DATA *dds_ptr;
    boolean   last_close;

#ifdef _POWER_MP
   lock_write(&c327_lock);
#else
   lockl(&c327_lock, LOCK_SHORT); 
#endif

    DDHKWD5 ( HKWD_DD_C327DD, DD_ENTRY_CLOSE, 0, dev, chan, 0, 0, 0 );

    adapter_number =  minor(dev);
    rc             = 0;

    if (adapter_number >= MAX_MINORS)
    {
       DDHKWD1 ( HKWD_DD_C327DD, DD_EXIT_CLOSE, ENXIO, (int)dev );

#ifdef _POWER_MP
       lock_write(&c327_lock);
#else
       lockl(&c327_lock, LOCK_SHORT); 
#endif

       return(ENXIO);
    }

    if (dev_control[adapter_number].dds_state <= DDS_NOT_EXIST)
    {
       DDHKWD1 ( HKWD_DD_C327DD, DD_EXIT_CLOSE, ENXIO, (int)dev );

#ifdef _POWER_MP
       lock_done(&c327_lock);
#else
       unlockl(&c327_lock);
#endif

       return(ENXIO);                       /* dds does not exist for device */
    }

    dds_ptr = dev_control[adapter_number].dds_ptr;      /* make dds_ptr good */

    switch (dev_control[adapter_number].dds_state)
    {
       case DDS_OPENED_DFTNS:

          rc = tcaclose(dev,chan);

          if (WRK.num_opens == 0)
             dev_control[adapter_number].dds_state = DDS_AVAILABLE;
          break;

       case DDS_OPENED_CUT:
          /* this is all there is to do for close of CUT device */
          c327Disconnect (dds_ptr);

          if (WRK.CUT_timer_started)
             untimeout((int (*)(void))c327cut_timeout, (int)dds_ptr);

          dev_control[adapter_number].dds_state = DDS_AVAILABLE;

          break;

       case DDS_OPENED_CUTFT:
          dev_control[adapter_number].dds_state = DDS_OPENED_CUT;
          break;

       case DDS_OPENED_DIAG:
          c327Disconnect (dds_ptr);

          dev_control[adapter_number].dds_state = DDS_AVAILABLE;

          break;

       default:
          dev_control[adapter_number].dds_state = DDS_AVAILABLE;

          rc = ENOMSG;

          break;
    }                                                         /* end switch */
/*
** if adapter is not in use, stop interrupts from adapter
*/
    if (dev_control[adapter_number].dds_state == DDS_AVAILABLE)
       PUTC_IOCC (dds_ptr, 2, 0x00);     /* turn off global enable bit */

/*
** look for adapters in use
*/
    for (i=0,last_close = TRUE; i < MAX_MINORS && last_close == TRUE; i++)
    {
       if ((dev_control[i].dds_state != DDS_AVAILABLE) &&
           (dev_control[i].dds_state != DDS_NOT_EXIST))
       {
          last_close = FALSE;
       }
    }
/*
** unpincode and stop DFT timer if it is the last close
*/
    if (last_close)
    {
       untimeout((int (*)(void))dftnsTimer);

       /* start defect 180148 */
       if ((global_bus_id[0].count)&&(global_bus_id[0].intr_added == TRUE))
	{
           i_clear(&c327intrstruct1);
	   global_bus_id[0].intr_added = FALSE;
	}
        if ((global_bus_id[1].count)&&(global_bus_id[1].intr_added == TRUE))
	{
           i_clear(&c327intrstruct2);
	   global_bus_id[1].intr_added = FALSE;
	}
       /* end defect 180148 */
       
       if ((xrc = unpincode((int (*)(void *))dftnsProcessInterrupt)) != 0)
          rc = xrc;
           C327TRACE3("upcl",1, rc);
    }


    DDHKWD1 ( HKWD_DD_C327DD, DD_EXIT_CLOSE, rc, (int)dev );

#ifdef _POWER_MP
      lock_done(&c327_lock);
#else
      unlockl(&c327_lock);
#endif

    return(rc);
}                                                          /* end c327close */
/*PAGE*/
/*
 * NAME: c327read()
 *
 * FUNCTION: read entry point for 3270 device driver
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 *
 * (RECOVERY OPERATION:)
 *
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *
 */

int c327read(dev_t dev, struct uio *uiop, int chan, int ext)
{
    int      adapter_number,rc;
    DDS_DATA *dds_ptr;

    DDHKWD5 ( HKWD_DD_C327DD, DD_ENTRY_READ, 0, 
              dev, (int)uiop, chan, ext, 0 );

    rc = 0;

    adapter_number =  minor(dev);

    if (adapter_number >= MAX_MINORS) {
       rc = ENXIO;
    }
    else
    {
       dds_ptr = dev_control[adapter_number].dds_ptr;

       if (HDW.PIO_error)                          /* permanent pio error */
       {
          DDHKWD1 ( HKWD_DD_C327DD, DD_EXIT_READ, EIO, (int)dev );
          return(EIO);
       }

       switch   (dev_control[adapter_number].dds_state)
       {
          case DDS_OPENED_DFTNS:
             if ( WRK.restart_in_progress == TRUE )
             {
                C327TRACE1 ("WTRD");
                while ( WRK.restart_in_progress == TRUE )
                   delay ( HZ );        /* wait one second then check again */

                DDHKWD1 ( HKWD_DD_C327DD, DD_EXIT_READ, EIO, dev );
                return(EIO);
             }
             rc = tcaread (dev, uiop, chan, ext);
             break;

          case DDS_OPENED_CUT:
          case DDS_OPENED_CUTFT:
             rc = c327cutread (dds_ptr,uiop);
             break;

          case DDS_OPENED_DIAG:
             rc = c327diagread (dds_ptr,uiop);
             break;

          default:
             rc = ENOMSG;
             break;

       } /* end switch */

       if (HDW.PIO_error)                          /* permanent pio error */
          rc = EIO;

    }

    DDHKWD1 ( HKWD_DD_C327DD, DD_EXIT_READ, rc, (int)dev );

    return(rc);
}                                                       /* end c327read */
/*PAGE*/
/*
 * NAME: c327write()
 *
 * FUNCTION: write entry point for 3270 device driver
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 *
 * (RECOVERY OPERATION:)
 *
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *
 */

int c327write(dev_t dev, struct uio *uiop, int chan, int ext)
{
    int     rc, adapter_number;
    DDS_DATA *dds_ptr;

    DDHKWD5 ( HKWD_DD_C327DD, DD_ENTRY_WRITE, 0, 
              dev, (int)uiop, chan, ext, 0 );

    rc = 0;

    adapter_number =  minor(dev);

    if(adapter_number >= MAX_MINORS)
    {
       rc = ENXIO;
    }
    else
    {
       dds_ptr = dev_control[adapter_number].dds_ptr;

       if (HDW.PIO_error)                          /* permanent pio error */
       {
          DDHKWD1 ( HKWD_DD_C327DD, DD_EXIT_WRITE, EIO, (int)dev );
          return(EIO);
       }

       switch   (dev_control[adapter_number].dds_state)
       {
          case DDS_OPENED_DFTNS:
             if ( WRK.restart_in_progress == TRUE )
             {
                C327TRACE1 ("WTWR");
                while ( WRK.restart_in_progress == TRUE )
                   delay ( HZ );         /* wait one second then check again */

                DDHKWD1 ( HKWD_DD_C327DD, DD_EXIT_WRITE, EIO, dev );
                return(EIO);
             }
             rc = tcawrite (dev, uiop, chan, ext);
             break;

          case DDS_OPENED_CUT:
          case DDS_OPENED_CUTFT:
             rc = c327cutwrite (dds_ptr, uiop);
             break;

          case DDS_OPENED_DIAG:
             rc = c327diagwrite (dds_ptr, uiop);
             break;

          default:
             rc = ENOMSG;
             break;

       } /* end switch */

       if (HDW.PIO_error)                          /* permanent pio error */
          rc = EIO;
    }
    DDHKWD1 ( HKWD_DD_C327DD, DD_EXIT_WRITE, rc, (int)dev );
    return(rc);

} /* end c327write */
/*PAGE*/
/*
 * NAME: c327select()
 *
 * FUNCTION: select entry point for 3270 device driver
 *           this routine will allways fail unless it
 *           is called from dft mode (whip)
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 *
 * (RECOVERY OPERATION:)
 *
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *
 */

int c327select(dev_t dev, ushort event, ushort *reventp, int chan)
{
    int      adapter_number;
    int      rc;
    DDS_DATA *dds_ptr;

    DDHKWD5 ( HKWD_DD_C327DD, DD_ENTRY_SELECT, 0, dev, (int)event, 
       (int)chan, 0, 0 );

    adapter_number =  minor(dev);

    if (adapter_number >= MAX_MINORS)
    {
       DDHKWD1 ( HKWD_DD_C327DD, DD_EXIT_SELECT, ENXIO, (int)dev );
       return(ENXIO);
    }

    if ( dev_control[adapter_number].dds_state == DDS_OPENED_DFTNS )
    {
       dds_ptr = dev_control[adapter_number].dds_ptr;
       if ( WRK.restart_in_progress == TRUE )
       {
          C327TRACE1 ("WTSE");
          while ( WRK.restart_in_progress == TRUE )
             delay ( HZ );               /* wait one second then check again */

          DDHKWD1 ( HKWD_DD_C327DD, DD_EXIT_SELECT, EIO, dev );
          return(EIO);
       }
       rc = tcaselect( dev, event, reventp, chan );
       DDHKWD1 ( HKWD_DD_C327DD, DD_EXIT_SELECT, rc, (int)dev );
       return( rc );
    }
    else
    {
       DDHKWD1 ( HKWD_DD_C327DD, DD_EXIT_SELECT, EINVAL, (int)dev );
       return (EINVAL);
    }

}                                                         /* end c327select */
/*PAGE*/
/*
 * NAME: c327ioctl()
 *
 * FUNCTION: ioctl entry point for 3270 device driver
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 *
 * (RECOVERY OPERATION:)
 *
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *
 */

int c327ioctl(dev_t dev, int cmd, caddr_t arg, int flag, int chan)
{
    int      adapter_number,rc;
    DDS_DATA *dds_ptr;
    DDHKWD5 ( HKWD_DD_C327DD, DD_ENTRY_IOCTL, 0, dev, cmd, flag, chan, 0 );

    rc = 0;

    adapter_number =  minor(dev);

    if (adapter_number >= MAX_MINORS)
    {
       DDHKWD1 ( HKWD_DD_C327DD, DD_EXIT_IOCTL, ENXIO, (int)dev );
       return(ENXIO);
    }

    dds_ptr = dev_control[adapter_number].dds_ptr;      /* make dds_ptr good */

    switch (dev_control[adapter_number].dds_state)
    {
       case DDS_OPENED_DFTNS:
          if ( ( WRK.restart_in_progress == TRUE ) && ( cmd != WDC_INQ ) )
          {
             C327TRACE1 ("WTIC");
             while ( WRK.restart_in_progress == TRUE )
                delay ( HZ );          /* wait one second then check again */

             DDHKWD1 ( HKWD_DD_C327DD, DD_EXIT_IOCTL, EIO, dev );
             return(EIO);
          }
          rc = tcaioctl (dev, cmd, (int)arg, flag, chan);
          break;

       case DDS_OPENED_CUT:
       case DDS_OPENED_CUTFT:
          if (HDW.PIO_error)                          /* permanent pio error */
          {
             rc = EIO;
             break;
          }
          rc = c327cutioctl (dds_ptr, cmd, arg);
          if (HDW.PIO_error)                          /* permanent pio error */
             rc = EIO;
          break;

       case DDS_OPENED_DIAG:
          if (HDW.PIO_error)                          /* permanent pio error */
          {
             rc = EIO;
             break;
          }
          rc = c327diagioctl (dds_ptr, cmd, arg);
          if (HDW.PIO_error)                          /* permanent pio error */
             rc = EIO;
          break;

       default:
          rc = ENOMSG;
          break;

    }

    DDHKWD1 ( HKWD_DD_C327DD, DD_EXIT_IOCTL, rc, (int)dev );

    return(rc);
}                                                          /* end c327ioctl */
