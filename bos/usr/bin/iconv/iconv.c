static char sccsid[] = "@(#)07  1.11.1.2  src/bos/usr/bin/iconv/iconv.c, cmdiconv, bos41J, 9512A_all 2/22/95 22:19:49";
/*
 *   COMPONENT_NAME:	CMDICONV
 *
 *   FUNCTIONS:		Iconv command.
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

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <locale.h>
#include <iconv.h>

#include "iconv_msg.h"

/*
 *	Internal use macros.
 */

#define	MSGSTR(Num)	catgets(_catd, MS_ICONV, Num, _defmsg[Num])
#define ALLOC_UNIT	4096

/*
 *	Static areas.
 */

static	uchar_t		*_inbuf    = NULL;	/* input  buffer          */
static	uchar_t		*_outbuf   = NULL;	/* output buffer          */
static	int		_inbufsize = ALLOC_UNIT;/* input  buffer size     */
static	int		_outbufsize= ALLOC_UNIT;/* output buffer size     */
static	int		_infd      = 0;		/* input  file descriptor */
static	int		_outfd     = 1;		/* output file descriptor */
static	iconv_t		_cd        = NULL;	/* conversion  descriptor */
static	nl_catd		_catd      = NULL;	/* msg catalog descriptor */
static	uchar_t		*_defmsg[] = {		/* default message        */
/*Dummy        */	"",
/*M_ARGBOO     */	"argument not valid\n",
/*M_FROMMISS   */	"fromcode is missing\n",
/*M_TOMISS     */	"tocode is missing\n",
/*M_CANNOTCONV */	"cannot open converter\n",
/*M_CANNOTFILE */	"cannot open input file\n",
/*M_INVALIDCHAR*/	"invalid chararacter found\n",
/*M_TRUNC      */	"truncated character found\n",
/*M_NOMEMORY   */	"unable to allocate enough memory\n",
/*M_IOERROR    */	"I/O error\n",
/*M_USAGE      */	"Usage: iconv -f FromCode -t ToCode [FileName...]\n"
};

/*
 *   NAME:	_process_one_file
 *
 *   FUNCTION:	Process one input file.
 *
 *   RETURNS:	Exit status code.
 *
 */

static	int		_process_one_file () {

	uchar_t		*inptr, *outptr;
	size_t		inleft, outleft, rc;
	int		bytesread, bytesinput, end_of_input;

	end_of_input = FALSE;
	inleft = 0;

	while (TRUE) {

		inptr   = _inbuf;
		outptr  = _outbuf;
		outleft = _outbufsize;

		if ((bytesread = read (_infd,
			inptr + inleft, _inbufsize - inleft)) < 0) {
			fprintf (stderr, MSGSTR (M_IOERROR));
			return 1;
		}
		if ((bytesinput = (inleft += bytesread)) == 0) {

			/*
			 *	No more input string, then reset status.
			 */

			end_of_input = TRUE;
			rc = iconv (_cd, (const char**)NULL, &inleft,
				&outptr, &outleft);
		}
		else	rc = iconv (_cd, (const char**)&inptr, &inleft,
				&outptr, &outleft);

		if (write (_outfd, _outbuf, _outbufsize - outleft) < 0) {
			fprintf (stderr, MSGSTR (M_IOERROR));
			return 1;
		}
		if ((rc != -1) && (inleft == 0)) {

			/*
			 *	Conversion is successfully completed.
			 */

			if (end_of_input) return 0;
			else		  continue;
		}
		switch (errno) {
		case EILSEQ:

			/*
			 *	Invalid character is found.
			 */

			fprintf (stderr, MSGSTR (M_INVALIDCHAR));
			return 2;

		case EINVAL:

			/*
			 *	Truncated character is at the end of input buffer.
			 */

			if (inptr == _inbuf) {

				/*
				 *	No conversion was performed.
				 */

				if (inleft == _inbufsize) {

					/*
					 *	Too small input buffer.
					 */

					_inbufsize += ALLOC_UNIT;
					if ((_inbuf = realloc (
						_inbuf, _inbufsize)) == NULL) {
						fprintf (stderr, MSGSTR (M_NOMEMORY));
						return 1;
					}
				}
				else if (bytesread == 0) {

					/*
					 *	No more string from the input file.
					 *	We can not process this truncated
					 *	character any more.
					 */ 

					fprintf (stderr, MSGSTR (M_TRUNC));
					return 2;
				}
			}

			/*
			 *	Move the left string to the beginning
			 *	of the input buffer for next read.
			 */

			memcpy (_inbuf, _inbuf + bytesinput - inleft, inleft);
			break;

		case E2BIG:

			/*
			 *	Lack of space in the output buffer.
			 */

			if (_inbuf == inptr) {

				/*
				 *	Too small to do conversion.
				 */

				_outbufsize += ALLOC_UNIT;
				if ((_outbuf = realloc (
					_outbuf, _outbufsize)) == NULL) {
					fprintf (stderr, MSGSTR (M_NOMEMORY));
					return 1;
				}
			}
			else {	/*
				 *	Move the left string to the beginning
				 *	of the input buffer for next read.
				 */

				memcpy (_inbuf, _inbuf + bytesinput - inleft, inleft);
			}
			break;

		default: return 1;
		}
	}
}

