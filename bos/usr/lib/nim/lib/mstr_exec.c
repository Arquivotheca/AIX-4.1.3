static char sccs_id[] = " @(#)93 1.22  src/bos/usr/lib/nim/lib/mstr_exec.c, cmdnim, bos411, 9428A410j  4/20/94  16:30:53";
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: From_Master
 *		LocalExec
 *		getPif
 *		master_exec
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


#include "cmdnim.h"
#include "cmdnim_obj.h"
#include <varargs.h> 
#include "cmdnim_ip.h" 
#include <sys/wait.h> 


/* ---------------------------- rcat 
 *
 * NAME:	rcat
 *
 * FUNCTION:	Given a nim machine name, and a file. Open a connection with 
 *		A remote cat command. 
 *
 * DATA STRUCTURES:
 *  parameters:
 *	Target		--	The nim machine name. 
 *	Pathname	--	The remote pathname to create.
 *	Fp		--	File pointer to the connection.
 *
 *  global: 
 *	niminfo		--	The global nim info structure
 *
 * RETURNS:
 *	FAILURE:  Unable to connect to the remote machine. 
 *
 * -------------------------------------------------------------------------
 */

int
rcat(	char *Target, 
		char *Pathname, 
		FILE **Fp	)

{
	struct nim_if  *ifInfo; 
 	struct servent *servPtr;

	int	rmt_sd;
	char	cmd[MAX_TMP];
	int	on=1;

	VERBOSE4("         rcat: Target=%s; Pathname=%s;\n",Target,Pathname,NULL,
				NULL)

	/* local or remote file? */
	if ( strcmp( Target, niminfo.nim_name ) == 0 )
	{
		/* local file - create it now */
      if ( (*Fp = fopen( Pathname, "w+" )) == NULL )
         ERROR( ERR_ERRNO, NULL, NULL, NULL )

		return( SUCCESS );
	}

	/* 
	 * We can ONLY execute this on the master so if we 
	 * are NOT then error out ! 
	 */
	if ( ( strcmp( niminfo.nim_name, ATTR_MASTER_T ) != 0 )  || 
	     ( strcmp( niminfo.nim_type, ATTR_MASTER_T ) != 0 ) ) {
		errno=EACCES;
		ERROR(ERR_ERRNO_LOG, "rcat", "not master", NULL);
	}	

	/* 
	 * Query the data base for the primary interface
	 */
	if ( (ifInfo=getPif(Target)) == NULL)
		return(FAILURE);

 	/* 
	 * Get the remote shell port number 
	 */
	if ( (servPtr=getservbyname("shell", "tcp")) == NULL ) 
		return(FAILURE);

	/* 
	 * Build the remote cat cmd.
	 */ 
	sprintf(cmd, "%s > %s", CAT, Pathname);

	/* 
	 * Send the request to the rshd via rcmd 
	 */
	rmt_sd=rcmd(&(ifInfo->hostname), servPtr->s_port, "root",
							"root", cmd, NULL);

	/*
	 * If rcmd had a problem with that then error out 
	 */
	if (rmt_sd < 0 )  
		ERROR( ERR_ERRNO_LOG, "rcat", "rcmd", NULL ); 
	
	/*
	 * Set the socket option to add SO_KEEPALIVE (just incase the 
	 * other end goes away.. we need to know these things). 
	 */ 
	if ( setsockopt(rmt_sd, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, 
							sizeof(on) ) == -1 ) 
		ERROR( ERR_ERRNO_LOG, "To_Master", "setsockopt", NULL ); 

	/* 
	 * Make a file pointer. 
	 */ 
	 if ( (*Fp=fdopen(rmt_sd, "a")) == NULL )
		ERROR( ERR_ERRNO_LOG, "rcat", "fdopen", NULL ); 
	/* 
	 * Dont buffer anything, just send it...
	 */
	setbuf( *Fp, NULL );

	return(SUCCESS);

} /* end of rcat */

