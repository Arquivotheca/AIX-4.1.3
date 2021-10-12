/* @(#)51	1.27  src/bos/usr/include/cf.h, cmdcfg, bos411, 9428A410j 6/9/94 16:42:52 */
#ifndef _H_CF
#define _H_CF
/*
 * COMPONENT_NAME: (CMDCFG) Generic config support cmd
 *
 * FUNCTIONS: cf.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*---------------------- flag definitions for findmcode -------------------*/
/*									   */
/*  VERSIONING = find highest version of microcode			   */
/*  ABSOLUTE   = input microcode name is absolute name (do not try to      */
/*               find the highest version.)				   */
/*  BASENAME   = input microcode name is basename only and FIRST match     */
/*               on that part of a file name should be returned.           */
/*               (equivalent to search for mcode_name*)                    */
#include <sys/types.h>

#define VERSIONING 0
#define ABSOLUTE   1
#define BASENAME   2

/*-------------------------------- message catalog ------------------------*/
/* NOTE - the default for these messages reside in com/lib/cfg/cfdefmsg.c  */
#define CFG_MSG_CATALOG		"cmdcfg.cat"
#define CFG_MSG_SET		1
#define CFG_ERR_SET		2
#define CFG_METH_SET		3

/*--------------------------- default messages ----------------------------*/
extern char *meth_err_msg[];

/*----------------------------------- error messages ----------------------*/
#define E_OK					0	
#define E_SYSTEM				1	
#define E_ODMINIT				2	
#define E_ODMLOCK				3	
#define E_ODMOPEN				4	
#define E_ODMCLOSE			5	
#define E_ODMGET				6	
#define E_ODMUPDATE			7	
#define E_ODMADD				8	
#define E_ODMDELETE			9	
#define E_ODMRUNMETHOD		10	
#define E_ARGS					11	
#define E_OPEN					12	
#define E_LNAME				13	
#define E_TYPE					14	
#define E_PARENT				15	
#define E_PARENT2				16
#define E_INVATTR				17
#define E_ATTRVAL				18
#define E_PFLAG				19
#define E_TFLAG				20	 
#define E_CHGCONNECT			21
#define E_INVCONNECT			22	 
#define E_NOCuDv				23	 
#define E_NOPdDv				24	 
#define E_NOCuDvPARENT		25	 
#define E_ALREADYDEF			26	 
#define E_DEVSTATE			27	 
#define E_PARENTSTATE		28	 
#define E_CHILDSTATE			29	 
#define E_DEPSTATE			30	 
#define E_AVAILCONNECT		31	 
#define E_NODEPENDENT		32	 
#define E_NOATTR				33	 
#define E_BADATTR				34	 
#define E_NOPdOBJ				35	 
#define E_NOCuOBJ				36	 
#define E_MAKENAME			37	 
#define E_LOADEXT				38	 
#define E_UNLOADEXT			39	 
#define E_CFGINIT				40	 
#define E_CFGTERM				41	 
#define E_MAJORNO				42	 
#define E_MINORNO				43	 
#define E_MKSPECIAL			44	 
#define E_DDS					45	 
#define E_NOUCODE				46	 
#define E_DEVACCESS			47	 
#define E_UCODE				48	 
#define E_VPD					49	 
#define E_NODETECT			50	 
#define E_WRONGDEVICE		51	 
#define E_BUSRESOURCE		52
#define E_SYSCONFIG			53
#define E_MALLOC				54	
#define E_RELDEVNO			55
#define E_DEVNO_INUSE		56
#define E_STAT					57
#define E_RMSPECIAL			58
#define E_SYMLINK				59
#define E_FORK					60
#define E_FINDCHILD			61
#define E_BUSY					62
#define E_NAME					63
#define E_UNLOADAIO				64
#define E_INSTNUM				65
#define E_PARENT3                               66
#define E_NOSLOTS                               67
#define E_LAST_ERROR                  		68

/*WARNING!!!!! - E_LAST_ERROR must be the last message - the high level */
/*   commands require this.  If you need to add a new message, put it   */
/*   in before E_LAST_ERROR, increment the value of E_LAST_ERROR,       */
/*   update the default messages in: 					*/
/*   		src/bos/usr/ccs/lib/libcfg/cfdefmsg.c,			*/
/*   and update the message catalog in:					*/
/*		src/bos/usr/sbin/lsdev/cmdcfg.msg			*/

/* Prototypes */

int		genmajor();
int		genseq();
int 	relmajor();
int 	*getminor();
int		*genminor();
int 	reldevno();
mid_t	loadext();
int		geninst();
int		relinst();
int		lsinst();
int		attrval();
struct CuAt *getattr();
int		putattr();

#endif	/* _H_CF */

