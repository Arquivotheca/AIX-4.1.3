static char sccsid[] = "@(#)50	1.1.1.4  src/bos/usr/bin/tcbck/tcblink.c, cmdsadm, bos411, 9440F411a 10/12/94 12:57:30";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: ck_hard_link, ck_soft_link, ck_links, ck_symlinks
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
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "tcbdbio.h"
#include "tcbmsg.h"
#include "tcbaudit.h"

/*
 * External variables, flags, and function calls
 */

extern	int	tree;

extern	struct	tcbent		**tcb_table;
extern	struct	symlink_rec	*symlist;

/*
 * NAME:	ck_hard_link
 *
 * FUNCTION:	Test a pair of files for being hard linked.
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * NOTES:
 *	If the two files are not hard linked this function will
 *	attempt to hard link them.  If a target exists but is not
 *	a link, the target will be removed.
 *
 * RETURNS:
 *	Zero for success, non-zero otherwise
 */

static int
ck_hard_link (char *a, char *b)
{
	struct	fullstat stat1;
	struct	fullstat stat2;
	int	need_link = 0;
	int	errors = 0;

	/* Must be an absolute pathname */

	if (*b != '/') {
		msg1 (Absolute_Link, b);
		return -1;
	}
	fullstat (a, FL_STAT|FL_NOFOLLOW, &stat1);

	/*
	** The destination must not exist, or must be a link to the
	** source file already.
	*/

	if (fullstat (b, FL_STAT|FL_NOFOLLOW, &stat2)) {
		msg2 (No_Such_Link, a, b);
		errors++;
		need_link++;

	/*
	** Check to see if these are really the same inode
	*/
	} else if (stat1.st_dev != stat2.st_dev ||
			stat1.st_ino != stat2.st_ino) {
		msg2 (Incorrect_Link, b, a);
		errors++;

		if (ck_query (Remove_File, b)) {
			if (xremove (b, &stat2)) {
				mk_vaudit (SYSCK_Check, AUDIT_FAIL,
					a, CantFix, "unlink %s", b, NULL);
				return -1;
			} else {
				mk_vaudit (SYSCK_Check, AUDIT_OK,
					a, Fixed, "unlink %s", b, NULL);
			}
			need_link++;
		} else {
			mk_vaudit (SYSCK_Check, AUDIT_FAIL,
				a, NotFixed, "unlink %s", b, NULL);
			return -1;
		}
	}

	/*
	** If the link does not exist, or the destination file was
	** removed, the link must be created.
	*/

	if (need_link) {
		if (ck_query (Create_Link, b)) {
			if (xlink (a, b)) {
				mk_vaudit (SYSCK_Check, AUDIT_FAIL,
					a, CantFix, "link %s %s", a, b, NULL);
				return -1;
			} else
				mk_vaudit (SYSCK_Check, AUDIT_OK,
					a, Fixed, "link %s %s", a, b, NULL);
		} else {
			mk_vaudit (SYSCK_Check, AUDIT_FAIL,
				a, NotFixed, "link %s %s", a, b, NULL);
			return -1;
		}
	}
	return errors ? ENOTRUST:0;
}

/*
 * NAME:	ck_soft_link
 *
 * FUNCTION:	Test for a soft link between two files.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * NOTES:
 *	If the target does not exist, a symbolic links shall be
 *	created.  If the target does exist, but is not a symbolic
 *	link to the source, the target shall be removed and
 *	recreated as a link to the source.
 *
 * RETURNS:	Zero for success, non-zero otherwise
 */

