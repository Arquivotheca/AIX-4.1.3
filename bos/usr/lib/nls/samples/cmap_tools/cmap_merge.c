/*****************************************************************************
 *
 * AIX 3.2 Internationalization Sample Environment Package
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 All Rights Reserved
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
 
/* cmap_merge.c -- Reads a list of charmap source file definitions and	     */
/*                 writes a merged version of these charmap files. Merges    */
/*                 both the CHARSET and CHARSETID parts of the charmap file. */
/* Usage -- cmap_merge [-m] <FILE1> <FILE2> [ <FILE3> ... ]			     */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmap_merge.h"


Table *table;		/* Global declaration on table structure */
Table **tableArray; 	/* Global declaration of table array used for sort */

void main(int argc, char* argv[])
/* Input: Command line arguments -- files to be merged	*/
/* Output: Merged file written to stdout		*/
{
    int i, size;
    Table *ptr;

    if (argc < 3) {
	printf("Usage: cmap_merge [-m] <FILE1> <FILE2> [ <FILE3> ... ]\n");
	return;
    }

    table = initEntry();		      /* Initilize first entry of table */

    for (i=1; i < argc; i++) {
	if (strcmp(argv[i], "-m") == 0)       /* -m option allows printing of all */
	    PRINT_ONE = 0;		      /* symbolic names associated with a codept */
					      /* Default prints first name only */
	else
	    processFile(argv[i]);	      /* Process one file at a time in the order */
	                                      /* that they appear. Mappings decrease in */
	                                      /* priority left to right */ 
	
/*	fillEmptyIDs();			      /* After each file has been processed, */
	                                      /* fill non-specified csid's with the */
	                                      /* default value 0 */
	
    }
    fillEmptyIDs();
  
    ptr = table;			      /* Determine number of table entries */
    for (size=0; ptr != NULL; size++, ptr = ptr->next);

    printTable(size-1);			      /* Print table structure */
    destroyTable(table);		      /* Free table space */
    free(tableArray);			      /* Free array space */
}


Table *initEntry()
/* Input: None				     	*/
/* Output: Pointer to initialized table entry   */
{
    Table *new;

    new = (Table *) malloc (sizeof(Table));

    new->nlist = NULL;
    new->codept = -1;
    new->csid = -1;
    new->next = NULL;
    return new;
}


void destroyTable(Table *table)
/* Input: Pointer to table to be destroyed 	*/
/* Output: None					*/
/* Result: Free space occupied by table		*/
{
    free (table);
}
	

void processFile(char *file)
/* Input: File name 				*/
/* Output: None					*/
/* Result: Open file for processing		*/
{
    FILE *fp;

    fp = fopen(file, "r");			
    if (fp == NULL) {
	fprintf(stderr, "Error: Unable to open file %s.\n", file);
	return;
    }

    scanFile(fp);				/* Pass file to processing procedure */
}


void scanFile(FILE *file)
/* Input: File descriptor				*/
/* Output: None						*/
/* Result: Parse apart file, scanning line by line 	*/
/*	   Determine code set name and min/max values	*/
/*	   Search for CHARSET and CHARSETID identifiers */
/*         and call appropriate sub-routines. Sort	*/
/*	   table before processing csids.		*/
{
  int i,j, size;
  int min, max;
  Table *ptr;

  char buf[LINE_LEN]; 
  char str[LINE_LEN];
  
  while (fgets(buf, LINE_LEN, file)) {
      i=0;
      switch (buf[i]) {
      case '<':
	  for(i++, j=0; buf[i] != '>'; i++, j++)
	      str[j]=buf[i];
	  str[j]='\0';

	  if (strcmp(str, "code_set_name") == 0) {	    
	      if (FIRST_FILE) {
		  while (buf[i] != '\"')  i++;
		  for(i++, j=0; buf[i] != '\"'; i++, j++)
		      str[j]=buf[i];
		  str[j]='\0';
		  printf("\n<code_set_name>			\"%s\"\n", str);
		  FIRST_FILE = 0;
	      }
	      else 
		  break;
	  }
	  else if (strcmp(str, "mb_cur_max") == 0) {
	      while (!isdigit(buf[i])) i++;
	      for (j=0; isdigit(buf[i]); i++, j++)
		  str[j]=buf[i];
	      max = atoi(str);
	      if (max > mb_max)			/* Update max if needed */
		  mb_max = max;
	  }
	  else if (strcmp(str, "mb_cur_min") == 0) {
	      while (!isdigit(buf[i])) i++;
	      for (j=0; isdigit(buf[i]); i++, j++)
		  str[j]=buf[i];
	      min = atoi(str);
/*	      sscanf(buf, "%d", &min); */
	      if (min < mb_min)			/* Update min if needed */
		  mb_min = min;
	  }

	  break;
	     
      case 'C':
	  for(i=0, j=0; buf[i] != '\n' && buf[i] != '\t' 
	      && buf[i] != ' '; i++, j++)
	      str[j]=buf[i]; 
	  str[j]='\0'; 

	  if (strcmp(str, "CHARMAP") == 0) {
	      fillTable(file);			       /* Fill table with entries */
	  }
	  
	  ptr = table;			      	      /* Determine number of table entries */
	  for (size=0; ptr != NULL; size++, ptr = ptr->next);
	  sortTable(size-1);			      /* Sort table by codepoint */

	  if (strcmp(str, "CHARSETID") == 0) {
	      fillID(file);				/* Add csid values to entries */
	      return;
	  }
	  break;
      default:
	  break;
      }
  }
  fprintf(stderr, "Error: File not completely processed.\n");
}


