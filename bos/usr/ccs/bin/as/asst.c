static char sccsid[] = "@(#)82	1.17.1.8  src/bos/usr/ccs/bin/as/asst.c, cmdas, bos411, 9428A410j 3/22/94 09:14:58";
/*
 * COMPONENT_NAME: (CMDAS) Assembler and Macroprocessor 
 *
 * FUNCTIONS: addrld, dmp_csect_tab, dmp_reloc, dmp_sect,
 *	      dmp_strpool, dmpst, dmpsym, dmpsymn, find_opcode,
 *	      lookup, new_csect, put_string, sort_sym, symalloc,
 *            cvt_asmmode
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
#include <string.h>
#include "as.h"
#include "as0.h"

/* symbol table definitions				*/
struct symtab *symtab=0, *lastsym=0;
struct symtab *esdanc=0;  	/* anchor for ESD chain	*/

/* section table definitions				*/
SECT *sect = 0; 		/* beginning of table	*/
SECT *lastsect=0;		/* end of table		*/
SECT *ccsect= 0;		/* current csect	*/
 
int maxalgn[2] = {0,0};		/* max algn per section	*/
				/* (log 2 of alignment)	*/
char	*malloc();		/* alloc memory routine */
				/* init hash table to 0	*/
struct symtab *shash[NSHASH] = { 0 };
 
/* declarations of print names of symbol table fields	*/
char *type_name[] = {"e_unk", "e_abs", "e_rel", "e_ext", 
		     "e_trel", "e_tocof","e_rext"};
char *stype_name[] = {"t_er","t_sd","t_ld","t_cm","t_em","t_us"};
char *rtype_name[] = {"R_POS", "R_NEG", "R_REL", "R_TOC", 
			"R_ROMP", "R_GL", "R_TCL", "R_TBT", 
			"R_BR","ILL", "R_RBR", 
			"ILL","ILL","ILL","ILL", "R_REF"};
char *storage_cl[] = {"C_NULL", "C_AUTO", "C_EXT", "C_STAT", "C_REG",
			"C_EXTDEF", "C_LABEL", "C_ULABEL", "C_MOS",  
			 "C_ARG", "C_STRTAG", "C_MOU", "C_UNTAG",
			  "C_TPDEF", "C_USTATIC", "C_ENTAG", "C_MOE",
			   "C_REGPARM", "C_FIELD"};
/* other possible values in the 100 range*/
/* "C_BLOCK", "C_FCN", "C_EOS", "C_FILE"*/
extern char *class_name[];

/* Head and tail of .bi/.ei include symbol chain. */
static struct symtab *include_head, *include_tail;

extern FILHDR hdr;		/* the output file header	*/
/****************************************************************/
/* Function: symalloc						*/
/* Purpose:  allocate symbols for the symbol table              */
/* Input:   len - number of bytes for memory allocation         */
/* Returns:  a pointer to a new symbol table entry		*/
/* Comments: lastsym points to the last symbol allocated 	*/
/****************************************************************/
struct symtab *symalloc(len)
int len;
{
	struct symtab *tmp;
 
	if (!(tmp = (struct symtab *)malloc(len))) {
                                                /* modified message 073 */
           yyerror( 73 );
	   delexit();
           }
	/* fill allocated entry with zeroes			*/
	bzero(tmp,len);
	if (!symtab) 
	   symtab = tmp;
	else lastsym->nxtsym = tmp;
	lastsym = tmp;
	return lastsym;
}
 
/****************************************************************/
/* Function:	lookup						*/
/* Purpose:	look up a symbol in the symbol table		*/
/*		if the symbol is not found, an entry is created	*/
/* 		in the symbol table for that symbol, and a 	*/
/* 		pointer to the new entry is returned		*/
/* Input:	symname - the symbol name to look up		*/
/*		class	- the class of the symbol to look up	*/
/* Returns:	a pointer to the symbol in the symbol table	*/
/* Comments:	Note that this is only called in pass 1, since 	*/
/*		in pass 2 direct pointers are read from the temp*/
/*		file	If an entry is added, the type is set to*/
/*		E_UNK and the etype to XTY_US			*/
/* 		This program supports multiple symbols with  the*/
/* 		same name, and different storage classes being	*/
/*		different entries in the symbol table		*/
/****************************************************************/
 struct symtab *
