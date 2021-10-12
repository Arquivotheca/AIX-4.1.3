static char sccsid[] = "@(#)32  1.2  src/bos/diag/tu/iopsl/misc.c, tu_iopsl, bos411, 9428A410j 3/6/92 17:02:54";
/*
 * COMPONENT NAME : TU_IOPSL
 *
 * FUNCTIONS      : getbyte, put_byte, get_word,
 *                  put_word
 *
 * ORIGINS        : 27
 *
 * IBM CONFIDENTIAL --
 *
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <ctype.h>

#include <diag/atu.h>
#include "misc.h"
#include "salioaddr.h"


/* The following functions perform read and write operations to specified
   addresses */

int getbyte(int fd, uchar_t *byte, ulong_t addr, int op)
{
  MACH_DD_IO iob;
  int rc;

  rc = SUCCESS;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(uchar_t);
  iob.md_data = byte;
  iob.md_addr = addr;

  if(ioctl(fd, op, &iob) == -1)
  {
    PRINTSYS("ioctl(GET)");
    rc = IO_ERROR;
  }

  return(rc);
}



int put_byte(int fd, uchar_t byte, ulong_t addr, int op)
{
  MACH_DD_IO iob;
  int rc;

  rc = SUCCESS;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(uchar_t);
  iob.md_data = &byte;
  iob.md_addr = addr;

  if(ioctl(fd, op, &iob) == -1)
  {
    PRINTSYS("ioctl(PUT)");
    rc = IO_ERROR;
  }

  return(rc);
}



int get_word(int fd, ulong_t *word, ulong_t addr, int op)
{
  MACH_DD_IO iob;
  int rc;

  rc = SUCCESS;
  iob.md_incr = MV_WORD;
  iob.md_size = sizeof(ulong_t);
  iob.md_data = (uchar_t *) word;
  iob.md_addr = addr;

  if(ioctl(fd, op, &iob) == -1)
  {
    PRINTSYS("ioctl(GET)");
    rc = IO_ERROR;
  }

  return(rc);
}



int put_word(int fd, ulong_t word, ulong_t addr, int op)
{
  MACH_DD_IO iob;
  int rc;

  rc = SUCCESS;
  iob.md_incr = MV_WORD;
  iob.md_size = sizeof(ulong_t);
  iob.md_data = (uchar_t *) &word;
  iob.md_addr = addr;

  if(ioctl(fd, op, &iob) == -1)
  {
    PRINTSYS("ioctl(PUT)");
    rc = IO_ERROR;
  }

  return(rc);
}




