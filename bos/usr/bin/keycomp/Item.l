%{
/* @(#)43	1.1  src/bos/usr/bin/keycomp/Item.l, cmdimkc, bos411, 9428A410j 7/8/93 19:58:11 */
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

#include "Rule.h"
#include <X11/X.h>
#include <bim.h>
#include "dim.h"
#include "Table.h"

/*
 *      Message Catalog stuffs.
 */
#include <nl_types.h>
#include "keycomp_msg.h"
extern	nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd, MS_KEYCOMP, Num, Str)

#define E_PARS  "keycomp : %1$s at line %2$d in %3$s\n"
#define E_KSYM  "unknown keysym"


extern	ParseTable	yylval;

static	unsigned char	filename[2048] = {'\0'};
%}

blank		[ \t\n]
letter		[^ \t\n;=*+|&#]
strletter	[^ \t\n]
digit		[0-9]
hex		[0-9A-Fa-f]

%%

^"#"{blank}*{digit}+({blank}+.*)?\n	line_number();

"BIM_PARSEKEYS"		return value(NUMBER, BIM_PARSEKEYS);
"LAYER_SWITCH"		return token(LAYER_SWITCH);
"="			return token('=');
"+"			return token('+');
";"			return token(';');
"*"			return value('*', XK_ALL);
"|"			return token('|');
"&"			return token('&');
"in"			return token(IN);
"XK_"{letter}+		return keysym(KEYSYM);
"ShiftMask"		return value(SHIFTMASK, ShiftMask);
"LockMask"		return value(LOCKMASK, LockMask);
"ControlMask"		return value(CONTROLMASK, ControlMask);
"Mod1Mask"		return value(MOD1MASK, Mod1Mask);
"Mod2Mask"		return value(MOD2MASK, Mod2Mask);
"Mod3Mask"		return value(MOD3MASK, Mod3Mask);
"Mod4Mask"		return value(MOD4MASK, Mod4Mask);
"Mod5Mask"		return value(MOD5MASK, Mod5Mask);
"\'".+"\'"		return string(CHARACTER);
"\""{strletter}+"\""	return string(STRING);
{digit}+		return number(NUMBER);
"0"[Xx]{hex}+		return hex(NUMBER);
"BEEP"			return value(BEEP, XK_BEEP);
"IGNORE"		return value(IGNORE, XK_IGNORE);
"UNBOUND"		return value(UNBOUND, XK_UNBOUND);
"COMPOSE_ERROR"		return token(COMPOSE_ERROR);
{blank}+		;
"#".*"\n"		;

%%

token(x)
int	x;
{
	strcpy(yylval.str, yytext);
	yylval.token = x;
	yylval.val = 0;
	return (int)(x);
}

value(x, y)
int	x, y;
{
	strcpy(yylval.str, yytext);
	yylval.token = x;
	yylval.val = y;
	return (int)(x);
}

keysym(x)
int	x;
{
	strcpy(yylval.str, &(yytext[3]));
	yylval.token = x;
	if(!lookupkeysym(yylval.str, &(yylval.val))){
		yyerror(MSGSTR(MN_KSYM, E_KSYM));
		exit(1);
	}
	return (int)(x);
}

string(x)
int	x;
{
	int		len = 1;
	unsigned char 	*p = &(yytext[1]);
	unsigned char 	c = yytext[0];

	while(*p){
		if(*p == '\\'){
			p = escchar((p+1), &(yylval.str[len]));
			if(yylval.str[len] == '*'){
				yylval.str[len++] = '\\';
				yylval.str[len] = '*';
			}else if(yylval.str[len] == '\\'){
				len++;
				yylval.str[len] = '\\';
			}
			len++;
		}else if(*p == c){
			break;
		}else{
			yylval.str[len++] = *p++;
		}
	}
	yylval.str[len] = '\0';
	yylval.str[0] = len - 1;
	yylval.token = x;
	yylval.val = XK_NONE;
	return (int)(x);
}

number(x)
int	x;
{
	strcpy(yylval.str, yytext);
	yylval.token = x;
	yylval.val = atoi(yylval.str);
	return (int)(x);
}

hex(x)
int	x;
{
	strcpy(yylval.str, yytext);
	yylval.token = x;
	yylval.val = strtol(yylval.str, NULL, 16);
	return (int)(x);
}

line_number()
{
	filename[0] = '\0';
	sscanf(yytext, "# %d %s", &yylineno, filename);
}

yyerror(s)
char	*s;
{
	unsigned char   *p;

	if((strlen(filename) > 0) && (strcmp(filename, "\"\"") != 0)){
		p = filename;
	}else{
		p = "stdin";
	}

	fprintf(stderr, MSGSTR(MN_PARS, E_PARS), s, yylineno, p);
}