void fillTable(FILE *file)
/* Input: File descriptor					*/
/* Output: None							*/
/* Result: For each symbolic character name, call insertEntry	*/
/*         to add the character to the table			*/
/*	   Return when END CHARMAP is found			*/
{
  char buf[LINE_LEN]; 
  char str[LINE_LEN];
  
  int i,j;
  i=0;

  while (fgets(buf, LINE_LEN, file)) {
      
      switch (buf[i]) {

      case '<':
	  insertEntry(buf, file, FALSE);	/* Pass buffer to insertEntry */
	  break;				/* FALSE flag indicates that codeset ids */
	                                        /* are NOT being processed */

      case 'E':
	  for(i=0, j=0; buf[i] != '\n' && buf[i] != '\t'; i++, j++)
	      str[j]=buf[i]; 
	  str[j]='\0'; 

	  if (strcmp(str, "END CHARMAP")==0) 
	      return;
	  break;

      default:
	  break;
      }
  }
}


void fillID(FILE *file)
/* Input: File descriptor				*/
/* Output: None						*/
/* Result: For each symbolic name, call inertID to insert */
/*         corresponding ID to table			*/
/*         Return when END CHARSETID is found		*/
{
    char buf[LINE_LEN];
    char str[LINE_LEN];

    int i,j;
    i=0;

    while (fgets(buf, LINE_LEN, file)) {
	
	switch (buf[i]) {
	    
	case '<':
	    insertEntry(buf, file, TRUE);
	    break;

	case 'E':
	    for(i=0, j=0; buf[i] != '\n' && buf[i] != '\t'
		&& buf[i] != ' '; i++, j++)
		str[j]=buf[i];
	    str[j]='\0';

	    if (strcmp(str, "END") == 0) {
		return;
	    }
	    break;

	default:
	    break;
	}
    }
}


