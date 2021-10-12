static char sccsid[] = "@(#)48	1.36.5.11  src/bos/usr/ccs/lib/libdbx/fortran.c, libdbx, bos411, 9433A411a 7/25/94 18:58:48";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: fort_arr_size, lassname, fortran_evalaref,
 *	      fortran_foldnames, fortran_hasmodules, fortran_init,
 *	      fortran_listparams, fortran_modinit, fortran_passaddr,
 *	      fortran_printarray, fortran_printdecl, fortran_printsubarray,
 *	      fortran_printval, fortran_typematch, islogical, isspecial,
 *	      mksubs, printcomplex, printint, printlogical, rev_index, isrange,
 *	      f_typename, is_string_param, is_fortran_padded, f_symname
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */
#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
/*              include file for message texts          */
#include "dbx_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/*
 * FORTRAN dependent symbol routines.
 */

#include "defs.h"
#include "symbols.h"
#include "printsym.h"
#include "languages.h"
#include "tree.h"
#include "eval.h"
#include "operators.h"
#include "mappings.h"
#include "process.h"
#include "runtime.h"
#include "machine.h"
#define MAXINT 0x7fffffff

#define isderived(t) ( \
    (t && t->type && \
     (t->type->class == RECORD || t->type->class == PACKRECORD)) \
)

/* Base type of fortran integer*1 is character type... */
#define fortran_isintegral(n) ( \
    isintegral(n) || \
    streq(ident(n),"unsigned char") || streq(ident(n),"signed char") \
)

#define isspecial(range) ( \
    (range->class == RANGE) && \
    (range->symvalue.rangev.upper == 0 and range->symvalue.rangev.lower > 0) \
)
 
#define islogical(range) ( \
    (range->type != nil and (!strncmp(ident(range->type->name),"logical",7))) \
     || (range->type != nil and (streq(ident(range->type->name),"$boolean"))) \
 )

#define isfloat(range) ( \
    (range->class == REAL) || \
    ((range->class == RANGE) && \
     (range->symvalue.rangev.upper == 0 and range->symvalue.rangev.lower > 0)) \
 )

#define iscomplex(range) ( \
    (range->type != nil) && \
    (((range->type->class == COMPLEX)) || \
     (!strncmp(ident(range->type->name),"complex",7)) || \
     (streq(ident(range->type->name),"double complex")) || \
     (streq(ident(range->type->name),"$qcomplex")) || \
     (streq(ident(range->type->name),"$complex"))) \
 )

#define isrange(t, name) (t->class == RANGE and istypename(t->type, name))

#define fort_arr_size(t) ( \
    (t->type->class != TYPE    && t->type->class != TAG &&      \
     t->type->class != FSTRING && t->type->class != CHARSPLAT &&   \
     t->type->class != 0)  \
)

#define MAXDIM  20

public Language fLang;
public int initializestring = 0;

extern  boolean  hexints, octints;
extern  boolean  unique_fns;
/*
 *  Variables for subarrays
 */
struct subdim {
	long ub;
	long lb;
	struct subdim *next, *back;
};
extern struct subdim *subdim_tail;
extern Symbol  array_sym;
extern Address array_addr;
extern boolean subarray;
Symbol  array_top;

private String f_typename();
private printlogical();
private printcomplex();
private printint();
private fortran_printarray();
private boolean is_string_param();

/*  array is zeroed out because it is static  */
static short usetype[TP_NTYPES];

/*
 * Initialize FORTRAN language information.
 */

void fortran_init ()
{
    fLang = language_define("fortran", ".f");
    language_setop(fLang, L_PRINTDECL, fortran_printdecl);
    language_setop(fLang, L_PRINTVAL, fortran_printval);
    language_setop(fLang, L_TYPEMATCH, fortran_typematch);
    language_setop(fLang, L_BUILDAREF, buildaref);
    language_setop(fLang, L_EVALAREF, fortran_evalaref);
    language_setop(fLang, L_MODINIT, fortran_modinit);
    language_setop(fLang, L_HASMODULES, fortran_hasmodules);
    language_setop(fLang, L_PASSADDR, fortran_passaddr);
    language_setop(fLang, L_FOLDNAMES, fortran_foldnames);

    usetype[-TP_REAL-1] = 1;
    usetype[-TP_FCHAR-1] = 1;
    usetype[-TP_LOGICAL1-1] = 1;
    usetype[-TP_LOGICAL2-1] = 1;
    usetype[-TP_LOGICAL4-1] = 1;
    usetype[-TP_COMPLEX-1] = 1;
    usetype[-TP_DCOMPLEX-1] = 1;
    usetype[-TP_INTEGER1-1] = 1;
    usetype[-TP_INTEGER2-1] = 1;
    usetype[-TP_INTEGER4-1] = 1;
#ifdef  TP_LOGICAL8
    /* To compile kdbx on AIX3.2 ... */
    usetype[-TP_LOGICAL8-1] = 1;
    usetype[-TP_INTEGER8-1] = 1;
#endif
}


