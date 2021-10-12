/*
 * @(#) 29 1.4  src/bldenv/bldtools/chksubsys/bldsbprint.c, bldprocess, bos412, GOLDA411a 5/6/93 15:44:40
 *
 * COMPONENT_NAME: (BLDPROCESS) BAI Build Process
 *
 * FUNCTIONS: n/a
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * !!!! bldsbprint.c: (vi: set tabstop=4) !!!!
 *
 * NAME:          bldsbprint
 *
 * PURPOSE:       bldsbprint takes a set of defect notes for a specific
 *                defect number and finds the appropriate subsystem note
 *                information
 *
 * INPUT:         a file or input stream containing defect notes
 *                listed in date-ascending order
 *
 * OUTPUT:        the appropriate block of lines in subsystem markers
 *                displayed without the markers
 *
 * OPTIONS:       -h          displays a help message
 *                -v          displays source version information
 *                -d          specifies the defect number (required)
 *                -i          specifies the input file (default: stdin)
 *                -o          specifies the output file (default: stdout)
 *
 */
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/utsname.h>

extern int optind;
extern char *optarg;


/* Program input defines .....
 */
#define MAXINPUTLINE            2048        /* 2048 chars/line */
#define NOTEOWNER_SZ            32          /* 31+1 bytes for note owner */
#define DEFECT_SZ               32          /* 31+1 bytes for defect name */
#define HOSTDATA_SZ             120         /* 119+1 bytes for host data */
#define FILEDATA_SZ             512         /* 511+1 bytes for host data */
#define JUNKSTR_SZ              256         /* 255+1 bytes for junk strings */

/* Input string lengths and field numbers .....
 */
#define START_SS_LEN            8           /* 8 bytes for START_SS */
#define STOP_SS_LEN             7           /* 7 bytes for STOP_SS */
#define NOTEOWNER_FIELD         5           /* field 5 for note owner */
#define NOTETEXT_FIELD          9           /* field 9 for note text */


/* Temporary file names .....
 */
#define TEMPFILE_NOTE           "Ssbpt.a"   /* template for temp filename */


/* SCCS information ....
 */
static char sbprint_sccsid[] =
       "@(#) 29 1.4  src/bldenv/bldtools/chksubsys/bldsbprint.c, bldprocess, bos412, GOLDA411a 5/6/93 15:44:40";
static char sbprint_ischeck[] = "@(#)";
static char sbprint_version[] = "1.4 - 93/05/06 15:44:40";


/*
 * -=-=-=- USAGE FUNCTIONS -=-=-=-
 */

/* usage - print program usage
 */
void usage(cname)
char *cname;
{
    char startin[JUNKSTR_SZ];   /* text for indentation */
    int  startspc;              /* number of spaces for indentation */

    /* Text for the usage message ....
     */
    char a0[] =
      "%s: usage: %s [-h] [-v] [-i input-file] [-o output-file]\n";
    char a1[] =
      "%s -d defect-number\n";
    char a2[] =
      "     where:\n";
    char a3[] =
      "          -h      displays usage message and exits\n";
    char a4[] =
      "          -v      displays source version information and exits\n";
    char a5[] =
      "          -i      specifies the input file (default: stdin)\n";
    char a6[] =
      "          -o      specifies the output file (default: stdout)\n";
    char a7[] =
      "          -d      specifies the defect number (required)\n\n";
    char a8[] =
      "     Takes defect notes in CMVC Report noteView -raw format\n";
    char a9[] =
      "     and searches for subsystem information delimited by\n";
    char aa[] =
      "     START_SS and STOP_SS markers.  Notes containing subsystem\n";
    char ab[] =
      "     information from a 'mustfix' ID will carry precedence\n";
    char ac[] =
      "     over those from ordinary users.\n\n";

    /* Calculate the number of spaces for indenting the usage
     * continuation .....
     */
    memset(startin, (char) 0, JUNKSTR_SZ);
    sprintf(startin, "%.80s: usage: %.80s", cname, cname);
    startspc = strlen(startin);
    memset(startin, (char) ' ', startspc);

    /* Display usage message ....
     */
    fprintf(stderr, a0, cname, cname);
    fprintf(stderr, a1, startin);
    fprintf(stderr, a2); fprintf(stderr, a3);
    fprintf(stderr, a4); fprintf(stderr, a5);
    fprintf(stderr, a6); fprintf(stderr, a7);
    fprintf(stderr, a8); fprintf(stderr, a9);
    fprintf(stderr, aa); fprintf(stderr, ab);
    fprintf(stderr, ac);
}


