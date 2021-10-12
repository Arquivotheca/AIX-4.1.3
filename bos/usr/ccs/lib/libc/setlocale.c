static char sccsid[] = "@(#)58	1.12.3.11  src/bos/usr/ccs/lib/libc/setlocale.c, libcloc, bos41J, 9519A_all 5/9/95 12:33:34";
/*
 * COMPONENT_NAME: (LIBCLOC) LIBC Locale functions
 *
 * FUNCTIONS: setlocale
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 , 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#include <sys/limits.h>
#include <sys/lc_sys.h>
#include <sys/localedef.h>
#include <sys/seg.h>
#include <langinfo.h>

#ifdef _AIX
#define LC_ALL31  		0xffff
#endif
#define _ValidCategory(c) ((c)>=LC_ALL && (c)<=LC_MESSAGES)
#define _LastCategory           LC_MESSAGES
#define _AIX32_NL_NUM_ITEMS     55

/*
 * These macros will return a pointer to a specified member
 * in _LC_load_object structure.  This is used to map
 * 3.2.3 type locales for use on 4.1 or later systems.
 *
 * _323_LOCALE_MEMBER returns a pointer to the specified member.
 *
 * _323_LOCALE_OBJ returns a pointer to the .obj of the specified
 * member.
 *
 * _323_LOCALE_METH returns a pointer to the .meth of the specified
 * member.
 *
 */

#define _323_LOCALE_MEMBER(__variable, __member) \
        ((_LC_323_locale_t *) __OBJ_DATA(((_LC_load_object_t *) \
        __variable)->handle))->__member

#define _323_LOCALE_OBJ(__variable, __member) \
        __OBJ_DATA(_323_LOCALE_MEMBER(__variable,__member))

#define _323_LOCALE_METH(__variable, __member) \
        __OBJ_METH(_323_LOCALE_MEMBER(__variable,__member))


/*
** global variables set up by setlocale()
*/
extern _LC_charmap_objhdl_t  	__lc_charmap;
extern _LC_ctype_objhdl_t   	__lc_ctype;
extern _LC_collate_objhdl_t   	__lc_collate;
extern _LC_numeric_objhdl_t   	__lc_numeric;
extern _LC_monetary_objhdl_t	__lc_monetary;
extern _LC_time_objhdl_t   	__lc_time;
extern _LC_resp_objhdl_t   	__lc_resp;
extern _LC_locale_objhdl_t   	__lc_locale;

static _LC_locale_objhdl_t *load_all_locales(char *, char *, char *);
static char *save_locale_state(int);
static char *locale_name(int category, char *name, char *lc_all);
static _LC_locale_objhdl_t *load_locale(char *name, char *locpath);
static char *saved_category_name(int category, char *name);

/*
** Static strings to record locale names used to load
** specific locale categories.
**
** This data is used to build the locale state string.
**
** This particular value for LOCALE_NAME_LEN was chosen since NAME_MAX
** is variable per filesystem, and this is a larger value.  PATH_MAX was
** considered, but the AIX value of PATH_MAX was 1024, and allocating 7K
** static structures here is a nice thing to avoid (ie, it would take an
** extra 7K for every process on the system).
** 
*/

#define LOCALE_NAME_LEN _POSIX_PATH_MAX
static char locale_state[(LOCALE_NAME_LEN+1)*6+1]="C C C C C C";
static char locale_names[][LOCALE_NAME_LEN+1]={
  "C", "C", "C", "C", "C", "C"
  };

/* 
** This is the static buffer for this process's nl_langinfo, localeconv
** data
*/

static struct lconv local_lconv;

static _LC_locale_t locale;


/*
** Template for old version 3.2.0 locale data type
*/

typedef struct {
	_LC_core_locale_t   	core;

	char		*nl_info[_AIX32_NL_NUM_ITEMS];
	struct lconv	*nl_lconv;
	
	_LC_charmap_t	*lc_charmap;
	_LC_collate_t	*lc_collate;
	_LC_ctype_t	*lc_ctype;
	_LC_monetary_t	*lc_monetary;
	_LC_numeric_t	*lc_numeric;
	_LC_resp_t	*lc_resp;
	_LC_time_t	*lc_time;
} _LC_320_locale_t;

/* Define 3.2.3 structures for compatibility */

typedef struct _LC_323_charmap_objhdl _LC_323_charmap_objhdl_t;
struct _LC_323_charmap_objhdl {
        _LC_charmap_t *obj;

#ifndef _PTR_METH
        void *(**meth)();
#endif
};


typedef struct _LC_323_collate_objhdl _LC_323_collate_objhdl_t;
struct _LC_323_collate_objhdl {
        _LC_collate_t *obj;

#ifndef _PTR_METH
        void *(**meth)();
#endif
};

typedef struct _LC_323_ctype_objhdl _LC_323_ctype_objhdl_t;
struct _LC_323_ctype_objhdl {
        _LC_ctype_t *obj;

#ifndef _PTR_METH
        void *(**meth)();
#endif
};

typedef struct _LC_323_monetary_objhdl _LC_323_monetary_objhdl_t;
struct _LC_323_monetary_objhdl {
        _LC_monetary_t *obj;

#ifndef _PTR_METH
        void *(**meth)();
#endif
};

typedef struct _LC_323_numeric_objhdl _LC_323_numeric_objhdl_t;
struct _LC_323_numeric_objhdl {
        _LC_numeric_t *obj;

#ifndef _PTR_METH
        void *(**meth)();
#endif
};

typedef struct _LC_323_resp_objhdl _LC_323_resp_objhdl_t;
struct _LC_323_resp_objhdl {
        _LC_resp_t *obj;

#ifndef _PTR_METH
        void *(**meth)();
#endif
};

typedef struct _LC_323_time_objhdl _LC_323_time_objhdl_t;
struct _LC_323_time_objhdl {
        _LC_time_t *obj;

#ifndef _PTR_METH
        void *(**meth)();
#endif
};

/*
** Template for old version 3.2.3 locale data type
*/

typedef struct {
	_LC_core_locale_t   	core;

	char		*nl_info[_AIX32_NL_NUM_ITEMS];
	struct lconv	*nl_lconv;

	_LC_323_charmap_objhdl_t  lc_charmap;
	_LC_323_collate_objhdl_t  lc_collate;
	_LC_323_ctype_objhdl_t    lc_ctype;
	_LC_323_monetary_objhdl_t lc_monetary;
	_LC_323_numeric_objhdl_t  lc_numeric;
	_LC_323_resp_objhdl_t     lc_resp;
	_LC_323_time_objhdl_t     lc_time;

} _LC_323_locale_t;


