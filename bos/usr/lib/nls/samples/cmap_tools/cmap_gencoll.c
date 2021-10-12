 


/* ********************************************************************

                               CHARMAP_CONV.C 

   ******************************************************************** */

/* ========================== INCLUDE_FILES =========================== */

#include <ctype.h>
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmap_gencoll.h"

/* #define DEBUG  */

/* ====================== GLOBAL_DATA_STRUCTURES ====================== */

TABLE *table1;   /* table for file 1 */
int pos1;        /* index for table 1 */
int t1_size;    /* size of table 1 */
TABLE subchar;           /* for storing substitute character */

/* ============================= MAIN () ============================== */

/*

FUNCTION: Takes a charmap file and outputs a binary LC_COLLATE definition

INPUT   : Input file format:
 _____________________________________________________________________________
|                                                                             |
|#                       -- lines of comments, empty lines, or                |
|<                       -- any strings in <>                                 |
|                                                                             |
|CHARMAP                 -- marks the beginning of character value for        |
|                        -- the table                                         |
|                                                                             |
|#                       -- lines of comments, or empty lines                 |
|                                                                             |
|<*>   \x?               -- insert the string in <>, and up to                |
|                        -- 16 hex numbers with \x format in the table        |
|                                                                             |
|<*>...<*>    \x?        -- example of another format                         |
|                                                                             |
|END CHARMAP             -- marks the end of character value for the table    |
|                                                                             |
|CHARSETID               -- marks the beginning of character setid for        |
|                        -- the table                                         |
|                                                                             |
|<*>...<*>        ?      -- range of character name in <> with setid          |
|                        -- value=?                                           |
|                                                                             |
|<*>...\                 -- example of another format                         |
|<*>              ?                                                           |
|                                                                             |
|<*>              ?      -- example of another format                         |
|                                                                             |
|END CHARSETID           -- marks the end of character setid for the table    |
|_____________________________________________________________________________|

OUTPUT  : Output to stdout format:
 _____________________________________________________________________________
|LC_COLLATE                                                                   | 
|                                                                             |
|order_start                                                                  |
|                                                                             |
|<*>                     -- <> contains the character name                    |
|                                                                             |
|<*>                                                                          |
|...                     -- example of another format                         |
|<*>          ?                                                               |
|                                                                             |
|UNDEFINED                                                                    |
|                                                                             |
|order_end                                                                    |
|                                                                             |
|END LC_COLLATE                                                               |
|_____________________________________________________________________________|

*/

int main (int argc, char *argv[]) {

  int i=0,
      num_bytes=0;

  unsigned int xnum;

  char *p;

  t1_size = TABLE_SIZE;
  /* takes care of <FromCode> file processing, and fill data in table1 */
  process_charmap (stdin, &table1, &pos1);

  /* qsort ( (void *)table1, t1_size, sizeof(TABLE), sort_function); */

  /* without printing character range with setid=0xff to stdout */
  print_collate (&table1, &pos1); 

  destroy_table (&table1);

  exit(0);

}

/* ======================== PROCESS_CHARMAP () ======================== */

/*

FUNCTION : Processes the file to find "CHARMAP" which signifies the beginning 
           of character value for table.

ALGORITHM:
  loop {
    reads a line from the input file;
    if the word "CHARMAP" is found then
      call make_table to fill the table;
      exit function;
  } 

*/

void process_charmap (FILE *fi1, TABLE **t, int *pos) { 

  #define S_SIZE 20          /* for storing the marker CHARMAP */

  char str[S_SIZE], 
       linebuf[LINE_LEN]; 

  int i,                     /* index for linebuf */
      j;                     /* index for str */


  for (;;) {
    
    i=0; 
    j=0;
    fgets (linebuf, LINE_LEN, fi1);    

    if (linebuf[i]=='C') {   
     
      /* get the string which begins with 'C', and store in 'str' variable */ 
      str[j++]=linebuf[i];
      while ( linebuf[i] != '\n' && linebuf[i] != '\t' ) {
        i++;          
        if (linebuf[i] != '\n' && linebuf[i] != '\t')
          str[j++]=linebuf[i]; 
      }
      str[j]='\0'; 

      if (strcmp(str, "CHARMAP")==0) {
        make_table (fi1, t, pos);
        return;
      }
    }

  } 
  
}

/* ========================== MAKE_TABLE () =========================== */

