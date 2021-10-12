/*
 * AIX 3.2 Internationalization Sample Environment Package
 *
 * (C) COPYRIGHT International Business Machines Corp. 1995 All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or disclosure 
 * restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THIS PACKAGE OF SAMPLE
 * ENVIRONMENT FILES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, AS IS, 
 * WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT
 * LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
 * PARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE 
 * SAMPLE ENVIRONMENT FILES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, IS
 * WITH YOU. SHOULD ANY PART OF THE SAMPLE ENVIRONMENT PACKAGE PROVE DEFECTIVE,
 * YOU (AND NOT IBM) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICE, SUPPORT,
 * REPAIR OR CORRECTION.
 *
 * Each copy of the AIX 3.2 ILS Sample Environment Package or derivative work
 * thereof must reproduce the IBM Copyright notice and the complete contents of
 * this notice.
 *
 *****************************************************************************/
 
/* cmDB.c -- Builds UCS DB of symbols in charmap                         */
/*                     Prints DB in format                                 */
/*                        Uxxxx|<symbol>:csid,...|hex_codepoint		   */
/*                        where                                            */
/*                          hex_codepoint is x??x??...                     */
/*                          csid is in decimal                             */
/*                                                                         */
/*                     removed to stdout                                   */
/* Usage -- cmDB [-c codeset] <CHARMAP FILE>                             */

#include <stdio.h>
#include <stdlib.h>
#include <iconv.h>
#include <fcntl.h>
#include <locale.h>
#include "cmDB.h"

/*
 * Global Variables
 */
iconv_t  cp2ucs_cd;
char verbose = False;

size_t conv (iconv_t cd, uchar_t* buf, const uchar_t* in, size_t len) {
/* Input: similiar to iconv, size of outbuf not checked */
/* Output: size of conversion */
/* Result: iconv conversion is put into buf */
    
        size_t  inleft = len;
	size_t  outleft = 1024;

	uchar_t* out = buf;
        int     ret;

        ret = iconv (cd, &in, &inleft, &out, &outleft);
	if ( ret < 0 ) {
	    printf ("iconv error %d (errno=%d)\n", ret, errno);
	    return (0);
	}
	return ( 1024 - outleft);
    }

char *itoa(int n, int numlen)
/* Input: Integer number and its length          */
/* Output: Number represented as an ascii string */
/* Result: Converts integer number into ascii */
{
    int i, j, k;
    char buf[NUM_LEN];
    char *rbuf;
    for(i=0; n > 0; i++, n = n / 10)    /* Compute ascii digits from number */
        buf[i] = n % 10 + '0';          /* in reverse order */
    buf[i] = '\0';
    j = strlen(buf)+1;
    /* Add leading zeros to end of buffer */
    for(k=0; k < numlen - j; k++, i++)  
        buf[i]='0';
    buf[i] = '\0';
    rbuf = (char *) malloc (strlen(buf)+2);
    for(i=strlen(buf)-1, j=0; i >= 0; i--, j++)         /* Reverse buffer */
        rbuf[j] = buf[i];
    rbuf[j++] = '>';                    /* Add ending '>' to complete name */
    rbuf[j] = '\0';
    return rbuf;
}


Charmap* initCharmap()
/* Input: None                                  */
/* Output: Pointer to initialized Charmap entry   */
/* Result: Creates Charmap entry and returns pointer to it */
{
    Charmap *new;
    new = (Charmap *) malloc (sizeof(Charmap));
    new->nlist = NULL;
    new->codept = UNASSIGNED;
    new->next = NULL;
    return new;
}

