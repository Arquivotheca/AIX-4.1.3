static char sccsid[] = "@(#)06	1.3  src/bos/kernext/c327/c327cutpn.c, sysxc327, bos411, 9430C411a 7/27/94 09:31:22";
/*
 * COMPONENT_NAME: (SYSXC327) c327 cut mode pinned functions
 *
 * FUNCTIONS:    c327cutProcessInterrupt(), c327cut_connect(),
 *               c327cut_timeout(), c327cutioctl()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
** cut (em78) mode device driver subroutines called from c327dd
*/
#   include <sys/types.h>
#   include <sys/devinfo.h>
#   include <sys/errno.h>
#   include <sys/intr.h>
#   include <sys/ioctl.h>
#   include <sys/param.h>
#   include <sys/sleep.h>
#   include <sys/io3270.h>
#   include "c327dd.h"
#ifdef _POWER_MP
#include <sys/lock_def.h> 
extern Simple_lock c327_intr_lock;
#endif

/*PAGE*/
/*
 * NAME: c327cut_connect()
 *
 * FUNCTION: connect used by both open and ioctl/reset
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

void c327cut_connect (DDS_DATA *dds_ptr, int seconds_to_wait)
{

    C327TRACE3 ("CUTC", dds_ptr, seconds_to_wait);
    c327ConnectWait (dds_ptr, seconds_to_wait);

    WRK.CUT_buffer_changed = FALSE;
    WRK.CUT_cursor_changed = FALSE;
    WRK.CUT_reset_command  = FALSE;
    WRK.CUT_key_out_busy   = FALSE;
    WRK.CUT_vis_snd        = 0;
    WRK.CUT_intr_stat      = 0;
    WRK.CUT_que_nextin     = 0;
    WRK.CUT_que_nextout    = 0;

    PUTC_BUSIO (dds_ptr, adapter_intr_stat_reg, 0x3F);
    PUTC_BUSIO (dds_ptr, adapter_term_id_reg, ~(0xE4));
    PUTC_BUSIO (dds_ptr, adapter_conn_ctrl_reg, CONN_CTRL_ENABLE_COAX);

    return;
}
/*PAGE*/
/*
 * NAME: c327cut_timeout
 *
 * FUNCTION:
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

void c327cut_timeout (DDS_DATA *dds_ptr)
{
    if (WRK.CUT_had_interrupt == FALSE)
    {  /* CU is dead or coax is unplugged */
       C327TRACE2 ("CUTT", dds_ptr);
       if (WRK.CUT_read_sleeping)
          e_wakeup ( (void *)&WRK.CUT_read_sleep_event );
       WRK.CUT_timer_started = FALSE;
    }
    else   /* had an interrupt from the control unit in the last 30 seconds */
    {
       if (WRK.CUT_key_out_busy == TRUE)
       {  /* restart timer since more interrupts should occur */
          timeout((int (*)(void))c327cut_timeout, (int)dds_ptr, (HZ*30));
          WRK.CUT_had_interrupt = FALSE;
       }
       else  /* don't restart timer since no reason to get interrupts */
          WRK.CUT_timer_started = FALSE;
    }
}