/* 

FUNCTION:  Creates a table in RAM with the following format:
 __________________________________________
|              |              |            |
|  character   |   # of hex   |  array of  |
|    name      |    values    | hex values |
|______________|______________|____________|
|     ...      |     ...      |    ...     |

ALGORITHM:
  call create_table to allocate memory space for the table entries;
  loop {
    reads a line from the input file;
    switch (the first character of the line) {
      case '<': call insert_entry to fill a table entry;
      case 'E': if the string is "END CHARMAP" then
                  exit function;
    }  
  } 

*/

void make_table (FILE *fp, TABLE **t, int *pos) {

  #define STR_LEN 40      /* for storing the marker END CHARMAP */

  char str[STR_LEN], 
       linebuf[LINE_LEN];

  int i,                  /* index for linebuf */
      j;                  /* index for str */


  create_table (t, TABLE_SIZE, pos);
  
  for (;;) {

    i=0; 
    j=0;
    fgets (linebuf, LINE_LEN, fp);    

    switch (linebuf[i]) {    
       
      case '<':
        /* insert an entry in the table */
        ++i;
        insert_entry (t, pos, linebuf, &i);
        break;       

      case 'E': 
        /* get the string which begins with 'E', 
           and store in 'str' variable */
        str[j++]=linebuf[i]; 
        while ( linebuf[i] != '\n' && linebuf[i] != '\t' ) {
          i++;          
          if (linebuf[i] != '\n' && linebuf[i] != '\t') 
            str[j++]=linebuf[i]; 
        }
        str[j]='\0'; 

        if (strcmp(str, "END CHARMAP")==0) 
          return;
        break;

      default:;
 
    }

  } 

}

/* =========================== GET_STR () ============================= */

/*

FUNCTION : Get a string indexed by il for line.

ALGORITHM:
  fill str with characters indexed by il for line until terminator is found;

*/

void get_str (char *line, int *il, char *str, char terminator) {

  register int i=0;


  while (line[*il] != terminator) {
    str[i++]=line[*il]; 
    (*il)++;      
  }
  str[i]='\0';
  
}

/* ========================= CREATE_TABLE () ========================== */

/*

FUNCTION : Allocates memory space for table entries.

ALGORITHM:
  malloc for table;
  initialize the index for table to 0;

*/

void create_table (TABLE **table, int table_size, int *pos) {


  *table = (TABLE *)  malloc ( table_size * sizeof(TABLE) ); 
  *pos = 0;

}

/* ========================= INSERT_ENTRY () ========================== */

/*

FUNCTION : Insert entry/entries in the table.
 
ALGORITHM:
  checks if the table is large enough to hold the next entry;
  if not then
    call extend_table to reallocate more memory space for the table;
    if unable to reallocate requested memory space then
      print error message, and exit program;

  get the character name, and put the string in the table;

  if a range of character names is found (ie for <*1>...<*2> format) then
    put "*1" in str1;
    get "*2" in str2;
    get the array of hex numbers, and put them in the table;
    save the alphabets of 'str1' in temp 
      (ie. for <jsup0110>, temp="jsup");
    put numeric value of "*1" in 'from' variable; 
    put numeric value of "*2" in 'to' variable;
    copy the range of character strings (ie from "*1" to "*2"), with the 
      array of hex values, and number of hex values in the table;
  else it is a single character name  
    store the array of hex values, and the number of hex values in the table;

*/

