static char sccsid[] = "@(#)44	1.2  src/bos/usr/ccs/lib/libs/pw_hist.c, libs, bos411, 9428A410j 4/19/94 18:05:16";
/*
 *   COMPONENT_NAME: LIBS
 *
 *   FUNCTIONS: _PWDHistory
 *		_PWDHistoryMaint
 *		_crypt_and_compare
 *		_delete_PWDHistory
 *		_get_histattr
 *		_insert_and_cleanup
 *		putuserpwhist
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/errno.h>
#include <usersec.h>	/* user attributes */
#include <userpw.h>	/* user passwd attributes */
#include <userconf.h>	/* security configuration attributes */
#include <pwd.h>
#include <string.h>
#include <fcntl.h>
#include <ndbm.h>
#include <sys/stat.h>
#include "libs.h"



#define N_WEEK		(24L * 7 * 60L * 60L )   /* Number of sec's in a week */    
#define ROOTID 		0           		 /* UID for "root"            */
#define SECURITYID	7           		 /* GID for "security"        */


/*  currently the dbm routines use PBLKSIZ when
 *  getting and putting data, thus we are limited
 *  to this value.  MAX_HISTSIZE was determined
 *  by dividing the size of the password,lastupdate
 *  pair which is PW_CRYPTLEN and the sizeof(ulong)
 *  into PBLKSIZ, which yields 1024/20 = 51 which
 *  was rounded down to an even 50.  If any of the
 *  values change, our algorithm for storing passwords
 *  will have to be adjusted, thus the following will 
 *  produce a compiling error.
*/

#if (PW_CRYPTLEN + 4) * MAX_HISTSIZE > PBLKSIZ
#error "Password History List Sizes Changed"
#endif

/* Global Routines */

static int _PWDHistoryMaint(struct userpw *);
static int _get_histattr(char *, int *, int *);
static int _insert_and_cleanup( datum *, struct userpw *, int, int);
int _PWDHistory( struct userpw *, char *, MsgBuf *);
static int _crypt_and_compare( char *, char * );
int putuserpwhist( struct userpw *, char **);   /* this is a published routine */



/*
 * NAME: _PWDHistoryMaint
 * 
 * FUNCTION:	This is the entry point for the history maintenance routines.
 *		Performs the creating of and maintenance of the history
 *              list, when HISTORY is enabled.
 * 
 * 		An error message is handled by the calling routine, putuserpwhist,
 *		upon failure.
 *
 * RETURNS: returns 
 *
 *      	-1 - Internal Error
 *               0 - Update was successful, or not needed.
 *
 *  called by: putuserpwhist() 
 */
static int
_PWDHistoryMaint( struct userpw *pw)
{
    int 	size; 
    int 	expire;
    int 	hist_enabled; 
    int 	item_size;
    int 	rc = 0;
    ulong 	expired_time;
    char 	buf[BUFSIZ]; 
    char	*cp = buf;
    datum 	key, data;
    DBM 	*db;
	
	 
	 /* First check if the password is NULL or * - in these cases,
	    the password history will not be updated, so exit. */

	if ( !pw->upw_passwd || (pw->upw_passwd[0] == '\0') || 
		    !strcmp(pw->upw_passwd,"*"))
		return 0;

	if ( (hist_enabled =_get_histattr( pw->upw_name, &size, &expire)) == -1)
		return -1;


	if (hist_enabled) 
	{
		key.dptr = pw->upw_name;
		key.dsize = strlen(pw->upw_name);

		if ((db = dbm_open(HISTFILE, O_CREAT|O_RDWR,0600)) == (DBM *)0)
			return -1;

		/* set up the ACLs on the two database files */

		fchown(db->dbm_dirf,ROOTID,SECURITYID);
		fchown(db->dbm_pagf,ROOTID,SECURITYID);

		if (_writelock( dbm_dirfno(db))== -1)
		{
			dbm_close(db);
			return -1;
		}

		data = dbm_fetch(db, key);
		if (data.dptr == (char *)NULL)     /* no entry found */
		{
			/* create the first record and store it */

			item_size = sizeof(pw->upw_lastupdate);
			bcopy( (char *)&pw->upw_lastupdate, cp, item_size);
			cp += item_size;

			strcpy (cp, pw->upw_passwd);
			cp += PW_CRYPTLEN;
			data.dptr = buf;
			data.dsize = cp - buf;
			rc = dbm_store(db, key, data, DBM_INSERT);
		}
		else
		{
			/* insert the new record in history list and conduct
			cleanup on the list, removing passwords now valid for reuse */

			if ( rc =_insert_and_cleanup( &data, pw, size, expire ) == 0)
				rc = dbm_store(db, key, data, DBM_REPLACE);

			free ((void *) data.dptr);
		} 
		_writeunlock(dbm_dirfno(db));
		dbm_close(db);


	} 
	else        /* history checking disabled */
		rc = _delete_PWDHistory( pw->upw_name );

	
	return rc;
}

