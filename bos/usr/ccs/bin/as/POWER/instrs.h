/* @(#)97	1.4.1.2  src/bos/usr/ccs/bin/as/POWER/instrs.h, cmdas, bos411, 9428A410j 11/12/93 13:49:00 */
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
/****************************************************************/
/* specific header for as	assembler			*/
/****************************************************************/

#define MACH	A_AMER	/* machine id used in header		*/

/* instruction types 						*/
#define A	0	/* A form instruction with all 4 args	*/
#define AB	1	/* A form w/ arg3 in 4th slot, no arg4	*/
#define AC	2	/* A form w/ 4th slot empty, no arg4	*/
#define DB	3	/* bc and bcl instructions		*/
#define DC	4	/* SVCA instructions			*/
#define DE	5	/* bXX & bXXl, branch relative ext mne	*/
#define DF	6	/* bbt, bbf, bbtl, bbfl			*/
#define DI	7	/* D form, imm disp (rt|bf|to),ra,si	*/
#define DM	8	/* D form, si = -si			*/
#define DN	9	/* bbta, bbfa, bbtla, bbfla		*/
#define DO	10	/* bXXa & bXXla				*/
#define DP	11	/* bca and bcla				*/
#define DR	12	/* D form, ra and rs reversed		*/
#define DS	13	/* D form, rs,d(ra)			*/
#define DT	14	/* li, lil, and liu			*/
#define DU	15	/* D form, imm disp (rt|bf),ra,ui	*/
#define DY	16	/* svc and svcl				*/
#define DZ	17	/* D form, ra,si			*/
#define LA	18	/* branch absolute   ba, bla		*/
#define LI	19	/* branch relative   b, bl		*/
#define M	20	/* M form, rotate left instructions	*/
#define ML	21	/* extended shift instructions, sli	*/
#define MR	22	/* extended shift, sri			*/
#define Q2	23	/* bXXr, bXXrl, bXXc, bXXcl		*/
#define X0	24	/* no arguments, output as is		*/
#define X1	25	/* X form	0   ,0   ,arg3		*/
#define X2	26	/* X form	0   ,arg2,0   		*/
#define X3	27	/* X form	0   ,arg2,arg3		*/
#define X4	28	/* X form	arg1,0   ,0   		*/
#define X5	29	/* X form	arg1,0   ,arg3		*/
#define X6	30	/* X form	arg1,arg2,0   		*/
#define X7	31	/* X form	arg1,arg2,arg3		*/
#define XC	32	/* XFX form	mtcrf instruction	*/
#define XE	33	/* X form	arg2,arg1,0   		*/
#define XF	34	/* X form	arg2,arg1,arg3		*/
#define XM	35	/* XFL form	mtfsf instrcution	*/
#define XR	36	/* X form	arg2,arg1,arg2		*/
#define YI	37	/* roll your own, ILLEGAL		*/
#define YM	38	/* D form, si = -si			*/
#define Z0	39	/* roll your own, ILLEGAL		*/
#define Z7	40	/* roll your own, ILLEGAL		*/
#define Z8	41	/* roll your own, ILLEGAL		*/
#define Z9	42	/* roll your own, ILLEGAL		*/
#define CALL	43	/* call instruction, NOT implemented	*/
#define CALLR	44	/* callr instruction, NOT implemented	*/
#define AD      45      /*  A form, without arg2 and arg4       */ 
#define D3      46      /*  for new DS form                     */
#define AE      47      /* for new AE form                      */
#define XV      48      /* X form    arg2, arg1, 0. ra is for   */
                        /* Upper or Lower SPR                   */
#define XP      49      /* X form    arg1, arg2, 0. ra is for   */
                        /* Upper or Lower SPR                   */  
#define DA      50      /* D form , for two input formats:      */
                        /* 4 or 3 input operands. cmpi and cmpli  */
#define XA      51      /* X form. for two input formats:      */
                        /* 4 or 3 input operands. cmp and cmpl  */
#define XB      52      /* X form.  duplicates the ARG2 to ARG3 */
#define XD      53      /* X form. duplicates the ARG1 to ARG2 and ARG3  */
#define XG      54      /* X form. arg1, arg3, arg2             */
#define XH      55      /* X form.  Two input formats:          */
                        /* 0, arg1, arg2   or  arg1, arg2, arg3 */
#define DH      56      /* D form.  two input formats:          */
                        /* 0,arg1, arg2 or  arg1,arg2,arg3      */
#define MA      57      /* M form. extlwi                      */
                        /* arg1, arg2, arg4, 0, arg3-1         */
#define MB      58      /* M form.   extrwi                    */
                        /* arg1, arg2, arg3+arg4, 32-arg3, 31  */