void insert_entry (TABLE **table, int *pos, char *line, int *il) {

  /* str1 and str2 are used if a range of character names is found 
     (ie <*1>...<*2>) */
  char *str1="                            ";
  char *str2="                            "; 

  /* temporary stores the string of alphabets if there is a range of 
     character strings (ie. for <jsup0101>, temp1="jsup") */
  char temp1[CHARNAME_SIZE];  
     
  /* temporary stores the numeric portion if there is a range of character 
     strings (ie. for <jsup0101>, temp2="0101") */                         
  char temp2[CHARNAME_SIZE];  
                                 
  unsigned int i=0, 
               j,
               from, 
               to;

  size_t digitlen;


  /* checks if the table is large enough to hold the next entry,
     if not call extend_table to allocate a bigger table */
  if ( *pos == t1_size ) 
    if ( !extend_table(table, &t1_size) ) {
      fprintf (stderr, "Error:  Out of memory\n");
      exit (1);
    }
    
  get_str (line, il, (*table)[*pos].name, '>');   
  (*il)++;

  /* if there is a range of character names with similar hex values
     (ie. <jsup010>...<jsup0194>) */
  if (line[*il] == '.') {

    /* gets the first character name, the last character name, and the
       hex values */
    strcpy ( str1, (*table)[*pos].name ); 
    while (line[*il] != '<') 
      (*il)++;
    (*il)++; 
    get_str (line, il, str2, '>');        
    get_num (line, table, pos);

    /* copies the string of alphabets in temp, and stores the numeric 
       value of the string in 'from' variable for 'str1' */
    while (!isdigit(*str1)) {
      if (isalpha(*str1)) 
        temp1[i++] = *str1;
      str1 += 1;    
    }
    temp1[i] = '\0';
    digitlen = strlen (str1);
    sscanf (str1, "%d", &from);

    /* stores the numeric value of str2 in 'to' variable */
    while (!isdigit(*str2)) 
      str2 += 1;    
    sscanf (str2, "%d", &to);

    /* fill the table with the range of character names and hex values */
    for (i = from; i <= to; i++) {

      /* i!=from because the 1st entry is already in the table*/
      if (i != from) {

        /* put character name in table after converting integer portion of the
           string to a string */
        strcpy ((*table)[*pos].name, temp1);
        itoa (i, temp2, digitlen);
        strcat ((*table)[*pos].name, temp2);

        (*table)[ *pos ].num_bytes = (*table)[ *pos - 1 ].num_bytes;

        /* copy the array of hex values from the previous table entry except 
           for the last hex value */
        for (j=0; j < ( (*table)[*pos].num_bytes -1 ); j++)
          (*table)[*pos].num[j] = (*table)[*pos - 1].num[j];
 
        /* increment the value of the last hex number by 1 */        
        if ( (int)(*table)[*pos].num[ (*table)[*pos].num_bytes - 1 ] > 0xff ) {
          fprintf (stderr, "Error:  %s exceeded the value of \\xff\n", 
                   (*table)[*pos].name );
          exit (1);
        }
        else {
          (*table)[*pos].num[ (*table)[*pos].num_bytes -1] = 
             (*table)[*pos - 1].num[ (*table)[*pos].num_bytes -1];
          (*table)[*pos].num[ (*table)[*pos].num_bytes -1]++;
	}

     } 

     (*pos)++;    
     if ( *pos == t1_size ) 
       if ( !extend_table(table, &t1_size) ) {
         fprintf (stderr, "Error:  Out of memory\n");
         exit (1);
       } 

     }

  }

  /* if there is no range of character names with the same hex values */
  else { 
    get_num (line, table, pos);
    (*pos)++;    
  }
 
}

/* ========================= EXTEND_TABLE () ========================== */

/*

FUNCTION : Assign t pointer to a larger piece of memory if the table is full.

ALGORITHM:
  if able to allocate a piece of memory which is twice the original size then
    return O.K.;
  else 
    return not O.K.;

*/

int extend_table (TABLE **t, int *table_size) {


  if ( (*t = (TABLE *) realloc (*t, sizeof(TABLE)*(*table_size) * 2)) != NULL)
  { 
    *table_size = *table_size * 2;
    return 1;
  }
  else 
    return 0;

}

/* ============================= ITOA () ============================== */

/*

FUNCTION : Converts an integer ranging from 0 to 9999 into a string.

ALGORITHM:
  do (remainder by 10) and (divide by 10) until the number is <= 0, 
    and save the value of the integer in 'str' variable;
  if the string is less than numlen (ie. 4) then
    pad the beginning of the string with 0's;

*/

void itoa (int i, char *str, size_t numlen) {

  int num=i,
      k;

  size_t j=numlen-1; 

  
  while (num > 0) {     
    switch (num % 10) {
      case 0: str[j--] = '0';
              break;
      case 1: str[j--] = '1';
              break;
      case 2: str[j--] = '2';
              break;
      case 3: str[j--] = '3';
              break;
      case 4: str[j--] = '4';
              break;
      case 5: str[j--] = '5';
              break;
      case 6: str[j--] = '6';
              break;
      case 7: str[j--] = '7';
              break;
      case 8: str[j--] = '8';    
              break;
      case 9: str[j--] = '9';
              break;
      default: ;
    }
    num = num / 10;
  }

  /* pad with 0's */
  for (k=j; k >= 0; k--) 
    str[k]='0';
  str[numlen]='\0';

}

