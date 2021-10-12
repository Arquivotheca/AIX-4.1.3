/* @(#)43       1.20  src/bos/usr/ccs/bin/common/cgram.y, cmdprog, bos411, 9428A410j 4/29/93 08:09:37 */
/*
 * COMPONENT_NAME: (CMDPROG) cgram.y
 *
 * FUNCTIONS: addcase, adddef, bdty, cleardim, dstash, mkty, resetbc, savebc 
 *            setdim, swend, swstart, tcopy                                   
 *
 * ORIGINS: 27 03 09 32 00 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Changes for ANSI C were developed by HCR Corporation for IBM
 * Corporation under terms of a work made for hire contract.
 *
 */
/*
 *  @OSF_COPYRIGHT@
*/
/*
 * Modified: May 91 by RWaters
 * Modified: June 91 by RWaters, P23321 Lint libraries always return something.
 * Modified: July 91 by RWaters, P27220 Allow wide char array initializations.
 */

/* AIWS C compiler */
%term NAME  2
%term STRING  3
%term WSTRING  600      /* New wide string type for lint */
%term ICON  4
%term FCON  5
%term PLUS   6
%term MINUS   8
%term MUL   11
%term AND   14
%term OR   17
%term ER   19
%term QUEST  21
%term COLON  22
%term ANDAND  23
%term OROR  24

/*      special interfaces for yacc alone */
/*      These serve as abbreviations of 2 or more ops:
        ASOP    =, = ops
        RELOP   LE,LT,GE,GT
        EQUOP   EQ,NE
        DIVOP   DIV,MOD
        SHIFTOP LS,RS
        ICOP    ICR,DECR
        UNOP    NOT,COMPL
        STROP   DOT,STREF

        */
%term ASOP  25
%term RELOP  26
%term EQUOP  27
%term DIVOP  28
%term SHIFTOP  29
%term INCOP  30
%term UNOP  31
%term STROP  32

/*      reserved words, etc */
%term TYPE  33
%term QUAL 131
%term TYDEF 132
%term ELLIPSIS 134
%term CLASS  34
%term STRUCT  35
%term RETURN  36
%term GOTO  37
%term IF  38
%term ELSE  39
%term SWITCH  40
%term BREAK  41
%term CONTINUE  42
%term WHILE  43
%term DO  44
%term FOR  45
%term DEFAULT  46
%term CASE  47
%term SIZEOF  48
%term ENUM 49


/*      little symbols, etc. */
/*      namely,

        LP      (
        RP      )

        LC      {
        RC      }

        LB      [
        RB      ]

        CM      ,
        SM      ;

        */

%term LP  50
%term RP  51
%term LC  52
%term RC  53
%term LB  54
%term RB  55
%term CM  56
%term SM  57
%term ASSIGN  58
%term ASM 59

/* ASG is defined in manifest.h as 1+  ... EARTH would collapse that was
   changed. The 1+ is hardwired in the following definitions.
*/

%term ASG_PLUS 7
%term ASG_MINUS 9
%term ASG_MUL 12
%term ASG_AND 15
%term ASG_DIV 61
%term ASG_MOD 63
%term ASG_LS 65
%term ASG_RS 67
%term ASG_OR 18
%term ASG_ER 20

/* at last count, there were 6 shift/reduce, 1 reduce/reduce conflicts
/* these involved:
        if/else
        recognizing functions in various contexts, including declarations
        error recovery
        */

%left CM
%right ASG_PLUS ASG_MINUS ASG_MUL ASG_AND ASG_DIV ASG_MOD ASG_LS ASG_RS ASG_OR ASG_ER ASOP ASSIGN
%right QUEST COLON
%left OROR
%left ANDAND
%left OR
%left ER
%left AND
%left EQUOP
%left RELOP
%left SHIFTOP
%left PLUS MINUS
%left MUL DIVOP
%right UNOP
%right INCOP SIZEOF
%left LB LP STROP
%{
# include "mfile1.h"
# include "messages.h"
#define YYMAXDEPTH 500
#define YYDEBUG 1
int realid;
int ghostid;
int oparamno;
extern int gdebug;
extern int foldadd;
extern eprint();
NODE *temppointer;
%}

        /* define types */
%start trans_unit

%type <intval>  ifelprefix ifprefix doprefix switchpart
                enum_head str_head name_lp type_qual push_level
%type <llval>   con_e opt_con_e ocon_e enum_e
%type <nodep>   e .e term decl_spec odecl_spec declaration_specifiers null_decl
                spec_qual_list type_spec enum_dcl cast_type cast_type_name
                funct_idn declarator fdeclarator nfdeclarator elist
                str_spec str_type ptr_qual pdeclarator

%token <intval> QUAL CLASS NAME STRUCT RELOP CM DIVOP PLUS MINUS SHIFTOP
                MUL AND OR ER ANDAND OROR ASSIGN STROP INCOP UNOP ICON FCON
                ASG_PLUS ASG_MINUS ASG_MUL ASG_AND ASG_DIV ASG_MOD ASG_LS
                ASG_RS ASG_OR ASG_ER
%token <nodep>  TYPE TYDEF

%%

%{
        extern int wloop_level; /* specifies while loop code gen.  */
        extern int floop_level; /* specifies for loop code gen.    */
        int typeFlag;           /* specifies type syntax           */
        char *getFakeName();
%}


trans_unit:        file_start ext_def_list
                |  file_start
                        {       /* "must have at least one declaration in translation unit" */
                                WERROR( devdebug[ANSI_PARSE], MESSAGE(192) );
                        }
                ;

file_start:
                        {
                                /*yydebug = 1;*/
                                /* produce mach. dep. output before anything else */
                                beg_file();
                                ftnend();
                        }
                ;

ext_def_list:      ext_def_list external_def
                |  external_def
                ;
external_def:      data_def
                        {  blevel = 0;  }
                |  error
                        {  blevel = 0;  }
                |  ASM SM
                        {  asmout();  blevel = 0;  }
                ;
data_def:
                   odecl_spec SM
                        {       if (typeFlag == 1)
                                        TagStruct($1);
                                else if (typeFlag > 0)
                                    /* "declaration is missing declarator" */
                                        WERROR( devdebug[ANSI_PARSE],
                                                MESSAGE(140) );
                                $1->in.op = FREE;
                        }
                |  odecl_spec init_dcl_list SM
                        {       if( typeFlag > 2 )
                                        /* "declaration must have explicit type specifiers" */
                                        WERROR( devdebug[ANSI_PARSE],
                                                MESSAGE(141) );
                                $1->in.op = FREE;
                        }
                |  odecl_spec fdeclarator {
                                switch( curclass ){
                                default:
                                        /* "function has illegal storage class" */
                                        UERROR( ALWAYS, MESSAGE(45) );
                                case SNULL:
                                case EXTERN:
                                        defid( tymerge($1, $2), EXTDEF );
                                        break;
                                case STATIC:
                                case FORTRAN:
                                        defid( tymerge($1, $2), curclass );
                                        break;
                                        }
#ifdef  CXREF
                                CXBeginBlk();
#endif
                                } function_body
                        {
                            if( blevel ) cerror(TOOLSTR(M_MSG_275, "function level error" ));
                            if( paramlevel ) cerror(TOOLSTR(M_MSG_276, "parameter level error" ));
                            if( reached ) retstat |= NRETVAL;
                            $1->in.op = FREE;
                            ftnend();
                            volEmitted = 0;     /* emit at most once per function */
                        }
                ;

