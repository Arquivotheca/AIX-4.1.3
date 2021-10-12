 



/* *****************************************************************

                              CSET_CONV.H                              

   ***************************************************************** */

/* =========================== CONSTANTS =========================== */

#define CHARNAME_SIZE 64
#define CHARNUM_SIZE 16
#define TABLE_SIZE 512     /* number of table entries */
#define LINE_LEN 120       /* for reading a line from input file */

/* ========================= DATA_STRUCTURE ======================== */

typedef struct table_entry {
  char name[CHARNAME_SIZE];
  char num_bytes; 
  char num[CHARNUM_SIZE];  
  char setid;
} TABLE;

/* ========================== PROTOTYPES =========================== */

int extend_table (TABLE **t, int *table_size);
int is_subcharval (const TABLE entry);
int linear_search (TABLE **table, int pos, char *key);
int nameinrange (char *prevname, char *nextname);
int numinrange (TABLE prev, TABLE next);
int sort_function (const TABLE *e1, const TABLE *e2);
int main (int argc, char *argv[]);
void create_table (TABLE **table, int table_size, int *pos);
void destroy_table (TABLE **t);
void make_table (FILE *fp, TABLE **t, int *pos);
void get_num (char *line, TABLE **t, int *pos);
void get_str (char *line, int *il, char *str, char terminator);
void insert_entry (TABLE **table, int *pos, char *line, int *il);
void insert_setid (TABLE **table, int *pos, char *line, int *il, 
                   FILE *fi1);
void itoa (int i, char *str, size_t numlen);
void print_collate (TABLE **t, int *pos);
void process_charmap (FILE *fi1, TABLE **t, int *pos); 

/* ================================================================= */















