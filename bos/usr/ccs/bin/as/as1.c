static char sccsid[] = "@(#)76	1.38  src/bos/usr/ccs/bin/as/as1.c, cmdas, bos41B, 412_41B_sync 12/8/94 10:09:18";
/*
 * COMPONENT_NAME: (CMDAS) Assembler and Macroprocessor 
 *
 * FUNCTIONS: atoen, callsys, ddbg, del, delete, delexit,
 *	      indbg, outb, outexpr, outh, outl, print_lineno,
 *	      process_args, putdbg, putflt, sortfile, xexit,
 *            set_cpuid, process_lang_id
 *
 * ORIGINS:  3, 27
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
/****************************************************************/
/* as assembler main line control				*/
/****************************************************************/
#include <signal.h>
#include <string.h>
#include <nl_types.h>
#include "as.h"
#include "as0.h"

extern FILHDR hdr; 		/* header for a.out file*/
extern SCNHDR text;		/* text section header */
extern SCNHDR data;		/* data section header */ 
extern SCNHDR bss;		/* bss section header */
extern SCNHDR typchk;		/* typchk section header */
extern SCNHDR debug;		/* debug section header */
extern SCNHDR text_aux;		/* text auxiliary section header */
extern SCNHDR data_aux;		/* data auxiliary section header */

extern int XN_TEXT,	        /* section definition flags      */
           XN_DATA,
           XN_BSS,
           XN_TYPCHK,
           XN_DEBUG,
	   XN_TEXT_AUX,
	   XN_DATA_AUX;

extern int text_nreloc,		/* number of rlds in text section */
	   data_nreloc,		/* number of rlds in data section */
	   text_nlnno,		/* number of line entries in text section */
	   data_nlnno;		/* number of line entries in data section */

extern int length_str;          /* length of string table */

#ifdef NFILE
	char tmpn1[] = "tmpn1";			/* name of intermediate*/
						/* file		       */
	static char tmpin[] = "tmpin";		/* input file copy     */
#else
	char tmpn1[] = "/tmp/asXXXXXX";		/* name of intermediate*/
						/* file		       */
	static char tmpin[] = "/tmp/asXXXXXX";	/* input file copy     */
#endif
static char *xcrossfile = NULL;		/* xcrossref file name	*/
static int  outfile_opened = 0;		/* Flag to tell if output*/
					/* file should be deleted.*/
int	lineno	= 1;			/* line number, used for*/
					/* yyerror		*/
int	listopt;

extern	char _sibuf[];			/* buffer used for stdin*/
unsigned char	_tiobuf[BUFSIZ+8];	/* iobuf for txt  file */
char *outfile = "a.out";		/* output file name	*/
char *infile; 	 			/* Name of input file. */
char *orig_infile_arg; 			/* Name of input file. */
 
char  *src_file_name = NULL;            /* the source file name in */
                                        /* .file pseudo-op        */
char	*mktemp(), *malloc();		/* library functions used*/
int exec_subject;			/* flag to tell if 	*/
					/* present instruction is*/
					/* the subject of a 	*/
					/* branch and execute	*/
int flag_extern_OK=0;			/* u option  undefines  */
					/* not flagged          */
int xflag=0;				/* assembler x option xrefer */
int passno = 0;				/* assembler pass number*/
int warning=NO_WFLAG;			/* tells if "-W", "-w" or */
                                        /* none of them is used. */
extern int tcn;				/* numbered toc entry 	*/
					/* name			*/
int sort_csects();			/* sort csects between	*/
					/* passes		*/
struct symtab *sort_sym();		/* sort symbols between	*/
					/* passes		*/
struct symtab * *scop_sym;              /* variables used in    */
struct symtab * *scop_p;                /* aspso.c initialized */
unsigned long inst_bmap;               /* instruction bit map  */
unsigned short cmd_asm_mode = COM;      /* this variable is used to save the */
                                        /* of -m flag option. It is preset  */
                                        /* to default value. The asm_mode   */
                                        /* is updated whenever a ".machine" */
                                        /* is used. But this variable is   */
                                        /* only for -m value. This value   */
                                        /* needs to be saved for Pass 2.   */
unsigned short asm_mode = COM;          /* assemble mode. It is init to     */
                                        /* the default value.              */
                                        /* If -m flag is used, the init     */
                                        /* value will be updated. Otherwise */
                                        /* this value will be updated in    */
                                        /* .machine pseudo ops, or never    */
                                        /* be updated                       */
#define MAX_ASM_MODE_LEN 4
char asm_mode_str[MAX_ASM_MODE_LEN+1] = "COM";
static char cmd_asm_mode_str[MAX_ASM_MODE_LEN+1] = "COM";
                                        /* these two variables are used     */
                                        /* for message 147 and 162          */
int modeopt = 0;                        /* tell if -m flag is  used         */
int cmd_modeopt = 0 ;                   /* this variable is used to save the */
                                        /* of -m flag status from the command */
                                        /* line. It indicates if a default   */
                                        /* mode is used or not. This value  */
                                        /* is needed for Pass 2.           */
unsigned long sum_bit_map = SRC_PW_PPC;  /* summary of the inst bit map      */
unsigned char  asm_mode_type = PowerPC;  /* asm mode category --            */
                                        /* '1' = POWER, '2' = PowerPC       */  
unsigned short src_scope;               /* source program scope  */
unsigned short  cpu_id;                 /* CPU ID information    */

#define  LANG_ID_UNKNOWN     99
unsigned short src_lang_id = LANG_ID_UNKNOWN; 
                                        /* source program langauge id  */
                                        /* init to "Assembler". If no  */
                                        /* ".source", assembler is default */ 
int file_flag = 0;                      /* .file indicator       */
int mn_xref;                            /* indicator of mnemonics xref */
char mn_buf[MAX_MNLEN+1] = "          " ;  /* POWER or PPC mnemonics has the */
                                           /* maximun length of 10           */ 

struct asmmode asmmode_tab[] = {        /* valid Assembly mode value table */
      {  "COM", PowerPC, COM },
      {  "PWR", POWER, PWR },
      {  "PWR2", POWER, PWR2 },       /* if input is "PWRX", equi. to "PWR2" */
      {  "PPC", PowerPC, PPC },
      {  "601", PowerPC, PPC_601 },
      {  "603", PowerPC, PPC_603 },
      {  "604", PowerPC, PPC_604 },
      {  "ANY", PowerPC, ANY}
};

