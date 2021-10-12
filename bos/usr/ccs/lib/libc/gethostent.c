static char sccsid[] = "@(#)50	1.33  src/bos/usr/ccs/lib/libc/gethostent.c, libcnet, bos411, 9428A410j 6/21/94 16:26:21";
/* 
 * COMPONENT_NAME: LIBCNET gethostent.c
 * 
 * FUNCTIONS: _gethtbyname, endhostent, gethostbyaddr, gethostbyname, 
 *            gethostent, sethostent, sethostfile
 *
 * ORIGINS: 26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
 
/*
 * Copyright (c) 1985, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * gethostnamadr.c	6.36 (Berkeley) 10/7/88
 * sethostent.c	6.5 (Berkeley) 6/27/88
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <nl_types.h>
#include "libc_msg.h"

#ifdef DEBUG
#include <sys/ldr.h>
#endif /* DEBUG */

#include "ts_supp.h"
#include "push_pop.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex		_hostservices_rmutex;
#endif /* _THREAD_SAFE */

#ifdef _THREAD_SAFE
#define	THIS_SERVICE		ht_data->this_service
#else
#define	THIS_SERVICE		this_service
static int			this_service;
#endif /* _THREAD_SAFE */

#define LIBNAME		"/usr/lib/netsvc/lib"
static	struct services service_array[_MAXSERVICES];

/*
 * THREAD_SAFE NOTE: 
 * _get_order() locks a portion of code to protect the static variables, 
 * service_array[] and count. With the exception of service_array[], 
 * these variables will only be modified once, that is, the locked portion
 * will only be executed once per process. _load_services() will add to
 * service_array[] at most "count" times per process and then it will only add
 * to the array. (count from _get_order() is the number of services read
 * from NSORDER, /etc/netsvc.conf or the default.) _load_services() also 
 * takes a lock when it adds to service_array[]. Thus the *_r routines in 
 * this file do not have to take a lock whenever they read service_array[]. 
 * Each thread will only need to read what is currently in service_array[] 
 * at any given point in time since it is a "per process" variable. 
 */

/*
 * This routine reads either the NSORDER environment variable or
 * the /etc/netsvc.conf file to determine how to resolve names or
 * ip addresses. If neither is set, then it uses a default.
 */

int
_get_order(void)
{

	FILE 		*servf;
	char		a_line[_MAXLINELEN];
	char		service[MAXPATHLEN];
	char		*cp;
	char		*p;
	char		*libname;
	static int 	count = 0;
	int		tmp_count;

	/*
	 * Check and see if the flag, "count" is 0. If it is 0, then 
	 * the process has not read the host ordering yet. After 
	 * _get_order() is done once, count will be set to a non-zero
	 * value indicating that the ordering has been read and need
	 * not be read by the process again.
	 */

	TS_LOCK(&_hostservices_rmutex);
	TS_PUSH_CLNUP(&_hostservices_rmutex);
	if (!count) { 
	
	a_line[0] = '\0';

	/*
	 * Check to see if the environment variable,
	 * NSORDER has been set.
	 */
	if ((p = getenv("NSORDER")) != NULL) {
		(void)strcpy(a_line, "hosts=");
		(void)strcat(a_line, p);
	}
	/*
	 * If NSORDER wasn't set, then try the /etc/netsvc.conf file.
	 */
	if (p == NULL || *p == '\0') {
		if ((servf = fopen(_PATH_SERVCONF, "r")) != NULL) {
			while ((p = fgets(a_line, _MAXLINELEN, servf)) != NULL) 
				if (!strncmp(a_line, "hosts", sizeof("hosts") - 1))
					break;
			fclose(servf);
		}
	}
	
	/*
	 * If we were unable to open /etc/netsvc.conf, then set the default 
	 */
	if (a_line[0] != 'h' || a_line[0] == '\0')
#ifdef _THREAD_SAFE
		strcpy(a_line, "hosts=bind_r, nis_r=auth, local_r");
#else
		strcpy(a_line, "hosts=bind, nis=auth, local");
#endif /* _THREAD_SAFE */
	strtok(a_line, "=");       
	while (cp = strtok(0, ",")) {
		while (*cp == ' ' || *cp == '\t')
			++cp;
		if (*cp) {
			if (strstr(cp, "auth") != NULL)
				service_array[count].auth = TRUE;
			p = service;
			while (*cp && !isspace(*cp) && *cp != '=')
				*p++ = *cp++;
			*p = '\0';
		
			libname = LIBNAME;
			if (!(service_array[count].name = malloc(strlen(libname) + strlen(service) + 1)))
				perror("_get_order:malloc() failed");
			
			(void)strcpy(service_array[count].name, libname);
			(void)strcat(service_array[count].name, service);
			count++;
		}
	}
	}
	TS_POP_CLNUP(0);
	tmp_count = (count == 0) ? 0 : count - 1;
	TS_UNLOCK(&_hostservices_rmutex);
	return(tmp_count);
}
	
