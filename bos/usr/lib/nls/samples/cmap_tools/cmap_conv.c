 


/* ********************************************************************

                               CHARMAP_CONV.C 

   ******************************************************************** */

/* ========================== INCLUDE_FILES =========================== */

#include <ctype.h>
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmap_conv.h"

/* #define DEBUG  */

/* ====================== GLOBAL_DATA_STRUCTURES ====================== */

TABLE *table1,   /* table for file 1 */
      *table2;   /* table for file 2 */
int pos1,        /* index for table 1 */
    pos2;        /* index for table 2 */ 
int t1_size;    /* size of table 1 */
int t2_size;    /* size of table 2 */
TABLE subchar;           /* for storing substitute character */

/* ============================= MAIN () ============================== */

/*

FUNCTION: Takes a charmap file and outputs another charmap file, after 
          converting it from one codeset to another codeset.

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
|CHARMAP                                                                      | 
|                                                                             |
|<*>   \x?               -- insert the string in <>, and up to                |
|                        -- 16 hex numbers with \x format in the table        |
|                                                                             |
|<*>...<*>     \x?       -- example of another format if there is a range     |
|                        -- of character strings                              |
|                                                                             |
|END CHARMAP                                                                  |
|                                                                             |
|CHARSETID                                                                    |
|                                                                             |
|<*>   ?                 -- <> contains the character name, and ? is the      |
|                        -- setid value                                       |
|                                                                             |
|<*>...\                 -- example of another format                         |
|<*>          ?                                                               |
|                                                                             |
|<*>...<*>    ?          -- example of another format                         |
|                                                                             |
|END CHARSETID                                                                |
|_____________________________________________________________________________|

*/

int main (int argc, char *argv[]) {

  int i=0,
      num_bytes=0;

  unsigned int xnum;

  char *p;


  /* get substitute character from command-line */
  switch (argc) {

    case 3:
      /* assign the default subchar which is "SUB"=\x1a */
      strcpy (subchar.name, "SUB");
      subchar.num[0] = (char) 0x1a;
      subchar.num_bytes = (char) 1;
      break;

    case 4:
      /* get character hex value from command-line */
      p = argv[1];
      do {
        p = strstr (p, "x");
        p += 1;
        sscanf (p, "%x", &xnum);
        subchar.num[i++] = (char) xnum;
        num_bytes++;
      } while (strlen(p) > 2);      
      subchar.num_bytes = (char) num_bytes;
      strcpy (subchar.name, "");
      break;

    case 5:
      /* get character hex value and name from command-line */
      p = argv[1];
      do {
        p = strstr (p, "x");
        p += 1;
        sscanf (p, "%x", &xnum);
        subchar.num[i++] = (char) xnum;
        num_bytes++;
      } while (strlen(p) > 2);      
      subchar.num_bytes = (char) num_bytes;
      strcpy (subchar.name, argv[2]);
      break;

    default:
      fprintf (stderr, "Error:  The format is     ");
      fprintf (stderr, "cset_conv  [<subvalue>  [subname]]  ");
      fprintf (stderr, "<FromCode>  <ToCode>\n");
      exit (1);
  }

  t1_size = TABLE_SIZE;
  /* takes care of <FromCode> file processing, and fill data in table1 */
  process_charmap (stdin, &table1, &pos1);
  process_charsetid (stdin, &table1, &pos1);    

  t2_size = TABLE_SIZE; 
  /* creates table2, and fill it with the converted data of table1 */
  create_conv_table (&table1, &pos1, &table2, &pos2, 
                     argv[argc -2], argv[argc -1]);
  print_table (&table2, &pos2); 


  #ifndef DEBUG
  /* without printing character range with setid=0xff to stdout */
  print_charsetid (&table2, &pos2); 
  #else
  /* prints character range with setid=0xff to stdout */
  db_print_charsetid (&table2, &pos2);
  #endif
 

  destroy_table (&table1);
  destroy_table (&table2);

  exit(0);

}