/*
 * Test if a symbol is a padded symbol
 */
Boolean is_fortran_padded(Symbol s)
{
    return (Boolean) ((s->class == TYPE || s->class == REAL || 
		       s->class == COMPLEX) &&
		      (s->language == fLang) &&
	    	      (s->symvalue.field.length > 0) &&
	              (s->symvalue.size > (unsigned int) 0) &&
	              (s->symvalue.size >= s->symvalue.field.length));
}
	    

/*
 * Test if two types are compatible.
 */

Boolean fortran_typematch(Symbol type1, Symbol type2)
{
    int varsize = 0;
    int expsize = 0;
    register Symbol t1, t2, temp_sym;
    Boolean isintegral();

    if (type1==nil or (type1->type==nil and type1->class != FPTR) or
        type2==nil or (type2->type==nil and type2->class != FPTR))
       return false;
    else if (type1->type == type2->type)
       return true;
    else if ((type1 == type2->type ) or (type2 == type1->type))
       return true;
    type1 = rtype(type1);
    type2 = rtype(type2);
    /* need to allow assign integer to FPTR (fortran pointer) */
    if ((fortran_isintegral(type1->type->name)) or type1->class == FPTR)
       return ((fortran_isintegral(type2->type->name)) or type2->class == FPTR);
    else if (isfloat(type1))
       return (isfloat(type2));
    else if (iscomplex(type1))
       return (iscomplex(type2));
    else if (islogical(type1))
       return (islogical(type2));
    else if (istypename(type1->type, "void"))
       return (istypename(type2->type, "void"));
    else {
       /*
        * Type checking required for FORTRAN character types
        */

       /* Handle typematching of regular and dynamic array   */
       /* A dynamic array matches regular array of any sizes */
       int lb, ub;
       Symbol r;
       if (type1->class == ARRAY and type2->class == ARRAY) {
         r = type1->chain;
         /* var-dim array should match all sizes at all time */
         if (r->symvalue.rangev.lowertype != R_ADJUST ||
             r->symvalue.rangev.lowertype != R_CONST &&
	     !getbound(type1,r->symvalue.rangev.lower,
		       r->symvalue.rangev.lowertype, &lb)) {
	   /* if lowerbound is dynamic, set their sizes be equal. */
	   varsize = expsize = 1;
         /* var-dim array should match all sizes at all time */
         } else if (r->symvalue.rangev.uppertype == R_ADJUST ||
                    r->symvalue.rangev.uppertype != R_CONST &&
	            !getbound(type1,r->symvalue.rangev.upper,
	 	              r->symvalue.rangev.uppertype, &lb)) {
		  /* if upperbound is dynamic, size matched. */
	          varsize = expsize = 1;	
	 } else {
	     /* else... */
             varsize = size(type1);
             expsize = size(type2);
         }
       }
       else {
	/* if they aren't arrays... */
        varsize = size(type1);
        expsize = size(type2);
       }

       temp_sym = type1;
       while (temp_sym && temp_sym->type->class != TYPE)
	  temp_sym = temp_sym->type;
       if ((temp_sym->class == CHARSPLAT) || (temp_sym->class == FSTRING))
       {
          if ((varsize != 1) && (expsize != 1))
          {
	     /* account for the null character of C string */
	     if (istypename(type2->type,"$char"))
               expsize--;
             if (!varIsSet("$unsafeassign") && (varsize != expsize))
             {
		if (varsize > expsize)
			initializestring++;
		else {
                (*rpt_output)(stdout,   catgets(scmc_catd, MS_fortran,
		     MSG_80, "Error: element size is %d characters, not %d\n"),
							      varsize,expsize);
                erecover();
		}
             }
          }
          if (expsize != varsize && !initializestring)
          {
             return false;
          }
       }
       else if ((varsize == 1) && (expsize > 1))
       {
          return false;
       }
       t1 = type1;
       t2 = type2;
       while (t1 && t1->class != TYPE) t1 = t1->type;
       while (t2 && t2->class != TYPE) t2 = t2->type;
       return ((Boolean)((!strcmp(t1->name->identifier,"char") &&
            	 (!strcmp(t2->name->identifier,"char") ||
                  !strcmp(t2->name->identifier,"$char"))) ||
                  fortran_typematch(t1,t2) ));
                  /* check array of other types, eg. double, float... */
    }
}

/* Name of parameters in functions with entry statements contains a '#'   */
/* sign, this routine strips out the '#' sign and return the actual name. */
static String f_symname (Symbol s)
{
   char *n = symname(s);
   return (*n == '#') ? n+1 : n;
}


