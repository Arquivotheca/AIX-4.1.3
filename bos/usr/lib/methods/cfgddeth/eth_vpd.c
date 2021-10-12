static char sccsid[] = "@(#)20        1.1  src/bos/usr/lib/methods/cfgddeth/eth_vpd.c, diagddient, bos411, 9428A410j 5/25/94 08:09:42";
/*
 *   COMPONENT_NAME: diagddient
 *
 *   FUNCTIONS: get_eth_addr
 *              get_eth601_addr
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *   eth_vpd.c - Common Integrated Ethernet subroutines for reading VPD
 *
 */

#include <stdio.h>
#include <cf.h>
#include <fcntl.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/types.h>
#include <sys/iocc.h>
#include <sys/mdio.h>
#include <sys/iplcb.h>
#include <sys/xmem.h>
#include <sys/err_rec.h>
#include "cfgdebug.h"

#define VPD_LENGTH  256         /* max length of VPD that could be read */

/*
 *  FUNCTION: This subroutine will read the VPD data for the Integrated
 *            Ethernet on the Salmon or Stillwell based boxes.  Another
 *            subroutine will be called to parse out the Ethernet network
 *            address from the VPD string.
 */

int get_eth_addr(int fd, uchar_t *net_addr, int slot_num, int offset)
{
  MACH_DD_IO    mdd;
  int i;
  int rc;
  char    vpd[VPD_LENGTH+2];
  uchar   temp_pos[1];           /* storage for pos register value */

  rc = 0;

  for (i = 0; i <= VPD_LENGTH; i++) {
          /*
           *  put source address for the VPD read
           *  in POS 6, get it from POS 3.
           */

          /* write pos register 6 */
          /*  *pos6 = i + 1;      */

          mdd.md_size = 1;          /* build mdd record */
          mdd.md_incr = MV_BYTE;
          mdd.md_addr = POSREG(6,offset);

          /* pos 6 - offset for vpd */
          temp_pos[0] = i + 1;

          mdd.md_data = temp_pos;        /* addr of data */
          if (0 > ioctl(fd, MIOCCPUT, &mdd))
          {
              perror("[busquery]ioctl()");
              fprintf(stderr, "Attempting to read slot %c\n",
                  offset);
              return(E_DEVACCESS);
          }

          /* read pos register 3 */
          /*  VPD.vpd[ i ] = *pos3;  */

          mdd.md_size = 1;          /* build mdd record */
          mdd.md_incr = MV_BYTE;
          mdd.md_addr = POSREG(3,offset);

          /* pos 3 - read one byte of vpd */

          mdd.md_data = &vpd[i];        /* addr of data */
          if (0 > ioctl(fd, MIOCCGET, &mdd))
          {
              perror("[busquery]ioctl()");
              fprintf(stderr, "Attempting to read slot %c\n",
                  offset);
              return(E_DEVACCESS);
          }

  }

  DEBUG_1("get_eth_addr(): slot checked = %d\n",offset)

  /* now copy ethernet address from vpd array to address storage */
  rc = search_eth_addr(vpd, VPD_LENGTH, net_addr);

  return(rc);
}



/*
 *  FUNCTION: This subroutine will read the VPD data for the Integrated
 *            Ethernet on the 601 based boxes.  Another
 *            subroutine will be called to parse out the Ethernet network
 *            address from the VPD string.
 */

int get_eth601_addr(int fd, uchar_t *net_addr, int slot_num)
{
  MACH_DD_IO    mdd;
  IPL_DIRECTORY *iplcb_dir;     /* iplcb.h */
  int rc;
  char    *vpd;

  rc = 0;

   /* First obtain pointer to IPL ROS directory */
    iplcb_dir = (IPL_DIRECTORY *) malloc (sizeof(IPL_DIRECTORY));
    mdd.md_incr = MV_BYTE;
    mdd.md_addr = 128;
    mdd.md_data = (char *) iplcb_dir;
    mdd.md_size = sizeof(*iplcb_dir);
    if ( ioctl(fd, MIOIPLCB, &mdd) )
        rc = 5;
    else {
        /* Copy VPD info into vpd array */
        vpd = (char *) malloc (iplcb_dir->system_vpd_size);

        mdd.md_incr = MV_BYTE;
        mdd.md_addr = iplcb_dir->system_vpd_offset;
        mdd.md_data = (char *) vpd;
        mdd.md_size = iplcb_dir->system_vpd_size;
        if ( ioctl(fd, MIOIPLCB, &mdd) )
           rc = 5;
    }

    /* now copy ethernet address from vpd array to address storage */
    rc = search_eth_addr(vpd, iplcb_dir->system_vpd_size, net_addr);

  return(rc);
}