/*
 * NAME: _get_histattr
 *                                                                    
 * FUNCTION:  Gets the values of the history attributes: histsize and histexpire.
 * 	      Also determines if history checking is enabled based upon either
 *            histsize or histexpire having non-zero values.
 *
 * RETURNS: 
 *		0 - History is NOT enabled
 *		1 - History is enabled.
 *             -1 - Internal Error (getuserattr failed)
 *
 * Called by:  _PWDHistory() and _PWDHistoryMaint
 */  

static int
_get_histattr(char *uname, int *size, int *expire)
{


	if (getuserattr(uname, S_HISTSIZE, size, SEC_INT) )
		if (errno == ENOATTR || errno == ENOENT )
			*size = MIN_HISTSIZE;  
		else
			return (-1);

	if (getuserattr(uname, S_HISTEXPIRE, expire, SEC_INT) )  
		if (errno == ENOATTR || errno == ENOENT )
			*expire = MIN_HISTEXPIRE;  
		else
			return (-1);


	if ( *expire < MIN_HISTEXPIRE ) 
		*expire = MIN_HISTEXPIRE;

	if ( *expire > MAX_HISTEXPIRE ) 
		*expire = MAX_HISTEXPIRE;

	if ( *size < MIN_HISTSIZE ) 
		*size = MIN_HISTSIZE;

	if  (*size > MAX_HISTSIZE) 
		*size = MAX_HISTSIZE;

	if ( !*expire && !*size )
		return 0;
	else 
		return 1;



}

/*
 * NAME: _insert_and_cleanup
 * 
 * FUNCTION:	called by _PWDHistoryMaint
 *              inserts the password and lastupdate value into the history list
 *              and performs maintenance on the list by removing
 *              passwords which are now valid for reuse based upon the
 *              expire and size values.
 * 
 * RETURNS:    0  Success
 *            -1 Internal Error   (malloc failure)	
 *				
 * Called by: _PWDHistoryMaint().
 */

