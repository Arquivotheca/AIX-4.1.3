static char sccsid[] = "@(#)44	1.2  src/bos/usr/bin/tcbck/tcbaudit.c, cmdsadm, bos411, 9428A410j 5/14/91 15:47:52";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: mk_audit, mk_vaudit, mk_audit_rec
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

#include <stdio.h>
#include <stdarg.h>

/*
 * Define some audit events and audit result strings
 */

char	*SYSCK_Check = "TCBCK_Check";
char	*SYSCK_Update = "TCBCK_Update";
char	*SYSCK_Install = "TCBCK_Install";
char	*Event;

char	*Fixed = "fixed";
char	*NotFixed = "not fixed";
char	*CantFix = "could not fix";

/*
 * String to represent the "sysck.cfg" audit object
 */

char	*SYSCK_Cfg = "sysck.cfg";

/*
 * NAME: mk_audit
 *                                                                    
 * FUNCTION: Create an audit record given the appropriate information
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * NOTES:
 *
 *	This routine creates fixed argument audit records.  It is
 *	called by mk_vaudit to create audit records for variable
 *	argument count audit records.  It is only useful for the
 *	three-part audit records generated for SYSCK_Check events.
 *
 * RETURNS: NONE
 */  

void
mk_audit (char *event, int status, char *object, char *result, char *error)
{
	auditwrite (event, status,
		object, strlen (object) + 1,
		error, strlen (error) + 1,
		result, strlen (result) + 1,
		0);
}

/*
 * NAME: mk_vaudit
 *                                                                    
 * FUNCTION: Create an audit record given the appropriate information
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * NOTES:
 *
 *	This routine is used for "SYSCK_Check"-style audit records
 *	which have three parts.
 *
 * RETURNS: NONE
 */  

void
mk_vaudit (char *event, int status, char *object, char *result, char *format, ...)
{
	char	buf[BUFSIZ];
	va_list	ap;

	va_start (ap, format);
	vsprintf (buf, format, ap);

	mk_audit (event, status, object, result, buf);
}

/*
 * NAME: mk_audit_rec
 *                                                                    
 * FUNCTION: Create an arbitrary length audit record from a variable
 *	number of entries.  This differs from mk_vaudit in that it
 *	doesn't catenate all the tail parts - it builds a buffer
 *	and calls auditlog with NUL-separated strings.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: NONE
 */  

void
mk_audit_rec (char *event, int status, ...)
{
	char	buf[BUFSIZ];
	char	*cp;
	char	*next;
	va_list	ap;

	va_start (ap, status);

	for (cp = buf;(next = va_arg (ap, char *)) != NULL;) {
		if (cp - buf + strlen (next) > BUFSIZ)
			break;

		strcpy (cp, next);
		cp += strlen (cp) + 1;
	}
	auditlog (event, status, buf, cp - buf);
}
