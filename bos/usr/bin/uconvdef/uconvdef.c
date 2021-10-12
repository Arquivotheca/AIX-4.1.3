static char sccsid[] = "@(#)05  1.6  src/bos/usr/bin/uconvdef/uconvdef.c, cmdiconv, bos41J, 9520A_all 5/9/95 12:46:30";
/*
 *   COMPONENT_NAME:    CMDICONV
 *
 *   FUNCTIONS:
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

#define _ILS_MACROS
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <fcntl.h>
#include <nl_types.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <ctype.h>

#include "uc_convP.h"
#include "uconvdef_msg.h"

/* Defines and macros -------------------------------------------------------*/

#define BLOCKS_IN_TABLE		256
#define ENTRIES_IN_BLOCK	256
#define ENTRIES_IN_TABLE	(ENTRIES_IN_BLOCK * BLOCKS_IN_TABLE)
#define U2M_SIZE		(sizeof (ushort_t)  * ENTRIES_IN_TABLE)
#define M2U_SIZE		(sizeof (UniChar)   * ENTRIES_IN_TABLE)
#define	MBCS_U2M_SIZE		(sizeof (_uc_u2m_t) * ENTRIES_IN_TABLE)

#define SKIP_TO_EOL		{ g_index = LINE_MAX; ungetToken (EOL); }

/* Definition of the tokens -------------------------------------------------*/

typedef enum {
	E_O_F = 256	,EOL		,CODE_SET_NAME	,CHAR_NAME_MASK	,
	MB_CUR_MAXX	,MB_CUR_MIN	,UCONV_CLASS	,SUBCHAR	,
	LOCALE		,COMMENT_CHAR	,ESCAPE_CHAR	,START_CHARMAP	,
	END_CHARMAP	,UNICHAR	,UNASSIGNED	,STRING		,
	NUMBER		,HEX_CONST	,ELLIPSIS	,SPACES		,
	UNKNOWN		,ERROR
} TOKENS;

typedef struct {
	uchar_t		*name;
	TOKENS		token;
} _tokenName_t;

static	_tokenName_t	g_tokenTable[] = {
	{ "CODE_SET_NAME"	,CODE_SET_NAME  },
	{ "CHAR_NAME_MASK"	,CHAR_NAME_MASK },
	{ "MB_CUR_MAX"		,MB_CUR_MAXX    },
	{ "MB_CUR_MIN"		,MB_CUR_MIN     },
	{ "UCONV_CLASS"		,UCONV_CLASS	},
	{ "SUBCHAR"		,SUBCHAR	},
	{ "LOCALE"		,LOCALE		},
	{ "COMMENT_CHAR"	,COMMENT_CHAR	},
	{ "ESCAPE_CHAR"		,ESCAPE_CHAR	},
	{ "UNASSIGNED"		,UNASSIGNED     },
	{ NULL			,UNKNOWN        }
};

/* Definition of the UCONV_CLASS --------------------------------------------*/

typedef struct {
	uchar_t		*name;
	ushort_t	class;
} _uconvClass_t;

static	_uconvClass_t	g_uconvClass[] = {
	{ "SBCS"		,UC_CLASS_SBCS            },
	{ "DBCS"		,UC_CLASS_DBCS            },
	{ "MBCS"		,UC_CLASS_MBCS            },
	{ "EBCDIC_STATEFUL"	,UC_CLASS_EBCDIC_STATEFUL },
	{ NULL			,UC_CLASS_INVAL           }
};

/* Global areas -------------------------------------------------------------*/

static	uchar_t		g_lineBuf[LINE_MAX+1];	/* Physical line buffer      */
static	int		g_index = LINE_MAX;	/* Char index in the line    */
static	TOKENS		g_ungetTokenBuf = ERROR;/* Ungot token buffer        */
static	uchar_t		g_commentChar   = '#';	/* Comment character         */
static	uchar_t		g_escapeChar    = '\\';	/* Escape  character         */
static	ushort_t	g_mbMin         = 1;	/* MB_CUR_MIN value          */
static	ushort_t	g_mbMax         = 1;	/* MB_CUR_MAX value          */

static	uchar_t		g_inputFile [PATH_MAX+1] = {'\0'};
static	uchar_t		g_outputFile[PATH_MAX+1] = {'\0'};
static	uchar_t		g_outTmpFile[PATH_MAX+1] = {'\0'};

static	FILE		*g_pInputFile  = NULL;	/* Input  file descriptor    */
static	FILE		*g_pOutTmpFile = NULL;	/* Output file descriptor    */

static	_ucmap_hdr_t	g_hdr;			/* Conversion table header   */
static	UniChar		*g_U2MTable    = NULL;	/* Conversion table U2M      */
static	UniChar		*g_M2UTable    = NULL;	/* Conversion table M2U      */

/* Conversion tables for MBCS -----------------------------------------------*/

static	_uc_u2m_t	*g_mbcsU2M     = NULL;	/* Conversion table U2M      */
static	_uc_row_t	*g_mbcsRows    = NULL;	/* Conversion rows           */
static	ulong_t		 g_mbcsRowSize = 0L;	/* Total size of the rows    */
static	_uc_stem_t	*g_mbcsStems   = NULL;	/* Stem table                */

/* MBCS plane information block ---------------------------------------------*/

typedef struct {
	uchar_t		stem[STEM_MAX+1];	/* Stem string               */
	ushort_t	stem_len;		/* Stem length               */
	uchar_t		n_rows;			/* Number of rows in plane   */
	uchar_t		l_row;			/* Lowest row byte value     */
	uchar_t		n_cols;			/* Number of cols in plane   */
	uchar_t		l_col;			/* Lowest col byte value     */
} _planeInfo_t;

static	_planeInfo_t	*g_mbcsPlanes = NULL;	/* MBCS plane info table     */
static	int		g_nPlanes = 0;		/* Number of MBCS planes     */

#define	PLANE_MAX_NUM	255
#define PLANE_MAX_SIZE	(sizeof (_planeInfo_t) * PLANE_MAX_NUM)

/* For interface of the getToken() function ---------------------------------*/

static	uchar_t		g_string[LINE_MAX+1];	/* STRING    value           */
static	ushort_t	g_number;		/* NUMBER    value           */
static	UniChar		g_unichar;		/* UNICHAR   value           */
static	uchar_t		g_hexStr[STEM_MAX+3];	/* HEX_CONST value           */
static	ushort_t	g_hexLen;		/* HEX_CONST value length    */

/* For displaying messages --------------------------------------------------*/

#define M_NO_ERROR	0
#define M_OTHER_ERROR	-1

static	int		g_verboseFlag = FALSE;	/* Verbose option flag       */
static	int		g_lineNumber  = 0;	/* Source file line number   */
static	int		g_last_index  = 0;	/* Index of the last token   */
static	nl_catd		g_catd;			/* Message catalog descriptor*/
static	uchar_t		*g_defaultMsg[] = {	/* Default error messages    */

/*Dumnny for index 0    */"",
/*M_USAGE               */"Usage: uconvdef [-f source_file] [-v] uconv_table\n",
/*M_START_PROCESSING    */"Start processing the source file '%s'.\n",
/*M_COMPLETED           */"%s created.\n",
/*M_UNCOMPLETED         */"Output file '%s' was not created.\n",
/*M_CANNOT_ALLOCATE     */"Not enough space for allocation.\n",
/*M_CANNOT_OPEN_INPUT   */"Cannot open input file '%s': %s.\n",
/*M_CANNOT_OPEN_TEMP    */"Cannot open temporary file: %s.\n",
/*M_CANNOT_CLOSE_INPUT  */"Cannot close input file '%s': %s.\n",
/*M_CANNOT_CLOSE_TEMP   */"Cannot close temporary file: %s.\n",
/*M_ERROR_RENAME        */"Cannot rename temporary file to output file '%s': %s.\n",
/*M_ERROR_READ          */"Error while reading input file '%s': %s.\n",
/*M_ERROR_WRITE         */"Error while writing temporary file: %s.\n",
/*M_INVAL_MB_CUR        */"\'MB_CUR_MIN\' is greater than \'MB_CUR_MAX\' default.\n",
/*M_INVAL_SPACE         */"Line %4.4d: space at the line top is not allowed.\n",
/*M_INVAL_FORMAT        */"Line %4.4d: illegal line format.\n",
/*M_INVAL_TOKEN         */"Line %4.4d: illegal token.\n",
/*M_INVAL_TOKEN_VALUE   */"Line %4.4d: illegal value for token '%s'.\n",
/*M_UNDEF_TOKEN_VALUE   */"Line %4.4d: token '%s' must have value.\n",
/*M_EXTRA_CS_NAME       */"Line %4.4d: code set name can be defined only once.\n",
/*M_EXTRA_UCONV_CLASS   */"Line %4.4d: \'UCONV_CLASS\' can be defined only once.\n",
/*M_EXTRA_CHARMAP       */"Line %4.4d: CHARMAP section is already started.\n",
/*M_EXTRA_CHARS         */"Line %4.4d: extra characters.\n",
/*M_CONFLICT_UCONV_CLASS*/"Line %4.4d: conflict with \'UCONV_CLASS\'.\n",
/*M_CONFLICT_MB_CUR     */"Line %4.4d: conflict with \'MB_CUR_MIN\' or \'MB_CUR_MAX\'.\n",
/*M_CONFLICT_LEN        */"Line %4.4d: conflicting code length.\n",
/*M_OVER_BYTE_RANGE     */"Line %4.4d: range exceeds limits of one byte.\n",
/*M_TOO_MANY_PLANES     */"Line %4.4d: too many planes.\n",
/*M_ESCAPE_COMMENT      */"Line %4.4d: escape and comment characters must be distinct.\n",
/*M_MISSING_CS_NAME     */"No code set name is defined.\n",
/*M_MISSING_UCONV_CLASS */"No \'UCONV_CLASS\' is defined.\n",
/*M_MISSING_CHARMAP     */"CHARMAP section must start with \'CHARMAP\'.\n",
/*M_MISSING_CHARMAP_END */"CHARMAP section has no \'END CHARMAP\'.\n"
};

/* Prototypes of internal sub routines --------------------------------------*/

static	int	processSource1 ();
static	int	processSource2 ();
static	int	processSource3 ();
static	int	completeOutput ();
static	void	terminate   (int);

static	int	getUconvClass  (uchar_t*,ushort_t*);
static	int	getUnichar     (uchar_t*,ushort_t*,ushort_t*,UniChar*,UniChar*);
static	int	getUnassigned  (uchar_t*,ushort_t*,ushort_t*,ushort_t*);
static  int	makeTableEntry (uchar_t*,ushort_t ,ushort_t ,UniChar ,UniChar );
static	int	freeTableEntry (uchar_t*,ushort_t ,ushort_t ,ushort_t );
static	int	setPlaneInfo   (uchar_t*,ushort_t);
static	int	getPlaneIndex  (uchar_t*,ushort_t);
static	int	set_mbcsM2U    (uchar_t*,ushort_t ,UniChar, int);
static	int	setCommentChar (TOKENS);
static	int	checkUntilEOL  ();

static	uchar_t	findUndefChar  ();
static	int	canBeUndefChar (int);

static	TOKENS	str2token  (uchar_t*);
static	uchar_t	*token2str (TOKENS);
static	void	ungetToken (TOKENS);
static	TOKENS	getToken   ();
static	int	getChar    ();

static	void	displayMsg (int, uchar_t*, uchar_t*);

/*
 * 	main
 */

