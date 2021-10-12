char	sccs_id[] = " @(#)12 1.19  src/bos/usr/lib/nim/methods/c_ch_nfsexp.c, cmdnim, bos411, 9438C411a  9/23/94  10:12:18";
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: bld_args
 *		ch_list_perm
 *		check_input
 *		is_exported
 *		main
 *		nfsperm_s2i
 *		parse_args
 *		parse_export_file
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


#include "cmdnim_cmd.h"

#define	CHILD		1
#define	PARENT		0

#define LIST_OK		0
#define LIST_UPD	1
#define LIST_MT		2


/* 
 * Note: This is some magic stuff that the chnfsexp script is looking for
 *		   could be exposed if NFS changes this... 
 */ 
#define NFS_NULL_PARM "_NULL_PARM"


ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] = {
	{ ATTR_LOCATION,  ATTR_LOCATION_T,   TRUE,   NULL },
	{ ATTR_GRANT,     ATTR_GRANT_T,      FALSE,  NULL },
	{ ATTR_REVOKE,    ATTR_REVOKE_T,     FALSE,  NULL },
	{ ATTR_NFS_PERMS, ATTR_NFS_PERMS_T,  FALSE,  NULL },
	{ 0,              NULL,              FALSE,  NULL }
};

typedef enum { OPT_ERR, DIR, ROOT, ACCESS, RO, RW, SECURE, ANON } nfs_opt ; 

struct nfs_options {
	nfs_opt		type;
	char 		*opt;
};


/* --------------------------- parse_args 
 *
 * NAME: parse_args
 *
 * FUNCTION:
 *     parses command line arguments
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *     calls nim_error
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *     parameters:
 *        argc        = argc from main
 *        argv        = argv from main
 *     global:
 *
 * RETURNS: (int)
 *     SUCCESS              = no syntax errors on command line
 *
 * OUTPUT:
 * -------------------------------------------------------------------------*/

int
parse_args( int argc, 
char *argv[] )

{
	extern char	*optarg;
	extern int	optind, optopt;
	int	c;
	int	syntax_err = FALSE;

	while ( (c = getopt(argc, argv, "a:")) != -1 ) {
		switch (c) {
		case 'a': /* attribute assignment */
			if (!parse_attr_ass( &attr_ass, valid_attrs, optarg, FALSE ) )
				nim_error( 0, NULL, NULL, NULL );
			break;

		case ':': /* option is missing a required argument */
			nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
			break;

		case '?': /* unknown option */
			nim_error( ERR_BAD_OPT,optopt,MSG_msg(MSG_C_CH_NFSEXP_SYNTAX), NULL );
			break;
		}

		if ( syntax_err )
			nim_error( ERR_SYNTAX, MSG_msg(MSG_C_CH_NFSEXP_SYNTAX), NULL, NULL );
	}

	return( SUCCESS );

} /* end of parse_args */


/* --------------------------- is_exported 
 *
 * NAME: is_exported
 *
 * FUNCTION:  This will examine two filenames and basically do a 
 *	sub string on them. If the flag child is set it will return 
 *	TRUE to indicate that a child directory is already exported, 
 *	else if the flag child is NOT set the function will return
 *	TRUE if the parent (or same) directory is exported. 
 *
 * NOTES:
 *     calls nim_error
 *
 * DATA STRUCTURES:
 *     parameters:
 *        exp_dir 	= the exported directory
 *        loc 		= the location pathname we're interested in
 *	  child		= the flag to say which way to check
 *
 * RETURNS: (int)
 *     TRUE 		= give the input flag setting a match was made
 *     FALSE  		= give the input flag setting a match was NOT made
 *
 * ------------------------------------------------------------------------- */
int
is_exported(char *exp_dir, char *loc, int child )
{
	char 	*ptr; 
	char 	filename[256]=""; 
	char	test_name[256];
	int	flag=0;

	strcpy( test_name, (child) ? exp_dir : loc ); 
	strcat( test_name, "/"); 

	while ( ptr=strtok( ( (flag) ? NULL : test_name ), "/" ) ) {
		flag++;
		strcat(filename, "/");
		strcat(filename, ptr);
		if ( strcmp( (child) ? loc:exp_dir, filename ) == 0  )
			return(TRUE);
	}
	return(FALSE);
}

