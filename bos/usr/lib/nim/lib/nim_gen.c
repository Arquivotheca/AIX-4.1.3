static char	sccs_id[] = " @(#)95 1.47  src/bos/usr/lib/nim/lib/nim_gen.c, cmdnim, bos411, 9428A410j  6/2/94  14:10:13";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: add_attr_ass
 *		add_to_LIST
 *		free_list
 *		get_glock
 *		get_list_space
 *		get_nim_env
 *		nim_exit
 *		nim_init
 *		nim_init_var
 *		nim_malloc
 *		nim_siginit
 *		parse_attr_ass
 *		pop_xop
 *		push_xop
 *		query_glock
 *		reset_LIST
 *		rm_glock
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

/*---------------------------- nim_malloc        ------------------------------
 *
 * NAME: nim_malloc
 *
 * FUNCTION:
 *		malloc's the specified amount of space, initializes it with NULL_BYTEs,
 *			and returns a ptr to it
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		calls nim_error on malloc failure
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			size	= num bytes to malloc
 *		global:
 *
 * RETURNS: (void *)
 *
 * OUTPUT:
 *---------------------------------------------------------------------------*/

void	*
nim_malloc( int size )

{	
	void	*space;

	VERBOSE4("         nim_malloc: size = %d\n",size,NULL,NULL,NULL)

	if ( (space = malloc( size )) == NULL )
		nim_error( ERR_ERRNO, NULL, NULL, NULL );

	/* initialize the space to NULL_BYTEs */
	memset( space, NULL_BYTE, size );

	/* return */
	return( space );

} /* end of nim_malloc */

/*---------------------------- nim_realloc        ------------------------------
 *
 * NAME: nim_realloc
 *
 * FUNCTION:
 *		uses realloc to get additional memory
 *		NOTE that you specify the "additional" memory you want - this is done
 *			so that this function understands what part of the new memory to
 *			initialize to NULL
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		calls nim_error on realloc failure
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			ptr					= ptr to originally malloc'd memory
 *			cur_size				= current size of <ptr>
 *			additional			= additional number of bytes to allocate
 *		global:
 *
 * RETURNS: (void *)
 *
 * OUTPUT:
 *---------------------------------------------------------------------------*/

void	*
nim_realloc(	void *ptr,
					int cur_size,
					int additional )

{	int total;
	void	*space;

	/* realloc wants the total size for the new space, so... */
	additional += 1;
	total = cur_size + additional;

	VERBOSE4("         nim_realloc: total size = %d\n",total,NULL,NULL,NULL)

	/* realloc the memory */
	if ( (space = realloc( ptr, total )) == NULL )
		nim_error( ERR_ERRNO, NULL, NULL, NULL );

	/* initialize just the new part of the space to NULL_BYTEs */
	ptr = (int) space + cur_size;
	memset( ptr, NULL_BYTE, additional );

	/* return */
	return( space );

} /* end of nim_malloc */

/* --------------------------- push_xop          
 *
 * NAME: push_xop
 *
 * FUNCTION:
 *		"push"s the specified operation onto the stack of exit operations
 *
 * DATA STRUCTURES:
 *  parameters:
 *	key	= optional; string to associate with the command if 
 *			  not used MUST be NULL.
 *	fmt	= The format for the command line (eg, "%s %d %s" ... )	
 *	Vargs 	= Supporting varible information that matches the fmt string. 
 *
 *  global:
 *	niminfo.xops	= name of file which contains exit operations
 *				for this command
 *
 * RETURNS: (int)
 *		SUCCESS	= specified command added to the xop file
 *		FAILURE	= unable to add the command
 * ---------------------------------------------------------------------------*/

int
push_xop( va_alist )	
va_dcl 	

{	
	va_list	ptr; 
	char	*fmt; 
	char	*key;
	char	key_fmt[MAX_TMP]; 
	char	*xop_fmt;
	FILE 	*xops;
	
	/* open the xops file */
	if ( (xops = fopen(niminfo.xops, "a")) == NULL )
		return( FAILURE );

	VERBOSE4("         push_xop: xops file = %s;\n",niminfo.xops,NULL,NULL,NULL)

	va_start(ptr); 

	key = va_arg(ptr, char *);
	fmt = va_arg(ptr, char *);	

	if (key == NULL)
		strcpy( key_fmt, " # #\n");
	else 
		sprintf( key_fmt, " # %s #\n", key);

	xop_fmt = (char *) nim_malloc( strlen(key_fmt)+strlen(fmt)+1 );
	strcpy(xop_fmt, fmt);
	strcat(xop_fmt, key_fmt);

	/* 
	 * add the specified command text to the file 
	 */
	vfprintf( xops, xop_fmt, ptr );

	fclose( xops );

	return( SUCCESS );

} 

/* --------------------------- pop_xop           ------------------------------
 *
 * NAME: pop_xop
 *
 * FUNCTION:
 *		retrieves exit operations from the xop file; the retrieved operation(s)
 *			can either be executed or thrown away
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			key			= string to match operations on; a value of NULL will
 *									match the last operation to be pushed 
 *			ignore		= a 1 indicates that matched operations are
 *									NOT to be executed
 *		global:
 *
 * RETURNS: (int)
 *		SUCCESS
 *
 * OUTPUT:
 * --------------------------------------------------------------------------
 */

int
pop_xop(	char *key, 
			int ignore )

{
	int	rc = SUCCESS;
	char	s_ignore[MAX_TMP];
	char	*Args[] = { C_POPXOP, niminfo.xops, key, s_ignore, NULL };

	VERBOSE4("         pop_xop: key = %s; ignore = %d;\n",key,ignore,NULL,NULL)

	sprintf( s_ignore, "%d", ignore );

	/* 
	 * exec the popxop method 
	 */
	client_exec( &rc, NULL, Args );

	return( rc );

} /* end of pop_xop */

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ startup & exit processing                         $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

/*---------------------------- nim_exit         -------------------------------
 *
 * NAME: nim_exit
 *
 * FUNCTION:
 *		performs exit processing which is common across all NIM commands
 *
 * EXECUTION ENVIRONMENT:
 *		this is called when the "exit" system call is executed
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			NONE
 *		global:
 *
 * RETURNS:
 *
 *---------------------------------------------------------------------------*/