/* ============================ GET_NUM () ============================ */

/*

FUNCTION : Puts the array of hex values, and the number of hex values in
           the table.

ALGORITHM:
  put the string of hex values in p;
  do {
    put the 1st hex value typecast as char in the array of hex values 
      in the table;    
    if there is another hex value then
      advance p pointer to the next hex value;
    else 
      advance p pointer to '\n';
    increase the number of hex values;
  } until p contains '\n';
  save the number of hex values in the table typecast as char;

*/

void get_num (char *line, TABLE **t, int *pos) {

  int i=0, 
      numbytes=0;

  unsigned int xnum; 

  char *p,
       *match="\\x";


  p = strstr(line, match); 
  p += 2;   /* advances past \x */

  do {  
    sscanf (p, "%x", &xnum);
    (*t)[*pos].num[i++] = (char) xnum;    

    if (strlen(p) > 3) p += 4;
    else p += 2;      

    numbytes++; 
  } while (p != "\n"  && *p != '\n');

  (*t)[*pos].num_bytes = (char)numbytes;

}

/* ========================== NAMEINRANGE () ========================== */

/*
FUNCTION : Returns true if prevname, and nextname differ by an increment 
           of 1 (ie. prevname=<jsup0101>, nextname=<jsup0102> returns true);
           Else return false.

ALGORITHM:
  parse prevname to get the non-numeric string, and the numeric string;
  parse nextname to get the non-numeric string, and the numeric string;
  if prevname has the same non-numeric string as nextname, and the value
  of prevname's numeric string is one less than the value of nextname's  
  numeric string then
    return yes;
  else return no;    (ie. if prevname=<jsup0101>, and nextname=<jsup0102> then
                            return yes;
                          else return no;

*/

int nameinrange (char *prevname, char *nextname) {

  /* stores a copy of prevname and nextname */
  char *str1="                         ";
  char *str2="                         ";

  /* temporary stores the string of alphabets if there is a range of
     character strings (ie. for <jsup0101>, alphastr1="jsup") */
  char alphastr1[CHARNAME_SIZE];
  char alphastr2[CHARNAME_SIZE];


  int i,
      from,
      to;
 
  
  strcpy (str1, prevname);
  strcpy (str2, nextname);

  /* copies non-numeric string of str1 to alphastr1, and stores numeric 
     value of str1 in 'from' variable for 'str1' variable */   
  i=0;
  while (!isdigit(*str1)) {
    if (isalpha(*str1)) 
      alphastr1[i++] = *str1;
     str1 += 1;    
  }
  alphastr1[i] = '\0';
  sscanf (str1, "%d", &from);
 
  /* do the same thing for str2 into alphastr2, and to variables */
  i=0;
  while (!isdigit(*str2)) {
    if (isalpha(*str2)) 
      alphastr2[i++] = *str2;
     str2 += 1;    
  }
  alphastr2[i] = '\0';
  sscanf (str2, "%d", &to);

  if ( strcmp(alphastr1, alphastr2) == 0 && (from+1) == to ) 
    return 1;
  else 
    return 0;

}

/* =========================== NUMINRANGE () ========================== */

/*
FUNCTION : Returns true if the character hex values in prev and next differ
           by an increment of 1 (ie. prev.num=\x45\x66, next.num=\x45\x67
           returns true);
           Else return false.

ALGORITHM:
  if the hex values in prev and next have the same number of bytes then  
    if the number of hex values is not 1 then
      copy all the hex values in prev except the last digit to xnotbyte1;
      copy all the hex values in next except the last digit to ynotbyte1;
      if the value of xnotbyte1 and ynotbyte1 is the same, and the last
        digit of hex values in prev and next differ by an increment of 1 then
        return true;
    else 
      if the only hex value of prev and next differ by an increment of 1 then
        return true;
  for all other combinations 
      return false;

*/

