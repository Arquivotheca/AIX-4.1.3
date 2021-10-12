#ifndef lint
static char sccsid[] = "@(#)47 1.13 src/bos/usr/bin/setmaps/setmaps.c, cmdtty, bos411, 9428A410j 3/22/94 02:55:40";
#endif
/*
 * COMPONENT_NAME: CMDTTY terminal control commands
 *
 * FUNCTIONS: main (setmaps.c)
 *
 * ORIGINS: 27, 83
 *
 */
/*
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/*
 * This program manages mappings on ttys. It serves three purposes:
 * 1) Tells whichs maps, if any are in use on the invoking tty
 * 2) connects the invoking tty to a previously loaded map
 * 3) loads a new map into the kernel, which can then be connected to
 *
 * This program reads input/output translation maps into the structures
 * defined in /usr/include/sys/ttmap.h. Maps are then loaded into the
 * kernel using the TCSMAP(set map) ioctl in tty.c. Only su can load
 * a map. Once loaded, a terminal may use the map by setting the imap
 * and omap parameters with stty or adding them in the /etc/ports file.
 * Both stty.c and getty.c call setmaps.
 *
 * A map file contains a set of rules which define the translation of
 * one or more characters to a replacement character or string. The
 * syntax of each rule is pattern:newstring. Examples:
 * \033b:ps -ef remaps 2nd func key on an ibm3161 terminal
 * \@:\? remap '@' to '?'. Slash req'd for special char
 * \x80:\x20 remap vt220 8-bit char to space
 * a?a:b$2b ? matches any single byte; $n outputs nth char
 * from pattern.
 *
 * The ttmapch() routine in tt1.c uses the loaded maps to do
 * translations on incoming and outgoing characters. The purpose of
 * these translations is to provide a consistent character set across
 * all ascii terminals.
 *
 * This program also manages code set maps (the -s option).
 * A code set map determines the length and width of each code point.
 * Length means the number of bytes of storage used and
 * width means the number of display columns to use.
 * 
 * The -i or -I option causes the program to install a code set map
 * by calling the c library routine setcsmap.  The setcsmap routine
 * reads the file specified, parses it, builds a csmap structure and
 * calls ioctl with the TCSCSMAP option to install it in the kernel.
 * The -o or -O option causes the program to retrieve the current code
 * set map by calling ioctl with the TCGCSMAP option, converting the
 * resulting csmap structure to file format and writing it to the file
 * specified.
 *
 * The code set map file format is 512 lines each ended with a '\n'.
 * The first 256 lines specify the length of the code points starting
 * with the value of the line number - 1.  The second 256 lines specify
 * the width of the code points starting with the value of the line
 * number - 257.  For example, if line 20 is "1'\n'", then the length
 * of code points starting with a value of 19 in the first byte is 1.
 *
 * main - program driver; read arguments presented on command line and 
 *        call the appropriate routines to take action.
 * clear_maps - disconnects this tty from the input, output, or both 
 * maps. If the map the tty was using has no other ttys connected to
 * it, the map is thrown away by the kernel, unless it was
 * specified as a sticky map when it was loaded.
 * use_map - connect a terminal to an already-loaded map.
 * load_map - read file provided by user and load it into the kernel
 * via the TCSMAP ioctl(must have root access to load a new map)
 * show_map - show which maps are in use(if any).
 * magic_key - Check a map key to see if it is a magic name which
 * really means to delete the mapping from the tty.
 * keyname - find the filename only given the entire pathname. The
 * .in and .out suffixes are part of the keyname.
 * fullpath - takes a map name and builds the full pathname of the
 * file we need to open to get the map.
 * usage - tell the user about the flags they can use.
 * makemap - read a line at a time from the specified map file and
 * store the pattern/replacement pair in a linked list of ttrules.
 * add_rule - Add a rule to the map.
 * chain_to_hash - Add a new explicit rule to the end of a hash chain.
 * chain_to_wild - Chain a new wildcard rule onto the end of the
 * wildcard rule chain.
 * hash_char - Determine the character which represents the first char
 * of a pattern
 * translate - Translate a mapping string from the external ascii form
 * to the internal form used by the kernel mapping code.
 * xlate_esc_char - Handle the various types of backslash escapes which
 * can be present in the input string.
 * xlate_char_set - Interpret a set of chars of the form [abcd] or [a-d]
 * no_op - Check to see if pattern/replacement pair really do anything
 * find_replace - Find the start of repl pattern in the input string.
 * is_meta: Is this a meta character?
 * has_meta: Are there meta characters in this string?
 * is_wild - Is this pattern a wildcard? 
 * asciify - Debug code. Given a char, return an ascii string
 * representing it.
 * asciistr - Debug code. Given a string and a pointer to a buffer,
 * make new string using asciify to convert non-printables to
 * printable representions.
 * dump_map - Debug code. Dump the map to stdout.
 * csmap_path - takes a map name and builds the full pathname of the
 * file we need to open to get the code set map.
 */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/termio.h>
#include <termios.h>
#include <sys/ttmap.h>
#include <string.h>
#include <stropts.h>
#include <sys/eucioctl.h>
#include <sys/str_tty.h>
#include        <locale.h>
#include        <nl_types.h>
#include        "setmaps_msg.h"
#include <sys/device.h>
#include <sys/sysconfig.h>

/* needed for cfg'ing the nls module after it's been loaded into
 * the kernel.  This is defined in snls.h
 */
typedef struct nlsdd_init {
	long which_dds;  /* must be NLS_DDS - defined in <sys/str_tty.h>*/
};



nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_SETMAPS,num,str)



#define NLS_DIR "/usr/lib/nls/termmap/"
#define CSMAP_DIR "/usr/lib/nls/csmap/"

#define IN_SUFFIX ".in"
#define OUT_SUFFIX ".out"

#define MAGIC_KEY1 "."			/* magic word to clear a map */
#define MAGIC_KEY2 "NOMAP"		/* magic word to clear a map */

#define NOTHING 0			/* Initial state */
#define SET_INPUT 1			/* set the input map */
#define SET_INPUT_FROM_PATH 2		/* set input map, given full path */
#define SET_OUTPUT 3			/* set the output map */
#define SET_OUTPUT_FROM_PATH 4		/* set output map, given full path */
#define SET_BOTH 5			/* set both input and output maps */
#define LOAD_ONLY 6			/* load the map, don't use it */
#define LOAD_ONLY_FROM_PATH 7		/* load the map, given full path */
#define CLEAR_MAPS 8			/* remove mapping */

#define NO_FLAGS 0			/* means just what it says */

#define OK 0				/* good result from an internal func */
#define BAD_RES -1				/* an internal func had a problem */

#define NLS_MODULE "nls"            /* name of the nls module, currently str_nls */

int verb = NOTHING;			/* what we're going to do */
int force_reload = FALSE;		/* force a load even if it's there */

int Dump_map = FALSE;			/* dump the map to stdout */
int verbose = FALSE;			/* print verbose info */

char *map_name = NULL;			/* map name provided by user */
char *explicit_key = NULL;		/* explicit key given by user */
char *nls_dir = NLS_DIR;		/* dir where maps live */
char *csmap_dir = CSMAP_DIR;            /* dir where csmaps live */
int csmap_option = 0;			/* whether code set map is specified */
char *arg0;				/* name we were invoked as */

/* Function prototypes */

void main(int argc, char **argv);
int clear_maps(long flags);
int use_map(int verb, char *map_name, char *explicit_key, int level);
int load_map(int verb, int flags, char *map_name, int key);
void show_map(int code);
int magic_key(char *p);
char *keyname(char *map_name, char *explicit_key, int verb);
char *fullpath(int verb, char *map_name);
void usage();
int makemap(struct ttmap *ttmap, char *path, char *mapname);
int add_rule(struct ttmap *ttmap, char *pat, char *rep);
void chain_to_hash(struct ttmap *ttmap, int hash, int new, int end);
void chain_to_wild(struct ttmap *ttmap, int first, int new);
int hash_char(char *p);
int translate(char *in, char *out, int bufsize, int which);
int xlate_esc_char(char *p, char **new);
int xlate_char_set(char *in, char **pp, char **qp, char *fence);
int no_op(char *p, char *q);
char *find_replace(char *p);
int is_meta(int c, int which);
int has_meta(char *s, int which);
int is_wild(char *s);
char *asciify(unsigned char c);
char *asciistr(unsigned char *p, char *buf);
void dump_map(struct ttmap *ttmap);
int x_ioctl(int fd, int cmd, struct tty_map *tty_map, int size);
char *csmap_path(int verb, char *map_name);
int check_nls();

