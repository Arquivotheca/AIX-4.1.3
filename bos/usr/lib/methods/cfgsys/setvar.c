static char sccsid[] = "@(#)03	1.8  src/bos/usr/lib/methods/cfgsys/setvar.c, cfgmethods, bos411, 9428A410j 4/1/94 15:48:39";

/*
 * COMPONENT_NAME: (CFGMETHODS) Set var structure variables
 *
 * FUNCTIONS: setvar
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   



#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/sysmacros.h>
#include <sys/device.h>
#include <sys/sysconfig.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/var.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include "cfgdebug.h"

extern struct	Class	*cusatt;	/* Customized attributes class ptr */
extern struct	Class	*preatt;	/* Predefined attributes class ptr */


int setvar( cusobj, phase )
struct	CuDv	*cusobj;
int				phase;
{
	char	tmpstr[20];
	char	sstring[200];
	struct	var		varbuf;
	int	attempts;
	int	rc;

	DEBUG_0("IN SETVAR()\n")

	attempts = 0;

	for(;;)
	{
		/* Read the var structure from the kernel */

		if( sysconfig( SYS_GETPARMS, &varbuf, sizeof(varbuf) ) == -1 )
		{
#ifdef CFGDEBUG
			switch( errno )
			{
			case EACCES:
				DEBUG_0(
					"Process does not have reqd privelege for SYS_GETPARMS")
			case EFAULT:
				DEBUG_0( "Invalid buffer for SYS_GETPARMS\n")
			default:
				DEBUG_1( "Unknown Error (%d) during SYS_GETPARMS\n", errno )
			}
			return(E_SYSCONFIG);
#endif
		}

		DEBUG_0("SYS_GETPARMS succeeded\n")

		DEBUG_0("Var buffer as received from kernel:\n")
#ifdef CFGDEBUG
		hexdump( &varbuf, sizeof(varbuf) );
#endif

		/* Overlay structure with new values */

		rc = getatt( &varbuf.v_bufhw, 'i', cusatt, preatt, cusobj->name,
			cusobj->PdDvLn_Lvalue, "maxbuf", (struct attr *)NULL );
		if (rc)
			return(rc);

		rc = getatt(&varbuf.v_mbufhw, 'i', cusatt, preatt, cusobj->name,
			cusobj->PdDvLn_Lvalue, "maxmbuf", (struct attr *)NULL);
		if (rc)
			return(rc);

		rc = getatt( &varbuf.v_maxup, 'i', cusatt, preatt, cusobj->name,
			cusobj->PdDvLn_Lvalue, "maxuproc", (struct attr *)NULL);
		if (rc)
			return(rc);

		rc = getatt( tmpstr, 's', cusatt, preatt, cusobj->name,
			cusobj->PdDvLn_Lvalue,"autorestart",
			(struct attr *)NULL);
		if (rc)
			return(rc);

		if(( tmpstr[0] == 't') || (tmpstr[0] == 'T' ))
			varbuf.v_autost = 1;
		else
			varbuf.v_autost = 0;
		
		rc = getatt( tmpstr, 's', cusatt, preatt, cusobj->name,
			cusobj->PdDvLn_Lvalue, "iostat", (struct attr *)NULL);
		if (rc)
			return(rc);

		if(( tmpstr[0] == 't') || (tmpstr[0] == 'T' ))
			varbuf.v_iostrun = 1;
		else
			varbuf.v_iostrun = 0;

		rc = getatt(&varbuf.v_maxpout,'i', cusatt, preatt, cusobj->name,
			cusobj->PdDvLn_Lvalue, "maxpout", (struct attr *)NULL);
		if (rc)
			return(rc);

		rc = getatt(&varbuf.v_minpout,'i', cusatt, preatt, cusobj->name,
			cusobj->PdDvLn_Lvalue, "minpout", (struct attr *)NULL);
		if (rc)
			return(rc);

		rc = getatt( tmpstr, 's', cusatt, preatt, cusobj->name,
			cusobj->PdDvLn_Lvalue, "memscrub", (struct attr *)NULL);

		if (rc==E_NOATTR)
		    varbuf.v_memscrub=0;
		    
		else if (rc)
			return(rc);

		else 
		    if(( tmpstr[0] == 't') || (tmpstr[0] == 'T' ))
			varbuf.v_memscrub = 1;
		    else
			varbuf.v_memscrub = 0;
		
		rc = getatt( tmpstr, 's', cusatt, preatt, cusobj->name,
			cusobj->PdDvLn_Lvalue, "fullcore", (struct attr *)NULL);
		if (rc)
			return(rc);

		if(( tmpstr[0] == 't') || (tmpstr[0] == 'T' ))
			varbuf.v_fullcore = 1;
		else
			varbuf.v_fullcore = 0;

		
		/* Send updated var structure to the kernel */

		DEBUG_0("Var buffer as sent to kernel:\n")
#ifdef CFGDEBUG
		hexdump( &varbuf, sizeof(varbuf) );
#endif

		if( sysconfig( SYS_SETPARMS, &varbuf, sizeof(varbuf) ) == -1 )
		{
			switch( errno )
			{
			case EACCES:
				DEBUG_0(
					"Process does not have reqd privelege for SYS_SETPARMS\n")
				return(E_SYSCONFIG);
			case EINVAL:
				DEBUG_0("Invalid value in SYS_SETPARMS\n")
				if( ( phase == 2 ) && ( !attempts ) )
				{
					/* Configuring, so erase customized entries and try again */
					sprintf(sstring, "name = %s AND attribute = maxbuf",
						cusobj->name );
					odm_rm_obj( cusatt, sstring );
					sprintf(sstring, "name = %s AND attribute = maxmbuf",
						cusobj->name );
					odm_rm_obj( cusatt, sstring );
					sprintf(sstring, "name = %s AND attribute = maxuproc",
						cusobj->name );
					odm_rm_obj( cusatt, sstring );
					sprintf(sstring, "name = %s AND attribute = autorestart",
						cusobj->name );
					odm_rm_obj( cusatt, sstring );
					sprintf(sstring, "name = %s AND attribute = iostat",
						cusobj->name );
					odm_rm_obj( cusatt, sstring );
					sprintf(sstring, "name = %s AND attribute = maxpout",
						cusobj->name );
					odm_rm_obj( cusatt, sstring );
					sprintf(sstring, "name = %s AND attribute = minpout",
						cusobj->name );
					odm_rm_obj( cusatt, sstring );
					sprintf(sstring, "name = %s AND attribute = memscrub",
						cusobj->name );
					odm_rm_obj( cusatt, sstring );
					sprintf(sstring, "name = %s AND attribute = fullcore",
						cusobj->name );
					odm_rm_obj( cusatt, sstring );

					attempts++;

					break;
				}
				else
					return(E_SYSCONFIG);
			case EAGAIN:
				/* There was another SYS_GETPARMS during the delay, retry */
				if( attempts++ >= 3 )
					return(E_SYSCONFIG);
				break;
			case EFAULT:
				DEBUG_0( "Invalid buffer for SYS_SETPARMS\n")
				return(E_SYSCONFIG);
			default:
				DEBUG_1( "Unknown Error (%d) during SYS_SETPARMS\n", errno )
				return(E_SYSCONFIG);
			}
		}
		else
		{
			/* The update worked */
			DEBUG_0("SYS_SETPARMS Accepted\n")
			return(0);
		}
	}
}

