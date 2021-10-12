static char sccsid[] = "@(#)81	1.36.1.14  src/bos/usr/ccs/bin/as/aspso.c, cmdas, bos41B, 9504A 1/18/95 17:25:17";
/*
 * COMPONENT_NAME: (CMDAS) Assembler and Macroprocessor 
 *
 * FUNCTIONS: doalign, forget, jalign, jbb, jbc, jbf, jbi, jbs, jbyte,
 *	      jcomm, jcsect, jdrop, jeb, jec, jef, jei, jes, jfile,
 *	      jfunction, jhash, jline, jorg, jreset, jstabx, jtc,
 *	      jusing, jxline, mkdisp, pop, popi, pop_scop, push, 
 *	      pushi, push_scop, jmachine, jsource, jmach_push, jmach_pop
 *            jref, find_mode, jlcomm
 *
 * ORIGINS:  27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	"as.h"
#include        <string.h>

extern   char *src_file_name;
extern   char *class_name[];
/****************************************************************/
/* This file contains the pseudo-op routines for as		*/
/****************************************************************/
FILE *dbiop = stdout;		/* debug file declare		*/
struct symtab *tocname = 0;	/* pointer to TOC[tc0]		*/
LINE *linenum = 0;              /* pointer to linenumber table  */
char *malloc();

SECT *ccsectx = 0;              /* == ccsect, except it tracks
                                   .comm's, .lcomm's as well as .csects.
                                   it is used by .stabx, 
                                   .bc/.ec and .bs/.es   */

int  in_comm = 0;               /* indicator of .comm or .lcomm scope  */
int asmmod_stack[MAX_ASMMOD_STK];
                               /* stack for saving the current  */
                               /* assembly mode value  and type */
int mstack_idx;                /* asmmod_stack pointer          */
int mach_push = -1;            /* .machine "push" flag          */
int mach_pop = -1;            /* .machine "pop" flag          */
/****************************************************************/
/* Section Numbers of Basic Section Headers in XCOFF            */
/****************************************************************/

       int XN_TEXT,
           XN_DATA,
           XN_BSS,
           XN_TYPCHK,
           XN_DEBUG;
extern int SECTION_NUM;

/****************************************************************/
/* Function:	jbyte 						*/
/* Purpose:	handle the .byte STRING pseudo op       	*/
/*              handle the .string STRING pseudo op       	*/
/*              each letter is placed on a byte boundary        */
/* Input:	p - the input string                      	*/
/****************************************************************/
jbyte(p, q)
char * p;
int q;
{
   while (*p)
      outb(*p++);
   if (q)
   outb(0);
}

/****************************************************************/
/* Function:	jalign						*/
/* Purpose:	handle the .align pseudo op			*/
/* Input:	xp - the expression with the align argument	*/
/****************************************************************/
jalign(xp)
register struct exp *xp;
{
	/* check if valid arg type				*/
	if (xp->xtype != E_ABS || xp->xvalue<0 || xp->xvalue>12) {
                                                /* message 052 */
		yyerror( 52 ); 
		return;
	}
	if(!xp->xvalue||!ccsect)	/* 0 value means no aligning 	*/
		return;
	doalign(1 << (int)xp->xvalue);
	/* need to keep track of maximum alignment per csect	*/
	ccsect->algnmnt = MAX(ccsect->algnmnt,xp->xvalue);
}

/****************************************************************/
/* Function:	doalign						*/
/* Purpose:	actually do an align, used for both .align and	*/
/*		before .long and .short				*/
/* Input:	n - align amount				*/
/****************************************************************/
doalign(n)
{
	int mask;
	int incode;
	unsigned char class;
#define BMASK (n&0x1)			/* byte mask		*/
#define HMASK (n&0x2)			/* halfwork mask	*/
        

	n--;
	mask = n&0xfff;
	/* mask off section and name unique bit, leave on TOC bit*/
	class = class&~0xa0;
	class = ccsect->ssclass&~0xa0;
	/* if in code then align with noops else with zeroes	*/
	incode = (class==XMC_PR||class==XMC_GL);

#ifdef IDEBUG
        if (indbg("doalign"))
	   fprintf(stderr,"%lx: .align %x sclass:%x mask:%x\n",
		   ccsect->dot,n,ccsect->ssclass,mask);
#endif
	if (incode&&(ccsect->dot&ALGNMSK))
                                                /* message 110 */
	   yywarn(110);

        if (ccsect->dot&BMASK) outb(0);	/* align to halfword	*/
        if (ccsect->dot&HMASK)		/* align to fullword	*/
	  outh((incode)? ZNOP: 0);
	while(ccsect->dot&mask)		/* align rest with full	*/
	   outl((incode)? NOP: 0);	/* word aligns		*/
}

/****************************************************************/
/* Function:	jorg						*/
/* Purpose:	process the .org command			*/
/* Input:	value - the org value				*/
/* Comments:	orgs anly allowed within current csect		*/
/****************************************************************/
jorg(xp)
register struct exp *xp;
{
long li;
	if (!((xp->xloc==ccsect) && 
	      (xp->xvalue>=0)    &&
	      (xp->xtype==E_REL || xp->xtype==E_EXT) )) {
                                                /* message 053 */
	     yyerror( 53 );
	     li = 0;
	     }
	else if (xp->xtype==E_EXT) 
	   li = xp->xvalue-xp->xloc->start;
	else li = xp->xvalue;
#ifdef IDEBUG
	if (indbg("jorg")) 
	   fprintf(stderr, "org: value: %x, ccsect->dot: %x\n",
			  xp->xvalue,ccsect->dot);
#endif
	/* save the largest dot value for the csect */
	if (passno==1  && ccsect->dot>ccsect->end)
	   ccsect->end = ccsect->dot;
	ccsect->dot = li;
}

/***********************************************************************
* base register manipulation.  Find base registers for various data   *
* addressability                                                 
**********************************************************************/

/* .using table - indexed by register number 			*/
struct using {
	long uoff;     	 /* offset from segment base 		*/
	SECT *usect; 	 /* pointer to csect with using expr 	*/
} using[NREGS];

/****************************************************************/
/* Function:	jreset						*/
/* Purpose:	reset the using table in between pass 1 and 2	*/
/****************************************************************/
jreset() {
	register struct using *up;
#ifdef _IBMRT
	extern int xaddr;
	xaddr = 0;
#endif
	for (up=using; up<using+16; up++) up->usect = 0;
}

