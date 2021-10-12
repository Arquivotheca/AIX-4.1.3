static char sccsid[] = "@(#)20	1.6  src/bos/usr/ccs/lib/libsrc/srcelog.c, libsrc, bos411, 9428A410j 2/26/91 14:54:38";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	srcelog
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 * OBJECT CODE OLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1984,1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
** IDENTIFICATION:
**    Name:	srcelog
**    Title:	Log SRC Error
** PURPOSE:
**	To log errors encountered by src functions.
** 
** SYNTAX:
**    srcelog(name, symptom, src_error, syserrno, file, line)
**    Parameters:
**	i char *name - name subsystem error applies to
**	i int symptom - symptom error code
**	i int src_error - SRC/Subsystem error code
**	i int syserrno - system defined error code
**	i char *file - source file detecting error
**	i int line - line number that detected error(associated with file)
**
** INPUT/OUTPUT SECTION:
**
** PROCESSING:
**	create error log record in the format of a err_rec structure
**	and send it to the error daemon.
**
** PROGRAMS/FUNCTIONS CALLED:
**	errlog
**
** OTHER:
**
** RETURNS:
**
**/

#include "src.h"
#include <sys/errids.h>

void srcelog(name, symptom, src_error, syserrno, file, line)
char *name;
int symptom;
int src_error;
int syserrno;
char *file;
int line;
{
	int rc ;
	int errlog();

	struct src_err
	{
		struct err_rec0 err_rec0;
		int symptom;
		int src_error;
		int err_code;
		char module[40];
		char subsys[20];
	} src_err;

	bzero(&src_err,sizeof(src_err));
	src_err.err_rec0.error_id = ERRID_SRC;
	strcpy(src_err.err_rec0.resource_name,"SRC");
	src_err.src_error = src_error;
	src_err.symptom = symptom;
	src_err.err_code = syserrno;

	if(file!=0)
		sprintf(src_err.module,"'%s'@line:'%d'",file,line);

	if(name!=0)
		strncpy(src_err.subsys,name,(size_t)20);

	errlog(&src_err,sizeof(src_err));
	return;
}