/*
** Template for old version 3.2.3 time data type
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

} _LC_323_time_t;

/*
** Template for old version 3.2.3 lc_collate data type
*/

typedef union {
    unsigned short  n[2];
    unsigned short  *p;
} _LC_323_weight_t;   

typedef struct {
    char            *ce_sym;
    _LC_323_weight_t ce_wgt;
} _LC_323_collel_t;

typedef struct {
    _LC_323_weight_t   ct_wgt;
    _LC_323_collel_t   *ct_collel;
} _LC_323_coltbl_t;

typedef struct {
    _LC_323_weight_t ss_act;
    char *ss_src;
    char *ss_tgt;
} _LC_323_subs_t;

typedef struct {
    _LC_core_collate_t core;

    unsigned char   co_nord;

    _LC_323_weight_t co_sort;

    wchar_t     co_wc_min;
    wchar_t     co_wc_max;

    wchar_t     co_col_min;
    wchar_t     co_col_max;

    _LC_323_coltbl_t *co_coltbl;
                            
    unsigned char   co_nsubs;
    _LC_323_subs_t  *co_subs;
} _LC_323_collate_t;


#define MIN(a,b) (((a)<(b))?(a):(b))

/* map a 3.x weight object to a 4.1 weight object */

void map_wgt (_LC_weight_t * new_wgt, const _LC_323_weight_t * old_wgt,
	      const int new_cnt, const int old_cnt)
{
	int i;
        for (i=0;i<=_COLL_WEIGHTS_MAX;i++)
	    new_wgt->n[i]=0;

	if (old_cnt<2) {
	    new_wgt->n[0] = old_wgt->n[0];
	    new_wgt->n[1] = old_wgt->n[1];
	    new_wgt->n[_COLL_WEIGHTS_MAX] = old_wgt->n[old_cnt];
	    }
	else {
	    for (i=0;i<=new_cnt;i++)
	        new_wgt->n[i]= old_wgt->p[i];
	    new_wgt->n[_COLL_WEIGHTS_MAX] = old_wgt->p[old_cnt];
	    }
}