void addName(Charmap* cm, char *name, UniChar ucs)
/* Input: Symbolic name and corresponding codepoint of a given character */
/* Output: None                                                          */
/* Result: Adds name and codepoint to table, first searching for the     */
/*         name in the table, then the num (codept). If neither are      */
/*         found, add name and num to table                              */
{
    Symbol *nameptr;
    Charmap *new;
    Charmap *ptr = cm;
    Charmap *last = (Charmap*)NULL;

    if ( verbose )
       fprintf(stderr, "addName: %s\n", name);

    /* Search for num */
    while (ptr != NULL && ptr->codept < ucs ) {
	last = ptr;
	ptr = ptr->next;
    }

    if (ucs == ptr->codept) {
        Symbol* last_name = (Symbol*)NULL;

        /* Search for name in table */
        nameptr = ptr->nlist;
        while (nameptr != NULL  && ! strcmp(name, nameptr->name) == 0) {
		last_name = nameptr;
		nameptr = nameptr->next;
	}

        if ( nameptr ) {
	        /* Name is found */
                if (ptr->codept != UNASSIGNED) {
                    if (ptr->codept== ucs)   
                        return;
                    else                     
                        fprintf(stderr, "Warning: Code point already set for character %s.\n", name);       
                }
                else { /* If codept has not been set, add it */
                    ptr->codept = ucs;       
                }
                return;
        }
        nameptr = last_name;
        nameptr->next = (Symbol *) malloc (sizeof(Symbol));  
        nameptr->next->name = (char *) malloc (strlen(name)+1);
        strcpy(nameptr->next->name, name);
        nameptr->next->next = NULL;
        nameptr->next->csid = 0xff; /* unknown at this time */
        return;
    }

    /* symbol/codept not found - add new entry */
    if ( ptr && last )  { /* Insert into the middle */
        last->next = initCharmap();
	last->next->next = ptr;
        ptr = last->next;
    }
    else if ( ptr ) { /* Insert to the very beginning */
        Charmap* ptr = initCharmap();
	ptr->codept = cm->codept;
	ptr->nlist  = cm->nlist;
	ptr->next   = cm->next;
	cm->next    = ptr;
	ptr = cm;
    }
    else { /* Insert into the end */
	last->next = initCharmap();
        ptr = last;
    }
    ptr->nlist = (Symbol *) malloc (sizeof(Symbol));  
    ptr->nlist->name = (char *) malloc (strlen(name)+1);
    strcpy(ptr->nlist->name, name);
    ptr->nlist->csid = 0xff; /* unknown at this time */
    ptr->nlist->next = NULL;
    ptr->codept = ucs;
}

void addRange(Charmap* cm, char *beg, char *end, int num)
/* Input: Symbolic names of beginning and end of range and */
/*        corresponding codept                             */
/* Output: None                                            */
/* Result: Adds range of entries to Charmap                  */
{
    int length, numlen, range, i, j;
    char start[NUM_LEN];
    char finish[NUM_LEN];
    char xnum[3];
    char numbuf[NUM_LEN];
    char namebuf[NUM_LEN];
    char sname[NUM_LEN];
    int nname;
    for(i=0; !isdigit(beg[i]); i++)
    for(j=0; isdigit(beg[i]); i++, j++)
        start[j] = beg[i];
    start[j] = '\0';
    for(i=0; !isdigit(end[i]); i++);   
    for(j=0; isdigit(end[i]); i++, j++)
        finish[j] = end[i];
    finish[j] = '\0';
    range = atoi(finish) - atoi(start);      
    for(i=0; !isdigit(*beg); i++, beg++)     
        sname[i] = *beg;                     
    sname[i] = '\0';                         
    numlen = strlen(beg);
    sscanf(beg, "%d", &nname);               
    for(i=0; i <= range; i++) {
        strcpy(namebuf,sname);
        strcat(namebuf, itoa(nname, numlen));
                                             
        addName(cm, namebuf, num);                 
        num++;                                 
        nname++;                               
        if ((num >> 24) >= 0xff) {             
            fprintf(stderr, "Error: Codepoint exceeded limit.\n");
            return;
        }
    }
}

void addCsid(Charmap* cm, char *name, int csid)
/* Input: Symbolic name and corresponding codepoint of a given character */
/* Output: None                                                          */
/* Result: Adds name and codepoint to table, first searching for the     */
/*         name in the table, then the num (codept). If neither are      */
/*         found, add name and num to table                              */
{
    Symbol *nameptr;
    Charmap* sym;
    Symbol *prev, *last;

    if ( (sym = findName( cm, name)) == (Charmap*)NULL) {
       fprintf(stderr, 
	       "Warning: CSID %s not defined in CHARMAP section.\n", name);
    }
    nameptr = sym->nlist;
    last = prev = (Symbol*)NULL;
    while(nameptr != (Symbol*)NULL) {
       if (strcmp(name, nameptr->name) == 0) {
	   /* found! */
           nameptr->csid = csid;
	   /* 
	    * Move to end of Symbol, so order is reflected
	    * at the end... any sym w/o csid will have 0xff.
            */
	   if ( nameptr->next == (Symbol*)NULL)
	      return;
	   for ( last = nameptr ; last->next ; last = last->next );
	   if (prev)
	      prev->next = nameptr->next;
	   else
	      sym->nlist = nameptr->next;
	   last->next = nameptr;
	   nameptr->next = (Symbol*)NULL;
	   return;
       }
       else {
	  prev = nameptr;
       }
       nameptr = nameptr->next;
    }
    return;
}

