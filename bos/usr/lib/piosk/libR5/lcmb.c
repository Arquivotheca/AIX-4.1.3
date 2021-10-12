static char sccsid[] = "@(#)34  1.6  src/bos/usr/lib/piosk/libR5/lcmb.c, cmdpiosk, bos411, 9428A410j 5/18/94 05:26:45";
/*
 *
 * COMPONENT_NAME: (CMDPIOSK)
 *
 * FUNCTIONS: set_loc_and_cs, init_pre_conv, init_post_conv
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include        "lc.h"
#include        "lcmb.h"
#include        "jfmtrs_msg.h"
#include        <stdio.h>
#include        <sys/stat.h>
#include        <fcntl.h>
#include        <string.h>
#include        <iconv.h>
#include        <langinfo.h>

int	tr_dir_num = 0;
Sp_cs_alias* sp_cs_alias = NULL;
char msgbuf[MAXLEN];

char    *sp_alias_to_name( char * );
int	open_post_iconvs (CURcodeset);

extern int udc  = 0;
extern int sudc = 0;

Trans_dir *init_mb_tr_tbl()
{
    char path[256];
    char arg[3][MAXLEN];
    FILE *fp;
    static Trans_dir *tr_dir = NULL, *cur_tr_dir;
    int ch, i, t = 0, tnum = 0, n, col;

    if (tr_dir)
	return (tr_dir);

    DEBUGPR((stderr, "Start init_mb_tr_tbl\n"));

    strcpy(path, MBTRANSPATH);
    strcat(path, MBTRANSDIR);

    /* read trans.dir and codeset.alias */
    if ((fp = fopen (path, "r")) == NULL){
	DEBUGPR((stderr, "can't open %s\n", path)); 
	return(NULL);
    }
    
    while (!feof (fp)){
	for (n = 0; n < 3; n++){
	    for (;(ch = getc (fp)) == ' ' || ch == '\t';);
	    ungetc (ch, fp);
	    for (col = 0; col < MAXLEN && (ch = getc (fp)) != EOF && 
		 ch != '\n' && ch != ' ' && ch != '\t' && ch != '#'; col++){
		arg[n][col] = (char) ch;
	    }
	    if (col == 0) break;
	    arg[n][col] = NULL;
	    if ( ch == '#'){
		n++;
		break;
	    }
	    if (n != 2 && (ch == '\n' || ch == EOF))
		break;
	}
	if (ch != '\n' && ch != EOF)
	    READLN(fp);
	if (n == 3){
	    if (tr_dir == NULL)
		tr_dir = cur_tr_dir = (Trans_dir *) 
		    malloc(sizeof(Trans_dir));
	    else 
		cur_tr_dir = cur_tr_dir->next = (Trans_dir *) 
		   malloc(sizeof(Trans_dir));

	    if((cur_tr_dir == NULL) ||
	       (cur_tr_dir->f_sp_cd = 
		(char *)malloc(strlen(arg[0])+1)) == NULL ||
	       (cur_tr_dir->t_sp_cd =
		(char *)malloc(strlen(arg[1])+1)) == NULL ||
	       (cur_tr_dir->tr_tbl = 
		(char *)malloc(strlen(arg[2])+1)) == NULL)
		errorexit(MSG_MEMALLOCERR, "");

	    strcpy(cur_tr_dir->f_sp_cd, arg[0]);
	    strcpy(cur_tr_dir->t_sp_cd, arg[1]);
	    strcpy(cur_tr_dir->tr_tbl, arg[2]);
	    cur_tr_dir->next = NULL;
	}
    }
    fclose(fp);
    
    return (tr_dir);
}

int	init_alias()
{
    char path[256];
    char arg[MAXLEN], buff[MAXLEN], abuff[MAXLEN], 
         al[MAXLEN], cs[MAXLEN], *str;

    static FILE *fp = -1;
    Sp_cs_alias *cur_sp_cs_alias; 
    Alias_list *cur_alias_l; 
    int i, t = 0, ch, len;
    
    strcpy(path, MBTRANSPATH);
    strcat(path, MBCSALIAS);
    if (fp == -1){
	if ((fp = fopen (path, "r")) == NULL){
	    DEBUGPR((stderr, "Cannot open file %s\n", path));
	    return(0);
	}
    }
    else if(fp == NULL){
	return(0);
    }

    while (!feof(fp)){
	for(str = arg; (ch = getc( fp )) != EOF && ch != '\n'; )
	    *str++ = (char)ch;
	*str = NULL;			
	str = arg;
	if(sscanf(str, "%s%s", cs, buff) == 2){
	    if(arg && cs && buff && cs[0] != '#' && buff[0] != '#'){
		if (!sp_cs_alias)
		    sp_cs_alias = cur_sp_cs_alias = 
			(Sp_cs_alias *) malloc(sizeof(Sp_cs_alias));
		else 
		    cur_sp_cs_alias = cur_sp_cs_alias->next =
			(Sp_cs_alias *) malloc(sizeof(Sp_cs_alias));
		if (!cur_sp_cs_alias){
		    errorexit(MSG_MEMALLOCERR, "");
		}
		cur_sp_cs_alias->next = NULL;
		len = strlen(cs);
		cur_sp_cs_alias->realname = (char *)malloc(len + 1);
		strcpy(cur_sp_cs_alias->realname, cs);
		cur_alias_l = NULL;

		str = strchr(str, buff[0]);
		while (sscanf(str, "%s", al) != EOF){
		    if(al && al[0] != '#'){
			if (!cur_alias_l)
			    cur_sp_cs_alias->alias_l = cur_alias_l =
				(Alias_list *) malloc(sizeof(Alias_list));
			else 
			    cur_alias_l = cur_alias_l->next =
				(Alias_list *) malloc(sizeof(Alias_list));
			if (!cur_alias_l){
			    errorexit(MSG_MEMALLOCERR, "");
			}
			len = strlen(al);
			cur_alias_l->aliasname = (char *)malloc(len + 1);
			strcpy(cur_alias_l->aliasname, al);
			cur_alias_l->next = NULL;
			str = strchr(str, al[0]) + len;
		    }
		}
	    }
	}
    }
    fclose(fp);
}


