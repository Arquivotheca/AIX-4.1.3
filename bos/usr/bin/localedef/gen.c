static char sccsid[] = "@(#)93	1.11.1.9  src/bos/usr/bin/localedef/gen.c, cmdnls, bos41B, 9504A 1/11/95 16:43:19";
/*
 * COMPONENT_NAME: (CMDNLS) Locale Database Commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27, 85
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.2
 */

#define _ILS_MACROS

#include <sys/localedef.h>
#include <sys/lc_sys.h>
#include <stdio.h>
#include <ctype.h>
#include "semstack.h"
#include "symtab.h"
#include "method.h"
#include "locdef.h"
#include "err.h"

#define MAX(a,b)	(a<b ? b : a)	/* return max of a,b */

static void gen_extern_meth(FILE *);
static void gen_ctype(FILE *, _LC_ctype_t *);
static void gen_charmap(FILE *, _LC_charmap_t *);
static void gen_collate(FILE *, _LC_collate_t *);
static void gen_monetary(FILE *, _LC_monetary_t *);
static void gen_time(FILE *, _LC_time_t *);
static void gen_numeric(FILE *, _LC_numeric_t *);
static void gen_msg(FILE *, _LC_resp_t *);

extern int copying_ctype;       /* set in copy.c */

#ifdef _PTR_METH

static void gen_locale(FILE *, _LC_charmap_t *, _LC_collate_t *,  
	   _LC_ctype_t *, _LC_monetary_t *, _LC_numeric_t *, _LC_resp_t *, 
	   _LC_time_t *, _LC_aix31_t *);
static void gen_instantiate(FILE *);

#else

static void gen_locale(FILE *, _LC_charmap_t *, _LC_collate_t *,  
	   _LC_ctype_t *, _LC_monetary_t *, _LC_numeric_t *, _LC_resp_t *, 
	   _LC_time_t *, _LC_aix31_t *, char *, char *, char *, char *, char *,
	   char *, char *, char *); 
static void gen_instantiate(FILE *, char * ,char *);

/*
** These are used to create the private method tables - only used when
** the index tables are the means for determining correct methods.
*/

static void gen_extern(FILE *,char *, char *, char *, char *, char *, char *,
		       char *, char *);
static void gen_charmap_tbl(FILE *, char *);
static void gen_collate_tbl(FILE *, char *);
static void gen_ctype_tbl(FILE *, char *);
static void gen_locale_tbl(FILE *, char *);
static void gen_monetary_tbl(FILE *, char *);
static void gen_numeric_tbl(FILE *, char *);
static void gen_resp_tbl(FILE *, char *);
static void gen_time_tbl(FILE *, char *);
static char *gen_tmp_variable(char *);

/*
** This is used when determining indexes which are different for global and 
** private method tables.
*/
extern int private_table;

/*
** This is a convenient way to distribute a pointer to the name of the
** locale object.  loc_ptr is used for private methods in the index method.
** If private_table is defined global_loc_ptr points to temp_loc_ptr used
** in gen(), otherwise, global_loc_ptr defaults to "lc_loc".
**
** Appropriate initialization is done in gen().  All routines which use
** global_loc_ptr are either directly or indirectly called from gen() so this
** is OK.
**
*/

char   *   global_loc_ptr = "lc_loc";

#endif


#define TERMINATOR      NULL

/* uppers used for determining default toupper table */
static  char *uppers[]={        /* upper case character symbols as      */
        "<A>",                  /* defined in XPG4 System Interface     */
        "<B>",                  /* sections 5.3.1 and 4.1, table 4-1    */
        "<C>",
        "<D>",
        "<E>",
        "<F>",
        "<G>",
        "<H>",
        "<I>",
        "<J>",
        "<K>",
        "<L>",
        "<M>",
        "<N>",
        "<O>",
        "<P>",
        "<Q>",
        "<R>",
        "<S>",
        "<T>",
        "<U>",
        "<V>",
        "<W>",
        "<X>",
        "<Y>",
        "<Z>",
        TERMINATOR};    /* terminating value which will never occur */

/* lowers used for determining default toupper table */
static  char *lowers[]={        /* lower case character symbols as      */
        "<a>",                  /* defined in XPG4 System Interface     */
        "<b>",                  /* sections 5.3.1 and 4.1, table 4-1    */
        "<c>",
        "<d>",
        "<e>",
        "<f>",
        "<g>",
        "<h>",
        "<i>",
        "<j>",
        "<k>",
        "<l>",
        "<m>",
        "<n>",
        "<o>",
        "<p>",
        "<q>",
        "<r>",
        "<s>",
        "<t>",
        "<u>",
        "<v>",
        "<w>",
        "<x>",
        "<y>",
        "<z>",
        TERMINATOR};    /* terminating value which will never occur */

/*
*  FUNCTION:  get_encoding(char * symb_name)
*
*  given the symbolic name, return the character's encoding
*  used to determine default toupper table
*
*/

static get_encoding(char * symb_name)
{
        extern symtab_t cm_symtab;
        extern _LC_ctype_t ctype;
        symbol_t *s;

        s = loc_symbol(&cm_symtab,symb_name,0);
        if (s == NULL) {
            error(ERR_NO_SYMBOLIC_NAME,symb_name);
            }

        return (s->data.chr->wc_enc);
}


/*
*  FUNCTION: fp_putstr
*
*  DESCRIPTION:
*  Standard print out routine for character strings in structure 
*  initialization data.
*/
static void fp_putstr(FILE *fp, char *s)
{
    extern int  copying;        /* might be using internal, non-escaped, data */
    extern char yytext[];       /* use this cause its big */

    if (s == NULL)
        fprintf(fp, "\t\"\",\n");
    else {
        if (copying) {		/* string might be coming from "core" */
            char *bptr = yytext;
            unsigned char c;

            while ((c=*s++)) {

		/* input string may be built from scratch in gram.y rather */
		/* copied from core.  In this case we must be sure that    */
		/* compiler ready strings like \\ \" \x## and \X## are     */
		/* not "fixed".                                            */
		
		if (c == '\\' && (*s == '\\' || *s == '"')) {
			/* input is in form \\ or \"  */
			/* substring is compiler ready so copy it verbatim */
			*bptr++ = c;
			*bptr++ = *s++;
			}
		else
			if (c == '\\' && (*s == 'X' || *s == 'x') 
				&& (s[1] && s[2])
				&& (isxdigit(s[1]) && isxdigit(s[2]))) {
					/* input is in form \x## or \X## */
					/* copy string over verbatim */
					*bptr++ = c;
					*bptr++ = *s++;
					*bptr++ = *s++;
					*bptr++ = *s++;
                                        if ((*s=='"') && (*s=='"')) {
                                                *bptr++ = *s++;
                                                *bptr++ = *s++;
                                                }
					}
		else	
			if (c != '\\' && c != '"' && isascii(c) && isprint(c))
				*bptr++ = c;
			else
				bptr += sprintf(bptr, "\\x%02x\"\"", c);
            }
            *bptr = '\0';
            fprintf(fp, "\t\"%s\",\n", yytext);
        } else
            fprintf(fp, "\t\"%s\",\n", s);
    }
}

/*
*  FUNCTION: fp_putsym
*
*  DESCRIPTION:
*  Standard print out routine for symbols in structure initialization
*  data.
*/
static void fp_putsym(FILE *fp, char *s)
{
    if (s != NULL)
	fprintf(fp, "\t%s,\n", s);
    else
	fprintf(fp, "\t0,\n");
}


/*
*  FUNCTION: fp_putdig
*
*  DESCRIPTION:
*  Standard print out routine for integer valued structure initialization
*  data.
*/
static void fp_putdig(FILE *fp, int i)
{
    fprintf(fp, "\t%d,\n", i);
}

/*
*  FUNCTION: fp_puthdr
*
*  DESCRIPTION:
*  Standard print out routine for method headers.
*/
static void fp_puthdr(FILE *fp, int i)
{
     fprintf(fp, "\t%d, _LC_MAGIC, _LC_VERSION, 0,\n", i);
}


/*
*  FUNCTION: fp_putmeth
*
*  DESCRIPTION:
*  Standard print out routine for method references. Either the
*  method name is print'd out (for _PTR_METH) or the index (either
*  global or private) is printed out.
*/
static void fp_putmeth(FILE *fp, int i, int prvt)
{

#ifdef _PTR_METH

    /* for pointer methods output the name twice */

    fprintf(fp, "\t%s,\t/* %s */\n", METH_NAME(i), METH_NAME(i));

#else

    /* for index methods, the index and the name needs to be printed */
    /* If the global table is used, then the method offset in the    */
    /* standard methods table is used. If the private table is used  */
    /* the index passed as prvt is used.			     */

    if (private_table)
	fprintf(fp, "\t%2d,\t/* %s */\n", prvt , METH_NAME(i));
    else
        fprintf(fp, "\t%2d,\t/* %s */\n", METH_OFFS(i), METH_NAME(i));

#endif

}


/*
*  FUNCTION: gen_hdr
*
*  DESCRIPTION:
*  Generate the header file includes necessary to compile the generated
*  C code.
*/