/*
 * -=-=-=- MISCELLANEOUS DATA HANDLING FUNCTIONS -=-=-=-
 */

/* strip_spaces - remove leading/trailing white space from a string
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

    /* Strip leading space last, that takes more effort .....
     */
    for (sp = s; isspace(*sp); ++sp)
        ;
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
 * string_lower - convert a string to all lowercase
 */
void string_lower(t)
char *t;
{
    char *p;

    for (p = t; *p; ++p) {
        *p = ((isupper(*p)) ? tolower(*p) : *p);
    }
}


/*
 * -=-=-=- SUPPORT ROUTINES -=-=-=-
 */

/*
 * is_mustfix - determines if the noteowner is a mustfix ID
 */
int is_mustfix(owner)
char *owner;
{
    /* Return true for mustfix/mustfix1 ID's .....
     */
    if (strcmp(owner, "mustfix") == 0) {
        return(1);
    }
    if (strcmp(owner, "mustfix1") == 0) {
        return(1);
    }
    return(0);
}


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


/*
 * build_tempfile_name - build the name of the temporary file to
 *                       be used
 */
void build_tempfile_name(targetname, template)
char *targetname, *template;
{
     char my_hostname[HOSTDATA_SZ];
     struct utsname st_myuname;
     char *tmp_envptr, *cp;
     char my_bldtmp[JUNKSTR_SZ];
     int hostok, unameok;

     /* Clean out the temporary variables we need first thing .....
      */
     memset(my_hostname, (char) 0, HOSTDATA_SZ);
     memset(my_bldtmp,   (char) 0, JUNKSTR_SZ);

     /* Get the value of BLDTMP, if it exists, and use that for
      * our temporary directory.  If it doesn't exist, then we'll
      * use /tmp .....
      */
     tmp_envptr = getenv("BLDTMP");
     if (tmp_envptr == (char *) 0) {
         strncpy(my_bldtmp, "/tmp", 4);
     } else {
         strncpy(my_bldtmp, tmp_envptr, JUNKSTR_SZ-1);
     }

     /* Build the target file name without the host name .....
      */
     sprintf(targetname, "%s/%s%d.", my_bldtmp,
                         template, getpid());

     /* Ask hostname/uname for my host's name .....
      */
     hostok = gethostname(my_hostname, HOSTDATA_SZ-1);
     if (hostok != 0) {
         unameok = uname(&st_myuname);
         if (unameok != 0) {
             strncat(targetname, "unknown", 8);
         } else {
             strncat(targetname, st_myuname.nodename, _SYS_NMLN);
         }
     } else {
         /* Take the first piece of the hostname and add that to the
          * target file name (we'll assume we're using networks as
          * boundaries) .....
          */
         cp = my_hostname;
         while (*cp && (*cp != '.')) ++cp;
         *cp = (char) 0;
         strncat(targetname, my_hostname, HOSTDATA_SZ);
     }
}


/*
 * dump_file - dump the contents of a file of lines to output
 */
