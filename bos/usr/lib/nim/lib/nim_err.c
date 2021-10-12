
static char     sccs_id[] = " @(#)97 1.19  src/bos/usr/lib/nim/lib/nim_err.c, cmdnim, bos41J, 9516B_all  4/21/95  12:56:26";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: disable_err_sig
 *		enable_err_sig
 *		errsig_handler
 *		errstr
 *		nene
 *		nim_err
 *		nim_error
 *		nim_error_plus
 *		nim_log
 *		nim_mail
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************************
 *
 *                             NIM function library
 *
 * this file contains functions which are common across ALL NIM commands
 * NONE of these files should reference the NIM database; any function which
 *		does belongs in the libmstr.c file
 ******************************************************************************/

#include "cmdnim.h"
#include "cmdnim_ip.h"
#include <varargs.h>
#include <sys/wait.h>
#include <sys/access.h>

/*---------------------------- errstr            ------------------------------
 *
 * NAME: errstr 
 *
 * FUNCTION:
 *		initializes the niminfo.errstr & niminfo.errno fields
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			error_code		= NIM error code
 *			str1			= string to include in error message
 *			str2			= string to include in error message
 *			str3			= string to include in error message
 *		global:
 *			niminfo
 *			protect_errstr	= global set by any calling routine to prevent the
 *									current niminfo.errstr from being overwritten
 *									by any subsequent calls
 *
 * RETURNS:
 *		SUCCESS
 *
 * OUTPUT:
 *----------------------------------------------------------------------------*/

void *
errstr(	int	error_code, 
			char 	*str1, 
			char 	*str2, 
			char 	*str3 )
{
	char *msg;
	char *message;
	int num_chars;
	char *tmp=NULL;

	/* is errstr protected? */
	if ( (protect_errstr) || (error_code <= 0) )
		return( SUCCESS );

	/* 
	 * Obtain the message from the catalog  
	 */
	msg = ERR_msg(error_code);
	if ( msg == NULL )
		msg = ERR_msg( ERR_SYS );

	if ( error_code == ERR_ERRNO )
		str1 = strerror( errno );
	else if ( error_code == ERR_ERRNO_LOG )
		str3 = strerror( errno );

	/* how many chars in msg? */
	num_chars = strlen(msg) + strlen(niminfo.cmd_name) + strlen(str1) +
					strlen(str2) + strlen(str3) + 1;

	/* malloc space for them */
	/* NOTE that niminfo.errstr is not initialized yet because str1, str2, */
	/*		or str3 might actually point to niminfo.errstr - this can happen */
	/* 	when exec_status sets the niminfo.errstr based on output from a */
	/*		method */
	/* THEREFORE, because we can't use niminfo.errstr as both an input and */
	/*		target string for sprintf, we'll create a tmp string first, then */
	/*		copy it in */
	tmp = nim_malloc( num_chars );
	sprintf( tmp, msg, niminfo.cmd_name, str1, str2, str3 );

	/* replace the current errstr with the new buffer */
	if ( niminfo.errstr != NULL )
		free( niminfo.errstr );
	niminfo.errstr = tmp;

	niminfo.errno = error_code;

	return( SUCCESS );

} /* end of errstr */
	
/* --------------------------- nim_log           
 *
 * NAME: nim_log
 *
 * FUNCTION:
 *		logs information to the common NIM_LOG file
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *	invokes the "alog" command, which manages the size of the log
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *	parameters:
 *		str	= information to log
 *		global:
 *
 * RETURNS: (int)
 *		SUCCESS	= string logged
 *		FAILURE	= unable to log
 *
 * OUTPUT:
 * --------------------------------------------------------------------------*/

int
nim_log ( char *str )