void	*
nim_exit ()

{
	int	rc;
	char	*Args[] = { RM, "-r", niminfo.tmps, NULL };
	int	no_kids; 
	pid_t	wait_stat; 

	VERBOSE4("         nim_exit\n",NULL,NULL,NULL,NULL)

	disable_err_sig();

	/* 
	 * perform any existing exit operations
	 */
	pop_xop( "all", FALSE );

	signal( SIGCHLD, SIG_DFL );
	/*
	 * WAIT on all children of this process 
	 */ 
	while ( wait(&no_kids) != -1 )
		     ;

	/* 
	 * release any locks held by this process 
	 */
	rm_glock( FALSE );

	/* 
	 * remove this process's tmp directory 
	 */
	client_exec( &rc, NULL, Args );

	/* 
	 * close the message catalog 
	 */
	catclose( niminfo.msgcat_fd );

} /* end of nim_exit */

/*---------------------------- nim_siginit       ------------------------------
 *
 * NAME: nim_siginit
 *
 * FUNCTION:
 *	initializes the errsig array with pointers to the errsig_handler
 *	function, which is provided for common NIM signal processing
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			NONE
 *		global:
 *			errsig
 *
 * RETURNS: (int)
 *		SUCCESS					= x
 *		FAILURE					= x
 *
 * OUTPUT:
 *---------------------------------------------------------------------------*/

int
nim_siginit()

{	
	int	rc = SUCCESS;

	extern int	errsig_handler();

	VERBOSE4("         nim_signinit\n",NULL,NULL,NULL,NULL)

	/* 
	 * common NIM error signal processing 
	 */
	errsig[SIGINT] = errsig_handler;
	errsig[SIGQUIT] = errsig_handler;
	errsig[SIGABRT] = errsig_handler;
	errsig[SIGALRM] = errsig_handler;
	errsig[SIGSEGV] = errsig_handler;
	errsig[SIGSYS] = errsig_handler;
	errsig[SIGTERM] = errsig_handler;
	 
	errsig[SIGTTIN] = SIG_IGN;
	errsig[SIGTTOU] = SIG_IGN;
	
	return( rc );

} /* end of nim_siginit */

/*---------------------------- get_nim_env       ------------------------------
 *
 * NAME: get_nim_env
 *
 * FUNCTION:
 *	initializes the niminfo structure with information from NIM_ENV_FILE
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		calls nim_error if NIM_ENV_FILE cannot be accessed
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			NONE
 *		global:
 *			niminfo
 *
 * RETURNS: (int)
 *		SUCCESS	= success
 *
 * OUTPUT:
 *---------------------------------------------------------------------------*/

int
get_nim_env()

{	
	FILE * fp;
	char	line[MAX_VALUE];
	char	*ptr, *i;
	int	count;
	int	must_have=0; 
	char	name[MAX_ENV_NAME];
	int	len;

	VERBOSE4("         get_nim_env: reading from %s\n",NIM_ENV_FILE,NULL,NULL,
				NULL)

	/* look in the NIM_ENV_FILE for the rest of the info */
	if ( (fp = fopen(NIM_ENV_FILE, "r")) == NULL ) 
	{
		/* if the error is no entry try looking in the spot */ 
		if ( errno != ENOENT )
			nim_error( ERR_NIM_ENV_FILE, NULL, NULL, NULL );

		if ( (fp = fopen(SPOT_ENV_FILE, "r")) == NULL ) 
			nim_error( ERR_NIM_ENV_FILE, NULL, NULL, NULL );
	}
			
	/* each line in this file should contain a variable assignment */
	while ( fgets(line, MAX_VALUE, fp) != NULL ) 
	{	/* separate name from value */
		if ( (ptr = strchr(line, '=')) != NULL ) 
		{	/* init the variable name */
			for (i = line, count = 0; 
			    (i < ptr) && (count < MAX_ENV_NAME); i++, count++)
				name[count] = *i;
			name[count] = '\0';

			/* ptr+1 points to value */
			/* remove the trailing '\n' also */
			ptr++;
			len = strlen( ptr );
			ptr[ len - 1 ] = '\0';


			/* does name match any NIM environment variable? */
			if ( strcmp(name, NIM_NAME) == 0 ) 
			{
				strncpy( niminfo.nim_name, ptr, MAX_NAME_BYTES );
				must_have++; 
			}
			else if ( strcmp(name, NIM_CONFIGURATION) == 0 )
				strncpy( niminfo.nim_type, ptr, MAX_NAME_BYTES );
			else if ( strcmp(name, NIM_MASTER_HOSTNAME) == 0 ) 
			{	/* malloc enough space to hold the hostname */
				if ( (niminfo.master.hostname = malloc(len)) == NULL )
					nim_error( ERR_SYS, "malloc", NULL, NULL );
				strncpy( niminfo.master.hostname, ptr, len );
				must_have++;
			} 
			else if ( strcmp(name, NIM_MASTER_UID) == 0 )
				strncpy( niminfo.master_uid, ptr, MAX_NAME_BYTES );
			else if ( strcmp(name, NIM_MASTER_PORT) == 0 ) 
			{
				niminfo.master_port = atoi( ptr );
				must_have++;
			}
			else if ( strcmp(name, NIM_COMM_RETRIES) == 0 )
				niminfo.nimcomm_retries = atoi( ptr );
			else if ( strcmp(name, NIM_COMM_DELAY) == 0 )
				niminfo.nimcomm_delay = atoi( ptr );
			else if ( strcmp(name, NIM_BOOTP_ENABLED) == 0 )
				niminfo.bootp_enabled = atoi( ptr );
			else if ( strcmp(name, NIM_USR_SPOT) == 0 )
				niminfo.usr_spot_server = atoi( ptr );
		}
	}
	/* nim_error if certain fields aren't defined */
	if ( must_have != 3 )
		nim_error(ERR_NIM_ENV_MISSING, NULL, NULL, NULL);
	/* return */
	return( SUCCESS );

} /* end of get_nim_env */

