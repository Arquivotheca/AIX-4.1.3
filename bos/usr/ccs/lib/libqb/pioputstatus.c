static char sccsid[] = "@(#)77  1.1  src/bos/usr/ccs/lib/libqb/pioputstatus.c, libqb, bos411, 9428A410j 12/14/93 16:10:57";
/*
 *   COMPONENT_NAME: CMDQUE
 *
 *   FUNCTIONS: pioputstatus
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
      The pioputstatus() subroutine provides a way to store status information
      of a currently running print job.

      The subroutine accepts a status structure containing print job 
      information that includes queue name, queue device name, job number,
      job status, etc, and then stores the specified information in the
      specified status file.

      The format of the status structure is determined by the version magic
      number specified by the VersionMagicNo parameter.  Each time there is
      a change in the status file structure for a new release, a unique
      number is assigned to its version magic number.  This is done to
      support older structure formats in future upgrades.

   RETURN VALUES:
      Upon successful completion, the pioputstatus subroutine returns a value
      of 1.  If there is an error in storing information in the status file,
      the subroutine returns a value of -1.
*/

int pioputstatus (int StatusFd, int VersionMagicNo, const void *StatusInfo)
{
	piostatus_t *ps;
	struct stfile s;
	struct stat buf;

	/* check to see if the file exists*/
	if (fstat(StatusFd, &buf) == -1)
		return (-1);

	if (VersionMagicNo != PIOSTATUS_VERMAGIC)
		return (-1);

	ps = StatusInfo;
	
	/* assigns the the Palladium members to the corresponding AIX
	   members.
	*/
	s.s_jobnum   = ps->s_jobnum;
	s.s_status   = ps->s_status;
	s.s_align    = ps->s_align;
	s.s_feed     = ps->s_feed;
	s.s_head     = ps->s_head;
	s.s_trail    = ps->s_trail;
	s.s_copies   = ps->s_copies;
	s.s_mailonly = ps->s_mailonly;
	s.s_was_idle = ps->s_was_idle;
	s.s_percent  = ps->s_percent;
	s.s_pages    = ps->s_pages;
	s.s_charge   = ps->s_charge;
	strcpy(s.s_qdate, ps->s_qdate);
	strcpy(s.s_to, ps->s_to);
	strcpy(s.s_from, ps->s_from);
	strcpy(s.s_title, ps->s_title);
	strcpy(s.s_device_name, ps->s_device_name);
	strcpy(s.s_queue_name, ps->s_queue_name);
	s.s_uid   = ps->s_uid;
	strcpy(s.s_cmdline, ps->s_cmdline);

	/* Write the status info. out to the status file */
	if (lseek(StatusFd, 0L, SEEK_SET) < 0)
		return (-1);
	if (write(StatusFd, &s, sizeof(s)) != sizeof(s))
		return (-1);

	return (1);
}
