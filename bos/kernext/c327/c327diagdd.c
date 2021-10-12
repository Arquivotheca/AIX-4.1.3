static char sccsid[] = "@(#)16	1.17  src/bos/kernext/c327/c327diagdd.c, sysxc327, bos411, 9428A410j 6/16/90 03:08:48";

/*
 * COMPONENT_NAME: (SYSXC327) c327 device driver diag entry points
 *
 * FUNCTIONS:    c327_diag_conn_test(), c327_rcvcmd(),
 *               c327_rdwrtcmd(), c327diagioctl(),
 *               c327diagread(), c327diagwrite()
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
** 3270 diagnostic mode device driver subroutines called from c327dd
*/

#   include <sys/devinfo.h>
#   include <sys/errno.h>
#   include <sys/intr.h>
#   include <sys/param.h>
#   include <sys/sleep.h>
#   include <sys/types.h>
#   include <sys/io3270.h>
#   include "c327dd.h"

/*PAGE*/
/*
 * NAME: c327_rdwrtcmd()
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

void c327_rdwrtcmd (DDS_DATA *dds_ptr, boolean write, uchar cmd)
{
    unsigned char     save_io_ctrl_reg;

    save_io_ctrl_reg = GETC_BUSIO (dds_ptr, adapter_io_ctrl_reg);

    PUTC_BUSIO (dds_ptr, adapter_io_ctrl_reg, 0x02);

    if ((write == TRUE) && (cmd != 0x1C))     /* if write & not diag reset   */
       PUTC_BUSIO (dds_ptr, 0x09, 0x04);     /* wrap ctrl reg for write omds */
    else
       PUTC_BUSIO (dds_ptr, 0x09, 0x00);  /* wrap ctrl reg for diag rst & rd */

    delay ( (int)HZ/16 );

    PUTC_BUSIO (dds_ptr, adapter_io_ctrl_reg, 0x00);
    PUTC_BUSIO (dds_ptr, adapter_scan_code_reg, ~cmd);

    delay ( (int)HZ/16 );

    PUTC_BUSIO (dds_ptr, adapter_conn_ctrl_reg,
                (0x38 | GETC_BUSIO (dds_ptr, adapter_conn_ctrl_reg)));

    delay ( (int)HZ/16 );

    PUTC_BUSIO (dds_ptr, adapter_io_ctrl_reg, save_io_ctrl_reg);

    return;
}                                                 /* end c327_rdwrtcmd */

/*PAGE*/
/*
 * NAME: c327_rcvcmd()
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

char c327_rcvcmd (DDS_DATA *dds_ptr, int reg)
{
    unsigned char   save_io_ctrl_reg;
    unsigned char   rc;

    save_io_ctrl_reg = GETC_BUSIO (dds_ptr, adapter_io_ctrl_reg);

    PUTC_BUSIO (dds_ptr, adapter_io_ctrl_reg, 0x02);

    rc = GETC_BUSIO (dds_ptr, reg);                    /* recv=A  cmnd=B reg */

    PUTC_BUSIO (dds_ptr, adapter_io_ctrl_reg, save_io_ctrl_reg);

    return (rc);
}                                                      /* end c327_rcvcmd    */

/*PAGE*/
/*
 * NAME: c327_diag_conn_test()
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

int c327_diag_conn_test (DDS_DATA *dds_ptr, C327DIAG_DATA *diag_ptr)
{
    int            maximum_waits;
    int            ndx;
    int            rc;

    rc = 0;

    for (ndx = 0; ndx < 5; ndx++)            /* clean out anything pending   */
    {
       c327_rdwrtcmd (dds_ptr, (boolean)FALSE, (uchar)0x01); /* poll request */

       delay ( (int)HZ/16 );

       if (c327_rcvcmd (dds_ptr, 0x0A) == 0x00)
          break;

       c327_rdwrtcmd(dds_ptr, (boolean)FALSE, (uchar)0x11); /* poll ack */

       delay ( (int)HZ/16 );
    }

    c327_rdwrtcmd (dds_ptr, (boolean)FALSE, (uchar)0x02); /* reset adapter   */

    delay ( (int)HZ/16 );

    PUTC_BUSIO (dds_ptr, adapter_intr_stat_reg, 0x3F);
    PUTC_BUSIO (dds_ptr, adapter_term_id_reg, 0xFB);
    PUTC_BUSIO (dds_ptr, adapter_scan_code_reg, 0xCB);
    PUTC_BUSIO (dds_ptr, adapter_conn_ctrl_reg, CONN_CTRL_INT_INH);

    delay ( (int)HZ/16 );

    c327ConnectWait (dds_ptr, 5);

    WRK.diag_intr_stat2 = 0;

    PUTC_BUSIO (dds_ptr,  adapter_conn_ctrl_reg,
                (CONN_CTRL_ENABLE_COAX | CONN_CTRL_KEY_AVAIL) );

    maximum_waits = 100;                /* wait until connected or time out  */

    while ( ((WRK.diag_intr_stat2 & 0x67) == 0 ) && (maximum_waits-- > 0))
       delay ( (int)HZ/16  );           /* compute delay in clock ticks      */

    /* disconect again */
    PUTC_BUSIO (dds_ptr, adapter_conn_ctrl_reg, CONN_CTRL_INT_INH);

    c327Disconnect (dds_ptr);

    if ( (WRK.diag_intr_stat2 & 0x67) == 0x00 )
       rc = EIO;

    diag_ptr->recv_byte = WRK.diag_intr_stat2;          /* return result    */

    C327TRACE3 ("DCOE", rc, WRK.diag_intr_stat2);

    return(rc);
}                                                /* end c327_diag_conn_test */