/*---------------------------- nim_init_var
 *
 * NAME: nim_init_var
 *
 * FUNCTION:	Performs initialization of the variable part of niminfo
 *
 * NOTES: 	This function initializes the global NIM information structure
 *		portions that have a variable content based on process id.
 *
 *
 * DATA STRUCTURES:
 *		parameters:
 *			char *cmd_name		The command name to use... 
 *		global:
 *			niminfo
 *
 * EXITS:
 *		This function will exit on encountering an error.
 *
 *----------------------------------------------------------------------------*/

void
nim_init_var(	char *cmd_name )

{	char tmp[MAX_TMP];

	/*  
	 * First set up the current process ids 
	 */
	niminfo.pid = getpid();
	niminfo.pgrp = getpgrp();
	sprintf( tmp, "/%d", niminfo.pid );

	VERBOSE4("         nim_init_var: cmd_name = %s; pid = %d; pgrp = %d\n",
				cmd_name,niminfo.pid,niminfo.pgrp,NULL)

	/* 
	 * set the cmd name 
	 */
	niminfo.cmd_name = cmd_name;

	/* 
	 * Build the xop pathname 
	 */
	niminfo.xops = nim_malloc( strlen(NIM_XOP_DIR) + strlen(tmp) + 1 );
	sprintf( niminfo.xops, "%s%s", NIM_XOP_DIR, tmp );

	/* 
	 * Now build a directory pathname for tmp files for this process 
	 */
	niminfo.tmps = nim_malloc( strlen(NIM_VAR_DIR) + strlen(tmp) + 1 );
	sprintf( niminfo.tmps, "%s%s", NIM_VAR_DIR, tmp );

	/* 
	 * Now make the directory. 
	 */
	if ( mkdir(	niminfo.tmps, 
					S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1 )
		if (errno != EEXIST)
			nim_error( ERR_ERRNO, NULL, niminfo.tmps, NULL );
}

/*---------------------------- nim_init          ------------------------------
 *
 * NAME: nim_init
 *
 * FUNCTION:
 *		performs common initialization processing for all NIM commands
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *	this function initializes the global NIM variables which are defined
 *	in each NIM command; search for the string "NIM globals" in the
 *	command .c file to find their definition
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *	parameters:
 *		name		= command name
 *		err_policy	= a #define from cmdnim_defines.h which specifies
 *											how to treat errors
 *		siginit		= when non-NULL, points to function which initializes
 *					  signal handling; otherwise, nim_siginit will be used
 *		exit_function		= function to call upon exit; 2 choices currently:
 *								nim_exit 	- common exit processing
 *								master_exit - exit processing for the master
 *		global:
 *			niminfo
 *			errsig
 *
 * RETURNS: (int)
 *		SUCCESS					= success
 *		FAILURE					= failure
 *
 * OUTPUT:
 *----------------------------------------------------------------------------*/

int
nim_init(	char *name, 
				unsigned int err_policy, 
				void (*siginit)(), 
				void (*exit_function)() )

{

	static char	base_name[MAX_TMP];
	char	*ptr;

	struct utsname uname_info;
	int	rc;
	int	i;
	char	tmp[MAX_TMP];

	/* 
	 * internationalization 
	 */
	setlocale( LC_ALL, "" );

	VERBOSE4("         nim_init: name = %s;\n",name,NULL,NULL,NULL)

	/* 
	 * initialize the niminfo struct 
	 */
	memset( &niminfo, '\0', sizeof(struct nim_info ) );

	/* alloc space for errstr */
	niminfo.errstr = malloc( MAX_NIM_ERRSTR + 1 );
	memset( niminfo.errstr, 0, MAX_NIM_ERRSTR+1 );

	/* 
	 * Reset the glock flag 
	 */
	niminfo.glock_held = FALSE;

	/* 
	 * Ensure we ahve the linked list of mount points NULL'ed 
	 */
	niminfo.mounts = NULL;

	/* 
	 * error policy controls behaviour of nim_error 
	 */
	niminfo.err_policy = err_policy;

	setbuf( (FILE * ) stdout, NULL);
	setbuf( (FILE * ) stderr, NULL);
	/* 
	 * make sure that an exit routine is called upon exit 
	 */
	if ( exit_function == NULL )
		exit_function = (void *) nim_exit;
	atexit( exit_function );

	/* 
  	 * command name (from argv[0]) is really just the basename.
	 */
	if ( (ptr = strrchr(name, '/')) == NULL )
		ptr = name;
	else
		ptr++;
	strncpy(base_name, ptr, MAX_TMP);

	/* 
	 * set the umask to 0x022 
	 */
	umask( S_IWGRP | S_IWOTH );

	/* 
	 * Do the variable init 
	 */
	nim_init_var( base_name );

	/* 
	 * initialize errsig 
	 */
	memset( errsig, NULL_BYTE, sizeof(void *) * SIGMAX );
	if ( siginit == NULL )
		nim_siginit();
	else
		(*siginit)();

	/* 
	 * enable signals (uses the errsig array) 
	 */
	enable_err_sig();

	/* 
	 * initialize the regular expression patterns 
	 */
	for (i = 0; i < MAX_NIM_ERE; i++)
	{	/*
		 * allocate space for the regex_t struct 
		 */
		if ( (nimere[i].reg = malloc(sizeof(regex_t))) == NULL )
			nim_error( ERR_SYS, "malloc", strerror(errno), NULL );

		/* compile the regulare expressions */
		if ( (rc = regcomp(	nimere[i].reg, nimere[i].pattern, 
									REG_EXTENDED)) != 0)
		{
			regerror( rc, nimere[i].reg, tmp, MAX_TMP );
			nim_error( ERR_SYS, "recomp", tmp, NULL );
		}
	}

	/* 
	 * this machine's cpu id 
	 */
	if ( uname(&uname_info) > -1 )
		strcpy( niminfo.cpu_id, uname_info.machine );

	/* open the cmdnim message catalog; MF_CMDNIM is generated automatically */
	/* when the catalog is built and is specified in the cmdnim_msg.h file   */
	niminfo.msgcat_fd = catopen(MF_CMDNIM, NL_CAT_LOCALE);

	/* 
	 * initialize niminfo with info from NIM_ENV_FILE 
	 */
	get_nim_env();

	return( SUCCESS );

} /* end of nim_init */

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ global lock manipulation                          $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

/*---------------------------- query_glock       ------------------------------
 *
 * NAME: query_glock
 *
 * FUNCTION:
 *		queries the NIM_GLOCK file & returns a value based on the query
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			NONE
 *		global:
 *			NIM_GLOCK
 *
 * RETURNS: (int)
 *		SUCCESS	= the NIM_GLOCK file does not exist (no locks)
 *		OK			= NIM_GLOCK exists and the current process owns the lock
 *		FAILURE	= couldn't access NIM_GLOCK -or- current process 
 *			  				doesn't own the lock
 *
 * OUTPUT:
 *---------------------------------------------------------------------------*/

int
query_glock()

{
	int	rc;
	FILE 	 * fp;
	char	tmp[MAX_TMP];
	pid_t lock_id;
	int	process_id;

	VERBOSE4("         query_glock\n",NULL,NULL,NULL,NULL)

	/* attempt to open the NIM_GLOCK file */
	if ( (fp = fopen(NIM_GLOCK, "r")) == NULL )
	{	/* check the errno */
		if ( errno == ENOENT )
			/* it's ok - the file doesn't exist */
			return( SUCCESS );

		/* some other problem */
		ERROR( ERR_ERRNO, NULL, NULL, NULL )
	}

	/* file opened - read the lock id */
	if ( fgets( tmp, MAX_TMP, fp) != NULL )
	{	lock_id = (pid_t) strtol( tmp, NULL, 0 );

		/* does this process or its process group own the lock? */
		if ( (niminfo.pid == lock_id) || (niminfo.pgrp == lock_id) )
			rc = OK;
		else
			/* no - somebody else owns it */
			rc = FAILURE;
	}
	else
		/* 
		 * Either format of NIM_GLOCK is bad or there was a read problem 
		 */
		rc = FAILURE;

	fclose( fp );

	return( rc );
}

/*---------------------------- get_glock         ------------------------------
 *
 * NAME: get_glock
 *
 * FUNCTION:
 *		obtains the NIM global lock, which is used to synchronize access to shared
 *			NIM resources
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		call errstr upon failure
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			retry		= number of times to retry
 *			delay		= period to wait between retries
 *		global:
 *			NIM_GLOCK	= the global lock file; when in existence, symbolizes
 *					  that a NIM process has exclusive access to the shared
 *						NIM resources
 *
 * RETURNS: (int)
 *		SUCCESS			= global NIM lock obtained
 *		OK					= NIM lock alreay owned by current process
 *		FAILURE			= failure; unable to obtain lock within specified 
 *									retries
 *
 * OUTPUT:
 *---------------------------------------------------------------------------*/

int
get_glock(	int retry,
				int delay )

{	int  glock_fd;
	char tmp[MAX_TMP];

	VERBOSE4("         get_glock: retry=%d; delay=%d;\n",retry,delay,NULL,NULL)

	/* first, check to see whether this process already owns the lock */
	if ( query_glock() == OK )
	{	/* yes - already owned by this process */
		niminfo.glock_held = TRUE;
		return( OK );
	}

	/* get ready to retry; retry & delay MUST be greater than zero */
	retry = ( retry <= 0 ) ? LOCK_RETRY : retry;
	delay = ( delay <= 0 ) ? LOCK_RETRY_DELAY : delay;

	/* don't want error interrupts here */
	disable_err_sig();

	/* loop until lock obtained for max retries reached */
	while (	((glock_fd = open( NIM_GLOCK, 
						O_RDWR | O_CREAT | O_EXCL, S_IREAD | S_IWRITE)) == -1) && 
				(errno == EEXIST) &&
	    		(--retry > 0) )
		/* wait to try again */
		sleep( delay );

	if ( glock_fd == -1 )
		/* error - couldn't obtain lock within specified retries */
		ERROR( ERR_CANT_GLOCK, NULL, NULL, NULL )
	else
	{	/* write process id into NIM_GLOCK */
		sprintf( tmp, "%d", niminfo.pid );
		write( glock_fd, tmp, strlen(tmp) );
		close( glock_fd );
		niminfo.glock_held = TRUE;
	}

	/* enable interrupts again */
	enable_err_sig();

	/* return */
	return( SUCCESS );

} /* end of get_glock */

/*---------------------------- rm_glock          ------------------------------
 *
 * NAME: rm_glock
 *
 * FUNCTION:
 *		releases the global NIM lock by removing the NIM_GLOCK file if it is
 *			owned by the current process -or- if the force option is used
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			force					= TRUE if NIM_GLOCK is to be removed unconditionally
 *		global:
 *			NIM_GLOCK
 *
 * RETURNS: (int)
 *		SUCCESS					= was able to remove the NIM_GLOCK
 *		FAILURE					= couldn't remove NIM_GLOCK because current process
 *											doesn't own it -or- error accessing NIM_GLOCK
 *
 * OUTPUT:
 *---------------------------------------------------------------------------*/

int
rm_glock( int force )

{	
	int	rc = SUCCESS;
	FILE 	*fp;
	char	name[80];
	int	process_id;

	VERBOSE4("         rm_glock: force=%d;\n",force,NULL,NULL,NULL)

	/* either force remove or verify that this process owns the lock */
	if ( (force) || ((rc = query_glock()) == OK) ) 
	{	/* ok to remove - do it now */
		remove( NIM_GLOCK );
		niminfo.glock_held = FALSE;
		rc = SUCCESS;
	} 
	else
		rc = FAILURE;

	/* return */
	return( rc );

} /* end of rm_glock */

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ LIST manipulation                                 $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

/*------------------------ get_list_space         ------------------------------
 *
 * NAME: get_list_space
 *
 * FUNCTION:
 *		initializes info in the <listptr> LIST
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			listptr				= ptr to LIST
 *			chunk_size			= LIST chunk_size
 *			init					= > 0 if LIST is to initialized
 *		global:
 *
 * RETURNS: (int)
 *		SUCCESS					= <listptr> initialized
 *		FAILURE					= malloc error?
 *
 * OUTPUT:
 *--------------------------------------------------------------------------*/

int
get_list_space(	LIST 	*listptr,
						int chunk_size,
						int init )

{

	VERBOSE4("         get_list_space: listptr=0x%x\n",listptr,NULL,NULL,NULL)

	/* ensure enough space in LIST */
	/* NOTE that each LIST is NULL terminated, so max_num is always 1 less */
	/*		than the space we really have (because the last entry is NULL) */
	if ( (init) || (listptr->list == NULL) ) 
	{	/* initialize the LIST */
		listptr->num = 0;
		listptr->max_num = chunk_size;
		listptr->chunk_size = chunk_size;
		listptr->list = (char **) nim_malloc( sizeof(char **) * (chunk_size + 1));
	} 
	else if ( (listptr->num + 1) > listptr->max_num ) 
	{	/* reached max - get more space */
		listptr->max_num = listptr->max_num + listptr->chunk_size;
		if ( (listptr->list = (char **)
		    realloc( listptr->list, 
		    sizeof(char **) * (listptr->max_num + 1) )) == NULL )
			ERROR( ERR_SYS, NULL, NULL, NULL );
	} 
	else
		return( SUCCESS );

	/* initialize the new space */
	/* remember: max_num is always 1 less than the number we really have */
	memset( (listptr->list + listptr->num), NULL, 
	    (sizeof(char **) * (listptr->max_num - listptr->num + 1)) );

	return( SUCCESS );

} /* end of get_list_space */

/*---------------------------- free_list         ------------------------------
 *
 * NAME: free_list
 *
 * FUNCTION:
 *		frees memory allocated to the <listptr> list_struct
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			listptr				= ptr to list_struct
 *		global:
 *
 * RETURNS: (int)
 *		SUCCESS					= memory free'd
 *
 * OUTPUT:
 *---------------------------------------------------------------------------*/

int
free_list(	LIST *listptr )

{	
	int	i;

	VERBOSE4("         free_list: listptr=0x%x\n",listptr,NULL,NULL,NULL)

	for (i = 0; i < listptr->num; i++)
		free( listptr->list[i] );

	free( listptr->list );

	return( SUCCESS );

} /* end of free_list */

/*---------------------------- reset_LIST        ------------------------------
*
* NAME: reset_LIST
*
* FUNCTION:
*		resets a LIST by freeing what the list points to, but leaves the memory
*			which has been allocated to the LIST
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
*			listptr				= ptr to LIST struct
*		global:
*
* RETURNS: (int)
*		SUCCESS					= <listptr> reset
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
reset_LIST(	LIST *listptr )

{	
	int	i;

	VERBOSE4("         reset_list: listptr=0x%x\n",listptr,NULL,NULL,NULL)

	/* free what the list points to */
	for (i = 0; i < listptr->num; i++) {	
		free( listptr->list[i] );
		listptr->list[i] = NULL;
	}

	/* reset the num */
	listptr->num = 0;

	return( SUCCESS );

} /* end of reset_LIST */

