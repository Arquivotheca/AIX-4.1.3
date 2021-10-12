/* @(#)73	1.35  src/bos/usr/ccs/bin/as/as.h, cmdas, bos41B, 412_41B_sync 12/8/94 10:10:16 */
/*
 * COMPONENT_NAME: (CMDAS) Assembler and Macroprocessor 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef	 _H_AS 
#define _H_AS
/* as.h	*/
/* header for as assembler contains common declarations for	*/
/* the assembler, and included in all the source files		*/
# include <stdio.h>

/* include object module format specific header file		*/
#include <xcoff.h>
/* include target machine format specific header file		*/
#include "instrs.h"
 
/* #define NFILE   0 */	/* used to debug files			*/
 
	/* ----- this section will define the macros ----- */

#define round(X,Y)	(((X)+(Y))&~(Y))

	/* ----- locale includes  ------------------------ */
#include	<langinfo.h>
#include	<locale.h>
#include	<ctype.h>
	/* ----- message handling ------------------------ */
/* #define	MAXMSG 181      maximum number of current messages   */
    /* MAXMSG is no longer used to avoid updating it every time  */
    /* when msg_defs array is updated. use num_msgs instead.  */
#define BUFLEN 80       /* max. line length used in msg serv    */
#define DEFMSG 142      /* default Error in Syntax message      */

#include "as_msg.h"
#define MSGSTR(Num) MSGbuf=catgets(catd, MS_AS, Num, msg_defs[ Num-1 ])
#define MESSAGE(Num) catgets(catd, MS_AS, Num, msg_defs[ Num-1 ])
nl_catd catd;
extern char	*msg_defs[ ];

/****************************************************************/
/* Expression value types					*/
/****************************************************************/
#define E_UNK	0	/* unkown expression type, usually FORW	*/
#define E_ABS	1	/* absolute expression			*/
#define E_REL	2	/* relocatable expression		*/
#define E_EXT	3	/* external type			*/
#define E_TREL	4	/* toc relative type			*/
#define E_TOCOF 5	/* tocof symbol				*/ 
#define E_REXT  6       /* external type with restrictions on   */
			/* where it can appear			*/
#define E_TMP   7       /* intermediate result                  */
/****************************************************************/
/* Csect storage classes 					*/
/****************************************************************/
#define C_NUM   17      /* number of csect storage mapping class*/
#define C_DSECT	C_NUM	/* set the class for dsects to a value  */
			/* which is higher than any valid class */
#define C_LC    C_NUM+1 /* local symbols                        */
#define C_DC    0xff    /* Don't Care				*/

/****************************************************************/
/* Debugger internal type					*/
/****************************************************************/
#define XTY_DBG    07   /* debugger information                 */

#define ERR	(-1)

/****************************************************************/
/* Actual argument syntax types					*/
/****************************************************************/
#define ADISP	1	/* expr(%r) */
#define AEXP	2	/* expr */
#define APRF	3	/* $ expr */
#define APREG	4	/* % name */

/****************************************************************/
/* address argument types 					*/
/****************************************************************/
#define AADDR	0		/* address can only be an A 	*/
#define AIDDR	1		/* address can be an A or an I	*/
 
/****************************************************************/
/* reference types for loader 					*/
/****************************************************************/
#define HW	01
#define FW	03
#define DW	07
 
/*****************************************************************
** space definition for assembler listing 
*****************************************************************/
#define SPACE_30    "                              "
#define SPACE_7     "       "
#define SPACE_23    "                       " /* replace message 129 */      
#define SPACE_13_X  "             %.8x  "     /* replace message 130 */
/*****************************************************************
** Assembly mode 
*****************************************************************/
#define   COM       0x0001   /* POWER and PowerPC intersection   */
#define   PWR       0x0002   /* RIOS/1 of POWER architecture     */
#define   PWR2      0x0004   /* RIOS/2 of POWER architecture     */
#define   PPC       0x0008   /* Common to PowerPC architecture   */
#define   PPC_601   0x0010   /* 601 implementation of PowerPC    */
#define   PPC_603   0x0020   /* 603 implementation of PowerPC    */
#define   PPC_604   0x0040   /* 604 implementation of PowerPC    */
#define   ANY       0x8000   /* for any valid instructions       */
/*****************************************************************
** Assembly mode  type
*****************************************************************/
#define   POWER       '1'
#define   PowerPC  '2'
/*****************************************************************
** Source Program Domain  definition

  a 32-bit pattern is used to describe the relationship 
  between the assembly mode and the instructions. When the
  bit is 1, it indicates that this instruction is implemented 
  in this mode.

  Each bit is defined as follows:
   
   Bit 31        com
   Bit 30        pwr
   Bit 29        pwr2(pwrx)
   Bit 28        ppc
   Bit 27        601
   Bit 26        603
   Bit 25        604
   Bit 24-17     Reserved
   Bit 16        any
   Bit 15 -0     Reserved

*****************************************************************/
#define  SRC_RS2_ONLY   0x00008004   /*  RIOS/2 unique instructions  */
#define  SRC_RS2_601    0x00008014   /*  RIOS/2 and 601 instructions */
                                     /*     not in RIOS/1 or PowerPC */