char* sp_alias_to_name( char* csname )
{
    Sp_cs_alias *x;
    Alias_list  *y;

    /* 
     * open alias file 
     */
    if (sp_cs_alias == NULL){
	init_alias ();
    }

    /*
     * check alias and find real codeset name
     */
    for( x = sp_cs_alias; x; x = x->next ){
	if( strcmp( x->realname, csname ) == 0 ){
	    return( x->realname );
	}
	for( y = x->alias_l; y; y = y->next){
	    if( strcmp( y->aliasname, csname ) == 0 ){
		return( x->realname );
	    }
	}
    }
    return( csname );
}

Trans_data * read_mb_tr_tbl(Trans_dir *tr_dir, int *num)
{
    Trans_data *trans_dat;

    int trfd;		/* translation file file descripter	*/
    int tr_dat_num, i;
    struct stat statbuf;
    char path[MAXLEN];
    struct tr_hdr{
	char hdrtext[16];	/* "PIOSMBCSXLATExxx"		*/
	unsigned long hdr_size;	/* header size (32 byte)	*/
	unsigned long cp_size;	/* code point size (4 byte)	*/
	unsigned long rsrv[2];	
    }trhdr;

    /* open a mbcs translation file */
    
    strcpy(path, MBTRANSPATH);
    strcat(path, tr_dir->tr_tbl);
    
    DEBUGPR((stderr, "%s \n", path));

    if (stat(path, &statbuf) != 0 || 
	(trfd = open (path, O_RDONLY, 0)) < 0){
	errorexit(MSG_BADTRASFILE, path);
    }

    /* read in the file header */
    if ((tr_dat_num = read(trfd, &trhdr, sizeof(trhdr))) < 0  ||
	strncmp(trhdr.hdrtext, "PIOSMBCSXLATE", 13) ||
	!trhdr.cp_size  || trhdr.cp_size > 8 ||
	trhdr.hdr_size != sizeof(trhdr)){
	errorexit(MSG_BADFORMATF, path);
    }
    DEBUGPR((stderr, "header size = %d\n", tr_dat_num));
	
    /* malloc and read in the translation talbe */
    *num = tr_dat_num = (statbuf.st_size - trhdr.hdr_size)/sizeof(Trans_data);
    DEBUGPR((stderr, "hole size = %d, header size = %d, data number = %d\n",
	     statbuf.st_size, trhdr.hdr_size, tr_dat_num));

    if (tr_dat_num){
	if ((trans_dat = (Trans_data *) 
	     malloc(statbuf.st_size - trhdr.hdr_size)) == NULL) {
	    errorexit(MSG_MEMALLOCERR, "");
	}
	if((tr_dat_num = read(trfd, &(trans_dat[0].reserv1),
			      statbuf.st_size - trhdr.hdr_size)) <= 0){
	    errorexit(MSG_BADFORMATF, path);
	}
    }
    close(trfd);
    
    return(trans_dat);
}

#define REG 13
#define ENC 14

