/* @(#)35	1.2 4/3/94 05:14:23 */
/*
 *
 * COMPONENT_NAME: (CMDPIOSK)
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#define	MBTRANSPATH	"/usr/lib/lpd/pio/transJP/"
#define	XNLSPATH	"/usr/lpp/X11/lib/X11/nls/"
#define	PETCPATH	"/usr/lib/lpd/pio/etc/"

#define	MBTRANSDIR	"trans.dir"
#define MBCSALIAS       "codeset.alias"
#define MBNLSDIR        "nls.dir"
#define MAXLEN	  256

#define MAXCSNUM  10

#define	READLN(fp) { \
	int ch; \
	while ((ch = getc (fp)) != '\n'); \
		if (ch == EOF) break;}

#define	READLINE(fp, arg) { \
	int ch; \
	while ((ch = getc (fp)) != '\n' && ch != EOF); \
		if (ch == EOF) break;}


#define SKIPBL(fp) { \
	int	ch; \
	while ((ch = getc (fp)) != ' ' && ch != '\t'); \
	if (feof(fp)) break; \
	ungetc(ch, fp);}

typedef struct _Alias_list {
	char * aliasname;
	struct _Alias_list * next;
} Alias_list;

typedef struct _Sp_cs_alias{
    char        * realname;
    Alias_list  *alias_l;
    struct _Sp_cs_alias *next;
} Sp_cs_alias;

typedef struct _Trans_dir {
	char	*f_sp_cd;
	char	*t_sp_cd;
	char	*tr_tbl;
	struct _Trans_dir *next;
} Trans_dir;

typedef struct _Trans_data {
    unsigned int reserv1;
    unsigned int f_cp;
    unsigned int reserv2;
    unsigned int t_cp;
} Trans_data;

typedef struct _In_to_proc {
    char * proc_cd;
    struct in_cd_list {
	char * in_cd;
	struct in_cd_list * next;
    } *in_cd_l;
    struct _In_to_proc * next;
} In_to_proc;

typedef struct _Proc_to_out {
    char * proc_cd;
    struct out_cd_list {
	char * out_cd;
	struct out_cd_list * next;
    } *out_cd_l;
    struct _Proc_to_out * next;
} Proc_to_out;

typedef struct _Csid_font {
    struct reg_list *reg_l;
    struct _Csid_font *next;
} Csid_font;

typedef struct _Nls_dir {
    char *localename;
    char *codesetname;
    struct _Nls_dir *next;
} Nls_dir;

#define NONE  (0x00)
#define INCD  (0x01)
#define PRCD  (0x02)
#define OUTCD (0x04)
#define GET   (0x08)
#define END   (0x40)
#define INVAL (0x80)

