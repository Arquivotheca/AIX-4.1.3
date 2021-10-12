static char sccsid[] = "@(#)51	1.13  src/bos/usr/lib/methods/chghia/chgssla.c, cfgmethods, bos411, 9428A410j 12/11/90 17:04:20";

/* 
 * COMPONENT_NAME: (CFGMETH) CHGSSLA (change method for msla adapter, ssla mode)
 *
 * FUNCTIONS : 	change_device,update_db
 * 
 * ORIGINS : 27 
 * 
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights -  Use, Duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include <sys/cfgdb.h>
#include <sys/sysconfig.h>
#include <string.h>
#include <sys/cfgodm.h> 
#include <cf.h>
#include "pparms.h"
#include "cfgdebug.h"

#define	S_LENGTH	256

/*
 * NAME     : check_parms 
 *
 * FUNCTION : This function checks the validity of the attributes. 
 *
 * EXECUTION ENVIRONMENT :
 *	This function is called by the change_device function to  
 *	check the validity of the attributes of the adapter.	
 * 
 * NOTES :
 *	
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 * 	Returns  0 on success, > 0 on failure.
 */


int
check_parms(newatt, Pflag, Tflag, lname, parent, location, badattr)
struct attr *newatt;	/* list of attributes passed to change method */
int	Pflag;		/* if Pflag == 1 then the -P flag was passed */
			/*             0 then the -P flag was NOT passed */
int	Tflag;		/* if Tflag == 1 then the -T flag was passed */
			/*             0 then the -T flag was NOT passed */
char	*lname;		/* logical name of device being changed */
char	*parent;	
char	*location;	
char	*badattr;	/* Place to strcpy() list of bad attr names */
{
	struct	CuDv	cusobj;		/* customized device object 	*/
	struct	Class	*preatt;	/* predefined attribute handle 	*/
	struct	Class	*cusatt;	/* customized attribute handle 	*/
	struct	Class	*cusdev;	/* customized device handle 	*/
	char	sstring[S_LENGTH];	/* search criteria string 	*/
	int	rc ;			/* return code 			*/
	short	new_num_sess;
	short	new_chan_addr;
	short	new_lower_la;
	short	new_num_5080_sess;
	short	new_lower_5080_la;

	DEBUG_0("chgssla: In check_parms\n")
	/* open the customized object class */
	if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1) 
	{	DEBUG_0("chgssla: error in opening CuDv\n")
		return(E_ODMOPEN);
	}
	/* get customized object */
	sprintf(sstring,"name = '%s'",lname);
	rc = (int)odm_get_obj(cusdev,sstring,&cusobj,TRUE);
	if(rc == 0){
		DEBUG_0("chgssla: error in getting object\n")
		return(E_NOCuDv);
	}
	else if (rc == -1){
		DEBUG_0("chgssla: get_obj failed \n")
		return(E_ODMGET);
	}
	/* open the predefined attribute class */
	if ((int)(preatt = odm_open_class(PdAt_CLASS)) == -1)
	{	DEBUG_0("chgssla: cannot open PdAt\n")
		return(E_ODMOPEN);
	}
	/* open the customized attribute class */
	if ((int)(cusatt = odm_open_class(CuAt_CLASS)) == -1)
	{	DEBUG_0("chgssla: cannot open CuAt\n")
		return(E_ODMOPEN);
	}

	/* get the new value for the attribute num_sessions   */
	rc = getatt(&new_num_sess,'h',cusatt,preatt,lname,
		cusobj.PdDvLn_Lvalue,"num_sessions",newatt);
	if(rc > 0){
		DEBUG_0("chgssla: check_parms, error in getting num_sessions\n")
		return(rc);
	}

	/* get the new value for the attribute lower_bond */
	rc = getatt(&new_lower_la,'h',cusatt,preatt,lname,
		cusobj.PdDvLn_Lvalue,"lower_bond", newatt);
	if(rc > 0){
		DEBUG_0("chgssla: check_parms, error in getting lower_bond\n")
		return(rc);
	}

	if ((new_num_sess + new_lower_la ) > 16)
	{
	    DEBUG_2("chghia: Invalid value for num_sessions (0x%x) or lower_bond (0x%x)\n",
		new_num_sess,new_lower_la)
		strcpy( badattr, "num_sessions" );
		return(E_ATTRVAL);
	}

	/* get the new 5080 values */
	rc = getatt(&new_num_5080_sess,'h',cusatt,preatt,lname,
		cusobj.PdDvLn_Lvalue,"num_5080_sess",newatt);
	if(rc > 0){
		DEBUG_0("chgssla: check_parms, error in getting num_5080_sess\n")
		return(rc);
	}

	/* get the new value for the attribute lower_bond */
	rc = getatt(&new_lower_5080_la,'h',cusatt,preatt,lname,
		cusobj.PdDvLn_Lvalue,"lower_5080_bond", newatt);
	if(rc > 0){
		DEBUG_0("chgssla: check_parms, error in getting lower_5080_bond\n")
		return(rc);
	}

	if ((new_num_5080_sess + new_lower_5080_la ) > 16)
	{
	    DEBUG_2("chghia: Invalid value for num_sessions (0x%x) or lower_bond (0x%x)\n",
		new_num_sess,new_lower_la)
		strcpy( badattr, "num_sessions" );
		return(E_ATTRVAL);
	}

	/* get the new value for the channel address for 5080 */
	rc = getatt(&new_chan_addr,'h',cusatt,preatt,lname,
		cusobj.PdDvLn_Lvalue,"addr_5080_chan", newatt);
	if(rc > 0){
		DEBUG_0("chgssla: check_parms, error in getting addr_5080_chan\n")
		return(rc);
	}



	return(E_OK);
}