int	main (int argc, char *argv[]) {

	extern int	optind;
	extern char	*optarg;
	int		flag, ret;

	setlocale (LC_ALL, "");
	g_catd = catopen (MF_UCONVDEF, NL_CAT_LOCALE);

	/*
	 *	Get line parameters.
	 *	f	- Source file name.
	 *	v	- Verbose option.
	 */

	while ((flag = getopt (argc, argv, "f:M:C:v")) != EOF) {
		switch (flag) {
		case 'f': strcpy (g_inputFile,  optarg); break;
		case 'v': g_verboseFlag = TRUE;          break;
		case '?':
		default : displayMsg (M_USAGE, NULL, NULL);
			  exit (M_USAGE);
		}
	}
	if (optind != argc - 1) {
		displayMsg (M_USAGE, NULL, NULL);
		exit (M_USAGE);
	}
	strcpy (g_outputFile, argv[optind]);

	/*
	 *	Open the source file.
	 */

	if (g_inputFile[0] == '\0')
		  g_pInputFile = stdin;
	else if ((g_pInputFile = fopen (g_inputFile, "r")) == NULL) {
		ret = M_CANNOT_OPEN_INPUT;
		displayMsg (ret, g_inputFile, strerror (errno));
		goto Exit;
	}

	/*
	 *	Create a temporary output file.
	 */

	sprintf (g_outTmpFile, "%s.uconvdef.tmp%05d",
		 g_outputFile, (time (NULL) % 100000L));
	if ((g_pOutTmpFile = fopen (g_outTmpFile, "wb")) == NULL) {
		ret = M_CANNOT_OPEN_TEMP;
		displayMsg (ret, strerror (errno), NULL);
		goto Exit;
	}

	/*
	 *	Process the source file from the 1st line to the line
	 *	just before charmap section, and make the header part.
	 */

	if ((ret = processSource1 ()) != M_NO_ERROR) goto Exit;

	/*
	 *	Process charmap section, and make the conversion tables.
	 */

	if ((ret = processSource2 ()) != M_NO_ERROR) goto Exit;
	if ((ret = processSource3 ()) != M_NO_ERROR) goto Exit;

	/*
	 *	Complete the header and the conversion tables,
	 *	and write them into the output file.
	 */

	ret = completeOutput ();

Exit:	terminate (ret);
}

/*
 *   NAME:	processSource1
 *
 *   FUNCTION:	This function parses each statement of the source file from
 *		the 1st line to the line of 'CHARMAP' after which codepoint
 *		mapping statements are allowed to be described.
 *		Then,   this function creates the header part in the static
 *		area (g_hdr), and allocates areas for the conversion tables.
 *
 *   RETURNS:	Error status code.
 *	M_INVAL_MB_CUR		- MB_CUR_MIN is greater than default.
 *	M_INVAL_FORMAT		- Invalid line format.
 *	M_INVAL_TOKEN		- Invalid token.
 *	M_INVAL_TOKEN_VALUE	- Invalid token value.
 *	M_UNDEF_TOKEN_VALUE	- Token value is not specified.
 *	M_EXTRA_CS_NAME		- Extra codeset name.
 *	M_EXTRA_UCONV_CLASS	- Extra UCONV_CLASS.
 *	M_EXTRA_CHARS		- Extra characters on line.
 *	M_CONFLICT_UCONV_CLASS	- Conflict with UCONV_CLASS.
 *	M_CONFLICT_MB_CUR	- Conflict with MB_CUR_MIN or MB_CUR_MAX.
 *	M_ESCAPE_COMMENT	- Escape and comment characters are duplicate.
 *	M_MISSING_CS_NAME	- Code set name is not defined.
 *	M_MISSING_UCONV_CLASS	- UCONV_CLASS is not defined.
 *	M_CANNOT_ALLOCATE	- Shortage of storage.
 *
 *   GLOBAL VARIABLES:
 *	g_string		- Token STRING value.
 *	g_number		- Token NUMBER value.
 *	g_mbMin			- MB_CUR_MIN value (default = 1).
 *	g_mbMax			- MB_CUR_MAX value (default = 1).
 *	g_hdr			- Conversion table header.
 *	g_U2MTable		- Conversion table U2M.
 *	g_M2UTable		- Conversion table M2U.
 *	g_mbcsU2M		- MBCS conversion table U2M.
 *	g_mbcsPlanes		- MBCS Plane information table.
 */

static	int		processSource1 () {

	int		ret, err_flag       = M_NO_ERROR,
			cs_name_seen        = FALSE,
			uconv_class_seen    = FALSE,
			mb_min_seen         = FALSE,
			mb_max_seen         = FALSE,
			subchar_seen        = FALSE,
#define 				      SUBCHAR_TRUE 1
#define 				      SUBCHAR_NONE 2
			charmap_seen        = FALSE;
	UniChar		sub_uni     	    = UC_DEF_SUB;
	ushort_t	allowed_max         = 1,
			subchar_len         = 0,
			uconv_class         = UC_CLASS_INVAL;
	uchar_t		cs_name[PATH_MAX+1] = {'\0'},
			locale [PATH_MAX+1] = {'\0'},
			subchar[STEM_MAX+3] = {'\0'},
			*copyright          = COPYRIGHT_STATEMENT;
	TOKENS		token;


	displayMsg (M_START_PROCESSING, g_inputFile, NULL);

	while ((token = getToken ()) != E_O_F) {
		switch (token) {
		case EOL: continue;

		case CODE_SET_NAME:

			if (cs_name_seen)
				     ret = M_EXTRA_CS_NAME;
			else if (getToken () != SPACES)
				     ret = M_INVAL_FORMAT;
			else switch (getToken ()) {
			case STRING: ret = M_NO_ERROR;          break;
			case EOL   : ret = M_UNDEF_TOKEN_VALUE; break;
			default    : ret = M_INVAL_TOKEN_VALUE; break;
			}
			if (ret != M_NO_ERROR) break;

			if ((ret = checkUntilEOL ()) != M_NO_ERROR) break;

			strcpy (cs_name, g_string);
			cs_name_seen = TRUE;
			break;

		case UCONV_CLASS:

			if (uconv_class_seen)
				     ret = M_EXTRA_UCONV_CLASS;
			else if (getToken () != SPACES)
				     ret = M_INVAL_FORMAT;
			else switch (getToken ()) {
			case STRING: ret = M_NO_ERROR;          break;
			case EOL   : ret = M_UNDEF_TOKEN_VALUE; break;
			default    : ret = M_INVAL_TOKEN_VALUE; break;
			}
			if (ret != M_NO_ERROR) break;

			if ((ret = getUconvClass (g_string, &uconv_class))
				!= M_NO_ERROR) break;
			switch (uconv_class) {
			case UC_CLASS_SBCS: allowed_max = 1; break;
			case UC_CLASS_EBCDIC_STATEFUL:
			case UC_CLASS_DBCS: allowed_max = 2; break;
			case UC_CLASS_MBCS: allowed_max = STEM_MAX+2; break;
			}
			if ((mb_max_seen && (g_mbMax > allowed_max)) ||
			    (mb_min_seen && (g_mbMin > allowed_max))) {
				ret = M_CONFLICT_MB_CUR; break;
			}
			if (subchar_seen && (subchar_len > allowed_max)) {
				ret = M_INVAL_TOKEN_VALUE; break;
			}
			if ((ret = checkUntilEOL ()) != M_NO_ERROR) break;

			uconv_class_seen = TRUE;
			break;

		case MB_CUR_MIN:

			if (getToken () != SPACES)
				     ret = M_INVAL_FORMAT;
			else switch (getToken ()) {
			case NUMBER: ret = M_NO_ERROR;          break;
			case EOL   : ret = M_UNDEF_TOKEN_VALUE; break;
			default    : ret = M_INVAL_TOKEN_VALUE; break;
			}
			if (ret != M_NO_ERROR) break;

			if (uconv_class_seen && (g_number > allowed_max)) {
				ret = M_CONFLICT_UCONV_CLASS; break;
			}
			if (mb_max_seen && (g_number > g_mbMax)) {
				ret = M_CONFLICT_MB_CUR; break;
			}
			if ((subchar_seen == SUBCHAR_TRUE) && (g_number > subchar_len)) {
				ret = M_INVAL_TOKEN_VALUE; break;
			}
			if ((ret = checkUntilEOL ()) != M_NO_ERROR) break;

			g_mbMin = g_number;
			mb_min_seen = TRUE;
			break;

		case MB_CUR_MAXX:

			if (getToken () != SPACES)
				     ret = M_INVAL_FORMAT;
			else switch (getToken ()) {
			case NUMBER: ret = M_NO_ERROR;          break;
			case EOL:    ret = M_UNDEF_TOKEN_VALUE; break;
			default:     ret = M_INVAL_TOKEN_VALUE; break;
			}
			if (ret != M_NO_ERROR) break;

			if (uconv_class_seen && (g_number > allowed_max)) {
				ret = M_CONFLICT_UCONV_CLASS; break;
			}
			if (mb_min_seen && (g_number < g_mbMin)) {
				ret = M_CONFLICT_MB_CUR; break;
			}
			if (subchar_seen && (g_number < subchar_len)) {
				ret = M_INVAL_TOKEN_VALUE; break;
			}
			if ((ret = checkUntilEOL ()) != M_NO_ERROR) break;

			g_mbMax = g_number;
			mb_max_seen = TRUE;
			break;

		case CHAR_NAME_MASK:

			if (getToken () != SPACES)
				     ret = M_INVAL_FORMAT;
			else switch (getToken ()) {
			case STRING: ret = M_NO_ERROR;          break;
			case EOL:    ret = M_UNDEF_TOKEN_VALUE; break;
			default:     ret = M_INVAL_TOKEN_VALUE; break;
			}
			if (ret != M_NO_ERROR) break;

			if (strcmp (g_string, "AXXXX") != 0) {
				ret = M_INVAL_TOKEN_VALUE; break;
			}
			ret = checkUntilEOL ();
			break;

		case SUBCHAR:

			if (getToken () != SPACES)
				ret = M_INVAL_FORMAT;
			else switch (getToken ()) {
			case HEX_CONST: ret = M_NO_ERROR;          break;
			case EOL:       ret = M_UNDEF_TOKEN_VALUE; break;
			case UNICHAR:   
					if (g_unichar != 0xffff )
					    ret = M_INVAL_TOKEN_VALUE; 
					else {
					    sub_uni = g_unichar;
					    ret = M_NO_ERROR;
					}
					break;
			default:        ret = M_INVAL_TOKEN_VALUE; break;
			}
			if (ret != M_NO_ERROR) break;

			if ( sub_uni == g_unichar ) {
				subchar_seen = SUBCHAR_NONE;
				break;
			}

			if ((mb_min_seen && (g_hexLen < g_mbMin)) ||
			    (mb_max_seen && (g_hexLen > g_mbMax))) {
				ret = M_CONFLICT_MB_CUR; break;
			}
			if (uconv_class_seen && (g_hexLen > allowed_max)) {
				ret = M_CONFLICT_UCONV_CLASS; break;
			}
			if (g_hexLen > (STEM_MAX+2)) {
				ret = M_INVAL_TOKEN_VALUE; break;
			}
			if ((ret = checkUntilEOL ()) != M_NO_ERROR) break;

			memcpy (subchar, g_hexStr, g_hexLen);
			subchar[g_hexLen] = '\0';
			subchar_len = g_hexLen;
			subchar_seen = SUBCHAR_TRUE;
			break;

		case LOCALE:

			if (getToken () != SPACES)
				     ret = M_INVAL_FORMAT;
			else switch (getToken ()) {
			case STRING: ret = M_NO_ERROR;          break;
			case EOL:    ret = M_UNDEF_TOKEN_VALUE; break;
			default:     ret = M_INVAL_TOKEN_VALUE; break;
			}
			if (ret != M_NO_ERROR) break;

			if ((ret = checkUntilEOL ()) != M_NO_ERROR) break;

			strcpy (locale, g_string);
			break;

		case COMMENT_CHAR:
		case ESCAPE_CHAR:

			ret = setCommentChar (token);
			break;

		case START_CHARMAP:	/* Comes to the end of this loop */

			if (!cs_name_seen)
				ret = M_MISSING_CS_NAME;
			else if (!uconv_class_seen)
				ret = M_MISSING_UCONV_CLASS;
			else if ((ret = checkUntilEOL ()) == M_NO_ERROR)
				charmap_seen = TRUE;
			break;

		case UNICHAR:
		case UNASSIGNED:
		case END_CHARMAP:	ret = M_MISSING_CHARMAP; break;
		case UNKNOWN:		ret = M_INVAL_TOKEN;     break;
		case ERROR:		ret = M_INVAL_FORMAT;    break;
		default:		ret = M_OTHER_ERROR;
		}
		if (ret != M_NO_ERROR) {
			switch (err_flag = ret) {
			case M_OTHER_ERROR: break;
			case M_UNDEF_TOKEN_VALUE:
			case M_INVAL_TOKEN_VALUE:
				displayMsg (ret, token2str (token), NULL); break;
			default:displayMsg (ret, NULL, NULL);
			}
			SKIP_TO_EOL
		}
		if (charmap_seen || (ret == M_MISSING_CHARMAP)) break;
	}
	if (err_flag != M_NO_ERROR) return err_flag;

	/*
	 *	Set MB_CUR_MIN / MB_CUR_MAX default values if not specified.
	 */

	if (!mb_max_seen) {
		g_mbMax = 1;			/* Default = 1 */
		if (mb_min_seen && (g_mbMin > g_mbMax)) {
			displayMsg (M_INVAL_MB_CUR, NULL, NULL);
			return      M_INVAL_MB_CUR;
		}
	}
	if (!mb_min_seen) g_mbMin = g_mbMax;	/* Default = MB_CUR_MAX */

	/*
	 *	Allocate memory for the conversion tables.
	 *	Fill all slots with '0xff' that means empty slot.
	 */

	if (uconv_class == UC_CLASS_MBCS) {

		if (((g_mbcsU2M    = malloc (MBCS_U2M_SIZE))  == NULL) ||
		    ((g_mbcsPlanes = malloc (PLANE_MAX_SIZE)) == NULL)) {
			displayMsg (M_CANNOT_ALLOCATE, NULL, NULL);
			return      M_CANNOT_ALLOCATE;
		}
		memset (g_mbcsU2M   , 0xff, MBCS_U2M_SIZE);
		memset (g_mbcsPlanes, 0   , PLANE_MAX_SIZE);
	}
	else {
		if (((g_U2MTable = malloc (U2M_SIZE)) == NULL) ||
		    ((g_M2UTable = malloc (M2U_SIZE)) == NULL)) {
			displayMsg (M_CANNOT_ALLOCATE, NULL, NULL);
			return      M_CANNOT_ALLOCATE;
		}
		memset (g_U2MTable, 0xff, U2M_SIZE);
		memset (g_M2UTable, 0xff, M2U_SIZE);
	}

	/*
	 *	Create the header part of the output file.
	 */

	memset (&g_hdr, 0, sizeof (g_hdr));

	switch (uconv_class) {
	case UC_CLASS_SBCS: g_hdr.com.size = sizeof (_ucmap_sbcs_t); break;
	case UC_CLASS_DBCS: g_hdr.com.size = sizeof (_ucmap_dbcs_t); break;
	case UC_CLASS_MBCS: g_hdr.com.size = sizeof (_ucmap_mbcs_t); break;
	case UC_CLASS_EBCDIC_STATEFUL:
		g_hdr.com.size = sizeof (_ucmap_ebcdic_stateful_t); break;
	}
	g_hdr.com.bom         = 0xfeff;		/* Big endian */
	g_hdr.com.version     = UC_VERSION;
	g_hdr.com.release     = UC_RELEASE;
	g_hdr.com.uconv_class = uconv_class;
	g_hdr.com.sub_uni     = sub_uni;
	strncpy (g_hdr.com.copyright, copyright, sizeof (g_hdr.com.copyright));
	strncpy (g_hdr.com.subchar,   subchar,   sizeof (g_hdr.com.subchar));
	strncpy (g_hdr.com.cs_name,   cs_name,   sizeof (g_hdr.com.cs_name));
	strncpy (g_hdr.com.locale,    locale,    sizeof (g_hdr.com.locale));

	if (uconv_class == UC_CLASS_MBCS) {
		memset (g_hdr.mbcs.code_len,  0xff, sizeof (g_hdr.mbcs.code_len));
		memset (g_hdr.mbcs.M2Uof4set, 0xff, sizeof (g_hdr.mbcs.M2Uof4set));
	}
	return M_NO_ERROR;
}

