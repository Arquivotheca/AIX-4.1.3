static char sccsid[] = "@(#)05	1.3  src/bos/usr/ccs/lib/libsrc/srcaudit.c, libsrc, bos411, 9428A410j 2/26/91 14:54:36";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	src_odm_auditlog
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#include <odmi.h>

#define newodmchar \
obuf==0 || strcmp(nbuf+elem->offset,obuf+elem->offset)!=0

#define newodmlong \
obuf==0 || *(long *)(nbuf+elem->offset) != *(long *)(obuf+elem->offset)

#define newodmshort \
obuf==0 || *(short *)(nbuf+elem->offset) != *(short *)(obuf+elem->offset)

void src_odm_auditlog(event,status,key,Class,nbuf,obuf)
char *event;		/* event to be audited */
char *key;		/* key for the event */
struct Class *Class;	/* odm class being audited */
char *nbuf;		/* new odm buffer */
char *obuf;		/* old odm buffer */
int status;		/* event status */
{
	char *ptr;
	char *svptr;
	struct ClassElem *elem;
	char *malloc();
	int i;

	/* get space for our audit record */
	ptr=malloc(Class->structsize+(Class->nelem*64));
	if(ptr==0)
		return;
	bzero(ptr,Class->structsize+(Class->nelem*64));
	svptr=ptr;

	strcpy(ptr,key);
	ptr=ptr+strlen(ptr);

	/* for each element that is different in the records passed
	 * add that element and it's new value to the audit log
	 */
	for(i=0,elem=Class->elem;i<Class->nelem;i++,elem++)
	{
		switch(elem->type)
		{
		case ODM_LONG:
			if(newodmlong)
				sprintf(ptr," %s=%ld",
				elem->elemname,*(long *)(nbuf+elem->offset));
			break;
		case ODM_SHORT:
			if(newodmshort)
				sprintf(ptr," %s=%d",
				elem->elemname,*(short *)(nbuf+elem->offset));
			break;
		default:
			if(newodmchar)
				sprintf(ptr," %s=%s",
				elem->elemname,nbuf+elem->offset);
			break;
		}
		ptr=ptr+strlen(ptr);

	}

	/* log our odm change */
	auditlog(event,status,svptr,strlen(svptr));

	/* free space allocated for audit record */
	free(svptr);
}
