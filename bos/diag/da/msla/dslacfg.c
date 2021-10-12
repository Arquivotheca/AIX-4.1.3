static char sccsid[] = "@(#)25  1.10  src/bos/diag/da/msla/dslacfg.c, damsla, bos411, 9428A410j 12/10/92 09:03:36";
/*
 *   COMPONENT_NAME: DAMSLA
 *
 *   FUNCTIONS: chk_chg_cfg
 *		close_db
 *		def_cfg_self
 *		restore_cfg
 *		syscmd
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1991,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/sysconfig.h>

#include "diag/tm_input.h"
#include "dslacfg.h"

#define	 CHK_ERROR 	if ( rc != 0 ) { 		\
				close_db();		\
				return ;         \
			}

static int     hia_cfged       = FALSE ;
static int     gsw_cfged       = FALSE ;
int     self_cfged       = FALSE ;
int	self_uncfged     = FALSE ;
int	other_uncfged    = FALSE ;
int	other_undefed    = FALSE ;
int	self_defined     = FALSE ;
int	self_undefined   = FALSE ;
int	parent_cfged	 = FALSE ;
int 	diskette_based   = FALSE; /* executing off diskette flag           */


extern struct tm_input tm_input;
struct Class *cusdev;
struct Class *predev;

struct CuDv  self_cusobj;
struct CuDv  *self_par_cusobj;
struct CuDv  other_cusobj;
struct PdDv  self_pdobj;
struct PdDv  other_pdobj;
struct CuDv  *new_cusobj;
struct listinfo new_info;
struct listinfo parent_info;

struct CuAt  hia_cusobj;
struct CuDv  hia_pdobj;
struct Class *cuat_oc;

struct hia_conf  hia_data;

/*
 * NAME:
 *	chk_chg_cfg
 *                                                                    
 * FUNCTION:
 *	This function checks the configuration of MSLA cards and changes
 *	it if necessary.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This function is called by the main procedure which is forked and
 *	exec'd by the diagnostic supervisor, which runs as a process.
 *	This function runs an an application under the diagnostic subsystem.
 *                                                                   
 * (NOTES:)
 *	Opens the necessary ODM databases to obtain information about
 *	the current configuration of MSLA cards.
 *	Changes the configuration to suit the running of the 
 *	Diagnostics application for MSLA.
 *	Initializes the global variables to store the information
 *	of configuration changes.
 *
 * (RECOVERY OPERATION:)
 *	Software errors, such as failures to open, are handled by
 *	 closing the ODM databases and returning to the diagnostic
 *	application main component.
 *
 * (DATA STRUCTURES:)
 *	This function modifies the following global structures
 *		and variables:
 *	'self_cusobj', other_cusobj, new_cusobj, self_pdobj, other_pdobj
 *	self_uncfged. self_undefined, other_uncfged, self_cfged, self_defined
 *
 * RETURNS:
 *	To the calling routine;
 *	Pointer to the name of device to be tested.
 *	On error a NULL pointer is returned.
 */  