/* this function maps the lc_collate structure from 3.2.x format to 4.1 */

 _LC_collate_t *map_collate(_LC_323_collate_t * old_collate,const int mblocale)
{
    static _LC_collate_t coll_buf;
    static void * coll_tbl_ptr = NULL;

    unsigned int i,j;
    int how_many_subs = 0;

    if (coll_tbl_ptr!=NULL)
	free(coll_tbl_ptr);

    /* copy simple values first */
    coll_buf.core	= old_collate->core;
    coll_buf.co_nord	= MIN(old_collate->co_nord,_COLL_WEIGHTS_MAX);
    coll_buf.co_wc_min	= old_collate->co_wc_min;
    coll_buf.co_wc_max	= old_collate->co_wc_max;
    coll_buf.co_col_min	= old_collate->co_col_min;
    coll_buf.co_col_max	= old_collate->co_col_max;

    coll_buf.co_special = 0;
	
    /* massage co_sort */
    map_wgt(&(coll_buf.co_sort),&(old_collate->co_sort),
		coll_buf.co_nord,old_collate->co_nord);

    /* allocate space for new collating table */
    coll_tbl_ptr = coll_buf.co_coltbl = (_LC_coltbl_t *) malloc 
	      (sizeof(_LC_coltbl_t)* (coll_buf.co_wc_max-coll_buf.co_wc_min+1));

    if (coll_buf.co_coltbl == NULL) {
	return (NULL);
	}
	
    /* create new collating table */
    for (i=coll_buf.co_wc_min;i<=coll_buf.co_wc_max;i++) {
        /* first do the weights */
	map_wgt(&(coll_buf.co_coltbl[i].ct_wgt),
			&(old_collate->co_coltbl[i].ct_wgt),
			coll_buf.co_nord,old_collate->co_nord);

	/* now do multi-character collating elements, if any */
	if (old_collate->co_coltbl[i].ct_collel) {
	    int num_collel=0;
	    coll_buf.co_special = _LOC_HAS_MCCE;
	    while (*(old_collate->co_coltbl[i].ct_collel[num_collel++].ce_sym)!=0);
	    coll_buf.co_coltbl[i].ct_collel = (_LC_collel_t *) malloc
					(sizeof(_LC_collel_t) * num_collel);

	    if (coll_buf.co_coltbl[i].ct_collel == NULL) {
		free(coll_buf.co_coltbl); /* half-hearted attempt to free mem*/
		coll_tbl_ptr = NULL;
		return(NULL);
		}
	    for (j=0;j<num_collel;j++) {
		coll_buf.co_coltbl[i].ct_collel[j].ce_sym =
		                  old_collate->co_coltbl[i].ct_collel[j].ce_sym;
	        map_wgt(&(coll_buf.co_coltbl[i].ct_collel[j].ce_wgt),
			&(old_collate->co_coltbl[i].ct_collel[j].ce_wgt),
			coll_buf.co_nord,old_collate->co_nord);
		    }
	    } /* ct_collel loop */
	else
	    coll_buf.co_coltbl[i].ct_collel =  (_LC_collel_t *) NULL;

	/* we are not going to handle multicharacter collating elements */
	/* with 1-to-many mapping to increase performance for the	*/
	/* following reasons:						*/
	/* 	1.  shipped 3.2.x locales did not have this construct   */
	/*	2.  3.2.x localedef did not support this properly       */
	/*	    anyway, so if we DID have locales with this, they   */
	/*	    were not defined properly, and could not have worked*/

    } /* loop for all process codes in locale */

    /*  create new substitution table */

    /* we are also not going to handle many to 1 or many to many	*/
    /* mapping if so specified in co_subs.  By default, mapping of      */
    /* multibyte characters will also not be attempted.                 */

    coll_buf.co_nsubs	= 0;
    coll_buf.co_subs 	= 0;

    /* Return massaged structure, if the old locale had no substitutions */
    /* or if we are in a multibyte locale, since we can't call mbtowc()  */
    /* yet, we can't handle the substitution mapping properly.           */

    if ((old_collate->co_nsubs==0) || (mblocale))
	return(&coll_buf);

    /* get a rough idea how many substitutions */
    for (i=0;i<old_collate->co_nsubs;i++)
	if (strlen(old_collate->co_subs[i].ss_src)==1) 
		how_many_subs++;

    coll_buf.co_subs = (_LC_subs_t *) malloc 
					(sizeof(_LC_subs_t) * how_many_subs);
    if (coll_buf.co_subs == NULL) {
	free(coll_buf.co_coltbl); /* half hearted attempt to free mem */
	coll_tbl_ptr = NULL;
	return(NULL);
	}

    /* create new subs table */
    for (i=0;i<old_collate->co_nsubs;i++) {
	int index;
	int c;

	if (strlen(old_collate->co_subs[i].ss_src)!=1)
	    continue; /* ignore if not single character */
	c=old_collate->co_subs[i].ss_src[0];
	
	if (coll_buf.co_coltbl[c].ct_wgt.n[0] != _SUB_STRING) {
	    /* create subs entry for this character */
	    index = coll_buf.co_nsubs++;
	    for (j=0;j<=_COLL_WEIGHTS_MAX;j++) {
		/* malloc mem */
		coll_buf.co_subs[index].tgt_wgt_str[j]=malloc(sizeof(short)+1); 
		if (coll_buf.co_subs[index].tgt_wgt_str[j]==NULL) {
		    free(coll_buf.co_coltbl); /* free some mem */
		    coll_tbl_ptr = NULL;
		    return(NULL);
		    }
		/* copy weights */
		coll_buf.co_subs[index].tgt_wgt_str[j][0]=
			((char *)&(coll_buf.co_coltbl[c].ct_wgt.n[j]))[0];
		coll_buf.co_subs[index].tgt_wgt_str[j][1]=
			((char *)&(coll_buf.co_coltbl[c].ct_wgt.n[j]))[1];
		coll_buf.co_subs[index].tgt_wgt_str[j][2]='\0';
	        }
	    coll_buf.co_coltbl[c].ct_wgt.n[0] = _SUB_STRING;
	    coll_buf.co_coltbl[c].ct_wgt.n[1] = index;
	    }
	else
	    index = coll_buf.co_coltbl[c].ct_wgt.n[1];

        /*  create weight string from old_collate->co_subs[i]->ss_tgt */
        /*   assuming single byte data                               */

	/* copy weights for specified orders */
	for (j=0;j<=coll_buf.co_nord;j++)
	  if (((old_collate->co_nord<2) && (old_collate->co_subs[i].ss_act.n[j]!=0))
	 || ((old_collate->co_nord>=2) && (old_collate->co_subs[i].ss_act.p[j]!=0)))
		{ /* current order is j */
		  /* coll_buf.co_subs[index].tgt_wgt_str[j] is ptr to new wgt str */
		  /* old_collate->co_subs[i].ss_tgt[] has source char str */
		  char * o_ptr, * n_ptr;
		  char character;

		  free (coll_buf.co_subs[index].tgt_wgt_str[j]);
		  coll_buf.co_subs[index].tgt_wgt_str[j]=
                       malloc(strlen(old_collate->co_subs[i].ss_tgt)*sizeof(short)+1);
		  if (coll_buf.co_subs[index].tgt_wgt_str[j]==NULL) {
		      free(coll_buf.co_coltbl); /* free some mem */
		      coll_tbl_ptr = NULL;
		      return(NULL);
		      }
		  
		  /* convert char string to weight string */
		  n_ptr = coll_buf.co_subs[index].tgt_wgt_str[j];
		  for (o_ptr=old_collate->co_subs[i].ss_tgt;*o_ptr!=0;o_ptr++) {
		      *n_ptr++=((char *)&(coll_buf.co_coltbl[*o_ptr].ct_wgt.n[j]))[0];
		      *n_ptr++=((char *)&(coll_buf.co_coltbl[*o_ptr].ct_wgt.n[j]))[1];
		   }
		   *n_ptr='\0';

	   } /* for j */
    } /* co_subs table */

    return(&coll_buf);
}


/*
** Instantiate function
*/

