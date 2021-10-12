/* @(#)77	1.2  src/bos/usr/lib/pios/piolimits.h, cmdpios, bos411, 9428A410j 3/24/94 18:42:50 */
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*******************************************************************************
*  piolimits.h - definitions used internally by the printer backend            *
*******************************************************************************/

	/* STRUCTURES AND DEFINES USED FOR LIMITS FIELD PROCESSING */

#ifndef _H_PIOLIMITS
#define _H_PIOLIMITS 

/* Misc. macros */
#define STRGZARG(a)		#a
#define STRGZS(a)		STRGZARG(a)
#define MIN(a,b)		((a) < (b) ? (a) : (b))
#define ARRAYSIZ(a)		(sizeof(a)/sizeof(*a))
#define ALWAYS			2	/* TRUE being 1 and FALSE being 0 */
#define WKBUFLEN		(1023)
#define MSGBUFLEN		(WKBUFLEN+1)
#define RDBUFSIZE		(WKBUFLEN+1)
#define ESCCHAR			'%'
#define ERRVAL			(-1)
#define WHSPCHRS		" \t"
#define SETBEGINCHAR		'['
#define SETENDCHAR		']'
#define RINGLISTSEP		','
#define RINGMSGSEP		';'
#define RINGAIXVSEP		'='
#define RANGELISTSEP		'.'
#define LISTSEPCHRSTR		","
#define BURSTPAGEATTR		"_B"
#define DEFMCATTR		"mD"
#define DEFMC_PREFIXPATH	"/usr/lib/lpd/pio/etc/"
#define MF_PIOBESETNO		(7)
#define RING_MC			"pioattr1.cat"
#define RING_MCSETNO		(2)
#define SMITMSGCAT		"smit.cat"
#define PIOSMITMSGCAT		"piosmit.cat"
#define MCNMFMT			"pioattr%c.cat"
#define DEFMCSETNO		1
#define MSGCATNMLEN		(80)	/* should've been PATH_MAX, but due to
					   space limitations, this size is
					   assumed for now */
#define SECTNMLEN		5
#define SHRTSTRLEN		(6)
#define SMITDIR			"/smit"
#define MAXQNMLEN		20
#define MAXVNMLEN		20
#define ODM_LOCKTIME		(5)
#define MAXNOOFREGFLGS		62
#define MAXNOOFCMPFLGS		62
#define MAXNOCFSPERATTR		10
#define MAXSMITDIRLEN		100
#define MAXVDIRLEN		100
#define MAXSCHCMDLEN		100
#define MAXNOATSINASCH		512
#define DEFSETSEQNO		"z"
#define DEFMINSEQNO		100
#define NULLSTR			""
#define ODM_SCHSTR		"sm_cmd_hdr"
#define ODM_SCOSTR		"sm_cmd_opt"
#define QPRT			"qprt"
#define PIOEVATTR		"/usr/lib/lpd/pio/etc/pioevattr"
#define PIOCHPQ			"/usr/lib/lpd/pio/etc/piochpq"
#define CHVIRPRT		"chvirprt"
#define FLGSCTNM		"__FLG"
#define SYSSCTNM		"__SYS"
#define PFLSCTNM		"__PFL"
#define FILSCTNM		"__FIL"
#define INPUTPIPEPREFIX		'i'
#define REGFLGPREFIX		'_'
#define CMPFLGPREFIX		'C'
#define PREFIXPREFIX		"-a "

/* sm_cmd_opt ODM object macros */
#define ODM_FLG_SCO_CRIT	"id LIKE 'ps_%." STRGZS(MAXQNMLEN) "s.%." \
				STRGZS(MAXVNMLEN) "s." FLGSCTNM ".*'"
#define ODM_SYS_SCO_CRIT	"id LIKE 'ps_%." STRGZS(MAXQNMLEN) "s.%." \
				STRGZS(MAXVNMLEN) "s." SYSSCTNM ".*'"
#define ODM_PFL_SCO_CRIT	"id LIKE 'ps_%." STRGZS(MAXQNMLEN) "s.%." \
				STRGZS(MAXVNMLEN) "s." PFLSCTNM ".*'"
#define ODM_FIL_SCO_CRIT	"id LIKE 'ps_%." STRGZS(MAXQNMLEN) "s.%." \
				STRGZS(MAXVNMLEN) "s." FILSCTNM ".*'"
#define ODM_SCO_IDFMT		"ps_%." STRGZS(MAXQNMLEN) "s.%." \
				STRGZS(MAXVNMLEN) "s.%." STRGZS(SECTNMLEN) \
				"s.%." STRGZS(ATTRNAMELEN) "s"
#define ODM_SCO_NMPREFIX	"Attribute "
#define ODM_SCO_RINGCMD		"print -r - "