/****************************************************************/
/* Function:	jusing						*/
/* Purpose:	process the .using pseudo-op by adding the	*/
/*		the register and the base address to the using	*/
/*		table						*/
/* Input:	apa - the argument with the address expression	*/
/* 		xpr - the expression which evaluates to the	*/
/*		      register to use for the using		*/
/* Comments:	this table is managed by jusing and jdrop, and	*/
/* 		is accessed by mkdisp				*/
/****************************************************************/
jusing(apa, xpr)
struct arg *apa;
register struct exp *xpr;
{
	register struct exp *xpa = apa->xp;
	register struct using *up;
	int regno, val, stype;

	/* get register number 	*/
	regno = xpr->xvalue;
	if(xpr->xtype != E_ABS || regno < 0 || regno > HIGHREG) {
                                                /* message 054 */
		yyerror( 54 );
		return;
	}
	up = using + regno;
	up->usect = 0;

	/* find the argument type */
	if(apa->atype != AEXP)
                                                /* modified message 055 */
		yyerror( 55);

	if (xpa->xtype==E_UNK && passno==1)
		return;              /* oh well -- wait til next pass */

	switch(xpa->xtype) {
	   case E_TREL:		/* toc relative using		*/
		if (xpa->xloc!=tocname->csect) {
                                                /* message 056 */
	     	   yyerror( 56 );
		   return;
		   }
	   case E_REL:
	 	val = xpa->xvalue;
		break;
	   case E_EXT:
		stype = xpa->xloc->stype;
		if (stype == XTY_ER) {
                                                /* message 057 */
		   yyerror( 57 );
		   return;
		   }
		else if ( stype==XTY_CM || stype==XTY_SD )
		   val = xpa->xvalue - xpa->xloc->start;
		else val = xpa->xvalue;
	        break; 
	   default:
                                                /* modified message 055 */
		yyerror( 55);
		return;
	   }
	up->usect = xpa->xloc;
	up->uoff = val;
}

/****************************************************************/
/* Function:	jdrop						*/
/* Purpose:	process the .drop pseudo-op by deleting the 	*/
/*              register entry from the using table		*/
/* Input:	xp - the register number			*/
/* Comments:	see jusing					*/
/****************************************************************/
jdrop(xp)
register struct exp *xp;
{

	register struct using *up;
	int regno = (int)xp->xvalue;

	if(xp->xtype != E_ABS || regno < 0 || regno > HIGHREG) {
                                                /* message 059 */
		yyerror( 59 );
		return;
	}
	up = using+regno;
	if((up->usect == 0) && (passno == 2))
                                                /* message 060 */
		yyerror( 60 );
	up->usect = 0;
}

#define ABS(x) ((x<0)?-x:x)
/****************************************************************/
/* Function: mkdisp                                             */
/* Purpose:  Try to convert arg to type ADISP by scanning .using*/
/*           registers that gives the smallest offset.		*/
/* Input:    ap - the argument which contains the d(r) info	*/
/* Comments: see jusing and jdrop				*/
/****************************************************************/
int mkdisp(ap)
register struct arg *ap;
{
	register struct using *up;
	register struct exp *xp = ap->xp;
	SECT *section = xp->xloc;
	int type = xp->xtype;
	int forward = (xp->xtype==E_UNK);
	int tocoff = 0;           /* indicator of TOC related disp. */
                                  /* when tocoff is on, the disp. is  */ 
                                  /* the offset to the TOC anchor    */
	int goodreg = -1;         /* current best base register */
	long goodoff = 0;         /* current best offset */
	long offset;

	if (forward && passno==1) return;
        if ( xp->xtype==E_TREL ) 
           tocoff = 1;
        else if ( xp->xtype==E_EXT && xp->xloc) { 
                if ( xp->xloc->secname->eclass==XMC_TD
                  || xp->xloc->secname->eclass==XMC_TC )
                  tocoff = 1;
        } else if  ( xp->xtype == E_EXT && xp->xname->etype == XTY_ER ) {
                if ( xp->xname->eclass == XMC_TD ) 
                  tocoff = 1;
        }

	if (ap->atype!=AEXP || (type != E_REL && type != E_REXT  &&
                     type != E_ABS && !(tocoff) && 
                     !( xp->xtype == E_EXT && xp->xloc->stype == XTY_SD ) &&
                     !( xp->xtype == E_EXT && xp->xloc->stype == XTY_CM )))
	     return;
            
        if ( type == E_REXT || type == E_ABS )
        {
           goodreg = 0;
           goodoff = xp->xvalue;
        } else if ( tocoff == 1 && xp->xname->etype == XTY_ER ) {
           goodreg = 2;
           goodoff = xp->xvalue;
        }
        else
	   for (up = using; up < using+NREGS; up++) {
		offset = xp->xvalue - up->uoff;
                if ( tocoff ) { /* for TOC relative disp., search for */
                                /* the TOC anchor only                */
	            if (up->usect==tocname->csect) {
			goodreg = up-using;
			goodoff = offset;
                    } else
                        continue;
                } else if ((up->usect) != section) 
		    continue;
		else if(goodreg < 0 || ABS(offset) < ABS(goodoff)) {
		    goodreg = up-using;
		    goodoff = offset;
	        }
	   }
		
	if(goodreg >= 0) {
		ap->atype = ADISP;
		ap->areg = goodreg;
		xp->xvalue = goodoff;
/*		xp->xtype = E_ABS;		*/
#ifdef IDEBUG
		if (indbg("mkdisp")) {
			fprintf(stderr, "%lx: using %x %lx\n",
				ccsect->dot,goodreg,goodoff);
		        if (xp->xloc) {
		           fprintf(stderr, "xp->xloc\n");
			   dmp_sect(xp->xloc);
			   }
			}
#endif
	  	}
}