static _LC_load_object_t *locale_instantiate(char *path, void *(*inst)())
{

	/* 
	** Declare place to contain old format locale 
	*/
	static _LC_locale_t 		loc_buf;

        /*
        ** Declare place to contain old format time data
        */
        static _LC_time_t               time_buf;

	/* 
	** Declare a static load object structure with the  
	** header information set up correctly.		    
	*/
	static _LC_load_object_t loc_load = {
		{ _LC_OBJ_HDL, _LC_MAGIC, 
		  _LC_VERSION, sizeof(_LC_load_object_t)},
		0
		};
	_LC_320_locale_t		*old_loc;
	
	/* 
	** Case where locale not found 
	*/
	if (inst == NULL) 
		return NULL;

	/*
        ** If this appears to be a 3.2.3 or later locale object
        */

        if (((_LC_load_object_t *) inst)->hdr.__magic == _LC_MAGIC) {

            /*
            ** Case where no work need be done (same major version)
            */
            if (_LC_MAJOR_PART(((_LC_load_object_t *) inst)->hdr.__version)
                                                                 == _LC_MAJOR)
                    return inst;

            /*
            ** Case where we can't handle new major version
            */
            if (_LC_MAJOR_PART(((_LC_load_object_t *) inst)->hdr.__version)
                                                                 > _LC_MAJOR)
                return NULL;

	    /* 
            ** This is a 3.2.3 major version, need to do slight massaging:
            */
	    if (_LC_MAJOR_PART(((_LC_load_object_t *) inst)->hdr.__version)
					== _LC_MAJOR_PART(_AIX323_LC_VERSION))
		{
		/* 
		**  Core gets copied from old object, therefore old version 
		**  number will be indicated. Copy all the nl_info items 
		**  addresses and the nl_lconv buffer.
		*/
                loc_buf.core = _323_LOCALE_MEMBER(inst, core);
                memcpy(loc_buf.nl_info, _323_LOCALE_MEMBER(inst, nl_info),
                        sizeof(char*) * _AIX32_NL_NUM_ITEMS);
                loc_buf.nl_lconv = _323_LOCALE_MEMBER(inst, nl_lconv);

                /*
                **  Copy old time data into new structure
                */
                memcpy(&time_buf,_323_LOCALE_OBJ(inst, lc_time),
                        sizeof(_LC_323_time_t));

		/* 
		** Copy all subclasses object portions 
		*/
                __OBJ_DATA(loc_buf.lc_charmap)  = _323_LOCALE_OBJ(inst,lc_charmap);
                __OBJ_DATA(loc_buf.lc_collate)  = map_collate(_323_LOCALE_OBJ(inst,lc_collate),(__OBJ_DATA(loc_buf.lc_charmap)->cm_mb_cur_max!=1));
                __OBJ_DATA(loc_buf.lc_ctype)    = _323_LOCALE_OBJ(inst,lc_ctype);
                __OBJ_DATA(loc_buf.lc_monetary) = _323_LOCALE_OBJ(inst,lc_monetary);
                __OBJ_DATA(loc_buf.lc_numeric)  = _323_LOCALE_OBJ(inst,lc_numeric);
                __OBJ_DATA(loc_buf.lc_resp)     = _323_LOCALE_OBJ(inst,lc_resp);
		__OBJ_DATA(loc_buf.lc_time)	= &time_buf;

		if (__OBJ_DATA(loc_buf.lc_collate)==NULL)
			return(NULL);

#ifndef _PTR_METH
		/* 
		** If index method is being used, set up the method table 
		** for all the subclasses using the global method table  
		** located in libc.					
		*/
                __OBJ_METH(loc_buf.lc_charmap)  = _323_LOCALE_METH(inst,lc_charmap);
                __OBJ_METH(loc_buf.lc_collate)  = _323_LOCALE_METH(inst,lc_collate);
                __OBJ_METH(loc_buf.lc_ctype)    = _323_LOCALE_METH(inst,lc_ctype);
                __OBJ_METH(loc_buf.lc_monetary) = _323_LOCALE_METH(inst,lc_monetary);
                __OBJ_METH(loc_buf.lc_resp)     = _323_LOCALE_METH(inst,lc_resp);
                __OBJ_METH(loc_buf.lc_numeric)  = _323_LOCALE_METH(inst,lc_numeric);
                __OBJ_METH(loc_buf.lc_time)     = _323_LOCALE_METH(inst,lc_time);
#endif
		
		/* 
		** Build locale object handle to point to copycat locale 
		*/
		__OBJ_DATA(loc_load.handle) = &loc_buf;
#ifndef _PTR_METH
	        __OBJ_METH(loc_load.handle) =
				__OBJ_METH(((_LC_load_object_t *) inst)->handle);

#endif

		return &loc_load;
		}  /* mapping for 3.2.3 locales */
        }  /* case with accesable current _LC_MAGIC */


	/*
	** this locale object has an older major version so we have to map it
	** into a current object structure.  Currently, if we get this far, 
	** we are mapping a 3.2.0 locale into new 3.2 structure.
	** new 3.2 and 4.0 are quite similar.  Those differences are handled
	** after the information is loaded in.
	**
	** However, when additional major versions are created, obsolete
	** versions will have to be tested for explicitly, and handled here.
	*/

	
	/* 
	** Call address as function pointer, a 320 locale structure 
	** should be returned.	
	*/
	old_loc = (*inst)();

	if (old_loc == NULL) {
		return NULL;
	}
	else {	
		/* 
		**  Core gets copied from old object, therefore old version 
		**  number will be indicated. Copy all the nl_info items 
		**  addresses and the nl_lconv buffer.
		*/
		loc_buf.core = old_loc->core;
		memcpy(loc_buf.nl_info, old_loc->nl_info, 
		       sizeof(char*) * _AIX32_NL_NUM_ITEMS);
		loc_buf.nl_lconv = old_loc->nl_lconv;

                /*
                **  Copy old time data into new structure
                */
                memcpy(&time_buf,old_loc->lc_time,sizeof(_LC_323_time_t));

		/* 
		** Copy all subclasses object portions 
		*/
		__OBJ_DATA(loc_buf.lc_charmap)	= old_loc->lc_charmap;
		__OBJ_DATA(loc_buf.lc_collate)	= map_collate(old_loc->lc_collate,(old_loc->lc_charmap->cm_mb_cur_max!=1));
		__OBJ_DATA(loc_buf.lc_ctype)	= old_loc->lc_ctype;
		__OBJ_DATA(loc_buf.lc_monetary)	= old_loc->lc_monetary;
		__OBJ_DATA(loc_buf.lc_numeric)	= old_loc->lc_numeric;
		__OBJ_DATA(loc_buf.lc_resp)	= old_loc->lc_resp;
		__OBJ_DATA(loc_buf.lc_time)	= &time_buf;

		if (__OBJ_DATA(loc_buf.lc_collate)==NULL)
			return (NULL);

	
#ifndef _PTR_METH
		/* 
		** If index method is being used, set up the method table 
		** for all the subclasses using the global method table  
		** located in libc.					
		*/
		__OBJ_METH(loc_buf.lc_charmap)	= (void *) &__method_tbl;
		__OBJ_METH(loc_buf.lc_collate) 	= (void *) &__method_tbl;
		__OBJ_METH(loc_buf.lc_ctype)  	= (void *) &__method_tbl;
		__OBJ_METH(loc_buf.lc_monetary)	= (void *) &__method_tbl;
		__OBJ_METH(loc_buf.lc_resp)  	= (void *) &__method_tbl;
		__OBJ_METH(loc_buf.lc_numeric) 	= (void *) &__method_tbl;
		__OBJ_METH(loc_buf.lc_time)  	= (void *) &__method_tbl;
#endif
		

		/* 
		** Build locale object handle to point to copycat locale 
		*/
		__OBJ_DATA(loc_load.handle) = &loc_buf;
#ifndef _PTR_METH
	        __OBJ_METH(loc_load.handle) = (void *) &__method_tbl;
#endif

		return &loc_load;
	}
}
	

#define C_TEST(loc) ((using_C=(*(loc)=='C' && (loc)[1]=='\0'))||(*(loc)=='P' && strcmp(loc,"POSIX")==0))

