static char sccsid[] = "@(#)66  1.2.1.5  src/bos/usr/bin/tcbck/tcbentry.c, cmdsadm, bos411, 9436D411a 9/8/94 18:07:00";
/*
 * COMPONENT_NAME: (CMDSADM) sysck - system configuration checker
 *
 * FUNCTIONS: ck_program, ck_pcl, ck_type, copy_file, ck_tcbent,
 *	ck_checksum
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/mode.h>
#include <sys/fullstat.h>
#include <sys/audit.h>
#include <sys/acl.h>
#include <sys/priv.h>
#include <sys/limits.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
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

extern	int	pflg;
extern	int	yflg;
extern	int	nflg;
extern	int	tflg;
extern	int	all;
extern	int	tree;

extern	struct	tcbent	**tcb_table;

extern	char	*get_program();
extern	struct	tcbent	*hash_find();

extern	int	ck_acl();
extern	int	ck_mode();
extern	char	*mk_sum();

/*
 * Program names
 */

extern	char	*Aclget;
extern	char	*Aclput;
extern	char	*Pclget;
extern	char	*Pclput;
extern	char	*Checksum;
extern	int	BuiltInSum;

/*
 * NAME:	ck_program
 *
 * FUNCTION:	Validate a file in the TCB using the file-dependent program
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * NOTES:
 *	TCB entries which fail the validation test will be disabled by
 *	having their modes set to 0.
 *
 * RETURNS:	Zero on success, non-zero otherwise
 */

int
#ifdef	_NO_PROTO
ck_program (tcbent)
struct	tcbent	*tcbent;
#else
ck_program (
struct	tcbent	*tcbent)
#endif
{
	int	i;
	char	buf[BUFSIZ];
	char	*file = tcbent->tcb_name;
	char	*program;
	struct	tcbent	*program_ent;

	/* 
	 * Set up program name
	 */

	if (tcbent->tcb_program)
		program = tcbent->tcb_program[0];
	else
		return 0;

	/*
	 * Verify that the test program exists and is executable.
	 */

	if (! program || program[0] != '/') {
		msg1 (Absolute_Program, program);
		mk_vaudit (SYSCK_Check, AUDIT_FAIL,
			file, CantFix, Absolute_Program, program, NULL);
		return -1;
	}
	if (access (program, X_ACC)) {
		msg1 (No_Program, program);
		return -1;
	}

	/*
	 * Create the command line buffer for the command to execute
	 */

	strcpy (buf, program);

	if (pflg)
		strcat (buf, " -p");
	else if (yflg)
		strcat (buf, " -y");
	else if (nflg)
		strcat (buf, " -n");
	else
		strcat (buf, " -t");

	/*
	 * Find any remaining arguments on the command line.
	 */

	if (tcbent->tcb_program[1]) {
		for (i = 1;tcbent->tcb_program[i];i++) {
			strcat (buf, " ");
			strcat (buf, tcbent->tcb_program[i]);
		}
	} else
		strcat (buf, " ALL");

	/*
	 * Execute the command and examine the return code.  A command
	 * which fails will cause a message to be put out, but no
	 * further action taken.
	 */

	if (system (buf) == 0)
		return 0;

	/*
	 * The file verify failed for some reason.  Complain about it.
	 */

	msg2 (Verify_Failed, program, file);
	return ENOTRUST;
}

/*
 * NAME:	ck_pcl
 *
 * FUNCTION:	Test a PCL for correctness
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * NOTES:
 *	This procedure may call the Pclput program to set the PCL
 *	on the named file.
 *
 * RETURNS:
 *	Zero for success, 1 for wrong PCL, -1 for error setting PCL
 */

int
#ifdef	_NO_PROTO
ck_pcl (tcbent)
struct	tcbent	*tcbent;
#else
ck_pcl (
struct	tcbent	*tcbent)
#endif
{
	char	*pcl;
	char	*file = tcbent->tcb_name;
	int	okay;
	int	rc;

	/*
	 * Fetch the PCL from the given file and see if it matches
	 * the PCL in the database.
	 */

	if (! (pcl = get_program (Pclget, file))) {
		rc = -1;
		goto error;
	}
	okay = fuzzy_compare (pcl, tcbent->tcb_pcl) != 0;
	free (pcl);

	/*
	 * If the PCLs matched, return OK.  Otherwise, print out
	 * and error message and try to fix the PCL.
	 */

	if (okay == 0)
		return 0;

	msg1 (Wrong_PCL, file);

	if (ck_query (Correct_PCL, file) &&
			put_program (Pclput, file, tcbent->tcb_pcl)) {
		rc = -1;
		goto error;
	}
	mk_audit (SYSCK_Check, AUDIT_OK, file, Fixed, "pcl");
	return ENOTRUST;

error:
	mk_audit (SYSCK_Check, AUDIT_FAIL, file, NotFixed, "pcl");
	return rc;
}