lookup(symname,class)
 register char *symname;
 unsigned char class;
{
	register int hash = 0;
	register struct symtab *hp, *sp;
	register char *p1;
	int cflag;
 
		/* if DC specified, don't care about class 	*/
	cflag = (class==C_DC);  	/* currently not used	*/

	/* hash the symbol 					*/
	for(p1 = symname; *p1; hash += hash + *p1++)
		;
	/* if there are any hash entries in this bucket		*/
	if(hp = sp = shash[hash = (hash & 077777) % NSHASH]) 
           do {				/* loop through bucket	*/
              hp = hp->next;		/* looking for symbol	*/
	      /* if the strings compare, then make sure the	*/
	      /* class compares, if so return pointer		*/
              if(!strcmp(hp->name, symname)) 
		 if (cflag || (hp->eclass == class))
		     return hp;	
	      } while(hp != sp);
	/* drop through the loop when the symbol not found	*/
	/* create a new symbol table entry, and initialze to	*/
	/* unset and unknown values				*/
	hp = symalloc(sizeof(struct symtab));
        hp->name = put_string(symname);
	hp->type = E_UNK;		/* no type yet		*/
	hp->etype = XTY_US;  		/* set type to unset 	*/
	hp->eclass = class; 		/* set class 		*/
	hp->rename = 0;			/* no renames yet	*/
	hp->nxtesd = 0;			/* esd pointer is null	*/
	hp->chktyp=0;			/* set typchk ptr to null*/
	hp->numaux=0;                   /* no aux entries       */
        hp->r_info_arr[0].r_rrtype = RLD_NOT_SET;
        hp->r_info_arr[1].r_rrtype = RLD_NOT_SET;
        hp->reflist_index = -1;
        hp->symlist_idx = -1;
        hp->rcount = 0;

	/* add entry to hash tree				*/
	if (sp) {
	     hp->next = sp->next;	sp->next = hp;
             } 
	else shash[hash] = hp->next = hp;
	return hp;			/* return ptr to new entry*/
}
 
#ifdef IDEBUG
/****************************************************************/
/* Function: 	dmpst						*/
/* Purpose:  	dump the entire symbol table			*/
/* Comments:	Used for debugging				*/
/****************************************************************/
dmpst()
{
struct symtab *p;

 fprintf(stderr, "\n     name     value     sectnum  type   etype ");
 fprintf(stderr, "eclass esdind numaux sclass\n");
   for (p=symtab; (p); p= p->nxtsym) {
      dmpsym(p);
      }
}

/****************************************************************/
/* Function:	dmpsym 						*/
/* Purpose:	dump one symbol from symbol table               */
/* Input: 	p - pointer to symbol table entry               */
/* Comments:	Used for debugging				*/
/****************************************************************/
dmpsym(p) 
struct symtab *p;
{
int clnm;				/* class number		*/
int stclnum;				/* storage class number sclass */

   clnm = (p->eclass>C_NUM+1)? C_NUM+2:p->eclass;
 if ((p->sclass > 18 || p->sclass < 0) || (p->type >= 0 || p->type <= 7))
     fprintf(stderr, 
      "%10.10s  0x%8.8x %7.7x   %4s   %4s  %4s %5.3d  %5.3d %7.3d\n",
     p->name, p->value, p->sectnumber, type_name[p->type],
   stype_name[p->etype],class_name[clnm],p->esdind, p->numaux, p->sclass);

 else
     fprintf(stderr, 
      "%10.10s  0x%8.8x %7.7x   %4x   %4s  %4s %5.3d  %5.3d %9.9s\n",
     p->name, p->value, p->sectnumber, p->type,
   stype_name[p->etype],class_name[clnm],p->esdind, p->numaux,
    storage_cl[p->sclass]);