/* ---------------------------- master_exec 
 *
 * NAME:	master_exec
 *
 * FUNCTION:	Execute a NIM command 		
 * Determine who the request is for.  If the Target is this machine (the 
 * master) then we call client_exec, else we MUST be the master and the 
 * request is for one of our clients so use From_Master.
 *
 * DATA STRUCTURES:
 *		parameters:
 *			Target	--		The NIM name of the target.
 *			Rc			--		The pointer to the returned exit code
 *			Stdout	--		A Pointer to a file pointer that is interpreted thus: 
 *		   	Stdout == NULL;  
 *			     If the Stdout pointer is NULL we do NOT want to see the output 
 *				from the remote command. 
 *
 *		   	*Stdout == NULL;  
 *				  The Stdout pointer points to a file pointer that has been 
 *				initialized to NULL. The function should create and open a temp 
 *				file and pipe 	the remote commands output to this file, rewind 
 *				the file, and pass back the opened file pointer in *Stdout. 
 *			
 *		   	*Stdout > NULL;  
 *				  If the Stdout pointer points to a file pointer that is greater 
 *				than NULL we assume that it points to a file pointer that has 
 *				already been opened (like stdout for this process). The function 
 *				will append to this file and rewind it upon exit. 
 *
 *			Args		--		An array of cmd + arguments to execute.
 *							
 *		global: niminfo
 *
 * RETURNS:
 *	FAILURE:  This is an invalid request for this machine type.
 *
 * -------------------------------------------------------------------------
 */

int
master_exec(	char *Target, int *Rc, FILE **Stdout, char *Args[]	)

{

	VERBOSE4("         master_exec: Target=%s; Args[0]=%s;\n",Target,Args[0],
				NULL,NULL)

	/* 
	 * We can ONLY execute this on the master so if we 
	 * are NOT then error out ! 
	 */
	if ( strcmp( niminfo.nim_name, ATTR_MASTER_T ) != 0 ) {
		errno=EACCES;
		ERROR(ERR_ERRNO_LOG, "master_exec", "not master", NULL);
	}
		
	/* 
	 * If the target is this machine then its a local call (No toll to 
	 * pay...)
	 */
	if ( strcmp( Target, niminfo.nim_name) == 0 )  
		return( client_exec(Rc, Stdout, Args) );
	

	if ( strcmp( niminfo.nim_type, ATTR_MASTER_T ) == 0 ) {
		return( From_Master( Target, Rc, Stdout, Args ) );
	}	
	
	errno=EACCES;
	ERROR(ERR_ERRNO_LOG, "master_exec", "exec", NULL);
}

/*----------------------------- From_Master 
 *
 * NAME:	 From_Master
 *
 * FUNCTION: 	Execute a NIM command on a NIM client.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 * 	Currently this is ONLY used by the NIM MASTER.
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			Target	--		The NIM name of the target.
 *			Rc			--		The pointer to the returned exit code
 *			Stdout	--		A Pointer to a file pointer that is interpreted thus: 
 *		   	Stdout == NULL;  
 *			     If the Stdout pointer is NULL we do NOT want to see the output 
 *				from the remote command. 
 *
 *		   	*Stdout == NULL;  
 *				  The Stdout pointer points to a file pointer that has been 
 *				initialized to NULL. The function should create and open a temp 
 *				file and pipe 	the remote commands output to this file, rewind 
 *				the file, and pass back the opened file pointer in *Stdout. 
 *			
 *		   	*Stdout > NULL;  
 *				  If the Stdout pointer points to a file pointer that is greater 
 *				than NULL we assume that it points to a file pointer that has 
 *				already been opened (like stdout for this process). The function 
 *				will append to this file and rewind it upon exit. 
 *
 *			Args		--		An array of cmd + arguments to execute.
 *							
 *		global: niminfo.
 *
 * RETURNS:
 *		FAILURE: Encountered a problem 
 *		SUCCESS: Exec worked (the remote may have failed check the value of Rc)
 *
 *----------------------------------------------------------------------------*/

int
From_Master( char	*Target, int	*Rc, FILE	**Stdout, char	*Args[] )

