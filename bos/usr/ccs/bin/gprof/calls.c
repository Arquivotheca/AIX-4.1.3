static char sccsid[] = "@(#)52	1.4  src/bos/usr/ccs/bin/gprof/calls.c, cmdstat, bos41B, 9504A 12/21/94 13:42:03";
/*
* COMPONENT_NAME: (CMDSTAT) gprof
*
* FUNCTIONS: calls
*
* ORIGINS: 26, 27
*
* This module contains IBM CONFIDENTIAL code. -- (IBM
* Confidential Restricted when combined with the aggregated
* modules for this product)
*                  SOURCE MATERIALS
* (C) COPYRIGHT International Business Machines Corp. 1989, 1994
* All Rights Reserved
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
* Copyright (c) 1980 Regents of the University of California.
* All rights reserved.  The Berkeley software License Agreement
* specifies the terms and conditions for redistribution.
*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
static char sccsid[] = "calls.c	5.1 (Berkeley) 6/4/85";
*/

#include "gprof_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_GPROF,n,s) 

#include	"gprof.h"

    /*
     *	a namelist entry to be the child of indirect calls
     */
static nltype	indirectchild = {
	"(*)" ,				/* the name */
	(unsigned long) 0 ,		/* the pc entry point */
	(unsigned long) 0 ,		/* entry point aligned to histogram */
	(double) 0.0 ,			/* ticks in this routine */
	(double) 0.0 ,			/* cumulative ticks in children */
	(long) 0 ,			/* how many times called */
	(long) 0 ,			/* how many calls to self */
	(double) 1.0 ,			/* propagation fraction */
	(double) 0.0 ,			/* self propagation time */
	(double) 0.0 ,			/* child propagation time */
	(bool) 0 ,			/* print flag */
	(int) 0 ,			/* index in the graph list */
	(int) 0 , 			/* graph call chain top-sort order */
	(int) 0 ,			/* internal number of cycle on */
	(struct nl *) &indirectchild ,	/* pointer to head of cycle */
	(struct nl *) 0 ,		/* pointer to next member of cycle */
	(arctype *) 0 ,			/* list of caller arcs */
	(arctype *) 0 			/* list of callee arcs */
    };

static operandenum
operandmode( modep )
    struct modebyte	*modep;
{
    long	usesreg = modep -> regfield;
    
    switch ( modep -> modefield ) {
	case 0:
	case 1:
	case 2:
	case 3:
	    return LITERAL;
	case 4:
	    return INDEXED;
	case 5:
	    return REG;
	case 6:
	    return REGDEF;
	case 7:
	    return AUTODEC;
	case 8:
	    return ( usesreg != PC ? AUTOINC : IMMEDIATE );
	case 9:
	    return ( usesreg != PC ? AUTOINCDEF : ABSOLUTE );
	case 10:
	    return ( usesreg != PC ? BYTEDISP : BYTEREL );
	case 11:
	    return ( usesreg != PC ? BYTEDISPDEF : BYTERELDEF );
	case 12:
	    return ( usesreg != PC ? WORDDISP : WORDREL );
	case 13:
	    return ( usesreg != PC ? WORDDISPDEF : WORDRELDEF );
	case 14:
	    return ( usesreg != PC ? LONGDISP : LONGREL );
	case 15:
	    return ( usesreg != PC ? LONGDISPDEF : LONGRELDEF );
    }
    /* NOTREACHED */
}

static char *
operandname( mode )
    operandenum	mode;
{
    
    switch ( mode ) {
	case LITERAL:
	    return "literal";
	case INDEXED:
	    return "indexed";
	case REG:
	    return "register";
	case REGDEF:
	    return "register deferred";
	case AUTODEC:
	    return "autodecrement";
	case AUTOINC:
	    return "autoincrement";
	case AUTOINCDEF:
	    return "autoincrement deferred";
	case BYTEDISP:
	    return "byte displacement";
	case BYTEDISPDEF:
	    return "byte displacement deferred";
	case BYTEREL:
	    return "byte relative";
	case BYTERELDEF:
	    return "byte relative deferred";
	case WORDDISP:
	    return "word displacement";
	case WORDDISPDEF:
	    return "word displacement deferred";
	case WORDREL:
	    return "word relative";
	case WORDRELDEF:
	    return "word relative deferred";
	case IMMEDIATE:
	    return "immediate";
	case ABSOLUTE:
	    return "absolute";
	case LONGDISP:
	    return "long displacement";
	case LONGDISPDEF:
	    return "long displacement deferred";
	case LONGREL:
	    return "long relative";
	case LONGRELDEF:
	    return "long relative deferred";
    }
    /* NOTREACHED */
}

static long
operandlength( modep )
    struct modebyte	*modep;
{
    
    switch ( operandmode( modep ) ) {
	case LITERAL:
	case REG:
	case REGDEF:
	case AUTODEC:
	case AUTOINC:
	case AUTOINCDEF:
	    return 1;
	case BYTEDISP:
	case BYTEDISPDEF:
	case BYTEREL:
	case BYTERELDEF:
	    return 2;
	case WORDDISP:
	case WORDDISPDEF:
	case WORDREL:
	case WORDRELDEF:
	    return 3;
	case IMMEDIATE:
	case ABSOLUTE:
	case LONGDISP:
	case LONGDISPDEF:
	case LONGREL:
	case LONGRELDEF:
	    return 5;
	case INDEXED:
	    return 1+operandlength( (struct modebyte *) ((char *) modep) + 1 );
    }
    /* NOTREACHED */
}