void insertEntry(char *buf, FILE *file, int ID)
/* Input: Buffer with line to be parsed, file descriptor, ID flag */
/*        -- If ID is FALSE, buffer could contain:		  */
/*	       <name>			codept			  */
/*	       <name>...<name>  	codept			  */
/*	       <name>...\					  */
/*	       <name>			codept			  */
/*	  -- If ID is TRUE, buffer could contain:		  */
/*	       <name>			csid			  */
/*	       <name>...<name>		csid			  */
/*	       <name>...\					  */
/*	       <name>			csid			  */
/* Output: None							  */
/* Result: Input line is parsed and one of four table insert	  */
/*         routines are called:					  */
/*		addName -- insert single name to table	  	  */
/*		addID -- insert ID into table			  */
/*		addRange -- insert range of names		  */
/*		addRangeID -- insert range of IDs		  */
{
    int i, j, x;
    char name[LINE_LEN];
    char end[LINE_LEN];
    char cpt[NUM_LEN];
    unsigned long num;

    for(i=0; buf[i] != '>'; i++)
	name[i] = buf[i];
    name[i] = '>';
    name[i+1] = '\0';
    i++;

    if (buf[i] == '.' || buf[i] == '\\') {		/* Indicate range of values */
	while (buf[i] != '<' && buf[i] != '\\')
	    i++;

	if (buf[i] == '\\') {
	    fgets (buf, LINE_LEN, file);     	/* Retrieve next line after continuation */
	    i=0;
	    while (buf[i] != '<')
		i++;
	}

	for(j=0; buf[i] != '>'; i++, j++)	/* Determine end of range name */
	    end[j] = buf[i];
	end[j] = '>';
	end[j+1] = '\0';
	i++;

	if(!ID) {				/* Parse apart codepoint value -- */
	    num = 0;				/* Value is stored as a single 32 bit */
	    while (buf[i] != '\n') {		/* number, each 8 bits representing */
		if (buf[i] == '\\') {		/* one hex number, i.e. \xa3\x57\xef */
		    i++;			/* is stored as 00a357ef */
		    if (buf[i] == 'x') {
			sscanf(&(buf[i+1]), "%2x", &x);	
			num = (num << 8) + x;
		    }
		}
		i++;
	    }
	}
	else 
	    sscanf(&buf[i], "%d", &num);		/* Simply retrieve csid if ID is TRUE */
	if (!ID)
	    addRange(name, end, num);
	else 
	    addRangeID(name, end, num);
    }

    else {					/* No range of values */
	if (!ID) {
	    num = 0;				/* Codepoint stored as described above */
	    while (buf[i] != '\n') {
		if (buf[i] == '\\') {
		    sscanf(&(buf[i+2]), "%2x", &x);	
		    num = (num << 8) + x;
		}
		i++;
	    }
	}
	else 
	    sscanf(&buf[i], "%d", &num);		/* Retrieve csid if ID is TRUE */
	if(!ID) {
	    addName(name, num);
	}
	else {
	    addID(findCodept(name), num);
	}
    }
}

void addName(char *name, int num)
/* Input: Symbolic name and corresponding codepoint of a given character */
/* Output: None								 */
/* Result: Adds name and codepoint to table, first searching for the 	 */
/*	   name in the table, then the num (codept). If neither are	 */
/*	   found, add name and num to table				 */
{
    List *nameptr;
    Table *ptr, *new;

    ptr = table;

    while (ptr != NULL && ptr->nlist != NULL) {		/* Search for name in table */
	nameptr = ptr->nlist;
	while (nameptr != NULL) {
	    if (strcmp(name, nameptr->name) == 0) {	/* Name is found */
		if (ptr->codept != -1) {		/* If codepoint has been set, return */
		    if (ptr->codept== num)		/* if it has the same value */
			return;
		    else			    /* If codept is different, issue warning */
			fprintf(stderr, "Warning: Code point already set for character %s.\n", name);	    
		}
		else {
		    ptr->codept = num;			/* If codept has not been set, add it */
		}
		return;
	    }
	    else {
		if (nameptr->next != NULL)		/* If name is not found, continue to */
		    nameptr = nameptr->next;		/* next entry */
		else 
		    break;
	    }
	}
	ptr = ptr->next;
    }

    ptr = table;	  				/* Set pointer to beginning of table */

    while (ptr != NULL && ptr->codept != -1) {  	/* Search for num */
	if (num == ptr->codept) {
	    nameptr = ptr->nlist;
	    while (nameptr->next != NULL) {		/* Continue to end of name list */
		nameptr = nameptr->next;
	    }
	    nameptr->next = (List *) malloc (sizeof(List));	/* Add name to list */
	    nameptr->next->name = (char *) malloc (strlen(name)+1);
	    strcpy(nameptr->next->name, name);
	    nameptr->next->next = NULL;
	    return;
	}
	else 
	    ptr = ptr->next;
    }

    ptr->nlist = (List *) malloc (sizeof(List));	   /* If name & codept not found, */
    ptr->nlist->name = (char *) malloc (strlen(name)+1);   /* add new entry */
    strcpy(ptr->nlist->name, name);
    ptr->nlist->next = NULL;
    ptr->codept = num;
    ptr->next = initEntry();				   /* Initialize next entry */
}

void addID(int cpt, int csid)
/* Input: Codepoint and corresponding csid 	    */
/* Output: None					    */
/* Result: Sets csid for entry corresponding to cpt */
/*	   Search for csid first and add if not set */
{
    Table *ptr;

    ptr = table;				/* Set pointer to beginning of table */
    
    while (ptr != NULL && ptr->nlist != NULL) {		/* Search for csid */
	if (cpt == ptr->codept) {			/* Found csid */
	    if (ptr->csid != -1) {			/* Csid already set */
		if (ptr->csid ==  csid)			/* Return if csid is the same */
		    return;
		else {					/* Issue warning if csid different */
		    fprintf(stderr, "Warning: CSID already set for codepoint %x\n", ptr->codept);
		    return;
		}
	    }
	    else {
		ptr->csid = csid;			/* Csid not set -- set it */
	    }
	}
	else
	    ptr = ptr->next;				/* Advance to next entry */
    }
}

