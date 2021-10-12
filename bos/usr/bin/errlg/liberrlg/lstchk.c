static char sccsid[] = "@(#)10	1.4  src/bos/usr/bin/errlg/liberrlg/lstchk.c, cmderrlg, bos411, 9428A410j 10/21/93 08:18:35";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: lst_init, flagsinit, lst_lookup, lst_chk
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Routines to compare an errlog entry to a command line
 * list of selection criteria.
 */

#include <ctype.h>
#include <string.h>
#include <sys/err_rec.h>
#include <errlg.h>

/*
 * This version of STREQ handles the wildcard character '*'
 */
#undef  STREQ
#define STREQ(s1,s2) \
	( (((s1)[0] == (s2)[0]) || (s2)[0] == '*') && streq(s1,s2) )

#define VCPY(from,to_array) \
	strncpy(to_array,from,sizeof(to_array)); \
	to_array[sizeof(to_array)-1] = '\0';

static Chkflg;			/* Set if we're checking for anything. */
static starttime;
static endtime;

/* This flag is set to TRUE when the errpt command is executed */
/* with the -Y option, otherwise it is set to FALSE.           */
extern Boolean errpt_symptomflg;

static char *strip();

/*
 * Flags mask.
 * The _F values are set if the flag was specified in any sense.
 * The _V values are set if it was positive.
 */
static Fmask;
#define MSK_REPORTABLE_F 0x02
#define MSK_REPORTABLE_V 0x01
#define MSK_ALERTABLE_F  0x04
#define MSK_ALERTABLE_V  0x08
#define MSK_LOGABLE_F    0x20
#define MSK_LOGABLE_V    0x10
#define REPORTABLE "reportable"
#define ALERTABLE  "alertable"
#define LOGABLE    "logable"

union ci {
	unsigned _i;
	char    *_c;
};

/*
 * ht consists of a number/character flag, an optional pointer to
 * a deflist for allowed values, the name of the related descriptor, and
 * the head of a linked list of desired values.
 */
struct ht {
	char       ht_valueflg;		/* values on hb_head list are numeric */
	char       ht_negflg;		/* check for NOT condition */
	char       ht_decimalflg;	/* numeric value is decimal, else hex */
	char      *ht_descrip;		/* label */
	struct nt *ht_deflist;		/* Allowable values (helps) */
	char      *ht_e;		/* ptr to data item in error record */
	struct hb *ht_head;		/* list of allowable values */
	int        ht_checkflg;		/* this item is being checked */
};

/* Defines list of values (*ht_head) */
struct hb {
	struct hb *hb_next;
	union ci  _hb;
};
#define hb_value _hb._i
#define hb_name  _hb._c
static struct hb *gethb();

/* Defines list of possible values (helps) 
 */
struct nt {
	char *nt_name;
	char *nt_comment;
};

static struct nt classlist[] = {
	{ "H", 0 },
	{ "S", 0 },
	{ "O", 0 },
	{ "U", 0 },
	0
};

static struct nt typelist[] = {
	{ "PERM", 0 },
	{ "TEMP", 0 },
	{ "PERF", 0 },
	{ "PEND", 0 },
	{ "UNKN", 0 },
	{ "INFO", 0 },
	0
};

#define HT_ID        0
#define HT_XID       1
#define HT_CLASS     2
#define HT_TYPE      3
#define HT_RCLASS    4
#define HT_RTYPE     5
#define HT_RESOURCE  6
#define HT_MACHINEID 7
#define HT_NODEID    8
#define HT_SEQUENCE  9
#define HT_LABEL     10
#define HT_XLABEL    11

/* Predefined attributes for error log fields. */
static struct ht Ht[] = {
	{1,0,0,"el_crcid",    0,        (char *)&T_errlog.el_crcid},/* id */
	{1,1,0,"el_crcid",    0,        (char *)&T_errlog.el_crcid},/* xid */
	{0,0,0,"el_class",    classlist,T_errlog.el_class},			/* class */
	{0,0,0,"el_type",     typelist, T_errlog.el_type},			/* type */
	{0,0,0,"el_rclass",   0,        T_errlog.el_rclass},		/* rclass */
	{0,0,0,"el_rtype",    0,        T_errlog.el_rtype},			/* rtype */
	{0,0,0,"el_resource", 0,        T_errlog.el_resource},		/* resource */
	{0,0,0,"el_machineid",0,        T_errlog.el_machineid},		/* machineid */
	{0,0,0,"el_nodeid",   0,        T_errlog.el_nodeid},		/* nodeid */
	{1,0,1,"el_sequence", 0,        (char *)&T_errlog.el_sequence},
	{0,0,0,"et_label",    0,        T_errtmplt.et_label},		/* label */
	{0,1,0,"et_label",    0,        T_errtmplt.et_label},		/* xlabel */
	{ 0 }
};

static void install();
static struct hb *lookup();