void main(int argc, char **argv)
{
    long flags;				/* state flags while parsing args */
    int rc, c;				/* scratch */
    char *p;				/* scratch */
    extern int optind;			/* option index, set by getopt */
    extern char *optarg;		/* argument of switch, set by getopt */

    int dflag = 0;		        /* flag for -d option */

    setlocale(LC_ALL,"") ;
    catd = catopen(MF_SETMAPS, NL_CAT_LOCALE);

    arg0 = *argv;			/* save name we were invoked with */
    p = strrchr(arg0, '/');
    if (p != NULL) {
	arg0 = ++p;			/* skip dir part if any */
    }

    /* 
     * No arguments to setmaps. Display names of maps currently
     * loaded for this tty.
     */
    if (argc == 1) {
	show_map(TM_INPUT);
	show_map(TM_OUTPUT);
	exit(0);
    }

    /* Process flags passed in by user. */
    while ((c = getopt(argc,argv,"t:i:o:I:O:k:l:L:d:rcDvhs")) != EOF) {
	switch (c) {

	    /*
	     * Code set map instead of terminal map.
	     */
        case 's':
	    if (Dump_map)                /* Check flag to see if */
		    usage();             /* prev args conflict */
            csmap_option = 1;
            break;

	    /* 
	     * Two-way mapping. To make sense, this must be a path 
	     * name or filename with no .in/.out suffix.
	     */
	case 't':
	    if (verb != NOTHING)	/* Check flag to see if */
		usage();		/* prev args conflict */
	    verb = SET_BOTH;
	    map_name = optarg;
	    break;

	    /* 
	     * Input map. Expecting filename only, with or without 
	     * suffix.
	     */
	case 'i':
	    if (verb != NOTHING)
		usage();
	    verb = SET_INPUT;
	    map_name = optarg;
	    break;

	    /* 
	     * Output map. Expecting filename only, with or without 
	     * suffix.
	     */
	case 'o':
	    if (verb != NOTHING)
		usage();
	    verb = SET_OUTPUT;
	    map_name = optarg;
	    break;

	    /* 
	     * Input map does not live in /usr/lib/nls/termmap. Use full 
	     * path name provided by user. User must include suffix.
	     */
	case 'I':
            if ((verb != NOTHING) ||
		dflag)
		usage();
	    verb = SET_INPUT_FROM_PATH;
	    map_name = optarg;
	    break;

	    /* Output map does not live in /usr/lib/nls/termmap. Use
	     * full path name provided by user. User must include
	     * suffix.
	     */
	case 'O':
            if ((verb != NOTHING) ||
		dflag)
		usage();
	    verb = SET_OUTPUT_FROM_PATH;
	    map_name = optarg;
	    break;

	    /* 
	     * Specify map name to be used in ttmap struct. Permits
	     * su to specify name of map in linked list of maps in
	     * the kernel. May or may not include a suffix.
	     */
	case 'k':
	    explicit_key = optarg;
	    break;

	    /* 
	     * Load a map. Only superuser may do this. Must include 
	     * suffix
	     */
	case 'l':
	    if (geteuid() != 0) {
	      fprintf(stderr,
		      MSGSTR(ROOT, "%s: must be root to load a map\n"),
		      arg0);
	      usage();
	    }
	    if (verb != NOTHING)
		    usage();
	    verb = LOAD_ONLY;
	    map_name = optarg;
	    break;

	case 'L':
	    if (geteuid() != 0) {
	      fprintf(stderr,
		      MSGSTR(ROOT, "%s: must be root to load a map\n"),
		      arg0);
	      usage();
	    }
	    if (verb != NOTHING)
		usage();
	    verb = LOAD_ONLY_FROM_PATH;
	    map_name = optarg;
	    break;

	case 'r':			/* Force reload of map */
            if (dflag)
		    usage();
	    if (geteuid() != 0) {
		fprintf(stderr,
			 MSGSTR(ROOT, "%s: must be root to reload a map\n"),
			 arg0);
		usage();
	    }
	    force_reload = TRUE;
	    break;

	case 'd':			/* Override default dir */
	    if (dflag || force_reload || 
		(verb == SET_INPUT_FROM_PATH) || 
		(verb == SET_OUTPUT_FROM_PATH))
		    usage();
	    else dflag = 1;
	    nls_dir = optarg;
	    break;

	case 'c':	    /* clear all mappings for this tty */
            if ((verb != NOTHING) || Dump_map || csmap_option)
		usage();
	    verb = CLEAR_MAPS;
	    break;

	case 'D':			/* Dump map to stdout */
            if ((verb == CLEAR_MAPS) || csmap_option)
		    usage();
	    Dump_map = TRUE;
	    break;

	case 'v':			/* be verbose */
	    verbose = TRUE;
	    break;

	case 'h':			/* print usage message and exit */
	    usage();
	    break;

	default:			/* Bad flag. */
	    usage();
	    /*NOTREACHED*/
	}				/* end of switch */
    }					/* end of while (getopt) */

    /* Any floating args, not preceded by switches? */
    if (optind < argc)
	usage();

    /*
     * If the -s option is used, then the code set map is being installed
     * or retrieved.  The only options that are compatible with -s are
     * -i, -I, -o, and -O, plus -v.
     */
    if (csmap_option) {
        char *path;

        /*
         * The mapname is required.
         */
        if (map_name == (char *)NULL)
            usage();

        rc = OK;

        switch (verb) {
        case SET_INPUT:
        case SET_INPUT_FROM_PATH:
	    /*
	     * Determine the pathname for the code set map.
	     */
            path = csmap_path(verb, map_name);
            if (verbose)
                fprintf(stderr,
                        MSGSTR(CSMAPPATH, "%s: csmap pathname is: %s\n"),
                        arg0, path);
	    /*
	     * Call the library routine which will read the code set map,
	     * parse it, build a csmap structure and install it in the kernel.
	     */
            rc = setcsmap(path);
            break;
	/* We no longer support the option of retrieving the 
	 * current code set and moving it to a file.
	 */
        case SET_OUTPUT:
        case SET_OUTPUT_FROM_PATH:
        default:
            usage();
        }

        exit(rc);

    }

    flags = NO_FLAGS;

    /* Check to see if map name given really means to delete a map */
    if (magic_key(map_name)) {
	switch (verb) {
	case CLEAR_MAPS:
	case SET_BOTH:
	    flags = TM_INPUT | TM_OUTPUT;
	    /* fall thru */
	case SET_INPUT:
	    if (flags == NO_FLAGS)
		flags = TM_INPUT;
	    /* fall thru */
	case SET_OUTPUT:
	    if (flags == NO_FLAGS)
		flags = TM_OUTPUT;
	    verb = CLEAR_MAPS;
	    break;
	default:
	    usage();
	    /*NOTREACHED*/
	}
    }

    rc = OK;

    switch (verb) {
	char *key;

    case CLEAR_MAPS:
	rc = clear_maps(flags);
	break;

    case SET_INPUT:
    case SET_INPUT_FROM_PATH:
	rc = use_map(verb, map_name, explicit_key, 0);
	break;

    case SET_OUTPUT:
    case SET_OUTPUT_FROM_PATH:
	rc = use_map(verb, map_name, explicit_key, 0);
	break;

    case SET_BOTH:
	if (explicit_key != NULL) {
	    fprintf(stderr, MSGSTR(CANTUSE, "%s: can't use -k with -t\n"),
		    arg0);
	    rc = BAD_RES;
	    break;
	}
	if ((rc = use_map(SET_INPUT, map_name, explicit_key, 0)) != OK)
	    break;
	rc = use_map(SET_OUTPUT, map_name, explicit_key, 0);
	break;

    case LOAD_ONLY:
    case LOAD_ONLY_FROM_PATH:
	key = keyname(map_name, explicit_key, verb);
	rc = load_map(verb,
		       TM_STICKY |((force_reload) ? TM_RELOAD : TM_LOAD),
		       map_name, (int)key);
	break;

    default:
	fprintf(stderr,
		MSGSTR(INTERN, "%s: internal error, unexpected verb=%d\n"),
		 arg0, verb);
	rc = BAD_RES;
	break;
    }

    exit(rc);
    /*NOTREACHED*/
}


