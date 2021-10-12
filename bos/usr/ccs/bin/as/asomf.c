static char sccsid[] = "@(#)80	1.18.1.6  src/bos/usr/ccs/bin/as/asomf.c, cmdas, bos411, 9428A410j 4/28/94 11:48:52";
/*
 * COMPONENT_NAME: (CMDAS) Assembler and Macroprocessor 
 *
 * FUNCTIONS: addDBGstr, addESDstr, addHSHstr, add_strt, addcs, addlist,
 *            bldtables, collect_for_ESD, decrypt, dmpESDs, dmp_hdr,
 *            dmpesd, dmpline, dmprld, dmpstring, encrypt, get_sc,
 *            line_collect, setascii, setebcdic, sort_csects, tocanchor,
 *            assign_hdr_num
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
/* as assembler object module specific routines, TOC version	*/
/****************************************************************/
#include <string.h>
#include "as.h"

/****************************************************************/
/* XCOFF header initialized to default settings			*/
/****************************************************************/
FILHDR hdr = {
        MAGIC_NUMBER,            /* magic number		*/
	0,			/* number of sections   	*/
	0,               	/* time & date stamp            */
	0,			/* file pointer to symtab       */
	0,			/* number of symtab entries     */
	0,		 	/* sizeof(optional hdr)		*/
	0			/* flags			*/
};

SCNHDR text = {
	_TEXT,                   	/* section name */
	0,   		        	/* physical address */
	0,  		        	/* virtual address */
	0,  		       		/* section size */
	0,  		         	/* file ptr to raw data for section */
	0,  		         	/* file ptr to relocation */
	0,	 	          	/* file ptr to line numbers */
	0,                              /* number of relocation entries */
	0,            	        	/* number of line number entries */
	STYP_TEXT  		       	/* flags */
	};

SCNHDR data = {
	_DATA,                   	/* section name */
	0,   		        	/* physical address */
	0,  		        	/* virtual address */
	0,  		       		/* section size */
	0,  		         	/* file ptr to raw data for section */
	0,  		         	/* file ptr to relocation */
	0,	 	          	/* file ptr to line numbers */
	0,                              /* number of relocation entries */
	0,            	        	/* number of line number entries */
	STYP_DATA   		       	/* flags */
	};

SCNHDR bss  = {
	_BSS,                    	/* section name */
	0,   		        	/* physical address */
	0,  		        	/* virtual address */
	0,  		       		/* section size */
	0,  		         	/* file ptr to raw data for section */
	0,  		         	/* file ptr to relocation */
	0,	 	          	/* file ptr to line numbers */
	0,                              /* number of relocation entries */
	0,            	        	/* number of line number entries */
	STYP_BSS   		       	/* flags */
	};

SCNHDR typchk  = {
	_TYPCHK,                 	/* section name */
	0,   		        	/* physical address */
	0,  		        	/* virtual address */
	0,  		       		/* section size */
	0,  		         	/* file ptr to raw data for section */
	0,  		         	/* file ptr to relocation */
	0,	 	          	/* file ptr to line numbers */
	0,                              /* number of relocation entries */
	0,            	        	/* number of line number entries */
	STYP_TYPCHK   		       	/* flags */
	};

SCNHDR debug  = {
	_DEBUG,                  	/* section name */
	0,   		        	/* physical address */
	0,  		        	/* virtual address */
	0,  		       		/* section size */
	0,  		         	/* file ptr to raw data for section */
	0,  		         	/* file ptr to relocation */
	0,	 	          	/* file ptr to line numbers */
	0,                              /* number of relocation entries */
	0,            	        	/* number of line number entries */
	STYP_DEBUG   		       	/* flags */
	};

SCNHDR text_aux = {
	_OVRFLO,                        /* section name: text overflow */
	0,   		        	/* physical address */
	0,  		        	/* virtual address */
	0,  		       		/* section size */
	0,  		         	/* file ptr to raw data for section */
	0,  		         	/* file ptr to relocation */
	0,	 	          	/* file ptr to line numbers */
	0,                              /* number of relocation entries */
	0,            	        	/* number of line number entries */
	STYP_OVRFLO		       	/* flags */
	};

SCNHDR data_aux = {
	_OVRFLO,                        /* section name: data overflow */
	0,   		        	/* physical address */
	0,  		        	/* virtual address */
	0,  		       		/* section size */
	0,  		         	/* file ptr to raw data for section */
	0,  		         	/* file ptr to relocation */
	0,	 	          	/* file ptr to line numbers */
	0,                              /* number of relocation entries */
	0,            	        	/* number of line number entries */
	STYP_OVRFLO 		       	/* flags */
	};

int length_str= 0;           /* length of string table */

/* array of csect storage class names used for both printing and	*/
/* by yylex (via get_cs) to set storage class				*/
char *class_name[] = {"PR", "RO", "DB", "TC", "UA", "RW", "GL",
                    "XO", "SV", "BS","DS", "UC", "TI", "TB",
                    "\0xff", "TC0", "TD", "DSECT","LC"}; 
 
/****************************************************************/
/* Section Numbers of Basic Section Headers in XCOFF            */
/****************************************************************/

       int SECTION_NUM=1;
extern int XN_TEXT,
           XN_DATA,
           XN_BSS,
           XN_TYPCHK,
	   XN_DEBUG; 

extern int text_nreloc,
	   data_nreloc;

extern int text_nlnno,
           data_nlnno;

int XN_TEXT_AUX,
    XN_DATA_AUX;

static char *symt=0,            /* sym table and reloc pointers  */
            *rloct=0;

static int DBGoffset;           /*offset into the DBG TABLE*/
static char *hsh=0,             /* hash table pointers */
            *hsh1=0;

static int cspsz=0;
static SECT *secte=0;