/*
 *   NAME:	processSource2 (for only MBCS type)
 *
 *   FUNCTION:	This function parses each statement in the CHARMAP section,
 *		classifies each codepoint into some PLANEs. After processing
 *		the CHARMAP section, it allocates max size of conversion ROWs.
 *
 *   NOTE:	THIS FUNCTION IS FOR ONLY MULTI-BYTE CODESETS.
 *
 *		This function only prepares conversion ROW '_uc_row_t' array
 *		of which entries will be completed by the processSource3().
 *
 *   RETURNS:	Error status code.
 *	M_INVAL_FORMAT		- Invalid line format.
 *	M_INVAL_TOKEN		- Invalid token.
 *	M_INVAL_TOKEN_VALUE	- Invalid token value.
 *	M_UNDEF_TOKEN_VALUE	- Token value is not specified.
 *	M_EXTRA_CS_NAME		- Extra codeset name.
 *	M_EXTRA_UCONV_CLASS	- Extra UCONV_CLASS.
 *	M_EXTRA_CHARMAP		- Extra CHARMAP.
 *	M_EXTRA_CHARS		- Extra characters on line.
 *	M_CONFLICT_MB_CUR	- Conflict with MB_CUR_MIN or MB_CUR_MAX.
 *	M_CONFLICT_LEN		- Conflicting code length.
 *	M_OVER_BYTE_RANGE	- Range exceeds one byte limit.
 *	M_TOO_MANY_PLANES	- Over 255 planes.
 *	M_ESCAPE_COMMENT	- Escape and comment characters are duplicate.
 *	M_MISSING_CHARMAP_END	- CHARMAP END is not found.
 *	M_CANNOT_ALLOCATE	- Shortage of storage.
 *
 *   GLOBAL VARIABLES:
 *	g_hdr			- Conversion table header
 *	g_mbcsRows		- Conversion rows.
 *	g_mbcsRowSize		- Total size of the conversion rows.
 *	g_mbcsPlanes		- MBCS PLANE information table.
 *	g_nPlanes		- Number of MBCS PLANEs.
 */

static	int		processSource2 () {

	uchar_t		save_escape, save_comment;
	size_t		save_pos;
	int		save_line;

	int		end_charmap_seen = FALSE,
			ret, err_flag    = FALSE;
	uchar_t		hex_str[STEM_MAX+3];
	ushort_t	hex_num, hex_len;
	UniChar		unichar;
	TOKENS		token;

	int		total_size, one_row_size, n_nodes, n_in_node,
			stem_len, plane_found, i, j;
	uchar_t		stem_min[STEM_MAX], stem_max[STEM_MAX];
	_planeInfo_t	*plane;


	if (g_hdr.com.uconv_class != UC_CLASS_MBCS) return M_NO_ERROR;

	save_line    = g_lineNumber;
	save_escape  = g_escapeChar;
	save_comment = g_commentChar;
	save_pos     = ftell (g_pInputFile);

	while ((token = getToken ()) != E_O_F) {
		switch (token) {
		case EOL: continue;

		case UNICHAR:

			/*
			 *	The setPlaneInfo() makes groups of codepoints,
			 *	called PLANE, in which all codepoints have the
			 *	same STEM, and update the following.
			 *
			 *	g_mbcsPlanes	- PLANE information table.
			 *	g_nPlanes	- Number of PLANEs.
			 */

			if ((ret = getUnichar (
				hex_str, &hex_len, &hex_num, &unichar, &unichar))
				!= M_NO_ERROR)
				break;
			ret = setPlaneInfo (hex_str, hex_len);
			break;

		case UNASSIGNED:	/* Only parse line */

			ret = getUnassigned (
				hex_str, &hex_len, &hex_num, &hex_num);
			break;

		case COMMENT_CHAR:
		case ESCAPE_CHAR:

			ret = setCommentChar (token);
			break;

		case END_CHARMAP:	/* Comes to the end of this loop */

			ret = M_NO_ERROR;
			end_charmap_seen = TRUE;
			break;

		case CODE_SET_NAME:	ret = M_EXTRA_CS_NAME;     break;
		case UCONV_CLASS:	ret = M_EXTRA_UCONV_CLASS; break;
		case START_CHARMAP:	ret = M_EXTRA_CHARMAP;     break;
		case UNKNOWN:		ret = M_INVAL_TOKEN;       break;
		case ERROR:		ret = M_INVAL_FORMAT;      break;
		default:		ret = M_OTHER_ERROR;
		}
		if (ret != M_NO_ERROR) {
			switch (err_flag = ret) {
			case M_OTHER_ERROR: break;
			case M_UNDEF_TOKEN_VALUE:
			case M_INVAL_TOKEN_VALUE:
				displayMsg (ret, token2str (token), NULL); break;
			default:displayMsg (ret, NULL, NULL);
			}
			SKIP_TO_EOL
		}
		if (end_charmap_seen) break;
	}
	if (err_flag != M_NO_ERROR) return err_flag;

	if (!end_charmap_seen) {
		displayMsg (M_MISSING_CHARMAP_END, NULL, NULL);
		return      M_MISSING_CHARMAP_END;
	}
	if (fseek (g_pInputFile, save_pos, SEEK_SET) != 0) {
		displayMsg (M_ERROR_READ, g_inputFile, strerror (errno));
		return      M_ERROR_READ;
	}
	g_escapeChar  = save_escape;
	g_commentChar = save_comment;
	g_lineNumber  = save_line;

	/*
	 *	All of multi-byte codepoints are grouped in several PLANEs.
	 *	All codepoints in the same PLANE have the same STEM string.
	 *
	 *	                 +--------------------------------+
	 *	MBCS codepoint = |   STEM   | ROW byte | COL byte |
	 *	                 +--------------------------------+
	 *
	 *	The PLANE information block '_planeInfo_t' array is set the
	 *	following by the setPlaneInfo() during the last loop.
	 *
	 *		n_rows / l_row ... Highest / lowest value of ROW byte.
	 *		n_cols / l_col ... Highest / lowest value of COL byte.
	 *
	 *	       l_row ..... n_rows
	 *	      +-------------+
	 *	l_col | PLANE       |-+
	 *	  :   |             | |-+
	 *	  :   |             | | |
	 *	  :   |             | | |
	 *	n_cols|             | | |
	 *	      +-------------+ | |
	 *	        +-------------+ |
	 *		  +-------------+
	 *
	 *	The values of n_rows and n_cols are changed to range of the
	 *	values of ROW and COLUMN. This range means possibility of
	 *	codepoint assignment within the PLANE.
	 *
	 *		n_rows / l_row ... Range of / lowest value of ROW byte.
	 *		n_cols / l_col ... Range of / lowest value of COL byte.
	 *
	 *	       1 2 3 ..... n_rows
	 *	      +-------------+
	 *	   1  | PLANE       |-+
	 *	   2  |             | |-+
	 *	   3  |             | | |
	 *	   :  |             | | |
	 *	n_cols|             | | |
	 *	      +-------------+ | |
	 *	        +-------------+ |
	 *		  +-------------+
	 */

	plane = g_mbcsPlanes;
	for (i = 0; i < g_nPlanes; i ++) {
		plane[i].n_rows -= (plane[i].l_row - 1);
		plane[i].n_cols -= (plane[i].l_col - 1);
	}

	/*
	 *	The '_uc_row_t' structure is code conversion table of each ROW,
	 *	indexed by COLUMN byte of which entry is target UCS codepoint.
	 *
	 *	PLANE                       _uc_row_t (indexed by ROW byte)
	 *	       1 2 3 .... n_rows ==>   1      2      3     .... n_rows
	 *	      +-------------+	     +--------------------------------+
	 *	   1  | | | |     | |	     |row#1 |row#2 |row#3 |....|row#N |
	 *	   2  | | | |     | |        +--------------------------------+
	 *	   3  | | | | ::: | |          |      |      |
	 *	   :  | | | |     | |          V      V      V
	 *	n_cols| | | |     | |        +----+ +----+ +----+ _uc_row_t
	 *	      +-------------+      1 |code| |code| |code| (indexed by
	 *				   2 | :  | | :  | | :  |  COLUMN byte)
	 *				   3 | :  | | :  | | :  |
	 *				   : | :  | | :  | | :  |
	 *			       n_cols| :  | | :  | | :  |
	 *				     +----+ +----+ +----+
	 *
	 *	Calculate size of the '_uc_row_t's for all PLANEs.
	 */

	total_size = 0;
	plane = g_mbcsPlanes;
	for (i = 0; i < g_nPlanes; i ++) {
		one_row_size = (plane[i].n_cols * sizeof (ushort_t) + 4);
		one_row_size += one_row_size & 2;	/* Round up to 4N */
		total_size += (plane[i].n_rows * one_row_size);
		total_size += (plane[i].n_rows * sizeof (ushort_t) + 4);
		total_size += total_size & 2;		/* Round up to 4N */
	}

	/*
	 *	ROW table of each PLANE is pointed by M2Uof4set array indexed
	 *	by the 1st byte of the STEM.
	 *
	 *	+------------------+
	 *	| STEM | ROW | COL |
	 *	+-+----------------+
	 *	  |
	 *	  |  M2Uof4set
	 *	  V  +------+     _uc_row_t (ROW table)
	 *	  1  |offset|--->+-----------------------------+
	 *	  2  |  :   |    |row#1|row#2|row#3|.....|row#N|
	 *	  3  |  :   |    +-----------------------------+
	 *	  :  |  :   |
	 * 	0xff |  :   |
	 *	     +------+
	 *
	 *	When STEM length is over 2 bytes, there are needed extra tables
	 *	indexed by 2nd, 3rd... byte of the STEM, of which entries point
	 *	to next offset table.
	 *
	 *	STEM
	 *	+-----------------------+
	 *	|   |   |   | ROW | COL |
	 *	+-+---+---+-------------+
	 *	  |   |   +---------------+
	 *	  |   +-------+           |
	 *	  |           |           |
	 *	  | M2Uof4set |           |
	 *	  V +------+  V _uc_row_t |
	 *	  1 |offset|--->+------+  V _uc_row_t
	 *	  2 |  :   |  1 |offset|--->+------+    _uc_row_t (ROW table)
	 *	  3 |  :   |  2 |  :   |  1 |offset|--->+---------------------+
	 *	  : |  :   |  3 |  :   |  2 |  :   |    |row#1|row#2|row#3|...|
	 *	  ff|  :   |  : |  :   |  3 |  :   |    +---------------------+
	 *	    +------+  ff|  :   |  : |  :   |
	 *                      +------+  ff|  :   |
	 *                                  +------+
	 *
	 *	Calculate size of the '_uc_row_t's linked from the M2Uof4set.
	 */

	for (stem_len = 2; stem_len <= STEM_MAX; stem_len ++) {

		/*
		 *	Search for PLANE with target length STEM.
		 */

		memset (stem_min, 0xff, sizeof (stem_min));
		memset (stem_max, 0x00, sizeof (stem_max));
		plane_found = FALSE;
		plane = g_mbcsPlanes;

		for (i = 0; i < g_nPlanes; i ++) {

			if (plane[i].stem_len != stem_len) continue;

			for (j = 0; j < stem_len; j ++) {
				if (stem_min[j] > plane[i].stem[j])
				    stem_min[j] = plane[i].stem[j];
				if (stem_max[j] < plane[i].stem[j])
				    stem_max[j] = plane[i].stem[j];
			}
			plane_found = TRUE;
		}
		if (!plane_found) continue;

		n_nodes = stem_max[0] - stem_min[0] + 1;
		for (i = 1; i < stem_len; i ++) {
			n_in_node = stem_max[i] - stem_min[i] + 1;
			one_row_size = n_in_node * sizeof (ushort_t) + 4;
			one_row_size += one_row_size & 2;	/* Round up to 4N */
			total_size += (n_nodes * one_row_size);
			n_nodes *= n_in_node;
		}
	}

	/*
	 *	Allocate max size of conversion ROW tables.
	 */

	if ((g_mbcsRows = malloc (total_size)) == NULL) {
		displayMsg (M_CANNOT_ALLOCATE, NULL, NULL);
		return      M_CANNOT_ALLOCATE;
	}
	memset (g_mbcsRows, 0xff, total_size);
	g_mbcsRowSize = 0L;

	return ret;
}