/* clear_maps - remove mappings for this tty */
int clear_maps(long flags)
{
    struct tty_map tty_map;
    struct strioctl i_str;

    memset((void *)&tty_map, 0, (size_t)sizeof(struct tty_map));
    tty_map.tm_version = TTMAP_VERSION;
    tty_map.tm_flags = flags | TM_CLEAR;

    i_str.ic_cmd = TCSMAP;
    i_str.ic_timout = 0;
    i_str.ic_len = sizeof(struct tty_map);
    i_str.ic_dp = (char *)&tty_map;

    if (ioctl(fileno(stdin), I_STR, &i_str) != 0 && errno != EINVAL) {
	perror("setmaps: clear_maps TCSMAP");
	return(BAD_RES);
    }
    if (verbose) {
	int both = TM_INPUT | TM_OUTPUT;

	if ((flags & both) == both)
	  fprintf(stderr, 
	      MSGSTR(RMBOTH, "%s: both mappings have been removed\n"),
	      arg0);
	else if ((flags & both) == TM_INPUT)
          fprintf(stderr, 
	      MSGSTR(RMINPUT, "%s: input mappings have been removed\n"),
	      arg0);
	else
	  fprintf(stderr, 
	      MSGSTR(RMOUTPUT, "%s: output mappings have been removed\n"),
	      arg0);
    }
    return(OK);
}


/* use_map - give a terminal access to an already-loaded map. */
int use_map(int verb, char *map_name, char *explicit_key, int level)
{
    long flags;
    char *key;
    struct tty_map tty_map;
    struct str_list strlist;
    int nmods;
    int size; 

    switch (verb) {
    case SET_INPUT:
    case SET_INPUT_FROM_PATH:
	flags = TM_INPUT;
	key = keyname(map_name, explicit_key, verb);
	break;
    case SET_OUTPUT:
    case SET_OUTPUT_FROM_PATH:
	flags = TM_OUTPUT;
	key = keyname(map_name, explicit_key, verb);
	break;
    default:
	fprintf(stderr,
		MSGSTR(INT2, "%s: internal error, bad verb in use_map: %d\n"),
		verb);
	break;
    }

    size = sizeof(struct tty_map);
    memset((void *)&tty_map, 0, (size_t)size);
    tty_map.tm_version = TTMAP_VERSION;
    strncpy(tty_map.tm_mapname, key, (size_t)TTMAP_NAMELEN);
    tty_map.tm_flags = flags | TM_USE;


    /* at this point will need to check for the nls module */
    /* if level != 0, then this is a second pass through use_map,
       and check_nls was done on the previous pass..
       */
    if ((level == 0) &&
	(check_nls() < 0)) {
	/* unable to verify the presence of the nls module or
	   unable to push the nls modules on the stream */
	fprintf(stderr, 
		MSGSTR(NLSDD, "%s: can't determine if NLS module on the stream\n"), arg0);
	return(BAD_RES);
    }

    if (verbose)
	fprintf(stderr, MSGSTR(ATTEMPT, "%s: attempting to use map %s\n"),
		arg0, key);

    if (x_ioctl(fileno(stdin), TCSMAP, &tty_map, size) == 0) {
	if (verbose)
	    fprintf(stderr,
		    MSGSTR(LOADED, "%s: %s already loaded, now using it\n"),
		    arg0, key);
	return(OK);			/* it worked, we're outta here */
    }

    /* 
     * ioctl failed. If errno is ENOENT, that means the requested map 
     * is not loaded in the kernel. Anything else is unexpected. If 
     * we're recursing on the second attempt to use a map, we'll 
     * bounce out thru here because we supposedly just loaded the map 
     * we're trying to use.
     */
    if ((level != 0) ||(errno != ENOENT)) {
	perror("setmaps: use_map TCSMAP");
	return(BAD_RES);
    }

    if (verbose)
	fprintf(stderr,
		MSGSTR(NOTLOADED, "%s: %s is not presently loaded\n"),
		arg0, key);

    /* Map isn't loaded, if we are root, we can continue, else error */
    if ((geteuid() != 0) && (Dump_map != TRUE)) {
      fprintf(stderr,
		MSGSTR(RLOAD, "%s: must be root to load a new map\n"), arg0);
      return(BAD_RES);
    }

    if (verbose)
	fprintf(stderr, MSGSTR(ATTEMPT2, "%s: will attempt to load %s\n"),
		arg0, key);

    /* 
     * The map isn't already in the kernel, and we have root access, 
     * so let's try loading it. If the load works, we recurse and try 
     * to select the map again. The level parm assures we only cycle 
     * once. If the second attempt to use the map fails after a 
     * successful load, there's probably something wrong in the kernel
     */
    if (load_map(verb, TM_LOAD, map_name, (int)key) != 0) {
	return(BAD_RES);
    }

    if (verbose)
	fprintf(stderr,
		MSGSTR(LOADOK, "%s: %s loaded ok, will try again to use it\n"),
		 arg0, key);

    /* The load worked, let's try using the map again. */
    return(use_map(verb, map_name, explicit_key, level + 1));
}

/* 
 * load_map - read file provided by user and store the data into ttmap 
 *            structure. ttmap struct(and related info) is defined in 
 *            <sys/ttmap.h>
 */
int load_map(int verb, int flags, char *map_name, int key)
{
    struct tty_map *tty_map;
    struct ttmap *ttmap;	  /* pointer to actual map */
    struct ttmap *ttmap_tmp;      /* pointer to map in tty_map */
    char *path;		          /* path of map file */
    int rc, n;
    int size;

    path = fullpath(verb, map_name);

    if (verbose)
      fprintf(stderr, MSGSTR(PATH, "%s: map pathname is: %s\n"), 
	      arg0, path);

    /* alloc space for map */
    ttmap =(struct ttmap *)malloc((size_t)TTMAP_MAXSIZE);
    if (ttmap == NULL_MAP) {
	fprintf(stderr,
		MSGSTR(NOMEM, "%s: can't allocate memory to build map\n"),
		arg0);
	return(BAD_RES);
    }
    /* make the map  - this will give us the actual size needed in 
       tm_len */
    n = makemap(ttmap, path, (char *)key);
    if (n <= 1) {			/* n = translations in map */
	fprintf(stderr, 
		MSGSTR(CREATE, "%s: unable to create map from %s\n"),
		arg0, path);
	return(BAD_RES);
    }
    /* create a tty_map struct with enough space to hold the map */
    size = sizeof(struct tty_map) + ttmap->tm_len;
    tty_map = (struct tty_map *)malloc((size_t)size);
    if (tty_map == NULL) {
      fprintf(stderr,
	      MSGSTR(NOMEM, "%s: can't allocate memory to build map\n"),
	      arg0);
      return(BAD_RES);
    }

    memset((void *)tty_map, 0, (size_t)size);
    tty_map->tm_version = TTMAP_VERSION;
    /* this indicates to nls that the ttmap is appended to the tty_map 
     * struct.
     */
    tty_map->tm_addr = (char *)0;

    strncpy(tty_map->tm_mapname, (char *)key, (size_t)TTMAP_NAMELEN);
    
    /* copy map to tty_map struct */
    ttmap_tmp = (struct ttmap *)(tty_map + 1);
    bcopy(ttmap, ttmap_tmp, ttmap->tm_len);
    
    if (Dump_map) 		/* do a debug map dump? */
      dump_map(ttmap); 
    
    /* We managed to grok the map, set map size in tty_map struct */
    tty_map->tm_len = ttmap->tm_len;
    tty_map->tm_flags = flags;

    /* Umm, excuse me Mr. Kernel. I have this map here, and... */
    rc = x_ioctl(fileno(stdin), TCSMAP, tty_map, size);
    if (rc < 0) {
	rc = BAD_RES;
	if (errno == EEXIST) {
	    fprintf(stderr,
		    MSGSTR(ALOADED, "%s: NLS map %s is already loaded\n"),
		    arg0, key);
	} else {
	    fprintf(stderr, "%s: ", arg0);
	    perror("TCSMAP");
	}
    } else {
	rc = OK;
    }

    free(ttmap);
    free(tty_map);
    return(rc);
}

/* 
 * show_map - determine if a map is already loaded. If so, print its 
 *            name. Otherwise, print "none installed". NB: The printf 
 *            statements in this function must remain in the same 
 *            format they are; they are parsed by stty. Stty should 
 *            really have a function similar to this one in it to 
 *            query the map state directly rather than calling this 
 *            program to do it.
 */