Csid_font *read_lc_font_info(char *proccodeset, int * csnum)
{
    char reg[MAXLEN], enc[MAXLEN], path[256], *arg;
    FILE *fp;
    Csid_font *csid_font, *cur_csid_font;
    struct reg_list  *cur_reg_l;
    int i, t = 0, tnum = 0, n, col, flag, len, ch, stat;

    csid_font = cur_csid_font = NULL;
    strcpy(path, XNLSPATH);
    strcat(path, proccodeset);

    if ((fp = fopen (path, "r")) == NULL){
	return(0);
    }
    flag = 0;
    ch = 0;

    n = 0;
    while(!feof(fp)){
	if (cur_csid_font == NULL || cur_csid_font->reg_l != NULL){
	    if (csid_font == NULL)
		csid_font = cur_csid_font = 
		    (Csid_font *)malloc(sizeof(Csid_font));
	    else
		cur_csid_font = cur_csid_font->next = 
		    (Csid_font *)malloc(sizeof(Csid_font));

	    if (cur_csid_font == NULL)
		errorexit(MSG_MEMALLOCERR, "");

	    tnum++;
	    cur_csid_font->next = NULL;
	    cur_csid_font->reg_l = NULL;
	}

	for (;(ch = getc(fp)) == ' ' || ch == '\t' || ch == ',' || ch == '\n';);
	if (ch == '#'){
	    READLN(fp);
	    continue;
	}
	ungetc(ch, fp);
	
	while(ch != '\n' && ch != EOF){
	    for (col = 0, arg = reg, stat = REG;
		 col < MAXLEN && (ch = getc(fp)) != EOF && ch != '\n' && 
		 ch != ' ' && ch != '\t' && ch != '#' && ch != ','; col++){
		if (stat == REG && ch == '-'){
		    if (col == 0){
			READLN(fp);
			break;
		    }
		    *arg = NULL;
		    arg = enc;
		    stat = ENC;
		}
		else{
		    *arg++ = (char)ch;
		}
	    }
	    *arg = NULL;	    
	    if (stat != ENC || !reg[0] || !enc[0])
		continue;
	    
	    if (cur_csid_font->reg_l == NULL)
		cur_csid_font->reg_l = cur_reg_l = 
		    malloc (sizeof(struct reg_list));
	    else
		cur_reg_l = cur_reg_l->next = 
		    malloc (sizeof(struct reg_list));
	    if (!cur_reg_l ||
		!(cur_reg_l->registry = malloc (strlen(reg)+1)) ||
		!(cur_reg_l->encoding = malloc (strlen(enc)+1)))
		errorexit(MSG_MEMALLOCERR, "");
	    cur_reg_l->next = NULL;
	    
	    strcpy ( cur_reg_l->registry, reg);
	    strcpy ( cur_reg_l->encoding, enc);
	}
    }
    fclose(fp);
    cur_csid_font = NULL;
    *csnum = --tnum;
    return (csid_font);
}

/*
 * Read File : /usr/lib/lpd/etc/nls.dir 
 */
Nls_dir *read_nls_dir()
{
    char path[MAXLEN], arg[MAXLEN], loc[MAXLEN], cs[MAXLEN], *str;
    Nls_dir *nls, *cur;
    FILE *fp;
    int ch;
    
    nls = NULL;
    strcpy(path, PETCPATH);
    strcat(path, MBNLSDIR);

    if((fp = fopen(path, "r")) == NULL){
	errorexit( MSG_BADNLSDIR, path );	    	
    }
    while( !feof( fp ) ){
	for(str = arg; (ch = getc( fp )) != EOF && ch != '\n'; )
	    *str++ = (char)ch;
	*str = NULL;			
	if(sscanf(arg, "%s%s", cs, loc) == 2){
	    if(arg && cs && loc && cs[0] != '#' && loc[0] != '#'){
		if (!nls)
		    nls = cur = (Nls_dir *)
			malloc(sizeof(Nls_dir));	
		else 
		    cur = cur->next = (Nls_dir *)
			malloc(sizeof(Nls_dir));
		if (!cur){
		    errorexit(MSG_MEMALLOCERR, "");
		}
		cur->codesetname = malloc (strlen(cs)+1);
		cur->localename  = malloc (strlen(loc)+1);
		strcpy( cur->codesetname, cs );
		strcpy( cur->localename,  loc );
		cur->next == NULL;
	    }
	}
    }
    return (nls);
}

/*
 * Set Locale and CodeSet
 */
int set_loc_and_cs (CURcodeset ccsp, char* localename, char* codesetname)
{
    static Nls_dir *cur, *nls_dir = NULL;
    int flag = 0;

    if( !nls_dir ){
	nls_dir = read_nls_dir();
    }
    for (cur = nls_dir; cur; cur = cur->next){
	if (strcmp (cur->codesetname, codesetname) == 0 ||
	    strcmp (cur->localename,  localename) == 0){
	    setlocale(LC_CTYPE, cur->localename);
	    if ((strcmp( cur->codesetname, nl_langinfo( CODESET))) == 0){
	        ccsp->localename   = cur->localename;
	        ccsp->proccodeset  = cur->codesetname;
		DEBUGPR((stderr, "Match : loc : %s -- proc : %s (in : %s )\n", 
		    ccsp->localename, ccsp->proccodeset, ccsp->inputcodeset));
		return(1);
	    }
#if defined (DEBUG)
	    flag = 1;
#endif /* DEBUG */
	}

    }
#if defined (DEBUG)
    if (flag){
	DEBUGPR((stderr, "Locale Data Base seems to be wrong!\n"));
    }
#endif /* DEBUG */
    setlocale(LC_CTYPE, "");
    return(0);
}

/*
 * Read 
 */
