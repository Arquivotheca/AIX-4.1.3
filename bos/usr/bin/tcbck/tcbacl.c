static char sccsid[] = "@(#)43	1.2  src/bos/usr/bin/tcbck/tcbacl.c, cmdsadm, bos411, 9428A410j 4/24/91 16:25:17";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: ck_acl, rm_acl, add_xacl
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/acl.h>
#include <sys/mode.h>
#include <sys/audit.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
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
extern	char	*get_program();
extern	int	put_program();
extern	struct	acl	*acl_get();

/*
 * Program names
 */

extern	char	*Aclget;
extern	char	*Aclput;

/*
 * NAME:	ck_acl
 *
 * FUNCTION:	Test an ACL for correctness
 *
 * NOTES:
 *	This procedure may call the Aclput program to add the ACL
 *	to the named file.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS:	Zero for success, non-zero otherwise
 */

int
#ifdef	_NO_PROTO
ck_acl (tcbent)
struct	tcbent	*tcbent;
#else
ck_acl (
struct	tcbent	*tcbent)
#endif
{
	char	*acl;
	char	*file = tcbent->tcb_name;
	int	okay;

	/*
	 * Get the ACL.  The program should be executable, so
	 * if the get fails blame it on the file!
	 */

	if (! (acl = get_program (Aclget, file)))
		return ENOENT;

	/*
	 * The ACLs should textually compare identical ignoring whitespace.
	 */

	okay = fuzzy_compare (acl, tcbent->tcb_acl) == 0;
	free (acl);

	/*
	 * Correct the ACL and report an error if the ACLs
	 * aren't identical.
	 */

	if (! okay) {
		msg1 (Wrong_ACL, file);

		if (ck_query (Correct_ACL, file))
			if (put_program (Aclput, file, tcbent->tcb_acl))
				mk_audit (SYSCK_Check, AUDIT_FAIL,
					file, CantFix, "acl");
			else
				mk_audit (SYSCK_Check, AUDIT_OK,
					file, Fixed, "acl");
		else
			mk_audit (SYSCK_Check, AUDIT_FAIL,
				file, NotFixed, "acl");
	}
	return okay ? 0:-1;
}

/*
 * NAME:	rm_acl
 *
 * FUNCTION:	Disable an object by turning of the access permissions
 *		and truncating any extended attributes.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS:	Zero for success, non-zero otherwise
 */

int
#ifdef	_NO_PROTO
rm_acl (tcbent)
struct	tcbent	*tcbent;
#else
rm_acl (
struct	tcbent	*tcbent)
#endif
{
	struct	acl	*acl;	/* pointer to fetched ACL */

	/*
	 * Start by getting the acl
	 */

	if (! (acl = acl_get (tcbent->tcb_name)))
		return ENOENT;

	/*
	 * Set the file permissions to 0 and turn off the
	 * ACL enabled bit.  Remove any extended attributes
	 * by setting the length = ACL_SIZ
	 */

	acl->acl_mode &= S_ISVTX;
	acl->u_access = 0;
	acl->g_access = 0;
	acl->o_access = 0;
	acl->acl_len = ACL_SIZ;

	/*
	 * Apply the modified ACL to the the file.  The extended
	 * entries are left in tact, but they have been disabled.
	 */

	if (acl_put (tcbent->tcb_name, acl, 1))
		return ENOENT;

	return 0;
}

/*
 * NAME:	add_xacl
 *
 * FUNCTION:	Add extended ACL entries to an existent ACL, or create
 *		a new ACL entry and add the extended entries to that.
 *
 * RETURN VALUE:
 *	Zero on success, non-zero on failure.
 */

int
add_xacl (tcb, xacl)
struct	tcbent	*tcb;
char	*xacl;
{
	char	*acl;
	char	*newacl;

	/*
	 * See if I need to get the ACL off of the file
	 */

	if (tcb->tcb_acl == 0) {
		if (! (tcb->tcb_acl = get_program (Aclget, tcb->tcb_name)))
			return -1;
	}

	/*
	 * Now allocate room for a new ACL which will have the extended
	 * ACL plastered to the end of it.
	 */

	acl = (char *)xmalloc (strlen (xacl) + strlen (tcb->tcb_acl) + 2);
	strcpy (acl, tcb->tcb_acl);
	strcat (acl, "\n");
	strcat (acl, xacl);

	/*
	 * The old ACL is freed and the newly extended ACL is put in its
	 * place.
	 */

	free (tcb->tcb_acl);
	tcb->tcb_acl = acl;

	return 0;
}