void show_map(int code)
{
    struct tty_map tty_map;		/* user ioctl structure */
    int rc;
    char *type =(code == TM_INPUT) ? "input" : "output";
    char buf [TTMAP_NAMELEN + 1];
    struct strioctl i_str;

    memset((void *)&tty_map, 0, (size_t)sizeof(struct tty_map));
    tty_map.tm_version = TTMAP_VERSION;
    tty_map.tm_flags = code;

    /* don't want the actual map, just the info about it */
    tty_map.tm_addr = NULL;
    tty_map.tm_len = TTMAP_MAXSIZE;

    /* for the stream framework, use the streams ioctl */
    i_str.ic_cmd = TCGMAP;
    i_str.ic_timout = 0;
    i_str.ic_len = sizeof(struct tty_map);
    i_str.ic_dp = (char *)&tty_map;
    rc = ioctl(0, I_STR, &i_str);               /* get map info */
    if (rc < 0) {
	if (errno == ENOENT || errno == EINVAL) {
	    printf(MSGSTR(NONEI, "%s map: none installed\n"), type);
	    return;
	} else {
	    fprintf(stderr, "%s: ", arg0);
	    perror("ioctl TCGMAP");
	}
    } else {
	strncpy(buf, tty_map.tm_mapname, (size_t)TTMAP_NAMELEN);
	buf [TTMAP_NAMELEN] = '\0';	/* make sure it's terminated */
	printf(MSGSTR(MAP, "%s map: %s\n"), type,(buf [0]) ? buf :
	       MSGSTR(UNKNOWN, "<UNKNOWN?>"));
	/*
	 * if the name is a null string, print <UNKNOWN?> instead.
	 * A map is in use on the tty, but doesn't have a name,
	 * this "can't happen".
	 */
    }
}

/* 
 * magic_key - Check a map key to see if it is a magic name which 
 *             really means to delete the mapping from the tty. The 
 *             prefered magic word (because I prefer it) is "NOMAP", 
 *             but this function also recognizes "." as a magic key, 
 *             for backward compatability. Also, if the pointer to the 
 *             key string is NULL we claim it's a magic word. We can't 
 *             use a null name as a map, so we assume that means 
 *             "don't use any map". It's unlikely we'll be called that 
 *             way, since getopt should protect us from missing parms, 
 *             but best to be paranoid.
 */
int magic_key(char *p)
{
    if (p == NULL)
	return(TRUE);
    if (strlen(p) == 0)
	return TRUE;
    if (strcmp(p, MAGIC_KEY1) == 0)
	return TRUE;
    if (strcmp(p, MAGIC_KEY2) == 0)
	return TRUE;
    return(FALSE);
}

/* 
 * keyname - extract the key name from the mapname given on the 
 *           command line. A suffix is added if approriate, if it 
 *           isn't already present.
 */
char *keyname(char *map_name, char *explicit_key, int verb)
{
    char *p, *q, *suffix;

    if (explicit_key != NULL) {
	return(explicit_key);
    }

    switch (verb) {
    case SET_INPUT:
	suffix = IN_SUFFIX;
	break;
    case SET_OUTPUT:
	suffix = OUT_SUFFIX;
	break;
    default:
	suffix = "";
	break;
    }


    q = strrchr(map_name, '/');		/* scan backwards for a slash */
    if (q == NULL) {
	q = map_name;			/* no slash, use full name */
    } else {
	q++;				/* point to char following slash */
    }
    p = (char *)malloc((size_t)(strlen(q) + strlen(suffix) + 1));
    strcpy(p, q);			/* copy it */
    if (strlen(suffix) == 0) {
	return(p);
    }
    q = strrchr(p, '.');		/* scan backwards for a dot */
    if (q != NULL) {
	if (strcmp(q, suffix) == 0) {	/* suffix already there? */
	    return(p);			/* yes */
	}
    }
    strcat(p, suffix);			/* append the suffix */
    return(p);				/* return what we think is the key */
}

/* 
 * fullpath - takes a map name and builds the full pathname of the 
 *            file we need to open to get the map. The pathname string 
 *            is built from <NLS directory> + <map name> + <suffix>. 
 *            The NLS directory may have been modified with a -d 
 *            option, if it doesn't end in / then one is appended. If 
 *            the map name already has the appropriate suffix on it 
 *            then it is not appended, otherwise .in or .out is added 
 *            as appropriate. Note that the wrong suffix will not be 
 *            caught, ie, foo.out as the name of an input map will 
 *            become foo.out.in. No suffix is appended for simple 
 *            loads, since we can't intuit what it should be, the user 
 *            will have to provide it, such as: setmaps -l foo.in. 
 *            Also, if we are given a full pathname, by the -I, -O or 
 *            -L options, we do nothing to the path name, it must be 
 *            the entire complete path including the suffix.
 */
char *fullpath(int verb, char *map_name)
{
    char *p;

    switch (verb) {
    case SET_INPUT_FROM_PATH:
    case SET_OUTPUT_FROM_PATH:
    case LOAD_ONLY_FROM_PATH:
	return(map_name);
    }

    /* space for <NLS dir> + <map name> + <slop for suffix, /, and '\0'> */
    p = (char *)malloc((size_t)(strlen(nls_dir) + strlen(map_name) + 10));
    strcpy(p, nls_dir);			/* copy in dir name */
    if (p [strlen(p) - 1] != '/')	/* and / if needed */
	strcat(p, "/");

    switch (verb) {
    case LOAD_ONLY:			/* no suffix append */
	strcat(p, map_name);		/* add map name */
	break;
    case SET_INPUT:
    case SET_OUTPUT:
	strcat(p, keyname(map_name, (char *)NULL, verb));
	break;
    default:
	fprintf(stderr, MSGSTR(FULL, "%s: internal error, fullpath botch\n"),
		arg0);
	return(NULL);
    }

    return(p);				/* return combined string */
}

/* 
 * usage - tell the user about the flags they can use. rint the usage 
 *         message to stderr. This is called when the -h option s 
 *         given, or when the user gives unrecognizeable arguments, or 
 *         when ome impossible combination of arguments is given. The 
 *         routine never eturns, it exits here.
 */

void usage()
{
    char *str;

    str = MSGSTR(USAGE, 
    "to use a map: setmaps [-v] -{i | o | t} mapname\n"
    " use one of -i, -o, or -t\n"
    " -i = use mapname.in as your input map\n"
    " -o = use mapname.out as your output map\n"
    " -t = two-way mapping, use both mapname.in and mapname.out\n"
    " the special name NOMAP will clear the corresponding mapping:\n"
    " setmaps -i NOMAP\n"
    "\n"
    "to clear all mappings on this terminal: setmaps -c\n"
    "\n"
    "to use a code set map: setmaps [-v] -s -{i | o} mapname\n"
    "\n"
    "-h = help, print this message(all other options ignored)\n"
    "-v = verbose\n"
    "\n"
    "Type: setmaps -v -h for advanced usage information\n");
    write(fileno(stdout), str, strlen(str));

    if ((getuid() == 0) || verbose) {
	str = MSGSTR(VUSAGE,
    "\n"
    "advanced usage(administrators only):\n"
    " -l mapname Load a map for later use(must include suffix if any)\n"
    " -k keyname Overrides mapname, normally filename is used\n"
    " -d dirpath Overrides directory where map files are assumed to be\n"
    " -I, -O, -L Same as -i, -o, -l, except full path is given by arg\n"
    " -r Force reload of a map even if already loaded\n"
    " -D Produce debug dump of map on stdout before loading\n"
    "(useful for making new maps, don't run as root until\n"
    " map is fully debugged to prevent actual load)\n"
    "\n"
    "All maps loaded must have unique names, use -k to eliminate conflicts\n"
    "Only -i, -o and -t implicitly add a suffix, other options specifying\n"
    "mapnames should include a suffix if appropriate.\n"
    "If a requested mapname is already loaded in the kernel, it will be\n"
    "used, even if the path info implies a different map. Administrator\n"
    "may force a reload with the -r option\n");
	write(fileno(stdout), str, strlen(str));
    }

    exit(BAD_RES);
}

/* 
 * From here on down is the code for reading in and parsing maps. This 
 * tuff really ought to be in a seperate .c file, so that other 
 * programs ould share the map making code. This was done for 
 * debugging with  "kernel simulator" to drive the tty code in a user 
 * program. This ode has been inserted back inline here at the end of 
 * setmaps.c because t was too painful at the time to go modify the 
 * library makefiles o build setmaps in two pieces.
 */
#define PAT 0				/* lets translation func know which */
#define REP 1				/* half of the pattern/replace pair */
					/* it's working on, sets([abc]) are */
					/* not recognized on replacement */

#define ESC 0x1b			/* ascii escape char code */

/* 
 * A map file contains a set of rules which define the translation of 
 * one or more characters to a replacement character or string. The 
 * syntax of each rule is pattern:newstring. Examples: \033b:ps -ef 
 * remaps 2nd func key on an ibm3161 terminal \@:\? remap '@' to '?'. 
 * Slash req'd for special char \x80:\x20 remap vt220 8-bit char to 
 * space a?a:a$2a ? matches any single byte; $n outputs nth char from 
 * pattern. Returns number of translations(rules) in map.
 */

