#ifndef _h_SSTBL
/* @(#)33	1.3  src/bos/kernel/sys/sstbl.h, syserrlg, bos411, 9428A410j 3/31/94 15:39:53 */

/*
 * COMPONENT_NAME:            include/sys/sstbl.h (syserrlg)
 *
 * FUNCTIONS:  Internal symptom data definition table.
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#define	_h_SSTBL

/* NOTE:  In this include, _SSGENVAR is used to indicate variables
 * are to be generated.  This is done by errdemon.
 */

/* Keyword action table.
 */
/* Flags */
#define REQD	0x0010		/* Required if set */
#define REQDIBM	0x0020		/* Required for IBM code */
#define SYMP	0x0040		/* Part of symptom string */
#define CODEPT	0x0080		/* This kywd gets its own code point */

/* Data types */
#define DT_SHORT	1	/* Data is a short */
#define DT_USHORT	2	/* Data is a ushort */
#define DT_INT		3	/* Data is an int (or int) */
#define DT_ULONG	4	/* Data is a unsigned int (or int) */
#define DT_STR		5	/* Data is a string */

/* Use to set up a table item. */
#ifdef	_SSGENVAR
#define SSTABSET(k,t,l,f,kw,v) k,t,l,f,kw,v
#else
#define SSTABSET(k,t,l,f,kw,v) k,t,l,f
#endif	/* _SSGENVAR */

/* Keyword action table for internal use only.
 * This table is used in both the error device driver in the kernel,
 * and in the error reporting code.
 * Note that the keyword name and veriable to set are only defined
 * for the error reporting (non kernel) code.  It's done this way so the
 * tables won't get out of sync.
 */
#define SEV_LEN 1

#ifdef _SSGENVAR
/* Variable names. */
char prod_name[LONGNAME_LEN+1];
char prod_owner[OWNER_LEN+1];
char prod_id[PIDS_LEN+1];
char prod_ver[LVLS_LEN+1];
char appl_id[APPLID_LEN+1];
char probe_id[PCSS_LEN+1];
char desc[DESC_LEN+1];
char sev[SEV_LEN+1];
char serno[SN_LEN+1];
#endif /* _SSGENVAR */

struct {
	int kwd;		/* Keyword id */
	short type;		/* Data type */
	short len;		/* String length */
	short flags;		/* flags */
#ifdef _SSGENVAR
	char *kwdn;		/* Keyword name */
	void *kvar;		/* Variable to set (NULL if none) */
#endif /* _SSGENVAR */
	} kwdtbl[] = {
{SSTABSET(SSKWD_LONGNAME,DT_STR,LONGNAME_LEN,CODEPT,"longname",&prod_name)},
{SSTABSET(SSKWD_OWNER,DT_STR,OWNER_LEN,CODEPT,"owner",&prod_owner)},
{SSTABSET(SSKWD_PIDS,DT_STR,PIDS_LEN,(REQDIBM | SYMP),"PIDS",&prod_id)},
{SSTABSET(SSKWD_LVLS,DT_STR,LVLS_LEN,(REQDIBM | SYMP),"LVLS",&prod_ver)},
{SSTABSET(SSKWD_APPLID,DT_STR,APPLID_LEN,CODEPT,"appl-id",&appl_id)},
{SSTABSET(SSKWD_PCSS,DT_STR,PCSS_LEN,(REQD | SYMP),"PCSS",&probe_id)},
{SSTABSET(SSKWD_DESC,DT_STR,DESC_LEN,CODEPT,"description",&desc)},
{SSTABSET(SSKWD_SEV,DT_INT,sizeof(int),CODEPT,"severity",&sev)},
{SSTABSET(SSKWD_AB,DT_INT,sizeof(int),SYMP,"AB",NULL)},
{SSTABSET(SSKWD_ADRS,DT_ULONG,sizeof(ulong),SYMP,"ADRS",NULL)},
{SSTABSET(SSKWD_DEVS,DT_STR,DEVS_LEN,SYMP,"DEVS",NULL)},
{SSTABSET(SSKWD_FLDS,DT_STR,FLDS_LEN,SYMP,"FLDS",NULL)},
{SSTABSET(SSKWD_MS,DT_STR,MS_LEN,SYMP,"MS",NULL)},
{SSTABSET(SSKWD_OPCS,DT_STR,OPCS_LEN,SYMP,"OPCS",NULL)},
{SSTABSET(SSKWD_OVS,DT_STR,OVS_LEN,SYMP,"OVS",NULL)},
{SSTABSET(SSKWD_PRCS,DT_INT,sizeof(int),SYMP,"PRCS",NULL)},
{SSTABSET(SSKWD_REGS,DT_STR,REGS_LEN,SYMP,"REGS",NULL)},
{SSTABSET(SSKWD_VALU,DT_ULONG,sizeof(ulong),SYMP,"VALU",NULL)},
{SSTABSET(SSKWD_RIDS,DT_STR,RIDS_LEN,SYMP,"RIDS",NULL)},
{SSTABSET(SSKWD_SIG,DT_INT,sizeof(int),SYMP,"SIG",NULL)},
{SSTABSET(SSKWD_SN,DT_STR,SN_LEN,CODEPT,"serial number",&serno)},
{SSTABSET(SSKWD_SRN,DT_STR,SRN_LEN,SYMP,"MS",NULL)},
{SSTABSET(SSKWD_WS,DT_STR,WS_LEN,SYMP,"WS",NULL)},
{SSTABSET(0,0,0,0,"",NULL)},
	};
#undef SSTABSET


#endif /* _h_SSTBL */
