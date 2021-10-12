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
 
#ifndef True
#define True 1
#endif
#ifndef False
#define False 0
#endif

#define T_CHARMAP 0
#define T_CHARSETID 1

#define LINE_LEN 200
#define NUM_LEN 50

#define UNASSIGNED 0xffff     /* UniChar mark */

/*
 * Structures
 */
typedef struct __SymbolRec {
    struct __SymbolRec* next;
    char* name;
    unsigned char csid;
} Symbol;

typedef struct __CharsetRec {
    struct __CharsetRec* next;
    Symbol* nlist;
    UniChar codept;
} Charmap;


void main(int argc, char **argv);
Charmap* initEntry();
void loadCharmap(Charmap* cm, FILE *file, char type);
void fillCharmap(Charmap* cm, FILE *file, char type);
void insertEntry(Charmap* cm, char *buf, FILE *file, char type);
void addName(Charmap* cm, char *name, UniChar ucs);
void addRange(Charmap* cm, char *beg, char *end, int num);
void addCsid(Charmap* cm, char *name, int csid);
Charmap* findName(Charmap* cm, char *name);
Charmap* findCode(Charmap* cm, UniChar cdpt);
char *itoa(int n, int numlen);
size_t conv (iconv_t cd, uchar_t* buf, const uchar_t* in, size_t len);
void printCM(Charmap* cm);
