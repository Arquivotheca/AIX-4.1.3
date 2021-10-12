static char sccsid[] = "@(#)65   1.14  src/bos/usr/bin/errlg/errupdate/pass1.c, cmderrlg, bos411, 9430C411a 7/27/94 18:05:54";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: pass1
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

/*
 * First pass scan of parsed template description file.
 * The first pass creates a list called Major which contains these
 *  the template entries. This will be the input to the second pass.
 * The first pass calculates the 'crcid' value for the template.
 *  This is a unique value to identify each template in the errtmplt ODM
 *  object class, and is calculated by crc.c from the alert ids in the
 *  template.
 *
 * The first pass finds an error when:
 *  ADD    a template (crcid) that is already in the .desc file
 *  DELETE a template (crcid) that is not     in the .desc file
 *  UPDATE a template (crcid) that is not     in the .desc file
 *
 * The first pass warns an error when:
 *  ADD    a template whose label  is already in the .desc file
 *
 * Note: A 'parsed' template is a linked list of 'symbol' entries,
 *  assembled by parse.y
 */

#include <stdio.h>
#include <errupdate.h>
#include <parse.h>

#define MODE_ADD 0
#define MODE_DEL 1
#define MODE_UPD 2
#define MODE_W   3
#define NMODES   4

/*
 * The md structure forms a linked list of ADD/DELETE/UPDATE commands
 * from the .desc file, and is used for checking duplicate and
 * non-existent entries and for remembering the line numbers for better
 * error reporting.
 *
 * Lookups are done by crcid and by label.
 *
 */
struct md {
	struct md *md_crcnext;
	struct md *md_labnext;
	char      *md_label;
	unsigned   md_crcid;
	int        md_mode;
	int        md_lineno;
};
static struct md *mdtab[NMODES];		/* one per mode */

/*
 * the hash table is for labels
 */
#define NHASH 32
#define HASHIDX(label) (label[0] % NHASH)
static struct md *hashtab[NHASH];

static struct md *getmd();
static struct md *install_crc();
static struct md *lookup_crc();
static struct md *lookup_lab();


static writemajor1_count;
extern FILE *Infp;
extern	int	overrideflg;

/*
 * Scan the .desc file template by template.
 * The call to writemajor1() writes the parsed template to a temp file
 *   to be recalled by readmajor1() in pass1_dupchk() 
 *   to do duplicate checking on the ODM database.
 */
pass1()
{
	int rv;
	int errflg;
	int errflgsv;
	int debugflgsv;

	Pass = 1;
	lexinit();				/* zero-out lex flags (lasttok, etc) */
	for(; !((Infp == stdin) && Errflg);) {
		syminit();			/* clear out symbol table (Major[]) */
		errflgsv = Errflg;
		Errflg = 0;
		rv = yyparse();
		errflg = Errflg;
		Errflg += errflgsv;
		cat_eprint(0,0);	/* flush yyerror messages */
		if(rv == 0)
			break;
		if(errflg == 0 && pass1a() == 0) {
			writemajor1_count++;
			writemajor1();
		}
	}
	if(noodmflg)
		return;
	pass1_dupchk();
}

/*
 * Check for duplicates within the .desc file itself and
 * install into md[] table if OK.
 */
static pass1a()
{
	char *label;
	unsigned crcid;

	switch(Major->s_type) {
	case IPLUS:
		majortoet();
		/*
		 * enforce rules for alertability.
		 */
		 if (T_errtmplt.et_alertflg && !alertable())
			return(-1);

		Major->s_number = ettocrc();
		crcid = Major->s_number;
		label = Major->s_string;
		if(lookup_lab(label)) {
			cat_error(CAT_UPD_PASS1IL,
				"The Label %s is already defined in the input file.\n",label);
			return(-1);
		} else if(*label == 0) {
			cat_lerror(CAT_UPD_YY1,
					"Supply a label after the '+' symbol on an add.\n");
			return(-1);
		}
		if(stanza_dupchk(MODE_ADD,crcid) < 0)
			return(-1); 
		if(headerflg)
			prh();
		install_crc(MODE_ADD,crcid,label);
		break;
	case IMINUS:
		crcid = Major->s_number;
		if(stanza_dupchk(MODE_DEL,crcid) < 0)
			return(-1);
		install_crc(MODE_DEL,crcid,0);
		break;
	case IEQUAL:
		majortoet();
		/*
		 * enforce rules for alertability.
		 */
		 if (T_errtmplt.et_alertflg && !alertable())
			return(-1);

		crcid = Major->s_number;
		if(stanza_dupchk(MODE_UPD,crcid) < 0)
			return(-1);
		install_crc(MODE_UPD,crcid,0);
		break;
	default:
		genexit_nostats(1);
	}
	return(0);
}