static String f_typename (Symbol s)
{
    int elsize;
    static char buf[20];
    char *pbuf;
    Symbol st,sc;
    String word;

    /* Allow definition in terms of other types for ease of use... */
    if ((s->type->class == TYPE) && (nilname(s->type))) {
        return f_typename((s->type));
    }
    else if (s->type->class == CHARSPLAT)
    {
       if (s->type->symvalue.iconval > 1)
          sprintf(buf,"character*%d",s->type->symvalue.multi.size);
       else
          sprintf(buf,"character");
       return(buf);
    }
    else if (s->type->class == FSTRING)
    {
       if ( !isactive(container(s)))
          strcpy(buf,"character*?");
       else
       {
          elsize = size(s->type);
	  if (elsize > 1)
             sprintf(buf,"character*%d",elsize);
	  else
             sprintf(buf,"character");
       }
       return(buf);
    }

    if (s->type->class != TYPE) {
        for (st = s->type; fort_arr_size(st); st = st->type);
    } else {
	st = s;
    }

    pbuf = buf;
    word = symname(st->type);
    if (isderived(st->type)) {
      sprintf(buf, "type (%s)",word);
      return(buf);
    }
    if (!strcmp(word,"char"))
      strcpy(pbuf,"character ");
    else if (!strcmp(word,"double"))
      strcpy(pbuf,"real*8 ");
    else if (!strcmp(word,"int"))
      strcpy(pbuf,"integer ");
    else if (!strcmp(word,"float"))
      strcpy(pbuf,"real*4 ");
    else if (!strcmp(word,"short"))
       return("integer*2");
    else if ((!strcmp(word,"unsigned char")) || (!strcmp(word,"signed char")))
       return("integer*1");
    else if (st->type->class == CHARSPLAT)
    {
       if (st->type->symvalue.multi.size > 1)
          sprintf(buf,"character*%d",st->type->symvalue.multi.size);
       else
          sprintf(buf,"character");
       return(buf);
    }
    else if (st->type->class == FSTRING)
    {
       if ( !isactive(container(s)))
          strcpy(buf,"character*?");
       else
       {
          elsize = size(st->type);
	  if (elsize > 1)
             sprintf(buf,"character*%d",elsize);
	  else
             sprintf(buf,"character");
       }
       return(buf);
    } else {
       return(word);
    }
    return(buf);
}

static Symbol mksubs (char** pbuf, Symbol st)
{   
   int lb, ub;
   Symbol r;

   if(st->class != ARRAY or
      (istypename(st->type, "char") and !isderived(st->type))) return;
   while (st->class == ARRAY) {
          assert((r = st->chain)->class == RANGE);

          if(r->symvalue.rangev.lowertype != R_CONST &&
             r->symvalue.rangev.lowertype != R_ADJUST) {
	      if( ! getbound(st,r->symvalue.rangev.lower, 
                    r->symvalue.rangev.lowertype, &lb))
		sprintf(*pbuf,"?:");
	      else 
		sprintf(*pbuf,"%d:",lb);
	  }
          else {
		lb = r->symvalue.rangev.lower;
		if (lb != 1)
		    sprintf(*pbuf,"%d:",lb);
	  }
    	  *pbuf += strlen(*pbuf);

          if(r->symvalue.rangev.uppertype != R_CONST &&
             r->symvalue.rangev.uppertype != R_ADJUST) {
	      if( ! getbound(st,r->symvalue.rangev.upper, 
                    r->symvalue.rangev.uppertype, &ub))
		sprintf(*pbuf,"?,");
	      else 
		sprintf(*pbuf,"%d,",ub);
	  } else if (r->symvalue.rangev.uppertype == R_ADJUST) {
		sprintf(*pbuf,"*,");
          } else {
		ub = r->symvalue.rangev.upper;
		sprintf(*pbuf,"%d,",ub);
	  }
    	  *pbuf += strlen(*pbuf);
	  st = st->type;
       }
}

/*
 * Print out the declaration of a FORTRAN variable.
 */

