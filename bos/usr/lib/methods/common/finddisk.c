static char sccsid[] = "@(#)52	1.5  src/bos/usr/lib/methods/common/finddisk.c, cfgmethods, bos411, 9428A410j 12/11/91 07:57:08";

/*
 * COMPONENT_NAME: (CFGMETH) finddisk.c
 *
 * FUNCTIONS: finddisk(), findname()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <stdio.h>
#include <errno.h>
#include <cf.h>
#include "cfgdebug.h"

char	*findname();
int	inlist();

struct	ulist_elt {
	char	utype[32];
	struct	ulist_elt *next;
};

/* The purpose of this function is to find where a disk is in
   the system and update its customized object accordingly. A
   match is made when the PVID and disk type in the database
   matches the PVID on the drive.				*/

int
finddisk(disk,par)
struct	CuDv *disk,*par;
{
struct	PdDv	parpd,		/* struct for parent predefined object	*/
		dskpd;		/* struct for disk's predefined object	*/
struct	PdCn	*conlist,*cur;	/* ptr to list of PdCn objects		*/
struct	CuDv	*parlst;	/* ptr to list of parent adapters	*/
struct	listinfo info;		/* info structure			*/
struct	ulist_elt *head,*tail;	/* ptrs for list of unique adapters	*/
char	*dsklst;		/* ptr to output from adapter cfg method*/
char	argstr[128];		/* string for building arguments	*/
char	sstr[1024];		/* string used for search criteria	*/
char	dmystr[128];
int	i,rc;

	/* make sure parent is AVAILABLE */
	if (par->status==AVAILABLE) {
		/* get the PdDv object for the parent so we
		   can run its config method		   		*/
		sprintf(sstr,"uniquetype = '%s'",par->PdDvLn_Lvalue);
		rc = (int)odm_get_first(PdDv_CLASS,sstr,&parpd);
		if (rc == 0 || rc == -1) {
			/* failed to get PdDv object for parent */
			DEBUG_1("finddisk: failed to get %s from PdDv\n",sstr)
			return((rc)?(E_ODMGET):(E_NOPdDv));
		}

		/* see if the drive is attached to the defined parent
		   by running the parent's config method and searching
		   the output for the disk name				*/
		sprintf(argstr,"-D -l %s",par->name);
		if ((rc=odm_run_method(parpd.Configure,argstr,&dsklst,NULL))==-1) {
			/* error running config method -- return with error */
			DEBUG_2("finddisk: error running %s %s\n",
				parpd.Configure,argstr)
			return(E_ODMRUNMETHOD);
		}

		DEBUG_1("finddisk: dsklst=%s\n",dsklst)

		if (findname(disk->name,dsklst)!=NULL) {
			/* disk was found and database updated to
			   reflect its current location. Get a new
			   copy of its CuDv object that shows the
			   correct information */
			sprintf(sstr,"name = '%s'",disk->name);
			rc = (int)odm_get_first(CuDv_CLASS,sstr,disk);
			if (rc == 0 || rc == -1) {
				/* failed to get CuDv obj for disk
				   return with an error */
				return((rc)?(E_ODMGET):(E_NOCuDv));
			}

			/* all done -- return successful */
			return(0);
		}
	}
	/* this disk's name was not returned or its parent is not
	   AVAILABLE -- we need to search all adapters for this disk */

	/* get list of all configured adapters that allow this
	   drive subclass to be connected to them */

	sprintf(sstr,"uniquetype = '%s'",disk->PdDvLn_Lvalue);
	rc = (int)odm_get_first(PdDv_CLASS,sstr,&dskpd);
	if (rc == 0 || rc == -1) {
		/* didn't get PdDv object for the disk */
		DEBUG_1("finddisk: failed to get %s from PdDv\n",sstr)
		return((rc)?(E_ODMGET):(E_NOPdDv));
	}

	/* get list of all possible adapters that allow this
	   disk to be connected */
	sprintf(sstr,"connkey = '%s'",dskpd.subclass);
	conlist = odm_get_list(PdCn_CLASS,sstr,&info,192,1);
	if ((conlist==(struct PdCn *)NULL) || ((int)conlist==-1)) {
		/* didn't get anything--return an error */
		DEBUG_1("finddisk: failed to get %s from PdCn\n",sstr)
		return((conlist)?(E_ODMGET):(E_INVCONNECT));
	}

