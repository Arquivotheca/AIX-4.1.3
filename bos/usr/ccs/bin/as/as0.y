/* @(#)75	1.44  src/bos/usr/ccs/bin/as/as0.y, cmdas, bos411, 9428A410j 6/8/94 19:15:06 */
/*
 * COMPONENT_NAME: (CMDAS) Assembler and Macroprocessor 
 *
 * FUNCTIONS: buf_write, error, getreg, yyerror, yywarn, getmsg, yyerror1
 *            error5, yywarn1, yyerror2, yywarn2, exp_result, gen_ref,
 *            convert, init_reflist, upd_ref_entry, add_reflist, add_symlist
 *            get_type, put_reflist, reset_symlist.
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************
* AIWS assembler - lexer and parser					     *
*****************************************************************************/
 
%union {
	struct symtab *stok;	/* symbol table token		*/
	int tok;		/* regular token		*/
	double *dtok;		/* .double token		*/
	long *itok;		/* integer token		*/
	struct exp *etok;	/* expression token		*/
	char *strtok;		/* string token			*/
	struct instab *instok; 	/* instruction table pointer 	*/
	struct sect *sectok;	/* csect table pointer token	*/
}
 
%token COLON
%token PLUS MINUS
%token MUL DIV
%token IOR XOR AND
%token TILDE LSH RSH
%token LITOP
%token CM NL LP RP SEMI
%token PCENT
%token LBRC RBRC LBRK RBRK
%token <stok> NAME QNAME
%token <itok> INT
%token NOCHAR SP ALPH DIG SQ SH DQ
%token <strtok> STRING
%token <instok> INST
%token ISPACE IBYTE ISTRING ISHORT ILONG LOC IVBYTE
%token IGLOBAL ILGLOBAL ISET ICOMM ILCOMM IALIGN
%token IFLOAT IDOUBLE IORG
%token IEXTERN IHASH
%token IUSING IDROP 
%token IDSECT ITBTAG IRENAME ITOCOF
%token ITOC ICSECT ITC
%token IFILE ISTABX IMACHINE ISOURCE
%token IFUNCTION IBF IEF IBB IEB ILINE IXLINE IBI IEI IBC IEC IBS IES
%token ICOPT IXADDR
%token <dtok> FLTNUM
%token <tok> ERRTOK
%token INV_OP
%token INV_STG_CLASS
%token IREF
 
%nonassoc COLON
%left <tok> PLUS MINUS
%left <tok> IOR XOR AND
%left <tok> MUL DIV LSH RSH TILDE
%left <tok> PCENT
 
%type <etok> expr_ref exprl expr nothing badinstr ignore
%type <stok> labels symbol 
 
%{
#include "as.h"
#include <stdarg.h> 

/* flags used in print_lineno					*/
#define DEF -1		/* definition, variable declaration 	*/
#define REF 1		/* variable referenced 			*/
/*                                                              */

 
struct	arg	*ap = arglist;		/* pointer to argument list 	*/
					/* common error messages	*/
struct	exp	*xp = explist;		/* pointer to expression list	*/
struct exp_ref_list reflist[MAX_REFLIST];  /* ref. list of expressions. */
struct exp_ref_list reftmp[MAX_REFLIST]; /* ref. list for subexpressions.*/
                                        /* subexp. followed or preceded  */
                                        /* by MUL DIV LSH RSH TILDE,     */
                                        /* IOR XOR AND                   */
int   cur_reflist = 0;                  /* current index of the reflist  */
int   cur_reftmp = 0;                   /* current index of the reftmp   */
struct symtab *symlist[MAX_REFLIST];    /* symbol list referred in an   */
                                        /* expression                   */
int  cur_symlist = 0;                   /* current index of the symlist  */
struct exp_pat_stack st_lp[MAX_REFLIST];/* expression terms stack.      */
int  st_inx = 0;                        /* current index of st_lp.       */
char  tmp_xtype;

static int begdotp = 0;			/* the begining location counter*/
					/* for an instruction		*/
static	int	curlen; 		/* current length for .word, etc */
int intoc = 0;				/* flag to indicate in toc	*/
int csect_label = 0;			/* flag to indicate label on csect*/
int prev_label = 0;                     /* flag to indicate previous lab*/
extern int listopt;			/* list option flag		*/
extern int fflag;	   		/* floating point flag 		*/
extern struct exec hdr;			/* output file header		*/
struct symtab *label_ptr;               /* prev label pointer           */
int nxtsym_sc;				/* the next symbols storage class*/
extern  int in_comm;                    /* .comm or .lcomm indicator    */
SECT *ccsect_save = 0;                  /* save the current ccsect      */

%}
 
%%
 
%{
	int	i, ii;
	long	li;
	long	bound;
%}
 
file:	/* empty */ {
		goto reset;
	}
	| file linstruction NL {
		if (listopt) 
		   newlinelst();		/* flush listing line */
		lineno++;			/* increment line number*/
		goto reset;
	}
	| file linstruction SEMI {		/* semicolon		*/
		if (listopt) {
                  if ( passno == 1 ) {
                     if (mn_xref) {
                        fputs(mn_buf, listfil);
                        fputs(mn_buf, tmpinfil);
                     }
                     fputs(linebuf, listfil);
                     fputs(linebuf, tmpinfil);
                  }
                }
		goto reset;
	}
	| file error NL {
		if (listopt) 
		   newlinelst();
		lineno++;
		yyerrok;			/* reset yyerror flag	*/
	reset:					/* reset arg list, art	*/
		ap = arglist; argcnt=0;		/* count, expression 	*/
		xp = explist;			/* list and begining of */
		begdotp = ccsect->dot;		/* instruction dot	*/
		csect_label = 0;		/* reset label flag     */
                mach_push = -1;
                mach_pop = -1;
                cur_reflist = 0;
	}
	;
 
labels:		/* empty */ {
                if (prev_label) {
			prev_label = 0;         /* reset label flag     */
			$$ = label_ptr;         /* chaining previous lab*/
			}
                else
			$$ = 0;
		}
	| labels NAME COLON {
		/* check for doubly defined symbols	*/
                struct symtab *p;
		if (passno==1) {
			/* static and external labels are in symbol table*/
			/* but don't redefine common (XTY_CM) variable   */
			if ((($2->sclass == C_EXT) 
				|| ($2->sclass == C_HIDEXT)) 
			        && $2->etype != XTY_CM) {

                                if ($2->etype == XTY_LD)
                                         /* modified message 001 */
                                yyerror( 1, $2->name );

			    	$2->etype = XTY_LD;
				if(!($2->numaux))	
				$2->numaux = 1;
			}
			else
                   	if ($2->type != E_UNK)
                                         /* modified message 001 */
		       		yyerror( 1, $2->name );

		/* set values of labels, see comment above		*/
		   for (p=$2; (p); p = p->nxtesd) {
		      p->csect  = ccsect;
	              p->type = E_REL;
					/*label contained in the unname*/
					/*csect                        */
		      if (ccsect==sect) 
			 ccsect->rent++;
		      if ((p->value = begdotp) & BRALIGN) 
                                                /* message 108 */
			 yywarn( 108,  p->name);
		      }
		    /* chain label names together for later		*/
		    /* Do this so that labels will get the addr of the	*/
		    /* instruction they are on as opposed to the addr	*/
		    /* of the end of the previous instruction. This is 	*/
		    /* important if there are several labels on a csect	*/
		    /* statement.  We want the semantic action of ICSECT*/
		    /* to be performed before assigning the labels values*/
		    $2->nxtesd = $1;
		    $$ = $2; 
		    }
	}
	;
 