/****************************************************************/
/* Function: jcsect                                             */
/* Purpose:  handle the .csect pseudo ops 			*/
/* Input:    class - storage class of this csect                */
/*           name  - name of the csect, 0 is unnamed		*/
/*           algn  - log 2 of the alignment of csect		*/
/****************************************************************/
jcsect(class,name,algn)
unsigned char class;
struct symtab *name;
int algn;
{
  unsigned char type = XTY_SD;


  if (!name) {
        name = lookup("",class);
#ifdef IDEBUG
	if (indbg("jcsect")) {
		fprintf(stderr,"made it into noname %s %c\n", name->name,
                         class);
		}
#endif
               }

  if ((name->etype==XTY_US||name->etype==XTY_ER)&&(passno==1)) {	
#ifdef IDEBUG
	if (indbg("jcsect")) {
		fprintf(stderr,"made it into newsect %s\n", name->name);
		}
#endif
			/*determine which sections are valid or in existance*/
	    switch(class) {   
	        case XMC_DB:
	        case XMC_PR:
	        case XMC_RO:
	        case XMC_GL:
	        case XMC_XO:
	        case XMC_SV:
	        case XMC_TI:
	        case XMC_TB:
		     XN_TEXT++;
		     break;
	        case XMC_TC0: 
	        case XMC_TC: 
	        case XMC_RW:
	        case XMC_DS:
	        case XMC_UA:
                case XMC_TD:
                     XN_DATA++;
		     break;
	        case XMC_BS:
		case XMC_UC:
                     XN_BSS++;
                     type = XTY_CM;
                     yywarn( 144, name->name );
		     break;
	        case C_DSECT:  /* DSECTS		*/
		     break;
	        default:
                        /* reset lineno to avoid user confusion*/
                        lineno=0;
                                                /* message 051 */
			yyerror( 51, class );
	      }
     name->csect = new_csect(class,type,name,algn);
     name->etype = type;
     name->value = 0;
     if (class==C_DSECT)
        name->type = E_REL;
     else name->type = E_EXT;
     }
  else if (name->etype != XTY_SD && name->etype != XTY_CM) {
                                                /* message 063 */
     yyerror( 63 );
     if (!name->csect) 
	return;
     }
  else if (name->csect->algnmnt != algn)
                                                /* message 111 */
     yywarn(111);
  ccsectx =                /* Record csect for stabs */
  ccsect = name->csect;    /* set the current csect */
  in_comm = 0;
}

/****************************************************************/
/* Function: jcomm						*/
/* Purpose:  handle the .comm and .lcomm pseudo ops		*/
/* Input:    name1 - name of .comm, or local name of .lcomm 	*/
/*           xp    - the length of the common section		*/	
/*           class - the storage class of common section which	*/
/*                   really tells if it is lcomm or comm	*/
/*           name2 - if .lcomm, name2 is the name of the	*/
/*                    containing csect				*/
/****************************************************************/
jcomm(name1,xp,class,name2,algn)
struct symtab *name1,*name2;
struct exp *xp;
unsigned char class;
int algn;
{
extern char *sections;
char  *addr;
int comm_length = xp->xvalue;

	if (xp->xtype != E_ABS) {
                                                /* message 064 */
		yyerror( 64 );
		return;
		}
	if (passno==1 && name1->type!=E_UNK && name1->type!=E_EXT) {
                                                /* modified message 001 */
		yyerror( 1, name1->name);
		return;
	}
        in_comm = 1;
        if ( class == XMC_RW || class == XMC_BS || class == XMC_UC )
               XN_BSS++;           /*indicates bss section created*/
        else if ( class == XMC_TD )
               XN_DATA++;          /* TD will be in the data section  */
        else {
               if ( passno == 2 )
                   yywarn( 182, class_name[(int)class] );
               class = XMC_RW;
        }

	if (passno==1 ) {
           if ( (name1->etype == XTY_US && name1->type == E_UNK) ||
                 (name1->etype == XTY_ER && name1->type == E_EXT )) {
            /* make sure that a csect has not defined for this symbol yet */
	      switch(class) {
                  case XMC_TD:
                  case XMC_UC:
                  case XMC_BS:
		  case XMC_RW:
                                                /*.comm pseudo-op      */
			name1->value = 0;
			name1->type = E_EXT;
                        ccsectx =        /* Record 'csect' for .stabs */
                                         /* .bc .ec .bs and .es */
			name1->csect = new_csect(class,XTY_CM,name1,algn);
			name1->etype = XTY_CM;
                        name1->sclass = C_EXT;
                        if (  class == XMC_TD )
                          name1->csect->dot = 0;  /* dot will be taken care */
                                             /* by the following while loop */
                        else 
			  name1->csect->dot = xp->xvalue;

/* this would be the spot for providing a pointer addresses
                        name2->    = name1->csect->dot possibly */
			break;
	      	  default:	
                                                /* message 067 */
			yyerror( 67 );
			break;
	      }
	   } else /* a csect has been opened for this symbol   */  
                  /* for non-TD comm, the value is accumulated */
                  /* for TD comm, it will be taken care by the */
                  /* following while loop                      */
               if ( class != XMC_TD )
                  name1->csect->dot += xp->xvalue;
        }

        if ( class == XMC_TD ) { /* the following code initialize the  */
                                 /* comm..[TD] with zeroes.            */
           ccsectx = name1->csect; /* in pass 2, ccsectx should be the .comm */
                                   /* containing csect as well               */
           while ( comm_length != 0 ) {
              if (listopt)                    /* put data in listing  */
                  collectlst(ccsectx->secname->name, ccsectx->dot,
                           (unsigned char)0, 1);
              if (passno==2) {                /* only write data in pass2*/
                  /* add the begining of the output buffer with   */
                  /* the offset into that buffer with the offset */
                  /* into the csect                               */
                  addr = sections + ccsectx->dot + ccsectx->start;
                  *addr = (unsigned char)0;
                  }
              ccsectx->dot++;  /* increment the current location counter */
              comm_length--;
           }
        }
}

