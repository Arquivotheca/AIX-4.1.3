# COMPONENT_NAME: CMDMH lexedit.sh
# @(#)21	1.7  src/bos/usr/bin/mh/zotnet/lexedit.sh, cmdmh, bos411, 9428A410j 12/9/93 10:52:40
# 
# FUNCTIONS: yylook 
#
# ORIGINS: 26  27  28  35 
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#! /bin/sh
${ODE_TOOLS}/usr/bin/cat <<! 
2,/^extern int yylineno;$/c\\
static int start_cond = 0;\\
#define BEGIN start_cond =
/^struct yysvf \*yyestate;$/,/^extern struct yysvf yysvec/d
/^# define YYNEWLINE /,/^int yynstr;/d
/^if (__once_yylex)/,/yymbcurmax=MB_CUR_MAX;/ d
/^while((yynstr = yylook()/,/^if(yywrap()) /d
/^case -1:$/,/^fprintf(yyout,"bad switch yylook /c\\
	default: return(0);
/defined(__cplusplus) && defined(_CPP_IOSTREAMS)/d
/^struct yysvf *yybgin = yysvec+1;$/d
/^int yylineno /,\$d
!