linstruction:	labels instruction {
		/* set values of labels, see comment above		*/
                struct symtab *p;
		if (passno==1) {
		   for (p=$1; (p); p = p->nxtesd) {
		      if (intoc){		/* labels on toc entries*/	
		      p->csect  = ccsect;
		      if ((p->value = begdotp) & BRALIGN) 
                                                /* message 108 */
			 yywarn( 108,  p->name);
	                 p->type = E_TREL;	/* are toc relative	*/
			}
		      if (csect_label)		/* labels on csect entries*/	
			 yywarn1( 58,  p->name, ccsect->secname->name);
		      if (xflag == 1)		/* xref definition	*/
		         print_lineno(p,DEF,lineno);
		      }
                   if (prev_label)
			label_ptr = $1;
		   }
		/* in pass2, may want to make sure label values are	*/
		/* the same as in pass 1				*/
		}
		;
 
instruction:
	ISET NAME CM expr {
                in_comm = 0;
		if ($2->etype==XTY_ER) {
			$2->etype=XTY_LD;
		      	if (ccsect==sect) 
				ccsect->rent++;
		}
		else
			if ((passno==1) && ($2->type != E_UNK))
                                                /* modified message 001 */
				yyerror( 1, $2->name);
		$2->type = $4->xtype;
		/* for TOCOF symbols, the value is a pointer to the 	*/
		/* tocof entry in the symbol table			*/
		if ($2->type==E_TOCOF)
			$2->value = (long)($4->xname);
		else $2->value = $4->xvalue;
                if ( $4->xtype != E_ABS ) { /* save the RLD info   */
                  $2->r_info_arr[0].r_rrtype = $4->x_rrtype;
                  $2->r_info_arr[0].r_name = $4->xname;
                  if ( $4->rldinfo.rrtype != RLD_NOT_SET ) {
                     $2->r_info_arr[1].r_rrtype =$4->rldinfo.rrtype;
                     $2->r_info_arr[1].r_name = $4->rldinfo.rname;
                  }
                }

		if (passno==1) {
			$2->csect = $4->xloc;	/* assign csect associate*/
						/* with SET expression	*/
		   	if (xflag==1)		/* xref define		*/
		      	     print_lineno($2, DEF, lineno);
		}
		if ($4->xtype==E_UNK)
                                                /* message 003 */
		   yyerror( 3 );

		/* if listing option specified, then display value of 	*/
		/* SET expression					*/
		if (listopt) 
	 		displayval($2->value);
	}
	| IGLOBAL NAME {
                in_comm = 0;
		/* make sure the name is valid for a global */
		if ( passno==1 && $2->type==E_UNK ) {
				$2->type = E_EXT;
		            	$2->etype = XTY_ER;
		            	$2->numaux = 1;
                            }
		if ( ($2->type == E_REL && $2->csect->ssclass != C_DSECT) || 
			($2->type==E_EXT) ) {
			    if ( $2->etype==XTY_US ) {
				$2->etype = XTY_LD;
				if(!($2->numaux))	
				$2->numaux = 1;
				}
                            $2->sclass = C_EXT; /*set global external class*/
                            }
                                                /* message 004 */
		else yyerror( 4 );
		if (passno==1 && xflag==1)	/* xref symbol reference */
		     print_lineno($2, REF, lineno);
	}
	| IGLOBAL QNAME {
                in_comm = 0;
		/* make sure the name is valid for a global */
                if (passno==2)
		 if ( ($2->type == E_REL && $2->csect->ssclass == C_DSECT) || 
                         ($2->type == E_EXT) )
                            $2->sclass = C_EXT; /*set global external class*/
                                                /* message 004 */
		else yyerror( 4 );
                else {
			if ( $2->type==E_UNK ) {
				$2->type = E_EXT;
		            	$2->etype = XTY_ER;
		            	$2->numaux = 1;
                       	     }
			if (xflag==1)	/* xref symbol reference */
		     		print_lineno($2, REF, lineno);
			}
	}
	| ILGLOBAL NAME {
                in_comm = 0;
		/* make sure the name is valid for a global */
		if ( passno==1 && $2->type==E_UNK ) {
				$2->type = E_EXT;
		            	$2->etype = XTY_ER;
		            	$2->numaux = 1;
                            }
		if ( ($2->type == E_REL && $2->csect->ssclass != C_DSECT) || 
			($2->type==E_EXT) ) {
			    if ( $2->etype==XTY_US ) {
				$2->etype = XTY_LD;
			        if(!($2->numaux))	
				$2->numaux = 1;
				}
                            $2->sclass = C_HIDEXT; /*set global external class*/
                            }
                                                /* message 004 */
		else yyerror( 4 );
		if (passno==1 && xflag==1)	/* xref symbol reference */
		     print_lineno($2, REF, lineno);
	}
	| ILGLOBAL QNAME {
                in_comm = 0;
		/* make sure the name is valid for a global */
                if (passno==2)
		 if ( ($2->type == E_REL && $2->csect->ssclass == C_DSECT) || 
                         ($2->type == E_EXT) )
                            $2->sclass = C_HIDEXT; /*set global external class*/
                                                /* message 004 */
		else yyerror( 4 );
                else {
			if ( $2->type==E_UNK ) {
				$2->type = E_EXT;
		            	$2->etype = XTY_ER;
		            	$2->numaux = 1;
                       	     }
			if (xflag==1)	/* xref symbol reference */
		     		print_lineno($2, REF, lineno);
			}
	}
	| ICSECT ERRTOK  args {
	  /* ERRTOK indicates an error, and $1 has the integer error code */
		switch ((int) $2) {
			case INV_STG_CLASS : 
                                                /* message 005 */
					yyerror( 5 );
					break;
                                                /* message 006 */
			default : yyerror( 6 );
			}
	} 
	| ICSECT {			/* unnamed csect		*/
	   jcsect(XMC_PR,0,2);
	   begdotp = ccsect->dot;
	   intoc = 0;
	   csect_label++;
	}
	| ICSECT QNAME {		/* named csect			*/
           if (!intoc && $2->eclass == XMC_TD)
                                                /* modified message 183 */
              yyerror( 183 );
           else if ( !intoc && $2->eclass == XMC_TC ) {
                   /* internally turn on the TOC scope  */
                 intoc = 1;
                 if ( passno==1)
                    tocanchor();            /* go set the TOC anchor        */
                 else
                    if (tocname->csect->end)
                                                /* message 061 */
                       yyerror( 61 );
           }
	   jcsect($2->eclass,$2,2);
	   begdotp = ccsect->dot;
           if ( $2->eclass != XMC_TC && $2->eclass != XMC_TD )
	     intoc = 0;
	   csect_label++;
	   if (passno==1 && xflag==1)	/* xref symbol definition	*/
	        print_lineno($2, DEF, lineno);
	}
	| ICSECT QNAME CM expr {	/* named csect with aligment	*/
	   if ($4->xtype!=E_ABS) 
                                                /* modified message 007 */
	      yyerror( 7);
           if (!intoc && $2->eclass == XMC_TD)
                                                /* modified message 018 */
              yyerror( 183 );
           else if ( !intoc && $2->eclass == XMC_TC ) {
                 intoc = 1;
                 if ( passno==1)
                    tocanchor();            /* go set the TOC anchor        */
                 else
                    if (tocname->csect->end)
                                                /* message 061 */
                       yyerror( 61 );
           }
	   jcsect($2->eclass,$2,$4->xvalue);
	   begdotp = ccsect->dot;
           if ( $2->eclass != XMC_TC && $2->eclass != XMC_TD )
	      intoc = 0;
	   csect_label++;
	   if (passno==1 && xflag==1)	/* xref symbol definition	*/
	        print_lineno($2, DEF, lineno);
	}
	| ICSECT CM expr {		/* unnamed csect with alignment	*/
	   if ($3->xtype!=E_ABS) 
                                                /* modified message 007 */
	      yyerror( 7);
	   jcsect(XMC_PR,0,$3->xvalue);
	   begdotp = ccsect->dot;
	   intoc = 0;
	   csect_label++;
	}
	| ITOC {			/* .toc				*/
           in_comm = 0;
           intoc = 1;			/* set in the toc flag		*/
	   if (passno==1) 
		tocanchor();		/* go set the TOC anchor	*/
	   else
	      if (tocname->csect->end)
                                                /* message 061 */
		yyerror( 61 );
            begdotp = ccsect->dot;
        }
	| tcx setlong explist 		/* part of .tc			*/
	| ITOCOF NAME CM symbol {	
           in_comm = 0;
	   if (passno==1) {
	      if ($2->type != E_UNK) 
                                                /* message 008 */
	         yyerror( 8 );
	      else if (($4->type != E_EXT) && ($4->type != E_UNK)) 
                                                /* message 010 */
		 yyerror( 10 );
	      $2->type = E_TOCOF;
	      $2->value = (long)$4;
	      if (xflag==1)		/* xref symbol definition	*/
	        print_lineno($2, DEF, lineno);
	      }
	   else if ($4->type != E_EXT)
                                                /* message 010 */
		 yyerror( 10 );
	}
	| IBYTE setchar explist { in_comm = 0;  }
	| IBYTE setchar STRING {
                in_comm = 0;
                jbyte($3,0);
        }
	| ISTRING setchar STRING {
                in_comm = 0;
                jbyte($3,1);
        }
	| ILONG  setlong explist { in_comm = 0;  }
	| ISHORT setword explist { in_comm = 0;  }
	| IVBYTE setv CM explist  { in_comm = 0;  }
	| ISPACE expr {
                in_comm = 0;
		if ($2->xtype == E_UNK && passno==1)
                                                /* message 011 */
			yyerror( 11 );
		else if ($2->xtype != E_ABS)
                                                /* message 012 */
			yyerror( 12 );
		if ($2->xvalue <0)
                                                /* message 013 */
			yyerror( 13 );
		else {
			li = $2->xvalue;
			while (li--) 
				outb(0);
		}
		
	}
	| IORG expr {
                in_comm = 0;
		jorg($2);
	}
        | ICOMM QNAME CM expr {
            if ( $2->eclass == XMC_TD && !intoc ) {
               intoc = 1;
               if ( passno==1) {
                  ccsect_save = ccsect;  /* since a .toc is issued internally*/
                                      /* so save the current ccsect pointer */
                  tocanchor();            /* go set the TOC anchor        */
                  ccsect = ccsect_save;   /* resotre the current ccsect   */
                                     /* so all the statements after .comm */
                               /* will associate with the restored ccsect */
               } else
                  if (tocname->csect->end)
                     yyerror( 61 );             /* message 061 */ 
            }
            jcomm($2,$4,$2->eclass,0,2);
            intoc = 0;
            if (passno==1 && xflag==1)
               print_lineno($2, DEF, lineno);
        }
        | ICOMM QNAME CM expr CM expr {
            if ( $2->eclass == XMC_TD && !intoc ) {
               intoc = 1;
               if ( passno==1) {
                  ccsect_save = ccsect;  /* since a .toc is issued internally*/
                                      /* so save the current ccsect pointer */
                  tocanchor();            /* go set the TOC anchor        */
                  ccsect = ccsect_save;   /* resotre the current ccsect   */
                                     /* so all the statements after .comm */
                               /* will associate with the restored ccsect */
               } else
                  if (tocname->csect->end)
                                                /* message 061 */
                     yyerror( 61 );
            }
            jcomm($2,$4,$2->eclass,0,$6->xvalue);
            intoc = 0;
            if (passno==1 && xflag==1)
               print_lineno($2, DEF, lineno);
        }
	| ICOMM NAME CM expr {
	   jcomm($2,$4,XMC_RW,0,2);
	   if (passno==1 && xflag==1)
	        print_lineno($2, DEF, lineno);
	}
	| ICOMM NAME CM expr CM expr {
	   jcomm($2,$4,XMC_RW,0,$6->xvalue);
	   if (passno==1 && xflag==1)
	        print_lineno($2, DEF, lineno);
	}
	| ILCOMM NAME CM expr {
	   jlcomm($2,$4,XMC_BS,0,2);
           intoc = 0;
	   if (passno==1 && xflag==1)
	        print_lineno($2, DEF, lineno);
	}
	| ILCOMM NAME CM expr CM NAME {
	   jlcomm($2,$4,XMC_BS,$6,2);
           intoc = 0;
	   if (passno==1 && xflag==1) {
	        print_lineno($2, DEF, lineno);
	        print_lineno($6, REF, lineno);
		}
	}
	| IALIGN expr {
                in_comm = 0;
		jalign($2);
	}
	| IUSING arg CM expr {
                in_comm = 0;
                jusing(ap, $4);
	}
	| IDROP expr { 
                in_comm = 0;
                jdrop($2);
	}
	| IEXTERN symbol {
                in_comm = 0; 
                if (passno==1) {
                	if ($2->type==E_UNK) {
	     	    		$2->type = E_EXT;
		    		$2->etype = XTY_ER;
                	}
			else if($2->type==E_REL)
		    		$2->etype = XTY_LD;
			if(!($2->numaux))	
		    	$2->numaux = 1;
		}
                if (($2->type != E_REL || $2->csect->ssclass == C_DSECT) &&
                        ($2->type!=E_EXT) )
                     yyerror (172);
                $2->sclass = C_EXT;       /*set global external class*/
                                          /*place symbol at end of table*/
	}
	| INST args {
                in_comm = 0;
		if(passno == 1)
			ins1out($1, arglist, argcnt);
		else
			ins2out($1, arglist,argcnt);
	}
	| floating flist
	| IDSECT NAME {
	   jcsect(C_DSECT,$2,2);
	   begdotp = ccsect->dot;
	   intoc = 0;
	   csect_label++;
	   if (passno==1 && xflag==1)
	        print_lineno($2, DEF, lineno);
	}
	| ITBTAG setlong outinstr setchar outexpr CM outexpr CM 
                 outexpr CM outexpr CM outexpr CM outexpr 
		 CM outexpr CM outexpr CM
		 setlong outexpr CM outexpr CM outexpr
		 CM outexpr CM outexpr CM 
 		 setword outexpr CM setchar outexpr CM outexpr {
                 in_comm = 0;
	}
	| ITBTAG setlong outinstr setchar outexpr CM outexpr CM 
                 outexpr CM outexpr CM outexpr CM outexpr 
		 CM outexpr CM outexpr 
		 setlong outnothing outnothing outnothing 
                 outnothing outnothing outnothing {
                 in_comm = 0;
	}
	| IRENAME symbol CM STRING {
           in_comm = 0;
	   if (passno==1)
	      $2->rename = $4;
	   else if ($2->etype == XTY_US)
                                                /* message 014 */
	      yyerror( 14 );
	}
	| IHASH symbol CM STRING {
                in_comm = 0;
		jhash($2,$4);
	}
	| IFILE STRING {
                in_comm = 0;
		if (passno==1)
			jfile($2);
        }
        | IMACHINE STRING {
          /*      if (passno == 1)    */
                in_comm = 0;
                jmachine($2);
        }
        | IMACHINE {
                in_comm = 0;
                jmachine("");
        }
        | ISOURCE STRING {
                in_comm = 0;
                if (passno == 1)
                   jsource($2);
	}
	| ISTABX STRING {
		if (passno==1)
		jstabx($2,0,C_DECL,0);
	}
	| ISTABX STRING CM expr CM expr CM expr {
		if (passno==1)
		jstabx($2,$4->xvalue,$6->xvalue,$8->xvalue);
	}
	| IFUNCTION QNAME CM expr CM expr CM expr CM expr {
		jfunction($2,$4,$6,$8,$10);
	}
	| IFUNCTION NAME CM expr CM expr CM expr CM expr {
		jfunction($2,$4,$6,$8,$10);
	}
	| IFUNCTION QNAME CM expr CM expr CM expr {
		jfunction($2,$4,$6,$8,0);
	}
	| IFUNCTION NAME CM expr CM expr CM expr {
		jfunction($2,$4,$6,$8,0);
	}
	| IFUNCTION QNAME CM expr CM expr CM expr PCENT NAME {
		jfunction($2,$4,$6,$8,0);
	}
	| IFUNCTION NAME CM expr CM expr CM expr PCENT NAME {
		jfunction($2,$4,$6,$8,0);
	}
	| ILINE expr {
                in_comm = 0;
		if($2->xtype==E_UNK)
                                                /* message 015 */
			yyerror( 15 );
		jline((short)($2->xvalue));
	}
	| IXLINE expr CM STRING {
                in_comm = 0;
		if($2->xtype==E_UNK)
                                                /* message 015 */
			yyerror( 15 );
	      jxline($2,$4,0);
	}
	| IXLINE expr CM STRING CM expr {
                in_comm = 0;
		if($6->xvalue!=C_BINCL&&$6->xvalue!=C_EINCL)
                                                /* message 015 */
			yyerror( 15 );
	      jxline($2,$4,$6->xvalue);
	}
	| IBF  expr {
			if ($2->xtype==E_UNK)
                                                /* message 015 */
				yyerror( 15 );
                        in_comm = 0;
			jbf((int)($2->xvalue));
	}
	| IEF  expr {
			if ($2->xtype==E_UNK)
                                                /* message 015 */
				yyerror( 15 );
			jef((int)($2->xvalue));
	}
	| IBB expr {
			if ($2->xtype==E_UNK)
                                                /* message 015 */
				yyerror( 15 );
                        in_comm = 0;
			jbb((int)($2->xvalue));
	}
	| IEB expr {
			if ($2->xtype==E_UNK)
                                                /* message 015 */
				yyerror( 15 );
			jeb((int)($2->xvalue));
	}
	| IBS symbol {
			jbs($2);
	}
	| IES {
	 		jes();
	}
	| IBI STRING {
                in_comm = 0;
                if (passno == 1)
			jbi($2);
	}
	| IEI STRING {
                if (passno == 1)
	 		jei($2);
	}
	| IBC STRING {
                if (passno == 1)
			jbc($2);
	}
	| IEC {
                if (passno == 1)
	 		jec();
	}
	| ICOPT ignorelist 
	| IXADDR expr {
	}
	| IREF
             {
                in_comm = 0;
                if ( ccsect->ssclass == C_DSECT )
                   yyerror1(176, ccsect->secname->name);
             }
             namelist
	| ERRTOK  args {
	  /* ERRTOK indicates an error, and $1 has the integer error code */
		switch ((int) $1) {
			case INV_OP : 
                                                /* message 016 */
					yyerror( 16 );
					break;
                                                /* message 017 */
			default : yyerror( 17 );
			}
	} 
	| /* empty */ {
		if( intoc )
		prev_label = 1;       /* set label flag       */
      		}
	;
