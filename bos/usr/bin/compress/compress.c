static char sccsid[] = "@(#)85  1.38  src/bos/usr/bin/compress/compress.c, cmdfiles, bos412, 9446C 11/14/94 16:47:47";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: compress
 *
 * ORIGINS: 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */
/* 
 * Compress - data compression program 
 */
#define _ILS_MACROS
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <sys/param.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <errno.h>

#define	min(a,b)	((a>b) ? b : a)

#include "compress_msg.h" 
static nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_COMPRESS,n,s) 
/*
 * Set USERMEM to the maximum amount of physical user memory available
 * in bytes.  USERMEM is used to determine the maximum BITS that can be used
 * for compression.
 *
 * SACREDMEM is the amount of physical memory saved for others; compress
 * will hog the rest.
 */
#ifndef SACREDMEM
#define SACREDMEM	0
#endif

#ifndef USERMEM
#define USERMEM 	450000	/* default user memory */
#endif

#ifndef BITS
#define BITS	16
#endif
#define HSIZE	69001
/*
 * a code_int must be able to hold 2**BITS values of type int, and also -1
 */
typedef long int	code_int;
typedef long int	  count_int;
typedef	unsigned char	char_type;
static char_type magic_header[] = { "\037\235" };	/* 1F 9D */

/* Defines for third byte of header */
#define BIT_MASK	0x1f
#define BLOCK_MASK	0x80
/* Masks 0x40 and 0x20 are free.  I think 0x20 should mean that there is
   a fourth header byte (for expansion).
*/
#define INIT_BITS 9			/* initial number of bits/code */

static int n_bits;				/* number of bits/code */
static int maxbits = BITS;			/* user settable max # bits/code */
static code_int maxcode;			/* maximum code, given n_bits */
static code_int maxmaxcode = 1 << BITS;	/* should NEVER generate this code */
#define MAXCODE(n_bits)	((1 << (n_bits)) - 1)

static count_int htab[HSIZE];
static unsigned short codetab[HSIZE];

#define htabof(i)	htab[i]
#define codetabof(i)	codetab[i]
static code_int hsize = HSIZE;			/* for dynamic table sizing */
static count_int fsize;

/*
 * To save much memory, we overlay the table used by compress() with those
 * used by decompress().  The tab_prefix table is the same size and type
 * as the codetab.  The tab_suffix table needs 2**BITS characters.  We
 * get this from the beginning of htab.  The output stack uses the rest
 * of htab, and contains characters.  There is plenty of room for any
 * possible stack (stack used to be 8000 characters).
 */

#define tab_prefixof(i)	codetabof(i)
#define tab_suffixof(i)	((char_type *)(htab))[i]
#define de_stack		((char_type *)&tab_suffixof(1<<BITS))

static code_int free_ent = 0;			/* first unused entry */
static int exit_stat = 0;			/* per-file status */
static int perm_stat = 0;			/* permanent status */

static code_int getcode();

#ifdef C_DEBUG
static int debug = 0;
#endif /* C_DEBUG */

static int nomagic = 0;	/* Use a 3-byte magic number header, unless old file */
static int zcat_flg = 0;	/* Write output on stdout, suppress messages */
static int uncomp_flg = 0;	/* Command = uncompress, not  compress -d */
static int precious = 1;	/* Don't unlink output file on interrupt */
static int quiet = 1;		/* don't tell me about compression */

/*
 * block compression parameters -- after all codes are used up,
 * and compression rate changes, start over.
 */
static int block_compress = BLOCK_MASK;
static int clear_flg = 0;
static long int ratio = 0;
#define CHECK_GAP 10000	/* ratio check interval */
static count_int checkpoint = CHECK_GAP;
/*
 * the next two codes should not be changed lightly, as they must not
 * lie within the contiguous general code space.
 */ 
#define FIRST	257	/* first free entry */
#define	CLEAR	256	/* table clear output code */

static int force = 0;
static char ofname [PATH_MAX];
#ifdef C_DEBUG
static int verbose = 0;
#endif /* C_DEBUG */
static int (*oldint)(void);
static int bgnd_flag;

static int do_decomp = 0;

static onintr (void);
static oops (void);

/*****************************************************************
 * TAG( main )
 *
 * Algorithm from "A Technique for High Performance Data Compression",
 * Terry A. Welch, IEEE Computer Vol 17, No 6 (June 1984), pp 8-19.
 *
 * Usage: compress [-dfvc] [-b bits] [file ...]
 * Inputs:
 *	-d:	    If given, decompression is done instead.
 *
 *      -c:         Write output on stdout, don't remove original.
 *
 *      -b:         Parameter limits the max number of bits/code.
 *
 *	-f:	    Forces output file to be generated, even if one already
 *		    exists, and even if no space is saved by compressing.
 *		    If -f is not used, the user will be prompted if stdin is
 *		    a tty, otherwise, the output file will not be overwritten.
 *
 *      -v:	    Write compression statistics
 *
 * 	file ...:   Files to be compressed.  If none specified, stdin
 *		    is used.
 * Outputs:
 *	file.Z:	    Compressed form of file with same mode, owner, and utimes
 * 	or stdout   (if stdin used as input)
 *
 * Assumptions:
 *	When filenames are given, replaces with the compressed version
 *	(.Z suffix) only if the file decreases in size.
 * Algorithm:
 * 	Modified Lempel-Ziv method (LZW).  Basically finds common
 * substrings and replaces them with a variable size code.  This is
 * deterministic, and can be done on the fly.  Thus, the decompression
 * procedure needs no input table, but tracks the way the table was built.
 */