/*
 * This routine loads a name resolver service and adds the get* addresses for
 * the service to the service_array[] when it executes the
 * entrypoint. This routine will only load and add to the 
 * service_array[] at most "count" number of times.
 * (count from _get_order() is the number of services configured)
 */ 

_load_services(int current)
{
	int	(*entrypoint)();
	char	*a_buffer[BUFSIZ];

	/*
	 * Check and see if one of the routines such as gethostbyname() 
	 * has already been assigned a function address. If so, then
	 * that means we have already loaded this service, so don't bother.
         */
	TS_LOCK(&_hostservices_rmutex);
	if (service_array[current].routine_addrs.gethostbyname != NULL) {
		TS_UNLOCK(&_hostservices_rmutex);
		return(TS_SUCCESS);
	}
	entrypoint = load(service_array[current].name, 1, NULL);
	if (entrypoint == NULL) {
#ifdef _THREAD_SAFE
		_Set_h_errno(SERVICE_UNAVAILABLE);
#else
		h_errno = SERVICE_UNAVAILABLE;
#endif /* _THREAD_SAFE */
#ifdef DEBUG
		fprintf(stderr, "_load_services: the load() subroutine could not load %s.\n", service_array[current].name);
		if (_res.options & RES_DEBUG) {
			loadquery(L_GETMESSAGES, &a_buffer[1], sizeof(a_buffer));
			execvp("/etc/execerror", a_buffer);
		}
#endif /* DEBUG */
		TS_UNLOCK(&_hostservices_rmutex);
		return(TS_FAILURE);
	}
	(*entrypoint)(&service_array[current].routine_addrs);
	TS_UNLOCK(&_hostservices_rmutex);
	return(TS_SUCCESS);
}
	