tcx:  ITC QNAME CM {
	   if (!intoc)
                                                /* modified message 018 */
	      yyerror( 18 );
	   else {
	   	jtc($2);
	   	begdotp = ccsect->dot;
	   	if (passno==1 && xflag==1)
	        	print_lineno($2, DEF, lineno);
	   	}
	}
	| ITC CM {
	   if (!intoc)
                                                /* modified message 018 */
	      yyerror( 18 );
	   else {
	   	jtc(0);
	   	begdotp = ccsect->dot;
	   	}
	}
	;
explist: {       /* empty */
             yywarn(185);
        }
	| explist CM outexpr
	| outexpr
	;
 
outexpr: expr_ref {
		/* if .short or .byte of externally defined symbol yyerror */
		if (passno==2 && curlen != 4 && $1->xtype==E_EXT) 
			if(curlen != 2)
                                                /* message 019 */
				yyerror( 19 );
			else 
                                                /* message 020 */
				yyerror( 20 );
		outexpr(curlen, $1);
                xp = explist;
                if ( passno == 2)
		  gen_ref();	
		cur_symlist = 0;
	}
 
floating: setlong IFLOAT {
                in_comm = 0;
		curlen = 4;
		fflag = 1;
	}
	| setlong IDOUBLE {
                in_comm = 0;
		curlen = 8;
		fflag = 1;
	}
 