main( argc, argv )
register int argc; char **argv;
{
    int overwrite = 0;	/* Do not overwrite unless given -f flag */
    char tempname[PATH_MAX];
    char **filelist, **fileptr;
    char *cp, *rindex();
    struct stat statbuf;
    char *optstr;
    int c, stdin_sav, stdout_sav;

    (void ) setlocale(LC_ALL,"");
    catd = catopen(MF_COMPRESS, NL_CAT_LOCALE);
    if ( (oldint = (int (*)(void))signal(SIGINT,SIG_IGN )) != (int (*)(void)) SIG_IGN) {
	signal ( SIGINT, (void (*)(int))onintr );
	signal ( SIGSEGV, (void (*)(int))oops );
    }
    bgnd_flag = (tcgetpgrp(0) !=  getpgrp());

    filelist = fileptr = (char **)(malloc(argc * sizeof(*argv)));
    *filelist = NULL;

    if((cp = rindex(argv[0], '/')) != 0) {
	cp++;
    } else {
	cp = argv[0];
    }
    if(strcmp(cp, "uncompress") == 0) {
	do_decomp = 1;
	uncomp_flg = 1;
	optstr = "cDfFnqvV";
    } else if(strcmp(cp, "zcat") == 0) {
	do_decomp = 1;
	zcat_flg = 1;
	optstr = "DnV";
    } else 
	optstr = "b:cCDdfFnqvV";

    setlinebuf( stderr );

    /* Argument Processing
     * All flags are optional.
     * -D => debug
     * -V => print Version; debug verbose
     * -d => do_decomp
     * -v => unquiet
     * -f => force overwrite of output file
     * -n => no header: useful to uncompress old files
     * -b maxbits => maxbits.  If -b is specified, then maxbits MUST be
     *	    given also.
     * -c => cat all output to stdout
     * -C => generate output compatible with compress 2.0.
     * if a string is left, must be an input filename.
     */
	while (( c = getopt(argc, argv, optstr)) != EOF) {
		switch (c) {
#ifdef C_DEBUG
		    case 'D':
			debug = 1;
			break;
		    case 'V':
			verbose = 1;
			version();
			break;
#else
		    case 'V':
			version();
			break;
#endif /* C_DEBUG */
		    case 'v':
			quiet = 0;
			break;
		    case 'd':
			do_decomp = 1;
			break;
		    case 'f':
		    case 'F':
			overwrite = 1;
			force = 1;
			break;
		    case 'n':
			nomagic = 1;
			break;
		    case 'C':
			block_compress = 0;
			break;
		    case 'b':
                        maxbits = (int)strtoul(optarg,&optarg,10);
                        if (*optarg != '\0') {
				fprintf(stderr,MSGSTR(MISSMAXBT,
				"compress: Invalid number for option %c.\n"),c);
				exit(1);
                        }
			break;
		    case 'c':
			zcat_flg = 1;
			break;
		    case 'q':
			quiet = 1;
			break;
		    default:
			Usage();
		}
    }

    argc -= optind;
    filelist = &argv[optind];

    if(maxbits < INIT_BITS) maxbits = INIT_BITS;
    if (maxbits > BITS) maxbits = BITS;
    maxmaxcode = 1 << maxbits;

    if (*filelist != NULL) {
	stdin_sav = dup(0);
	stdout_sav = dup(1);
	for (fileptr = filelist; *fileptr; fileptr++) {
	    exit_stat = 0;
	    if (!strcmp(*fileptr, "-")) {
		FILE		*tfp;

		(void) fclose(stdin);
		(void) fclose(stdout);
		(void) dup(stdin_sav);
		(void) dup(stdout_sav);
		tfp = fdopen(0, "r");
		*stdin = *tfp;
		tfp = fdopen(1, "w");
		*stdout = *tfp;
		(void) do_stdin();
		continue;
	    }
            if(strlen(*fileptr) > (PATH_MAX-2) ) {
		fprintf(stderr, MSGSTR(NAME2LONG, "%s: filename is too long\n"), /*MSG*/
        	*fileptr);
		perm_stat = 1;
        	continue;
     	    }
	    if (do_decomp) {			/* DECOMPRESSION */
		/* Check for .Z suffix */
		if (strcmp(*fileptr + strlen(*fileptr) - 2, ".Z") != 0) {
		    /* No .Z: tack one on */
		    strcpy(tempname, *fileptr);
		    strcat(tempname, ".Z");
		    *fileptr = tempname;
		}
		/* Open input file */
		if ((freopen(*fileptr, "r", stdin)) == NULL) {
		    perror(*fileptr);
		    perm_stat = 1;
		    continue;
		}
		/* Check the magic number */
		if (nomagic == 0) {
		    if ((getchar() != (magic_header[0] & 0xFF))
		     || (getchar() != (magic_header[1] & 0xFF))) {
			fprintf(stderr, MSGSTR(NOTCOMP, "%s: not in compressed format\n"), /*MSG*/
			    *fileptr);
		    	perm_stat = 1;
			continue;
		    }
		    maxbits = getchar();	/* set -b from file */
		    block_compress = maxbits & BLOCK_MASK;
		    maxbits &= BIT_MASK;
		    maxmaxcode = 1 << maxbits;
		    if(maxbits > BITS) {
			fprintf(stderr,
			MSGSTR(BITSWRONG, "%s: compressed with %d bits, can only handle %d bits\n"), /*MSG*/
			*fileptr, maxbits, BITS);
			continue;
		    }
		}
		/* Generate output filename */
		strcpy(ofname, *fileptr);
		ofname[strlen(*fileptr) - 2] = '\0';  /* Strip off .Z */
	    } else {					/* COMPRESSION */
		if (strcmp(*fileptr + strlen(*fileptr) - 2, ".Z") == 0) {
		    	fprintf(stderr, MSGSTR(ALREADYZ, "%s: already has .Z suffix -- no change\n"), /*MSG*/
			    *fileptr);
		    continue;
		}
		/* Open input file */
		if ((freopen(*fileptr, "r", stdin)) == NULL) {
		    perror(*fileptr);
		    perm_stat = 1;
		    continue;
		}

	        /* get stat on input file. If it is not a regular file,
	           eg, a directory, the compress or uncompress will not do any
	           thing on this entry */
	        if (lstat(*fileptr, &statbuf)) {
		    perror(*fileptr);
		    exit(1);
	        }
    	        if ((statbuf.st_mode & S_IFMT/*0170000*/) != S_IFREG/*0100000*/)
	        {
		    fprintf(stderr, MSGSTR(NOTREG2, "%s: -- not a regular file: unchanged\n"), *fileptr); /*MSG*/
		    exit_stat = 1;
		    continue;
	        }

		fsize = (long) statbuf.st_size;
		/*
		 * tune hash table size for small files -- ad hoc,
		 * but the sizes match earlier #defines, which
		 * serve as upper bounds on the number of output codes. 
		 */
		hsize = HSIZE;
		if ( fsize < (1 << 12) )
		    hsize = min ( 5003, HSIZE );
		else if ( fsize < (1 << 13) )
		    hsize = min ( 9001, HSIZE );
		else if ( fsize < (1 << 14) )
		    hsize = min ( 18013, HSIZE );
		else if ( fsize < (1 << 15) )
		    hsize = min ( 35023, HSIZE );
		else if ( fsize < 47000 )
		    hsize = min ( 50021, HSIZE );
		else
		    hsize = HSIZE;

		/* Generate output filename */
		strcpy(ofname, *fileptr);
		strcat(ofname, ".Z");
	    }
	    /*
	     * If -f is not specified (overwrite == 0), compress/uncompress
	     * should only prompt to overwrite if it is a foreground process;
	     * otherwise, it should write a diagnostic message to stderr
	     * and set the exit status > 0.
	     */
	    if (overwrite == 0 && zcat_flg == 0) {
		if (stat(ofname, &statbuf) == 0) {
		    char response[100];
		    response[0] = 'n';
		    fprintf(stderr, MSGSTR(ALRDYEXST, "%s already exists;"), ofname);
		    if (!isatty(stdin_sav) || bgnd_flag || !isatty(2)) {
		  	perm_stat = 1;
		    } else {
			fprintf(stderr, MSGSTR(WSHOVRWRIT, " do you wish to overwrite %s (y or n)? "),
			ofname);
			fflush(stderr);
			{
				FILE *tempfp = fopen("/dev/tty", "r");
				if (tempfp == NULL) 
					strcpy(response, "n\n");
				else
					while (fgets(response, (int)sizeof(response), tempfp) != NULL)
						if (response[0] != '\n')
							break;
			}
			response[strlen(response) - 1] = (char) NULL;
			if (rpmatch(response) != 1) 
				response[0] = 'n';
			else
				response[0] = 'y';
		    }
		    if (response[0] != 'y') {
			fprintf(stderr, MSGSTR(NOTOVRWRIT, "\tnot overwritten\n")); /*MSG*/
			continue;
		    }
		}
	    }
	    if(zcat_flg == 0) {		/* Open output file */
		if (freopen(ofname, "w", stdout) == NULL) {
		    perror(ofname);
		    perm_stat = 1;
		    continue;
		}
		precious = 0;
		if(!quiet)
			fprintf(stderr, "%s: ", *fileptr);
	    }

	    /* Actually do the compression/decompression */
	    if (do_decomp == 0)	compress();
#ifndef C_DEBUG
	    else			decompress();
#else
	    else if (debug == 0)	decompress();
	    else			printcodes();
	    if (verbose)		dump_tab();
#endif /* C_DEBUG */
	    if(zcat_flg == 0) {
		copystat(*fileptr, ofname);	/* Copy stats */
		precious = 1;
	    }
	}
    } else	/* Standard input */
	(void) do_stdin();
    exit(perm_stat ? perm_stat : exit_stat);
}

