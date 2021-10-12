static char sccsid[] = "@(#)47	1.1  src/bos/usr/bin/tcbck/tcbsrc.c, cmdsadm, bos411, 9428A410j 3/12/91 18:30:31";

/*
 * COMPONENT_NAME: (CMDSADM) sysck - system configuration checker
 *
 * FUNCTIONS: cp_source, ck_source
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

#include <sys/acl.h>
#include <sys/priv.h>
#include <sys/fullstat.h>
#include <sys/stat.h>
#include <sys/audit.h>
#include <sys/mode.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "tcbdbio.h"
#include "tcbmsg.h"
#include "tcbaudit.h"

/*
 * All of the security relevant mode bits
 */

#define	ALL_MODES (S_ITP|S_ITCB|S_ISUID|S_ISGID|S_ISVTX|0777)

/*
 * External variables, flags, and function calls
 */

extern	int	fixit;
extern	char	*Aclput;
extern	char	*Pclput;

/*
 * NAME:	ck_source
 *
 * FUNCTION:	Check a source file for proper format and existence
 *
 * RETURNS:	Zero on success, ENOTRUST if entry is not an absolute
 *		path name, and ENOENT if the entry does not exist.
 */

int
ck_source (struct tcbent *tcbent)
{
	char	*src = tcbent->tcb_source;
	char	*dst = tcbent->tcb_name;
	struct	fullstat sb;

	/*
	 * Start with not being NULL or "".  Both of these are
	 * perfectly OK.  Then verify that the name begins with
	 * a "/" and exists.
	 */

	if (src == (char *) 0 || *src == '\0')
		return 0;

	if (*src != '/') {
		fmsg2 (Absolute_Source, dst, src);
		mk_vaudit (SYSCK_Check, AUDIT_FAIL,
			SYSCK_Cfg, CantFix, Absolute_Source, dst, src, NULL);
		return ENOTRUST;
	}
	if (fullstat (src, FL_STAT|FL_NOFOLLOW, &sb)) {
		msg2 (No_Such_Source, dst, src);
		return -1;
	}

	/*
	 * Now make certain the file type of the source is correct.
	 */

	switch (tcbent->tcb_type) {
		case TCB_FILE:
			if (S_ISREG (sb.st_mode))
				return 0;
			break;
		case TCB_DIR:
			if (S_ISDIR (sb.st_mode))
				return 0;
			break;
		case TCB_FIFO:
			if (S_ISFIFO (sb.st_mode))
				return 0;
			break;
		case TCB_BLK:
			if (S_ISBLK (sb.st_mode))
				return 0;
			break;
		case TCB_CHAR:
			if (S_ISCHR (sb.st_mode) && ! S_ISMPX (sb.st_mode))
				return 0;
			break;
		case TCB_MPX:
			if (S_ISMPX (sb.st_mode))
				return 0;
			break;
	}

	/*
	 * The source object did not have the same type as the
	 * destination object.
	 */

	msg1 (Wrong_File_Type, src);
	return -1;
}

/*
 * NAME:	src_file
 */

static int
src_file (struct tcbent *tcbent)
{
	char	*dst = tcbent->tcb_name;
	char	*src = tcbent->tcb_source;
	char	buf[512];
	int	cnt;
	int	ifd,
		ofd;
	struct	fullstat sb;

	/*
	 * If there is no value for the source attribute, just attempt
	 * to create the destination file and return success based on
	 * the mknod() succeeding.
	 */

	if (*src == '\0') {
		if (fullstat (dst, FL_STAT|FL_NOFOLLOW, &sb) == 0 &&
				xremove (dst, &sb))
			return -1;

		if (xmknod (dst, S_IFREG, 0))
			return -1;

		return 0;
	}

	/*
	 * Source exists and is a regular file.  Try to unlink the
	 * destination file and create a new destination file for
	 * writing.
	 */

	if (fullstat (dst, FL_STAT|FL_NOFOLLOW, &sb) == 0) {
		if (xremove (dst, &sb))
			return -1;
	}
	if ((ofd = open (dst, O_CREAT|O_TRUNC|O_WRONLY, 0)) < 0)
		return -1;

	/*
	 * Now open the source file for reading and start copying
	 * blocks from the source to the destination.
	 */

	if ((ifd = open (src, O_RDONLY)) < 0) {

		/*
		 * Oh no, the open failed so I must close the
		 * destination file and unlink it.
		 */

		close (ofd);
		unlink (dst);
		return -1;
	}

	while ((cnt = read (ifd, buf, sizeof buf)) > 0) {
		if (write (ofd, buf, cnt) != cnt) {

			/*
			 * The write() failed!  There are good reasons
			 * for this to happen, but I'd rather not try
			 * to handle every pathological case.
			 */

			break;
		}
	}

	/*
	 * If "cnt" is not 0 then an error occured and I need to bail
	 * out now.  This can happen if read returns -1 or if a write
	 * returns other than "cnt".
	 */

	if (cnt != 0) {
		perror (cnt == -1 ? "read":"write");
		close (ofd);
		close (ifd);
		xunlink (dst);
		return -1;
	}

	/*
	 * The file was copied successfully, close the file descriptors
	 * and return success!
	 */

	close (ofd);
	close (ifd);
	return 0;
}