flist:	/* empty */
	| flist CM fltout
	| fltout
	;
 
fltout: FLTNUM {
		putflt($1, curlen);
		fflag = 0;
	}
	;
 
setv:  expr {
               if ($1->xtype != E_ABS )
                                                /* message 021 */
		      yyerror( 21 );
		else if ($1->xvalue<1 || $1->xvalue >4)
                                                /* message 022 */
			yyerror( 22 );
		else curlen = $1->xvalue;
		}
setchar: {
		curlen = 1;
	}

setword: {
		long tmp_dot;
		
		/* hold the value of dot to compare after align		*/
		tmp_dot = ccsect->dot;
		curlen = 2;
		doalign(2);	/* force to halfword boundary */

		/* if doalign actually changed the alignment		*/
		/* print warning.. NOTE: doalign prints a warning also	*/
		if(tmp_dot != ccsect->dot) 
                                                /* message 027 */
			yywarn( 27 );
	}
 
 
setlong: {
		long tmp_dot;
		
		/* hold the value of dot to compare after align		*/
		tmp_dot = ccsect->dot;
		curlen = 4;
		doalign(4);	/* force to fullword boundary		*/

		/* if doalign actually changed the alignment		*/
		/* print warning.. NOTE: doalign prints a warning also	*/
		if(tmp_dot != ccsect->dot) 
                                                /* message 109 */
			yywarn( 109 );
	}
 
namelist:  namelist CM outname
	|  outname
	;
outname:  symbol {
                if ( passno == 2 ) {
                   if ( $1->type == E_UNK )
                     yyerror1(23, $1->name);
                   else
                     jref($1);
                }
	} 
 	;

args:	arg {
		++ap; ++argcnt;
	}
	| args CM arg = {
			++ap; ++argcnt;
	}
	|	/* no args */
	;

arg:	expr {
		ap->atype = AEXP;
		ap->xp = $1;
		ap->areg = 0;
	}
	| expr LP expr RP {
		ap->atype = ADISP;
		ap->xp = $1;
		ap->areg = getreg($3);
	}
	| LITOP expr {		/* vestigal */
		ap->atype = APRF;
		ap->areg = 0;
		ap->xp = $2;
	}
	;
expr_ref: exprl {
            for ( i = 0; i<st_inx; i++)
              put_reflist(i, &cur_reflist, reflist);
            exp_result($1, reflist, cur_reflist);
            $$ = $1;
            cur_reflist = 0;
            st_inx = 0;
	}
	;
expr:   exprl {
            for ( i = 0; i<st_inx; i++)
              put_reflist(i, &cur_reflist, reflist);
            exp_result($1, reflist, cur_reflist);
            $$ = $1;
            cur_reflist = 0;
            st_inx = 0;
            reset_symlist();
        }
        ;
 