/*
 * The skinny on the internal map structure.
 *
 * The ttmap struct contains a hash table and an array of translation rules.
 * The hash table is used to quickly locate the first applicable rule when
 * starting a new pattern. For instance, in the kernel, if the character 'a'
 * is received it is used as an index into the hash table to determine the
 * first translation rule to use(rule = ttmap->tm_hash ['a']). The 
 * translation rules in ttmap->tm_rule [rule] are then applied to the char 'a',
 * if it satisfies the pattern then the replacement pattern is inserted
 * in the data stream and the cycle starts over. If the pattern isn't
 * satisfied, then the next rule(ttmap->tm_rule [rule].next) is tried,
 * and so on, until a match is found. The last rule in every chain is the
 * default rule(?:$1) which will match anything. A sample map setup 
 * will probably be useful.(Assume a char set which ranges from a to d
 * with a = 0)
 *
 * ttmap:
 * tm_num_rules = 7
 * tm_default = 0
 * tm_first_wild = 4
 *
 * Rules list:
 * Hash table(p = tm_pattern, r = tm_replace)
 * ---------- --------------------------------
 * tm_hash ['a'] = 1 tm_rule [0]: p = ? r = $1 tm_next = -1
 * tm_hash ['b'] = 4 [1]: p = abc r = bca tm_next = 3
 * tm_hash ['c'] = 5 [2]: p = dab r = bad tm_next = 4
 * tm_hash ['d'] = 2 [3]: p = a??d r = $3a tm_next = 4
 * [4]: p = ?b r = c$1b tm_next = 6
 * [5]: p = c[ad] r = cc tm_next = 4
 * [6]: p = [b-c]a r = a$1 tm_next = 0
 *
 * Notice that all the chains converge on the first wildcard rule(4), which
 * ultimately winds up at the default rule(0)
 *
 * Whaddaya call a data structure like this, a hydra? :-)
 */

int makemap(struct ttmap *ttmap, char *path, char *mapname)
{
    FILE *file;
    char *p, *q;
    int no_ops = 0;			/* num no-op rules ignored */
    int bad_rules = 0;			/* num mangled rules ignored */
    char buf [256];			/* buffer to hold an input line */
    char pattern [TTMAP_PAT_SIZE * 2];	/* processed pattern(temp) */
    char replace [TTMAP_REP_SIZE * 2];	/* processed replacement(temp) */


    add_rule(NULL_MAP, (char *)NULL, (char *)NULL);	/* reset rule handler */
    memset((void *)ttmap, 0, (size_t)sizeof(struct ttmap)); /* clear map buffer */
    strcpy(ttmap->tm_mapname, mapname); /* copy in map name */
    ttmap->tm_len = sizeof(struct ttmap); /* account for struct */

    file = fopen(path,"r");		/* open the map file */
    if (file ==(FILE *)0) {		/* doesn't exist */
	perror("setmaps: makemap");
	return(BAD_RES);
    }

    /*
     * Read a line at a time from the map file into buf. A return value
     * of NULL from fgets indicates EOF.
     */
    while (fgets(buf, (int)sizeof(buf), file) != NULL) {
	int len;

	len = strlen(buf);
	if ((len == (sizeof(buf)-1)) && (buf[len - 1] != '\n')) {
	    /* line is too long, if not a comment then complain */
	    if ((buf[0] != '#') && (buf[0] != '\0')) {
	        fprintf(stderr,MSGSTR(LONGLN,
			"%s: input line too long, ignored.\n"),arg0);
		bad_rules++;
	    }
	    /* throw away the rest of the line */
	    while (fgets(buf, (int)sizeof(buf), file) != NULL) {
		len = strlen(buf);
		if ((len != (sizeof(buf)-1)) || (buf[len - 1] == '\n')) 
		    break;
	    }
	    continue;
	}
	buf [len - 1] = '\0';		/* snuff the \n */

	p = q = buf;			/* point at line just read */
	if ((*p == '#') ||(*p == '\0'))
	    continue;			/* comment or null line */

	/* find second half, following unescaped ':' */
	if ((q = find_replace(p)) == NULL) {
	    bad_rules++;
	    continue;
	}

	/*
	 * Translate each half of the pattern/replacement pair from
	 * external ascii to internal strings, converting octal and
	 * hex escapes and such. If translation fails, it kicks out
	 * a message and returns BAD_RES. In that case we ignore this
	 * rule and go get the next one.
	 */
	if (translate(p, pattern, sizeof(pattern), PAT) == BAD_RES ||
	    translate(q, replace, sizeof(replace), REP) == BAD_RES) {
	    bad_rules++;
	    continue;
	}

	/* does this mapping do anything? */
	if (no_op(pattern, replace)) {
	    no_ops++;			/* nope, count it and toss it */
	    if (verbose) {
		fprintf(stderr,
			MSGSTR(IGNORED, "%s: null mapping ignored, pat=%s, rep=%s\n"),
			arg0, p, q);
	    }
	    continue;
	}

	/*
	 * So far so good, add this rule to the map. If it can't
	 * be added for some reason, we complain and press on
	 */
	if (add_rule(ttmap, pattern, replace) == BAD_RES) {
	    fprintf(stderr,
		    MSGSTR(CANTADD, "%s: can't add rule, pat=%s, rep=%s\n"),
		    arg0, p, q);
	    bad_rules++;
	    continue;
	}
    }					/* end while (fgets()) */

    fclose(file);			/* finished with the file */
    if (ttmap->tm_num_rules < 2) {	/* anything besides default? */
	fprintf(stderr, MSGSTR(MAKEMAP, "%s: makemap, no rules loaded\n"),
		arg0);
	return(BAD_RES);
    }

    /* If verbose, and any no_op rules were ignored, say so */
    if (verbose &&(no_ops != 0)) {
	fprintf(stderr, MSGSTR(NULLMAP, "%s: %d null mappings ignored\n"),
		arg0, no_ops);
    }
    /* If verbose, and any rules were rejected, syay so */
    if (verbose &&(bad_rules != 0)) {
	fprintf(stderr, MSGSTR(BADRULE, "%s: %d bad rules rejected\n"),
		arg0, bad_rules);
    }

    return(ttmap->tm_num_rules);	/* finished */
}


/* 
 * Add a rule to the map. The rule is stored as two seperate strings, 
 * one is the pattern to be matched in the input stream, the other is 
 * the eplacement string to substitute in its place. ote: References 
 * to "pointers" to rules in the comments below refer to ndexes of 
 * ttrule structs in the tm_rule array in the ttmap struct. verything 
 * in the ttmap struct is in a contiguous block of storage so hat it 
 * can be copied into and out of the kernel as a block.
 */
