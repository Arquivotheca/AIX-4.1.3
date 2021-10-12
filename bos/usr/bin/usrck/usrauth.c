static char sccsid[] = "@(#)41	1.8.1.2  src/bos/usr/bin/usrck/usrauth.c, cmdsadm, bos41J, 9515B_all 4/13/95 08:41:53";
/*
 *   COMPONENT_NAME: CMDSADM
 *
 *   FUNCTIONS: ck_auth
 *		ck_authmethod
 *		ck_authsystem
 *		ck_parsetree
 *		ck_registry
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/types.h>
#include <sys/audit.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <usersec.h>
#include <userconf.h>
#include "usrck_msg.h"
#include "usrck.h"

/*
 * Global data
 */

extern	int	fixit;
extern	int	verbose;

/*
 * NAME: ck_auth
 *
 * FUNCTION: Validate an authentication method
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * NOTES:
 *	ck_auth() is called with the name of the authentication method
 *	the be validated.
 *
 * RETURNS: Zero if the authentication method is either SYSTEM or NONE,
 *	or the named method has a valid program attribute.  Otherwise a
 *	non-zero number is returned.
 */

ck_auth (struct users *user, char *auth)
{
	char	*method;
	char	*nextmethod;
	char	*program;
	char	*authname;
	char	*name = user->usr_name;
	int	id;
	int	errors = 0;

	/*
	 * If there is no user file entry, there is no point
	 * in even trying.
	 */

	if (! user->usr_user) {
		msg1 (MSGSTR (M_NOSECUSER, DEF_NOSECUSER), user->usr_name);
		if (ck_query (MSGSTR (M_ADDSECUSER, DEF_ADDSECUSER),
				user->usr_name)) {

			/*
			 * Add just the stanza name - this is a hack
			 * to do this and still keep the file locking
			 * code, etc. in use.
			 */

			putuserattr (name, S_AUTH1, (void **) 0, SEC_DELETE);

			/*
			 * See if I should add the system default values.
			 * These are the same values the system will
			 * assume the user has if no values are present.
			 */

			if (strcmp (auth, S_AUTH1) == 0) {
				putuserattr (name, S_AUTH1, "SYSTEM", 0);
				putuserattr (name, S_AUTH2, "NONE", 0);
			}
			mk_audit_rec (AUDIT_OK, user->usr_name,
				"add user file entry", Fixed);
			user->usr_user = 1;
			return ENOTRUST;
		}
		errors++;
	}

	/*
	 * Get the list of authentication methods.
	 */

	if (getuserattr (name, auth, (void *) &method, 0)) {
		msg2 (MSGSTR (M_BADGET, DEF_BADGET), name, auth);

		return -1;
	}
	if (method == 0 || *method == '\0')
		return 0;

	/*
	 * The authentication method is a comma separated list of
	 * methods and user names.  The method is separated from the 
	 * authentication name by a ";".  The entire list of
	 * terminated with two NUL characters.
	 */

	while (*method) {
		nextmethod = method + strlen (method) + 1;

		if (authname = strchr (method, (int) ';'))
			*authname++ = '\0';
		else
			authname = 0;

		/*
		 * See if the authentication name exists
		 */

		if (strcmp (method, "SYSTEM") == 0 && authname != 0 &&
				getuserattr (authname, S_ID, &id, 0)) {
			msg1 (MSGSTR (M_NOUSER, DEF_NOUSER), authname);
			msg2 (MSGSTR (M_BADAUTH, DEF_BADAUTH), name, method);

			errors++;
			goto again;
		}
		if (! strcmp (method, "SYSTEM") || ! strcmp (method, "NONE"))
			goto again;

		/*
		 * Get the program attribute of the method.
		 */

		if (getconfattr (method, SC_AUTHPROGRAM, (void *) &program, 0)
				|| ! program) {
			msg2 (MSGSTR (M_BADAUTH, DEF_BADAUTH), name, method);

			errors++;
			goto again;
		}

		/*
		 * See if the program really exists.
		 */

		if (access (program, X_ACC)) {
			msg2 (MSGSTR (M_BADPROG, DEF_BADPROG), program, name);

			errors++;
		}
again:
		method = nextmethod;
	}
	return errors ? -1:0;
}

/*
 * NAME: ck_registry
 *
 * FUNCTION: validates the user's registry for a known or user defined 
 *	     administration domain.   The "registry" variable is used 
 *	     to define where the user is administered.  It is used whenever 
 *	     there is a possibility of resolving the user to other domains 
 *	     due to services being down or databases being replicated locally.
 *
 * RETURNS:  0 or error
 * 
 */