/*
*  FUNCTION: setlocale
*
*  DESCRIPTION:
*  Loads the specified 'category' from the locale database 'locname'.
*
*  RETURNS:
*  A space separated string of names which represents the effective locale 
*  names for each of the categories affected by 'category'.
*/
char *setlocale(int category, const char *locname)
{
    extern _LC_locale_objhdl_t _C_locale; /* extern for permanently */
					/* loaded C locale data */
    
    _LC_locale_objhdl_t *lp_ptr;	/* locale temporory */
    _LC_locale_objhdl_t lp;
   
    char path[_POSIX_PATH_MAX+1];       /* buffer to hold locale */
					/* database path  */
    
    char *lc_all;			/* value of LC_ALL environment */
					/* variable  */
    
    char *lang;				/* value of LANG environment */
					/* variable  */

    int used_lang=0;			/* used for C fast path... */

    int using_C = 0;			/* usef for C fast path... */
    
    char *locpath;			/* value of LOCPATH */
					/* environment variable */
    
    char *s;			        /* temporary string pointer */
    

#ifdef _AIX
    if (category==LC_ALL31)
	category=LC_ALL;
#endif


    /* 
    ** Verify category parameter
    */
    if (!_ValidCategory(category)) 
	return NULL;
    

    /* 
    ** Check special locname parameter overload which indicates that current
    ** locale state is to be saved.
    */
    if (locname == NULL)
	return saved_category_name(category, locale_state);
    
    /* 
    ** get values of environment variables which are globally useful to
    ** avoid repeated calls to getenv() 
    */
    lc_all  = getenv("LC_ALL");
    if (lc_all==NULL) lc_all = "\0";
    lang  = getenv("LANG");
    if (lang==NULL) lang = "\0";

    if ((C_TEST(locname) || ((*locname=='\0') && (C_TEST(lc_all)
			     || (*lc_all=='\0' && (used_lang=C_TEST(lang))))))
	&& (category==LC_ALL) && (strcmp(locale_state,"C C C C C C")==0)) {
	   if (used_lang) {
		char *value;
		value=getenv("LC_MESSAGES");
		if ((value!=NULL) || (value != '\0'))
			goto I_used_goto;
		value=getenv("LC_COLLATE");
		if ((value!=NULL) || (value != '\0'))
			goto I_used_goto;
		value=getenv("LC_CTYPE");
		if ((value!=NULL) || (value != '\0'))
			goto I_used_goto;
		value=getenv("LC_MONETARY");
		if ((value!=NULL) || (value != '\0'))
			goto I_used_goto;
		value=getenv("LC_NUMERIC");
		if ((value!=NULL) || (value != '\0'))
			goto I_used_goto;
		value=getenv("LC_TIME");
		if ((value!=NULL) || (value != '\0'))
			goto I_used_goto;
	   }
	   if (using_C)
		return(locale_state);
	   else {
		int i;
		for (i=0; i<=LC_MESSAGES; i++) 
		    strcpy(locale_names[i], (*locname!='\0') ? locname :
					    ((*lc_all!='\0') ? lc_all : lang));
                return save_locale_state(category);
		}
	}

    I_used_goto:
    
    /* 
    ** get values of environment variables which are globally useful to
    ** avoid repeated calls to getenv() 
    */
    locpath = getenv("LOCPATH");
    if (locpath==NULL) locpath = "\0";
    
    /*
    ** Split logic for loading all categories versus loading a single 
    ** category.
    */
    if (category == LC_ALL) {

	/*
 	** load all locale categories
 	*/
	lp_ptr = load_all_locales(lc_all, locname, locpath);


	/*
	** If the load failed, return NULL
	*/
	if (lp_ptr == NULL) 
		return NULL;

	/*
	** Set up the global object handles for each of the categories 
	*/
	__lc_collate  = __OBJ_DATA(*lp_ptr)->lc_collate;
	__lc_ctype    = __OBJ_DATA(*lp_ptr)->lc_ctype;
	__lc_charmap  = __OBJ_DATA(*lp_ptr)->lc_charmap;
	__lc_monetary = __OBJ_DATA(*lp_ptr)->lc_monetary;
	__lc_numeric  = __OBJ_DATA(*lp_ptr)->lc_numeric;
	__lc_time     = __OBJ_DATA(*lp_ptr)->lc_time;
	__lc_resp     = __OBJ_DATA(*lp_ptr)->lc_resp;
	__lc_locale   = *lp_ptr;


	/*   note that all of these values are set to &__lc_locale          */

	__lc_charmap.loc_hdl = __OBJ_DATA(__lc_locale)->lc_charmap.loc_hdl   =
	__lc_collate.loc_hdl = __OBJ_DATA(__lc_locale)->lc_collate.loc_hdl   =
	__lc_ctype.loc_hdl   = __OBJ_DATA(__lc_locale)->lc_ctype.loc_hdl     =
	__lc_monetary.loc_hdl= __OBJ_DATA(__lc_locale)->lc_monetary.loc_hdl  =
	__lc_numeric.loc_hdl = __OBJ_DATA(__lc_locale)->lc_numeric.loc_hdl   =
	__lc_time.loc_hdl    = __OBJ_DATA(__lc_locale)->lc_time.loc_hdl      =
	__lc_resp.loc_hdl    = __OBJ_DATA(__lc_locale)->lc_resp.loc_hdl      =
	__lc_locale.loc_hdl  = &__lc_locale;


    } else {
	/*
	 ** load a specific category of locale information
	 */

	lp_ptr = load_locale(s=locale_name(category, locname, lc_all), locpath);

	/*
	** If the load failed, return NULL
	*/
	if (!lp_ptr) 
		return NULL;

	/* 
  	** setup for building of locale save string 
	*/
	strcpy(locale_names[category], s);

        /* 
	** Set up the static writable locale to be used
	*/
	locale.lc_charmap       = __lc_charmap;
	locale.lc_collate       = __lc_collate;
	locale.lc_ctype         = __lc_ctype;
	locale.lc_monetary      = __lc_monetary;
	locale.lc_numeric       = __lc_numeric;
	locale.lc_time          = __lc_time;
	locale.lc_resp          = __lc_resp;
	locale.nl_lconv		= &local_lconv;

	/*
	** Correctly set up the object and meth (if needed) portions
	** of the locale. First statement is to assign the meth section
	** if needed, without adding another ifndef _PTR_METH. 
	*/
	__lc_locale = *lp_ptr;
	locale.core = __OBJ_DATA(*lp_ptr)->core;
	__OBJ_DATA(__lc_locale) = &locale;
	__lc_locale.loc_hdl  = &__lc_locale;


	/*
	** Correctly set up the information for the category loaded.
	*/
	switch (category) {
	  case LC_COLLATE:
	    locale.lc_collate 	= __OBJ_DATA(*lp_ptr)->lc_collate;
	    locale.lc_charmap 	= __OBJ_DATA(*lp_ptr)->lc_charmap;
	    __lc_collate      	= __OBJ_DATA(*lp_ptr)->lc_collate;
	    __lc_charmap      	= __OBJ_DATA(*lp_ptr)->lc_charmap;
	    __lc_charmap.loc_hdl= __OBJ_DATA(__lc_locale)->lc_charmap.loc_hdl =
	    __lc_collate.loc_hdl= __OBJ_DATA(__lc_locale)->lc_collate.loc_hdl
				= &__lc_locale;

	    break;
	  case LC_CTYPE:
	    locale.lc_ctype 	= __OBJ_DATA(*lp_ptr)->lc_ctype;
	    locale.lc_charmap 	= __OBJ_DATA(*lp_ptr)->lc_charmap;
	    __lc_ctype 		= __OBJ_DATA(*lp_ptr)->lc_ctype;
	    __lc_charmap 	= __OBJ_DATA(*lp_ptr)->lc_charmap;
	    __lc_charmap.loc_hdl= __OBJ_DATA(__lc_locale)->lc_charmap.loc_hdl =
	    __lc_collate.loc_hdl= __OBJ_DATA(__lc_locale)->lc_collate.loc_hdl
				= &__lc_locale;
	    break;
	  case LC_MONETARY:
	    locale.lc_monetary 	= __OBJ_DATA(*lp_ptr)->lc_monetary;
	    __lc_monetary 	= __OBJ_DATA(*lp_ptr)->lc_monetary;
	    __lc_monetary.loc_hdl= __OBJ_DATA(__lc_locale)->lc_monetary.loc_hdl
				= &__lc_locale;
	    break;
	  case LC_NUMERIC:
	    locale.lc_numeric 	= __OBJ_DATA(*lp_ptr)->lc_numeric;
	    __lc_numeric 	= __OBJ_DATA(*lp_ptr)->lc_numeric;
	    __lc_numeric.loc_hdl= __OBJ_DATA(__lc_locale)->lc_numeric.loc_hdl
				= &__lc_locale;
	    break;
	  case LC_TIME:
	    locale.lc_time 	    = __OBJ_DATA(*lp_ptr)->lc_time;
	    locale.nl_info[NLLDATE] = __OBJ_DATA(*lp_ptr)->nl_info[NLLDATE];
	    locale.nl_info[NLTMISC] = __OBJ_DATA(*lp_ptr)->nl_info[NLTMISC];
	    locale.nl_info[NLTSTRS] = __OBJ_DATA(*lp_ptr)->nl_info[NLTSTRS];
	    locale.nl_info[NLTUNITS]= __OBJ_DATA(*lp_ptr)->nl_info[NLTUNITS];
	    locale.nl_info[NLYEAR]  = __OBJ_DATA(*lp_ptr)->nl_info[NLYEAR];
	    __lc_time 		    = __OBJ_DATA(*lp_ptr)->lc_time;
	    __lc_time.loc_hdl       = __OBJ_DATA(__lc_locale)->lc_time.loc_hdl
				    = &__lc_locale;

	    break;
	  case LC_MESSAGES:
	    locale.lc_resp 	= __OBJ_DATA(*lp_ptr)->lc_resp;
	    __lc_resp 		= __OBJ_DATA(*lp_ptr)->lc_resp;
	    __lc_resp.loc_hdl   = __OBJ_DATA(__lc_locale)->lc_resp.loc_hdl
			        = &__lc_locale;
	}

    }

    /* 
    ** In all cases (load all categories or just a single one), if the init 
    ** method is not NULL, initialize the locale
    */

    if (__OBJ_DATA(__lc_locale)->core.__init != -1)
  	_CALLMETH(__lc_locale,__init)(__lc_locale);

    return save_locale_state(category);
}