/*---------------------------- add_to_LIST       ------------------------------
*
* NAME: add_to_LIST
*
* FUNCTION:
*		adds the specified string to the specified LIST
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			listptr					= ptr to LIST struct
*			str					= string to add
*		global:
*
* RETURNS: (int)
*		SUCCESS					= <str> added
*		FAILURE					= couldn't malloc?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
add_to_LIST(	LIST *listptr,
					char *str )

{
	VERBOSE4("         add_to_LIST: listptr=0x%x; str=%s;\n",listptr,str,NULL,
				NULL)

	/* reserve space */
	if ( get_list_space( listptr, 1, FALSE ) == FAILURE )
		return( FAILURE );

	/* malloc space for <str> */
	listptr->list[ listptr->num ] = nim_malloc( strlen(str) + 1 );

	strcpy( listptr->list[ listptr->num ], str );

	/* incre the count */
	listptr->num = listptr->num + 1;

	return( SUCCESS );

} /* end of add_to_LIST */

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ attribute assignment processing                   $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

/*---------------------------- add_attr_ass      ------------------------------
 *
 * NAME: add_attr_ass
 *
 * FUNCTION:
 *		adds a new element to the ATTR_ASS_LIST
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		this function ASSUMES that the memory which the <name> & <value> char 
 *			ptrs point to will remain intact; ie, this function does NOT copy 
 *			the string - they only point to it
 *		this function is INTENDED to be used with strings which have been
 *			initialized in argv, so they should be persistant
 *		it is ASSUMED that <attr_ass> has already been initialized
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			attr_ass				= ptr to ATTR_ASS_LIST
 *			pdattr				= pdattr.attr if known
 *			name					= name of attr
 *			value					= value of attr
 *			seqno					= seqno of attr
 *		global:
 *
 * RETURNS: (int)
 *		SUCCESS					= new element added
 *		FAILURE					= unable to add
 *
 * OUTPUT:
 *---------------------------------------------------------------------------*/