/* ======================== PROCESS_CHARMAP () ======================== */

/*

7FUNCTION : Processes the file to find "CHARMAP" which signifies the beginning 
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

/* ======================= PROCESS_CHARSETID () ======================== */

/*

FUNCTION : Processes the file to find "CHARSETID" which signifies the beginning 
           of character setid for table.

ALGORITHM:
  loop {
    reads a line from the input file;
    if the word "CHARSETID" is found then
      initialize all setid in the table to \xff; 
      call add_setid to add setid in the table;
      exit function;
  } 

*/

void process_charsetid (FILE *fi1, TABLE **t, int *pos) { 

  #define S_SIZE 20          /* for storing the marker CHARSETID */

  char str[S_SIZE], 
       linebuf[LINE_LEN]; 

  int i,                     /* index for linebuf */
      j,                     /* index for str */
      k;                     


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

      if (strcmp(str, "CHARSETID")==0) {

        /* initialize all setid to \xff */      
        for (k=0; k < *pos; k++) 
          (*t)[k].setid = (char) 0xff;

        add_setid (fi1, t, pos); 
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

/* =========================== ADD_SETID () =========================== */

/* 

FUNCTION:  Parses the input file to get setid until "END CHARSETID" is 
           found.

ALGORITHM:
  loop {
    reads a line from the input file;
    switch (the first character of the line) {
      case '<': call insert_setid to fill a table entry;
      case 'E': if the string is "END CHARSETID" then
                  exit function;
    }  
  } 

*/

void add_setid (FILE *fp, TABLE **t, int *pos) {

  char str[STR_LEN], 
       linebuf[LINE_LEN];

  int i,                  /* index for linebuf */
      j;                  /* index for str */


  for (;;) {

    i=0; 
    j=0;
    fgets (linebuf, LINE_LEN, fp);    

    switch (linebuf[i]) {    
       
      case '<':
        /* insert an entry in the table */
        ++i;
        insert_setid (t, pos, linebuf, &i, fp); 
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

        if (strcmp(str, "END CHARSETID")==0) 
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

/* ========================= INSERT_SETID () ========================== */

/*

FUNCTION : Inserts the charsetid for each setid field of TABLE.

INPUT    : The three format found in the charmap file for the charsetid
           are:
                 ex1:  <*>...<*>     ?
                 ex2:  <*>...\
                       <*>           ?
                 ex3:  <*>           ?

ALGORITHM:
  get the first string in <>;
  find the index of the table where the first string is located;
  if there is a range then
    get the second string in <> where the range ends;
    find the index of the table where the second string is located;
    get the setid of the string/strings;
    put the setid of the string/strings in the table; 
  else if there is no range then
    get the setid of the string;
    put the setid of the string in the table;

*/

void insert_setid (TABLE **table, int *pos, char *line, int *il, FILE *fi1) {

  /* str1 and str2 store the range of the character names */
  char *str1="                            ";
  char *str2="                            "; 
  
  unsigned int i,
               setid;

  signed int to,
             from;


  /* gets the first string */
  get_str (line, il, str1, '>');
  (*il)++;

  /* finds the index of the table where the first string can be located */ 
  if ( (from = linear_search (table, *pos, str1)) == -1 ) {
    fprintf (stderr, "Error:  Cannot find %s in CHARSETID", str1);  
    exit (1);
  }

  /* if there is a range of character names */
  if (line[*il] == '.') {
    while (line[*il] != '<' && line[*il] != '\\')
      (*il)++;

    switch (line[*il]) { 

      /* if the format is <*>...\
                          <*>        ? */                           
      case '\\':
        fgets (line, LINE_LEN, fi1);
        *il = 1;
        get_str (line, il, str2, '>');
        break; 

      /* if the format is <*>...<*>  ? */
      case '<':
        (*il)++; 
        get_str (line, il, str2, '>');        
        break;

      default:;

    }

    /* finds the index of the table where the second string is located */
    if ( (to = linear_search (table, *pos, str2)) == -1 ) {
      fprintf (stderr, "Error:  Cannot find %s in CHARSETID", str2);  
      exit (1);
    }   

    /* gets the setid */
    while ( !isdigit (line[*il]) )
      (*il)++;
    line += *il;
    sscanf (line, "%d", &setid);

    /* put the setid in the table for the range of character names */
    for (i=from; i<=to; i++)
      (*table)[i].setid = (char) setid;

  }
  /* if the format is <*>    ? */ 
  else {
    while ( !isdigit (line[*il]) )
      (*il)++;
    line += *il;
    sscanf (line, "%d", &setid);
    (*table)[from].setid = (char) setid; 
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

/* ========================== PRINT_TABLE () ========================== */

/*

FUNCTION : Output all the entries of the converted table to stdout.

OUTPUT   : Output to stdout with the following format:
 ________________________________________________________________________
|                                                                        |
|<*>   \x?               -- insert the string in <>, and up to           |
|                        -- 16 hex numbers with \x format in the table   |
|                                                                        |
|<*>...<*>     ?         -- another output format if there is a range    |
|________________________________________________________________________|

ALGORITHM:
  do {
    get the index and the content of the current entry in the table;
    while the character name and the character value of the current index
    and the next index of the table differ by an increment of 1 do 
      increment the index of the table;
    get the index and the content of the current entry when the condition
    of the while loop is false;    
    output to the screen this format (<*>    ?) if there is no range; 
    output to the screen this format (<*>...<*>    ?) if there is a range;
  } while it is not the last entry of the table;

*/

void print_table (TABLE **t, int *pos) {

  #define NAME_LEN 40          

  TABLE efrom, eto;   /* to save the contents of the range of table entries */   

  register int i=0,              
               j,
               k,
               ifrom,          
               ito,
               maxlen;


  printf ("CHARMAP\n\n");

  do {
     
    ifrom = i;
    efrom = (*t)[i];

    while ( nameinrange ((*t)[i].name, (*t)[i+1].name) &&
            numinrange ((*t)[i], (*t)[i+1]) ) 
      i++; 

    ito = i;
    eto = (*t)[i];

    if (ifrom == ito) {
     
    #ifndef DEBUG  

    /* does not print characters with substitute character value to stdout */
      if (!(is_subcharval (efrom) && (strcmp (efrom.name, subchar.name) != 0))) { 
        printf ("<%s>", efrom.name);
      
        /* to output spaces between the character name and the 
           character value */
        for (k=strlen (efrom.name); k < NAME_LEN; k++)
          putchar (' ');

        for (j=0; j < efrom.num_bytes; j++) 
          printf ("\\x%02x", (int) efrom.num[j]); 

        /* printf ("      %d", (int) efrom.setid); */     /* db */
        putchar ('\n');
      }
      else {} 

    #else

    /* prints character's with num[0]=0x1a to stdout */
      printf ("<%s>", efrom.name);
      
      /* to output spaces between the character name and the 
         character value */
      for (k=strlen (efrom.name); k < NAME_LEN; k++)
        putchar (' ');

      for (j=0; j < efrom.num_bytes; j++) 
        printf ("\\x%02x", (int) efrom.num[j]); 

      printf ("      %d", (int) efrom.setid); 
      putchar ('\n');

    #endif

    }
    else {
      printf ("<%s>...<%s>", efrom.name, eto.name);
      maxlen = strlen(efrom.name) + strlen (eto.name);
             
      /* to output spaces between the character name and the 
         character value */
      for (k=maxlen + 5; k < NAME_LEN; k++)
        putchar (' ');

      for (j=0; j < efrom.num_bytes; j++) 
        printf ("\\x%02x", (int) efrom.num[j]); 
      putchar ('\n');
    }
     
    i++;

  } while (i < *pos);
 
  printf ("\nEND CHARMAP\n\n");

}

/* ======================== PRINT_CHARSETID () ========================== */

/*

FUNCTION : Output setid field of the table to stdout.

OUTPUT   : The three formats of charsetid are:
               ex1:  <*>...<*>    ?
               ex2:  <*>...\
                     <*>          ?
               ex3:  <*>          ?

ALGORITHM:
  do {
    get setid from the table if it is not 0xff, or the character's
      hex value is not the same as the substitute character's
      hex value;
    if the character hex values is more than a byte then
      copy all the hex values except the last hex value to notbyte1;
    get the range of character names with the same setid, or if 
      setid=0xff, or if the character's hex value is the same as
      the substitute character's hex value, and if all hex values 
      are constant except for the last byte; 
    if there is only one character name with the setid then    
      output with the format of ex1;
    else if there is a range of values then
      output with the format of ex1 or ex2;  
  } until there is no more table entry;

*/

void print_charsetid (TABLE **t, int *pos) {

  #define NAMELEN 40

  register int i=0, 
               j,
               k,
               to,
               from,
               setid,
               maxlen,
               newcharsetid;

  char notbyte1[CHARNUM_SIZE];


  printf ("\nCHARSETID\n\n");

  do {
    
    /* get the first character value's setid from the table 
       which is not 0xff, and which character's hex value is not 
       the substitute character's hex value */
    setid = (*t)[i].setid;
    while ( (setid == 0xff || is_subcharval ((*t)[i])) && i != *pos ) 
      if (strcmp ((*t)[i].name, subchar.name) == 0) {
        if (setid != 0xff) 
          break;
        else { 
	  setid = (*t)[++i].setid;
	}
      }
      else setid = (*t)[++i].setid;

    /* If the character value has more than 1 hex value, copy the 
       hex values except the last in notbyte1 variable */                   
    if ((*t)[i].num_bytes != 1)
      for (j=0; j < ((*t)[i].num_bytes - 1); j++) 
        notbyte1[j] = (*t)[i].num[j];

    if ( i != *pos ) {

      from = i;

      /* 'to' variable gets the index of the table which has the 
         equivalent setid as the 'setid' variable, or if setid is 
         0xff, or if the character's hex value is not the substitute
         character's hex value */
      if ((*t)[i].num_bytes == 1) {
        while ( ( (*t)[i].setid ==setid || (*t)[i].setid ==0xff
                || is_subcharval((*t)[i]) ) && i != *pos ) {
          if ( (*t)[i].setid != 0xff && !is_subcharval((*t)[i])) 
            to = i;
          i++;
        } 
      }

      /* this module has similar function as above with the addition 
         of checking 'notbyte1' variable with all the hex values index
         by 'to' variable to see they are equivalent  */   
      else {  /*  if ((*t)[i].num_bytes != 1)  */
        newcharsetid = 0;     
        while ( ( (*t)[i].setid == setid || (*t)[i].setid ==0xff 
                || is_subcharval((*t)[i]) ) && i != *pos ) {     

          if ( (*t)[i].setid != 0xff && !is_subcharval((*t)[i])) {
            /* checks all other hex values of a character value beside 
               the last hex value to make sure they are constant */
            for (j=0; j < ((*t)[i].num_bytes -1); j++)
              if (notbyte1[j] != (*t)[i].num[j]) {
                newcharsetid = 1;
                break;
              }
            if (!newcharsetid) 
              to = i;
          }
 
          if (newcharsetid) break;

          i++;

        } /* while */
      } /* else */

      /* output this format:  <*>     ? */
      if ( from == to ) {
        printf ("<%s>", (*t)[from].name);

        /* to output spaces between the character name and setid */ 
        for (k=strlen ((*t)[from].name); k < NAMELEN; k++)
          putchar (' ');
      }    

      /* there is a range of character values */
      else {      

        /* maxlen is the length of str1 + str2 */
        maxlen = strlen((*t)[from].name) + strlen((*t)[to].name); 

        /* output this format:  <*>...<*>    ? */
        if ( maxlen <= (NAMELEN - 10)) {
          printf ("<%s>...<%s>", (*t)[from].name, (*t)[to].name);

          for (k=maxlen + 5; k < NAMELEN; k++)
            putchar (' ');
        }

        /* output this format:  <*>...\
                                <*>          ? */      
        else {
          printf ("<%s>...\\\n", (*t)[from].name);
          printf ("<%s>", (*t)[to].name);

          for (k=strlen ((*t)[to].name); k < NAMELEN; k++)
            putchar (' ');
        }
      }

      if ((int) (*t)[from].setid == 0xff) 
	  printf("0\n");
      else 
	  printf ("%d\n", (int)(*t)[from].setid);
    }

  } while (i < *pos);

  printf ("\nEND CHARSETID\n");

}

/* ====================== DB_PRINT_CHARSETID () ========================= */

/*

FUNCTION : Output the range of characters with the same setid to stdout
           including those with setid=0xff and num values=0x1a.

OUTPUT   : The three formats of charsetid are:
               ex1:  <*>...<*>    ?
               ex2:  <*>...\
                     <*>          ?
               ex3:  <*>          ?

ALGORITHM:
  do {
    get setid from the table; 
    if the character hex values is more than a byte then
      copy all the hex values except the last hex value to notbyte1;
    get the range of character names with the same setid, and that has
      the same hex values (except the last hex value) as notbyte1;
    if there is only one character name with the setid then    
      output with the format of ex1;
    else if there is a range of values then
      output with the format of ex1 or ex2;  
  } until there is no more table entry;

*/

void db_print_charsetid (TABLE **t, int *pos) {

  #define NAMELEN 40

  register int i=0, 
               j,
               k,
               to,
               from,
               setid, 
               maxlen,
               newcharsetid;

  char notbyte1[CHARNUM_SIZE];


  printf ("\nCHARSETID\n\n");

  do {
    
    setid = (*t)[i].setid;
    /* If the character value has more than 1 hex value, copy the 
       hex values except the last byte in notbyte1 variable */    
    if ((*t)[i].num_bytes != 1)
      for (j=0; j < ((*t)[i].num_bytes - 1); j++) 
        notbyte1[j] = (*t)[i].num[j];

    if ( i != *pos ) {

      from = i;

      /* 'to' variable gets the index of the table which has the 
         equivalent setid as the 'setid' variable */
      if ((*t)[i].num_bytes == 1) {
        while ( (*t)[i].setid ==setid && i != *pos ) 
          i++;
        to = i-1;
      }

      /* this module has similar function as above with the addition 
         of checking 'notbyte1' variable with all the hex values 
         in the table to see they are equivalent  */   
      else {  /*  if ((*t)[i].num_bytes != 1)  */
        newcharsetid = 0;     
        while ( (*t)[i].setid == setid && i != *pos ) {     

          /* checks all other hex values of a character value beside 
             the last hex value to see that they are constant */
          for (j=0; j < ((*t)[i].num_bytes -1); j++)
            if (notbyte1[j] != (*t)[i].num[j]) {
              newcharsetid = 1;
              break;
            }

          if (!newcharsetid) {
            to = i;
            i++;
          }
          else 
            /* if it is the only one with equivalent notbyte1 */
            if (i==from)
              to = i;
            /* if it has a range of values with the same notbyte1 */
            else  
              to = i-1;

          if (newcharsetid) break;
        } /* while */
      } /* else */

      /* output this format:  <*>     ? */
      if ( from == to ) {
        printf ("<%s>", (*t)[from].name);

        /* to output spaces between the character name and setid */ 
        for (k=strlen ((*t)[from].name); k < NAMELEN; k++)
          putchar (' ');
      }    

      /* there is a range of character values */
      else {      

        /* maxlen is the length of str1 + str2 */
        maxlen = strlen((*t)[from].name) + strlen((*t)[to].name); 

        /* output this format:  <*>...<*>    ? */
        if ( maxlen <= (NAMELEN - 10)) {
          printf ("<%s>...<%s>", (*t)[from].name, (*t)[to].name);

          for (k=maxlen + 5; k < NAMELEN; k++)
            putchar (' ');
        }

        /* output this format:  <*>...\
                                <*>          ? */      
        else {
          printf ("<%s>...\\\n", (*t)[from].name);
          printf ("<%s>", (*t)[to].name);

          for (k=strlen ((*t)[to].name); k < NAMELEN; k++)
            putchar (' ');
        }
      }
  
      if ((int) (*t)[from].setid == 0xff) 
	  printf("0\n");
      else 
	  printf ("%x\n", (int)(*t)[from].setid);

    }    

  } while (i < *pos);

  printf ("\nEND CHARSETID\n");

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

/* ======================= CREATE_CONV_TABLE () ========================= */

/*

FUNCTION : Creates a table, and fill it with converted code values of the 
           first table.
      
ALGORITHM:
  create an empty table;
  call iconv_open to initialize a code set converter;    
  for all the table1 entries {
    create a bigger table if there are not enough memory space to hold 
      more table entries;
    if the code is converted successfully then
      insert the value in the new table;
    else
      insert substitute character value in the new table;   
  }
  call iconv_close to close the code set converter;
  call q_sort to sort the table in ascending order of the character's 
    hex value;

*/

void create_conv_table (TABLE **t1, int *i1, TABLE **t2, int *i2,
                        char *conv_f1, char *conv_f2) {

  int i, 
      found;

  char ibuf[CHARNUM_SIZE], 
       obuf[CHARNUM_SIZE];

  char *ip, 
       *op; 

  size_t ileft,
         oleft;

  iconv_t cd;

  int same_code = FALSE;


  create_table (t2, TABLE_SIZE, i2);

  /* If no conversion... (Just want to make ranges) */
  if (!strcmp(conv_f1, conv_f2)) {
    fprintf(stderr, "same_code = TURE\n");
    same_code = TRUE;
  }
  else if ((cd = iconv_open (conv_f2, conv_f1)) == -1) {
    perror("iconv_open");
    exit(1);
  }

  for (i=0; i<*i1; i++) {         
   
    if ( i == t2_size ) 
      if ( !extend_table(t2, &t2_size) ) {
        fprintf (stderr, "Error:  Out of memory\n");
        exit (1);
      } 

    if (same_code) {
      (*t2)[i].num_bytes = (int) (*t1)[i].num_bytes;
      memcpy ( (*t2)[i].num, (*t1)[i].num, (*t2)[i].num_bytes);
    }

    else {

      ileft = (int) (*t1)[i].num_bytes;
      memcpy (ibuf, (*t1)[i].num, ileft);
      oleft = CHARNUM_SIZE;

      ip = ibuf;
      op = obuf;

      strcpy ( (*t2)[i].name, (*t1)[i].name);

      if ( iconv (cd, &ip, &ileft, &op, &oleft) == 0 ) {     
	(*t2)[i].num_bytes = (char)(CHARNUM_SIZE - oleft);      
	memcpy ( (*t2)[i].num, obuf, (*t2)[i].num_bytes);
      }
      else { 
	(*t2)[i].num_bytes = subchar.num_bytes;
	memcpy ((*t2)[i].num, subchar.num, (*t2)[i].num_bytes);
      }
    }
    strcpy ((*t2)[i].name, (*t1)[i].name);
    (*t2)[i].setid = (*t1)[i].setid;

  }
  *i2 = i;

  if (!same_code) {
    iconv_close (cd);  
  }

  qsort ( (void *)*t2, *i2, sizeof(TABLE), sort_function);

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