static int
ck_soft_link (char *a, char *b)
{
	char	link[PATH_MAX];	/* buffer to read symbolic link into     */
	int	length;		/* length of symbolic link name          */
	int	need_link = 0;	/* a link needs to be created            */
	int	errors = 0;	/* count of unrecoverable errors in here */
	struct	fullstat stat;	/* stat buffer for file link testing     */

	/* Must be an absolute pathname */
	 
	if (*b != '/') {
		msg1 (Absolute_Link, b);
		return -1;
	}

	/* The source file must exist already */
        /* Comment out - We will allow symbolic links to exist alone
           without their corresponding source files
           We will issue the error message, but will not return -1
           which was causing problems for install and de-install.
           These problems will go away in 4.2 with the removal of the
           bos.compat.links.il file.
        */


	if (access (a, 0)) {
		msg1 (No_Such_File, a);
	/*		return -1;	*/
			return 0;
	}

	/*
	** The destination must either not exist, or already be a
	** symbolic link pointing to the destination file, unless
	** tree mode was specified by user.
	*/

	if (fullstat (b, FL_NOFOLLOW|FL_STAT, &stat)) {
		msg2 (No_Such_Symlink, a, b);
		errors++;
		need_link++;
	} 
	else {

		/*
		 * File and symbolic link both exist.  Read the
		 * contents of the link and verify they are correct.
		 */

		if ((length = readlink (b, link, BUFSIZ)) <0) {
			/*
			** The readlink failed.  Try to remove the file
			** if it's there.  If it's not, remember that
			** we need to add the sym link (need_link++)
			*/
			msg2 (No_Such_Symlink, a, b);
			errors++;

			if (ck_query (Remove_File, b)) {
				if (xremove (b, &stat)) {
					mk_vaudit (SYSCK_Check, AUDIT_FAIL,
						a, NotFixed,
						"unlink %s", b, NULL);
					return -1;
				} 
				else 
					mk_vaudit (SYSCK_Check, AUDIT_OK,
						a, Fixed, "unlink %s", b, NULL);
			} 
			else {
				mk_vaudit (SYSCK_Check, AUDIT_FAIL,
					a, CantFix, "unlink %s", b, NULL);
				return -1;
			}
			need_link++;
		} 
		else {
			/* 
			** The readlink succeded.  NULL terminate the name
			** and compare it with the target name passed in.
			*/
			link[length] = '\0';

			if (strcmp(a, link)) {
				/*
				** Link does not point to passed in target name
				*/
				msg2(Incorrect_Symlink, b, a);
				errors++;

				/*
				** Nuke b, if it's there, and remember that
				** we need to create a new symlink (need_link++)
				*/
				if (ck_query (Remove_File, b)) {
					if (xremove (b, &stat)) {
						mk_vaudit (SYSCK_Check,
							AUDIT_FAIL, a, NotFixed,
							"unlink %s", b, NULL);
						return -1;
					}
					else
						mk_vaudit (SYSCK_Check,
						AUDIT_OK, a, Fixed,
						"unlink %s", b, NULL);
				} 
				else {
					mk_vaudit (SYSCK_Check, AUDIT_FAIL,
						a, CantFix,
						"unlink %s", b, NULL);
					return -1;
				}
				need_link++;
			}
		}
	}

	/*
	** A new symbolic links needs to be created either because
	** the old one was removed or because there was none to begin
	** with.
	*/

	if (need_link) {
		if (ck_query (Create_Link, b)) {
			if (xsymlink (a, b)) {
				mk_vaudit (SYSCK_Check, AUDIT_FAIL, a, CantFix,
					"symlink %s %s", a, b, NULL);
				return -1;
			} else
				mk_vaudit (SYSCK_Check, AUDIT_OK, a, Fixed,
					"symlink %s %s", a, b, NULL);
		} 
		else {
			mk_vaudit (SYSCK_Check, AUDIT_FAIL, a, NotFixed,
				"symlink %s %s", a, b, NULL);
			return -1;
		}
	}
	return errors ? ENOTRUST:0;
}

/*
 * NAME:	ck_links
 *
 * FUNCTION:	Test hard links for a TCB entry.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * NOTES:
 *	Counts the number of links and calls ck_hard_links to
 *	validate each one.
 *
 * RETURNS:	Zero for success, non-zero otherwise
 */