int
add_attr_ass(	ATTR_ASS_LIST  *attr_ass,
					int pdattr,
					char *name,
					char *value,
					int seqno )

{	
	int	i;
	char tmp[MAX_TMP];

	VERBOSE4("         add_attr_ass: pdattr=%d; name=%s; value=%s; seqno=%d;\n",
				pdattr,name,value,seqno)

	/* ensure that this is a unique assignment */
	for (i = 0; i < attr_ass->num; i++)
		if ( strcmp( attr_ass->list[i]->name, name ) == 0 )
		{
			/* attr with same name found */
			/* check the seqno */
			if ( seqno == 0 )
			{
				if ( attr_ass->list[i]->seqno > 0 )
					/* unable to determine what seqno this attr will be assigned, */
					/*		so, in order to avoid potential collisions, tell the */
					/*		user that a seqno is required in this context */
					ERROR( ERR_MISSING_SEQNO, name, NULL, NULL )

				else if ( strcmp( attr_ass->list[i]->value, value ) == 0 )
               /* attr names and values are the same - treat as a duplicate */
					ERROR( ERR_DUP_ATTR_ASS, name, NULL, NULL );
			}
			else if ( attr_ass->list[i]->seqno == 0 )
				ERROR( ERR_MISSING_SEQNO, name, NULL, NULL )
			else if ( attr_ass->list[i]->seqno == seqno )
			{
				sprintf( tmp, "%s%d", name, seqno );
				ERROR( ERR_DUP_ATTR_ASS, tmp, NULL, NULL );
			}
		}

	/* get space for a new element */
	if ( !get_list_space( attr_ass, 1, FALSE ) )
		return( FAILURE );

	/* initialize a new element */
	if ( (attr_ass->list[ attr_ass->num ] = 
	    (ATTR_ASS * ) nim_malloc( ATTR_ASS_SIZE )) == NULL )
		return( FAILURE );

	attr_ass->list[ attr_ass->num ]->pdattr = pdattr;
	attr_ass->list[ attr_ass->num ]->name = name;
	attr_ass->list[ attr_ass->num ]->value = value;
	attr_ass->list[ attr_ass->num ]->seqno = seqno;
	attr_ass->num = attr_ass->num + 1;

	return( SUCCESS );

} /* end of add_attr_ass */

