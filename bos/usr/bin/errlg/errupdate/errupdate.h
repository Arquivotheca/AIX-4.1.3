/* "@(#)05	1.11  src/bos/usr/bin/errlg/errupdate/errupdate.h, cmderrlg, bos411, 9428A410j 3/3/93 09:14:34" */

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: header file for errupdate
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <cmderrlg_msg.h>
#include <errlg.h>

extern char *Infile;
extern char ybuf[];
extern Pass;
extern Lineno;
extern Lineno0;
extern Errflg;
extern forceflg;
extern headerflg;
extern checkflg;
extern noodmflg;
extern promptflg;
extern linitflg;
extern hexflg;
extern quietflg;
extern detailflg;

extern char *stracpy();
extern char *symtokstr();
extern char *tokstr();

extern unsigned ettocrc();

extern instanza;

extern Addcount;
extern Deletecount;
extern Updatecount;

struct symbol {
	struct symbol *s_next;
	int   s_type;			/* token IFORMAT, ... */
	char *s_name;
	int   s_number;
	union {
		struct symbol *_s_symp;
		char *_s_string;
	} _s_u;
};
#define s_symp   _s_u._s_symp
#define s_string _s_u._s_string

typedef struct symbol symbol;
extern symbol *reslookup();
extern symbol *resinstall();
extern symbol *getsym();
extern symbol *lookup();
extern symbol *install();

extern symbol *ymajor();
extern symbol *yminors();
extern symbol *yminor();
extern symbol *ynumlist();
extern symbol *ydetaillist();

extern symbol Major[];

extern char Tempfile[32];
extern char Labelfile[32];
extern char Addfile[32];
extern char Updatefile[32];
extern char Deletefile[32];

#define NDETAILS 10

/*
 * default values
 */
#define ERR_LOGFLG     TRUE
#define ERR_ALERTFLG   FALSE
#define ERR_REPORTFLG  TRUE

/*
 * Unique codes for internal encoding.
 * The actual values do not have any meaning.
 */
#define ERRCLASS_HARDWARE 1
#define ERRCLASS_SOFTWARE 2
#define ERRCLASS_OPERATOR 3
#define ERRCLASS_UNKNOWN  4

#define ERRTYPE_PERM 0x01
#define ERRTYPE_TEMP 0x02
#define ERRTYPE_PERF 0x03
#define ERRTYPE_PEND 0x11
#define ERRTYPE_UNKN 0x12
#define ERRTYPE_INFO 0x13