 fprintf(stderr, ".typchk      .debug\n");
 fprintf(stderr, "%10.10s  %10.10s\n", p->chktyp, p->dbug);
}

/****************************************************************/
/* Function:	dmpsymn						*/
/* Purpose:	dump a symbol table entry by giving the name	*/
/* Input: 	symname - the symbol name to dump		*/
/* Comments:	Used for debugging				*/
/****************************************************************/
dmpsymn(symname)
char *symname;
{
   register int hash = 0;
   register struct symtab *hp, *sp;
   register char *p1;
   /* returns pointer to name*/
 
   /* pulled this out of lookup above				*/
   /* hash the name						*/
   for(p1 = symname; *p1; hash += hash + *p1++)
	;
   /* look through hash bucket					*/
   if(hp = sp = shash[hash = (hash & 077777) % NSHASH])
   do {
      hp = hp->next;
      if(!strcmp(hp->name, symname)) {
         dmpsym(hp);
         break;
         }
      } while(hp != sp);
}
 
#endif

/****************************************************************/
/* Function:	new_csect					*/
/* Purpose:	initialize a new csect				*/
/* Input:	sclass - storage class of new type 		*/
/*        	type   - type of csect				*/
/*         	sname  - pointer to symbol name in symbol table	*/
/* Returns: 	ptr to new entry in csect table 		*/
/*       	0 - failure					*/
/* Comments:                                    		*/
/****************************************************************/
SECT *new_csect(sclass,type,sname,algn)
unsigned char sclass;
unsigned char type;
struct symtab *sname;
unsigned char algn;
{
	SECT *tmp;

	/* allocate a new entry					*/
	if (!(tmp = (SECT *)malloc(SECTSZ))){
                                                /* message 073 */
           yyerror( 73 );
	   delexit();
           }
	if(!sect) 			/* if the first then 	*/
	   sect = tmp;			/* put on front		*/
	else lastsect->nxt = tmp;	/* else put on end	*/
	lastsect = tmp;

	/* initialize the field values				*/
	lastsect->ssclass = sclass;	/* passed class in	*/
	lastsect->stype = type;		/* passed type in	*/
	lastsect->dot = 0;		/* set csect loc to 0	*/
	lastsect->end = 0;		/* no size yet		*/
	lastsect->start = 0;		/* no starting offset	*/
	lastsect->secname = sname;	/* point to csect name	*/
	lastsect->rent = 0;		/* no rld entries	*/
	lastsect->reflist_idx = -1;     /* init value for reflist */
     if (algn<0 || algn > 31)
                                                /* message 062 */
	yyerror( 62 );
	lastsect->algnmnt = algn;	/* log 2 byte alignment	*/
	lastsect->nxt = 0;		/* next = NULL		*/

        if (sname->sclass!=C_EXT)
        /*if not set to global external symbol                  */
        /*initialize class to indicate local external symbol    */
        sname->sclass = C_HIDEXT;       /*csect local to compile*/
	/*initialize auxillary entry of csect symtab*/
        sname->numaux= 1;               /* one auxiliary entry  */
        sname->csect_aux.x_scnlen= 0; /* no size yet     */
        sname->csect_aux.x_parmhash= 0; /*no typchk yet  */
        sname->csect_aux.x_snhash= 0;   /*no typchk yet  */
	sname->csect_aux.x_smtyp=
              (algn<<3) | (type & 07);        /*csect type     */
        sname->csect_aux.x_smclas= 
               sclass;                         /*csect class    */
        sname->csect_aux.x_stab= 0;     /*no debug yet   */
        sname->csect_aux.x_snstab= 0;   /*no debug yet   */


	return lastsect;		/* return created entry	*/
}

#ifdef IDEBUG
/****************************************************************/
/* Function:	dmp_csect_tab 					*/
/* Purpose:	dump the csect table				*/
/* Comments:	Used for debugging				*/
/****************************************************************/
dmp_csect_tab() 
{
SECT *p;

   fprintf(stderr, "name             dot      start       end       ");
   fprintf(stderr, "esdtype esdclass algn\n");
   for (p=sect; (p); p= p->nxt) {
      dmp_sect(p);
      }
}