static void gen_hdr(FILE *fp)
{
    fprintf(fp, "#include <sys/localedef.h>\n");

}


/*
*
*   FUNCTION: gen_forward_dcl
*
*   DESCRIPTION:
*   Generate a forward data declaration of the lc_loc (or temp_loc_ptr if
*   private method:  See comments for global_loc_ptr) structure.  This
*   must be the same as for the definition of the locale object in 
*   gen_locale().
*
*/

static void gen_forward_dcl(FILE *fp)
{
#ifdef _PTR_METH

  fprintf(fp, "static _LC_locale_t lc_loc;\n");

#else

  if (private_table){
      fprintf(fp, "_LC_locale_t %s;\n",global_loc_ptr);
  }
  else {
      fprintf(fp, "static _LC_locale_t %s;\n",global_loc_ptr);
      /* global_loc_ptr is "lc_loc" in this case */
  }

#endif
}


/* 
*  FUNCTION: gen
*
*  DESCRIPTION:
*  Common entry point to code generation.  This function calls each of the
*  functions in turn which generate the locale C code from the in-memory
*  tables built parsing the input files.
*/
void gen(FILE *fp_data,FILE *fp_meth)
{
    extern _LC_charmap_t  charmap;
    extern _LC_collate_t  *collate_ptr;
    extern _LC_ctype_t    *ctype_ptr;
    extern _LC_monetary_t *monetary_ptr;
    extern _LC_numeric_t  *numeric_ptr;
    extern _LC_time_t     *lc_time_ptr;
    extern _LC_resp_t     *resp_ptr;
    extern _LC_aix31_t    lc_aix31;


#ifndef _PTR_METH
    /*
    ** These variables are used to hold names that will not pollute the
    ** namespace when extern'd from the locale.
    */
    extern char *locname;
    char *temp_loc_ptr;
    char *temp_charmap_tbl_ptr;
    char *temp_collate_tbl_ptr;
    char *temp_ctype_tbl_ptr;
    char *temp_locale_tbl_ptr;
    char *temp_monetary_tbl_ptr;
    char *temp_numeric_tbl_ptr;
    char *temp_resp_tbl_ptr;
    char *temp_time_tbl_ptr;

    temp_loc_ptr = '\0';
    if (private_table) {
	/*
	** The gen_tmp_variable routine will create a namespace clean
 	** string for these variables. This routine will malloc the
	** space needed.
	*/ 
        temp_loc_ptr          = gen_tmp_variable(locname);
	temp_charmap_tbl_ptr  = gen_tmp_variable("charmap_tbl");
	temp_collate_tbl_ptr  = gen_tmp_variable("collate_tbl");
	temp_ctype_tbl_ptr    = gen_tmp_variable("ctype_tbl");
	temp_locale_tbl_ptr   = gen_tmp_variable("locale_tbl");
	temp_monetary_tbl_ptr = gen_tmp_variable("monetary_tbl");
	temp_numeric_tbl_ptr  = gen_tmp_variable("numeric_tbl");
	temp_resp_tbl_ptr     = gen_tmp_variable("resp_tbl");
	temp_time_tbl_ptr     = gen_tmp_variable("time_tbl");
        global_loc_ptr 	      = temp_loc_ptr;
    }
#endif


    gen_hdr(fp_data);
    gen_forward_dcl(fp_data);


#ifdef _PTR_METH
    gen_extern_meth(fp_data);
#else
    if (private_table) {
	/* 
	** If there is to be a private method table for this locale, then
	** a method file must be generated.
	*/
	if (fp_meth == NULL) 
	    INTERNAL_ERROR;
	gen_hdr(fp_meth);
        gen_extern_meth(fp_meth);
	gen_charmap_tbl(fp_meth,temp_charmap_tbl_ptr);
	gen_collate_tbl(fp_meth,temp_collate_tbl_ptr);
	gen_ctype_tbl(fp_meth,temp_ctype_tbl_ptr);
	gen_locale_tbl(fp_meth,temp_locale_tbl_ptr);
	gen_monetary_tbl(fp_meth,temp_monetary_tbl_ptr);
	gen_numeric_tbl(fp_meth,temp_numeric_tbl_ptr);
	gen_resp_tbl(fp_meth,temp_resp_tbl_ptr);
	gen_time_tbl(fp_meth,temp_time_tbl_ptr);
    }
    gen_extern(fp_data,temp_charmap_tbl_ptr, temp_collate_tbl_ptr, 
	       temp_ctype_tbl_ptr, temp_locale_tbl_ptr, temp_monetary_tbl_ptr, 
	       temp_numeric_tbl_ptr, temp_resp_tbl_ptr, temp_time_tbl_ptr);
#endif


    gen_charmap(fp_data, &charmap);
    gen_ctype(fp_data, ctype_ptr);
    gen_collate(fp_data, collate_ptr);
    gen_monetary(fp_data, monetary_ptr);
    gen_numeric(fp_data, numeric_ptr);
    gen_time(fp_data, lc_time_ptr);
    gen_msg(fp_data, resp_ptr);

#ifdef _PTR_METH
    gen_locale(fp_data, &charmap, collate_ptr, ctype_ptr, 
	       monetary_ptr, numeric_ptr, resp_ptr, lc_time_ptr, &lc_aix31);
    gen_instantiate(fp_data);
#else
    gen_locale(fp_data, &charmap, collate_ptr, ctype_ptr, monetary_ptr, 
	       numeric_ptr, resp_ptr, lc_time_ptr, &lc_aix31, temp_loc_ptr,
	       temp_charmap_tbl_ptr, temp_collate_tbl_ptr, temp_ctype_tbl_ptr,
	       temp_monetary_tbl_ptr, temp_numeric_tbl_ptr, temp_resp_tbl_ptr, 
	       temp_time_tbl_ptr);
    if (private_table) {
        gen_instantiate(fp_meth,temp_loc_ptr,temp_locale_tbl_ptr);
    }
    else {
       gen_instantiate(fp_data,global_loc_ptr,"(void *(**)())__METHOD_TBL");
       /* setlocale() replaces __METHOD_TBL(-1) with pointer to __method_tbl */
    }
#endif

    
}

/* 
*  FUNCTION: gen_tmp_variable
*
*  DESCRIPTION:
*  This function generates a unique variable for extern-ing from the
*  loaded files. These are used so that the namespace is not polluted.
*  The pid is used as a seed to random to create a random number to tack onto 
*  the end of the string. The string __ is prepended to the name. All
*  "-", "/", and "." are removed because they are not valid in variable
*  names.
*
*/

#ifndef _PTR_METH

char *gen_tmp_variable(char *name)
{
    char *temp_ptr;
    char *slash_ptr;
    char *dot_ptr;
    char *dash_ptr;
    int x;
    pid_t pid;


    pid = getpid();
    srand(pid);
    x = rand();
    temp_ptr = (char *)malloc(strlen(name) + 30);
    if (temp_ptr == NULL) {
	error(ERR_MEM_ALLOC_FAIL);
    }
    sprintf(temp_ptr,"__%s%d",name,x);
    while ((slash_ptr = strchr(temp_ptr,'/')) != NULL) {
	*slash_ptr = '_';
    }
    while ((dot_ptr = strchr(temp_ptr,'.')) != NULL) {
	*dot_ptr = '_';
    }
    while ((dash_ptr = strchr(temp_ptr,'-')) != NULL) {
	*dash_ptr = '_';
    }
    return(temp_ptr);
}

#endif


/* 
*  FUNCTION: gen_extern_meth
*
*  DESCRIPTION:
*  This function generates the externs which are necessary to reference the
*  function names inside the locale objects.  
*/
static void gen_extern_meth(FILE *fp)
{
  int i;
  char *s;

  for (i=0; i<=LAST_METHOD; i++) {
      s = METH_NAME(i);
      if ((s != NULL) && (strcmp(s,"0")))
          fprintf(fp, "extern %s();\n", s);
  }
}

/* 
*  FUNCTION: gen_extern
*
*  DESCRIPTION:
*  This function generates the externs which are necessary to reference the
*  function names inside the locale objects.  
*/

#ifndef _PTR_METH

static void gen_extern(FILE *fp, char * char_tbl_ptr, char * coll_tbl_ptr, 
		       char * ctype_tbl_ptr, char * loc_tbl_ptr, 
		       char * mon_tbl_ptr, char * num_tbl_ptr, 
		       char * resp_tbl_ptr, char * time_tbl_ptr)
{
  int i;
  char *s;

  if (private_table) {
      fprintf(fp,"extern void * (*%s)();\n",char_tbl_ptr);
      fprintf(fp,"extern void * (*%s)();\n",coll_tbl_ptr);
      fprintf(fp,"extern void * (*%s)();\n",ctype_tbl_ptr);
      fprintf(fp,"extern void * (*%s)();\n",loc_tbl_ptr);
      fprintf(fp,"extern void * (*%s)();\n",mon_tbl_ptr);
      fprintf(fp,"extern void * (*%s)();\n",num_tbl_ptr);
      fprintf(fp,"extern void * (*%s)();\n",resp_tbl_ptr);
      fprintf(fp,"extern void * (*%s)();\n",time_tbl_ptr);
  }
}