/* --------------------------- in_list 
 *
 * NAME: in_list
 *
 * FUNCTION:  This will examine a colon sep list for the target and return 
 *	a positive value if present or a zero if not present. 
 *
 * DATA STRUCTURES:
 *     parameters:
 *        list_str 	= A pointer to the colon sep list string
 *        target    	= A pointer to the target 
 *
 * RETURNS: (int)
 *     +ve   		= in the list  
 *     0   		= not in the list 
 *
 * --------------------------------------------------------------------------*/
 in_list (  char *list_str,
	    char *target	)
{
	char	*list, *ptr;
	int	init_pass=0; 
	int	target_in_list=0;

	/* 
	 * need to preserve the org list_str... 
	 */
	list = nim_malloc( strlen(list_str)+1);	
	strcpy(list, list_str); 

	while ( ptr=strtok( (init_pass) ? NULL:list, ":") ) {
		init_pass++;
		if ( strcmp(ptr, target) == 0 ) {
			target_in_list++; 
			break;
		}
	}	

	free(list); 
	return(target_in_list);
}

/* --------------------------- ch_list_perm 
 *
 * NAME: ch_list_perm
 *
 * FUNCTION:  This will examine the option to see what needs to 
 *	be done in order to satisfy the user input (revoke/grant).
 *
 * NOTES:
 *     calls nim_error
 *
 * DATA STRUCTURES:
 *     parameters:
 *        list_str 	= A pointer to the host list string
 *        host_name 	= A pointer to the host name 
 *        operation  	= what to do with the name ..  
 *
 * RETURNS: (int)
 *     2  		= NULL host list.. so RMNFSEXP 
 *     1  		= A change is needed 
 *     0   		= Everything is hunky dory no changes made
 *
 * --------------------------------------------------------------------------*/
int
ch_list_perm( char **list_str, char *host_name, int operation )
{
	char	*new_str; 
	char	*ptr, *Sptr, *Eptr;
	int	init_pass=0; 

	switch ( operation ) {
		case ATTR_GRANT: 
			/*
		 	 * Going to add,  but make sure its not already in 
		 	 * the list
		 	 */ 
			if ( in_list(*list_str, host_name) )
				return(LIST_OK); 
			new_str = nim_malloc( strlen(*list_str)+strlen(host_name)+2);	
			strcpy(new_str, *list_str); 
			if (strlen(new_str) > 0 )
				strcat(new_str, ":" );
			strcat(new_str, host_name );
			free(*list_str); 
			*list_str = new_str; 
		break; 
	
		case ATTR_REVOKE: 
			/*
		 	 * Going to delete 
		 	 */ 
			if ( !in_list(*list_str, host_name) )
				return(LIST_OK); 
		 	new_str=nim_malloc( strlen(*list_str) );	
			while ( ptr=strtok( (init_pass) ? NULL:*list_str, ":") ) {
				init_pass++;
				if ( strcmp(ptr, host_name) == 0 )
					continue;
				if (*new_str!=0)
					strcat(new_str,":");
				strcat(new_str, ptr);
			}	
			free(*list_str);
			*list_str = new_str; 
		break; 

		default: 
			return(LIST_OK); 
	}
	if (strlen(*list_str))
		return(LIST_UPD);
	return(LIST_MT);
}


/* --------------------------- nfsperm_s2i 
 *
 * NAME: nfsperm_s2i
 *
 * FUNCTION:  This will convert the nfs_perms string to enumerated equiv.  
 *
 * DATA STRUCTURES:
 *     parameters:
 *	        ptr	= string to test / convert. 
 *
 * --------------------------------------------------------------------------*/

nfs_opt
nfsperm_s2i( char *ptr ) 

{
	if ( strncmp(ptr, ATTR_NFS_ROOT_T, strlen(ATTR_NFS_ROOT_T)) == 0 ) 
		return( ROOT );
	
	if ( strncmp(ptr, ATTR_NFS_ACCESS_T, strlen(ATTR_NFS_ACCESS_T)) == 0 ) 
		return( ACCESS );
	
	if ( strncmp(ptr, ATTR_NFS_RO_T, strlen(ATTR_NFS_RO_T)) == 0 ) 
		return( RO );

	if ( strncmp(ptr, ATTR_NFS_RW_T, strlen(ATTR_NFS_RW_T)) == 0 ) 
		return( RW );
	
	if ( strncmp(ptr, ATTR_NFS_SECURE_T, strlen(ATTR_NFS_SECURE_T)) == 0 ) 
		return( SECURE );

	if ( strncmp(ptr, ATTR_NFS_ANON_T, strlen(ATTR_NFS_ANON_T)) == 0 )
		return( ANON );

	export_errstr( ERR_CH_NFSEXP_UO, ptr, NULL, NULL );
	return( OPT_ERR );
}

