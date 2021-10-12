static char sccsid [] = "@(#)50  1.10  src/bos/usr/bin/ex/ex_data.c, cmdedit, bos41B, 9504A 12/19/94 11:46:31";
/*
 * COMPONENT_NAME: (CMDEDIT) ex_data.c
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 3, 10, 13, 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1981 Regents of the University of California
 * 
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

#include "ex.h"
#include "ex_tty.h"
#include "ex_tune.h"

/* option strings remain char * */

/*
 * Initialization of option values.
 * The option #defines in ex_vars.h are made
 * from this file by the script makeoptions.
 *
 * These initializations are done char by char instead of as strings
 * to confuse xstr so it will leave them alone.
 */
static char	direct[ONMSZ] =
	{'/','v','a','r','/', 't', 'm', 'p'}; 
static char	paragraphs[ONMSZ] = {
	'I', 'P', 'L', 'P', 'P', 'P', 'Q', 'P',		/* -ms macros */
	'P', ' ', 'L', 'I',				/* -mm macros */
	'p', 'p', 'l', 'p', 'i', 'p', 			/* -me macros */
	'b', 'p'					/* bare nroff */
};
/* closing punctuation that should never be wrapped to the next line */
/*
 * multi-character closing punctuation should be preceded by the number
 * of characters (up to 9).  For example, an ellipsis would be input as,
 *	:set cp=3...
 * If, for exmaple, ".." crosses the wrapmargin, the ".." is treated as
 * two dots, not as an ellipsis.
 */
char	closepunct[ONMSZ] = {
	'\'', '"', '.', ',',			/* ' " . , */
	';', ')', ']', '}'			/* ; { [ } */
};
static char	sections[ONMSZ] = {
	'N', 'H', 'S', 'H',				/* -ms macros */
	'H', ' ', 'H', 'U',				/* -mm macros */
	'u', 'h', 's', 'h', '+', 'c'			/* -me macros */
};
char	shell[ONMSZ] =
	{ '/', 'b', 'i', 'n', '/', 's', 'h' };
/* AIX security enhancement */
#if !defined(TVI)
static char	tags[ONMSZ] = {
	't', 'a', 'g', 's', ' ',
	'/', 'u', 's', 'r', '/', 'l', 'i', 'b', '/', 't', 'a', 'g', 's'
};
#endif
/* TCSEC Division C Class C2 */
char termtype[ONMSZ];

/*
 * The partial character indicator appears when a double-wide character
 * is pushed from the last display column to the next display line.
 */
static char partcharind[ONMSZ] = { '-'	};	/* partial character indicator */
static char wraptype[ONMSZ] = { 'w', 'o', 'r', 'd' };

struct	option options[NOPTS + 1] = {
	"autoindent",	"ai",	ONOFF,		0,	0,	0,
	"autoprint",	"ap",	ONOFF,		1,	1,	0,
	"autowrite",	"aw",	ONOFF,		0,	0,	0,
	"beautify",	"bf",	ONOFF,		0,	0,	0,
/* closing punctuation is used for some wraptype rules */
	"closepunct",	"cp",	CLOSEP,		0,	0,	closepunct,
	"directory",	"dir",	STRING,		0,	0,	direct,
	"edcompatible",	"ed",	ONOFF,		0,	0,	0,
	"errorbells",	"eb",	ONOFF,		0,	0,	0,
	"exrc",		"exr",	ONOFF,		0,	0,	0,
	"flash",	"fl",	ONOFF,		1,	1,	0,
	"hardtabs",	"ht",	NUMERIC,	8,	8,	0,
	"ignorecase",	"ic",	ONOFF,		0,	0,	0,
#ifndef UNIX_SBRK
	"linelimit",	"ll",	NUMERIC, MAXLINES, MAXLINES,	0,
#endif
	"lisp",		0,	ONOFF,		0,	0,	0,
	"list",		0,	ONOFF,		0,	0,	0,
	"magic",	0,	ONOFF,		1,	1,	0,
	"mesg",		0,	ONOFF,		1,	1,	0,
/* AIX security enhancement */
#if !defined(TVI)
	"modeline",	0,	ONOFF,		0,	0,	0,
#endif
/* TCSEC Division C Class C2 */
	"number",	"nu",	ONOFF,		0,	0,	0,
	"novice",	0,	ONOFF,		0,	0,	0,
	"optimize",	"opt",	ONOFF,		0,	0,	0,
	"paragraphs",	"para",	STRING,		0,	0,	paragraphs,
/*
 * The partial character indicatar appears when a double-wide char wuold have
 * been in the last display column.   The default pci is a '-' (minus sign).
 */
	"partialcharacter",	"pc",	ONECOL,	0,	0,	partcharind,
	"prompt",	0,	ONOFF,		1,	1,	0,
	"readonly",	"ro",	ONOFF,		0,	0,	0,
	"redraw",	"re",	ONOFF,		0,	0,	0,
	"remap",	0,	ONOFF,		1,	1,	0,
	"report",	0,	NUMERIC,	5,	5,	0,
	"scroll",	"scr",	NUMERIC,	12,	12,	0,
	"sections",	"sect",	STRING,		0,	0,	sections,
	"shell",	"sh",	STRING,		0,	0,	shell,
	"shiftwidth",	"sw",	NUMERIC,	TABS,	TABS,	0,
	"showmatch",	"sm",	ONOFF,		0,	0,	0,
	"showmode",	"smd",	ONOFF,		0,	0,	0,
	"slowopen",	"slow",	ONOFF,		0,	0,	0,
	"tabstop",	"ts",	NUMERIC,	TABS,	TABS,	0,
/* AIX security enhancement */
#if !defined(TVI)
	"taglength",	"tl",	NUMERIC,	0,	0,	0,
	"tags",		"tag",	STRING,		0,	0,	tags,
#endif
/* TCSEC Division C Class C2 */
	"term",		0,	OTERM,		0,	0,	termtype,
	"terse",	0,	ONOFF,		0,	0,	0,
	"timeout",	"to",	ONOFF,		1,	1,	0,
	"ttytype",	"tty",	OTERM,		0,	0,	termtype,
	"warn",		0,	ONOFF,		1,	1,	0,
	"window",	"wi",	NUMERIC,	23,	23,	0,
	"wrapscan",	"ws",	ONOFF,		1,	1,	0,
	"wrapmargin",	"wm",	NUMERIC,	0,	0,	0,
	"writeany",	"wa",	ONOFF,		0,	0,	0,
/*
 * See comment in ex_vops2.c for more details.
 * wraptype indicates to wrap on words or Japanese style
 *	=general is wrap general purpose on "word breaks" where word break is
 *	         defined as white space or between two nonascii characters.
 *	         general is a combination of word and flexible.
 * 	=word  is wrap on words
 *	=rigid is wrap on column and before closing punctuation
 *	=flexible is wrap on column but 1 closing punctuation may extend past margin
 */
	"wraptype",	"wt",	WRAP,		0,	0,	wraptype,
	0,		0,	0,		0,	0,	0,
};