#endif

char *tcpr[] = {"  ","TC"};

#ifdef IDEBUG
/****************************************************************/
/* Function:	dmp_sect					*/
/* Purpose:	dump an entry in the csect table		*/
/* Input:	p - a pointer to a csect table entry		*/
/* Comments:	Used for debugging				*/
/****************************************************************/
dmp_sect(p)
SECT *p;
{
int clnm;
XRLD *r;

   /* determine invalid ssclasses  ?????????????????????????????*/
   clnm = ((p->ssclass)>C_NUM+1) ? C_NUM+2 : p->ssclass;
   
   fprintf(stderr, 
	 "%10.10s 0x%8.8x   0x%8.8x   0x%8.8x   %4s   %2s%2s 0x%2.2x",
	 p->secname->name, p->dot, p->start, p->end, 
	 stype_name[p->stype], tcpr[p->ssclass&0x40],class_name[clnm],
	 p->algnmnt);
    fprintf(stderr,"\n");

    if (passno==2)
    for (r=p->rent; (r); r=r->rnxt)	
    	dmp_reloc(r);
}

/****************************************************************/
/* Function:	dmp_reloc					*/
/* Purpose:	dump the relocation entries for a csect		*/
/* Input:	r  - a pointer to a csect's relocation entries	*/
/* Output:	dumps the relocation table to standard error	*/
/* Comments:	Used for debugging				*/
/****************************************************************/
dmp_reloc(r)
XRLD *r;
{
   if (r) {
	fprintf(stderr,
	 	"\t\t%6.6s   0x%2.2x   0x%8.8x", 
		rtype_name[r->rtype], r->rlen, r->raddr);
	if (r->rname)
	   fprintf(stderr," %s\n",r->rname->name);
	else fprintf(stderr," No Name\n");
   }
}

#endif

int text_nreloc,
    data_nreloc;
/****************************************************************/
/* Function:	addrld						*/
/* Purpose:	get a new relocation entry                      */
/* Input:  	rrtype - relocation type of entry   		*/
/*          	rrlen - length of relocation entry              */
/*         	rrname  - pointer to symbol name in symbol table*/
/* Comments: 	Need to sort these according to raddr		*/
/****************************************************************/
int addrld(rrtype,rrlen,rrname)
unsigned char rrtype;   	/* rld type			*/
unsigned char rrlen;    	/* length of RLD		*/
struct symtab *rrname;  	/* esd relative to		*/
{
XRLD *tmp,*p;
extern SCNHDR text, data;        /* external section headers    */ 
					/* no rlds for dsects	*/
	if (ccsect->ssclass==C_DSECT) return;
					/* allocate a new entry	*/
	if (!(tmp = (XRLD *)malloc(XRLDSZ))){
                                                /* message 073 */
           yyerror( 73 );
	   delexit();
           }

	/* fill in passed in values				*/
        tmp->rtype = rrtype;
#ifdef SIGNED_RLD
	tmp->rlen = rrlen;
#else
	if(rrtype==R_RBA||rrtype==R_RRTBI||rrtype==R_RBR)
           tmp->rlen=(RLDSIGN<<7)|((rrlen-1) & 0x1f);  /* length */
	else
           tmp->rlen=((rrlen-1) & 0x1f);               /* length */
#endif
	/* if the symbol is a label (LD) or not set then use 	*/
	/* name of the containing csect				*/
	if (rrname->etype==XTY_LD||rrname->etype==XTY_US)
	   tmp->rname = rrname->csect->secname;
	else tmp->rname = rrname;
	/* relocation address is current location		*/
        if ( rrtype == R_REF )
          tmp->raddr = ccsect->start;
        else
	  tmp->raddr = ccsect->dot+ccsect->start;
	tmp->rnxt = 0;