/*PAGE*/
/*
 * NAME: c327cutProcessInterrupt()
 *
 * FUNCTION:
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

void c327cutProcessInterrupt(DDS_DATA *dds_ptr)
{
    register unsigned char temp_intr_stat;
    int                    saved_intr_level;
    boolean                do_wakeup;

    DISABLE_INTERRUPTS(saved_intr_level);

    temp_intr_stat    = WRK.CUT_intr_stat;
    WRK.CUT_intr_stat = 0;

    RESTORE_INTERRUPTS(saved_intr_level);

    WRK.CUT_had_interrupt = TRUE;                 /* say CU is still active */
                                                  /* for timeout routine */

    do_wakeup = FALSE;

    if (temp_intr_stat & CUT_VISOUND)             /* if visual/sound update */
       do_wakeup = TRUE;

    if (temp_intr_stat & CUT_BBMODCOM)             /* buffer modify complete */
    {
       WRK.CUT_buffer_changed = TRUE;
       do_wakeup = TRUE;
    }
    else
    {
       if (temp_intr_stat & CUT_LDIOADR)         /* load i/o address decoded */
       {
          if (!(temp_intr_stat & CUT_BBMOD))   /* and no buf mod in progress */
          {
             WRK.CUT_cursor_changed = TRUE;
             do_wakeup = TRUE;
          }
       }
    }

    if (temp_intr_stat & CUT_RESETOP)            /* reset command, */
    {
       c327SimulateReset (dds_ptr);
       c327cut_connect (dds_ptr, 1);             /* changes flags just set ! */
       WRK.CUT_reset_command = TRUE;
       do_wakeup = TRUE;
    }

    if (do_wakeup)
    {
       if (WRK.CUT_read_sleeping)
       e_wakeup ( (void *)&WRK.CUT_read_sleep_event );
    }
}
/*PAGE*/
/*
 * NAME: c327cutioctl()
 *
 * FUNCTION:
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

int c327cutioctl(DDS_DATA *dds_ptr, int cmd, caddr_t arg)
{
    short          old_mask;
    int            saved_intr_level;
    int            temp_int;
    int            temp_ndx;
    int            rc;

    C327TRACE2 ("CIOC", cmd);
    rc=0;

    if ( (cmd & 0xFF00) == EMKEY )                     /* handle keystrokes */
    {
       if (WRK.CUT_timer_started == FALSE)
       {
          timeout((int (*)(void))c327cut_timeout, (int)dds_ptr, (HZ*30));
          WRK.CUT_had_interrupt = FALSE;
          WRK.CUT_timer_started = TRUE;
       }

       DISABLE_INTERRUPTS(saved_intr_level);

       if (WRK.CUT_key_out_busy == FALSE)
       {
          /* any previous keystroke has been ack'ed */
          PUTC_BUSIO (dds_ptr, adapter_scan_code_reg, ((~cmd) & 0xFF));

          PUTC_BUSIO (dds_ptr, adapter_conn_ctrl_reg, (0x08 |
                      GETC_BUSIO (dds_ptr, adapter_conn_ctrl_reg)));

          WRK.CUT_key_out_busy = TRUE;
       }
       else
       {
          /* previous keystroke not ack'ed -- check que space*/
          temp_ndx = WRK.CUT_que_nextin + 1;

          if (temp_ndx >= CUT_QUE_SIZE)
             temp_ndx = 0;

          if (temp_ndx == WRK.CUT_que_nextout)
          {
             rc = EBUSY;                                  /* no room in que */
          }
          else
          {
             /* que this keystoke and let interrupt handler output it */
             WRK.CUT_que[WRK.CUT_que_nextin++] = (~cmd) & 0xFF;

             if (WRK.CUT_que_nextin >= CUT_QUE_SIZE)
                WRK.CUT_que_nextin = 0;
          }
       }
       RESTORE_INTERRUPTS(saved_intr_level);
       return(rc);
    }
    switch (cmd)                   /* handle special CUT mode ioctl commands */
    {
       case IOCINFO:
          {
             struct devinfo devinfo;
             devinfo.devtype = DD_EM78;
             devinfo.flags = 0;

             if (copyout((void *)&devinfo, (void *)arg, sizeof(devinfo)) )
                rc = EINVAL;
          }
          break;

       case EMSEEK:
          WRK.seek_displacement = (int)arg;
          break;

       case EMWAIT:
          WRK.CUT_read_without_wait = FALSE;
          break;

       case EMNWAIT:
          WRK.CUT_read_without_wait = TRUE;
          break;

       case EMXPOR:
          c327Disconnect (dds_ptr);
          c327cut_connect (dds_ptr, 1);
          break;

       case EMIMASK:
          if (copyin ((void *)arg, (void *)&temp_int, sizeof(int)))
          {
             rc = EFAULT;
             break;
          }

          old_mask = WRK.CUT_intr_mask;           /* save old mask value */
          WRK.CUT_intr_mask = temp_int & 0xFFFF;  /* set new mask value */
          temp_int = old_mask;                    /* now send old value back */

          if (copyout ((void *)&temp_int, (void *)arg, sizeof(int)))
             rc = EFAULT;

          break;

       case EMCPOS:
          temp_int = (GETC_BUSIO (dds_ptr, adapter_msb_cur_reg) << 8) |
                     (GETC_BUSIO (dds_ptr, adapter_lsb_cur_reg)     );

          if (copyout ((void *)&temp_int, (void *)arg, sizeof(int)))
             rc = EFAULT;

          break;

       case EMVISND:
          DISABLE_INTERRUPTS(saved_intr_level);
          temp_int = WRK.CUT_vis_snd;
          WRK.CUT_vis_snd = 0;
          RESTORE_INTERRUPTS(saved_intr_level);

          if (copyout ((void *)&temp_int, (void *)arg, sizeof(int)))
             rc = EFAULT;

          break;

       default:
          rc = EINVAL;
          break;
    }                                     /* end switch on CUT mode commands */
    return(rc);
}