In_to_proc *read_i_to_p(char *strti)
{
    int len, i = 0, stat = NONE;
    char *strs, *stre, **strp; 
    In_to_proc *i_to_p, *pre_i_to_p, *cur_i_to_p = NULL;
    struct in_cd_list *pre_in_cd_l, *cur_in_cd_l = NULL;
    
    strs = stre = strti;

    pre_i_to_p = NULL;
    if ((cur_i_to_p = (In_to_proc *) 
	 malloc (sizeof (In_to_proc))) == NULL ||
	(cur_i_to_p->in_cd_l = cur_in_cd_l = (struct in_cd_list *) 
	 malloc (sizeof (struct in_cd_list))) == NULL){
	errorexit(MSG_MEMALLOCERR, "");
    }
    i_to_p = cur_i_to_p;
    cur_i_to_p->next = NULL;
    cur_in_cd_l->next = NULL;
    
    while (1){
	switch (*stre){
	case '[':
	    if (stat == NONE){
		stat = INCD;
		strs = ++stre;
	    }
	    else 
		stat = INVAL;
	    break;
	    
	case ',':
	    if (stat & INCD){
		stat |= GET;
		strp = &(cur_in_cd_l->in_cd);
		pre_in_cd_l = cur_in_cd_l;
		if ((cur_in_cd_l = (struct in_cd_list *) 
		     malloc (sizeof (struct in_cd_list))) == NULL){
		    errorexit(MSG_MEMALLOCERR, "");
		}
		pre_in_cd_l->next = cur_in_cd_l;
		cur_in_cd_l->next = NULL;
	    }
	    else if (stat == PRCD){
		stat = NONE | GET;
		strp = &(cur_i_to_p->proc_cd);
		pre_i_to_p = cur_i_to_p;
		if ((cur_i_to_p = (In_to_proc *) 
		     malloc (sizeof (In_to_proc))) == NULL ||
		    (cur_i_to_p->in_cd_l = cur_in_cd_l = (struct in_cd_list *) 
		     malloc (sizeof (struct in_cd_list))) == NULL){
		    errorexit(MSG_MEMALLOCERR, "");
		}
		pre_i_to_p->next = cur_i_to_p;
		cur_i_to_p->next = NULL;
		cur_in_cd_l->next = NULL;
	    }
	    else
		stat = INVAL;
	    break;
	    
	case ']':
	    if (stat & INCD){
		stat = PRCD | GET;
		strp = &(cur_in_cd_l->in_cd);
	    }
	    else 
		stat = INVAL;
	    break;
	    
	default:
	    if (*stre == NULL){
		stat |= GET | END;
		strp = &(cur_i_to_p->proc_cd);
	    }	    
	    else
		stre++;
	}
	
	if (stat & GET){
	    stat &= ~GET;
	    *stre = NULL;
	    stre++;
	    if ((*strp = (char *) malloc (stre - strs)) == NULL)
		errorexit(MSG_MEMALLOCERR, "");
	    strcpy(*strp, strs);
	    if (stat & END)
		break;
	    strs = stre;
	}
	else if (stat == INVAL)
	    errorexit(MSG_BADFORMATP, "Ti");

    }
    return(i_to_p);
}

/*
 * From the value of attribute Ti, specify the input codeset
 * and corresponding processcodeset. 
 */
int init_i_to_p(CURcodeset ccsp, char* strti, char*  codesetname)
{
    In_to_proc *i_to_p, *cur_i_to_p;
    struct in_cd_list *cur_in_cd_l;
    
    DEBUGPR((stderr, "Start init_i_to_p\n"));

    i_to_p = read_i_to_p (strti);
    
    ccsp->inputcodeset = sp_alias_to_name( codesetname );

    DEBUGPR((stderr, "codsetname = %s, inputcodeset realname = %s\n",
	     codesetname, ccsp->inputcodeset));
    
    for (cur_i_to_p = i_to_p; cur_i_to_p; cur_i_to_p = cur_i_to_p->next){
	for (cur_in_cd_l = cur_i_to_p->in_cd_l; 
	     cur_in_cd_l; cur_in_cd_l = cur_in_cd_l->next){
	    if (strcmp(sp_alias_to_name(cur_in_cd_l->in_cd), 
		       ccsp->inputcodeset) == 0){
		if ((ccsp->proccodeset = 
		     malloc (strlen (cur_in_cd_l->in_cd) + 1)) == NULL){
		    errorexit(MSG_MEMALLOCERR, "");
		}
		strcpy (ccsp->proccodeset, cur_i_to_p->proc_cd);
		return(set_loc_and_cs (ccsp, "", ccsp->proccodeset));
	    }
	}
    }
}


/*
 * NAME:      init_pre_conv
 *
 * FUNCTION:  open iconv converter and translation table 
 *            for converting process code-point to output code-point
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:   	input code --(trans)--(iconv)--> output code
 *
 * RETURNS:   int     TRUE:   success
 *                    FALSE:  failure
 */