/* --------------------------- parse_export_file 
 *
 * NAME: parse_export_file
 *
 * FUNCTION:  This will parse the file pointed to by the file pointer 
 *	expecting the format to be an "export" file (/etc/exports or /etc/xtab) 
 *
 * DATA STRUCTURES:
 *     parameters:
 *	        fp	= The file pointer
 *             opt 	= An array of nfs options
 *	  location	= The location
 *
 * RETURNS: (int)
 *     1  		= A change is needed 
 *     0   		= Everything is ok.
 *
 * --------------------------------------------------------------------------*/

#define	MAX_CHARS_PER_LINE		4096

int
parse_export_file( FILE *fp, 
		   struct nfs_options opt[], 
		   char	*location, 
		   int	*opt_cnt 
	)

{

	char	*ptr;
	char	exp_line[MAX_CHARS_PER_LINE+1];

	/* these two variable CANNOT be local arrays becuase the opt array will */
	/*		be initialized with pointers to this information */
	char *rest=NULL;
	char *exported_dir=NULL;

	/*
	 * Examine the file line by line: 
	 *	Skip the comment lines. Break up the line into the dir exported and 
	 *	"the rest" of the line. Test to see if the dir in the export file 
	 *	matches our location (match will work if parent export too..).
	 *	If a match is found and we are not expecting it error out.  If match is 
	 *	found break up "the rest" of the line into its option parts. 
	 */
	while ( fgets(exp_line, MAX_CHARS_PER_LINE, fp) != NULL ) {
		/*
		 * Should not have any comments... but just incase skip em
		 */
		if ( *exp_line == '#' )
			continue; 
		if ( (ptr=strtok(exp_line, " \n")) == NULL )
			continue; 

		exported_dir = nim_malloc( strlen(ptr) + 1 );
		strcpy( exported_dir, ptr );

		ptr = strtok( NULL, "\n" );
		rest = nim_malloc( strlen(ptr) + 1 );
		strcpy( rest, ptr );

		/*
		 * Test if the exported dir and location are the same or a parent
		 */ 
		if ( is_exported(exported_dir, location, PARENT) ) {
			*opt_cnt=0; 
			opt[*opt_cnt].type=DIR;
			opt[*opt_cnt].opt=exported_dir;
			/*
			 * Break up the opts into the opt array
			 */
			while ( ptr=strtok((*opt_cnt) ? NULL : rest, ",") ) {
				if ( *ptr == '-' )
					ptr++;
				if ( *opt_cnt == 0 ) 
					(*opt_cnt)++;
				if ( (opt[*opt_cnt].type = nfsperm_s2i(ptr)) == OPT_ERR )
					return( FAILURE );
				while (*ptr != '=' && *ptr)
					ptr++;		   	
		       		if (*ptr == '=') 	
					ptr++; 
				if (strlen(ptr)) {
					opt[*opt_cnt].opt=nim_malloc(strlen(ptr)+1);
					strcpy(opt[*opt_cnt].opt , ptr);
				}
				else
					opt[*opt_cnt].opt = NULL; 
				(*opt_cnt)++; 
			}
			return(SUCCESS); 
		}
	}
	return(FAILURE); 
} 


/* --------------------------- bld_args  
 *
 * NAME: bld_args 
 *
 * FUNCTION:  This will convert the type/opt into the cmdline equiv.
 *
 * DATA STRUCTURES:
 *     parameters:
 *	        type 	= nfs_opt type. 
 *	         opt 	= optional string 
 *	        Args 	= array for the cmd args
 *	      argcnt 	= current index into the array
 *
 * --------------------------------------------------------------------------*/