int max_mode_num = sizeof(asmmode_tab)/sizeof(struct asmmode);

SCNHDR *p;                      /* temporary pointer */

#define NUMSECT 7               /* maximum number of sections that  */
                                /* can be generated by the Assembler  */
#define TYPCHK_SECT_NUM   3     /* position of type-check section  */
                                /* in secthdr array                */ 
#define DEBUG_SECT_NUM    4     /* position of debug section       */
                                /* in secthdr array                */
#define TEXT_AUX_NUM      5     /* position of text overflow section */
                                /* in secthdr array                */
#define DATA_AUX_NUM      6     /* position of data overflow section */
                                /* in secthdr array                */

				/* array of section hdr pointers */
SCNHDR *secthdr[NUMSECT] = { &text, &data, &bss, &typchk, 
			     &debug, &text_aux, &data_aux };
int *sectdfn[NUMSECT] = { &XN_TEXT, &XN_DATA, &XN_BSS, &XN_TYPCHK, 
			  &XN_DEBUG, &XN_TEXT_AUX, &XN_DATA_AUX };

char *sections;   		/* pointer to combined read 	*/
				/* only and read/write sections	*/
 
void process_lang_id(void);
/****************************************************************/
/* Function:	main						*/
/* Purpose:	as main line control routine, handles i/o and	*/
/*		calls pass one and pass 2 			*/
/* Input:	argc						*/
/*		argv						*/
/* Comments:	see process_args for explanation of command line*/
/*		arguments					*/
/****************************************************************/
main(argc, argv)
 char **argv;
{
	register struct symtab *sp;	/* symbol table pointer	*/
	register int c; 
	char *infnm=NULL;
	char	*fn;			/* file name used in tag*/
	unsigned int tagoff;		/* tag offset in string	*/
					/* table		*/
	unsigned int hashoff;		/* strtab hash offset	*/
	unsigned int stroff;		/* string table offset	*/
					/* used for ESD name	*/
	unsigned int stg;		/* used for ESD	 storage*/
					/* class		*/
	unsigned int algn;		/* used for ESD alignment*/
	SECT *csp;			/* tmp csect pointer	*/
	XRLD *relp;			/* tmp rld pointer	*/
	char *str_table;		/* string table pointers*/
	char *esd_table;		/* ESD table pointers	*/
	char *rld_table; 		/* RLD table pointers	*/
	char *line_table; 		/* LINENO table pointers */
	char *hsh_table; 		/* hash (typchk) table pointers */
        char *dbg_table;                /* debug table pointer   */
	char *listfile = NULL;		/* listing file argument */
 
	int delexit();
	int del();
	void delsig();
	void delexitsig();

	int curr_sect;                  /* section index */
        int curr_fptr;                  /* file ptr index */
        int hdr_length;                 /*total length of header */

#ifdef IDEBUG
        extern dmp_hdr();                /* header debug function */
#endif
	setlocale(LC_ALL, "");   /* set the locale from the default C  */
                                 /* locale to the locale specified by the */
                                 /* environment variables for all LC_*    */
                                 /* categories.                           */ 
        setlocale(LC_NUMERIC, "C");  /* set LC_NUMERIC category to C      */
	catd = catopen(MF_AS,NL_CAT_LOCALE);
 
	/* process input arguments (flags and file names) */
	process_args(argc,argv,&outfile,&xcrossfile,&listfile,&infnm);
 
	/* initialize symtab	*/
        symtab = 0;

	passno = 1;			/* init to pass 1	*/

	sect = 0;			/* initialize csect	*/
					/* table		*/
	jcsect(XMC_PR,0,2);   		/* add a noname PR csect*/
					/* to start it off in case*/
        ccsectx =                       /* Record csect for stabs */
	ccsect = sect;    		/* no csect statement	*/
	
	/* set to catch error signals */
	signal(SIGBUS, delsig);
	signal(SIGSEGV, delsig);
	if(signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, delexitsig);

        /*  open the temporary file to put intermediate results */
#ifndef NFILE
	mktemp(tmpn1);
#endif
	if(!(tmpfil = fopen(tmpn1, "w"))) {
                                                /* modified message 026 */
		yyerror( 26);
		delexit();
	}
	setbuf(tmpfil, _tiobuf);
 
#ifndef NFILE
	mktemp(tmpin);
#endif
	if(!(tmpinfil = fopen(tmpin, "w"))) {
                                                /* modified message 026 */
		yyerror( 26);
		delexit();
	}
	if (listopt)
		initlst(listfile, infnm);
	if (xflag)		/* if cross ref flag open tmp2 to dump xref */
                initxcross(&xcrossfile, infnm);

	yyparse();			/* read and parse input	*/ 
					/* for pass 1 		*/
        if (!file_flag )   /* no .file in the src prog  */
            if (!orig_infile_arg)
              jfile("noname");
            else
              jfile(orig_infile_arg);
        if ( !anyerrs)
           set_cpuid();       /* save the cpuid info into the .file */
                              /* entry of the symbol table          */
	xfclose(tmpfil,tmpn1);		/* tmpfile has the token*/ 
					/* ized input		*/
	if (listopt)
		cleanlst();
	xfclose(tmpinfil,tmpin);

	if(anyerrs) 			/* if any errors	*/
		delexit();		/* delete and exit	*/

	if (xflag) {			/* close tmp2 file, exit successfuly */
                xfclose(xoutfil,xcrossfile);
                sortfile();
		xexit();
	}
	jreset();			/* reset the using table */

	/* reopen the temporary file, and set up the input buffer*/
	if(!(tmpfil = fopen(tmpn1, "r"))) {
                                                /* modified message 028 */
		yyerror( 28);
		delexit();
	}
	setbuf(tmpfil,_sibuf);
	
#ifdef IDEBUG
	/* debug code	*/
        if (indbg("symdmp1")) {
	   fprintf(stderr,"after pass 1\n");
	   dmpst();
	   dmp_csect_tab();
	   }
#endif
	/* fill in section values and sort csects */
	sort_csects();
	
#ifdef IDEBUG
	/* debug code to print out sorted csect table	*/
        if (indbg("sort_cs")) {
	   fprintf(stderr,"after sorting csects\n");
	   fprintf(stderr,"rosz= %d rwsz= %d\n",text.s_size,
	           data.s_size);
	   dmp_csect_tab();
	   }
        if (indbg("symdmp2")) {
	   fprintf(stderr,"before assigning final symbol values\n");
	   dmpst();
	   dmp_csect_tab();
	   }
#endif
 
	/* assign final values to symbols and count esds 	*/
	esdanc = sort_sym();

#ifdef IDEBUG
        if (indbg("symdmp2")) {
	   fprintf(stderr,"after assigning final symbol values\n");
	   dmpst();
	   dmp_csect_tab();
	   }
	/* prints esd chain					*/
        if (indbg("esdanc")) {
	   fprintf(stderr,"ESD chain\n");
           fprintf(stderr,
             "name          value       csect     type   etype\n");
           for (sp=esdanc; (sp); sp = sp->nxtesd) 
	      dmpsym(sp);
	}
#endif

	/* the actual r/w and r/o sections are allocated as one	*/
	/* large allocated buffer, and any data processed during*/
	/* pass 2 are simply put into that buffer and the entire*/
	/* buffer is written to the output file at once after 	*/
	/* pass 2						*/
	/* the sizes of the sections were calculated by 	*/
	/* sort_csects and put directly into the header (hdr)	*/
	if (c = text.s_size+data.s_size)
	if (!(sections = malloc(c))) {
                                                /* message 029 */
	   yyerror( 29 );
	   delexit();
	}
	else bzero(sections,c);	/* fill the buffer with 0*/

        /* reset any flags for pass 2				*/
	intoc = 0;			/* we are not in a toc	*/
	tcn = 0;			/* reset unnamed toc 	*/
					/* numbering		*/
	exec_subject = 0;		/* reset branch and ex	*/
					/* subject instr flag	*/
	lineno = 1;			/* back to line	1	*/
	passno = 2;			/* pass number 2	*/
	infile = orig_infile_arg; 	/* restore infile to	*/
					/* as infile argument  */
        scop_p = scop_sym;              /* reset flag for as4.c*/

	if (listopt) 			/* initilize the list-	*/
	   initlst2(tmpin);		/* ing if there is one	*/

        asm_mode = cmd_asm_mode;        /* reset the -m flag value */
                                        /* in case asm_mode is updated */
                                        /* by ".machine"s in Pass 1    */
        modeopt = cmd_modeopt;          /* reset the modeopt value for */
                                        /* Pass 2 in case modeopt is   */
                                        /* updated by "".machine"s in Pass 1*/
        strcpy(asm_mode_str, cmd_asm_mode_str);
	yyparse();			/* PASS 2		*/

	/* pass 2 is over now need to start getting ready to 	*/
	/* write out the output file				*/
	/* first open the output file and set the buffer	*/
        
        process_lang_id();
	txtfil = fopen(outfile, "w+");
	if(txtfil==NULL) {
                                                /* message 030 */
		yyerror( 30 , outfile);
		delexit();
	}
	outfile_opened = 1;		/* delete it on exits.*/
	setbuf(txtfil, _tiobuf);

	/* header values have been calculated and put into the	*/
	/* header as needed in between pass one and two		*/
	/* here the rest of the values are filled in		*/


#ifdef IDEBUG
	if (indbg("dmphdr1")) {
        	fprintf(stderr,"++++ before mallocs ++++\n");
        	dmp_hdr();
	   	dmpst();
	   	dmp_csect_tab();
		}
#endif
	/* the next step is to build the sections       	*/
	/* there is not a loader section, because the binder	*/
	/* (linker) will build that				*/
	/* the number of ESDs is known, because they were counted*/
	/* during the symbol sorting, and the RLDs were counted	*/
	/* during pass 2 whenever the addrld routine was called	*/
	if (hdr.f_nsyms)
	if (!(esd_table = malloc(hdr.f_nsyms*SYMESZ))) {
                                                /* message 031 */
	   yyerror( 31 );
	   delexit();
	}
	if (c = text_nreloc+data_nreloc)
	if (!(rld_table = malloc(c*RELSZ))) {
                                                /* message 032 */
	   yyerror( 32 );
	   delexit();
	}
        if (length_str) { 
	if (!(str_table = malloc(length_str+4))) {
                                                /* message 033 */
	   yyerror( 33 );
	   delexit();
	}
	else bzero(str_table,length_str+4);
                           /* initialize string table counter*/
        *(int*)str_table = 4;
           }
	if (typchk.s_size)
	if (!(hsh_table = malloc(typchk.s_size))) {
                                                /* message 066 */
	   yyerror( 66 );
	   delexit();
	}
	if (c = text_nlnno+data_nlnno)
	if (!(line_table = malloc(c*LINESZ))) {
                                                /* message 034 */
	   yyerror( 34 );
	   delexit();
	}
	if (debug.s_size)
	if (!(dbg_table = malloc(debug.s_size))) {
                                                /* message 050 */
	   yyerror( 50 );
	   delexit();
	}


        /* set time/date stamp */
        hdr.f_timdat= time((long*) 0);

	/* determine if auxiliary sections are needed for text  */
	/* or data sections - make nreloc and nlnno assignments */
	assign_hdr_num();

	/* count up valid sections */
	for (curr_sect=0; curr_sect<NUMSECT; ++curr_sect)
		if(*(sectdfn[curr_sect])) {
			hdr.f_nscns++;
	}

        /* set section position pointers */        
        curr_fptr=
        hdr_length= FILHSZ + hdr.f_nscns*SCNHSZ;

        /* section address relative to bottomg of header sections */
        for (curr_sect=0; curr_sect<NUMSECT; ++curr_sect)
           if (*(sectdfn[curr_sect])) {
                if ( curr_sect != TYPCHK_SECT_NUM &&
                      curr_sect != DEBUG_SECT_NUM &&
                       curr_sect != TEXT_AUX_NUM  &&
                       curr_sect != DATA_AUX_NUM )
                            /* make sure that the current section is not  */
                            /* type check section or debug section  or    */
                            /* text overflow or data overflow section     */
                {
                    secthdr[curr_sect]->s_paddr= 
                    secthdr[curr_sect]->s_vaddr=  curr_fptr - hdr_length;
                }
                   /* make assignment for all sections except      */
                   /* BSS section or section with zero length      */
                if (curr_sect!=2 && secthdr[curr_sect]->s_size) {
                   secthdr[curr_sect]->s_scnptr= curr_fptr;
                   curr_fptr += secthdr[curr_sect]->s_size;
                }
           }

        /* set reloc position ptrs*/
        if (text_nreloc) {
            text.s_relptr= text_aux.s_relptr= curr_fptr;
            curr_fptr += text_nreloc*RELSZ;
            }  
        
        if (data_nreloc) {
            data.s_relptr= data_aux.s_relptr= curr_fptr;
            curr_fptr += data_nreloc*RELSZ;
            }  
        
        /* set line number position ptrs*/
        if (text_nlnno) {
            text.s_lnnoptr= text_aux.s_lnnoptr= curr_fptr;
            curr_fptr += text_nlnno*LINESZ;
            }  
        
        if (data_nlnno) {
            data.s_lnnoptr= data_aux.s_lnnoptr= curr_fptr;
            curr_fptr += data_nlnno*LINESZ;
            }  

        /* set header sym table ptr */
        hdr.f_symptr= curr_fptr;


			/* bldtables fills malloc mememory 	*/
	bldtables(str_table,esd_table,rld_table,hsh_table,dbg_table,
		  line_table);

#ifdef IDEBUG
	if (indbg("dmphdr2")) {
        	fprintf(stderr,"++++ before xfwrite ++++\n");
        	dmp_hdr();
	   	dmpst();
	   	dmp_csect_tab();
		}
#endif

	/* output header 					*/
        xfwrite(&hdr,1,sizeof(hdr),txtfil,outfile);

	/* output section headers 				*/
	for (curr_sect=0; curr_sect<NUMSECT; ++curr_sect)
		if(*(sectdfn[curr_sect])) 
		xfwrite(secthdr[curr_sect],1,SCNHSZ,txtfil,outfile);

	/* output read only and read/write section 		*/
	if (text.s_size+data.s_size)
           xfwrite(sections,1,text.s_size+data.s_size,txtfil,outfile);

	/* let the binder worry about the loader section	*/

        /* output .hash section and .debug section */
        if (typchk.s_size)
           xfwrite(hsh_table,1,typchk.s_size,txtfil,outfile);
        if (debug.s_size)
           xfwrite(dbg_table,1,debug.s_size,txtfil,outfile);
	/* output remaining sections */
        if (text_nreloc+data_nreloc)
        xfwrite(rld_table,1,(text_nreloc+data_nreloc)*RELSZ,txtfil,outfile);
        if (text_nlnno+data_nlnno)
        xfwrite(line_table,1,(text_nlnno+data_nlnno)*LINESZ,txtfil,outfile);
        xfwrite(esd_table,1,hdr.f_nsyms*SYMESZ,txtfil,outfile);

        if (length_str)
        xfwrite(str_table,1,length_str+4,txtfil,outfile);

        xfclose(txtfil,outfile);


	if (listopt) 
	   cleanlst();

	delete();		 	/* delete any files	*/
#if 0
        exit(anyerrs !=0);   /* It was decided to generate the  */
                             /* obj file even if there is any   */
                             /* error for AIX 3.2.x compatibility */
#endif
        if ( anyerrs)       /* this change is for AIX 4.1: if there is  */
            delexit();      /* any error, delete all the files that  */
                            /* have  been created, and exit.         */
	/* NOTREACHED */
}
 
