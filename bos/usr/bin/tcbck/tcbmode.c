static char sccsid[] = "@(#)56	1.1.1.4  src/bos/usr/bin/tcbck/tcbmode.c, cmdsadm, bos411, 9428A410j 6/10/94 17:20:36";

/*
 * COMPONENT_NAME: (CMDSADM) sysck - system configuration checker
 *
 * FUNCTIONS: ck_acl
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

#include <sys/acl.h>
#include <sys/priv.h>
#include <sys/fullstat.h>
#include <sys/mode.h>
#include <sys/audit.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "tcbaudit.h"
#include "tcbdbio.h"
#include "tcbmsg.h"

/*
 * All of the security relevant mode bits
 */

#define	ALL_MODES (S_ITP|S_ITCB|S_ISUID|S_ISGID|S_ISVTX|0777)

/*
 * External variables, flags, and function calls
 */

extern	int	fixit;
extern	int	pflg;
#ifdef	_NO_PROTO
extern	struct	acl	*acl_get();
extern	struct	pcl	*priv_get();
int	ch_mode();
#else
extern	struct	acl	*acl_get (char *);
extern	struct	pcl	*priv_get (char *);
int	ch_mode (char *, mode_t);
#endif

/*
 * List of legal file permissions in each position
 */

static	char	*legal_modes[9] = {
	"-r", "-w", "-x",
	"-r", "-w", "-x",
	"-r", "-w", "-x"
};

/*
 * NAME:	ck_mode
 *
 * FUNCTION:	verify a file has the correct mode
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURN VALUE:
 *	Zero if the mode is correct, non-zero otherwise
 */

int
#ifdef	_NO_PROTO
ck_mode (tcbent, statbuf)
struct	tcbent	*tcbent;
struct	fullstat *statbuf;
#else
ck_mode (
struct	tcbent	*tcbent,
struct	fullstat *statbuf)
#endif
{
	int	errors    = 0;	/* count of errors testing mode 	*/
	int	needfixTCB= 0;	/* indicates that we still need to fix 	*/
				/* even if we got no 'errors'		*/
	struct	pcl	*pcl;
	char	*file = tcbent->tcb_name;

	/*
	 * Is a valid mode defined for this file?
	 */

	if (tcbent->tcb_mode == -1)
		return 0;

	/*
	 * Test for the proper TCB bit value
	 */

	if ((tcbent->tcb_mode & S_ITCB) != (statbuf->st_mode & S_ITCB)) {
 		/*
 		** Only display message if neither the -p (pflg) nor
		** the -y (yflg) flags have been specified.  Also,
 		** only return an error if neither the -y nor the -p
 		** flag has been specified.  Since fixit=yflg||pflg (see
 		** tcbck.c), we can say 
		** "if (!fixit) ...print message and record errors"
 		*/
 		if (!fixit) {
 			msg1 (Wrong_TCB_Flag, file);
			errors++;
		}
		/*
		** If fixit was specified, we still need to fix this
		** problem!  But since we didn't increment errors, if
		** that was the ONLY problem with the file, the old check
		** below (    if (!errors)  return (0)   ) would succeed
		** and we would have returned w/o fixing the TCB problem.
		** The new variable, needfix, indicates this new condition -
		** that there were no 'errors' but that we need to fix
		** something.
		*/
		needfixTCB=TRUE;
	}

	/*
	 * Test for the proper TP bit value
	 */

	if ((tcbent->tcb_mode & S_ITP) != (statbuf->st_mode & S_ITP)) {
		msg1 (Wrong_TP_Flag, file);
		errors++;
	}

	/*
	 * Test for the proper permission bits
	 */

	if (tcbent->tcb_type != TCB_SYMLINK) {

		if ((tcbent->tcb_mode & 07777) != (statbuf->st_mode & 07777)) {
			msg1 (Wrong_File_Modes, file);
			errors++;
		}
	}

	/*
	 * Fix any modes that need to be fixed, or just return now if
	 * everything was OK.
	 */

	if (!errors && !needfixTCB)
		return 0;

	/*
	 * Correct the mode bits which are wrong.
	 */

	if (tcbent->tcb_type != TCB_SYMLINK) {
		if (ck_query (Correct_Modes, file)) {
			if (ch_mode (file, (mode_t) (tcbent->tcb_mode & ALL_MODES))) {
				msg1 (Chmod_Failed, file);
				mk_audit (SYSCK_Check, AUDIT_FAIL,
					file, CantFix, "mode");
				return -1;
			}
			mk_audit (SYSCK_Check, AUDIT_OK, file, Fixed, "mode");
		} else {
			mk_audit (SYSCK_Check, AUDIT_FAIL, file, NotFixed, "mode");
			return -1;
		}
		if (needfixTCB)
			return(0);
		else
			return ENOTRUST;
	}

        return 0;

}