int
bld_args(	nfs_opt type,
		char	*opt, 
		char	*Args[], 
		int	*argcnt
		)
{

	char	*ptr; 

	switch (type) { 

		case DIR   : 	Args[(*argcnt)++]="-d";
				ptr=nim_malloc(strlen(opt)+1);
				strcpy( ptr, opt);
				Args[(*argcnt)++]=ptr;
		break; 

		case ROOT  :	Args[(*argcnt)++]="-r";
				if ( strlen(opt) ) {
					ptr=nim_malloc(strlen(opt)+1);
					strcpy( ptr, opt);
				}
				else {
					ptr=nim_malloc(strlen(NFS_NULL_PARM)+1);
					strcpy( ptr, NFS_NULL_PARM );
				}	
				Args[(*argcnt)++]=ptr;
		break; 

		case ANON: 	Args[(*argcnt)++]="-a";
				if ( strlen(opt) ) {
					ptr=nim_malloc(strlen(opt)+1);
					strcpy( ptr, opt);
				}
				else {
					ptr=nim_malloc(strlen(NFS_NULL_PARM)+1);
					strcpy( ptr, NFS_NULL_PARM );
				}	
				Args[(*argcnt)++]=ptr;
		break; 

		case ACCESS: 	Args[(*argcnt)++]="-c";
				if ( strlen(opt) ) {
					ptr=nim_malloc(strlen(opt)+1);
					strcpy( ptr, opt);
				}
				else {
					ptr=nim_malloc(strlen(NFS_NULL_PARM)+1);
					strcpy( ptr, NFS_NULL_PARM );
				}	
				Args[(*argcnt)++]=ptr;
		break; 

		case RO	   : 	Args[(*argcnt)++]="-tro";
				if (opt)
					bld_args(ROOT, opt, Args, argcnt);
		break; 

		case RW	   : 	Args[(*argcnt)++]= "-trw";
				if (opt)
					bld_args(ROOT, opt, Args, argcnt);
		break; 

		case SECURE: 	Args[(*argcnt)++]="-s";
		break; 
	}
}

/* ---------------------------- check_input
 *
 * NAME: check_input
 *
 * FUNCTION:
 * checks to make sure that the information supplied by user is sufficient
 * to complete operation
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *    calls nim_error
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *    global:
 *       valid_attrs
 *       attr_ass
 *    Parameters: 
 *	 nfscmd	- What we're going to do. 
 *
 * RETURNS: (int)
 *    SUCCESS  = nothing missing
 *    FAILURE  = definition incomplete
 *
 * OUTPUT:
 * ------------------------------------------------------------------------ */

void
check_input( 	char	**host_name, 	
		int	*operation, 
		char 	**location,
		nfs_opt *nfsopt   )

{
	int	va_indx, attr_indx;

	VALID_ATTR	*va_ptr;
	ATTR_ASS     	*aa_ptr;
	int		opts = 0; 
	char		*nfs_perms = ATTR_NFS_RO_T; 

	/* 
 	 * check for required attrs 
	 */
	for (va_indx = 0; valid_attrs[va_indx].pdattr; va_indx++) {
		va_ptr = &valid_attrs[va_indx];
		if ( va_ptr->required ) {
			for (attr_indx = 0; attr_indx < attr_ass.num; attr_indx++) {
				aa_ptr = attr_ass.list[attr_indx];
				if ( aa_ptr->pdattr == va_ptr->pdattr )
					break;
			}
			if ( attr_indx == attr_ass.num )
				nim_error( ERR_MISSING_ATTR, va_ptr->name, NULL, NULL );
		}
	}

	/*
	 * Set up some stuff based on the cmdline user entered. 
	 */ 
	for (attr_indx = 0; attr_indx < attr_ass.num; attr_indx++) {
		aa_ptr = attr_ass.list[attr_indx];
		switch ( aa_ptr->pdattr ) {
			case ATTR_REVOKE :
			case ATTR_GRANT  :
				if (opts)
					nim_error( ERR_CH_NFSEXP_NBA, MSG_msg(ATTR_REVOKE), 
							    MSG_msg(ATTR_GRANT), NULL ); 
				opts++;
				*host_name = aa_ptr->value; 
				*operation = aa_ptr->pdattr;
			break;

			case ATTR_LOCATION:
				*location = aa_ptr->value;
			break;

			case ATTR_NFS_PERMS:
				nfs_perms = aa_ptr->value;
			break;
		}
	}

	if (!opts)
		nim_error( ERR_SYNTAX, MSG_msg(MSG_C_CH_NFSEXP_SYNTAX), 
			NULL, NULL );

	/*
	 * map the option text to a nfs_opt... 
	 */
	if ( (*nfsopt = nfsperm_s2i(nfs_perms)) == OPT_ERR )
		nim_error(0, NULL, NULL, NULL );
	if ( *nfsopt != RO  &&  *nfsopt != RW )
		nim_error(ERR_CH_NFSEXP_UO, nfs_perms, NULL, NULL );
}

/* --------------------------- MAIN c_ch_nfsexp  
 *
 * NAME: c_ch_nfsexp 
 *
 * This method will add, change or remove a location from the export list 
 *
 * ------------------------------------------------------------------------- */

