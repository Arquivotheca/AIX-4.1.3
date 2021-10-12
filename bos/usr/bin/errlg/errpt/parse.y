/* "@(#)58	1.1  src/bos/usr/bin/errlg/errpt/parse.y, cmderrlg, bos411, 9428A410j 8/30/89 16:57:09" */

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
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

%{
#include "errpt.h"
#include "sql.h"

extern struct columnd Allmethods;
extern struct columnd Allcolumns[];
extern struct columnd Countcolumns;

%}

%union {
            int     yint;
            char   *ystr;
            struct columndefd *ycolumndefd;
            struct columnd    *ycolumnd;
            struct valued     *yvalued;
            struct opd        *yopd;
            struct seld       *yseld;
};

%token               ICREATE ITABLE IINSERT IINFO ICOLUMNS IFOR IINTO ISTRUCTURE
%token               IDATABASE IVALUES IDROP ISELECT IFROM IQUIT ITABLES
%token               ISHELL IDELETE IUPDATE ISET IORDER IBY
%token               IASCENDING IDESCENDING
%token <yint>        INUMBER
%token <ystr>        INAME ISTRING
%token <yint>        IINTEGER ISMALLINT ICHAR ILINK IVLINK ITO IMETHOD ILONGCHAR
%token               ICOLUMNDEFD
%token               IMATCHES INOT IEQ IGE IGT ILE ILT IIS INULL
%token               IADD ISUB IMUL IDIV
%token               IWHERE IUNLOAD ICOLON ISTANZA IHEX IDECIMAL
%token               INOTMATCHES INOTNULL ICOND
%token               IAND IOR
%token               IINDEXED IMETHODONLY IREMARK ICOUNT
%type  <ycolumndefd> columndefs type
%type  <yvalued>     valuelist orderby orderbylist
%type  <ycolumnd>    columnlist iterator methodlist scolumnlist
%type  <yopd>        expr rel wherelist wherespec
%type  <ystr>        string
%type  <yint>        dispmode optasc dispmodebit
%type  <yseld>       selectd
%left IOR
%left IAND
%left IGE IGT ILE ILT IEQ INE IMATCHES
%left '+' '-'
%left '*' '/'
%left INOT UNARYMINUS

%%

file        : command                            { ycommand(1); }
            | file command                       { ycommand(2); }
            | file error                         { ysync(); }
            | file IQUIT                         { yquit(); }
            ;

command     : nullcmd
            | remark
            | creattab
            | droptab
            | database
            | infocolumns
            | infotables
            | insert
            | update
            | select
            | delete
            | unload
            | set
            | shell
            ;

set         : ISET          ';'                   { yset(0); }
            | ISET   string ';'                   { yset($2); }
            ;

shell       : ISHELL        ';'                   { yshell(0); }
            | ISHELL string ';'                   { yshell($2); }
            ;

remark      : IREMARK        ';'                  { yremark(0); }
            | IREMARK string ';'                  { yremark($2); }
            ;

nullcmd     : ';'

/*
load        : ILOAD IFROM string IINSERT
                 IINTO string ';'                 { yload($3,$6); }
*/

unload      : IUNLOAD            selectd     ';'  { yunload( 0,$2); }
            | IUNLOAD ITO string selectd     ';'  { yunload($3,$4); }
            ;

database    : IDATABASE string ';'               { ydatabase($2); }

creattab    : ICREATE ITABLE string
                '(' columndefs ')' ';'           { ycreattab($3,$5); }
            | ICOND ICREATE ITABLE string
                '(' columndefs ')' ';' { if(Forceflg) ycreattab($4,$6); }
            ;

droptab     : IDROP ITABLE string  ';'            { ydroptab($3); }
            | ICOND IDROP ITABLE string ';' { if(Forceflg) ydroptab($4); }
            ;

infocolumns : IINFO ICOLUMNS IFOR string  ';'     { yinfocolumns($4, 0); }
            | IINFO ICOLUMNS IFOR string
                       ISTRUCTURE string  ';'     { yinfocolumns($4, 0); }
            ;

infotables  : IINFO ITABLES               ';'     { yinfotables(0); }
            | IINFO ITABLES IFOR string   ';'     { yinfotables($4); }
            ;

select      : selectd                     ';'     { yselect($1); }

selectd     : ISELECT dispmode scolumnlist IFROM string
                 orderby
                          { $$=yselectd($5,$3, 0,$2, 0, $6); }
            | ISELECT dispmode scolumnlist IFROM string
                 IWHERE wherespec
                 orderby
                          { $$=yselectd($5,$3,$7,$2, 0,$8); }
            | ISELECT dispmode scolumnlist IFROM string
                 IMETHOD methodlist
                 orderby
                          { $$=yselectd($5,$3, 0,$2,$7,$8); }
            | ISELECT dispmode scolumnlist IFROM string
                 IWHERE  wherespec
                 IMETHOD methodlist
                 orderby
                          { $$=yselectd($5,$3,$7,$2,$9,$10); }
            ;

orderby     :                                    { $$ =  0; }
            | IORDER IBY orderbylist             { $$ = $3; }
            | IORDER IBY orderbylist IASCENDING  { $$ = $3; }
            | IORDER IBY orderbylist IDESCENDING { $$ = $3; }
            ;

orderbylist : string optasc                   { $$ = yorderbylist( 0,$1,$2); }
            | orderbylist ',' string optasc   { $$ = yorderbylist($1,$3,$4); }
            ;

optasc      :                                 { $$ = IASCENDING; }
            | IASCENDING                      { $$ = IASCENDING; }
            | IDESCENDING                     { $$ = IDESCENDING; }
            ;