int init_pre_conv (CURcodeset ccsp, char *strTi, char *codesetname)
{
    Trans_dir      *tr_dir, *cur;

    DEBUGPR((stderr, "Start init_pre_conv\n"));

    if(!init_i_to_p(ccsp, strTi, codesetname)) {
	DEBUGPR((stderr, "fail init_i_to_p\n")); 
	return(0);
    }

    DEBUGPR((stderr, "proccodeset =  %s,   inputcodeset = %s\n"
	      , ccsp->proccodeset, ccsp->inputcodeset));
    if((ccsp->pre_iconv = 
	iconv_open(ccsp->proccodeset, ccsp->inputcodeset)) == -1){
	ccsp->pre_iconv = NULL;
	ccsp->pre_trans = NULL;
	tr_dir = init_mb_tr_tbl();
        for(cur = tr_dir; cur; cur = cur->next){
	    if(strcmp(cur->f_sp_cd, ccsp->inputcodeset) == 0){
		if(strcmp(cur->t_sp_cd, ccsp->proccodeset) == 0){
		    /* 
		     * input code --(trans)--> process code
		     */ 
                    if (ccsp->pre_trans = 
			read_mb_tr_tbl(cur, &(ccsp->pre_trans_num))){
                     	DEBUGPR((stderr, "trans_open(%s <-- %s)\n",
					cur->t_sp_cd, cur->f_sp_cd));
                       	break;
		    }
		}
		/* 
		 * input code --(trans)--(iconv)--> process code
		 */ 
		else if((ccsp->pre_iconv = 
			 iconv_open(ccsp->proccodeset, cur->t_sp_cd)) != -1 &&
			(ccsp->pre_trans = 
			 read_mb_tr_tbl(cur, &(ccsp->pre_trans_num))) != NULL){
		    DEBUGPR((stderr, "iconv_open(%s <-- %s)\n", 
			     ccsp->proccodeset, cur->t_sp_cd));	
		    DEBUGPR((stderr, "trans_open(%s <-- %s)\n",
			     cur->t_sp_cd, cur->f_sp_cd));	
		    break;
		}
		else{
		    ccsp->pre_iconv = NULL;
		}
	    }
	}
	if (!cur){ 
	    DEBUGPR((stderr, "Can't find a way to convert \"%s\" to \"%s\"\n",
		     ccsp->inputcodeset, ccsp->proccodeset));
	    return (FALSE);
	}
    }
#if defined (DEBUG)
    else{
	/* 
	 * input code --(iconv)--> process code
	 */ 
    	DEBUGPR((stderr, "iconv_open ( %s, %s )\n", 
		 ccsp->proccodeset, ccsp->inputcodeset));
    }
#endif	/* DEBUG */
    return (TRUE);
}

Csid_font *init_lc_font_info (CURcodeset ccsp)
{
    int csnum = 0, fontnum, i, flag = 0;
    Csid_font *csid_font, *cur_csid_font;
    struct reg_list  *cur_reg_l;
    FONTinfo fip, cur_fip;
    CSinfo   csip;

    if((csid_font = read_lc_font_info(ccsp->proccodeset, &csnum)) == NULL){
	return(0);
    }

    if (csnum > ccsp->CSnum){
	if ((ccsp->CSP = (CSinfo) 
	    realloc(ccsp->CSP, sizeof(CSinforec)*(csnum + 1)) ) == NULL)
	    errorexit(MSG_MEMALLOCERR, "");

	for (csip = ccsp->CSP, i = ccsp->CSnum; i <= csnum; i++){
	    csip[i].iconvname = NULL;
	    csip[i].flags = FONT_X;
	    csip[i].pFont = 0;
	    csip[i].wid = -1;
	}
	ccsp->CSnum = csnum;
    }

    for (csnum = 0, fontnum = 0, cur_csid_font = csid_font; cur_csid_font; 
	cur_csid_font = cur_csid_font->next, csnum++){
	ccsp->CSP[csnum].reg = cur_csid_font->reg_l;
	DEBUGPR((stderr, " CSP[%d] = %d\n", csnum, ccsp->CSP[csnum].flags));
	for (cur_reg_l = cur_csid_font->reg_l;
	     ccsp->CSP[csnum].flags == FONT_X && cur_reg_l && ++fontnum;
	     cur_reg_l = cur_reg_l->next){
	    DEBUGPR((stderr, "ccsp->CSP[%d].flags = %d : use X font!!!\n",
		     csnum, ccsp->CSP[csnum].flags));
	}
    }
    
    DEBUGPR((stderr, "CSnum = %d\n", ccsp->CSnum));
    DEBUGPR((stderr, "fontnum = %d\n", fontnum));
    
    /*
     * Ja_JP and ja_JP unique for handle udcJP
     * This file includes both IBM-defined fonts and User-defined fonts.
     * Some printers have IBM-defeined fonts in its ROM. So we have to 
     * Handle the IBM-defined and User-defined.
     */
    if( strcmp(ccsp->localename, "Ja_JP") == 0 ||
	strcmp(ccsp->localename, "ja_JP") == 0) {
	/* copy  csid:3 --> csid:4 */
	sudc = 3;
    }
    else if(strcmp(ccsp->localename, "UNIVERSAL") == 0){
	/* copy Òcsid:12 --> csid:19 */
	sudc = 12;
    }

	    DEBUGPR((stderr, "sudc = %d\n", sudc));

    if( sudc ){	
	csip = ccsp->CSP;

	DEBUGPR((stderr, "csip[%d].flags = %d\n", sudc, csip[sudc].flags));

	if( csip[sudc].flags == FONT_P ){

	    DEBUGPR((stderr, "csnum++ for udcJP\n"));
	    udc = ccsp->CSnum++;
	    csip[udc].reg = csip[sudc].reg;

	    for (cur_reg_l = csip[udc].reg; cur_reg_l;
		cur_reg_l = cur_reg_l->next, fontnum++){
	    }
            DEBUGPR((stderr, "CSnum = %d\n", ccsp->CSnum));
            DEBUGPR((stderr, "fontnum = %d\n", fontnum));
	}
    }

    /*
     * Search corresponding registry and encoding for each csid
     */
    if((ccsp->FIP = (FONTinfo) 
	malloc(sizeof(FONTinforec) * (fontnum+1))) == NULL)
	errorexit(MSG_MEMALLOCERR, "");
    for (fip = ccsp->FIP, csnum = 0, csip = ccsp->CSP; 
	 csnum < ccsp->CSnum; csnum++, csip++){
	for (cur_reg_l = csip->reg;
	     ccsp->CSP[csnum].flags == FONT_X && cur_reg_l;
	     cur_reg_l = cur_reg_l->next){
	    DEBUGPR((stderr, "search CSID = %d ( %s - %s ) \n",
		     csnum, cur_reg_l->registry, cur_reg_l->encoding));
	    for(cur_fip = ccsp->FIP; cur_fip < fip; cur_fip++){
		if (strcmp(cur_fip->cs_registry, cur_reg_l->registry) == 0 &&
		    strcmp(cur_fip->cs_encoding, cur_reg_l->encoding) == 0){
		    flag = 1;
		    break;
		}
	    }
	    if (flag != 1 || cur_fip == fip){
		fip->flags = NO_QUERY;
    		fip->filename = NULL;
		fip->pFont = NULL;
		fip->cs_registry = cur_reg_l->registry;
		fip->cs_encoding = cur_reg_l->encoding;
		fip++;
		flag = 0;
	    }
	}
    }
    fip->flags = NO_QUERY;
    fip->pFont = NULL;
    fip->cs_registry = NULL;
    fip->cs_encoding = NULL;

    return( csid_font );
}

