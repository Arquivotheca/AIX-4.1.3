/*
 * @(#) 66 1.3  src/bldenv/bldtools/afscpsrc/bldafsdcp.c, bldtools, bos412, GOLDA411a 9/19/93 14:56:46
 *
 * COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
 *
 * FUNCTIONS: n/a
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * Licensed Materials - Property of IBM
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * CRC-32 Implementation Copyright (C) Gary S. Brown, 1986.
 *     The CRC-32/ANSI X3.66 implementation from ZMODEM was
 *     provided for use without restriction by the author.
 *     Additional modifications to the CRC-32 implementation
 *     are (C) COPYRIGHT International Business Machines Corp.,
 *     1993.
 *
 * !!!! bldafsdcp.c: (vi: set tabstop=4) !!!!
 *
 * NAME:          bldafsdcp
 *
 * PURPOSE:       bldafsdcp takes a list of input files and source/
 *                target directories and copies the files from the
 *                source directory to the target directory
 *
 * INPUT:         a list of files with relative paths to be copied
 *
 * OUTPUT:        copied files, standard error listing of anomalies
 *
 * OPTIONS:       -h          displays a help message
 *                -i          displays source version information
 *                -v          test/verify the files via CRC-32
 *                            (default: no verification)
 *
 */
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <errno.h>
#include <utime.h>
#include <time.h>
#include <pwd.h>

extern int optind;
extern char *optarg;

/* Program input defines .....
 */
#define MAXINPUTLINE        2048        /* 2047+1 chars for filenames */
#define COPYBUFSIZ           512        /* 512 bytes per block copied */
#define JUNKSTR_SZ           256        /* 255+1  chars for junk strings */
#define LEVELS_PRINT           3        /* 3 levels of directory status */

/* Global program statistics .....
 */
unsigned long total_files   = 0L;       /* total files copied */
unsigned long total_errors  = 0L;       /* non-recoverable errors */
unsigned long total_softerr = 0L;       /* recoverable errors */

/* Global program jump buffer and interrupt stuff .....
 */
int     program_intr = 0;               /* interrupted program? */
jmp_buf saved_context;                  /* saved program context */
int     loop_intr    = 0;               /* interrupt loop */

/* SCCS information ....
 */
static char afsdcp_sccsid[] =
       "@(#) 66 1.3  src/bldenv/bldtools/afscpsrc/bldafsdcp.c, bldtools, bos412, GOLDA411a 9/19/93 14:56:46";
static char afsdcp_ischeck[] = "@(#)";
static char afsdcp_version[] = "1.3 - 93/09/19 14:56:46";


/*
 * -=-=-=- USAGE/FRIENDLY FUNCTIONS -=-=-=-
 */

/* usage - print program usage
 */
void usage(cname)
char *cname;
{
    /* Text for the usage message ....
     */
    char a0[] =
      "%s: usage: %s [-h|-i] [-v] source-directory target-directory\n";
    char a1[] =
      "     where:\n";
    char a2[] =
      "          -h      displays usage message and exits\n";
    char a3[] =
      "          -i      displays source version information and exits\n";
    char a4[] =
      "          -v      test/verify the files via CRC-32\n";
    char a5[] =
      "                  (default: no verification)\n\n";
    char a6[] =
      "     Takes a list of input files and source/target directories\n";
    char a7[] =
      "     and copies the files from the source directory to the target\n";
    char a8[] =
      "     directory, optionally verifying the files via CRC-32.\n\n";

    /* Display usage message ....
     */
    fprintf(stderr, a0, cname, cname);
    fprintf(stderr, a1); fprintf(stderr, a2);
    fprintf(stderr, a3); fprintf(stderr, a4);
    fprintf(stderr, a5); fprintf(stderr, a6);
    fprintf(stderr, a7); fprintf(stderr, a8);
}


/* print_time - print time for banner
 */
void print_time(fp)
FILE *fp;
{
    char *tprint, *tprint2;
    struct tm *curtm;
    long curtime;

    curtime = time((long *) 0);
    curtm   = localtime(&curtime);

    /*
     * Fetch day of week .....
     */
    switch(curtm->tm_wday) {
        case 0:  tprint = "Sun"; break;
        case 1:  tprint = "Mon"; break;
        case 2:  tprint = "Tue"; break;
        case 3:  tprint = "Wed"; break;
        case 4:  tprint = "Thu"; break;
        case 5:  tprint = "Fri"; break;
        case 6:  tprint = "Sat"; break;
        default: tprint = "   "; break;
    }

    /* Fetch month .....
     */
    switch (curtm->tm_mon) {
        case 0:  tprint2 = "Jan"; break;
        case 1:  tprint2 = "Feb"; break;
        case 2:  tprint2 = "Mar"; break;
        case 3:  tprint2 = "Apr"; break;
        case 4:  tprint2 = "May"; break;
        case 5:  tprint2 = "Jun"; break;
        case 6:  tprint2 = "Jul"; break;
        case 7:  tprint2 = "Aug"; break;
        case 8:  tprint2 = "Sep"; break;
        case 9:  tprint2 = "Oct"; break;
        case 10: tprint2 = "Nov"; break;
        case 11: tprint2 = "Dec"; break;
        default: tprint2 = "   "; break;
    }

    /* Print date string like date command .....
     */
    fprintf(fp, "%s %s %02.2d %02.2d:%02.2d:%02.2d %s %d\n",
            tprint, tprint2, curtm->tm_mday, curtm->tm_hour,
            curtm->tm_min, curtm->tm_sec, tzname[curtm->tm_isdst],
            1900 + curtm->tm_year);
}