function_body:     arg_dcl_list {
#if     defined (LINT) || defined (CFLOW)
                                OutFtnDef();
#endif
                        } compoundstmt
                        {
#if     defined (LINT) || defined (CFLOW)
                                short usage;
                                usage = (retstat & RETVAL) ? LINTRET : 0;
                        /*
                         * RW: P23321
                         * Standard lint libraries now use prototypes ONLY.
                         * This means that a return statement is never
                         * seen, even though the function returns a value.
                         * Set the flags accordingly.
                         * (Void returns are handled correctly.)
                         */
                                if ( lintlib ) {
                                        usage |= LINTLIB;
                                        usage |= LINTRET;
                                        usage |= LINTDEF;
                                }

                                OutFtnUsage(&stab[curftn], usage);
#endif
                                cleardim();
                        }
                ;
arg_dcl_list:      arg_dcl_list declaration
                        {
                                /* "mix of old and new style argument declaration" */
                                if (funcstyle == NEW_STYLE)
                                        WARNING( ALWAYS, MESSAGE(137) );
                        }
                |       {       blevel = 1;
                                setdim();
                                setty();
                        }
                ;

declaration:       decl_spec declarator_list  SM
                        {  $1->in.op = FREE;  }
                |  decl_spec SM
                        {  $1->in.op = FREE;  }
                |  error  SM
                ;

odecl_spec:        decl_spec
                |  /* void */
                        {       $$ = mkty(tyalloc(INT));
                                curclass = SNULL;
                                typeFlag = 3;
                        }
                ;
decl_spec:        declaration_specifiers
                        {  $1->in.type = ResultType($1->in.type);  }
                ;

declaration_specifiers: CLASS
                        {  curclass = $1;
                           $$ = mkty(tyalloc(INT));
                           typeFlag = 2;
                        }
                |  type_qual
                        {  curclass = SNULL;
                           $$ = mkty(tyalloc(INT));
                        }
                |  type_spec
                        {  curclass = SNULL;
                           $$ = $1;
                        }
                |  declaration_specifiers CLASS
                        {  if (curclass != SNULL)
			     /* "only one storage class specifier allowed" */
                             UERROR( ALWAYS, MESSAGE(136) );
                           else
			     /* "storage class not the first type specifier" */
                             WARNING( WOBSOLETE, MESSAGE(186) );
                           curclass = $2;
                           $$ = $1;
                        }
                |  declaration_specifiers type_qual
                        {  $$ = $1;  }
                |  declaration_specifiers type_spec
                        {  $$ = $2;  $1->in.op = FREE;  }
                ;
/* Defect 54608: need specifier_qualifier list for casts and sizeof */
spec_qual_list:    type_qual
                        {  $$ = mkty(tyalloc(INT));  }
                |  type_spec
                        {  $$ = $1;  }
                |  spec_qual_list type_qual
                        {  $$ = $1;  }
                |  spec_qual_list type_spec
                        {  $$ = $2;  $1->in.op = FREE;  }
                ;

type_qual:         QUAL
                        {  CheckQualifier($1);  }
                ;

type_spec:         TYPE
                        {       $1->in.type = CheckType($1->in.type);
                                if (curLevel == 0) typeFlag = 2;
                        }
                |  TYDEF
                        {       $1->in.type = copytype(
                                        CheckTypedef($1->in.type), blevel);
                                if (curLevel == 0) typeFlag = 2;
                        }
                |  str_spec
                        {  CheckStruct();  }
                |  enum_dcl
                        {  CheckEnum();  }
                ;

enum_dcl:          enum_head LC moe_list noptcomma RC
                        {       $$ = dclstruct($1);
                                if (curLevel == 0) typeFlag = 0;
                        }
                |  ENUM NAME
                        {       $$ = rstruct($2,0);
                                stwart = instruct;
                                if (curLevel == 0) typeFlag = 2;
#ifdef  CXREF
                                CXRefName($2, lineno);
#endif
                        }
                ;

enum_head:         ENUM
                        {       $$ = bstruct(-1,0);
                                stwart = SEENAME;
                        }
                |  ENUM NAME
                        {       $$ = bstruct($2,0);
                                stwart = SEENAME;
#ifdef  CXREF
                                CXRefName($2, lineno);
#endif
                        }
                ;

moe_list:          moe
                |  moe_list CM moe
                ;

moe:               NAME
                        {       moedef( $1 );
#ifdef  CXREF
                                CXDefName($1, lineno);
#endif
                        }
                |  NAME ASSIGN enum_e
                        {       
                                strucoff = (int)$3; moedef( $1 );
#ifdef  CXREF
                                CXDefName($1, lineno);
#endif
                        }
                ;

enum_e:
                        {       $<intval>$=instruct;
                                stwart=instruct=0;
                                foldMask = INTEGRAL_CONSTANT;
                        } e %prec CM
                        {       $2 = foldexpr($2);
                                /* "illegal type for enumeration constant" */
                                if (!ISBINTEGRAL($2->in.type))
                                    UERROR(ALWAYS, MESSAGE(73));
                                $$ = icons($2);
                                /* "value of enumeration constant is out of range" */
                                if($$ > (CONSZ)INT_MAX || $$ < (CONSZ)INT_MIN)
                                    UERROR(ALWAYS, MESSAGE(74));
                                foldadd = 0;
                                instruct=$<intval>1;
                                foldMask = EXPRESSION;
                        }
                ;
                                

str_spec:          str_head LC uplevel str_dcl_list optsemi downlevel RC
                        {       $$ = dclstruct($1); }
                |  STRUCT NAME
                        {       if (curLevel == 0) {
                                        if (TOPTYPE(stab[$2].stype) == UNDEF)
                                                /* rstruct will define it */
                                                typeFlag = 0;
                                        else
                                                /* might have to tag it */
                                                typeFlag = 1;
                                }
                                $$ = rstruct($2,$1);
#ifdef  CXREF
                                CXRefName($2, lineno);
#endif
                        }
                ;

str_head:          STRUCT
                        {       $$ = bstruct(-1,$1);
                                stwart = 0;
                                if (curLevel == 0) typeFlag = 2;
                        }
                |  STRUCT NAME
                        {       $$ = bstruct($2,$1);
                                stwart = 0;
                                if (curLevel == 0) typeFlag = 0;
#ifdef  CXREF
                                CXRefName($2, lineno);
#endif
                        }
                ;

str_dcl_list:      str_dcl
                |  str_dcl_list SM str_dcl
                |  error
                ;

str_dcl:           str_type declarator_list
                        {  stwart = 0;  $1->in.op = FREE;  }
                ;

uplevel:
                        {  curLevel++;  }
                ;

downlevel:
                        {  curLevel--;  }
                ;


str_type:          decl_spec
                        {       if( curclass != SNULL ){
                                        /* "struct/union field cannot have storage class" */
                                        WERROR( ALWAYS, MESSAGE(190) );
                                        curclass = SNULL;
                                }
                        }
                ;

declarator_list:   declarator
                        {       defid( tymerge($<nodep>0,$1), curclass);
                                stwart = instruct;
                        }
                |  declarator_list  CM
                                {       $<nodep>0->in.type = copytype(
                                                $<nodep>0->in.type, blevel);
                                        $<nodep>$ = $<nodep>0;
                                }  declarator
                        {       defid( tymerge($<nodep>0,$4), curclass);
                                stwart = instruct;
                        }
                ;