void fortran_printdecl (Symbol s)
{
    String fn;

    if (s && s->type && (s->type->isVolatile ||
                         (s->type->class == PTR &&
                          s->type->type &&
                          s->type->type->isVolatile))) {
       (*rpt_output)(stdout, " volatile ");
    }
    switch (s->class) {
        case LABEL:
            /* fortran named construct */
            (*rpt_output)(stdout, " constructname %s", symname(s));
            break;

	case CONST:
	    (*rpt_output)(stdout, "parameter %s = ", symname(s));
	    eval(s->symvalue.constval);
            printval(s, 0);
	    break;

        case FPTR:
            (*rpt_output)(stdout, "pointer  integer*4 %s ", symname(s));
            break;

        case REF:
            (*rpt_output)(stdout, " (dummy argument) ");
        case FPTEE:
	    if (s->class == FPTEE)
               (*rpt_output)(stdout, "pointee");
	case VAR:
	case TOCVAR:
        case FIELD:
          {
            Boolean is_active = true;
            Boolean isallocatable = s->type->isallocatable;
            Boolean ispointer = s->type->ispointer;

            if (is_f90_sym(s))
              s = convert_f90_sym(s, &is_active);

            if (s->type->class == ARRAY) 
            {
               char bounds[130], *p1, **p;

               if (is_f90_sym(s))
                 s = convert_f90_sym(s, &is_active);

               /*  if this is a normal array, or an allocatable
                     array that has been allocated  */
               if (is_active)
               {
                 /*  find the array bounds  */
	         bounds[0] = '\0';
                 p1 = bounds;
                 p = &p1;
                 mksubs(p,s->type);
                 *p -= 1; 
                 **p = '\0';   /* get rid of trailing ',' */
               }

               (*rpt_output)(stdout, " %s", f_typename(s));

               if (isallocatable)
                 (*rpt_output)(stdout, " allocatable");
               else if (ispointer)
                  (*rpt_output)(stdout, " pointer");

               (*rpt_output)(stdout, " %s", f_symname(s));
              
	       if (((s->type->type->class == CHARSPLAT) || 
                   (s->type->type->class == FSTRING)) && (!strcmp(bounds,"1")))
		  /* f_symname is used here to strip out '#' for names of */
		  /* parameters from functions with entry statements.     */
		  /* Only needed when class == REF.			  */
                  break;

               /*  if the array is not allocated  */
               /*  NOTE : this can only happen with fortran arrays  */
               if (!is_active)
               {
                 if (isallocatable)
                   (*rpt_output)(stdout, "(not allocated)");
                 else if (ispointer)
                   (*rpt_output)(stdout, "(not associated)");
                 else
                   (*rpt_output)(stdout, "(not active)");
               }
	       else
                  (*rpt_output)(stdout, "(%s)", bounds);
            } else if (s->type->class == FFUNC) {
               if (not istypename(s->type->type, "void"))
                  (*rpt_output)(stdout, " %s function %s",
                                                f_typename(s), f_symname(s));
               else
                  (*rpt_output)(stdout, " subroutine %s", f_symname(s));
            } else {
               (*rpt_output)(stdout, " %s",f_typename(s));
               if (ispointer)
                  (*rpt_output)(stdout, " pointer");
               (*rpt_output)(stdout, " %s", f_symname(s));
               if (!is_active && ispointer)
                 (*rpt_output)(stdout, "(not associated)");
	    }
            break;
          }

        case GENERIC:
            if (not istypename(s->type, "void")) {
                (*rpt_output)(stdout, " generic function %s\n", symname(s));
            } else {
                (*rpt_output)(stdout, " generic subroutine %s\n", symname(s));
            }
            break;

	case FUNC:
	case CSECTFUNC:
	    if (not istypename(s->type, "void")) {
                (*rpt_output)(stdout, " %s %s ", f_typename(s),
					    isentry(s) ? "entry" : "function");
	    } else if (isentry(s)) {
		(*rpt_output)(stdout, " entry");
	    } else {
		(*rpt_output)(stdout, " subroutine");
	    }
	    (*rpt_output)(stdout, " %s ", symname(s));
	    fortran_listparams(s);
	    break;

	case MODULE:
	    fn = symname(s);
	    (*rpt_output)(stdout, catgets(scmc_catd, MS_fortran, MSG_232,
			    "source file \"%s.f\""), (unique_fns) ? ++fn : fn);
	    break;

	case PROG:
	    (*rpt_output)(stdout,  catgets(scmc_catd, MS_fortran, MSG_233,
					"executable file \"%s\""), symname(s));
	    break;

	case TYPE:
            /* Display fortran derived types (structure) */
            if (isderived(s)) {
               Symbol f;
               (*rpt_output)(stdout, "type %s\n", symname(s));
               if (s->type->class == PACKRECORD)
                   (*rpt_output)(stdout, "  sequence\n");
               for (f = s->type->chain; f != nil; f = f->chain) {
                   (*rpt_output)(stdout, " ");
                   fortran_printdecl(f);
               }
               (*rpt_output)(stdout, "end type %s", symname(s));
            } else {
               (*rpt_output)(stdout,  catgets(scmc_catd, MS_fortran, MSG_234,
                                              "data type \"%s\""), symname(s));
            }
	    break;

	default:
	    error( catgets(scmc_catd, MS_fortran, MSG_235,
			       "class %s in fortran_printdecl"), classname(s));
    }
    (*rpt_output)( stdout, "\n");
}

/*
 * List the parameters of a procedure or function.
 * No attempt is made to combine like types.
 */