/*
 * -=-=-=- MISCELLANEOUS DATA HANDLING FUNCTIONS -=-=-=-
 */

/* strip_spaces - remove leading/trailing white space from a filename
 *                (also removes leading './' for filenames)
 */
void strip_spaces(s)
char *s;
{
    char *cp, *sp, *tp;

    /* Strip trailing space first, that's easy .....
     */
    cp = s + strlen(s);
    --cp; cp = ((cp < s) ? s : cp);
    for (; cp >= s; --cp) {
        if (isspace(*cp)) {
            *cp = (char) 0;
        } else {
            break;
        }
    }

    /* Strip leading space and relative path junk last, that
     * takes more effort .....
     */
    for (sp = s; isspace(*sp); ++sp)
        ;

    if ((sp[0] == '.') && (sp[1] == '/')) {     /* remove relative junk */
        ++sp; ++sp;
    }

    if (sp > s) {
       /* Copy chars starting at sp into tp, which is before sp .....
        */
       for (tp = s; ; ++tp, ++sp) {
           *tp = *sp;
           if (*sp == '\0') break;
       }
    }
}


/*
 * -=-=-=- SUPPORT ROUTINES -=-=-=-
 */


/*
 * chew_input_line - chew on the input line to remove padding
 *                   and garbage in it
 */
void chew_input_line(rawstring, cookedstring, max_size)
char *rawstring, *cookedstring;     /* raw string, cooked (optional) */
int  max_size;                      /* maximum size of raw/cooked */
{
    char *sp;                       /* substitute pointer */

    /* If no string's provided, then don't do anything .....
     */
    if (rawstring == (char *) 0)
        return;

    /* Strip CR/LF from end of line ....
     */
    sp = rawstring + strlen(rawstring);
    --sp; sp = ((sp < rawstring) ? rawstring : sp);
    if ((*sp == '\r') || (*sp == '\n')) {
        *sp = (char) 0;
    }
    --sp;
    sp = ((sp < rawstring) ? rawstring : sp);
    if ((*sp == '\r') || (*sp == '\n')) {
        *sp = (char) 0;
    }

    /* If string space is provided for a cooked version of
     * the line, then strip the spaces off the line and
     * copy the line there .....
     */
    if (cookedstring != (char *) 0) {
        strncpy(cookedstring, rawstring, max_size);
        strip_spaces(cookedstring);
    }
}


/* restore_context - called by signal handler to bail out of loop
 */
void restore_context(sig)
int sig;
{
    longjmp(saved_context, 1);
}


/* catch_signal - catch the bad signal once, ignore it evermore
 */
void catch_signal(sig)
int sig;
{
    signal(sig, SIG_IGN);
    loop_intr = 1;
}


/* set_signals - set signals to trapped or disabled
 */
void set_signals(t)
int t;                              /* 0 = disabled, !0 = trapped */
{
    if (t == 0) {
        /* Ignore signals but act like they're queued events .....
         */
        signal(SIGHUP, catch_signal);  signal(SIGINT, catch_signal);
        signal(SIGQUIT, catch_signal); signal(SIGABRT, catch_signal);
        signal(SIGTERM, catch_signal); signal(SIGPIPE, catch_signal);
    } else {
        /* Handle signals when we can handle them .....
         */
        signal(SIGHUP, restore_context);  signal(SIGINT, restore_context);
        signal(SIGQUIT, restore_context); signal(SIGABRT, restore_context);
        signal(SIGTERM, restore_context); signal(SIGPIPE, restore_context);
    }
}


/*
 * -=-=-=- CRC-32 VERIFICATION ROUTINES -=-=-=-
 */

/*
 * START OF CRC-32 IMPLEMENTATION
 *
 * Routines to compute the 32-bit CRC used as the frame check
 * sequence in ADCCP (ANSI X3.66, also known as FIPS PUB 71 and
 * FED-STD-1003, the U.S. versions of CCITT's X.25 link-level
 * protocol).  The 32-bit FCS was added via the Federal Register,
 * 1 June 1982, p.23798.  I presume but don't know for certain that
 * this polynomial is or will be included in CCITT V.41, which
 * defines the 16-bit CRC (often called CRC-CCITT) polynomial.  FIPS
 * PUB 78 says that the 32-bit FCS reduces otherwise undetected
 * errors by a factor of 10^-5 over 16-bit FCS.
 *
 * First, the polynomial itself and its table of feedback terms.
 * The polynomial is:
 *
 *     X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+
 *         X^8+X^7+X^5+X^4+X^2+X^1+X^0
 *
 * Note that we take it "backwards" and put the highest-order term in
 * the lowest-order bit.  The X^32 term is "implied"; the LSB is the
 * X^31 term, etc.  The X^0 term (usually shown as "+1") results in
 * the MSB being 1.
 *
 * Note that the usual hardware shift register implementation, which
 * is what we're using (we're merely optimizing it by doing eight-bit
 * chunks at a time) shifts bits into the lowest-order term.  In our
 * implementation, that means shifting towards the right.  Why do we
 * do it this way?  Because the calculated CRC must be transmitted in
 * order from highest-order term to lowest-order term.  UARTs transmit
 * characters in order from LSB to MSB.  By storing the CRC this way,
 * we hand it to the UART in the order low-byte to high-byte; the UART
 * sends each low-bit to hight-bit; and the result is transmission bit
 * by bit from highest- to lowest-order term without requiring any bit
 * shuffling on our part.  Reception works similarly.
 *
 * The feedback terms table consists of 256, 32-bit entries.  Notes:
 *
 *  1. The table can be generated at runtime if desired; code to do so
 *     is shown later.  It might not be obvious, but the feedback
 *     terms simply represent the results of eight shift/xor opera-
 *     tions for all combinations of data and CRC register values
 *
 *  2. The CRC accumulation logic is the same for all CRC polynomials,
 *     be they sixteen or thirty-two bits wide.  You simply choose the
 *     appropriate table.  Alternatively, because the table can be
 *     generated at runtime, you can start by generating the table for
 *     the polynomial in question and use exactly the same logic
 *     if your application needn't simultaneously handle two CRC
 *     polynomials.
 *
 *  3. For 16-bit CRCs, the table entries need be only 16 bits wide;
 *     of course, 32-bit entries work OK if the high 16 bits are zero.
 *
 *  4. The values must be right-shifted by eight bits by the CRC
 *     logic; the shift must be unsigned (bring in zeroes).  On some
 *     hardware you could probably optimize the shift in assembler by
 *     using byte-swap instructions.
 */

