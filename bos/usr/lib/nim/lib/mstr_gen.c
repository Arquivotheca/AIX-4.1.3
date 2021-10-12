static char	sccs_id[] = " @(#)90 1.13.1.2  src/bos/usr/lib/nim/lib/mstr_gen.c, cmdnim, bos411, 9436D411a  9/7/94  15:11:12";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: 
 *		mstr_exit
 *		mstr_init
 *		stat_file
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
 *                        NIM master function library
 *
 * this library contains NIM database manipulation routines, which should only
 *		be accessed by the NIM master
 ******************************************************************************/

#include "cmdnim.h"
#include "cmdnim_obj.h"
#include <sys/vmount.h>

struct	Class *nim_object_CLASS;
struct	Class *nim_attr_CLASS;
struct	Class *nim_pdattr_CLASS;

/*---------------------------- mstr_exit         -------------------------------
 *
 * NAME: mstr_exit
 *
 * FUNCTION:
 *		performs common exit processing for NIM master commands
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
 *		global:
 *			backups
 *
 * RETURNS:
 *
 * OUTPUT:
 *-----------------------------------------------------------------------------*/

void *
mstr_exit()

{	

	VERBOSE4("         mstr_exit\n",NULL,NULL,NULL,NULL)

	/* are there any objects which have been backed up & need restoring? */
	if ( backups.num > 0 )
		restore_backups( &backups );

	/* perform common exit processing */
	nim_exit();

	/* 
	 * release all locks held by this process 
	 *  _IFF_ it is the group leader
	 */ 
	if ( niminfo.pid == niminfo.pgrp )
		rm_all_obj_locks();

	/* odm terminate */
	odm_terminate();

} /* end of mstr_exit */
	
