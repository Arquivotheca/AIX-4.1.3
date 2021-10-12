/* @(#)29	1.14.1.7  src/bos/kernel/sys/localedef.h, libcloc, bos411, 9428A410j 5/10/94 14:40:36 */

/*
 *
 * COMPONENT_NAME: (LIBCLOC) Locale Database
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef __H_LOCALEDEF
#define __H_LOCALEDEF

#define __METHOD_TBL -1         /* defect 84106: pseudo ptr to __method_tbl */

#include <sys/limits.h>
#include <sys/lc_core.h>
#include <locale.h>

#include <sys/localedef31.h>

#if defined(COLL_WEIGHTS_MAX)
#  define _COLL_WEIGHTS_MAX  COLL_WEIGHTS_MAX
#else
#  if defined(_POSIX2_COLL_WEIGHTS_MAX)
#    define _COLL_WEIGHTS_MAX _POSIX2_COLL_WEIGHTS_MAX
#  else
#    define _COLL_WEIGHTS_MAX 4
#  endif
#endif

#define _UCW_ORDER      _COLL_WEIGHTS_MAX
#define _LOC_HAS_MCCE	1

/* _LC_charmap_t
**
** Structure representing CHARMAP class which maps characters to process code
** and vice-verse.
*/
typedef struct {

    _LC_core_charmap_t core;
    
    char     *cm_csname;	/* codeset name  */
    
    size_t   cm_mb_cur_max;     /* max encoding length for this codeset  */
    size_t   cm_mb_cur_min;	/* min encoding length for this codeset  */

    unsigned char
	     cm_max_disp_width; /* max display width of any char */
				/* in the codeset */

    unsigned 
	char *cm_cstab;		/* character set id table. */

    struct __LC_locale 		/* back pointer to locale record(_LC_locale_t)*/
             *loc_rec;

    void     *__meth_ptr;	/* method extensions */
    void     *__data_ptr;	/* data extensions   */
    
} _LC_charmap_t;

typedef struct _LC_charmap_objhdl _LC_charmap_objhdl_t;

struct _LC_charmap_objhdl {
	_LC_charmap_t *obj;

#ifndef _PTR_METH
	void *(**meth)();
#endif

struct _LC_locale_objhdl *loc_hdl;	/* setlocale() assigns this to point  */
					/* to global locale handle (lc_locale)*/
					/* for current process.               */
};


/* _LC_monetary_t
**
** Structure representing MONETARY class which defines the formatting
** of monetary quantities for a locale.
*/
typedef	struct {

    _LC_core_monetary_t   core;
    
    char *int_curr_symbol;	   /* international currency symbol	*/
    char *currency_symbol;	   /* national currency symbol		*/
    char *mon_decimal_point;	   /* currency decimal point		*/
    char *mon_thousands_sep;	   /* currency thousands separator	*/
    char *mon_grouping;		   /* currency digits grouping		*/
    char *positive_sign;	   /* currency plus sign		*/
    char *negative_sign;	   /* currency minus sign		*/
    signed char int_frac_digits;   /* internat currency fract digits	*/
    signed char frac_digits;	   /* currency fractional digits	*/
    signed char p_cs_precedes;	   /* currency plus location		*/
    signed char p_sep_by_space;	   /* currency plus space ind.		*/
    signed char n_cs_precedes;	   /* currency minus location		*/
    signed char n_sep_by_space;	   /* currency minus space ind.		*/
    signed char p_sign_posn;	   /* currency plus position		*/
    signed char n_sign_posn;	   /* currency minus position		*/
    char *debit_sign;		   /* currency debit symbol		*/
    char *credit_sign;		   /* currency credit symbol		*/
    char *left_parenthesis;	   /* currency left parenthesis		*/
    char *right_parenthesis;	   /* currency right parenthesis	*/
    
    struct __LC_locale 		   /* back pointer to locale record	*/
             *loc_rec;		   /* ie to _LC_locale_t structure	*/

    void     *__meth_ptr;	   /* method extensions			*/
    void     *__data_ptr;	   /* data extensions			*/
    
} _LC_monetary_t;