/*
 * NAME:	src_char_dev
 */

static int
src_char_dev (struct tcbent *tcbent)
{
	char	*dst = tcbent->tcb_name;
	char	*src = tcbent->tcb_source;
	struct	fullstat sb;

	/*
	 * The source must exist for character devices to be copied
	 * since the device information is gotten from the source
	 * file.
	 */

	if (*src == '\0') {
		msg1 (Donot_Know_How, dst);
		return -1;
	}

	/*
	 * Have to remove the destination file if it exists
	 */

	if (fullstat (dst, FL_NOFOLLOW, &sb) == 0 && xremove (dst, &sb))
		return -1;

	/*
	 * File doesn't exist, so I can just mknod() it and see
	 * what happens
	 */

	if (xmknod (dst, S_IFCHR, sb.st_rdev))
		return -1;

	return 0;
}

/*
 * NAME:	src_blk_dev
 */

static int
src_blk_dev (struct tcbent *tcbent)
{
	char	*dst = tcbent->tcb_name;
	char	*src = tcbent->tcb_source;
	struct	fullstat sb;

	/*
	 * The source must exist for block devices to be copied
	 * since the device information is gotten from the source
	 * file.
	 */

	if (src == 0 || *src == '\0' || fullstat (src, 0, &sb)) {
		msg1 (Donot_Know_How, dst);
		return -1;
	}

	/*
	 * Have to remove the destination file if it exists
	 */

	if (fullstat (dst, FL_NOFOLLOW, &sb) == 0 && xremove (dst, &sb))
		return -1;

	/*
	 * File doesn't exist, so I can just mknod() it and see
	 * what happens
	 */

	if (xmknod (dst, S_IFBLK, sb.st_rdev))
		return -1;

	return 0;
}

/*
 * NAME:	src_mpx_dev
 */

static int
src_mpx_dev (struct tcbent *tcbent)
{
	char	*dst = tcbent->tcb_name;
	char	*src = tcbent->tcb_source;
	struct	fullstat sb;

	/*
	 * The source must exist for multiplexed devices to be copied
	 * since the device information is gotten from the source
	 * file.
	 */

	if (src == 0 || *src == '\0' || fullstat (src, 0, &sb)) {
		msg1 (Donot_Know_How, dst);
		return -1;
	}

	/*
	 * Have to remove the destination file if it exists
	 */

	if (fullstat (dst, FL_NOFOLLOW, &sb) == 0 && xremove (dst, &sb))
		return -1;

	/*
	 * File doesn't exist, so I can just mknod() it and see
	 * what happens
	 */

	if (xmknod (dst, S_IFMPX, sb.st_rdev))
		return -1;

	return 0;
}

/*
 * NAME:	src_directory
 */

static int
src_directory (struct tcbent *tcbent)
{
	char	*dst = tcbent->tcb_name;
	char	*src = tcbent->tcb_source;
	struct	fullstat sb;

	/*
	 * The source is completely ignored since we aren't going
	 * to copy the directory, just create it.
	 */

	if (fullstat (dst, 0, &sb) == 0 && S_ISDIR (sb.st_mode))
		return 0;

	/*
	 * Something might be there, try to unlink it if there
	 * is an object there.
	 */

	if (fullstat (dst, FL_NOFOLLOW, &sb) == 0 && xremove (dst, &sb))
		return -1;

	/*
	 * Create a fresh new empty directory
	 */

	if (xmkdir (dst, (mode_t) 0))
		return -1;

	return 0;
}