int add_rule(struct ttmap *ttmap, char *pat, char *rep)
{
    static int first_time = TRUE;	/* first call? */
    static int first_wildcard = 0;	/* index of first wildcard */
    static int new_rule = 1;		/* index where next rule goes */
    int i;				/* scratch */

    if (ttmap == NULL_MAP) {		/* re-init for next map */
	first_time = TRUE;		/* reset, for first rule */
	first_wildcard = 0;		/* reset indexes */
	new_rule = 1;
	return(OK);
    }

    /*
     * If this is the first call, setup the default rule and initialize
     * the hash table so that all entries point to the default.
     */
    if (first_time == TRUE) {
	ttmap->tm_rule [0].tm_next = -1; /* default rule */
	strcpy(ttmap->tm_rule [0].tm_pattern, "?");
	strcpy(ttmap->tm_rule [0].tm_replace, "$1");
	ttmap->tm_default = 0;		/* default is in slot 0 */
	ttmap->tm_num_rules = 1;	/* one rule in the map */
	first_time = FALSE;		/* initial setup is done */
	for (i = 0; i < 256; i++) {	/* setup hash table */
	    ttmap->tm_hash [i] = 0;	/* point all to default */
	}
    }

    /* make sure the rule strings will fit in the rule struct */
    if (strlen(pat) > TTMAP_PAT_SIZE || strlen(rep) > TTMAP_REP_SIZE) {
	fprintf(stderr,
		MSGSTR(TOOLONG, "%s: mapping too long, pat=%s, replace=%s\n"),
		arg0, pat, rep);
	return(BAD_RES);
    }

    /* put the translation info into the next available rule slot */
    ttmap->tm_rule [new_rule].tm_next = 0;
    strcpy(ttmap->tm_rule [new_rule].tm_pattern, pat);
    strcpy(ttmap->tm_rule [new_rule].tm_replace, rep);

    if (is_wild(pat)) {			/* is this a wildcard rule? */
	if (first_wildcard == 0) {	/* yes, first one? */
	    /*
	     * Yes, wildcards should be searched before reaching
	     * the default rule, so when we see the first one we
	     * we scan the hash list and change any entries
	     * which are pointing at the default to point to
	     * this wildcard rule. This means that this wildcard
	     * rule will be the first one tried for any character
	     * which does not have an explicit rule. After
	     * updating the hash list, we then scan all the loaded
	     * rules and patch the next pointers of any rules
	     * which point at the default rule to point at this
	     * wildcard. This has the effect of chaining the list
	     * of all wildcards to the end of every explicit rule
	     * chain. The default rule is at the end of the
	     * wildcard chain, and therefore at the end of every
	     * chain.
	     */
	    ttmap->tm_first_wild = first_wildcard = new_rule;
	    for (i = 0; i < 256; i++) { /* patch hash */
		if (ttmap->tm_hash [i] == 0) {
		    ttmap->tm_hash [i] = new_rule;
		}
	    }
	    for (i = 1; i < new_rule; i++) { /* patch rules */
		if (ttmap->tm_rule [i].tm_next == 0) {
		    ttmap->tm_rule [i].tm_next = new_rule;
		}
	    }
	} else {
	    /*
	     * This is a wildcard, other wildcards already present,
	     * chain this wildcard to the end of the wildcard
	     * list. This effectively inserts it at the end of
	     * the wildcard list just before the default rule
	     */
	    chain_to_wild(ttmap, first_wildcard, new_rule);
	}
    } else {
	/*
	 * Not a wildcard rule. This rule begins with a specific,
	 * quantifiable character. The first character of this
	 * explicit rule is used as a hash key. This character
	 * is used as an index into the hash table and the entry at that
	 * location will point to the head of the chain of all rules
	 * beginning with this char. All the explicit rule chains
	 * link to the first wildcard rule, which is the head of the
	 * wildcard rule chain, which has the default rule as its
	 * last entry(the default rule is a wildcard, and may be the
	 * only entry in the wildcard chain). This new explicit rule
	 * is chained to the end of this explict rule chain, just
	 * before the first wildcard.
	 */
	chain_to_hash(ttmap, hash_char(pat), new_rule,
		      first_wildcard);
    }
 
    new_rule++;				/* move to next rule slot */
    ttmap->tm_num_rules++;		/* another rule was added */
    ttmap->tm_len += sizeof(struct ttrule); /* adjust total map length */
    return(OK);				/* all according to plan */
}


/*
 * Add a new explicit rule to the end of a hash chain. Each explicit chain is
 * pointed at by an entry in the hash table, indexed by the first character
 * in the rule pattern string. The parm hash is the hash table index that
 * this new rule should be chained onto, new is the index of the new rule
 * being chained in, and end is the index of the rule following the last 
 * rule in this chain(The first wildcard rule, in other words. In this
 * routine end means "when you see this rule, that's the end of your search").
 * The new rule is linked into the chain pointed at by the hash entry, just
 * before the rule pointed at by end.
 */

void chain_to_hash(struct ttmap *ttmap, int hash, int new, int end)
{
    int i;

    /*
     * Is the hash chain empty? If the hash entry for this hash key
     * points at the end rule(first wildcard), then there is no explicit
     * chain present for this slot. Insert the index of this rule in the
     * hash table, then have this new rule point at what the hash entry
     * was pointing at.
     */
    if (ttmap->tm_hash [hash] == end) {
	ttmap->tm_hash [hash] = new;
	ttmap->tm_rule [new].tm_next = end;
	return;
    }

    /*
     * Already have one or more rules in the explicit chain which hangs
     * off this hash entry. Scan the chain until we find the end of the
     * explicit rules, then link the new guy in after the last explicit
     * rule in the chain and before the first wildcard
     */
    i = ttmap->tm_hash [hash];		/* get chain head from hash table */
    while (ttmap->tm_rule [i].tm_next != end) { /* watch for end */
	i = ttmap->tm_rule [i].tm_next;
    }
    ttmap->tm_rule [new].tm_next = end; /* link new guy to first wild */
    ttmap->tm_rule [i].tm_next = new;	/* link last explicit to new */
}


/*
 * Chain a new wildcard rule onto the end of the wildcard rule chain. The
 * idea here is similar to the previous routine, but is somewhat simpler
 * because the hash table is not involved. We simply start searhing at the
 * first wildcard and scan until we find a rule pointing at the default rule
 *(default is always in slot zero). We then link the new rule into the
 * chain just before the default.
 */
void chain_to_wild(struct ttmap *ttmap, int first, int new)
{
    int i = first;

    while (ttmap->tm_rule [i].tm_next != 0)
	i = ttmap->tm_rule [i].tm_next;
    ttmap->tm_rule [new].tm_next = 0;	/* point new guy at default */
    ttmap->tm_rule [i].tm_next = new;	/* last guy points at me now */
}


/*
 * Determine the character which represents the first character of a pattern
 * and which therefore is used as the hash key for this rule. There are
 * really only two cases to worry about here, when the rule starts with '@'
 * and when the first char is escaped with '\'. The '@' char is a state
 * requirement and is not part of the match pattern, so we skip it and resume
 * looking with the third char. For escapes, there is a special case for \0
 * which represents null(rules are stored as strings and can't contain a
 * real null). Other escapes have the real char following them. There won't
 * be any \x12 or \123 type escapes here, those have all been processed
 * already.
 */
int hash_char(char *p)
{
    if (*p == '@')
	p += 2;				/* skip @n */
    if (*p == '\\') {
	if (*++p == '0')		/* special case */
	    return(0);			/* return the value zero */
    }
    return((int) *p);			/* return the char itself */
}


/*
 * Translate a mapping string from the external ascii form to the internal
 * form used by the kernel mapping code. This basically boils down to
 * interpreting the escapes which represent other things, such as \xf2 and
 * \n. The which parm let's this routine know if it is processing the match
 * pattern or the replacement string, because the [ meta char should not be
 * interpreted in the replacement string since it has no special meaning
 * there.
 */
int translate(char *in, char *out, int bufsize, int which)
{
    char *p = in, *q = out;		/* working pointers */
    char *fence = out + bufsize;	/* don't store beyond this address */
    int c;				/* scratch */


    while ((q < fence) && *p) {		/* while more input and space left */
	switch (*p) {			/* let's see here... */
	case '\\':			/* next char is escaped */
	    p++;
	    c = xlate_esc_char(p, &p);
	    if (is_meta(c, which) ||(c == '\0') ||(c == '\\')) {
		*q++ = '\\';		/* needs escape in output */
		if (c == '\0')
		    c = '0';		/* special case for NULL */
	    }
	    *q++ = c;
	    break;

	case '$':			/* copy char from source spec */
	    if (which == REP) {		/* only applies to rep string */
		c = *(p + 1);		/* verify legality */
		if (c < '1' || c > '9') {
		    fprintf(stderr,
			    MSGSTR(XLATE, "%s: xlate, illegal $ spec(%c%c) in replacement: %s\n"),
			    arg0, *p, *(p+1), in);
		    return(BAD_RES);
		}
	    }
	    *q++ = *p++;		/* copy out $, second char will be */
	    break;			/* handled by default case next time */

	case '[':			/* begin set, [abc] or [a-f] */
	    /* only do this case on pattern string */
	    if (which == PAT) {
		if (xlate_char_set(in, &p, &q, fence) == BAD_RES)
		    return(BAD_RES);
		/* fall out */
	    }
	    /* else fall through */

	default:
	    *q++ = *p++;		/* maps to itself, nothing special */
	    break;
	}
    }

    if (q == fence) {
	fprintf(stderr,
		MSGSTR(OVERRAN, "%s: xlate, overran translation buffer: %s\n"),
		arg0, in);
	return(BAD_RES);
    }
    *q = '\0';				/* terminate output string */
    return(OK);
}


/*
 * Handle the various types of backslash escapes which can be present in the
 * input string. The new parm is where scanning should resume. This routine
 * uses a library function called strtol() which actually scans the hex and
 * octal escapes. This allows this routine to handle short numbers properly
 * so that a string like this:
 * a\41c
 * will be properly parsed as:
 * a!c(octal 041 is ascii !)
 * This routine does protect itself from long strings of digits though, so:
 * a\041345
 * is parsed as:
 * a!345
 */