/*
 * NAME: lst_init
 *                                                                    
 * FUNCTION: Do any flag-specific setup of Ht.
 *	Global setup is done by iinit().
 *	Called from get_cmdline().
 *                                                                    
 * RETURNS: Nothing
 */  

lst_init(code,list)
char *list;
{
	int n;

	tzset();			/* initialize timezone */

	Chkflg++;
        errpt_symptomflg = FALSE;

	switch(code) {
	case 'F': flagsinit(list);              return;
	case 'l': n = HT_SEQUENCE  ; break;
	case 'n': n = HT_NODEID    ; break;
	case 'm': n = HT_MACHINEID ; break;
	case 'N': n = HT_RESOURCE  ; break;
	case 'd': n = HT_CLASS     ; break;
	case 'T': n = HT_TYPE      ; break;
	case 'S': n = HT_RCLASS    ; break;
	case 'R': n = HT_RTYPE     ; break;
	case 'j': n = HT_ID        ; break;
	case 'k': n = HT_XID       ; break;
	case 'J': n = HT_LABEL     ; break;
	case 'K': n = HT_XLABEL    ; break;
	case 'L': n = HT_LABEL     ; break;
	/* The times don't use the table, Ht. */
	case 's':
		if((starttime = datetosecs(list)) == -1)
			cat_fatal(CAT_INVALID_S_E_TIME,
					"Invalid start or end time supplied: %s\n",list);
		return;
	case 'e':
		if((endtime = datetosecs(list)) == -1)
			cat_fatal(CAT_INVALID_S_E_TIME,
					"Invalid start or end time supplied: %s\n",list);
		return;
	case 'Y':
		errpt_symptomflg = TRUE;
		break;
	default:
		genexit(1);
	}

	/* Perform standard table setup */
	iinit(&Ht[n],list);
}

/*
 * NAME: iinit
 *                                                                    
 * FUNCTION: Add the values in the "list" to this "htp"'s values in Ht.
 *	Called from lst_init()
 *                                                                    
 * RETURNS: nothing
 */  

static iinit(htp,list)
struct ht *htp;
char *list;
{
	char *cp;

	/* for each element in the list */
	for(cp = strtok(list,", \t"); cp; cp = strtok(0,", \t"))
		install(htp,cp);
}

/*
 * NAME: install
 *                                                                    
 * FUNCTION: Add the "entry" to the list of values for this htp
 *	Called from iinit()
 *                                                                    
 * RETURNS: nothing
 */  

static void install(htp,entry)
struct ht *htp;
char *entry;
{
	unsigned value;
	char *name;
	struct hb *hbp;

	/* If entry is a value (not a string) */
	if(htp->ht_valueflg) {
		value = strtoul(entry,0,htp->ht_decimalflg ? 10 : 16);
		if(lookup(htp,&value))
			return;
	} else {
		/* It's a string. */
		name = entry;
		if(lookup(htp,entry))
			return;
	}
	htp->ht_checkflg++;	/* Show we're now checking this one */

	/* Allocate and link in a new hb member. */
	hbp = gethb();
	hbp->hb_next = htp->ht_head;
	htp->ht_head = hbp;
	if(htp->ht_valueflg)
		hbp->hb_value = value;
	else
		hbp->hb_name = name;
}

/*
 * NAME: lookup
 *                                                                    
 * FUNCTION: Check for a specific entry in the ht_head list.
 *                                                                    
 * RETURNS: a pointer to the entry or NULL.
 */  

static struct hb *lookup(htp,entry)
struct ht *htp;
char *entry;
{
	register struct hb *hbp;
	unsigned value;
	char *name;
	char buf[128];

	if(htp->ht_valueflg) {
		value = *(unsigned *)entry;
		for(hbp = htp->ht_head; hbp; hbp = hbp->hb_next)
			if(hbp->hb_value == value)
				return(hbp);
		return(0);
	} else {
		VCPY(entry,buf);
		name = strip(buf);
		for(hbp = htp->ht_head; hbp; hbp = hbp->hb_next)
			if(STREQ(name,hbp->hb_name))
				return(hbp);
		return(0);
	}
}

/* Allocate an nb structure. */
static struct hb *gethb()
{

	return(MALLOC(1,struct hb));
}

#if 0
/* See if the named item is in the nt table pointed to by ntp0 */
static is_nt(ntp0,name)
char *name;
struct nt *ntp0;
{
	struct nt *ntp;

	for(ntp = ntp0; ntp->nt_name; ntp++)
		if(streq(name,ntp->nt_name))
			return(1);
	return(0);
}
#endif /* 0 */

/*
 * NAME: lst_lookup
 *                                                                    
 * FUNCTION: Return the value string for the Ht item specified.
 *	Used for concurrent error report.
 *                                                                    
 * RETURNS: String for the first value or NULL if not found.
 */  
