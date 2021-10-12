static char     sccs_id[] = " @(#)96 1.26  src/bos/usr/lib/nim/lib/nim_exec.c, cmdnim, bos411, 9437A411a  9/10/94  17:21:48";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: To_Master
 *		client_exec
 *		exec_status
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
#include <sys/stat.h>

/*----------------------------- exec_status 
 *
 * NAME:	 exec_status
 *
 * FUNCTION: 	Look for any output from the NIM execution
 *
 * NOTES:
 * If the value of *Stdout is NULL user wants us to build tmp file and 
 * return an open file pointer. If the value of *Stdout is > 0 user has 
 * already opened a file so we spool output to that file. 
 *
 * If Stdout is NULL then user does not want to see the standard output.
 * If Rc is NULL then user does not want to see the standard error.
 *
 * DATA STRUCTURES:
 *		parameters:
 *			rmt_sd  -	A file(socket) descriptor for stdout.
 *			rmt_err -	A file(socket) descriptor for stderr.
 *			Rc      -	A pointer to the return code int var (we place the 
 *							return code is returned)
 *			Stdout  -	A Pointer to a file pointer that is interpated thus: 
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
 * RETURNS:
 *
 *----------------------------------------------------------------------------*/
int
exec_status(	int 	rmt_sd, 
	 	int 	rmt_err, 
		int 	*Rc, 
		FILE 	**Stdout,
		int	wait_for_this_pid )

{

	fd_set	inset, rdyset; 
	int	max_fd; 
	int	ret; 

	char	*alloc_ptr, *end_ptr;
	int	min_rc_size; 
	int	errstr_size, rc, wc; 
	int	read_size=0, buff_size=0 ;
	int	msg_offset=0;

	char	gen_buffer[MAX_NIM_PKT*2]; 
	char	tmp_name[MAX_TMP]; 
	int	tmp_fd = 0; 

 	union wait kidStat;

	regmatch_t rc_info[ ERE_RC_NUM ];
	int local_rc;
	int incre = 0;

	FD_ZERO(&inset);

	/* if Rc wasn't passed in, use a local variable */
	if ( Rc == NULL )
		Rc = &local_rc;
	*Rc = 0;

	/* 
	 * stdout to be intercepted ? 
	 */
	if (Stdout) {
		/*
		 * Did caller supply an already opened file pointer ?
		 */ 
		if ( *Stdout != NULL ) {
			/* 
			 * Get the file descriptor 
			 */
			if ( (tmp_fd = fileno(*Stdout)) == -1 )
				ERROR( ERR_ERRNO, NULL, NULL, NULL )
		} 
		else { 
			/*
			 * Caller wants us to create a tmp file.
			 */
			sprintf(tmp_name, "%s/nx.%ld", niminfo.tmps, time(NULL));

			VERBOSE4("      exec_status: opening %s\n",tmp_name,NULL,NULL,NULL)

			if ( (tmp_fd = open(	tmp_name,
											(O_RDWR|O_CREAT),(S_IREAD|S_IWRITE)) ) == -1 ) 
				ERROR( ERR_ERRNO_LOG, "exec_status", "open", NULL )
			*Stdout=fdopen(tmp_fd, "r");
		}
	 	FD_SET(rmt_sd, &inset);
	}
	else 
		if ( rmt_sd != -1 ) {
			tmp_fd = STDOUT_FILENO;
	 		FD_SET(rmt_sd, &inset);
		}

	/*
	 * we always collect the stderr set the bit.
	 */ 
	FD_SET(rmt_err, &inset);
	read_size = MAX_NIM_PKT;
	buff_size = read_size;
	alloc_ptr = nim_malloc(MAX_NIM_PKT+1);
	end_ptr = alloc_ptr; 
	min_rc_size = strlen(ERR_RETURN_TOKEN)+(3*sizeof(char));

   /* 
    * Calc and set the max fd 
    */
   max_fd = ( (rmt_sd > rmt_err) ? rmt_sd : rmt_err);

   /* 
    * The select deals with the NUMBER of file descriptors. As we start 
    * at zero we'll need to bump max_fd by one.. 
    */
   max_fd++;

	while ( (rmt_sd != -1 && FD_ISSET(rmt_sd, &inset)) ||
							FD_ISSET(rmt_err, &inset) ) {

		rdyset = inset;
			
		if (select( max_fd, &rdyset, 0, 0, NULL) < 0 ) 
			if ( errno != EINTR ) {
				ERROR(ERR_ERRNO, "Select in exec_status", NULL, NULL);
			}
			else
				continue;
		
		if ( rmt_sd != -1 && FD_ISSET( rmt_sd, &rdyset) ) {
			if ( (rc=read(rmt_sd, gen_buffer, (MAX_NIM_PKT*2)-1 )) > 0 ) 
				write(tmp_fd, gen_buffer, rc); 
			else {
				if ( rc < 0 )
					ERROR(ERR_ERRNO, "reading stdout", NULL, NULL);
				/* 
 				 * stop collecting at EOF ! 
 				 */
				close(rmt_sd);
				FD_CLR( rmt_sd, &inset );
			}			
		}

		if (FD_ISSET(rmt_err, &rdyset) ) {
			if ( read_size <= 0 ) {
				/* 
				 * resize the buffer
				 */
				alloc_ptr = nim_realloc( alloc_ptr, buff_size, MAX_NIM_PKT );
				read_size = MAX_NIM_PKT;
				end_ptr = alloc_ptr+buff_size; 
				buff_size += MAX_NIM_PKT;	
		 	} 
			if ( (rc=read(rmt_err, end_ptr, read_size)) > 0 ) {
				read_size-=rc;
				end_ptr+=rc;
			}
			else {
				if ( rc < 0 )
					ERROR( ERR_ERRNO, "reading stderr", NULL, NULL);
				close(rmt_err);
				FD_CLR( rmt_err, &inset );
			}
		}	
	}

	/* what's the exit code from the command we just executed? */
	if (	(wait_for_this_pid) && 
			((ret=waitpid(wait_for_this_pid, &kidStat, 0)) > 0) && 
			(ret) )
		*Rc=WEXITSTATUS(kidStat);

	/* was "rc=[0-9]" written to stderr? */
	if (	(regexec(	nimere[ RC_ERE ].reg, alloc_ptr, 
							ERE_RC_NUM, rc_info, 0 ) == 0) &&
			(rc_info[1].rm_so >= 0) )
	{
		/* use the exit code written to stderr */
		if ( alloc_ptr[ rc_info[1].rm_eo ] == NULL_BYTE )
			incre = rc_info[1].rm_eo;
		else
		{
			alloc_ptr[ rc_info[1].rm_eo ] = NULL_BYTE;
			incre = rc_info[1].rm_eo + 1;
		}
		*Rc = (int) strtol( (alloc_ptr + rc_info[1].rm_so), NULL, 0 );
		alloc_ptr += incre;
	}

	if (!protect_errstr && (alloc_ptr[0] != NULL_BYTE) ) {
		if ( niminfo.errstr != NULL )
			free(niminfo.errstr); 
		niminfo.errstr=alloc_ptr; 
		/* look for warnings */
		if (*Rc == 0) 
			nene(0, NULL, NULL, NULL);
	}
		
	VERBOSE4("      exec_status: Rc=%d;\n",*Rc,NULL,NULL,NULL)

	if (Stdout) 
		rewind(*Stdout);
	return(SUCCESS);

} /* end of exec_status */