/* sm_cmd_hdr ODM object macros for 'Print A File' dialog */
#define ODM_QPRT_SCHALL_CRIT	"id LIKE 'ps_qprt_%." STRGZS(MAXQNMLEN) "s.%." \
				STRGZS(MAXVNMLEN) "s.*'"
#define ODM_SCH_QPRT_IDFMT	"ps_qprt_%." STRGZS(MAXQNMLEN) "s.%." \
				STRGZS(MAXVNMLEN) "s.%c"
#define ODM_SCH_QPRT_OPIDFMT	"ps_qprt_commonDOpt,ps_qprt_fixed.*,#%." \
				STRGZS(MAXSMITDIRLEN) \
				"s#ps_%." STRGZS(MAXQNMLEN) "s.%." \
				STRGZS(MAXVNMLEN) "s." FLGSCTNM "._[%." \
				STRGZS(MAXNOOFREGFLGS) "s],#%." \
				STRGZS(MAXSMITDIRLEN) "s#ps_%." \
				STRGZS(MAXQNMLEN) "s.%." STRGZS(MAXVNMLEN) \
				"s." FLGSCTNM ".C[%." STRGZS(MAXNOOFCMPFLGS) \
				"s]"
#define ODM_SCH_QPRT_NAME	"Start a Print Job"
#define ODM_SCH_QPRT_MSGSET	18
#define ODM_SCH_QPRT_MSGID	119
#define ODM_SCH_QPRT_CTEFMT	"PIOVARDIR=%." STRGZS(MAXVDIRLEN) "s %." \
				STRGZS(MAXSCHCMDLEN) "s -d%c -#v -#j"
#define ODM_SCH_QPRT_CTDFMT	"PIOVARDIR=%." STRGZS(MAXVDIRLEN) "s %." \
				STRGZS(MAXSCHCMDLEN) "s -q%." \
				STRGZS(MAXQNMLEN) "s -d%." \
				STRGZS(MAXVNMLEN) "s " BURSTPAGEATTR " " \
				"2>/dev/null "
#define ODM_SCH_QPRT_HMI	"180036"

/* sm_cmd_hdr ODM object macros for 'Change/Show Default Print Job Attributes'
   dialog */
#define ODM_JOBATTRS_SCH_CRIT	"id = 'ps_jobattrs_%." STRGZS(MAXQNMLEN) "s.%."\
				STRGZS(MAXVNMLEN) "s'"
#define ODM_SCH_JOBATTRS_IDFMT	"ps_jobattrs_%." STRGZS(MAXQNMLEN) "s.%." \
				STRGZS(MAXVNMLEN) "s"
#define ODM_SCH_JOBATTRS_OPIDFMT "ps_chpq_fixed,ps_qprt_fixed._B,#%." \
				STRGZS(MAXSMITDIRLEN) \
				"s#ps_%." STRGZS(MAXQNMLEN) "s.%." \
				STRGZS(MAXVNMLEN) "s." FLGSCTNM ".*"
#define ODM_SCH_JOBATTRS_NAME	"Change / Show Default Print Job Attributes"
#define ODM_SCH_JOBATTRS_MSGSET	1
#define ODM_SCH_JOBATTRS_MSGID	86
#define ODM_SCH_JOBATTRS_CTDFMT	"%." STRGZS(MAXSCHCMDLEN) "s -q%." \
				STRGZS(MAXQNMLEN) "s -d%." \
				STRGZS(MAXVNMLEN) "s " BURSTPAGEATTR " " \
				"2>/dev/null "
#define ODM_SCH_JOBATTRS_HMI	"1810192"

/* sm_cmd_hdr ODM object macros for 'Change/Show Printer Setup'
   dialog */
#define ODM_SETUP_SCH_CRIT	"id = 'ps_setup_%." STRGZS(MAXQNMLEN) "s.%." \
				STRGZS(MAXVNMLEN) "s'"
#define ODM_SCH_SETUP_IDFMT	"ps_setup_%." STRGZS(MAXQNMLEN) "s.%." \
				STRGZS(MAXVNMLEN) "s"
#define ODM_SCH_SETUP_OPIDFMT	"ps_chpq_fixed,#%." STRGZS(MAXSMITDIRLEN) \
				"s#ps_%." STRGZS(MAXQNMLEN) "s.%." \
				STRGZS(MAXVNMLEN) "s." SYSSCTNM ".*,#%." \
				STRGZS(MAXSMITDIRLEN) "s#ps_%." \
				STRGZS(MAXQNMLEN) "s.%." STRGZS(MAXVNMLEN) \
				"s." PFLSCTNM ".*"
#define ODM_SCH_SETUP_NAME	"Change / Show Printer Setup"
#define ODM_SCH_SETUP_MSGSET	1
#define ODM_SCH_SETUP_MSGID	87
#define ODM_SCH_SETUP_CTDFMT	"%." STRGZS(MAXSCHCMDLEN) "s -q%." \
				STRGZS(MAXQNMLEN) "s -d%." \
				STRGZS(MAXVNMLEN) "s " \
				"2>/dev/null "