#endif

/* 
*  FUNCTION: gen_charmap
*
*  DESCRIPTION:
*  This function generates the C code which implements a _LC_charmap_t
*  data structure corresponding to the in memory charmap build parsing the
*  charmap sourcefile.
*/
static void gen_charmap(FILE *fp, _LC_charmap_t *lc_cmap)
{
  extern wchar_t max_wchar_enc;
  extern int     max_disp_width;
  int i, j;

  fprintf(fp, 
"/*------------------------- CSID TABLE  -------------------------*/\n");
  fprintf(fp, "static unsigned char cm_csids[] = {\n");
  for (i=0; i<=max_wchar_enc; i+=16) {
    for (j=0; j < 16 && i+j <= max_wchar_enc; j++)
      fprintf(fp, "%2d, ", 
	      (lc_cmap->cm_cstab != NULL?lc_cmap->cm_cstab[i+j]:0));
    fprintf(fp, "\n");
  }
  fprintf(fp, "};\n\n");
      
  fprintf(fp, 
"/*------------------------- CHARMAP OBJECT  -------------------------*/\n");
  fprintf(fp, "static _LC_charmap_t lc_cmap = {\n");

  /* class core */
  fp_puthdr(fp, _LC_CHARMAP);
  fp_putmeth(fp, CHARMAP_NL_LANGINFO,  PRVT_CHARMAP_NL_LANGINFO);
  fp_putmeth(fp, CHARMAP_MBTOWC,       PRVT_CHARMAP_MBTOWC);
  fp_putmeth(fp, CHARMAP_MBSTOWCS,     PRVT_CHARMAP_MBSTOWCS);
  fp_putmeth(fp, CHARMAP_WCTOMB,       PRVT_CHARMAP_WCTOMB);
  fp_putmeth(fp, CHARMAP_WCSTOMBS,     PRVT_CHARMAP_WCSTOMBS);
  fp_putmeth(fp, CHARMAP_MBLEN,        PRVT_CHARMAP_MBLEN);
  fp_putmeth(fp, CHARMAP_WCSWIDTH,     PRVT_CHARMAP_WCSWIDTH);
  fp_putmeth(fp, CHARMAP_WCWIDTH,      PRVT_CHARMAP_WCWIDTH);
  fp_putmeth(fp, CHARMAP___MBTOPC,     PRVT_CHARMAP___MBTOPC);
  fp_putmeth(fp, CHARMAP___MBSTOPCS,   PRVT_CHARMAP___MBSTOPCS);
  fp_putmeth(fp, CHARMAP___PCTOMB,     PRVT_CHARMAP___PCTOMB);
  fp_putmeth(fp, CHARMAP___PCSTOMBS,   PRVT_CHARMAP___PCSTOMBS);
  fp_putmeth(fp, CHARMAP_CSID,         PRVT_CHARMAP_CSID);
  fp_putmeth(fp, CHARMAP_WCSID,        PRVT_CHARMAP_WCSID);
  fp_putmeth(fp, CHARMAP_CHARMAP_INIT, PRVT_CHARMAP_CHARMAP_INIT);
  fp_putsym(fp, "0");

  /* class extension */
  fp_putstr(fp, lc_cmap->cm_csname);
  fp_putdig(fp, mb_cur_max);
  fp_putdig(fp, 1);
  fp_putdig(fp, max_disp_width);
  fp_putsym(fp, "cm_csids");

#ifdef _PTR_METH
  fp_putsym(fp, "&lc_loc, 0, 0");
#else
  fprintf(fp,"\t&%s, 0, 0,\n",global_loc_ptr);
#endif
		 	  /* values for loc_rec, __meth_ptr, __data_ptr */
  
  fprintf(fp, "};\n\n");
}


/* 
*  FUNCTION: compress_mask
*
*  DESCRIPTION:
*  Take all masks for codepoints above 255 and assign each unique mask
*  into a secondary array.  Create an unsigned byte array of indices into
*  the mask array for each of the codepoints above 255.
*/
static int compress_masks(_LC_ctype_t *ctype)
{
  extern wchar_t max_wchar_enc;
  static unsigned char qidx[256];
  static int nxt_idx = 1;
  int    umasks;
  
  int i, j;

  if (ctype->mask == NULL)
      return 0;

  /* allocate memory for masks and indices */
  ctype->qidx = (unsigned char *)calloc(max_wchar_enc - 256 +1, sizeof(unsigned char));
  ctype->qmask = MALLOC(unsigned int, 256);
  
  umasks = 1;
  for (i=256; i<= max_wchar_enc; i++) {      /* for each codepoint > 255 */
    /* 
      Search for a mask in the 'qmask' array which matches the mask for
      the character corresponding to 'i'.
    */
    for (j=0; j < umasks; j++) 
      if (ctype->qmask[j] == ctype->mask[i]) {
	/* mask already exists, place index in qidx for character */
	ctype->qidx[i-256] = j;	
	break;
      }

    if (j==umasks) {

      /* couldn't find mask which would work, so add new mask to 'qmask' */

      ctype->qidx[i-256] = nxt_idx;
      ctype->qmask[nxt_idx] = ctype->mask[i];

      nxt_idx++;
      umasks++;
    }

    /* only support 256 unique masks for multi-byte characters */
    if (nxt_idx >= 256) INTERNAL_ERROR;
  }

  return nxt_idx;
}