/****************************************************************/
/* Function: jlcomm						*/
/* Purpose:  handle the .lcomm pseudo ops		        */
/* Input:    name1 - local name of .lcomm      	                */
/*           xp    - the length of the common section		*/	
/*           class - the storage class of common section      	*/
/*           name2 -   name2 is the name of the	                */
/*                    containing csect				*/
/*           algn -  log 2 of the alignment of common section   */
/****************************************************************/
jlcomm(name1,xp,class,name2,algn)
struct symtab *name1,*name2;
struct exp *xp;
unsigned char class;
int algn;
{
SECT *bss;
long bs;

        XN_BSS++;                       /*indicates bss section created*/
	if (xp->xtype != E_ABS) {
                                                /* message 064 */
		yyerror( 64 );
		return;
		}
	if (passno==1 && name1->type!=E_UNK && name1->type!=E_EXT) {
                                                /* modified message 001 */
		yyerror( 1, name1->name);
		return;
		}
        in_comm = 1;
	if (passno==1) {
	   switch(class) {
	      	case XMC_BS:	                /*.lcomm pseudo-op     */
	                /* Redefine symbol if previously initialized   */
			if (name1->sclass) {
	  		  name1->sclass = name1->type = name1->numaux = 0;
	  		  name1->etype = XTY_US;	/* unset */
			}

			if (!name2)
  		   	   name2 = lookup("",XMC_BS);
			if (name2->etype==XTY_US && name2->type==E_UNK) {
		  	   name2->value = 0;
			   name2->type = E_EXT;
                           ccsectx =        /* Record 'csect' for .stabs */
                                            /* .bb .eb .bc .ec .bs and .es */
			   name2->csect = new_csect(class,XTY_CM,name2,algn);
			   name2->etype = XTY_CM;
			   }
			bss = name2->csect;
			if (name2->etype!=XTY_CM || bss->ssclass!=class){
                                                /* modified message 001 */
		   		yyerror( 1, name2->name);
				break;
				}
			bs = xp->xvalue;
			switch(bs) {
		    	   case 0: 
		   	   case 1: break;
		   	   case 2: 
		   	   case 3: bss->dot = round(bss->dot,HW); break;
		   	   default: bss->dot = round(bss->dot,FW); break;
		   	   }
			name1->value = bss->dot;
			name1->type = E_REL;
			name1->csect = bss;
			bss->dot += bs;
			break;	
	      	default:	
                                                /* message 067 */
			yyerror( 67 );
			break;
			}
	      }
}

/****************************************************************/
/* Function:	jtc						*/
/* Purpose:	process the .tc pseudo-op			*/
/* Input:	tc_name - the entry in the symbol table that	*/
/* 			  is the name for the toc entry		*/
/* Comments:	the naming of toc entries is still a little 	*/ 
/*		shaky in my opion				*/
/****************************************************************/
static char tcnn[10]; 
int tcn = 0;
jtc(tc_name)
struct symtab *tc_name;
{
unsigned char tc_class;

	tc_class = (tc_name)?tc_name->eclass:XMC_TC;
        if (tc_name->name=="TOC")
	tc_class = XMC_TC0;
	/* if the toc entry is unnamed, then make up an 	*/
	/* impossible name that begins with an % and has a  	*/
	/* and has a number.  The number is global and is reset */
	/* in between passes so that pass 2 gets the same named	*/
	/* toc entries						*/
	if (!(tc_name) || !(tc_name->name)) {
	   sprintf(tcnn,"%%tcnn%d",tcn++);
   	   tc_name = lookup(tcnn,tc_class);
	   }

#ifdef IDEBUG
	if (indbg("jtc")) {
		fprintf(stderr,"before call to jcsect %x\n", tc_name);
		dmpsym(tc_name);
		}
#endif
	/* toc entries are really just csects so do the same 	*/
	/* as a csect						*/
	jcsect(tc_class,tc_name,2); 
#ifdef IDEBUG
	if (indbg("jtc")) {
		fprintf(stderr,"after call to jcsect %x\n", tc_name);
		dmpsym(tc_name);
		}
#endif
}


/****************************************************************/
/* Function:	jhash						*/
/* Purpose:	process the .hash pseudo-op			*/
/* Input:	sym  - the symbol to add the hash value to	*/
/*		symhash  - the hash value to add		*/
/****************************************************************/
jhash(sym,symhash)
struct symtab *sym;
char *symhash;
{
char *t;
char *strpbrk();
static char *hexsyms = "0123456789ABCDEFabcdef";

	if (passno==1) { 
	   if (sym->chktyp) 
                                                /* message 068 */
		yyerror( 68 , sym->name); 
	   else {
	      for (t=symhash; (*t); t++)
		  if (!isxdigit(*t)) {
                                                /* message 069 */
		     yyerror( 69 ,t[0]);
		     return;
		     }
	     sym->chktyp = symhash;
	     }
           XN_TYPCHK++;
        }
       else {
             if (sym->etype == XTY_US)
                                                /* message 070 */
                 yyerror( 70 );

             if ( strlen(sym->chktyp) != 20 && warning)
                                               /* message 160  */
                 yywarn(160);
       }

}

# define MAX_FILE 99	/* max number of file_table entries	*/
int     file_no;	/* index of current file in file_table	*/
int     last = -1;	/* index of last file name in table     */
char *file_table[MAX_FILE];	/* table of pointers toknown file names */

struct symtab *file_entry;	/* .file entry */

                        /* variables used in storing pseudo-op pointers*/
struct symtab * pop_scop();
extern struct symtab **scop_sym;
extern struct symtab **scop_p;
#define SCOPSZ sizeof(struct symtab **)
#define SCOPAREA SCOPSZ
int pusharea=0;
int poparea=0;
			/* variables used in storing pseudo-op pointers*/
struct symtab * stacki[NSTACK];
int stackip;
                         /*                                            */   
unsigned curblk;                /* used to indicate blk in scoping of var*/
int nextline= 0;               
LINE *lastlinenum;              /* pointer to last linenumber entry */

extern SCNHDR text, data;            /*external section headers */


/****************************************************************/
/* Function:	jfile 						*/
/* Purpose:	process the .file   pseudo-op			*/
/* Input:	s - filename string                             */
/****************************************************************/