{	int	rc=SUCCESS;
	time_t 	timer;
	char 	date[MAX_TMP];
	int 	num_chars;
	char 	message[MAX_TMP];
	FILE 	*fp;

	VERBOSE5("         nim_log: str=%s;\n",str,NULL,NULL,NULL)

	/* 
	 * get the current date & time 
	 */
	if ( (timer = time(NULL)) == -1 )
		rc = FAILURE;
	else if ( strftime(date,MAX_TMP,"%c", localtime(&timer) ) > 0 )
	{	
		sprintf( message,"<%s> <%s> <%d> <", date, niminfo.nim_name, niminfo.pid);

		/* 
		 * need NIM_GLOCK in order to log 
		 */
		if (! niminfo.glock_held )
			rc = get_glock(1,1);

		/* 
		 * log the specified str 
		 */
		if ( niminfo.glock_held )
		{	/* 
			 * open a pipe to ALOG; 
			 * not using odm_run_method here because don't want 
			 * to pass message as command line arg because don't 
			 * want shell to parse the message 
			 */
			if (access("/usr/bin/alog", X_OK) == 0)
			{
				if ( (fp = popen(ALOG, "w")) != NULL ) 
				{
					/* 
				 	* log the message by writing to the pipe 
				 	*/
					fprintf( fp, "%s%s>\n", message, str );
					fclose( fp );
				}
			} else {
				if ( (fp = fopen("/var/adm/nim/nim.error", "a")) != NULL ) 
				{
					/* 
				 	* log the message in the nim error log
				 	*/
					fprintf( fp, "%s%s>\n", message, str );
					fclose( fp );
				}
			}
		}
		/*  
		 * release the NIM_GLOCK if just obtained 
		 */
		if ( niminfo.glock_held )
			rm_glock( FALSE );

	}
	else
		rc = FAILURE;

	return( rc );
} /* end of nim_log */

/*---------------------------- warning      ------------------------------
 *
 * NAME: warning
 *
 * FUNCTION:
 *		displays error messages as warnings
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		RETURNS with specified error code
 *		uses the error policy in niminfo to decide where the error message should
 *			be sent
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			error_code	= NIM error code; when 0, it indicates that the
 *					error information in niminfo has already 
 *					been initialized
 *			str1		= string to include in error message
 *			str2		= string to include in error message
 *			str3		= string to include in error message
 *		global:
 *			niminfo
 *
 * RETURNS:
 *
 * OUTPUT:
 *		writes an error message on stderr
 *----------------------------------------------------------------------------*/

void *
warning( int error_code,
			char *str1,
			char *str2,
			char *str3 )

{	
	char *msg;
	char *warning_msg;
	char *new_msg;

	/* no interruptions */
	disable_err_sig();

	/* set the errstr */
	if ( error_code > 0 )
		errstr( error_code, str1, str2, str3 );

	/* reset the errno - warnings are non-fatal errors */
	niminfo.errno = 0;

	/* prepend the string with the word "warning" */

	/* get warning message */
	warning_msg = MSG_msg( MSG_WARNING );

	/* malloc enough space for new msg */
	new_msg = nim_malloc( strlen(warning_msg) + strlen(niminfo.errstr) + 3 );
	/* initialize new msg */
	sprintf (new_msg, "%s: %s", warning_msg, niminfo.errstr);

	/* replace old msg */
	if ( niminfo.errstr != NULL )
		free( niminfo.errstr );
	niminfo.errstr = new_msg;

	/* display, log, etc. */
	export_errstr( 0, str1, str2, str3 );

	enable_err_sig();

	return( SUCCESS );

} /* end of warning */

/*---------------------------- nene         ------------------------------
 *
 * NAME: nene
 *
 * FUNCTION:
 *		displays the specified error message & RETURNS
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		RETURNS with specified error code
 *		uses the error policy in niminfo to decide where the error message should
 *			be sent
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			error_code	= NIM error code; when 0, it indicates that the
 *					error information in niminfo has already 
 *					been initialized
 *			str1		= string to include in error message
 *			str2		= string to include in error message
 *			str3		= string to include in error message
 *		global:
 *			niminfo
 *
 * RETURNS:
 *
 * OUTPUT:
 *		writes an error message on stderr
 *----------------------------------------------------------------------------*/

void *
nene( va_alist )
va_dcl

{

	va_list ptr; 

	int	error_code; 
	char	*str1, *str2, *str3; 

	va_start(ptr); 

	error_code = va_arg(ptr, int); 
	str1  = va_arg(ptr, char *); 
	str2  = va_arg(ptr, char *); 
	str3  = va_arg(ptr, char *); 
	
	disable_err_sig();
	error_code = export_errstr( error_code, str1, str2, str3 );
	enable_err_sig();
	return( error_code );

} /* end of nene */

/*---------------------------- nim_error         ------------------------------
 *
 * NAME: nim_error
 *
 * FUNCTION:
 *		displays the specified error message & exits
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		exits with specified error code
 *		uses the error policy in niminfo to decide where the error message should
 *			be sent
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			error_code	= NIM error code; when 0, it indicates that the
 *					error information in niminfo has already 
 *					been initialized
 *			str1		= string to include in error message
 *			str2		= string to include in error message
 *			str3		= string to include in error message
 *		global:
 *			niminfo
 *
 * RETURNS:
 *
 * OUTPUT:
 *		writes an error message on stderr
 *----------------------------------------------------------------------------*/