void fortran_listparams (Symbol s)
{
    register Symbol t;

    (*rpt_output)( stdout, "(");
    for (t = s->chain; t != nil; t = t->chain) {
	/* f_symname is used here to strip out '#' in name... */
	(*rpt_output)(stdout, "%s", f_symname(t));
	if (t->chain != nil) {
	    (*rpt_output)(stdout, ", ");
	}
    }
    (*rpt_output)( stdout, ")");
    if (s->chain != nil) {
	(*rpt_output)(stdout, "\n");
	for (t = s->chain; t != nil; t = t->chain) {
	    /*
	     * Make sure argument is passed by reference or value
	     */
	    if (t->class != REF && t->class != VAR) {
		panic( catgets(scmc_catd, MS_fortran, MSG_239,
			       "unexpected class %d for parameter"), t->class);
	    }
	    printdecl(t, 0);
	}
    } else {
	(*rpt_output)( stdout, "\n");
    }
}

/*
 * Print out the value on the top of the expression stack
 * in the format for the type of the given symbol.
 */

void fortran_printval (Symbol s)
{
    register Symbol t;
    register int len;
    double d1, d2;
    int i;
    Address a;

    switch (s->class) {
	case CONST:
	case TYPE:
	case VAR:
	case TOCVAR:
	case REF:
	case FVAR:
	case PTR:
	case TAG:
        case FPTEE:
	    /* account for padded element when AUTODBL */
	    if (s->class == TYPE && !subarray && is_fortran_padded(s))
	       sp -= (s->symvalue.size - s->symvalue.field.length);
	    fortran_printval(s->type);
	    break;

        case FFUNC:
            a = pop(Address);
            t = whatblock(a);
            if (a == prolloc(t)) {
                PRINTF "%s()", symname(t));
            } else {
                PRINTF "0x%x", a);
            }
            break;

        case FPTR:
            a = pop(Address);
            if (a == 0)
               (*rpt_output)( stdout, "nil");
            else
               (*rpt_output)( stdout, "0x%x", a);
            break;

	case RECORD:
        case PACKRECORD:
	    if (subarray) {
	      printsubarray(0, subdim_tail, array_sym->type, s->class);
	      reset_subarray_vars();
            } else {
	      printrecord(s);
	    }
	    break;

	case FIELD:
	    fortran_printval(s->type);
	    break;

	case CHARSPLAT:
	case FSTRING:
	    if (subarray)
	    {
		fortran_printsubarray(0, subdim_tail,
						 array_sym->type, s->class);
	        reset_subarray_vars();
	    }
            else
	    {
		len = size(s);
		sp -= len;
		if (len == 1 and varIsSet("$hexchars")) 
                  PRINTF "0x%lx", *sp);
		else {
		  PRINTF "'");
		  for (i=0; i<len; i++) {
		     PRINTF "%c", *(sp+i));
		  }
		  PRINTF "'");
		}
	    }
	    break;

	case ARRAY:
	    fortran_printarray(s);
	    break;

	case RANGE:
	    if (subarray) {
		fortran_printsubarray(0, subdim_tail, array_sym->type, RANGE);
	        reset_subarray_vars();
	    } else {
	     if (isspecial(s)) {
		switch (s->symvalue.rangev.lower) {
		    case sizeof(short):
			if (istypename(s->type, "logical*2")) {
			    printlogical(pop(short));
			}
			break;

		    case sizeof(float):
			if (istypename(s->type, "logical")) {
			    printlogical(pop(long));
			} else {
			    prtreal((double)pop(float), true);
			}
			break;

		    case sizeof(double):
			if (istypename(s->type,"complex")) {
			    d1 = (double) pop(float);
			    d2 = (double) pop(float);
			    PRINTF "(");
			    prtreal(d1, true);
			    PRINTF ",");
			    prtreal(d2, true);
			    PRINTF ")");
			} else {
			    prtreal(pop(double), false);
			}
			break;

		    case 2*sizeof(double):
			d1 = pop(double);
			d2 = pop(double);
			PRINTF "(");
			prtreal(d1, false);
			PRINTF ",");
			prtreal(d2, false);
			PRINTF ")");
			break;

		    default:
		      panic( catgets(scmc_catd, MS_fortran, MSG_240,
		      "cannot evaluate \"%d\" byte logical, complex, or real"),
						     t->symvalue.rangev.lower);
		      break;
		}
	    } else if (islogical(s)) {
		switch(size(s)) {
		    case sizeof(char):
			    printlogical(pop(char));
			    break;
		    case sizeof(short):
			    printlogical(pop(short));
			    break;
		    case sizeof(long):
			    printlogical(pop(long));
			    break;
                    case sizeofLongLong:
                    default:
                            printlogical ((pop(LongLong) == (LongLong) 0)
                                           ? 0 : 1);
                            break;
		}
	     } else {
                if (size(s) == sizeofLongLong)
                  printlonglong (pop(LongLong), s);
                else
                  printint(popsmall(s), s);
	     }
	    }
	    break;

	case REAL:
	    if (subarray) {
		   fortran_printsubarray(0, subdim_tail, array_sym, REAL);
	    	   reset_subarray_vars();
	    } else {
		printreal(s);
	    }
	    break;

	case COMPLEX:
	    if (subarray) {
		   fortran_printsubarray(0, subdim_tail, array_sym, COMPLEX);
	    	   reset_subarray_vars();
	    } else {
		printcomplex(s);
	    }
	    break;

	default:
	    if (ord(s->class) > ord(FSTRING)) {
		panic( catgets(scmc_catd, MS_fortran, MSG_241,
				     "printval: bad class %d"), ord(s->class));
	    }
	    error( catgets(scmc_catd, MS_fortran, MSG_242,
			"don't know how to print a %s"), fortran_classname(s));
	    /* NOTREACHED */
    }
}