/*
 * Utility routines for pass1 to install and lookup crcids and labels
 * for duplicate checking.
 */

static struct md *lookup_crc(mode,crcid)
unsigned crcid;
{
	struct md *mdp;

	mdp = mdtab[mode];
	while(mdp) {
		if(mdp->md_crcid == crcid)
			return(mdp);
		mdp = mdp->md_crcnext;
	}
	return(0);
}

static struct md *lookup_lab(label)
char *label;
{
	struct md *mdp;
	int idx;

	idx = HASHIDX(label);
	mdp = hashtab[idx];
	while(mdp) {
		if(STREQ(mdp->md_label,label))
			return(mdp);
		mdp = mdp->md_labnext;
	}
	return(0);
}

static struct md *install_crc(mode,crcid,label)
unsigned crcid;
char *label;
{
	struct md *mdp;
	int idx;

	mdp = getmd();
	mdp->md_crcnext = mdtab[mode];
	mdtab[mode] = mdp;
	mdp->md_crcid  = crcid;
	mdp->md_lineno = Lineno0;
	if(mode != MODE_ADD)
		return(mdp);
	idx = HASHIDX(label);
	if(label) {
		mdp->md_label = MALLOC(strlen(label)+1,char);
		strcpy(mdp->md_label,label);
	} else {
		mdp->md_label = "";
	}
	mdp->md_labnext = hashtab[idx];
	hashtab[idx] = mdp;
	return(mdp);
}

static struct md *getmd()
{
	struct md *mdp;

	mdp = MALLOC(1,struct md);
	return(mdp);
}

static prh()
{
	symbol *sp;
	char buf[32];

	sprintf(buf,"ERRID_%s",Major->s_string);
	headerpr("#define %-20s 0x%08x ",buf,Major->s_number);
	for(sp = Major->s_next; sp; sp = sp->s_next)
		if(sp->s_type == ICOMMENT)
			break;
	if(sp)
		headerpr("/* %-.40s */",sp->s_string);
	headerpr("\n");
}

static stanza_dupchk(mode,crcid)
unsigned crcid;
{
	struct md *mdp;

	if (mode != MODE_DEL) {
		if(mdp = lookup_crc(MODE_ADD,crcid)) {
			if(!forceflg)
				cat_warn(CAT_UPD_PASS1DUPLW,
"The Error Identifier %08x is already defined in the template file\n\
for label %s.\n",crcid,mdp->md_label);
			if(mode == MODE_ADD && headerflg)
				prh();
			return(-1);
		}
	}
	if (mode != MODE_ADD) {
		if(mdp = lookup_crc(MODE_DEL,crcid))
			return(-1);
	}

	if(mdp = lookup_crc(MODE_UPD,crcid))
		return(-1);
	return(0);
}

/*
 * NAME:
 * pass1_dupchk
 *
 * FUNCTION:
 * Pre-second pass scan of parsed template description file.
 * The input to the second pass is a linked list of pointers to
 *  linked lists of symbol structures. Each of these lists represents
 *  an entry in the template description file.
 * The input is free of syntax errors and duplicate values.
 *
 * This pass checks for these error conditions:
 *  ADD    a template (crcid) that is already in the ODM
 *  DELETE a template (crcid) that is not     in the ODM
 *  UPDATE a template (crcid) that is not     in the ODM
 *
 * It creates a list of work to do for the second pass, which will do
 *  the actual modification of the ODM.
 */

static pass1_dupchk()
{

	readmajor1_init();
	while(readmajor1())
		pass1_dupchka();
}

/*
 * The call to writemajor2() will output 'work' in the form of the 'Major'
 * linked list.
 */