static int
_insert_and_cleanup( datum *data, struct userpw *pw, int list_size_limit, 
		     int expire)
{

    char 	*line;    /* contains new modified history list */
    char 	*tp;      /* temp pointer to line */
    char 	*cp;      /* current pointer to the history list */
    char 	old_passwd[PW_CRYPTLEN];
    ulong 	time_stamp;
    ulong 	now; 
    ulong	exp_time;
    int 	list_size;
    int 	new_list_size = 0;
      

	if ((line = (char *) malloc (data->dsize + PW_CRYPTLEN 
	     + sizeof(ulong))) == (char *)NULL)
			return -1;

	tp = line;
	cp = data->dptr;

	       /* copy the new information into the new history list */


	bcopy( (char *) &pw->upw_lastupdate, tp, sizeof(ulong));
	tp += sizeof(ulong);
	strcpy( tp, pw->upw_passwd);
	tp += PW_CRYPTLEN;   /* move the tmp pointer past new pair */

	new_list_size++;  /* keep track of the new item */

	list_size = data->dsize / ( sizeof(ulong) + PW_CRYPTLEN );

        if (expire)
	{
		now = (ulong) time((time_t *) 0);
		exp_time = expire * N_WEEK; /* convert from weeks to seconds */

	}
                  /* begin determining passwords 
		     which are now valid for reuse */
	
	while (list_size)
	{
		time_stamp = *(ulong *)cp;
		cp += sizeof(ulong);
		strncpy( old_passwd, cp, PW_CRYPTLEN);
		cp += PW_CRYPTLEN;
		new_list_size ++;


             /* 
	      *
	      * If only the histsize attribute (value of list_size_limit)
	      * is set, the max. number of passwords to keep is list_size_limit.
	      *
	      * If only the histexpire attribute (value of expire) is set,
	      * the size of the can grow only to the system limit of
	      * MAX_HISTSIZE (value determined due to the limitation of
	      * the dbm routines of 1024 bytes).
	      *
	      * If both attributes are set, the one which will produce the
	      * largest password list is enforced.  The algorithm will
	      * satisfy both values, keeping the list from exceeding
	      * MAX_HISTSIZE.
	      *
	      */

		if ( !expire && list_size_limit)
		{
			if (new_list_size > list_size_limit)
				break;  /* met the size limit */
		}

		else if (expire && !list_size_limit)
		{
			if ((now > (time_stamp + exp_time)) || (new_list_size > MAX_HISTSIZE))
				break;  /* the rest are invalid */

		}
				
		else    /* both attributes are set */
		{
			if ((now > (time_stamp + exp_time) && 
			   (new_list_size > list_size_limit) ) ||
			   (new_list_size > MAX_HISTSIZE))
				break;
		}


		  /* if we get here the old password is valid, 
		     so copy it into tp */

		bcopy( (char *) &time_stamp, tp, sizeof(ulong));
		tp += sizeof(ulong);
		strcpy( tp, old_passwd);
		tp += PW_CRYPTLEN;   /* move the tmp pointer past new pair */

		list_size--;
	}

	data->dptr = line;
	data->dsize = tp - line;


	return 0;
}



/*
 * NAME: _PWDHistory
 * 
 * FUNCTION:	This is the entry point for the history checking.
 *		Compares a newly requested password with the old passwords
 *              in the history list and determines if the password
 *              is valid based upon the resuse rules.  If it is 
 *		invalid, msg will contain an informative message.
 * 
 * RETURNS: returns 
 *
 *      	-1 - Internal Error
 *		 1 - Failure - password cannot be reused
 *               0 - Successs - password is ok     
 *
 *  called by:  _PasswordChecks() from pw_rest.c
 */

