static char sccsid[] = "@(#)92  1.2  src/bldenv/pkgtools/do_tape.c, pkgtools, bos412, GOLDA411a 1/29/93 15:50:10";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: init_tape
 *		write_file_to_tape
 *		write_tape_eot
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "adepackage.h"


int init_tape(Tapeinfo * tapeinfo)
{
 
  switch (tapeinfo->tapeflag)
    {
    case TAR:
      tapeinfo->tape_fd = open(tapeinfo->device_name, O_WRONLY|O_TRUNC|O_CREAT, 0666);
      if (tapeinfo->tape_fd < 0) {
	fprintf(stderr, "%s \n", strerror(errno));
	return(-1);
      }
      else
	buf_allocate(tapeinfo->blocksize);
      break;

    case BACKUP:
      if (init_backup(tapeinfo) != 0)
	return(-1);
      break;

    default:
      printf("Error: Illegal tape format \n");
      return(-1);
    }
  return(0);
}


int write_file_to_tape(Tapeinfo * tapeinfo, Fileinfo * fileinfo)
{
  int error;
  
  switch(tapeinfo->tapeflag)
    {
    case TAR:
      error = write_tar_file(tapeinfo, fileinfo);
      if (error != 0)
	return(-1);
      break;

    case BACKUP:
      if (write_backup_file(tapeinfo, fileinfo) != 0)
	return(-1);
      break;

    default:
      fprintf(stderr, "ERROR: Bad tape format specified \n");
      return(-1);
      break;
    }
  return(0);
}

void write_tape_eot(Tapeinfo *tapeinfo)
{

  switch(tapeinfo->tapeflag) {
  case TAR:
    write_tar_eot(tapeinfo);
    break;
 
  case BACKUP:
    write_backup_eot(tapeinfo);
    break;

  default:
    fprintf(stderr, "ERROR: Bad tape format specified \n");
    break;
  }
}