/* --------------------------- add_attr 
 *
 * NAME: add_attr 
 *
 * FUNCTION:
 *	Will create space for name and value and call add_attr_ass
 *
 * DATA STRUCTURES:
 *		global:
 *			valid_attrs
 *			attr_ass
 *
 * RETURNS: (int)
 *		SUCCESS	= nothing missing
 *		FAILURE	= definition incomplete
 *
 * ---------------------------------------------------------------------------*/

add_attr(	ATTR_ASS_LIST  *attr_ass, 
				int tag, 
				char *tag_t, 
				char *value )

{
	char 	*attr_value=NULL; 
	int	sz; 

	VERBOSE4("         add_attr: tag=%d; tag_t=%s; value=%s;\n",tag,tag_t,value,
				NULL)

	sz = strlen(value)+1;
	if ( (attr_value = malloc(sz)) == NULL)
		return(FAILURE); 
	strcpy(attr_value, value);
	return(add_attr_ass( attr_ass, tag, tag_t, attr_value, 0));
}
	
/*---------------------------- find_attr_ass     ------------------------------
*
* NAME: find_attr_ass
*
* FUNCTION:
*		searches the attr_ass array for the specified attr; if found, the index
*			of the entry is returned
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			attr_ass				= ptr to ATTR_ASS_LIST
*			attr					= attr being searched for
*		global:
*
* RETURNS: (int)
*		>= 0						= index into <attr_ass> for <attr>
*		-1							= <attr> not found
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
find_attr_ass(	ATTR_ASS_LIST *attr_ass,
					int attr )

{	int i;

	VERBOSE4("         find_attr_ass: attr=%d\n",attr,NULL,NULL,NULL)

	for (i=0; i < attr_ass->num; i++)
		if ( attr_ass->list[i]->pdattr == attr )
			return( i );

	/* <attr> not found */
	return( -1 );

} /* end of find_attr_ass */

/*---------------------------- rm_attr_ass       ------------------------------
*
* NAME: rm_attr_ass
*
* FUNCTION:
*		removes the specified <attr> from the specified ATTR_ASS_LIST
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			attr_ass				= ptr to ATTR_ASS_LIST
*			attr					= attr to be removed
*		global:
*
* RETURNS: (int)
*		SUCCESS					= <attr> removed (or wasn't there to begin with)
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
rm_attr_ass(	ATTR_ASS_LIST *attr_ass,
					int attr )

{	int i,j;
	ATTR_ASS *ptr;

	VERBOSE4("         rm_attr_ass: attr=%d\n",attr,NULL,NULL,NULL)

	if ( (i = find_attr_ass( attr_ass, attr )) >= 0 )
	{
		/* save address of this node */
		ptr = attr_ass->list[i];

		/* move all preceeding attrs down one */
		for (j=i; j < (attr_ass->num - 1); j++)
			attr_ass->list[j] = attr_ass->list[j+1];
		attr_ass->num = attr_ass->num - 1;

		/* free the removed node */
		free( ptr );
	}

	return( SUCCESS );

} /* end of rm_attr_ass */
	
/* --------------------------- attr_value 
 *
 * NAME: attr_value 
 *
 * FUNCTION:
 *	Will return the value of a given attr if in the attr list. 
 *
 * DATA STRUCTURES:
 *		Parms:
 *			attr_ass		- list of attrs
 *			attr  		- attr to get value of.. 
 *
 * RETURNS: (int)
 *		> NULL 	= the value 
 *		NULL 	= value not found. 
 *
 * ---------------------------------------------------------------------------*/

char *
attr_value(	ATTR_ASS_LIST *attr_ass, 
				int attr )

{	int i;

	VERBOSE4("         attr_value: attr=%d\n",attr,NULL,NULL,NULL)

	if ( (i = find_attr_ass( attr_ass, attr )) >= 0 )
		return( attr_ass->list[i]->value );

	return( NULL );

} /* end of attr_value */

/*---------------------------- parse_attr_ass    ------------------------------
 *
 * NAME: parse_attr_ass
 *
 * FUNCTION:
 *		separates an attribute name from it's value, verifies that the name
 *			corresponds to a valid attribute, then initializes another
 *			attr_assignment struct
 *		note that when no validation routine is specified for a specific 
 *			attribute, this function will only verify that <attr_ass> is in the
 *			form of:
 *					<attr>=
 *		it does NOT require that the <value> be present; this is done so that
 *			attribute values may be reset with assignments like: spot=""
 *		it is up to all specific validation routines (eg. valid_pdattr_ass) to
 *			require a <value> or not
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		this function ASSUMES that the valid_attrs array will be terminated by
 *			a NULL entry
 *		sets errstr on failure
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			list					= list of attr_assignment structs
 *			valid_attrs			= ptr to valid_attr_list struct
 *			attr_ass				= attribute assignment string
 *			change_op			= TRUE if NULL <value>s are acceptable
 *		global:
 *
 * RETURNS: (int)
 *		SUCCESS					= <attr_ass> is valid & accepted
 *		FAILURE					= bad <attr_ass>
 *
 * OUTPUT:
 *----------------------------------------------------------------------------*/

int
parse_attr_ass(	ATTR_ASS_LIST 	*list,
						VALID_ATTR 		*valid_attrs,
						char *attr_ass,
						int change_op )