   	switch (ccsect->ssclass) {
		case XMC_PR:
		case XMC_RO:
		case XMC_GL:
		case XMC_XO:
		case XMC_SV:
		case XMC_TI:
		case XMC_TB:
	             text_nreloc++;
		     break;
		case XMC_TC0:	
		case XMC_TC:
		case XMC_TD:
		case XMC_RW:	
		case XMC_UA:	
		case XMC_DS:	
                     data_nreloc++;
		     break;
		default:
                                                /* message 079 */
			yyerror( 79 );
		}
  	/* rlds must be in ascending order when output, but 	*/
  	/* maintaining an ascending-order linked list would	*/
  	/* require full-list traversals on virtually every rld, */
  	/* leading to an n-squared algorithm.		        */
  	/* So keep the rlds in descending order, and reverse the*/
  	/* order in one pass when they're output.		*/
#ifdef IDEBUG
	if (indbg("addrld")) {
	   fprintf(stderr,"current csect\n");
           dmp_sect(ccsect);
	   fprintf(stderr,"XRLD to add\n");
	   dmp_reloc(tmp);
	   }
#endif
	/* if no entries for this csect, put at beginning	*/
	if (!ccsect->rent)
	   ccsect->rent = tmp;
	/* else if this entry can be inserted at beginning	*/
         else if ((tmp->raddr > ccsect->rent->raddr) ||
                  (tmp->raddr==ccsect->rent->raddr&&ccsect->rent->rnxt)) {
	      tmp->rnxt = ccsect->rent;
	      ccsect->rent = tmp;
	      }
	/* else go ahead and search through table		*/
	 else {
	    for (p = ccsect->rent; 
		 (p->rnxt)&&(p->rnxt->raddr > tmp->raddr); 
		 p = p->rnxt);

	    tmp->rnxt = p->rnxt;
	    p->rnxt = tmp;
	    }
}

