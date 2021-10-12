static char sccsid[] = "@(#)56  1.3  src/bos/usr/ccs/bin/gprof/header.c, cmdstat, bos41B, 9504A 12/21/94 13:42:11";
/*
 * COMPONENT_NAME: (CMDSTAT) gprof
 *
 * FUNCTIONS: printheader
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
 */

#define NUM_FLAT	29
#define NUM_CALLG	105

#include "gprof_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_GPROF,n,s) 

#include "gprof.h"

/****************************************************************************/
/*                            FLAT PROFILE HEADER                           */
/****************************************************************************/

static char *flat_header[NUM_FLAT] = {
"",
"flat profile:",
"",
" %         the percentage of the total running time of the",
"time       program used by this function.",
"",
"cumulative a running sum of the number of seconds accounted",
" seconds   for by this function and those listed above it.",
"",
" self      the number of seconds accounted for by this",
"seconds    function alone.  This is the major sort for this",
"           listing.",
"",
"calls      the number of times this function was invoked, if",
"           this function is profiled, else blank.",
"",
" self      the average number of milliseconds spent in this",
"ms/call    function per call, if this function is profiled,",
"	   else blank.",
"",
" total     the average number of milliseconds spent in this",
"ms/call    function and its descendents per call, if this ",
"	   function is profiled, else blank.",
"",
"name       the name of the function.  This is the minor sort",
"           for this listing. The index shows the location of",
"	   the function in the gprof listing. If the index is",
"	   in parenthesis it shows where it would appear in",
"	   the gprof listing if it were to be printed."};

/****************************************************************************/
/*                           CALLG PROFILE HEADER                           */
/****************************************************************************/

static char *callg_header[NUM_CALLG] = {
"",
"call graph profile:",
"          The sum of self and descendents is the major sort",
"          for this listing.",
"",
"          function entries:",
"",
"index     the index of the function in the call graph",
"          listing, as an aid to locating it (see below).",
"",
"%time     the percentage of the total time of the program",
"          accounted for by this function and its",
"          descendents.",
"",
"self      the number of seconds spent in this function",
"          itself.",
"",
"descendents",
"          the number of seconds spent in the descendents of",
"          this function on behalf of this function.",
"",
"called    the number of times this function is called (other",
"          than recursive calls).",
"",
"self      the number of times this function calls itself",
"          recursively.",
"",
"name      the name of the function, with an indication of",
"          its membership in a cycle, if any.",
"",
"index     the index of the function in the call graph",
"          listing, as an aid to locating it.",
"",
"",
"",
"          parent listings:",
"",
"self*     the number of seconds of this function's self time",
"          which is due to calls from this parent.",
"",
"descendents*",
"          the number of seconds of this function's",
"          descendent time which is due to calls from this",
"          parent.",
"",
"called**  the number of times this function is called by",
"          this parent.  This is the numerator of the",
"          fraction which divides up the function's time to",
"          its parents.",
"",
"total*    the number of times this function was called by",
"          all of its parents.  This is the denominator of",
"          the propagation fraction.",
"",
"parents   the name of this parent, with an indication of the",
"          parent's membership in a cycle, if any.",
"",
"index     the index of this parent in the call graph",
"          listing, as an aid in locating it.",
"",
"",
"",
"          children listings:",
"",
"self*     the number of seconds of this child's self time",
"          which is due to being called by this function.",
"",
"descendent*",
"          the number of seconds of this child's descendent's",
"          time which is due to being called by this",
"          function.",
"",
"called**  the number of times this child is called by this",
"          function.  This is the numerator of the",
"          propagation fraction for this child.",
"",
"total*    the number of times this child is called by all",
"          functions.  This is the denominator of the",
"          propagation fraction.",
"",
"children  the name of this child, and an indication of its",
"          membership in a cycle, if any.",
"",
"index     the index of this child in the call graph listing,",
"          as an aid to locating it.",
"",
"",
"",
"          * these fields are omitted for parents (or",
"          children) in the same cycle as the function.  If",
"          the function (or child) is a member of a cycle,",
"          the propagated times and propagation denominator",
"          represent the self time and descendent time of the",
"          cycle as a whole.",
"",
"          ** static-only parents and children are indicated",
"          by a call count of 0.",
"",
"",
"",
"          cycle listings:",
"          the cycle as a whole is listed with the same",
"          fields as a function entry.  Below it are listed",
"          the members of the cycle, and their contributions",
"          to the time and call counts of the cycle."};

/****************************************************************************/
/*                           PRINT HEADER ROUTINE                           */
/****************************************************************************/

printheader(header)
int header;
{
    int i, num, set, first;
    char **list;

    switch(header) {
	case FLAT_HEADER:
	    list = flat_header;
	    num = NUM_FLAT;
	    set = MS_FLAT;
	    first = MF1;
	    break;
	case CALLG_HEADER:
	    list = callg_header;
	    num = NUM_CALLG;
	    set = MS_CALLG;
	    first = MC1;
	    break;
	default:
	    fprintf(stderr, MSGSTR(NOSUCH, "gprof: No such profile header.\n"));
	    exit(1);
    }
    for(i=0; i<num; i++)
	printf("%s\n", catgets(catd,set,i+first,list[i]));
    printf("\n");
}

