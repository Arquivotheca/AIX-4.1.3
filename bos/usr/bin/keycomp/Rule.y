/* @(#)45	1.1  src/bos/usr/bin/keycomp/Rule.y, cmdimkc, bos411, 9428A410j 7/8/93 19:59:07 */
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

%{
#include <stdio.h>
#include "dim.h"
#include "Table.h"

#define YYMAXDEPTH	3000
#define YYSTYPE		ParseTable

unsigned int	compose_error = XK_UNBOUND;
unsigned char	compose_error_str[2048] = {'\0'};

static	unsigned int	keysym[128];
static	unsigned int	state[128];
static	int		key_num = 0;
static	unsigned int	layer = 0;
%}

%token LAYER_SWITCH
%token '='
%token '+'
%token ';'
%token '*'
%token '|'
%token '&'
%token IN
%token KEYSYM
%token SHIFTMASK
%token LOCKMASK
%token CONTROLMASK
%token MOD1MASK
%token MOD2MASK
%token MOD3MASK
%token MOD4MASK
%token MOD5MASK
%token CHARACTER
%token STRING
%token NUMBER
%token BEEP
%token IGNORE
%token UNBOUND
%token COMMENT
%token COMPOSE_ERROR

%%

definitions
	: compose_definitions
	| layer_switch_definitions compose_definitions

layer_switch_definitions
	: layer_switch_definition
	| layer_switch_definition layer_switch_definitions

layer_switch_definition
	: LAYER_SWITCH layer '=' key_state_pair ';'
		{
			AddLayerSwitch($2.val, keysym[0], state[0], 0);
			key_num = 0;
		}
	| LAYER_SWITCH layer '=' key_state_pair IN layer ';'
		{
			AddLayerSwitch($2.val, keysym[0], state[0], $6.val);
			key_num = 0;
		}

compose_definitions
	: compose_definition
	| compose_definition compose_definitions

compose_definition
	: compose_target '=' COMPOSE_ERROR ';'
		{
			if((compose_error = $1.val) == XK_NONE){
				strcpy(compose_error_str, $1.str);
			}else{
				compose_error_str[0] = '\0';
			}
			key_num = 0;
		}
	| compose_target '=' compose_sequence ';'
		{
			AddSequence($1.val, $1.str,
					keysym, state, key_num, 0);
			key_num = 0;
		}
	| compose_target '=' compose_sequence IN layer ';'
		{
			AddSequence($1.val, $1.str,
					keysym, state, key_num, $5.val);
			key_num = 0;
		}

compose_target
	: key
	| CHARACTER
	| STRING
	| BEEP
	| IGNORE
	| UNBOUND

compose_sequence
	: key_state_pair
	| key_state_pair compose_sequence

key_state_pair
	: key
		{ keysym[key_num] = $1.val; state[key_num++] = 0; }
	| key '+' key_states
		{ keysym[key_num] = $1.val; state[key_num++] = $3.val; }

key
	: keysym
	| NUMBER

key_states
	: key_state
	| and_key_state
	| or_key_state
	| '*'

and_key_state
	: key_state '&' key_state
		{ $$.val = $1.val & $3.val; }
	| key_state '&' and_key_state
		{ $$.val = $1.val & $3.val; }

or_key_state
	: key_state '|' key_state
		{ $$.val = OR_STATE_FLAG | $1.val | $3.val; }
	| key_state '|' or_key_state
		{ $$.val = OR_STATE_FLAG | $1.val | $3.val; }

key_state
	: SHIFTMASK
	| LOCKMASK
	| CONTROLMASK
	| MOD1MASK
	| MOD2MASK
	| MOD3MASK
	| MOD4MASK
	| MOD5MASK

keysym
	: '*'
	| KEYSYM

layer
	: '*'
	| NUMBER

%%
