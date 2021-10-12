static char sccsid[] = "@(#)48	1.2  src/bos/usr/lib/methods/common/aio.c, cfgmethods, bos411, 9428A410j 12/1/92 09:14:53";
/*
 * COMPONENT_NAME: (CFGMETH) common code for aio methods
 *
 * FUNCTIONS: check_parms
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

static char stubnm[] = "chgdd";
#include <sys/types.h>
#include <sys/sysconfig.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/fd.h>
#include <sys/i_machine.h>
#include <sys/device.h>
#include <sys/cfgodm.h>
#include <sys/sched.h>
#include <sys/errno.h>
#include <cf.h>
#include "pparms.h"
#include "cfgdebug.h"

check_parms(attrs,Pflag,Tflag,logical_name,parent,location)
        struct attr *attrs;
        int Pflag,Tflag;
        char *logical_name,*parent,*location;
{
	struct CuAt *cuat;
	int maxservers;
	int minservers;
	int rc;
	int value;
	int i;
	i = 0;
	
	while(attrs[i].attribute != NULL) {
		if(!strcmp(attrs[i].attribute,"autoconfig")) {
			if(strcmp(attrs[i].value,"available") && 
				strcmp(attrs[i].value,"defined")) {
				strcpy(location, "autoconfig");
				return E_INVATTR;	
			}
			else {
				i++;
				continue;
			}
		}
		value = atoi(attrs[i].value);
		if(!strcmp(attrs[i].attribute,"minservers")) {
			cuat = getattr(logical_name,"maxservers",FALSE,&rc);
			if(cuat != NULL) maxservers = atoi(cuat[i].value);
			if((value < 0) ||(value > maxservers)) {
				strcpy(location, "minservers");
				return E_INVATTR;
			}
		}
		else
		 if(!strcmp(attrs[i].attribute,"maxservers")) {
			cuat = getattr(logical_name,"minservers",FALSE,&rc);
			if(cuat != NULL) minservers = atoi(cuat[i].value);
			if((value < minservers)||(value > 1<<16)) {
				strcpy(location, "maxservers");
				return E_INVATTR;
			}
		 }
		else
		  if(!strcmp(attrs[i].attribute,"kprocprio")) {
			if((value < PRIORITY_MIN)||(value > PRIORITY_MAX)) {
				strcpy(location, "kprocprio");
				return E_INVATTR;
			}
		  }
		 else
		  if(!strcmp(attrs[i].attribute,"maxreqs")) {
			if((value < 0)||(value > 1<<16))  {
				strcpy(location, "maxreqs");
				return E_INVATTR;
			}
		  }
		i++;
	} /* while */
	return(0);			/* always return true */
}