#ifdef CHAR_SET
/****************************************************************/
/* Function:	atoen						*/
/* Purpose:	convert ascii strings to ebcidic strings	*/
/* Input:	ebcdic_str - target string to contain ebcdic	*/
/*			     characters				*/
/*		ascii_str  - the ascii input string to convert	*/
/*		n          - the number of characters to convert*/
/* Output:	a converted string in ebcdic_str		*/
/****************************************************************/
atoen(ebcdic_str, ascii_str, n) 
char *ebcdic_str; 			/* target		*/
char *ascii_str; 			/* source		*/
int n;					/* number of chars 	*/
{
int j;	

		/* the ascii to ebcidic table converted from the PL8	*/
		/* runtime routine of the same function			*/
static  char ebcdic_base[] ={
/* 0    1    2    3    4    5    6    7    8    9    a   b     c    d    e  f*/
0x00,0x01,0x02,0x03,0x37,0x2d,0x2e,0x2f,0x16,0x05,0x25,0x0b,0x0c,0x0d,0x0e,0x0f,
0x10,0x11,0x12,0x13,0x3c,0x3d,0x32,0x26,0x18,0x19,0x3f,0x27,0x22,0x1d,0x35,0x1f,
0x40,0x5a,0x7f,0x7b,0x5b,0x6c,0x50,0x7d,0x4d,0x5d,0x5c,0x4e,0x6b,0x60,0x4b,0x61,
0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0x7a,0x5e,0x4c,0x7e,0x6e,0x6f,
0x7c,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,
0xd7,0xd8,0xd9,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xad,0xe0,0xbd,0x5f,0x6d,
0x79,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x91,0x92,0x93,0x94,0x95,0x96,
0x97,0x98,0x99,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xc0,0x4f,0xd0,0xa1,0x07,
0x43,0x20,0x21,0x1c,0x23,0xeb,0x24,0x9b,0x71,0x28,0x38,0x49,0x90,0xba,0xec,0xdf,
0x45,0x29,0x2a,0x9d,0x72,0x2b,0x8a,0x9a,0x67,0x56,0x64,0x4a,0x53,0x68,0x59,0x46,
0xea,0xda,0x2c,0xde,0x8b,0x55,0x41,0xfe,0x58,0x51,0x52,0x48,0x69,0xdb,0x8e,0x8d,
0x73,0x74,0x75,0xfa,0x15,0xb0,0xb1,0xb3,0xb4,0xb5,0x6a,0xb7,0xb8,0xb9,0xcc,0xbc,
0xab,0x3e,0x3b,0x0a,0xbf,0x8f,0x3a,0x14,0xa0,0x17,0xcb,0xca,0x1a,0x1b,0x9c,0x04,
0x34,0xef,0x1e,0x06,0x08,0x09,0x77,0x70,0xbe,0xbb,0xac,0x54,0x63,0x65,0x66,0x62,
0x30,0x42,0x47,0x57,0xee,0x33,0xb6,0xe1,0xcd,0xed,0x36,0x44,0xce,0xcf,0x31,0xaa,
0xfc,0x9e,0xae,0x8c,0xdd,0xdc,0x39,0xfb,0x80,0xaf,0xfd,0x78,0x76,0xb2,0x9f,0xff
/* 0    1    2    3    4    5    6    7    8    9    a   b     c    d    e  f*/
};
 
   for (j=0; j<n; j++)
	ebcdic_str[j] = ebcdic_base[ascii_str[j]];
}
#endif

