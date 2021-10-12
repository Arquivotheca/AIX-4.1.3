static char sccsid[] = "@(#)89  1.3     src/bldenv/pkgtools/processPtf/tree.c, pkgtools, bos412, 9445B.bldenv 11/7/94 10:34:38";
/*
 *   COMPONENT_NAME: pkgtools
 *
 *   FUNCTIONS: cmpvrmf
 *		insert
 *		traverse
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include "ptf.h"
#include "tree.h"

extern struct btree_node *treeroot;

/*---------------------------------------------------------
| This function compares two vrmf type entries. It checks  |
| to see if the version, release, mod and fix levels       |
| for the two vrmf entries are equal, greater or smaller   |
| and returns 0 if equal, -1 if smaller and 1 for greater. |
----------------------------------------------------------*/

int 
cmpvrmf(vrmftype v1, vrmftype v2)
{
	int ver1, ver2;
	int rel1, rel2;
	int mod1, mod2;
	int fix1, fix2;

	/*-----------------------------------------------------------
	| convert the character type release, version, mod and fix  |
	| levels to integer type for comarison.                     |
	------------------------------------------------------------*/
	ver1 = atoi(v1.version);
	ver2 = atoi(v2.version);
	if (ver1 < ver2) return -1;
	if (ver1 > ver2) return +1;

	rel1 = atoi(v1.release);
	rel2 = atoi(v2.release);
	if (rel1 < rel2) return -1;
	if (rel1 > rel2) return +1;

	mod1 = atoi(v1.mod);
	mod2 = atoi(v2.mod);
	if (mod1 < mod2) return -1;
	if (mod1 > mod2) return +1;

	fix1 = atoi(v1.fix);
	fix2 = atoi(v2.fix);
	if (fix1 < fix2) return -1;
	if (fix1 > fix2) return +1;

	/*----------------------------------------------------
	| If equal return zero.                              |
	----------------------------------------------------*/
	return 0;
}

/*---------------------------------------------------------------
| This functions keeps track of the highest requisite type and  |
| vrmf number for an fileset. If a node in the tree for the      |
| fileset does not exist it creates a new one with the current   |
| requisite type, fileset name and vrmf number. If one already   |
| exists then it checks to see if the new requisite type is     |
| greater than the one stored for that fileset and replaces it   |
| with the new greater requisite type. The requisite types are  |
| in the increasing order of ifreq, coreq and prereq.           |
| Then it checks to see if the vrmf type of the fileset is       |
| greater than the one already stored and replaces it           |
| with greater of the two values.                               |
| This way it keeps a track of highest requisite type and       |
|vrmf number for an fileset.                                     |
---------------------------------------------------------------*/
void 
insert(char * fileset, vrmftype vrmfentry, reqtype requisite)
{
	struct btree_node *temp ; /* temporary pointer*/
	struct btree_node *current; /* pointer to current node in the tree*/
	struct btree_node *prev; /* pointer to previous node in the tree*/
	int found = 0;

	/*-----------------------------------------------------------
	| initialize the current and prev pointers to the root of   |
	| the tree.                                                 |
	------------------------------------------------------------*/
	current = treeroot;
	prev = treeroot ;

	/*------------------------------------------------------------
	| Traverse the tree look for proper place to put key.        |
	------------------------------------------------------------*/
	while(current != NULL)  
	{
		prev = current;
		/*---------------------------------------------------
		| check to see if a node for the fileset exists and  |
		| mark found.                                       |
		----------------------------------------------------*/
		if (!strcmp(fileset, current->filesetFld))
		{
			found = 1;
			break;
		}
		else 
			if ( strcmp(fileset, current->filesetFld) < 0 )
				current = current->left;
			else 
				current = current->right;
	}
	/*--------------------------------------------------------------
	| If a match was found, then update the requisite type and     |
	| vrmf number with the greater of the two values.              |
	| we do not need to compare the requisite type as we will      |
	| be processing them in the order ifreqs, coreqs and prereqs.  |
	| So we can always keep the new requisite type.                |
	---------------------------------------------------------------*/ 
	if (found)
	{
		if (requisite > current->reqFld)
			current->reqFld = requisite;
		if (cmpvrmf(vrmfentry, current->vrmfFld) > 0)
			current->vrmfFld = vrmfentry;
	}
	else 
	{
	/*--------------------------------------------------------------
	| allocate space for a new tree node for the fileset and       |
	| fill in the requisite and vrmf types.                        |
	---------------------------------------------------------------*/
		temp = (struct btree_node *)xmalloc(sizeof(struct btree_node));
		temp->filesetFld = strdup(fileset);
		strcpy(temp->vrmfFld.version, vrmfentry.version);
		strcpy(temp->vrmfFld.release, vrmfentry.release);
		strcpy(temp->vrmfFld.mod, vrmfentry.mod);
		strcpy(temp->vrmfFld.fix, vrmfentry.fix);
		temp->reqFld = requisite;
		temp->left = NULL;
		temp->right = NULL;

		/*------------------------------------------------------------
		| if this is the first time then insert the new node as      |
		| the root of the tree, else use prev.
		|                                                            |
		-------------------------------------------------------------*/
		if (!treeroot) 
			treeroot = temp;
		else 
			if(strcmp(prev->filesetFld, fileset) < 0) 
				prev->right = temp;
			else 
				prev->left = temp;
	} /*else*/
}

/*----------------------------------------------------------------
| This function traverse the tree and write the requisites to    |
| the fileset.prereq file.                                        |
-----------------------------------------------------------------*/

void 
traverse(struct btree_node *tree, FILE * prereqFp)
{
	char buf[BUFSIZE];/* to store the req */

	/*------------------------------------------------------
	| If the tree is empty then return.                    |
	-------------------------------------------------------*/
	if (!tree)  return;

	/*-------------------------------------------------------------
	| traverse the tree recursively and print the reqs in the     |
	| fileset.prereq file.                                         |
	--------------------------------------------------------------*/
	traverse(tree->left, prereqFp);
	switch(tree->reqFld) 
	{
		case IFREQ:
			fprintf (prereqFp, "*ifreq %s %s.%s.%s.%s\n", 
				 tree->filesetFld,	
				 skipLeadingZeros(tree->vrmfFld.version), 
				 skipLeadingZeros(tree->vrmfFld.release), 
				 skipLeadingZeros(tree->vrmfFld.mod), 
				 skipLeadingZeros(tree->vrmfFld.fix));
		break;
		case COREQ:
			fprintf (prereqFp, "*coreq %s %s.%s.%s.%s\n", 
				 tree->filesetFld,
				 skipLeadingZeros(tree->vrmfFld.version), 
				 skipLeadingZeros(tree->vrmfFld.release), 
				 skipLeadingZeros(tree->vrmfFld.mod), 
				 skipLeadingZeros(tree->vrmfFld.fix));
			break;
		case PREREQ:
			fprintf (prereqFp, "*prereq %s %s.%s.%s.%s\n", 
				 tree->filesetFld,
				 skipLeadingZeros(tree->vrmfFld.version), 
				 skipLeadingZeros(tree->vrmfFld.release), 
				 skipLeadingZeros(tree->vrmfFld.mod), 
				 skipLeadingZeros(tree->vrmfFld.fix));
			break;
		default:
			fatal(InvalidReq, tree->reqFld);
	}
	traverse(tree->right, prereqFp);
}