/****************************************************************/
/* Function:	bldtables					*/
/* Purpose:	bld toc specific tables to put in the binder 	*/
/*		section, the tables built are			*/
/*			String table				*/
/*			ESD table				*/
/*			RLD table				*/
/* Input:	strtab - the pointer to the string table	*/
/*		esd    - pointer to esd table to fill in	*/
/*		rld    - pointer to rld table to fill in	*/
/*		hshtab - pointer to typchk table to fill in	*/
/*		dbgtab - pointer to debug table to fill in	*/
/*		linetab - pointer to lineno table to fill in	*/
/* Output:	tables filled in				*/
/****************************************************************/
bldtables(strtab,binsym,rld,hshtab,dbgtab,linetab) 
char *strtab;			/* string table pointers	*/
char *binsym;			/* symbol table pointers 	*/
char *rld; 			/* RLD table pointers		*/
char *hshtab;			/* hash table pointer		*/
char *dbgtab;                   /* debug table pointer          */
char *linetab;                  /* linenumber table pointer     */
{

	SECT *csp;			/* tmp csect pointer	*/
	XRLD *relp;			/* tmp reloc pointer	*/
	SYMENT sym;         		/* ESD structure     	*/
	RELOC rld1; 			/* RLD structure    	*/
	struct symtab *sp;
	AUXENT	taux;			/* auxilary structure */
        unsigned char  save_type;       /* C_FILE type        */
 

	/* initialize the extra table pointers		*/
        symt = binsym; rloct = rld; 
        hsh = hsh1 = hshtab;

/****************************************************************/
/* ASSIGN SYMBOL TABLE ENTRIES **********************************/
/****************************************************************/
				
	/* --- loop over esd chain ----- */
	for (sp=esdanc; (sp); sp = sp->nxtesd) {
 	/* initialize symbol entries */
	   bzero(&sym,SYMESZ);

/* DETERMINE AND ASSIGN VALUE FIELD ****************************/
	if (sp->etype!=XTY_DBG&&sp->sectnumber!=N_DEBUG) {
           if (!(sp->etype & sp->csect_aux.x_smtyp))
	         sp->csect_aux.x_smtyp=
                         (0<<3) | (sp->etype & 07);        /*csect type     */

	   switch(  ((sp->csect_aux.x_smtyp) & 07)  ) {
	      case XTY_LD:
		sym.n_value = sp->value+sp->csect->start;
		sp->csect_aux.x_smclas = sp->csect->ssclass;
                sp->sectnumber = sp->csect->secname->sectnumber;
                sp->csect_aux.x_scnlen = sp->csect->secname->esdind;
		break;
	      case XTY_SD:
	       	sym.n_value = sp->value;
                sp->csect_aux.x_scnlen = sp->csect->end;
	       	break;
	      case XTY_CM:
	       	sym.n_value = sp->value;
                sp->csect_aux.x_scnlen = sp->csect->end;
	       	break;
	      case XTY_ER:
		sym.n_value = sp->value;
		break;
	      default:
                                                /* message 048 */
		yyerror( 48 );
	      }
        }   
	else if (sp->sclass==C_BINCL||sp->sclass==C_EINCL)
		sym.n_value = sp->value+text.s_lnnoptr;
	     else
		sym.n_value = sp->value;

/* DETERMINE AND ASSIGN NAME FIELD *****************************/
		/* Assign NAME field                 */
		/* from mutually exclusive conditions*/
	   if (sp->rename)
	   	addESDstr(sp->rename, &sym, strtab);
	   else 
		/* Assign symbols with generated or no strings*/
	   if (!(sp->name)||(*sp->name=='%')) {
		sym.n_zeroes = 0;
		sym.n_offset = 0;
		sym.n_sclass = C_HIDEXT;  /* unnamed local external symbol */
                }
	   else 
		/* Assign symbols with debug strings*/
           if (sp->dbug)
                addDBGstr(sp->dbug, &sym, dbgtab);
	   else
		/* Assign symbols with just names  */
                addESDstr(sp->name, &sym, strtab);

		/* Assign symbols with hash strings*/
           if (sp->chktyp)
	   	 addHSHstr(sp, sp->chktyp);

/* DETERMINE AND ASSIGN SECTION NUMBER FIELD *******************/
	   sym.n_scnum = sp->sectnumber;
/* DETERMINE AND ASSIGN CLASS FIELD ****************************/
	   sym.n_sclass = sp->sclass;
/* DETERMINE AND ASSIGN TYPE FIELD *****************************/
	                 /* symbol type */
	   if(sp->numaux==2) {
		sym.n_type = DT_FCN<<4;
		sp->sclass = C_EXT;
		}
	   else
	   	sym.n_type = DT_NON;		/* symbol type */
/* DETERMINE AND ASSIGN NUMAUX FIELD ***************************/
	   sym.n_numaux = sp->numaux;	                                   
           if ( sp->sclass == C_FILE ) {
              sym.n_type = src_lang_id << 8;
              if ( sp->type != '\0' ) {
                save_type = sp->type;
                sym.n_type |= sp->type;
              } else
                sym.n_type |= save_type;
           }
	   encrypt(symt,&sym,SYMESZ);   /*place sym in symbol table*/
           symt+=SYMESZ;
/* DETERMINE AND ASSIGN AUX ENTRIES ****************************/
	   while (sp->numaux--) {
 	/* initialize auxiliary entries*/
	   bzero(&taux,AUXESZ);
                switch (sp->sclass) {
                             /* .file entry */
		case C_FILE:
		     if (sp->rename) {
                        taux.x_file._x.x_zeroes = 0;
                        taux.x_file._x.x_offset = sym.n_offset;
                        sym.n_offset = 0; 
                        strcpy(sym.n_name,".file");
                        encrypt(symt-SYMESZ,&sym,SYMESZ);
                        }
                      else
                        strcpy(taux.x_file.x_fname,sp->file_aux.name);
                      break;
		case C_FCN:
			     /* .bf/.ef entry */
		      taux.x_sym.x_misc.x_lnsz.x_lnno= sp->be_aux.lnno;
                      break;
                    
			    /* .function entry */
		case C_EXT:
		      if(sp->numaux){
			    /* produce function auxiliary information */
		      taux.x_sym.x_misc.x_fsize= sp->func_aux.fsize;
		      taux.x_sym.x_fcnary.x_fcn.x_lnnoptr= 
			sp->func_aux.lnnoptr+text.s_lnnoptr;
		      taux.x_sym.x_fcnary.x_fcn.x_endndx= sp->func_aux.endndx;
		      break;
		      }
		     

			    /* .csect entry */
                default:
	   	      taux.x_csect.x_scnlen = sp->csect_aux.x_scnlen;
	   	      taux.x_csect.x_parmhash = sp->csect_aux.x_parmhash;
	   	      taux.x_csect.x_snhash = sp->csect_aux.x_snhash;
	   	      taux.x_csect.x_smtyp = sp->csect_aux.x_smtyp;
                      if (sp->csect_aux.x_smclas)
	   	      taux.x_csect.x_smclas = sp->csect_aux.x_smclas;
                      else
                      if(sp->eclass<C_NUM) 
	   	      taux.x_csect.x_smclas = sp->eclass;
	   	      taux.x_csect.x_stab = sp->csect_aux.x_stab;
	   	      taux.x_csect.x_snstab = sp->csect_aux.x_snstab;
                      break;
                }
	    	encrypt(symt,&taux,AUXESZ); /*place aux in symbol table*/
                symt+=AUXESZ;
	   }
#ifdef IDEBUG
	       	     if (indbg("bldsym"))
  			fprintf(stderr,
		          "bldsym: name: %s scnm: %d numaux: %d\n",
				sp->name, sp->sectnumber,
				sp->numaux);
#endif
	 }
/****************************************************************/
/* ASSIGN LINENO TABLE ENTRIES **********************************/
/****************************************************************/
				/* fill linetab if necessary */
           if (linenum)
	    line_collect(linetab);	
/****************************************************************/
/* ASSIGN RELOCATION TABLE ENTRIES ******************************/
/****************************************************************/
#ifdef IDEBUG
        if (indbg("dmpstring")) {
       fprintf(stderr,"BEFORE RLDSOUT \n");
            dmpstring(strtab);
         }
#endif
			/* -- loop over csect chain -- */
         for (csp=sect; (csp); csp= csp->nxt)
  	    if (csp->ssclass != C_DSECT) {
  /* Reverse order of rld list, so addrs increase */
  	XRLD *relp2, *relp3;
  
  	       if (0!=(relp2 = csp->rent) && 0!= (relp = relp2->rnxt)) {
  			/* Reversal needed only if list length >= 2 */
  		  relp3 = 0;
  		  do {
  		     if (relp2->raddr < relp->raddr) yyerror(32);
  					/* sanity check */
  		     relp2->rnxt = relp3;
  		     relp3 = relp2;
  		     relp2 = relp;
  		     }
  		  while (0!=(relp = relp->rnxt));
  		  relp2->rnxt = relp3;
  		  csp->rent = relp2;
  	       }
			/* --- start looping over reloc --- */
	       for (relp = csp->rent; (relp); relp = relp->rnxt) {
                     rld1.r_rtype =  relp->rtype; 
		     rld1.r_rsize = 
#ifdef SIGNED_RLD
                             (RLDSIGN<<7)|((relp->rlen-1) & 0x1f); /* length */
#else
		             relp->rlen;
#endif
		     rld1.r_symndx =  relp->rname->esdind;
	       	     rld1.r_vaddr =  relp->raddr;
#ifdef IDEBUG
	       	     if (indbg("bldrld"))
  			fprintf(stderr,
          "bldrld: rtype: %x rlen: %x esdind: %x  r_addr: %x r_rsize: %x\n",
				relp->rtype,relp->rlen,
				relp->rname->esdind,
		  		rld1.r_vaddr, rld1.r_rsize);
        if (indbg("dmpstring")) {
       fprintf(stderr,"IN RLDSOUT \n");
            dmpstring(strtab);
         }
#endif
	    	encrypt(rloct,&rld1,RELSZ);
                rloct+=RELSZ;
	       }
	    }
#ifdef IDEBUG
        if (indbg("dmpesd")) dmpesd(binsym,strtab);
        if (indbg("dmprld")) dmprld(rld, binsym,strtab);
        if (indbg("dmpESDs")) dmpESDs();
        if (indbg("dmpstring")) dmpstring(strtab);
#endif
}
 