/****************************************************************/
/* Function:	process_args					*/
/* Purpose:	process the arguments from the command line	*/
/* Input:	argc - number of arguments			*/
/*		argv - the argument vector			*/
/* 		outf -  pointer to output file name		*/
/* 		xcrossf - pointer to xcrossref file name	*/
/* 		listf - pointer to listing file name		*/
/* 		infnm - pointer to input file name		*/
/* Output:	outf -						*/
/* 		listf - 					*/
/* 		infnm - 					*/
/****************************************************************/
process_args(argc,argv,outf,xcrossf,listf,infnm)
 int argc;
 char **argv;
 char **outf;
 char **listf;
 char **xcrossf;
 char **infnm;
{
	int infound = 0;		/* input file already 	*/
					/* specified flag	*/
	char s[100];			/* buffer for error message */
	char *t, *t_save;		/* temp string pointer	*/
	char ff[6];			/* file format string	*/
        unsigned short asm_mode_save;
        int i;
 
	/* loop through arguments				*/
	while(argc > 1) {
		if(argv[1][0] == '-') {		/* next argument a flag */
			switch(argv[1][1]) {
#ifdef IDEBUG
			case 'd':		/* debug flag		*/
				if(isdigit(argv[1][2]))
						i_debug = atoi(&argv[1][2]);
					else
						++i_debug;
				break;
			case 'D':		/* debug flag 		*/
			   putdbg(put_string(&argv[1][2]));
			   break;
#endif
			case 'l':		/* produce listing flag	*/
				listopt++;	/* set list option flag	*/
				/* if there is a string after the l, 	*/
				/* then it is the listing name		*/
				if (argv[1][2]) {
					*listf = &argv[1][2];
				}
				break; 
			case 'n':	/* specifies alternate name	*/
					/* to go into listing		*/
					/* format is -nname		*/
				if(argv[1][2]) {
					*infnm = &argv[1][2];
				} else {
					/* or format can be -n name	*/
					if(argc <3) {
                                                /* modified message 167 */
						yyerror( 167,'n');
                                                yyerror( 170);
						exit(1);
					}
					*infnm = argv[2];
					--argc, ++argv;
				}
				break;
			case 'o':	/* specify output file name  	*/
					/* format is -oname		*/
				if(argv[1][2]) {
					*outf = &argv[1][2];
				} else {
					/* or format is -o name		*/
					if(argc <3) {
                                                /* modified message 167 */
						yyerror( 167,'o');
                                                yyerror( 170);
						exit(1);
					}
                                        /*if((argv[2][1]=='-')||(!(argv-2)){*/
                                                /* generate object name */
                                        /*}*/
                                        else
					*outf = argv[2];
					--argc, ++argv;
				}
				break;
			case 'w':		/* display warning	*/
						/* flags to user	*/
				warning = wFLAG;
				break;
			case 'W':		/* do not display warn-	*/
						/* ing flags to user	*/
				warning = WFLAG;
				break;
			case 'x':		/* assembler flag for xref */
				xflag = 1;
				/* if there is a string after the x, 	*/
				/* then it is the xcrossreffile name    */
				if (argv[1][2]) {
					*xcrossf = &argv[1][2];
				}
				break;
#ifdef CHAR_SET
			case 'e':		/* produce ebcidic symbols*/
				setebcdic();
				break;
#endif
			case 'u':  	  /* undefined symbols not flagged*/
					  /* undefined symbols made E_EXT */
				flag_extern_OK = 1;
				break;

                        case 'm':         /* if -m is used, then assembler */
                                          /* mode is given. if -m is not   */
                                          /* used, then the default ( i.e. */
                                          /* COM value is given   )      */
                               modeopt++;
                               if (argv[1][2])   /* format is -mmodename */
                                  t = &argv[1][2];
                               else {  /* format can be -m modename   */
                                  if (argc < 3 ) {
                                     yyerror ( 167, 'm');
                                     yyerror ( 170);
                                     exit(1);
                                  }
                                  t = argv[2];
                                  --argc, ++argv;
                               }
                               if ( *t == '\0') /* a null string argument */
                                 modeopt = 0;
                               else {
                                 t_save = t;
                                 for (  ; (*t); t++ )
                                    *t = toupper(*t);
                                 asm_mode = cvt_asmmode(t_save, '1'); 
                                 strcpy(asm_mode_str, t_save);
                                 if ( asm_mode == 0 )  /* if an invalid value */
                                         /* was given, reset asm_mode using  */
                                         /* default value for the rest of the inst.   */
                                         /* validation */
                                 {
                                    strcpy(asm_mode_str, "COM");
                                    asm_mode = COM ;
                                 }
                                 cmd_asm_mode = asm_mode; /* save this assembly */
                                                        /* mode value for     */
                                                        /* Pass 2             */
                                 strcpy(cmd_asm_mode_str, asm_mode_str);
                               }
                               cmd_modeopt = modeopt;
                               break;

                        case 's':   /* show xref for POWER and PowerPC    */
                                    /* mnemonics in assemble listing.     */
                                    /* -s also implies -l                 */ 
                                listopt++;
			            /* if there is a string after the s,   */
                                    /*  and listing name has not defined by */
                                    /* the '-l' option, then it is the      */
				    /* the listing name		*/
			       if (argv[1][2] && !*listf )
					*listf = &argv[1][2];
                               mn_xref = 1;
                               break;

			default:
                                                /* modified message 168 */
				yyerror( 168,argv[1]);
                                yyerror(170);
				exit(-1);
			}   /* end of switch  */
		} else {
			if(infound) {
                                                /* message 169 */
				yyerror( 169 );
                                yyerror ( 170);
				exit(1);
			}
			infound++;
			infile = argv[1];
			strcpy(ff,"r");
			if(freopen(infile,ff, stdin) == NULL) {
                                                /* message 038 */
				yyerror( 38 , argv[1]);
				exit(2);
			}
			/* setbuf does not work on CMS		*/
			setbuf(stdin,_sibuf);
			orig_infile_arg = infile;
		}
		--argc, ++argv;
	}
#ifdef IDEBUG
   if (indbg("testdbg"))
      ddbg();
#endif
}

