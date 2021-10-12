/* @(#)60	1.8  src/bos/kernel/sys/syspest.h, sysdb, bos411, 9428A410j 1/12/94 12:35:48 */
#ifndef _H_SYSPEST
#define _H_SYSPEST

/*
 * COMPONENT_NAME: (SYSDB)  Kernel debug macros
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1984
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifdef DEBUG

/*
 *  This version of assert causes the traditional printf/panic and
 *  is intendend for environments where source is available.
 */
#define assert(p) {if(!(p)){printf("[%s #%d]\n",__FILE__,__LINE__); panic("assert(p)");}} 
#define ASSERT(p) assert(p)
	
	      /* suggested values for "DBUGL" */
#define BUGNFO 0x1       /* information: e.g., file open/close */
#define BUGACT 0x2       /* statement of program action */
#define BUGNTF 0x3       /* interfaces: names, data, return codes */
#define BUGNTA 0x4       /* interfaces for subordinate routines */
#define BUGNTX 0x5       /* detailed interface data */
#define BUGGID 0x99      /* gory internal detail */

#define BUGVDEF(bugvar,lev) \
    int bugvar = (lev);

#define BUGXDEF(bugvar) \
    extern bugvar;

#define BUGPR(prspec) \
    {printf("[%s #%d]  ", __FILE__, __LINE__); \
     printf prspec;}
#define BUGLPR(bugvar,buglev,prspec) {if ((buglev) <= (bugvar)) \
    {printf("[%s #%d]  ", __FILE__, __LINE__); \
     printf prspec;}}

#define BUGC(expr,comnt) {if (!(expr)) \
    {printf("[%s #%d]  %s\n", \
    __FILE__, __LINE__,comnt);}}
#define BUGLC(bugvar,buglev,expr,comnt) {\
	if ((!(expr)) && ((buglev) <= (bugvar))) \
    		{printf("[%s #%d]  %s\n", __FILE__, __LINE__,comnt);}}

#define BUGX(expr,funct) {if (!(expr)) \
    {printf("[%s #%d]\n", \
    __FILE__, __LINE__);funct;}}

#define BUGLX(bugvar,buglev,expr,funct) {\
	if ((!(expr)) && ((buglev) <= (bugvar))) \
    		{printf("[%s #%d]\n", __FILE__, __LINE__);funct;}}

#define BUGCX(expr,comnt,funct) { \
	if (!(expr))\
    		{printf("[%s #%d]  %s\n", __FILE__, __LINE__,comnt);funct;}}

#define BUGLCX(bugvar,buglev,expr,comnt,funct) { \
	if ((!(expr)) && ((buglev) <= (bugvar))) \
    		{printf("[%s #%d]  %s\n", __FILE__, __LINE__,comnt);funct;}}

#define BUGVT(variable,type)  \
    {printf("[%s #%d]  variable = %type\n", __FILE__, __LINE__,(variable));}

#define BUGLVT(bugvar,buglev,variable,type)  {\
	if ((buglev) <= (bugvar)) \
    		{printf("[%s #%d]  variable = %type\n", \
		__FILE__, __LINE__,(variable));}}

#define BUGRT(comnt,variable,type)  \
    {printf("[%s #%d]  %s...variable = %type\n", \
    __FILE__, __LINE__,comnt,(variable));}

#define BUGLRT(bugvar,buglev,comnt,variable,type)  {\
	if ((buglev) <= (bugvar)) \
    		{printf("[%s #%d]  %s...variable = %type\n",\
		 __FILE__, __LINE__,comnt,(variable));}}

#define BUGDM(comnt,dumpaddr,dumpl) \
    {printf("[%s #%d]  %s\n", \
    __FILE__, __LINE__,comnt); xdump((dumpaddr),(dumpl));}

#define BUGLDM(bugvar,buglev,comnt,dumpaddr,dumpl) {if (buglev <= bugvar) \
    {printf("[%s #%d]  %s\n", \
    __FILE__, __LINE__,comnt); xdump((dumpaddr),(dumpl));}}

#define	BUGS1(bugvar,buglev,s1) {\
	if ((buglev) <= (bugvar))\
		printf("[%s #%d] s1 =  %d\n", __FILE__, __LINE__, (s1));}

#define	BUGS2(bugvar,buglev,s1,s2) {\
	if ((buglev) <= (bugvar))\
	printf("[%s #%d] s1 = %x s2 = %x\n", __FILE__, __LINE__, (s1),(s2));}

#define BUGFUNCT(funct) {funct;}
#define BUGLFUNCT(bugvar,buglev,expr,funct)\
	 {if ((buglev) <= (bugvar)) funct;}

#else

/*
 *  This version of assert causes the compiler to generate an inline
 *  trap instead of a printf/panic. It is intended for shipped code
 *  where source is not available.
 */

#define assert(p) {__assert2(__assert1((unsigned)(p),0,99));}

/*
 * Debug assert resolves to nothing for the base kernel.  For kernel
 * extensions use the old definition for source compatibility
 */
#ifdef _KERNSYS
#define ASSERT(p)
#else
#define ASSERT(p) {(p);}
#endif


#define BUGVDEF(bugvar,lev)
#define BUGXDEF(bugvar)
#define BUGPR(prspec)
#define BUGLPR(bugvar,buglev,prspec)
#define BUGC(expr,comnt)
#define BUGLC(bugvar,buglev,expr,comnt)
#define BUGX(expr,funct)
#define BUGLX(bugvar,buglev,expr,funct)
#define BUGVT(variable,type)
#define BUGLVT(bugvar,buglev,variable,type)
#define BUGRT(comnt,variable,type)
#define BUGLRT(bugvar,buglev,comnt,variable,type)
#define BUGCX(expr,comnt,funct)
#define BUGLCX(bugvar,buglev,expr,comnt,funct)
#define BUGDM(comnt,dumpaddr,dumpl)
#define BUGLDM(bugvar,buglev,comnt,dumpaddr,dumpl)
#define	BUGS1(bugvar,buglev,s1)
#define	BUGS2(bugvar,buglev,s1,s2)
#define BUGFUNCT(funct)
#define BUGLFUNCT(bugvar,buglev,expr,funct)

#endif /* DEBUG */

#endif /* _H_SYSPEST */
