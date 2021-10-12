static char sccsid[] = "@(#)95	1.10.1.4  src/bos/usr/ccs/bin/as/POWER/as3.c, cmdas, bos411, 9428A410j 3/10/94 09:23:01";
/*
 * COMPONENT_NAME: (CMDAS) Assembler and Macroprocessor 
 *
 * FUNCTIONS: 
 *
 * ORIGINS:  3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifdef CMS
\option MAXD(6000,6000) MAXA(32667,32667);
#endif
/****************************************************************/
/* as assembler.  Pseudo-op and instruction table		*/
/****************************************************************/
# include "as.h"
# include "as0.h"

/****************************************************************/
/* preprocessor hackery to help generate the opcode		*/
/*  the syntax is                                               */
/* OP(name,bit_map,ifmt,ifmt2,ofmt,skel,name1, type_id)         */ 
/*  where 							*/
/*	name  - the pseudo-op, or instruction mnemonic (POWER or */
/*               PowerPC ).                                      */
/*      bit_map - the bit pattern for instructions to indicate  */
/*              the availibility of the inst. in all the archi- */
/*              tectures and implementations. For all the       */
/*              pseudo ops, the bit_map should be '0xFFFF'.     */
/*	ifmt  - the input format, this field is a character    	*/
/*		string where each character stands for an input */
/* 		argument, and the characters tell what type of	*/
/*		argument is expected (see amer.ops file)	*/
/*	ifmt2 - the alternate input format (used for optional 	*/
/*		arguments)					*/
/*	ofmt  - the output format				*/
/*	skel  - the instruction skeleton which needs filling in	*/
/*      name1 - An alternative mnemonic name for the inst. in   */
/*              the name field                                  */
/*      type_id - It indicates the name field holds a POWER or  */
/*              PowerPC mnemonic. '1' = POWER; '2' = PowerPC;   */
/*              '0' = instruction has no alternative mnemonic.  */ 
/*								*/
/* this file is generated using makeops which takes amer.ops as	*/
/* input							*/
/****************************************************************/
/** #define OP(a,b,c,d,e) {a,INST-256,d,"b","c",e}   **/
#define OP(a,b,c,d,e,f,g,h) {a, INST-256,e,b,c,d,f,g,'h'}

/****************************************************************/
/* this is the instruction table				*/
/* keep this table in alphabetical order, binary search is used */
/* the pseudo ops are already in here, and the instructions are	*/
/* included using the instrs file which is the output from 	*/
/* makeops							*/
/* the routine find_opcode searches this table			*/
/****************************************************************/
struct instab instab[] = {
{".align",	IALIGN-256,0,SRC_PW_PPC,0},
{".bb", 	IBB-256,0,SRC_PW_PPC,0},
{".bc", 	IBC-256,0,SRC_PW_PPC,0},
{".bf", 	IBF-256,0,SRC_PW_PPC,0},
{".bi", 	IBI-256,0,SRC_PW_PPC,0},
{".bs", 	IBS-256,0,SRC_PW_PPC,0},
{".byte",	IBYTE-256,0,SRC_PW_PPC,0},
{".comm",	ICOMM-256,0,SRC_PW_PPC,0},
{".copt",	ICOPT-256,0,SRC_PW_PPC,0},
{".csect",	ICSECT-256,0,SRC_PW_PPC,0},
{".double",	IDOUBLE-256,0,SRC_PW_PPC,0},
{".drop",	IDROP-256,0,SRC_PW_PPC,0},
{".dsect",	IDSECT-256,0,SRC_PW_PPC,0},
{".eb", 	IEB-256,0,SRC_PW_PPC,0},
{".ec", 	IEC-256,0,SRC_PW_PPC,0},
{".ef", 	IEF-256,0,SRC_PW_PPC,0},
{".ei", 	IEI-256,0,SRC_PW_PPC,0},
{".es", 	IES-256,0,SRC_PW_PPC,0},
{".extern",	IEXTERN-256,0,SRC_PW_PPC,0},
{".file",	IFILE-256,0,SRC_PW_PPC,0},
{".float",	IFLOAT-256,0,SRC_PW_PPC,0},
{".function",	IFUNCTION-256,0,SRC_PW_PPC,0},
{".globl",	IGLOBAL-256,0,SRC_PW_PPC,0},
{".hash",	IHASH-256,0,SRC_PW_PPC,0},
{".lcomm",	ILCOMM-256,0,SRC_PW_PPC,0},
{".lglobl",	ILGLOBAL-256,0,SRC_PW_PPC,0},
{".line",	ILINE-256,0,SRC_PW_PPC,0},
{".long",      	ILONG-256,0,SRC_PW_PPC,0},
{".machine",	IMACHINE-256,0, SRC_PW_PPC,0},
{".org",	IORG-256,0,SRC_PW_PPC,0},
{".ref",	IREF-256,0,SRC_PW_PPC,0},
{".rename",	IRENAME-256,0,SRC_PW_PPC,0},
{".set",	ISET-256,0,SRC_PW_PPC,0},
{".short",     	ISHORT-256,0,SRC_PW_PPC,0},
{".source",	ISOURCE-256,0, SRC_PW_PPC,0},
{".space",     	ISPACE-256,0,SRC_PW_PPC,0},
{".stabx",      ISTABX-256,0,SRC_PW_PPC,0},
{".string",     ISTRING-256,0,SRC_PW_PPC,0},
{".tbtag",	ITBTAG-256,0,SRC_PW_PPC,0},
{".tc",		ITC-256,0,SRC_PW_PPC,0},
{".toc",       ITOC-256,0,SRC_PW_PPC,0},
{".tocof",     ITOCOF-256,0,SRC_PW_PPC,0},
{".using",	IUSING-256,0,SRC_PW_PPC,0},
{".vbyte",	IVBYTE-256,0,SRC_PW_PPC,0},
{".xaddr",	IXADDR-256,0,SRC_PW_PPC,0},
{".xline",	IXLINE-256,0,SRC_PW_PPC,0},
#include "instrs"
0 };

int ninstab = (sizeof(instab)/sizeof(struct instab));
char *verid=versionID;		/* version id from instrs*/