void dump_file(infp, outfp)
FILE *infp, *outfp;
{
    char inputbuf[MAXINPUTLINE], *ok_fgets;

    /* Scan the file, remove the junk, and copy the output .....
     */
    while (!feof(infp)) {
        memset(inputbuf, (char) 0, MAXINPUTLINE);
        ok_fgets = fgets(inputbuf, (MAXINPUTLINE-1), infp);
        if (!ok_fgets) continue;        /* bad read, continue */

        /* Skip null reads and comments ....
         */
        if (*inputbuf == (char) 0) continue;
        if (*inputbuf == '#') continue;

        /* Chew up the input line from fgets .....
         */
        chew_input_line(inputbuf, (char *) 0, MAXINPUTLINE);

        /* Spit out an output record .....
         */
        fprintf(outfp, "%s\n", inputbuf);
    }
}


/* process_files - process the input file to create the proper
 *                 block of subsystem information on the output
 *                 file
 */
void process_files(cname, infp, outfp, temp_notefile, defectname)
char *cname;                /* command name */
FILE *infp, *outfp;         /* input/output file pointers */
char *temp_notefile;        /* temporary note file name */
char *defectname;           /* defect name specified */
{
    FILE *temp_notefp = (FILE *) NULL;
    char inputline[MAXINPUTLINE], cookedline[MAXINPUTLINE];
    char noteowner[NOTEOWNER_SZ], *fgets_ok, *cp;
    char templine[MAXINPUTLINE];
    int  frozen_tag = 0, statewalk = 0;
    int  pos, numfields, next_note;

    /* Set states for reading the text .....
     */
    frozen_tag = 0;             /* data not frozen for mustfix ID */
    statewalk  = 0;             /* start state, see below */

    /*
     * States for walking the notes:
     *
     *      0 = program start state (reached only once)
     *      1 = in the middle of a note (all non-subsystem text)
     *      2 = in the middle of a subsystem note
     *
     *  So, if we're in state 2 and we've not frozen the results of
     *  the previous search, we just keep dumping lines at a file
     *  until we're out of start/stop subsystem text.
     *
     */

    /* Perform loading of the defect notes .....
     */
    while (!feof(infp)) {
        memset(inputline,  (char) 0, MAXINPUTLINE);
        fgets_ok = fgets(inputline, (MAXINPUTLINE-1), infp);
        if (!fgets_ok) continue;        /* bad read, continue */

        /* Skip null reads and comments ....
         */
        if (*inputline == (char) 0) continue;
        if (*inputline == '#') continue;

        /* Chew up the input line from fgets, skip blank lines .....
         */
        chew_input_line(inputline, cookedline, MAXINPUTLINE);
        if (*cookedline == (char) 0) continue;

        /* Figure out if this line's one that's the start of a
         * defect note .....
         */
        if (!(isspace(*inputline))) {
            /* Scan the input line for field separators .....
             */
            numfields = 0; next_note = 0; cp = inputline;
            while (*cp) {
                numfields += ((*cp == '|') ? 1 : 0);
                ++cp;
            }

            /* If we got 8 field separators, we've got a possible
             * candidate for a note starting entry, so let's be
             * sure - check the first field for the defect number .....
             */
            if (((strncmp(inputline, defectname, \
                  strlen(defectname))) == 0) &&
                  (numfields == (NOTETEXT_FIELD-1))) {
                next_note = 1;
            }

            /* If we've really got a note, then we'll grab the owner and
             * the remainder of the line from this line ..... */
            if (next_note == 1) {
                /* Get noteowner (field 5) .....
                 */
                memset(noteowner, (char) 0, NOTEOWNER_SZ);
                statewalk = 1; cp = inputline;
                numfields = 0; pos = 0;
                while (*cp) {
                    if (numfields >= NOTEOWNER_FIELD) break;
                    if (*cp == '|') {
                        ++numfields;
                    } else {
                        if (numfields == (NOTEOWNER_FIELD-1)) {
                            noteowner[pos] = *cp;
                            ++pos;
                        }
                    }
                    ++cp;
                }   /* while *cp */

                /* Get remainder of line (field 9) .....
                 */
                memset(templine, (char) 0, MAXINPUTLINE);
                cp = inputline; numfields = 0; pos = 0;
                while (*cp) {
                    if (*cp == '|') {
                        ++numfields;
                    } else {
                        if (numfields == (NOTETEXT_FIELD-1)) {
                            templine[pos] = *cp;
                            ++pos;
                        }
                    }
                    ++cp;
                }   /* while *cp */

                /* Copy new input buffer to old input buffer,
                 * redo space stripping .....
                 */
                strncpy(inputline, templine, MAXINPUTLINE);
            }   /* if next_note == 1 */
        }   /* if !isspace(*inputline) */

        /* Now that we've got a note (maybe) and a state (definitely),
         * let's see if we've got some subsys text .....
         */
        strncpy(templine, inputline, START_SS_LEN);
        templine[START_SS_LEN+1] = (char) 0;
        string_lower(templine);

        /* If we get a match on START_SS, then we'll start handling
         * the records .....
         */
        if ((strncmp(templine, "start_ss", START_SS_LEN)) == 0) {
            switch (statewalk) {
                case 0:
                    /* New START_SS, no known owner .....
                     */
                    strncpy(noteowner, "unknown", 8);
                    statewalk = 1;
                    continue;
                case 1:
                    /* START_SS in the middle of a note .....
                     */
                    if (is_mustfix(noteowner)) frozen_tag = 0;
                    if (frozen_tag == 0) {
                        unlink(temp_notefile); unlink(temp_notefile);
                        if (temp_notefp != (FILE *) NULL) {
                            fclose(temp_notefp);
                        }
                        temp_notefp = (FILE *) NULL;
                    }
                    statewalk = 2; continue;
                default:
                    /* START_SS in the middle of a subsystem note
                     * (ignore it, and it'll go away???) .....
                     */
                    statewalk = 2; continue;
            }   /* switch statewalk */
        }   /* if start_ss */

        /* If we get a match on STOP_SS, then we'll go back to
         * handling ordinary note records .....
         */
        if ((strncmp(templine, "stop_ss", STOP_SS_LEN)) == 0) {
            switch (statewalk) {
                case 0:
                    /* STOP_SS, no START_SS (ignore it) .....
                     */
                    statewalk = 0; continue;
                case 1:
                    /* STOP_SS in the middle of a note (ignore it) .....
                     */
                    statewalk = 1; continue;
                default:
                    /* STOP_SS after START_SS, process lines .....
                     */
                    if (is_mustfix(noteowner)) {
                        frozen_tag = 1;     /* freeze text for mustfix ID */
                    }
                    statewalk = 1; continue;
            }   /* switch statewalk */
        }   /* if stop_ss */

        /* And now the annoying part, handling the append of
         * lines between START_SS/STOP_SS markers .....
         */
        if ((frozen_tag == 0) && (statewalk == 2)) {
            /* If the file's not open, create it before trying
             * to write to it .....
             */
            if (temp_notefp == (FILE *) NULL) {
                /* Remove twice for filesystem inertia .....
                 */
                unlink(temp_notefile); unlink(temp_notefile);
                if ((temp_notefp = fopen(temp_notefile, "w")) ==
                    (FILE *) NULL) {
                    fprintf(stderr, "%s: cannot open workfile %s\n",
                            cname, "for write!");
                    unlink(temp_notefile); unlink(temp_notefile);
                    exit(1);
                }
            }
            fprintf(temp_notefp, "%s\n", inputline);
        }
    }   /* while !feof(infp) */

    /* Now dump the contents of the final note file to the
     * desired note file .....
     */
    if (temp_notefp != (FILE *) NULL) {
        fflush(temp_notefp);
        fclose(temp_notefp);
    }
    if ((temp_notefp = fopen(temp_notefile, "r")) != (FILE *) NULL) {
       dump_file(temp_notefp, outfp);
       fclose(temp_notefp);
    }

    /* Remove temporary note files (twice for filesystem inertia) .....
     */
    unlink(temp_notefile); unlink(temp_notefile);
}