/* Need an unsigned type capable of holding 32 bits .....
 */
typedef unsigned long int UNS_32_BITS;

/* Feedback terms table for CRC-32:
 *     WARNING: There must be 64 lines for this table.
 */
static UNS_32_BITS crc_32_tab[] = { /* CRC polynomial 0xedb88320 */
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,     /* 1 */
        0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,     /* 2 */
        0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,     /* 3 */
        0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,     /* 4 */
        0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,     /* 5 */
        0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,     /* 6 */
        0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,     /* 7 */
        0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,     /* 8 */
        0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,     /* 9 */
        0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,     /* 10 */
        0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,     /* 11 */
        0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,     /* 12 */
        0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,     /* 13 */
        0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,     /* 14 */
        0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,     /* 15 */
        0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,     /* 16 */
        0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,     /* 17 */
        0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,     /* 18 */
        0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,     /* 19 */
        0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,     /* 20 */
        0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,     /* 21 */
        0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,     /* 22 */
        0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,     /* 23 */
        0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,     /* 24 */
        0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,     /* 25 */
        0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,     /* 26 */
        0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,     /* 27 */
        0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,     /* 28 */
        0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,     /* 29 */
        0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,     /* 30 */
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,     /* 31 */
        0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,     /* 32 */
        0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,     /* 33 */
        0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,     /* 34 */
        0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,     /* 35 */
        0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,     /* 36 */
        0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,     /* 37 */
        0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,     /* 38 */
        0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,     /* 39 */
        0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,     /* 40 */
        0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,     /* 41 */
        0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,     /* 42 */
        0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,     /* 43 */
        0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,     /* 44 */
        0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,     /* 45 */
        0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,     /* 46 */
        0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,     /* 47 */
        0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,     /* 48 */
        0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,     /* 49 */
        0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,     /* 50 */
        0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,     /* 51 */
        0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,     /* 52 */
        0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,     /* 53 */
        0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,     /* 54 */
        0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,     /* 55 */
        0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,     /* 56 */
        0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,     /* 57 */
        0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,     /* 58 */
        0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,     /* 59 */
        0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,     /* 60 */
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,     /* 61 */
        0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,     /* 62 */
        0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,     /* 63 */
        0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d      /* 64 */
};

/* Macro to update CRC for 32 bits .....
 */
#define UPDC32(octet,crc) (crc_32_tab[((crc)^(octet))&0xff]^((crc)>>8))

/* Structure for CRC-32 verification
 */
struct st_crctable {
    int           crcinit;              /* CRC structure initialized? */
    unsigned long charcnt;              /* number of characters */
    UNS_32_BITS   oldcrc32;             /* previous CRC-32 result */
    UNS_32_BITS   crc32;                /* current CRC-32 result */
    UNS_32_BITS   finalcrc;             /* final CRC-32 result */
};

/* initcrc32 - initialize CRC-32 seed values
 */
void initcrc32(crctab)
struct st_crctable *crctab;
{
    memset(crctab, (char) 0, sizeof(struct st_crctable));
    crctab->crcinit  = 1;
    crctab->oldcrc32 = (UNS_32_BITS) 0xFFFFFFFF;
    crctab->charcnt  = 0;
}


/* crc32block - perform CRC-32 test on block (cumulative if
 *              structure not reset prior to entry)
 */
void crc32block(table, b, n)
struct st_crctable *table;      /* CRC-32 verification table */
char *b;                        /* pointer to data block */
int   n;                        /* number of bytes in block */
{
    int block_pos = 0;

    /* If table has not yet been initialized, do it .....
     */
    if (table->crcinit != 1) {
        initcrc32(table);
    }

    /* Perform CRC-32 checksum .....
     */
    for (block_pos = 0; block_pos < n; ++block_pos) {
        table->oldcrc32 = UPDC32(b[block_pos],table->oldcrc32);
        ++(table->charcnt);
    }
    table->crc32    = table->oldcrc32;
    table->finalcrc = ~(table->oldcrc32);
}


/* chkcrc32 - verify that the named file has the same
 *            CRC as in the specified CRC structure
 */
int chkcrc32(fname, tabcrc)
char *fname;                                /* name of file to verify */
struct st_crctable *tabcrc;                 /* previously computed CRC */
{
    struct st_crctable crcread;             /* CRC computed for read */
    char   readbuffer[COPYBUFSIZ];          /* file read buffer */
    int    readfd;                          /* read file descriptor */
    int    attempts;                        /* read file attempts */
    int    inbytes;                         /* bytes read on a read */
    int    errors_io;                       /* errors on reads */
    int    read_ok;                         /* was file read ok? */
    int    verify_ret;                      /* verify return code */