/*PAGE*/
/*
 * NAME: c327diagread()
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

int c327diagread (DDS_DATA *dds_ptr, struct uio *uiop)
{
    if ( uiop->uio_resid > HDW.bus_mem_size)
       return(EFAULT);                                /* request too large   */
    else
       return( gets_busmem(dds_ptr, 0, uiop) );
}                                                    /* end c327diagread    */

/*PAGE*/
/*
 * NAME: c327diagwrite()
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

int c327diagwrite (DDS_DATA *dds_ptr, struct uio *uiop)
{
    if ( uiop->uio_resid > HDW.bus_mem_size)
       return(EFAULT);                                  /* request too large */
    else
       return( puts_busmem(dds_ptr, 0, uiop) );
}                                                      /* end c327diagwrite */

/*PAGE*/
/*
 * NAME: c327diagioctl()
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

int c327diagioctl (DDS_DATA *dds_ptr, int cmd, caddr_t arg)
{
    C327DIAG_DATA     diag_data;
    int               temp_reg;
    int               rc;

    rc = 0;

    if (copyin ((void *)arg, (void *)&diag_data, sizeof(C327DIAG_DATA)))
       return(EINVAL);

    diag_data.recv_byte = 0;

    switch (cmd)
    {
       case C327DIAG_TCA_SIZE:
          diag_data.address = HDW.bus_mem_size;
          break;

       case C327DIAG_TCA_READ:
          if (diag_data.address >= HDW.bus_mem_size)
          {
             rc = EINVAL;
             break;
          }
          diag_data.recv_byte = GETC_BUSMEM (dds_ptr, diag_data.address);
          break;

       case C327DIAG_TCA_TEST:
          if (diag_data.address >= HDW.bus_mem_size)
          {
             rc = EINVAL;
             break;
          }

          PUTC_BUSMEM (dds_ptr, diag_data.address, diag_data.send_byte);
          diag_data.recv_byte = GETC_BUSMEM (dds_ptr, diag_data.address);
          break;

       case C327DIAG_REG_READ:
          temp_reg = diag_data.address & 0x0F;
          diag_data.recv_byte = GETC_BUSIO (dds_ptr, temp_reg);
          break;

       case C327DIAG_REG_TEST:
          temp_reg = diag_data.address & 0x0F;
          PUTC_BUSIO (dds_ptr, temp_reg, diag_data.send_byte);
          diag_data.recv_byte = GETC_BUSIO (dds_ptr, temp_reg);
          break;

       case C327DIAG_SIM_RD_CMD:
          c327_rdwrtcmd (dds_ptr, (boolean)FALSE, diag_data.send_byte);
          break;

       case C327DIAG_SIM_WR_CMD:
          c327_rdwrtcmd (dds_ptr, (boolean)TRUE, diag_data.send_byte);
          break;

       case C327DIAG_GET_RCV_CMD:
          temp_reg = diag_data.address & 0x0F;
          diag_data.recv_byte = c327_rcvcmd (dds_ptr, temp_reg);
          break;

       case C327DIAG_I_STAT_RESET:
          WRK.diag_intr_stat = 0;
          break;

       case C327DIAG_I_STAT_READ:
          diag_data.recv_byte = WRK.diag_intr_stat;
          break;

       case C327DIAG_SIM_RESET:
          c327SimulateReset (dds_ptr);
          break;

       case C327DIAG_DISCONNECT:
          c327Disconnect (dds_ptr);
          break;

       case C327DIAG_CONN_TEST:
          rc = c327_diag_conn_test (dds_ptr, &diag_data);
          break;

       default:
          rc = EINVAL;
          break;
    }                                                    /* end switch       */

    if (copyout ((void *)&diag_data, (void *)arg, sizeof(C327DIAG_DATA)))
       rc = EINVAL;

    C327TRACE5 ("DIOE", rc, diag_data.address, diag_data.send_byte,
                diag_data.recv_byte);
                
    return(rc);
}