/*
 * -=-=-=- MAIN PROGRAM -=-=-=-
 */

/* main - do start/stop subsystem info search
 */
int main(argc, argv)
int argc;
char **argv;
{
    FILE *inputfp, *outputfp;
    char *inputfile = (char *) 0;       /* input file name */
    char *outputfile = (char *) 0;      /* output file name */
    char notefile_temp[FILEDATA_SZ];    /* note temporary file name */
    char defect_name[DEFECT_SZ];        /* defect name text */
    int  opt;                           /* option scan */


    /* Build the names of all the temporary files we'll need
     * and set all the buffers we need to null .....
     */
    memset(notefile_temp, (char) 0, FILEDATA_SZ);
    memset(defect_name,   (char) 0, DEFECT_SZ);
    build_tempfile_name(notefile_temp, TEMPFILE_NOTE);

    /* Get command line options ....
     */
    while ((opt = getopt(argc, argv, "hvd:i:o:")) != EOF) {
        switch (opt) {
            case 'h':
                usage(argv[0]); exit(0);
                break;
            case 'v':
                if (*(sbprint_ischeck) == '%') {
                    fprintf(stdout, "%s is not checked in.\n", argv[0]);
                } else {
                    fprintf(stdout, "%s: version %s\n", argv[0],
                        sbprint_version);
                }
                exit(0);
                break;
            case 'd':
                if (*defect_name) {
                    fprintf(stderr, "%s: %s!\n", argv[0],
                            "defect name defined multiple times!");
                    exit(1);
                } else {
                    strncpy(defect_name, optarg, DEFECT_SZ);
                }
                break;
            case 'i':
                if (inputfile != (char *) 0) {
                    fprintf(stderr, "%s: %s!\n", argv[0],
                            "input file defined multiple times!");
                    exit(1);
                } else {
                    inputfile = optarg;
                }
                break;
            case 'o':
                if (outputfile != (char *) 0) {
                    fprintf(stderr, "%s: %s!\n", argv[0],
                            "output file defined multiple times!");
                    exit(1);
                } else {
                    outputfile = optarg;
                }
                break;
            default:
                usage(argv[0]); exit(1);
                break;
        }
    }

    /* Check for too many arguments ....
     */
    if ((argc - optind) > 0) {
        fprintf(stderr, "%s: must not supply additional arguments!\n",
                argv[0]);
        usage(argv[0]); exit(1);
    }

    /* Check for an unset defect name .....
     */
    if (!(*defect_name)) {
        fprintf(stderr, "%s: defect number must be defined!\n",
                argv[0]);
        usage(argv[0]); exit(1);
    }

    /* Open the input file, if defined .....
     */
    if (*inputfile) {
       /* If we've got an input file defined, then open it .....
        */
       if ((inputfp = fopen(inputfile, "r")) == (FILE *) NULL) {
           fprintf(stderr, "%s: cannot open input file!\n", argv[0]);
           exit(1);
       }
    } else {
       /* No input file defined, use stdin .....
        */
       inputfile = "<standard input>";
       inputfp   = stdin;
    }

    /* Open the output file, if defined .....
     */
    if (*outputfile) {
       /* If we've got an output file defined, clobber the old
        * one, then open the new one .....
        */
       unlink(outputfile); unlink(outputfile);
       if ((outputfp = fopen(outputfile, "w")) == (FILE *) NULL) {
           fprintf(stderr, "%s: cannot open output file!\n", argv[0]);
           exit(1);
       }
    } else {
       /* No output file defined, use stdout .....
        */
       outputfile = "<standard output>";
       outputfp   = stdout;
    }

    /* Process the input and output files .....
     */
    process_files(argv[0], inputfp, outputfp, notefile_temp,
                  defect_name);

    /* Close input/output if not from stdin/stdout .....
     */
    fflush(outputfp);
    if (inputfp != stdin)   fclose(inputfp);
    if (outputfp != stdout) fclose(outputfp);

    /* That's all folks .....
     */
    exit(0);
}

/*
 * End of bldsbprint.c.
 */

