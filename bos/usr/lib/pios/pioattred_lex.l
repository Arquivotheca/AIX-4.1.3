 /* @(#)96       1.4  src/bos/usr/lib/pios/pioattred_lex.l, cmdpios, bos411, 9428A410j 7/23/93 16:23:23 */
 /*
  * COMPONENT_NAME: (CMDPIOS) Printer Backend
  *
  * FUNCTIONS:
  *
  * ORIGINS: 27
  *
  * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
  * combined with the aggregated modules for this product)
  *                  SOURCE MATERIALS
  * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
  * All Rights Reserved
  *
  * US Government Users Restricted Rights - Use, duplication or
  * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
  */

attr    [@_a-zA-Z][a-zA-Z0-9]
ltr     [a-zA-Z]
bang    [!]
alfltr  [a-zA-Z0-9]
sign    [+-]

%{

#   include "y.tab.h"
#   include "pioattred.h"

#   define  GET_TOK(x)      copy_it(); return(x) 
#   define  SAVE_INT(n)     yylval = (int)yytext[n]

extern int yylval;
extern char attributes[], tok_name[], *this, *lnptr, *inptr, *unptr;

copy_it()
{
strcat(attributes,yytext);
strcpy(tok_name,yytext);
}

%}

%%

","                 { GET_TOK(COMMA); };
"]"                 { GET_TOK(R_BRAK); };
{attr}              { GET_TOK(ATTR); };
.                   { SAVE_INT(0); GET_TOK(CHAR); };
"%%"                { GET_TOK(PERC_PERC); };
%d                  { GET_TOK(PERC_d); };
%[1-9]d             { SAVE_INT(1); GET_TOK(PERC_num_d); };
%c                  { GET_TOK(PERC_c); };
%h                  { GET_TOK(PERC_h); };
%a                  { GET_TOK(PERC_a); };
%I{attr}            { GET_TOK(PERC_I); };
%I\[                {
                    /*
                    ** Find the current end of the string.  Make 'this' point
                    ** to it so that, when this new token is added, 'this' will
                    ** point to it.  This allows the error handler to display
                    ** everything after the token regardless of how
                    ** un-tokenizable the gorp after it is.
                    */
                    this = attributes;
                    while ( *this ) this++;
                    GET_TOK(PERC_I_brak);
                    };
%G{attr}            { GET_TOK(PERC_G); };
%\'.\'              { SAVE_INT(2); GET_TOK(PERC_sq); };
%\"[^"\n]*\"        { GET_TOK(PERC_dq); };
%\{{sign}?[0-9]+\}  { yylval = atoi( yytext+2 ); GET_TOK(PERC_brac); };
%S{attr}            { GET_TOK(PERC_S); };
%L{attr}            { GET_TOK(PERC_L); };
%P[a-z]             { SAVE_INT(2); GET_TOK(PERC_P); };
%Z[a-z]             { SAVE_INT(2); GET_TOK(PERC_Z); };
%g[a-z]             { SAVE_INT(2); GET_TOK(PERC_g); };
%[\+\-\*\/m\=\>\<\!\&\|\^\~] { SAVE_INT(1); GET_TOK(PERC_math); };
%\?                 { GET_TOK(PERC_if); };
%t                  { GET_TOK(PERC_then); };
%e                  { GET_TOK(PERC_else); };
%;                  { GET_TOK(PERC_endif); };
%x                  { GET_TOK(PERC_x); };
%w{ltr}             { SAVE_INT(2); GET_TOK(PERC_w); };
%o                  { GET_TOK(PERC_o); };
%r                  { GET_TOK(PERC_r); };
%i[a-zA-Z!]         { SAVE_INT(2); GET_TOK(PERC_i); };
%p                  { GET_TOK(PERC_p); };
%z                  { GET_TOK(PERC_z); };
%C{alfltr}          { SAVE_INT(2); GET_TOK(PERC_C); };
%F{alfltr}{alfltr}  { GET_TOK(PERC_F); };
%F{bang}{alfltr}    { GET_TOK(PERC_F); };
%F\[{alfltr}+\]     { GET_TOK(PERC_F_brak); };
%f{alfltr}{alfltr}  { GET_TOK(PERC_f); };
%f{bang}{alfltr}    { GET_TOK(PERC_f); };
%f\[{alfltr}+\]     { GET_TOK(PERC_f_brak); };
%v{ltr}{ltr}        { GET_TOK(PERC_v); };
%v{bang}{ltr}       { GET_TOK(PERC_v); };
%v\[{ltr}+\]        { GET_TOK(PERC_v_brak); };
%U{alfltr}          { GET_TOK(PERC_U); };
%U\[{alfltr}+\]     { GET_TOK(PERC_U_brak); };
%D{attr}            { GET_TOK(PERC_D); };
%`{attr}            { GET_TOK(PERC_bq); };
%`\"                { GET_TOK(PERC_bq); };
%#{attr}            { GET_TOK(PERC_REGEXP); };
