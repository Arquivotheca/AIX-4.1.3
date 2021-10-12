static char sccsid[] = "@(#)15  1.8  src/bos/usr/bin/genxlt/genxlt.c, cmdiconv, bos411, 9428A410j 5/19/94 15:13:44";
/*
 *   COMPONENT_NAME:    CMDICONV
 *
 *   FUNCTIONS:         main
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _ILS_MACROS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <iconv.h>
#include <iconvTable.h>
#include <ctype.h>

#include "genxlt_msg.h" 

nl_catd catd;

#define MSGSTR(Num, Str)	catgets(catd, MS_genxlt, Num, Str)
#ifndef	LINE_MAX
#define	LINE_MAX		1024
#endif
#define NO_MARK			0
#define VALID_MARK		1
#define	INVAL_MARK		2
#define SUBST_MARK		3
#define INVAL_CHARCODE		"invalid"
#define SUBST_CHARCODE		"substitution"
#define SUBST_TARGET		"SUB"

/*
 *   NAME:	main
 *
 *   FUNCTION:	Creates a compiled version of a translation tables suitable
 * 		for use by ICONV table converter. The source is expected to
 *		be standard input.  The target is standard output or a file
 *		which can be optionally specified on the command line.
 *
 *   RETURN VALUE DESCRIPTION: (reterns to the execution environment)
 *	0	- Successfully completed.
 *	1	- Error was occurred.
 *
 *   NOTE: Translation table statement
 *
 *	Each statement is pare of conversion source and destination code.
 *
 *		<source code>  <destination code>  (<comment>)
 *
 *	Statement beginning with '#' is a comment line.
 *
 *		# This is a comment line.
 *
 *	For source character code that does not belong to the souce code set,
 *	"invalid" is in place of destination code.
 *
 *		0x00           invalid
 *
 *	When source character code has no target code for identical conversion,
 *	it is to be converted to substitution character  defined in the target
 *	code set.  In this case "substitution" is in place of destination code.
 *
 *		0x7f           substitution
 *
 *	And substitution code is defined by a statement beginning with "SUB".
 *
 *		SUB            0x3f
 */
/*
 *	IMPORTANT !
 *	This comand generates ccsid data table which is used by the libiconv
 *	cstoccsid() and ccsidtocs() functions for CS NAME<->CCSID conversion,
 *	in addition to ordinal single byte iconv data table. 
 *
 *	The syntax is:
 *		genxlt -ccsid
 *
 *		If the flag [-ccsid] is specified, the command takes input
 *		from stdin and outputs a ccsid table to stdout.  NO INPUT/
 *		OUTPUT FILE CAN BE SPECIFIED.
 *
 *	A DATA FORMAT OF CCSID TABLE IS COMPLETELY DIFFERENT FROM THAT OF THE
 *	ICONV TABLE.  It is recommended to have a new command (e.g. genccsid)
 *	to produce ccsid tables,   and this new option should be removed from
 *	the genxlt command in the future release.
 */

extern	int	genccsid (nl_catd catd);