void *
nim_error ( va_alist )
va_dcl

{

	va_list ptr; 

	int	error_code; 
	char	*str1, *str2, *str3; 

	va_start(ptr); 

	error_code = va_arg(ptr, int); 
	str1  = va_arg(ptr, char *); 
	str2  = va_arg(ptr, char *); 
	str3  = va_arg(ptr, char *); 

	/* 
	 * disable error signals 
	 */
	disable_err_sig();

	/* 
	 * go do the error thing and exit
	 */
	exit( export_errstr( error_code, str1, str2, str3 ) );

} 

/*---------------------------- nim_error_plus   ------------------------------ *
* NAME: nim_error_plus
*
* FUNCTION: 	Performs the same job as nim_error by displaying the
*		specified error message and exitting *PLUS* displays
*		an addtional specified message before exitting.
*		(This is basically a wrapper to the functionality of nim_error
*		which allows the return code of the first error to be
*		returned while displaying additional useful information.)
*		
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		calls nene
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES: 
*                      error_code      = NIM error code
*
*                      str1            = string to include in error message
*                      str2            = string to include in error message
*                      str3            = string to include in error message
*                      plus            = additional string to print
*
*              global:
*                      niminfo
* RETURNS: 
*
* OUTPUT:     writes an error message to stderr
*-----------------------------------------------------------------------------*/

void *
nim_error_plus ( va_alist )
va_dcl

{

        va_list ptr;

        int     error_code;
        char    *str1, *str2, *str3, *plus;

        va_start(ptr);

        error_code = va_arg(ptr, int);

        str1  = va_arg(ptr, char *);
        str2  = va_arg(ptr, char *);
        str3  = va_arg(ptr, char *);

        plus  = va_arg(ptr, char *);
 
	nene(error_code, str1, str2, str3);

	niminfo.errstr = plus;
	nene( 0, NULL, NULL, NULL );

	exit( error_code );

} /* end of nim_error_plus */

/*---------------------------- nim_mail
 *
 * NAME: 		nim_mail
 *
 * FUNCTION:	Mail out something for NIM  
 *
 * DATA STRUCTURES:
 *		parameters:
 *			userids = string of user ids to send the mail to
 *			subject	= the subject of the mail
 *			text	= the content of the mail
 *		global:
 *			niminfo
 *
 * OUTPUT: 	writes the mail to a pipe 
 *
 *----------------------------------------------------------------------------*/

int
nim_mail(	char	*userids, 
			char 	*subject, 
			char 	*text)

{
	char	mailCmd[MAX_VALUE]; 
	FILE	*pipe; 

	VERBOSE5("         nim_mail: userids=%s; subject=%s; text=%s;\n",
				userids,subject,text,NULL)

	sprintf(mailCmd,"%s -s \"%s\" %s", MAIL, subject, userids); 
	if ( (pipe = popen(mailCmd, "w")) != NULL ) {
		fprintf (pipe, MSG_msg (MSG_NIM_MAIL), niminfo.cmd_name, niminfo.pid, 
																								text);
		fclose( pipe );
	}	
	return(SUCCESS);
}
	
/*---------------------------- display_errstr    ------------------------------
*
* NAME: display_errstr
*
* FUNCTION:
*		writes niminfo.errstr to stdout
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*		global:
*
* RETURNS: (int)
*		SUCCESS					= info written
*
* OUTPUT:
*		writes niminfo.errstr to stderr
*-----------------------------------------------------------------------------*/

int
display_errstr()

{
	/* 
	 * display the message? 
	 */
	if (	(niminfo.err_policy & ERR_POLICY_DISPLAY) &&
			(niminfo.errstr != NULL ) &&
			(niminfo.errstr[0] != NULL_BYTE) )
		fprintf( stderr, "%s\n", niminfo.errstr );

	/* unprotect the errstr; ie, we just printed it, so allow it to be reset */
	protect_errstr = FALSE;

	return( SUCCESS );

} /* end of display_errstr */