declarator:        fdeclarator
                        {       if( paramno > oparamno ){
                                        /* clear prototype symbols */
                                        clearst();
                                        paramno = oparamno;
                                        if( funcstyle == OLD_STYLE )
                                                /* "arg list in declaration" */
                                                UERROR( ALWAYS, MESSAGE(158) );
                                }
                        }
                |  nfdeclarator
                |  nfdeclarator COLON con_e
                        %prec CM
                        {       int fsize;
                                if( !(instruct&(INSTRUCT|INUNION)) )
                                        /* "field outside of structure" */
                                        UERROR( ALWAYS, MESSAGE(38) );
                                if( $3<(UCONSZ)0 || $3 >= (UCONSZ)FIELD ){
                                        /* "illegal field size" */
                                        UERROR( ALWAYS, MESSAGE(56) );
                                        fsize = 1;
                                }
                                else {
                                        fsize = (int)$3;
                                }
                                defid( tymerge($<nodep>0,$1), FIELD|fsize );
                                $$ = NIL;
                        }
                |  COLON con_e
                        %prec CM
                        {       int val;
                                if( !(instruct&(INSTRUCT|INUNION)) )
                                        /* "field outside of structure" */
                                        UERROR( ALWAYS, MESSAGE(38) );
                                /* alignment or hole */
                                if($2 > (UCONSZ)INT_MAX)
                                    val = INT_MAX;
                                else if($2 < (UCONSZ)INT_MIN)
                                    val = INT_MIN;
                                else
                                    val = (int)$2;
                                falloc( stab, val, -1, $<nodep>0 );
                                if( instruct & INUNION ) strucoff = 0;
                                $$ = NIL;
                        }
                |  error
                        {  $$ = NIL; }
                ;

ptr_qual:          qual_list
                        {       $$ = mkty(tyalloc(INT));
                                $$->in.type = ResultType($$->in.type);
                        }
                ;

qual_list:         /* void */
                |  qual_list type_qual
                ;

ocon_e:                 {       $<intval>$=instruct;
                                stwart=instruct=0;
                                foldMask = INTEGRAL_CONSTANT;
                        } opt_con_e
                        {       instruct=$<intval>1;
                                foldMask = EXPRESSION;
				$$ = $2;
                        }
                ;

opt_con_e:            e %prec CM
                        {       $$ = icons(foldexpr($1));
                                foldadd = 0;
                                if ((int)$$ <= 0) {
                                    /* "zero or negative subscript" */
                                    WARNING( ALWAYS, MESSAGE(119) );
                                    $$ = 1;
                                }
                        }
                |  /* void */
                        {       $$ = 0;
                        }
                ;
 
                /* int (a)();   is not a function --- sorry! */
nfdeclarator:      MUL ptr_qual nfdeclarator
                        {  umul:
                                $$ = bdty( UNARY MUL, $3, 0 );
                                $$->in.right->in.type = $2->in.type;
                                $2->in.op = FREE;
                        }
                |  nfdeclarator  LP RP
                        {  uftn:
                                $$ = bdty( UNARY CALL, $1, 0 );
                        }
                |  nfdeclarator  LP proto_list RP
                        {       $$ = bdty( UNARY CALL, $1 , 0 );
                                paramno = AttachProto($$);
#if defined(LINT) || defined(CFLOW) || defined(CXREF)
                                func_ptr = 2;
                                clearst();      /* clear prototype symbols */
                                func_ptr = 0;
#else
                                clearst();      /* clear prototype symbols */
#endif
                                stwart = 0;
                        }
                |  nfdeclarator LB ocon_e RB
                        {  $$ = bdty(LB, $1, $3);  }
                |  NAME
                        {       if( paramlevel > 0 ){
                                        /* parameter declaration */
                                        $$ = bdty( NAME, NIL, ftnarg( $1 ) );
                                } else {
                                        $$ = bdty( NAME, NIL, $1 );
#ifdef  CXREF
                                        if( blevel != 1 ) CXDefName($1, lineno);
#endif
                                }
                        }
                |   LP  nfdeclarator  RP
                        {  $$=$2;  }
                ;
fdeclarator:       MUL ptr_qual fdeclarator
                        {  goto umul;  }
                |  fdeclarator  LP RP
                        {  goto uftn;  }
                |  fdeclarator  LP proto_list RP
                        {       $$ = bdty( UNARY CALL, $1 , 0 );
                                paramno = AttachProto($$);
                                stwart = 0;
                        }
                |  fdeclarator LB ocon_e RB
                        {  $$ = bdty(LB,$1,$3);  }
                |   LP  fdeclarator  RP
                        {  $$ = $2;  }
                |  name_lp  param_list  RP
                        {       $$ = bdty( UNARY CALL, bdty(NAME,NIL,$1), 0 );
                                oparamno = AttachProto($$);
                                stwart = 0;
                        }
                |  name_lp  name_list  RP
                        {       blevel--;
                                $$ = bdty( UNARY CALL, bdty(NAME,NIL,$1), 0 );
                                stwart = 0;
                        }
                |  name_lp  RP
                        {       blevel--;
                                $$ = bdty( UNARY CALL, bdty(NAME,NIL,$1), 0 );
                                stwart = 0;
                        }
                ;

name_lp:          NAME LP
                        {       funcstyle = 0;
                                oparamno = paramno;
#ifdef  CXREF
                                CXDefFtn($1, lineno); CXDefName($1, lineno);
#endif
                                if( paramlevel > 0 ){
                                        $$ = ftnarg( $1 );
                                } else if( stab[$1].sclass == SNULL ){
                                        stab[$1].stype =
                                                INCREF(tyalloc(UNDEF),FTN);
                                }
                                blevel++;
                        }
                ;

name_list:         NAME
                        {       /* "old style argument declaration" */
#ifdef LINT
                                WARNING( WPROTO && WKNR, MESSAGE(138) );
#else
                                WARNING( WPROTO, MESSAGE(138) );
#endif
                                funcstyle = OLD_STYLE;
                                ftnarg( $1 );
#ifdef  CXREF
                                CXRefName($1, lineno);
#endif
                        }
                |  name_list  CM  NAME
                        {       ftnarg( $3 );
#ifdef  CXREF
                                CXRefName($3, lineno);
#endif
                        }
                |  error
                ;
pdeclarator:      declarator
                | null_decl
                        {       switch( curclass ){
                                default:
                                        /* "illegal class" */
                                        WERROR( ALWAYS, MESSAGE(52) );
                                case SNULL:
                                case REGISTER:
                                        break;
                                }
                                curclass = PARAMFAKE;
                        }
                ;
proto_list:       start_param param_list
                ;
param_list:       push_level decl_spec pdeclarator
                        {       paramFlg = 0;   /* void set by defid() */
                                defid( tymerge( $2, $3 ), curclass );
                                stwart = instruct;
                                $2->in.op = FREE;
                        }
                | param_list CM decl_spec pdeclarator
                        {       defid( tymerge( $3, $4 ), curclass );
                                stwart = instruct;
                                $3->in.op = FREE;
                                if( paramFlg & SAW_VOID ){
                                        /* "illegal use of void type" */
                                        UERROR( ALWAYS, MESSAGE(147) );
                                        paramFlg &= ~SAW_VOID;
                                }
                                if( paramFlg & SAW_ELLIP ){
                                        /* "illegal use of ellipsis" */
                                        UERROR( ALWAYS, MESSAGE(173) );
                                        paramFlg &= ~SAW_ELLIP;
                                }
                        }
                | param_list CM ELLIPSIS
                        {       NODE *p;
                                paramFlg |= SAW_ELLIP;
                                p = bdty( NAME, NIL,
                                          ftnarg(lookup(getFakeName(),0)));
                                p->in.type = tyalloc( TELLIPSIS );
                                defid( p, PARAM );
                                p->in.op = FREE;
                        }
                ;

start_param:
                        {  blevel++;  }
                ;