main (int argc, char **argv, char **envp) {

	IconvTable	iconvtable;	/* Conversion Table   */
	FILE 		*out_fd;
	uchar_t		line_buf[LINE_MAX], *src_ptr, *dst_ptr, *p;
	int		line, src_code, dst_code, subst_defined, i, j;
	uchar_t		src_char_map[256];
	uchar_t		dst_char_map[256];
	uchar_t		inval_char[2];


	setlocale (LC_ALL, "");
	catd = catopen (MF_GENXLT, NL_CAT_LOCALE);

	/*
	 *	The following statements will be removed when a new
	 *	command to produce a ccsid table is added to the AIX.
	 */
	
	if ((argc >= 2) && (strcmp (argv[1], "-ccsid") == 0))
		exit (genccsid (catd));

	if (argc >= 3) {
		fprintf (stderr, MSGSTR (M_MSG_1, "Usage: genxlt [target]\n"));
		exit (1);
	}

	/*
	 *	Initialize conversion table.
	 */

	iconvtable.magic        = ICONV_REL2_MAGIC;
	iconvtable.inval_handle = FALSE;
	iconvtable.inval_char   = 0;
	iconvtable.sub_handle   = FALSE;
	iconvtable.sub_mark     = 0;
	iconvtable.sub_char     = 0;
	memset (iconvtable.dummy,0, sizeof (iconvtable.dummy));
	memset (iconvtable.data, 0, sizeof (iconvtable.data));
	memset (src_char_map, NO_MARK, sizeof (src_char_map));
	memset (dst_char_map, NO_MARK, sizeof (dst_char_map));
	subst_defined = FALSE;

	/*
	 *	Build up the table.
	 */

	line = 0;
	while (TRUE) {

		if (fgets (line_buf, LINE_MAX, stdin) == NULL) break;
		line ++;

		/*
		 *	Get source and destination character code.
		 */

		for (p = line_buf; isspace (*p); p ++);
		if ((*p == '#') ||		/* Comment line */
		    (*p == '\0')) continue;	/* No statement */

		src_ptr = p;
		for (; isgraph (*p); p ++); if (*p != '\0') { *p = '\0'; p ++; }
		for (; isspace (*p); p ++); if (*p == '\0') {
			fprintf (stderr, MSGSTR (M_MSG_5,
			"genxlt: Invalid format at line %d\n"), line);
			exit (1);
		}
		dst_ptr = p;
		for (; isgraph (*p); p ++); if (*p != '\0') *p = '\0';

		/*
		 *	Get substitution character code.
		 */

		if (strcmp (src_ptr, SUBST_TARGET) == 0) {
			if (sscanf (dst_ptr, "%x", &dst_code) == -1) {
				fprintf (stderr, MSGSTR (M_MSG_5,
				"genxlt: Invalid format at line %d\n"), line);
				exit (1);
			}
			iconvtable.sub_char = (uchar_t)dst_code;
			subst_defined = TRUE;
			continue;
		}

		/*
		 *	Set source and destination code into the table.
		 */

		if (sscanf (src_ptr, "%x", &src_code) == -1) {
			fprintf (stderr, MSGSTR (M_MSG_5,
			"genxlt: Invalid format at line %d\n"), line);
			exit (1);
		}
		if (strcmp (dst_ptr, INVAL_CHARCODE) == 0)
			src_char_map[src_code] = INVAL_MARK;
		else if (strcmp (dst_ptr, SUBST_CHARCODE) == 0)
			src_char_map[src_code] = SUBST_MARK;
		else {
			if (sscanf (dst_ptr, "%x", &dst_code) == -1) {
				fprintf (stderr, MSGSTR (M_MSG_5,
				"genxlt: Invalid format at line %d\n"), line);
				exit(1);
			}
			src_char_map[src_code] = VALID_MARK;
			dst_char_map[dst_code] = VALID_MARK;
			iconvtable.data[src_code] = dst_code;
		}
	}

	/*
	 *	Search un-used character code in the destination code set
	 *	for use of invalid and/or substitution code mark.
	 */

	for (i = j = 0; (i < 256) && (j < 2); i++) {
		if (dst_char_map[i] == NO_MARK) {
			inval_char[j ++] = (uchar_t)i;
		}
	}

	/*
	 *	Set invalid and/or substitution code marks into the table.
	 */

	for (i = j = 0; i < 256; i++) {
		if (src_char_map[i] == INVAL_MARK) {
			if (!iconvtable.inval_handle) {
				iconvtable.inval_handle = TRUE;
				iconvtable.inval_char = inval_char[j ++]; 
			}
			iconvtable.data[i] = iconvtable.inval_char;
		}
		else if (src_char_map[i] == SUBST_MARK) {
			if (!subst_defined) {
				fprintf (stderr, MSGSTR (M_MSG_6,
				"genxlt: No substitution character is defined.\n"));
				exit (1);
			}
			if (!iconvtable.sub_handle) {
				iconvtable.sub_handle = TRUE;
				iconvtable.sub_mark = inval_char[j ++];
			}
			iconvtable.data[i] = iconvtable.sub_mark;
		}
		else if (src_char_map[i] == NO_MARK) {
			fprintf (stderr, MSGSTR (M_MSG_3,
			"genxlt: There was no assignment for index %d\n"), i);
			exit(1);
		}
	}

	/*
	 *	Write created table into output file.
	 */

	if (argc == 2) {
		if ((out_fd = fopen (argv[1], "w")) == NULL) {
			fprintf (stderr, MSGSTR (M_MSG_2,
			"genxlt: Unable to open target file.\n"));
			exit (1);
		}
	}
	else	out_fd = stdout;

	if (fwrite (&iconvtable, sizeof (IconvTable), 1, out_fd) != 1) {
		fprintf (stderr, MSGSTR (M_MSG_4,
		"genxlt: Unable to write to target file.\n"));
		exit (1);
	}
	fclose (out_fd);
	exit (0);
}
