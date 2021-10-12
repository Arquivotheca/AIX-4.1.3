static char sccsid[] = "@(#)85  1.3.1.4  src/bos/diag/tu/fddi/tu034.c, tu_fddi, bos411, 9428A410j 11/4/93 11:08:36";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: chck_vpd
 *              tu034
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

Function(s) Test Unit 034 - Read the VPD Data

Module Name :  tu034.c
SCCS ID     :  1.13

Current Date:  5/23/90, 11:17:53
Newest Delta:  2/27/90, 14:36:44

This test unit reads the VPD information from the FDDI adapter.

*****************************************************************************/

#include <stdio.h>
#include <sys/device.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/sysconfig.h>
#include <sys/watchdog.h>
#include <sys/xmem.h>
#include <sys/err_rec.h>
#include <sys/sysmacros.h>

#include <sys/comio.h>

#include "fdditst.h"
#include "diagddfddiuser.h"
#include "diagddfddidds.h"

#ifdef debugg
    extern detrace();
#endif


int
chck_vpd (fdes, tucb_ptr)
    int     fdes;
    TUTYPE *tucb_ptr;
{
    int     rc;
    ushort  cal_crc, rd_crc;
    unsigned long status;
    register uchar *buf;        /* area with data whose crc value is to be
                                 * computed */
    register int len;           /* number of bytes is data area */
    register uchar work_msb;
    register uchar work_lsb;
    register uchar value_msb;
    register uchar value_lsb;
    struct htx_data *htx_sp;
    struct cfg_dd cfg_dd_s;
    fddi_vpd_t fddi;

    /*
     * Set up a pointer to HTX data structure to increment counters in case
     * TU was invoked by hardware exerciser.
     */
    htx_sp = tucb_ptr->fddi_s.htx_sp;

#ifdef debugg
    detrace (0, "\n\tStart of VPD test.\n");
#endif

    cfg_dd_s.kmid = 0;
    cfg_dd_s.cmd = CFG_QVPD;
    cfg_dd_s.ddsptr = (char *) &fddi;
    cfg_dd_s.ddslen = sizeof (fddi);
    rc = get_devno (tucb_ptr,&cfg_dd_s.devno);
    if (rc != 0)
    {
#ifdef debugg
    {
    int maj, min;
    maj = major(cfg_dd_s.devno);
    min = minor(cfg_dd_s.devno);
    detrace (0, "major = %d    minor = %d\n", maj,min);
    }
#endif
        if (htx_sp != NULL)
            (htx_sp->bad_others)++;
        return (mktu_rc (tucb_ptr->header.tu, LOG_ERR, rc));
    }

    /*
     * Read VPD information via sysconfig command.
     */
    rc = sysconfig (SYS_CFGDD, &cfg_dd_s, sizeof (cfg_dd_s));
    if (rc == -1)
    {

#ifdef debugg
        perror ("sysconfig");
#endif
        if (htx_sp != NULL)
            (htx_sp->bad_others)++;
        return (mktu_rc (tucb_ptr->header.tu, LOG_ERR, RD_VPD_ERR));
    }

    status = fddi.status;

    switch (status)
    {
    case FDDI_VPD_NOT_READ:
#ifdef debugg
        detrace (0, "VPD_NOT_READ status returned.\n");
#endif
        if (htx_sp != NULL)
            (htx_sp->bad_others)++;
        return (mktu_rc (tucb_ptr->header.tu, LOG_ERR, VPD_NOT_RD));
        break;

    case FDDI_VPD_INVALID:
#ifdef debugg
        detrace (0, "INVALID_VPD status returned.\n");
#endif
        if (htx_sp != NULL)
            (htx_sp->bad_others)++;
        return (mktu_rc (tucb_ptr->header.tu, LOG_ERR, VPD_INVAL));
        break;

    case FDDI_VPD_VALID:
#ifdef debugg
        detrace (0, "\n\tVALID_VPD returned.\n\n");
#endif

        /*
         * Calculate the VPD CRC
         */
        buf = &fddi.vpd[7];
        len = fddi.l_vpd - 7;

        for (value_msb = 0xFF, value_lsb = 0xFF; len > 0; len--)
        {
            value_lsb ^= *buf++;
            value_lsb ^= (value_lsb << 4);

            work_msb = value_lsb >> 1;
            work_lsb = (value_lsb << 7) ^ value_lsb;

            work_lsb = (work_msb << 4) | (work_lsb >> 4);
            work_msb = ((work_msb >> 4) & 0x07) ^ value_lsb;

            value_lsb = work_lsb ^ value_msb;
            value_msb = work_msb;
        }

        cal_crc = ((ushort) value_msb << 8) | value_lsb;
        rd_crc = (fddi.vpd[5] << 8) | fddi.vpd[6];
#ifdef debugg
        detrace (0, "\n\tCalculated VPD CRC = %d\n",cal_crc);
        detrace (0, "\tBurned in VPD CRC = %d\n\n", rd_crc);
#endif
        if (cal_crc != rd_crc)
        {
#ifdef debugg
            detrace (0, "Calculated VPD CRC does not compare");
            detrace (0, "with read VPD CRC.\n");
#endif
            if (htx_sp != NULL)
                (htx_sp->bad_others)++;
            return (mktu_rc (tucb_ptr->header.tu, LOG_ERR, VPD_CRC_ERR));
        }
        else
        if (htx_sp != NULL)
                (htx_sp->good_others)++;
        break;

    default:
#ifdef debugg
        detrace (0, "Undefined VPD status return code.\n");
        detrace (0, "Returned status = %x\n", status);
#endif
        if (htx_sp != NULL)
            (htx_sp->bad_others)++;
        return (mktu_rc (tucb_ptr->header.tu, LOG_ERR, VPD_UNDEFINE));
        break;
    }

#ifdef debugg
    detrace (0, "\tEnd of VPD test.\n");
#endif
    return (0);
}


/*****************************************************************************

tu034

*****************************************************************************/

int
tu034 (fdes, tucb_ptr)
    int     fdes;
    TUTYPE *tucb_ptr;
{
    int     rc;

    rc = chck_vpd (fdes, tucb_ptr);

    return (rc);
}