/* 
*  FUNCTION: gen_ctype
*
*  DESCRIPTION:
*  Generate C code which implements the _LC_ctype_t locale
*  data structures.  These data structures include _LC_classnms_t,
*  an array of wchars for the upper and lower case translations,
*  and the container class _LC_ctype_t itself.
*/
#define N_PER_LINE 8
static void gen_ctype(FILE *fp, _LC_ctype_t *lc_ctype)
{
  extern wchar_t max_wchar_enc;
  int i, j;
  int n_idx;
  wchar_t *temp;

  fprintf(fp, 
"/*------------------------- CHARACTER CLASSES -------------------------*/\n");

  fprintf(fp, "static _LC_classnm_t classes[] = {\n");
  for (i=0; i<lc_ctype->nclasses;i++) {
    fprintf(fp, "\"%s\",\t0x%04x,\n",
	    lc_ctype->classnms[i].name, 
	    lc_ctype->classnms[i].mask);
  }
  fprintf(fp, "};\n\n");

  
  fprintf(fp, 
"/*------------------------- UPPER XLATE -------------------------*/\n");
  fprintf(fp,"static wchar_t upper[] = {\n");
  if (lc_ctype->upper == NULL) {
      lc_ctype->upper = (wchar_t *)calloc(sizeof(int), max_wchar_enc+1);
      for (i=0; i <=max_wchar_enc; i++)
	  lc_ctype->upper[i] = i;
      for (i = 0; uppers[i] != TERMINATOR; i++)
          lc_ctype->upper[get_encoding(lowers[i])]=get_encoding(uppers[i]);
  }
  for (i=0; i<=max_wchar_enc; i+=N_PER_LINE) {
    for (j=0; j < N_PER_LINE && i+j <= max_wchar_enc; j++)
      fprintf(fp, "0x%04x, ", lc_ctype->upper[i+j]);
    fprintf(fp, "\n");
  }
  fprintf(fp, "};\n\n");

  fprintf(fp, 
"/*------------------------- LOWER XLATE -------------------------*/\n");
  fprintf(fp,"static wchar_t lower[] = {\n");
  if (lc_ctype->lower == NULL) {
      /* tolower unspecified, make it the inverse of toupper */
      lc_ctype->lower = (wchar_t *)calloc(sizeof(int), max_wchar_enc+1);
      /* assign identity */
      for (i=0; i<=max_wchar_enc; i++)
	  lc_ctype->lower[i] = i;
      
      /* assign inverse */
      for (i=0; i<=max_wchar_enc; i++) {
	  if (lc_ctype->upper[i] != i) {
	      lc_ctype->lower[lc_ctype->upper[i]] = i;
	  }
      }
  }
  for (i=0; i<=max_wchar_enc; i+=N_PER_LINE) {
      for (j=0; j < N_PER_LINE && i+j <= max_wchar_enc; j++)
	  fprintf(fp, "0x%04x, ", lc_ctype->lower[i+j]);
      fprintf(fp, "\n");
  }
  fprintf(fp, "};\n\n");

#if 0
/* the following structure is only required for 3.1 compatibility and */
/* is no longer needed in 4.x                                         */
#ifdef _AIX
  fprintf(fp, 
"/*------------------------- XLATE_31 -------------------------*/\n");
  fprintf(fp,"static wchar_t xlate_31[] = {\n");
  temp = (wchar_t *)calloc(sizeof(int), max_wchar_enc+1);
  if (lc_ctype->upper == NULL) {
      diag_error(ERR_TOUPPER_NOT_OPT);
      for (i=0; i <=max_wchar_enc+1; i++)
	  temp[i] = i;
  }
  else {
      for (i = 0; i <= max_wchar_enc+1; i++)
	  temp[i + 1 ] = lc_ctype->upper[i];
      for (i = 0; i <= max_wchar_enc+1; i++) {
	  if (temp[i+1] == i)
	      temp[i + 1] = lc_ctype->lower[i];
      }
  }
     
  /* lc_ctype->core.__data = (void *)temp; */
  /* this last line is in R/O space if using the copy directive */
  /* plus, this appears to be unused null code                  */
  for (i=0; i<=max_wchar_enc+1; i+=N_PER_LINE) {
    for (j=0; j < N_PER_LINE && i+j <= max_wchar_enc+1; j++)
      fprintf(fp, "0x%04x, ", temp[i+j]);
    fprintf(fp, "\n");
  }
  fprintf(fp, "};\n\n");
#endif
#endif

  fprintf(fp, 
"/*------------------------- CHAR CLASS MASKS -------------------------*/\n");

  /* 
    print the data for the standard linear array of class masks.
  */
  fprintf(fp,"static unsigned int masks[] = {\n");
  for (i=0; i < 256; i+=N_PER_LINE) {

    for (j=0; j < N_PER_LINE && i+j < 256; j++)
      fprintf(fp, "0x%04x, ", 
	      ((lc_ctype->mask!=NULL)?(lc_ctype->mask[i+j]):0));

    fprintf(fp, "\n");
  }
  fprintf(fp, "};\n\n");

  /* 
   If there are more than 256 codepoints in the codeset, the
    implementation attempts to compress the masks into a two level
    array of indices into masks.

    If we are copying an existing locale, and there were qidx and
    qmask pointers defined, we need to duplicate them.
  */
  n_idx =0;

  if (((max_wchar_enc > 255) && !copying_ctype) ||
       (copying_ctype && (lc_ctype->qidx) && (lc_ctype->qmask))) {
    if (!copying_ctype)
      n_idx = compress_masks(lc_ctype);
    else {
      for (i=256;i<=max_wchar_enc;i++)
        if (lc_ctype->qidx[i-256] > n_idx)
          n_idx = lc_ctype->qidx[i-256];
      n_idx++;  /* need to add one since we need the number of entries, not   */
                /* an index to the last element...  Beware the Off By One Bug */
      }
   
 
    /* Print the index array 'qidx' */
    fprintf(fp, "static unsigned char qidx[] = {\n");
    for (i=256; i <= max_wchar_enc; i+=N_PER_LINE) {
    
      for (j=0; j<N_PER_LINE && i+j <=max_wchar_enc; j++)
	fprintf(fp, "0x%02x, ", lc_ctype->qidx[i+j-256]);
    
      fprintf(fp, "\n");
    }
    fprintf(fp, "};\n\n");
  
    /* Print the mask array 'qmask' */
    fprintf(fp, "static unsigned int qmask[] = {\n");
    for (i=0; i<n_idx; i+= N_PER_LINE) {
 
      for (j=0; j < N_PER_LINE && i+j < n_idx; j++)
	fprintf(fp, "0x%04x, ", lc_ctype->qmask[i+j]);
      
      fprintf(fp, "\n");
    }
    fprintf(fp, "};\n\n");
  }
      
	     
  fprintf(fp, 
"/*-------------------------   CTYPE OBJECT   -------------------------*/\n");
  fprintf(fp,"static _LC_ctype_t lc_ctype = {\n");

  /* class core */
  fp_puthdr(fp, _LC_CTYPE);
  fp_putmeth(fp, CTYPE_TOWUPPER,   PRVT_CTYPE_TOWUPPER);
  fp_putmeth(fp, CTYPE_TOWLOWER,   PRVT_CTYPE_TOWLOWER);
  fp_putmeth(fp, CTYPE_GET_WCTYPE, PRVT_CTYPE_GET_WCTYPE);
  fp_putmeth(fp, CTYPE_IS_WCTYPE,  PRVT_CTYPE_IS_WCTYPE);
  fp_putmeth(fp, CTYPE_CTYPE_INIT, PRVT_CTYPE_CTYPE_INIT);
#if 0
/* xlate_31 is no longer needed */
  fp_putsym(fp, "xlate_31");
#else
  fp_putdig(fp,0);
#endif

  /* class extension */
  /* max and min process code (required by toupper, et al) */
  fp_putdig(fp, 0);
  fp_putdig(fp, max_wchar_enc);

  /* case translation arrays */
  fp_putsym(fp, "upper");
  fp_putsym(fp, "lower");
  fp_putsym(fp, "masks");

  if (n_idx > 0) {
    fp_putsym(fp, "qmask");
    fp_putsym(fp, "qidx");
  } else {
    fp_putsym(fp, "0");
    fp_putsym(fp, "0");
  }    
  fprintf(fp, "\t%d,\n", lc_ctype->nclasses);
  fp_putsym(fp, "classes");

#ifdef _PTR_METH
  fp_putsym(fp, "&lc_loc, 0, 0");
#else
  fprintf(fp,"\t&%s, 0, 0,\n",global_loc_ptr);
#endif
		 	  /* values for loc_rec, __meth_ptr, __data_ptr */

  fprintf(fp, "};\n\n");
}