/*
 * Print out a logical
 */

static printlogical (int i)
{
    if (i == 0) {
	PRINTF ".false.");
    } else {
	PRINTF ".true.");
    }
}

/*
 * Print out a complex
 */

static printcomplex (Symbol s)
{
    int size;
    double realpart, imaginary;
    boolean issingle, isextended;
    quadf qreal, qimag;
    boolean ispaddedextended;

    size = s->symvalue.size;
    issingle = (Boolean) (size == sizeof(double));
    isextended = (Boolean) (size == 4*sizeof(double));
    ispaddedextended = (Boolean) (size == 8*sizeof(double));
    if (!isextended && !ispaddedextended) {
        imaginary = (issingle) ? (double) pop(float) : pop(double); 
        realpart  = (issingle) ? (double) pop(float) : pop(double); 
    } else {
	if (ispaddedextended)
           sp -= 2*sizeof(double);        /* pop out the padding */
	qimag.val[1] = pop(double);
	qimag.val[0] = pop(double);
	if (ispaddedextended)
           sp -= 2*sizeof(double);        /* pop out the padding */
	qreal.val[1] = pop(double);
	qreal.val[0] = pop(double);
    }
    PRINTF "(");
    if (!isextended && !ispaddedextended)
       prtreal(realpart, issingle);
    else 
       printquad(rpt_output, stdout, qreal);
    PRINTF ",");
    if (!isextended && !ispaddedextended)
       prtreal(imaginary, issingle);
    else 
       printquad(rpt_output, stdout, qimag);
    PRINTF ")");
}
/*
 * Print out an int 
 */

static printint (int i, Symbol t)
{
    int t_size;

    t_size = size(t->type);
    if (istypename(t->type, "addr")) {
	PRINTF "0x%lx", i);
    } else if ((t_size > 1) || (istypename(t->type,"integer*1")) ||
				(istypename(t->type,"signed char"))) {
	if (hexints)
	   PRINTF "0x%lx", i);
	else if (octints)
	   PRINTF "0%lo", i);
	else
	   PRINTF "%ld", i);
    } else if (istypename(t->type, "char") || istypename(t->type,"character")){
	if (varIsSet("$hexchars"))
	   PRINTF "0x%lx", i);
	else
	   PRINTF "%c", i);
    } else {
	error( catgets(scmc_catd, MS_fortran, MSG_243,
					  "unknown type in fortran printint"));
    }
}

/*
 * Return the FORTRAN name for the particular class of a symbol.
 */

String fortran_classname (Symbol s)
{
    String str;

    switch (s->class) {
	case REF:
	    str = "dummy argument";
	    break;

	case CONST:
	    str = "parameter";
	    break;

	default:
	    str = classname(s);
    }
    return str;
}

/* reverses the indices from the expr_list; should be folded into buildaref
 * and done as one recursive routine
 */
static Node rev_index (Node here, Node n)
{
 
  register Node i;

  if( here == nil  or  here == n) i=nil;
  else if( here->value.arg[1] == n) i = here;
  else i=rev_index(here->value.arg[1],n);
  return i;
}

/*
 * Evaluate a subscript index. The following explanation was written well
 * after this routines creation, but I'm pretty sure it is right. When
 * a array references are evaluated, they are done one at a time.
 * This means that the first time this routine is called, the
 * bounds of the ENTIRE array are stored since this cannot be
 * done latter on the fly. The middle section checks the
 * current index to make sure it is a legal value. The third
 * and final section either pushes the current offset on the stack
 * for use late on (if the array is still undergoing processing)
 * or calculates the final offset for that set of indexes.
 */