/*
 * NAME:	src_fifo
 */

static int
src_fifo (struct tcbent *tcbent)
{
	char	*dst = tcbent->tcb_name;
	char	*src = tcbent->tcb_source;
	struct	fullstat sb;

	/*
	 * Have to remove the destination file if it exists
	 */

	if (fullstat (dst, FL_NOFOLLOW, &sb) == 0 && xremove (dst, &sb))
		return -1;

	/*
	 * File doesn't exist, so I can just mknod() it and see
	 * what happens
	 */

	if (xmknod (dst, S_IFIFO, 0))
		return -1;

	return 0;
}

/*
 * NAME:	cp_source
 *
 * FUNCTION:	Copy a file setting modes, permissions, etc.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURN VALUE:
 *	Zero if the copy succeeds, non-zero on error
 */

cp_source (struct tcbent *tcbent)
{
	char	*src = tcbent->tcb_source;
	char	*dst = tcbent->tcb_name;
	int	exist;
	int	i;
	int	rc;
	uid_t	uid;
	gid_t	gid;
	struct	fullstat sb;

	/*
	 * Source file must have an absolute pathname and it must
	 * exist if specified.
	 */

	if (*src && *src != '/') {
		fmsg2 (Absolute_Source, dst, src);
		mk_vaudit (SYSCK_Check, AUDIT_FAIL,
			SYSCK_Cfg, CantFix, Absolute_Source, dst, src, NULL);
		return ENOTRUST;
	}

	if (*src && fullstat (src, FL_STAT|FL_NOFOLLOW, &sb)) {
		msg2 (No_Such_Source, dst, src);
		return -1;
	}

	/*
	 * Target may exist already if it is a directory or a FIFO.
	 * For objects which do not exist, the owner, group and mode
	 * or acl must be specified.
	 */

	exist = access (dst, 0) == 0;

	if (! exist && (tcbent->tcb_owner == (uid_t) -1 ||
			tcbent->tcb_group == (gid_t) -1 ||
			(tcbent->tcb_mode == (mode_t) -1 &&
			tcbent->tcb_acl == (char *) 0))) {
		if (tcbent->tcb_owner == (uid_t) -1)
			msg2 (Needs_An_Attribute, "owner", dst);

		if (tcbent->tcb_group == (gid_t) -1)
			msg2 (Needs_An_Attribute, "group", dst);

		if (tcbent->tcb_mode == (mode_t) -1 && ! tcbent->tcb_acl)
			msg2 (Needs_An_Attribute, "mode", dst);

		return -1;
	}

	/*
	 * Each file type has it's own copy routine.  Pick the
	 * appropriate one.
	 */

	switch (tcbent->tcb_type) {
		case TCB_FILE:
			sig_ignore ();
			rc = src_file (tcbent);
			sig_reset ();
			break;
		case TCB_DIR:
			rc = src_directory (tcbent);
			break;
		case TCB_FIFO:
			rc = src_fifo (tcbent);
			break;
		case TCB_BLK:
			rc = src_blk_dev (tcbent);
			break;
		case TCB_CHAR:
			rc = src_char_dev (tcbent);
			break;
		case TCB_MPX:
			rc = src_mpx_dev (tcbent);
			break;
	}

	/*
	 * Examine the return code - copy has to succeed before
	 * the modes, etc., can be copied.  Directories and fifos
	 * which existed already have their modes left intact.
	 */

	if (rc)
		return rc;

	if (! exist || (tcbent->tcb_type != TCB_DIR &&
			tcbent->tcb_type != TCB_FIFO)) {
		if (xchown (dst, tcbent->tcb_owner, tcbent->tcb_group))
			rc = -1;

		if (tcbent->tcb_mode != (mode_t) -1)
			acl_set (dst, (tcbent->tcb_mode & 0700 >> 6),
				      (tcbent->tcb_mode & 070 >> 3),
				      (tcbent->tcb_mode & 07));
		else
			put_program (Aclput, dst, tcbent->tcb_acl);
	}
	return rc;
}