/* 
*  FUNCTION: gen_collate
*
*  DESCRIPTION:
*  Generate C code which implements the _LC_collate_t locale 
*  data structure.
*/
static void gen_collate(FILE *fp, _LC_collate_t *coll)
{
    extern wchar_t max_wchar_enc;
    int i, j, k;

    /* 
      Generate local definitions of _LC_coltbl_t, _LC_collel_t, and
      _LC_weight_t to handle the non-default side of the _LC_weight_t union.
      This is necessary to allow auto-initialization of the data types.

      If structure definitions change in localedef.h this will also need to
      change.
    */
    
    /* lc_weight_t */
    fprintf(fp, "typedef struct {\n\tshort x[_COLL_WEIGHTS_MAX+1];\n");

    fprintf(fp, "} lc_weight_t;\n");

    /* lc_collel_t */
    fprintf(fp, "typedef struct {\n");
    fprintf(fp, "\tchar *ce_sym;\n\tlc_weight_t ce_wgt;\n");
    fprintf(fp, "} lc_collel_t;\n");

    /* lc_coltbl_t */
    fprintf(fp, "typedef struct {\n");
    fprintf(fp, "\tlc_weight_t ct_wgt;\n\tlc_collel_t *ct_collel;\n");
    fprintf(fp, "} lc_coltbl_t;\n");

    /* lc_subs_t */
    fprintf(fp, "typedef struct {\n");
    fprintf(fp, "\tchar *tgt_wgt_str[_COLL_WEIGHTS_MAX+1];\n");

    fprintf(fp, "} lc_subs_t;\n\n");
    
    /* lc_collate_t */
    fprintf(fp, "typedef struct {\n");
    fprintf(fp, "\t_LC_core_collate_t core;\n");
    fprintf(fp, "\tunsigned char co_nord;\n");
    fprintf(fp, "\tlc_weight_t co_sort;\n");
    fprintf(fp, "\twchar_t co_wc_min;\n");
    fprintf(fp, "\twchar_t co_wc_max;\n");
    fprintf(fp, "\twchar_t co_col_min;\n");
    fprintf(fp, "\twchar_t co_col_max;\n");
    fprintf(fp, "\tlc_coltbl_t *co_coltbl;\n");
    fprintf(fp, "\tunsigned  char co_nsubs;\n");
    fprintf(fp, "\tlc_subs_t *co_subs;\n");
    fprintf(fp, "\tunsigned short co_special;\n");
    fprintf(fp, "\tstruct __LC_locale *loc_rec;\n");
    fprintf(fp, "\tvoid *__meth_ptr;\n");
    fprintf(fp, "\tvoid *__data_ptr;\n");

    fprintf(fp, "} lc_collate_t;\n");

    /*
      Generate code to implement collation elements.  
    */
    fprintf(fp,
"/*------------------------- COLLELLS --------------------------------*/\n");

    /*
      Generate collation elements 
    */
    if (coll->co_coltbl != NULL) {
	for (i=0; i<=max_wchar_enc; i++) {
	    
	    /* if there are any collation elements beginning with i */
	    if (coll->co_coltbl[i].ct_collel != NULL) {
	    
		_LC_collel_t *ce;
		
		fprintf(fp, "static lc_collel_t ce%d[]={\n", i);

		/* one entry for each collation elementing beginning with i */
		for (j=0, ce=&(coll->co_coltbl[i].ct_collel[j]); 
		     ce->ce_sym != NULL; 
		     ce=&(coll->co_coltbl[i].ct_collel[++j])) {

		    fprintf(fp, "{ \"%s\", ", ce->ce_sym);
		    fprintf(fp,"{");
		    for (k=0;k<=_COLL_WEIGHTS_MAX;k++)
                        fprintf(fp," 0x%04x,",ce->ce_wgt.n[k]);
                    fprintf(fp," } },\n");
		}
		fprintf(fp,"{ 0, {");
		for (k=0;k<=_COLL_WEIGHTS_MAX;k++)
                    fprintf(fp," 0,");
                fprintf(fp," } },\n");

		fprintf(fp, "};\n");
	    }
	}
    }

    fprintf(fp,
"/*------------------------- COLLTBL ---------------------------------*/\n");
    if (coll->co_coltbl != NULL) {
	fprintf(fp, "static lc_coltbl_t colltbl[] ={\n");

	for (i=0; i<=MAX(max_wchar_enc,255); i++) {
	    _LC_coltbl_t *ct;

	    if (i > max_wchar_enc) { /* make sure at least 256 chars exported */
		fprintf(fp,"{ {");
		for (k=0;k<=_COLL_WEIGHTS_MAX;k++)
                    fprintf(fp," 0x0000,");
                fprintf(fp," }, 0 },\t/* %04x */\n",i);
		continue;
	    }

	    ct = &(coll->co_coltbl[i]);

	    /* generate weight data */
	    fprintf(fp,"{ {");
	    for (k=0;k<=_COLL_WEIGHTS_MAX;k++)
                fprintf(fp," 0x%04x,", ct->ct_wgt.n[k]);
             fprintf(fp," }, ");
		
	    /* generate collation elements if present */
	    if (ct->ct_collel != NULL)
		fprintf(fp, "ce%d },\t/* %04x */\n", i, i);
	    else
		fprintf(fp, "0 },\t/* %04x */\n",i);
	}
	fprintf(fp, "};\n");
    }
    
    if (coll->co_coltbl != NULL && coll->co_nsubs > 0) {
	fprintf(fp,
"/*-----------------SUBSTITUTION STR (1-to-many mapping)--------------*/\n");

	fprintf(fp, "static lc_subs_t substrs[] = {\n");
	for (i=0; i<coll->co_nsubs; i++) {
		char * ptr;

		/* weights are 2 byte values */
		fprintf(fp,"\t{");
		for (j=0;j<=_COLL_WEIGHTS_MAX;j++) {
			ptr=coll->co_subs[i].tgt_wgt_str[j];
			if (ptr && (!*ptr))   /* IGNORE */
			    fprintf(fp,"\"\\x00\\x00\",");
			else {
			    fprintf(fp,"\"");
			    while (*ptr != 0) {
				    fprintf(fp,"\\x%02x",*ptr++);
				    }
			    fprintf(fp,"\",");
			    }
			}

		fprintf(fp,"},\n");
		}

	fprintf(fp, "};\n\n");
    }
    
    fprintf(fp,
"/*------------------------- COLLATE OBJECT  -------------------------*/\n");
	
    fprintf(fp, "static lc_collate_t lc_coll = {\n");

    /* class core */
    fp_puthdr(fp, _LC_COLLATE);
    fp_putmeth(fp, COLLATE_STRCOLL,  PRVT_COLLATE_STRCOLL);
    fp_putmeth(fp, COLLATE_STRXFRM,  PRVT_COLLATE_STRXFRM);
    fp_putmeth(fp, COLLATE_WCSCOLL,  PRVT_COLLATE_WCSCOLL);
    fp_putmeth(fp, COLLATE_WCSXFRM,  PRVT_COLLATE_WCSXFRM);
    fp_putmeth(fp, COLLATE_FNMATCH,  PRVT_COLLATE_FNMATCH);
    fp_putmeth(fp, COLLATE_REGCOMP,  PRVT_COLLATE_REGCOMP);
    fp_putmeth(fp, COLLATE_REGERROR, PRVT_COLLATE_REGERROR);
    fp_putmeth(fp, COLLATE_REGEXEC,  PRVT_COLLATE_REGEXEC);
    fp_putmeth(fp, COLLATE_REGFREE,  PRVT_COLLATE_REGFREE);
    /* 
    ** _LC_collate_t *(*init)(); 
    */
    fp_putmeth(fp, COLLATE_COLLATE_INIT, PRVT_COLLATE_COLLATE_INIT); 
    fp_putdig(fp, 0);		          /* void *data; */
    
    /* class extension */
    fp_putdig(fp, coll->co_nord-1);

    fprintf(fp,"\t{");
    for (k=0;k<=_COLL_WEIGHTS_MAX;k++)
	fprintf(fp," 0x%04x,", coll->co_sort.n[k]);
    fprintf(fp," }, \n");

    if (coll->co_coltbl != NULL) {
	fp_putsym(fp, "0");		      /* wchar_t co_wc_min; */
	fp_putdig(fp, max_wchar_enc);	      /* wchar_t co_wc_max; */
    
	fprintf(fp, "\t0x%04x,\n",
		coll->co_col_min);	      /* wchar_t co_col_min; */
	fprintf(fp, "\t0x%04x,\n",
		coll->co_col_max);	      /* wchar_t co_col_max; */
    
	fp_putsym(fp, "colltbl");	      /* _LC_coltbl_t *co_tbl; */
	fp_putdig(fp, coll->co_nsubs);	      /* number of subs strs */
	if (coll->co_nsubs > 0)
	    fp_putsym(fp, "substrs");	      /* substitution strings */
	else 
	    fp_putsym(fp, "0");
	fp_putdig(fp, coll->co_special);      /* co_special (for fast path)*/
    } else {
	fp_putsym(fp, "0");		      /* wchar_t co_wc_min; */
	fp_putdig(fp, max_wchar_enc);	      /* wchar_t co_wc_max; */
    
	fprintf(fp, "\t0x%04x,\n", 0);	      /* wchar_t co_col_min; */
	fprintf(fp, "\t0x%04x,\n",
		max_wchar_enc);		      /* wchar_t co_col_max; */
    
	fp_putsym(fp, "0");		      /* _LC_coltbl_t *co_tbl; */
	fp_putdig(fp, 0);		      /* number of subs strs */
	fp_putsym(fp, "0");		      /* substitution strings */
	fp_putdig(fp, 0);		      /* co_special (for fast path)*/
    }

#ifdef _PTR_METH
    fp_putsym(fp, "&lc_loc, 0, 0");
#else
    fprintf(fp,"\t&%s, 0, 0,\n",global_loc_ptr);
#endif
		 	  /* values for loc_rec, __meth_ptr, __data_ptr */

    fprintf(fp, "};\n\n");
}


/* 
*  FUNCTION: gen_monetary
*
*  DESCRIPTION:
*  Generate C code which implements the _LC_monetary_t locale 
*  data structure.
*/
static void gen_monetary(FILE *fp, _LC_monetary_t *lc_mon)
{
    int i;

    fprintf(fp, 
"/*------------------------- MONETARY OBJECT  -------------------------*/\n");
    fprintf(fp, "static _LC_monetary_t lc_mon={\n");

    /* class core */
    fp_puthdr(fp, _LC_MONETARY);
    fp_putmeth(fp, MONETARY_NL_LANGINFO,   PRVT_MONETARY_NL_LANGINFO);
    fp_putmeth(fp, MONETARY_STRFMON,       PRVT_MONETARY_STRFMON);	
    fp_putmeth(fp, MONETARY_MONETARY_INIT, PRVT_MONETARY_MONETARY_INIT);	
    fp_putdig(fp, 0);				

    /* class extension */
    fp_putstr(fp, lc_mon->int_curr_symbol);  /* char *int_curr_symbol; */
    fp_putstr(fp, lc_mon->currency_symbol);  /* char *currency_symbol; */
    fp_putstr(fp, lc_mon->mon_decimal_point);/* char *mon_decimal_point; */
    fp_putstr(fp, lc_mon->mon_thousands_sep);/* char *mon_thousands_sep; */
    fp_putstr(fp, lc_mon->mon_grouping);     /* char *mon_grouping; */
    fp_putstr(fp, lc_mon->positive_sign);    /* char *positive_sign; */
    fp_putstr(fp, lc_mon->negative_sign);    /* char *negative_sign; */
    fp_putdig(fp, lc_mon->int_frac_digits);  /* signed char int_frac_digits;*/
    fp_putdig(fp, lc_mon->frac_digits);	     /* signed char frac_digits; */
    fp_putdig(fp, lc_mon->p_cs_precedes);    /* signed char p_cs_precedes; */
    fp_putdig(fp, lc_mon->p_sep_by_space);   /* signed char p_sep_by_space; */
    fp_putdig(fp, lc_mon->n_cs_precedes);    /* signed char n_cs_precedes; */
    fp_putdig(fp, lc_mon->n_sep_by_space);   /* signed char n_sep_by_space; */
    fp_putdig(fp, lc_mon->p_sign_posn);	     /* signed char p_sign_posn; */
    fp_putdig(fp, lc_mon->n_sign_posn);	     /* signed char n_sign_posn; */
    fp_putstr(fp, lc_mon->debit_sign);	     /* char *debit_sign; */
    fp_putstr(fp, lc_mon->credit_sign);	     /* char *credit_sign; */
    fp_putstr(fp, lc_mon->left_parenthesis); /* char *left_parenthesis; */
    fp_putstr(fp, lc_mon->right_parenthesis);/* char *right_parenthesis; */

#ifdef _PTR_METH
    fp_putsym(fp, "&lc_loc, 0, 0");
#else
    fprintf(fp,"\t&%s, 0, 0,\n",global_loc_ptr);
#endif
		 	  /* values for loc_rec, __meth_ptr, __data_ptr */

    fprintf(fp, "};\n");
}