/*
 *   NAME:	processSource3
 *
 *   FUNCTION:	This function parses each statement of CHARMAP section
 *		of the source file, and build the conversion tables up.
 *
 *   RETURNS:	Error status code.
 *	M_INVAL_FORMAT		- Invalid line format.
 *	M_INVAL_TOKEN		- Invalid token.
 *	M_INVAL_TOKEN_VALUE	- Invalid token value.
 *	M_UNDEF_TOKEN_VALUE	- Token value is not specified.
 *	M_EXTRA_CS_NAME		- Extra codeset name.
 *	M_EXTRA_UCONV_CLASS	- Extra UCONV_CLASS.
 *	M_EXTRA_CHARMAP		- Extra CHARMAP.
 *	M_EXTRA_CHARS		- Extra characters on line.
 *	M_CONFLICT_MB_CUR	- Conflict with MB_CUR_MIN or MB_CUR_MAX.
 *	M_CONFLICT_LEN		- Conflicting code length.
 *	M_OVER_BYTE_RANGE	- Range exceeds one byte limit.
 *	M_ESCAPE_COMMENT	- Escape and comment characters are duplicate.
 *	M_MISSING_CHARMAP_END	- CHARMAP END is not found.
 *
 *   GLOBAL VARIABLES:
 *	g_hdr			- Conversion table header
 *	g_verboseFlag		- Verbose option flag.
 */

static	int		processSource3 () {

	int		ret, err_flag    = M_NO_ERROR,
			end_charmap_seen = FALSE, c;
	uchar_t		hex_str[STEM_MAX+3];
	ushort_t	from_hex, to_hex, hex_num, hex_len;
	UniChar		from_uni, to_uni;
	TOKENS		token;


	if (g_hdr.com.uconv_class == UC_CLASS_MBCS)

		/*
		 *	This is the 2nd pass for MBCS type,
		 *	no need to use verbose option again.
		 */

		g_verboseFlag = FALSE;

	while ((token = getToken ()) != E_O_F) {
		switch (token) {
		case EOL: continue;

		case UNICHAR:

			if ((ret = getUnichar (
				hex_str, &hex_len, &hex_num, &from_uni, &to_uni))
				!= M_NO_ERROR)
				break;
			ret = makeTableEntry (
				hex_str, hex_len, hex_num, from_uni, to_uni);
			break;

		case UNASSIGNED:

			if ((ret = getUnassigned (
				hex_str, &hex_len, &from_hex, &to_hex))
				!= M_NO_ERROR)
				break;
			ret = freeTableEntry (
				hex_str, hex_len, from_hex, to_hex);
			break;

		case COMMENT_CHAR:
		case ESCAPE_CHAR:

			ret = setCommentChar (token);
			break;

		case END_CHARMAP:	/* Comes to the end of this loop */

			ret = M_NO_ERROR;
			end_charmap_seen = TRUE;
			break;

		case CODE_SET_NAME:	ret = M_EXTRA_CS_NAME;     break;
		case UCONV_CLASS:	ret = M_EXTRA_UCONV_CLASS; break;
		case START_CHARMAP:	ret = M_EXTRA_CHARMAP;     break;
		case UNKNOWN:		ret = M_INVAL_TOKEN;       break;
		case ERROR:		ret = M_INVAL_FORMAT;      break;
		default:		ret = M_OTHER_ERROR;
		}
		if (ret != M_NO_ERROR) {
			switch (err_flag = ret) {
			case M_OTHER_ERROR: break;
			case M_UNDEF_TOKEN_VALUE:
			case M_INVAL_TOKEN_VALUE:
				displayMsg (ret, token2str (token), NULL); break;
			default:displayMsg (ret, NULL, NULL);
			}
			SKIP_TO_EOL;
		}
		if (end_charmap_seen) break;
	}
	if (err_flag != M_NO_ERROR) return err_flag;

	if (!end_charmap_seen) {
		displayMsg (M_MISSING_CHARMAP_END, NULL, NULL);
		return      M_MISSING_CHARMAP_END;
	}
	return M_NO_ERROR;
}

/*
 *   NAME:	getUconvClass
 *
 *   FUNCTION:	Search uconv class name table and return integer
 *		value of the class.
 *
 *   RETURNS:	Error status code.
 *	M_INVAL_TOKEN_VALUE	- Invalid token value.
 *
 *   GLOBAL VARIABLES:
 *	g_uconvClass		- uconv class name table.
 */

static	int		getUconvClass (
	uchar_t		*name,			/* Uconv class name    */
	ushort_t	*class) {		/* Returns class value */

	_uconvClass_t	*pUconvClass;


	if (name == NULL) return M_OTHER_ERROR;

	/*
	 *	Search the class name table. Note that input class name
	 *	is capital letter string (returned from the getToken()).
	 */

	for (pUconvClass = g_uconvClass;
	     pUconvClass->name != NULL;
	     pUconvClass ++) {
		if (strcmp (pUconvClass->name, name) == 0) {
			*class = pUconvClass->class;
			break;
		}
	}
	if (*class == UC_CLASS_INVAL)
		return M_INVAL_TOKEN_VALUE;
	else	return M_NO_ERROR;
}

/*
 *   NAME:	getUnichar
 *
 *   FUNCTION:	Parse UNICHAR line and get codepoints of UCS and MBCS.
 *
 *   RETURNS:	Error status code.
 *	M_INVAL_FORMAT		- Invalid line format.
 *	M_INVAL_TOKEN_VALUE	- Invalid token value.
 *	M_CONFLICT_MB_CUR	- Conflict with MB_CUR_MIN or MB_CUR_MAX.
 *
 *   GLOBAL VARIABLES:
 *	g_unichar		- UCS  codepoint
 *	g_number		- MBCS codepoint (last 2 byte)
 *	g_hexStr		- MBCS codepoint
 *	g_hexLen		- MBCS code length
 *	g_mbMin			- MB_CUR_MIN value
 *	g_mbMax			- MB_CUR_MAX value
 */

static	int		getUnichar (
	uchar_t		 hex_str[STEM_MAX+3],	/* Returns MBCS codepoint    */
	ushort_t	*hex_len,		/* Returns MBCS code length  */
	ushort_t	*hex_num,		/* Returns MBCS 2 byte code  */
	UniChar		*from_uni,		/* Returns from-UCS code     */
	UniChar		*to_uni) {		/* Returns   to-UCS code     */

	int 		ret = M_NO_ERROR;

	/*
	 *	Get UCS codepoint.
	 */

	*from_uni = g_unichar;

	switch (getToken ()) {
	case SPACES:	*to_uni = *from_uni; break;
	case ELLIPSIS:

		/*
		 *	Range notation. (<Uxxxx>...<Uxxxx>)
		 */

		if ((getToken () != UNICHAR) ||
		    (getToken () != SPACES)) {
			return M_INVAL_FORMAT;
		}
		if (g_unichar < *from_uni) {
			return M_INVAL_TOKEN_VALUE;
		}
		*to_uni = g_unichar;
		break;

	default:return M_INVAL_FORMAT;
	}

	/*
	 *	Get MBCS codepoint.
	 */

	if (getToken () != HEX_CONST) return M_INVAL_FORMAT;

	if ((g_hexLen < g_mbMin) || (g_hexLen > g_mbMax))
		return M_CONFLICT_MB_CUR;

	*hex_num = g_number;
	*hex_len = g_hexLen;
	strncpy (hex_str, g_hexStr, g_hexLen);

	return checkUntilEOL ();
}