static
do_stdin()
{
	if (do_decomp == 0) {
		compress();
#ifdef C_DEBUG
		if(verbose)		dump_tab();
#endif /* C_DEBUG */
	} else {
	    /* Check the magic number */
	    if (nomagic == 0) {
		if ((getchar()!=(magic_header[0] & 0xFF))
		 || (getchar()!=(magic_header[1] & 0xFF))) {
		    fprintf(stderr, MSGSTR(INNOTCOMP, "stdin: not in compressed format\n")); /*MSG*/
		exit(1);
		}
		maxbits = getchar();	/* set -b from file */
		block_compress = maxbits & BLOCK_MASK;
		maxbits &= BIT_MASK;
		maxmaxcode = 1 << maxbits;
		fsize = 100000;		/* assume stdin large for USERMEM */
		if(maxbits > BITS) {
			fprintf(stderr,
			MSGSTR(STDINBITS, "stdin: compressed with %d bits, can only handle %d bits\n"), /*MSG*/
			maxbits, BITS);
			exit(1);
		}
	    }
#ifndef C_DEBUG
	    decompress();
#else
	    if (debug == 0)	decompress();
	    else		printcodes();
	    if (verbose)	dump_tab();
#endif /* C_DEBUG */
	}
	if (fclose(stdout)) {
		perror("compress");
		exit(errno);
	}
}