/*
*  FUNCTION: save_locale_state
*
*  DESCRIPTION:
*  Saves the settings of the locale names for each of the locale categories
*
*  RETURNS:
*  Space separated string of locale names.
*/
static char *save_locale_state(int category) 
{
    
	sprintf(locale_state, 
		"%s %s %s %s %s %s",
		locale_names[LC_COLLATE],
		locale_names[LC_CTYPE],
		locale_names[LC_MONETARY],
		locale_names[LC_NUMERIC],
		locale_names[LC_TIME],
		locale_names[LC_MESSAGES]);
	if (category == LC_ALL)
		return locale_state;
	else
		return locale_names[category];
}


/*
*  FUNCTION: load_all_locales
*
*  DESCRIPTION:
*  loads all of the locales from the appropriate locale databases.
*
*  RETURNS:
*  A pointer to a static locale handle in the user's data segment which 
*  refers to the locale object built by loading all locale categories 
*  as requried.
*
*  This function sets up the locale name for each of the categories
*/
static _LC_locale_objhdl_t *load_all_locales(char *lc_all, char *locname, 
					    char *locpath)
{
    extern _LC_locale_objhdl_t _C_locale;
    static _LC_locale_objhdl_t lp;
    _LC_locale_objhdl_t *lp_ptr;
    char *s;
    char save_str[_POSIX_PATH_MAX+1];
    int i;
    
	/*
	** Build locale piecemeal from each of the categories
	*/

	s=locale_name(LC_COLLATE, locname, lc_all);
	strcpy(save_str, s);
	lp_ptr = load_locale(s, locpath);
	if (!lp_ptr) 
	    return lp_ptr;
	strcpy(locale_names[LC_COLLATE], s);
	locale.lc_collate = __OBJ_DATA(*lp_ptr)->lc_collate;
	
	s=locale_name(LC_CTYPE, locname, lc_all);
	if (strcmp(s, save_str)!=0) {
	    strcpy(save_str, s);
	    lp_ptr = load_locale(s, locpath);
	    if (!lp_ptr) 
		return lp_ptr;
	}
	strcpy(locale_names[LC_CTYPE], s);
	locale.lc_ctype = __OBJ_DATA(*lp_ptr)->lc_ctype;
	locale.lc_charmap = __OBJ_DATA(*lp_ptr)->lc_charmap;
	
	s=locale_name(LC_MONETARY, locname, lc_all);
	if (strcmp(s, save_str) != 0) {
	    strcpy(save_str, s);
	    lp_ptr = load_locale(s, locpath);
	    if (!lp_ptr) 
		return lp_ptr;
	}
	strcpy(locale_names[LC_MONETARY], s);
	locale.lc_monetary = __OBJ_DATA(*lp_ptr)->lc_monetary;
	
	s=locale_name(LC_NUMERIC, locname, lc_all);
	if (strcmp(s, save_str) != 0) {
	    strcpy(save_str, s);
	    lp_ptr = load_locale(s, locpath);
	    if (!lp_ptr) 
		return lp_ptr;
	}
	strcpy(locale_names[LC_NUMERIC], s);
	locale.lc_numeric = __OBJ_DATA(*lp_ptr)->lc_numeric;
	
	s=locale_name(LC_TIME, locname, lc_all);
	if (strcmp(s, save_str) != 0) {
	    strcpy(save_str, s);
	    lp_ptr = load_locale(s, locpath);
	    if (!lp_ptr) 
		return lp_ptr;
	}
	strcpy(locale_names[LC_TIME], s);
	locale.lc_time 		  = __OBJ_DATA(*lp_ptr)->lc_time;
	locale.nl_info[NLLDATE]   = __OBJ_DATA(*lp_ptr)->nl_info[NLLDATE];
	locale.nl_info[NLTMISC]   = __OBJ_DATA(*lp_ptr)->nl_info[NLTMISC];
	locale.nl_info[NLTSTRS]   = __OBJ_DATA(*lp_ptr)->nl_info[NLTSTRS];
	locale.nl_info[NLTUNITS]  = __OBJ_DATA(*lp_ptr)->nl_info[NLTUNITS];
	locale.nl_info[NLYEAR]    = __OBJ_DATA(*lp_ptr)->nl_info[NLYEAR];

	s=locale_name(LC_MESSAGES, locname, lc_all);
	if (strcmp(s, save_str) != 0) {
	    strcpy(save_str, s);
	    lp_ptr = load_locale(s, locpath);
	    if (!lp_ptr) 
		return lp_ptr;
	}
	strcpy(locale_names[LC_MESSAGES], s);
	locale.lc_resp = __OBJ_DATA(*lp_ptr)->lc_resp;
	
 	/* 
      	** Set up core part of locale container object 
    	*/
	locale.core = __OBJ_DATA(*lp_ptr)->core;
	locale.nl_lconv         = &local_lconv;

	lp = *lp_ptr;
	__OBJ_DATA(lp) = &locale;

	return &lp;
}