#define MAXSHORT 65534
#define OVRFLONUM 0xFFFF
/****************************************************************/
/* Function:	assign_hdr_num					*/
/* Purpose:	assign number of relocation entries         	*/
/*         	and assign number of line number entries        */
/* Input:	none                             		*/
/* Output:	none                                            */
/****************************************************************/
assign_hdr_num()
{
	if (text_nreloc > MAXSHORT || text_nlnno > MAXSHORT) {
		XN_TEXT_AUX = SECTION_NUM++;

		/* indicate that auxiliary header contains values*/
		text.s_nreloc= text.s_nlnno= OVRFLONUM;

		/* make auxiliary assignments                   */
		text_aux.s_paddr = text_nreloc;
		text_aux.s_vaddr = text_nlnno;	
		text_aux.s_nreloc= text_aux.s_nlnno= XN_TEXT;
	}
	else {
		text.s_nreloc = (short)text_nreloc;
		text.s_nlnno  = (short)text_nlnno;
	}

	if (data_nreloc > MAXSHORT || data_nlnno > MAXSHORT) {
		XN_DATA_AUX = SECTION_NUM++;

		/* indicate that auxiliary header contains values*/
		data.s_nreloc= data.s_nlnno= OVRFLONUM;

		/* make auxiliary assignments                   */
		data_aux.s_paddr = data_nreloc;
		data_aux.s_vaddr = data_nlnno;	
		data_aux.s_nreloc= data_aux.s_nlnno= XN_DATA;
	}
	else {
		data.s_nreloc = (short)data_nreloc;
		data.s_nlnno  = (short)data_nlnno;
	}
}