push_level:
                        {       protopush( paramno );
                                paramlevel++;
                                if( blevel == 1 ){
                                        funcstyle = NEW_STYLE;
                                }
                        }
                ;


                /* always preceeded by decl_spec: thus the $<nodep>0's */
init_dcl_list:     init_declarator
                        %prec CM
                |  init_dcl_list  CM
                                {       $<nodep>0->in.type = copytype(
                                                $<nodep>0->in.type, blevel);
                                        $<nodep>$ = $<nodep>0;
                                }  init_declarator
                ;
                /* always preceeded by decl_spec */
xnfdeclarator:     nfdeclarator ASSIGN
                        {       TPTR type;
/* Defect 54608: assignment typedef declaraion cannot have initialisers */
                                if (curclass == TYPEDEF)
				        /* "illegal initialization" */
                                        WERROR(ALWAYS, MESSAGE(61));
                                $1 = tymerge($<nodep>0, $1);
                                if( ISFTN($1->in.type) ){
                                        /* "cannot initialize function variable" */
                                        UERROR( ALWAYS, MESSAGE(166) );
                                        $1->in.type = INCREF($1->in.type, PTR);
                                }
                                Initializer = SINGLE;
                                foldMask = GENERAL_CONSTANT;
                                defid( $1, (curclass==EXTERN && blevel==0) ?
                                        EXTDEF : curclass );
                                ghostid = -1;
                                realid = $1->tn.rval;
                                type = stab[realid].stype;
                                if (blevel) {
                                        if (ISARY(type) &&
                                           (ISCHAR(DECREF(type)) || ISWCHAR(DECREF(type)))) {
                                                NODE *tempNode = $1;
                                                ghostid = makeghost(realid);
                                                tempNode = bdty(NAME, NIL, ghostid);
                                                tempNode->in.type = stab[ghostid].stype;
                                                beginit(ghostid);
                                                tempNode->in.op = FREE;
                                        }
                                        if( stab[realid].sclass == EXTERN )
                                                stab[realid].suse = -lineno;
                                } else
                                        beginit(realid);
                        }
                ;
                /* always preceeded by decl_spec */
init_declarator:   nfdeclarator
                   {    nidcl( tymerge($<nodep>0,$1) );
                   }
                |  xnfdeclarator xnf_init_list
		   {    Initializer = NOINIT;
                        foldMask = EXPRESSION;
                        foldadd = 0;
                   }
                |  fdeclarator
                        {       defid(tymerge($<nodep>0,$1), uclass(curclass));
                                if( paramno > oparamno ){
                                        /* clear prototype symbols */
                                        clearst();
                                        paramno = oparamno;
                                        if( funcstyle == OLD_STYLE )
                                                /* "arg list in declaration" */
                                                UERROR( ALWAYS, MESSAGE(158) );
                                }
                        }
                |  error
                ;

/* Defect 54608: break out initialisers
   to make it simpler to catch bad storage class */
xnf_init_list:          e
                   %prec CM
                   {    TPTR t = stab[ghostid<0?realid:ghostid].stype;
#define ANYCHAR(t)  (ISCHAR(DECREF(t)) || ISWCHAR(DECREF(t)))
                        /*
			   if we have an automatic array of unknown size
                           and only one expression as initializer,
                           then we have an illegal initialization
                           (except for arrays of chars or arrays of wide chars)
                            N.B.: see below for a similar test
                        */
                        if (ISARY(t) && !ANYCHAR(t)) {
                                UERROR(ALWAYS, MESSAGE(61));
                        } else if (!blevel) {
				if (TOPTYPE(t) == STRTY || ISUNION(t))
                                        UERROR(ALWAYS, MESSAGE(61));
                                doinit($1);
                                endinit();
                        } else if (!ISARY(t)) {
                                beginit(realid);
                                doinit($1);
                                endinit();
                        } else { /* must be array of chars or wchars */
                                doinit($1);
                                goto copyghost;
                        }
                   }
                |  LC
                   {    TPTR t = stab[ghostid<0?realid:ghostid].stype;
                        Initializer = LIST;
                        if (blevel) {
                                if (curclass != STATIC &&
                                    (ISAGGREGATE(t) || ISUNION(t))) {
                                        if (ISARY(t) && ANYCHAR(t))
                                                goto arychar;
                                        ghostid = makeghost(realid);
                                }
                                if (curclass != STATIC || !ISARY(t) || !ANYCHAR(t))
                                        beginit(ghostid<0?realid:ghostid);
                        }
arychar:
                                ;
                        }
                    init_list optcomma RC
                        {       NODE *r;
                                TPTR t;
copyghost:
                                endinit();
                                t = stab[realid].stype;
                                if (stab[realid].sclass != STATIC && blevel &&
                                        (ISAGGREGATE(t) || ISUNION(t))) {
                                        /*
                                         * we have an automatic aggregate
                                         * or union initialization.
                                         * copy the contents of the "ghost"
                                         * into the "real" identifier.
                                         *
                                         * However, arrays are special case:
                                         * fix up the auto declaration for an
                                         * array of unknown size. Now that the
                                         * size is known.
                                         */
                                        if (ISARY(t) && t->ary_size == 0) {
                                            struct symtab *p= &stab[realid];
                                            stab[realid].stype->ary_size =
                                                stab[ghostid].stype->ary_size;
                                            oalloc(p, &autooff);
#ifndef XCOFF
                                            StabInfoPrint(p);
#endif
                                        }

                                        /*
                                         * fool the init. system() into
                                         * doing the aggregate/union assignment
                                         * note that arrays are special case.
                                         */
                                        Initializer = SINGLE;
                                        if (ISARY(t)) {
                                                assary(realid, ghostid);
                                        } else {
                                                idname = ghostid;
                                                r = buildtree(NAME, NIL, NIL);
                                                beginit(realid);
                                                doinit(r);
                                                endinit();
                                                r->in.op = FREE;
                                        }
                                }
                        }
                ;

init_list:         initializer
                        %prec CM
                |  init_list  CM  initializer
                ;
initializer:       e
                        %prec CM
                        {  doinit( $1 );  }
                |  ibrace init_list optcomma RC
                        {  irbrace();  }
                ;

                /* the comma is optional in both modes */
optcomma        :       /* VOID */
                |  CM
                ;

                /* the comma is optional sometimes */
noptcomma       :       /* VOID */
                |  CM
                        {       /* "extraneous comma" */
                                WERROR( devdebug[ANSI_PARSE], MESSAGE(142) );
                        }
                ;


optsemi         :       /* VOID */
                        {       /* "structure members must be terminated by ';'" */
                                WERROR( ALWAYS, MESSAGE(143) );
                        }
                |  SM
                ;

ibrace          : LC
                        {  ilbrace();  }
                ;

dcl_stat_list   :  dcl_stat_list decl_spec SM
                        {       if (typeFlag == 1)
                                        TagStruct($2);
                                else if (typeFlag > 0)
                                    /* "declaration is missing declarator" */
                                        WERROR( devdebug[ANSI_PARSE],
                                                MESSAGE(140) );
                                $2->in.op = FREE;
                        }
                |  dcl_stat_list decl_spec init_dcl_list SM
                        {  $2->in.op = FREE;  }
                |  /* empty */
                ;

stmt_list:         stmt_list statement
                |  /* empty */
                        {       bccode();
                                locctr(PROG);
                        }
                ;

/*      STATEMENTS      */