void fortran_evalaref (Symbol s, Address base, long i)
{
    Symbol r, t;
    long lb, ub;
    static long  array_lb[10], array_ub[10], array_indexes[10];
    static short index_count;
    static Boolean initialized = false;
    long array_offset, factor, j, element_size;

   /*                            
    * Starting to process array, initialize tables.
    * Loop through symbol graph and calculate and store the 
    * upper and lower bounds for the array - only done once per array.
    */
    if (!initialized)
    {
       Symbol temp;
       short j = 0;
       index_count = 0;
       if (s->class == ARRAY)
            temp = s;
       else
	    temp = s->type;
       while (temp->class == ARRAY)
       {
	  if ((temp->chain->symvalue.rangev.lowertype != R_CONST) &&
	      (temp->chain->symvalue.rangev.lowertype != R_ADJUST))
	     getbound( temp, temp->chain->symvalue.rangev.lower, 
		       temp->chain->symvalue.rangev.lowertype, &array_lb[j]);
	  else  if (temp->chain->symvalue.rangev.lowertype == R_CONST)
             array_lb[j]   = temp->chain->symvalue.rangev.lower;
	  else  if (temp->chain->symvalue.rangev.lowertype == R_ADJUST)
             array_lb[j]   = 1;

	  if (temp->chain->symvalue.rangev.uppertype != R_CONST)
	     getbound( temp, temp->chain->symvalue.rangev.upper, 
		       temp->chain->symvalue.rangev.uppertype, &array_ub[j]);
	  else
             array_ub[j]   = temp->chain->symvalue.rangev.upper;

          j++;
          temp = temp->type;
       }
       initialized = true;
    }

    /* This middle section checks each subscript on the fly to make sure
       it is in the legal range of that array */

    array_indexes[index_count++] = i; /* store array indexes as they come in */
    t = rtype(s);
    r = t->chain;
    if ( r->symvalue.rangev.lowertype != R_CONST &&
         r->symvalue.rangev.lowertype != R_ADJUST) 
    {
	if (not getbound( s, r->symvalue.rangev.lower, 
                          r->symvalue.rangev.lowertype, &lb)) 
        {
          initialized = false;
          error( catgets(scmc_catd, MS_fortran, MSG_244,
				    "dynamic bounds not currently available"));
	}
    } 
    else 
    {
      lb = r->symvalue.rangev.lower;
    }
    if (r->symvalue.rangev.uppertype == R_ADJUST) {
           ub = MAXINT; 			/* assumed sized arrays */
    } else if ( r->symvalue.rangev.uppertype != R_CONST ) {
	if (not getbound( s, r->symvalue.rangev.upper, 
                          r->symvalue.rangev.uppertype, &ub)) 
        {
          initialized = false;
          error( catgets(scmc_catd, MS_fortran, MSG_244,
			 	    "dynamic bounds not currently available"));
	}
    } 
    else 
    {
      ub = r->symvalue.rangev.upper;
    }
    /* 
     * Check for legal index
     */
    if ((i < lb or i > ub) && (!varIsSet("$unsafebounds")))
    {
        initialized = false;
	error( catgets(scmc_catd, MS_fortran, MSG_246,
						    "subscript out of range"));
    }
    /*
     * Last section: if through with array, calculate offset,
     *               else push base for eventual final calculation.
     */
    if ((t->type->class == TYPE) || (t->type->class == CHARSPLAT) ||
	(t->type->class == FSTRING)) {
       /*
        *  Done with array. Calculate offset for storage or printing.
        */
       factor = 1;
       element_size = size(t->type);
       array_offset = (array_indexes[0] - array_lb[0]) * element_size;
       for (j = 1; j < index_count; j++)
       {
          factor *= (array_ub[j-1] - array_lb[j-1] + 1);
          array_offset += factor*(array_indexes[j] - array_lb[j])*element_size;
       }
       array_offset += base;
       push(long, array_offset);
       initialized = false;
    }
    else 
    {
       /* 
	* More indices to come !! 
	*/
       push(long, base);
    }

}