/* --------------------------- client_exec
 *
 * NAME: client_exec
 *
 * FUNCTION:
 *		executes the specified program
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		sets errstr on failure
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			Rc			- 	ptr to int (for return code of method )
 *			Stdout	- 	ptr to FILE ptr (if you want the method's stdout)
 *			Stdout	-	A Pointer to a file pointer that is interpated thus: 
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
 *			Args		-	An array of cmd + arguments to execute.
 *
 * RETURNS: (int)
 *		SUCCESS					= operation successful
 *		FAILURE					= method returned an error or client_exec error
 *
 * ----------------------------------------------------------------------------
 */

int
client_exec(	int *Rc,
					FILE **Stdout,
					char *Args[] )

{	union wait kidStat;
	int	cmdPid=0;
	int	cmdRc;
	int	rc;
	int	stdOut_pipe[2] = { -1, -1 };
	int	stdErr_pipe[2];
	struct stat status;

	VERBOSE4("      client_exec: Args[0]=%s;\n",Args[0],NULL,NULL,NULL)

	/* 
	 * bail out if no rc address specified
	 */
	if (Rc==NULL)
		return(FAILURE);

	/* 
	 * make sure method is executable 
	 */
	if (	(stat( Args[0], &status ) == -1) || ( ! (status.st_mode & S_IXUSR) ) )
		ERROR( ERR_FILE_MODE, MSG_msg(MSG_EXECUTE), Args[0], NULL );

	/* 
	 * If sdtOut wanted do the pipe jive 
	 */
	if (Stdout != NULL) {
		if ( pipe(stdOut_pipe) < 0 ) {
			ERROR(ERR_ERRNO_LOG, "client_exec", "pipe stdout", NULL);
		}
	}

	/*
	 * Always do the stderr capture...
	 */
	if ( pipe(stdErr_pipe) < 0 ) {
		ERROR(ERR_ERRNO_LOG, "client_exec", "pipe stdErr", NULL);
	}

	/* 
	 * -- Fork a new process --
	 */
	if ( (cmdPid=fork()) < 0 ) 
		ERROR(ERR_ERRNO_LOG, "client_exec", "fork", NULL); 

	if ( cmdPid == 0 ) {
		/* 
	 	 * --------------  Child Process Continues from here --------------
	 	 */
		if (Stdout != NULL) {
			if (dup2(stdOut_pipe[1], 1) != 1)  
				ERROR(ERR_ERRNO_LOG, "client_exec", "dup stdout", NULL);
			close( stdOut_pipe[0] );
			close( stdOut_pipe[1] );
		}

		if (dup2(stdErr_pipe[1], 2) != 2) 
			ERROR(ERR_ERRNO_LOG, "client_exec", "dup stderr", NULL);
		close( stdErr_pipe[0] );
		close( stdErr_pipe[1] );

		execvp(	Args[0], Args );
		/* 
	 	 * If the execvp returns then we are unable to execute the command so 
	 	 * write a note of that fact and  ** EXIT ** 
	 	 */
		nim_error(ERR_ERRNO_LOG, "client_exec", "execvp", NULL);	
		/* 
	 	 * --------------  Child Process ENDS here --------------
	 	 */
	}

	if (Stdout != NULL) 
		close( stdOut_pipe[1] );

	close( stdErr_pipe[1] );

	rc = exec_status(stdOut_pipe[0], stdErr_pipe[0], Rc, Stdout, cmdPid);

	return( rc );

} /* end of client_exec */


