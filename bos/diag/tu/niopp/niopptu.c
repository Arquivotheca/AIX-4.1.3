static char sccsid[] = "@(#)95  1.10.2.6  src/bos/diag/tu/niopp/niopptu.c, tu_niopp, bos41J, 9513A_all 3/28/95 17:15:20";
/*
 * COMPONENT_NAME:  tu_niopp
 *
 * FUNCTIONS:  exectu ()
 *             dd_ioctl ()
 *             exit_tu ()
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/lpio.h>
#include <unistd.h>
#include <diag/atu.h>
#include <diag/ttycb.h>
#include "diag/diag.h"
#include "diag/diago.h"

extern  int     errno;

#define FAILED          -1

/* Planar definitions */

#define SIO     95              /* 0x5f - SIO planar (release 1) */
#define SIO2    230             /* 0xe6 - SIO planar (release 2) */
#define SIO3    254             /* 0xfe - SIO planar (release 2) */
#define SIO7    217             /* 0xd9 - SIO planar (IOD)       */

/*      Test unit definitions   */

#define TEST10                  0x10
#define TEST20                  0x20
#define TEST30                  0x30
#define TEST40                  0x40
#define TEST50                  0x50
#define TEST60                  0x60

struct  lpdiag  *lp;

/*
 * NAME: exectu
 *
 * FUNCTIONS:  This module contains 6 test units to test the NIO Parallel Port
 *             adapter functions via ioctl function calls to the Line Printer
 *             Device Driver.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:    none
 */

int     exectu (fdes, p)        /* begin exectu */
int     fdes;
struct  tucb_data       *p;
{
        int     i = 0;
        int     err = 0;
        int     data = 0;
        int     cmpdata = 0;
        int     cdata = 0xe0;
        int     sdata = 0xc0;
        int     or567 = 0xe0;
        int     not13 = 0xf5;
        uchar   model_id = p->ttycb.adapter;
        long    c = p->header.loop;             /* set counter to loop value */
        int     ar[] =
        {
                0x37, 0x2f, 0x17, 0x0f, 0x77, 0x6f, 0x57, 0x4f,
                0xb7, 0xaf, 0x97, 0x8f, 0xf7, 0xef, 0xd7, 0xcf
        };

        /* Allocate memory for structure lp */
        lp = (struct lpdiag *) calloc (1, sizeof (struct lpdiag));