typedef struct _LC_monetary_objhdl _LC_monetary_objhdl_t;
struct _LC_monetary_objhdl {
	_LC_monetary_t *obj;

#ifndef _PTR_METH
	void *(**meth)();
#endif

struct _LC_locale_objhdl *loc_hdl;	/* setlocale() assigns this to point  */
					/* to global locale handle (lc_locale)*/
					/* for current process.               */
};

/* _LC_numeric_t  	
**
** Structure representing NUMERIC class which defines the formatting 
** of numeric quantities in a locale.
*/
typedef struct {

    _LC_core_numeric_t core;
    
    char     *decimal_point;
    char     *thousands_sep;
    unsigned
	char *grouping;
    
    struct __LC_locale 		   /* back pointer to locale record	*/
             *loc_rec;		   /* ie to _LC_locale_t structure	*/

    void     *__meth_ptr;	   /* method extensions			*/
    void     *__data_ptr;	   /* data extensions			*/
    
} _LC_numeric_t;

typedef struct _LC_numeric_objhdl _LC_numeric_objhdl_t;
struct _LC_numeric_objhdl {
	_LC_numeric_t *obj;

#ifndef _PTR_METH
	void *(**meth)();
#endif

struct _LC_locale_objhdl *loc_hdl;	/* setlocale() assigns this to point  */
					/* to global locale handle (lc_locale)*/
					/* for current process.               */
};


/* _LC_resp_t
** 
** Structure representing RESPONSE class which defines the content
** of affirmative versus negative responses in a locale.
*/
typedef struct  {

    _LC_core_resp_t core;

    char     *yesexpr;	     /* POSIX: Expression for affirmative.     */
    char     *noexpr;	     /* POSIX: Expression for negative.        */
    char     *yesstr;	     /* X/OPEN: colon sep str for affirmative. */
    char     *nostr;	     /* X/OPEN: colon sep str for negative.    */

    struct __LC_locale	     /* back pointer to locale record          */
             *loc_rec;	     /* ie to _LC_locale_t structure	       */

    void     *__meth_ptr;    /* method extensions		       */
    void     *__data_ptr;    /* data extensions			       */
    
} _LC_resp_t;

typedef struct _LC_resp_objhdl _LC_resp_objhdl_t;
struct _LC_resp_objhdl {
	_LC_resp_t *obj;

#ifndef _PTR_METH
	void *(**meth)();
#endif

struct _LC_locale_objhdl *loc_hdl;	/* setlocale() assigns this to point  */
					/* to global locale handle (lc_locale)*/
					/* for current process.               */
};


/* _LC_time_t  
** 
** Structure representing the TIME class which defines the formatting
** of time and date quantities in this locale.
*/
typedef struct {

    _LC_core_time_t core;
    
    char *d_fmt;  
    char *t_fmt;  
    char *d_t_fmt;
    char *t_fmt_ampm;
    char *abday[7];  
    char *day[7];    
    char *abmon[12];  
    char *mon[12];    
    char *am_pm[2];
    char *era;
    char *era_year;
    char *era_d_fmt;
    char *alt_digits;
    char *era_d_t_fmt;
    char *era_t_fmt;
     
    struct __LC_locale	     /* back pointer to locale record          */
             *loc_rec;	     /* ie to _LC_locale_t structure	       */

    void     *__meth_ptr;    /* method extensions		       */
    void     *__data_ptr;    /* data extensions			       */

} _LC_time_t;

typedef struct _LC_time_objhdl _LC_time_objhdl_t;
struct _LC_time_objhdl {
	_LC_time_t *obj;

#ifndef _PTR_METH
	void *(**meth)();
#endif

struct _LC_locale_objhdl *loc_hdl;	/* setlocale() assigns this to point  */
					/* to global locale handle (lc_locale)*/
					/* for current process.               */
};


/* _LC_weight_t
** 
** Array of collation weights for a character or collating symbol.
*/
typedef struct {
    unsigned
	short  n[_COLL_WEIGHTS_MAX+1];
} _LC_weight_t;    

  
/* _LC_collel_t
**
** Collation data for a collation symbol
*/
typedef struct {

    char         *ce_sym;	/* value of collation symbol           */
    _LC_weight_t ce_wgt;	/* The weights associated with a       */
				/* collating symbol matching ce_sym    */
} _LC_collel_t;