/****************************************************************/
/* Function:	sort_sym					*/
/* Purpose:	go through symbol table and assign symbols 	*/
/*	    	their value					*/
/*		also chain the ESD symbols together		*/
/*        	one additional function is to count ESDs and 	*/
/*          	assign ESD indexes				*/
/* Comments:	first go through all symbols and assign values	*/
/*          	ER symbols are all chained together at beginning*/
/*          	LD entries go with containing SD entries and are*/
/*          	initially chained to the CSECT table to be 	*/
/*		collected at the end of the sort symbol loop.	*/
/*		CM entries go at the very end			*/
/****************************************************************/
struct symtab *
sort_sym()
{
struct symtab *fesd,*lesd,*p,*sp;
SECT *csp,*contcs;
int in_function = 0;

   /* set first and last esd chain pointers		*/
   fesd = lesd = 0;

   /* loop through symbol table, assigning values and adding	*/
   /* to chains							*/
   for (sp=symtab; (sp); sp=sp->nxtsym) {

      /* convert unknown symbols into externals. */
      if (sp->type == E_UNK && sp->etype == XTY_US && flag_extern_OK) {
      sp->type = E_EXT;
      sp->etype = XTY_ER;
      sp->numaux = 1;
      sp->sclass = C_EXT;
      }
      
      switch(sp->etype) {
	 case XTY_SD:
	    sp->value = sp->csect->start;
	    break;
	 case XTY_LD:
	    if (sp->type == E_UNK) 	/* undefined globl symbol*/
	       break;
	    /* .bf, .bb, .eb, .ef are sorted like other label def. */
needs_sorting:
	    contcs = sp->csect;		/* get containing csect ptr*/
	    /* chain LD entry onto CSECT in address order	*/
	    if (contcs) {
	    for (p = contcs->secname;
	         (p->nxtesd)&&
		   ((p->nxtesd->value<=sp->value) ||
		    (p->nxtesd->etype == XTY_DBG &&
                        p->nxtesd->sclass != C_FCN && 
			p->nxtesd->sclass != C_BLOCK));
		 p = p->nxtesd)
	    ;
            if (p) {
            	sp->nxtesd = p->nxtesd;
	    	p->nxtesd = sp;
		/* mark the last entered XTY_LD, just like stabs */
		contcs->lastsym = sp;
		}
	    }
	    break;
	 case XTY_DBG:
	    /* DBG information                                  */
	    /* chain DBG entry onto CSECT in arrival order	*/

            /* place .bi/.ei pairs to at beginning of esd chain. */
  
           /* save .bi/.ei symbols in an include chain. */
  
           if (sp->sclass == C_BINCL || sp->sclass == C_EINCL) {
             if (include_head) {
               include_tail->nxtesd = sp;
               include_tail = sp;
             }
             else
               include_head = include_tail = sp;
             break;
           }

	    if (sp->sclass == C_FCN)
	      if (strcmp (".bf", sp->name) == 0)
		in_function = 1;
	      else if (strcmp (".ef", sp->name) == 0) {
		in_function = 0;
	      }

	    /* .bb/.eb,.bf/.ef symbols need sorting just like   */ 
	    /*	any other symbol.  They have an address value,  */
 	    /*	 and their  location in the symbol table        */
	    /*	will tell us things about the scoping rule.     */
  	    if ( sp->sclass == C_BLOCK || sp->sclass == C_FCN || 
  						sp->sclass == C_FUN)
		goto needs_sorting;

            if ((contcs = sp->csect)->nxt!=sect) {

	   /* if we know the last one, add to the end                   */
	   if ( in_function && contcs->lastsym ) {
	     sp->nxtesd = contcs->lastsym->nxtesd;
	     contcs->lastsym->nxtesd = sp;
	     contcs->lastsym = sp;
	   }
	   else
	   {
	    for (p = contcs->secname;
	         (p->nxtesd); 
	         p = p->nxtesd);
            if (p) {
            	sp->nxtesd = p->nxtesd;
	    	p->nxtesd = sp;
		contcs->lastsym = sp;
	    }
	    break;
	   }
	   }
	   break;
	 case XTY_ER:
	    /* ERs go at begginning so add them to actual ESD 	*/
	    /* chain 						*/
	    if (fesd)  
               lesd = lesd->nxtesd = sp;
	    else fesd =  lesd = sp;
	    lesd->nxtesd = 0;
	    break;
	 case XTY_CM:
          sp->value = sp->csect->start; break;
         case XTY_US:
	    if (sp->type==E_TREL)
		sp->value = sp->value+sp->csect->start-tocname->csect->start;
            break;
		
	 default:
	    break;
	 }
   }

   /* if there are include symbols (.bi/.ei), stick the include
      chain right after .file symbol. */
  
   if (include_head) {
      if (!fesd) {
        fesd = include_head;
        lesd = include_tail;
      }
      else if (fesd->sclass == C_FILE) {
        include_tail->nxtesd = fesd->nxtesd;
        fesd->nxtesd = include_head;
        if (lesd==fesd) lesd = include_tail;
      }
      else {
        include_tail->nxtesd = fesd;
        fesd = include_head;
      }
    }

   /* assigning esd numbers to external symbols (as well as
       .bi/.ei include symbols) */
    
     for (sp = fesd; sp; sp = sp->nxtesd) {
          sp->esdind = hdr.f_nsyms;
          collect_for_ESD (sp);
      }

   /* at this point all the symbols have values, all the	*/
   /* ER entries are already on the ESD chain, and all the LD	*/
   /* entries are chained to their associated CSECT table entry */
   /* in address order.   Now go through the csect table and add */
   /* each csect's entries to the main ESD chain		*/
   for (csp=sect; (csp); csp=csp->nxt) {
	/* don't put dsects on the ESD table		*/
	if (csp->ssclass==C_DSECT &&
		(csp->secname->csect_aux.x_smclas!=XMC_BS &&
		 csp->secname->csect_aux.x_smclas!=XMC_UC )
		) break;
	/* put this csects ESD chain on the end of the main chain */
	if (fesd) 
		lesd = lesd->nxtesd = csp->secname;
	else fesd = lesd = csp->secname;

	/* move lesd down this added piece's chain and assign	*/
	/* esd indexes so that at the end lesd will point to the */
	/* end of the list					*/
        for (;;) {
		lesd->esdind = hdr.f_nsyms;
                collect_for_ESD(lesd);
		if (lesd->nxtesd) 
		   lesd = lesd->nxtesd;
		else break;
	 	}
     }  
   return fesd;
}

