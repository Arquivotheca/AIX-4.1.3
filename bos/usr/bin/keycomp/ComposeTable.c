/* @(#)42	1.1  src/bos/usr/bin/keycomp/ComposeTable.c, cmdimkc, bos411, 9428A410j 7/8/93 19:57:41 */
/*
 * COMPONENT_NAME : (cmdimkc) AIX Input Method Keymap Compiler
 *
 * FUNCTIONS : keycomp
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#define XK_GREEK
#define XK_PUBLISHING
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include "dim.h"

/*
 *      Message Catalog stuffs.
 */
#include <nl_types.h>
#include "keycomp_msg.h"
extern  nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd, MS_KEYCOMP, Num, Str)

#define E_MEM	"keycomp : cannot allocate memory\n"


#define		ALLOC_UNIT		256

ComposeTable	*CT = NULL;
unsigned int	CT_max = 0;
unsigned int	CT_num = 0;

unsigned char	*String = NULL;
unsigned int	String_size = 0;
unsigned int	String_len = 0;

/*----------------------------------------------------------------------*
 *	get new ComposeTable structure
 *----------------------------------------------------------------------*/
static	ComposeTable	*GetCT()
{
	if(CT == NULL){
		CT_num = 0;
		CT_max = ALLOC_UNIT;
		CT = (ComposeTable *)malloc(sizeof(ComposeTable) * CT_max);
		if(CT == NULL){
			fprintf(stderr, MSGSTR(MN_MEM, E_MEM));
			exit(1);
		}
	}else if(CT_num >= CT_max){
		CT_max += ALLOC_UNIT;
		CT = (ComposeTable *)realloc(CT,
					sizeof(ComposeTable) * CT_max);
		if(CT == NULL){
			fprintf(stderr, MSGSTR(MN_MEM, E_MEM));
			exit(1);
		}
	}

	return(&CT[CT_num++]);
}


/*----------------------------------------------------------------------*
 *	get new string area
 *----------------------------------------------------------------------*/
static	unsigned int	GetString(int size)
{
	unsigned int	i;

	if(String == NULL){
		String_len = 0;
		String_size = ALLOC_UNIT;
		String = (unsigned char *)malloc(String_size);
		if(CT == NULL){
			fprintf(stderr, MSGSTR(MN_MEM, E_MEM));
			exit(1);
		}
	}else if(String_len + size > String_size){
		String_size += ALLOC_UNIT;
		String = (unsigned char *)realloc(String, String_size);
		if(CT == NULL){
			fprintf(stderr, MSGSTR(MN_MEM, E_MEM));
			exit(1);
		}
	}

	i = String_len;
	String_len += size;
	return i;
}


/*----------------------------------------------------------------------*
 *	create and initalize a new ComposeTable structure
 *----------------------------------------------------------------------*/
ComposeTable	*NewCT()
{
	ComposeTable *ctp;

	ctp = GetCT();
	ctp->keysym = XK_VoidSymbol;
	ctp->state = 0;
	ctp->layer = 0;
	ctp->result_keysym = XK_NONE;
	ctp->result_string = (unsigned char *)(-1);
	ctp->brother = (ComposeTable *)0;
	ctp->child = (ComposeTable *)0;

	return(ctp);
}


/*----------------------------------------------------------------------*
 *	Search the brother ComposeTable structures
 *----------------------------------------------------------------------*/
unsigned int	SearchCT(
	unsigned int	ct,
	unsigned int	keysym,
	unsigned int	state,
	unsigned int	layer)
{
	unsigned int	pre_ct;			/* previous CT pointer */

	/* find the CT which matches the conditions */
	for(pre_ct = 0; ct != 0; pre_ct = ct, ct = (int)CT[ct].brother){
		if((CT[ct].keysym == keysym) && (CT[ct].state == state) &&
						(CT[ct].layer == layer)){
			break;
		}
	}

	/* if no CT is found, create new one */
	if(ct == 0){
		ct = NewCT() - CT;
		CT[ct].keysym = keysym;
		CT[ct].state = state;
		CT[ct].layer = layer;
		if(pre_ct != 0){
			CT[pre_ct].brother = (ComposeTable *)ct;
		}
	}

	return(ct);
}


/*----------------------------------------------------------------------*
 *	initialize a dummy CT tree head
 *----------------------------------------------------------------------*/
void	InitCTtree(
	ComposeTable	**head)		/* head of compose table */
{
	*head = NewCT();
}


/*----------------------------------------------------------------------*
 *	add a compose sequence definition to CT tree
 *----------------------------------------------------------------------*/
void	AddSequence(
	unsigned int	result_keysym,	/* might be XK_VoidSymbol */
	unsigned char	*result_string, /* might be NULL */
	unsigned int	keysym[],	/* the last is for the first key */
	unsigned int	state[],	/* the last is for the first key */
	int		keysym_num,
	unsigned int	layer)
{
	unsigned int	ct1 = 0, ct2;
	int		i;

	if(keysym_num <= 0){
		return;
	}

	/* set the sequence of keys */
	for(i = 0, ct1 = 0; i < keysym_num; i++, ct1 = ct2){
		ct2 = SearchCT((int)CT[ct1].child, keysym[i], state[i], layer);
		if(CT[ct1].child == 0){
			CT[ct1].child = (ComposeTable *)ct2;
		}
	}
	if((CT[ct1].result_keysym = result_keysym) == XK_NONE){
		i = (*result_string) + 1;
		CT[ct1].result_string = (char *)GetString(i);
		memcpy(&String[(int)CT[ct1].result_string], result_string, i);
	}else{
		CT[ct1].result_string = (unsigned char *)(-1);
	}
}