/*---------------------------- export_errstr     ------------------------------
 *
 * NAME: export_errstr
 *
 * FUNCTION:
 *		"exports" niminfo.errstr by: mailing it, logging it, and/or displaying it
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		uses the error policy in niminfo to decide where the 
 *		error message should be sent
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *	parameters:
 *		error_code	= NIM error code; when 0, it indicates that the
 *				error information in niminfo has already 
 *				been initialized
 *		str1		= string to include in error message
 *		str2		= string to include in error message
 *		str3		= string to include in error message
 *	global:
 *		niminfo
 *
 * RETURNS:
 *		value of niminfo.errno
 *
 * OUTPUT: 	writes an error message on stderr
 *----------------------------------------------------------------------------*/

int
export_errstr(	int	error_code, 
					char *str1, 
					char *str2, 
					char *str3 )

{
	/* 
	 * if errstr not already formatted - do so now 
	 */
	if ( error_code > 0 )
		errstr( error_code, str1, str2, str3 );

	/* anything to display? */
	if ( (niminfo.errstr == NULL) || (niminfo.errstr[0] == NULL_BYTE) )
		return( SUCCESS );

	/* 
 	* log the message? 
 	*/
	if (	niminfo.err_policy & ERR_POLICY_LOG )
		nim_log( niminfo.errstr );

	/* 
 	* mail the message? 
 	*/
	if ( niminfo.err_policy & ERR_POLICY_MAIL )
		nim_mail(niminfo.master_uid, MSG_msg (MSG_NIMERR_REPORT), niminfo.errstr);

	/* 
 	* write "rc=" message?
 	*/
	if ( niminfo.err_policy & ERR_POLICY_WRITE_RC )
		fprintf( stderr, "rc=%d\n", niminfo.errno );

	/* 
	 * display the message? 
	 */
	display_errstr();

	/* errstr has been exported, so reset it now (don't want it to */
	/*		accidentally redisplayed) */
	error_code = niminfo.errno;
	niminfo.errno = 0;
	niminfo.errstr[0] = NULL_BYTE;

	return( error_code );

} /* end of export_errstr */
	
/*---------------------------- errsig_handler    ------------------------------
 *
 * NAME: errsig_handler
 *
 * FUNCTION:
 *		performs common error signal handling
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		calls nim_error
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			signum					= signal number
 *		global:
 *			undo_on_interrupt		= ptr to function to execute when interrupt occurs
 *
 * RETURNS: (int)
 *
 * OUTPUT:
 *----------------------------------------------------------------------------*/

int
errsig_handler( int signum )

{	char num[MAX_TMP];

	VERBOSE5("         errsig_handler: signum=%d;\n",signum,NULL,NULL,NULL)

	/* no other interrupts allowed */
	disable_err_sig();

	/* cache signal number */
	sprintf(num,"%d",signum);

	/* send the signal SIGTERM to other members of the process group */ 
	/* iff we are the process group leader.                         */
	if (niminfo.pgrp == niminfo.pid)
		killpg(niminfo.pgrp, SIGTERM); 

	/* undo interrupt function provided? */
	if ( undo_on_interrupt > NULL )
		(* undo_on_interrupt)( ERR_SIGNAL, num, NULL, NULL );

	nim_error( ERR_SIGNAL, num, NULL, NULL );

} /* end of errsig_handler */
	
/*---------------------------- enable_err_sig    ------------------------------
 *
 * NAME: enable_err_sig
 *
 * FUNCTION:
 *		initializes NIM signal handling for common signals
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *		global:
 *			errsig
 *
 * RETURNS: (int)
 *		SUCCESS
 *
 * OUTPUT:
 *----------------------------------------------------------------------------*/

int
enable_err_sig()

{	int rc=SUCCESS;
	int i;

	VERBOSE5("         enable_err_sig\n",NULL,NULL,NULL,NULL)

	/* enable signals & save the old value */
	for (i=0; i <= SIGMAX; i++)
		if ( errsig[i] != NULL )
			signal( i, errsig[i] );

	/* return */
	return( rc );

} /* end of enable_err_sig */

/*---------------------------- disable_err_sig   ------------------------------
 *
 * NAME: disable_err_sig
 *
 * FUNCTION:
 *		disables NIM error signal handling
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *		global:
 *			errsig
 *
 * RETURNS: (int)
 *		SUCCESS
 *
 * OUTPUT:
 *----------------------------------------------------------------------------*/

int
disable_err_sig()

{	int rc=SUCCESS;
	int i;

	VERBOSE5("         disable_err_sig\n",NULL,NULL,NULL,NULL)

	/* disable signals & save original values */
	for (i=0; i <= SIGMAX; i++)
		if ( errsig[i] != NULL )
			errsig[i] = signal( i, SIG_IGN );

	/* return */
	return( rc );

} /* end of disable_err_sig */
