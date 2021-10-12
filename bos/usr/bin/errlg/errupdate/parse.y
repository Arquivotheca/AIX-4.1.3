/*  @(#)28        1.13  src/bos/usr/bin/errlg/errupdate/parse.y, cmderrlg, bos411, 9428A410j 3/31/94 17:05:18 */ 

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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

%{

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errupdate.h>

%}

%union {
    symbol *ysym;
};

%token        IERROR IUMINOR IMINOR ICOLON INUMLIST IDETLIST IUPDATE
%token <ysym> ISTRING INUMBER IDECIMAL
%token <ysym> IEQUAL IPLUS IMINUS
%token <ysym> IERRCLASS IREPORT ILOG IALERT IERRTYPE IERRDESC
%token <ysym> IPROBCAUS IUSERCAUS IUSERACTN IINSTCAUS IINSTACTN
%token <ysym> IFAILCAUS IFAILACTN IDETAILDT ICOMMENT
%type  <ysym> uminors uminor minors minor
%type  <ysym> optstr numlist detlist numstr
%type  <ysym> add delete update

%%

file    :
        |   file '\n'                  { detailflg = 0; }
        |   file stanza                { detailflg = 0; return(1); }
        |   file errorst               { detailflg = 0; return(2); }
        |   file error '\n' '\n'       { yyerrok; detailflg = 0; return(2); }
        ;

stanza  :  add    { instanza=1; }   minors '\n' { ystanza($1,$3); instanza=0; }
        |  delete                          '\n' { ystanza($1, 0); instanza=0; }
        |  update { instanza=1; }  uminors '\n' { ystanza($1,$3); instanza=0; }
        |  noadd  { instanza=1; }   minors '\n' {                 instanza=0; }
        ;

errorst :   IPLUS  error     ':'  '\n' '\n'     { ierror(IPLUS);  return(3); }
        |   IMINUS error     ':'  '\n' '\n'     { ierror(IMINUS); return(3); }
        |   IEQUAL error     ':'  '\n' '\n'     { ierror(IEQUAL); return(3); }
        |   IPLUS  error          '\n' '\n'     { ierror(ICOLON); return(3); }
        |   IMINUS error          '\n' '\n'     { ierror(ICOLON); return(3); }
        |   IEQUAL error          '\n' '\n'     { ierror(ICOLON); return(3); }
        ;

noadd   :   '#'    numstr  ':'             '\n'
        |   '#'    numstr  ':' '=' numstr  '\n'
        ;
/*
 * The second rule is for overriding the crc algorithm by providing
 * the error_id as:
 * + NAME: = 33232039
 */
add     :   IPLUS  ISTRING ':'             '\n' { $$=ymajor(IPLUS, $2); }
        |   IPLUS  ISTRING ':' '=' INUMBER '\n'
            { $$=ymajor(IPLUS, $2); $$->s_number = $5->s_number; }
        ;
delete  :   IMINUS INUMBER ':'    '\n'          { $$=ymajor(IMINUS,$2); }
update  :   IEQUAL INUMBER ':'    '\n'          { $$=ymajor(IEQUAL,$2); }

uminors :                                       { $$=0; }
        |  uminors uminor                       { $$=yminors($1,$2); }
        ;

uminor  :   IREPORT   '='    optstr     '\n'    { $$=yminor($1,$3); }
        |   ILOG      '='    optstr     '\n'    { $$=yminor($1,$3); }
        |   IALERT    '='    optstr     '\n'    { $$=yminor($1,$3); }
        |   error                       '\n'    { ierror(IUMINOR); }
        ;

minors  :                                       { $$=0; }
        |   minors minor                        { $$=yminors($1,$2); }
        ;

minor   :   IERRCLASS '='    numstr     '\n'    { $$=yminor($1,$3); }
        |   IREPORT   '='    optstr     '\n'    { $$=yminor($1,$3); }
        |   ILOG      '='    optstr     '\n'    { $$=yminor($1,$3); }
        |   IALERT    '='    optstr     '\n'    { $$=yminor($1,$3); }
        |   IERRTYPE  '='    numstr     '\n'    { $$=yminor($1,$3); }
        |   IERRDESC  '='    INUMBER    '\n'    { $$=yminor($1,$3); }
        |   IPROBCAUS '='    numlist    '\n'    { $$=yminor($1,$3); }
        |   IUSERCAUS '='    numlist    '\n'    { $$=yminor($1,$3); }
        |   IUSERACTN '='    numlist    '\n'    { $$=yminor($1,$3); }
        |   IINSTCAUS '='    numlist    '\n'    { $$=yminor($1,$3); }
        |   IINSTACTN '='    numlist    '\n'    { $$=yminor($1,$3); }
        |   IFAILCAUS '='    numlist    '\n'    { $$=yminor($1,$3); }
        |   IFAILACTN '='    numlist    '\n'    { $$=yminor($1,$3); }
        |   IDETAILDT '='    detlist    '\n'    { $$=yminor($1,$3); }
        |   ICOMMENT  '='    ISTRING    '\n'    { $$=yminor($1,$3); }
        |   ISTRING   '='    anything   '\n'
            {
				cat_lerror(M(CAT_UPD_YY9),"The keyword %s is not a valid error record template keyword.\n",$1->s_string);
				$$ = 0;
            }
        |   error                       '\n'    { ierror(IMINOR); if (!checkflg) return(0); else return(1); }
        ;

anything:
        | numstr
        | anything ',' numstr
        ;

numstr  : ISTRING                               { $$=$1; }
        | INUMBER                               { $$=$1; }
        ;

numlist :                                       { $$=0; }
        | INUMBER                               { $$=ynumlist( 0,$1); }
        | numlist ',' INUMBER                   { $$=ynumlist($1,$3); }
        | numlist ',' error    '\n'             { ierror(INUMLIST); $$=0; }
        ;

detlist :                                       { $$=0; }
        | INUMBER ',' INUMBER ',' { detailflg++; } 
                                  ISTRING       { $$=ydetaillist($1,$3,$6); }
        | error                   '\n'          { ierror(IDETLIST); $$=0; }
        ;

optstr  :                                       { $$=0; }
        | ISTRING                               { $$=$1; }
        ;

%%

static ierror(code)
{

	yyerrok;
	Errflg++;
	switch(code) {
	case IPLUS:
		cat_lerror(M(CAT_UPD_YY1),"Supply a label after the '+' symbol on an add.\n");
		break;
	case IEQUAL:
		cat_lerror(M(CAT_UPD_YY2),"Supply an error id after the '=' symbol on an update.\n");
		break;
	case IMINUS:
		cat_lerror(M(CAT_UPD_YY3),"Supply an error id after the '-' symbol on a delete.\n");
		break;
	case IUMINOR:
		cat_lerror(M(CAT_UPD_YY4),"\
You may only change the Log, Report, or Alert fields of an\n\
error record template on an update.\n");
		break;
	case IMINOR:
		cat_lerror(M(CAT_UPD_YY5),"The format for this line should be KEYWORD = VALUE.\n");
		break;
	case ICOLON:
		cat_lerror(M(CAT_UPD_YY6),"There is a colon missing after a label or an error id.\n");
		break;
	case INUMLIST:
		cat_lerror(M(CAT_UPD_YY7),"At least one number in the list for this KEYWORD is either\n\
out of range or is not a valid hexadecimal number.\n");
		break;
	case IDETLIST:
		cat_lerror(M(CAT_UPD_YY8),"The format for the Detail Data field is incorrect.  It should\n\
be in the form Detail_Data = LENGTH, ID, ENCODING.\n");
		break;
	}
}