/*----------------------------- To_Master 
 *
 * NAME:	 To_Master
 *
 * FUNCTION:
 *	Execute a NIM command on a NIM MASTER.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 *		parameters:
 *
 *		Target	- 	A nim name. The target of this command.
 *		Rc			- 	A pointer to an int for the return code to be placed in.
 *		Stdout	-	A Pointer to a file pointer that is interpated thus: 
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
 *		nargs			- Number of arguments.
 *		Args			- An array of arguments.
 *
 * RETURNS:
 *		FAILURE:  Unable to get pif or malloc or rcmd port number.
 *		SUCCESS:  The call worked.
 *
 *----------------------------------------------------------------------------*/

To_Master( 	char *Target, 
		int  *Rc, 
		FILE **Stdout, 
		int  nargs, 
		char *Args[]  )


{	
	int		rmt_err;
	int		rmt_sd;
	int		argNdx;
	int		actual_size;
	char		gen_buffer[MAX_NIM_PKT]; 
	char		*bufPtr; 
	int		on=1;

	VERBOSE4("      To_Master: Target=%s; Args[0]=%s;\n",Target,Args[0],NULL,
				NULL)

	memset(gen_buffer, 0, MAX_NIM_PKT);
	/* 
	 * Send a pucker-up request to the NIM Master via rcmd (using NIM 
	 * well known port)
	 */
	rmt_sd=rcmd( &(niminfo.master.hostname), niminfo.master_port, 
		"root", "root", niminfo.nim_name, &rmt_err );

	/*
	 * If rcmd had a problem then error out 
	 */
	if (rmt_sd < 0 )  
		ERROR( ERR_ERRNO_LOG, "To_Master", "rcmd", NULL ); 
        
	/*
	 * Set the socket option to add SO_KEEPALIVE (just incase the 
	 * other end goes away.. we need to know these things). 
	 */ 
	if ( setsockopt(rmt_sd, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, 
							sizeof(on) ) == -1 ) 
		ERROR( ERR_ERRNO_LOG, "To_Master", "setsockopt", NULL ); 

	/*
	 * Now we have an active connection so build and then 
	 * send the NIM packet.
	 */
	sprintf( gen_buffer, "%s\n%s\n%s\n", niminfo.nim_name, 
		 niminfo.cpu_id, (Stdout == NULL) ? "No" : "Yes");

	actual_size = strlen(gen_buffer); 

	/* 
	 * Make the list of args like rcmd by making the \n into \0 this is
	 * so the remote parser has to simply look for \0.. 
	 */
	bufPtr=gen_buffer;
	while ( *bufPtr ) {
		if (*bufPtr == '\n' )
			*bufPtr = '\0';
		bufPtr++;
	}
	
	if ( write(rmt_sd, gen_buffer, actual_size ) < 0 ) {
		nene( ERR_ERRNO_LOG, "To_Master", "write pkt", NULL ); 
		close(rmt_sd); 
		return(FAILURE);
	}

	/* 
	 * Now we have an established connection process the args 
	 * that have been passed to us. 
	 */
	
	/* 
	 * Tell the remote server how many args follow
	 */
	sprintf(gen_buffer, "%d", nargs);
	if ( write(rmt_sd, gen_buffer, strlen(gen_buffer)+1 ) < 0 ) {
		nim_error( ERR_ERRNO_LOG, "To_Master", "write nargs", NULL ); 
		close(rmt_sd); 
		return(FAILURE);
	}

	/* 
	 * Now send the arguments 
	 */ 
	for ( argNdx=0;  argNdx < nargs; argNdx++ ) {
		if ( write(rmt_sd, Args[argNdx], strlen(Args[argNdx])+1 ) < 0 ) {
			nene( ERR_ERRNO_LOG, "To_Master write arg", Args[argNdx], NULL ); 
			close(rmt_sd); 
			return(FAILURE);
		}
	}

	/* 
	 * Collect the remote status 
	 */ 
	exec_status(rmt_sd, rmt_err, Rc, Stdout, 0);

	close(rmt_err);

	return(SUCCESS);
}
