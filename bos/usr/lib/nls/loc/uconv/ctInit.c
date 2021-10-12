static char sccsid[] = "@(#)11  1.2  src/bos/usr/lib/nls/loc/uconv/ctInit.c, ils-zh_CN, bos41J, 9514A_all 3/28/95 15:12:34";
/*
 *   COMPONENT_NAME:	CMDICONV
 *
 *   FUNCTIONS:		
 *			_get_next_locpath
 *			NextToken
 * 			InitializeCharsetDB
 *			RetrieveCfgDB
 *			_iconv_FindWellKnownCharset
 *			_ctUCS_init
 *			_ctUCS_close
 *			_ctUCS_b_swap
 *			_ctUCS_reset_state
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <iconv.h>
#include <iconvP.h>
#include <uconv.h>
#include "fcs.h"
#include "ct.h"

ctCharsetDescRec utfInvalidCharset;
ctCharsetDescRec utfCharset;

/*
 * The following is the default CFG table if non
 * is found in the filesystem.
 */
				/* NOTE: ASCII is defaulted */
static uchar_t  default_cfg[] =
   "ISO8859-1-GL\n\
    ISO8859-2-GL\n\
    ISO8859-3-GL\n\
    ISO8859-4-GL\n\
    ISO8859-5-GL\n\
    ISO8859-6-GL\n\
    ISO8859-7-GL\n\
    ISO8859-8-GL\n\
    ISO8859-9-GL\n\
    JISX0208.1983-GL\n\
    JISX0201.1976-GL\n\
    IBM-udcJP-GL\n\
    KSC5601.1987-GL\n\
    GB2312.1980-0-GL\n\
    CNS11643.1986-1-GL\n\
    CNS11643.1986-2-GL\n\
    IBM-udcTW-GL\n\
    IBM-sbdTW-GL\n\
    IBM-850-GL";


/*
 *   NAME:	nextToken
 *
 *   FUNCTION:	parses for next non-space character,
 *		if comment line, skips to end of line
 *
 *   RETURNS:	None
 */
static uchar_t* nextToken( uchar_t* p)
{
	while (1) {
		for (; isspace (*p); p ++);
		if (*p == '#' ) { /* Comment line */
			for (p ++; (*p != '\n') && (*p != '\0'); p ++);
		}
		return p;
	}
}

/*
 *   NAME:      _get_next_locpath
 *
 *   FUNCTION:  Get the next path element from the given $LOCPATH.
 *              The result followed by '/' is stored in the buffer,
 *              and the pointer to LOCPATH element is advanced.
 *
 *   RETURNS:   Length of the got path element.
 */

static  int             _get_next_locpath (
        uchar_t         **locpath,      /* Pointer to LOCPATH element */
        uchar_t         *name_buf) {    /* Returns path name          */

        uchar_t         *p;
        int             len;

        p = *locpath;
        while ((*p != '\0') && (*p != ':')) p ++;
        if ((len = p - *locpath) == 0) {
                name_buf[0] = '.';
                len = 1;
        }
        else    memcpy (name_buf, *locpath, len);

        if (*p != '\0') p ++;
        *locpath = p;
        return len;
}

		
/*
 *   NAME:      FindWellKnownCharset
 *
 *   FUNCTION:  Searches for Charsets definitions that are hard coded
 *		in the libiconv library.
 *
 *   RETURNS:   EscTbl entry that is static  memory
 *
 *   NOTE:      This function should really be inside of libiconv
 *		where the ett tables are defined.
 */
EscTbl* _iconv_FindWellKnownCharset( _LC_ucs_ct_iconv_t *cd,
				uchar_t* name, size_t len)
{
	EscTblTbl* ett = cd->ett;

	while ( ett->name != NULL ) {
		EscTbl* p = ett->etbl;
		int n = ett->netbl;
		while (n--) {
		    if (     ( ! strncmp( p->name, name, len)) 
			  || ( p->name == name               )) 
			return (p);
		    ++p;
		}
		++ett;
	}
}
	
/*
 *   NAME:      FindWellKnownEscape
 *
 *   FUNCTION:  Searches for Charsets definitions using escape sequence.
 *
 *   RETURNS:   EscTbl entry that is static  memory
 *
 *   NOTE:      This function should really be inside of libiconv
 *		where the ett tables are defined.
 */
EscTbl* _iconv_FindWellKnownEscape( _LC_ucs_ct_iconv_t *cd,
 				    EscTblTbl** p_ett,
				    const uchar_t* esc, size_t len)
{
        EscTblTbl* ett = *p_ett;
        ctCharsetDesc cs;

	while ( ett->name != NULL ) {
		EscTbl* p = ett->etbl;
		int n = ett->netbl;
		while (n--) {
		    if ( ! strncmp( p->str, esc, (p->len > len ? len : p->len ))) {
			/***  check if cd already has esc seq defined ...*/
			for (cs = cd->top; cs && strcmp( p->name, cs->escape.name) ;
			                                         cs = cs->next);
			if ( cs ) continue;
			*p_ett = ett;
			return (p);
		    }	
		    ++p;
		}
		++ett;
	}
	*p_ett = ett;
        return ((EscTbl*)NULL);
}
	