main(	int argc, 
	char *argv[])

{
extern	int	debug;
	char	*ptr;
	char	*host_name=NULL; 
	int	operation = 0 ; 
	char	*location; 
	
	int	argcnt=0, opt_cnt, loop=0; 
	int	changed_nfs_perms=0; 

	int	list_upd=0;
	int	do_rmnfsexp=0;

	int	got_root=0;
	int	got_access=0;
	int	got_opt=0; 
	
	struct	nfs_options opt[100]; 
	nfs_opt	nfsopt; 	

	FILE	*expFP;
	char	*Args[METH_MAX_ARGS]; 
	int	rc; 
	FILE	*stdOut=NULL; 

	/* 
	 * Do the common nim init stuff. 
	 */
	nim_init( argv[0], ERR_POLICY_FOREGROUND, NULL, NULL );
	parse_args( argc, argv );
	check_input( &host_name, &operation, &location, &nfsopt );
		
	/*
	 * If we can open the export file and the location exists and we're 
	 * doing a change (grant/revoke), then update the host list and do a chnfsexp.
	 */
	if ( (expFP=fopen(EXPORT_FILE, "r" )) != NULL &&
		( parse_export_file(expFP, opt, location, &opt_cnt) == SUCCESS ) &&
		( operation != 0) )   {

		for ( loop=0; loop <= opt_cnt; loop++ ) {
			switch ( opt[loop].type ) {

				case RO   :  
					if (nfsopt == RW ) {
						opt[loop].type = RW; 
						changed_nfs_perms++;
					}
					got_opt++;
				break;

				case RW   :  
					if (nfsopt == RO ) {
						opt[loop].type = RO; 
						changed_nfs_perms++;
					}
					got_opt++;
				break; 	

				/*
				 * Both the root and client lists should be in sync 
				 */ 
				case ACCESS :  
				case ROOT :  
					if ((list_upd=ch_list_perm(&opt[loop].opt,host_name, 
								operation)) == LIST_OK ) 
						if (!changed_nfs_perms) 
							exit(0);

					got_root   += ( opt[loop].type == ROOT );
					got_access += ( opt[loop].type == ACCESS );
					do_rmnfsexp+= ( list_upd == LIST_MT ); 
				break; 
			}
		}

		/*
		 * Need to build a cmd line and execute cmd. 
		 */
		if ( !do_rmnfsexp ) {
			Args[argcnt++] = CHNFSEXP;
			Args[argcnt++] = "-B"; 

			if (!got_root) 
				bld_args( (got_opt) ? ROOT : nfsopt, host_name, Args, &argcnt );
			
			for ( loop=0; loop <= opt_cnt; loop++ )  
				bld_args( opt[loop].type, opt[loop].opt, Args, &argcnt );
		}
		else {
			Args[argcnt++] = RMNFSEXP;
			Args[argcnt++] = "-B"; 
			bld_args( DIR, location, Args, &argcnt); 
		}	
		Args[argcnt]=NULL;
		
		if ( client_exec( &rc, &stdOut, Args ) == FAILURE )
			nim_error( 0, NULL, NULL, NULL );
		else if ( rc > 0 )
			nim_error( ERR_CMD, Args[0], niminfo.errstr, NULL );

		if ( operation == ATTR_GRANT && list_upd == LIST_UPD )
			/*
	 		 * Tell master method WE added to export file 
	 		 */ 
			printf("%s=%s\n", ATTR_EXPORTED_T, location);

		exit(0);

	}

	/*
	 * To proceed to the MKNFSEXP we make sure that the operation is 
	 * grant and we did not have the location in the export file. 
	 */ 
	if ( (operation != ATTR_GRANT) || opt_cnt)
		exit(0);

	/*
	 * Lets add the export by building the mknfsexp command line
	 */ 
	argcnt=0;
	Args[argcnt++]=MKNFSEXP; Args[argcnt++]="-B"; Args[argcnt++]="-d"; 
	Args[argcnt++]=location; 
	bld_args( nfsopt, host_name, Args, &argcnt );
	Args[argcnt]=NULL; 

	if ( client_exec( &rc, &stdOut, Args ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );
	else if ( rc > 0 )
		nim_error( ERR_CMD, Args[0], niminfo.errstr, NULL );

	/*
	 * Tell master method WE added to export file 
	 */ 
	printf("%s=%s\n", ATTR_EXPORTED_T, location);

	exit(0);
}
