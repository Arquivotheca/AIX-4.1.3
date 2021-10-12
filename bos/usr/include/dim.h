/* @(#)39	1.1  src/bos/usr/include/dim.h, cmdims, bos411, 9428A410j 7/8/93 19:44:36 */
/*
 * COMPONENT_NAME : (cmdims) SBCS Input Method
 *
 * FUNCTIONS : header file for Dymamic Composing IM (DIM)
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef	_dim_h
#define	_dim_h

#include <im.h>
#include <imP.h>

/*----------------------------------------------------------------------*
 *	DIM version number (current 1.1)
 *----------------------------------------------------------------------*/
#define DIMIMVersionNumber	((1 << 16) | 1)

/*----------------------------------------------------------------------*
 *	pseudo keysyms for dim
 *----------------------------------------------------------------------*/
#define	XK_NONE		0x7fffff01
#define	XK_BEEP		0x7fffff02
#define	XK_UNBOUND	0x7fffff03
#define	XK_IGNORE	0x7fffff04
#define	XK_ALL		0x7fffff05

#define	DIM_BEEPPERCENT		50

#define OR_STATE_FLAG		0x80000000


/*----------------------------------------------------------------------*
 *	tables of compose key sequences
 *----------------------------------------------------------------------*/
typedef struct _ComposeTable {
	unsigned int		keysym;
	unsigned int		state;
	unsigned int		layer;
	unsigned int		result_keysym;
	unsigned char		*result_string;
	struct _ComposeTable	*brother;
	struct _ComposeTable	*child;
} ComposeTable;

/*----------------------------------------------------------------------*
 *	tables of layer switch
 *----------------------------------------------------------------------*/
typedef struct _LayerSwitch {
	unsigned int		keysym;
	unsigned int		state;
	unsigned int		layer;
	unsigned int		result_layer;
} LayerSwitch;

/*----------------------------------------------------------------------*
 *	Defenition of DIM FEP structure.
 *----------------------------------------------------------------------*/
typedef struct _DIMFep {
	IMFepCommon	common;		/* IMFEP common */
	unsigned int	version;	/* DIM version */
	IMKeymap	*immap;		/* imkeymap */
	unsigned char	*compose;	/* area to store compose file */
	ComposeTable	*compose_table;	/* compose table */
	unsigned int	compose_error;	/* keysym for compose error */
	unsigned char	*compose_error_str; /* string for compose error */
	LayerSwitch	*layer_switch;	/* layer switch definitions */
	unsigned int	layer_switch_num; /* the number of layer switches */
} DIMFepRec, *DIMFep;

/*----------------------------------------------------------------------*
 *	Defenition of DIM Object structure.
 *----------------------------------------------------------------------*/
#define MAX_NUM		16

typedef struct _DIMObject {
	IMObjectCommon	common;		/* IM Common info */
	IMBuffer	output;		/* output buffer */
	ComposeTable	*compose;	/* compose state pointer */
	unsigned int	layer;		/* layer */
	unsigned int	keysym[MAX_NUM];/* keysyms in compose sequence */
	unsigned int	state[MAX_NUM];	/* state in compose sequence */
	ComposeTable	*item[MAX_NUM];	/* compose item in compose sequence */
	int		item_num;	/* the number of keysym */
} DIMObjectRec, *DIMObject;

#endif	/* _dim_h */