xexit()
{
	delete();
	exit(0);
}

/****************************************************************/
/* Function:	del						*/
/* Purpose:	delete opened files without exit		*/
/* Comments:	vector here for mem fault, etc. 		*/
/****************************************************************/
del()	
{
	delete();
	if (outfile_opened)
	  unlink(outfile);
}

/****************************************************************/
/* Function:	delsig						*/
/* Purpose:	delete opened files without exit with signal	*/
/* Comments:	vector here for mem fault, etc. 		*/
/****************************************************************/
void
delsig()
{
	del();
}
 
/****************************************************************/
/* Function:	delexit						*/
/* Purpose:	delete opened files and exit			*/
/* Comments:	exits with a value of 1, called on error	*/
/****************************************************************/
delexit()
{
	delete();
	if (outfile_opened)
	  unlink(outfile);
	exit(1);
}
 
/****************************************************************/
/* Function:	delexitsig					*/
/* Purpose:	delete opened files and exit with signal	*/
/* Comments:	exits with a value of 1, called on error	*/
/****************************************************************/
void
delexitsig()
{
	delexit();
}
 
/****************************************************************/
/* Function:	delete						*/
/* Purpose:	delete opened temporary files			*/
/****************************************************************/
delete()
{
	register c;
 
#ifndef NFILE
	 if(tmpn1)
		unlink(tmpn1);
	 if(tmpin)
		unlink(tmpin);
#endif
}
 