#ifdef _THREAD_SAFE
int
gethostbyname_r(const char *name, struct hostent *htent,
		struct hostent_data *ht_data)
{
	int	ret_code = TS_FAILURE;
#else
struct hostent *
gethostbyname(const char *name)
{
	struct hostent 	*htent = (struct hostent *)NULL;
#endif /* _THREAD_SAFE */
	int	i, process_count, loaded;

	TS_EINVAL((htent == 0 || ht_data == 0));

	process_count = _get_order();

	for (i = 0; i <= process_count; i++) {
		/*
		 * If we could not load the service for some reason,
		 * try the next one. If all have been tried and none
		 * loaded, then just return NULL. We did our error
	 	 * message in load_services() so don't bother here.
		 */
		if ((loaded = _load_services(i)) == TS_FAILURE) {
			if (i == process_count) 
				break;
			else
				continue;
		}
		
#ifdef _THREAD_SAFE
	     ret_code = (*service_array[i].routine_addrs.gethostbyname)(name, htent, ht_data);
	     if (ret_code != TS_FAILURE || 
		 (service_array[i].auth == TRUE && 
		  h_errno != SERVICE_UNAVAILABLE))  
#else
	     htent = (*service_array[i].routine_addrs.gethostbyname)(name);
	     if (htent != NULL || 
		 (service_array[i].auth == TRUE && 
		 h_errno != SERVICE_UNAVAILABLE)) 
#endif /* _THREAD_SAFE */
		break;
	}
#ifdef _THREAD_SAFE
	if (ret_code == TS_FAILURE)
#else
	if (htent == NULL)
#endif /* _THREAD_SAFE */
		return(TS_FAILURE);
	else
		return(TS_FOUND(htent));
}

#ifdef _THREAD_SAFE
int
gethostbyaddr_r(const char *addr, int len, int type, 
		struct hostent *htent, struct hostent_data *ht_data)
{
	int	ret_code = TS_FAILURE;
#else
struct hostent *
gethostbyaddr(const char *addr, int len, int type)
{
	struct hostent 	*htent = (struct hostent *)NULL;
#endif /* _THREAD_SAFE */
	int	i, process_count, loaded;

	TS_EINVAL((htent == 0 || ht_data == 0));

	process_count = _get_order();

	for (i = 0; i <= process_count; i++) {
		/*
		 * If we could not load the service for some reason,
		 * try the next one. If all have been tried and none
		 * loaded, then just return NULL. We did our error
	 	 * message in load_services() so don't bother here.
		 */
		if ((loaded = _load_services(i)) == TS_FAILURE) {
			if (i == process_count) 
				break;
			else
				continue;
		}

#ifdef _THREAD_SAFE
		ret_code = (*service_array[i].routine_addrs.gethostbyaddr)(addr, len, type, htent, ht_data);
		if (ret_code != TS_FAILURE || (service_array[i].auth == TRUE && h_errno != SERVICE_UNAVAILABLE))
#else 
		htent = (*service_array[i].routine_addrs.gethostbyaddr)(addr, len, type);
		if (htent != NULL || (service_array[i].auth == TRUE && h_errno != SERVICE_UNAVAILABLE)) 
#endif /* _THREAD_SAFE */
			break;
	}

#ifdef _THREAD_SAFE
	if (ret_code == TS_FAILURE)
#else
	if (htent == NULL)
#endif /* _THREAD_SAFE */
		return(TS_FAILURE);
	else
		return(TS_FOUND(htent));
}

#ifdef _THREAD_SAFE
sethostent_r(int stayopenflag, struct hostent_data *ht_data)
#else
sethostent(int stayopenflag)
#endif /* _THREAD_SAFE */
{
	int	process_count, i, loaded;

	TS_EINVAL((ht_data == 0));

	process_count = _get_order();

	for (i = 0; i <= process_count; i++) {
		/*
		 * If we could not load the service for some reason,
		 * try the next one. If all have been tried and none
		 * loaded, then just return. We did our error message
	 	 * in load_services() so don't bother here.
		 */
		if ((loaded = _load_services(i)) == TS_FAILURE) {
			if (i == process_count) 
				break;
			else
				continue;
		}
#ifdef _THREAD_SAFE
		(*service_array[i].routine_addrs.sethostent)(stayopenflag, ht_data);
#else
	        (*service_array[i].routine_addrs.sethostent)(stayopenflag);
#endif /* _THREAD_SAFE */
	        if (h_errno != SERVICE_UNAVAILABLE) 
			break;
	}
	THIS_SERVICE = i;
}

#ifdef _THREAD_SAFE
endhostent_r(struct hostent_data *ht_data)
#else
void
endhostent(void)
#endif /* _THREAD_SAFE */
{
	int loaded;

	/*
	 * Let's do a _get_order() here just in case someone calls    
	 * endhostent() first before anything else. True it doesn't
	 * make any sense to call endhostent() first, but let's be 
	 * a "little" robust and to make sure that service_array[]
	 * has been filled before calling load().
	 */
	
#ifdef _THREAD_SAFE
	if (ht_data == 0) return;
#endif /* _THREAD_SAFE */

	(void)_get_order();

	/*
	 * Since we only want to load one specific service, if it
	 * could not be loaded, just return.
	 */
	if ((loaded = _load_services(THIS_SERVICE)) == TS_FAILURE)
		return;
	(*service_array[THIS_SERVICE].routine_addrs.endhostent)();
}

sethostfile(name)
char	*name;
{
#ifdef lint
	name = name;
#endif
}

#ifdef _THREAD_SAFE
int
gethostent_r(struct hostent *htent, struct hostent_data *ht_data)
{
	int	ret_code = TS_FAILURE;
#else
struct hostent *
gethostent(void)
{
	struct hostent 	*htent = (struct hostent *)NULL;
#endif /* _THREAD_SAFE */
	int  process_count, i, loaded;
	

	TS_EINVAL((htent == 0 || ht_data == 0));
#ifdef _THREAD_SAFE
	_Set_h_errno(0);
#else
	h_errno = 0;
#endif /* _THREAD_SAFE */
	process_count = _get_order();

	for (i = 0; i <= process_count; i++) {
		/*
		 * If we could not load the service for some reason,
		 * try the next one. If all have been tried and none
		 * loaded, then just return NULL. We did our error 
	 	 * message in load_services() so don't bother here.
		 */
		if ((loaded = _load_services(i)) == TS_FAILURE) {
			if (i == process_count) 
				break;
			else
				continue;
		}
#ifdef _THREAD_SAFE
		ret_code = (*service_array[i].routine_addrs.gethostent)(htent, ht_data);
#else
    	    	htent = (*service_array[i].routine_addrs.gethostent)();
#endif /* _THREAD_SAFE */

		if (h_errno != SERVICE_UNAVAILABLE) 
			break;
	}
#ifdef _THREAD_SAFE
	if (ret_code == TS_FAILURE)
#else
	if (htent == NULL)
#endif /* _THREAD_SAFE */
		return (TS_FAILURE);
	else
		return (TS_FOUND(htent));	
}


/* 
 * The routine _gethtbyname() is being exported and really should not be. 
 * The _get.. routines in gethostent.c are usually not exported.
 * In order not to break the culprit program in AIX that is using
 * this, we must keep it. It is not thread-safe, but no application program
 * should be using it anyway.
 */ 

struct hostent *
_gethtbyname(char *name)
{
	return(gethostbyname(name));
}