/*
 *   NAME:      InitializeCharsetDB
 *
 *   FUNCTION:  Parse configuration table and build chain of 
 * 		ctCharsetDesc.
 *
 *   RETURNS:   Head of ctCharsetDesc list
 */
static
ctCharsetDesc InitializeCharsetDB(_LC_ucs_ct_iconv_t *cd, uchar_t default_cfg ) 
{
	/*
	 *	Search the configuraion table for code set names, and see
	 *	if there is a matching entry in the escape sequence table.
	 *	If not, ignore it.
	 *
	 *	Note: cd is not initialized except ett entry...
	 */

	int line = 1;
	uchar_t* p = cd->cfg_table;
	ctCharsetDesc topCharset = (ctCharsetDesc)NULL;
	ctCharsetDesc lastCharset = (ctCharsetDesc)NULL;
	ctCharsetDesc tmpCharset;
	EscTbl* escSeq;

	while (*p != '\0') {
		uchar_t* seq = NULL;
		uchar_t* cs_name;
		size_t cs_name_len;


		p = nextToken(p);
		if (*p == '\n') { ++p; continue; };
		if (*p == '\0') break;

		/* We have something */

		cs_name = p;
		cs_name_len = 0;
		for (; *p && (! isspace (*p)); p ++, cs_name_len ++);

		if ( *p == '\0' ) {
		} 
		else if ( *p == '\n' ) {
			*p++ = '\0' ;
		} 
		else {
			*p++ = '\0';

			p = nextToken(p);
			if ( *p == '\n' ) ++p;
			else if (*p == '#') 
				while ( *p != '\0' && *p != '\n') ++p;
			else if ( *p != '\0') {
				seq = p ;
				for (; ! isspace (*p); p ++);
				if ( *p == '\0' ) {
				}
				else if ( *p++ = '\n') *p++ = '\0';
				else {
					*p++ = '\0';
					while ( *p != '\0' && *p != '\n') ++p;
				}
			}
		}

		if ( (tmpCharset = (ctCharsetDesc) 
				malloc ( sizeof(ctCharsetDescRec))) == NULL) {
			return (ctCharsetDesc)NULL;
		}
		tmpCharset->conv = (iconv_t)NULL;
		tmpCharset->func = (ctUCS_Func)NULL;
		tmpCharset->next = (ctCharsetDesc)NULL;
		tmpCharset->free = True;

		if ( seq != NULL ) {
			size_t n = strlen(seq);
			tmpCharset->escape.name   = cs_name;
			tmpCharset->escape.str    = seq ;
			tmpCharset->escape.len    = n;
			if ( seq[1] == 0x25 && seq[2] == 0x2f ) {
				seq[4] = '\0';
				tmpCharset->escape.len = 4;
				tmpCharset->escape.seg = &seq[6];
				tmpCharset->escape.seglen = n - 4 - 2;
			}
			else {
				tmpCharset->escape.seg    = NULL ;
				tmpCharset->escape.seglen = 0 ;
			}
			if ( ! strncmp( cs_name + (cs_name_len - 3), 
								"-GL", 3) ) 
			       	tmpCharset->escape.gl     = True;
		    	else
			       	tmpCharset->escape.gl     = False;
		}
		else if ( (escSeq = _iconv_FindWellKnownCharset(cd, 
							cs_name, 
							cs_name_len)) 
				!= (EscTbl *)NULL ) {
			memcpy( &(tmpCharset->escape), escSeq, 
					sizeof(EscTbl));
		}
		else {
			/* Only for debug !!! */
			fprintf( stderr, "ctUCS: Cannot find charset: %s %s\n",
					cs_name, seq);
			/* Invalid entry... ignore the entire cfg_table */
			free( tmpCharset );
			if ( !default_cfg) {
				while (topCharset) {
					tmpCharset = topCharset->next;
					free(topCharset);
					topCharset = tmpCharset;
				}
				return (ctCharsetDesc)NULL;
			}
			else continue;
		}
		if ( lastCharset )
			lastCharset->next = tmpCharset;
		else
			topCharset = tmpCharset;
		lastCharset = tmpCharset;

	}	
	return ( topCharset );
}

/*
 *   NAME:	RetrieveCfgDB
 *
 *   FUNCTION:	Open, Read Cfg Table
 *
 *   RETURNS:	This function returns a pointer to a copy of the
 *		cfg table ina malloc'd buffer.
 *
 */