/****************************************************************/
/* Function:	outb						*/
/* Purpose:	write a byte to the object file			*/
/* Input:	val - the byte to write				*/
/* Comments:	This routine depends on ccsect to be set to	*/
/* 		the current csect				*/
/****************************************************************/
outb(val)
 int val;
{
char *addr;

        if (listopt)			/* put data in listing	*/
	      collectlst(ccsect->secname->name, ccsect->dot, 
	      		(unsigned char)val, 1);
	if (passno==2) {		/* only write data in pass2*/
	   /* do not output data if we are in a dsect		*/
	   if (ccsect->ssclass!=C_DSECT) {
		/* add the begining of the output buffer with 	*/
		/* the offset into that buffer with the offset */
		/* into the csect				*/
	   	addr = sections + ccsect->dot + ccsect->start;
           	*addr = (unsigned char)val;
		}
	   }
	ccsect->dot++;			/* increment the current*/
}					/* location counter	*/
 
/****************************************************************/
/* Function:	outh						*/
/* Purpose:	write a short (halfword) to the object file	*/
/* Input:	val - the value to write			*/
/* Comments:	This routine depends on ccsect to be set to	*/
/* 		the current csect				*/
/****************************************************************/
outh(val)
int val;
{
char *addr;

	if (listopt)			/* write data in listing*/
	      collectlst(ccsect->secname->name, ccsect->dot, 
			(unsigned short)val, 2);
	if (passno==2) {		/* only write in pass 2	*/
	   if (ccsect->ssclass!=C_DSECT) {	/* no output to dsect */
		/* addr of output buf + offset in csect + offset in buf*/
	   	addr = (char *)(sections + ccsect->dot + ccsect->start);
           	*addr++ = val>>8;
		*addr = val;
	   	}
	   }
	ccsect->dot += 2;	/* increment current location counter*/
}
 
/****************************************************************/
/* Function:	outl						*/
/* Purpose:	write a long (fullword) to the object file	*/
/* Input:	val - the value to write			*/
/* Comments:	This routine depends on ccsect to be set to	*/
/* 		the current csect				*/
/****************************************************************/
outl(val)
long val;
{
char *addr;

	   if (listopt)			/* write data in listing*/
	      collectlst(ccsect->secname->name, ccsect->dot, val, 4);
	if (passno==2) {		/* only write in pass 2 */
	   /* do not output to dsect	*/
	   if (ccsect->ssclass!=C_DSECT) {	
		/* addr of output buf + offset in csect + offset in buf*/
	   	addr = (char *)(sections + ccsect->dot+ccsect->start);
           	*addr++ = val>>24;
           	*addr++ = val>>16;
           	*addr++ = val>>8;
           	*addr = val;
		}
	   }
	ccsect->dot += 4;	/* increment current location counter */
}
 
/****************************************************************/
/* Function:	putflt						*/
/* Purpose:	write floating point number			*/
/* Input:	fp   - pointer to floating point number to put	*/
/*		size - the size of the floating point number	*/
/*			4 or 8 for float or double		*/
/* Output:	writes to the output buffer (sections)		*/
/****************************************************************/
putflt(fp, size)
 register double *fp;
 register int size;
{
	union {			/* union allows output of a 	*/
		float f;	/* long instead of a float	*/
		long fvalue;
	} _fv;

	union {			/* union allows output of	*/
		double d;	/* a word at a time		*/
		struct {
			long x;
			long y;
		} _v;
	} _dv;
 
	if(size == sizeof(float)) {	/* if float then		*/
		_fv.f = *fp;		/* on AIX caste converts	*/
					/* double to float		*/
		outl(_fv.fvalue); 
	} else {			/* else is a .double	*/
			doalign(4);	/* align to double word	*/
			/* keep track of max alignment in csect	*/
			ccsect->algnmnt = MAX(ccsect->algnmnt,3);
			_dv.d = *fp;
			outl(_dv._v.x);
			outl(_dv._v.y);
	}
}
 
 
/* outexpr: write an expression for .byte, .short, or .long */
/* called in pass 2 */
 
/****************************************************************/
/* Function:	outexpr						*/
/* Purpose:	write an expression for .byte, .short, or .long	*/
/* Input:	len - the length of the of the data to write	*/
/*		xp  - a pointer to the expression to write	*/
/* Output:	outputs the appropriate length expression 	*/
/* Comments:	called in pass 2				*/
/****************************************************************/
outexpr(len, xp)
struct exp *xp;
{
int bitlen;
char *p;
 