int xlate_esc_char(char *p, char **new)
{
    int c, tmp;				/* scratch */
    char *q;				/* scratch */
    char *newp = p + 1;			/* default resume position */

    switch (*p) {			/* whadda we got here... */
    case '\0':				/* a \ was at the end of the string */
	c = '\\';
	break;
    case 'n':				/* new line */
	c = '\n';
	break;
    case 't':				/* tab */
	c = '\t';
	break;
    case 'f':				/* form feed */
	c = '\f';
	break;
    case 'r':				/* carriage return */
	c = '\r';
	break;
    case 'E':				/* ascii escape ala termcap/terminfo */
	c = ESC;
	break;

    case 'x':				/* two digit hex number */
    case 'X':
	p++;				/* point at first digit */
	q = p + 2;			/* pos of char after second digit */
	tmp = *q;			/* save that char */
	*q = '\0';			/* temporarily shorten the string */
	c = strtol(p, &newp, 16);	/* interpret the string */
	*q = tmp;			/* restore saved char */
	break;

    case '0':				/* three digit octal number */
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
	q = p + 3;			/* pos of char following third digit */
	tmp = *q;			/* save it */
	*q = '\0';			/* end the string there */
	c = strtol(p, &newp, 8);	/* interpret the string */
	*q = tmp;			/* restore the saved char */
	break;

    default:				/* hmm, \x, don't know any special */
	c = *p;				/* meaning for x, just return it */
	break;
    }

    if (new != NULL)			/* if they gave us a pointer to a */
	*new = newp;			/* pointer send back new p value */
    return(c);
}


/*
 * Interpret a set of chars of the form [abcd] or [a-d]. These are just like
 * in regular expresions, but we don't support "not" sets. We do, however
 * support \ escapes within the []s, such as [\05-a], or [\E\n\r].
 * The parm in is a pointer to the beginning of the pattern(for error
 * messages), pp is a pointer to a pointer to the first char of the set
 *(the '['), qp is a pointer to a pointer to the current output position,
 * fence is the upper boundary of the output buffer. We need pointers to
 * pointers here because we're handling an unknown number of input chars and
 * are adding to the output so we need to adjust the caller's state.
 */
int xlate_char_set(char *in, char **pp, char **qp, char *fence)
{
    int c;
    char *p = *pp, *q = *qp;

    *q++ = *p++;			/* copy out the '[' */

    while (*p != ']') {
	if (*p == '\0') {		/* premature end of string */
	    fprintf(stderr,
		    MSGSTR(NOCLOSE, "%s: xlate, no closing ']': %s\n"),
		    arg0, in);
	    return(BAD_RES);
	}
	if ((*p == '-') &&(*(p + 1) == ']')) { /* bad range spec */
	    fprintf(stderr,
		    MSGSTR(MISSEND, "%s: xlate, missing end-of-range: %s\n"),
		    arg0, in);
	    return(BAD_RES);
	}
	if (*p == '\\') {		/* an escaped char */
	    p++;
	    c = xlate_esc_char(p, &p);
	    if (is_meta(c, PAT) ||(c == '\0') ||(c == '\\')) {
		*q++ = '\\';
		if (c == '\0')
		    c = '0';		/* special case for NULL */
	    }
	    *q++ = c;
	    continue;
	}
 
	*q++ = *p++;			/* copy the char to output */
	if (q >= fence) {		/* watch for end of buf */
	    fprintf(stderr,
		    MSGSTR(REPBUF, "%s: xlate, overran replacement buf: %s\n"),
		    arg0, in);
	    return(BAD_RES);
	}
    }
    *pp = p;
    *qp = q;
    return(OK);
}

/*
 * Check to see if a pattern/replacement pair really do anything. If not
 * then indicate so. With no explict rules, chars map to themselves anyway,
 * there's no need to fill up the search space with unnecessary rules,
 * especially since the mapping is done in the kernel at interrupt level.
 */
int no_op(char *p, char *q)
{
    if (strcmp(p, q) != 0)		/* doesn't match exactly */
	return(FALSE);
    if (has_meta(p, PAT))
	return(FALSE);			/* may be a meta-char in pattern */
    if (has_meta(q, REP))
	return(FALSE);			/* may be a meta-char in replacement */
    return(TRUE);			/* maps to self */
}


/*
 * Find the replacement string in the input string. We scan the input
 * string looking for an unescaped colon, when we find it we replace it
 * with a null and return the address of the char following the colon
 * which is the beginning of the replacement string. These are each
 * translated to remove most escapes and then stored as two sepereate
 * strings in the actual map.
 */
char *find_replace(char *p)
{
    char *q = p;

    while (TRUE) {
	q = strchr(q, ':');		/* look for a ':' */
	if ((q == NULL) ||(q == p)) {
	    /* something's fishy here */
	    fprintf(stderr,
		    MSGSTR(MAL, "%s: malformed NLS pattern: %s\n"),
		    arg0, p);
	    return(NULL);
	}
	/* is it escaped? */
	if ((*(q - 1) == '\\') &&(*(q - 2) != '\\')) {
	    q++;			/* yep, keep looking */
	    continue;
	}
	/*
	 * Found the pattern/replacement seperator, replace
	 * the colon with a null. Now we have two strings
	 * in the buffer, p points at the pattern, q points
	 * at the replacement string.
	 */
	*q++ = '\0';
	return(q);			/* all done */
    }
}


/* Is this a meta character? */
int is_meta(int c, int which)
{
    if (c == '@')
	return(TRUE);
    if (which == PAT) {
	if (c == '?')
	    return(TRUE);
	if (c == '[')
	    return(TRUE);
	if (c == ']')
	    return(TRUE);
    } else {
	if (c == '$')
	    return(TRUE);
    }
    return(FALSE);
}

/* Are there meta characters in this string? */

int has_meta(char *s, int which)
{
    char *p = s;

    while (*p) {
	if (is_meta((int)(*p), which)) {
	    if (p == s)
		return(TRUE);		/* first char, no \ */
	    if (*(p - 1) == '\\') {	/* \ preceding? */
		if (*(p - 2) == '\\') { /* was it \\ ? */
		    return(TRUE);	/* yes, not escaped */
		}
		/* meta char was escaped, keep going */
	    }
	}
	p++;				/* next char */
    }
    return(FALSE);			/* I don't see no meta chars */
}

/*
 * Is this pattern a wildcard? Only the first char counts. Meta chars
 * imbedded in the middle of a pattern don't make the pattern a wildcard,
 * only if it doesn't start with a quantifiable char 
 */

int is_wild(char *s)
{
    if (*s == '@')			/* skip state spec */
	s += 2;
    if ((*s == '?') ||(*s == '['))	/* first char multi-matcher? */
	return(TRUE);
    return(FALSE);
}
 

/*
 * Debug code
 * Given a char, return an ascii string representing it. Printables return
 * a one byte string of themselves. Non-printables return the hex
 * representation enclosed in <>. This is only used by the dump_map routine
 * for printing the hash table and the rule strings.
 */

char *asciify(unsigned char c)
{
    static char buf [10];

    if ((c >= ' ') &&(c <= '~')) {	/* in printable range */
	buf [0] = c;
	buf [1] = '\0';
	return(buf);
    }
    sprintf(buf, "<%02x>",(unsigned int)c);
    return(buf);
}


/*
 * Debug code.
 * Given a string and a pointer to a buffer, make new string using asciify
 * above to convert non-printables to printable representions. This is only
 * used by dump_map for printing the rule strings.
 */

char *asciistr(unsigned char *p, char *buf)
{
    buf [0] = '\0';
    while (*p) {
	strcat(buf, asciify(*p++));
    }
    return(buf);
}

/*
 * Debug code.
 * Dump the map to stdout. This is invoked by the -D option. This is 
 * for debug.
 */

void dump_map(struct ttmap *ttmap)
{
    int i;
    char buf1 [32], buf2 [32];

    printf(MSGSTR(NAME, "\nMap dump, name: %s\n"), ttmap->tm_mapname);
    printf(MSGSTR(TMAP, " total map length:\t%d\n"), ttmap->tm_len);
    printf(MSGSTR(RULES, " rules in map:\t%d\n"), ttmap->tm_num_rules);
    printf(MSGSTR(DEF, " default rule:\t%d\n"), ttmap->tm_default);
    printf(MSGSTR(WILD, " first wildcard:\t%d\n\n"), ttmap->tm_first_wild);
    printf(MSGSTR(HASH, "Hash table:\n"));
    for (i = 0; i< 256; i++) {
	printf(" %d('%s')\t%d\n", i, asciify((unsigned char)i), 
		ttmap->tm_hash [i]);
    }
    printf(MSGSTR(RULE2, "\nRules:\n"));
    for (i = 0; i < ttmap->tm_num_rules; i++) {
	printf(MSGSTR(STATS, " %d\tpat=%s\trep=%s\tnext=%d\n"), i,
	       asciistr(ttmap->tm_rule [i].tm_pattern, buf1),
	       asciistr(ttmap->tm_rule [i].tm_replace, buf2),
	       ttmap->tm_rule [i].tm_next);
    }
    fflush(stdout);
}