/*
*  FUNCTION: locale_name
*
*  DESCRIPTION:
*  This function returns the name which should be used to load a 
*  locale category given the 'name' parameter passed to setlocale() 
*  the value of the LC_ALL environment variable.
*
*  RETURNS:
*  Name for the specified category.
*/
char *locale_name(int category, char *name, char *lc_all)
{
    char *env_name;
    
    static const char *category_name[]={
	"LC_COLLATE", "LC_CTYPE", "LC_MONETARY", "LC_NUMERIC", 
	"LC_TIME",  "LC_MESSAGES"
	};
    
    /* 
    ** specified locale name overrides all environment variables 
    */
    if (name[0] != '\0')
	/* 
	** check if name is a saved locale state string.
	*/
	return saved_category_name(category, name);
    
    /*
    ** The value of LC_ALL overrides all other environment variables
    */
    if (lc_all[0] != '\0') return lc_all;
    
    /* 
    ** check environment variable for specified category
    */
    env_name = getenv(category_name[category]);
    
    /* 
    ** use it if it is specified, otherwise use value of LANG
    */
    if (env_name==NULL || env_name[0] == '\0')
	env_name = getenv("LANG");
    
    /*
    ** if even LANG is undefined, then use "C"
    */
    if (env_name==NULL || env_name[0]=='\0')
	env_name = "C";
    
    return env_name;
}


/*
*  FUNCTION: saved_category_name
*
*  DESCRIPTION: 
*  Returns the locale name associated with 'category'.  If 'name' is a saved
*  category string then there will be list of locale names separated by 
*  space characters.  This is decomposed by using strchr() until the 
*  category(th) string is found.  If there are no spaces in the string, 
*  then the 'name' is returned.
*
*  RETURNS:
*  locale name from 'name' string.
*/
char *saved_category_name(int category, char *name)
{
    static char saved_name[LOCALE_NAME_LEN+1];
    char *endptr, *s;
    int  i, len;
    
    endptr = name;
    if (category != LC_ALL) {
	
	for (i=0; i<category; i++) {
	    endptr = strchr(endptr, ' ');
	    if (endptr == NULL) 
		break;
	    endptr++;
	}
	
	if (endptr != NULL) {

	    s = strchr(endptr, ' ');
	    if (s==NULL) 
		len = LOCALE_NAME_LEN;
	    else
		len = s - endptr;

	} else {
	    endptr = name;
	    len = LOCALE_NAME_LEN;
	}

	strncpy(saved_name, endptr, len);
	saved_name[len] = NULL;
	
    } else
	strncpy(saved_name, name, LOCALE_NAME_LEN);
    
    return saved_name;
}