#define  SRC_RS2_PPC    0x0000807C   /*  RIOS/2 and PowerPC instructions */
                                     /*     not in RIOS/1            */
#define  SRC_PW_NO601   0x00008006   /* POWER  unique inst., not in 601 */
#define  SRC_PW_601     0x00008016   /* POWER unique inst., supported in 601 */
#define  SRC_PW_PPC     0x0000807F   /* POWER and PowerPC intersection */
#define  SRC_PPC_601    0x00008078   /* PowerPC inst., not in POWER,  */
                                     /* but in 601, 603 and 604        */
#define  SRC_PPC_NO601  0x00008068   /* PowerPC inst., not in POWER,   */
                                     /* not in 601, but in 603 and 604 */
#define  SRC_601_ONLY   0x00008010   /* 601 unique inst.    */
#define  SRC_603_ONLY   0x00008020   /* 603 unique inst.    */
#define  SRC_PPC_OPT1   0x00008070   /*  PowerPC optional inst. set 1.    */
                                     /* in 601, 603 and 604.             */
#define  SRC_PPC_OPT2   0x00008060   /* PowerPC optional inst. set 2.    */
                                     /* not in 601, but in 603 and 604.  */
#define  SRC_ANY        0x00008000   /*  Mixed inst.        */

/*****************************************************************
** CPU ID definition
*****************************************************************/
#define   CPUID_PPC_32BIT   1 
#define   CPUID_PPC_64BIT   2 
#define   CPUID_COM         3
#define   CPUID_RIOS_1      4
#define   CPUID_ANY         5
#define   CPUID_601         6
#define   CPUID_603         7
#define   CPUID_604         8
#define   CPUID_RIOS_2      224

/************************************************************
** warning flag values
************************************************************/
#define   WFLAG        0   /* -W flag is used             */
#define   wFLAG        1   /* -w flag is used             */
#define   NO_WFLAG     2   /* no -w and no -W is used     */

#define MAXLINE        512  /* max. number of chars. in source file */
 
#define  MAX_REFLIST  50   /* max. number of terms in an expression */
#define  VALID_EXP   2     /* the max. number of non-absolute terms in  */
                           /* a possible valid result expression.       */
#define  RLD_NOT_SET  0xFF

char mn_line_buf[MAXLINE+10];
extern unsigned short asm_mode;         /* See as1.c for definition    */
extern char     asm_mode_str[];          /* See as1.c for definition    */
extern unsigned char asm_mode_type;     /* See as1.c for definition    */
extern int mn_xref;                     /* See as1.c for definition    */
extern char mn_buf[];                   /* See as1.c for definition    */
extern unsigned long sum_bit_map;      /* See as1.c for definition    */
extern unsigned short src_lang_id;      /* See as1.c for definition    */
extern int file_flag;                   /* See as1.c for definition    */
extern int src_flag;                    /* See as1.c for definition    */
extern int modeopt;                    /* See as1.c for definition    */
extern struct symtab *lastsym;         /* see definition in asst.c    */
extern struct symtab *symtab;   /* thread linking all symbols 	*/
                                /* see definition in asst.c     */
extern struct symtab *esdanc;	/* anchor for ESD chain		*/
extern struct symtab *tocname;	/* $TOC  in the sym tab		*/
extern struct instab instab[];	/* instruction (and pseudo-op) table	*/
#ifndef NSHASH
#define NSHASH	201		/* number of entries in HASH tab*/
#endif
				/* symbol hash table		*/