extern int ninstab;
/****************************************************************/
/* Function:	find_opcode  					*/
/* Purpose: 	find an opcode in the opcode table              */
/* Input:   	s - opcode to look for				*/
/*              b_predict  -  if it is a branch prediction      */
/*                            0  -  no                          */
/*                            1  -  yes                         */
/* Returns: 	pointer to an instruction table entry		*/ 
/*		NULL - if not found				*/
/* Comments:	currently the table is ordered and a binary	*/
/*		search is used, if performance becomes a problem*/
/*		it can be changed to a hashing scheme later	*/
/****************************************************************/
struct instab *
find_opcode(char *s, int *b_predict )
{
int top,bot,mid,r,mn_len,i;
int  r1 = 99;
int r1_mid = 0;
char  br[MAX_MNLEN], *brs, *str;

   if ( *b_predict == 1 ) { /* create a string without '-' or '+' */
      str = s;
      brs = br;
      while ( (*brs = *str ) != '+' &&  ( *brs != '-' )){
         brs++;
         str++;
      }
      *brs = '\0';
      brs = br;
   }
   top = ninstab - 2;
   bot = 0;
   do {
      mid = (top+bot)/2;
#ifdef IDEBUG
      if (indbg("find_opcode"))
	 printf("find_opcode: top %d bot %d mid:%d name[mid]:%s s:%s\n",
		 top,bot,mid,instab[mid].name,s);			
#endif
      if ( *b_predict == 1 )
        r=strncmp(brs,instab[mid].name,MAX_MNLEN);
      else 
        r=strncmp(s,instab[mid].name,MAX_MNLEN);

      if (r > 0 )
         bot = mid+1;
      else if (r==0) {
         if (*b_predict == 1) {
           if ( !strncmp(s,instab[mid+1].name,MAX_MNLEN) )
                   mid = mid+1;
           else if ( !strncmp(s,instab[mid+2].name,MAX_MNLEN)  )
                   mid = mid + 2;
           else 
                 *b_predict = 88 ;
         }
         if (mn_xref ) { /* if mnemonics xref. flag is on,   */
                          /* create the mn_buf.              */
            if ( instab[mid].name1[0] ) {
               if ( (instab[mid].type_id == POWER && 
                         asm_mode_type == PowerPC ) ||
                        ( instab[mid].type_id == PowerPC &&
                            asm_mode_type == POWER ) ) {
                   mn_len = strlen(instab[mid].name1);
                   strncpy(mn_buf, instab[mid].name1, mn_len);
                   if ( mn_len < MAX_MNLEN )
                      strncpy(mn_buf+mn_len, "          ",MAX_MNLEN-mn_len);
                   mn_buf[MAX_MNLEN] = '\0';
               }
            }
         } 
         if ( (asm_mode & instab[mid].inst_bmap) == 0 ) {
            if ( asm_mode != ANY )
            {
               instrs_warn(149, instab[mid].name, asm_mode_str);
            }
            if ( asm_mode != ANY && !modeopt )   /* default assembly mode */
                sum_bit_map &= instab[mid].inst_bmap;
         }
         else
            sum_bit_map &= instab[mid].inst_bmap;

        return &instab[mid];
      }
      else 
         top = mid-1;
   } while (bot<=top);
   return NULL;
}

/* string pool structure declarations				*/
/* each string pool is 1024 long with a next pointer to point	*/
/* to another 1k string pool					*/
#define SPSIZE   1024+sizeof(struct str_pool *)
#define PSIZE	SPSIZE-sizeof(struct str_pool *)
int delexit();
static struct str_pool {
     struct str_pool *next;
     char pool[1];
     } *start = 0, *current = 0; 
static char *nxt = 0, *end= 0;

