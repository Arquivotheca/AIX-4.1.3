/* @(#)80	1.11  src/bos/usr/include/toc.h, cmdld, bos411, 9428A410j 6/16/90 00:15:17 */
/* (#)toc.h	1.3 - 87/08/19 - 10:53:09  	*/
/*
 * COMPONENT_NAME: SYSDB
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* ******************************************************************** */
#ifndef _H_TOC
#define _H_TOC	1

struct exec {
      unsigned short  a_magic;		/* a.out magic 			*/
      unsigned char   a_flags;      	/* rebind and charset flag 	*/
      unsigned char   a_version;    	/* version number		*/
      long            a_orgaddr;    	/* starting address of text 	*/
      long            a_ctladdr;    	/* address of $ctlist (sect 2)	*/
      long            a_tocaddr;    	/* address of $toc (sect 2) 	*/
      long            a_commonlen;  	/* length of common at end of 	*/
					/* section 2 			*/
      long            a_entryaddr;  	/* address of entry point 	*/
					/* (section 1 or 2) 		*/
      unsigned char   a_entrytype[8];	/* hash type of entry point 	*/
      unsigned char   a_moduletype[2];  /* '1L' not reusable,		*/
					/* 'SR' serially reusable,	*/
                                      	/* 'RE' module is reentrant,	*/
                                      	/* 'RC' recursively callable 	*/
      unsigned short  a_maxalign;   	/* log2 most stringent csect 	*/
					/* alignment		 	*/
      long 	      a_rteaddr;	/*address of $rte (section 2) 	*/
					/* (for debugger) 		*/
      long            a_spare1;     	/* spare 			*/
      long            a_spare2;     	/* spare 			*/
      long            a_spare3;     	/* spare 			*/
      unsigned short  a_spare4;     	/* spare 			*/
      unsigned short  a_objectmach;     /* spare 			*/
      long            a_sectlen[4]; 	/* total section size in bytes 	*/
      long            a_strlen[2];  	/* characters of string storage */
      long            a_esdcount[2]; 	/* number of esd entries 	*/
      long            a_rldcount[2]; 	/* number of rld entries 	*/
   };
	/* ----- this section will define the macros ------*/

#define ROUND(X,Y)	(((X)+(Y))&~(Y))
#define A_READONLY(X)	(sizeof(X))
#define A_READWRITE(X)	(A_READONLY(X)+(X).a_sectlen[0])
#define A_LOADSTRPOS(X) (A_READONLY(X)+(X).a_sectlen[0]+(X).a_sectlen[1])
#define A_LOADESDPOS(X) ROUND((A_LOADSTRPOS(X)+(X).a_strlen[0]), 3)
#define A_LOADRLDPOS(X) ((A_LOADESDPOS(X)+((X).a_esdcount[0]*ESDSZ)))
#define A_BINDSTRPOS(X) ((A_LOADRLDPOS(X)+((X).a_rldcount[0]*RLDSZ)))
#define A_BINDESDPOS(X) ROUND((A_BINDSTRPOS(X)+(X).a_strlen[1]), 3)
#define A_BINDRLDPOS(X) ((A_BINDESDPOS(X)+((X).a_esdcount[1]*ESDSZ)))

#define A_NOBIND	0x01
#define A_CHARSET	0x02

#define VER_CHARSET(p)  (p).a_flags&0x02
#define VER_BIND(p)     (p).a_flags&0x01
#define IS_EBCDIC(p)   (!(VER_CHARSET(p)))
#define SET_EBCDIC(X)	(X).a_flags &= ~A_CHARSET;
#define SET_ASCII(X)    (X).a_flags |= A_CHARSET;
#define EBCDIC		0
#define ASCII   	1
#define REBIND		0
#define NOREBIND 	1
#define AOUT_MAGIC	0x0000		/* who uses this ? */
#define A_GPOFF_MAGIC	0x0103		/* exec support of this may be added */
#define A_TOC_MAGIC	0x0000		/* exec currently supports only this */
#define VER6		6
#define A_VERSION	6
#define A_MAGIC		0x0000		/* who uses this ? */

/* add for XTOC compatibility				*/
#define A_NONE	0
#define A_ROMP	800
#define A_AMER 	1776

#define A_AIWS 	A_ROMP
#define ROMP	A_ROMP
#define AMERICA A_AMER
#define UNKMACH	A_NONE

#define LOG2_WALIGN	2
#define LOG2_DALIGN	3
/*
   External Symbol Dictionary

   ESD types:
        ER  external reference
        SD  section definition
        LD  label definition
        CM  common
        EM  Error Message - Parameter hash mismatch (Special Internal)

   ESD storage classes:
        PR  program
        XO  extended opcode
        SV  supervisor call
        GL  cross-module glue
        RO  read only data
        DB  debug
        TC  table of contents
        UA  unavailable
        RW  read write data
        BS  uninitialized static
*/

#define N_TYPE  (0xf0000000)
#define T_ER    0
#define T_SD    1
#define T_LD    2
#define T_CM    3
/* #define T_EM    4		*/
/* #define T_US	5	unset	*/
#define T_NUM   6
#define N_CLASS (0xf0000000)
#define C_PR    0
#define C_RO    1
#define C_DB    2
#define C_TC    3
#define C_UA    4
#define C_RW    5
#define C_GL    6
#define C_XO    7
#define C_SV    8
#define C_BS    9
#define C_NUM  	10   	/* number of classes */

/* External Symbol Dictionary */
#define ESD	struct esd
#define ESDSZ	sizeof(struct esd)
struct esd   {
      long        e_addr;   /* address of symbol                 */
      long        e_len;    /* storage in bytes                  */
                            /* if LD, containing SD	         */
			    /* if ER, 0				 */
      long        e_typnam; /* 3rd word of ESD                   */
	                    /* 0-3  ESD type                     */
			    /*      if LD - type of containing SD*/
                            /* 4-31 Offset of name in string stg */
      long        e_stgdcl; /* 0-3  Storage Class	         */
		            /* 4-31 Offset of parm hash in      */
		            /*       string storage             */
      long        e_algntag;/* 0-4  log 2 of alignement	 */
		            /* 5-31 Offset of tag in string stg */
};

#if 0
 This stuff was moved to ldr/loader.h
#define   R_POS     0    /* A(sym)        positive relocation */
#define   R_NEG     1    /* A(-sym)       negative relocation */
#define   R_REL     2    /* A(sym-*)      relative to self */
#define   R_TOC     3    /* A(sym-TOC)    relative to TOC */
#define   R_ROMP    4    /* A((sym-*)/2)  for romp iar rel branch */
#define   R_GL	    5 	 /* A(TOC of sym) sym ER (cross module glue) */
#define   R_TCL     6    /* A(TOC of sym) local toc */

/* defines to be compatiable with XTOC				*/
#define	R_BR	R_POS
#define R_RBR	R_REL
#endif /* 0 */

/* ReLocation Dictionary */
#define RLD	struct rld
#define RLDSZ 	sizeof(struct rld)
struct rld {
      long        r_word1;     /* 0-3   RLD type                    */
			       /* 4-8   Length of adcon -1 (in bits)*/
			       /* 9     flag for signed adcon       */
			       /* 10-31 ESD Symbol index            */
      long        r_addr;      /* address of the adcon              */
};

struct strt {
    unsigned char s_len;	/* length of string field */
    char          s_str[1];	/*                        */
    };
#define S_LENSZ	1		/* size of string length field		*/

#endif	/* _H_TOC */