  if (passno == 2) {
    if (len>4 && xp->xtype!=E_ABS) 
                                                /* message 039 */
       yyerror( 39 );
    else {
                              /* ignore output BS or UC csects*/
       if (ccsect->secname->etype==XTY_CM)
           return;

       bitlen = len*8;		/* number of bits to output	*/
       switch(xp->xtype) {	/* output appropriate RLD accord*/
				/* ing to expression type	*/
	   case E_ABS:		/* absolute expression, no RLD	*/
		   break;
	   case E_REL:		/* relative expr	*/
                   if (xp->x_rrtype == R_POS || xp->x_rrtype == RLD_NOT_SET) {
		     xp->xvalue += xp->xloc->start;
                     addrld(R_POS,bitlen,xp->xname);
                   } else {
                      if(xp->x_rrtype==R_NEG) {
                          xp->xvalue -=  xp->xloc->start;
                          addrld(R_NEG,bitlen,xp->xname);
                      }
                   }
                   break;
	   case E_EXT:		  /* external expr                 */
                   if ( xp->x_rrtype == RLD_NOT_SET )
                     addrld(R_POS, bitlen, xp->xname);
                   else 
		     addrld(xp->x_rrtype,bitlen,xp->xname);
		   break;
	   case E_TREL:		/* TOC relative expression	*/
                   if (xp->x_rrtype == R_POS || 
                                    xp->x_rrtype == RLD_NOT_SET) {
                     xp->xvalue += tocname->csect->start;
                     addrld(R_POS,bitlen,xp->xname);
                   } else if( xp->x_rrtype==R_NEG) {
                          xp->xvalue -= tocname->csect->start;
                          addrld(R_NEG,bitlen,xp->xname);
                   }
		   break;
	   case E_TOCOF:		/* TOCOF symbol			*/
		   if (xp->xname->etype==XTY_ER)	/* via .tocof	*/
		      addrld(R_GL,bitlen,xp->xname);
		   else if (xp->xname->etype==XTY_SD) /* my TOC	*/
		      addrld(R_TCL,bitlen,xp->xname);
		   break;
	   case E_REXT:		/* red external, means need two	*/
				   /* RLDs 			*/
		   addrld(xp->rldinfo.rrtype,bitlen,xp->rldinfo.rname);
		   addrld(xp->x_rrtype,bitlen,xp->xname);
		   break;
          case E_UNK:
          default:
                                                /* message 040 */
	          yyerror( 40 );
	          break;
	  }
      }	
   }
	switch(len) {
case 1: 	outb((int)xp->xvalue);	break;
case 2: 	outh((unsigned short)xp->xvalue);	break;
case 4: 	outl((long)xp->xvalue); break;
default:	for (p = (char *)&xp->xvalue + 4-len; (len); len--) 
		outb(*p++);
	}
}

#ifdef IDEBUG
/****************************************************************/
/* debug table and debug routines				*/
/****************************************************************/
#define NDT 20			/* size of debug table		*/
static char *dbgtab[NDT];	/* debug string pointer table	*/
static int ndt = 0;		/* next debug table entry	*/
#endif

#ifdef IDEBUG
/****************************************************************/
/* Function:	putdb						*/
/* Purpose:	put a string in the debug table 		*/
/* Input:	s - the string to put				*/
/****************************************************************/
putdbg(s)
char *s;
{
   if (ndt>=NDT) 
   fprintf(stderr,"debug tab full:%s\n",s);
   else dbgtab[ndt++] = s;
}
#endif

#ifdef IDEBUG
/****************************************************************/
/* Function:	indbg						*/
/* Purpose:	check if a string is in the debug table (ala PL8 */
/*     		indebug)					*/
/* Input:	s - the string to search for			*/
/* Returns:	the index of the input string or 		*/
/*		0 - if not found				*/
/* Comments:	usually called with debug code - 		*/
/*		if (indebug("blah")) debug print statements	*/
/****************************************************************/
int indbg(s)
char *s;
{
int i;

   for(i=0; i<ndt; i++)
      if (!strcmp(s,dbgtab[i])) 
	 return 1;
   return 0;
}
#endif

#ifdef IDEBUG
/****************************************************************/
/* Function:	ddbg						*/
/* Purpose:	debug debug routines				*/
/* Output:	dumps the debug strings				*/
/****************************************************************/
ddbg()
{
int i;

   for(i=0; i<ndt; i++) {
      printf("dbg[%d]:%s ",i,dbgtab[i]);
      if (!((i+1)%5)) printf("\n");
     }
   printf("\n");
}

#endif

/****************************************************************/
/* Function:	print_lineno					*/
/* Purpose:	prints the symbol and line number as yacc parses*/
/*		through the assembler code for symbols & lineno	*/
/* Input:	sym  - the symbol that goes with the line number*/
/*		r    - the DEF or REF flag			*/
/*		lineno - the linenumber that goes with the sym	*/
/* Output: 	writes xref information to xoutfil		*/
/****************************************************************/
print_lineno(sym, r, lineno)
struct symtab *sym; int r, lineno;
{
	char *csectname,*cname;	char *star;
	
	if ( *(sym->name)) {
	    csectname= ((sym->csect) && *(cname=sym->csect->secname->name)
			 && (*cname != '%'))  ? cname: "--";
	    star = (r < 0) ? "*": " ";
            if (strlen(sym->name) < 8)
	    fprintf(xoutfil, "%s\t\t%s", sym->name, infile);
            else
	    fprintf(xoutfil, "%s\t%s", sym->name, infile);

            if (strlen(csectname) < 8)
	    fprintf(xoutfil, "\t%s\t\t\t%d\t%s\n", csectname, lineno, star);
            else
	    fprintf(xoutfil, "\t%s\t\t%d\t%s\n", csectname, lineno, star); 

	}
}

