/* @(#)10	1.10  src/bos/usr/bin/adb/mode.h, cmdadb, bos411, 9428A410j 6/15/90 20:06:10 */
#ifndef  _H_MMODE
#define  _H_MMODE
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

typedef int             BOOL;
typedef char            *STRING;
typedef char            *ADBMSG;
typedef struct map      MAP;
typedef MAP             *MAPPTR;
typedef struct syment   SYMTAB;
typedef SYMTAB          *SYMPTR;
typedef struct symslave SYMSLAVE;
typedef struct bkpt     BKPT;
typedef BKPT            *BKPTR;

typedef enum {NOSTACK, NEWSTACK, ENDSTACK, BADSTACK} STACKSTATUS;

/* file address maps */
struct map {
    unsigned long b1;
    unsigned long e1;
    unsigned long f1;
    unsigned long b2;
    unsigned long e2;
    unsigned long f2;
    int      ufd;
};

/* slave table for symbols */
struct symslave {
    long valslave;
    int  typslave;
};

#define MAXCOM 64
struct bkpt {
    long loc;
    int  ins;
    int  count;
    int  initcnt;
    int  flag;
    char comm[MAXCOM];
    BKPT *nxtbkpt;
};

typedef struct reglist REGLIST;
typedef REGLIST        *REGPTR;
struct reglist {
    STRING rname;
    int    roffs;
};
/* ldinfo and fdinfo together reflect the memory and file usage of a process. */
struct ldinfo {
    unsigned int ldinfo_next;	/* Offset of next entry from here or 0 if end */
    int ldinfo_fd;		/* File descriptor returned by loader */
    unsigned int textorg;	/* Text origin */
    unsigned int textsize;	/* Text size */
    unsigned int dataorg;	/* Data origin */
    unsigned int datasize;	/* Data size */
};

struct fdinfo {
    char *pathname;		/* Pathname */
    char *membername;		/* Membername */
};

struct ldinfo *loader_info;
struct fdinfo *fd_info;
struct ld_info {
    unsigned int ldinfo_next;	/* Offset of next entry from here or 0 if end */
    int ldinfo_fd;		/* File descriptor returned by loader */
    unsigned int *ldinfo_textorg;	/* Text origin */
    unsigned int ldinfo_textsize;	/* Text size */
    unsigned int *ldinfo_dataorg;	/* Data origin */
    unsigned int ldinfo_datasize;	/* Data size */
    char	 filename[2];
};
typedef struct {
	unsigned int  begin;
	unsigned int  end;
	unsigned int  seekaddr;
} Map;

static Map datamap, *txtmap;
MAP stkmap;

typedef struct FileMaps {
	unsigned long begin_txt;  /* Beginning of text section */
	unsigned long end_text;   /* end of text section */
	unsigned long txt_seekadr;  /* Seek address of the first byte in the
				       text section */
	unsigned long begin_data;  /* Address of beginning of data section */
	unsigned long end_data;    /* Address of the end of the data section */
	unsigned long data_seekadr;/* Seek address of the first byte in the
				       data section */
	unsigned long st_seekadr;  /* Seek address of the first byte of the
				      symbol table */
	long	 fd;		   /* File descriptor */
	unsigned long entrypoint;  
	unsigned long txt_scnptr;  /* Begining of the text scn */
} FILEMAP;

typedef struct symbol_info {
	SYMSLAVE    *symvec;
	SYMPTR      symnxt, symend;
	char 	    *strtab, *dbgtab;
	long        symnum ;
	int         data_scn_num, bss_scn_num, text_scn_num, dbg_scn_num;
	unsigned long datsize, txtsize;
	long        symcnt;
}  SYMINFO;

#endif  /* _H_MMODE */
