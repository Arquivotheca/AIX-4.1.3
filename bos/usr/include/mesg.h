/* @(#)53	1.14  src/bos/usr/include/mesg.h, libcmsg, bos411, 9428A410j 1/12/93 17:00:07 */

/*
 * COMPONENT_NAME: LIBCMSG
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_MESG
#define _H_MESG

#ifndef _H_NL_TYPES
#include <nl_types.h>
#endif

#ifndef _H_LIMITS
#include <limits.h>
#endif

#define CAT_MAGIC 	505
#define CATD_ERR 	((nl_catd) -1)
#define NL_MAXOPEN	10

struct _message {
	unsigned short 	_set,
			_msg;
	char 		*_text;
	unsigned	_old;
};

struct _header {
	int 		_magic;
	unsigned short	_n_sets,
			_setmax;
	char 		_filler[20];
};
struct _catset {
	unsigned short 	_setno,
			_n_msgs;
	struct _msgptr 	*_mp;
	char	**_msgtxt;
};

#ifndef _H_STDIO
#include <stdio.h>
#endif

struct _catalog_descriptor {
	char		*_mem;
	char		*_name;
	FILE 		*_fd;
	struct _header 	*_hd;
	struct _catset 	*_set;
	int		_setmax;
	int 		_count;
	int		_pid;
	int		_oflag;
};


struct _msgptr {
	unsigned short 	_msgno,
			_msglen;
	unsigned long	_offset;
};

#endif /* _H_MESG */