/****************************************************************/
/* Function:	encrypt						*/
/* Purpose:	place structures in malloc sections         	*/
/* Input:	ptr1 - pointer to malloc section 		*/
/*       	ptr2 - pointer to structure      		*/
/*       	size - size of structure         		*/
/* Output:	ptr1 is advanced size bytes                     */
/****************************************************************/
encrypt(ptr1,ptr2,size)
char *ptr1, *ptr2;
int size;
{
     int i;
     for (i=0; i<size; i++)
     ptr1[i]=ptr2[i];
}

#ifdef IDEBUG
/****************************************************************/
/* Function:	decrypt						*/
/* Purpose:	remove structures in malloc sections         	*/
/* Input:	ptr1 - pointer to malloc section 		*/
/*       	ptr2 - pointer to structure      		*/
/*       	size - size of structure         		*/
/* Output:	ptr1 is advanced size bytes                     */
/****************************************************************/
decrypt(ptr1,ptr2,size)
char *ptr1, *ptr2;
int size;
{
     int i;
     for (i=0; i<size; i++)
     ptr2[i]=ptr1[i];
}

/****************************************************************/
/* Function:	dmpesd 						*/
/* Purpose:	dump the binary symbol table for debugging	*/
/* Input:	binsym - pointer to symbol table 		*/
/* Output:	prints symbol entries				*/
/****************************************************************/
dmpesd(binsym,strt)
char *strt;
char *binsym;
{ 
	SYMENT tsyment;		/* pointer to symbol structure */
	int i, naux;			/* loop counters */
	char *name;
	AUXENT tauxent;		/* pointer to aux. structure */
	int numof_aux;			/* number of auxialry entry */
	int flag_aux=0;			/* flag whether any aux entry */

   symt=binsym;
   if (binsym) 
      fprintf(stderr, "symbol       len       type       sclass       ");
      fprintf(stderr, "value     secnum      numaux\n");
		/* looping over all symbols */
       for (i=hdr.f_nsyms; (i); i--) {

       decrypt(symt,&tsyment,SYMESZ);
       symt += SYMESZ;
	if (tsyment.n_zeroes )  
	       name=tsyment.n_name; 
	else  if (tsyment.n_offset)
	       name= strt+(tsyment.n_offset); 
              else name="blanksym";


	fprintf(stderr,"%-10s   %3d       0x%.4x     ", 
	  name, strlen(name), tsyment.n_type);
	fprintf(stderr, "0x%.2x         %d          %d          %d\n",
	  tsyment.n_sclass, tsyment.n_value, tsyment.n_scnum, 
	  tsyment.n_numaux);

	  numof_aux =tsyment.n_numaux;
	  				/* loop over the auxiliary enteries */
	  if (numof_aux > 0)	/*   if any aux. symbol print header*/ 
		fprintf(stderr,"          AUX: scnlen    parmhash     ");
		fprintf(stderr, "snhash    align   type   smclas\n");
	  if (numof_aux >0)  {	
          decrypt(symt,&tauxent,AUXESZ);
          symt += AUXESZ;
          --i;
		fprintf(stderr,"          %8x     %8x      ",
			tauxent.x_csect.x_scnlen,
			 tauxent.x_csect.x_parmhash,
			  tauxent.x_csect.x_snhash);
		fprintf(stderr, "%4x         %1x      %1x      %2x\n",
			   (tauxent.x_csect.x_smtyp&0xf8)>>3,
			    tauxent.x_csect.x_smtyp&0x0f,
			     tauxent.x_csect.x_smclas);       
		flag_aux =1;
 
	 }
	 if (flag_aux > 0)
		fprintf(stderr,"\n");
	 flag_aux =0;

   }
}

/****************************************************************/
/* Function:	dmprld						*/
/* Purpose:	dump the rld table, mainly used for debugging	*/
/* Input:	rld - pointer to relocation table		*/
/*		binsym - pointer to esd table which rld entries	*/
/*		      reference					*/
/****************************************************************/
 dmprld(reloc,binsym,strt)
 char *reloc;
 char *binsym;
 char *strt;
 {
 SYMENT relsym;
 RELOC treloc;
 int i,j;
 char *ename;

    rloct=reloc;                        /*reset pointer to RELOC table*/
    if (reloc)
       fprintf(stderr,
	 "     addr      symbolind        symbol        type      length\n");
   for (i=text_nreloc+data_nreloc; (i); i--) {

       decrypt(rloct,&treloc,RELSZ);
       rloct += RELSZ;
                           /*obtain symbol entry*/
       decrypt(binsym+treloc.r_symndx*SYMESZ,&relsym,SYMESZ);
	if (relsym.n_zeroes)  
		ename=relsym.n_name; 
	else if (relsym.n_offset) 
		ename= strt+( relsym.n_offset); 
             else
		ename="blanksym";
      fprintf(stderr,
	"    %x          %d          %-10s         %x        %x\n",
	treloc.r_vaddr, 
	treloc.r_symndx, ename,
        treloc.r_rtype, treloc.r_rsize);
	}   
}

/****************************************************************/
/* Function: 	dmpESDs						*/
/* Purpose:  	dump the esd chain symbol table			*/
/* Comments:	Used for debugging				*/
/****************************************************************/
dmpESDs()
{
struct symtab *p;

 fprintf(stderr, "\n     name     value     sectnum  ");
 fprintf(stderr, "type   etype eclass esdind numaux sclass\n");
   for (p=esdanc; (p); p= p->nxtesd) {
      dmpsym(p);
      }
}