chk_chg_cfg(dev_name)
  char   dev_name[];
 {
	char   sstring[S_LENGTH];
	char   str_self[S_LENGTH];
	char   str_other[S_LENGTH];
	char   par_str[S_LENGTH];
	int    exenvflg;
	int    rc ;
	char   *cmd_str;
	char 	*malloc();

	cmd_str = malloc( MAX_CMD_LENGTH * sizeof(char) );
	if ( cmd_str == NULL ) 
	{
		return;
	}
	dev_name[0] = NULL;
	cusdev = NULL;
	predev = NULL;

        /* determine execution mode */
        exenvflg = ipl_mode( &diskette_based );

	/* dname will be of the form mslan, as in msla0 */
	sprintf( str_self," parent = %s ",tm_input.dname);
	sprintf( par_str," name = %s ",tm_input.dname);

/*.................... Open CuDv ...............................
 */
	cusdev = odm_open_class( CuDv_CLASS );
	if ( cusdev == NULL ) 
	{
		return;
	}
/*............. Access CuDv to get info. about device mslan ......
 */
	rc = (int)odm_get_obj(cusdev,str_self,&self_cusobj,TRUE);
	if ( rc == -1 )               /* system error */
	{
		close_db();
		return;
	}
	if ( rc == 0 )                 /* device not found */
	{
		self_cusobj.status = NOT_PRESENT;
        	self_par_cusobj = get_CuDv_list(CuDv_CLASS, par_str, 
						&parent_info, 1, 2);
        	if ( self_par_cusobj == (struct CuDv *) -1 || !parent_info.num )
		{
			close_db();
			return;
		}
	}

/*............... Open PdDV to get the 'prefix' of the child ............*
 *............... device of 'self MSLA'  ...............*
 *............... ie. to know if the child is 'gsw' or 'hia' ............*
 */
	predev = odm_open_class( PdDv_CLASS );
	if ( predev == NULL ) 
	{
		close_db();
		return;  /* no entry for device */
	}
	if ( self_cusobj.status != NOT_PRESENT ) 
	{
		sprintf(sstring,"uniquetype = '%s'",self_cusobj.PdDvLn_Lvalue );
		rc = (int)odm_get_obj(predev,sstring,&self_pdobj,TRUE);
		if (( rc == -1 ) || ( rc == 0 )) 
		{
			close_db();
			return;
		}
	}

	/* case 1
	/* if already configured as gsw or hia, note that and return */
	if ( self_cusobj.status == AVAILABLE ) 
	{
	strcpy(dev_name,DEVPATH);  /*init to "/dev" */
	strcat(dev_name,self_cusobj.name);
	}
	/* case 2.. defined but not configured */
	else if ( self_cusobj.status == DEFINED ) 
	{
		if ( ( rc = strcmp(self_pdobj.prefix,HIA_PREFIX) == 0 )) 
		{
			hia_cfged=TRUE;
		}
		else if ( ( rc = strcmp(self_pdobj.prefix,GSW_PREFIX) == 0 )) 
		{
			gsw_cfged=TRUE;
		}
		strcpy(cmd_str,CFG_GSW_CMD); /* same string for gsw/hia */
		rc = syscmd(cmd_str,self_cusobj.name, 0 );
		CHK_ERROR;
		self_cfged = TRUE;
		if ( dev_name == NULL )
		{
			close_db();
			return;
		}
		strcpy(dev_name,DEVPATH);  /*init to "/dev" */
		strcat(dev_name,self_cusobj.name);
	}

	/* Case 3 .. not defined as either gsw or hia,          */
	/* so configure as hia since multiple instances allowed */
	else if ( self_cusobj.status == NOT_PRESENT ) 
	{       /* defined but not available */
		if (self_par_cusobj->status == DEFINED ) 
		{
			if ( diskette_based )
				sprintf(cmd_str,"%s -l ", 
					self_par_cusobj->PdDvLn->Configure);
			else
				strcpy(cmd_str,CFG_PARENT);
			rc = syscmd(cmd_str,tm_input.dname, 0);
			CHK_ERROR;
			parent_cfged = TRUE;
			self_par_cusobj->status = AVAILABLE;  
		}
		if (self_par_cusobj->status == AVAILABLE ) 
		/* if msla is not defined as either gsw or hia
		   it should now be defined and configured as hia */
		{
			rc = def_cfg_self();
			CHK_ERROR;
			self_defined = TRUE;
			self_cfged = TRUE;
			if ( dev_name == NULL )
			{
				close_db();
				return;
			}
			strcpy(dev_name,DEVPATH);  /*init to "/dev" */
			strcat(dev_name,new_cusobj->name);
		}
	}
/*............ Close the CuDv and PdDv .............................*/
	rc = close_db();
	if ( rc != 0 ) {
		return;
	}
	return;
}



/**************************************************************************
 * NAME:
 *	def_cfg_self
 *                                                                    
 * FUNCTION:
 *	This function defines and configures the MSLA
 *	card under test as GSW.
 *                                                                    
 *
 * (DATA STRUCTURES:)
 *	This function modifies the following global structures
 *		and variables:
 *	self_cfged, self_defined
 *
 * RETURNS:
 *	To the calling routine;
 *	On successful reconfiguration: 0.
 *	On error: -1
 */  