void addRange(char *beg, char *end, int num)
/* Input: Symbolic names of beginning and end of range and */
/*	  corresponding codept				   */
/* Output: None						   */
/* Result: Adds range of entries to table  		   */
{
    int length, numlen, range, i, j;
    char start[NUM_LEN];
    char finish[NUM_LEN];
    char xnum[3];
    char numbuf[NUM_LEN];
    char namebuf[NUM_LEN];
    char sname[NUM_LEN];
    int nname;


    for(i=0; !isdigit(beg[i]); i++);			/* Find numeric portion of name */
    for(j=0; isdigit(beg[i]); i++, j++)			/* for start entry and ...	   */
	start[j] = beg[i];
    start[j] = '\0';
    for(i=0; !isdigit(end[i]); i++);			/* for end entry */
    for(j=0; isdigit(end[i]); i++, j++)
	finish[j] = end[i];
    finish[j] = '\0';

    range = atoi(finish) - atoi(start);			/* Calculate range */

    for(i=0; !isdigit(*beg); i++, beg++)		/* Find alphabetic portion of start */
	sname[i] = *beg;				/* name ... assumes alphabetic portion */
    sname[i] = '\0';					/* of end name is identical */

    numlen = strlen(beg);
    sscanf(beg, "%d", &nname);				/* Scan in corresponding number */

    for(i=0; i <= range; i++) {
	strcpy(namebuf,sname);
	strcat(namebuf, itoa(nname, numlen));		/* Determine name -- concatenation of */
	                                                /* alphabetic and numeric portions */
	addName(namebuf, num);				/* Add single name to table */

	num++;						/* Advance to next codepoint in range */
	nname++;					/* Advance to next character in range */

	if ((num >> 24) >= 0xff) {			/* Check for legal codept value in range */
	    fprintf(stderr, "Error: Codepoint exceeded limit.\n");
	    return;
	}
    }
}

void addRangeID(char *name, char *end, int num)
/* Input: Symbolic name for beginning and end of range and */
/*        corresponding csid (num)			   */
/* Output: None						   */
/* Result: Add csid to table for all entries in range, by  */
/*	   finding the starting codepoint and stepping     */
/*	   through table until the end codepoint is found  */
{
    int i, size;
    int cpt, endcpt;
    Table  *ptr;

    cpt = findCodept(name);			/* Find corresponding codept for start of range */
    endcpt = findCodept(end);			/* Find corresponding codept for end of range */
    
    ptr = table;
    for (size=0; ptr != NULL; size++, ptr = ptr->next);   /* Determine size of table and array */
    for (i=0; i < size-1 && cpt != tableArray[i]->codept; i++);
    ptr = tableArray[i];

    while (endcpt != ptr->codept && i < size-1) {  /* Loop while codept is not equal to end codept */
	if (ptr->csid != -1) {			/* Csid already set */
	    if (ptr->csid != num) {		/* Issue warning if csid different */
		fprintf(stderr, "Warning: CSID already set for codepoint %x\n", ptr->codept);
	    }
	}
	else { 
	    ptr->csid = num;			/* Csid not set -- set it */
	}
	ptr = tableArray[++i];			/* Advance to next sorted entry */
    }

    if (ptr->csid != -1) {		/* Do same for last entry */
	if (ptr->csid != num)	{	/* Issue warning if csid different */
	    fprintf(stderr, "Warning: CSID already set for codepoint %x\n", ptr->codept);
	}
    }
    else { 
	ptr->csid = num;			/* Csid not set -- set it */
    }
}