exprl:  symbol {
                init_explist_elm(xp);
		$$ = xp++;
                if (( xp - explist ) > MAX_REFLIST )
                {
                   yyerror (178 );
                   delexit();
                } 
		$$->xtype = $1->type;
		$$->xname = $1;
		switch ($1->type) {
		   case E_UNK:
			if (passno==2)
                                                /* message 023 */
			   yyerror( 23, $1->name); 
			$$->xvalue = 0;
			break;
		   case E_TOCOF:
			$$->xvalue = 0;
			$$->xname = (struct symtab *)($1->value);
			$$->xloc = ccsect;
			break;
		   default:
			$$->xvalue = $1->value;
			$$->xloc = $1->csect;
			}

                        /* If this symbol represents a exp. by .set    */
                  if ( $1->r_info_arr[0].r_name != NULL ) {
                    $$->xname = $1->r_info_arr[0].r_name;
                    $$->x_rrtype = $1->r_info_arr[0].r_rrtype;
                    if ( $1->r_info_arr[1].r_rrtype != RLD_NOT_SET ) {
                       $$->rldinfo.rrtype = $1->r_info_arr[1].r_rrtype ;
                       $$->rldinfo.rname = $1->r_info_arr[1].r_name ;
                    }
                 }
	}
	| LOC {
                init_explist_elm(xp);
		$$ = xp++;
                if (( xp - explist ) > MAX_REFLIST )
                {
                   yyerror (178 );
                   delexit();
                }
		$$->xtype = E_REL;
		$$->xvalue = ccsect->dot;
		$$->xloc = ccsect;
		$$->xname = NULL;
	}
	| INT {
                init_explist_elm(xp);
		$$ = xp++;
                if (( xp - explist ) > MAX_REFLIST )
                {
                   yyerror (178 );
                   delexit();
                }
		$$->xtype = E_ABS;
		$$->xvalue = *$1;
		$$->xloc = 0;
		$$->xname = NULL;
                $$->st_inx = -1;
                
	}
	| FLTNUM {
                init_explist_elm(xp);
		$$ = xp++;
                if (( xp - explist ) > MAX_REFLIST )
                {
                   yyerror (178 );
                   delexit();
                }
		$$->xtype = E_ABS;
		$$->dvalue = *$1;
		$$->xloc = 0;
		$$->xname = NULL;
                $$->st_inx = -1;
	}
	| LP exprl RP {
                $$ = $2;
	}
	| exprl PLUS exprl {
                $$ = combine($2, $1, $3);
	}
	| exprl MINUS exprl {
                          if ( $3->xtype == E_TMP)
                             convert($3);
                          $$ = combine($2, $1, $3);
	}
	| exprl TILDE
                   {
                     if ( $1->xtype == E_TMP )
                         get_type($1);
                   }
                     exprl {
                     if ( $4->xtype == E_TMP )
                        get_type($4);
		     goto comb;
	}
	| exprl IOR 
                   {
                     if ( $1->xtype == E_TMP )
                         get_type($1);
                   }
                     exprl {
                     if ( $4->xtype == E_TMP )
                        get_type($4);
		     goto comb;
	}
	| exprl XOR 
                   {
                     if ( $1->xtype == E_TMP )
                         get_type($1);
                   }
                     exprl {
                     if ( $4->xtype == E_TMP )
                        get_type($4);
		     goto comb;
	}
	| exprl AND 
                   {
                     if ( $1->xtype == E_TMP )
                         get_type($1);
                   }
                     exprl {
                     if ( $4->xtype == E_TMP )
                        get_type($4);
		     goto comb;
	}
	| exprl LSH 
                   {
                     if ( $1->xtype == E_TMP )
                         get_type($1);
                   }
                     exprl {
                     if ( $4->xtype == E_TMP )
                        get_type($4);
		     goto comb;
	}
	| exprl RSH  
                   {
                     if ( $1->xtype == E_TMP )
                         get_type($1);
                   }
                     exprl {
                     if ( $4->xtype == E_TMP )
                        get_type($4);
		     goto comb;
	}
	| exprl MUL 
                   { 
                     if ( $1->xtype == E_TMP ) 
                         get_type($1);
                   }
                    exprl {
                     if ( $4->xtype == E_TMP ) 
                        get_type($4);
		     goto comb;
	}
	| exprl DIV 
                   {
                     if ( $1->xtype == E_TMP )
                         get_type($1);
                   }
                    exprl {
                     if ( $4->xtype == E_TMP )
                        get_type($4);
	comb:
	        	$$ = combine($2, $1, $4);
	}
	| nothing MINUS exprl {
                          if ($3->xtype == E_TMP )
                            convert($3);
                          $$ = combine($2, $1, $3);
        }
	| nothing TILDE exprl {
                     if ( $3->xtype == E_TMP )
                        get_type($3);
                        $$ = combine($2, $1, $3);
	}
	| nothing PLUS exprl {
                $$ = combine($2, $1, $3);
	}
	;
 
symbol: NAME {
	if( passno==1 && xflag==1)
	        print_lineno($1, REF, lineno);
	$$ = $1;
	}
	| QNAME {
	if( passno==1 && xflag==1)
	        print_lineno($1, REF, lineno);
	$$ = $1;
	}
	;

outinstr:   badinstr {
			outexpr(curlen, $1);
	}
	;
badinstr: {
                init_explist_elm(xp);
		$$ = xp++;
                if (( xp - explist ) > MAX_REFLIST )
                {
                   yyerror (178 );
                   delexit();
                }
		$$->xtype = E_ABS;
		$$->xloc = 0;
		$$->xvalue = BADINSTR;
	}
	;
outnothing:   nothing {
			outexpr(curlen, $1);
	}
	;
nothing: {
                init_explist_elm(xp);
		$$ = xp++;
                if (( xp - explist ) > MAX_REFLIST )
                {
                   yyerror (178 );
                   delexit();
                }
		$$->xtype = E_ABS;
		$$->xloc = 0;
		$$->xvalue = 0;
                $$->st_inx = -1;
	}
	;

ignorelist: 
	| ignorelist CM ignore
	| ignore
	;
ignore: NAME {
	}
	| INT {
	}	
	| ignore MINUS ignore {
	}	
	| ignore PLUS ignore {
	}	
	;
%%
char g_errbuf[30*ERLINES];
char *g_errbuf_ptr=g_errbuf;
char errbuf[30*ERLINES];
char *errbuf_ptr=errbuf;
int line_error;
char *cur_err=errbuf; 
extern  int num_msgs;


/****************************************************************/
/* Function:	yyerror						*/
/* Purpose:	output error messages routine			*/
/* Input:	index - index into message table                */
/*		a - optional data paramater to output 		*/
/* 		   like printf(s,a)				*/
/* Output:	writes to stderr the error message		*/
/****************************************************************/
yyerror(index,a)
int index;
{

char msgbuf[BUFLEN];
char *MSGbuf=msgbuf;

	if (anyerrs==0 && anywarns==0 )
			/* header to stderr indicating Asm program */
		fprintf(stderr, MESSAGE( 117 ));
	anyerrs++;
                   /*use message services to retrieve error message*/      
        if (index >= 1 && index <= num_msgs)
		MSGSTR(index);
                    /*compensate for yacc generated error messages*/
                else  
		MSGSTR(DEFMSG);   /*  generate default message (142) */
                                 /*generate message in the listing*/
	error(MSGbuf, a);
                                    /*output message to the screen*/
	fputs(cur_err, stderr);
	if (listopt) 
		line_error++;
	else
		errbuf_ptr=errbuf;
	yyerrok;
}

/****************************************************************/
/* Function:	yywarn						*/
/* Purpose:	the parallel to yyerror for warning messages	*/
/* Input:	index - index into the message table            */
/*		a - the data part of the warning message	*/
/* Output:	writes the message to standard error if the 	*/
/*		warning flag was on (-w) 			*/
/****************************************************************/
yywarn(index, a)
int index;
{

char msgbuf[BUFLEN];
char *MSGbuf=msgbuf;

	if (warning==WFLAG || warning==NO_WFLAG) return;
	if (anyerrs==0 && anywarns==0) {
			/* header to stderr indicating Asm program */
           if ( (index != 149 && index != 185) || passno != 2 ) 
                        /* warning 149 or 185 on the screen only at Pass 1   */
		fprintf(stderr, MESSAGE( 117 ));
        }
      	anywarns++;
		
        if (index >= 1 && index <= num_msgs)
		MSGSTR(index);
                    /*compensate for yacc generated error messages */
                    /*produce a generalized "errorinsyntax" message*/
                else  
		MSGSTR(DEFMSG);
                                 /*generate message in the listing*/
	error(MSGbuf, a);
                                    /*output message to the screen*/
        if ( (index != 149 && index != 185) || passno != 2 ) 
                    /* warning 149 or 185 on the screen only at Pass 1   */
	   fputs(cur_err, stderr);
	if (listopt) 
		line_error++;
	else
		errbuf_ptr=errbuf;
}