extern struct	symtab *shash[NSHASH];
 
/* aux entries */
/* type 1 used by .bf, .ef, .bb, .eb	*/
struct aux_f1 {
	long	tagndx;
	short	lnno;
	short	size;
	long	lnnoptr;
	long	endndx;
};
 
 
/* type 2 used by ave, sve	*/
struct aux_f2 {
	long	tagndx;
	short	lnno;
	short	size;
	short	dim[4];
};
 
 
/* type 3 used by .function	*/
struct aux_f3 {
	long	tagndx;
	long	fsize;
	long	lnnoptr;
	long	endndx;
};
 
 
/* type 4 used by .file	*/
struct aux_f4 {
	char name[14];
};
 
/* type 5 used by .csect	*/
struct aux_f5 {
		long		x_scnlen;	/* csect length   */
		long		x_parmhash;	/* parm type hash index */
		unsigned short	x_snhash;	/* sect num with parm hash */
		unsigned char	x_smtyp;	/* symbol align and type  */
						/* 0-4  Log 2 of alignment */
						/* 5-7  symbol type        */ 
		unsigned char	x_smclas;	/* storage mapping class */
		long		x_stab;		/* dbx stab info index */ 
		unsigned short	x_snstab;	/* sect num with dbx stab */
	};

/****************************************************************/
/* symbol table 						*/
/****************************************************************/
struct	symtab {
	struct	symtab *nxtsym; /* Next consecutive entry in tab*/
	struct	symtab *nxtesd; /* Next esd entry in symtab	*/
	struct	symtab *next;	/* hash bucket link 		*/
	long           value;	/* type dependant value of sym	*/
	struct  sect  *csect;  /* pointer to containing csect 	*/ 
        short sectnumber;       /* symbol section number        */
	unsigned char type;	/* symbol type			*/
        unsigned char sclass;   /* symbol storage class         */

            /* The following array is for two RLD entries.      */
            /* An expression can have at most two RLD entries. These */
            /* fields are needed in case the symbol is a representative */
            /* of an expression.                                     */
        struct rld_info {
           struct symtab  *r_name;
           unsigned char   r_rrtype;
        } r_info_arr[2];

        /*etype and eclass should be removed since they appear
        in aux_f5 (.csect structure)                            */
	unsigned char etype;	/* esd type			*/
	unsigned char eclass;	/* esd class			*/

        int reflist_index;      /* index to reflist             */
        int symlist_idx;        /* index to symlist             */
        int rcount;             /* reference count in an exp.   */

	unsigned int esdind;	/* ESD index			*/
        char numaux;            /*Number of aux entries         */
	struct aux {
                struct aux *nextaux;   /*pointer to next union*/
	        union {	
                  struct aux_f1 f1;     /*unused at this time*/
		  struct aux_f2 f2;     /*unused at this time*/
		  struct aux_f3 f3;     /*function*/
		  struct aux_f4 f4;     /*file*/
		  struct aux_f5 f5;     /*csect*/
                } aux_entry;
	} aux;
        short   blk_id;         /* scoping variable             */
	char	*name;		/* pointer to string table 	*/
	char    *rename;	/* pointer to string table 	*/
	char    *chktyp;        /* pointer to hash in .typchk   */
        char    *dbug;          /* point to debugger string     */
};

#define func_aux  aux.nextaux->aux_entry.f3
#define file_aux  aux.aux_entry.f4
#define csect_aux aux.aux_entry.f5
#define be_aux    aux.aux_entry.f1
#define f2_aux    aux.aux_entry.f2
/****************************************************************/
/* linenumber table                                         	*/
/****************************************************************/
#define LINE struct line
extern LINE *linenum;                   /* anchor for line table*/

struct line
{
        struct line *nextline;         /*pointer to next line structure*/
	union
	{
		long	l_symndx ;	/* sym. table index of function name
						iff l_lnno == 0      */
		long	l_paddr ;	/* (physical) address of line number */
	}		l_addr ;
	unsigned short	l_lnno ;	/* line number */
} ;
/****************************************************************/
/* rld table 							*/
/****************************************************************/
#define XRLD	struct xrld
#define XRLDSZ  sizeof(struct xrld)
struct xrld {
     XRLD 	*rnxt;		/* forward chain		*/
     unsigned char rtype;	/* relocation type		*/
     unsigned char rlen;	/* relocation length		*/
     struct symtab *rname;	/* symbol table entry relative to*/
     long  raddr;		/* address			*/
     };