static int offset;
static long int in_count = 1;			/* length of input */
static long int bytes_out;			/* length of compressed output */
static long int out_count = 0;			/* # of codes output (for debugging) */

/*
 * compress stdin to stdout
 *
 * Algorithm:  use open addressing double hashing (no chaining) on the 
 * prefix code / next character combination.  We do a variant of Knuth's
 * algorithm D (vol. 3, sec. 6.4) along with G. Knott's relatively-prime
 * secondary probe.  Here, the modular division first probe is gives way
 * to a faster exclusive-or manipulation.  Also do block compression with
 * an adaptive reset, whereby the code table is cleared when the compression
 * ratio decreases, but after the table fills.  The variable-length output
 * codes are re-sized at this point, and a special CLEAR code is generated
 * for the decompressor.  Late addition:  construct the table according to
 * file size for noticeable speed improvement on small files.  Please direct
 * questions about this implementation to ames!jaw.
 */

static
compress() {
    register long fcode;
    register code_int i = 0;
    register int c;
    register code_int ent;
    register int disp;
    register code_int hsize_reg;
    register int hshift;

    if (nomagic == 0) {
	putchar(magic_header[0]); putchar(magic_header[1]);
	putchar((char)(maxbits | block_compress));
	if(ferror(stdout))
		writeerr();
    }

    offset = 0;
    bytes_out = 3;		/* includes 3-byte header mojo */
    out_count = 0;
    clear_flg = 0;
    ratio = 0;
    in_count = 1;
    checkpoint = CHECK_GAP;
    maxcode = MAXCODE(n_bits = INIT_BITS);
    free_ent = ((block_compress) ? FIRST : 256 );

    ent = getchar ();

    hshift = 0;
    for ( fcode = (long) hsize;  fcode < 65536L; fcode *= 2L )
    	hshift++;
    hshift = 8 - hshift;		/* set hash code range bound */

    hsize_reg = hsize;
    cl_hash( (count_int) hsize_reg);		/* clear hash table */

    while ( (c = getchar()) != EOF ) {
	in_count++;
	fcode = (long) (((long) c << maxbits) + ent);
 	i = ((c << hshift) ^ ent);	/* xor hashing */

	if ( htabof (i) == fcode ) {
	    ent = codetabof (i);
	    continue;
	} else if ( (long)htabof (i) < 0 )	/* empty slot */
	    goto nomatch;
 	disp = hsize_reg - i;		/* secondary hash (after G. Knott) */
	if ( i == 0 )
	    disp = 1;
probe:
	if ( (i -= disp) < 0 )
	    i += hsize_reg;

	if ( htabof (i) == fcode ) {
	    ent = codetabof (i);
	    continue;
	}
	if ( (long)htabof (i) > 0 ) 
	    goto probe;
nomatch:
	output ( (code_int) ent );
	out_count++;
 	ent = c;
	if ( free_ent < maxmaxcode ) {
 	    codetabof (i) = free_ent++;	/* code -> hashtable */
	    htabof (i) = fcode;
	}
	else if ( (count_int)in_count >= checkpoint && block_compress )
	    cl_block ();
    }
    /*
     * Put out the final code.
     */
    output( (code_int)ent );
    out_count++;
    output( (code_int)-1 );

    /*
     * Print out stats on stderr
     */
    if(zcat_flg == 0 && !quiet) {
#ifdef C_DEBUG
	fprintf( stderr,
		"%ld chars in, %ld codes (%ld bytes) out, compression factor: ",
		in_count, out_count, bytes_out );
	prratio( stderr, in_count, bytes_out );
	fputs("\tCompression as in compact: ",stderr);
	prratio( stderr, in_count-bytes_out, in_count );
	fprintf( stderr, "\tLargest code (of last block) was %d (%d bits)\n",
		free_ent - 1, n_bits );
#else /* !C_DEBUG */
	fprintf( stderr, MSGSTR(COMPRESSION, "Compression: ") ); /*MSG*/
	prratio( stderr, in_count-bytes_out, in_count );
#endif /* C_DEBUG */
    }
    if(bytes_out > in_count)	/* exit(2) if no savings */
	exit_stat = 2;
    return;
}