int init_post_conv(CURcodeset ccsp, 
		   char *strto, 
		   char *fontslist, 
		   char *fontpath)
{
    static struct out_cd_list *p;
    static Csid_font   *cs_font, *f;
    static struct reg_list    *reg;
    static FONTinfo           fip;
    static CSinfo             csip;
    static int	csnum, i, j;
    char deffontpath[]  = "/usr/lpp/X11/lib/X11/fonts/";
    char deffontslist[] = "*";
    
    DEBUGPR((stderr, "Start init_post_conv\n"));

    init_p_to_o(ccsp, strto);

    DEBUGPR((stderr, "CSnum = %d\n", ccsp->CSnum));

    if (!*fontpath) fontpath = deffontpath;
    if (!*fontslist) fontslist = deffontslist;
    DEBUGPR((stderr, "fontpath = %s\n", fontpath));
    DEBUGPR((stderr, "fontslist = %s\n", fontslist));
    /* 
     *  Use X font 
     */
    if ((cs_font = init_lc_font_info(ccsp)) && fontslist[0]) {
	init_Fontpath(fontpath);
	init_Fontlist(fontslist, ccsp->FIP );
    
	DEBUGPR((stderr, "CSnum = %d\n", ccsp->CSnum));
	
	for(csip = ccsp->CSP, f = cs_font, csnum = 0; csnum < ccsp->CSnum; 
	    csip++, f = f->next, csnum++){
	    DEBUGPR((stderr, "(CSID = %d)", csnum));
	    if( csip->flags == FONT_X ){
		DEBUGPR((stderr, " : FONT_X\n "));
		for (reg = csip->reg, csip->pFont = NULL; reg; 
		     reg = reg->next){
		    for( fip = ccsp->FIP; 
			fip && (fip->cs_registry || fip->cs_registry); fip++){
			if (strcmp(reg->registry, fip->cs_registry) == 0 &&
			    strcmp(reg->encoding, fip->cs_encoding) == 0 && 
			    fip->flags == FOUND){
			    csip->pFont = &fip->pFont;
			    csip->filename = fip->filename;
			    break;
			}
		    }
		    if( (csip->pFont && *csip->pFont) || csip->filename ){
			break;
		    }
		}
		if( !csip->pFont /* && !csip->filename */){
		    csip->flags = NO_FONT;
		    DEBUGPR((stderr, " : NO_FONT\n "));
		}
	        else{
		    if((csip->iconvname = (char *)
			malloc(strlen(csip->reg->registry)+
			strlen(csip->reg->encoding)+2)) == NULL){
			errorexit(MSG_MEMALLOCERR, "");
		    }
		    strcpy(csip->iconvname, csip->reg->registry);
		    strcat(csip->iconvname, "-");
		    strcat(csip->iconvname, csip->reg->encoding);
		}
	    }
	}
    }
    else{
	for(csip = ccsp->CSP, csnum = 0; csnum <= ccsp->CSnum; csip++, csnum++){
	    if( csip->flags == FONT_X ){
		csip->flags = NO_FONT;
	    }
	}
	/*
	 * for IBM_udcJP
	 */
	if((strcmp(ccsp->localename, "Ja_JP") == 0 ||
	    strcmp(ccsp->localename, "ja_JP") == 0) && ccsp->CSnum == 4){
	    ccsp->CSnum++;
	}
    }
    if(ccsp->CSnum)
	return( open_post_iconvs( ccsp ) );
    else{
	strcpy(msgbuf, XNLSPATH);
	strcat(msgbuf, ccsp->proccodeset);
	errorexit(MSG_BADTOORNLS , msgbuf);
    }
}