#define RLDSIGN 1               /* sign field on rld entry      */

/****************************************************************/
/* csect table 							*/
/****************************************************************/
#define SECT	struct sect
#define SECTSZ  sizeof(struct sect)
struct sect {
	SECT 	*nxt;		/* forward chain		*/
	long    dot;	 	/* current csect loc cnt   	*/	
	long    start;		/* csect start address		*/
	long	end;		/* size of csect		*/
	unsigned char   stype;  /* section type SD, CM, ER	*/
        unsigned char   ssclass; /* storage class		*/
	char    algnmnt;	/* csect alignement		*/
	struct symtab *secname; /* section name in symtab	*/
	XRLD	*rent;		/* relocation entries 		*/
	struct symtab *lastsym; /* last symbol entered		*/
        int  reflist_idx;       /* index for the reflist in exp. */
                                /* process.                      */
	};

extern SECT *sect;		/* section table anchor 	*/
 
/* arguments passed to instructions				*/
struct	arg {
	char	atype;		/* argument type 		*/
	char	areg;		/* register number, if x(d) 	*/
	struct	exp *xp;	/* value of argument		*/
};
 
/* expression struction to hold expr tokens			*/
struct	exp {
	SECT	*xloc;		/* which section we are in	*/
	char	xtype;		/* type				*/
        unsigned char x_rrtype; /* type of RLD of xname symbol  */
        short   st_inx;         /* index to the expression stack */
                                /* for this exp. entry          */
        struct  symtab *xname;  /* name that it's relative to   */
                                /* it is also the name the 1st  */
                                /* RLD entry relative to        */
        struct {                /* The second  rld info         */
          unsigned char rrtype; /* type of RLD                  */
          struct symtab *rname; /* name RLD relative to         */
        } rldinfo;

	union {			/* actual value of expression	*/
		long	_xvalue; /* usual value		*/
		double _dvalue;	/* value if it is a double	*/
	} _xydu;
};
/* defines to make expression value access easier		*/
#define xvalue	_xydu._xvalue
#define dvalue	_xydu._dvalue
 
struct	arg	arglist[6];	/* instruction operands 6 max		*/
struct	exp	explist[MAX_REFLIST];	/* list of expressions		*/

extern int     file_no;		/* index of current file in file_table	*/
extern int     last; 		/* index of last file name in table     */
extern char    *infile;         /* pointer to current file name		*/
extern char    *orig_infile_arg;/* pointer to original source file name	*/
extern char    *file_table[];	/* table of pointers toknown file names */
FILE	*listfil;		/* listing file				*/
FILE	*xoutfil;		/* xcrossref output file                */
FILE	*txtfil;		/* output text file			*/
FILE	*tmpfil;		/* temporary intermediate token file	*/ 
extern char tmpn1[];            /* temporary intermediate token filename*/
FILE 	*tmpinfil;		/* intermidiate source file */

extern int     lineno;		/* current line number			*/
long	bssdot; 		/* bss location counter for .lcomm's 	*/
extern int     passno;		/* pass number				*/
int	argcnt;			/* argument count			*/
int	anyerrs;		/* any error flat, incremented by	*/
				/* yyerror				*/
int     anywarns;               /* any warnings, incremented by yywarn  */

extern SECT	*ccsect;	/* current csect pointer		*/
extern SECT     *ccsectx;       /* current csect pointer, maintained for
                                  .comm's, .lcomm's  as well as for csects */
extern int	maxalgn[2];	/* max alignment encountered for each	*/
				/* section				*/
extern int intoc;		/* intoc flag set to 1 when inside of 	*/
				/* toc					*/
extern char redefined[];	/* common error message			*/
extern char	*verid;		/* assembler version id			*/

/* option flags								*/
int	i_debug;		/* debug flag				*/
extern int flag_extern_OK;	/* -u flag for undeclared symbols      	*/
extern int  xflag;		/* -x flag for the assembler cross ref  */
extern int warning;		/* warning on option flag		*/
extern int	 listopt;	/* produce listing option		*/
#define ERLINES 10*5		/* max. number of error lines 		*/
extern int 	line_error;	/* error flag 				*/
extern char	g_errbuf[30*ERLINES]; 
extern char	errbuf[30*ERLINES]; 
extern char 	linebuf[];
extern char	*linebuf_ptr;	/* pointer to linebuf array		*/
extern char 	*errbuf_ptr;	/* pointer to errbuf array		*/
extern char 	*g_errbuf_ptr;	/* pointer to g_errbuf array		*/