	/* go through list and find all unique adapter types */
	cur = conlist;
	head = tail = (struct ulist_elt *)NULL;
	for (i=0; i<info.num; i++) {
		if (inlist(cur->uniquetype,head)==-1) {
			/* add this adapter utype to the list */
			if (head==(struct ulist_elt *)NULL) {
				head = malloc(sizeof(struct ulist_elt));
				tail = head;
			} else {
				tail->next = malloc(sizeof(struct ulist_elt));
				tail = tail->next;
			}
			tail->next = (struct ulist_elt *)NULL;
			strcpy(tail->utype,cur->uniquetype);
		}
		cur++;
	}
	odm_free_list(conlist, &info);

	/* take returned list and build a search string that returns
	   a list of all configured adapters that allow this disk to
	   be atached to them other than the defined parent */
	sstr[0] = '\0';
	while (head != (struct ulist_elt *)NULL) {
		struct ulist_elt *t;

		sprintf(sstr,"PdDvLn = '%s' AND status = %d AND name != '%s'",
			head->utype,AVAILABLE,par->name);
		t = head;
		head = head->next;
		free(t);

		DEBUG_1("finddisk: big sstr=%s\n",sstr)

		parlst = odm_get_list(CuDv_CLASS,sstr,&info,8,1);
		if ((int)parlst == -1) {
			/* error! */
			DEBUG_1("finddisk: error getting %s\n",sstr)
			return(E_ODMGET);
		} else if (parlst!=(struct CuDv *)NULL) {
			/* now go through each adapter and see if our
			   disk is attached to it */
			while (info.num > 0) {
				/* get the PdDv object for the parent so we
				   can run its config method */
				sprintf(sstr,"uniquetype = '%s'",
					parlst->PdDvLn_Lvalue);
				rc = (int)odm_get_first(PdDv_CLASS,sstr,&parpd);
				if (rc == 0 || rc == -1) {
					/* failed to get PdDv obj for parent */
					return((rc)?(E_ODMGET):(E_NOPdDv));
				}

				/* see if drive is on the defined parent
				   by running the parent's config method
				   and searching the output for the name */
				sprintf(argstr,"-D -l %s",parlst->name);
				if ((rc=odm_run_method(parpd.Configure,argstr,
							&dsklst,NULL))==-1) {
					/* error running config method
					   return with error */
					return(E_ODMRUNMETHOD);
				}
				if (findname(disk->name,dsklst)!=NULL) {
					/* disk found and database updated to
					   reflect its current location. Get
					   a new copy of its CuDv object that
					   shows the correct information */
					sprintf(sstr,"name = '%s'",disk->name);
					rc=(int)odm_get_first(CuDv_CLASS,sstr,
							      disk);
					if (rc == 0 || rc == -1) {
						/* failed to get CuDv obj for
						   disk return with an error */
						return((rc)?
							(E_ODMGET):(E_NOCuDv));
					}

					/* all done -- return successful */
					return(0);
				}
				parlst++;
				info.num--;
			}

		} /* end else found an adapter */

	} /* end while adapter utypes to check */

	/* tried everything and didn't find the disk--return error */
	DEBUG_1("finddisk: tried everything and didn't find %s\n",
		disk->name)
	return(-1);

}

char *
findname(name,list)
char	*name,*list;
{
char	*testname,*list2;

	DEBUG_2("findname: name=*%s* list=*%s*\n",name,list)

	list2 = list;
	while ((testname = strtok(list2, " \n,\t"))!=NULL) {
		if (strncmp(name,testname)==0) {
			DEBUG_0("findname: name was found\n")
			return(testname);
		}
		list2 = NULL;
	}

	DEBUG_0("findname: name not found\n")
	return(NULL);

}


int
inlist(utype,list)
char	*utype;
struct	ulist_elt *list;
{

	while (list!=(struct ulist_elt *)NULL) {
		if (strcmp(utype,list->utype)==0)
			return(0);
		list = list->next;
	}

	return(-1);

}