/*********************************************************
*  Function: sortfile
*  Purpose:  Construct the File and ArgumentV  parameters for 
*            execvp routine, then call callsys routine to sort
*            symbol crodd reference file.
*  Return: report error 115 if call failed.
**********************************************************/
char sort[] = "/bin/sort";  /* sort routine called*/
sortfile()
{
	/* sorts xcrossfile  */

        char *arv[9];             /*arguments that syscall uses*/
	register int status;

	arv[0] = "sort";
	arv[1] = "-o";
	arv[2] = xcrossfile;
	arv[3] = "+0";
	arv[4] = "-1";      
	arv[5] = "+3";      
	arv[6] = "-4";      
	arv[7] = "-n";      
	arv[8] = xcrossfile;
	arv[9] = 0;
	/* execute sort */
	if ((status = callsys(sort, arv)) > 0)
				/* message 115 */
		yyerror( 115 , status);
}

/*****************************************************************
*  Function: callsys
*  Purpose:  Creates a child process to execute a new program 
*            which is a shell procedure.
*  input: f - a pointer to the name of the file containing the
*             shell procedure
*         v - An array of pointers to null-terminated char 
*             strings. These strings constructed the argument 
*             list available to the executing shell procedure.
*  return:  Child process status.
******************************************************************/
callsys(f, v)
	char f[], *v[];
{
	register int pid, w;
	int status;

	if ((pid = fork()) == 0) {
		/* only the child gets here */
		execvp(f, v);
		yyerror( 116, f);
	}
	else
		if (pid == -1)
			/* fork failed - tell user */
			yyerror( 116, f);
	/*
	 * loop while calling "wait" to wait for the child.
	 * "wait" returns the pid of the child when it returns or
	 * -1 if the child can not be found.
	 */
	while (((w = wait(&status)) != pid) && (w != -1));
	if (w == -1) 
		/* child was lost - inform user */
		yyerror( 116, f);
	else {
		/* check system return status */
		if (((w = status & 0x7f) != 0) && (w != SIGALRM)) {
			/* don't complain if the user interrupted us */
			if (w != SIGINT) {
				fprintf(stderr, "Fatal error in %s", f);
				yyerror( 116, f);
			};
			/* remove temporary files */
			delexit();
		};
	};
	/* return child status only */
	return((status >> 8) & 0xff);
}

/*****************************************************************
*  Function: set_cpuid
*  Purpose:  Convert the sum_bit_map into the CPU ID, and stored
*            into the .file entry of the symbol table.
******************************************************************/
set_cpuid()
{
    switch (sum_bit_map) {
       case  SRC_PW_PPC:
           symtab->type = CPUID_COM;
           break;
       case SRC_PW_601:
       case SRC_PW_NO601:
           symtab->type = CPUID_RIOS_1;
           break;
       case SRC_RS2_ONLY:
           symtab->type = CPUID_RIOS_2;
           break;
       case SRC_RS2_PPC:
            /* asm_mode could not be COM or PWR for this case.  */
           if ( asm_mode == PWR2 )
              symtab->type = CPUID_RIOS_2;
           else  /* asm_mode is PPC, 601 or ANY  */
              symtab->type = CPUID_PPC_32BIT;
           break; 
       case SRC_RS2_601:
           if ( asm_mode == PWR2 )
              symtab->type = CPUID_RIOS_2;
	   else /* asm_mod must be 601 ... cpuid "601" not set */
              symtab->type = CPUID_ANY;
           break;
       case SRC_PPC_601:
       case SRC_PPC_NO601:
           symtab->type = CPUID_PPC_32BIT;
           break;
       case SRC_601_ONLY: /* CPUID "601" is no longer counted. */
       case SRC_PPC_OPT1:
       case SRC_PPC_OPT2:
       case SRC_603_ONLY:
       case SRC_ANY:
           symtab->type = CPUID_ANY;
           break;
       default:
           if ( asm_mode == ANY )
           	symtab->type = CPUID_ANY;
	   else
            	yyerror(152);
           break;
    }
} 
      
/*****************************************************************
*  Function: process_lang_id
*  Purpose:  If the src_lang_id indicates "assembler" but the 
*            file suffix in .file entry is not ".s", then it may 
*            happen that the default value is used because no
*            .source is present. The src_lang_id should be adjusted
*            according to the file suffix in .file entry.
*  Input:    None
*  Output:   src_lang_id is updated
******************************************************************/
void
process_lang_id(void)
{   
     char *suf_str;       /* hold language suffix  */
     int  i;
     int id_match = 0;

     char *C_sufs[] = {".c", NULL};   /* C, id = 0 */
     char *F_sufs[] = {".f", ".f77", ".for",NULL}; /* fortran, id = 1  */
     char *PAS_sufs[] = {".pas", ".p", NULL}; /* pascal, id = 2  */
     char *ADA_sufs[] = {".ada",NULL};  /* ada , id = 3    */
     char *PL1_sufs[] = {".pl1", ".pli",NULL}; /* PL/1 , id = 4   */
     char *BAS_sufs[] = {".bas",NULL}; /* BASIC , id = 5  */
     char *LISP_sufs[] = {".lisp", ".el",NULL }; /* LISP , id = 6 */
     char *CBL_sufs[] = {".CBL", ".cbl", ".int",NULL}; /* COBOL , id = 7 */
     char *MOD_sufs[] = {".mod",NULL}; /* MODULA-2 , id = 8 */
     char *CPLUS_sufs[] = {".C", ".cc",NULL}; /* C++ , id = 9     */
     char *RPG_sufs[] = {".RPG", ".rpg",NULL}; /* RPG , id = 10  */
     char *PL8_sufs[] = {".pl8", ".plix", NULL}; /* PL8 or PLIX , id = 11 */

     char  **suf_arr[] = {
             C_sufs, F_sufs, PAS_sufs, ADA_sufs, PL1_sufs,
             BAS_sufs, LISP_sufs, CBL_sufs, MOD_sufs, CPLUS_sufs,
             RPG_sufs, PL8_sufs}; 

#define   ASM_ID   12
#define   C_ID     0
   if ( src_lang_id == LANG_ID_UNKNOWN  && src_file_name)
      if ( (suf_str = strrchr(src_file_name,'.')) != '\0' ) {
         for ( i = C_ID; i< ASM_ID ; i++) {
            while (*suf_arr[i] != NULL ) {
              if (!strcmp(*suf_arr[i], suf_str )) {
                 id_match = 1;
                 break;
              }
              suf_arr[i]++;
            }
            if ( id_match )
               break;
         }
         src_lang_id = i;
      }

   if ( src_lang_id == LANG_ID_UNKNOWN )
      src_lang_id = ASM_ID;
}