unsigned char*	mb_trans( Trans_data *tr_dat, unsigned char* p, int num )
{
    unsigned int    pc = 0; 
    unsigned char   *pchar;
    int	ulim = num, llim, cur, i, j;	/* upper/lower limit for b-search */
   
    
    /*
     * check only multi byte characters
     */
    if( p == 0 || p[0] == 0 || p[1] == 0 )	return( p );
    
    for(i = 0; i < MB_LEN_MAX && p[i]; i++)
	pc = (pc << 8) + p[i];

    llim = 0;
    while (llim <= ulim) {
	cur = llim + ulim >> 1;
	if (pc < tr_dat[cur].f_cp)
	    ulim = cur - 1;
	else if (pc > tr_dat[cur].f_cp)
	    llim = cur + 1;
	else {
	    DEBUGPR((stderr, "Hit 0x%x --> 0x%x\n",
		     tr_dat[cur].f_cp, tr_dat[cur].t_cp));
	    pc = tr_dat[cur].t_cp;
	    pchar = (unsigned char *)&pc;
	    for( i = 0, j = 0; i < MB_LEN_MAX && !pchar[i]; i++)
		if (pchar[i])
		    break;
	    for( j = 0; i < MB_LEN_MAX; i++, j++)
		p[j] = pchar[i];
	    p[j] = 0;
	    break;
	}
    }
    return( p );
}

Proc_to_out *read_p_to_o(char *strto)
{
    int len, i = 0, stat = NONE;
    char *strs, *stre, **strp; 
    Proc_to_out *p_to_o, *pre_p_to_o, *cur_p_to_o;
    struct out_cd_list *pre_out_cd_l, *cur_out_cd_l;

    DEBUGPR((stderr, "Start read_p_to_o\n"));

    stat = PRCD;
    strs = stre = strto;

    pre_p_to_o = NULL;

    if ((cur_p_to_o = (Proc_to_out *) 
	 malloc (sizeof (Proc_to_out))) == NULL ||
	(cur_p_to_o->out_cd_l = cur_out_cd_l = (struct out_cd_list *) 
	 malloc (sizeof (struct out_cd_list))) == NULL)
	errorexit(MSG_MEMALLOCERR, "");
    p_to_o = cur_p_to_o;
    cur_p_to_o->next = NULL;
    cur_out_cd_l->next = NULL;

    while (1){
	switch (*stre){
	case '[':
	    if (stat == PRCD){
		stat = OUTCD | GET;
		strp = &(cur_p_to_o->proc_cd);
	    }
	    else
		stat = INVAL;
	    break;

	case ',':
	    if (stat & OUTCD){
		stat |= GET;
		strp = &(cur_out_cd_l->out_cd);
		pre_out_cd_l = cur_out_cd_l;
		if ((pre_out_cd_l->next = cur_out_cd_l = (struct out_cd_list *) 
		     malloc (sizeof (struct out_cd_list))) == NULL)
		    errorexit(MSG_MEMALLOCERR, "");
		cur_out_cd_l->next = NULL;
	    }
	    else if (stat == NONE){
		stat = PRCD;
		strs = ++stre;

		pre_p_to_o = cur_p_to_o;
		if ((cur_p_to_o = (Proc_to_out *) 
		     malloc (sizeof (Proc_to_out))) == NULL ||
		    (cur_p_to_o->out_cd_l = cur_out_cd_l = 
		     (struct out_cd_list *) 
		     malloc (sizeof (struct out_cd_list))) == NULL)
		    errorexit(MSG_MEMALLOCERR, "");
		pre_p_to_o->next = cur_p_to_o;
		cur_p_to_o->next = NULL;
		cur_out_cd_l->next = NULL;
	    }
	    else
		stat = INVAL;
	    break;

	case ']':
	    if (stat & OUTCD){
		stat = NONE | GET;
		strp = &(cur_out_cd_l->out_cd);
	    }
	    else
		stat = INVAL;
	    break;

	default:
	    if (*stre == NULL){
		if(stat == NONE)
		    stat = END;
		else
		    stat = INVAL;
	    }
	    else
		stre++;
	}

	if (stat & END) break;

	if (stat & GET){
	    stat &= ~GET;
	    *stre = NULL;
	    stre++;
	    if(stre - strs){
		if ((*strp = (char *) malloc (stre - strs)) == NULL)
		    errorexit(MSG_MEMALLOCERR, "");
		strcpy(*strp, strs);
	    }
	    else
		*strp = NULL;
	    strs = stre;
	}
	else if (stat == INVAL)
	    errorexit(MSG_BADFORMATP, "To");
    }
    return(p_to_o);
}