/*****************************************************************
 * TAG( output )
 *
 * Output the given code.
 * Inputs:
 * 	code:	A n_bits-bit integer.  If == -1, then EOF.  This assumes
 *		that n_bits =< (long)wordsize - 1.
 * Outputs:
 * 	Outputs code to the file.
 * Assumptions:
 *	Chars are 8 bits long.
 * Algorithm:
 * 	Maintain a BITS character long buffer (so that 8 codes will
 *      fit in it exactly).  When the buffer fills up empty it and start over.
 */

static char buf[BITS];

static char_type lmask[9] = {0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00};
static char_type rmask[9] = {0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};

static
output( code )
code_int  code;
{
#ifdef C_DEBUG
    static int col = 0;
#endif /* C_DEBUG */

    /*
     * On the VAX, it is important to have the register declarations
     * in exactly the order given, or the asm will break.
     */
    register int r_off = offset, bits= n_bits;
    register char * bp = buf;

#ifdef C_DEBUG
	if ( verbose )
	    fprintf( stderr, "%5d%c", code,
		    (col+=6) >= 74 ? (col = 0, '\n') : ' ' );
#endif /* C_DEBUG */
    if ( code >= 0 ) {
/* 
 * byte/bit numbering on the VAX is simulated by the following code
 */
	/*
	 * Get to the first byte.
	 */
	bp += (r_off >> 3);
	r_off &= 7;
	/*
	 * Since code is always >= 8 bits, only need to mask the first
	 * hunk on the left.
	 */
	*bp = (*bp & rmask[r_off]) | (code << r_off) & lmask[r_off];
	bp++;
	bits -= (8 - r_off);
	code >>= 8 - r_off;
	/* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
	if ( bits >= 8 ) {
	    *bp++ = code;
	    code >>= 8;
	    bits -= 8;
	}
	/* Last bits. */
	if(bits)
	    *bp = code;
	offset += n_bits;
	if ( offset == (n_bits << 3) ) {
	    bp = buf;
	    bits = n_bits;
	    bytes_out += bits;
	    do
		putchar(*bp++);
	    while(--bits);
	    offset = 0;
	}

	/*
	 * If the next entry is going to be too big for the code size,
	 * then increase it, if possible.
	 */
	if ( free_ent > maxcode || (clear_flg > 0))
	{
	    /*
	     * Write the whole buffer, because the input side won't
	     * discover the size increase until after it has read it.
	     */
	    if ( offset > 0 ) {
		if( fwrite((void *)buf, (size_t)1, (size_t)n_bits, stdout ) != n_bits)
			writeerr();
		bytes_out += n_bits;
	    }
	    offset = 0;

	    if ( clear_flg ) {
    	        maxcode = MAXCODE (n_bits = INIT_BITS);
	        clear_flg = 0;
	    }
	    else {
	    	n_bits++;
	    	if ( n_bits == maxbits )
		    maxcode = maxmaxcode;
	    	else
		    maxcode = MAXCODE(n_bits);
	    }
#ifdef C_DEBUG
	    if ( debug ) {
		fprintf( stderr, "\nChange to %d bits\n", n_bits );
		col = 0;
	    }
#endif /* C_DEBUG */
	}
    } else {
	/*
	 * At EOF, write the rest of the buffer.
	 */
	if ( offset > 0 )
	    fwrite((void *)buf, (size_t)1, (size_t)((offset + 7) / 8), stdout);
	bytes_out += (offset + 7) / 8;
	offset = 0;
	fflush( stdout );
#ifdef C_DEBUG
	if ( verbose )
		 fputc('\n',stderr);
#endif /* C_DEBUG */
	if( ferror( stdout ) )
		writeerr();
	}
}

/*
 * Decompress stdin to stdout.  This routine adapts to the codes in the
 * file building the "string" table on-the-fly; requiring no table to
 * be stored in the compressed file.  The tables used herein are shared
 * with those of the compress() routine.  See the definitions above.
 */