int
_PWDHistory( struct userpw *pw, char *clear_passwd, MsgBuf *mb)
{

    struct stat sbuf;
    ulong 	now, exp_time, last_update = 0;
    int 	histsize, histexpire = 0;
    char 	old_pw[PW_CRYPTLEN];
    char 	*cp;
    datum 	key, data;
    DBM 	*db;
    int 	passwds_to_check;
    int 	rc = 0; 
    int 	enabled;
    int		tmp_errno;
    int		count; 


	 /* First check if the new password is NULL - 
	    if so, there is no need to do history checking
	    as null passwords are valid for reuse */

	if ( !clear_passwd || (clear_passwd[0] == '\0') )
		return 0;

	if ( (enabled = _get_histattr( pw->upw_name, &histsize, &histexpire)) == -1)
	{
		tmp_errno = errno;
		(void) _MBReplace(mb, MSGSTR(M_HISTATTR, DEF_HISTATTR) );
		errno = tmp_errno;
		return(-1);
	}
	else if ( !enabled )    
		return 0;


	if (histexpire)
	{
		now = (ulong) time((time_t *) 0);
		exp_time = histexpire * N_WEEK;
	}


	if (stat(HISTPGFILE, &sbuf) == -1)   /* do the dbm files exist */
		return 0;             /* success, no history list to check */
			
	if ( (db = dbm_open(HISTFILE, O_RDONLY,0600)) == (DBM *)0)
	{
		tmp_errno = errno;
		(void) _MBReplace(mb, MSGSTR(M_HISTDBOPEN, DEF_HISTDBOPEN) );
		errno = tmp_errno;
		return(-1);
	}
	key.dptr = pw->upw_name;
	key.dsize = strlen(pw->upw_name);

	data = dbm_fetch(db, key);
	dbm_close(db);   /* go ahead and close, we have what we need */

	if (data.dptr == (char *)NULL)  
		return 0;     /* success, no history list to check */

		
	cp = data.dptr;

	passwds_to_check = data.dsize / (sizeof(ulong) + PW_CRYPTLEN);

	for (count = 1; count <= passwds_to_check; count++)
	{
		last_update = *(ulong *)cp;
		cp += sizeof(ulong);
		strncpy( old_pw, cp, PW_CRYPTLEN);
		cp += PW_CRYPTLEN;

		if (_crypt_and_compare( clear_passwd, old_pw))
		{

		   /*
		    * The next if clause determines if a matched password is
		    * invalid for reuse. 
		    *
		    * The cases are:
		    *  1.  only histsize is set, it's invalid if it is within
		    *      the histsize size limit. (This catches the case where
		    *      histsize has changed since the last time the password was
		    *      updated.)
		    *      
		    *  2.  only histexpire is set, so the password is invalid only
		    *      if it has not expired yet.
		    *
		    *  3.  both are set, so the size of the list is variable depending
		    *      upon which attribute gave the largest list.  So, if there
		    *      is a match and it has expired (normally reusable) but
		    *      the histsize attribute has not been met, it is invalid for
		    *      reuse.
		    *  4.  By the time we get here, all above conditions have been
		    *      examined and we simply have a password which has not
		    *      expired yet, thus it is invalid for reuse.
		    *
		    */

			if ( (!histexpire && histsize && count <= histsize) ||
			   ( histexpire && !histsize && (now < (last_update + exp_time))) ||
			   ((now > (last_update + exp_time)) && count <= histsize) ||
			   ((now < (last_update + exp_time))))
			{
				rc = 1;
				if ( _MBAppend(mb, MSGSTR(M_NOPASSREUSE, DEF_NOPASSREUSE)))
					rc = -1;
				break;
			}
		}

			
		
	}
		
	return rc;

}  	 
 
/*
 * NAME: _crypt_and_compare
 *
 * FUNCTION: This function takes a clear password and encrypts it with
 *           the salt of an existing password and determines if they
 *           are the same.
 *
 * RETURNS: 
 *           0 - passwords do not match
 *           1 - passwords match
 */

static int
_crypt_and_compare( char *clear_pw, char * old_pw)
{
    char *tp;       /* temp pointer */
    int rc = 0;
			/* if existing (old) password is null, */
			  /* consider it a no match */
	if ( old_pw && old_pw[0] ) 
	{
		tp = crypt(clear_pw, old_pw);
		if (!strcmp(old_pw, tp))
			rc = 1;
	}

	return rc;
}


/*
 * NAME: putuserpwhist    THIS IS A PUBLISHED ROUTINE
 *
 * FUNCTION: This function is a wrapper routine for the putuserpw subroutine. 
 *           It is responsible for updating the password history list if the
 *	     call to putuserpw is successful.  If it isn't successful, there 
 *	     is no need to update the password history database.
 *
 *	     This routine will fail only if the putuserpw call fails.  In the
 * 	     case where updating the password history fails, this routine will
 *	     return 0, but will return a message to the user warning of the
 *	     problem with the password history update.
 *
 * Called by:  chpass() and pwdadm.c's setpasswd() subroutine.
 *          
 *
 * RETURNS: 
 *   		 0 Success        
 * 		 non zero - putuserpw failed.
 *           
 */

int
putuserpwhist( struct userpw *newpw, char **msg )
{
int               rc;


	if ((rc = putuserpw(newpw)) == 0)   /* successful update */
	{
		if( _PWDHistoryMaint( newpw ) )
			*msg = strdup(MSGSTR(M_PUTHISTERR, DEF_PUTHISTERR));
	}

	return rc;
}