static fortran_printarray (Symbol a)
{
   struct Bounds { int lb, val, ub; } dim[MAXDIM];

   Symbol sc,st,eltype;
   char buf[50];
   char *subscr;
   int i, ndim, elsize, array_size;
   Boolean done;
   Stack *savesp;

   sp -= array_size = size(a);
   savesp = sp;

   done = 0;
   ndim = 0;
   st = a;
   if (is_string_param(st)) {
       (*rpt_output)(stdout, "'");
       for (i=0; i<array_size; i++)
          (*rpt_output)(stdout, "%c", *(sp+i));
       (*rpt_output)(stdout, "'");
       return;
   }
   for (;;) 
   {
      sc = st->chain;
      if (sc->symvalue.rangev.lowertype != R_CONST &&
          sc->symvalue.rangev.lowertype != R_ADJUST)
      {
         if ( !getbound(a,sc->symvalue.rangev.lower, 
               sc->symvalue.rangev.lowertype, &dim[ndim].lb))
         {
               error( catgets(scmc_catd, MS_fortran, MSG_244,
				   " dynamic bounds not currently available"));
         }
      }
      else 
         dim[ndim].lb = sc->symvalue.rangev.lower;

      if (sc->symvalue.rangev.uppertype == R_ADJUST) { /* Adjustable array */
	Node vardim_node;

	vardim_node = (Node) findvar(identname("$vardim",true));
	if (vardim_node == nil)
           dim[ndim].ub = 10;
	else 
	{
           dim[ndim].ub = vardim_node->value.lcon;
	}
      }
      else if (sc->symvalue.rangev.uppertype != R_CONST) {
         if ( !getbound(a,sc->symvalue.rangev.upper, 
               sc->symvalue.rangev.uppertype, &dim[ndim].ub))
               error( catgets(scmc_catd, MS_fortran, MSG_244,
				   " dynamic bounds not currently available"));
      }
      else 
         dim[ndim].ub = sc->symvalue.rangev.upper;

      ndim++;
      if (st->type->class == ARRAY)
         st=st->type;
      else 
         break;
   }


   eltype = st->type;
   elsize=size(eltype);
   sp += elsize;

   ndim--;
   for (i=0;i<=ndim;i++)
      dim[i].val=dim[i].lb;


   while (!done) 
   {
      buf[0]=',';
      subscr = buf+1;

      for (i=1; i<=ndim; i++)  {              /* set up text to represent */
         sprintf(subscr,"%d,",dim[i].val);    /* row and depth as needed  */
         subscr += strlen(subscr);
      }
      *--subscr = '\0';

      for (i=dim[0].lb; i<=dim[0].ub; i++)    /* print columns */ 
      {
         PRINTF "(%d%s)\t",i,buf);
         fortran_printval(eltype);
         PRINTF "\n");
         sp += 2*elsize;           /* advance sp twice as much since      */
                                   /* eval.pop decrements sp - careful    */
                                   /* of overshoot when done this routine */
      }

      if (ndim == 0) break;        /* only a 1 dimensional array  */
                                   /* so we can now exit          */

      dim[1].val++;                /* increment row subscript     */
      for (i=1; i<=ndim; i++)
         if (dim[i].val > dim[i].ub)
         {
            dim[i+1].val++;              /* increment next subscript */
            if (dim[ndim].val > dim[ndim].ub )      /* done printing */
               done = true;
            else
               dim[i].val = dim[i].lb;   /* reset current subscript  */
         }
    }
    sp -= elsize;    /* reduce stack pointer by current element size to   */
                     /* make up for overshoot in the above code           */
    if ((int)sp % 4) sp-=((int)sp % 4);  /* arrays - realign on boundary  */
    sp = savesp;
}

/*
 * Determine if a data type is a const string, i.e. PARAMETER CHAR*N, etc...
 */

static boolean is_string_param (Symbol t)
{
    boolean b;

    b = (boolean) 
        (t->language == primlang && t->class == ARRAY && t->type == t_char);
    return b;
}

/*
 * Initialize typetable at beginning of a module.
 */

void fortran_modinit ()
{
  addlangdefines(usetype);
}

boolean fortran_hasmodules ()
{
    return false;
}

boolean fortran_passaddr (Symbol param, Symbol exprtype)
{
    /* fortran parameters are passed by addresses */
    return true;
}

cases fortran_foldnames ()
{
    return lower;
}

fortran_printsubarray(long y, struct subdim *ptr,
                      Symbol curbound, Symclass Type)
{
   struct subdim *tptr;
   static long index_list[10];
   static int ndims;
   long i, len;
   Address addr;

   /* upon first entry set ptr to head and get number of dimensions */

   if ( y == 0 )
      for ( ptr = subdim_tail, ndims = 1;
         ptr->back; ptr = ptr->back, ndims++)
         ;
   if (ptr)
   {
      for (i = ptr->lb; i <= ptr->ub; i++)
      {
	  /* process from rightmost subscript leftward */

	  index_list[ ndims - 1 - y ] = i;  
	  fortran_printsubarray(y+1, ptr->next, curbound, Type);
      }
   }
   else
   {
      addr = array_addr;
      PRINTF "(");
      for (i = 0; i < y; i++)
      {
	 if (i+1 >= y)
            PRINTF "%d",index_list[i]);
	 else
            PRINTF "%d,",index_list[i]);
	 fortran_evalaref( curbound, addr, index_list[i]);
	 addr = pop(long);
	 if ((i == 0) && (curbound->class != ARRAY))
	    curbound = curbound->type->type;
	 else
	    curbound = curbound->type;
      }
      rpush(addr, size(curbound));
      PRINTF ") = ");
      subarray = false;
      fortran_printval(curbound);
      subarray = true;
      PRINTF "\n");
   }
}