int
def_cfg_self()
{
	int rc ;
	char   sstring[S_LENGTH];
	char   *cmd_str;
	/* char   *strcpy(); */
	char 	*malloc();
	struct PdDv  *pddv;
	struct listinfo gsw_info;

	cmd_str = malloc( MAX_CMD_LENGTH * sizeof(char) );
	if ( cmd_str == NULL ) {
		return(-1);
	}

	if ( diskette_based )
        {
                pddv = get_PdDv_list(PdDv_CLASS, "type = gsw", &gsw_info, 1, 1);
		if ( pddv == (struct PdDv *) -1  || !gsw_info.num )
			return ( -1); 
		sprintf(cmd_str,"%s -t gsw -c msla -s msla -w 0 -p ",pddv->Define);
	}
	else
		strcpy (cmd_str,DEF_HIA_CMD);

	rc = syscmd( cmd_str,tm_input.dname, 0 );
	if ( rc != 0 ) {
		free(cmd_str);
		return(-1);
	}
	sprintf( sstring," parent = '%s' ",tm_input.dname);
        new_cusobj = get_CuDv_list(CuDv_CLASS, sstring, &new_info, 1, 2);
        if ( new_cusobj == (struct CuDv *) -1  || !new_info.num )
	      {
		free(cmd_str);
                return ( -1);
	      }
	if ( diskette_based )
		sprintf(cmd_str,"%s -l ", new_cusobj->PdDvLn->Configure);
	else
		strcpy(cmd_str,CFG_HIA_CMD);
	rc = syscmd(cmd_str,new_cusobj->name, 0 );
	free(cmd_str);
	return(rc);
}

/*
 * NAME:
 *	syscmd
 *                                                                    
 * FUNCTION:
 *	This function uses the system call 'system()' to make the
 *	configuration changes.
 *                                                                    
 *
 * RETURNS:
 *	To the calling routine;
 *	On successful reconfiguration: 0.
 *	On error: -1
 */  
int
syscmd(cmd,lname,hia_flg) 
char *cmd;
char *lname;
int  hia_flg;
{
	int rc;
	extern int system();

	strcat(cmd,lname);
       /* if(hia_flg == 1)
	    restore_hia(cmd);         */
	strcat(cmd," > /dev/null");
#ifdef DEBUG_MSLA
	diag_asl_msg("\n Executing command [ %s ]\n",cmd);
#endif
	rc = system(cmd);
	if ( rc == -1 ) {
		return(-1); 
	} 
	return(0);
}

/*
 * NAME:
 *	restore_cfg
 *                                                                    
 * FUNCTION:
 *	This function restores the configuration of MSLA cards to the
 *	 original condition.
 *                                                                    
 *                                                                   
 * (NOTES:)
 *	Obtains information on configuration changes from the
 *      global variables 'self_ucfged' etc.
 *	Opens the necessary ODM databases to obtain information about
 *	the current configuration of MSLA cards.
 *	Restores the configuration to suit the running of the 
 *	Diagnostics application for MSLA.
 *
 * (RECOVERY OPERATION:)
 *	Software errors, such as failures to open, are handled by
 *	 closing the ODM databases and returning to the diagnostic
 *	application main component.
 *
 *
 * RETURNS:
 *	To the calling routine;
 *	On successful reconfiguration: 0.
 *	On error: -1
 */  
