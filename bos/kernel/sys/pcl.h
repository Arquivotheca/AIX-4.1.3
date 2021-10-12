/* @(#)29	1.2  src/bos/kernel/sys/pcl.h, syssdac, bos411, 9428A410j 6/16/90 00:33:30 */

/*
 * COMPONENT_NAME: SYSSEC - Security Component
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_PCL
#define _H_PCL

/*
 *		PCL Structure and Definitions
 *
 * COMPONENT_NAME: tcbpriv
 *
 * ORIGIN: IBM
 *
 * Copyright International Business Machines Corp. 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * NOTES:
 *	This header file should be included in any source file which
 *	accesses privilege vectors.
 */


/* the format for each process characteristic is: */
struct pce_id
{
	unsigned short	id_len;		/* length of identifier */
	unsigned short	id_type;	/* type of specifier */
#define	PCEID_USER	1
#define	PCEID_GROUP	2
	long	id_data[1];	/* length of data is actually (id_len - 4) */
};

/* address past the last ID in the pcl */
#define	pcl_id_last(a) \
	((struct pce_id *)(((char *)(a)) + (a)->pce_len))

/* advance to the next identifier in an pcl entry */
#define	pcl_id_nxt(id) \
	((struct pce_id *) (((char*)(id))+(((struct pce_id *)(id))->id_len))) 

/* general format of a pcl entry (variable length) */
struct	pcl_entry
{
	unsigned long	pce_len;	/* total length of pcl entry */
	priv_t	pce_privs;		/* privileges granted by this entry */
	/*
	 * list of attributes to be satisfied
	 * for this pcl entry to be applicable
	 */
	struct pce_id	pce_id[1];
};

/*
 * meanings of the fields:
 *
 *	pce_len		The total length of the privilege control entry.  This
 *			is always a multiple of 4, and includes the header 
 *			fields and the sum of the lengths of the process
 *			identifiers.
 *
 *	pce_privs	The privileges to be acquired if the process satisfies
 *			the identification.
 *
 *	pce_id		A variable length list of process characteristics, each
 *			structure specifies a process attribute which must be
 *			satisfied for the privilege control entry to be 
 *			applicable.  The format of these are identical to the
 *			definition of the acl_id structure in <sys/acl.h>.
 */

/* advance to the next entry in an pcl */
#define	pcl_nxt(a) \
	((struct pcl_entry *)(((char *)(a))+(((struct pcl_entry *)(a))->pce_len))) 

/* address past the last entry in the pcl */
#define	pcl_last(a) \
	((struct pcl_entry *)(((char *)(a)) + (a)->pcl_len))

/*
 * Privilege Control List definition
 */
struct	pcl
{
	unsigned long	pcl_len;
	unsigned long	pcl_mode;
	priv_t	pcl_default;
	struct	pcl_entry	pcl_ext[1];
};

#define	PCL_SIZ	((int) &(((struct pcl *) 0)->pcl_ext[0]))

/*
 * meanings of the fields:
 *
 *	pcl_len		The total length of the privilege control list.  This
 *			should always be a multiple of 4.
 *
 *	pcl_default	The default set of privileges acquired by executing
 *			this file.
 *
 *	pcl_ext		Extended privilege control entries.  The format of
 *			these is described below.  These can only increase
 *			the privileges granted by the file.
 */

#endif /* _H_PCL */