    /* Make a few passes at verifying the file .....
     */
    for (attempts = 0, read_ok = 0;
         (read_ok == 0) || (attempts >= 3);
         ++attempts) {
        /* Initialize the CRC-32 checksum structure .....
         */
        memset(&crcread, (char) 0, sizeof(struct st_crctable));
        initcrc32(&crcread);

        /* Open file for reading .....
         */
        if ((readfd = open(fname, O_RDONLY)) < 0) {
            continue;                       /* can't find it, go on ..... */
        }

        /* Verify the file one block at a time .....
         */
        for (errors_io = 8; errors_io > 0;) {
            inbytes = read(readfd, readbuffer, COPYBUFSIZ);
            if (inbytes == 0) {             /* 0 bytes, EOF (!O_NDELAY) */
                read_ok = 1; break;
            }
            if (inbytes < 0) {              /* can't read, tick error flag */
                --errors_io; continue;
            }
            crc32block(&crcread, readbuffer, inbytes);
        }
        close(readfd);
    }

    /* Now that we're done reading the file (or we've given up),
     * see if the CRC during the read is the same as the one
     * given to us .....
     */
    if (read_ok != 1) verify_ret = 1;       /* couldn't do it! */
    if ((tabcrc->finalcrc == crcread.finalcrc) &&
        (tabcrc->charcnt  == crcread.charcnt)) {
        verify_ret = 0;                     /* CRC verified OK! */
    } else {
        verify_ret = 1;                     /* default: can't do it! */
    }
    return(verify_ret);
}


/*
 * END OF CRC-32 IMPLEMENTATION
 */

/*
 * -=-=-=- FILE COPY ROUTINES -=-=-=-
 */

/* print_status - print directory status line if high up in tree
 */
void print_status(s)
char *s;
{
    char *cp;
    int  count_slash;

    for (cp = s, count_slash = 0;
         (*cp) && (count_slash <= LEVELS_PRINT);
         ++cp) {
        if (*cp == '/') ++count_slash;
    }

    if (count_slash <= LEVELS_PRINT) {
        fprintf(stderr,
            "COPY STATUS:  directory %s [%ld processed]\n",
            s, total_files);
        fflush(stderr);
    }
}


/* touch_file - change access/modification times on target file
 */
void touch_file(tfile, statinfo, warn)
char *tfile;                        /* target file name */
struct stat *statinfo;              /* stat information from source */
int warn;                           /* print warnings (yes if 0) */
{
    struct utimbuf timeset_buf;
    int utim_ok;

    /* Set file creation/modification times based on source
     * file characteristics .....
     */
    timeset_buf.actime  = (time_t) statinfo->st_ctime;
    timeset_buf.modtime = (time_t) statinfo->st_mtime;
    utim_ok = utime(tfile, &timeset_buf);
    if (utim_ok != 0 && warn == 0) {
        fprintf(stderr,"COPY ERROR: cannot set mode for file: %s\n",
                tfile);
        fflush(stderr);
    }
    errno = 0;                      /* reset errno just in case */
}


/* chmod_file - change mode of target file
 */
void chmod_file(tfile, statinfo, warn)
char *tfile;                        /* target file name */
struct stat *statinfo;              /* stat information from source */
int warn;                           /* print warnings (yes if 0) */
{
    int chm_ok;

    /* Change mode of target file to match (as closely as
     * possible) the source file .....
     */
    chm_ok = chmod(tfile, (statinfo->st_mode & 07777));
    if (chm_ok != 0) {
        /* The chmod() failed for the plock (sticky)/setuid/setgid
         * bits, so let's try it without those .....
         */
        chm_ok = chmod(tfile, (statinfo->st_mode & 0777));
        if (chm_ok != 0 && warn == 0) {
            fprintf(stderr,"COPY ERROR: cannot set mode for file: %s\n",
                    tfile);
            fflush(stderr);
        }
    }
    errno = 0;                      /* reset errno just in case */
}


/* make_directories - make directory chain for a specified file
 */
void make_directories(srfile, filename, srctop, trgtop)
char *srfile;                           /* source filename */
char *filename;                         /* target filename */
char *srctop;                           /* source directory top */
char *trgtop;                           /* target directory top */
{
    char srcmake[MAXINPUTLINE];         /* current source for mkdir */
    char trgmake[MAXINPUTLINE];         /* current target for mkdir */
    char trdir[MAXINPUTLINE];           /* directory part of file */
    char srdir[MAXINPUTLINE];           /* source directory */
    char *twalkp, *swalkp;              /* string scan pointers */
    char *tcopyp, *scopyp;              /* string copy pointers */
    struct stat statbuf;                /* source file statistics */
    int srlen, trlen;                   /* source/target lengths */
    int stat_ret;                       /* chars to compare, stat return */

    /* Take the base part of the target filename .....
     */
    memset(trdir, (char) 0, MAXINPUTLINE);
    strcpy(trdir, filename);
    twalkp = trdir + strlen(trdir); --twalkp;
    while (twalkp >= trdir && *twalkp != '/') --twalkp;
    *twalkp = '/'; ++twalkp; *twalkp = (char) 0;

    /* Take the base part of the source filename .....
     */
    memset(srdir, (char) 0, MAXINPUTLINE);
    strcpy(srdir, srfile);
    swalkp = srdir + strlen(srdir); --swalkp;
    while (swalkp >= srdir && *swalkp != '/') --swalkp;
    *swalkp = '/'; ++swalkp; *swalkp = (char) 0;

    /* Take the directories and walk down the names, starting
     * from the top, creating directories .....
     */
    memset(srcmake, (char) 0, MAXINPUTLINE);
    memset(trgmake, (char) 0, MAXINPUTLINE);
    srlen  = strlen(srctop); ++srlen;
    trlen  = strlen(trgtop); ++trlen;
    scopyp = srdir + srlen; swalkp = srcmake + srlen;
    tcopyp = trdir + trlen; twalkp = trgmake + trlen;
    memcpy(srcmake, srdir, srlen);
    memcpy(trgmake, trdir, trlen);

    /* Walk the portions of the source and target paths below
     * the top level directory and make directories on the
     * target for each .....
     */
    for (;*scopyp && *tcopyp;
         ++swalkp, ++twalkp, ++tcopyp, ++scopyp) {
        if (*tcopyp == '/') {
            stat_ret = stat(srcmake, &statbuf);
            if (stat_ret != 0) {
                fprintf(stderr,
                       "COPY ERROR: cannot find source directory: %s\n",
                        srcmake);
                fflush(stderr);
                mkdir(trgmake, 0777);           /* make dir anyhow */
            } else {
                mkdir(trgmake, statbuf.st_mode & 07777);
                touch_file(trgmake, &statbuf, 1);
            }
        }
        *swalkp = *scopyp;          /* copy char for source directory */
        *twalkp = *tcopyp;          /* copy char for target directory */
    }
    errno = 0;                      /* reset errno from any failures */
}


