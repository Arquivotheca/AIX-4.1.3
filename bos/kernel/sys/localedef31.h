/* @(#)02	1.2  src/bos/kernel/sys/localedef31.h, libcloc, bos411, 9428A410j 9/24/93 16:25:14 */
#ifndef _H_LOCALEDEF
#define _H_LOCALEDEF
/*
 * COMPONENT_NAME: (LIBCLOC) Locale database
 *
 * FUNCTIONS: localedef.h 
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

/*
**	LC_COLLATE Tables:
*/
/*
**	Struct for extended collating descriptors.
*/
typedef struct  coldesc  {                     /* descriptor for col */
	short     cd_stroff;                   /* see NLcolval */
	short	  cd_repoff;
	short     cd_cval;
	short	  cd_cuniq;
} coldesc_t;

/*  
**	Struct for collation tables.
*/
typedef struct collation_table {                /* LC_COLLATE */
	short    lc_version;                    /* 1 for now */
	short	 lc_length;			/* length of this table */
	char     *lc_locale_name;		/* pointer to locale name */
	int      len_collate;
	short    *lc_collate;                   /* ptr to coll tbl */
	int      len_coluniq;
	short    *lc_coluniq;                   /* ptr to 2nd wt tbl */
	int      len_coldesc;
	coldesc_t *lc_coldesc;                  /* ptr to coldesc */
	int	 len_strings;			
	wchar_t  *lc_strings;			/* ptr to coldesc strings */
	int	 high_cvalue;			/* largest allocated uniq */
} col_t;

/*
**	LC_CTYPE Table:
**	CHARACTER COLLATING/CLASSIFICATION INFO.
**  	Struct for extended char class & converison tables
*/

typedef struct char_classification_table {      /* LC_CTYPE */
	short      lc_version; 		/* version 1 */
	short	   lc_length;  		/* length of this table */
	short      lc_code_type; 	/* 0 for now */
	short      mb_cur_max;   	/* 2 bytes max for a character */
	short      mb_cur_min;   	/* 1 byte minimum for a character */
	short      lc_dsp_width;                  
	char       *lc_locale_name; 	/* pointer to locale name */
	int        len_caseconv;   	/* table length */
	wchar_t    *lc_caseconv;   	/* ptr to tbl */
	int	   len_ctype;
	unsigned short *lc_ctype;	/* old ctype */	
} ctype_t;

/*
**	LC_MONETARY Table
**	Struct for Monetary values
*/
typedef	struct lc_monetary_table {
	short  	lc_version;
	short 	lc_length;		/* length of this table */
	char   	*lc_locale_name;	/* pointer to locale name */
	char 	*int_curr_symbol;	/* international currency symbol*/
	char 	*currency_symbol;	/* national currency symbol	*/
	char 	*mon_decimal_point;	/* currency decimal point	*/
	char 	*mon_thousands_sep;	/* currency thousands separator*/
	char 	*mon_grouping;		/* currency digits grouping	*/
	char 	*positive_sign;		/* currency plus sign		*/
	char 	*negative_sign;		/* currency minus sign		*/
	char 	int_frac_digits;	/* internat currency fract digits*/
	char 	frac_digits;		/* currency fractional digits	*/
	char 	p_cs_precedes;		/* currency plus location	*/
	char 	p_sep_by_space;		/* currency plus space ind.	*/
	char 	n_cs_precedes;		/* currency minus location	*/
	char 	n_sep_by_space;		/* currency minus space ind.	*/
	char 	p_sign_posn;		/* currency plus position	*/
	char 	n_sign_posn;		/* currency minus position	*/
} mon_t;

/*  	
**	LC_NUMERIC Table:
**	Struct for numeric editing tables
*/
typedef struct numeric_table {                  /* LC_NUMERIC */
	short	lc_version;
	short	lc_length;		/* length of this table */
	char    *lc_locale_name;	/* pointer to locale name */
	char 	*decimal_point;
	char 	*thousands_sep;
	char	*grouping;
} num_t;

/* 
**	LC_MESSAGES Table:
**	Structure for message support 
*/
typedef struct lc_messages_table {
	short	lc_version;
	short	lc_length;		/* length of this table */
	char    *lc_locale_name;	/* pointer to locale name */
	char 	*messages;		/* Message Catalog name */
	char 	*yes_string;		/* Response string for affirmation */
	char 	*no_string;		/* Response string for negation */
} msg_t;

/*  
** 	LC_TIME table:
**	Struct for date/time editing tables
*/
typedef struct lc_time_table {
	short   lc_version;
	short	lc_length;	 /* length of this table */
	char    *lc_locale_name; /* pointer to locale name */
	char    *t_fmt;         /* NLTIME; date %X descriptor */
	char    *d_fmt;         /* NLDATE; date %x descriptor */
	char    *nlldate;       /* NLLDATE  long form         */
	char    *d_t_fmt;       /* NLDATIM, date %c descriptor */
	char    *abday;         /* NLSDAY; date %a descriptor */
	char    *day;           /* NLLDAY; date %A descriptor */
	char    *abmon;         /* NLSMONTH; date %b descriptor */
	char    *mon;           /* NLLMONTH; date %B descriptor */
/* 
**	Posix extensions needed to add capability needed for some 
**	commands like at. This allows for translation into other languages.
*/
	char    *misc;          /* NLTMISC  at;each;every;on;through  */
	char    *tstrs;         /* NLTSTRS */
	char    *tunits;        /* NLTUNITS */
/*
** 	Extended capability to name the year 
*/
	char	*year;		/* Name of the year and the starting time */
	char    *am_pm;         /* am and pm */
} tim_t;

/*
**	A table driven file code to process code mapping is 
**	achieved through this table. (For 3.2 use).
*/

typedef struct wchar_mapping_table {            /* used for wchar_t map */
	short    lc_version;
	short	 lc_length;			/* length of this table */
	char     *lc_identifier;
} map_t;


/*  Struct for runtime locale tables
 */

#define NLCTMAG0	(unsigned char)0x01
#define NLCTMAG1	(unsigned char)0x05

typedef struct localeinfo_table {           
	char     lc_mag0, lc_mag1;      /* magic numbers... */
	short    lc_version;            /* identifier */
	short    lc_code_type;                 
	short	 lc_length;		/* length of this table */
	col_t    *lc_coltbl;		/* LC_COLLATE */
	ctype_t  *lc_chrtbl;		/* LC_CTYPE */
	mon_t    *lc_montbl;		/* LC_MONETARY */
	num_t    *lc_numtbl;		/* LC_NUMERIC */
	tim_t    *lc_timtbl;		/* LC_TIME */
	msg_t    *lc_msgtbl;		/* LC_MESSAGES */
	map_t    *lc_maptbl;		/* For code page work later */
} loc_t;
#endif  /* _H_LOCALEDEF */