int x_ioctl(int fd, int cmd, struct tty_map *tty_map, int size)
{
    int err;
    struct strioctl i_str;

    i_str.ic_cmd = cmd;
    i_str.ic_timout = 0;
    i_str.ic_len = size;
    i_str.ic_dp = (char *)tty_map;


    if ((err = ioctl(fd, I_STR, &i_str)) && errno == EINVAL) {
      /* ioctl may fail due to lack of nls module on the stream; 
       * check for presence of nls module and retry
       */
      if (check_nls() < 0) {
	fprintf(stderr,
		MSGSTR(NLSDD,
		       "%s: can't determine if NLS module on the stream\n"), 
		arg0);
	exit(1);
      }
      err = ioctl(fd, I_STR, &i_str);
    }
     return err;
}


/* 
 * csmap_path - takes a code set map name and builds the full pathname of the 
 *              file we need to open to get the map. The pathname string 
 *              is built from <CSMAP directory> + <map name>. 
 *              If we are given a full pathname, by the -I or -O
 *              options, we do nothing to the path name because it is
 *              the entire path.
 */
char *csmap_path(int verb, char *map_name)
{
    char *p;

    switch (verb) {
    case SET_INPUT_FROM_PATH:
    case SET_OUTPUT_FROM_PATH:
        return(map_name);
    }

    /*
     * Allocate space for <CSMAP dir> + <map name>, /, and '\0'.
     */
    p = (char *)malloc((size_t)(strlen(csmap_dir) + strlen(map_name) + 2));
    /*
     * Copy in dir name and / if needed.
     */
    strcpy(p, csmap_dir);
    if (p [strlen(p) - 1] != '/')
        strcat(p, "/");

    /*
     * Copy in mapname.
     */
    switch (verb) {
    case SET_INPUT:
    case SET_OUTPUT:
        strcat(p, map_name);
        break;
    default:
        fprintf(stderr,
                MSGSTR(CSMAPINTL, "%s: internal error, csmap_path botch\n"),
                arg0);
        return(NULL);
    }
    /*
     * Return combined string.
     */
    return(p);
}


/* 
 * Function that checks the stream to see if the nls module
 * had been pushed.  If it has been pushed, just return 0.
 * If it has not been pushed, attempt to push.  If successful
 * return 0, otherwise return -1.
 * 
 */
int
check_nls()
{
  struct str_list strlist;
  int nmods;
  char buf1[FMNAMESZ+1];
  char buf2[FMNAMESZ+1];
  unsigned size;
  struct str_mlist *p;
  int ii;
  struct termios ldt;
  int res = OK;
  int fd = fileno(stdin);
  struct cfg_load cfg_l;
  struct cfg_kmod cfg;
  struct nlsdd_init nlsdd_init;
  mid_t res2;
  /* where to find the nls module.  there are two paths because sysconfig
   * will view as different the link in /etc/drivers and the actual file
   * in /usr/lib/drivers.  this is a problem since we can't be sure
   * from where the file was actually loaded, we can only check the
   * the two most likely places. 
   */
  char *nls_path1 = "/etc/drivers/nls";         
  char *nls_path2 = "/usr/lib/drivers/nls";

  /* get list of modules in the stream */
  if ((nmods = ioctl(fd, I_LIST, (char *)0)) < 0) {
    fprintf(stderr, 
	    MSGSTR(NLSDD, "%s: can't determine if NLS module on the stream\n"), arg0);
    return (BAD_RES);
  }

  size = nmods * sizeof(struct str_mlist);
  strlist.sl_nmods = nmods;
  strlist.sl_modlist = (struct str_mlist *)malloc(size);
  memset(strlist.sl_modlist, '\0', (int)size);
  if ((nmods = ioctl(fileno(stdin), I_LIST, (char *)&strlist)) < 0) {
    fprintf(stderr, 
	    MSGSTR(NLSDD, "%s: can't determine if NLS module on the stream\n"), arg0);
       return (BAD_RES);
  }
  else if (nmods != strlist.sl_nmods) {
    fprintf(stderr, 
	    MSGSTR(NLSDD, "%s: can't determine if NLS module on the stream\n"), arg0);
    return (BAD_RES);
  }

  /* search for the nls module  - assume the name is 'nls' */
  /* already have the list.... if the nls module is present, */
  /* it will be module just before the driver, i.e., the next */
  /* to last item in the list... */

  p = strlist.sl_modlist;
  nmods = strlist.sl_nmods;
  p += nmods - 2;

  if (strcmp(p->l_name, NLS_MODULE) == 0) {
    return (OK);
  }

  /* nls module not on the stream, put it there - just above
     the driver... must be root to do this
   */
  /* we must cfg in all cases because we cannot tell if the
     module has been configed even if it has been loaded...
     */
  cfg_l.libpath = (char *) NULL; 
  cfg_l.path = nls_path1;
  if (sysconfig(SYS_QUERYLOAD, (void *)&cfg_l, 
		      (int)sizeof(struct cfg_load)) < 0) {
	  perror("setmaps ");
	  return(BAD_RES);
        }
  if (cfg_l.kmid == (mid_t) NULL) {
    cfg_l.path = nls_path2;
    if (sysconfig(SYS_QUERYLOAD, (void *)&cfg_l, 
		      (int)sizeof(struct cfg_load)) < 0) {
	  perror("setmaps ");
	  return(BAD_RES);
        }
  }
  if (cfg_l.kmid == (mid_t) NULL) {
    /* not loaded; use /etc/drivers/nls for the nls path as this is the 
     * default used by loadext.
     */
    cfg_l.path = nls_path1;
    if (sysconfig(SYS_SINGLELOAD, (void *)&cfg_l, 
		  (int)sizeof(struct cfg_load)) < 0) {
      perror("setmaps ");
      fprintf(stderr, 
	      MSGSTR(NLSLOAD, "%s: can't load NLS module into the kernel\n"), arg0);
      return(BAD_RES);
    }
  }

  cfg.kmid = cfg_l.kmid;
  cfg.cmd = CFG_INIT; 
  nlsdd_init.which_dds = NLS_DDS;
  cfg.mdiptr = (caddr_t)&nlsdd_init;
  cfg.mdilen = sizeof(struct nlsdd_init);
  if ((sysconfig(SYS_CFGKMOD, (void *)&cfg,
		 (int)sizeof(struct cfg_kmod))) < 0) {
    fprintf(stderr, 
	    MSGSTR(NLSCFG, "%s: can't configure NLS module\n"),
	    arg0);
    return (BAD_RES);
  }

  /* now pop them all, push str_nls, and push them all */

  /* before popping off the modules, save the state of ldterm
     so it can be restored after we push it back on the stream
   */

  if (tcgetattr(fd, &ldt) == -1) {
    fprintf(stderr, "setmaps: ");
    perror("tcgetattr");
    return(BAD_RES);
  }

  nmods = strlist.sl_nmods - 1; /* decr to prevent trying to pop the 
				   driver */

  p = strlist.sl_modlist;
  for (ii=0;ii < nmods; ii++) {
     if (ioctl(fd, I_POP, 0) < 0) {
       /* a failure to pop will be a disaster, if the failure occurs
	  after some pops have already occurred - do we attmpt to
	  clean up ???
	*/
       fprintf(stderr, 
	       MSGSTR(NLSPOP, "%s: unable to pop module from stream\n"),
	       arg0);
       return (BAD_RES);
     }
  }

  /* push the nls module */
  if (ioctl(fd, I_PUSH, NLS_MODULE) < 0) {
    fprintf(stderr, 
	    MSGSTR(NLSPUSH, "%s: unable to push module %s on stream\n"),
	    arg0, NLS_MODULE);
    res = BAD_RES;
  }
  
  /* now push modules back on the stream - remember these must
     be pushed on in reverse order */
  nmods = strlist.sl_nmods - 1;
  p = strlist.sl_modlist;
  for (ii=nmods-1;ii >= 0; ii--) {
     if (ioctl(fd, I_PUSH, (p+ii)->l_name) < 0) {
       fprintf(stderr, 
	       MSGSTR(NLSPUSH, "%s: unable to push module %s on stream\n"), 
	       arg0, (p+ii)->l_name);
       return (BAD_RES);
     }
  }
  if (tcsetattr(0, TCSANOW, &ldt) == -1) {
    fprintf(stderr, "setmaps: ");
    perror("setattr");
    return (BAD_RES);
  }
   return (res); 
}


