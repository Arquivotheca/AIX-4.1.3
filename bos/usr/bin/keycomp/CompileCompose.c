/* @(#)41	1.1  src/bos/usr/bin/keycomp/CompileCompose.c, cmdimkc, bos411, 9428A410j 7/8/93 19:57:14 */
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

/*----------------------------------------------------------------------*
 *	Include Files
 *----------------------------------------------------------------------*/
#include <stdio.h>
#include "dim.h"

/*
 *      Message Catalog stuffs.
 */
#include <nl_types.h>
#include "keycomp_msg.h"
extern  nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd, MS_KEYCOMP, Num, Str)

#define E_CPP	"keycomp : CPP error\n"
#define E_OUT	"keycomp : output error\n"


extern ComposeTable	*CT;
extern unsigned int	CT_num;

extern LayerSwitch	*layer_switch;
extern unsigned int	layer_switch_num;

extern unsigned char	compose_error_str[];
extern unsigned int	compose_error;

extern unsigned char	*String;
extern unsigned int	String_len;


#ifndef CPP
#define CPP	"/usr/lib/cpp"
#endif

/*----------------------------------------------------------------------*
 *	execute CPP (C Pre-Processor) before parsing
 *----------------------------------------------------------------------*/
int	cpp(
	char *file)		/* file name to be passed to CPP	*/
{
	extern	FILE	*yyin;
	char	cmd[2048];

	strcpy(cmd, CPP);
	strcat(cmd, " ");
	strcat(cmd, file);

	if((yyin = (FILE *)popen(cmd, "r")) == NULL){
		return -1;
	}else{
		return 0;
	}
}

#define	NUM	6
/*----------------------------------------------------------------------*
 *	output compiled compose definition to stdout
 *----------------------------------------------------------------------*/
static int	OutputCompose()		/* sccess -> 0  error -> -1 */
{
	unsigned int	tmp[NUM];

	/* output first NUM integers */
	tmp[0] = DIMIMVersionNumber;
	tmp[1] = CT_num;
	tmp[2] = String_len;
	tmp[3] = layer_switch_num;
	tmp[4] = compose_error;
	tmp[5] = strlen(compose_error_str);
	if(fwrite(tmp, sizeof(unsigned int), NUM, stdout) != NUM){
		return(-1);
	}

	/* output ComposeTables */
	if(tmp[1] != 0){
		if(fwrite(CT, sizeof(ComposeTable), tmp[1], stdout) != tmp[1]){
			return(-1);
		}
	}

	/* output result strings */
	if(tmp[2] != 0){
		if(fwrite(String, sizeof(unsigned char), tmp[2], stdout)
								!= tmp[2]){
			return(-1);
		}
	}

	/* output LayerSwithes */
	if(tmp[3] != 0){
		if(fwrite(layer_switch, sizeof(LayerSwitch), tmp[3], stdout)
								!= tmp[3]){
			return(-1);
		}
	}

	/* output compose error string */ 
	if(tmp[5] != 0){
		if(fwrite(compose_error_str, sizeof(unsigned char),
						tmp[5], stdout) != tmp[5]){
			return(-1);
		}
	}

	return 0;
}


/*----------------------------------------------------------------------*
 *	compile and output compose definition to stdout
 *----------------------------------------------------------------------*/
int	CompileCompose()		/* sccess -> 0  error -> -1 */
{
	extern FILE	*yyin;

	/* initalize ComposeTable */
	InitCTtree(&CT);

	/* open pipe from CPP. CPP read stdout */
	if(cpp("") != 0){
		fprintf(stderr, MSGSTR(MN_CPP, E_CPP));
		return(-1);
	}

	/* parse compose definition */
	if(yyparse() != 0){
		return(-1);
	}

	/* close pipe */
	pclose(yyin);

	/* sort layer switches */
	SortLayerSwitch();

	/* output compose definition */
	if(OutputCompose() != 0){
		fprintf(stderr, MSGSTR(MN_OUT, E_OUT));
		return(-1);
	}

	return 0;
}