/*
 *   NAME:	getUnassigned
 *
 *   FUNCTION:	Parse UNASSIGNED line and get unassigned codepoints.
 *
 *   RETURNS:	Error status code.
 *	M_INVAL_FORMAT		- Invalid line format.
 *	M_INVAL_TOKEN_VALUE	- Invalid token value.
 *	M_UNDEF_TOKEN_VALUE	- Token value is not specified.
 *	M_CONFLICT_MB_CUR	- Conflict with MB_CUR_MIN or MB_CUR_MAX.
 *
 *   GLOBAL VARIABLES:
 *	g_unichar		- UCS  codepoint
 *	g_number		- MBCS codepoint (last 2 byte)
 *	g_hexStr		- MBCS codepoint
 *	g_hexLen		- MBCS code length
 *	g_mbMin			- MB_CUR_MIN value
 *	g_mbMax			- MB_CUR_MAX value
 */

static	int		getUnassigned (
	uchar_t		hex_str[STEM_MAX+3],	/* Returns MBCS codepoint    */
	ushort_t	*hex_len,		/* Returns MBCS code length  */
	ushort_t	*from_hex,		/* Returns from-MBCS code    */
	ushort_t	*to_hex) {		/* Returns to  -MBCS code    */

	TOKENS		token;

	/*
	 *	Get MBCS codepoint.
	 */

	if (getToken () != SPACES)
			return M_INVAL_FORMAT;
	switch (getToken ()) {
	case HEX_CONST: if ((g_hexLen >= g_mbMin) && (g_hexLen <= g_mbMax)) break;
			return M_CONFLICT_MB_CUR;
	case EOL      :	return M_UNDEF_TOKEN_VALUE;
	default       :	return M_INVAL_TOKEN_VALUE;
	}
	*from_hex = g_number;
	*hex_len  = g_hexLen;
	strncpy (hex_str, g_hexStr, g_hexLen);

	if ((token = getToken ()) == ELLIPSIS) {

		/*
		 *	Range notation. (\xXX...\xXX)
		 */

		if (getToken () != HEX_CONST)
			return M_INVAL_FORMAT;
		if ((g_hexLen < g_mbMin) || (g_hexLen > g_mbMax))
			return M_CONFLICT_MB_CUR;
		if ((g_hexLen != hex_len) ||
		    (memcmp (g_hexStr, hex_str, hex_len) < 0))
			return M_INVAL_TOKEN_VALUE;

		*to_hex = g_number;
	}
	else {
		*to_hex = *from_hex;
		ungetToken (token);
	}
	return checkUntilEOL ();
}

/*
 *   NAME:	setCommentChar
 *
 *   FUNCTION:	Change comment / escape character.
 *
 *   RETURNS:	Error status code.
 *	M_INVAL_TOKEN_VALUE	- Invalid token value.
 *	M_UNDEF_TOKEN_VALUE	- Token value is not specified.
 *	M_ESCAPE_COMMENT	- Escape and comment chars are duplicate.
 *
 *   GLOBAL VARIABLES:
 *	g_commentChar		- Comment character
 *	g_escapeChar		- Escape character
 */

#define ILLEGAL_COMMENT_CHARS	" \t\n\r\f.x\"U<>"

static	int		setCommentChar (
	TOKENS		token) {

	int		c, ret;

	switch (getToken ()) {
	case SPACES:	c = getChar (); break;
	case EOL:	return M_UNDEF_TOKEN_VALUE;
	default:	return M_INVAL_FORMAT;
	}
	if ((isalnum (c)) ||
	    (strchr (ILLEGAL_COMMENT_CHARS, c) != NULL)) {
		return M_INVAL_TOKEN_VALUE;
	}
	if (token == COMMENT_CHAR) {
		if (c == g_escapeChar)
			return M_ESCAPE_COMMENT;
		if ((ret = checkUntilEOL ()) != M_NO_ERROR)
			return ret;
		g_commentChar = (uchar_t)c;
	}
	else {	/* the token is ESCAPE_CHAR */
		if (c == g_commentChar)
			return M_ESCAPE_COMMENT;
		if ((ret = checkUntilEOL ()) != M_NO_ERROR) 
			return ret;
		g_escapeChar  = (uchar_t)c;
	}
	return M_NO_ERROR;
}

/*
 *   NAME:	setPlaneInfo (for only MBCS type)
 *
 *   FUNCTION:	Make a element of MBCS plane information block array.
 *
 *   RETURNS:	Error status code.
 *	M_CONFLICT_LEN		- Conflicting code length.
 *	M_TOO_MANY_PLANES	- Over 255 planes.
 *
 *   GLOBAL VARIABLES:
 *	g_hdr			- Conversion table header
 *	g_mbcsPlanes		- MBCS plane information table.
 *	g_nPlanes		- Number of MBCS planes.
 */

static	int		setPlaneInfo (
	uchar_t		hex_str[STEM_MAX+3],
	ushort_t	hex_len) {

	int		plane_num;
	ushort_t	code_len;
	_planeInfo_t	*plane;

	/*
	 *	Set the entry of the code length table.
	 *
	 *	NOTE:	When MBCS is based on the ISO2022, the first byte of
	 *		codepoint (usually single shift code) determines the
	 *		length of the code.
	 */

	if ((code_len = g_hdr.mbcs.code_len[hex_str[0]]) == 0xffff)
		g_hdr.mbcs.code_len[hex_str[0]] = hex_len;
	else if (code_len != hex_len) return M_CONFLICT_LEN;

	if (hex_len == 1) return M_NO_ERROR;	/* Single byte code */

	/*
	 *	Search for existing PLANE information block.
	 */

	if ((plane_num = getPlaneIndex (hex_str, hex_len-2)) > PLANE_MAX_NUM)
		return M_TOO_MANY_PLANES;

	if (plane_num >= 0) {

		/*
		 *	Update this unit.
		 *
		 *	n_rows <--- Highest value of ROW byte
		 *	l_row  <--- Lowest  value of ROW byte
		 *	n_cols <--- Highest value of COL byte
		 *	l_col  <--- Lowest  value of COL byte
		 */

		plane = g_mbcsPlanes + plane_num;

		if (plane->n_rows < hex_str[hex_len-2])
		    plane->n_rows = hex_str[hex_len-2];
		if (plane->l_row  > hex_str[hex_len-2])
		    plane->l_row  = hex_str[hex_len-2];
		if (plane->n_cols < hex_str[hex_len-1])
		    plane->n_cols = hex_str[hex_len-1];
		if (plane->l_col  > hex_str[hex_len-1])
		    plane->l_col  = hex_str[hex_len-1];
	}
	else {	/*
		 *	Get a new unit.
		 */

		plane = g_mbcsPlanes + g_nPlanes;
		g_nPlanes ++;

		if ((hex_len - 2) > 0)
			memcpy (plane->stem, hex_str, hex_len - 2);
		else	plane->stem[0] = '\0';
		plane->stem_len = hex_len - 2;
		plane->n_rows   = hex_str [hex_len-2];
		plane->l_row    = hex_str [hex_len-2];
		plane->n_cols   = hex_str [hex_len-1];
		plane->l_col    = hex_str [hex_len-1];
	}
	return M_NO_ERROR;
}

/*
 *   NAME:	getPlaneIndex
 *
 *   FUNCTION:	Get index of PLANE information block array.
 *
 *   RETURNS:	Returns index number of found unit.
 *
 *   GLOBAL VARIABLES:
 *	g_hdr		- Conversion table header.
 *	g_mbcsPlanes	- MBCS plane information table.
 *	g_nPlanes		- Number of MBCS planes.
 */

static	int		getPlaneIndex (
	uchar_t		*stem,
	ushort_t	stem_len) {

	int		i, found = FALSE;

	for (i = 0; i < g_nPlanes; i ++) {
		if (stem_len == 0) {
			if ((g_mbcsPlanes[i].stem_len == 0) && 
			    (g_mbcsPlanes[i].l_row    != 0)) {
				found = TRUE;
				break;
			}
		}
		else if (memcmp (g_mbcsPlanes[i].stem, stem, stem_len) == 0) {
			found = TRUE;
			break;
		}
	}
	if (found) return i;
	else	   return (int)-1;
}

/*
 *   NAME:	makeTableEntry
 *
 *   FUNCTION:	Make entries of the conversion table.
 *
 *   RETURNS:	Error status code.
 *	M_OVER_BYTE_RANGE	- Range exceeds one byte limit.
 *
 *   GLOBAL VARIABLES:
 *	g_hdr		- Conversion table header
 *	g_U2MTable	- Conversion table U2M
 *	g_M2UTable	- Conversion table M2U
 *	g_mbcsStems	- MBCS STEM information table.
 */

static	int		makeTableEntry (
	uchar_t		hex_str[STEM_MAX+3],	/* MBCS codepoint          */
	ushort_t	hex_len,		/* MBCS code length        */
	ushort_t	hex_num,		/* MBCS code (last 2 byte) */
	UniChar		from_uni,		/* From UCS code           */
	UniChar		to_uni) {		/* To   UCS code           */

	int		plane_num, ret = M_NO_ERROR;
	UniChar		unichar;


	if (g_hdr.com.uconv_class == UC_CLASS_MBCS) {
		if (hex_len > 1)
			plane_num = getPlaneIndex (hex_str, hex_len - 2);
		else	plane_num = -1;
	}
	for (unichar  = from_uni;
	     unichar <= to_uni;
	     unichar ++, hex_num ++) {

		if ((hex_num & 0xff) > 0xff) return M_OVER_BYTE_RANGE;

		/*
		 *	Make U2M table entry.
		 */

		if (g_hdr.com.uconv_class == UC_CLASS_MBCS) {

			g_mbcsU2M[unichar].stem_index = plane_num;
			g_mbcsU2M[unichar].code = hex_num;

			ret = set_mbcsM2U (hex_str, hex_len, unichar, plane_num);
			if (ret != M_NO_ERROR) return ret;
		}
		else {
			g_U2MTable[unichar] = hex_num;
			g_M2UTable[hex_num] = unichar;
		}

		/*
		 *	Make code length table entry when DBCS type.
		 */

		if (g_hdr.com.uconv_class == UC_CLASS_DBCS) {
			if (hex_num < 0x0100)
				g_hdr.dbcs.code_len[hex_num   ] = 1;
			else	g_hdr.dbcs.code_len[hex_num>>8] = 2;
		}
	}
	return M_NO_ERROR;
}

/*
 *   NAME:	set_mbcsM2U (for only MBCS type)
 *
 *   FUNCTION:	Make the entries of MBCS conversion ROWs.
 *
 *   RETURNS:	Error status code.
 *	M_CONFLICT_LEN		- Conflicting code length.
 *
 *   GLOBAL VARIABLES:
 *	g_hdr			- Conversion table header.
 *	g_mbcsRows		- Conversion rows.
 *	g_mbcsRowSize		- Real size of the conversion rows.
 *	g_mbcsPlanes		- MBCS plane information table.
 */

