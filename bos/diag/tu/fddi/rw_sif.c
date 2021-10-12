static char sccsid[] = "@(#)49  1.2.1.3  src/bos/diag/tu/fddi/rw_sif.c, tu_fddi, bos411, 9428A410j 11/4/93 11:06:23";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: sif_addr
 *              sif_wr
 *              sif_rd
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*****************************************************************************

Function(s) Read/Write SIF Register - All SIF registers are accessed through
the machine device driver.

Module Name :  rw_sif.c
SCCS ID     :  1.8

Current Date:  5/23/90, 11:17:47
Newest Delta:  1/19/90, 16:26:02

*****************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include "fdditst.h"
#include <sys/mdio.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/cfgodm.h>


#ifdef debugg
extern void detrace();
#endif

/*****************************************************************************

sif_addr - Returns the address of the sif registers.  This address is needed
by the machine device driver.

*****************************************************************************/

int sif_addr(address,tucb_ptr)
ulong *address;
TUTYPE *tucb_ptr;
{
        struct CuAt cusatt;             /* customized device attribute */
        struct PdAt preatt;             /* predefined device attribute */
        char    bufp[128];              /* search string and filename buffer */
        int     rc;
        char    addr[256];
        char    dev[80];
            int bus_io_addr;

        rc = get_port(tucb_ptr,dev);

        /*
         * Check to see if FDDI device found from slot number.
         */
        if (rc == -1)
          {
#ifdef debugg
            detrace(0,"slot value = %c\n", tucb_ptr->slot);
            detrace(0,"device value = %s\n",dev);
#endif
            return(INVALID_SLOT);
          }

    /* start up odm */
    if (odm_initialize () == -1)
    {
        /* initialization failed */
#ifdef debugg
        detrace (0, "odm: odm_initialize() failed\n");
#endif
        return (ODMINIT_ERR);
    }

    /*
     * Get child's Customized Object
     */
    sprintf (bufp, "attribute = bus_io_addr and name = %s", dev);

    rc = (int) odm_get_first (CuAt_CLASS, bufp, &cusatt);
    if (rc == -1)
    {
#ifdef debugg
        detrace (0, "odm1: ODM fail on CuAt %s\n", bufp);
#endif
        return (ODMGET_ERR);
    }
    else
    if (rc == 0)
    {
        sprintf (bufp, "uniquetype = adapter/mca/fddi and attribute = bus_io_addr");
        rc = (int) odm_get_first (PdAt_CLASS, bufp, &preatt);
        if (rc == -1)
        {
#ifdef debugg
            detrace (0, "odm1: ODM fail on PdAt %s\n", bufp);
#endif
            return (ODMGET_ERR);
        }
        else
        if (rc == 0)
        {
            return (NOATTR_ERR);
        }
        else
        {
            strcpy (addr, preatt.deflt);
        }
    }
    else
    {
        strcpy (addr, cusatt.value);
    }

    odm_terminate ();

#ifdef debugg
    detrace (0, "dev = %s\n", dev);
#endif

        sscanf(addr,"%x",&bus_io_addr);

#ifdef debugg
        detrace (0,"bus_io_addr = %x\n",bus_io_addr);
#endif

        *address = bus_io_addr;
        return(0);

 }

/*****************************************************************************

sif_wr

*****************************************************************************/