char *lst_lookup(descrip)
char *descrip;
{
	struct ht *htp;

	if(streq(descrip,"et_alertflg")) {
		if(!(Fmask & MSK_ALERTABLE_F))
			return(0);
		return(Fmask & MSK_ALERTABLE_V ? "TRUE" : "FALSE");
	}
	for(htp = Ht; htp->ht_descrip; htp++)
		if(STREQ(descrip,htp->ht_descrip))
			return(htp->ht_head ? htp->ht_head->hb_name : 0);
	return(0);
}

/*
 * Allow '*' at the end of the string
 */
static streq(a,w)
register char *a,*w;
{
	register c1,c2;

	for(;;) {
		c1 = *a++;
		c2 = *w++;
		if(c1 == c2) {
			if(c1 == '\0')
				return(1);
			continue;
		}
		if(c2 == '*')
			return(1);
		return(0);
	}
}

/*
 * NAME: lst_chk
 *                                                                    
 * FUNCTION: See if this record matches the criteria in Ht.
 *	Used by errpt and errclear
 *                                                                    
 * RETURNS: TRUE - Record matches, FALSE otherwise.
 */  

lst_chk()
{
	register struct ht *htp;

	/* Match if no criteria being checked. */
	if(!Chkflg)
		return(1);

 	if(starttime && T_errlog.el_timestamp < starttime ||
 	   endtime   && T_errlog.el_timestamp > endtime)
 		return(0);

	/* Do not print the error if errpt was executed with */
	/* the -Y flag and there is no symptom data.         */
	if ((errpt_symptomflg) && (T_errlog.el_symptom_length == 0))
		return(0);

	/* Check the binary options (see the declaration of Fmask) */
	if(Fmask) {
		if(Fmask & MSK_ALERTABLE_F) {
			if( (Fmask & MSK_ALERTABLE_V  && !T_errtmplt.et_alertflg) ||
			   !(Fmask & MSK_ALERTABLE_V) &&  T_errtmplt.et_alertflg)
				return(0);
		}
		if(Fmask & MSK_REPORTABLE_F) {
			if( (Fmask & MSK_REPORTABLE_V  && !T_errtmplt.et_reportflg) ||
			   !(Fmask & MSK_REPORTABLE_V) &&  T_errtmplt.et_reportflg)
				return(0);
		}
		if(Fmask & MSK_LOGABLE_F) {
			if( (Fmask & MSK_LOGABLE_V  && !T_errtmplt.et_logflg) ||
			   !(Fmask & MSK_LOGABLE_V) &&  T_errtmplt.et_logflg)
				return(0);
		}
	}
	/* Check each item in Ht that's being used. */
	for(htp = Ht; htp->ht_descrip; htp++) {
		if(!htp->ht_checkflg)
			continue;	/* Not in use */
		/* Return false if item matches and it's not supposed to,
		 * or if it's supposed to but doesn't.
		 */
		if( htp->ht_negflg &&  lookup(htp,htp->ht_e) ||
		   !htp->ht_negflg && !lookup(htp,htp->ht_e))
			return(0);
	}
	/* Return a match */
	return(1);
}

/*
 * NAME: flagsinit
 *                                                                    
 * FUNCTION: Setup the flags (Fmask) for the -F flag.
 *                                                                    
 * RETURNS: nothing
 */  

flagsinit(line)
char *line;
{
	char *cp;
	char *name,*value;

	while(cp = strtok(line,", \t\n")) {
		line = 0;
		name = cp;
		while(*cp && *cp != '=')
			cp++;
		if(*cp != '=')
			continue;
		*cp = '\0';
		value = cp+1;
		opts(name,value);
	}
}

/* Setup the Fmask bits for the 'name'd flag. */
static opts(name,value)
char *name;	/* Flag name */
char *value;	/*   value */
{
	int maskf=0;
	int maskv=0;

	if(streq_cn(name,REPORTABLE)) {
		maskf = MSK_REPORTABLE_F;
		maskv = MSK_REPORTABLE_V;
	} else if(streq_cn(name,ALERTABLE)) {
		maskf = MSK_ALERTABLE_F;
		maskv = MSK_ALERTABLE_V;
	} else if(streq_cn(name,LOGABLE)) {
		maskf = MSK_LOGABLE_F;
		maskv = MSK_LOGABLE_V;
	}
	
	Fmask |= maskf;
	if(streq_cn(value,"no") || streq(value,"0") || streq_cn("false"))
		Fmask &= ~maskv;
	else
		Fmask |=  maskv;
}

#define ISSPACE(c) ((c) == ' ' || (c) == '\t')

/*
 * strip off leading and trailing whitespace
 */
static char *strip(str)
char *str;
{
	char *cp;
	char *cpstart;

	cp = str;
	while(ISSPACE(*cp))
		cp++;
	cpstart = cp;
	cp = cpstart + strlen(cpstart) - 1;
	while(ISSPACE(*cp) && cp > cpstart) {
		*cp = '\0';
		cp--;
	}
	return(cpstart);
}