/****************************************************************/
/* Function:	error                   			*/
/* Purpose:	writes error message to errbufi			*/
/* Input:	s- error string					*/
/*		a - the data part of the message		*/
/* Output:	x						*/
/* Returns:	x						*/
/* Comments:	x						*/
/****************************************************************/
error(s, a)
	char *s;
{

	cur_err = errbuf_ptr;
	if (infile)
	buf_write("%s: ", infile, &errbuf_ptr);
	buf_write(MESSAGE( 118 ), lineno, &errbuf_ptr);
	buf_write(s, a, &errbuf_ptr);
}

/****************************************************************/
/* Function:	buf_write                  	 		*/
/* Purpose:	writes a string to a buffer			*/
/* Input:	s- error string					*/
/*		a - the data part of the string 		*/
/*		bufptr - buffer                      		*/
/* Output:	x						*/
/* Returns:	x						*/
/* Comments:	x						*/
/****************************************************************/
buf_write(s, a, bufptr) 
	char *s;
	char **bufptr;
{
	sprintf(*bufptr, s, a);
	*bufptr += strlen(*bufptr);
}	

/****************************************************************/
/* Function:	getreg						*/
/* Purpose:	get a register number from an expression 	*/
/* Input:	xp - the expression to get the reg number from	*/
/* Returns:	the register number				*/
/* Comments:	returns 0 in pass 1, because don't care til p2	*/
/****************************************************************/
getreg(xp)
register struct exp *xp;
{
	if(passno == 1)
		return 0;       /* assume all OK for now */

	/* check if the expression is right type & within range	*/
	if(xp->xtype != E_ABS || xp->xvalue < 0 || 
	   xp->xvalue > HIGHREG)
                                                /* message 025 */
		yyerror( 25 );
	return (int)xp->xvalue;
}

/****************************************************************/
/* Function:    getmsg                                          */
/* Purpose:     get message string from message catalog         */
/* Input:       index - index into message table                */
/* Output:      message string assigned to global error buffer  */
/****************************************************************/
getmsg(int index)
{

char msgbuf[BUFLEN];
char *MSGbuf=msgbuf;

                   /*use message services to retrieve error message*/
        if (index >= 1 && index <= num_msgs)
                MSGSTR(index);
                    /*compensate for yacc generated error messages*/
                else
                MSGSTR(DEFMSG);   /*  generate default message (142)  */
         g_errbuf_ptr = MSGbuf;
}

/****************************************************************/
/* Function:    yyerror1                                        */
/* Purpose:     output error messages routine                   */
/* Input:       g_msg - error buffer pointer containing the     */
/*                      message                                 */
/* Output:      writes to stderr and the listing file the error */
/*               message                                        */
/****************************************************************/
yyerror1(int index, ...)
{

va_list   ap;

        if (anyerrs==0 && anywarns== 0 ) {
                        /* header to stderr indicating Asm program */
            fprintf(stderr, MESSAGE( 117 ));
        }
        anyerrs++;
        getmsg(index);
                                 /*generate message in the listing*/
        va_start(ap, index);
        cur_err = errbuf_ptr;
        if (infile)
            buf_write("%s: ", infile, &errbuf_ptr);
        buf_write(MESSAGE( 118 ), lineno, &errbuf_ptr);
        vsprintf(errbuf_ptr, g_errbuf_ptr, ap);
        errbuf_ptr += strlen(errbuf_ptr);

                                    /*output message to the screen*/
        fputs(cur_err, stderr);
        if (listopt)
                line_error++;
        else
                errbuf_ptr=errbuf;
        yyerrok;
        va_end(ap);   /* clean up when done  */
}
           
/****************************************************************/
/* Function:    error5                                           */
/* Purpose:     This function is almost the same as routine      */
/*              error except without writing out the line number */
/* Input:       s- error string                                 */
/*              a - the data part of the message                */
/* Output:      x                                               */
/* Returns:     x                                               */
/* Comments:    x                                               */
/****************************************************************/
error5(char *s, void *a)
{

        cur_err = errbuf_ptr;
        buf_write(s, a, &errbuf_ptr);
}
/***************************************************************
*  Function:   init_explist_elm  
*  Purpose:    initialize the element of explist array
*  Input:      xp -  pointer to explist element
*  Output:     the element of explist pointed by xp is initialized
****************************************************************/
init_explist_elm(struct  exp  *xp)
{
                xp->xtype = E_UNK;
                xp->xloc = NULL;
                xp->xname = NULL;
                xp->x_rrtype = RLD_NOT_SET;
                xp->rldinfo.rrtype = RLD_NOT_SET;
                xp->rldinfo.rname = NULL;
                xp->dvalue = 0;
                xp->xvalue = 0;
		
}

/****************************************************************/
/* Function:    yywarn1                                         */
/* Purpose:     output  warning messages                        */
/* Input:       index - index into the message table            */
/* Output:      writes the message to standard error if the     */
/*              warning flag was on (-w)                        */
/****************************************************************/
yywarn1(int index, ...)
{

va_list   ap;

        if (warning==WFLAG || warning== NO_WFLAG) return;
        va_start(ap, index);
        if (anyerrs==0 && anywarns==0) {
                        /* header to stderr indicating Asm program */
                        /* warning 149 or 176 on the screen only at Pass 1 */
           if ( (index != 149 && index !=176) || passno != 2 ) 
                fprintf(stderr, MESSAGE( 117 ));
        }
        anywarns++;
        getmsg(index);
                                 /*generate message in the listing*/
        cur_err = errbuf_ptr;
        if (infile)
            buf_write("%s: ", infile, &errbuf_ptr);
        buf_write(MESSAGE( 118 ), lineno, &errbuf_ptr);
        vsprintf(errbuf_ptr, g_errbuf_ptr, ap);
        errbuf_ptr += strlen(errbuf_ptr);

                           /*output message to the screen*/
                           /* warning 149 or 176 on the screen only at Pass 1  */
        if ( (index != 149 && index !=176)  || passno != 2 ) 
           fputs(cur_err, stderr);
        if (listopt)
                line_error++;
        else
                errbuf_ptr=errbuf;
        va_end(ap);   /* clean up when done  */
}