{ 
	char 	**argPtr;
	int	argCntr=0;

	int	rmt_err;
	int	rmt_sd;
	int	argNdx;
	int	strSize=0; 

	struct nim_if  *ifInfo; 
 	struct servent *servPtr;
	
	char	*theCmd; 
	int	on=1;

	VERBOSE4("         From_Master: Target=%s; Args[0]=%s;\n",Target,Args[0],
				NULL,NULL)

	/* 
	 * Query the data base for the primary interface
	 */
	if ( (ifInfo=getPif(Target)) == NULL)
		return(FAILURE);

 	/* 
	 * Get the remote shell port number 
	 */
	if ( (servPtr=getservbyname("shell", "tcp")) == NULL ) 
		return(FAILURE);

	/*
	 * size up the total length needed to ship them to 
	 * the master. 
	 */
	argPtr = Args;
	while ( *argPtr != NULL )
		/* strlen of arg + room for surrounding quotes and a trailing blank */
		strSize+=strlen(*argPtr++)+3;

	/* 
	 * Allocate enough space (the final frontier..) for the whole Cmd 
	 * plus the pathname of the NIMPUSH a space and a \0
	 */
	strSize+=strlen(C_NIMPUSH)+1;
	if ( (theCmd=malloc(strSize+1)) == NULL ) 
		ERROR( ERR_ERRNO_LOG, "From_Master", "malloc", NULL ); 
	
	/* 
	 * Build the Cmd line 
	 */
	sprintf(theCmd, "%s ", C_NIMPUSH);
	argPtr = Args;
	while ( *argPtr  != NULL ) {
		/* surround the arg with quotes */
		strcat( theCmd, "\"" );
		strcat( theCmd, *argPtr++ );
		strcat( theCmd, "\" " );
	}	

	/* 
	 * Send the request to the rshd via rcmd 
	 */
	rmt_sd=rcmd(&(ifInfo->hostname),servPtr->s_port,"root","root",theCmd,
						&rmt_err);

	/*
	 * If rcmd had a problem with that then error out 
	 */
	if (rmt_sd < 0 )  
		ERROR( ERR_ERRNO_LOG, "From_Master", "rcmd", NULL ); 
	
	/*
	 * Set the socket option to add SO_KEEPALIVE (just incase the 
	 * other end goes away.. we need to know these things). 
	 */ 
	if ( setsockopt(rmt_sd, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, 
							sizeof(on) ) == -1 ) 
		ERROR( ERR_ERRNO_LOG, "To_Master", "setsockopt", NULL ); 


	/* 
	 * Collect the remote status 
	 */ 
	exec_status(rmt_sd, rmt_err, Rc, Stdout, 0);

	return(SUCCESS);

} /* end of From_Master */

/*----------------------------- LocalExec 
 *
 * NAME:	LocalExec
 *
 * NOTES:  THIS IS ONLY USED BY NIMESIS... 
 *
 * FUNCTION: 	Execute a local command.
 *  	Call this function like thus: 
 * 	LocalExec( char *CmdArgs[0..NULL] )
 *
 * DATA STRUCTURES:
 *
 *  parameters:
 * 	Args		-	An array of the command and its arguments
 *
 * EXIT: This function will exit if unable to fork a child.. 
 *
 * RETURNS: The kid process id.
 *
 *----------------------------------------------------------------------------*/

int
LocalExec (char *Args[] )
{ 
	int	cmdPid; 

	VERBOSE4("         LocalExec: Args[0]=%s;\n",Args[0],NULL,NULL,NULL)

	if ( (cmdPid=fork()) < 0 ) 
		ERROR(ERR_ERRNO_LOG, "LocalExec", "fork", NULL); 
	

	if ( cmdPid > 0 )
		return(cmdPid);
	/* 
	 * --------------  Child Process Continues from here --------------
	 */

	execvp(	Args[0], Args );
	/* 
	 * If the execvp returns then we are unable to execute the command so 
	 * write a note of that fact and exit.. 
	 */
	nim_error(ERR_ERRNO_LOG, "LocalExec", "execvp", NULL);	
}