/* _LC_coltbl_t
** 
** Array of per-character collation data for locale.
*/
typedef struct {
    _LC_weight_t   ct_wgt;    /* The collation weights for this      */
			      /* character.                          */

    _LC_collel_t   *ct_collel;/* Pointer to collation symbol array   */

} _LC_coltbl_t;


/* when this constant is found in the primary weight, then all weights  */
/* for that character are found in the _LC_subs_t structure, indexed by */
/* the second weight                                                    */

#define _SUB_STRING	(USHRT_MAX - 1)

/* _LC_subs_t
**
** weight strings for 1-to-many mappings
*/
typedef struct {
    char *tgt_wgt_str[_COLL_WEIGHTS_MAX+1];
    /* contains a pointer to a string of weights for each order 	 */
    /* collating table contains index pointing to an array of _LC_subs_t */
} _LC_subs_t;


/* _LC_collate_t
**
** Structure representing COLLATE class defining the collation rules
** for a locale.
*/
typedef struct {

    _LC_core_collate_t core;
    
    unsigned			            /* number of collation orders    */
	char    co_nord;	            /* supported in this locale      */
    
    _LC_weight_t co_sort;	            /* sort order                    */
					    /* processing flags              */
    
    wchar_t     co_wc_min;		    /* min process code              */
    wchar_t     co_wc_max;		    /* max process code              */
    
    wchar_t     co_col_min;		    /* min coll weight               */
    wchar_t     co_col_max;		    /* max coll weight               */
    
    _LC_coltbl_t *co_coltbl;		    /* array of collation            */
				            /* weights, symbols              */
    unsigned
	char    co_nsubs;		    /* number of sub strs            */
    _LC_subs_t  *co_subs;		    /* substitution strs             */
    
    unsigned
	short	co_special;		    /* has multi-character collating */
					    /* elements if _LOC_HAS_MCCE'th  */
                                            /* bit is set.  (ie can not use  */
					    /* fast path collating methods)  */

    struct __LC_locale	                    /* back pointer to locale record */
                *loc_rec;	            /* ie to _LC_locale_t structure  */

    void        *__meth_ptr;                /* method extensions	     */
    void        *__data_ptr;                /* data extensions		     */

} _LC_collate_t;

typedef struct _LC_collate_objhdl _LC_collate_objhdl_t;
struct _LC_collate_objhdl {
	_LC_collate_t *obj;

#ifndef _PTR_METH
	void *(**meth)();
#endif

struct _LC_locale_objhdl *loc_hdl;	/* setlocale() assigns this to point  */
					/* to global locale handle (lc_locale)*/
					/* for current process.               */
};

/*
** MASKS for the co_sort[] sort modifier array
*/
#define _COLL_FORWARD_MASK   1
#define _COLL_BACKWARD_MASK  2
#define _COLL_NOSUBS_MASK    4
#define _COLL_POSITION_MASK  8

/* _LC_classnm_t
**
** Element mapping class name to a bit-unique mask.
*/
typedef struct {

    char    *name;
    unsigned
	int mask;

} _LC_classnm_t;


/* _LC_ctype_t
**
** Structure representing CTYPE class which defines character
** membership in a character class.
*/
typedef struct {

  _LC_core_ctype_t core;
 
  /* min and max process code */
  wchar_t      min_wc;
  wchar_t      max_wc;

  /* upper, lower translation */
  wchar_t      *upper;
  wchar_t      *lower;
  
  /* character class membership */
  unsigned
      int      *mask;        /* Array of masks for CPs 0..255   */
  unsigned
      int      *qmask;	     /* Array of masks for CPs 255..+   */
  unsigned
      char     *qidx;	     /* index into qmask for CPs 255..+ */

  /* class name mapping */
  unsigned
      char     nclasses;
 _LC_classnm_t *classnms;
  
    struct __LC_locale	     /* back pointer to locale record   */
               *loc_rec;     /* ie to _LC_locale_t structure    */

    void       *__meth_ptr;  /* method extensions	        */
    void       *__data_ptr;  /* data extensions	                */

} _LC_ctype_t;