/*
 * copy_file - copy a file from source directory to target
 *             directory with optional verify
 *
 * N.B.  Error codes don't get propagated back, but lots of
 *       nice and friendly error messages do when things are
 *       not going well .....
 */
void copy_file(need_verify, srcfile, trgfile,
               sourcetop, targettop, srcstat)
int need_verify;                        /* verify flag */
char *srcfile, *trgfile;                /* source/target filenames */
char *sourcetop;                        /* source tree top directory */
char *targettop;                        /* target tree top directory */
struct stat *srcstat;                   /* source file stat struct */
{
    struct st_crctable crctable;        /* CRC-32 write verification */
    char copy_buffer[COPYBUFSIZ];       /* file copy buffer */
    int sourcefd, targetfd;             /* source/target fd's */
    int bytesin, bytesout;              /* size of i/o */
    int io_errors, softerr;             /* maximum errors, soft errors */
    int copy_ok, tries;                 /* copy status */
    int crc_ok;                         /* CRC-32 verification status */

    for (tries = 0, copy_ok = 0, softerr = 0;
         tries < 3; ++tries) {
        if (copy_ok != 0) break;        /* break out if copy worked */

        /* If we're verifying the file, initialize the CRC-32
         * checksum structure .....
         */
        if (need_verify != 0) {
            memset(&crctable, (char) 0, sizeof(struct st_crctable));
            initcrc32(&crctable);
        }

        /* Attempt to open source file and if opened successfully,
         * blow away the target file before copying .....
         */
        if ((sourcefd = open(srcfile, O_RDONLY)) < 0) {
            ++softerr;                  /* soft errors on this file */
            continue;                   /* have another whack at it ..... */
        }
        unlink(trgfile);                /* blow away target file */

        /* Open target file for writing .....
         */
        if ((targetfd = open(trgfile, O_WRONLY | O_CREAT, 0666)) < 0) {
            make_directories(srcfile, trgfile, sourcetop, targettop);
            if ((targetfd = open(trgfile, O_WRONLY | O_CREAT, 0666)) < 0) {
                ++softerr;              /* soft errors on this file */
                continue;               /* have another whack at it ..... */
            }
        }

        /* Copy the file one block at a time .....
         */
        for (io_errors = 8; io_errors > 0;) {
            bytesin = read(sourcefd, copy_buffer, COPYBUFSIZ);
            if (bytesin == 0) {             /* 0 bytes, EOF (!O_NDELAY) */
                copy_ok = 1; break;
            }

            if (bytesin < 0) {              /* can't read, tick error flag */
                --io_errors; continue;
            }
            bytesout = write(targetfd, copy_buffer, bytesin);
            if (bytesout != bytesin) break; /* pothole in file */

            if (need_verify != 0) {
                crc32block(&crctable, copy_buffer, bytesout);
            }
        }
        close(targetfd); close(sourcefd);
        if (copy_ok != 1) {
            ++softerr;                      /* soft errors on this file */
        } else {
            if (need_verify != 0) {
                crc_ok = chkcrc32(trgfile, &crctable);
                if (crc_ok != 0) {
                    copy_ok = 0;            /* copy really was not OK! */
                    ++softerr;              /* and mark it as a soft error */
                }
            }   /* if need_verify != 0 ..... */
        }   /* if copy_ok != 1 ..... */
    }   /* for copy_ok ..... */

    /* Now that the copy is done (or we've given up),
     * print an error message if we didn't get a file
     * copied .....
     */
    if (copy_ok != 1) {
        fprintf(stderr, "COPY ERROR: could not copy to file: %s\n",
                trgfile);
        fflush(stderr);
        ++total_errors;                     /* skip copy, can't write file */
        return;
    } else {
        if (softerr > 0) ++total_softerr;
    }

    /* Make sure the file modes are set and warn the user if
     * we couldn't set them .....
     */
    chmod_file(trgfile, srcstat, 0);    /* change file modes */
    touch_file(trgfile, srcstat, 0);    /* change file date */
}


/* copy_link - copy symbolic links
 */