static unsigned long
reladdr( modep )
    struct modebyte	*modep;
{
    operandenum	mode = operandmode( modep );
    char	*cp;
    short	*sp;
    long	*lp;

    cp = (char *) modep;
    cp += 1;			/* skip over the mode */
    switch ( mode ) {
	default:
	    fprintf( stderr , MSGSTR(NOTRELADDR, "[reladdr] not relative address\n") ); /*MSG*/
	    return (unsigned long) modep;
	case BYTEREL:
	    return (unsigned long) ( cp + sizeof *cp + *cp );
	case WORDREL:
	    sp = (short *) cp;
	    return (unsigned long) ( cp + sizeof *sp + *sp );
	case LONGREL:
	    lp = (long *) cp;
	    return (unsigned long) ( cp + sizeof *lp + *lp );
    }
}

findcalls( parentp , p_lowpc , p_highpc )
    nltype		*parentp;
    unsigned long	p_lowpc;
    unsigned long	p_highpc;
{
    unsigned char	*instructp;
    long		length;
    nltype		*childp;
    operandenum		mode;
    operandenum		firstmode;
    unsigned long	destpc;

    if ( textspace == 0 ) {
	return;
    }
    if ( p_lowpc < s_lowpc ) {
	p_lowpc = s_lowpc;
    }
    if ( p_highpc > s_highpc ) {
	p_highpc = s_highpc;
    }
#   ifdef _DEBUG
	if ( debug & CALLSDEBUG ) {
	    printf( "[findcalls] %s: 0x%x to 0x%x\n" ,
		    parentp -> name , p_lowpc , p_highpc );
	}
#   endif _DEBUG
    for (   instructp = textspace + p_lowpc ;
	    instructp < textspace + p_highpc ;
	    instructp += length ) {
	length = 1;
	if ( *instructp == CALLS ) {
		/*
		 *	maybe a calls, better check it out.
		 *	skip the count of the number of arguments.
		 */
#	    ifdef _DEBUG
		if ( debug & CALLSDEBUG ) {
		    printf( "[findcalls]\t0x%x:calls" , instructp - textspace );
		}
#	    endif _DEBUG
	    firstmode = operandmode( (struct modebyte *) (instructp+length) );
	    switch ( firstmode ) {
		case LITERAL:
		case IMMEDIATE:
		    break;
		default:
		    goto botched;
	    }
	    length += operandlength( (struct modebyte *) (instructp+length) );
	    mode = operandmode( (struct modebyte *) ( instructp + length ) );
#	    ifdef _DEBUG
		if ( debug & CALLSDEBUG ) {
		    printf( "\tfirst operand is %s", operandname( firstmode ) );
		    printf( "\tsecond operand is %s\n" , operandname( mode ) );
		}
#	    endif _DEBUG
	    switch ( mode ) {
		case REGDEF:
		case BYTEDISPDEF:
		case WORDDISPDEF:
		case LONGDISPDEF:
		case BYTERELDEF:
		case WORDRELDEF:
		case LONGRELDEF:
			/*
			 *	indirect call: call through pointer
			 *	either	*d(r)	as a parameter or local
			 *		(r)	as a return value
			 *		*f	as a global pointer
			 *	[are there others that we miss?,
			 *	 e.g. arrays of pointers to functions???]
			 */
		    addarc( parentp , &indirectchild , (long) 0 );
		    length += operandlength(
				(struct modebyte *) ( instructp + length ) );
		    continue;
		case BYTEREL:
		case WORDREL:
		case LONGREL:
			/*
			 *	regular pc relative addressing
			 *	check that this is the address of 
			 *	a function.
			 */
		    destpc = reladdr( (struct modebyte *) (instructp+length) )
				- (unsigned long) textspace;
		    if ( destpc >= s_lowpc && destpc <= s_highpc ) {
			childp = nllookup( destpc );
#			ifdef _DEBUG
			    if ( debug & CALLSDEBUG ) {
				printf( "[findcalls]\tdestpc 0x%x" , destpc );
				printf( " childp->name %s" , childp -> name );
				printf( " childp->value 0x%x\n" ,
					childp -> value );
			    }
#			endif _DEBUG
			if ( childp -> value == destpc ) {
				/*
				 *	a hit
				 */
			    addarc( parentp , childp , (long) 0 );
			    length += operandlength( (struct modebyte *)
					    ( instructp + length ) );
			    continue;
			}
			goto botched;
		    }
			/*
			 *	else:
			 *	it looked like a calls,
			 *	but it wasn't to anywhere.
			 */
		    goto botched;
		default:
		botched:
			/*
			 *	something funny going on.
			 */
#		    ifdef _DEBUG
			if ( debug & CALLSDEBUG ) {
			    printf( "[findcalls]\tbut it's a botch\n" );
			}
#		    endif _DEBUG
		    length = 1;
		    continue;
	    }
	}
    }
}