typedef struct _LC_ctype_objhdl _LC_ctype_objhdl_t;
struct _LC_ctype_objhdl {
	_LC_ctype_t *obj;

#ifndef _PTR_METH
	void *(**meth)();
#endif

struct _LC_locale_objhdl *loc_hdl;	/* setlocale() assigns this to point  */
					/* to global locale handle (lc_locale)*/
					/* for current process.               */
};


/* _LC_locale_t
** 
** Entry point to locale database.  setlocale() receives a pointer to 
** this structure from __lc_load().
*/

/**********
** IF THIS NUMBER CHANGES, IT MUST ALSO BE CHANGED IN
** langinfo.h
**********/
#define _NL_NUM_ITEMS 63

typedef struct __LC_locale {

    _LC_core_locale_t core;
    
    char           *nl_info[_NL_NUM_ITEMS];
    struct lconv   *nl_lconv;
    
    _LC_charmap_objhdl_t  lc_charmap;
    _LC_collate_objhdl_t  lc_collate;
    _LC_ctype_objhdl_t    lc_ctype;
    _LC_monetary_objhdl_t lc_monetary;
    _LC_numeric_objhdl_t  lc_numeric;
    _LC_resp_objhdl_t     lc_resp;
    _LC_time_objhdl_t     lc_time;
    
    struct __LC_locale    *loc_rec;       /* back pointer to locale record  */
                      	                  /* ie to _LC_locale_t structure   */

    void                  *__meth_ptr;    /* method extensions              */
    void                  *__data_ptr;    /* data extensions                */

} _LC_locale_t;

typedef struct _LC_locale_objhdl _LC_locale_objhdl_t;
struct _LC_locale_objhdl {
	_LC_locale_t *obj;

#ifndef _PTR_METH
	void *(**meth)();
#endif

struct _LC_locale_objhdl *loc_hdl;	/* setlocale() assigns this to point  */
					/* to global locale handle (lc_locale)*/
					/* for current process.               */
};


/* _LC_object_handle_t
 *
 * Generic object reference.  The ptr field refers to the data for the object
 * and the context field points to the functions which implement the
 * methods for the object.
 */
typedef struct _LC_object_handle _LC_object_handle_t;
struct _LC_object_handle {
    union {
	_LC_object_t	      lc_object;
	_LC_locale_objhdl_t   lc_locale;
        _LC_charmap_objhdl_t  lc_charmap;
        _LC_collate_objhdl_t  lc_collate;
        _LC_ctype_objhdl_t    lc_ctype;
        _LC_monetary_objhdl_t lc_monetary;
        _LC_numeric_objhdl_t  lc_numeric;
        _LC_resp_objhdl_t     lc_resp;
        _LC_time_objhdl_t     lc_time;
    } obj;

#ifndef _PTR_METH
    void    *(**meth)();
#endif

struct _LC_locale_objhdl *loc_hdl;	/* setlocale() assigns this to point  */
					/* to global locale handle (lc_locale)*/
					/* for current process.               */
};


typedef struct {
    _LC_object_t	hdr;		  /* header for load object */
    _LC_locale_objhdl_t handle;		  /* handle for object  */
} _LC_load_object_t;

#define _LC_MAX_OBJECTS   256
#define _DFLT_LOC_PATH    "/usr/lib/nls/loc/"

/*
 * Global locale state pointers
 */
extern _LC_charmap_objhdl_t  __lc_charmap;
extern _LC_collate_objhdl_t  __lc_collate;
extern _LC_ctype_objhdl_t    __lc_ctype;
extern _LC_monetary_objhdl_t __lc_monetary;
extern _LC_numeric_objhdl_t  __lc_numeric;
extern _LC_resp_objhdl_t     __lc_resp;
extern _LC_time_objhdl_t     __lc_time;
extern _LC_locale_objhdl_t   __lc_locale;

#endif