#define ODM_SCH_SETUP_HMI	"1810191"

/* sm_cmd_hdr ODM object macros for 'Change/Show Pre-processing Filters'
   dialog */
#define ODM_FILTERS_SCH_CRIT	"id = 'ps_filters_%." STRGZS(MAXQNMLEN) "s.%." \
				STRGZS(MAXVNMLEN) "s'"
#define ODM_SCH_FILTERS_IDFMT	"ps_filters_%." STRGZS(MAXQNMLEN) "s.%." \
				STRGZS(MAXVNMLEN) "s"
#define ODM_SCH_FILTERS_OPIDFMT "ps_chpq_fixed,#%." STRGZS(MAXSMITDIRLEN) \
				"s#ps_%." STRGZS(MAXQNMLEN) "s.%." \
				STRGZS(MAXVNMLEN) "s." FILSCTNM ".*"
#define ODM_SCH_FILTERS_NAME	"Change / Show Pre-processing Filters"
#define ODM_SCH_FILTERS_MSGSET	1
#define ODM_SCH_FILTERS_MSGID	28
#define ODM_SCH_FILTERS_CTDFMT	"%." STRGZS(MAXSCHCMDLEN) "s -u -q%." \
				STRGZS(MAXQNMLEN) "s -d%." \
				STRGZS(MAXVNMLEN) "s " \
				"2>/dev/null "
#define ODM_SCH_FILTERS_HMI	"1810113"

/* Macros for operation types */
#define ENTRYOP			'E'	/* entry type		- optype */
#define LISTOP			'L'	/* list			- optype */
#define MLISTOP			'M'	/* multi-select list	- optype */
#define RINGOP			'R'	/* ring			- optype */
#define MRINGOP			'T'	/* multi-select ring	- optype */
#define RANGEOP			'G'	/* range list		- optype */
#define VALIDATEOP		'V'	/* validate expression	- optype */
#define SEQNOOP			'S'	/* sequence no.		- optype */
#define DISPLAYOP		'D'	/* display mode		- optype */
#define CTLISTOP		'F'	/* command to list mode	- optype */
#define REQDOP			'Q'	/* required		- optype */
#define COMPOP			'C'	/* composite flag	- optype */
#define HELPMSGOP		'H'	/* help message specs	- optype */
#define HELP1MSGOP		'I'	/* help info specs	- optype */


/* Typedefs for operation information */
typedef enum {
		ONECHAROPND = 1,	/* one char operand     - opndtype */
		BRACKETOPND		/* bracket operand      - opndtype */
	     } opndtype_t;

typedef struct opinfo {
		char		optype;		/* operator type */
		opndtype_t	opndtype;	/* operand type */
		int		(*opproc)(const char *, ushort_t, const char *,
					  const char *, const struct opinfo *);
						/* op processing funcion */
	       } opinfo_t;
typedef struct opndinfo {
		struct opndinfo	*nextp;
		const char	*opndp;		/* pointer to operand */
		ushort_t	opndlen;	/* operand length */
		opinfo_t	oi;		/* op info. */
	       } opndinfo_t;

/* Typedef for attribute info. that is not stored in ddi file. */
typedef struct {
		char		anm[ATTRNAMELEN];	/* attr name */
		char		mcnm[MSGCATNMLEN+1];	/* msg cat name */
		short		msgno;			/* msg number */
		char		sctnm[SECTNMLEN+1];	/* section name */
	       } attrothinfo_t;

/* Typedef for datastream info. */
typedef struct {
		char		dsop;		/* datastream op */
		char		(*refats)[ATTRNAMELEN];
						/* list of referenced flags */
		ushort_t	norefats;	/* no of referenced flags */
		char		regflgs[MAXNOOFREGFLGS][ATTRNAMELEN];
						/* regular flags in dialog */
		ushort_t	noregflgs;	/* no of regular flags */
		char		cmpflgs[MAXNOOFCMPFLGS][ATTRNAMELEN];
						/* composite flags in dialog */
		ushort_t	nocmpflgs;	/* no of composite flags */
	       } dsinfo_t;


/* External function declarations */
extern int		piogetstr(char *, char *, int, error_info_t *);
extern char		*getmsg(char *, int, int);
extern int		piogetrefs(const char *attrnm,int flag,
				   char (**refattrs)[ATTRNAMELEN]);
extern char		*piovalav(const char *attrnm);
extern void		piorebuild(const char *,char const *,const char *,
				   unsigned int,const attrothinfo_t *);

/* External variable declarations */
extern char *hash_table;	/* address of the primary hash table */
extern int *mem_start;		/* address of the start of the odm data */
extern char *odm_table;		/* address of the odm table */
extern unsigned int objtot;	/* number of objects in odm table */
extern int piomode;		/* indicates the set of attr. values in use */

#endif /* _H_PIOLIMITS */