/*
 * NAME:	ch_mode
 *
 * FUNCTION:	Set the mode bits on a file
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURN VALUE:
 *	Zero if the mode was set, non-zero on error
 */

int
#ifdef	_NO_PROTO
ch_mode (file, mode)
char	*file;		/* File name */
mode_t	mode;		/* File modes */
#else
ch_mode (
char	*file,
mode_t	mode)
#endif
{
	struct	acl	*acl;	/* File ACL */
	struct	pcl	*pcl;	/* File PCL */

	/* 
	 * Get the ACL to be changed and set the permission bits
	 * and extended attribute bits [ SUID, SGID, SVTX ]
	 */

	if (! (acl = acl_get (file)))
		return -1;

	acl->acl_mode &= ~(S_ISUID|S_ISGID|S_ISVTX|0777);
	acl->acl_mode |= (S_ISUID|S_ISGID|S_ISVTX) & mode;
	acl->u_access = (mode >> 6) & 07;
	acl->g_access = (mode >> 3) & 07;
	acl->o_access = mode & 07;

	if (acl_put (file, acl, 1)) {
		perror ("acl_put");
		return -1;
	}

	/*
	 * Get the PCL to be changed and set the trusted process
	 * and trusted computing base bits.  This only needs to be
	 * performed if any privileged bits are present in either
	 * the mode or the already existent PCL and they don't
	 * agree.
	 */

	if (! (pcl = priv_get (file)))
		return -1;

	if ((mode & (S_ITP|S_ITCB)) != (pcl->pcl_mode & (S_ITP|S_ITCB))) {
		pcl->pcl_mode &= ~(S_ITP|S_ITCB);
		pcl->pcl_mode |= (S_ITP|S_ITCB) & mode;

		if (priv_put (file, pcl, 1)) {
			perror ("priv_put");
			return -1;
		}
	} else
		free (pcl);

	/*
	 * Haven't returned because of an error yet, so everything
	 * must have worked just fine ...
	 */

	return 0;
}

/*
 * NAME:	mode2string
 *
 * FUNCTION:	verify a file has the correct mode
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURN VALUE:
 *	Zero if the mode is correct, non-zero otherwise
 */

int
#ifdef	_NO_PROTO
mode2string (mode, string)
mode_t	mode;
char	*string;
#else
mode2string (
mode_t	mode,
char	*string)
#endif
{
	char	*cp;

	cp = string;

	/*
	 * Start with the spelt out attributes.  The five
	 * security relevant attributes are:
	 *
	 * TCB	- Trusted computing base
	 * TP	- Trusted process
	 * SUID	- Set User ID
	 * SGID	- Set Group ID
	 * SVTX	- Save Text on Swap
	 *
	 * These attributes are added as literal values.
	 */

	if (mode & S_ITCB) {
		strcpy (cp, "TCB");
		cp += strlen (cp) + 1;
	}
	if (mode & S_ITP) {
		strcpy (cp, "TP");
		cp += strlen (cp) + 1;
	}
	if (mode & S_ISUID) {
		strcpy (cp, "SUID");
		cp += strlen (cp) + 1;
	}
	if (mode & S_ISGID) {
		strcpy (cp, "SGID");
		cp += strlen (cp) + 1;
	}
	if (mode & S_ISVTX) {
		strcpy (cp, "SVTX");
		cp += strlen (cp) + 1;
	}

	/*
	 * Now convert the nine remaining permission bits into
	 * a permissions string.  This implies that octal modes
	 * are automatically converted into string representation.
	 */

	*cp++ = mode & S_IRUSR ? 'r':'-';
	*cp++ = mode & S_IWUSR ? 'w':'-';
	*cp++ = mode & S_IXUSR ? 'x':'-';

	*cp++ = mode & S_IRGRP ? 'r':'-';
	*cp++ = mode & S_IWGRP ? 'w':'-';
	*cp++ = mode & S_IXGRP ? 'x':'-';

	*cp++ = mode & S_IROTH ? 'r':'-';
	*cp++ = mode & S_IWOTH ? 'w':'-';
	*cp++ = mode & S_IXOTH ? 'x':'-';

	*cp++ = '\0';
	*cp = '\0';
}