/****************************************************************/
/* Function: 	dmpstring					*/
/* Purpose:  	dump the string table produced by bldtables     */
/* Comments:	Used for debugging				*/
/****************************************************************/
dmpstring(strt)
char *strt;
{
	char *ptr;
        int maxset,item;

	maxset = 4 + *(int*)strt;	
        for(item=4;item<maxset;item++) {
	ptr = strt + item;
        if (ptr[0]=='\0')
           fprintf(stderr,"0");
        else
         fprintf(stderr,"%c",ptr[0]);
        if (item%75==0)
           fprintf(stderr,"\n");
        }
        fprintf(stderr,"\n");
} 
#endif
       
/****************************************************************/
/* Function:	addDBGstr					*/
/* Purpose:	add an entry to the debug string table		*/
/* Input:	s     - string to add				*/
/*		symtab   - symbol table entry        		*/
/****************************************************************/
addDBGstr(s, symtab, str_table)
char *s;                                /*debug string*/
SYMENT *symtab;				/* pointer to symbol table struct */
char *str_table;			/* string allocated malloc pointer */
{
char *ptr;
short slen;  				/* length of string */
					/* of the table to string table */
   if (!s) return 0;			/* if no string or symbol name */
   slen = (strlen(s)+1);

   if ( slen <= SYMNMLEN+1 )		/* if string length is 8 or less */
	 strcpy((symtab)->n_name, s);	/* add name to symbol table */
   else	{				/* if string length is 8 or more */
        DBGoffset+=2;
	ptr = str_table + DBGoffset;    /*curloc-2 of end of DEBUG TABLE*/
        strcpy(ptr, s);      	        /* copy string to allocated space */

        *(ptr-2) = slen>>8;        	/*length of string in first two bytes*/
        *(ptr-1) = slen;       		/*length of string in first two bytes*/
	symtab->n_zeroes = 0;
	symtab->n_offset=DBGoffset;	/* put the string tab. offset */
	               			/* in the symbol table */
        DBGoffset+=slen;                /* increase offset */
	}
}


        
#ifdef CHAR_SET
	int ebcdic=0;			/* ebcidic char set flag */
#endif

/****************************************************************/
/* Function:	addESDstr					*/
/* Purpose:	add an entry to the binsym string table		*/
/* Input:	s     - string to add				*/
/*		symtab   - symbol table entry        		*/
/****************************************************************/
addESDstr(s, symtab, strt)
char *s;
SYMENT *symtab;				/* pointer to symbol table struct */
char *strt;				/* string allocated malloc pointer */
{
unsigned int slen;			/* length of string */
					/* of the table to string table */
   if (!s) return 0;			/* if no string or symbol name */
   slen = (strlen(s)+1);

#ifdef CHAR_SET
	   if ( ebcdic )
   	      atoen(s,s,slen-1);
#endif

   if ( slen <= SYMNMLEN+1 )		/* if string length is 8 or less */
	strcpy((symtab)->n_name, s);	/* add name to symbol table */
   else	{				/* if string length is 8 or more */
	(symtab)->n_offset=		/* put the string tab. offset */
		add_strt(strt, s, slen);	/* add string to table */
	               			/* in the symbol table */
	symtab->n_zeroes = 0;
	}
}

/****************************************************************/
/* Function:	add_strt      					*/
/* Purpose:	adds symbol names longer than 8 character    	*/
/*		to the string table, and store the offset    	*/
/*		into the first 4 bytes of the string table 	*/
/* Input:	strt - the malloc pointer to string table       */
/*		s - string name					*/
/*		slen - length of the string name plus terminator*/
/* Output:	offset form begining of table to the string tab.*/
/* Returns:	offset                    		 	*/
/****************************************************************/
int add_strt(strt, s, slen)
char *strt;				/* malloc pointer to string table */
char *s;				/* symbol name as string */
int slen;				/* length of the string or symbol */
{
	char *ptr;
	int offset;			/* offset to the string table */

	offset = *(int*)strt;	
	ptr = strt + offset;
	strcpy(ptr, s);			/* copy string to allocated space */

	*(int*)strt +=slen;
#ifdef IDEBUG
        if (indbg("dmpstring")) {
       		fprintf(stderr,"IN ADD_STR offset=%d string=%s tabst=%s\n",
			offset,s,ptr);
            dmpstring(strt);
         }
#endif
	return(offset);
}


 
/****************************************************************/
/* Function:	addHSHstr					*/
/* Purpose:	add an entry to the HSH string table		*/
/* Input:	s     - string to add				*/
/****************************************************************/
int addHSHstr(sym, s)
struct symtab *sym;
char *s;
{
int flag,val;
short slen;
char *hsh2;

   if (!s) return;
   slen = (strlen(s)+1)/2;

   /* sanity check	*/
   if (hsh1+slen+2 > hsh+typchk.s_size) {
                                                /* message 049 */
	yyerror( 49 );
  	 return;
   	}

   if (slen) {
	   hsh2 = hsh1;
	*hsh2++ = slen>>8;
	*hsh2++ = slen;
	   flag = 0;
   	   while (*s) {
		if ((val = toupper(*s++))>='A'&&val<='F')
	   	   val = 10+val-'A';
		else val = val-'0';
	   	if (flag)
	   	   *hsh2++ |= val;
		else *hsh2 = val<<4;
		flag = ~flag;
		}
        hsh1 = hsh1 + 2;
        val  = hsh1-hsh;
        hsh1 = hsh1 + slen;

	}
   else return;
   if (!sym->csect_aux.x_parmhash) {
        sym->csect_aux.x_parmhash = val;
        sym->csect_aux.x_snhash = XN_TYPCHK;
        }
}