/* 
*  FUNCTION: gen_time
*
*  DESCRIPTION:
*  Generate C code which implements the _LC_time_t locale data structure.
*/
static void gen_time(FILE *fp, _LC_time_t *lc_time)
{
    int i;

    fprintf(fp, 
"/*-------------------------   TIME OBJECT   -------------------------*/\n");
    fprintf(fp, "static _LC_time_t lc_time={\n");
  
    /* class core */
    fp_puthdr(fp, _LC_TIME);
    /* 
    ** char *(*nl_langinfo)(); 
    */
    fp_putmeth(fp, TIME_NL_LANGINFO, PRVT_TIME_NL_LANGINFO); 
    /* 
    ** size_t *(strftime)();   
    */
    fp_putmeth(fp, TIME_STRFTIME,    PRVT_TIME_STRFTIME);	   
    /* 
    ** char *(*strptime)();    
    */
    fp_putmeth(fp, TIME_STRPTIME,    PRVT_TIME_STRPTIME);	   
    /* 
    ** size_t (*wcsftime)();   
    */
    fp_putmeth(fp, TIME_WCSFTIME,    PRVT_TIME_WCSFTIME);	  
    /*
    ** _LC_time_t *(*init)()   
    */
    fp_putmeth(fp, TIME_TIME_INIT,   PRVT_TIME_TIME_INIT);        
    fp_putdig(fp, 0);			   /* void *data;             */
    
    /* class extension */
    fp_putstr(fp,lc_time->d_fmt);            /* char *d_fmt; */
    fp_putstr(fp, lc_time->t_fmt);	     /* char *t_fmt; */
    fp_putstr(fp, lc_time->d_t_fmt);	     /* char *d_t_fmt; */
    fp_putstr(fp, lc_time->t_fmt_ampm);	     /* char *t_fmt_ampm; */
    fprintf(fp, "\t{\n");
    for (i=0; i<7; i++) 
	fp_putstr(fp, lc_time->abday[i]);    /* char *abday[7]; */
    fprintf(fp, "\t},\n");
    fprintf(fp, "\t{\n");
    for (i=0; i<7; i++) 
	fp_putstr(fp, lc_time->day[i]);	     /* char *day[7]; */
    fprintf(fp, "\t},\n");
    fprintf(fp, "\t{\n");
    for (i=0; i<12; i++) 
	fp_putstr(fp, lc_time->abmon[i]);    /* char *abmon[12]; */
    fprintf(fp, "\t},\n");
    fprintf(fp, "\t{\n");
    for (i=0; i<12; i++) 
	fp_putstr(fp, lc_time->mon[i]);	     /* char *mon[12]; */
    fprintf(fp, "\t},\n");
    fprintf(fp, "\t{\n");
    for (i=0; i<2; i++) 
	fp_putstr(fp, lc_time->am_pm[i]);    /* char *am_pm[2]; */
    fprintf(fp, "\t},\n");
    fp_putstr(fp, lc_time->era);	     /* char *era; */
    fp_putstr(fp, lc_time->era_year);	     /* char *era_year; */
    fp_putstr(fp, lc_time->era_d_fmt);	     /* char *era_d_fmt; */
    fp_putstr(fp, lc_time->alt_digits);	     /* char *alt_digits */
    fp_putstr(fp, lc_time->era_d_t_fmt);     /* char *era_d_t_fmt */
    fp_putstr(fp, lc_time->era_t_fmt);	     /* char *era_t_fmt */

#ifdef _PTR_METH
    fp_putsym(fp, "&lc_loc, 0, 0");
#else
    fprintf(fp,"\t&%s, 0, 0,\n",global_loc_ptr);
#endif
		 	  /* values for loc_rec, __meth_ptr, __data_ptr */

    fprintf(fp, "};\n");
}


/* 
*  FUNCTION: gen_numeric
*
*  DESCRIPTION:
*  Generate C code which implements the _LC_numeric_t locale 
*  data structure.
*/
static void gen_numeric(FILE *fp, _LC_numeric_t *lc_num)
{
  fprintf(fp, 
"/*------------------------- NUMERIC OBJECT  -------------------------*/\n");
  fprintf(fp, "static _LC_numeric_t lc_num={\n");

  /* class core */
  fp_puthdr(fp, _LC_NUMERIC);
  /* 
  ** char *nl_langinfo();
  */
  fp_putmeth(fp, NUMERIC_NL_LANGINFO,  PRVT_NUMERIC_NL_LANGINFO);	   
  /* 
  ** char *__numeric_init 
  */
  fp_putmeth(fp, NUMERIC_NUMERIC_INIT, PRVT_NUMERIC_NUMERIC_INIT);	   
  fp_putdig(fp, 0);			   /* void *data; */

  /* class extension */
  if (lc_num->decimal_point[0] == '\0')	    /* char *decimal_point; */
      lc_num->decimal_point = ".";
  fp_putstr(fp, lc_num->decimal_point);
  fp_putstr(fp, lc_num->thousands_sep);	    /* char *thousands_sep; */
  fp_putstr(fp, lc_num->grouping);	    /* char *grouping;      */

#ifdef _PTR_METH
  fp_putsym(fp, "&lc_loc, 0, 0");
#else
  fprintf(fp,"\t&%s, 0, 0,\n",global_loc_ptr);
#endif
		 	  /* values for loc_rec, __meth_ptr, __data_ptr */

  fprintf(fp, "};\n\n");
}


/* 
*  FUNCTION: gen_msg
*
*  DESCRIPTION:
*  Generate C code which implements the _LC_resp_t locale data structure.
*/
static void gen_msg(FILE *fp, _LC_resp_t *lc_resp)
{
  fprintf(fp, 
"/*------------------------- MESSAGE OBJECT  -------------------------*/\n");
  fprintf(fp, "static _LC_resp_t lc_resp={\n");

  /* class core */
  fp_puthdr(fp, _LC_RESP);
  fp_putmeth(fp, RESP_NL_LANGINFO, PRVT_RESP_NL_LANGINFO);
  fp_putmeth(fp, RESP_RPMATCH,     PRVT_RESP_RPMATCH);
  fp_putmeth(fp, RESP_RESP_INIT,   PRVT_RESP_RESP_INIT);
  fp_putdig(fp, 0);			     /* void *data;               */

  /* class extension */
  fp_putstr(fp, lc_resp->yesexpr);	    /* char *yesexpr;            */
  fp_putstr(fp, lc_resp->noexpr);	    /* char *noexpr;             */
  fp_putstr(fp, lc_resp->yesstr);           /* char *yesstr;             */
  fp_putstr(fp, lc_resp->nostr);	    /* char *nostr;              */

#ifdef _PTR_METH
  fp_putsym(fp, "&lc_loc, 0, 0");
#else
  fprintf(fp,"\t&%s, 0, 0,\n",global_loc_ptr);
#endif
		 	  /* values for loc_rec, __meth_ptr, __data_ptr */

  fprintf(fp, "};\n\n");
}


/* 
*  FUNCTION: gen_locale
*
*  DESCRIPTION:
*  Generate C code which implements the _LC_locale_t locale
*  data structures. This routine collects the data from the various
*  child classes of _LC_locale_t, and outputs the pieces from the child
*  classes as appropriate.
*/
#ifdef _PTR_METH

static void
gen_locale(FILE *fp, _LC_charmap_t *lc_cmap, _LC_collate_t *lc_coll,  
	   _LC_ctype_t *lc_ctype, _LC_monetary_t *lc_mon, 
	   _LC_numeric_t *lc_num, _LC_resp_t *lc_resp, 
	   _LC_time_t *lc_time, _LC_aix31_t *lc_aix31)

#else

static void
gen_locale(FILE *fp, _LC_charmap_t *lc_cmap, _LC_collate_t *lc_coll,  
	   _LC_ctype_t *lc_ctype, _LC_monetary_t *lc_mon, _LC_numeric_t *lc_num,
	   _LC_resp_t *lc_resp, _LC_time_t *lc_time, _LC_aix31_t *lc_aix31, 
	   char *loc_ptr, char *char_tbl, char *coll_tbl, char *ctype_tbl,
	   char *mon_tbl, char *num_tbl, char *resp_tbl, char *time_tbl)