jfile(s)
 char *s;
{
	register struct symtab *p;

/* remember where the .file entry is, so we can point it at first extdef */
/**	file_entry = p =
	  (struct symtab *)symalloc(sizeof (struct symtab) );        **/

        if ( !( p = (struct symtab *)malloc(sizeof (struct symtab)))) {
           yyerror (73);
           delexit();
        }
        file_entry = p; 
        bzero(p, sizeof(struct symtab));
        if (symtab)                    /* put '.file' entry at the     */
           p->nxtsym = symtab;         /* beginning of the internal    */
        else   /* symtab == 0 */       /* symbol table.                */ 
           lastsym = p;
        symtab = p;
              
        p->name = put_string(".file");    

        if (strlen(s)>14) {
           p->rename = put_string(s);
           src_file_name = p->rename;
        } else {
	   strcpy(p->file_aux.name, s);
           src_file_name = p->file_aux.name;
        }
        p->aux.nextaux = 0;
	p->type = T_NULL;
	p->sclass = C_FILE;
	p->etype = XTY_ER;
	p->csect = ccsect;
	p->sectnumber = N_DEBUG;
	p->numaux = 1;	

        file_flag = 1;

}
/****************************************************************/
/* Function:	jmachine 					*/
/* Purpose:	process the .machine   pseudo-op		*/
/*              this .machine overwrites the -m flag            */
/* Input:	as_mode - Assemble mode                         */
/****************************************************************/
jmachine(char *as_mode)
{
      char *s;
      unsigned short asm_mode_save;

      if ( *as_mode != '\0' ) {
         modeopt = 1;
         for ( s=as_mode; (*s); s++) *s = toupper(*s);
         if ( !strcmp(as_mode , "PUSH") )
             jmach_push();
         else if ( !strcmp(as_mode, "POP"))
             jmach_pop();
         else {
            asm_mode_save = asm_mode;   /* save the current asm mode value  */ 
            asm_mode = cvt_asmmode(as_mode, '2');
            if (asm_mode == 0 )   /* if .machine gives an invalid assembly mode   */
              asm_mode = asm_mode_save;  /*  reset asm_mode using the saved value */
                                         /* for the rest of the inst. validation */
            else                         /* update the asm mode string value  */
              strcpy(asm_mode_str, as_mode); 
         }
      } else {
         modeopt = 0;
         asm_mode = COM;    /* restore the default assembly mode    */
         asm_mode_type = PowerPC;
         strcpy(asm_mode_str,  "COM");
      }

}
/****************************************************************/
/* Function:	jsource 					*/
/* Purpose:	process the .source   pseudo-op	        	*/
/* Input:       src_id - source id                              */
/****************************************************************/
jsource (char *src_id)
{
   int i, match = 0;
   char *s;
   char *lang_name[] = {
        "C", "FORTRAN", "PASCAL", "ADA", "PL/1",
        "BASIC", "LISP", "COBOL", "MODULA2", "C++",
        "RPG", "PL8", "ASSEMBLER" };

   if ( src_id ) {
      for ( s=src_id; (*s); s++ )
          *s = toupper(*s);
      for ( i = 0; i <= 12; i++)
         if ( !strcmp( src_id , lang_name[i] ) ) {
             src_lang_id = i; 
             return;
         }
      if (  !strcmp(src_id, "PLIX") ) 
            src_lang_id = 11; 
      else {
         yyerror ( 148, src_id);  /* if .language has an invalid input value */
         src_lang_id = 12;  /* language ID is reset as "Assembler"       */
      }
   }
}
/****************************************************************/
/* Function:	jstabx                  			*/
/* Purpose:	process the .stab pseudo-op    			*/
/* Input:	pstring -  string debugger information          */
/*		value -  symbol value of the char string 	*/
/*		sclass - storage class                 		*/
/*		type -   symbol type of the char string		*/
/****************************************************************/

jstabx(pstring, value, sclass, type)
 char *pstring;
 long value;
 unsigned char sclass, type;
{
     register struct symtab *p;
     char *name,*stopstr;

	p = (struct symtab *)symalloc(sizeof(struct symtab));
                /* make field assignments*/
        p->name = put_string(".stabx");    
        p->dbug = put_string(pstring);
	p->value = value;
	p->type = type;
	p->etype = XTY_DBG;
        if ( in_comm) /* in .lcomm or .comm scope, then p->csect should  */
                      /* point to .lcomm or .comm section pointer   */
	   p->csect = ccsectx;
        else         /* not in .lcomm or .comm scope, then p->csect should */
                     /* point to current csect pointer                     */
           p->csect = ccsect;
	p->sclass = sclass;
        p->numaux = 0;
	p->sectnumber = N_DEBUG;
		/* set debug section flag */
	if(strlen(pstring)>SYMNMLEN)
		XN_DEBUG++;
	p->numaux=0;
                /*Scoping variable*/
	p->blk_id = myblk(); 
}

extern SCNHDR text, data;            /*external section headers */
int text_nlnno, data_nlnno;          /* number of line entries  */

/****************************************************************/
/* Function:	jfunction                			*/
/* Purpose:	process the .function pseudo-op			*/
/* Input:	p -  function name (symbol structure)           */
/*		exp - Top entry symbol of the function 		*/
/*		sclass - storage class of the function 		*/
/*		type - type of the funtion            		*/
/*              size - size of the function ( in bytes )        */
/****************************************************************/

jfunction(p, exp, sclass, type, size)
 register struct symtab *p;
 struct exp *exp, *sclass, *type, *size;
{
	LINE *nl;

        in_comm = 0;
	if(passno == 1) {
		p->aux.nextaux= (struct aux *)malloc(sizeof (struct aux));
		p->func_aux.lnnoptr = nextline++*LINESZ;
		p->numaux = 2;
	} else {                           /* pass 2 */
                  /* Save function pointer for later assignment */
	        push(++curblk, p);
		p->numaux = 2;
		  /* make sure size is absolute */
		if (size && size->xtype != E_ABS)
                                                /* message 143 */
			yyerror( 143 );
		  /* if size defined use defined value */
		if (size && size->xvalue)
			p->func_aux.fsize = size->xvalue;
	 	else	p->func_aux.fsize = p->csect->end;
					/* allocate a new entry	*/
                if (!(nl = (LINE *)malloc(sizeof (struct line) ))){
                                                /* message 073 */
                    yyerror( 73 );
	            delexit();
                    }
                if (!linenum)
                         linenum= nl;
                    else lastlinenum->nextline= nl;
                lastlinenum= nl;

                nl->nextline = 0;
		if(flag_extern_OK&&exp->xname)
		nl->l_addr.l_symndx = exp->xname->esdind;
		else
		nl->l_addr.l_symndx = p->esdind;
		nl->l_lnno = 0;

                if ((p->sectnumber=ccsect->secname->sectnumber) == XN_TEXT)
				text_nlnno++;
                else 	/*print out message if not in text section*/	
						/* message 074 */
				yyerror( 74 );

	}
}

/****************************************************************/
/* Function:	jbf  						*/
/* Purpose:	process the .bf   pseudo-op (begin function)    */
/* Input:	lnno -               line number                */
/****************************************************************/

jbf(lnno)
 int lnno;
{
	register struct symtab *p;

        if(passno==2) {
           if(!(p = pop_scop()))
              return;
           if(!((p->sectnumber=ccsect->secname->sectnumber) == XN_TEXT))
               	/*print out message if not in text section*/	
						/* message 074 */
				yyerror( 74 );
        }
        else { 
        if(!( p = (struct symtab *)symalloc(sizeof(struct symtab)))) 
              return; 
        p->name = put_string(".bf");
	p->type = T_NULL;
	p->etype = XTY_DBG;
	p->csect = ccsect;
	p->sclass = C_FCN;
	p->numaux = 1;
	p->be_aux.lnno = lnno;
        push_scop(p);
        }
	p->value = ccsect->start+ccsect->dot;
}