compoundstmt:      begin dcl_stat_list stmt_list RC
                        {       blevel--;
                                clearst();
                                cleardim();
                                if (blevel == 1) {
                                        /* Clear parameters also */
                                        blevel = 0;
                                        paramchk = 1;   /* enable parameter usage warning */
                                        clearst();
                                        paramchk = 0;
#if     defined (LINT) || defined (CFLOW)
                                        lintargu = 0;
#endif
                                }
#ifdef  CXREF
                                CXEndBlk();
#endif
                                checkst( blevel );
                                autooff = *--psavbc;
                                regvar = *--psavbc;
                                fpregvar = *--psavbc;
                        }
                ;

begin:            LC
                        {
#ifdef  CXREF
                                if( blevel > 1 ) CXBeginBlk();
#endif
                                if( blevel == 1 ) dclargs();
                                blevel++;
                                setdim();
                                setty();
                                if( psavbc > &asavbc[BCSZ-2] ) cerror(TOOLSTR(M_MSG_278, "nesting too deep" ));
                                *psavbc++ = fpregvar;
                                *psavbc++ = regvar;
                                *psavbc++ = autooff;
                        }
                ;

statement:         e   SM
                        {       if( gdebug ) eline();
                                ecomp( $1 );
                        }
                |  ASM SM
                        {       if( gdebug ) eline();
                                asmout();
                        }
                |  compoundstmt
                |  ifprefix statement
                        {       deflab($1);
                                reached = 1;
                        }
                |  ifelprefix statement
                        {       if( $1 != NOLAB ){
                                        deflab( $1 );
                                        reached = 1;
                                }
                        }
                |  WHILE LP e RP
                        {       if( gdebug ) eline();
                                savebc();
                                if (!reached)
                                        /* "loop not entered at top" */
                                        WERROR( ALWAYS, MESSAGE(75) );
                                reached = 1;
                                brklab = getlab();
                                contlab = getlab();
                                loop_level = wloop_level;
                                switch (loop_level) {
                                default:
                                        cerror(TOOLSTR(M_MSG_279, "bad while loop code gen value"));
                                        /*NOTREACHED*/
                                case LL_TOP:    /* test at loop top */
                                        deflab(contlab);
                                        if ($3->in.op == ICON &&
                                                        $3->tn.lval != 0) {
                                                flostat = FLOOP;
                                                tfree($3);
                                        } else
                                                ecomp(buildtree(CBRANCH, $3,
                                                        bcon(brklab)));
                                        break;
                                case LL_BOT:    /* test at loop bottom */
                                        if ($3->in.op == ICON &&
                                                        $3->tn.lval != 0) {
                                                flostat = FLOOP;
                                                tfree($3);
                                                deflab(contlab);
                                        } else {
                                                branch(contlab);
                                                deflab($<intval>$ = getlab());
                                        }
                                        break;
                                case LL_DUP:    /* dup. test at top & bottom */
                                        if ($3->in.op == ICON &&
                                                        $3->tn.lval != 0) {
                                                flostat = FLOOP;
                                                tfree($3);
                                                deflab($<intval>$ = contlab);
                                        } else {
# ifndef LINT
                                                register NODE *sav;
                                                extern NODE *tcopy();

                                                sav = tcopy($3);
                                                ecomp(buildtree(CBRANCH,$3,
                                                        bcon(brklab)));
                                                $3 = sav;
# endif
                                                deflab($<intval>$ = getlab());
                                        }
                                        break;
                                }
                        }
                        statement
                        {
                                switch (loop_level) {
                                default:
                                cerror(TOOLSTR(M_MSG_279, "bad while loop code gen. value"));
                                        /*NOTREACHED*/
                                case LL_TOP:    /* test at loop top */
                                        branch(contlab);
                                        break;
                                case LL_BOT:    /* test at loop bottom */
                                        if (flostat & FLOOP)
                                                branch(contlab);
                                        else {
                                                reached = 1;
                                                deflab(contlab);
                                                ecomp(buildtree(CBRANCH,
                                                        buildtree(NOT, $3, NIL),
                                                        bcon($<intval>5)));
                                        }
                                        break;
                                case LL_DUP:    /* dup. test at top & bottom */
                                        if (flostat & FLOOP)
                                                branch(contlab);
                                        else {
                                                if (flostat & FCONT) {
                                                        reached = 1;
                                                        deflab(contlab);
                                                }
                                                ecomp(buildtree(CBRANCH,
                                                        buildtree(NOT, $3, NIL),
                                                        bcon($<intval>5)));
                                        }
                                        break;
                                }
                                if ((flostat & FBRK) || !(flostat & FLOOP))
                                        reached = 1;
                                else
                                        reached = 0;
                                deflab(brklab);
                                resetbc(0);
                        }
                |  doprefix statement WHILE  LP  e  RP   SM
                        {       deflab( contlab );
                                if( flostat & FCONT ) reached = 1;
                                if( gdebug ) eline();
                                ecomp( buildtree( CBRANCH,
                                        buildtree( NOT, $5, NIL ),
                                        bcon( $1 ) ) );
                                deflab( brklab );
                                reached = 1;
                                resetbc(0);
                        }
                |  FOR LP .e SM .e SM
                        {       if( gdebug ) eline();
                                if ($3)
                                        ecomp($3);
                                else if (!reached)
                                        /* "loop not entered at top" */
                                        WERROR( ALWAYS, MESSAGE(75) );
                                savebc();
                                contlab = getlab();
                                brklab = getlab();
                                reached = 1;
                                loop_level = floop_level;
                                switch (loop_level) {
                                default:
                                        cerror(TOOLSTR(M_MSG_279, "bad for loop code gen. value"));
                                        /*NOTREACHED*/
                                case LL_TOP:    /* test at loop top */
                                        deflab($<intval>$ = getlab());
                                        if (!$5)
                                                flostat |= FLOOP;
                                        else if ($5->in.op == ICON &&
                                                        $5->tn.lval != 0) {
                                                flostat |= FLOOP;
                                                tfree($5);
                                                $5 = NIL;
                                        } else
                                                ecomp(buildtree(CBRANCH, $5,
                                                        bcon(brklab)));
                                        break;
                                case LL_BOT:    /* test at loop bottom */
                                        if (!$5)
                                                flostat |= FLOOP;
                                        else if ($5->in.op == ICON &&
                                                        $5->tn.lval != 0) {
                                                flostat |= FLOOP;
                                                tfree($5);
                                                $5 = NIL;
                                        } else
                                                branch($<intval>1 = getlab());
                                        deflab($<intval>$ = getlab());
                                        break;
                                case LL_DUP:    /* dup. test at top & bottom */
                                        if (!$5)
                                                flostat |= FLOOP;
                                        else if ($5->in.op == ICON &&
                                                        $5->tn.lval != 0) {
                                                flostat |= FLOOP;
                                                tfree($5);
                                                $5 = NIL;
                                        } else {
# ifndef LINT
                                                register NODE *sav;
                                                extern NODE *tcopy();

                                                sav = tcopy($5);
                                                ecomp(buildtree(CBRANCH, $5,
                                                        bcon(brklab)));
                                                $5 = sav;
# endif
                                        }
                                        deflab($<intval>$ = getlab());
                                        break;
                                }
                        }
                        .e RP statement
                        {       if (flostat & FCONT) {
                                        deflab(contlab);
                                        reached = 1;
                                }
                                if ($8)
                                        ecomp($8);
                                switch (loop_level) {
                                default:
                                cerror(TOOLSTR(M_MSG_279, "bad for loop code gen. value"));
                                        /*NOTREACHED*/
                                case LL_TOP:    /* test at loop top */
                                        branch($<intval>7);
                                        break;
                                case LL_BOT:    /* test at loop bottom */
                                        if ($5)
                                                deflab($<intval>1);
                                        /*FALLTHROUGH*/
                                case LL_DUP:    /* dup. test at top & bottom */
                                        if ($5) {
                                                ecomp(buildtree(CBRANCH,
                                                        buildtree(NOT, $5, NIL),
                                                        bcon($<intval>7)));
                                        } else
                                                branch($<intval>7);
                                        break;
                                }
                                deflab(brklab);
                                if ((flostat & FBRK) || !(flostat & FLOOP))
                                        reached = 1;
                                else
                                        reached = 0;
                                resetbc(0);
                        }
                | switchpart statement
                        {       extern int adebug;
                                if( reached ) branch( brklab );
                                deflab( $1 );
                                swend();
                                if( adebug ) printf("\t.copt\tfreg,0,2\n");
                                deflab(brklab);
                                if( (flostat&FBRK) || !(flostat&FDEF) )
                                        reached = 1;
                                resetbc(FCONT);
                        }
                |  BREAK  SM
                        {       if( gdebug ) eline();
                                if( brklab == NOLAB )
                                        /* "illegal break" */
                                        UERROR( ALWAYS, MESSAGE(50) );
                                else if(reached) branch( brklab );
                                flostat |= FBRK;
                                goto rch;
                        }
                |  CONTINUE  SM
                        {       if( gdebug ) eline();
                                if( contlab == NOLAB )
                                        /* "illegal continue" */
                                        UERROR( ALWAYS, MESSAGE(55) );
                                else branch( contlab );
                                flostat |= FCONT;
                                goto rch;
                        }
                |  RETURN  SM
                        {       retstat |= NRETVAL;
                                if( gdebug ) eline();
                                if( TOPTYPE(DECREF(stab[curftn].stype)) !=
                                                TVOID )
                                        /* "function %s must return a value" */
                                        WARNING( WRETURN, MESSAGE(144),
                                                stab[curftn].psname );
                                retgen (tyalloc(TVOID), retlab);
                        rch:
                                /* "statement not reached" */
                                WARNING( WREACHED && !lintnrch && !reached,
                                        MESSAGE(100) );
                                reached = 0;
                                lintnrch = 0;
                        }
                |  RETURN e  SM
                        {       register NODE *temp;
                                register TPTR type;
                                if( gdebug ) eline();
                                idname = curftn;
                                temp = buildtree( NAME, NIL, NIL );
                                type = temp->in.type = DECREF(temp->in.type);
                                if( TOPTYPE(type) == TVOID )
                                        /* "void function %s cannot return value" */
                                        UERROR( ALWAYS, MESSAGE(116),
                                                stab[idname].psname);
                                temp = buildtree( RETURN, temp, $2 );
                                /* now, we have the type of the RHS correct */
                                temp->in.left->in.op = FREE;
                                temp->in.op = FREE;
                                temp = buildtree( FORCE, temp->in.right, NIL );
                                temp->tn.rval = FORCEREG;
                                ecomp( temp );
                                retstat |= RETVAL;
                                retgen( type, retlab );
                                reached = 0;
                                lintnrch = 0;
                        }
                |  GOTO NAME SM
                        {       register NODE *q;
                                if( gdebug ) eline();
                                q = block(FREE, NIL, NIL, &labltype);
                                q->tn.rval = idname = $2;
                                defid( q, ULABEL );
                                stab[idname].suse = -lineno;
                                branch( stab[idname].offset );
#ifdef  CXREF
                                CXRefName($2, lineno);
#endif
                                goto rch;
                        }
                |   SM
                |  error  SM
                |  error RC
                |  label statement
                ;