/*---------------------------- mstr_init          ------------------------------
*
* NAME: mstr_init
*
* FUNCTION:
*		performs common initialization processing for all NIM commands
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		this function initializes the global NIM variables which are defined
*			in each NIM command; search for the string "NIM globals" in the
*			command .c file to find their definition
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*	parameters:
*		name		= command name
*			err_policy			= a #define from cmdmstr_defines.h which specifies
*											how to treat errors
*			siginit				= when non-NULL, points to function which initializes
*											signal handling; otherwise, nim_siginit will
*											be used
*		global:
*			niminfo
*			errsig
*
* RETURNS: (int)
*		SUCCESS					= success
*		FAILURE					= failure
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
mstr_init(	char *name, 
				unsigned int err_policy, 
				void (*siginit)() )

{	

	VERBOSE4("         mstr_init: name=%s;\n",name,NULL,NULL,NULL)

	/* common initialization */
	nim_init( name, err_policy, siginit, mstr_exit );

	/* make sure that this is the NIM "master" - it is the only machine which */
	/*		can access the NIM database */
	if ( strcmp(niminfo.nim_name,ATTR_MASTER_T) )
		/* error - this is NOT the master! */
		nim_error( ERR_NOT_MASTER, NULL, NULL, NULL );

	/* ODM */
	if (odm_initialize() == -1)
		nim_error(ERR_ODM, (char *) odmerrno, NULL, NULL);

	/* Now mount the classes we need */ 
	if (((nim_object_CLASS=odm_mount_class(NIM_OBJECT_CLASSNAME)) == -1) || 
			  nim_object_CLASS == NULL) 
		nim_error(ERR_ODM, (char *) odmerrno, NULL, NULL);

	if (((nim_attr_CLASS=odm_mount_class(NIM_ATTR_CLASSNAME)) == -1) || 
			  nim_attr_CLASS == NULL) 
		nim_error(ERR_ODM, (char *) odmerrno, NULL, NULL);

	if (((nim_pdattr_CLASS=odm_mount_class(NIM_PDATTR_CLASSNAME)) == -1) || 
			  nim_pdattr_CLASS == NULL) 
		nim_error(ERR_ODM, (char *) odmerrno, NULL, NULL);

	/* initialize the backups LIST */
	if ( ! get_list_space( &backups, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	return( SUCCESS );

} /* end of mstr_init */

/*---------------------------- stat_file         ------------------------------
*
* NAME: stat_file
*
* FUNCTION:
*		executes the C_STAT method on the specified server to "stat" the specified
*			pathname
*		if st_mode and/or st_vfstype are not supplied, and res_type is, this
*			function will search the database for the required values
*		otherwise, defaults will be used
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
*			st_mode				= stat.st_mode value to use
*			st_vfstype			= stat.st_vfstype value to use
*			res_type				= resource type for <location>
*			server				= machine name of server
*			location				= pathname to stat
*		global:
*
* RETURNS: (int)
*		SUCCESS					= <location> has the specified st_mode & st_vfstype
*		FAILURE					= either can't stat file or it failed checks
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
stat_file(	char *st_mode,
				char *st_vfstype,
				char *res_type,
				char *server,
				char *location )

{	NIM_PDATTR( type_pdattr, pinfo )
	NIM_PDATTR( mode_pdattr, minfo )
	NIM_PDATTR( vfstypes_pdattr, vinfo )
	char *args[] = { C_STAT, NULL, NULL, NULL, NULL };
	int rc;
	FILE *c_stdout=NULL;

	VERBOSE2("   stat_file: server=%s; location=%s;\n",server,location,NULL,NULL)

	/* if res_type supplied, then assume we'll need the pdattr for this type */
	if ( res_type != NULL )
	{
		if ( get_pdattr(	&type_pdattr, &pinfo, ATTR_CLASS_RESOURCES,
								ATTR_SUBCLASS_TYPE, 0, 0, res_type ) <= 0 )
			ERROR( ERR_BAD_TYPE_FOR, res_type, ATTR_CLASS_RESOURCES_T, NULL )
	}

	/* st_mode supplied? */
	if ( st_mode == NULL )
	{
		/* if res_type supplied, look in database for required mode */
		if (	(res_type != NULL) &&
				(get_pdattr(	&mode_pdattr, &minfo, 0, 0, type_pdattr->attr, 
									ATTR_MODE, NULL ) > 0) )
				st_mode = mode_pdattr->value;

		/* st_mode is "directory" by default */
		if ( st_mode == NULL )
		{
			st_mode = nim_malloc( MAX_TMP );
			sprintf( st_mode, "0%o", S_IFDIR );
		}
	}

	/* st_vfstype supplied? */
	if ( st_vfstype == NULL )
	{
		/* if res_type supplied, look in database for acceptable vfstypes */
		if (	(res_type != NULL) &&
				(get_pdattr(	&vfstypes_pdattr, &vinfo, 0, 0, type_pdattr->attr, 
									ATTR_VFSTYPE, NULL ) > 0) )
				st_vfstype = vfstypes_pdattr->value;

		/* st_vfstype is "JFS" by default */
		if ( st_vfstype == NULL )
		{
			st_vfstype = nim_malloc( MAX_TMP );
			sprintf( st_vfstype, "%d", MNT_JFS );
		}
	}

	VERBOSE2("      file must have: st_mode = 0%o; st_vfstype = \"%s\"\n",
				st_mode,st_vfstype,NULL,NULL)
	VERBOSE2("      stat-ing the file \"%s\"\n",location,NULL,NULL,NULL)

	/* build parameter string for C_STAT */
	args[1] = nim_malloc( strlen( ATTR_LOCATION_T ) + strlen( location ) + 4 );
	sprintf( args[1], "-a%s=%s", ATTR_LOCATION_T, location );
	args[2] = nim_malloc( strlen( ATTR_MODE_T ) + strlen( st_mode ) + 4 );
	sprintf( args[2], "-a%s=%s", ATTR_MODE_T, st_mode );
	args[3] = nim_malloc( strlen( ATTR_VFSTYPE_T ) + strlen( st_vfstype ) + 4 );
	sprintf( args[3], "-a%s=%s", ATTR_VFSTYPE_T, st_vfstype );

	/* stat the resource location */
	if ( master_exec( server, &rc, &c_stdout, args ) == FAILURE )
		return( FAILURE );
	else if ( rc > 0 )
		ERROR( ERR_METHOD, server, niminfo.errstr, NULL )

	return( SUCCESS );

} /* end of stat_file */
	