/****************************************************************/
/* Function:    yywarn2                                         */
/* Purpose:     output  warning messages                        */
/* Input:       index - index into the message table            */
/*              ap - points to caller's unnamed arg list        */
/* Output:      writes the message to standard error if the     */
/*              warning flag  is NO_WFLAG                       */
/****************************************************************/
void
yywarn2(int index, va_list ap)
{

        if (!warning) return;
        if ( anyerrs==0&& anywarns== 0) {
                        /* header to stderr indicating Asm program */
           if ( index != 149 || passno != 2 ) /* warning 149 on the screen */
                                              /* only at Pass 1            */
                fprintf(stderr, MESSAGE( 117 ));
        }
        anywarns++;
        getmsg(index);
                                 /*generate message in the listing*/
        cur_err = errbuf_ptr;
        if (infile)
            buf_write("%s: ", infile, &errbuf_ptr);
        buf_write(MESSAGE( 118 ), lineno, &errbuf_ptr);
        vsprintf(errbuf_ptr, g_errbuf_ptr, ap);
        errbuf_ptr += strlen(errbuf_ptr);

                                    /*output message to the screen*/
        if ( index != 149 || passno != 2 ) /* warning 149 on the screen */
                                           /* only at Pass 1            */
           fputs(cur_err, stderr);
        if (listopt)
                line_error++;
        else
                errbuf_ptr=errbuf;
}
/****************************************************************/
/* Function:    yyerror2                                        */
/* Purpose:     output error messages routine                   */
/* Input:       index - index into message table                */
/*              ap - points to caller's unnamed arg list        */
/* Output:      writes to stderr the error message              */
/****************************************************************/
void
yyerror2(int index, va_list ap)
{

        if (anyerrs==0 && anywarns==0){
                        /* header to stderr indicating Asm program */
            fprintf(stderr, MESSAGE( 117 ));
        }
        anyerrs++;
        getmsg(index);

        cur_err = errbuf_ptr;
        if (infile)
            buf_write("%s: ", infile, &errbuf_ptr);
        buf_write(MESSAGE( 118 ), lineno, &errbuf_ptr);
        vsprintf(errbuf_ptr, g_errbuf_ptr, ap);
        errbuf_ptr += strlen(errbuf_ptr);

                                    /*output message to the screen*/
        fputs(cur_err, stderr);
        if (listopt)
                line_error++;
        else
                errbuf_ptr=errbuf;
        yyerrok;
}

/****************************************************
  Function: exp_result
  Purpose: get the final result of an expression according
           to the reflist
  Input:    fexp --  the pointer to the target expression
            reflist -- the array that holds the RLD information for
                       the target expression
            cur_reflist -- current index to reflist
  Output:
*******************************************************/
void
exp_result( struct exp * fexp,
            struct exp_ref_list reflist[],
            int cur_reflist)
{
   int inx_arr[2];
   int non_abs= 0;  /* number of non-absolute type entries */
                    /* in reflist                          */
   int sum_n, sum_n_t;
   struct exp exp_arr[2], *exp_ptr;

   if ( cur_reflist > 0 ) {
      exp_ptr = exp_arr;
      for ( i=0; i<cur_reflist; i++)
        if ( ( (reflist[i].n_rel + reflist[i].n_ext) == 0 
                /* label minus its containing csect name  */ 
                && reflist[i].n_trel == 0 ) || 
             ( ( reflist[i].n_trel + reflist[i].n_ext) == 0 )  
                /* TOC relative label  minus  its containing TD/TC csect name */
               && reflist[i].n_rel == 0  )
        { /* this entry has an absolute exp. type */
            if ( reflist[i].sec_ptr )
              reflist[i].sec_ptr->reflist_idx = -1;
            else  /* external symbol reference   */
              reflist[i].s_name->reflist_index = -1;
        } else {
          if ( non_abs < VALID_EXP ) {
            inx_arr[non_abs++] = i;
            if ( reflist[i].sec_ptr )
              reflist[i].sec_ptr->reflist_idx = -1;
            else
              reflist[i].s_name->reflist_index = -1;
          } else {
            fexp->xtype = ERR;
            yyerror(43);
            break;
          }
        }
      while ( i<cur_reflist ) {
        if ( reflist[i].sec_ptr )
          reflist[i].sec_ptr->reflist_idx = -1;
        else
          reflist[i].s_name->reflist_index = -1;
        i++;
      }

      if ( fexp->xtype != (char)ERR ) 
         if ( non_abs == 0) {
           fexp->xtype = E_ABS;
           fexp->x_rrtype == RLD_NOT_SET;
         } else { 
           for ( i=0; i< non_abs; i++) {
              if ( (sum_n = reflist[inx_arr[i]].n_rel +
                 reflist[inx_arr[i]].n_ext ) > 1 || sum_n < -1 ||
                   (sum_n_t = reflist[inx_arr[i]].n_trel + 
                    reflist[inx_arr[i]].n_ext ) > 1 || sum_n_t < -1 ) {
                   fexp->xtype = ERR;
                   yyerror(43);
                   break;
              } else {
                  if (sum_n_t == 1 ) {
                     if (reflist[inx_arr[i]].n_trel > 
                                reflist[inx_arr[i]].n_ext )
                         exp_ptr->xtype = E_TREL;
                     else
                         exp_ptr->xtype = E_EXT;
                     exp_ptr->x_rrtype = R_POS;
                  } else if ( sum_n_t == -1 ) {
                     if ( reflist[inx_arr[i]].n_trel > 
                                  reflist[inx_arr[i]].n_ext )
                         exp_ptr->xtype = E_EXT;
                     else
                         exp_ptr->xtype = E_TREL;
                     exp_ptr->x_rrtype = R_NEG;
#if 0
                      /* E_ABS - E_TREL is invalid */
                    fexp->xtype = ERR;
                    yyerror(43);
                    break;
#endif
                  } else if ( sum_n == 1) {
                     if ( reflist[inx_arr[i]].n_rel > 
                                 reflist[inx_arr[i]].n_ext )
                        exp_ptr->xtype = E_REL;
                     else  /* must be n_rel < n_ext */
                        exp_ptr->xtype = E_EXT;
                     exp_ptr->x_rrtype = R_POS;
                  } else if ( sum_n == -1) {
                     if ( reflist[inx_arr[i]].n_rel > 
                                 reflist[inx_arr[i]].n_ext )
                        exp_ptr->xtype = E_EXT;
                     else
                        exp_ptr->xtype = E_REL;
                     exp_ptr->x_rrtype = R_NEG;
                  }
                  exp_ptr->xloc = reflist[inx_arr[i]].sec_ptr;
                  exp_ptr->xname = reflist[inx_arr[i]].s_name;
              }
              exp_ptr++;
           }
           if ( fexp->xtype != (char)ERR ) 
             if ( non_abs == 1) {
               fexp->xtype = exp_arr[0].xtype;
               fexp->x_rrtype = exp_arr[0].x_rrtype;
               fexp->xloc = exp_arr[0].xloc;
               fexp->xname = exp_arr[0].xname;
               if ( fexp->xtype == E_REL )
                     /* the csect address had been included for */
                     /* calculation in combine routine. Now reset */
                     /* the value to be offset only              */
                  if ( fexp->x_rrtype == R_POS )
                     fexp->xvalue -= fexp->xloc->start;
                  else /* must be R_NEG */
                     fexp->xvalue += fexp->xloc->start;
               else if ( fexp->xtype == E_TREL )
                     /* the TOC anchor address had been included for */
                     /* calculation in combine routine. Now reset */
                     /* the value to be TOC relative offset only   */
                  if ( fexp->x_rrtype == R_POS )
                     fexp->xvalue -= tocname->csect->start;
                  else /* must be R_NEG */
                     fexp->xvalue += tocname->csect->start;
             } else  /* non_abs > 1 */
                if (( exp_arr[0].x_rrtype == R_POS &&
                        exp_arr[1].x_rrtype == R_POS ) || 
                       ( exp_arr[0].x_rrtype == R_NEG &&
                         exp_arr[1].x_rrtype == R_NEG ) ) {
                    fexp->xtype = ERR;
                    yyerror(43);
                } else {    
                    fexp->xtype = E_REXT;
                    fexp->x_rrtype = exp_arr[0].x_rrtype;
                    fexp->xloc = exp_arr[0].xloc;
                    fexp->xname = exp_arr[0].xname;
                    fexp->rldinfo.rrtype = exp_arr[1].x_rrtype;
                    fexp->rldinfo.rname = exp_arr[1].xname;
                }
         }
   }   
}
/****************************************************
  Function: gen_ref
  Description: generate R_REF RLDs from the symlist for
            an expression. Reset the rcount and symlist_idx
            fields for symtab entries pointed by the symlist.
  Input: none
  Output: RLDs are generated.
*****************************************************/
void
gen_ref(void)
{

    int i ;
   
    for ( i=0; i<cur_symlist; i++) {
       if ( symlist[i]->rcount == 0 ) 
           addrld(R_REF, 1, symlist[i]);
       symlist[i]->symlist_idx = -1;
       symlist[i]->rcount = 0;
    }
}
/************************************************************
  Function: convert
  Description: convert the sign of each entry in exp. stack
  Input:   r -- expression entry in the exp. stack
  output:  the exp. stack is updated
************************************************************/
void
convert(struct exp *r)
{
   int i;

       for (i=r->st_inx; i<st_inx; i++)
           if ( st_lp[i].sign == PLUS )
              st_lp[i].sign = MINUS;
           else
              st_lp[i].sign = PLUS;
}