/****************************************************************/
/* Function:	collect_for_ESD					*/
/* Purpose:	increment esd count, and increment the string	*/
/*		length size that will be needed for the ESDs,	*/
/*		increment number of auxiliary entries in ESD,	*/
/*		increment hash size in bytes                 	*/
/* Input:	sp - the pointer to the symbol table entry which*/
/*			be going into the ESD			*/
/* Output:	increments f_nsyms, length_str, and hash.s_size */
/* Returns:	esd index for this symbol			*/
/****************************************************************/
int collect_for_ESD(sp)
struct symtab *sp;
{
   int c;

   if (sp->rename){
        if ((c=strlen(sp->rename))>=SYMNMLEN+1)
  	length_str += c+1;
      }
   else if ((c=strlen(sp->name))>=SYMNMLEN+1)
        length_str += c+1;

       /*two bytes preceed every hash string*/
   if (sp->chktyp) typchk.s_size += (strlen(sp->chktyp)+1)/2+2;
   if (sp->dbug) 
        if ((c=strlen(sp->dbug))>=SYMNMLEN+1)
			/* 2 bytes proceed debug string in .debug table */
		debug.s_size += c+3;
   if (sp->numaux) hdr.f_nsyms += sp->numaux;
   return (++hdr.f_nsyms);
}



/****************************************************************/
/* Function:	line_collect					*/
/* Purpose:	will loop throught line numbers and fill the    */
/*		line number table				*/
/* Input:       linep -  malloced line storage area             */
/* Comments:	there is one line number entry for every        */
/*		"breakpointable" source line in a section.	*/
/*		line numbers are grouped on a per function basis*/
/*		the first entry in a function grouping will have*/
/*		l_lnno =0 and in place of physical address will */
/*		be the symbol table index of the function name  */
/****************************************************************/
line_collect(linep)
char *linep;                            /*malloced line storage area*/
{
	LINE *lm;			/* assembler line structure pointer */
 	LINENO xline;   	        /* xcoff line structure             */
 	char * lineptr; 	        /* temp malloc line storage area ptr*/

	/* initilize the char table pointer */
	if (!(lineptr = linep))
              return;
			   /* loop through line number anchor nextline */
	for ( lm=linenum; (lm); lm = lm->nextline ) {

                if (lm->l_lnno)       /*assign physical address*/
		xline.l_addr.l_paddr = lm->l_addr.l_paddr;
                else                  /*assign symbol index    */
                xline.l_addr.l_symndx = lm->l_addr.l_symndx;

		xline.l_lnno = lm->l_lnno;
                encrypt(lineptr,&xline,LINESZ);
	        lineptr+=LINESZ;    /*increment malloced line storage area*/
	}
}

#ifdef CHAR_SET
/****************************************************************/
/* Function:	setebcdic					*/
/* Purpose:	set the character set flag to ebcdic		*/
/* Comments:	the header flag is set, and the characters in   */
/*		the module type are converted			*/
/****************************************************************/
setebcdic()
{
/*
	ebcdic++;
	SET_EBCDIC(hdr);
  	atoen(hdr.a_moduletype,
	hdr.a_moduletype,2);
*/
}
#endif


/****************************************************************/
/* Function:	get_sc						*/
/* Purpose:	given a character string with the storage class */
/* 		return the actual value of the storag class	*/
/* Input:	sc - a pointer to the storage class string	*/
/* Returns:	the storage class number			*/
/* 		-1 for an invalid storage class			*/
/* Comments:	This routine is called by yylex because that is	*/
/*		the routine which puts symbols in the symbol	*/
/*		table, and that is where the storage class is	*/
/*		parsed						*/
/****************************************************************/
int get_sc(s)
char *s;
{
int i;

	/* loop through the class_name table looking for s	*/
	for (i=0; i<C_NUM; i++) 
	   if (strcmp(class_name[i],s)==0) 
	       return((unsigned char) i);
	       
	return(-1); 
}