/* 
*  FUNCTION: load_locale 
*
*  DESCRIPTION:
*  This function loads the locale specified by the locale name 'locale',
*  from the list of pathnames provided in 'locpath'.
*
*  RETURNS:
*  Pointer to locale database found in 'locpath'.
*/
static _LC_locale_objhdl_t *load_locale(char *name, char *locpath)
{
    extern _LC_locale_objhdl_t  _C_locale;
    
    static _LC_locale_objhdl_t *handle_ptr = NULL;
    static _LC_locale_t *locale_ptr = NULL;

    char path[_POSIX_PATH_MAX+1];
    char *p;

    _LC_load_object_t *load_ptr = NULL;
    

    /*
    ** Check if this is a setuid program.  If so, strip the 'name' of
    ** any path and set locpath to the default system repository.
    */
    if (__issetuid()) {
	char *s;
	
	for (s=name+strlen(name)-1; *s != '/' && s != name; s--);
	name = s;

	locpath = _DFLT_LOC_PATH;
    }
    /*
    ** If locpath is not set, use the default locale path
    */
    if (locpath[0] == '\0')
	locpath = _DFLT_LOC_PATH;

    /* 
    ** Check if the locale requested contains any '/' characters which
    ** indicate that the value is a path name.  If locale is a pathname,
    ** then load this pathname directly.  If locale is not a pathname,
    ** then parse locpath into paths and search each one for locale.
    */
    if (strchr(name, '/') == NULL) {

	if ((name[0] == 'C' && name[1] == '\0') || (strcmp(name,"POSIX")==0)){
	    return &_C_locale;
	}
	else {
	    while (load_ptr == NULL && *locpath != '\0') {

		int i;

		for (i=0; *locpath != ':' && *locpath != '\0';i++,locpath++)
		    path[i] = *locpath;

		if (*locpath==':')
		    locpath++;

		/* 
		** append '/' 
		*/
		path[i++] = '/';
		path[i] = '\0';

		/* 
		** append locale name 
		*/
		strcat(path, name);
		    
		load_ptr = __lc_load(path, locale_instantiate);
		/*
		** If the load_ptr is not NULL, see if it is the
		** correct object type. If load_ptr is NULL, see if
		** the locpath is also NULL, if it is, then return a
		** NULL.
		*/
		if (load_ptr != NULL) {
		    	if (load_ptr->hdr.__type_id != _LC_OBJ_HDL)
				return NULL;
		}
		else if (*locpath == NULL) {
			return NULL;
		}
	    } 
	}

    } else {
	load_ptr = __lc_load(name,locale_instantiate);
	if (load_ptr == NULL || load_ptr->hdr.__type_id != _LC_OBJ_HDL)
		return NULL;
    }
    
   #ifndef _PTR_METH
   /* re: defect 84106:  values of __METHOD_TBL generated by localedef    */
   /* need to be replaced with the address of __method_tbl.  This is done */
   /* to prevent the loader from causing multiple copies of the same      */
   /* locale from being loaded in...                                      */

   /* however, we can't directly modify data stored in R/O memory, so we */
   /* will make local copies of necessary data structures so that we can */
   /* modify them...                                                     */

   if (handle_ptr != NULL) {
	free(handle_ptr);
	handle_ptr = NULL;
	}

   if (locale_ptr != NULL) {
	free(locale_ptr);
	locale_ptr = NULL;
	}

   if ((__OBJ_METH(load_ptr->handle)==__METHOD_TBL) ||
   (__OBJ_METH(load_ptr->handle.obj->lc_charmap)==__METHOD_TBL) ||
   (__OBJ_METH(load_ptr->handle.obj->lc_collate)==__METHOD_TBL) ||
   (__OBJ_METH(load_ptr->handle.obj->lc_ctype)==__METHOD_TBL) ||
   (__OBJ_METH(load_ptr->handle.obj->lc_monetary)==__METHOD_TBL) ||
   (__OBJ_METH(load_ptr->handle.obj->lc_numeric)==__METHOD_TBL) ||
   (__OBJ_METH(load_ptr->handle.obj->lc_resp)==__METHOD_TBL) ||
   (__OBJ_METH(load_ptr->handle.obj->lc_time)==__METHOD_TBL)) {
      handle_ptr=(_LC_locale_objhdl_t *)malloc(sizeof(_LC_locale_objhdl_t));
      handle_ptr->obj = load_ptr->handle.obj;
      handle_ptr->meth = load_ptr->handle.meth;
      }


   if (__OBJ_METH(load_ptr->handle)==__METHOD_TBL)
      handle_ptr->meth = (void *) &__method_tbl;

   if ((__OBJ_METH(load_ptr->handle.obj->lc_charmap)==__METHOD_TBL) ||
   (__OBJ_METH(load_ptr->handle.obj->lc_collate)==__METHOD_TBL) ||
   (__OBJ_METH(load_ptr->handle.obj->lc_ctype)==__METHOD_TBL) ||
   (__OBJ_METH(load_ptr->handle.obj->lc_monetary)==__METHOD_TBL) ||
   (__OBJ_METH(load_ptr->handle.obj->lc_numeric)==__METHOD_TBL) ||
   (__OBJ_METH(load_ptr->handle.obj->lc_resp)==__METHOD_TBL) ||
   (__OBJ_METH(load_ptr->handle.obj->lc_time)==__METHOD_TBL)) {
      locale_ptr = (_LC_locale_t *) malloc (sizeof(_LC_locale_t));
      memcpy(locale_ptr, load_ptr->handle.obj,
      sizeof(_LC_locale_t));
      handle_ptr->obj = locale_ptr;
      }

   if (__OBJ_METH(load_ptr->handle.obj->lc_charmap)==__METHOD_TBL)
      __OBJ_METH(locale_ptr->lc_charmap) = (void *) &__method_tbl;

   if (__OBJ_METH(load_ptr->handle.obj->lc_collate)==__METHOD_TBL)
      __OBJ_METH(locale_ptr->lc_collate) = (void *) &__method_tbl;

   if (__OBJ_METH(load_ptr->handle.obj->lc_ctype)==__METHOD_TBL)
      __OBJ_METH(locale_ptr->lc_ctype) = (void *) &__method_tbl;

   if (__OBJ_METH(load_ptr->handle.obj->lc_monetary)==__METHOD_TBL)
      __OBJ_METH(locale_ptr->lc_monetary) = (void *) &__method_tbl;

   if (__OBJ_METH(load_ptr->handle.obj->lc_numeric)==__METHOD_TBL)
      __OBJ_METH(locale_ptr->lc_numeric) = (void *) &__method_tbl;

   if (__OBJ_METH(load_ptr->handle.obj->lc_resp)==__METHOD_TBL)
      __OBJ_METH(locale_ptr->lc_resp) = (void *) &__method_tbl;

   if (__OBJ_METH(load_ptr->handle.obj->lc_time)==__METHOD_TBL)
      __OBJ_METH(locale_ptr->lc_time) = (void *) &__method_tbl;

   #else

	" ATTENTION PLEASE: "

	/* Yes, the line above is supposed to stop a compile if _PTR_METH
         * is on.  This is because code must be added here to replace
         * the constant __METHOD_TBL with the pointer __method_tbl, as is 
         * done above for the index method.  If this code is not added,
         * occurrences of __METHOD_TBL (-1) will be interpreted as a pointer
         * to a table, and will likely cause a run time core dump.
         */

   #endif

   if (handle_ptr)
      return (handle_ptr);
   else
      return (&load_ptr->handle);
}