label:             NAME COLON
                        {       register NODE *q;
                                q = block(FREE, NIL, NIL, &labltype);
                                q->tn.rval = fixlab($1);
                                defid( q, LABEL );
                                reached = 1;
#ifdef  CXREF
                                CXDefName(q->tn.rval, lineno);
#endif
                        }
                |  CASE { foldMask = INTEGRAL_CONSTANT; } e COLON
                        {       addcase(foldexpr($3));
                                foldadd = 0;
                                reached = 1;
                                foldMask = EXPRESSION;
                        }
                |  DEFAULT COLON
                        {       reached = 1;
                                adddef();
                                flostat |= FDEF;
                        }
                ;
doprefix:       DO
                        {       savebc();
                                if( !reached )
                                        /* "loop not entered at top" */
                                        WERROR( ALWAYS, MESSAGE(75) );
                                brklab = getlab();
                                contlab = getlab();
                                deflab( $$ = getlab() );
                                reached = 1;
                        }
                ;
ifprefix:       IF LP e RP
                        {       if( gdebug ) eline();
                                $$ = getlab();
                                ecomp( buildtree( CBRANCH, $3, bcon( $$ )));
                                reached = 1;
                        }
                ;
ifelprefix:       ifprefix statement ELSE
                        {       if( reached ) branch( $$ = getlab() );
                                else $$ = NOLAB;
                                if( gdebug ) eline();
                                deflab( $1 );
                                reached = 1;
                        }
                ;

switchpart:        SWITCH  LP  e  RP
                        {       savebc();
                                $3 = foldexpr($3);
                                foldadd = 0;
                                if (ISCONSTANT($3))
                                        /* "constant in conditional context" */
                                        WARNING( WCONSTANT || WHEURISTIC, MESSAGE(24) );
                                if (!ISINTEGRAL($3->in.type))
                                        /* "non-integral controlling expression of a switch" */
                                        UERROR(ALWAYS, MESSAGE(179));
                                if( gdebug ) eline();
                                brklab = getlab();
                                if( TOPTYPE($3->in.type) == LONG ||
                                    TOPTYPE($3->in.type) == ULONG ) 
                                        /* "long in case or switch statement
                                        ** may be truncated in non-ansi
                                        ** compilers" */
                                        WARNING( WPORTABLE, MESSAGE(123) );
                                if (TOPTYPE($3->in.type) == LNGLNG ||
                                    TOPTYPE($3->in.type) == ULNGLNG )
                                        /* "long long in case or switch 
                                            statement may be truncated" */
                                        WARNING( WPORTABLE, MESSAGE(122) );
                                $$ = getlab();
                                swstart();
                                swprolog( $$, $3 );
                                reached = 0;
                        }
                ;
/*      EXPRESSIONS     */
con_e:
                        {       $<intval>$=instruct;
                                stwart=instruct=0;
                                foldMask = INTEGRAL_CONSTANT;
                        } e %prec CM
                        {       $$ = icons(foldexpr($2));
                                foldadd = 0;
                                instruct=$<intval>1;
                                foldMask = EXPRESSION;
                        }
                ;
.e:                e
                |
                        {  $$ = 0;  }
                ;
elist:             e
                        %prec CM
                |  elist  CM  e
                        {  goto bop;  }
                ;

