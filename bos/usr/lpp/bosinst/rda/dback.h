/* @(#)381.5 src/bos/usr/lpp/bosinst/rda/dback.h, bosinst, bos411, 9428A410j 93/01/11 13:50:07   */
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System installation
 *
 * FUNCTIONS: dback.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* Header file for database save/restore tool */

/* #define CFGDEBUG */

#include   <stdio.h>
#include   <ctype.h>
#include   <cf.h>
/* #include   "cfgdebug.h" */

#define    TRUE    1
#define    FALSE   0
                                        /* States for parser                 */
#define    S_FIND  0                    /* Find a stanza header              */
#define    S_ATTR  1                    /* Get an attribute                  */

                                        /* Define line types                 */
#define    L_COMMENT   0                /* Comment line                      */
#define    L_BLANK     0                /* Blank line                        */
#define    L_STANZA    1                /* Stanza header                     */
#define    L_ATTR      2                /* Attribute line                    */

/*
 * Header for the shell script which will contain the device config commands
 */
#define	   PRESCRIPT      "#!/bin/ksh\n\
export TALLY=\n"


#define	   POSTSCRIPT     "num_cmds=%d\n\
i=0\n\
while [ $i -lt $num_cmds ]\n\
do\n\
	eval \\$cmd_$i > /dev/null\n\
	if [ $? -ne 0 ]\n\
	then\n\
		TALLY=\"$TALLY$(eval echo \\$cmd_$i)\\n\"\n\
	fi\n\
	(( i+=1 ))\n\
done\n\
if [ -n \"$TALLY\" ]\n\
then\n\
	echo Machine not identical to previous configuration.\n\
	echo The following items failed:\n\
	echo $TALLY\n\
fi\n"

struct	sindex
	{
	char		name[NAMESIZE];	/* Stanza name  		*/
	long		oset;		/* File offset to name 		*/
	struct sindex	*next;		/* Pointer to next record 	*/
	};

			/* Structure for mkdev info	*/

struct	dev_struct			/* Structure for mkdev info	*/
	{
	char            name[NAMESIZE];
	char            real_name[NAMESIZE];
	char            location[NAMESIZE];
	char            parent[NAMESIZE];
	char            connwhere[NAMESIZE];
	char            PdDvLn[UNIQUESIZE];
	};

struct  attr_struct			/* Structure for chdev info	*/
	{
	char		attribute[ATTRNAMESIZE];
	char		value[ATTRVALSIZE];
	struct attr_struct *next;
	};

struct	attr_stanza
	{
	char            name[NAMESIZE];
	struct		attr_struct *fields;
	};


/* DEBUGGING AIDS: */

/* #define CFGDEBUG */
#ifdef CFGDEBUG
#include <stdio.h>
#define DEBUG_0(A)			{fprintf(stderr,A);fflush(stderr);}
#define DEBUG_1(A,B)			{fprintf(stderr,A,B);fflush(stderr);}
#define DEBUG_2(A,B,C)			{fprintf(stderr,A,B,C);fflush(stderr);}
#define DEBUG_3(A,B,C,D)		{fprintf(stderr,A,B,C,D);fflush(stderr);}
#define DEBUG_4(A,B,C,D,E)		{fprintf(stderr,A,B,C,D,E);fflush(stderr);}
#define DEBUG_5(A,B,C,D,E,F)		{fprintf(stderr,A,B,C,D,E,F);fflush(stderr);}
#define DEBUG_6(A,B,C,D,E,F,G)		{fprintf(stderr,A,B,C,D,E,F,G);fflush(stderr);}
#define DEBUGELSE			else
#else
#define DEBUG_0(A)
#define DEBUG_1(A,B)
#define DEBUG_2(A,B,C)
#define DEBUG_3(A,B,C,D)
#define DEBUG_4(A,B,C,D,E)
#define DEBUG_5(A,B,C,D,E,F)
#define DEBUG_6(A,B,C,D,E,F,G)
#define DEBUGELSE
#endif