Charmap* findName(Charmap* cm, char *name) 
/* Input: Symbolic name of character    */
/* Output: Codept corresponding to name */
/* Result: Finds codept that corresponds to character name */
{
    Symbol *nameptr;
    Charmap *ptr;
    int ret;
    ptr = cm;
    /* Loop through each entry and its */
    while (ptr != NULL && ptr->nlist != NULL) {
        nameptr = ptr->nlist;  
        while(nameptr != NULL) {
            if (strcmp(name, nameptr->name) == 0) {
                return ptr;
            }
            else {
                if (nameptr->next != NULL)
                    nameptr = nameptr->next;
                else
                    break;
            }
        }
        ptr = ptr->next;
    }
    fprintf(stderr, "Error: Could not find< entry for %s.\n", name);
    exit(1);
}

Charmap* findCode(Charmap* cm, UniChar cdpt)
/* Input: Integer codepoint                      */
/* Output: Character string                      */
/* Result: Searches charmap Charmap to find the    */
/*         name that corresponds with cdpt       */
/*         If not found, return NULL             */
{
    Charmap *ptr = cm;
    
    while (ptr != NULL) {               /* Loop through each codept */
        if (ptr->codept == cdpt)
            return (ptr);    /* Return name if found */
        else
            ptr = ptr->next;
    }
    return NULL;                        /* Return NULL if not found */
}

void printCM(Charmap* cm)
/* Input: None                                  */
/* Output: Prints charmap symbols - 1 per line  */
/* Result: Reads entire table and prints it out */
{
    Symbol *p;
    char* comma;
    while (cm != NULL && cm->next != NULL) {
        printf("U%04X|",cm->codept);
        comma="";
        for( p = cm->nlist;
	       p != NULL; p = p->next) {
	  printf("%s%s:%d", comma, p->name, p->csid);
	  comma=",";
	}
	printf("\n");
	cm = cm->next;
    }
}


void insertEntry(Charmap* cm, char *buf, FILE *file, char type)
/* Input: Buffer with line to be parsed, file descriptor, ID flag */
/*        -- If ID is FALSE, buffer could contain:                */
/*             <name>                   codept                    */
/*             <name>...<name>          codept                    */
/*             <name>...\                                         */
/*             <name>                   codept                    */
/* Output: None                                                   */
/* Result: Input line is parsed and one of four Charmap insert      */
/*         routines are called:                                   */
/*              addName -- insert single name to Charmap            */
/*              addRange -- insert range of names                 */
{
    int i, j, x;
    char name[LINE_LEN];
    char end[LINE_LEN];
    char cpt[NUM_LEN];
    unsigned long num;
    UniChar ucs;
    char utf[7];
    /* Find symbol name */
    for(i=0; buf[i] != '>'; i++)
        name[i] = buf[i];
    name[i] = '>';
    name[i+1] = '\0';
    i++;
    /* Handle range of values */
    if ((type == T_CHARMAP) && (buf[i] == '.' || buf[i] == '\\')) {
        while (buf[i] != '<' && buf[i] != '\\')
            i++;
        if (buf[i] == '\\') {
            /* Retrieve next line after continuation */
            fgets (buf, LINE_LEN, file);
            i=0;
            while (buf[i] != '<')
                i++;
        }

        /* Determine end of range name */
        for(j=0; buf[i] != '>'; i++, j++)
            end[j] = buf[i];
        end[j++] = '>';
        end[j] = '\0';
        i++;
        /* Value is stored as a single 32 bit */
        num = 0; 
        while (buf[i] != '\n') {        /* number, each 8 bits representing */
            if (buf[i] == '\\') {       /* one hex number, i.e. \xa3\x57\xef */
                if (buf[i+1] == 'x') {
                    sscanf(&(buf[i+2]), "%2x", &x);     
                    utf[num++] = x;
                }
            }
            i++;
        }
	conv( cp2ucs_cd, (uchar_t*)&ucs, utf, num);
        addRange(cm, name, end, ucs);
    }
    else if ( type == T_CHARMAP) { /* No range of values */
        num = 0; 
        while (buf[i] != '\n') {
            if (buf[i] == '\\') {
                sscanf(&buf[i+2], "%2x", &x);   
                utf[num++] = x;
            }
            i++;
        }
	conv( cp2ucs_cd, (uchar_t*)&ucs, utf, num);
        addName(cm, name, ucs);
    }
    else { /* Add Csid to name */
        num = 0; 
        while (buf[i] != '\n' && !isdigit(buf[i])) ++i; 
        while (isdigit(buf[i])) {
            num = (num * 10) + ( buf[i] - '0' );
            i++;
	}    
        while (buf[i] != '\n') ++i;
        addCsid( cm, name, num);
    }
}