void copy_link(need_verify, srcfile, trgfile, srcdir, trgdir, srcstat)
int need_verify;                    /* verify flag */
char *srcfile, *trgfile;            /* source/target files */
char *srcdir;                       /* top source directory */
char *trgdir;                       /* top target directory */
struct stat *srcstat;               /* source file stat struct */
{
    char linkpoint[MAXINPUTLINE];       /* where symlink points */
    char linktarget[MAXINPUTLINE];      /* where symlink should point */
    char srclbase[MAXINPUTLINE];        /* symlink source base name */
    char trglbase[MAXINPUTLINE];        /* symlink target base name */
    char commpath[MAXINPUTLINE];        /* common base path component */
    int link_rd, link_wr;               /* link read/write status */
    int srclen, charscomm;              /* src dir length, chars common */
    char *walkptr, *headptr;            /* source string walking pointers */
    char *theadptr;                     /* target string walking pointers */
    int countslash;                     /* number of directory components */

    /* Attempt to read the source link .....
     */
    memset(linktarget, (char) 0, MAXINPUTLINE);
    memset(linkpoint,  (char) 0, MAXINPUTLINE);
    link_rd = readlink(srcfile, linkpoint, MAXINPUTLINE);
    if (link_rd < 0) {
        fprintf(stderr, "COPY ERROR: cannot read symbolic link: %s\n",
                srcfile);
        fflush(stderr);
        ++total_errors;                 /* skip copy, can't read file */
        return;
    }

    /* Compute link destination based on partial paths matching
     * if we've been given a full path .....
     */
    if (*linkpoint != '/') {
        /* The path isn't a full path .....
         */
        memcpy(linktarget, linkpoint, MAXINPUTLINE);
    } else {
        srclen = strlen(srcdir);
        if ((strncmp(srcdir, linkpoint, srclen)) != 0) {
            /* The path isn't relative to where we're copying
             * from, so preserve the link name .....
             */
            memcpy(linktarget, linkpoint, MAXINPUTLINE);
        } else {
            /* Handle the special case of link = top first .....
             */
            if ((strcmp(srcdir, linkpoint)) == 0) {
                memcpy(linktarget, trgdir, MAXINPUTLINE);
            } else {
                /* The path is relative to where we're copying
                 * from, so figure out how to take it apart .....
                 */
                memset(trglbase, (char) 0, MAXINPUTLINE);
                memset(srclbase, (char) 0, MAXINPUTLINE);
                strcpy(trglbase, linkpoint+srclen+1);   /* target base */
                strcpy(srclbase, srcfile+srclen+1);     /* source base */

                /* Now we should have a path relative to the top, but
                 * we have to figure out how many directory levels
                 * we can prune off the top .....
                 */
                memset(commpath, (char) 0, MAXINPUTLINE);
                for (headptr  = srclbase, walkptr = srclbase,
                     theadptr = trglbase;; ) {
                    while (*walkptr && *walkptr != '/') ++walkptr;
                    if (!*walkptr) break;

                    charscomm = (int) (walkptr - headptr); ++charscomm;
                    if ((strncmp(headptr, theadptr, charscomm)) == 0) {
                        strncat(commpath, headptr, charscomm);
                        ++walkptr; headptr = walkptr;
                        ++theadptr; theadptr += charscomm;
                    } else {
                        break;
                    }
                }

                /* Now that we know what part of the path is common,
                 * let's figure out how many directory components we
                 * have to add to the head of the final target .....
                 */
                if (*commpath) {
                    for (countslash = 0, walkptr = (srclbase+strlen(commpath));
                         *walkptr; ++walkptr) {
                        if (*walkptr == '/') ++countslash;
                    }
                } else {
                    for (countslash = 0, walkptr = srclbase;
                         *walkptr; ++walkptr) {
                        if (*walkptr == '/') ++countslash;
                    }
                }

                /* Add directory components to link target and copy
                 * base portion of link name .....
                 */
                for (; countslash > 0; --countslash) {
                    strncat(linktarget, "../", 3);
                }
                strcat(linktarget, trglbase+(strlen(commpath)));
            }   /* else path not = top */
        }   /* else path relative to top */
    }   /* else *linkpoint != '/' */

    /* Now that we know where the link's going, create it .....
     */
    link_wr = symlink(linktarget, trgfile);
    if (link_wr != 0) {
        make_directories(srcfile, trgfile, srcdir, trgdir);
        link_wr = symlink(linktarget, trgfile);
        if (link_wr != 0) {
            fprintf(stderr, "COPY ERROR: cannot write symbolic link: %s\n",
                    trgfile);
            fflush(stderr);
            ++total_errors;             /* skip copy, can't write file */
            return;
        }
    }
}


/*
 * process_files - perform copy of files from source to target
 *                 directory
 */