int sif_wr (sif_addr, sif_val, status, tucb_ptr)
   ulong         sif_addr;
   ushort        sif_val;
   int          *status;
   TUTYPE *tucb_ptr;
   {
        int rc;
        int bytes;
        union u_data{
                uchar sif_data[3];
                ushort sif_val;
        } data;
        uchar temp;
        struct htx_data *htx_sp;
        MACH_DD_IO iob;

        /*
         * set up a pointer to HTX data structure to
         * increment counters in case tu was invoked by
         * hardware exerciser.
         */
        htx_sp = tucb_ptr->fddi_s.htx_sp;

        /*
         * fill in union structure with input value.  null terminate
         * character string
         */
        data.sif_val = sif_val;
        data.sif_data[2] = '\0';

        /*
         * data must be byte swapped for intel processor
         */
        temp = data.sif_data[0];
        data.sif_data[0] = data.sif_data[1];
        data.sif_data[1] = temp;

        /*
         * Aug 9, 1991.  The machine device driver will perform either
         *               1 byte transfers or 4 byte transfers.  For the
         *               the purposes of writing to the SIF registers two
         *               1-byte transfers will be performed.  This should
         *               cause any problems.
         */

        bytes = 2;
        iob.md_data = data.sif_data;
        iob.md_incr = MV_BYTE;
        iob.md_size = (ulong) bytes;
        iob.md_addr = sif_addr;

        *status = 0;
        rc = ioctl(tucb_ptr->mdd_fd, MIOBUSPUT, &iob);

#ifdef debugg
       detrace(0,"\nsif_wr:");
       detrace(0,"\n sif.int val(in) is %x ",sif_val);
       detrace(0,"\n sif.status is %x ",rc);
       detrace(0,"\n sif.addr is %x\n",iob.md_addr);
#endif

        if (rc != 0)
        {
                if (htx_sp != NULL)
                        (htx_sp->bad_others)++;
                *status  = MDD_IOCTL_ERR;
                return(MDD_IOCTL_ERR);
        }

        if (htx_sp != NULL)
           (htx_sp->good_others)++;
        return(0);
   }

/*****************************************************************************

sif_rd

*****************************************************************************/

ushort sif_rd (sif_addr, status, tucb_ptr)
   ulong         sif_addr;
   int           *status;
   TUTYPE *tucb_ptr;
   {
        int rc;
        int bytes;
        union u_data{
                uchar sif_data[4];
                ushort sif_val;
        }data;
        uchar temp;
        struct htx_data *htx_sp;
        MACH_DD_IO iob;

#define SIF_REG_MASK 0x000F

        /*
         * set up a pointer to HTX data structure to
         * increment counters in case tu was invoked by
         * hardware exerciser.
         */
        htx_sp = tucb_ptr->fddi_s.htx_sp;

        /*
         * Aug 9, 1991.  The machine device driver will perform either
         *               1 byte transfers or 4 byte transfers.  The 1 byte
         *               transfer will not work for the HSR.  The entire
         *               register will be cleared when the first byte is
         *               read.  A 4 byte transfer will be performed when
         *               reading the HSR register and two 1-byte transfers
         *               will be performed when reading all other SIF
         *               registers.
         */


        if ((sif_addr & SIF_REG_MASK) != HSR_REG)
        {
                bytes = 2;
                iob.md_incr = MV_BYTE;
        }
        else
        {
                bytes = 1;
                iob.md_incr = MV_WORD;
        }

        iob.md_data = data.sif_data;
        iob.md_size = (ulong) bytes;
        iob.md_addr = sif_addr;

        /*
         * null terminate character string
         */
        data.sif_data[2] = '\0';

        *status = 0;
        rc = ioctl(tucb_ptr->mdd_fd, MIOBUSGET, &iob);

#ifdef debugg
       detrace(0,"\nsif_rd");
       detrace(0,"\n sif.val int is %x(s) ",data.sif_val);
       detrace(0,"\n sif.status is %x ",rc);
       detrace(0,"\n sif.addr is %x\n",iob.md_addr);
#endif

        if (rc)
        {
                if (htx_sp != NULL)
                        (htx_sp->bad_others)++;
                *status  = MDD_IOCTL_ERR;
                return(MDD_IOCTL_ERR);
        }

        /*
         * data must be byte swapped because of intel processor on adapter
         */
        temp = data.sif_data[0];
        data.sif_data[0] = data.sif_data[1];
        data.sif_data[1] = temp;

        if (htx_sp != NULL)
           (htx_sp->good_others)++;
        return(data.sif_val);
   }