/* comon assembler routines						*/
struct	symtab	*lookup();	/* lookup symbols in symtab		*/
struct instab *find_opcode();	/* find opcodes in instab		*/
struct symtab	*symalloc();	/* allocate a symbol			*/
struct	exp	*combine();	/* combine expressions			*/
char    *put_string();		/* put a string in the string pool	*/
extern SECT	*new_csect();	/* create a new csect table entry	*/
unsigned short cvt_asmmode(char *,char); /* convert input string of    */
                                    /* assembly mode into integer  */
int yyerror1(int, ...);      /* output error message     */
int getmsg(int);                /* get message string from message catalog */
int error5(char *, void *);     /* writes error message to error buffer  */
void instrs_warn(int, ...);   /* report instructional warnings */
void jmach_push(void);
void jmach_pop(void);

#define MAX(a,b)	(((a)>(b))?(a):(b))
#define MIN(a,b)	(((a)<(b))?(a):(b))
#define HASHVALUE ""		/* Null hash value			*/

/************************************************************************/
/* redefine putc and fwrite to detect & clean up after			*/
/* I/O errors during output						*/
/************************************************************************/
#define xfwrite(a,b,n,s,g)(fwrite(a,b,n,s)!=n?perror("as"),yyerror(161,g),\
delexit():n)
#define xputc(c,f,g)(putc((c)&0xff,f)==EOF?perror("as"),yyerror(161,g),\
delexit():c)
#define xfclose(f,g)(fclose(f)==EOF?perror("as"),yyerror(161,g),\
delexit():0)

#ifndef BUFSIZ
#define BUFSIZ _BUFSIZE		/* added for CMS version of BUFSIZE	*/
#endif

/*
 * scoping stack
 */
#define NSTACK	100
struct {
	unsigned blkid; /* to uniquely restrict scope */
	struct symtab *ptr;
} stack[NSTACK];
 
#define myblk() (stackp>0? stack[stackp-1].blkid: 0)
int stackp;

/************************************************************************/
/*  Valid Assembly mode value table                                     */
/************************************************************************/
struct asmmode {  
    char *mode_str;
    unsigned char  as_md_type;
    unsigned short mode_value;
};
/************************************************************************/
/*  The old Assembly mode "pwrx" index in  the asmmode_tab table        */
/************************************************************************/
#define PWRX_IDX_ASMTAB    2

extern struct asmmode asmmode_tab[];
extern int max_mode_num;
/************************************************************************/
/*  Assembly mode stack                                                 */
/************************************************************************/
#define  MAX_ASMMOD_STK   100
extern  int mach_push;
extern  int mach_pop;

/*********************************************************/
/* RLD information list for every term in an expression  */
/* each entry is either for a section in which the symbols */ 
/* defined are referred in the expression or for an external */
/* symbol ( section pointer is NULL )                      */
/*********************************************************/
struct exp_ref_list {
   int  n_ext;                 /* number of E_EXT   */
   int  n_rel;                 /* number of E_REL   */
   int  n_trel;                /* number of E_TREL  */
   SECT   *sec_ptr;            /* which section it is        */
   struct symtab *s_name;      /* symbol name  it related    */
};

extern  struct exp_ref_list reflist[];
extern  int cur_reflist;
extern  struct symtab *symlist[];
extern  int cur_symlist;


/*********************************************************/
/*      expression entry in the expression stack         */
/*********************************************************/
struct  exp_pat_stack {     
  int  sign;                   /* the  term sign    */
  struct exp *exp_ptr;         /* expression pointer    */
  char x_type;                 /* this term type. This field is needed */
                               /* because the exp_ptr->xtype could be  */
                               /* altered as E_TMP in the combine routine */
};
 
extern  struct exp_pat_stack st_lp[];
extern  int st_inx;

void put_reflist(int,  int *, struct exp_ref_list []);
void push_exp (int, struct exp *);

#endif 	/*_H_AS*/