static
decompress() {
    register char_type *stackp;
    register int finchar;
    register code_int code, oldcode, incode;
    char_type *stackp_limit = (char_type *)((int)&htab[0] + sizeof( htab ));

    /*
     * As above, initialize the first 256 entries in the table.
     */
    maxcode = MAXCODE(n_bits = INIT_BITS);
    for ( code = 255; code >= 0; code-- ) {
	tab_prefixof(code) = 0;
	tab_suffixof(code) = (char_type)code;
    }
    free_ent = ((block_compress) ? FIRST : 256 );

    finchar = oldcode = getcode();
    if(oldcode == -1)	/* EOF already? */
	return;			/* Get out of here */
    putchar( (char)finchar );		/* first code must be 8 bits = char */
    if(ferror(stdout))		/* Crash if can't write */
	writeerr();
    stackp = de_stack;

    while ( (code = getcode()) > -1 ) {

	if ( (code == CLEAR) && block_compress ) {
	    for ( code = 255; code >= 0; code-- )
		tab_prefixof(code) = 0;
	    clear_flg = 1;
	    free_ent = FIRST - 1;
	    if ( (code = getcode ()) == -1 )	/* O, untimely death! */
		break;
	}
	incode = code;
	/*
	 * Special case for KwKwK string.
	 */
	if ( code >= free_ent ) {
            *stackp++ = finchar;
	    code = oldcode;
	}

	/*
	 * Generate output characters in reverse order
	 */
	while ( code >= 256 ) {
	    if ( stackp >= stackp_limit )
	    	oops() ;
	    *stackp++ = tab_suffixof(code);
	    code = tab_prefixof(code);
	}
	*stackp++ = finchar = tab_suffixof(code);

	/*
	 * And put them out in forward order
	 */
	do
	    putchar ( *--stackp );
	while ( stackp > de_stack );

	/*
	 * Generate the new entry.
	 */
	if ( (code=free_ent) < maxmaxcode ) {
	    tab_prefixof(code) = (unsigned short)oldcode;
	    tab_suffixof(code) = finchar;
	    free_ent = code+1;
	} 
	/*
	 * Remember previous code.
	 */
	oldcode = incode;
    }
    fflush( stdout );
    if(ferror(stdout))
	writeerr();
}

/*****************************************************************
 * TAG( getcode )
 *
 * Read one code from the standard input.  If EOF, return -1.
 * Inputs:
 * 	stdin
 * Outputs:
 * 	code or -1 is returned.
 */

code_int
getcode() {
    /*
     * On the VAX, it is important to have the register declarations
     * in exactly the order given, or the asm will break.
     */
    register code_int code;
    static int offset = 0, size = 0;
    static char_type buf[BITS];
    register int r_off, bits;
    register char_type *bp = buf;

    if ( clear_flg > 0 || offset >= size || free_ent > maxcode ) {
	/*
	 * If the next entry will be too big for the current code
	 * size, then we must increase the size.  This implies reading
	 * a new buffer full, too.
	 */
	if ( free_ent > maxcode ) {
	    n_bits++;
	    if ( n_bits == maxbits )
		maxcode = maxmaxcode;	/* won't get any bigger now */
	    else
		maxcode = MAXCODE(n_bits);
	}
	if ( clear_flg > 0) {
    	    maxcode = MAXCODE (n_bits = INIT_BITS);
	    clear_flg = 0;
	}
	size = fread((void *)buf, (size_t)1, (size_t)n_bits, stdin );
	if ( size <= 0 )
	    return -1;			/* end of file */
	offset = 0;
	/* Round size down to integral number of codes */
	size = (size << 3) - (n_bits - 1);
    }
    r_off = offset;
    bits = n_bits;
	/*
	 * Get to the first byte.
	 */
	bp += (r_off >> 3);
	r_off &= 7;
	/* Get first part (low order bits) */
	code = (*bp++ >> r_off);
	bits -= (8 - r_off);
	r_off = 8 - r_off;		/* now, offset into code word */
	/* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
	if ( bits >= 8 ) {
	    code |= *bp++ << r_off;
	    r_off += 8;
	    bits -= 8;
	}
	/* high order bits. */
	code |= (*bp & rmask[bits]) << r_off;
    offset += n_bits;

    return code;
}

static char *
rindex(s, c)		/* For those who don't have it in libc.a */
register char *s, c;
{
	char *p;
	for (p = NULL; *s; s++)
	    if (*s == c)
		p = s;
	return(p);
}

#ifdef C_DEBUG
static
printcodes()
{
    /*
     * Just print out codes from input file.  For debugging.
     */
    code_int code;
    int col = 0, bits;

    bits = n_bits = INIT_BITS;
    maxcode = MAXCODE(n_bits);
    free_ent = ((block_compress) ? FIRST : 256 );
    while ( ( code = getcode() ) >= 0 ) {
	if ( (code == CLEAR) && block_compress ) {
   	    free_ent = FIRST - 1;
   	    clear_flg = 1;
	}
	else if ( free_ent < maxmaxcode )
	    free_ent++;
	if ( bits != n_bits ) {
	    fprintf(stderr, "\nChange to %d bits\n", n_bits );
	    bits = n_bits;
	    col = 0;
	}
	fprintf(stderr, "%5d%c", code, (col+=6) >= 74 ? (col = 0, '\n') : ' ' );
    }
    putc( '\n', stderr );
    exit( 0 );
}