int numinrange (TABLE prev, TABLE next) {

  char xnotbyte1[CHARNUM_SIZE];
  char ynotbyte1[CHARNUM_SIZE];

  int j;  


  if ( prev.num_bytes == next.num_bytes ) {
    if ( prev.num_bytes != 1 ) {

      for (j=0; j < (prev.num_bytes -1); j++) {
        xnotbyte1[j] = prev.num[j];
        ynotbyte1[j] = next.num[j];        
      }
      xnotbyte1[j] = ynotbyte1[j] = '\0';

      if ( strcmp (xnotbyte1, ynotbyte1) == 0 ) {
        if (prev.num[prev.num_bytes -1] +1 == next.num[next.num_bytes -1]) {
          return 1;
        }
      }
    }

    else { /* if (prev.num_bytes == 1) */
      if (prev.num[0] +1 == next.num[0]) 
        return 1;      
    }
  }

  return 0;

}

/* ========================= IS_SUBCHARVAL () ========================= */

/*

FUNCTION : Returns true if 'entry' has substitute character hex values.
           Returns false otherwise.
       
ALGORITHM:
  if the number of hex bytes and each byte of 'entry' is not equivalent to the
  substitute character's then
    return 0;
  else return 1;

*/

int is_subcharval (const TABLE entry) {

  int i;

  if (subchar.num_bytes != entry.num_bytes) 
    return 0;
  else  
    for (i=0; i < subchar.num_bytes; i++)
      if (entry.num[i] != subchar.num[i])
        return 0;

  return 1;

}

/* ======================== PRINT_COLLATE () ========================== */

/*

FUNCTION : Output setid field of the table to stdout.

OUTPUT   : The two formats:
               ex1:  <*>
               ex2:  <*>\n...\n<*>

ALGORITHM:
  do {
    if the character hex values is more than a byte then
      copy all the hex values except the last hex value to notbyte1;
    get the range of character names with if all hex values 
      are constant except for the last byte; 
    if there is only one character name with the setid then    
      output with the format of ex1;
    else if there is a range of values then
      output with the format of ex2;  
  } until there is no more table entry;

*/

void print_collate (TABLE **t, int *pos) {

  #define NAMELEN 40

  register int num_bytes,
               to = 0,
               from;

  char notbyte1[CHARNUM_SIZE];


  printf ("\nLC_COLLATE\n\n");
  printf ("order_start\n\n");

  do {
    
    from = to;
    to = from + 1;
    num_bytes = (*t)[from].num_bytes;
    while ((to < *pos) &&
	   ((*t)[to].num_bytes == num_bytes) &&
	   ((*t)[to].num[num_bytes - 1] == 
	     ((*t)[to - 1].num[num_bytes - 1]) + 1) &&
	   (!(memcmp((*t)[to].num, (*t)[from].num, (*t)[to].num_bytes - 1)))) {
      to++;
    } /* while */
    to--;

    /* output this format:  <*> */
    if ( from == to ) {
      printf ("<%s>\n", (*t)[from].name);
    }    
    /* there is a range of character values */
    else {      
      /* output this format:  <*>\n...\n<*> */
      printf ("<%s>\n...\n<%s>\n", (*t)[from].name, (*t)[to].name);
    }

    to++;

  } while (to < *pos);

  printf ("\nUNDEFINDED\n\n");
  printf ("order_end\n\n");
  printf ("END LC_COLLATE\n\n");

}


/* ========================= DESTROY_TABLE () =========================== */

/*

FUNCTION : Free memory space pointed to by t.

*/

void destroy_table (TABLE **t) {


  free (*t);

}

/* ========================= LINEAR_SEARCH () =========================== */

/*

FUNCTION : Returns the index of the table where key is found.  If unable
           to find the key, linear search returns with -1.

*/

int linear_search (TABLE **table, int pos, char *key) {

  int i;

  for (i=0; i<pos; i++) 
    if (strcmp ((*table)[i].name, key) == 0)
      return i;

  return -1;

}

/* ========================= SORT_FUNCTION () =========================== */

/*

FUNCTION:  Used by qsort function in create_conv_table function.

*/

int sort_function (const TABLE *e1, const TABLE *e2) {

  /* for sorting in ascending order by character names */
  /* return (strcmp (e1->name, e2->name)); */
  
  /* for sorting in ascending order by hex values */
  /* return (strcmp (e1->num, e2->num)); */
  if ((e1->num_bytes) < (e2->num_bytes)) {
    return -1;
  }
  if ((e1->num_bytes) > (e2->num_bytes)) {
    return 1;
  }

  return memcmp(e1->num, e2->num, e1->num_bytes);

}

/* ==================================================================== */