static	int		set_mbcsM2U (
	uchar_t		hex_str[STEM_MAX+3],	/* MBCS codepoint   */
	ushort_t	hex_len,		/* MBCS code length */
	UniChar		unichar,		/* UCS  codepoint   */
	int		plane_num) {		/* PLANE number     */

	int		ret = M_NO_ERROR, row_base, i_stem, i;
	uchar_t		this_byte, n_slots, l_value, c;
	ushort_t	*p_of4set, code_len;
	ulong_t		row_size;
	_uc_row_t	*row;

	/*
	 *	When single byte code, set UCS code simply to the M2Uof4set.
	 */

	code_len = g_hdr.mbcs.code_len[hex_str[0]];

	if (code_len == 1) {
		g_hdr.mbcs.M2Uof4set[hex_str[0]] = (ushort_t)unichar;
		return M_NO_ERROR;
	}

	/*
	 *	When multi byte code, the 1st byte of the codepoint is index of
	 *	the M2Uof4set array of which element is (offset/4) to the ROW.
	 *
	 *	NOTE:	MBCS conversion table structure.
	 *
	 *	+--------------------------------+ -+
	 *	| _ucmap_mbcs_t | _ucmap_com_t   |  |
	 *	|               |----------------|  |
	 *	|               | U2Mof4set[256] |  |
	 *	|               |----------------|  | Header part
	 *	|               | M2Uof4set[256] |  |
	 *	|               |----------------|  |
	 *	|               | code_len[256]  |  |
	 *	|--------------------------------| -+
	 *	| _uc_stem_t[g_nPlanes]          |    STEM info table
	 *	|--------------------------------|
	 *	| _uc_row_t * N                  |    Conversion ROWs
	 *	|--------------------------------|
	 *	| _uc_u2m_t * N                  |    Conversion table U2M
	 *	+--------------------------------+
	 */

	row_base = sizeof (_ucmap_mbcs_t) + sizeof (_uc_stem_t) * g_nPlanes;

	p_of4set = &(g_hdr.mbcs.M2Uof4set[hex_str[0]]);

	for (code_len --, i_stem = 1;
	     code_len > 0;
	     code_len --, i_stem ++) {

		this_byte = hex_str[i_stem];

		if (*p_of4set != 0xffff) {

			row = (_uc_row_t*)((char*)g_mbcsRows
			    + (int)(*p_of4set) * 4 - row_base);
		}
		else {	/*
			 *	Get number of slots for new ROW table.
			 */

			if (code_len > 2) {	/* This byte is in STEM */

				n_slots = l_value = this_byte;
				for (i = 0; i < g_nPlanes; i ++) {

					if (g_mbcsPlanes[i].stem_len == 0) continue;

					if (memcmp (g_mbcsPlanes[i].stem,
						hex_str, i_stem) == 0) {
						c = g_mbcsPlanes[i].stem[i_stem];
						if (l_value > c) l_value = c;
						if (n_slots < c) n_slots = c;
					}
				}
				n_slots -= (l_value - 1);
			}
			else if (code_len == 2) {
				n_slots = g_mbcsPlanes[plane_num].n_rows;
				l_value = g_mbcsPlanes[plane_num].l_row;
			}
			else {
				n_slots = g_mbcsPlanes[plane_num].n_cols;
				l_value = g_mbcsPlanes[plane_num].l_col;
			}

			/*
			 *	Make new ROW table.
			 */

			row = (_uc_row_t*)((char*)g_mbcsRows + g_mbcsRowSize);
			row->n_slots = (ushort_t)n_slots;
			row->l_value = (ushort_t)l_value;
			row_size = sizeof (ushort_t) * n_slots + 4;
			row_size += (row_size & 2);	/* Round up to 4N */

			/*
			 *	Set (offset/4) of this ROW.
			 */

			*p_of4set = (ushort_t)((row_base + g_mbcsRowSize) / 4);
			g_mbcsRowSize += row_size;
		}

		/*
		 *	Set the pointer at the slot indexed by this byte
		 *	for setting address of next ROW.
		 */

		this_byte -= row->l_value;	/* Make index of the table */
		p_of4set = &(row->nextOf4set[this_byte]);
	}

	/*
	 *	Set UCS code to the final ROW.
	 */

	*p_of4set = unichar;
	return M_NO_ERROR;
}

/*
 *   NAME:	freeTableEntry
 *
 *   FUNCTION:	Make specified entry of the conversion table free.
 *
 *   RETURNS:	Error status code.
 *	M_NO_ERROR		- Successful completion.
 *	M_OVER_BYTE_RANGE	- Specified range exceeds limit of one byte.
 *
 *   GLOBAL VARIABLES:
 *	g_hdr			- Conversion table header
 *	g_mbcsRows		- Conversion rows.
 */

static	int		freeTableEntry (
	uchar_t		hex_str[STEM_MAX+3],	/* MBCS codepoint        */
	ushort_t	hex_len,		/* MBCS codepoint length */
	ushort_t	from_hex,		/* From codepoint        */
	ushort_t	to_hex) {		/* To   codepoint        */

	int		ret = M_NO_ERROR, row_base, i;
	UniChar		unichar;
	ushort_t	hex_num, of4set, code_len;
	uchar_t		c;
	_uc_row_t	*row;


	if (g_hdr.com.uconv_class == UC_CLASS_MBCS) {

		if ((code_len = g_hdr.mbcs.code_len[hex_str[0]]) == 0xffff)
			return M_NO_ERROR;	/* Already unassigned */

		if ((of4set = g_hdr.mbcs.M2Uof4set[hex_str[0]]) == 0xffff)
			return M_NO_ERROR;	/* Already unassigned */

		row_base = sizeof (_ucmap_mbcs_t) + sizeof (_uc_stem_t) * g_nPlanes;
	}
	for (hex_num  = from_hex;
	     hex_num <= to_hex;
	     hex_num ++) {

		if ((hex_num & 0xff) > 0xff) return M_OVER_BYTE_RANGE;

		/*
		 *	Get UCS character code from input MBCS code.
		 */

		if (g_hdr.com.uconv_class == UC_CLASS_MBCS) {

			for (i = 1; i < code_len; i ++) {
				row = (_uc_row_t*)((char*)g_mbcsRows
				    + ((int)of4set * 4 - row_base));
				c  = hex_str[i];
				c -= row->l_value;
				of4set = row->nextOf4set[c];
			}
			if (of4set == 0xffff) continue;

			g_mbcsU2M[of4set].stem_index = 0xffff;
			g_mbcsU2M[of4set].code       = 0xffff;
			if (hex_len == 1)
				g_hdr.mbcs.M2Uof4set[hex_num] = 0xffff;
			else	row->nextOf4set[c] = 0xffff;
		}
		else {
			unichar = g_M2UTable[hex_num];
			if (unichar == 0xffff) continue;

			g_U2MTable[unichar] = 0xffff;
			g_M2UTable[hex_num] = 0xffff;
		}

	}
	return M_NO_ERROR;
}

/*
 *   NAME:	completeOutput
 *
 *   FUNCTION:	Complete the output file.
 *
 *   RETURNS:	Error status code.
 *	M_ERROR_WRITE		- Error while writing to output file.
 *
 *   GLOBAL VARIABLES:
 *	g_pOutTmpFile		- Descriptor of temporary output file.
 *	g_hdr			- Conversion table header
 *	g_U2MTable		- Conversion table U2M.
 *	g_M2UTable		- Conversion table M2U.
 *	g_mbcsU2MTable		- Conversion table U2M for MBCS.
 *	g_mbcsRows		- Conversion ROWs for MBCS.
 *	g_mbcsRowSize		- Conversion ROW total size.
 *	g_mbcsStems		- MBCS STEM information table.
 */