/*
 * NAME:	ck_checksum
 *
 * FUNCTION:	Test a checksum for correctness
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * NOTES:
 *	This procedure calls the Checksum program to determine the
 *	file checksum, which it compares against the checksum stored
 *	for the file.
 *
 * RETURNS:	Zero for success, non-zero otherwise
 */

int
#ifdef	_NO_PROTO
ck_checksum (tcbent)
struct	tcbent	*tcbent;
#else
ck_checksum (
struct	tcbent	*tcbent)
#endif
{
	char	*checksum;
	char	*file = tcbent->tcb_name;
	int	okay;
	int	rc;

	/*
	 * See if there is a checksum - return OK if there is none.
	 */

	if (tcbent->tcb_checksum == 0 || tcbent->tcb_vchecksum == 1)
		return 0;

	/*
	 * Run the checksum program and save the output.  The output
	 * of the checksum command must equal the value which was
	 * present before.
	 */

	if (! BuiltInSum) {
		if (! (checksum = get_program (Checksum, file))) {
			rc = -1;
			goto error;
		}
	} else {
		if (! (checksum = mk_sum (tcbent))) {
			rc = -1;
			goto error;
		}
	}
	okay = fuzzy_compare (checksum, tcbent->tcb_checksum) != 0;

	if (! BuiltInSum)
		free (checksum);

	/*
	 * Free the checksum output and see if there was a match.
	 * Report an error and disable the object if the two values
	 * are different.
	 */

	if (okay == 0)
		return 0;

	msg1 (Wrong_Checksum, file);

	if (ck_query (Disable_ACL, file)) {
		rm_acl (tcbent);
		mk_audit (SYSCK_Check, AUDIT_OK, file, Fixed, "checksum");
		rc = ENOTRUST;
	} else {
		rc = ENOTRUST;
		goto error;
	}
	errno = rc;
	return -1;

error:
	mk_audit (SYSCK_Check, AUDIT_FAIL, file, NotFixed, "checksum");
	errno = rc;
	return -1;
}

/*
 * NAME:	ck_size
 *
 * FUNCTION:	verify the correct file size
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURN VALUE:
 *	Zero if the size is correct, non-zero otherwise
 */

int
#ifdef	_NO_PROTO
ck_size (tcbent, stat)
struct	tcbent	*tcbent;
struct	fullstat *stat;
#else
ck_size (
struct	tcbent	*tcbent,
struct	fullstat *stat)
#endif
{
	char	*file = tcbent->tcb_name;
	int	rc;

	/*
	 * File type must be REGULAR FILE.  Sizes on other
	 * stuff is pretty meaningless.
	 */

	if (tcbent->tcb_type != TCB_FILE || tcbent->tcb_size == -1)
		return 0;

	/*
	 * Compare size against the value stored in the
	 * database.  The object is disabled if there is a
	 * discrepancy.
	 */

	if (tcbent->tcb_size == stat->st_size)
		return 0;

	msg1 (Wrong_Size, file);

	if (ck_query (Disable_ACL, file)) {
		rm_acl (tcbent);
		rc = ENOTRUST;
	} else {
		rc = ENOTRUST;
		goto error;
	}
	mk_audit (SYSCK_Check, AUDIT_OK, file, Fixed, "size");
	errno = rc;
	return -1;

error:
	mk_audit (SYSCK_Check, AUDIT_FAIL, file, NotFixed, "size");
	errno = rc;
	return -1;
}

/*
 * NAME:	ck_type
 *
 * FUNCTION:	verify a file has the correct type
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURN VALUE:
 *	Zero if the type exists and is correct, non-zero otherwise
 */

int
#ifdef	_NO_PROTO
ck_type (tcbent, statbuf)
struct	tcbent	*tcbent;
struct	fullstat *statbuf;
#else
ck_type (
struct	tcbent	*tcbent,
struct	fullstat *statbuf)
#endif
{
	mode_t	mode = statbuf->st_mode;
	char	*file = tcbent->tcb_name;
	int	rc;

	/*
	 * Test for the proper file type according to the type= attribute
	 * in the TCB file.
	 */

	switch (tcbent->tcb_type) {
		case TCB_FILE:
			if (S_ISREG (mode))
				return 0;
			break;
		case TCB_LINK:
			if (S_ISREG (mode))
				return 0;
			break;
		case TCB_DIR:
			if (S_ISDIR (mode))
				return 0;
			break;
		case TCB_FIFO:
			if (S_ISFIFO (mode))
				return 0;
			break;
		case TCB_SYMLINK:
			if (S_ISLNK (mode))
				return 0;
			break;
		case TCB_BLK:
			if (S_ISBLK (mode))
				return 0;
			break;
		case TCB_CHAR:
			if (S_ISCHR (mode) && ! S_ISMPX (mode))
				return 0;
			break;
		case TCB_MPX:
			if (S_ISMPX (mode))
				return 0;
			break;
	}

	/*
	 * If the file had the wrong type, report the error and disable
	 * the object by setting the mode to 0.
	 */

	msg1 (Wrong_File_Type, file);

	if (ck_query (Disable_ACL, file)) {
		rm_acl (tcbent);
		rc = ENOTRUST;
	} else {
		rc = ENOTRUST;
		goto error;
	}
	mk_audit (SYSCK_Check, AUDIT_OK, file, Fixed, "type");
	errno = ENOTRUST;
	return -1;

error:
	mk_audit (SYSCK_Check, AUDIT_FAIL, file, NotFixed, "type");
	errno = ENOTRUST;
	return -1;
}

