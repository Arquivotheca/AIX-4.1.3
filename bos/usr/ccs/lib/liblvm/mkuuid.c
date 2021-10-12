static char sccsid[] = "@(#)15	1.2  src/bos/usr/ccs/lib/liblvm/mkuuid.c, liblvm, bos411, 9428A410j 6/16/90 02:05:54";
/*
 * COMPONENT_NAME:
 *
 * FUNCTIONS: mkuuid
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/***********************************************************************
 *   Include files                                                     *
 ***********************************************************************
 */

#include <sys/types.h>

#include <sys/time.h>

#include <sys/utsname.h>



/***********************************************************************
 *                                                                     *
 * NAME:  mkuuid                                                       *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine returns a unique id.                                 *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     unique_id                                                       *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     *unique_id                                                      *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value = 0 indicates successful return.                  *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *                                                                     *
 ***********************************************************************
 */


int mkuuid (unique_id)


/***********************************************************************
 *   Parameter declarations                                            *
 ***********************************************************************
 */

struct unique_id *unique_id;
  /* a pointer to the structure where the unique id is to be stored */


{ /* BEGIN mkuuid  */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

struct timestruc_t cur_time;
  /* a structure to contain the current time from the system clock */

struct utsname uname_buf;
  /* buffer in which to hold output from the uname system call which is
     used to get the machine id */

long machine_id;
  /* the machine id value for the system */

int retcode;
  /* the return code */


/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/***********************************************************************
 *   For this release, only two words of the unique id structure will  *
 *   be used.  Words 3 and 4 will contain zeroes for this release.     *
 *   Word 2 is generated from the current time in seconds.  Word 1     *
 *   is generated from the machine id.                                 *
 ***********************************************************************
 */

bzero ((caddr_t) unique_id, sizeof (struct unique_id));
  /* zero out structure where unique id is to be returned */

retcode = gettimer (TIMEOFDAY, &cur_time);
  /* get the current time from the system clock */

if (retcode == -1)
  /* if an error occurred */
    {
    return (retcode);
      /* return with error code */
    }

retcode = uname (&uname_buf);
  /* call system routine which will return the machine id of the
     system */

if (retcode == -1)
  /* if an error occurred */
    {
    return (retcode);
      /* return with error code */
    }

machine_id = 0;
  /* initialize machine id to 0 */

retcode = sscanf (uname_buf.machine, "%8X", &machine_id);
  /* convert character machine id to an integer */

unique_id -> word1 = machine_id;
  /* store the machine id into the first word of the unique id */

unique_id -> word2 = cur_time.tv_sec * 1000 + cur_time.tv_nsec / 1000000;
  /* store the current time in milliseconds into the second word of the
     unique id */

return (0);
  /* return with successful return code */

} /* END mkuuid */







