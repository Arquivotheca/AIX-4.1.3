static char sccsid[] = "@(#)76  1.1  src/bos/usr/ccs/lib/libqb/piogetstatus.c, libqb, bos411, 9428A410j 12/14/93 16:10:37";
/*
 *   COMPONENT_NAME: CMDQUE
 *
 *   FUNCTIONS: piogetstatus
 *              
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/stat.h>
#include <IN/stfile.h>

/* DESCRIPTION:  
      The piogetstatus() subroutine provides a way to retrieve status of a
      currently running print job.

      The information it returns includes queue name, queue device name, job
      number, job status, percent done, number of pages printed, etc.  The
      subroutine reads the specified status file, and then populates the
      information into the passed StatusInfo structure, the format of which
      is determined by the version magic number specified by the VersionMagicNo
      parameter.  Each time there is a change in the status file structure for
      a new release, a unique number is assigned to its version magic number.
      This is done to support older structure formats in future upgrades.

   RETURN VALUES:
      Upon successful completion, the piogetstatus() subroutine returns a 
      value of 1.  If there is an error in retrieving information for the
      status file, the subroutine returns a value of -1.
*/

int piogetstatus (int StatusFd, int VersionMagicNo, void *StatusInfo)
{
	piostatus_t *ps;
	struct stfile s;
	struct stat buf;
	
	/* check to make sure file is opened */
	if (fstat(StatusFd, &buf) == -1)
		return (-1);

	/* for future upgrades: if struct piostatus is changed 
	   a new magic number is added
	*/
	if (VersionMagicNo != PIOSTATUS_VERMAGIC)
		return (-1);

	ps = StatusInfo;
	
	if (lseek(StatusFd, 0L, SEEK_SET) < 0)
		return (-1);
	if (read(StatusFd, &s, sizeof(s)) != sizeof(s))
		return (-1);

	/* assign the members from the AIX structure to the Palladium
	   structure.
	*/
	ps->s_jobnum   = s.s_jobnum;
	ps->s_status   = s.s_status;
	ps->s_align    = s.s_align;
	ps->s_feed     = s.s_feed;
	ps->s_head     = s.s_head;
	ps->s_trail    = s.s_trail;
	ps->s_copies   = s.s_copies;
	ps->s_mailonly = s.s_mailonly;
	ps->s_was_idle = s.s_was_idle;
	ps->s_percent  = s.s_percent;
	ps->s_pages    = s.s_pages;
	ps->s_charge   = s.s_charge;
	strcpy(ps->s_qdate, s.s_qdate);
	strcpy(ps->s_to, s.s_to);
	strcpy(ps->s_from, s.s_from);
	strcpy(ps->s_title, s.s_title);
	strcpy(ps->s_device_name, s.s_device_name);
	strcpy(ps->s_queue_name, s.s_queue_name);
	ps->s_uid   = s.s_uid;
	strcpy(ps->s_cmdline, s.s_cmdline);

	return (1);
}