/*
 * NAME:	ck_tcbent
 *
 * FUNCTION:	Test the attributes of a single file in the TCB
 *
 * NOTES:
 *	ck_tcbent calls a variety of functions to test the different
 *	attributes of a file.  ck_tcbent itself and these other functions
 *	may change the attributes of a file and create new files and
 *	directories in the course of doing their work.
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * RETURNS: Count of errors encountered correcting file.
 */

int
#ifdef	_NO_PROTO
ck_tcbent (tcbent)
struct	tcbent	*tcbent; /* TCB attribute structure for file */
#else
ck_tcbent (
struct	tcbent	*tcbent)
#endif
{
	char	*file;		/* Name of file being tested                */
	char	buf[BUFSIZ];	/* Scratch character buffer                 */
	int	just_created;	/* This file had to be copied or created    */
	int	i;		/* Scratch counter                          */
	int	errors = 0;	/* Count of errors testing current file     */
	int	severe = 0;	/* Count of severe errors on current file   */
	int	rc;		/* Return code from subroutines             */
	struct	fullstat statbuf; /* Information about current file         */
	FILE	*fp;		/* File pointer used for piping commands    */

	/*
	 * Initialize local variables and such
	 */

	file = tcbent->tcb_name;
	just_created = 0;

	/*
	 * See if this file has a source attribute.  It must be copied
	 * afresh if there is one.  Also, the source attribute must be
	 * an absolute path name =and= exist, even if it is not to be
	 * copied.
	 */

	if (! ck_source (tcbent)) {
		if (tcbent->tcb_source) {
			if (ck_query (Create_File, file)) {
				if (cp_source (tcbent)) {
					mk_audit (SYSCK_Check, AUDIT_FAIL,
						file, CantFix, "source");
					if (tcbent->tcb_source[0])
						msg2 (Copy_Failed,
							tcbent->tcb_source,
							file);
					else
						msg1 (Create_Failed, file);

					severe++;
				} else
					mk_audit (SYSCK_Check, AUDIT_OK,
						file, Fixed, "source");
			} else {
				mk_audit (SYSCK_Check, AUDIT_FAIL,
					file, NotFixed, "source");

				severe++;
			}
			errors++;
		}
	} else
		errors++;

	/*
	 * File should exist by this time
	 */

	if (fullstat (file, FL_STAT|FL_NOFOLLOW, &statbuf)) {
		msg1 (No_Such_File, file);
		return -1;
	}

	/*
	 * Begin by checking the file type for correctness.  Give
	 * up immediately if it is wrong.
	 */

	if (ck_type (tcbent, &statbuf))
		return -1;

	/*
	 * Check for the correct file owner.
	 */

	if (tcbent->tcb_owner != (uid_t) -1 &&
			tcbent->tcb_owner != statbuf.st_uid) {
		errors++;
		msg1 (Wrong_File_Owner, file);

		if (ck_query (Correct_Owner, tcbent->tcb_name)) {
			if (tcbent->tcb_type == TCB_SYMLINK) {
				if (xlchown (file, tcbent->tcb_owner, statbuf.st_gid)) {
					severe++;
					mk_audit (SYSCK_Check, AUDIT_FAIL,
						file, CantFix, "owner");
				} else {
					statbuf.st_uid = tcbent->tcb_owner;
					mk_audit (SYSCK_Check, AUDIT_OK,
						file, Fixed, "owner");
				}
			}
			else {
				if (xchown (file, tcbent->tcb_owner, statbuf.st_gid)) {
					severe++;
					mk_audit (SYSCK_Check, AUDIT_FAIL,
						file, CantFix, "owner");
				} else {
					statbuf.st_uid = tcbent->tcb_owner;
					mk_audit (SYSCK_Check, AUDIT_OK,
						file, Fixed, "owner");
				}
			}
		} else {
			mk_audit (SYSCK_Check, AUDIT_FAIL,
				file, NotFixed, "owner");
		}
	}

	/*
	 * Check for the correct file group.
	 */

	if (tcbent->tcb_group != (gid_t) -1 &&
			tcbent->tcb_group != statbuf.st_gid) {
		errors++;
		msg1 (Wrong_File_Group, file);

		if (ck_query (Correct_Group, file)) {
			if (tcbent->tcb_type == TCB_SYMLINK) {
				if (xlchown (file, statbuf.st_uid, tcbent->tcb_group)) {
					mk_audit (SYSCK_Check, AUDIT_FAIL,
						file, CantFix, "group");
					severe++;
				} else {
					statbuf.st_gid = tcbent->tcb_group;
					mk_audit (SYSCK_Check, AUDIT_OK,
						file, Fixed, "group");
				}
			}
			else {
				if (xchown (file, statbuf.st_uid, tcbent->tcb_group)) {
					mk_audit (SYSCK_Check, AUDIT_FAIL,
						file, CantFix, "group");
					severe++;
				} else {
					statbuf.st_gid = tcbent->tcb_group;
					mk_audit (SYSCK_Check, AUDIT_OK,
						file, Fixed, "group");
				}
			}
		} else {
			mk_audit (SYSCK_Check, AUDIT_FAIL,
				file, NotFixed, "group");
		}
	}

	/*
	 * Check for the correct file modes
	 */

	if (tcbent->tcb_mode != -1 && tcbent->tcb_type != TCB_SYMLINK 
				&& tcbent->tcb_type != TCB_LINK) {
		if (rc = ck_mode (tcbent, &statbuf)) {
			if (rc == -1)
				severe++;

			errors++;
		}
	}

	/*
	 * Check for the correct ACL.
	 */

	if (tcbent->tcb_acl) {
		if (rc = ck_acl (tcbent)) {
			if (rc == -1)
				severe++;

			errors++;
		}
	}

	/*
	 * Check for the correct PCL.
	 */

	if (tcbent->tcb_pcl) {
		if (rc = ck_pcl (tcbent)) {
			if (rc == -1)
				severe++;

			errors++;
		}
	}

	/*
	** Check for the correct number of hard links and each link
	** having the proper destination.  But only do this if the
	** entry is not a symlink.  You can't have a tcbent that's
	** a hard link - hard links are merely an attribute for a
	** file and are only kept in the hash table.  Also, you 
	** can't have a hard link to a symlink.  The hard link
	** will end up being a hard link to the original file,
	** assuming its in the same file system.  If it's not,
	** ln will bark about the files being in different file
	** systems.
	*/

	if ((tcbent->tcb_links) && (tcbent->tcb_type!=TCB_SYMLINK)) {
		if (rc = ck_links (tcbent, &statbuf)) {
			if (rc == -1)
				severe++;
			errors++;
		}
	}
	else if (! S_ISDIR (statbuf.st_mode) && statbuf.st_nlink != 1 && 
				tcbent->tcb_type != TCB_LINK) {
		errors++;
		msg1 (Too_Many_Links, file);
		if (! tree)
			msg (Use_Tree_Option);
	}

	/*
	 * Check for the correct symbolic links.
	 */

	if (rc = ck_symlinks (tcbent)) {
		if (rc == -1)
			severe++;
		errors++;
	}

	/*
	 * Validate the file using the specified program.  It is
	 * presumed the checking program has made any required
	 * modifications to the file if an error is detected.
	 */

        /*
         * If the type is SYMLINK then check if the symlink exists.
         */

        if (tcbent->tcb_type == TCB_SYMLINK) {
                if (rc = ck_symlink_type (tcbent))
                        errors++;
        }

        /*
         * Else if the type is LINK then check if the hard link exists.
         */

        else if (tcbent->tcb_type == TCB_LINK) {
                if (rc = ck_link_type (tcbent))
                        errors++;
        }

	if (tcbent->tcb_program) {

		if (rc = ck_program (tcbent)) {
			if (rc == -1)
				severe++;

			errors++;
		}
	}

	/*
	 * Validate the file using the checksum.  This is an uncorrectable
	 * error.
	 */

	if (tcbent->tcb_checksum) {
		if (rc = ck_checksum (tcbent)) {
			severe++;
			errors++;
		}
	}

	/*
	 * Validate the file's size.  This is an uncorrectable error.
	 */

	if (tcbent->tcb_size != -1) {
		if (rc = ck_size (tcbent, &statbuf)) {
			severe++;
			errors++;
		}
	}

	/*
	 * Return error result.  If all of errors could be fixed I return
	 * ENOTRUST and set errno == ENOTRUST.  If any of the problems could
	 * =not= be fixed I return -1 and leave errno alone.
	 */

	if (severe)
		return -1;

	if (errors)
		return (errno = ENOTRUST);

	return 0;
}