void fillCharmap(Charmap* cm, FILE *file, char type)
/* Input: Charmap Symbol, File descriptor, END delimiter string   */
/* Output: None                                                 */
/* Result: For each symbolic character name, call insertEntry   */
/*         to add the character to the Charmap                    */
/*         Return when END CHARMAP is found                     */
{
  char buf[LINE_LEN]; 
  char str[LINE_LEN];
  char* end = (type == T_CHARMAP ? "END CHARMAP" : "END CHARSETID");
  
  int i,j;
  i=0;
  while (fgets(buf, LINE_LEN, file)) {
      i=0;
      switch (buf[i]) {
      case '<':
          insertEntry(cm, buf, file, type); 
          break;  /* FALSE flag indicates that codeset ids */
                                                /* are NOT being processed */
      case 'E':
          for(i=0, j=0; buf[i] != '\n' && buf[i] != '\t'; i++, j++)
              str[j]=buf[i]; 
          str[j]='\0'; 
          if (strcmp(str, end)==0)  /* Look for "END _______" */
              return;
          break;
      default:
          break;
      }
  }
}

void loadCharmap(Charmap* cm, FILE *file, char type)
/* Input: File descriptor for charmap                   */
/* Output: None                                         */
/* Result: Parse apart file, scanning line by line      */
/*         Search for CHARSET identifier                */
/*         and call appropriate sub-routines.           */
{
  int i,j;
  char buf[LINE_LEN]; 
  char str[LINE_LEN];
  char* start = (type == T_CHARMAP ? "CHARMAP" : "CHARSETID");
  
  while (fgets(buf, LINE_LEN, file)) {
      i=0;
      switch (buf[i]) {
      case 'C':
          for(i=0, j=0; buf[i] != '\n' && buf[i] != '\t' 
              && buf[i] != ' '; i++, j++)
              str[j]=buf[i]; 
          str[j]='\0'; 
          if (strcmp(str, start) == 0) {
              fillCharmap(cm, file, type);
              return;
          }
          break;
      default:
          break;
      }
  }
  fprintf(stderr, "Error: File not completely processed.\n");
}


/*****************************************************************/

void main(int argc, char **argv)
/* Input: Command line arguments -- charmap and locsrc files */
/* Output: New locsrc file written to stdout                 */
{
    Charmap *cm; 
    int i;
    FILE *fp1;
    char* codeset = "UTF-8";
    char** p = argv;

    fp1 = 0;
    while (1) {
       if ( *p[1] != '-' ) break;
       switch (*(p[1]+1)) {
	  case 'c' : codeset = p[1];
                     p += 2;
                     break;
          case 'h' :
          case '?' : fprintf(stderr, 
                               "Usage: cmDB [-c codeset] <CHARMAP_FILE>\n");
                      exit (1);
	  case 'v' : verbose = True;
		     p += 1;
		     break;
          default  : fp1 = stdin ;
                     break;
       }
    }
    argv = p;

    cp2ucs_cd = iconv_open( "UCS-2", codeset);
    if ( cp2ucs_cd == (iconv_t) NULL ) {
        printf("%s: iconv_open failed (errno=%)\n",argv[0],errno);
        exit(0);
    }


    if (fp1 == 0) fp1 = fopen(argv[1], "r");
    if (fp1 == 0) {
        fprintf(stderr, "Error: Unable to open file %s\n", argv[1]);
        return;
    }

    cm = initCharmap();
    loadCharmap(cm, fp1, T_CHARMAP);
    fprintf(stderr, "Starting CHARSETID\n");
    loadCharmap( cm, fp1, T_CHARSETID);
    fprintf(stderr, "Starting Printing\n");
    printCM(cm);
    exit (0);
}
