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
 
/* cmap_merge.h -- Header file for cmap_merge.c */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


#define LINE_LEN 200
#define NUM_LEN 50

typedef struct list_entry List;
typedef struct merged_table Table;

int mb_max = INT_MIN;		/* Current multi-byte maximum (mb_cur_max) */
int mb_min = INT_MAX;		/* Current multi-byte minimum (mb_cur_min) */
int FIRST_FILE = 1;		/* Flag to determine whether file being processed is the */
                                /* first one */
int PRINT_ONE = 1;		/* Flag to determine whether the -m option was used */


/* Structure definitions */

struct list_entry {		/* List structure for symbolic names associated with a */
    char *name;			/* particular code point */
    struct list_entry *next;
};

struct merged_table {		/* Table entry structure -- one per codepoint */
    List *nlist;		/* Mutliple names possible */
    unsigned long codept;	/* Unique code point  -- maximum of FOUR bytes */
    int csid;			/* Codeset ID */
    struct merged_table *next;
};


/* Function declarations */

void main(int argc, char* argv[]);
Table *initEntry();
void destroyTable(Table *table);
void processFile(char *file);
void scanFile(FILE *file);
void fillTable(FILE *file);
void fillID(FILE *file);
void insertEntry(char *buf, FILE *file, int ID);
void addName(char *name, int num);
void addID(int cpt, int csid);
void addRange(char *name, char *end,  int num);
void addRangeID(char *name, char *end, int num);
int findCodept(char *cpt);
void fillEmptyIDs();
char *itostr(unsigned long num);
char *itoa(int n, int numlen);
int nameInRange(char *prevname, char *nextname);
void sortTable();
int sort(Table **t1, Table **t2);
void printTable(int size);