/****************************************************************/
/* Function:	jef  						*/
/* Purpose:	process the .ef   pseudo-op (end function)      */
/* Input:	lnno -               line number                */
/****************************************************************/

jef(lnno)
 int lnno;
{
	struct symtab *p, *q;
	unsigned bid;

        if(passno==2) {
           if(!(p = pop_scop()))
              return;
           if (!((p->sectnumber=ccsect->secname->sectnumber) == XN_TEXT))
               	/*print out message if not in text section*/	
						/* message 074 */
				yyerror( 74 );
		/*Recover .function pointer */
	   pop(&bid,&q);
           if (!q)
              return;
		/*Scope method - C */
	   forget(bid);
		/* Index of next entry beyond this function */
	   q->func_aux.endndx = p->esdind+2;
	   if(q->func_aux.lnnoptr == nextline) /* function has no lines */
		q->func_aux.lnnoptr = 0;
        }
        else { 
  	if(!(p = (struct symtab *)symalloc(sizeof(struct symtab))))
            return;
        p->name = put_string(".ef"); 
	p->type = T_NULL;
	p->etype = XTY_DBG;
	p->csect = ccsect;
	p->sclass = C_FCN;
	p->be_aux.lnno = lnno;
        push_scop(p);
	p->numaux = 1;	
        }
	p->value = ccsect->start+ccsect->dot;
}

/****************************************************************/
/* Function:	jbb  						*/
/* Purpose:	process the .bb   pseudo-op (begin block)       */
/* Input:	lnno -               line number                */
/****************************************************************/

jbb(lnno)
 int lnno;
{
	register struct symtab *p;

        if(passno==2) {
           if(!(p = pop_scop()))
              return;
                  /* Save .bb pointer for later assignment */
	   push(++curblk, p);
        }
        else { 
	if(!(p = (struct symtab *)symalloc(sizeof(struct symtab))))
           return;
        p->name = put_string(".bb");
	p->type = T_NULL;
	p->sectnumber = XN_TEXT;
	p->sclass = C_BLOCK;
	p->etype = XTY_DBG;
	p->csect = ccsect;
	p->numaux = 1;
	p->be_aux.lnno = lnno;
                /* keep track of pointer */
        push_scop(p);
        }
	p->value = ccsect->start+ccsect->dot;
}

/****************************************************************/
/* Function:	jeb  						*/
/* Purpose:	process the .eb   pseudo-op (end block)         */
/* Input:	lnno -               line number                */
/****************************************************************/

jeb(lnno)
 int lnno;
{
	struct symtab *p, *q;
	unsigned bid;

        if(passno==2) {
           if(!(p = pop_scop()))
              return;
		/*Recover .bb pointer */
	   pop(&bid, &q);
           if (!q)
              return;
		/*Scope*/
	   forget(bid);
		/* Index of next entry beyond this bb */
	   q->be_aux.endndx = p->esdind+2;
        }
        else { 
	if(!(p = (struct symtab *)symalloc(sizeof(struct symtab))))
           return;
        p->name = put_string(".eb");
	p->type = T_NULL;
	p->sectnumber = XN_TEXT;
	p->sclass = C_BLOCK;
	p->etype = XTY_DBG;
	p->csect = ccsect;
	p->be_aux.lnno = lnno;
        push_scop(p);
	p->numaux = 1;
        }
	p->value = ccsect->start+ccsect->dot;
}

/****************************************************************/
/* Function:	jbc  						*/
/* Purpose:	process the .bc   pseudo-op (begin common)      */
/* Input:	s - common block name?                          */
/****************************************************************/

jbc(s)
 char *s;
{
	register struct symtab *p;

	p = (struct symtab *)symalloc(sizeof (struct symtab));
        p->name = put_string(".bc");    /* placeholder */
        p->dbug = put_string(s);
	p->value = 0;
	p->type = T_NULL;
        p->sectnumber = N_DEBUG;
        /* set debug section flag */
        if (strlen(s) > SYMNMLEN)
                XN_DEBUG++;
	p->sclass = C_BCOMM;
	p->etype = XTY_DBG;
        if (in_comm )
          p->csect =  ccsectx;
        else
          p->csect =  ccsect;
	p->numaux = 0;	
}

/****************************************************************/
/* Function:	jec  						*/
/* Purpose:	process the .ec   pseudo-op (end common)        */
/* Input:	                                                */
/****************************************************************/

jec()
{
	register struct symtab *p;

	p = (struct symtab *)symalloc(sizeof (struct symtab));
        p->name = put_string(".ec");
	p->value = 0;
	p->type = T_NULL;
	p->sectnumber = N_DEBUG;
	p->sclass = C_ECOMM;
	p->etype = XTY_DBG;
        if (in_comm )
          p->csect =  ccsectx;
        else
          p->csect =  ccsect;
	p->numaux = 0;	
}

/****************************************************************/
/* Function:	jbs  						*/
/* Purpose:	process the .bs   pseudo-op (begin static)      */
/* Input:	s - static csect                                */
/****************************************************************/

jbs(s)
 struct symtab *s;
{
	register struct symtab *p;

        if(passno==2) {
           if(!(p = pop_scop()))
		return;
	   if(s->csect->ssclass!=XMC_BS)
                                                /* message 067 */
		yywarn( 67 );
	   p->value = s->csect->secname->esdind;
           if ( in_comm)
              p->sectnumber=ccsectx->secname->sectnumber;
           else
              p->sectnumber=ccsect->secname->sectnumber;
	}
        else { 
	   p = (struct symtab *)symalloc(sizeof (struct symtab));
           p->name = put_string(".bs");
	   p->type = T_NULL;
	   p->sclass = C_BSTAT;
	   p->etype = XTY_DBG;
           if ( in_comm) /* in .lcomm or .comm scope, then p->csect should  */
                         /* point to .lcomm or .comm section pointer   */
             p->csect = ccsectx;
           else         /* not in .lcomm or .comm scope, then p->csect should */
                        /* point to current csect pointer                     */
             p->csect = ccsect;

           push_scop(p);
	   p->numaux = 0;	
	}
}

/****************************************************************/
/* Function:	jes  						*/
/* Purpose:	process the .es   pseudo-op (end static)        */
/* Input:	                                                */
/****************************************************************/