static	int	completeOutput () {

	int		ret = M_NO_ERROR, header_size, slot_size,
			cur_block, cur_base, block, base, i;
	ushort_t	*U2M0f4set, *M2Uof4set;
	ulong_t		size;
	uchar_t		c;


	switch (g_hdr.com.uconv_class) {
	case UC_CLASS_SBCS:

		header_size = sizeof (_ucmap_sbcs_t);
		slot_size   = 1;
		U2M0f4set   = g_hdr.sbcs.U2Mof4set;
		M2Uof4set   = NULL;

		/*	Search for un-used codepoint for using it as invalid
		 *	character mark. Note that the value '0xffff' can not
		 *	be used since the U2M table is one byte value array.
		 */

		c = findUndefChar ();
		g_hdr.sbcs.undef_char = c;
		g_hdr.sbcs.undef_uni  = g_M2UTable[c];

		for (i = 0; i < ENTRIES_IN_TABLE; i ++) {
			if (g_U2MTable[i] == 0xffff)
				g_U2MTable[i] = g_hdr.sbcs.undef_char;
		}
		break;

	case UC_CLASS_DBCS:

		header_size = sizeof (_ucmap_dbcs_t);
		slot_size   = 2;
		U2M0f4set   = g_hdr.dbcs.U2Mof4set;
		M2Uof4set   = g_hdr.dbcs.M2Uof4set;
		break;

	case UC_CLASS_MBCS:

		header_size = sizeof (_ucmap_mbcs_t);
		slot_size   = sizeof (_uc_u2m_t);
		U2M0f4set   = g_hdr.mbcs.U2Mof4set;
		M2Uof4set   = g_hdr.mbcs.M2Uof4set;

		/*
		 *	Make STEM information block '_uc_stem_t' array from
		 *	the PLANE information blocks.
		 */

		if ((g_mbcsStems = malloc (
			sizeof (_uc_stem_t) * g_nPlanes)) == NULL) {
			displayMsg (M_CANNOT_ALLOCATE, NULL, NULL);
			return      M_CANNOT_ALLOCATE;
		}
		for (i = 0; i < g_nPlanes; i ++) {
			memcpy (g_mbcsStems[i].stem, g_mbcsPlanes[i].stem,
					     sizeof (g_mbcsPlanes[i].stem));
			g_mbcsStems[i].stem_len    = g_mbcsPlanes[i].stem_len;
		}
		free (g_mbcsPlanes);
		break;

	case UC_CLASS_EBCDIC_STATEFUL:

		header_size = sizeof (_ucmap_ebcdic_stateful_t);
		slot_size   = 2;
		U2M0f4set   = g_hdr.ebcdic_stateful.U2Mof4set;
		M2Uof4set   = g_hdr.ebcdic_stateful.M2Uof4set;
		break;
	}

	/*
	 *	Skip the header part.
	 */

	if (fseek (g_pOutTmpFile, header_size, SEEK_SET) != 0) {
		displayMsg (M_ERROR_WRITE, strerror (errno), NULL);
		return      M_ERROR_WRITE;
	}

	/*
	 *	Write MBCS STEM information table & conversion ROWs.
	 *
	 *	NOTE:	MBCS conversion table structure.
	 *
	 *	+--------------------------------+
	 *	| _ucmap_mbcs_t                  | Header
	 *	|--------------------------------|
	 *	| _uc_stem_t[g_nPlanes]          | STEM info table
	 *	|--------------------------------|
	 *	| _uc_row_t * N                  | Conversion ROWs
	 *	|--------------------------------|
	 *	| _uc_u2m_t * N                  | Conversion table U2M
	 *	+--------------------------------+
	 */

	if (g_hdr.com.uconv_class == UC_CLASS_MBCS) {

		if ((fwrite (g_mbcsStems, sizeof (_uc_stem_t) * g_nPlanes,
			1, g_pOutTmpFile) != 1) ||
		    (fwrite (g_mbcsRows, g_mbcsRowSize,
			1, g_pOutTmpFile) != 1)) {
			displayMsg (M_ERROR_WRITE, strerror (errno), NULL);
			return      M_ERROR_WRITE;
		}
	}

	/*
	 *	Write conversion table U2M.
	 *
	 *	NOTE:	There are 256 blocks each of that has 256 entries that
	 *		contains conversion target code value. Position of the
	 *		written block is stored in the U2Mof4set table that is
	 *		indexed by high byte of a UCS character code. The slot
	 *		of the blocks are indexed by low byte of the UCS code.
	 *
	 *	g_U2MTable                        Output File
	 *	      +---------+ BLOCK #1        +--------------------+
	 *	U0000 |char code|-+ BLOCK #2      | U2Mof4set |offset/4|-----+
	 *	      |---------| |-+ BLOCK #3    |           |--------|     |
	 *	U0001 |char code| | |             |           |offset/4|---+ |
	 *	      |---------| | |             |           |--------|   | |
	 *	  :   |    ::   | | |             |           |   ::   |   | |
	 *	  :   |    ::   | | |             |           |   ::   |   | |
	 *	      |---------| | |             |           |--------|   | |
	 *	U00FF |char code| | |             |           |offset/4|   | |
	 *	      +---------+ | |             |--------------------|<---+
	 *	        +---------+ |             | BLOCK #1           |   |
	 *	          +---------+             |--------------------|<--+
	 *	               ::                 | BLOCK #2           |
	 *	               ::                 |--------------------|
	 *	          +---------+ BLOCK #256  |         ::         |
	 *	    UFF01 |char code|             |         ::         |
	 *	          |---------|             |--------------------|
	 *	    UFF02 |char code|             | BLOCK #N           |
	 *	          |---------|             +--------------------+
	 *	      :   |    ::   |
	 *	      :   |    ::   |
	 *	          |---------|
	 *	    UFFFF |char code|
	 *	          +---------+
	 */

	size = ftell (g_pOutTmpFile);

	for (cur_block = 0; cur_block < BLOCKS_IN_TABLE; cur_block ++) {

		cur_base  = cur_block * ENTRIES_IN_BLOCK;
		for (block = 0; block < cur_block; block ++) {

			base  = block * ENTRIES_IN_BLOCK;
			if (g_hdr.com.uconv_class == UC_CLASS_MBCS) {

				for (i = 0; i < ENTRIES_IN_BLOCK; i ++)
					if ((g_mbcsU2M[    base+i].stem_index !=
					     g_mbcsU2M[cur_base+i].stem_index) ||
					    (g_mbcsU2M[    base+i].code !=
					     g_mbcsU2M[cur_base+i].code)) break;
			}
			else {
				for (i = 0; i < ENTRIES_IN_BLOCK; i ++)
					if (g_U2MTable[    base+i]
					 != g_U2MTable[cur_base+i]) break;
			}
			if (i == ENTRIES_IN_BLOCK) {

				/*
				 *	All enties in this block is absolutely
				 *	same as current one, so just link from
				 *	the same element of the U2Mof4set.
				 */

				U2M0f4set[cur_block] = U2M0f4set[block];
				break;
			}
		}
		if (block == cur_block) {

			/*
			 *	There is no same block as current one, write it.
			 */

			base = cur_block * ENTRIES_IN_BLOCK;

			switch (g_hdr.com.uconv_class) {
			case UC_CLASS_SBCS:

				for (i = 0; i < ENTRIES_IN_BLOCK; i ++) {
					c = (uchar_t)(g_U2MTable[base+i] & 0xff);
					if (fwrite (&c, 1, 1, g_pOutTmpFile) != 1) {
						ret = M_ERROR_WRITE; break;
					}
				}
				break;

			case UC_CLASS_MBCS:

				if (fwrite (&g_mbcsU2M[base],
					sizeof (_uc_u2m_t) * ENTRIES_IN_BLOCK,
					1, g_pOutTmpFile) != 1)
					ret = M_ERROR_WRITE;
				break;

			case UC_CLASS_DBCS:
			case UC_CLASS_EBCDIC_STATEFUL:

				if (fwrite (&g_U2MTable[base],
					sizeof (ushort_t) * ENTRIES_IN_BLOCK,
					1, g_pOutTmpFile) != 1)
					ret = M_ERROR_WRITE;
				break;
			}
			if (ret == M_ERROR_WRITE) {
				displayMsg (ret, strerror (errno), NULL);
				return ret;
			}
			U2M0f4set[cur_block] = (ushort_t)(size / 4);
			size += (ENTRIES_IN_BLOCK * slot_size);
		}
	}

	/*
	 *	Write conversion table M2U.
	 */

	switch (g_hdr.com.uconv_class) {
	case UC_CLASS_SBCS:

		for (i = 0; i < ENTRIES_IN_BLOCK; i ++) {
			g_hdr.sbcs.M2Utable[i] = g_M2UTable[i];
		}
		break;

	case UC_CLASS_MBCS: break;	/* No M2U table for MBCS type */

	case UC_CLASS_DBCS:
	case UC_CLASS_EBCDIC_STATEFUL:

		for (cur_block = 0; cur_block < BLOCKS_IN_TABLE; cur_block ++) {

			cur_base  = cur_block * ENTRIES_IN_BLOCK;
			for (block = 0; block < cur_block; block ++) {

				base  = block * ENTRIES_IN_BLOCK;
				for (i = 0; i < ENTRIES_IN_BLOCK; i ++)
					if (g_M2UTable[    base+i]
					 != g_M2UTable[cur_base+i]) break;

				if (i >= ENTRIES_IN_BLOCK) {
					M2Uof4set[cur_block] = M2Uof4set[block];
					break;
				}
			}
			if (block >= cur_block) {
				if (fwrite (
					&g_M2UTable[cur_block * ENTRIES_IN_BLOCK],
					ENTRIES_IN_BLOCK * sizeof (UniChar),
					1, g_pOutTmpFile) != 1) {
					displayMsg (M_ERROR_WRITE,strerror(errno),NULL);
					return      M_ERROR_WRITE;
				}
				M2Uof4set[cur_block] = (ushort_t)(size / 4);
				size += (ENTRIES_IN_BLOCK * sizeof (UniChar));
			}
		}
		break;
	}

	/*
	 *	Now write the header.
	 */

	g_hdr.com.size = size;

	fflush (g_pOutTmpFile);
	if ((fseek (g_pOutTmpFile, 0L, SEEK_SET) != 0) ||
	    (fwrite (&g_hdr, header_size, 1, g_pOutTmpFile) != 1)) {
		displayMsg (M_ERROR_WRITE, strerror (errno), NULL);
		return      M_ERROR_WRITE;
	}
	return M_NO_ERROR;
}

/*
 *   NAME:	findUndefChar
 *
 *   FUNCTION:	Find codepoint that is not assigned character, that will be
 *		used as invalid character mark in SBCS_UCS conversion table.
 *
 *   RETURNS:
 *
 *   GLOBAL VARIABLES:
 *	g_hdr			- Conversion table header
 *	g_U2MTable		- Conversion table U2M.
 *	g_M2UTable		- Conversion table M2U.
 */

static	uchar_t	findUndefChar () {

	int	i;


	for (i = 0; i < 256; i++) if (g_M2UTable[i] == 0xffff) return (uchar_t)i;
			
	/*
	 *	If all of codepoints have equivalent UCS characters,
	 *	then try to find a codepoint that is converted from
	 *	only one UCS character.
	 */

	if (canBeUndefChar (0x7f)) return (uchar_t)0x7f;

	for (i = 0x01; i <= 0x08; i ++) if (canBeUndefChar (i)) return (uchar_t)i;
	for (i = 0x0e; i <= 0x1f; i ++) if (canBeUndefChar (i)) return (uchar_t)i;

	if (canBeUndefChar (0x00)) return (uchar_t)0x00;

	for (i = 0x09; i <= 0x0d; i ++) if (canBeUndefChar (i)) return (uchar_t)i;
	for (i = 0xff; i >= 0x80; i --) if (canBeUndefChar (i)) return (uchar_t)i;
	for (i = 0x21; i <= 0x7e; i ++) if (canBeUndefChar (i)) return (uchar_t)i;

	return (uchar_t)0x20;	/* use SPACE at last */
}

/*
 * NAME:	canBeUndefChar
 *
 * FUNCTION:	Check if specified codepoint is translated from only one
 *		UCS character.
 *
 * RETURNS:	TRUE if yes, or FALSE.
 *
 * GLOBAL VARIABLES:
 *	g_U2MTable		- Conversion table U2M.
 */

static	int	canBeUndefChar (
	int	codepoint) {

	int	i;


	for (i = 0; i < ENTRIES_IN_TABLE; i ++) 
		if (g_U2MTable[i] == codepoint) break;

        if (i >= ENTRIES_IN_TABLE)

		/*
		 *	This codepoint is never translated from any UCS codes,
		 *	so this can be used as undefined in the codepage.
		 */

		return TRUE;

        for (i ++; i < ENTRIES_IN_TABLE; i ++)
                if (g_U2MTable[i] == codepoint) break;

        if (i == ENTRIES_IN_TABLE)

		/*
		 *	This codepoint is translated from only one UCS code.
		 *	In this case, pair of codepoints of MBCS and UCS is
		 *	defined as invalid codepoint mark.
		 */

		return TRUE;
	else	return FALSE;
}

/*
 *   NAME:	terminate
 *
 *   FUNCTION:	Termination.
 *
 *   RETURNS:	None.
 *
 *   GLOBAL VARIABLES:
 *	g_outputFile		- Output file name.
 *	g_outTmpFile		- Temporary output file name.
 *	g_pInputFile		- Descriptor of the input file.
 *	g_pOutTmpFile		- Descriptor of the temporary output file.
 *	g_U2MTable		- Conversion table U2M.
 *	g_M2UTable		- Conversion table M2U.
 *	g_mbcsU2M		- Conversion table U2M for MBCS.
 *	g_mbcsRows		- Conversion ROWs for MBCS.
 *	g_mbcsPlanes		- MBCS plane information table.
 *	g_mbcsStems		- MBCS stem information table.
 */

static	void	terminate (int exit_stat) {

	/*
	 *	Close the source file.
	 */

	if (g_pInputFile != NULL) {
		if (fclose (g_pInputFile) == EOF) {
			if (exit_stat == M_NO_ERROR) {
				exit_stat = M_CANNOT_CLOSE_INPUT;
				displayMsg (exit_stat, g_inputFile, strerror (errno));
			}
		}
	}

	/*
	 *	Close the temp file and rename it to the output file.
	 */

	if (g_pOutTmpFile != NULL) {
		if (fclose (g_pOutTmpFile) == EOF) {
			if (exit_stat == M_NO_ERROR) {
				exit_stat = M_CANNOT_CLOSE_TEMP;
				displayMsg (exit_stat, strerror (errno), NULL);
			}
		}
		else if (exit_stat == M_NO_ERROR) {
			if (rename (g_outTmpFile, g_outputFile) != 0) {
				exit_stat = M_ERROR_RENAME;
				displayMsg (exit_stat, g_outputFile, strerror (errno));
			}
		}
	}

	/*
	 *	Display end message.
	 */

	if (exit_stat != M_NO_ERROR) {
		displayMsg (M_UNCOMPLETED, g_outputFile, NULL);
		remove (g_outTmpFile);
	}
	else	displayMsg (M_COMPLETED, g_outputFile, NULL);

	/*
	 *	Free allocated storage.
	 */

	if (g_U2MTable   != NULL) free (g_U2MTable);
	if (g_M2UTable   != NULL) free (g_M2UTable);
	if (g_mbcsU2M    != NULL) free (g_mbcsU2M);
	if (g_mbcsRows   != NULL) free (g_mbcsRows);
	if (g_mbcsPlanes != NULL) free (g_mbcsPlanes);
	if (g_mbcsStems  != NULL) free (g_mbcsStems);

	exit (exit_stat);
}

/*
 *   NAME:	str2token
 *
 *   FUNCTION:	Get corresponding token of specified string.
 *
 *   RETURNS:	(TOKENS)token
 *
 *   GLOBAL VARIABLES:
 *	g_tokenTable	- String : Token table.
 */

static	TOKENS		str2token (uchar_t *string) {

	uchar_t		wk_buf[LINE_MAX+1];
	int		i;
	_tokenName_t	*token_tbl;

	for (i = 0; string[i] != '\0'; i ++)
		wk_buf[i] = toupper (string[i]);
	wk_buf[i] = '\0';

	for (token_tbl = g_tokenTable;
	     token_tbl->name != NULL;
	     token_tbl ++) if (strncmp (wk_buf, token_tbl->name, i) == 0) break;

	return  token_tbl->token;
}

