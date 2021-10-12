/* "@(#)76	1.12  src/bos/usr/bin/trcrpt/parse.y, cmdtrace, bos411, 9428A410j 6/15/90 23:52:12" */

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: yyparse
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * FUNCTION: yacc grammar description of trace format templates
 */  

%{
#include "td.h"
%}

%union {
            int yint;
            union  td         *ytd;
            struct formatd    *yformatd;
            struct switchd    *yswitchd;
            struct cased      *ycased;
            struct stringd    *ystringd;
            struct macdefd    *ymacdefd;
            struct exprd      *yexprd;
            struct functiond  *yfunctiond;
            struct flagsd     *yflagd;
};

%token              ISWITCH ICASE IBITFLAGS IBITFIELD
%token              ILBRACE IRBRACE IEOD ILOOP ILBRACEM IRBRACEM
%token              IMACDEF ICONSTANT IEXPR IFUNCTION IXQUOTE IQDESCRP
%token <ystringd>   ISTRING IMATCH ISTRING0
%token <yformatd>   IFORMAT ILEVEL IREG IREGFMT
%type  <ytd>        descriptor newdesc optlvl arg
%type  <yformatd>   loop
%type  <ytd>        qdescrp arglist qdescrplist
%type  <yexprd>     expr
%type  <yswitchd>   switch bitflags
%type  <ycased>     caselist case
%type  <yflagd>     flaglist flag
%type  <ystringd>   matchvalue stringlist strreg
%type  <ymacdefd>   macdef
%type  <yfunctiond> function

%%

template    : optlvl ISTRING0            IEOD   {ytemplate(1,$1,$2, 0);}
            | optlvl ISTRING0 descriptor IEOD   {ytemplate(2,$1,$2,$3);}
            ;

optlvl      :                                   {$$=yoptlvl(1, 0);}
            | ILEVEL                            {$$=yoptlvl(2,$1);}
            ;

descriptor  : arg                               {$$=ydescriptor(1,  0,$1);}
            | switch                            {$$=ydescriptor(2,  0,$1);}
            | loop                              {$$=ydescriptor(3,  0,$1);}
            | macdef                            {$$=ydescriptor(4,  0,$1);}
            | function                          {$$=ydescriptor(5,  0,$1);}
            | qdescrp                           {$$=ydescriptor(6,  0,$1);}
            | descriptor arg                    {$$=ydescriptor(7, $1,$2);}
            | descriptor switch                 {$$=ydescriptor(8, $1,$2);}
            | descriptor loop                   {$$=ydescriptor(9, $1,$2);}
            | descriptor macdef                 {$$=ydescriptor(10,$1,$2);}
            | descriptor function               {$$=ydescriptor(11,$1,$2);}
            | descriptor qdescrp                {$$=ydescriptor(12,$1,$2);}
            | bitflags                          {$$=ydescriptor(13, 0,$1);}
            | descriptor bitflags               {$$=ydescriptor(14,$1,$2);}
            | funcdef                           {$$=ydescriptor(15, 0, 0);}
            | descriptor funcdef                {$$=ydescriptor(16,$1, 0);}
            ;

arg         : IFORMAT                           {$$=yarg(1,$1, 0);}
            | IREG                              {$$=yarg(2,$1, 0);}
            | IREGFMT IFORMAT                   {$$=yarg(3,$1,$2);}
            | ISTRING                           {$$=yarg(4,$1, 0);}
            | ILEVEL                            {$$=yarg(5,$1, 0);}
            ;

loop        : ILOOP  IFORMAT newdesc            {$$=yloop(1,$2,$3);}
            | ILOOP  IREG    newdesc            {$$=yloop(2,$2,$3);}
            ;

qdescrp     : IXQUOTE             IXQUOTE       {$$=yqdescrp(1, 0);}
            | IXQUOTE qdescrplist IXQUOTE       {$$=yqdescrp(2,$2);}
            ;

function    : ISTRING   '('         ')'         {$$=yfunction(1,$1, 0);}
            | ISTRING   '(' arglist ')'         {$$=yfunction(2,$1,$3);}
            ;

macdef      : ILBRACEM IREG          IRBRACEM   {$$=ymacdef(1,$2, 0);}
            | ILBRACEM IREG '=' expr IRBRACEM   {$$=ymacdef(2,$2,$4);}
            ;

funcdef     : ILBRACEM ISTRING '(' stringlist ')' {funcdef(1,$2,$4, 0);}
              descriptor IRBRACEM                 {funcdef(2,$2,$4,$7);}
            ;

stringlist  : ISTRING                           {$$=ystringlist(1, 0,$1);}
            | stringlist ',' ISTRING            {$$=ystringlist(1,$1,$3);}
            ;

bitflags    : IBITFLAGS expr ',' flaglist       {$$=ybitflags(1,$2,$4);}
            ;

flaglist    : flag                              {$$=yflaglist(1, 0,$1);}
            | flaglist ',' flag                 {$$=yflaglist(2,$1,$3);}
            ;

flag        : ISTRING         strreg            {$$=yflagentry(1, 0,$1,$2, 0);}
            | ISTRING strreg  strreg            {$$=yflagentry(1, 0,$1,$2,$3);}
            | '&' ISTRING ISTRING strreg        {$$=yflagentry(1,$2,$3,$4, 0);}
            ;

strreg      : ISTRING                           {$$=$1;}
            | IREG                              {$$=(struct stringd *)$1;}
            ;

switch      : expr ',' caselist                 {$$=yswitch(1,$1,$3);}
            ;

caselist    : case                              {$$=ycaselist(1, 0,$1);}
            | caselist ',' case                 {$$=ycaselist(2,$1,$3);}
            ;

case        : matchvalue         newdesc        {$$=ycase(1,$1, 0,$2);}
            | matchvalue ISTRING                {$$=ycase(2,$1,$2, 0);}
            | matchvalue ISTRING newdesc        {$$=ycase(3,$1,$2,$3);}
            ;

matchvalue  : IMATCH                            {$$=ymatchvalue(1,$1);}
            | ISTRING                           {$$=ymatchvalue(2,$1);}
            | IREG                              {$$=ymatchvalue(3,$1);}
            | qdescrp                           {$$=ymatchvalue(4,$1);}
            ;

newdesc     : ILBRACE descriptor IRBRACE        {$$=ynewdesc(1,$2);}
            | ILBRACE            IRBRACE        {$$=ynewdesc(3, 0);}
            ;

qdescrplist : arg                               {$$=yarglist(1, 0,$1);}
            | qdescrplist arg                   {$$=yarglist(1,$1,$2);}
            ;

arglist     : arg                               {$$=yarglist(1, 0,$1);}
            | arglist ',' arg                   {$$=yarglist(1,$1,$3);}
            ;

expr        : arg                               {$$=yexpr(1,$1,  0, 0);}
            | arg '+' arg                       {$$=yexpr(2,$1,'+',$3);}
            | arg '-' arg                       {$$=yexpr(3,$1,'-',$3);}
            | arg '*' arg                       {$$=yexpr(4,$1,'*',$3);}
            | arg '/' arg                       {$$=yexpr(5,$1,'/',$3);}
            | arg '&' arg                       {$$=yexpr(5,$1,'&',$3);}
            ;

%%