static code_int sorttab[1<<BITS];	/* sorted pointers into htab */

static
dump_tab()	/* dump string table */
{
    register int i, first;
    register ent;
#define STACK_SIZE	15000
    int stack_top = STACK_SIZE;
    register c;

    if(do_decomp == 0) {	/* compressing */
	register int flag = 1;

	for(i=0; i<hsize; i++) {	/* build sort pointers */
		if((long)htabof(i) >= 0) {
			sorttab[codetabof(i)] = i;
		}
	}
	first = block_compress ? FIRST : 256;
	for(i = first; i < free_ent; i++) {
		fprintf(stderr, "%5d: \"", i);
		de_stack[--stack_top] = '\n';
		de_stack[--stack_top] = '"';
		stack_top = in_stack((htabof(sorttab[i])>>maxbits)&0xff, 
                                     stack_top);
		for(ent=htabof(sorttab[i]) & ((1<<maxbits)-1);
		    ent > 256;
		    ent=htabof(sorttab[ent]) & ((1<<maxbits)-1)) {
			stack_top = in_stack(htabof(sorttab[ent]) >> maxbits,
						stack_top);
		}
		stack_top = in_stack(ent, stack_top);
		fwrite( &de_stack[stack_top], 1, STACK_SIZE-stack_top, stderr);
	   	stack_top = STACK_SIZE;
	}
   } else if(!debug) {	/* decompressing */

       for ( i = 0; i < free_ent; i++ ) {
	   ent = i;
	   c = tab_suffixof(ent);
	   if ( isascii(c) && isprint(c) )
	       fprintf( stderr, "%5d: %5d/'%c'  \"",
			   ent, tab_prefixof(ent), c );
	   else
	       fprintf( stderr, "%5d: %5d/\\%03o \"",
			   ent, tab_prefixof(ent), c );
	   de_stack[--stack_top] = '\n';
	   de_stack[--stack_top] = '"';
	   for ( ; ent != NULL;
		   ent = (ent >= FIRST ? tab_prefixof(ent) : NULL) ) {
	       stack_top = in_stack(tab_suffixof(ent), stack_top);
	   }
	   fwrite( &de_stack[stack_top], 1, STACK_SIZE - stack_top, stderr );
	   stack_top = STACK_SIZE;
       }
    }
}

static int
in_stack(c, stack_top)
	register c, stack_top;
{
	if ( (isascii(c) && isprint(c) && c != '\\') || c == ' ' ) {
	    de_stack[--stack_top] = c;
	} else {
	    switch( c ) {
	    case '\n': de_stack[--stack_top] = 'n'; break;
	    case '\t': de_stack[--stack_top] = 't'; break;
	    case '\b': de_stack[--stack_top] = 'b'; break;
	    case '\f': de_stack[--stack_top] = 'f'; break;
	    case '\r': de_stack[--stack_top] = 'r'; break;
	    case '\\': de_stack[--stack_top] = '\\'; break;
	    default:
	 	de_stack[--stack_top] = '0' + c % 8;
	 	de_stack[--stack_top] = '0' + (c / 8) % 8;
	 	de_stack[--stack_top] = '0' + c / 64;
	 	break;
	    }
	    de_stack[--stack_top] = '\\';
	}
	return stack_top;
}
#endif /* C_DEBUG */

static
writeerr()
{
    perror ( ofname );
    unlink ( ofname );
    exit ( 1 );
}

static
copystat(ifname, ofname)
char *ifname, *ofname;
{
    struct stat statbuf;
    /* AIX security enhancement */
				/* aclp replaces 'int mode' variable defined previously */
    char	*aclp;		/* pointer for acl_get() and acl_put() */
    char	*acl_get();
    /* TCSEC DAC mechanism */
    time_t timep[2];

    if (fclose(stdout)) {
	perror("compress");
	exit(errno);
    }
    if (stat(ifname, &statbuf)) {		/* Get stat on input file */
	perror(ifname);
	return;
    }


    if (statbuf.st_nlink > 1) {
	if(quiet)
	    	fprintf(stderr, "%s: ", ifname);
	fprintf(stderr, MSGSTR(HASLINKS2, " -- has %d other links: unchanged\n"), /*MSG*/
		statbuf.st_nlink - 1);
	exit_stat = 1;
	perm_stat = 1;
    } else if (exit_stat == 2 && (!force)) { /* No compression: remove file.Z */
		fprintf(stderr, MSGSTR(UNCHANGE2, " -- file unchanged\n")); /*MSG*/
    } else {			/* ***** Successful Compression ***** */
	exit_stat = 0;

       /* AIX security enhancement */
	/* get acl information so we can copy it to .Z file */
	if ( (aclp = acl_get (ifname)) == NULL)
	{
		perror(ifname);
		return;
    	}
	/* copy acl information (includes the upper and lower mode bits). */
	/* 1==free aclp pointer */
	if (acl_put (ofname, aclp, 1) < 0)
	    perror(ofname);
       /* TCSEC DAC mechanism */

	chown(ofname, statbuf.st_uid, statbuf.st_gid);	/* Copy ownership */
	timep[0] = statbuf.st_atime;
	timep[1] = statbuf.st_mtime;
	utime(ofname, timep);	/* Update last accessed and modified times */
	if (unlink(ifname))	/* Remove input file */
	    perror(ifname);
	if(!quiet)
		fprintf(stderr, MSGSTR(REPLACEW2, " -- replaced with %s\n"), ofname); /*MSG*/
	return;		/* Successful return */
    }

    /* Unsuccessful return -- one of the tests failed */
    if (unlink(ofname))
	perror(ofname);
}