jes()
{
	register struct symtab *p;

        if(passno==2) {
           if(!(p = pop_scop()))
              return;
           if ( in_comm)
              p->sectnumber=ccsectx->secname->sectnumber;
           else
              p->sectnumber=ccsect->secname->sectnumber;
	}
	else {
	   p = (struct symtab *)symalloc(sizeof (struct symtab));
           p->name = put_string(".es");
	   p->value = 0;
	   p->type = T_NULL;
	   p->sectnumber = N_DEBUG;
	   p->sclass = C_ESTAT;
	   p->etype = XTY_DBG;
           if ( in_comm) /* in .lcomm or .comm scope, then p->csect should  */
                         /* point to .lcomm or .comm section pointer   */
             p->csect = ccsectx;
           else         /* not in .lcomm or .comm scope, then p->csect should */
                        /* point to current csect pointer                     */
             p->csect = ccsect;
           push_scop(p);
	   p->numaux = 0;	
	}
}

/****************************************************************/
/* Function:	jbi  						*/
/* Purpose:	process the .bi   pseudo-op (begin include)     */
/* Input:	s - filename                                    */
/****************************************************************/

jbi(s)
 char *s;
{
	register struct symtab *p;

	p = (struct symtab *)symalloc(sizeof (struct symtab));
        p->name = put_string(s);
        p->value = nextline*LINESZ;
	p->type = T_NULL;
	p->sectnumber = N_DEBUG;
	p->sclass = C_BINCL;
	p->etype = XTY_DBG;
	p->csect = ccsect;
	p->numaux = 0;	
		/* save the bi pointer for use by jei */
	pushi(p);
}

/****************************************************************/
/* Function:	jei  						*/
/* Purpose:	process the .ei   pseudo-op (end include)       */
/* Input:	s - filename                                    */
/****************************************************************/

jei(s)
 char *s;
{
	struct symtab *p, *q;

	p = (struct symtab *)symalloc(sizeof (struct symtab));
        p->name = put_string(s);
	p->type = T_NULL;
	p->sectnumber = N_DEBUG;
	p->sclass = C_EINCL;
	p->etype = XTY_DBG;
	p->csect = ccsect;
	p->numaux = 0;	
		/* restore the bi pointer for use in determining */
		/* p->value, the ptr to last line in this file   */
	popi(&q);
       	p->value = q->be_aux.lnnoptr;
}


/****************************************************************/
/* Function:	jline						*/
/* Purpose:	process the .line pseudo-op			*/
/* Input:	slno - same function line number                */
/****************************************************************/

jline(slno)	/* call with nonzero to indicate another line same function */
 unsigned slno;
{
	LINE *nl;
	struct symtab *q;
	unsigned bid;
	
	if(passno == 2) {

					/* allocate a new entry	*/
                if (!(nl = (LINE *)malloc(sizeof (struct line) ))){
                                                /* modified message 073 */
                   yyerror( 73 );
                   delexit();
                   }
                if (!linenum)
                        linenum= nl;
                   else lastlinenum->nextline= nl;

                lastlinenum= nl;

                nl->nextline = 0;
		nl->l_lnno = slno; 
                if (slno)
                nl->l_addr.l_paddr  = ccsect->start+ccsect->dot;
                else {
				/*Recover .function/.bb pointer */
	   	   	pop(&bid, &q);
           		if (q)
				/*Symbol table index of function name*/
			nl->l_addr.l_symndx = q->esdind;
				/*Return .function/.bb pointer */
	        	push(bid, q);
		}

                if (ccsect->secname->sectnumber == XN_TEXT)
                                text_nlnno++;
                else if (ccsect->secname->sectnumber == XN_DATA)
                                data_nlnno++;
                     else
                                                /* message 076 */
				yyerror( 76 );

		++nextline;
          }
	 else
		++nextline;
}

int Incl_status;
/****************************************************************/
/* Function:	jxline						*/
/* Purpose:	process the .xline pseudo-op			*/
/* Input:	ln_num  - the new value for the lineno          */
/*		ln_file - the new value for infile   		*/
/*		Incl    - indicates top/bot of include file     */
/****************************************************************/

int
jxline(xp,ln_file,Incl)
register struct exp *xp;
char *ln_file;
int Incl;

	{
	register int  i = 0;
	struct symtab *p;

	if(infile)	/* IF INPUT IS NOT STANDARD IN DONT DO ANY OF THIS  */
		{       /* NOTE: if infile is zero then standard in is input*/
		i = 0;  /* because it points to no name                     */
		/* get position of ln_file in file table; i>last ==> no find*/
		while(i <= last &&  strcmp(ln_file, file_table[i]))
			i++;
		if(i>last){
			/* need to add ln_file to the file_table 	     */
			if(++last <= MAX_FILE){
				/* enough room in table for one more */
				file_table[last] = ln_file;
				infile = file_table[last];
				file_no = last;
				lineno = xp->xvalue;
				}
			else	{
				/* error - table to small           	     */
				/* not big enough MAX_FILE          	     */
                                                /* message 077 */
				yyerror( 77 );
				return(1);
				}
			}
		else	{
			/* ln_file found in table         		     */
			infile = ln_file;
			file_no = i;
			lineno = xp->xvalue;
			}

		if (Incl) { 
		   if (passno==1) {
		   	if (Incl==C_BINCL)
		   		jbi(ln_file);
		   	else
		   		jei(ln_file);
			}
		   }
		return(0);
		}
	}


/****************************************************************
 Function:	push 				
 Purpose:      Process the .function, or .bb pseudo-op. Saving
               the function pointer or .bb pointer into the 
               scope stack.
 Input:        a - scope block id.
               b -  symbol table entry pointer for .function or
                    .bb
 Return:	the scope stack pointer is incremented.
****************************************************************/
push(a,b)
 unsigned a;
 struct symtab *b;
{
	if(stackp >= NSTACK) {
                            /* message 002 */
		yyerror( 2 );	stackp = NSTACK-1;
	}
	stack[stackp].blkid = a;	stack[stackp++].ptr = b;
}

/****************************************************************
 Function:	pop
 Purpose:       take out the saved information for .function or
                .bb from the scope stack.
 Input:         a - pointer of the block id
                b - pointer of pointer of the symbol table entry for
                    .function or .bb

 Return:        block id and symbol table entry pointer.
                scope stack pointer is decremented.	
****************************************************************/
pop(a,b)
 unsigned *a;
 struct symtab **b;
{
	if(stackp<=0) {
                            /* message 047 */
		yyerror( 47 );	stackp = 0;
	}
	*b =stack[--stackp].ptr;
	*a = stack[stackp].blkid;
}