/*
 *   NAME:	token2str
 *
 *   FUNCTION:	Get corresponding string of specified token.
 *
 *   RETURNS:	Pointer to the string.
 *
 *   GLOBAL VARIABLES:
 *	g_tokenTable	String : Token table.
 */

static	uchar_t		*token2str (TOKENS token) {

	_tokenName_t	*token_tbl;

	for (token_tbl = g_tokenTable;
	     token_tbl->name != NULL;
	     token_tbl ++) if (token == token_tbl->token) return token_tbl->name;

	return NULL;
}

/*
 *   NAME:	checkUntilEOL
 *
 *   FUNCTION:	Parse the rest of the line from that a valid token
 *		has already been got.
 *
 *   RETURNS:	Error status code.
 *	M_EXTRA_CHARS		- Extra token in line.
 */

static	int	checkUntilEOL () {

	TOKENS	token;

	while (TRUE) {
		token = getToken ();
		if ((token == EOL) || (token == E_O_F)) break;
		if (token == SPACES) continue;
		return M_EXTRA_CHARS;
	}
	ungetToken (token);
	return M_NO_ERROR;
}

/*
 *   NAME:	ungetToken
 *
 *   FUNCTION:	Put a token back in the buffer.
 *
 *   RETURNS:	None
 *
 *   GLOBAL VARIABLES:
 *	g_ungetTokenBuf		- Buffer for ungot token.
 */

static	void	ungetToken (TOKENS token) {

	g_ungetTokenBuf = token;
}

/*
 *   NAME:	getToken
 *
 *   FUNCTION:	Get next token.
 *
 *   RETURNS:	This function returns (TOKENS)token, sets its value to the
 *		global variables shown as below,  and advances the g_index
 *		(character index in the line buffer) to the next token.
 *
 *   GLOBAL VARIABLES:
 *	g_lineBuf		- Source line buffer.
 *	g_index			- Character index in the line.
 *	g_ungetTokenBuf		- Buffer for ungot token.
 *
 *	These are where value of token is returned.
 *
 *	g_unichar		- UNICHAR
 *	g_hexStr		- HEX_CONST
 *	g_hexLen		- HEX_CONST
 *	g_string		- STRING
 *	g_number		- NUMBER / HEX_CONST (digit value)
 */

static	TOKENS		getToken () {

	TOKENS		token;
	ulong_t		ucs_char, len;
	uchar_t		h, l, wk_buf[LINE_MAX+1], *p;
	int		c, i, j;


	if (g_ungetTokenBuf != ERROR) {
		token = g_ungetTokenBuf;
		g_ungetTokenBuf = ERROR;	/* Reset */
		return token;
	}

	/*
	 *	Get one character from the source file and advance the g_index
	 *	(character index in the line buffer). If I/O error is occurred
	 *	the getChar() exits directly through the terminate().
	 */

	if ((c = getChar ()) == E_O_F) return E_O_F;

	/*
	 *	EOL
	 */

	if ((c == '\n') || (c == '\0') || (c == g_commentChar)) {
		if (g_index == 0) g_last_index = 0;
		else		  g_last_index = g_index - 1;
		g_index = LINE_MAX;	/* Logical EOL */
		return EOL;
	}

	/*
	 *	SPACES		Skip to next non-space character.
	 */

	if (isspace (c)) {
		for (i = g_index; isspace (c = g_lineBuf[i]); i ++);
		g_last_index = g_index;
		g_index = i;
		return SPACES;
	}

	/*
	 *	Here is a TOKEN SIGN '<'
	 */

	if (c == '<') {

		/*
		 *	UNICHAR  <Uxxxx>
		 */

		if (g_lineBuf[g_index] == 'U') {
			ucs_char = strtoul (&g_lineBuf[g_index+1], &p, 16);
			if ((*p == '>') &&
				((p - &g_lineBuf[g_index]) == 5) &&
				(ucs_char <= 0x0ffffL)) {
				g_unichar = (UniChar)(ucs_char & 0xffff);
				g_index += 6;
				return UNICHAR;
			}
		}

		/*
		 *	ANOTHER TOKEN  <xxxxx>
		 */

		for (i = g_index; g_lineBuf[i] != '>'; i ++) {
			if (isspace (g_lineBuf[i]) || (g_lineBuf[i] == '\0'))
				break;
		}
		if ((i - g_index) == 0)  return ERROR;
		if (g_lineBuf[i] != '>') return ERROR;
		g_lineBuf[i] = '\0';
		token = str2token (&g_lineBuf[g_index]);
		g_index = i + 1;
		return token;
	}

	/*
	 *	Here is a ESCAPE SIGN.
	 */

	if (c == g_escapeChar) {

		/*
		 *	HEX_CONST  \xXX\xXX
		 */

		i = g_index;
		j = len = 0;

		while (TRUE) {
			if ((g_lineBuf[i] == 'x') &&
				isxdigit (h = g_lineBuf[i+1]) &&
				isxdigit (l = g_lineBuf[i+2])) {

				i += 3;

				wk_buf[j ++] = h;
				wk_buf[j ++] = l;
				if (isdigit (h))
					g_hexStr[len] = h -'0';
				else	g_hexStr[len] = toupper (h) -'A'+10;
				g_hexStr[len] <<= 4;
				if (isdigit (l))
					g_hexStr[len]|= l -'0';
				else	g_hexStr[len]|= toupper (l) -'A'+10;

				if ((++ len) >= STEM_MAX+2) return ERROR;

				if (g_lineBuf[i ++] != g_escapeChar) break;
			}
			else	return ERROR;
		}

		/*
		 *	Convert the last 2 bytes of the codepoint to
		 *	digit value, and returns it via 'g_number'.
		 */

		wk_buf[j] = '\0';
		j = ((j - 4) > 0) ? (j - 4) : 0;
		g_number = (ushort_t)strtoul (&wk_buf[j], &p, 16);
		g_hexLen = (ushort_t)len;
		g_hexStr[len] = '\0';
		g_index = i - 1;
		return HEX_CONST;
	}

	/*
	 *	ELLIPSIS   ...
	 */

	if (c == '.') {
		if ((g_lineBuf[g_index  ] == '.') &&
		    (g_lineBuf[g_index+1] == '.')) {
			g_index += 2;
			return ELLIPSIS;
		}
		else	return UNKNOWN;
	}

	/*
	 *	STRING  "xxxxx"
	 */

	if (c == '\"') {
		for (i = g_index, j = 0;
		    (c = g_lineBuf[i]) != '\"';
		     i ++, j ++) {
			if ((c == '\n') || (c == '\0')) return ERROR;
			g_string[j] = c;
		}
		if (j == 0) return ERROR;
		g_string[j] = '\0';
		g_index = i + 1;
		return STRING;
	}

	/*
	 *	NUMBER
	 */

	if (isdigit (c)) {
		g_number = (ushort_t)strtol (&g_lineBuf[g_index-1], &p, 10);
		g_index += (int)(p - &g_lineBuf[g_index]);
		return NUMBER;
	}

	/*
	 *	CHARMAP
	 */

	if ((strncmp (&g_lineBuf[g_index-1], "charmap", 7) == 0) ||
	    (strncmp (&g_lineBuf[g_index-1], "CHARMAP", 7) == 0)) {
		g_index += 6;
		return START_CHARMAP;
	}

	/*
	 *	END CHARMAP
	 */

	if ((strncmp (&g_lineBuf[g_index-1], "end", 3) == 0) ||
	    (strncmp (&g_lineBuf[g_index-1], "END", 3) == 0)) {
		i = (g_index += 2);
		if (!isspace (g_lineBuf[i])) return ERROR;
		for (i ++; isspace (g_lineBuf[i]); i ++);
		if (!isgraph (g_lineBuf[i])) return ERROR;

		if ((strncmp (&g_lineBuf[i], "charmap", 7) == 0) ||
		    (strncmp (&g_lineBuf[i], "CHARMAP", 7) == 0)) {
			g_index = i + 7;
			return END_CHARMAP;
		} else	return ERROR;
	}
	return UNKNOWN;
}

/*
 *   NAME:	getChar
 *
 *   FUNCTION:	Get next logical character, bypassing comments and
 *		crossing physical line boundaries.
 *
 *   RETURNS:	Character got.
 *
 *   GLOBAL VARIABLES:
 *	g_lineNumber		- Counter of the source lines.
 *	g_lineBuf		- Buffer for the source lines.
 *	g_index			- Position of the character in the line buffer.
 */

static	int	getChar () {

	if (g_index >= LINE_MAX) {

		/*
		 *	If end of the source line,  Read next line.
		 *	If I/O error, exit through the terminate().
		 */

		while (TRUE) {
			g_lineBuf[0] = '\0';
			if (fgets (g_lineBuf, LINE_MAX, g_pInputFile) == NULL) {
				if (feof (g_pInputFile) && (g_lineBuf[0] == '\0'))
					return E_O_F;
				if (ferror (g_pInputFile)) {
					displayMsg (M_ERROR_READ,
					g_inputFile, strerror (errno));
					terminate  (M_ERROR_READ);
				}
			}
			g_lineNumber ++;
			g_index = g_last_index = 0;

			if (g_verboseFlag) {

				/*
				 *	If verbose option on, display the statement.
				 */

				uchar_t		rep_buf[LINE_MAX+5];
				int		len;

				len = strlen (g_lineBuf);
				if (g_lineBuf[len-1] != '\n') {
					g_lineBuf[len] = '\n';
					g_lineBuf[len+1] = '\0';
				}
				sprintf (rep_buf, "%4.4d %s", g_lineNumber, g_lineBuf);
				fwrite  (rep_buf, strlen (rep_buf), 1, stdout);
			}
			if (isspace (g_lineBuf[0]))
				displayMsg (M_INVAL_SPACE, NULL, NULL);
			else	break;
		}
	}
	g_last_index = g_index;
	return g_lineBuf [g_index ++];
}

/*
 *   NAME:	displayMsg
 *
 *   FUNCTION:	Display error messages.
 *
 *   RETURN:	None.
 *
 *   GLOBAL VARIABLES:
 *	g_lineBuf	- Buffer for source statements.
 *	g_lineNumber	- Counter of source statements.
 *	g_index		- Character index in the buffer.
 *	g_last_index	- Index of the last got token.
 *	g_verboseFlag	- Verbose option flag.
 */

static	void		displayMsg (
	int		errCode,	/* Error code                    */
	uchar_t		*msg1,		/* Texts to complete the message */
	uchar_t		*msg2) {	/*                               */

	uchar_t		*pMsg, msg_buf[LINE_MAX+1], err_pos[LINE_MAX+1];
	int		line = FALSE, i;

	pMsg = catgets (g_catd, UCONVDEF, errCode, g_defaultMsg[errCode]);

	if (strstr (pMsg, "%4.4d") != NULL) {
		line = TRUE;
		sprintf (msg_buf, pMsg, g_lineNumber, msg1, msg2);
	} else	sprintf (msg_buf, pMsg,               msg1, msg2);

	if (!g_verboseFlag) fprintf (stderr, msg_buf);

	else {	/*
		 *	Verbose option is ON.
		 */

		i = strlen (msg_buf);
		msg_buf[i  ] = '\n';
		msg_buf[i+1] = '\0';

		if ((errCode == M_USAGE)            ||
		    (errCode == M_START_PROCESSING) ||
		    (errCode == M_COMPLETED)        ||
		    (errCode == M_UNCOMPLETED)) {
			/* Not error messages */
			fprintf (stdout, msg_buf);
			return;
		}
		if (line) {
			i = g_last_index + 5;
			memset (err_pos, ' ', i);
			err_pos[i  ] = '^';
			err_pos[i+1] = '\n';
			err_pos[i+2] = '\0';
			fprintf (stdout, err_pos);
		} else	fprintf (stdout, "\n");
		fprintf (stdout, msg_buf);
	}
}