int process_files(cname, infp, do_verify, sourcedir, targetdir)
char *cname;                            /* program name */
FILE *infp;                             /* input file pointer */
int do_verify;                          /* verify flag */
char *sourcedir, *targetdir;            /* source/target top directories */
{
    char inputline[MAXINPUTLINE];       /* raw input line w/filename */
    char cookedline[MAXINPUTLINE];      /* filename with junk removed */
    char currentdir[MAXINPUTLINE];      /* full path of current directory */
    char sourcefull[MAXINPUTLINE];      /* full name of source directory */
    char targetfull[MAXINPUTLINE];      /* full name of target directory */
    char sourcefile[MAXINPUTLINE];      /* filename on source directory */
    char targetfile[MAXINPUTLINE];      /* filename on target directory */
    char tempbuffer[MAXINPUTLINE];      /* temporary buffer */
    int  cd_ok, stat_ok, md_ok;         /* cd, stat, mkdir success */
    struct stat source_stat;            /* source file statistics */
    struct stat target_stat;            /* target file statistics */
    char *fgets_ok, *cp;                /* fgets() success, misc. ptr. */
    char *devdesc;                      /* device description */
    long curtime;                       /* current time */

    /* Save current directory for future use .....
     */
    cp = getwd(currentdir);
    if (cp == (char *) NULL) {
        sprintf(tempbuffer,
                "%s: FATAL ERROR: error finding current directory",
                cname);
        perror(tempbuffer);
        exit(1);
    }

    /* Figure out if source directory exists and what its real
     * pathname is .....
     */
    cd_ok = chdir(sourcedir);
    if (cd_ok != 0) {
        sprintf(tempbuffer,
                "%s: FATAL ERROR: error changing to source directory",
                cname);
        perror(tempbuffer);
        exit(1);
    }
    cp = getwd(sourcefull);
    if (cp == (char *) NULL) {
        sprintf(tempbuffer,
                "%s: FATAL ERROR: error finding source directory",
                cname);
        perror(tempbuffer);
        exit(1);
    }

    /* Figure out if target directory exists and what its real
     * pathname is .....
     */
    cd_ok = chdir(currentdir);
    if (cd_ok != 0) {
        sprintf(tempbuffer,
                "%s: FATAL ERROR: error changing to current directory",
                cname);
        perror(tempbuffer);
        exit(1);
    }
    cd_ok = chdir(targetdir);
    if (cd_ok != 0) {
        sprintf(tempbuffer,
                "%s: FATAL ERROR: error changing to target directory",
                cname);
        perror(tempbuffer);
        exit(1);
    }
    cp = getwd(targetfull);
    if (cp == (char *) NULL) {
        sprintf(tempbuffer,
                "%s: FATAL ERROR: error finding target directory",
                cname);
        perror(tempbuffer);
        exit(1);
    }
    (void) chdir(currentdir);           /* go back home */

    /* Make sure source != target .....
     */
    if ((strcmp(sourcefull, targetfull)) == 0) {
        fprintf(stderr, "%s: source and target directories identical!\n",
                cname);
        fflush(stderr);
        exit(1);
    }

    /* Build the jump vector for breaking out if we catch
     * a bad signal .....
     */
    if ((setjmp(saved_context)) != 0) {
        program_intr = 1;
        goto bailout;                   /* restore context from signal */
    }
    set_signals(1);                     /* handle signals when we can */

    /* PRINT STARTUP BANNER
     */
    fprintf(stderr,"COPY STARTED: ");
    print_time(stderr);
    fflush(stderr);

    /* Read each file from standard input and copy it .....
     */
    while (!feof(infp)) {
        /* Handle signals before doing anything .....
         */
        set_signals(1);                 /* pay attention to signals */
        if (loop_intr != 0) {
            set_signals(0);             /* disable signals */
            longjmp(saved_context, 1);  /* bail out! */
        }

        /* Read the file name from standard input .....
         */
        memset(inputline,  (char) 0, MAXINPUTLINE);
        fgets_ok = fgets(inputline, (MAXINPUTLINE-1), infp);
        if (!fgets_ok) continue;        /* bad read, continue */

        /* Skip null reads and comments .....
         */
        if (*inputline == (char) 0) continue;
        if (*inputline == '#') continue;

        /* Chew up the input line from fgets, skip blank lines and top .....
         */
        chew_input_line(inputline, cookedline, MAXINPUTLINE);
        if (*cookedline == (char) 0) continue;
        if ((cookedline[0] == '.') && (cookedline[1] == (char) 0))
            continue;

        /* Looks like we're going to copy a file, so update the
         * count .....
         */
        ++total_files;

        /* Now that we have a clean version of the base filename,
         * build full paths for the file and figure out what kind
         * of file it is .....
         */
        sprintf(sourcefile, "%s/%s", sourcefull, cookedline);
        sprintf(targetfile, "%s/%s", targetfull, cookedline);

        /* Load source file statistics .....
         */
        stat_ok = lstat(sourcefile, &source_stat);
        if (stat_ok != 0) {
            fprintf(stderr, "COPY ERROR: cannot find source file: %s\n",
                    sourcefile);
            fflush(stderr);
            ++total_errors;                 /* skip copy, can't read file */
            continue;                       /* can't find it */
        }

        /* Figure out what kind of file this is from the
         * mode and copy the file .....
         *
         * Copy directories .....
         */
        if (S_ISDIR(source_stat.st_mode)) {     /* directory */
            errno = 0;
            md_ok = mkdir(targetfile, source_stat.st_mode);
            if (errno == EEXIST || errno == 0) {
                chmod_file(targetfile, &source_stat, 1);
                touch_file(targetfile, &source_stat, 1);
                errno = 0;
            } else if (errno != 0) {
                make_directories(sourcefile, targetfile,
                                 sourcefull, sourcefull);
                md_ok = mkdir(targetfile, source_stat.st_mode);
                if (errno == EEXIST || errno == 0) {
                    chmod_file(targetfile, &source_stat, 1);
                    touch_file(targetfile, &source_stat, 1);
                    errno = 0;
                } else {
                    fprintf(stderr,
                        "COPY ERROR: cannot create directory: %s\n",
                        targetfile);
                    fflush(stderr);
                    ++total_errors;         /* skip copy, can't write file */
                    continue;
                }
            }

            /* If we've got a directory at a sufficiently high
             * level below the top level, print out a status
             * message .....
             */
            print_status(cookedline);
            continue;
        }

        /* Disable signals before copying these .....
         */
        set_signals(0);

        /* Copy block/character devices, fifos, sockets .....
         */
        if ( (S_ISBLK(source_stat.st_mode)) ||
             (S_ISCHR(source_stat.st_mode)) ||
             (S_ISFIFO(source_stat.st_mode)) ||
             ((source_stat.st_mode & S_IFSOCK) == S_IFSOCK) ) {
            unlink(targetfile);                 /* remove before update */
            errno = 0;
            md_ok = mknod(targetfile, source_stat.st_mode,
                          (dev_t) source_stat.st_rdev);
            if (md_ok != 0) {
                /* Build directory tree for file just in case .....
                 */
                make_directories(sourcefile, targetfile,
                                 sourcefull, sourcefull);
                md_ok = mknod(targetfile, source_stat.st_mode,
                              (dev_t) source_stat.st_rdev);
                if (md_ok != 0) {
                    /* Figure out device type for message .....
                     */
                    devdesc = (char *) 0;
                    if (S_ISBLK(source_stat.st_mode))
                        devdesc = "block device";
                    if (S_ISCHR(source_stat.st_mode))
                        devdesc = "character device";
                    if (S_ISFIFO(source_stat.st_mode))
                        devdesc = "FIFO";
                    if ((source_stat.st_mode & S_IFSOCK) == S_IFSOCK)
                        devdesc = "socket";
                    if (devdesc == (char *) 0)
                        devdesc = "device";

                    fprintf(stderr,
                            "COPY ERROR: cannot copy to %s: %s\n",
                            devdesc, targetfile);
                    fflush(stderr);
                    ++total_errors;         /* skip copy, can't write file */
                    continue;
                } else {
                    chmod_file(targetfile, &source_stat, 0); /* chg mode */
                    touch_file(targetfile, &source_stat, 0); /* chg modtimes */
                }
            } else {
                chmod_file(targetfile, &source_stat, 0);     /* chg mode */
                touch_file(targetfile, &source_stat, 0);     /* chg modtimes */
            }
            continue;
        }

        /* Copy symbolic links .....
         */
        if ((source_stat.st_mode & S_IFLNK) == S_IFLNK) {       /* symlink */
            unlink(targetfile);                 /* remove before update */
            errno = 0;
            copy_link(do_verify, sourcefile, targetfile, sourcefull,
                      targetfull, &source_stat);
            continue;
        }

        /* Otherwise, it must be an ordinary file, let's try
         * to copy it anyhow .....
         */
        copy_file(do_verify, sourcefile, targetfile, sourcefull,
                  targetfull, &source_stat);
    }   /* while !feof(stdin) */

    /* Let the user know how many files we copied .....
     */
bailout:
    set_signals(0);                 /* disable signals for final report */
    if (program_intr != 0) {
        fprintf(stderr, "\n-=-=-=- FILE TRANSFER INTERRUPTED -=-=-=-\n");
    }
    fprintf(stderr, "TOTAL FILES:  %ld files read\n", total_files);
    fprintf(stderr, "TOTAL COPIED: %ld files %s\n",
            (total_files - total_errors),
            ((do_verify != 0) ? "copied/verified" : "copied"));
    fprintf(stderr, "TOTAL ERRORS: %ld non-recoverable\n", total_errors);
    fprintf(stderr, "TOTAL ERRORS: %ld recoverable\n", total_softerr);
    if (total_errors > 0L || program_intr != 0) {
        fprintf(stderr, "-=-=-=- FILES MISSING IN TRANSFER -=-=-=-\n");
    }
    fflush(stderr);
}