e:                 e RELOP e
                        { preconf:
                                if( yychar==RELOP||yychar==EQUOP||yychar==AND||
                                                yychar==OR||yychar==ER ){
                          precplaint:
                                        /* "precedence confusion possible: parenthesize!" */
                                        WARNING( WEORDER || WHEURISTIC, MESSAGE(92) );
                                }
                          bop:
                                $$ = buildtree( $2, $1, $3 );
                        }
                |  e CM e
                        {       $2 = COMOP;
                                goto bop;
                        }
                |  e DIVOP e
                        {  goto bop;  }
                |  e PLUS e
                        {       if( yychar==SHIFTOP ) goto precplaint;
                                else goto bop;
                        }
                |  e MINUS e
                        {       if( yychar==SHIFTOP ) goto precplaint;
                                else goto bop;
                        }
                |  e SHIFTOP e
                        {       if( yychar==PLUS||yychar==MINUS )
                                        goto precplaint;
                                else goto bop;
                        }
                |  e MUL e
                        {  goto bop;  }
                |  e EQUOP  e
                        {  goto preconf;  }
                |  e AND e
                        {       if( yychar==RELOP||yychar==EQUOP ) goto preconf;
                                else goto bop;
                        }
                |  e OR e
                        {       if( yychar==RELOP||yychar==EQUOP ) goto preconf;
                                else goto bop;
                        }
                |  e ER e
                        {       if( yychar==RELOP||yychar==EQUOP ) goto preconf;
                                else goto bop;
                        }
                |  e ANDAND e
                        {  goto bop;  }
                |  e OROR e
                        {  goto bop;  }
                |  e ASG_MUL e
                        {  abop:
                                $$ = buildtree( $2, $1, $3 );
                        }
                |  e ASG_MOD e
                        {  goto abop;  }
                |  e ASG_DIV e
                        {  goto abop;  }
                |  e ASG_PLUS e
                        {  goto abop;  }
                |  e ASG_MINUS e
                        {  goto abop;  }
                |  e ASG_LS e
                        {  goto abop;  }
                |  e ASG_RS e
                        {  goto abop;  }
                |  e ASG_AND e
                        {  goto abop;  }
                |  e ASG_OR e
                        {  goto abop;  }
                |  e ASG_ER e
                        {  goto abop;  }
                |  e QUEST e COLON e
                        {       $$ = buildtree(QUEST, $1,
                                        buildtree( COLON, $3, $5 ) );
                        }
                |  e ASOP e
                        {       /* "old-fashioned assignment operator"  */
                                WERROR( ALWAYS, MESSAGE(87) );
                                goto bop;
                        }
                |  e ASSIGN e
                        {  goto bop;  }
                |  term
                ;
term:              term INCOP
                        {  $$ = buildtree( $2, $1, bcon(1) );  }
                |  MUL term
                        {  ubop:
                                $$ = buildtree( UNARY $1, $2, NIL );
                        }
                |  AND term
                        {       if( notlval($2, 1) ){
                                        /* "unacceptable operand of &" */
                                        UERROR( ALWAYS, MESSAGE(110) );
                                        $$ = $2;
                                        $$->in.type = INCREF($$->in.type, PTR);
                                } else {
                                        markaddr($2);
                                        goto ubop;
                                }
                        }
                |  MINUS term
                        {       if( !ISARITHM($2->in.type) ){
                                        UERROR( ALWAYS, MESSAGE(188), "unary -" );
                                        $$ = $2;
                                } else {
                                        /* Perform integral promotion */
                                        $$ = buildtree($1, bcon(0), $2);
                                }
                        }
                |  PLUS term
                        {       if( !ISARITHM($2->in.type) ){
                                        UERROR( ALWAYS, MESSAGE(188), "unary +" );
                                        $$ = $2;
                                } else {
                                        /* Perform integral promotion */
                                        $$ = buildtree($1, bcon(0), $2);
                                }
                        }
                |  UNOP term
                        {       register NODE *q;

                                if( ISARITHM($2->in.type) ){
                                        /* Perform integral promotion */
                                        q = buildtree(PLUS, bcon(0), $2);
                                } else {
                                        q = $2;
                                }
                                $$ = buildtree( $1, q, NIL );
                        }
                |  INCOP term
                        {       $$ = buildtree( $1==INCR ? ASG_PLUS : ASG_MINUS,
                                                $2, bcon(1) );
                        }
                |  SIZEOF term
                        {  $$ = doszof( $2 );  }
                |  LP cast_type RP term  %prec INCOP
                        {       $$ = buildtree( CAST, $2, $4 );
                                $$->in.left->in.op = FREE;
                                $$->in.op = FREE;
                                $$ = $$->in.right;
                        }
                |  SIZEOF LP cast_type RP  %prec SIZEOF
                        {  $$ = doszof( $3 );  }
                |  term LB e RB
                        {       $$ = buildtree( UNARY MUL,
                                        buildtree( PLUS, $1, $3 ), NIL );
                        }
                |  funct_idn  RP
                        {       $$ = buildtree( UNARY CALL, $1, NIL );
#if     defined (LINT) || defined (CFLOW)
                                OutFtnRef( $$, ($1->in.type->ftn_parm!=PNIL)?1:0 );
#endif
                        }
                |  funct_idn elist  RP
                        {       $$ = buildtree( CALL, $1, $2 );
#if     defined (LINT) || defined (CFLOW)
                                OutFtnRef( $$, ($1->in.type->ftn_parm!=PNIL)?1:0 );
#endif
                        }
                |  term STROP NAME
                        {       if( $2 == DOT ){
                                        $1 = buildtree( UNARY AND, $1, NIL );
                                }
                                idname = $3;
#ifdef  CXREF
                                CXRefName($3, lineno);
#endif
                                $$ = buildtree( STREF, $1,
                                        buildtree( NAME, NIL, NIL ) );
                                stab[$3].suse = -lineno;
                        }
                |  NAME
                        {       idname = $1;
#ifdef  CXREF
                                CXRefName($1, lineno);
#endif
                                /* recognize identifiers in initializations */
                                if( blevel == 0 &&
                                                TOPTYPE(stab[idname].stype) ==
                                                UNDEF) {
                                        register NODE *q;
                                        /* "undeclared initializer name %s" */
                                        WERROR( ALWAYS, MESSAGE(111),
                                                stab[idname].psname );
                                        q = block(FREE, NIL, NIL, tyalloc(INT));
                                        q->tn.rval = idname;
                                        defid( q, EXTERN );
                                        idname = q->tn.rval;
                                }
                                $$ = buildtree( NAME, NIL, NIL );
                                stab[idname].suse = -lineno;
#ifdef LINT
                                /* count reference to a function address as
                                   a use.  Defect 72807 */
                                if (ISFTN(stab[idname].stype)) {
                                        OutFtnAddrRef(&stab[idname]);
                                }
#endif
                        }
                |  ICON
                        {       $$ = bcon(lastcon);
                                $$->in.type = tyalloc($1);
                        }
                |  FCON
                        {       $$ = bcon(0);
                                $$->in.op = FCON;
# ifndef NOFLOAT
                                $$->fpn.dval = dcon;
# endif
                                $$->fn.type = tyalloc($1);
                                
                        }
                |  STRING
                        {  $$ = getstr(STRING); /* get string contents */ }

                |  WSTRING
                        {  $$ = getstr(WSTRING); /* get wide string contents */ }

                |   LP  e  RP
                        {  $$ = $2;  }
                ;

cast_type:        cast_type_name null_decl
                        {  $1->in.type = ResultType($1->in.type);
                           $$ = tymerge($1,$2);
                           $$->in.op = NAME;
                           $1->in.op = FREE;
                        }
                ;

/* Defect 54608: need good message for storage class in casts and sizeof */
cast_type_name:   spec_qual_list
                       {  $$ = $1; }
                |  CLASS spec_qual_list
                       {  if (devdebug[ANSI_MODE])  /* "illegal class" */
			    UERROR (ALWAYS, MESSAGE(52));
                          else
			    WARNING (ALWAYS, MESSAGE(52));
			  $$ = $2;
			}
                ;