int RetrieveCfgDB(
	uchar_t*		*cfg_table,
	uchar_t			*t_name,
	uchar_t			*f_name) {

	uchar_t			name_buf[PATH_MAX+1], *fname, *locpath, *p;
	int			fname_len, locpath_len, def_path, file_exist,
				t_len, f_len, i, l, err_flag,
				fd;
	struct stat		stat_buf;

	
	/*
	 *	Search the configuration file.
	 */

	fname_len = strlen (UCONV_ctUCS) + strlen (UCONV_METHOD_PATH);
	if ((fname = malloc (fname_len + 1)) == NULL) {
		errno = ENOMEM;
		return -1;
	}

	/* /uconv/iso2022.cfg */
	strcpy (fname, UCONV_METHOD_PATH);
	strcat (fname, UCONV_ctUCS);

	if (!__issetuid ()) {
		locpath = getenv ("LOCPATH"); def_path = FALSE;
	} else  locpath = NULL;
	if ((locpath == NULL) || (*locpath == '\0')) {
		locpath = DEF_LOCPATH; def_path = TRUE;
	}
	else if (strcmp (locpath, DEF_LOCPATH) == 0) def_path = TRUE;

	file_exist = FALSE;
	while (TRUE) {
		if (*locpath == '\0') {
			if (!def_path) {
				locpath = DEF_LOCPATH; def_path = TRUE;
			}
			else {
				errno = EINVAL;
				break;
			}
		}
		locpath_len = _get_next_locpath (&locpath, &name_buf[0]);
		if ((locpath_len + fname_len) > PATH_MAX) {
			errno = ENAMETOOLONG;
			break;
		}
		strcpy (&name_buf[locpath_len], fname);
		if (stat (name_buf, &stat_buf) == 0) {
			file_exist = TRUE;
			break;
		}
	}
	if (!file_exist) {
		/* Use default configuration table */
		*cfg_table = NULL;
		return 0;
	}

	/*
	 *	Read the configuration table.
	 */

	if ((*cfg_table = malloc (stat_buf.st_size + 1)) == NULL) {
		errno = ENOMEM;
		free (fname);
		return -1;
	}
	if ((fd = open (name_buf, O_RDONLY)) < 0) {
		free (*cfg_table);
		free (fname);
		return -1;
	}
	else if (read (fd, *cfg_table, stat_buf.st_size) != stat_buf.st_size) {
		close (fd);
		free (*cfg_table);
		free (fname);
		return -1;
	}
	close (fd);
	free (fname);
	(*cfg_table)[stat_buf.st_size] = '\0';
	return 0;
}


/*
 *   NAME:	_ctUCS_init
 *
 *   FUNCTION:	Open UCS converter.
 *
 *   RETURNS:	This function returns pointer to a conversion descriptor to
 *		the variable pointed by 'cd' argument as described in XPG4.
 *		If failed, (_LC_ucs_iconv_t*)-1 is returned.
 *
 *   NOTE:	Codeset name of the 'unicode' or 'ISO10646' is not passed as
 *		a string, but is indicated by empty string.
 */

      	_LC_ucs_ct_iconv_t	*_ctUCS_init (
	_LC_core_iconv_t	*core,
	uchar_t			*t_name,
	uchar_t			*f_name,
	EscTblTbl		*ett) {

	uchar_t			*modifier;
	int			t_len, f_len, i, l;
	uchar_t			to_name[PATH_MAX+1];
	_LC_ucs_ct_iconv_t*	cd;
        ctCharsetDesc		asciiCharset;
	EscTbl*			asciiEscSeq;

	/*
	 *    Allocate _LC_ucs_ct_iconv_t object
         */

        if ((cd = (_LC_ucs_ct_iconv_t*) malloc (
                sizeof (_LC_ucs_ct_iconv_t))) == NULL) {
                return (_LC_ucs_ct_iconv_t*)-1;
        }

	/*
	 *    Parse the to/from names for modifiers
	 */

	if ( __iconv_parse_modifier( t_name, f_name, &cd->modifier, to_name)
		== NULL ) {
		free(cd);
                return (_LC_ucs_ct_iconv_t*)-1;
	}

	cd->ett = ett;

	/* Read in configuration table, if any... */

	if ( RetrieveCfgDB( &(cd->cfg_table), f_name, t_name ) == -1 )
		return (_LC_ucs_ct_iconv_t*)-1;

	/*
	 *	Set ASCII entry at the top of the escape sequence table.
	 * 	  NOTE: cfg_table is modified by InitializedCharsetDesc()
	 */

	if ( cd->cfg_table ) {
        	cd->top = InitializeCharsetDB( cd, False );
	}
	else cd->top = (ctCharsetDesc)NULL;

	/* Try default configuration table if custom failed...*/

	if ( cd->top == (ctCharsetDesc)NULL ) {
		if ((cd->cfg_table=malloc (sizeof(default_cfg))) == NULL) {
			errno = ENOMEM;
			return (_LC_ucs_ct_iconv_t*)-1;
		}
		sprintf( cd->cfg_table, default_cfg);
		/* memcpy( cd->cfg_table, default_cfg, sizeof(default_cfg));*/
		cd->top = InitializeCharsetDB( cd, True );
		/* The above should never fail... but */
		if ( cd->top == (ctCharsetDesc)NULL ) {
			errno = EINVAL;
			return (_LC_ucs_ct_iconv_t*)-1;
		}
	}

	/* 
	 * add an asciiCharset at the top to assure all is well...
	 */
	if ( (asciiEscSeq = _iconv_FindWellKnownCharset(cd,
                                                        NULL,
                                                        0))
                                != (EscTbl *)NULL ) {
		asciiCharset       = &cd->asciiCharsetRec;
		asciiCharset->next = cd->top;
		asciiCharset->conv = (iconv_t) NULL;
		asciiCharset->func = (ctUCS_Func)NULL; /* filled by caller*/
		asciiCharset->gl   = True;
		asciiCharset->free = False;
		memcpy( &(asciiCharset->escape), asciiEscSeq, sizeof(EscTbl));
		cd->top = asciiCharset;
	}
	else {
		errno = EINVAL;
		return (_LC_ucs_ct_iconv_t*)-1;
	}


	cd->current 			= cd->top;
	cd->gl	 			= cd->top;  /* init to ASCII */
	cd->gr	 			= &utfInvalidCharset;
	cd->object.core           	= *core;  
	cd->isctl			= ett->isctl;
	return cd;
}