int init_p_to_o(CURcodeset ccsp, char * strto)
{
    Proc_to_out *p_to_o, *cur_p_to_o;
    struct out_cd_list *cur_out_cd_list, *cur;
    CSinfo csp;
    int csnum;

    DEBUGPR((stderr, "Start init_p_to_o\n"));

    if(strlen(strto) && (p_to_o = read_p_to_o(strto))){
	for (cur_p_to_o = p_to_o; cur_p_to_o; cur_p_to_o = cur_p_to_o->next){
	    if (strcmp(cur_p_to_o->proc_cd, ccsp->proccodeset) == 0){
		break;
	    }
	}
	/*
	 * If process code is not found, use the codeset for "*". 
	 */
	if (!cur_p_to_o){
	    for (cur_p_to_o = p_to_o; cur_p_to_o; 
		 cur_p_to_o = cur_p_to_o->next){
		if (strcmp(cur_p_to_o->proc_cd, "*") == 0){
		    break;
		}
	    }
	}
	/*
	 * Count the number of code set ID. for current process code set. 
	 */

	for (csnum = 0, cur = cur_p_to_o->out_cd_l; cur; 
	     csnum++, cur = cur->next);
    }
    else{
	/*
	 * Attribute To is NULL or written in wrong format
	 */
	csnum = 0;
    }
    ccsp->CSnum = csnum;

    if((ccsp->CSP = (CSinfo)
	malloc(sizeof(CSinforec) * (csnum+1))) == NULL){
	errorexit(MSG_MEMALLOCERR, "");
    }
    
    /*
     * Initialize ccsp->CSP 
     */
    csp = ccsp->CSP;
    if(csnum){
	for (cur = cur_p_to_o->out_cd_l; cur;
	     cur = cur->next, csp++){
	    if (cur->out_cd[0]){
		csp->flags = FONT_P;
	    }
	    else{
		csp->flags = FONT_X;
	    }
	    csp->pFont = 0;
	    csp->iconvname = cur->out_cd;
	    DEBUGPR((stderr, "csp->iconvname = %s \n", csp->iconvname));
	    csp->wid = -1;
	}
    }
    csp->pFont = 0;
    csp->iconvname = NULL;
    csp->flags = FONT_X; /* or NO_FONT */
    csp->wid = -1;

    return (ccsp->CSnum);
}

/*
 * NAME:	open_post_iconvs
 *
 * FUNCTION:	open all iconv converters and translation tables 
 *		for converting process code-point to output code-point
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:	process code --(iconv)--(trans)--> output code
 *
 * RETURNS:	int	TRUE:	success
 * 			FALSE:	failure
 */

int	open_post_iconvs (CURcodeset ccsp)
{
    int            i ;
    unsigned long  x;    
    Trans_dir      *tr_dir, *cur;
    CSinfo         csip;
    
    DEBUGPR((stderr, "Start open_post_iconvs\n"));
    
    if ((ccsp->post_iconv = calloc (sizeof(iconv_t), ccsp->CSnum)) == 0 ||
	(ccsp->post_trans = calloc (sizeof(Trans_data *), ccsp->CSnum)) == 0 ||
	(ccsp->post_trans_num = calloc (sizeof(int), ccsp->CSnum )) == 0){
	errorexit(MSG_MEMALLOCERR, "");
    }

    /*
     * setting post convs
     */
    for (i = 0, csip = ccsp->CSP; i < ccsp->CSnum; i++, csip++ ){
	ccsp->post_iconv[i] = NULL;
	ccsp->post_trans[i] = NULL;
	DEBUGPR((stderr, "CSID = %d\n",i));
	if(csip->iconvname == NULL || csip->flags == NO_FONT){
	    DEBUGPR((stderr, "Can't find a way to convert or No font!!\n",i));
	    continue;
	}
	if (strcmp (csip->iconvname, ccsp->proccodeset)){  
	    if ((ccsp->post_iconv[i] = (unsigned long *)
		 iconv_open (csip->iconvname, ccsp->proccodeset)) == -1){
		ccsp->post_iconv[i] = NULL;
		tr_dir = init_mb_tr_tbl();
		for (cur = tr_dir; cur; cur = cur->next){
		    if (strcmp (cur->t_sp_cd, csip->iconvname) == 0){
			if (strcmp (cur->f_sp_cd, ccsp->proccodeset) == 0){
	  		    /* 		
		 	     * process code --(trans)--> output code
			     */ 
			    if ((ccsp->post_trans[i] = read_mb_tr_tbl
				 ( cur, &(ccsp->post_trans_num[i]))) != NULL){
DEBUGPR((stderr, "post_trans_open[%d](%s <-- %s)\n", 
	 i, cur->t_sp_cd, cur->f_sp_cd));
				break;
			    }
			}
	  		/* 		
			 * process code --(iconv)--(trans)--> output code
	 		 */ 
			else if((ccsp->post_iconv[i] = 
				 iconv_open (cur->f_sp_cd, 
					     ccsp->proccodeset)) != -1 &&
				(ccsp->post_trans[i] = 
				 read_mb_tr_tbl(cur, &(ccsp->post_trans_num[i])
						)) != NULL){
DEBUGPR((stderr, "post_iconv_open[%d](%s <-- %s)\n",
	 i, cur->f_sp_cd, ccsp->proccodeset));	
DEBUGPR((stderr, "post_trans_open[%d](%s <-- %s)\n",
	 i, cur->t_sp_cd, cur->f_sp_cd));	
			    break;
			}
			else{
			    ccsp->post_iconv[i] = 0;
			}
		    }
		}
		if (!cur){
DEBUGPR((stderr, "Can't find a translation method!\n"));
DEBUGPR((stderr, " ( %s --> %s )\n", ccsp->proccodeset, csip->iconvname));
		    return(FALSE);
		}
	    }
	    /* 		
	     * process code --(iconv)--> output code
	     */ 
#if defined (DEBUG)
	    else{
DEBUGPR((stderr, "post_iconv_open[%d](%s <-- %s)\n",
	 i, csip->iconvname, ccsp->proccodeset));	
	    }
#endif	/* DEBUG */
	}
    }
    return(TRUE);
}