/*
 * -=-=-=- MAIN PROGRAM -=-=-=-
 */

/* main - perform intelligent copy of files
 */
int main(argc, argv)
int argc;
char **argv;
{
    struct passwd *pws;               /* root password entry */
    int opt, files_ok;                /* option scan, copy success */
    int verify_flag = 0;              /* copy verify flag */
    char *source_name;                /* source directory name */
    char *target_name;                /* target directory name */
    int myid, myeuid;                 /* getuid/geteuid return */

    /* Disable signals until we get our bearings .....
     */
    set_signals(0);

    /* Get command line options ....
     */
    while ((opt = getopt(argc, argv, "hiv")) != EOF) {
        switch (opt) {
            case 'h':
                usage(argv[0]); exit(0);
                break;
            case 'i':
                if (*(afsdcp_ischeck) == '%') {
                    fprintf(stdout, "%s is not checked in.\n", argv[0]);
                } else {
                    fprintf(stdout, "%s: version %s\n", argv[0],
                        afsdcp_version);
                }
                exit(0);
                break;
            case 'v':
                verify_flag = 1;
                break;
            default:
                usage(argv[0]); exit(1);
                break;
        }
    }

    /* Check for too many arguments ....
     */
    if ((argc - optind) != 2) {
        fprintf(stderr, "%s: specify source/target directories!\n",
                argv[0]);
        usage(argv[0]); exit(1);
    }

    /* Print a warning if we're not running this as root .....
     */
    pws = getpwnam("root"); myid = getuid();
    myeuid = geteuid();
    if (pws->pw_uid != myid && pws->pw_uid != myeuid) {
        fprintf(stderr,
                "%s: WARNING: run as root to ensure accurate copy!\n",
                argv[0]);
    }

    /* Get source/target directories and process list of
     * files to be copied .....
     */
    source_name = argv[optind];         /* first argument left */
    target_name = argv[optind+1];       /* second argument left */

    /* Process list of files to be copied .....
     */
    files_ok = process_files(argv[0], stdin, verify_flag,
                             source_name, target_name);

    /* PRINT FINISHED BANNER
     */
    fprintf(stderr,"COPY ENDED:   ");
    print_time(stderr);

    /* Close input/output if not from stdin/stdout .....
     */
    fflush(stderr); fflush(stdout);

    /* That's all folks .....
     */
    if ((total_errors != 0L) || (program_intr != 0)) {
        exit(1);
    }
    exit(0);
}

/*
 * End of bldafsdcp.c.
 */