int
ck_links (struct tcbent	*tcbent, struct	fullstat *sb)
{
	char	*file = tcbent->tcb_name; /* file we are processing	*/
	int	nlinks=0;		  /* # of hard links found	*/
	int	i;			  /* for going thru tcb_table	*/
	int	rc;			  /* what ck_hard_link returns	*/
	int	errors = 0;		  /* error count		*/
	int	severe = 0;		  /* severe error count		*/


	for (nlinks = 0;tcbent->tcb_links[nlinks];nlinks++)
		;

	/* add one for the object itself */
	nlinks++;

	/*
	 * Each link in the list is now tested for validity.  The
	 * inode data is compared to see if the two filenames refer
	 * to the same filesystem object.
	 */

	for (i = 0;tcbent->tcb_links[i];i++) {
		if (rc = ck_hard_link (file, tcbent->tcb_links[i])) {
			if (rc == -1)
				severe++;

			errors++;
		}
	}

	/*
	 * There should be one more link than listed in the TCB
	 * entry - that is for the file itself.  Any more or less
	 * gets an error message.
	 */

	if (nlinks != sb->st_nlink) {
		errors++;
		if (nlinks > sb->st_nlink)
			msg1 (Wrong_Link_Count, file);

		if (nlinks < sb->st_nlink) {
			errors++;
			msg1 (Too_Many_Links, file);

			if (! tree)
				msg (Use_Tree_Option);
		}
	}
	if (severe)
		return -1;

	return errors ? ENOTRUST:0;
}

/*
 * NAME:	ck_symlinks
 *
 * FUNCTION:	Test symbolic links for a TCB entry.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * NOTES:
 *	Each symbolic link is tested for validity.
 *
 * RETURNS:	Zero for success, non-zero otherwise
 */

int
ck_symlinks (struct tcbent *tcbent)
{
	char	*file=tcbent->tcb_name;	     	/* file we are processing   */
	struct	symlink_rec	*curlink;	/* list traversal ptr	    */
	int	rc;				/* return from ck_soft_link */
	int	errors = 0;			/* total error count	    */
	int	severe = 0;			/* severe error count	    */

	/*
	** All of the symlinks found in the sysck.cfg file are kept on the
	** linked list, 'symlinks'.  This list is set up in add_tcb_table
	** in tcball.c.
	*/
	
	/*
	** Short circuit out if there is nothing on the list.
	*/
	if (!symlist)
		return(0);

	/*
	** Get ready to traverse the list.
	*/
	curlink=symlist;

	while (curlink) {
		/*
		** Does this link's target point to the filename
		** of the tcbent passed in?
		*/
		if (!strcmp(file,curlink->tcbent->tcb_target)) {

			/*
			** Yep, so this is a symlink to this tcbent.
			** gotta go validate it.
			*/
			if (rc=ck_soft_link(file,curlink->tcbent->tcb_name)) {

				/*
				** There was a problem
				*/
				if (rc==-1)
					severe++;
				errors++;
			}
		}
		curlink=curlink->next;
	}
	if (severe)
		return -1;
	return errors ? ENOTRUST:0;
}



/*
 * NAME:        ck_symlink_type
 *
 * FUNCTION:    Checks to see if the symlinks exists, and creates it
 *              if it doesn't.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      User process
 *
 * RETURNS:     Zero for success, non-zero otherwise
 */

int
ck_symlink_type (struct tcbent *tcbent)
{
        int     rc;
        int     severe = 0;

        if (rc = ck_soft_link (tcbent->tcb_target, tcbent->tcb_name)) {
                if (rc == -1)
                        severe++;
        }

        if (severe)
                return -1;
        else
                return 0;

}

/*
 * NAME:        ck_link_type
 *
 * FUNCTION:    Test hard link and create if it doesn't exist.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      User process
 *
 * RETURNS:     Zero for success, non-zero otherwise
 */

int
ck_link_type (struct tcbent *tcbent)
{
        char    *file = tcbent->tcb_name;
        int     rc;
        int     severe = 0;

        /*
         * The link is now tested for validity.  The inode
         * data is compared to see if the two filenames
         * refer to the same filesystem object.
         */

        if (rc = ck_hard_link (tcbent->tcb_target,file)) {
                if (rc == -1)
                        severe++;

        }

        if (severe)
                return -1;
        else
                return 0;
}