/************************************************************
  Function:  init_reflist
  Purpose:   init the entry of reflist
  Input:     inx -- index to the reflist
             reflist -- array of the struct exp_ref_list to
                        hold RLD information for each section
                        or symbol
  Output:    the reflist entry is initialized
************************************************************/
void
init_reflist(int inx,
              struct exp_ref_list reflist[])
{
    reflist[inx].n_ext = 0;
    reflist[inx].n_rel = 0;
    reflist[inx].n_trel = 0;
    reflist[inx].sec_ptr = NULL;
    reflist[inx].s_name = NULL;
}
/************************************************************
  Function:  upd_ref_entry
  Purpose:   updated the ref. number in the reflist entry
  Input:     op -- operator
             inx -- index to the reflist
             x_type -- expression type 
             reflist -- array of the struct exp_ref_list to
                        hold RLD information for each section
                        or symbol
  Output:    the reflist entry is updated
************************************************************/
void
upd_ref_entry( int op,
               int inx,
               char x_type,
               struct exp_ref_list reflist[])
{
     switch (op) {
          case PLUS:
            if ( x_type == (char)E_REL )
               reflist[inx].n_rel++;
            else if (x_type == (char)E_EXT)
               reflist[inx].n_ext++;
            else if (x_type == (char)E_TREL )
               reflist[inx].n_trel++;
            else 
               yyerror(43);
             break;
          case MINUS:
            if ( x_type == (char)E_REL )
               reflist[inx].n_rel--;
            else if (x_type ==(char) E_EXT)
               reflist[inx].n_ext--;
            else if (x_type == (char)E_TREL )
               reflist[inx].n_trel--;
            else 
               yyerror(43);
             break;
         }
}

/************************************************************
  Function:  add_reflist
  Purpose:   Add the RLD information of a term into the array
             of the struct exp_ref_list
  Input:     op --  PLUS or MINUS
             rexp -- pointer to an expression term
             cur_reflist -- current index to array reflist
             reflist -- array of the struct exp_ref_list to
                        hold RLD information for each section
                        or symbol
  Output:    the reflist is updated
************************************************************/
void
add_reflist(int op, 
            struct exp *rexp,
            int *cur_reflist,
            struct exp_ref_list reflist[])
{
   int i;
    
    if ( (rexp->xloc == NULL && rexp->xname->reflist_index == -1 ) 
               /* This external symbol is never in the reflist */  
         || ( rexp->xloc != NULL && rexp->xloc->reflist_idx == -1 ) ) { 
               /* this section is never in the reflist         */  
      if ( *cur_reflist < MAX_REFLIST ){
        if ( rexp->xloc != NULL )
           rexp->xloc->reflist_idx = *cur_reflist;  /* update the reflist_idx in SECT */
        else
           rexp->xname->reflist_index = *cur_reflist;
        init_reflist(*cur_reflist, reflist);
        reflist[*cur_reflist].s_name = rexp->xname;
        reflist[*cur_reflist].sec_ptr = rexp->xloc;
        upd_ref_entry(op, *cur_reflist, rexp->xtype, reflist);
        (*cur_reflist)++;
      } else {
        yyerror(178);
        delexit();
      }
    }
    else 
      if ( rexp->xloc != NULL )
         upd_ref_entry(op, rexp->xloc->reflist_idx, rexp->xtype,
                       reflist);
      else
         upd_ref_entry(op, rexp->xname->reflist_index, rexp->xtype,
                       reflist);
}        
/************************************************************
  Function:  add_symlist
  Purpose:   Add the symlist
  Input:     op  -- PLUS or MINUS
             symptr -- pointer to a symbol table entry
  Output:    the symlist is updated
************************************************************/
void
add_symlist(int op, struct symtab *symptr)
{
     if ( symptr->symlist_idx == -1 ){
       if ( cur_symlist < MAX_REFLIST ) {
          symptr->symlist_idx = cur_symlist;
          symlist[cur_symlist++] = symptr;
       } else {
             yyerror (178);
             delexit();
       }

     }
     if ( op == PLUS )
        symptr->rcount++;
     else
        symptr->rcount--;
}
/************************************************************
 Function: get_type
 description: get the sub-expression type when this 
              subexp. is followed or preceded by some
              operators other than '+' and '-'.
 input:      r1 -- pointer to an expression
 output:     the expression type is resolved and save into
             the expression sturct pointed by r1
*************************************************************/
void
get_type(struct exp *r1)
{
    int i;

        for ( i = r1->st_inx; i<st_inx; i++) {
            put_reflist(i, &cur_reftmp, reftmp );
            }
        st_inx = r1->st_inx;
        exp_result(r1, reftmp, cur_reftmp);
        cur_reftmp = 0;
}

/************************************************************
   Function: put_reflist
   description: put the expression items in the  exp. stack
                into the refernce list array.
   input:    i -- index to the exp. stack
             cur_reflist -- pointer to the current index of the array reflist
             reflist -- array of the struct exp_ref_list to
                        hold RLD information for each section
                        or symbol
   output:  the reflist is updated
************************************************************/
void
put_reflist(int i,
            int *cur_reflist,
            struct exp_ref_list reflist[])
{
   int tmp_op;

       st_lp[i].exp_ptr->xtype = st_lp[i].x_type;
       if ( st_lp[i].sign == PLUS ) {
          if ( st_lp[i].exp_ptr->x_rrtype == RLD_NOT_SET || 
                     st_lp[i].exp_ptr->x_rrtype == (char)R_POS ){
              add_reflist(PLUS, st_lp[i].exp_ptr, cur_reflist, reflist);
              tmp_op = PLUS;
          } else{
               add_reflist(MINUS, st_lp[i].exp_ptr, cur_reflist, reflist);
               tmp_op = MINUS;
          }
          if ( passno == 2 && st_lp[i].exp_ptr->xname != NULL) 
              /* $ will has type of E_REL and xname is NULL */ 
            add_symlist(tmp_op, st_lp[i].exp_ptr->xname);
       }
       if ( st_lp[i].sign == MINUS ) {
          if ( st_lp[i].exp_ptr->x_rrtype == RLD_NOT_SET || 
                 st_lp[i].exp_ptr->x_rrtype == (char)R_POS ){
            add_reflist(MINUS, st_lp[i].exp_ptr, cur_reflist, reflist);
            tmp_op = MINUS;
          } else {
            tmp_op = PLUS;
            add_reflist(PLUS, st_lp[i].exp_ptr, cur_reflist, reflist);
          }
          if ( passno == 2 && st_lp[i].exp_ptr->xname != NULL) 
              /* $ will has type of E_REL and xname is NULL */ 
            add_symlist(tmp_op, st_lp[i].exp_ptr->xname);
       }
}

/******************************************************
  Function: reset_symlist
  Description: reset rcount and symlist_idx fields of symtab
               entries pointed by the symlist.
               Reset the global variable cur_symlist.

  Input: None.
  Output: symlist and cur_symlist are reset
*******************************************************/
void
reset_symlist(void)
{

  int i;

    for ( i=0; i<cur_symlist; i++) {
       symlist[i]->symlist_idx = -1;
       symlist[i]->rcount = 0;
    }
    cur_symlist = 0;
}


