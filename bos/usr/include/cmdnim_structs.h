/* @(#)58	1.18  src/bos/usr/include/cmdnim_structs.h, cmdnim, bos411, 9428A410j  2/28/94  11:43:11 */
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/include/cmdnim_structs.h
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_CMDNIM_STRUCTS
#define _H_CMDNIM_STRUCTS
/*******************************************************************************
*
*                             cmdnim_structs.h
*
* this file contains definitions for common NIM data structures
*******************************************************************************/

/*******************************************************************************
*********************** network info structs           *************************
*******************************************************************************/

/*---------------------------- nim_if           ------------------------------*/
/* used to store network interface info */
struct nim_if {
	char network[MAX_NAME_BYTES];			/* net object name */
	char *hostname;							/* full hostname */
	char *ip;									/* ip addr of hostname */
	char hard_addr[MAX_NET_HARD_ADDR];	/* hw addr of adapter */
	char adapter[MAX_ADAPTER_NAME];		/* net adapter logical dev name */
};

/*---------------------------- nim_route        ------------------------------*/
/* info needed for the "route" command (to add a route to a network) */
struct nim_route {
	char dest[MAX_IP_ADDR];						/* destination network address */
	char snm[MAX_IP_ADDR];						/* dest net subnetmask */
	char gateway[MAX_IP_ADDR];					/* address of gateway */
};

/*---------------------------- route_ass        ------------------------------*/
/* info specified during ATTR_ROUTING assignment */
/* expecting a format like: */
/*		<name of target net> <gateway to target> <target's gateway back> */
struct routing_ass {
	char *dest;
	char *gateway;
	char *dest_gateway;
};

/*---------------------------- res_access       ------------------------------*/
/* specific interface info about how to access a resource */
/* this structure is initialized in mstr_res.c and is used whenever NIM must */
/*		pass interface info to a client (eg, creating a .info file) */
struct res_access {
	char name[MAX_NAME_BYTES];						/* NIM name of resource */
	int type;											/* res type (nim_pdattr.attr) */
	char server[MAX_NAME_BYTES];					/* NIM name of server */
	char *location;									/* res location on server */
	struct nim_if nimif;								/* interface info */
};
#define RES_ACCESS_SIZE			sizeof( struct res_access )

/*******************************************************************************
*********************** libnim structs                 *************************
*******************************************************************************/

/*---------------------------- mount_info           --------------------------*/
/* used to store information about remote directories which the current */
/*		process has mounted */
/* a link list is constructed out of this structure & the head is pointed to */
/*		from the nim_info struct */
struct mount_info {
	char *host;
	char *remote_dir;
	char *local_dir;
	struct mount_info *prev;
	struct mount_info *next;
};

/*---------------------------- nim_info             --------------------------*/
/* used to store process/machine specific info */
struct nim_info {
	char *cmd_name;							/* command name */
	pid_t pid;									/* PID of current process */
	pid_t pgrp;									/* process group of current process */
	char *xops;									/* pathname of xop file */
	char *tmps;									/* pathname of tmp directory */
	int glock_held;							/* TRUE if glock held by this process */
	struct mount_info *mounts;				/* ptr to 1st mount point */
	char cpu_id[SYS_NMLN];					/* host's CPU id */
	unsigned int err_policy;				/* indicates how to process errors */
	char *errstr;								/* error message string */
	int errno;									/* NIM errno */
	nl_catd msgcat_fd;						/* NIM message catalog fd */
	char nim_name[MAX_NAME_BYTES];		/* host's NIM name */
	char nim_type[MAX_NAME_BYTES];		/* host's NIM configuration type */
	struct nim_if master;					/* master info for the host */
	char master_uid[MAX_NAME_BYTES];		/* master's uid */
	short master_port;						/* master's port # */
	short nimcomm_retries;					/* num retries for master/client comm */
	short nimcomm_delay;						/* delay period for master/client comm */
	short bootp_enabled;						/* TRUE is host is BOOTP enabled */
	short usr_spot_server;					/* TRUE if host servers a /usr SPOT */
};

/*---------------------------- nimere              --------------------------*/
/* used to store compiled regular expressions */
struct nim_ere {
	char *pattern;
	regex_t *reg;
};

/*******************************************************************************
*********************** LIST manipulation structs      *************************
*******************************************************************************/

/*---------------------------- str_list            --------------------------*/
/* used to manage a list of character pointers */
/* this structure is used by the list_init & list_free routines in libnim */
/* it is also the template for creating other kinds of lists, like */
/*		attr_ass_list below: since the first field <list> is just a pointer to */
/*		a pointer, it can point to anything */
/* so, other kinds of lists may be created by copying this struct & replacing */
/*		"char **" with "<your struct> **" */
typedef struct list_struct {
	char **list;						/* ptr to attry of elements */
	int max_num;						/* total num elements in pointed to by list */
	int num;								/* currently available list index */
	int chunk_size;					/* num elements to alloc for */
} LIST;

/*---------------------------- valid_attr_list     --------------------------*/
/* used to store a list of attrs which a program will accept */
typedef struct valid_attr_list {
	int pdattr;
	char *name;
	int required;
	int (*validate)();
} VALID_ATTR;

/*---------------------------- attr_assignment     --------------------------*/
/* used to store info about attr assignments */
typedef struct attr_assignment {
	int pdattr;
	char *name;
	char *value;
	int seqno;
} ATTR_ASS;

#define ATTR_ASS_SIZE				sizeof( ATTR_ASS )

/*---------------------------- attr_ass_list       --------------------------*/
/* used to manage a list of ATTR_ASS structs */
typedef struct attr_ass_list {
	ATTR_ASS **list;
	int max_num;
	int num;
	int chunk_size;
} ATTR_ASS_LIST;

#endif /* _H_CMDNIM_STRUCTS */