/******************************************************************/
/* Function: sort_csects                                          */
/* Purpose:  sorts csects according to rw and ro 		  */
/* Input:    a pointer to a csect table entry                     */
/* Comments: this routine collects all the csects in the rw and all*/
/*           in the read only and assigns addresses to csects	  */
/*           section sizes in the hdr (a_sectlen) are also set	  */
/******************************************************************/
sort_csects()
{
SECT *csrw,*csro,*cstc,*csds,*cscm;
SECT *csrwe,*csroe,*cstce,*csdse,*cscme;
SECT *csp;

   /* here the csects are sorted into 5 lists			*/
   /*      ro - read only  section 				*/
   /*      rw - read write section 				*/
   /*      tc - toc entries	   				*/
   /*	   cm - common storage	   				*/
   /*	   ds - dsects		   				*/
   csro = csrw = cstc = cscm = csds = 0;
   csroe = csrwe = cstce = cscme = csdse = 0;


   /* do some funny business with first csect, if unused unnamed */
   /* PR csect this is done to take care of the case where	*/
   /* no csect is used, then we imply an unnamed PR csect by 	*/
   /* always creating an unnamed csect as the first one, then 	*/
   /* we figure out whether it is used or not, if not then just	*/ 
   /* delete by bumping the csect pointer			*/
   if (sect->dot==0&&(!sect->rent)) {
      if (sect->secname==symtab) symtab = symtab->nxtsym;
      sect = sect->nxt;
      XN_TEXT--;
      }
   else
      sect->rent=0;
   /* define the section numbers depending on what is actually  */
   /* defined                                                   */
   if (XN_TEXT)
	XN_TEXT   = SECTION_NUM++;
   if (XN_DATA)
	XN_DATA   = SECTION_NUM++;
   if (XN_BSS)
	XN_BSS    = SECTION_NUM++;
   if (XN_TYPCHK)
	XN_TYPCHK = SECTION_NUM++;
   if (XN_DEBUG)
	XN_DEBUG  = SECTION_NUM++;

   ccsectx =                    /* Record csect for stabs       */
   ccsect = sect;		/* reset ccsect for pass 2 	*/

   /* go through every csect and add it to the appropriate list	*/
   for (csp=sect; (csp); csp=csp->nxt) {
      if (csp->end>csp->dot) csp->dot = csp->end;
      if (csp->stype == XTY_SD) {
	    switch(csp->ssclass) {   
	        case XMC_DB:
	        case XMC_PR:
	        case XMC_RO:
	        case XMC_GL:
	        case XMC_XO:
	        case XMC_SV:
	        case XMC_TI:
	        case XMC_TB:
		     addcs(csp,&csro,&csroe);
		     csp->secname->sectnumber = XN_TEXT;
		     break;
	        case XMC_TC0: 
	        case XMC_TC:
	        case XMC_TD:
		     addcs(csp,&cstc,&cstce);
		     csp->secname->sectnumber = XN_DATA;
		     break;
	      case XMC_RW:
	      case XMC_DS:
	      case XMC_UA:
		     addcs(csp,&csrw,&csrwe);
		     csp->secname->sectnumber = XN_DATA;
		     break;
	      case C_DSECT:  /* DSECTS		*/
		     addcs(csp,&csds,&csdse);
		     break;
	      default:
                        /* reset lineno to avoid user confusion*/
                        lineno=0;
                                                /* message 051 */
			yyerror( 51, csp->ssclass );
	      }
	}
	else if (csp->stype==XTY_CM) { 
            switch(csp->ssclass) {
              case XMC_BS:
              case XMC_UC:
                     addcs(csp,&cscm,&cscme);
                     csp->secname->sectnumber = XN_BSS;
                     csp->ssclass = C_DSECT;
                     break;
              case XMC_TD:  /* For data-in-toc common block   */
		     addcs(csp,&cstc,&cstce);
		     csp->secname->sectnumber = XN_DATA;
                     break;
              default:
                     addcs(csp,&cscm,&cscme);
                     csp->secname->sectnumber = XN_BSS;
            }
	}
     }  

   /* now that the csects are ordered, rebuild the csect list	*/	
   /* according to XCOFF section (i.e. ro first, rw next...)	*/
   sect = secte = 0;		/* reset the beginning of the 	*/
     
   addlist(csro,csroe,&maxalgn[0]);
   text.s_size = cspsz;

   addlist(csrw,csrwe,&maxalgn[1]);

   if (cstc) { 
      addlist(cstc,cstce,&maxalgn[1]);
      }
   /* now set the size of the read write section	*/
   data.s_size = cspsz - text.s_size;

   addlist(cscm,cscme,&maxalgn[1]);
   bss.s_size = cspsz - (text.s_size+data.s_size);

   /* do dsects seperately, because they are different		*/
   if (csdse) csdse->nxt = 0;	
   for (csp=csds; (csp); csp = csp->nxt) {
      csp->start=0;
      addcs(csp,&sect,&secte);
      csp->end = csp->dot;
      csp->dot = 0;
      }
}


/****************************************************************/
/* Function:	addcs						*/
/* Purpose:	add a csect to a csect list			*/
/* Input:	csp - the csect to add				*/
/*		cs  - the beginning point of the csect list	*/
/*		cse - the end pointer of the csect list		*/
/* Comments:	this routine is only used by sort_csect 	*/
/*		csects are sorted by adding them to different	*/
/*		sort lists and them combining the different 	*/
/*		lists at the end				*/
/****************************************************************/
addcs(csp,cs,cse)
SECT *csp,**cs,**cse;
{
   
   /*  put csects in the corect list 				*/
   if (*cs)  {
      (*cse)->nxt = csp;
      *cse = csp;
      }
   else *cse = *cs = csp;
}

/****************************************************************/
/* Function:	addlist						*/
/* Purpose:	add a csect list back to the main csect list	*/
/* Input:	p  - the list to add				*/
/*		pe - the end of the list to add			*/
/*		ma - maxalignment				*/
/* Output:	p is added to the end of the section table 	*/
/*		and maxalign is updated if any alignment is >	*/
/*		than previous alignment				*/
/* 		This routine also sets updates the following:	*/
/*		cspsz  - cummulative size of all csects		*/
/*		csp->start - the beginning offset of a csect	*/
/*		csp->end   - the end or size of a csect		*/
/*		csp->dot   - resets the csect loc back to 0 for	*/
/*				pass 2				*/
/****************************************************************/
addlist(p,pe,ma)
SECT *p,*pe;
int *ma;
{
SECT *csp;
int rndmask;

   if (pe) 			/* end the list so the loop	*/ 
      pe->nxt = 0;		/* will terminate		*/
   for (csp=p; (csp); csp = csp->nxt) {
      rndmask = 0x7fffffff>>(31-csp->algnmnt);
      cspsz = round(cspsz,rndmask);
      *ma = MAX(csp->algnmnt,*ma);
      csp->start = cspsz;
      addcs(csp,&sect,&secte);
      /* increment the count for this section			 */
      cspsz =  cspsz+csp->dot;
      csp->end = csp->dot;
      csp->dot = 0;
      }
   cspsz = round(cspsz,FW);	/* round off end of section	*/

#ifdef IDEBUG
   if (indbg("addlist")) dmp_csect_tab();
#endif

}

/****************************************************************/
/* Function:	tocanchor					*/
/* Purpose:	create the TC csect which will be the toc anchor*/
/* Output:	tocname is set					*/
/* Comments:	This routine was created to set TOC          	*/
/*		                                    		*/
/****************************************************************/
tocanchor()
{
   	tocname = lookup("TOC",XMC_TC0);/* set TOC               	*/
        jcsect(XMC_TC0,tocname,2);	/*                         	*/
}

#ifdef IDEBUG