#endif
{
  int i;

  fprintf(fp, 
"/*-------------------------- LCONV STRUCT ---------------------------*/\n");
  fprintf(fp, "static struct lconv lc_lconv = {\n");
  fp_putstr(fp, lc_num->decimal_point);
  fp_putstr(fp, lc_num->thousands_sep);
  fp_putstr(fp, lc_num->grouping);
  fp_putstr(fp, lc_mon->int_curr_symbol);
  fp_putstr(fp, lc_mon->currency_symbol);
  fp_putstr(fp, lc_mon->mon_decimal_point);
  fp_putstr(fp, lc_mon->mon_thousands_sep);
  fp_putstr(fp, lc_mon->mon_grouping);
  fp_putstr(fp, lc_mon->positive_sign);
  fp_putstr(fp, lc_mon->negative_sign);
  fp_putdig(fp, lc_mon->int_frac_digits);
  fp_putdig(fp, lc_mon->frac_digits);
  fp_putdig(fp, lc_mon->p_cs_precedes);
  fp_putdig(fp, lc_mon->p_sep_by_space);
  fp_putdig(fp, lc_mon->n_cs_precedes);
  fp_putdig(fp, lc_mon->n_sep_by_space);
  fp_putdig(fp, lc_mon->p_sign_posn);
  fp_putdig(fp, lc_mon->n_sign_posn);
  fp_putstr(fp, lc_mon->left_parenthesis);
  fp_putstr(fp, lc_mon->right_parenthesis);
  fprintf(fp, "};\n\n");
  
  fprintf(fp, 
"/*-------------------------- LOCALE OBJECT --------------------------*/\n");

/*
** if the declaration of the lc_loc structure changes be sure to change
** gen_forward_dcl() also.
*/

#ifdef _PTR_METH

  fprintf(fp, "static _LC_locale_t lc_loc={\n");

#else

  if (private_table){
      fprintf(fp, "_LC_locale_t %s={\n",loc_ptr);
  }
  else {
      fprintf(fp, "static _LC_locale_t %s={\n",global_loc_ptr);
      /* in this case global_loc_ptr is lc_loc */
  }

#endif

  /* class core */
  fp_puthdr(fp, _LC_LOCALE);
  fp_putmeth(fp, LOCALE_NL_LANGINFO, PRVT_LOCALE_NL_LANGINFO);
  fp_putmeth(fp, LOCALE_LOCALECONV,  PRVT_LOCALE_LOCALECONV);
  fp_putmeth(fp, LOCALE_LOCALE_INIT, PRVT_LOCALE_LOCALE_INIT);
  fp_putdig(fp, 0);

  /* class extension */
					   /* -- lc_time data -- */
  fp_putstr(fp, "");		           /* dummy for 31 compat */
  fp_putstr(fp, lc_time->d_t_fmt);	   /* char *d_t_fmt; */
  fp_putstr(fp,lc_time->d_fmt);            /* char *d_fmt; */
  fp_putstr(fp, lc_time->t_fmt);	   /* char *t_fmt; */
  for (i=0; i<2; i++) 
    fp_putstr(fp, lc_time->am_pm[i]);	   /* char *am_pm[2]; */
  for (i=0; i<7; i++) 
    fp_putstr(fp, lc_time->abday[i]);	   /* char *abday[7]; */
  for (i=0; i<7; i++) 
    fp_putstr(fp, lc_time->day[i]);	   /* char *day[7]; */
  for (i=0; i<12; i++) 
    fp_putstr(fp, lc_time->abmon[i]);	   /* char *abmon[12]; */
  for (i=0; i<12; i++) 
    fp_putstr(fp, lc_time->mon[i]);	   /* char *mon[12]; */

					   /* -- lc_numeric data -- */
  fp_putstr(fp, lc_num->decimal_point);	   /* char *decimal_point; */
  fp_putstr(fp, lc_num->thousands_sep);	   /* char *thousands_sep; */

					   /* -- v3.1 yes/no strings -- */
  fp_putstr(fp, lc_resp->yesstr);	   /* char *yesstr; */
  fp_putstr(fp, lc_resp->nostr);	   /* char *nostr; */

					   /* X/Open CRNCYSTR */
  fp_putstr(fp, lc_mon->currency_symbol);  /* char *currency_symbol */

					   /* -- lc_cmap data -- */
  fp_putstr(fp, lc_cmap->cm_csname);	   /* char *cm_csname; */

					   /* -- lc_aix31 data -- */
  fp_putstr(fp, lc_aix31->nlldate);        /* char *nlldate;  */
  fp_putstr(fp, lc_aix31->nltmisc);	   /* char *nltmisc;  */
  fp_putstr(fp, lc_aix31->nltstr);	   /* char *nltstr;   */
  fp_putstr(fp, lc_aix31->nltunits);	   /* char *nltunits; */
  fp_putstr(fp, lc_aix31->nlyear);         /* char *nlyear;   */

  fp_putstr(fp, lc_time->t_fmt_ampm);	   /* char *t_fmt_ampm */
  fp_putstr(fp, lc_time->era);	 	   /* char *era */
  fp_putstr(fp, lc_time->era_d_fmt);	   /* char *era_d_fmt */
  fp_putstr(fp, lc_time->era_d_t_fmt);	   /* char *era_d_t_fmt */
  fp_putstr(fp, lc_time->era_t_fmt);	   /* char *era_t_fmt */
  fp_putstr(fp, lc_time->alt_digits);	   /* char *alt_digits */
  fp_putstr(fp, lc_resp->yesexpr);	   /* char *yesexpr */
  fp_putstr(fp, lc_resp->noexpr);	   /* char *noexpr */

					   /* -- lconv structure -- */
  fp_putsym(fp, "&lc_lconv");		   /* struct lconv *lc_lconv; */

#ifdef _PTR_METH
					   /* pointers to other classes */
  fp_putsym(fp, "&lc_cmap");		   /* _LC_charmap_t *charmap; */
  fp_putsym(fp, "(_LC_collate_t *)&lc_coll");/* _LC_collate_t *collate; */
  fp_putsym(fp, "&lc_ctype");		   /* _LC_ctype_t *ctype; */
  fp_putsym(fp, "&lc_mon");		   /* _LC_monetary_t *monetary; */
  fp_putsym(fp, "&lc_num");		   /* _LC_numeric_t *numeric; */
  fp_putsym(fp, "&lc_resp");		   /* _LC_resp_t *resp; */
  fp_putsym(fp, "&lc_time");		   /* _LC_time_t *time; */

#else

  /* 
  ** Because there can be a global table or a private table for each of the
  ** categories, a check must be done before the table information is 
  ** put in the file.
  */
					   /* pointers to other classes */
  fp_putsym(fp, "{&lc_cmap");		   /* _LC_charmap_t *charmap; */
  if (private_table) {
      fprintf(fp, "\t&%s,\n",char_tbl);
  }
  else
  {
     fp_putsym(fp, "(void *(**)())__METHOD_TBL");
     /* setlocale() replaces __METHOD_TBL(-1) with pointer to __method_tbl */
  }
  fp_putsym(fp, "0}");  /* place holder for loc_hdl, filled in by setlocale() */

  fp_putsym(fp, "{(_LC_collate_t *)&lc_coll");/* _LC_collate_t *collate; */
  if (private_table) {
      fprintf(fp, "\t&%s,\n",coll_tbl);
  }
  else
  {
     fp_putsym(fp, "(void *(**)())__METHOD_TBL");
     /* setlocale() replaces __METHOD_TBL(-1) with pointer to __method_tbl */
  }
  fp_putsym(fp, "0}");  /* place holder for loc_hdl, filled in by setlocale() */

  fp_putsym(fp, "{&lc_ctype");		   /* _LC_ctype_t *ctype; */
  if (private_table) {
      fprintf(fp, "\t&%s,\n",ctype_tbl);
  }
  else
  {
     fp_putsym(fp, "(void *(**)())__METHOD_TBL");
     /* setlocale() replaces __METHOD_TBL(-1) with pointer to __method_tbl */
  }
  fp_putsym(fp, "0}");  /* place holder for loc_hdl, filled in by setlocale() */

  fp_putsym(fp, "{&lc_mon");		   /* _LC_monetary_t *monetary; */
  if (private_table) {
      fprintf(fp, "\t&%s,\n",mon_tbl);
  }
  else
  {
     fp_putsym(fp, "(void *(**)())__METHOD_TBL");
     /* setlocale() replaces __METHOD_TBL(-1) with pointer to __method_tbl */
  }
  fp_putsym(fp, "0}");  /* place holder for loc_hdl, filled in by setlocale() */

  fp_putsym(fp, "{&lc_num");		   /* _LC_numeric_t *numeric; */
  if (private_table) {
      fprintf(fp, "\t&%s,\n",num_tbl);
  }
  else
  {
     fp_putsym(fp, "(void *(**)())__METHOD_TBL");
     /* setlocale() replaces __METHOD_TBL(-1) with pointer to __method_tbl */
  }
  fp_putsym(fp, "0}");  /* place holder for loc_hdl, filled in by setlocale() */

  fp_putsym(fp, "{&lc_resp");		   /* _LC_resp_t *resp; */
  if (private_table) {
      fprintf(fp, "\t&%s,\n",resp_tbl);
  }
  else
  {
     fp_putsym(fp, "(void *(**)())__METHOD_TBL");
     /* setlocale() replaces __METHOD_TBL(-1) with pointer to __method_tbl */
  }
  fp_putsym(fp, "0}");  /* place holder for loc_hdl, filled in by setlocale() */

  fp_putsym(fp, "{&lc_time");		   /* _LC_time_t *time; */
  if (private_table) {
      fprintf(fp, "\t&%s,\n",time_tbl);
  }
  else
  {
     fp_putsym(fp, "(void *(**)())__METHOD_TBL");
     /* setlocale() replaces __METHOD_TBL(-1) with pointer to __method_tbl */
  }
  fp_putsym(fp, "0}");  /* place holder for loc_hdl, filled in by setlocale() */

#endif

#ifdef _PTR_METH
  fp_putsym(fp, "&lc_loc, 0, 0");
#else
  fprintf(fp,"\t&%s, 0, 0,\n",global_loc_ptr);
#endif
		 	  /* values for loc_rec, __meth_ptr, __data_ptr */

  fprintf(fp, "};\n\n");
}


