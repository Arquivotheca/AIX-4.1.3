/* @(#)90       1.2  src/bldenv/pkgtools/processPtf/tree.h, pkgtools, bos412, GOLDA411a 5/12/94 14:50:17 */
/*
 *   COMPONENT_NAME: pkgtools
 *
 *   FUNCTIONS: none
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


typedef struct btree_node {
	struct btree_node *left;
	struct btree_node *right;
        char *filesetFld;
	vrmftype   vrmfFld;
        reqtype reqFld;
};

/*-------------------------------------------
| Function declarations.                    |
-------------------------------------------*/
int cmpvrmf(vrmftype v1, vrmftype v2);
void insert(char *fileset, vrmftype vrmfentry, reqtype requisite);
void traverse( struct btree_node *treeroot, FILE *prereqFp);