/****************************************************************/
/* Function:	dmp_hdr						*/
/* Purpose:	dump a XCOFF header				*/
/* Comments:	Used for debugging				*/
/****************************************************************/
dmp_hdr()
{
	fprintf(stderr,"*****************************************\n");
	fprintf(stderr,"             HEADER\n");
	fprintf(stderr,"*****************************************\n");
	fprintf(stderr, 
	" f_magic: 0%4.4o\n f_nscns:   %d\n f_timdat:   %d\n",
	hdr.f_magic,hdr.f_nscns,hdr.f_timdat);
	fprintf(stderr, 
	" f_symptr: %4.4d\n f_nsyms:   %d\n f_opthdr:   %d\n f_flags:  %x\n",
	hdr.f_symptr,hdr.f_nsyms,hdr.f_opthdr,hdr.f_flags);

        fprintf(stderr,"\n");	
        fprintf(stderr,"*****************************************\n");
	fprintf(stderr,"           TEXT SECTIONHDR\n");
	fprintf(stderr,"*****************************************\n");
	fprintf(stderr,
	" s_name: %s\n s_paddr: %d\n s_vaddr: %d\n",
	text.s_name,text.s_paddr,text.s_vaddr);
	fprintf(stderr,
	" s_size: %d\n s_scnptr: %d\n s_relptr: %d\n",
	text.s_size,text.s_scnptr,text.s_relptr);
	fprintf(stderr,
	" s_lnnoptr: %d\n s_nreloc: %d\n s_nlnno: %d\n s_flags:  %x",
	text.s_lnnoptr,text.s_nreloc,text.s_nlnno,text.s_flags);

        fprintf(stderr,"\n");	
        fprintf(stderr,"*****************************************\n");
	fprintf(stderr,"           DATA SECTIONHDR\n");
	fprintf(stderr,"*****************************************\n");
	fprintf(stderr,
	" s_name: %s\n s_paddr: %d\n s_vaddr: %d\n",
	data.s_name,data.s_paddr,data.s_vaddr);
	fprintf(stderr,
	" s_size: %d\n s_scnptr: %d\n s_relptr: %d\n",
	data.s_size,data.s_scnptr,data.s_relptr);
	fprintf(stderr,
	" s_lnnoptr: %d\n s_nreloc: %d\n s_nlnno: %d\n s_flags:  %x",
	data.s_lnnoptr,data.s_nreloc,data.s_nlnno,data.s_flags);

        fprintf(stderr,"\n");	
        fprintf(stderr,"*****************************************\n");
	fprintf(stderr,"           BSS SECTIONHDR\n");
	fprintf(stderr,"*****************************************\n");
	fprintf(stderr,
	" s_name: %s\n s_paddr: %d\n s_vaddr: %d\n",
	bss.s_name,bss.s_paddr,bss.s_vaddr);
	fprintf(stderr,
	" s_size: %d\n s_scnptr: %d\n s_relptr: %d\n",
	bss.s_size,bss.s_scnptr,bss.s_relptr);
	fprintf(stderr,
	" s_lnnoptr: %d\n s_nreloc: %d\n s_nlnno: %d\n s_flags:  %x",
	bss.s_lnnoptr,bss.s_nreloc,bss.s_nlnno,bss.s_flags);
        fprintf(stderr,"\n");

        fprintf(stderr,"\n");	
        fprintf(stderr,"*****************************************\n");
	fprintf(stderr,"           TEXT_AUX SECTIONHDR\n");
	fprintf(stderr,"*****************************************\n");
	fprintf(stderr,
	" s_name: %s\n s_paddr: %d\n s_vaddr: %d\n",
	text_aux.s_name,text_aux.s_paddr,text_aux.s_vaddr);
	fprintf(stderr,
	" s_size: %d\n s_scnptr: %d\n s_relptr: %d\n",
	text_aux.s_size,text_aux.s_scnptr,text_aux.s_relptr);
	fprintf(stderr,
	" s_lnnoptr: %d\n s_nreloc: %d\n s_nlnno: %d\n s_flags:  %x",
	text_aux.s_lnnoptr,text_aux.s_nreloc,text_aux.s_nlnno,text_aux.s_flags);
        fprintf(stderr,"\n");

        fprintf(stderr,"\n");	
        fprintf(stderr,"*****************************************\n");
	fprintf(stderr,"           DATA_AUX SECTIONHDR\n");
	fprintf(stderr,"*****************************************\n");
	fprintf(stderr,
	" s_name: %s\n s_paddr: %d\n s_vaddr: %d\n",
	data_aux.s_name,data_aux.s_paddr,data_aux.s_vaddr);
	fprintf(stderr,
	" s_size: %d\n s_scnptr: %d\n s_relptr: %d\n",
	data_aux.s_size,data_aux.s_scnptr,data_aux.s_relptr);
	fprintf(stderr,
	" s_lnnoptr: %d\n s_nreloc: %d\n s_nlnno: %d\n s_flags:  %x",
	data_aux.s_lnnoptr,data_aux.s_nreloc,data_aux.s_nlnno,data_aux.s_flags);
        fprintf(stderr,"\n");
}

/****************************************************************/
/* Function:	dmpline						*/
/* Purpose:	dump the line table, for debugging		*/
/* Input:	n - which linecount to use (should really always*/
/*		    be 1)					*/
/*		lineptr - pointer to linenumber table		*/
/* Output:	prints linenumber table entries			*/
/****************************************************************/
dmpline(n,lineptr)
int n;
LINE *lineptr;
{ 
LINE *tline;
int i;

   if (lineptr)
      { fprintf(stderr,
        "symbolindex/addr      linenumber\n");
        for (tline=lineptr,i=text_nlnno+data_nlnno; (i); i--,tline++)
        	fprintf(stderr,"%d %d\n",
        	  tline->l_addr.l_symndx, tline->l_lnno);
      }
}
 
#endif