dispmode    :                         { $$  = 0;  }
            | dispmode dispmodebit    { $$ |= $2; }
            ;

dispmodebit : ICOLON                  { $$  = B_COLON; }
            | IHEX                    { $$  = B_HEX; }
            | IDECIMAL                { $$  = B_DECIMAL; }
            | ISTANZA                 { $$  = B_STANZA; }
            | IMETHODONLY             { $$  = B_METHODONLY; }
            ;

delete      : IDELETE          IFROM string 
                                     ';' { ydelete($3, 0); }
            | IDELETE          IFROM string
                 IWHERE wherespec    ';' { ydelete($3,$5); }
            ;

update      : IUPDATE       string ISET '*' '='
               '(' valuelist ')'            ';'   { yupdate($2, 0,$7, 0); }
            | IUPDATE       string ISET
               '(' columnlist ')' '='
               '(' valuelist  ')'           ';'   { yupdate($2,$5,$9, 0); }
            | IUPDATE       string ISET '*' '='
               '(' valuelist ')'
               IWHERE wherespec  ';'              { yupdate($2, 0,$7,$10); }
            | IUPDATE       string ISET
               '(' columnlist ')' '='
               '(' valuelist  ')'
               IWHERE wherespec ';'               { yupdate($2,$5,$9,$12); }
            ;

insert      : IINSERT IINTO string IVALUES
               '(' valuelist ')'         ';'     { yinsert($3, 0,$6); }
            | IINSERT IINTO string
               '(' columnlist ')' IVALUES
               '(' valuelist  ')'        ';'     { yinsert($3,$5,$9); }
            ;

methodlist  : '*'                                { $$ = &Allmethods; }
            | columnlist                         { $$=$1; }
            ;

scolumnlist : '*'                                { $$ = Allcolumns; }
            | ICOUNT                             { $$ = &Countcolumns; }
            | IMETHODONLY                        { $$=0; }
            | columnlist                         { $$=$1; }
            ;

columnlist  : string                             { $$=ycolumnlist( 0,$1); }
            | columnlist ',' string              { $$=ycolumnlist($1,$3); }
            ;

valuelist   : string                             { $$=yvaluelist( 0,$1, 0); }
            | INUMBER                            { $$=yvaluelist( 0, 0,$1); }
            | valuelist ',' string               { $$=yvaluelist($1,$3, 0); }
            | valuelist ',' INUMBER              { $$=yvaluelist($1, 0,$3); }
            ;

columndefs  : INAME type                      { $$=ycolumndefs( 0,$1,$2); }
            | columndefs ',' INAME type       { $$=ycolumndefs($1,$3,$4); }
            ;

type        : IINTEGER                   iterator  { $$=ytype($1, 0,$2, 0, 0); }
            | ISMALLINT                  iterator  { $$=ytype($1, 0,$2, 0, 0); }
            | ILONGCHAR '(' INUMBER ')'  iterator  { $$=ytype($1,$3,$5, 0, 0); }
            | ICHAR     '(' INUMBER ')'  iterator  { $$=ytype($1,$3,$5, 0, 0); }
            | IMETHOD                              { $$=ytype($1, 0, 0, 0, 0); }
            ;

iterator    :                           { $$=0; }
            | '[' INUMBER ']'           { $$=yiterator($2, 0); }
            | '[' INUMBER ']' IINDEXED  { $$=yiterator($2,IINDEXED); }
            | IINDEXED                  { $$=yiterator( 0,IINDEXED); }
            | IINDEXED '[' INUMBER ']'  { $$=yiterator($3,IINDEXED); }
            ;

wherespec   : ISTRING                   { $$ = yexpr($1,ISTRING,0); }
            | wherelist                 { $$ = $1; }
            ;

wherelist   : rel                       { $$ = $1; }
            | '(' wherelist ')'         { $$ = $2; }
            | wherelist IAND wherelist  { $$ = ywherelist($1,IAND,$3); }
            | wherelist IOR  wherelist  { $$ = ywherelist($1,IOR, $3); }
            ;

rel         : expr IMATCHES       expr  { $$ = yrel($1,IMATCHES,$3); }
            | expr INOT IMATCHES  expr  { $$ = yrel($1,INOTMATCHES,$4); }
            | expr '='            expr  { $$ = yrel($1,IEQ,$3); }
            | expr IEQ            expr  { $$ = yrel($1,IEQ,$3); }
            | expr INE            expr  { $$ = yrel($1,INE,$3); }
            | expr IGE            expr  { $$ = yrel($1,IGE,$3); }
            | expr IGT            expr  { $$ = yrel($1,IGT,$3); }
            | expr ILE            expr  { $$ = yrel($1,ILE,$3); }
            | expr ILT            expr  { $$ = yrel($1,ILT,$3); }
            | expr IIS INULL            { $$ = yrel($1,INULL, 0); }
            | expr IIS INOT INULL       { $$ = yrel($1,INOTNULL, 0); }
            ;

expr        : INUMBER            { $$ = yexpr($1,INUMBER,0); }
            | '-' INUMBER %prec UNARYMINUS { $$ = yexpr(-$2,INUMBER,0); }
            | INAME              { $$ = yexpr($1,INAME,  0); }
            | ISTRING            { $$ = yexpr($1,ISTRING,0); }
            | '(' expr ')'       { $$ = $2; }
            | expr '+' expr      { $$ = yexpr($1,IADD,$3); }
            | expr '-' expr      { $$ = yexpr($1,ISUB,$3); }
            | expr '*' expr      { $$ = yexpr($1,IMUL,$3); }
            | expr '/' expr      { $$ = yexpr($1,IDIV,$3); }

string      : INAME                               { $$ = $1; }
            | ISTRING                             { $$ = $1; }
            ;

%%

