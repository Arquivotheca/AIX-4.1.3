/* @(#)27       1.2  src/bldenv/pkgtools/list.h, pkgtools, bos412, GOLDA411a 1/29/93 14:33:04 */
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Define the "Link" structure used in the list	handling routines. */

struct _Link {
	struct _Link	*next;
	char		*data;
};
typedef	struct _Link	Link;

/* Define the "List" structure used in the list	handling routines. */

struct _List {
	Link	*last;
	Link	*current;
	int	count;
};
typedef	struct _List	List;

/* Function prototypes */

List		*listNew();
Link		*linkNew();
void		listAppend(List *, char *);
void		listInit(List *);
void		listInsert(List *, char *);
void		listRewind(List *);
char		*listGet(List *);
int		listCompare(List *, char *);
char		*space (int);