/* 
*  FUNCTION: gen_instantiate
*
*  DESCRIPTION:
*  Generates a _LC_load_object_t structure which contains the address of
*  of the the _LC_locale_t structure and if necessary the address of the
*  method table being used (for index method).
*/
#ifdef _PTR_METH

static void gen_instantiate(FILE *fp)

#else

static void gen_instantiate(FILE *fp, char *loc_ptr, char *loc_tbl)

#endif
{


#ifndef _PTR_METH

  if (private_table)
	fprintf(fp, "extern _LC_locale_t %s;\n",loc_ptr);

#endif

  fprintf(fp,"_LC_load_object_t lc_obj_hdl = {\n");
  fprintf(fp,"\t{_LC_OBJ_HDL,\n");
  fprintf(fp,"\t_LC_MAGIC,\n");
  fprintf(fp,"\t_LC_VERSION,\n");
  fprintf(fp,"\tsizeof(_LC_object_t)},\n");

#ifdef _PTR_METH

  fprintf(fp,"\t&lc_loc,\n");

#else

  fprintf(fp,"\t\t{&%s,\n", loc_ptr);

  if (strcmp(loc_tbl,"(void *(**)())__METHOD_TBL") == 0)
     fprintf(fp,"\t\t(void *(**)())__METHOD_TBL},\n");
     /* setlocale() replaces __METHOD_TBL(-1) with pointer to __method_tbl */
  else
     fprintf(fp,"\t\t(void *(**)())&%s},\n", loc_tbl);

#endif

  fprintf(fp,"\t};\n");

}

#ifndef _PTR_METH

/* 
*  FUNCTION: gen_charmap_tbl
*
*  DESCRIPTION:
*  Generates code to be compiled into the private method table for
*  the __lc_charmap variable. Dependent on the ordering of the 
*  PRVT_CHARMAP_xxxx defines in the "method.h" header file. 
*
*/
static void gen_charmap_tbl(FILE *fp, char *name)
{
	/* This routine is very dependent on the header file "method.h" */

	extern std_method_t loc_std_methods[];

	fprintf(fp,"/* Private Method table for __lc_charmap */\n\n");
	fprintf(fp,"int  (*%s[])() = {\n",name);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CHARMAP_CHARMAP_INIT].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CHARMAP_NL_LANGINFO].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CHARMAP___MBSTOPCS].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CHARMAP___MBTOPC].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CHARMAP___PCSTOMBS].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CHARMAP___PCTOMB].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CHARMAP_CSID].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CHARMAP_MBLEN].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CHARMAP_MBSTOWCS].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CHARMAP_MBTOWC].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CHARMAP_WCSID].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CHARMAP_WCSTOMBS].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CHARMAP_WCSWIDTH].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CHARMAP_WCTOMB].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CHARMAP_WCWIDTH].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t };\n\n");
}

/* 
*  FUNCTION: gen_collate_tbl
*
*  DESCRIPTION:
*  Generates code to be compiled into the private method table for
*  the __lc_collate variable. Dependent on the ordering of the 
*  PRVT_COLLATE_xxxx defines in the "method.h" header file. 
*
*/
static void gen_collate_tbl(FILE *fp, char *name)
{
	/* This routine is very dependent on the header file "method.h" */

	extern std_method_t loc_std_methods[];
	char p[200];

	fprintf(fp,"/* Private Method table for __lc_collate */\n\n");
	fprintf(fp,"int  (*%s[])() = {\n",name);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[COLLATE_COLLATE_INIT].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[COLLATE_FNMATCH].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[COLLATE_REGCOMP].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[COLLATE_REGERROR].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[COLLATE_REGEXEC].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[COLLATE_REGFREE].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[COLLATE_STRCOLL].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[COLLATE_STRXFRM].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[COLLATE_WCSCOLL].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[COLLATE_WCSXFRM].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t };\n\n");
}

/* 
*  FUNCTION: gen_ctype_tbl
*
*  DESCRIPTION:
*  Generates code to be compiled into the private method table for
*  the __lc_ctype variable. Dependent on the ordering of the 
*  PRVT_CTYPE_xxxx defines in the "method.h" header file. 
*
*/
static void gen_ctype_tbl(FILE *fp, char *name)
{
	/* This routine is very dependent on the header file "method.h" */

	extern std_method_t loc_std_methods[];
        char p[200];
	int x;

	fprintf(fp,"/* Private Method table for __lc_ctype */\n\n");
	fprintf(fp,"int  (*%s[])() = {\n",name);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CTYPE_CTYPE_INIT].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CTYPE_GET_WCTYPE].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CTYPE_IS_WCTYPE].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CTYPE_TOWLOWER].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[CTYPE_TOWUPPER].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t };\n\n");
}

/* 
*  FUNCTION: gen_locale_tbl
*
*  DESCRIPTION:
*  Generates code to be compiled into the private method table for
*  the __lc_locale variable. Dependent on the ordering of the 
*  PRVT_LOCALE_xxxx defines in the "method.h" header file. 
*
*/
static void gen_locale_tbl(FILE *fp, char *name)
{
	/* This routine is very dependent on the header file "method.h" */

	extern std_method_t loc_std_methods[];

	fprintf(fp,"/* Private Method table for __lc_locale */\n\n");
	fprintf(fp,"int  (*%s[])() = {\n",name);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[LOCALE_LOCALE_INIT].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[LOCALE_NL_LANGINFO].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[LOCALE_LOCALECONV].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t };\n\n");
}

/* 
*  FUNCTION: gen_monetary_tbl
*
*  DESCRIPTION:
*  Generates code to be compiled into the private method table for
*  the __lc_monetary variable. Dependent on the ordering of the 
*  PRVT_MONETARP_xxxx defines in the "method.h" header file. 
*
*/
static void gen_monetary_tbl(FILE *fp, char *name)
{
	/* This routine is very dependent on the header file "method.h" */

	extern std_method_t loc_std_methods[];

	fprintf(fp,"/* Private Method table for __lc_monetary */\n\n");
	fprintf(fp,"int  (*%s[])() = {\n",name);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[MONETARY_MONETARY_INIT].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[MONETARY_NL_LANGINFO].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[MONETARY_STRFMON].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t };\n\n");
}

/* 
*  FUNCTION: gen_resp_tbl
*
*  DESCRIPTION:
*  Generates code to be compiled into the private method table for
*  the __lc_resp variable. Dependent on the ordering of the 
*  PRVT_RESP_xxxx defines in the "method.h" header file. 
*
*/
static void gen_resp_tbl(FILE *fp, char *name)
{
	/* This routine is very dependent on the header file "method.h" */

	extern std_method_t loc_std_methods[];

	fprintf(fp,"/* Private Method table for __lc_resp */\n\n");
	fprintf(fp,"int  (*%s[])() = {\n",name);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[RESP_RESP_INIT].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[RESP_NL_LANGINFO].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[RESP_RPMATCH].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t };\n\n");
}

/* 
*  FUNCTION: gen_numeric_tbl
*
*  DESCRIPTION:
*  Generates code to be compiled into the private method table for
*  the __lc_numeric variable. Dependent on the ordering of the 
*  PRVT_NUMERIC_xxxx defines in the "method.h" header file. 
*
*/
static void gen_numeric_tbl(FILE *fp, char *name)
{
	/* This routine is very dependent on the header file "method.h" */

	extern std_method_t loc_std_methods[];

	fprintf(fp,"/* Private Method table for __lc_numeric */\n\n");
	fprintf(fp,"int  (*%s[])() = {\n",name);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[NUMERIC_NUMERIC_INIT].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[NUMERIC_NL_LANGINFO].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t };\n\n");
}

/* 
*  FUNCTION: gen_time_tbl
*
*  DESCRIPTION:
*  Generates code to be compiled into the private method table for
*  the __lc_time variable. Dependent on the ordering of the 
*  PRVT_TIME_xxxx defines in the "method.h" header file. 
*
*/
static void gen_time_tbl(FILE *fp, char *name)
{
	/* This routine is very dependent on the header file "method.h" */

	extern std_method_t loc_std_methods[];

	fprintf(fp,"/* Private Method table for __lc_time */\n\n");
	fprintf(fp,"int  (*%s[])() = {\n",name);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[TIME_TIME_INIT].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[TIME_NL_LANGINFO].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[TIME_STRFTIME].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[TIME_STRPTIME].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t %s,\n",
		loc_std_methods[TIME_WCSFTIME].c_symbol[USR_DEFINED]);
	fprintf(fp,"\t };\n\n");
}

#endif 
