/* @(#)85	1.4.1.1  src/bos/kernel/sys/acl.h, syssdac, bos411, 9428A410j 2/18/94 16:52:56 */
/*
 *   COMPONENT_NAME: syssdac
 *
 *   FUNCTIONS: acl_last
 *		acl_nxt
 *		id_last
 *		id_nxt
 *
 * ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _ACL_H
#define _ACL_H

#include <sys/types.h>
#include <sys/access.h>

struct	ace_id
{
	unsigned short	id_len;		/* length of identifier */
	unsigned short	id_type;	/* type of specifier */
/* only 1 uid or gid per ace_id struct when id_type is one of these: */
#		define	ACEID_USER	1
#		define	ACEID_GROUP	2
	long	id_data[1];		/* length of data is actually */
					/* (id_len - 4)		      */
};

/* size of the header of an acl entry */
#define	ID_SIZ	((int) &(((struct ace_id *) 0) -> id_data[0]))

/* address past the last ID in the acl */
#define	id_last(a) \
	((struct ace_id *)(((char *)(a)) + (a)->ace_len))

/* advance to the next identifier in an acl entry */
#define	id_nxt(id) \
	((struct ace_id *) (((char*)(id))+(((struct ace_id *)(id))->id_len))) 

/* General format of an acl entry (variable length) */
struct	acl_entry
{
	unsigned short	ace_len;	/* total length of the acl entry */
	unsigned 	ace_type : 2;	/* the type of acl entry:  see   */
					/* <sys/access.h> for more info  */
	unsigned 	ace_access : 14;    /* permission bits           */
					    /* (granted = 1, denied = 0) */

	struct	ace_id	ace_id[1];          /*
	 				     * list of attributes to be 
					     * satisfied for this acl entry 
					     * to be applicable
					     */
};

#define	ACE_SIZ	((int) &(((struct acl_entry *) 0) -> ace_id[0]))

/* advance to the next entry in an acl */
#define	acl_nxt(a) \
	((struct acl_entry *) (((char*)(a))+(((struct acl_entry*)(a))->ace_len))) 

#define acl_last(a) \
	((struct acl_entry *)(((char *)(a)) + (a)->acl_len))

struct	acl
{
	ulong	acl_len;
	ulong	acl_mode;
		/* TBD: this should include S_TCB */
#		define	ACL_MODE	(S_IXACL|S_ISUID|S_ISGID|S_ISVTX)
	ushort	acl_rsvd;
	ushort	u_access;
	ushort	g_access;
	ushort	o_access;
	struct	acl_entry	acl_ext[1];
};

#define	ACL_SIZ	((int) &(((struct acl *) 0) -> acl_ext[0]))

#endif	/* _ACL_H */