int findCodept(char *name) 
/* Input: Symbolic name of character    */
/* Output: Codept corresponding to name */
/* Result: Finds codept that corresponds to character name */
{
    List *nameptr;
    Table *ptr;
    int ret;

    ptr = table;

    while (ptr != NULL && ptr->nlist != NULL) {		/* Loop through each entry and its */
	nameptr = ptr->nlist;				/* name list */
	while (nameptr != NULL) {
	    if (strcmp(name, nameptr->name) == 0) {
		ret = ptr->codept;
		return ret;
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
    fprintf(stderr, "Error: Could not find entry for %s.\n", name);
    exit(1);
}

void fillEmptyIDs()
/* Input: None						*/
/* Output: None						*/
/* Result: Loop through entire table, filling in 	*/
/*	   empty csid values with 0, the default value  */
{
    Table *ptr;
    
    ptr = table;			/* Set pointer to beginning of table */

    while (ptr != NULL && ptr->nlist != NULL) {		/* Loop through table */
	if (ptr->csid == -1) {
	    ptr->csid = 0;
	}
	ptr = ptr->next;
    }
}

char *itostr(unsigned long num)
/* Input: Codepoint						*/
/* Output: Pointer to parsed codepoint				*/
/* Result: Transforms numeric codepoint into the standard form, */
/*         i.e. 00a357ef into \xa3\x57\xef			*/
{
    int mask = 0xff000000;
    char ret[NUM_LEN] = "\0";
    char buf[2];
    int x;

    if (num == 0)			/* Return \x00 if codept equals 0 */
	return strcat(ret, "\\x00");
    while ((num & mask) >> 24 == 0)	/* Advance past leading zeros */
	num = num << 8;
    
    while (num) {			/* While num > 0 */
	x = (num & mask) >> 24;		/* Shift first hex byte into lower bits */
	strcat(ret, "\\x");		
	sprintf(buf, "%02x", x);	/* Extract hex number as a string */
	strcat(ret, buf);		/* Construct \x** string */
	num = num << 8;			/* Shift next byte into high bits */
    }
    return ret;
}

char *itoa(int n, int numlen)
/* Input: Integer number and its length 	 */
/* Output: Number represented as an ascii string */
/* Result: Converts integer number into ascii */
{
    int i, j, k;
    char buf[NUM_LEN];
    char *rbuf;

    for(i=0; n > 0; i++, n = n / 10) 	/* Compute ascii digits from number */
	buf[i] = n % 10 + '0';		/* in reverse order */
    buf[i] = '\0';
    j = strlen(buf)+1;
    for(k=0; k < numlen - j; k++, i++)	/* Add leading zeros to end of buffer */
	buf[i]='0';
    buf[i] = '\0';

    rbuf = (char *) malloc (strlen(buf)+2);
    for(i=strlen(buf)-1, j=0; i >= 0; i--, j++)		/* Reverse buffer */
	rbuf[j] = buf[i];
    rbuf[j++] = '>';			/* Add ending '>' to complete name */
    rbuf[j] = '\0';

    return rbuf;
}

int nameInRange(char *prevname, char *nextname)
/* Input: Symbolic names of two consecutive characters */
/* Output: TRUE -- Names are consecutive, i.e. <j0101> and <j0102> */
/*	   FALSE -- Names are not consecutive		           */
/* Result: Determines whether two symbolic names are consecutive   */
{
    int i, from, to;
    char *alphastr1, *alphastr2;

    alphastr1 = (char *) malloc(strlen(prevname));
    alphastr2 = (char *) malloc(strlen(nextname));

    for (i=0; *prevname == '<' || isalpha(*prevname); i++, prevname++) 
	alphastr1[i] = *prevname;		/* Extract alphabetic portion of name1 */
    alphastr1[i] = '\0';

    sscanf(prevname, "%d", &from);		/* Extract numeric portion of name1 */
	
    for (i=0; *nextname == '<' || isalpha(*nextname); i++, nextname++) 
	alphastr2[i] = *nextname;		/* Extract alphabetic portion of name2 */
    alphastr2[i] = '\0';		
    
    sscanf(nextname, "%d", &to);		/* Extract numeric portion of name2 */

    if (strcmp(alphastr1, alphastr2) == 0 && (from+1) == to)  	/* Compare */
	return 1;   	/* TRUE */
    else
	return 0;	/* FALSE */
}

void sortTable(int size)
/* Input: Number of entries in table 			      */
/* Output: None						      */
/* Result: Creates an array of pointers to the table entries  */
/*         and uses the qsort utility to sort the array - the */
/*	   sorting is based on the codepoints		      */
{
   Table *ptr;
   int i;

   tableArray = (Table **) malloc(size * sizeof(Table *));  	/* Create array */
   ptr = table;

   for(i=0; i < size; i++) {	       		/* Copy the table ptrs into the array */
	tableArray[i] = ptr;
	ptr = ptr->next;
   }
   
   qsort (tableArray, size, sizeof(Table *), sort);	/* C utility function */
}

int sort(Table **t1, Table **t2)
/* Input: Two table entries to be compared	     */
/* Output: 1 -- t1 codept greater than t2 codept     */
/*	   -1 -- t1 codept less than t2 codept       */
/*	   0 -- t1 codept equals t2 codept	     */
/* Result: Determines which entry has greater codept */
{
    if ((*t1)->codept > (*t2)->codept)
	return 1;
    if ((*t1)->codept < (*t2)->codept)
	return -1;
    if ((*t1)->codept == (*t2)->codept)
	return 0;
}

void printTable(int size)
/* Input: Number of entries in array		*/
/* Output: Prints table to stdout		*/
/* Result: Reads entire array and prints it out */
/*	   the table entries			*/
{
    int x,y,i;
    List *nameptr;
    Table *start, *end;


    printf("<mb_cur_max>			%d\n", mb_max);
    printf("<mb_cur_min>			%d\n", mb_min);

    printf("\nCHARMAP\n\n");			/* Start with CHARMAP section */

    for (i=0; i < size; i++) {
	start = tableArray[i];			/* Determine range */
	while (i < size-1) {
	    if (nameInRange(tableArray[i]->nlist->name, tableArray[i+1]->nlist->name)   
		   && (tableArray[i]->codept == tableArray[i+1]->codept-1)) {	
		i++;				/* Names and codepts must differ only by one */
	    }
	    else
		break;
	}
	end = tableArray[i];

	nameptr = start->nlist;
	if (start == end) {		/* No range found */
	    for (; nameptr != NULL; nameptr = nameptr->next) {	/* Loop through name list */
		printf("%s", nameptr->name);
		x = strlen(nameptr->name);
		for (y = 0; y < 32-x; y++)		/* Print spaces based on name length */
		    printf(" ");
		printf("%s\n", itostr(start->codept)); 	/* Covert codept before printing out */
		if (PRINT_ONE)				/* If -m option not specified, print only */
		    break;				/* first name in name list */
	    }
	}
	else {						/* Range found */
	    x = strlen(start->nlist->name)+strlen(end->nlist->name)+3;
	    if (x > 30) {				/* Wrap names in range */
		printf("%s...\\\n",start->nlist->name);
		printf("%s", end->nlist->name);
		for (y=0; y < 32-strlen(end->nlist->name); y++)
		    printf(" ");
	    }
	    else {					/* Standard range format */
		printf("%s...%s", start->nlist->name, end->nlist->name);
		for (y=0; y < 32-x; y++)
		    printf(" ");
	    }
	    printf("%s\n", itostr(start->codept));
	}
    }

    printf("\nEND CHARMAP\n\n");

    printf("\nCHARSETID\n\n");				/* Continue with CHARSETID section */


    for (i=0; i < size; i++) {
	start = tableArray[i];

	while (i < size) {	     /* Determine range -- csid */
	    if (tableArray[i]->csid == tableArray[i+1]->csid &&	
		tableArray[i]->codept == tableArray[i+1]->codept-1)
		i++;		/* csids must be the same and codepts can only */
	    else		/* differ by one */
		break;
	}
	end = tableArray[i];

	nameptr = tableArray[i]->nlist;
	if (start == end) {		  		/* No range found */
	    for(; nameptr != NULL; nameptr = nameptr->next) {	/* Loop through name list */
		printf("%s", nameptr->name);
		x = strlen(nameptr->name);
		for (y = 0; y < 32-x; y++)		/* Print spaces based on name length */
		    printf(" ");
		printf("%d\n", start->csid);
		if (PRINT_ONE)				/* Print only first name if -m option */
		    break;				/* specified */
	    }	
	}
	else {						/* Range found */
	    x = strlen(start->nlist->name)+strlen(end->nlist->name)+3;
	    if (x > 30) {				/* Wrap around */
		printf("%s...\\\n",start->nlist->name);
		printf("%s", end->nlist->name);
		for (y=0; y < 32-strlen(end->nlist->name); y++)
		    printf(" ");
	    }
	    else {					/* Standard range format */
		printf("%s...%s", start->nlist->name, end->nlist->name);
		for (y=0; y < 32-x; y++)
		    printf(" ");
	    }
	    printf("%d\n", start->csid);
	}
    }

    printf("\nEND CHARSETID\n\n");
}
    