/*
 *   NAME:	_my_exit
 *
 *   FUNCTION:	Free all allocated resources, and exit program.
 *
 *   RETURNS:	None
 */

static	void	_my_exit (
	int	exit_stat) {	/* exit status code */

	if (_infd   != 0)	close       (_infd);
	if (_outfd  != 1)	close       (_outfd);
	if (_inbuf  != NULL)	free        (_inbuf);
	if (_outbuf != NULL)	free        (_outbuf);
	if (_cd     != NULL)	iconv_close (_cd);
	if (_catd   != NULL)	catclose    (_catd);

	exit (exit_stat);
}

/*
 *	MAIN.
 */

main (int argc, char *argv[]) {

	uchar_t		*fcode    = NULL;	/* source code set name       */
	uchar_t		*tcode    = NULL;	/* target code set name       */
	int		exit_stat = 0;		/* exit status code save area */
	int		c;			/* command line flag letter   */


	setlocale (LC_MESSAGES, "");
	_catd = catopen (MF_ICONV, NL_CAT_LOCALE);

	/*
	 *	Get line parameters.
	 */

	if (argc < 2) {
		fprintf (stderr, MSGSTR (M_USAGE));
		_my_exit (1);
	}
	opterr = 0;
	while ((c = getopt (argc, argv, "f:t:")) != EOF) {
		switch (c) {
		case 'f': fcode = optarg; break;
		case 't': tcode = optarg; break;
		case '?':
		default : fprintf (stderr, MSGSTR (M_ARGBOO));
			  fprintf (stderr, MSGSTR (M_USAGE));
			  _my_exit (1);
		}
	}
	if (fcode == NULL) {
		fprintf (stderr, MSGSTR (M_FROMMISS));
		fprintf (stderr, MSGSTR (M_USAGE));
		_my_exit (1);
	}
	if (tcode == NULL) {
		fprintf (stderr, MSGSTR (M_TOMISS));
		fprintf (stderr, MSGSTR (M_USAGE));
		_my_exit (1);
    	}
	
	/*
	 *	Initial allocation of I/O buffers.
	 */

	if ((_inbuf = malloc (_inbufsize)) == NULL) {
		fprintf (stderr, MSGSTR (M_NOMEMORY));
		_my_exit (1);
	}
	if ((_outbuf = malloc (_outbufsize)) == NULL) {
		fprintf (stderr, MSGSTR (M_NOMEMORY));
		_my_exit (1);
	}

	/*
	 *	Open iconv converter.
	 */

	if ((_cd = iconv_open ((const char*)tcode, (const char*)fcode))
		== (iconv_t)-1) {
		fprintf (stderr, MSGSTR (M_CANNOTCONV));
		_my_exit (1);
	}

	/*
	 *	If no file operand specified, use STDIN.
	 */

	if (optind == argc) _my_exit (_process_one_file ());

	/*
	 *	Process input files.
	 */

	for (; optind < argc; optind ++) {

		if (strcmp (argv[optind], "-") == 0) {

			_infd = 0;
		}
		else if ((_infd = open (argv[optind], O_RDONLY)) < 0) {
			fprintf (stderr, MSGSTR (M_CANNOTFILE));
			exit_stat = 1;
			continue;
		}
		exit_stat |= _process_one_file ();
		if (_infd > 0) close (_infd);
	}
	_my_exit (exit_stat);
}