/* ---------------------------- ok_to_display
 *
 * NAME: ok_to_display
 *
 * FUNCTION:	Check the pdattr mask and see if its ok to display this
 *		thingy
 *
 * RETURNS: int
 *
 * --------------------------------------------------------------------- */
int
ok_to_display(int attr)
{
	ODMQUERY
	struct nim_pdattr pdattr;

	if (force)
		return( TRUE );
	sprintf( query, "attr=%d", attr);
	if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) <= 0 )
		return( FALSE );
	if ( pdattr.mask &  PDATTR_MASK_DISPLAY )
		return( TRUE );
	return( FALSE );
}

/* ---------------------------- mstr_what
 *
 * NAME: mstr_what
 *
 * FUNCTION: Romp through the array for the required attributes then romp 
 * 	through for the optional attributes printing them out as we go.. 
 *
 * DATA STRUCTURES:
 *		global:
 *			valid_attrs: 
 *
 * RETURNS: (void)
 *
 * ---------------------------------------------------------------------
 */

int
mstr_what(VALID_ATTR 	valid_attrs[], VALID_ATTR resources[] )
{

	int		va_indx;
	int		print_msg = TRUE;
	VALID_ATTR 	*va_ptr;

	/* 
	 * If we have resource to show off do it...
	 */
	if (resources != NULL && resources[0].name != NULL)
		res_query(resources);

	/*
	 * If we dont have attrs dont do the rest of this ! 
	 */
	if (valid_attrs == NULL || valid_attrs[0].name == NULL)
		return(0);

	/* 
	 * check for required attrs 
	 */
	for (va_indx = 0; valid_attrs[va_indx].pdattr; va_indx++) {
		va_ptr = &valid_attrs[va_indx];
		if ( va_ptr->required && ok_to_display(va_ptr->pdattr)) {
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
		if ( !va_ptr->required && ok_to_display(va_ptr->pdattr) )
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
/* --------------------------- res_query         
 *
 * NAME: res_query
 *
 * FUNCTION:
 *          displays the required and optional resources required by this operation
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *               parameters:
 *               global:
 *                       resources
 *                       resources
 *
 * RETURNS: (int)
 *               exits with SUCCESS
 *
 * OUTPUT:
 * ----------------------------------------------------------------------------*/

int
res_query(VALID_ATTR resources[])

{       int print_mess=TRUE, i;

	for (i=0; resources[i].name != NULL; i++) 
		if ( resources[i].required ) {
			if (print_mess) {
				printf("\n%s\n", MSG_msg(MSG_REQUIRED_RES));
				print_mess=FALSE;
			}
			printf("\t%s\n", ATTR_msg(resources[i].pdattr));
		}	

	print_mess=TRUE;	

	for (i=0; resources[i].name != NULL; i++)
		if ( ! resources[i].required ) {
			if (print_mess) {
				printf("\n%s\n", MSG_msg(MSG_OPTIONAL_RES));
				print_mess=FALSE;
			}
			printf("\t%s\n", ATTR_msg(resources[i].pdattr));
		}
	return( 0 );

} /* end of res_query */

/*---------------------------- add_nim_hosts     
 *
 * NAME: add_nim_hosts
 *
 * FUNCTION:
 *  adds the specified host to the nim_hosts list if its not already there
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 * parameters:
 *		nim_hosts	= ptr to char ptr which contains hosts list
 *		nimif		= ptr to nim_if struct for new host to add
 * global:
 *
 * RETURNS: (int)
 *  SUCCESS		= host added (or was already there)
 *
 * OUTPUT:
 *-------------------------------------------------------------------------
 */

int
add_nim_hosts(	char **nim_hosts,
					struct nim_if *nimif )

{	
	int	len;
	char	*new_stanza;

	/* construct a new stanza */
	len = strlen(nimif->ip) + strlen(nimif->hostname) + 4;
	new_stanza = nim_malloc( len );
	sprintf( new_stanza, " %s:%s ", nimif->ip, nimif->hostname );

	/* initialize nim_hosts? */
	if ( *nim_hosts == NULL ) {
		*nim_hosts = new_stanza;
	} else if ( strstr( *nim_hosts, new_stanza ) == NULL ) {
		/* new host - add to end of list */
		*nim_hosts = 
		   nim_realloc( *nim_hosts, (strlen(*nim_hosts) + 1), len );
		strcat( *nim_hosts, new_stanza );
	}

	return( SUCCESS );

} /* end of add_nim_hosts */



/* --------------------------- add_nim_mounts    
 *
 * NAME: add_nim_mounts
 *
 * FUNCTION:
 *  adds the specified mount to the nim_mounts list if its not already there
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *   parameters:
 *		nim_mounts	= ptr to char ptr which contains mount info
 *		hostname	= hostname of server of resource to be mounted
 *		location	= pathname of resource on <server>
 *		raccess		= ptr to res_access struct for mount resource
 *		mntpnt		= local mount point in the RAM filesystem
 *		mntpnt_type	= either "file" or "dir"
 *
 * RETURNS: (int)
 *   SUCCESS	= mount added (or was already there)
 *
 * OUTPUT:
 * --------------------------------------------------------------------------
 */

int
add_nim_mounts(	char **nim_mounts,
						char *hostname,
						char *location,
						char *mntpnt,
						char *mntpnt_type )

{	
	int	len;
	char	*new_stanza;

	/* construct a new stanza */
	len = strlen(hostname) + strlen(location) + 
	    strlen(mntpnt) + strlen(mntpnt_type) + 6;
	new_stanza = nim_malloc( len );
	sprintf( new_stanza, " %s:%s:%s:%s ", hostname, location, 
			mntpnt, mntpnt_type);

	/* initialize nim_hosts? */
	if ( *nim_mounts == NULL ) {
		*nim_mounts = new_stanza;
	} else if ( strstr( *nim_mounts, new_stanza ) == NULL ) {
		/* new host - add to end of list */
		*nim_mounts = 
		   nim_realloc( *nim_mounts, (strlen(*nim_mounts) + 1), len );
		strcat( *nim_mounts, new_stanza );
	}

	return( SUCCESS );

} /* end of add_nim_mounts */


/* --------------------------- mk_niminfo        
 *
 * NAME: mk_niminfo
 *
 * FUNCTION:
 *		creates a .info file for the specified machine
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		sets errstr on failure
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *  parameters:
 *    mac	= target's nim_object
 *    list	= LIST of res_access structs
 *    fp	= FILE ptr to .info file (can be local or remote)
 *
 * RETURNS: (int)
 *	SUCCESS		= .info file created
 *	FAILURE		= unable to create some or all of the file
 *
 * OUTPUT:
 * ------------------------------------------------------------------------
 */
 
#define LPP_SOURCE_MNT_PNT	"/SPOT/usr/sys/inst.images"
#define RTE_PATHNAME		LPP_SOURCE_MNT_PNT"/bos"

int
mk_niminfo(	struct nim_object *mac,
		LIST *list,
		FILE *fp )

{	
	NIM_OBJECT( robj, rinfo )
	NIM_ATTR( mp, mp_info )
	struct res_access *raccess;
	int	i;
	char	*nim_hosts = NULL;
	char	*nim_mounts = NULL;
	char	mntpnt[MAX_TMP];
	char	tmp[MAX_TMP];
	struct nim_if nimif;
	int	debug = FALSE;
	int	pull = FALSE;
	char	*source = NULL;
	struct res_access *simages = NULL;
	struct res_access *mksysb = NULL;
	int	source_type = ATTR_RTE;
	int	size = 0;
	long	master_id=0;

	VERBOSE("   creating a niminfo file\n", NULL, NULL, NULL, NULL)

	/* add the target's PIF info the the NIM_HOSTS list */
	if ( (i = find_attr( mac, NULL, NULL, 1, ATTR_IF )) < 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, PIF, mac->name, NULL )
	else if ( s2nim_if( mac->attrs[i].value, &nimif ) == FAILURE )
		ERROR( ERR_NET_INFO, PIF, mac->name, NULL )

	add_nim_hosts( &nim_hosts, &nimif );

	/* look for flags */
	debug = (find_attr_ass( &attr_ass, ATTR_DEBUG ) >= 0 );
	pull = (find_attr_ass( &attr_ass, ATTR_PULL ) >= 0 );
	source = attr_value( &attr_ass, ATTR_SOURCE );
	if ( source != NULL ) {
		if ( strcmp( source, ATTR_MKSYSB_T ) == 0 )
			source_type = ATTR_MKSYSB;
		else if ( strcmp( source, ATTR_SPOT_T ) == 0 )
			source_type = ATTR_SPOT;
	}

	/* 
	 * make sure there's valid info in the res_access LIST 
	 * we're expecting the last entry to be the master's info 
	 */
	if ( list->num > 0 ) {
		raccess = (struct res_access *) list->list[ list->num - 1 ];
		if ( strcmp( raccess->name, ATTR_MASTER_T ) != 0 )
			return( FAILURE );

		/* add the master's IP info to the NIM_HOSTS variable */
		add_nim_hosts( &nim_hosts, &raccess->nimif );
	} else
		return( FAILURE );

	fprintf(fp, "#------------------ Network Install Manager ---------------\n");
	fprintf(fp, "# warning - this file contains NIM configuration information\n");
	fprintf(fp, "# \tand should only be updated by NIM\n");

	/* add comments if any specified for this machine */
	if ( (i = find_attr( mac, NULL, NULL, 0, ATTR_COMMENTS )) >= 0 )
		fprintf( fp, "# %s\n\n", mac->attrs[i].value );

	/* NIM_NAME */
	fprintf( fp, "%s=%s\n", NIM_NAME, mac->name );

	/* hostname of target's primary interface */
	fprintf( fp, "%s=%s\n", NIM_HOSTNAME, nimif.hostname );

	/* configuration type */
	fprintf( fp, "%s=%s\n", NIM_CONFIGURATION, mac->type->name );

	/* hostname of master's interface which this mac should use */
	fprintf( fp, "%s=%s\n", NIM_MASTER_HOSTNAME, raccess->nimif.hostname);

	/* port # for nimesis */
	master_id = get_id(ATTR_MASTER_T);
	if ( get_attr( &mp, &mp_info, master_id, NULL, 0, ATTR_MASTER_PORT ) > 0 )
		fprintf( fp, "%s=%s\n", NIM_MASTER_PORT, mp[0].value );

	/* use the NIM pull interface? */
	if ( pull )
		fprintf( fp, "%s=yes\n", NIM_MENU );

	/* 
	 * if boot processing is going to occur, set RC_CONFIG 
	 * based on Cstate 
	 */
	switch ( get_state( mac->id, NULL, ATTR_CSTATE ) ) {
	case STATE_BOS_INST :
		fprintf( fp, "%s=%s\n", NIM_RC_CONFIG, RC_BOS_INST );

		if ( debug )
			fprintf( fp, "%s=\"%s%s -v\"\n", 
			    NIM_BOSINST_ENV, SPOT_OFFSET, C_BOSINST_ENV );
		else
			fprintf( fp, "%s=\"%s%s\"\n", 
			    NIM_BOSINST_ENV, SPOT_OFFSET, C_BOSINST_ENV );

		if ( debug )
			fprintf( fp, "%s=\"%s%s -v -a %s=%s\"\n", 
			    NIM_BOSINST_RECOVER, SPOT_OFFSET, C_BOSINST_ENV, 
			    ATTR_HOSTNAME_T, nimif.hostname );
		else
			fprintf( fp, "%s=\"%s%s -a %s=%s\"\n",
			    NIM_BOSINST_RECOVER, SPOT_OFFSET, C_BOSINST_ENV, 
			    ATTR_HOSTNAME_T, nimif.hostname );
		break;

	case STATE_DTLS_INIT :
		/* 
		 * has client already been initialized as a dataless machine? 
		 */
		if ( find_attr( mac, NULL, NULL, 0, ATTR_ROOT_INITIALIZED ) < 0 ) {
			/* 
			 * no previous initialization.   setup the env variables 
			 * to perform dataless install 
			 */
			fprintf( fp, "%s=yes\n", NIM_MK_DATALESS );

			/* check for dataless specific attrs */
			if ( (i = find_attr_ass( &attr_ass, ATTR_SIZE )) >= 0 ) {
				/* 
				 * ATTR_SIZE is specified in Megabytes, but rc.dd_boot wants 
				 *		paging size in number of logical partitions 
				 * since the logical partition size is 4 meg, divide by 4 
				 */
				size = (int) strtol( attr_ass.list[i]->value, NULL, 0 );
				size /= 4;
				if ( size <= 0 )
					size = 1;
				fprintf( fp, "%s=%d\n", NIM_DTLS_PAGING_SIZE, size );
			}
			i = find_attr( mac, NULL, NULL, 0, ATTR_TMP );
			if (	(find_attr( mac, NULL, NULL, 0, ATTR_HOME ) < 0) && 
			    (find_attr( mac, NULL, NULL, 0, ATTR_SHARED_HOME ) < 0) ) {
				if ( i < 0 )
					fprintf( fp, "%s=\"tmp home\"\n", NIM_DTLS_LOCAL_FS );
				else
					fprintf( fp, "%s=\"home\"\n", NIM_DTLS_LOCAL_FS );
			} else if ( i < 0 )
				fprintf( fp, "%s=\"tmp\"\n", NIM_DTLS_LOCAL_FS );
		}

		/* NOTE - no break here because we want to continue into DD_BOOT */

	case STATE_DKLS_INIT :
	case STATE_DD_BOOT :
		fprintf( fp, "%s=%s\n", NIM_RC_CONFIG, RC_DD_BOOT );
		break;

	case STATE_DIAG :
		fprintf( fp, "%s=%s\n", NIM_RC_CONFIG, RC_DIAG );
		break;
	}

	/*
	 * look for specific resources which need a variable set in the .info file 
	 * NOTE that because each resource is served by a machine, we need the 
	 *		interface info in order to understand how to access the resource
	 * so, we'll build a list of ip:hostname stanzas which we'll then use
	 *		to initialize the NIM_HOSTS variable with 
	 * also, we need to build a list of resource which need to be mounted 
	 */
	for (i = 0; i < list->num; i++) {
		raccess = (struct res_access *) list->list[i];

		/* add the servers to the NIM_HOSTS list */
		add_nim_hosts( &nim_hosts, &raccess->nimif );

		/*
		 * NOTE that in the following section, "/tmp" is never used as a
		 *		local mount point.  This is avoided because BOS install 
		 *		overmounts /tmp early in the install process 
		 */

		switch ( raccess->type ) {
		case ATTR_ROOT	:
			fprintf( fp, "%s=%s:%s\n", NIM_ROOT,
			    raccess->nimif.hostname, raccess->location );
			break;

		case ATTR_SPOT :
			fprintf( fp, "%s=%s:%s\n", NIM_SPOT,
			    raccess->nimif.hostname, raccess->location );
			break;

		case ATTR_DUMP :
			fprintf( fp, "%s=%s:%s\n", NIM_DUMP,
			    raccess->nimif.hostname, raccess->location );
			break;

		case ATTR_LPP_SOURCE :
			simages = raccess;
			add_nim_mounts(	&nim_mounts, raccess->nimif.hostname, 
			    raccess->location,
			    "/SPOT/usr/sys/inst.images", "dir" );
			break;

		case ATTR_MKSYSB :
			/* we set the mksysb info after this loop - look below */
			mksysb = raccess;
			break;

		case ATTR_BOSINST_DATA :
			sprintf( mntpnt, "/%s", NIM_BOSINST_DATA_T );
			fprintf( fp, "%s=%s\n", NIM_BOSINST_DATA, mntpnt );
			add_nim_mounts(	&nim_mounts, raccess->nimif.hostname,
			    raccess->location, mntpnt, "file" );
			break;

		case ATTR_IMAGE_DATA :
			sprintf( mntpnt, "/%s", NIM_IMAGE_DATA_T );
			fprintf( fp, "%s=%s\n", NIM_IMAGE_DATA, mntpnt );
			add_nim_mounts(	&nim_mounts, raccess->nimif.hostname,
			    raccess->location, mntpnt, "file" );
			break;

		case ATTR_NIM_SCRIPT :
			sprintf( tmp, "%s:%s/%s.script", 
			    raccess->nimif.hostname, raccess->location, mac->name );
			if ( debug )
				fprintf( fp, "%s=\"%s%s -v -a %s=%s\"\n", NIM_CUSTOM, SPOT_OFFSET,
				    C_SCRIPT, ATTR_LOCATION_T, tmp );
			else
				fprintf( fp, "%s=\"%s%s -a %s=%s\"\n", NIM_CUSTOM, SPOT_OFFSET,
				    C_SCRIPT, ATTR_LOCATION_T, tmp );
			break;

		case ATTR_LOG :
			fprintf( fp, "%s=\" %s:%s \"\n", NIM_LOG, raccess->nimif.hostname,
			    raccess->location );
			add_nim_mounts(	&nim_mounts, raccess->nimif.hostname,
			    raccess->location, "/var/adm/ras", "dir" );
			break;
		}
	}

	if (	(mac->type->attr != ATTR_DISKLESS) && 
	    (mac->type->attr != ATTR_DATALESS) ) {
		/*
		 * set NIM_BOS_IMAGE/FORMAT based on the type of source specified 
		 * NOTE that if "source" was not specified, we'll use "rte" by default 
		 */
		if ( (source_type == ATTR_MKSYSB) && (mksysb != NULL) ) {
			/* use the mksysb image */
			sprintf( mntpnt, "/%s", NIM_BOS_IMAGE_T );
			fprintf( fp, "%s=%s\n", NIM_BOS_IMAGE, mntpnt );
			fprintf( fp, "%s=%s\n", NIM_BOS_FORMAT, ATTR_MKSYSB_T );
			add_nim_mounts(	&nim_mounts, mksysb->nimif.hostname,
			    mksysb->location, mntpnt, "file" );
		} 
		else if ( source_type == ATTR_SPOT ) {
			/* use the SPOT as the source for the BOS image */
			fprintf( fp, "%s=/SPOT\n", NIM_BOS_IMAGE );
			fprintf( fp, "%s=%s\n", NIM_BOS_FORMAT, ATTR_SPOT_T );
		} 
		else {
			/* use the BOS rte image which resides in the lpp_source */
			fprintf( fp, "%s=%s\n", NIM_BOS_IMAGE, RTE_PATHNAME );
			fprintf( fp, "%s=%s\n", NIM_BOS_FORMAT, ATTR_RTE_T );
		}

		/*
		 * See if this machine is a /usr spot server. 
		 */
		i = -1;
		while ( (i=find_attr( mac, &i, NULL, 0, ATTR_SERVES )) >= 0) { 
			if ( lag_object( 0, mac->attrs[i].value, &robj, &rinfo ) > 0 ) {
				if (	(robj->type->attr == ATTR_SPOT) &&
						(find_attr( robj, NULL , "^/usr", 0, ATTR_LOCATION ) >= 0) ) {
					fprintf( fp, "%s=%s\n", NIM_USR_SPOT, robj->name );
					uaf_object( robj, &rinfo, FALSE );
					break;
				}
				uaf_object( robj, &rinfo, FALSE );
			}
		}
	}

	/* add the NIM hosts & mounts */
	fprintf( fp, "%s=\"%s\"\n", NIM_HOSTS, nim_hosts );
	fprintf( fp, "%s=\"%s\"\n", NIM_MOUNTS, nim_mounts );

	/* add NIM routing info */
	return( write_nim_routes( mac, fp ) );

} /* end of mk_niminfo */