static
onintr (void)
{
    if (!precious)
	unlink ( ofname );
    exit ( 1 );
}

static
oops (void)	/* wild pointer -- assume bad input */
{
    if ( do_decomp ) 
    	fprintf ( stderr, MSGSTR(CORRUPTIN, "uncompress: corrupt input\n") ); /*MSG*/
    if (!precious)
	unlink ( ofname );
    exit ( 1 );
}

static
cl_block ()		/* table clear for block compress */
{
    register long int rat;

    checkpoint = in_count + CHECK_GAP;
#ifdef C_DEBUG
	if ( debug ) {
    		fprintf ( stderr, "count: %ld, ratio: ", in_count );
     		prratio ( stderr, in_count, bytes_out );
	}
#endif /* C_DEBUG */

    if(in_count > 0x007fffff) {	/* shift will overflow */
	rat = bytes_out >> 8;
	if(rat == 0) {		/* Don't divide by zero */
	    rat = 0x7fffffff;
	} else {
	    rat = in_count / rat;
	}
    } else {
	rat = (in_count << 8) / bytes_out;	/* 8 fractional bits */
    }
    if ( rat > ratio ) {
	ratio = rat;
    } else {
	ratio = 0;
#ifdef C_DEBUG
	if(verbose)
		dump_tab();	/* dump string table */
#endif
 	cl_hash ( (count_int) hsize );
	free_ent = FIRST;
	clear_flg = 1;
	output ( (code_int) CLEAR );
#ifdef C_DEBUG
	if(debug)
			fputs("clear\n",stderr);
#endif /* C_DEBUG */
    }
}

static
cl_hash(hsize)		/* reset code table */
	register count_int hsize;
{
	register count_int *htab_p = htab+hsize;
	register long i;
	register long m1 = -1;

	i = hsize - 16;
 	do {				/* might use Sys V memset(3) here */
		*(htab_p-16) = m1;
		*(htab_p-15) = m1;
		*(htab_p-14) = m1;
		*(htab_p-13) = m1;
		*(htab_p-12) = m1;
		*(htab_p-11) = m1;
		*(htab_p-10) = m1;
		*(htab_p-9) = m1;
		*(htab_p-8) = m1;
		*(htab_p-7) = m1;
		*(htab_p-6) = m1;
		*(htab_p-5) = m1;
		*(htab_p-4) = m1;
		*(htab_p-3) = m1;
		*(htab_p-2) = m1;
		*(htab_p-1) = m1;
		htab_p -= 16;
	} while ((i -= 16) >= 0);
    	for ( i += 16; i > 0; i-- )
		*--htab_p = m1;
}

static
prratio(stream, num, den)
FILE *stream;
long int num, den;
{
	register int q;			/* Doesn't need to be long */

	if(num > 214748L) {		/* 2147483647/10000 */
		q = num / (den / 10000L);
	} else {
		q = 10000L * num / den;		/* Long calculations, though */
	}
	if (q < 0) {
		putc('-', stream);
		q = -q;
	}
	fprintf(stream, "%d.%02d%% ", q / 100, q % 100);
}

static
version()
{
	/* fprintf(stderr, "%s, Berkeley 5.9 5/11/86\n", rcs_ident); */
	fprintf(stderr, MSGSTR(OPTS, "Options: ")); /*MSG*/
#ifdef C_DEBUG
	fputs("DEBUG, ",stderr);
#endif
	fprintf(stderr, "BITS = %d\n", BITS);
}

static
Usage() {
#ifdef C_DEBUG
if (zcat_flg && !uncomp_flg) {
  fputs("Usage: zcat [-DFfnV] [file...]\n",stderr);
} else if (uncomp_flg) {
  fputs("Usage: uncompress [-cDFfnqVv] [file...]\n",stderr);
} else {
  fputs("Usage: compress [-CcDdFfnqVv] [-b Bits] [file...]\n",stderr);
}
#else
if (zcat_flg && !uncomp_flg) {
  fprintf(stderr,MSGSTR(ZCATUSAGE2,
	"Usage: zcat [-nV] [file...]\n")); /*MSG*/
} else if (uncomp_flg) {
  fprintf(stderr,MSGSTR(UNCPUSAGE2,
	"Usage: uncompress [-cfFnqvV] [file...]\n")); /*MSG*/
} else {
  fprintf(stderr,MSGSTR(CPUSAGE2,
	"Usage: compress [-cCdfFnqvV] [-b Bits] [file...]\n")); /*MSG*/
}
#endif /* C_DEBUG */
exit(1);
}