/*
 * NAME:	string2mode
 *
 * FUNCTION:	Convert mode strings to permission bits
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS:
 *	Zero on success, non-zero if an error occurs parsing 
 *	the strings.
 */

int
string2mode (string, mode, file)
char	*string;	/* Character string representing mode word */
mode_t	*mode;		/* pointer to mode word for output */
char	*file;		/* Name of the file the mode is for */
{
	char	*flag;	/* flag value being processed right now */
	char	*dummy;	/* for strtol() */
	mode_t	modes = 0; /* mode being built up */
	mode_t	perms = 0; /* 9 low order permission bits */
	int	i;	/* looping variable */
	int	errors = 0; /* count of errors parsing modes */

	for (flag = string;*flag;) {

		/*
		 * See what we have.  The privilege bits all begin
		 * with characters [A-Z], permissions start with a
		 * digit or [r-]
		 */

		if (*flag >= 'A' && *flag <= 'Z') {

			/*
			 * flag is one of SUID, SGID, SVTX, TCB
			 * or TP.  Waddle on down and figure it out.
			 */

			if (strcmp (flag, "SUID") == 0)
				modes |= S_ISUID;
			else if (strcmp (flag, "SGID") == 0)
				modes |= S_ISGID;
			else if (strcmp (flag, "SVTX") == 0)
				modes |= S_ISVTX;
			else if (strcmp (flag, "TCB") == 0)
				modes |= S_ITCB;
			else if (strcmp (flag, "TP") == 0)
				modes |= S_ITP;
			else {
				fmsg2 (Unknown_Mode, file, flag);
				errors++;
			}
		} else if (*flag >= '0' && *flag <= '7') {

			/*
			 * This should be an octal specification of the
			 * file mode.  Only the nine low order bits can
			 * be specified and it MUST be OCTAL.  Leading
			 * zeroes are tolerated for hardcore octal wimps.
			 */

			perms = strtol (flag, &dummy, 8);

			if (dummy == flag || (perms & ~0777)) {
				fmsg2 (Unknown_Mode, file, flag);
				errors++;
			} else if (dummy == (char *) 0 || *dummy != '\0') {
				fmsg2 (Unknown_Mode, file, flag);
				errors++;
			}
			modes |= perms;
		} else if (*flag == '-' || *flag == 'r') {

			/*
			 * Scan the nine permissions characters and convert
			 * into nine permission bits.  Each character is
			 * checked for validity against a list of legal
			 * characters.
			 */

			if (strlen (flag) != 9) {
				fmsg2 (Unknown_Mode, file, flag);
				errors++;
				goto next;
			}
			for (i = 0;i < 9;i++) {
				if (strchr (legal_modes[i], (int) flag[i])) {
					continue;
				} else {
					fmsg2 (Unknown_Mode, file, flag);
					errors++;
					break;
				}
			}

			/*
			 * See if there was an error parsing the string.
			 * Go do the next string if there was an illegal
			 * character in the permission string.
			 */

			if (i != 9)
				goto next;

			/*
			 * Convert each of the remaining 9 characters into its
			 * correct collection of bits
			 */

			/*
			 * User bits
			 */

			if (flag[0] == 'r')
				modes |= S_IRUSR;
			if (flag[1] == 'w')
				modes |= S_IWUSR;
			if (flag[2] == 'x')
				modes |= S_IXUSR;

			/*
			 * Group bits
			 */

			if (flag[3] == 'r')
				modes |= S_IRGRP;
			if (flag[4] == 'w')
				modes |= S_IWGRP;
			if (flag[5] == 'x')
				modes |= S_IXGRP;

			/*
			 * Other bits
			 */

			if (flag[6] == 'r')
				modes |= S_IROTH;
			if (flag[7] == 'w')
				modes |= S_IWOTH;
			if (flag[8] == 'x')
				modes |= S_IXOTH;
		} else {
			fmsg2 (Unknown_Mode, file, flag);
			errors++;
		}

		/*
		 * Done with the current flag.  Either there was an error
		 * processing the string, or the string parsed correctly
		 * and processing just landed here.  Skip to the end of
		 * the current value.  flag will be positioned at the
		 * beginning of the next value if there is one.
		 */
next:
		while (*flag++)
			;
	}

	if (! errors && modes)
		*mode = modes;
	else
		*mode = -1;

	return errors;
}