null_decl:         /* empty */
                        {       if( paramlevel > 0 )
                                        $$ = bdty( NAME, NIL, ftnarg(
                                                lookup( getFakeName(), 0 ) ) );
                                else
                                        $$ = bdty( NAME, NIL, -1 );

                        }
                |  LP RP
                        {       if( paramlevel > 0 ){
                                        $$ = bdty( UNARY CALL,
                                                bdty( NAME, NIL,
                                                    ftnarg(
                                                        lookup( getFakeName(),
                                                        0 )
                                                    )
                                                ), 0 );
                                        stwart = 0;
                                } else
                                        $$ = bdty( UNARY CALL,
                                                bdty( NAME, NIL, -1 ), 0 );
                        }
                | LP proto_list RP
                        {       if( paramlevel > 1 ){
                                        $$ = bdty( UNARY CALL,
                                                bdty( NAME, NIL,
                                                    ftnarg(
                                                        lookup( getFakeName(),
                                                        0 )
                                                    )
                                                ), 0 );
                                        stwart = 0;
                                } else
                                        $$ = bdty( UNARY CALL,
                                                bdty( NAME, NIL, -1 ), 0 );
                                paramno = AttachProto($$);
                                clearst();      /* clear prototype symbols */
                        }
                |  LP null_decl RP LP RP
                        {       $$ = bdty( UNARY CALL, $2, 0 );
                        }
                |  LP null_decl RP LP proto_list RP
                        {       $$ = bdty( UNARY CALL, $2, 0);
                                paramno = AttachProto($$);
                                clearst();      /* clear prototype symbols */
                                stwart = 0;
                        }
                |  MUL ptr_qual null_decl
                        {  goto umul;  }
                |  null_decl LB ocon_e RB
                        {  $$ = bdty(LB, $1, $3);  }
                |  LP null_decl RP
                        {  $$ = $2;  }
                ;

funct_idn:         NAME  LP
                        {       idname = $1;
                                if (TOPTYPE(stab[idname].stype) == UNDEF) {
                                        register NODE *q;
                                        q = block(FREE, NIL, NIL,
                                                INCREF(tyalloc(INT), FTN));
                                        q->tn.rval = idname;
                                        defid( q, EXTERN );
                                        idname = q->tn.rval;
                                }
#ifdef  CXREF
                                CXRefName($1, lineno);
#endif
                                $$ = buildtree( NAME, NIL, NIL );
                                stab[idname].suse = -lineno;
                        }
                |  term  LP
                ;
%%

NODE *
mkty( t ) TPTR t; {
        return( block( TYPE, NIL, NIL, t ) );
}

NODE *
bdty( int op, NODE *p, CONSZ v ) 
        {
        register NODE *q;

        q = block(op, p, NIL, tyalloc(INT));

        switch( op ){

        case UNARY MUL:
                q->in.right = mkty(tyalloc(INT));
                break;

        case UNARY CALL:
                break;

        case LB:
                q->in.right = bcon(v);
                break;

        case NAME:
                q->tn.rval = (int)v;
                break;

        default:
                cerror(TOOLSTR(M_MSG_280, "bad bdty"));
        }

        return( q );
}

dstash( n ){ /* put n into the dimension table */
        if( curdim >= ndiments-1 ){
                cerror(TOOLSTR(M_MSG_281, "dimension table overflow"));
        }
        dimtab[ curdim++ ] = n;
}

savebc() {
        if( psavbc > & asavbc[BCSZ-4 ] ){
                cerror(TOOLSTR(M_MSG_282, "whiles, fors, etc. too deeply nested"));
        }
        *psavbc++ = brklab;
        *psavbc++ = contlab;
        *psavbc++ = flostat;
        *psavbc++ = swx;
        *psavbc++ = loop_level;
        flostat = 0;
}

resetbc(mask){
        loop_level = *--psavbc;
        swx = *--psavbc;
        flostat = *--psavbc | (flostat&mask);
        contlab = *--psavbc;
        brklab = *--psavbc;
}

addcase(p) NODE *p; { /* add case to switch */
        if( p->in.op == FCON ){
                /*
                 * "floating point expression in an integral
                 * constant expression"
                 */
                UERROR( ALWAYS, MESSAGE(151) );
                return;
        } else if( p->in.op != ICON ){
                /* "non-constant case expression" */
                UERROR( ALWAYS, MESSAGE(80) );
                return;
        }
        if( TOPTYPE(p->in.type) == LONG || TOPTYPE(p->in.type) == ULONG )
                /* "long in case or switch statement may be truncated
                ** in non-ansi compilers" */
                WARNING( WPORTABLE, MESSAGE(123) );
        if( swp == swtab ){
                /* "case not in switch" */
                UERROR( ALWAYS, MESSAGE(20) );
                return;
        }
        if( swp >= &swtab[SWITSZ] ){
                cerror(TOOLSTR(M_MSG_283, "switch table overflow" ));
        }
        swp->sval = p->tn.lval;
        deflab( swp->slab = getlab() );
        ++swp;
        tfree(p);
}

adddef(){ /* add default case to switch */
        if( swp == swtab ){
                /* "default not inside switch" */
                UERROR( ALWAYS, MESSAGE(29) );
                return;
        }
        if( swtab[swx].slab >= 0 ){
                /* "duplicate default in switch" */
                UERROR( ALWAYS, MESSAGE(34) );
                return;
        }
        deflab( swtab[swx].slab = getlab() );
}

swstart(){
        /* begin a switch block */
        if( swp >= &swtab[SWITSZ] ){
                cerror(TOOLSTR(M_MSG_283, "switch table overflow"));
        }
        swx = swp - swtab;
        swp->slab = -1;
        ++swp;
}

swend(){ /* end a switch block */
        register struct sw *swbeg, *p, *q, *r, *r1;
        CONSZ temp;
        int tempi;

        swbeg = &swtab[swx+1];

        /* sort */

        r1 = swbeg;
        r = swp-1;

        while( swbeg < r ){
                /* bubble largest to end */
                for( q=swbeg; q<r; ++q ){
                        if( q->sval > (q+1)->sval ){
                                /* swap */
                                r1 = q+1;
                                temp = q->sval;
                                q->sval = r1->sval;
                                r1->sval = temp;
                                tempi = q->slab;
                                q->slab = r1->slab;
                                r1->slab = tempi;
                        }
                }
                r = r1;
                r1 = swbeg;
        }

        /* it is now sorted */

        for( p = swbeg+1; p<swp; ++p ){
                if( p->sval == (p-1)->sval ){
                        /* "duplicate case in switch, %d" */
                        UERROR( ALWAYS, MESSAGE(33), p->sval );
                        return;
                }
        }

        swepilog( swbeg-1, swp-swbeg );
        swp = swbeg-1;
}

setdim() { /*  store dimtab info on block entry */
        dimptr++;
        if( dimptr >= &dimrec[BNEST] )
                UERROR( ALWAYS, TOOLSTR(M_MSG_320, "block nesting too deep") );
        dimptr->index  = curdim;
        dimptr->cextern = 0;
}

cleardim(){ /* clear dimtab info on block exit */
        if(dimptr->cextern == 0) {
                curdim = dimptr->index;
                dimptr--;
        } else {
                dimptr--;
                if (dimptr >= dimrec) dimptr->cextern = 1;
        }
}

# ifndef ONEPASS
/* -------------------- tcopy -------------------- */
/*      The extra lope test code added for SysVr2 needs tcopy() in
 *      pass1 of the split compiler.
 */

NODE *
tcopy( p ) register NODE *p; {
        /* make a fresh copy of p */

        register NODE *q;

        q = talloc();
        *q = *p;

        switch( optype(q->in.op) ){

        case BITYPE:
                q->in.right = tcopy(p->in.right);
        case UTYPE:
                q->in.left = tcopy(p->in.left);
        }

        return(q);
}

# endif ONEPASS