        /* Make sure at least one pass is executed */
        if (c == 0)
                c=1;                    /* if p->header.loop = 0 set c = 1 */
        while (c > 0)
        {
                switch (p->header.tu)
                {
                case TEST10:
                case TEST20:
                        while (data < 256)
                        {
                                lp->cmd = LP_W_DATA;    /* write data        */
                                lp->value = data;
                                if ((err = dd_ioctl (fdes, p->header.tu)) != 0)
                                        break;
                                lp->cmd = LP_R_DATA;    /* read data         */
                                if ((err = dd_ioctl (fdes, p->header.tu)) != 0)
                                        break;
                                if (lp->value != data)
                                {
                                        err = p->header.tu+2;
                                        break;
                                }  /* endif */
                                data=data*2;
                                if (data == 0)
                                        data++;
                                if (data == 256)
                                        data = 255;
                                if (data == 510)
                                        data = 85;
                        }  /* end while */
                        data = 0;
                        if (p->header.tu == TEST10)
                                data = 85;
                        if (p->header.tu == TEST20 && model_id != SIO3)
                        {
                                lp->cmd = LP_R_CNTL;    /* read control */
                                if ((err = dd_ioctl (fdes, p->header.tu)) != 0)
                                        break;
                                cmpdata = or567;
                                cmpdata &= not13;
                                if (model_id == SIO)
                                        lp->value &= not13;
                                else
                                        lp->value &= cdata;
                                if (lp->value != cmpdata)
                                {
                                        err = p->header.tu+3;
                                        break;
                                }  /* endif */
                        }
                        while (data < 256 && err == 0)
                        {
                                lp->cmd = LP_W_CNTL;    /* write control     */
                                lp->value = data;
                                if ((err = dd_ioctl (fdes, p->header.tu)) != 0)
                                        break;
                                if (p->header.tu == TEST20)
                                {
                                        lp->cmd = LP_R_CNTL; /* read control */
                                        err = dd_ioctl (fdes, p->header.tu);
                                        if (err != 0)
                                                break;
                                        if (model_id == SIO3)
                                                cmpdata = data | sdata;
                                        else
                                                cmpdata = data | or567;
                                        cmpdata &= not13;
                                        lp->value &= not13;
                                        if (lp->value != cmpdata)
                                        {
                                                err = p->header.tu+3;
                                                break;
                                        }  /* endif */
                                        data=data*2;
                                        if (data == 0)   /* skip strobe high */
                                                data += 2;
                                        if (data == 256) /* 0xfe, strobe low */
                                                data = 254;
                                        if (data == 508) /* 0x54, strobe low */
                                                data = 84;
                                        if (data == 0xa8)
                                                data = 0xaa;
                                }  /* endif */
                                if (data == 85)
                                        data = 256;
                        }  /* end while */
                        /* skip this section if there was an */
                        /* error in the previous while loop, if we are */
                        /* running TU10, or if we are testing a SuperI/O */
                        /* chip */
                        if (err != 0 || p->header.tu == TEST10 ||
                                model_id == SIO3 || model_id == SIO7 ||
                                model_id == SIO10)
                        {
                                break;
                        }  /* endif */
                        data = 0xbd;
                        cdata = 0xe4;
                        lp->cmd = LP_W_DATA;    /* write data                */
                        lp->value = data;
                        if ((err = dd_ioctl (fdes, p->header.tu)) != 0)
                                break;
                        lp->cmd = LP_W_CNTL;    /* write control             */
                        lp->value = cdata;
                        if ((err = dd_ioctl (fdes, p->header.tu)) != 0)
                                break;
                        lp->cmd = LP_R_DATA;    /* read data                 */
                        if ((err = dd_ioctl (fdes, p->header.tu)) != 0)
                                break;
                        if (lp->value != data)
                        {
                                err = p->header.tu+4;
                                break;
                        }  /* endif */
                        lp->cmd = LP_R_CNTL;    /* read control              */
                        if ((err = dd_ioctl (fdes, p->header.tu)) != 0)
                                break;
                        cmpdata = cdata | or567;
                        cmpdata &= not13;
                        lp->value &= not13;
                        if (model_id == SIO && p->header.tu == TEST10)
                                cmpdata = or567;
                        if (lp->value != cmpdata)
                        {
                                err = p->header.tu+5;
                                break;
                        }  /* endif */
                        break;
                case TEST30:
                        data = 0x55;
                        lp->value = sdata;      /* set direction = write     */
                        for (i=1; i < 3; i++)
                        {
                                lp->cmd = LP_W_CNTL;    /* write control     */
                                if ((err = dd_ioctl (fdes, p->header.tu)) != 0)
                                        break;
                                lp->cmd = LP_W_DATA;
                                lp->value = data;
                                if ((err = dd_ioctl (fdes, p->header.tu)) != 0)
                                        break;
                                lp->cmd = LP_R_DATA;
                                if ((err = dd_ioctl (fdes, p->header.tu)) != 0)
                                        break;
                                if (i == 1)
                                {                       /* direction = write */
                                        if (lp->value != data)
                                        {
                                                err = p->header.tu+2;
                                                break;
                                        }  /* endif */
                                        lp->value = cdata;      /* set read */
                                }  /* endif */
                                else
                                {                       /* direction = read  */
                                        if (lp->value != 0)
                                        {
                                                err = p->header.tu+3;
                                                break;
                                        }  /* endif */
                                }  /* endelse */
                        }  /* endfor */
                        break;
                case TEST40:
                        cdata = 0x55;
                        for (i=1; i<3; i++)
                        {
                                lp->cmd = LP_W_CNTL;
                                lp->value = cdata;
                                if ((err = dd_ioctl (fdes, p->header.tu)) != 0)
                                        break;
                                lp->cmd = LP_R_CNTL;
                                if ((err = dd_ioctl (fdes, p->header.tu)) != 0)
                                        break;
                                if (model_id == SIO3)
                                        cdata |= sdata;
                                else
                                        cdata |= or567;
                                if (lp->value != cdata)
                                {
                                        err = p->header.tu+2;
                                        break;
                                }  /* endif */
                                cdata = 0xaa;
                        }  /* endfor */
                        break;
                case TEST50:
                        /*
                           read status register to assure that bit 2 is
                           up
                         */
                        lp->cmd = LP_R_STAT;
                        if ((err = dd_ioctl (fdes, p->header.tu)) != 0)
                               break;
                        for (i = 0; i < 16; i++)
                        {
                                lp->cmd = LP_W_DATA;    /* write data        */
                                lp->value = i;
                                if ((err = dd_ioctl (fdes, p->header.tu)) != 0)
                                        break;
                                lp->cmd = LP_W_CNTL;    /* write control     */
                                lp->value = i;
                                if ((err = dd_ioctl (fdes, p->header.tu)) != 0)
                                        break;
                                lp->cmd = LP_R_STAT;
                                if ((err = dd_ioctl (fdes, p->header.tu)) != 0)
                                        break;
                                if (lp->value != ar[i])
                                {
                                        err = p->header.tu+2;
                                        break;
                                }  /* endif */
                        }  /* endfor */
                        break;
                case TEST60:
                        break;
                        /* This test has been deleted since the Device Driver
                           no longer supports it, code is presently left here
                           to preserve it in order to provide replacement in
                           near future */

                default:                                /* invalid TU number */
                        err = p->header.tu;
                        break;
                }  /* endswitch */
                c--;
        }  /* endwhile */
        free (lp);
        return (err);
}  /* exectu end */

/*
 * NAME: dd_ioctl
 *
 * FUNCTIONS:  Executes the device driver function requested by the test unit.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  rc
 */

int     dd_ioctl (fd, tu)
int     fd, tu;
{
        int     rc = 0;

        if ((rc = ioctl (fd, LPDIAG, lp)) != 0)
        {
                if (errno == EIO)
                        return (rc = tu+1);
                else
                        return (rc = tu);
        }  /* endif */
        return (rc);
}  /* dd_ioctl end */