int
ck_registry(struct users *user)
{
	int  rc;
	char *ret = (char *)NULL;
	char *name = user->usr_name;
	char *registry = (char *)NULL;

	if (getuserattr(name, S_REGISTRY, (void *)&registry, SEC_CHAR))
	{
		if (errno != ENOENT && errno != ENOATTR)
		{
			msg2(MSGSTR(M_BADGET, DEF_BADGET), name, S_REGISTRY);
			return(-1);
		}
		return(0);
	}

	if ((rc = ck_authmethod(registry, &ret)) == 0)
	{
		/*
	 	 * AUTH_COMPAT isn't a valid registry since it is a name 
		 * resolution token that signals the getpw* and getgr* 
	  	 * routines to follow normal resolutions through local 
		 * and NIS databases.  AUTH_NONE_SEC makes no sense as a 
		 * registry either since this user has to be administered 
		 * somewhere.
	 	 */
		if (ISCOMPAT(ret) || ISNONE(ret))
			rc = -1;
	}
	if (rc)
		msg2(MSGSTR(M_BADREGISTRY, DEF_BADREGISTRY), name, registry);
		
	return(rc);
}

/*
 * NAME: ck_authmethod
 * 
 * FUNCTION: validates the supplied variable to see if it is a well 
 *	     known method, such as "NIS", "NONE", "compat" or "files"
 *	     or possibly another method defined in login.cfg
 * 
 * Returns:  0 or error
 *
 */
static int
ck_authmethod(char *val, char **ret)
{
	char *method;		/* User defined authentication method   */
	char *vp;		/* "val" variable pointer		*/
	int  i;			/* For loop counter			*/
	int  rc = -1;		/* return code				*/

	static struct token	/* Well known methods and their capital */
	{			/* forms.				*/
		char 	*CAPNAME;
		char	*name;
	} *tptr, tokens[]   =	{ 	"FILES",	AUTH_FILES,
			  		"NIS",		AUTH_NIS,
			  		"COMPAT",	AUTH_COMPAT,
			  		"NONE",		AUTH_NONE_SEC,
					(char *)NULL,	(char *)NULL
				};

	*ret = val;

	/*
	 * First attempt to retrieve a method name from the database to
	 * see whether it is indeed a defined authentication domain.
 	 */
        if (!getconfattr (val, SC_AUTHPROGRAM, (void *)&method, SEC_CHAR))
	{	
		if (!accessx(method, X_OK, ACC_SELF))
			return(0);
	}

	/*
 	 * If this fails then there is one standard directory where all
	 * methods should be residing.
	 */
	if ((method = (char *)malloc(strlen(AUTH_DEFPATH) + strlen(val) + 1)) ==
	     (char *)NULL) return(-1);

	strcpy(method, AUTH_DEFPATH);
	strcat(method, val);
	if (!accessx(method, X_OK, ACC_SELF))
	{	
		free(method);
		return(0);
	}
	free(method);

	if ((vp = (char *)malloc(strlen(val) + 1)) == (char *)NULL)
		return(-1);

	/* 
	 * Switch to capitalized versions of the tokens, for their check 
	 */
	for (i = 0; val && *val; i++, val++)
		vp[i] = islower(*val) ? toupper(*val) : *val;

	vp[i] = (char)NULL;
	
        for (tptr = tokens; tptr->name; tptr++)
	{
                if (!strcmp(tptr->CAPNAME, vp))
		{
			if ((*ret = strdup(tptr->name)) == (char *)NULL)
				rc = -1;
			else
				rc = 0;
                        break;
		}
	}
	free(vp);

	if (rc)
		*ret = (char *)NULL;

	return(rc);
}

/*
 * NAME: ck_authsystem
 * 
 * FUNCTION: Validates the supplied authentication grammar.  This routine
 *	     takes the grammar and ensures its correctness by creating
 *	     a parse tree.  
 * 
 * RETURNS:  0 or error
 * 
 */
int
ck_authsystem(struct users *user)
{
	struct secgrammar_tree *tptr;
	char   *grammar = (char *)NULL;
	char   *name = user->usr_name;
	int    rc = 0;

	if (getuserattr(name, S_AUTHSYSTEM, (void *)&grammar, SEC_CHAR))
	{
		if (errno != ENOENT && errno != ENOATTR)
		{
			msg2(MSGSTR(M_BADGET, DEF_BADGET), name, S_AUTHSYSTEM);
			return(-1);
		}
		return(0);
	}

	/*
	 * If the grammar supplied parses correctly, then we will consider
	 * it valid.
	 */
	if (grammar && (*grammar != (char)NULL))
	{
		if ((tptr = _create_parse_tree(grammar)) != 
		    (struct secgrammar_tree *)NULL)
		{
			rc = ck_parsetree(tptr);
	        	_release_parse_tree(tptr);
		}
		else
			rc = -1;
	}
	if (rc)
		msg2(MSGSTR(M_BADGRAMMAR, DEF_BADGRAMMAR), name, grammar);

	return(rc);
}

/*
 * NAME: ck_parsetree
 * 
 * FUNCTION: Helper routine for ck_authsystem().  This routine checks each
 *	     leaf of the grammar parse tree to ensure that it is a defined
 *	     method.
 * 
 * RETURNS:  0 or error
 *
 */
static int
ck_parsetree(struct secgrammar_tree *tptr)
{
	char *ret;
	int  rc = 0;

	switch(tptr->type)
	{
		case LEAF:
			return(ck_authmethod(tptr->name, &ret));
		case NODE:
			if (tptr->left)
				rc = ck_parsetree(tptr->left);
			if (!rc && tptr->right)
				rc = ck_parsetree(tptr->right);
			break;
		default:
			rc = -1;
	}
	return(rc);
}