{	
	regmatch_t mattrass[ERE_ATTR_ASS_NUM];
	regmatch_t mname[ERE_ATTR_NAME_NUM];
	int	pdattr = 0;
	char	*name;
	char	*value;
	int	seqno = 0;
	int	i;
	int	found = FALSE;
	int	(*v_routine)() = NULL;
	char	*ass;

	VERBOSE4("         parse_attr_ass: attr_ass=%s; change_op=%d;\n",
				attr_ass,change_op,NULL,NULL)

	/* cache the assignment str */
	ass = nim_malloc( strlen( attr_ass ) + 1 );
	strcpy( ass, attr_ass );

	/* valid attr assignment? format is <attr>=<value> */
	if (	(regexec( nimere[ATTR_ASS_ERE].reg, ass, 
	    ERE_ATTR_ASS_NUM, mattrass, 0 ) != 0) || 
	    (mattrass[1].rm_so < 0) )
		ERROR( ERR_VALUE, attr_ass, MSG_msg(MSG_ATTR_ASS), NULL )
	else if ( mattrass[2].rm_so <= 0 )
		value = NULL;
	else
		value = ass + mattrass[2].rm_so;
	
	/* separate name from value */
	ass[ mattrass[1].rm_eo ] = NULL_BYTE;
	name = ass;

	/* appropriate attr name? (must begin with a letter) */
	if (	(regexec(	nimere[ATTR_NAME_ERE].reg, name, 
	    					ERE_ATTR_NAME_NUM, mname, 0 ) != 0) || 
	    	(mname[1].rm_so < 0) ) 
		ERROR( ERR_VALUE, name, MSG_msg(MSG_ATTR_NAME), NULL )

	/* seqno appended to attr name? */
	if ( mname[2].rm_eo > mname[2].rm_so ) 
	{	/* convert seqno into number */
		seqno = (int) strtol( name + mname[2].rm_so, NULL, 0 );

		/* make sure seqno is between 1 and MAX_SEQNO */
		if (	(seqno < 1) || (seqno > MAX_SEQNO) )
			ERROR( ERR_BAD_SEQNO, name, MAX_SEQNO, NULL )

		/* separate seqno from name */
		name[ mname[1].rm_eo ] = NULL_BYTE;
	}

	/* check for special attrs */
	/* if found, set the global flag and return before the attr is added */
	/*		into the attr_ass */
	if ( strcmp( name, ATTR_FORCE_T ) == 0 )
	{
		force = TRUE;
		return( SUCCESS );
	}
	else if ( strcmp( name, ATTR_VERBOSE_T ) == 0 )
	{
		verbose = (int) strtol( value, NULL, 0 );
		return( SUCCESS );
	}
	else if ( strcmp( name, ATTR_IGNORE_LOCK_T ) == 0 )
	{
		ignore_lock = TRUE;
		return( SUCCESS );
	}

	/* check for valid attrs? */
	if ( valid_attrs != NULL ) 
	{	/* if name of first entry in valid_attrs is NULL, we're going to assume */
		/*		that the caller will accept all attrs */
		if ( valid_attrs[0].name == NULL )
		{	
			/* Flag found and set the validateion routine 
			 * user MUST specify a valid ptr to a function or
			 * NULL. 
			 */
			found = TRUE;
			v_routine = valid_attrs[0].validate;
		} 
		else
		{	/* make sure attr name is in the list which the caller will accept */
			for (i = 0; valid_attrs[i].name != NULL; i++)
			{	if ( strcmp( name, valid_attrs[i].name ) == 0 )
				{
					/* attr is accepted by caller */
					found = TRUE;
					pdattr = valid_attrs[i].pdattr;

					/* specific attr validation function? */
					v_routine = valid_attrs[i].validate;
					break;
				}
			}
		}

		if ( found ) 
		{	
			if ( v_routine != NULL )
				return( (*v_routine) ( list, name, value, seqno, change_op ) );
		} 
		else
			/* not an acceptable attribute name */
			ERROR( ERR_INVALID_ATTR, name, NULL, NULL )
	} 
	else
		found = TRUE;

	if ( found ) 
	{	/* we get here on 2 conditions: */
		/*		1) the valid_attrs list is empty (ie, will accept all attrs) */
		/*		2) attr is acceptable and there is not special validation routine */
		/* in either case, add the attr to the list if the value is not NULL */
		/*		or NULL <value>s are ok */
		if ( (value == NULL) && (!change_op) )
			ERROR( ERR_MISSING_VALUE, attr_ass, NULL, NULL );

		/* check list */
		if ( list == NULL )
			/* caller doesn't any ATTR_ASS struct added */
			return( SUCCESS );
		else
			/* add a new ATTR_ASS */
			return( add_attr_ass( list, pdattr, name, value, seqno ) );
	}

	/* if we get here, name wasn't found in valid_attrs */
	ERROR( ERR_CONTEXT, name, NULL, NULL )

} /* end of parse_attr_ass */

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ command query support                             $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

/* ---------------------------- cmd_what
 *
 * NAME: cmd_what
 *
 * FUNCTION:	Given that the valid_attrs array is in global memory. 
 *		Romp through the array for the required attributes then romp through 
 *		for the optional attributes printing them out as we go.. 
 *
 * DATA STRUCTURES:
 *		global:
 *			valid_attrs: 
 *
 * RETURNS: (void)
 *
 * ---------------------------------------------------------------------
 */

void
cmd_what(VALID_ATTR 	valid_attrs[])
{

			int			va_indx;
			int			print_msg = TRUE;
			VALID_ATTR 	*va_ptr;

	/* 
	 * check for required attrs 
	 */
	for (va_indx = 0; valid_attrs[va_indx].pdattr; va_indx++) {
		va_ptr = &valid_attrs[va_indx];
		if ( va_ptr->required ) {
			if (print_msg)
				{
					printf ("\n%s\n", MSG_msg (MSG_REQUIRED_ATTR));
					print_msg = FALSE;
				}
			printf("\t-a %s=%s\n", va_ptr->name, MSG_msg (MSG_VALUE));
		}
	}

	print_msg = TRUE;

	/* 
	 * Now optional attrs 
	 */
	for (va_indx = 0; valid_attrs[va_indx].pdattr; va_indx++) {
		va_ptr = &valid_attrs[va_indx];
		if ( !va_ptr->required )
		{	if (print_msg)
			{
				printf ("\n%s\n", MSG_msg (MSG_OPTIONAL_ATTR));
				print_msg = FALSE;
			}
			printf("\t-a %s=%s\n", va_ptr->name, MSG_msg (MSG_VALUE));
		}
	}
	printf("\n\n");
}

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ regular expression processing                     $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