/****************************************************************
 Function:	forget 				
 Purpose:	unlink all entries where blk_id == bid 
 Input:		bid - block id
****************************************************************/
forget(bid)	/* unlink all entries where blk_id == bid */
 unsigned bid;
{
	register struct symtab *p, *q, *s;
	register int i;
 
	for(i = 0; i < NSHASH; ++i)
		if(q = s = shash[i]) {
			do {
				if((p = q->next)->blk_id == bid) {
					q->next = p->next;
					p->blk_id = -1;
				}
				q = p;
			} while(p != s);
			if(s->blk_id == -1)
				shash[i] = s->next==s? 0: s->next;
		}
}
 
/****************************************************************
 Function:	push_scop 				
 Purpose:       In Pass 1, it tracks the .bf, .ef, .bb, .eb
                .bs, .es pseudo-ops. The pointers of those symbol pointers
                are pushed into a queue area scop_sym, the queue pointer
                is scop_p.
 Input:         b -  symbol pointer of .bf/.ef/.bb/.eb/.bs/.es
 Return:	
****************************************************************/
unsigned int scoparea=SCOPAREA;
push_scop(b)
 struct symtab *b;
{
        int scopsz;

        scopsz = SCOPSZ;
			/*add symbol pointer to scope stack*/
	if (pusharea%(scoparea)) {
		scop_p++;
	   }
	else {
		if (scop_sym) {
                 scoparea += SCOPAREA;
		 if (!(scop_sym = (struct symtab **)realloc(
			(char *)scop_sym,scoparea))){
                                                /* message 073 */
                     yyerror( 73 );
                     return;
                 }
                 scop_p = scop_sym + (pusharea/scopsz);
                } 
	 	else 
        	 if (!(scop_p = scop_sym = 
			(struct symtab **)malloc(scoparea))){
                                                /* message 073 */
                     yyerror( 73 );
                     return;
                 }
           }
	pusharea += SCOPSZ;
	*scop_p = b;
#ifdef IDEBUG
  		if(i_debug&64) {
		   fprintf(dbiop, "PUSH_SCOP = 0x%x lnno=%d value= 0x%x\n",
                                   *scop_p,b->aux.aux_entry.f1.lnno,b->value);
	}
#endif
  
}

/****************************************************************
 Function:	pop_scop 				
 Purpose:       In Pass 2, the symbol pointer are poped from the
                queue scop_sym
 Return:	
 Comment:    the queue pointer scop_p is reset to the beginning
             of the queue  in as1.c in the beginning of Pass 2.
****************************************************************/
struct symtab *
pop_scop()
{
 struct symtab *q;

	if (poparea>=pusharea) {
                                                /* message 009 */
  		yyerror( 9 );
		return;
	    }
	
	q = *scop_p;
	poparea += SCOPSZ;

#ifdef IDEBUG
  		if(i_debug&64) {
		   fprintf(dbiop, "POP_SCOP = 0x%x lnno=%d value= 0x%x\n",
				    *scop_p,q->aux.aux_entry.f1.lnno,q->value);
	}
#endif
  
	scop_p++;

	return q;
}     	

/****************************************************************/
/* Function:	popi 						*/
/* Purpose:	process bi pseudo-op pointers			*/
/*         	on stackpi and return the last pointer		*/
/* Return:	bi pseudo-op pointer	                        */
/****************************************************************/
popi(a)
 struct symtab **a;
{
	if(stackip<=0) {
                            /* message 047 */
		yyerror( 47 );	stackip = 0;
	}
	*a = stacki[--stackip];
	stacki[stackip]->be_aux.endndx = nextline-1;
	if((stackip-1>=0)
	  &&(stacki[stackip]->value==stacki[stackip-1]->value))
		stacki[stackip-1]->value = nextline*LINESZ;
	if((stackip+1<NSTACK)
          &&(stacki[stackip]->be_aux.endndx==stacki[stackip+1]->be_aux.endndx))
		stacki[stackip]->be_aux.lnnoptr 
			= stacki[stackip+1]->value-LINESZ;
	else
		stacki[stackip]->be_aux.lnnoptr = (nextline-1)*LINESZ;
}


/****************************************************************/
/* Function:	pushi						*/
/* Purpose:	place bi pseudo-op pointers			*/
/*         	on stackpi                 			*/
/* Input:	a - bi pseudo-op pointers                       */
/****************************************************************/
pushi(a)
 struct symtab *a;
{
	if(stackip >= NSTACK) {
                            /* message 002 */
		yyerror( 2 );	stackip = NSTACK-1;
	}
	stacki[stackip++] = a;
}
/****************************************************************/
/* Function:    find_mode                                      */
/* Purpose:     return the index of asmmode_tab for a assm mode */
/****************************************************************/
int
find_mode(void)
{
   int i;

      for ( i=0; i<max_mode_num; i++)
         if ( asmmode_tab[i].mode_value == asm_mode )
           return(i);
}

/****************************************************************/
/* Function:    jmach_push                                      */
/* Purpose:     Save the current assembly mode into             */
/*              machine_stack                                   */
/****************************************************************/
void 
jmach_push(void)
{
        if (mstack_idx >= MAX_ASMMOD_STK )
           yyerror1(174);
        else {
          mach_push = find_mode();
          asmmod_stack[mstack_idx++] = mach_push;
        }
}
/****************************************************************/
/* Function:    jmach_pop                                      */
/* Purpose:     Restore the saved assembly mode value          */
/****************************************************************/
void 
jmach_pop(void)
{

      if (--mstack_idx < 0) {
         yyerror1(175);
         mstack_idx++;
      }
      else {
         mach_pop = asmmod_stack[mstack_idx];
         strcpy(asm_mode_str,asmmode_tab[mach_pop].mode_str );
         asm_mode = asmmode_tab[mach_pop].mode_value;
         asm_mode_type = asmmode_tab[mach_pop].as_md_type;
      }
}
/****************************************************************/
/* Function:    jref                                            */
/* Purpose:     process .ref pseudo-op                          */
/* input:       ref_sym  --  symbol as .ref operand             */
/****************************************************************/
void 
jref( struct symtab *ref_sym)
{
    if ( (ref_sym->csect != NULL && ref_sym->csect->ssclass == C_DSECT ) 
                       /* ref_sym is defined in a dsect, or a csect with */
                       /* BS or UC class                     */
           || ( ref_sym->type == E_ABS )  )   
        yyerror( 177, ref_sym->name );
    else 
         addrld(R_REF, 1, ref_sym);
}
   
