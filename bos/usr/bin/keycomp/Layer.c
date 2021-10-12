/* @(#)44	1.1  src/bos/usr/bin/keycomp/Layer.c, cmdimkc, bos411, 9428A410j 7/8/93 19:58:37 */
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
#include "dim.h"

#include <nl_types.h>
#include "keycomp_msg.h"
extern  nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd, MS_KEYCOMP, Num, Str)

#define E_MEM	"keycomp : cannot allocate memory\n"

#define		ALLOC_UNIT		256

LayerSwitch	*layer_switch = NULL;
unsigned int	layer_switch_max = 0;
unsigned int	layer_switch_num = 0;

/*----------------------------------------------------------------------*
 *	Add data to the array of the LayerSwitch structure
 *----------------------------------------------------------------------*/
void	AddLayerSwitch(
	unsigned int result_layer,
	unsigned int keysym,
	unsigned int state,
	unsigned int layer)
{
	if(layer_switch == NULL){
		layer_switch_num = 0;
		layer_switch_max = ALLOC_UNIT;
		layer_switch = (LayerSwitch *)
				malloc(sizeof(LayerSwitch) * layer_switch_max);
		if(layer_switch == NULL){
			fprintf(stderr, MSGSTR(MN_MEM, E_MEM));
			exit(1);
	}
	}else if(layer_switch_num >= layer_switch_max){
		layer_switch_max += ALLOC_UNIT;
		layer_switch = (LayerSwitch *)
		realloc(layer_switch, sizeof(LayerSwitch) * layer_switch_max);
		if(layer_switch == NULL){
			fprintf(stderr, MSGSTR(MN_MEM, E_MEM));
			exit(1);
		}
	}

	layer_switch[layer_switch_num].result_layer = result_layer;
	layer_switch[layer_switch_num].keysym = keysym;
	layer_switch[layer_switch_num].state = state;
	layer_switch[layer_switch_num].layer = layer;
	layer_switch_num++;
}


/*----------------------------------------------------------------------*
 *	Compare two layer
 *----------------------------------------------------------------------*/
static int	CompareLayerSwitch(
		LayerSwitch	*l1,
		LayerSwitch	*l2
	)
{
	return(l1->layer - l2->layer);
}


/*----------------------------------------------------------------------*
 *	Sort the array of the LayerSwith structure
 *----------------------------------------------------------------------*/
void	SortLayerSwitch()
{
	qsort(layer_switch, layer_switch_num, sizeof(LayerSwitch),
						CompareLayerSwitch);
}