/*---------------------------- first_field       ------------------------------
*
* NAME: first_field
*
* FUNCTION:
*		regular expression matching for a string containing 2 or more fields which
*			are separated by a space
*		if found, the string is terminated after the first field
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			str					= string to match on
*		global:
*
* RETURNS: (int)
*		TRUE						= string matched by FIRST_FIELD_ERE
*		FALSE						= string not matched by FIRST_FIELD_ERE
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
first_field(	char *str )

{	regmatch_t pmatch[ERE_FIRST_FIELD_NUM];

	VERBOSE4("         first_field: str=%s\n",str,NULL,NULL,NULL)

	/* matched by FIRST_FIELD_ERE? */
	if (	(regexec( nimere[FIRST_FIELD_ERE].reg, str,
						ERE_FIRST_FIELD_NUM, pmatch, 0) != 0) ||
			(pmatch[1].rm_so < 0) )
		return( FALSE );

	/* yes - terminate the string after the first field */
	str[ pmatch[1].rm_eo ] = NULL_BYTE;

	return( TRUE );

} /* end of first_field */

/*---------------------------- two_fields        -------------------------------
*
* NAME: two_fields
*
* FUNCTION:
*		separates <value> into two fields; separator is a space
*		each field is copied into a buffer pointed to by <first> and <second>
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			value					= string to separate
*			first					= ptr to buffer for first field
*			second				= ptr to buffer for second field
*		global:
*
* RETURNS: (int)
*		SUCCESS					= <value> matches TWO_FIELDS_ERE
*		FAILURE					= <value> doesn't match TWO_FIELDS_ERE
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
two_fields(	char *value,
				char **first,
				char **second )

{	regmatch_t fields[ERE_TWO_FIELDS_NUM];

	VERBOSE4("         two_field: value=%s\n",value,NULL,NULL,NULL)

	if ( first != NULL )
		*first = NULL;
	if ( second != NULL )
		*second = NULL;

	/* separate <value> */
  	if (	(regexec( nimere[TWO_FIELDS_ERE].reg, value,
                 	ERE_TWO_FIELDS_NUM, fields, 0) != 0) ||
        	(fields[1].rm_so < 0) ||
        	(fields[2].rm_so < 0) )
     	return( FAILURE );
	value[ fields[1].rm_eo ] = NULL_BYTE;

	/* initialize each field we have a ptr for */
	if ( first != NULL )
	{	*first = nim_malloc( strlen(value) + 1 );
		strcpy( *first, value );
	}
	if ( second != NULL )
	{	*second = nim_malloc( strlen(value + fields[2].rm_so) + 1 );
		strcpy( *second, (value + fields[2].rm_so) );
	}

	/* put separator back */
	value[ fields[1].rm_eo ] = ' ';

	return( SUCCESS );

} /* end of two_fields */
	
/*---------------------------- three_fields      -------------------------------
*
* NAME: three_fields
*
* FUNCTION:
*		separates <value> into three fields; separator is a space
*		each field is copied into a buffer pointed to by <first>, <second>, &
*			<third>
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			value					= string to separate
*			first					= ptr to buffer for first field
*			second				= ptr to buffer for second field
*			third					= ptr to buffer for third field
*		global:
*
* RETURNS: (int)
*		SUCCESS					= <value> matches THREE_FIELDS_ERE
*		FAILURE					= <value> doesn't match THREEE_FIELDS_ERE
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
three_fields(	char *value,
					char **first,
					char **second,
					char **third )

{	regmatch_t fields[ERE_THREE_FIELDS_NUM];

	VERBOSE4("         three_field: value=%s\n",value,NULL,NULL,NULL)

	/* separate <value> */
  	if (	(regexec( nimere[THREE_FIELDS_ERE].reg, value,
                 	ERE_THREE_FIELDS_NUM, fields, 0) != 0) ||
        	(fields[1].rm_so < 0) ||
        	(fields[2].rm_so < 0) ||
        	(fields[3].rm_so < 0) )
     	return( FAILURE );
	value[ fields[1].rm_eo ] = NULL_BYTE;
	value[ fields[2].rm_eo ] = NULL_BYTE;

	/* initialize each field we have a ptr for */
	if ( first != NULL )
	{	*first = nim_malloc( strlen(value) + 1 );
		strcpy( *first, value );
	}
	if ( second != NULL )
	{	*second = nim_malloc( strlen(value + fields[2].rm_so) + 1 );
		strcpy( *second, (value + fields[2].rm_so) );
	}
	if ( third != NULL )
	{	*third = nim_malloc( strlen(value + fields[3].rm_so) + 1 );
		strcpy( *third, (value + fields[3].rm_so) );
	}

	/* put separator back */
	value[ fields[1].rm_eo ] = ' ';
	value[ fields[2].rm_eo ] = ' ';

	return( SUCCESS );

} /* end of three_fields */
	
/*---------------------------- smit_no_exec      ------------------------------
*
* NAME: smit_no_exec 
*
* FUNCTION:
*		if the SMIT_NO_EXEC environment variable is set to "0042" (the NIM SMIT
*			id), then this function will print out the command line and exit
*		this is done so that the NIM SMIT screens can be tested without actually
*			performing any NIM operations
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			argc					= argc from main
*			argv					= argv from main
*		global:
*
* RETURNS: (int)
*		FAILURE					= indicates that the SMIT_NO_EXEC env variable is
*											not set to the NIM_SMIT_SET (ie, continue
*											with normal processing)
*
* OUTPUT:
*		writes command line to stdout
*-----------------------------------------------------------------------------*/

#define SMIT_NO_EXEC					"SMIT_NO_EXEC"
#define NIM_SMIT_SET					"0042"

int
smit_no_exec(	int argc,
					char *argv[] )

{	int i;
	char *ptr;

	/* SMIT_NO_EXEC initialized for NIM no execution? */
	if (	((ptr = getenv( SMIT_NO_EXEC )) == NULL) ||
			(strcmp( ptr, NIM_SMIT_SET ) != 0) )
		return( FAILURE );

	/* print out the command line */
	for (i=0; i < argc; i++)
		if ( i == 0 )
			printf( "%s", argv[i] );
		else
			printf( " %s", argv[i] );
	printf( "\n" );

	exit( 0 );

} /* end of smit_no_exec */
	