int
restore_cfg()
{
	int rc;
	char   *cmd_str;
	char 	*malloc();
	char   sstring[S_LENGTH];

	cusdev	= NULL;
	cmd_str = malloc( MAX_CMD_LENGTH * sizeof(char) );
	if ( cmd_str == NULL ) {
		return(-1);
	}
/*....... Unconfigure and undefine self GSW, if it was done by us ....*/
	if (( self_cfged == TRUE ) && ( self_defined == TRUE ) ) {
		if ( diskette_based )
			sprintf(cmd_str,"%s -l ",new_cusobj->PdDvLn->Unconfigure);
		else
			strcpy(cmd_str,UNCFG_CMD);
		rc = syscmd( cmd_str,new_cusobj->name, 0 );
		if ( rc != 0 ) {
			return(-1);
		}
		if ( diskette_based )
			sprintf(cmd_str,"%s -l ",new_cusobj->PdDvLn->Undefine);
		else
			strcpy(cmd_str,UNDEF_CMD);
		rc = syscmd( cmd_str ,new_cusobj->name, 0);
		if ( rc != 0 ) {
			return(-1);
		}
	}
	else if (( self_cfged == TRUE ) && ( self_defined == FALSE ) ) {
		strcpy(cmd_str,UNCFG_CMD);
		rc = syscmd( cmd_str,self_cusobj.name, 0 );
		if ( rc != 0 ) {
			return(-1);
		}
	}
	if ( parent_cfged == TRUE ) {
		if ( diskette_based )
			sprintf(cmd_str,"%s -l ",
					self_par_cusobj->PdDvLn->Unconfigure);
		else
			strcpy(cmd_str,UNCFG_CMD);
		rc = syscmd( cmd_str,tm_input.dname, 0);
		if ( rc != 0 ) {
			return(-1);
		}
	}
/*...... Define the other MSLA as GSW if it was undefined by us ...*/
	if (( other_undefed == TRUE ) && ( other_uncfged == FALSE )) {
		strcpy(cmd_str,DEF_GSW_CMD);
		rc = syscmd( cmd_str,other_cusobj.parent, 0);
	}
/*...... Configure the other MSLA as GSW if it was unconfigured by us ...*/
	if (( other_undefed == TRUE ) && ( other_uncfged == TRUE )) {
		strcpy(cmd_str,DEF_CFG_GSW_CMD);
		rc = syscmd( cmd_str,other_cusobj.parent, 0);
	}
/*...... Define and Configure self MSLA back to HIA if needed ....*/
	if ( self_undefined == TRUE ) {
		strcpy(cmd_str,DEF_HIA_CMD);
		rc = syscmd( cmd_str,tm_input.dname, 1);
		if ( rc != 0 ) {
			return(-1);
		}
	}
	if ( self_uncfged == TRUE ) {
		/*......... Open CuDv .......*/
		cusdev = odm_open_class( CuDv_CLASS );
		if ( cusdev == NULL ) {
			return(-1);
		}
		/*...... Access CuDv to get info. about 'self MSLA '.....*/
		sprintf( sstring," parent = '%s' ",tm_input.dname);
		rc = (int)odm_get_obj(cusdev,sstring,&new_cusobj,TRUE);
		if (( rc == -1 ) || ( rc == 0 )) {
			close_db();
			return(-1);
		}
		strcpy(cmd_str,CFG_HIA_CMD);
		rc = syscmd(cmd_str,new_cusobj->name, 0 );
		if ( rc != 0 ) {
			close_db();
			return(-1);
		}
	}
	rc = close_db();
	return(rc);
}

/*
 * NAME:
 *	close_db
 *                                                                    
 * FUNCTION:
 *	This function closes the ODM databases which are open.
 *	 original condition.
 *                                                                    
 *
 * RETURNS:
 *	To the calling routine;
 *	On successful reconfiguration: 0.
 *	On error: -1
 */  
int
close_db()
{
	int rc;

	/*...... Close PdDv .......*/
	if ( predev != NULL ) 
	{
		rc = odm_close_class( predev );
		if ( rc < 0 ) 
		{
			return(-1);
		}
	}

	/*...... Close CuDv .......*/
	if ( cusdev != NULL ) 
	{
		rc = odm_close_class ( cusdev );
		if ( rc < 0 ) 
		{
			return (-1);
		}
	}

	/*...... Close CuAt .......*/
	if ( cuat_oc != NULL ) 
	{
		rc = odm_close_class (cuat_oc);
		if ( rc < 0 ) 
		{
			return (-1);
		}
	}
	return(0);
}