/*
 *   NAME:	_ctUCS_close
 *
 *   FUNCTION:	Termination.
 *
 *   RETURNS:	0	- Successful completion.
 *		-1	- Error.
 */

       int	_ctUCS_close (_LC_ucs_ct_iconv_t *cd) {

	ctCharsetDesc	p;

	if ((cd == NULL) || (cd == (_LC_ucs_ct_iconv_t*)-1)) {
		errno = EBADF;
		return -1;
	}
	while (cd->top) {
		p = cd->top;
		cd->top = p->next;
		if (p->conv) iconv_close (p->conv);
		free(p);
	}
	if (cd->cfg_table) free(cd->cfg_table) ;

	free (cd);
	return 0;
}


/**********************************************************************
 *
 *  The following are not part of the init functions but are common 
 *  routines for both to/from ct functions.
 *
 *  Such routines should have prefix "_ctUCS"
 *
 **********************************************************************
 */
 

/*
 *   NAME:      _ctUCS_b_swap
 *
 *   FUNCTION:  Swap low and high byte of UCS-2 character.
 *
 *   RETURNS:   None
 */

void            _ctUCS_b_swap (
        uchar_t         *target,        /* Target buffer           */
        uchar_t         *source,        /* Source buffer           */
        size_t          len) {          /* Length of source string */

        uchar_t         tmp;
        size_t          i;

        len -= len & 1;

        if (target == NULL) target = source;
	for (i = 0; i < len; i += 2) {
		tmp = source[i];
		target[i] = source[i+1];
		target[i+1] = tmp;
        }
}


/*
 *   NAME:      _ctUCS_reset_state
 *
 *   FUNCTION:  Reset back to default cd and put out escape sequence
 *
 *   RETURNS:   >= TRUE - Reset occurred.
 *               = FALSE- Error.
 *
 */

       int			_ctUCS_reset_state (
	_LC_ucs_ct_iconv_t	*cd,
	uchar_t			**out_buf,
	size_t			*out_left) 
{
	size_t			len;

	cd->gl = cd->top;
	cd->gr = &utfInvalidCharset;

	/* 
	 * For converstions from ct->xxxx not outbuf modification is
	 * neccessary...
	 */
        if ( out_left == NULL ) {
		cd->current = cd->top;
		return TRUE;
	}
	else if ( cd->top == cd->current )
		/*
		 * converting to xxxx->ct, but ASCII alread set...
	 	 * assume escape sequence previously sent to outbuf
		 */
		return TRUE;
	
	/* If we are still using the default cd, return TRUE as nothing
         *  needs to be changed.  */

	cd->current = cd->top;



	/* 
	 * Else converting xxxx->ct ... so out_buf needed...
	 */
	if (out_buf == NULL) {
		errno = EINVAL;
		return FALSE;
	}

	/*
	 *	Put out default escape sequence and return.
	 */

	len  = (size_t)(cd->top->escape.len);
	if (len > *out_left) {
		errno = E2BIG; return FALSE;
	}
	memcpy (*out_buf, cd->top->escape.str, len);
	*out_buf  += len;
	*out_left -= len;
	return TRUE;
}