/***************************************************************/
/* Function: put_string                                        */
/* Purpose:  put a string in the string pool                   */
/* Input:    s   - the string to put in the string pool	       */
/* Returns:  pointer to the string in the string pool          */ 
/***************************************************************/
char  *put_string(s)
char *s;
{
struct str_pool *p;
char *s1;
int req_len;

#ifdef IDEBUG
    if (indbg("put_string"))
	{
       fprintf(stderr,"put_string: %s\n",s);
       fprintf(stderr,"length of string: %d\n",strlen(s));
	}
#endif
    /* make sure the entire string fits into this string pool	*/
    if (nxt+(req_len = strlen(s)+1)>end) {
       /* allocate a new string pool				*/
       if (p = (struct str_pool *) 
		calloc(MAX(SPSIZE,req_len+sizeof(struct str_pool *)),1)) {
          p->next = 0;
          nxt = p->pool;
          end =nxt+(MAX(PSIZE,req_len));
          }
       else {
                                                /* modified message 073 */
	  yyerror( 73 );
	  delexit();
	  }
       /* chain this string pool onto string pool list		*/
       if (!start) 
          start = current = p;
       else {
	  current = current->next = p;
	  }
       }
    /* add the string 						*/
    s1 = strcpy(nxt,s);
    nxt += req_len;		/* point nxt pased added str	*/
#ifdef IDEBUG
	  if (i_debug&16)
             dmp_strpool();	
#endif
    return s1;			/* return ptr to place in string*/
}				/* tab where string added 	*/
 
#ifdef IDEBUG
/****************************************************************/
/* Function:	dmp_strpool					*/
/* Purpose: 	dump the string pool				*/
/* Comments: 	Used for debugging				*/
/****************************************************************/
int dmp_strpool()
{
char *p,*e;
struct str_pool *s;
int cnt;

	printf("STRING POOL\n");
   for(s = start; s; s=s->next) {
      if (s->next) 
	 e = (char *)s+MAX(SPSIZE,strlen(s->pool));
      else 
	 e = (char *)s+SPSIZE; 

      while(!(*e--));   /* look for last terminator */
      e++;
         
      cnt = 0;
      for(p=s->pool; p<e; p += strlen(p)+1) {
	 printf("%s ",p); 
	 if (cnt>24) {
	    printf("\n");
	    cnt = 0;
	    }
	 else cnt += strlen(p)+1;
	 }
      printf("\n");
   } 
}
#endif
/****************************************************************/
/* Function:	cvt_asmmode					*/
/* Purpose: 	convert the input string of the assembly mode   */
/*              into the interger value                         */
/* Input:    in_mode - input string of the assembly mode        */
/*           caller - '1' caller is process_arg                 */
/*                    '2' caller is jmachine                    */
/* Return:   integer to indicate the  pre-defined assembly mode */
/****************************************************************/ 
unsigned short cvt_asmmode(char *in_mode, char caller)
{
   int i;

    for (i=0; i<max_mode_num; i++)
    {
       if (!strcmp(asmmode_tab[i].mode_str, in_mode)) {
          asm_mode_type = asmmode_tab[i].as_md_type;
          return(asmmode_tab[i].mode_value);
       }
    }
    if (!strcmp("PWRX", in_mode)) {
       asm_mode_type = POWER;
       return(asmmode_tab[PWRX_IDX_ASMTAB].mode_value);
    }
    if (caller == '1')    /* caller is process_arg  */
       yyerror(162, in_mode);
    else
       yyerror( 147, in_mode);
    for (i=0; i<max_mode_num-1; i++)
    {   
        fputs(asmmode_tab[i].mode_str, stderr);
        fputs(" ",stderr);
        error5(asmmode_tab[i].mode_str, NULL);
        error5(" ", NULL);
     }
     fputs(asmmode_tab[max_mode_num-1].mode_str, stderr);
     fputs("\n", stderr);
     error5(asmmode_tab[max_mode_num-1].mode_str, NULL);
     error5("\n", NULL);
     return(0);
}