#define MC      59      /* M form.  rotlwi                    */
                        /*  arg1,arg2,arg3,0,31               */
#define MD      60      /* M form.                   rotrwi   */
                        /*  arg1,arg2,32-arg3,0,31            */
#define ME      61      /* M form. clrlwi                     */
                        /*   arg1, arg2, 0, arg3, 31          */
#define MF      62      /* M form.       clrrwi               */
                        /*  arg1, arg2, 0, 0, 31-arg3         */
#define MG      63      /* M form.     clrslwi                */ 
                        /* arg1, arg2, arg4, arg3-arg4, 31-arg3 */
#define MH      64      /* M form.    rotlw                     */
                        /*  arg1,arg2,arg3,0,31                 */
#define MJ      65      /* M form.     inslwi                 */
                        /* arg1,arg2,32-arg4, arg4, arg3+arg4-1 */
#define MK      66      /* M form.     insrwi                 */
                        /* arg1, arg2, 32-(arg3+arg4), arg4, arg3+arg4-1 */
#define XI      67      /* X form.  mtspr. arg2 and arg1 reversed. */
                        /* arg1 is 10-bit SPR which contains       */
                        /* low-order 5 bits followed by high-order */
                        /* 5 bits. The Assembler has to swap them  */
                        /* back to the normal order before generating */
#define XJ      68      /* X form.  mfspr.                  */
                        /* arg2 is 10-bit SPR which contains       */
                        /* low-order 5 bits followed by high-order */
                        /* 5 bits. The Assembler has to swap them  */
                        /* back to the normal order before generating */
                        /* the binary code.                         */
#define XK      69      /* handling the basic mnemonics of the branch  */
                      /* conditional to LR or CTR with '+'suffix prediction */
#define XL      70      /* handling the basic mnemonics of the branch */
                      /* conditional to LR or CTR with '-' suffix prediction  */
#define DG      71        /* for extended mnemonics of the relative and   */
                          /* absolute branch prediction with '+' suffix  */
#define DJ      72        /* for extended mnemonics of the relative and   */
                          /* absolute branch prediction with '-' suffix  */
#define DK      73        /* for basic mnemonics of the relative and   */
                          /* absolute branch prediction with '+' suffix  */
#define DL      74        /* for basic mnemonics of the relative and   */
                          /* absolute branch prediction with '-' suffix  */
#define XN      75   /* for mftb inst.        */
#define XO      76   /* for mfsri invalid form checking   */
#define XQ      77   /* for lswi and lsi invalid form checking  */
#define XS      78   /* update form of storage access inst.. RA==0 invalid */
                     /* form check         */
#define XT      79   /* update form of storage access inst.. RA==0    */
                     /* and RA==RT invalid form check                 */
                     /* form check         */
#define XU      80   /*  for lswx and lsx inst.   */
#define XW      81   /* for mfdec inst.   */
/****************************************************************/
/* the instruction table					*/
/****************************************************************/
#define MAXARGS 5		/* maximum number of args allowed*/
#define LIFMT	6		/* input format field length	*/
#define MAX_MNLEN      10       /* maximum length of a mnemonic   */

struct instab {
	char	name[MAX_MNLEN]; /*  pseudo-op,or  PowerPC or    */
                                /*  POWER mnemonics             */
	char	tag;		/* token tag value used by yylex*/
	char	ofmt;		/* output format 		*/
        unsigned long inst_bmap;  /* indicate inst. scope      */
	char    ifmt[LIFMT]; 	/* input format			*/ 
	char 	ifmt2[LIFMT];	/* alternate input format	*/
	unsigned int opcode;	/* skeleteton instr with opcode	*/
        char    name1[MAX_MNLEN]; /* POWER or Powerpc mnemonics   */ 
                                /*  for xref. If name field has */
                                /* PowerPC MN, then this field  */
                                /* has POWER MN, and vice versa */
        unsigned char type_id;  /* This field indicates that    */
                                /* name field holds POWER or    */
                                /* PowerPC mnemonics.           */ 
                                /*  '1' = POWER,  '2' = PowerPC */
};

#define HIGHREG	31		/* the number of the highest reg*/
#define NREGS	HIGHREG+1 	/* number of registers		*/

#define NOP 0x60000000	    	/* ori 0,0,0 			*/
#define ZNOP 0x0000		/* used in doalign		*/

#define BRALIGN	3		/* branch address alignment	*/
#define ALGNMSK	0x3		/* instruction alignment mask	*/
/*invalid instruction for this instruction set                  */
#define BADINSTR 0x0
#define MAGIC_NUMBER U802TOCMAGIC /*machine specific magic numb */