static pass1_dupchka()
{
	char *label;
	unsigned crcid;

	crcid = Major->s_number;
	label = Major->s_string;
	switch(Major->s_type) {
	case IPLUS:
		if(udb_iscrcid(crcid)) {
			Major->s_type = IUPDATE;
			if(!forceflg) {
				cat_error(CAT_UPD_PASS1DUPC,
				  "The Error Identifier %08x for the template %s is\n\
already defined in the template file.\n.", crcid, label);	
				break;
			} else
				undopr_tmplt(crcid,IPLUS);
		} else if(udb_islabel(label)) {
			if(!forceflg) {
				cat_error(CAT_UPD_PASS1DUPL,
					"The Label %s is already defined in template file.\n",
									label);
				break;
			} else
				undopr_tmplt(crcid,IPLUS);
		} else
			undopr_tmplt(crcid,IMINUS);

		writemajor2();
		break;
	case IMINUS:
		if(!udb_iscrcid(crcid))
			break;
		undopr_tmplt(crcid,IPLUS);
		writemajor2();
		break;
	case IEQUAL:
		if(!udb_iscrcid(crcid)) {
			cat_error(CAT_UPD_PASS1UPD,
				"The Error Identifier %08x is not defined in the template file.\n", crcid);
			break;
		}
		undopr_tmplt(crcid,IEQUAL);
		writemajor2();
		break;
	default:
		genexit_nostats(1);
	}
}

extern char *symtokstr();
#define tf(v)         ( (v) ? "TRUE" : "FALSE" )
#define encodestr(c) \
( \
(c) == 'A' ? "ALPHA" : \
(c) == 'H' ? "HEX" : \
(c) == 'D' ? "DEC" : \
"?" \
)

/*
 * look up the current contents of the template and print to the undo file
 * in errids.desc stanza format.
 */
static undopr_tmplt(crcid,mode)
{
	int i;

	if(quietflg)
		return;
	if(mode == IMINUS) {
		undopr("- %08x:\n\n",crcid);
		return;
	}
	if(udb_tmpltget(crcid) <= 0)
		return;
	switch(mode) {
	case IEQUAL:
		undopr("\
= %08x:\n\
	%s = %s\n\
	%s = %s\n\
	%s = %s\n\n",
			crcid,
			symtokstr(IREPORT),tf(T_errtmplt.et_reportflg),
			symtokstr(ILOG),   tf(T_errtmplt.et_logflg),
			symtokstr(IALERT), tf(T_errtmplt.et_alertflg));
		return;
	case IPLUS:
	case IUPDATE:
		undopr("+ %s:\n",T_errtmplt.et_label);
		undopr("\
	%s = %s\n\
	%s = %s\n\
	%s = %s\n",
			symtokstr(IREPORT),tf(T_errtmplt.et_reportflg),
			symtokstr(ILOG),   tf(T_errtmplt.et_logflg),
			symtokstr(IALERT), tf(T_errtmplt.et_alertflg));
		undopr("	%s = %s\n",symtokstr(IERRTYPE), T_errtmplt.et_type);
		undopr("	%s = %s\n",symtokstr(IERRCLASS),T_errtmplt.et_class);
		undopr("	%s = %04x\n",symtokstr(IERRDESC), T_errtmplt.et_desc);
		ipr4(symtokstr(IPROBCAUS),T_errtmplt.et_probcauses);
		ipr4(symtokstr(IUSERCAUS),T_errtmplt.et_usercauses);
		ipr4(symtokstr(IINSTCAUS),T_errtmplt.et_instcauses);
		ipr4(symtokstr(IFAILCAUS),T_errtmplt.et_failcauses);
		ipr4(symtokstr(IUSERACTN),T_errtmplt.et_useraction);
		ipr4(symtokstr(IINSTACTN),T_errtmplt.et_instaction);
		ipr4(symtokstr(IFAILACTN),T_errtmplt.et_failaction);
		for(i = 0; i < 8; i++) {
			if(T_errtmplt.et_detail_encode[i] == 0)
				break;
			undopr("	%s = %d, %04X, %s\n",
				symtokstr(IDETAILDT),
				T_errtmplt.et_detail_length[i],
				T_errtmplt.et_detail_descid[i],
				encodestr(T_errtmplt.et_detail_encode[i]));
		}
		undopr("\n");
		return;
	}
	genexit_nostats(1);
}

/*
 * Print undo template in the form:
 *   Prob_causes = 0122, 3344
 */
static ipr4(str,v)
char *str;
unsigned short *v;
{
	char *comma;
	int i;

	undopr("	%s = ",str);
	comma = "";
	for(i = 0; i < 4; i++) {
		undopr("%s%04x",comma,v[i]);
		comma = ", ";
		if(v[i] == 0xFFFF || v[i] == 0x0000)
			break;
	}
	undopr("\n");
}
