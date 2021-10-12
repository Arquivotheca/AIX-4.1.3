static char sccsid[] = "@(#)80  1.2  src/bos/usr/bin/bterm/defaults.c, libbidi, bos411, 9428A410j 10/5/93 17:58:58";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: BDExtractResources
 *              BDInitArabicMap
 *              BDInitMap
 *              BDLoadMaps
 *              BDSegInit
 *              BDSetBidiAtts
 *              GetBidiPath
 *              TrueFalse
 *              convert_to_lower
 *              getHomeDir
 *              mergeDatabases
 *              parseBtermCommand
 *              set_bterm_defaults
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


# define	APP_DEFAULTS	"/app-defaults/"
# define	BIDI_DEFAULTS	".Bidi-defaults"
# define	BTERM		"bterm"

#include	"X/Xlib.h"
#include	"X/Xresource.h"

#include	<stdio.h>
#include	<malloc.h>
#include	<termio.h>
#include	<ctype.h>
#include	<pwd.h>

#include	"global.h"
#include	"trace.h"

/* names of conversion maps */
# define	KBRD		"kbrd.MAP"
# define	SYMETRIC        "symetric.MAP"
# define	MAPS		"/maps/"

/* array to store character conversion map         */
unsigned char KBRD_MAP[256][3];		/* nl layer kbd  */ 
unsigned char SYMETRIC_MAP[256];	/* symetric and graphics conversion */
/* structure to store BDSeg defaults */
BDSeg_st *BDSegDefaults;

static	char	prog[256];
static	char	termname[20];
static	XrmDatabase	commandlineDB = NULL;
static	XrmDatabase	rDB = NULL;
static	char *AppName;
char    *bidipath;

/* command line options */
static	XrmOptionDescRec argTable[] = {
{"-help",     ".help",       XrmoptionNoArg,	(caddr_t) "on"},
{"-keywords", ".keywords",   XrmoptionNoArg,	(caddr_t) "on"},
{"-nobidi",   ".nobidi",     XrmoptionNoArg,	(caddr_t) "on"},
{"-symmetric",".symmetric",  XrmoptionNoArg,	(caddr_t) "on"},
{"+symmetric",".symmetric",  XrmoptionNoArg,	(caddr_t) "off"},
{"-autopush", ".autopush",   XrmoptionNoArg,	(caddr_t) "on"},
{"+autopush", ".autopush",   XrmoptionNoArg,	(caddr_t) "off"},
{"-orient",   ".orientation",XrmoptionSepArg,	(caddr_t) NULL},
{"-text",     ".textType",   XrmoptionSepArg,	(caddr_t) NULL},
{"-nss",      ".numShape",   XrmoptionSepArg,	(caddr_t) NULL},
{"-csd",      ".charShape",  XrmoptionSepArg,	(caddr_t) NULL},
{"-maps",     ".maps",	     XrmoptionSepArg,	(caddr_t) NULL},
{"-tail",     ".expandTail", XrmoptionNoArg,	(caddr_t) "on"},
{"+tail",     ".expandTail", XrmoptionNoArg,	(caddr_t) "off"},
{"-nonulls",  ".noNulls",    XrmoptionNoArg,	(caddr_t) "on"},
{"-nulls",    ".noNulls",    XrmoptionNoArg,	(caddr_t) "off"},
};

#define  argTableEntries	(sizeof(argTable)/sizeof(argTable[0]))

struct _resource
{
	char	*resource_name;
 	char	*resource_class;
  	char	*resource_val;
  	char	*default_val;
};
typedef	struct	_resource	resource_st;

/* order of entries is important, don't change */
static	resource_st resourceTable[] = {
{ "maps",	"Maps",		NULL,	NULL},
{ "fScrRev",	"FScrRev",	NULL,	"on"},
{ "fLTR",	"FLTR",		NULL,	"on"},
{ "fRTL",	"FRTL",		NULL,	"on"},
{ "fAutoPush",	"FAutoPush",	NULL,	"on"},
{ "fPush",	"FPush",	NULL,	"on"},
{ "fEndPush",	"FEndPush",	NULL,	"on"},
{ "fASD",	"FASD",		NULL,	"on"},
{ "fShapeIS",	"FShapeIS",	NULL,	"on"},
{ "fShapeIN",	"FShapeIN",	NULL,	"on"},
{ "fShapeM",	"FShapeM",	NULL,	"on"},
{ "fShapeF",	"FShapeF",	NULL,	"on"},
{ "fShapeP",	"FShapeF",	NULL,	"on"},
{ "textType",	"TextType",	NULL,	"implicit"},
{ "symmetric",	"Symmetric",	NULL,	"on"},
{ "orientation","Orientation",	NULL,	"LTR"},
{ "numShape",	"NumShape",	NULL,	"bilingual"},
{ "charShape",	"CharShape",	NULL,	"automatic"},
{ "autopush",	"Autopush",	NULL,	"off"},
{ "expandTail", "Tail",	        NULL,	"off"},
{ "nobidi",     "NoBidi",	NULL,	"off"},
{ "noNulls",    "NoNulls",	NULL,	"off"},
{ NULL, NULL, NULL, NULL }};

/* index of resources in resourceTable[]
** order is important, do not change  
*/
# define	DEF_MAPS 	0	/* maps		*/
# define	DEF_FSCRREV	1	/* fScrRev	*/
# define	DEF_FLTR	2	/* fLTR		*/	
# define	DEF_FRTL	3	/* fRTL		*/
# define	DEF_FAUTOPUSH	4	/* fAutoPush	*/
# define	DEF_PFUSH	5	/* fPush	*/
# define	DEF_FENDPUSH	6	/* fEndPush	*/
# define	DEF_FASD     	7	/* fASD		*/
# define	DEF_FSHAPEIS   	8	/* fShapeIS	*/
# define	DEF_FSHAPEIN   	9	/* fShapeIN	*/
# define	DEF_FSHAPEM   	10	/* fShapeM	*/
# define	DEF_FSHAPEF   	11	/* fShapeF	*/
# define	DEF_FSHAPEP   	12	/* fShapeP	*/
# define	DEF_TEXTTYPE  	13	/* textType	*/
# define	DEF_SYMETRIC 	14	/* symetric	*/
# define	DEF_ORIENTATION 15	/* orientation	*/
# define	DEF_NSS 	16	/* numshape	*/
# define	DEF_CSD 	17	/* charshape	*/
# define	DEF_AUTOPUSH   	18	/* autopush    	*/
# define	DEF_TAIL   	19	/* tail    	*/
# define	DEF_NOBIDI 	20	/* no bidi     	*/
# define	DEF_NONULLS 	21	/* no nulls     */


/*
**	convert_to_lower 
**
**	convert a given string to lower case and move it to 'local'
**	buffer
*/

#define MAXSTRING	64

char *convert_to_lower(choice)
char *choice;
{
	register char	*src, *dst;
	static	char 	local[MAXSTRING+1];

	if ( strlen(choice) > MAXSTRING )
	{
		memcpy(local, choice, MAXSTRING);
		local[MAXSTRING] = '\0';
		return local;
	}

	src = choice;
	dst = local;

	while (*src)
	{
		if (isspace(*src))
			src++;
		else if (isupper(*src)) {
	    		*dst++ = tolower(*src);
	    		src++;
		}
		else
 			*dst++ = *src++;
	}
	*dst = '\0';

	return local;
}

static int  TrueFalse(choice)
char *choice;
{
	int	result;

	if (strcmp (choice, "true") == 0)
		result = TRUE;
	else if (strcmp (choice, "false") == 0)
		result = FALSE;
	else if (strcmp (choice, "on") == 0)
		result = TRUE;
	else if (strcmp (choice, "off") == 0)
		result = FALSE;
	else if (strcmp (choice, "yes") == 0)
		result = TRUE;
	else if (strcmp (choice, "no") == 0)
		result = FALSE;

 	return result;
}

int  BDSetBidiAtts (att, user_choice)
char *att;
char *user_choice;
{
      static char *local;

      local = convert_to_lower(user_choice);

      if  (strcmp (att, "orientation") == 0) {
        if (strcmp (local, "rtl") == 0)
                set_RTL_mode();
        else if (strcmp (local, "ltr") == 0)
                set_LTR_mode();
        else BDKeywordError(att, user_choice);
    }

    else if (strcmp (att, "numShape") == 0)
    {
        if (strcmp (local, "bilingual") == 0)
        {
           if (is_implicit_text())
                set_bilingual_nss();
           else set_arabic_nss();
        }

        else if (strcmp (local, "hindi") == 0)
                set_hindu_nss();

        else if (strcmp (local, "arabic") == 0)
                set_arabic_nss();

        else if (strcmp (local, "passthru") == 0)
                set_passthru_nss();

        else BDKeywordError(att, user_choice);
    }

     else if (strcmp (att, "charShape") == 0) 
     {
        if ((strcmp (local, "automatic") == 0)
           || (strcmp (local, "auto") == 0))
                set_asd_on();

        else if (strcmp (local, "passthru") == 0)
                set_passthru_csd();

        else if (strcmp (local, "isolated") == 0)
        {
             if (is_visual_text())

                set_isolated_csd();
        }
        else if (strcmp (local, "initial") == 0)
        {
             if (is_visual_text())
                set_initial_csd();
        }
        else if (strcmp (local, "middle") == 0)
        {
             if (is_visual_text())
                set_middle_csd();
        }
        else if (strcmp (local, "final") == 0)
        {
             if (is_visual_text())
                set_final_csd();
        }
        else BDKeywordError(att, user_choice);
    }

    else if  (strcmp (att, "textType") == 0)
    {
        if (strcmp (local, "implicit") == 0)
                set_implicit_text();
        else if (strcmp (local, "visual") == 0)
                set_visual_text();
        else BDKeywordError(att, user_choice);
    }
}

BDExtractResources(rDB, appname)
XrmDatabase rDB;
char *appname;
{
	char	*rdb_val, *str_type[20];
	XrmValue	value;
	char	resource_name[100], *resource_name_ptr;
	char	resource_class[100], *resource_class_ptr;
	int	appnamelen;
	resource_st  *r;
	
	appnamelen = strlen(appname);

	/* Set resource name */
	strcpy(resource_name, appname);
	resource_name[0] = tolower(resource_name[0]);
	resource_name[appnamelen] = '.';
	resource_name_ptr = &resource_name[appnamelen +1];

	/* Set resource class */
	strcpy(resource_class, appname);
	resource_class[0] = toupper(resource_class[0]);
	resource_class[appnamelen] = '.';;
	resource_class_ptr = &resource_class[appnamelen +1];
	
	for (r = resourceTable; r->resource_name; r++)
	{
		/* Get Resource from Resource Database */
		strcpy(resource_name_ptr, r->resource_name);
		strcpy(resource_class_ptr, r->resource_class);

		if (XrmGetResource(rDB, resource_name, resource_class,
			str_type, &value) == True) 
		{
			if (value.size == 0)
				continue;

			if ((rdb_val = malloc(value.size)) == NULL)
			{
				printf("extractOpt malloc Error\n");
				continue;
			}

			strncpy(rdb_val, value.addr, (int) value.size);
			r->resource_val = rdb_val;
		}
	}
}


BDSegInit()
{
	resource_st  *r;
	char	*rval, *val;
	int	i;
	
	for (i = 0, r = resourceTable; r->resource_name; i++, r++)
	{
		if (r->resource_val)
			rval = r->resource_val;
		else
			rval = r->default_val;

		val = convert_to_lower(rval);

		switch (i) 
		{
		 case	DEF_MAPS:
			if (!rval)
			{
	 		  /* No return from BDKeywordError */
			  BDKeywordError("maps", "NULL");
			}
			strcpy(BDMaps, rval);
			continue;

		 case	DEF_FSCRREV:
			if (TrueFalse(val))
				set_fScrRev_on();
			else	set_fScrRev_off();
			continue;

		 case	DEF_FLTR:
			if (TrueFalse(val))
				set_fLTR_on();
			else	set_fLTR_off();
			continue;

		 case	DEF_FRTL:
			if (TrueFalse(val))
				set_fRTL_on();
			else	set_fRTL_off();
			continue;

		 case	DEF_FAUTOPUSH:
			if (TrueFalse(val))
				set_fAutoPush_on();
			else	set_fAutoPush_off();
			continue;

		 case	DEF_PFUSH:
			if (TrueFalse(val))
				set_fPush_on();
			else	set_fPush_off();
			continue;

		 case	DEF_FENDPUSH:
			if (TrueFalse(val))
				set_fEndPush_on();
			else	set_fEndPush_off();
			continue;

		 case	DEF_FASD:
			if (TrueFalse(val))
				set_fASD_on();
			else	set_fASD_off();
			continue;

		 case	DEF_FSHAPEIS:
			if (TrueFalse(val))
				set_fShapeIS_on();
			else	set_fShapeIS_off();
			continue;

		 case	DEF_FSHAPEIN:
			if (TrueFalse(val))
				set_fShapeIN_on();
			else	set_fShapeIN_off();
			continue;

		 case	DEF_FSHAPEM:
			if (TrueFalse(val))
				set_fShapeM_on();
			else	set_fShapeM_off();
			continue;

		 case	DEF_FSHAPEF:
			if (TrueFalse(val))
				set_fShapeF_on();
			else	set_fShapeF_off();
			continue;

		 case	DEF_FSHAPEP:
			if (TrueFalse(val))
				set_fShapeP_on();
			continue;

		 case	DEF_TEXTTYPE:
                        BDSetBidiAtts(r->resource_name,val);
                        if (is_visual_text())
                            set_arabic_nss();
			continue;
  
		 case	DEF_SYMETRIC:
			if (TrueFalse(val))
				set_symetric_mode();
			else	reset_symetric_mode();
			continue;

		 case	DEF_ORIENTATION:
                        BDSetBidiAtts(r->resource_name,val);
                        if (is_RTL_mode())
                            set_nl_kbd();
			continue;

		 case	DEF_NSS:
                        BDSetBidiAtts(r->resource_name,val);
                        if (is_visual_text() && is_bilingual_nss())
                            set_arabic_nss();
			continue;

		 case	DEF_CSD:
                        BDSetBidiAtts(r->resource_name,val);
			continue;

		 case	DEF_AUTOPUSH:
                        if (TrueFalse(val))
                        {
                            if (is_LTR_mode())
                                set_left_autopush();
                            else  set_right_autopush();
                        }
                        else  {
                             if (is_LTR_mode())
                                 reset_left_autopush();
                             else  reset_right_autopush();
                        }
			continue;

		 case  DEF_TAIL:
                       if (TrueFalse(val))
                         reset_onecell_mode();
                       else set_onecell_mode();
                       continue;

		 case  DEF_NOBIDI:
                       if (TrueFalse(val))
                         reset_bidi_mode();
                       else set_bidi_mode();
                       continue;

		 case  DEF_NONULLS:
                       if (TrueFalse(val))
                         set_nonulls_mode();
                       else set_nulls_mode();
                       continue;

		} /* end of switch */

	} /* end of for */
}

char *GetBidiPath()
{
	extern	char	*getenv();
	char	*path = NULL;

	if (path = getenv ("BIDIPATH"))
	{
		/* variable may have the value of empty string */
		if (strlen(path) == 0)
			path = NULL;
	}

	return path;
}

/* Get user home directory */
static char *getHomeDir(dest)
char	*dest;
{
	int uid;
	extern char *getenv();
	extern uid_t getuid();
	extern struct passwd *getpwuid(), *getpwnam();
	struct passwd *pw;
	register char *ptr;

	if((ptr = getenv("HOME")) != NULL) {
		(void) strcpy(dest, ptr);

	} else {
		if((ptr = getenv("USER")) != NULL) {
			pw = getpwnam(ptr);
		} else {
			uid = getuid();
			pw = getpwuid(uid);
		}
		if (pw) {
			(void) strcpy(dest, pw->pw_dir);
		} else {
		        *dest = '\0';
		}
	}
	return dest;
}


/* Get System,  Application and User default */
static mergeDatabases(locale_name,appname)
char *locale_name;
char *appname;
{
	XrmDatabase  homeDB, appDB, sysDB, btermDB;

	char	filenamebuf[256];
	char	*filename = &filenamebuf[0];
	char	*classname;
	char	name[255];

	bidipath = &name[0];
        bidipath = GetBidiPath();

       /* search for .Bidi-defaults file in HOME dir. */
       /* looking for:  $HOME/.Bidi-defaults */
	(void) getHomeDir(filename);
	(void) strcat(filename, "/");
	(void) strcat(filename, BIDI_DEFAULTS);
	homeDB = XrmGetFileDatabase(filename);
	if (homeDB != NULL)
		(void) XrmMergeDatabases(homeDB, &rDB);

    /* if the home defaults is not found,search for the bterm defaults*/
    /* looking for :   $BIDIPATH/localename/app-defaults/BTerm */
        if (!homeDB)
        {
	(void) strcpy(name, bidipath);
	(void) strcat(name, "/");
	(void) strcat(name, locale_name);
	(void) strcat(name, APP_DEFAULTS);
	(void) strcat(name, "BTerm");
TRACE (("name %s \n",name));
	btermDB = XrmGetFileDatabase(name);
	if (btermDB)
		(void) XrmMergeDatabases(btermDB, &rDB);
	}

        if (!homeDB && !btermDB)
        {
        /* Get default  bidipath file if any */
        /* looking for : /usr/lib/nls/bidi/localename/app-defaults/BTerm */
        (void) strcpy(name, "/usr/lib/nls/bidi/");
	(void) strcat(name, locale_name);
        (void) strcat(name, APP_DEFAULTS);
        (void) strcat(name, "BTerm");
        sysDB = XrmGetFileDatabase(name);
        if (sysDB)
                (void) XrmMergeDatabases(sysDB, &rDB);
	}

	/* Command line takes precedence over everything */
	XrmMergeDatabases(commandlineDB, &rDB);
}

		
/* Command line options table.  Only resources are entered here.
** There is a pass over the remaining options after XrmParseCommand
*/
static parseBtermCommand(argcp, argv)
int *argcp;
register char *argv[];
{
	int	Argc = *argcp;
	char	**Argv = argv;
	int	j;

	extern	char	*user_termname;
	/* Check command arguments:
	** search for -help, -keywords and -name parameters
	/* -name parameter is needed for two reasons
	** 1. to parse command arguments into commandDB
	** 2. to get default application resource file 
	*/
        for (Argc--, Argv++; Argc > 0; Argc--, Argv++)
    	{
        	if (strcmp (*Argv, "-help") == NULL) 
		{
			/* there is no return from BDHelp() */
			BDHelp();
		}

        	if (strcmp (*Argv, "-keywords") == NULL) 
		{
			/* there is no return from BDListKeywords() */
			BDListKeywords();
		}

        	if (strcmp (*Argv, "-name") == NULL) 
		{
			strcpy(prog, *(++Argv));
			AppName = &prog[0];
		}

    	}
	XrmParseCommand(&commandlineDB, argTable, argTableEntries,
			AppName, argcp, argv);

	/* Check for any arguments left, only -e command is allowed */
    	for (j=1; j < *argcp; j++)
    	{
		if (*argv[j] == '-')
		{
			if (argv[j][1] != 'e') 
			{
				/* BDSyntax() never returns */
				BDSyntax(argv[j]);
			}
		}
    	}
}

int set_bterm_defaults(locale_name,argcp, argv)
char *locale_name;
int *argcp;
register char	*argv[];
{
        /* allocate and initialize BDSeg structure */
        bidi_init();

	/* Initialize Resource Manager Database */
	XrmInitialize();

	/* Parse Command line first so we can store any options
	** in the command line database
	*/
	AppName = BTERM;
        parseBtermCommand(argcp, argv);

	/* Get Program Defaults, .Bidi-defaults;
	** merge them and finally the command line
	*/
	mergeDatabases(locale_name,AppName);

	/* Extract values from database and convert to form usable
	** by this program
	*/
	BDExtractResources(rDB, AppName);

	/* Initialize Bidi Segment according to values stored
	** in the merged resource DataBase, rdb
	*/
	BDSegInit();
      
        /* save default BDSeg structure */
        BDSegDefaults = (BDSeg_st * )malloc(BDSEGSIZE);
        memcpy(BDSegDefaults,BDCurSeg,BDSEGSIZE);
}


/* Initialize conversion map   */
int BDInitMap(map_file, map_array)
char *map_file;
unsigned char  map_array[];
{
	FILE *fd;
	char map_file_name[128];
	char line[80], *lpoint;
	int i, jvalue; 
	char *bidipath;

	bidipath = GetBidiPath();

	strncpy(map_file_name, bidipath, 80);
	strcat(map_file_name, MAPS);
	strcat(map_file_name, BDMaps);
	strcat(map_file_name, "/");
	strncat(map_file_name, map_file, 18);

	if( (fd = fopen(map_file_name, "r")) == NULL)
		return(-1);

	while( !feof(fd) )
	{
		lpoint = fgets(line, 80, fd);
		lpoint = strtok(line, " ");
		if(*lpoint != '#')
		{
			i = atoi(lpoint);
			lpoint = strtok(NULL, " ");
			jvalue = atoi(lpoint); 
			map_array[i] = jvalue;
		}
	}

	fclose(fd);
	return(0);
}

int BDInitArabicMap(map_file, map_array)
char *map_file;
unsigned char  map_array[256][3];
{
	FILE *fd;
	char map_file_name[128];
	char line[80], *lpoint;
	int i;   /*, jvalue; */
	int jvalue[3];
	char *bidipath;

	bidipath = GetBidiPath();

	strncpy(map_file_name, bidipath, 80);
	strcat(map_file_name, MAPS);
	strcat(map_file_name, BDMaps);
	strcat(map_file_name, "/");
	strncat(map_file_name, map_file, 18);

	if( (fd = fopen(map_file_name, "r")) == NULL)
		return(-1);

	while( !feof(fd) )
	{
		lpoint = fgets(line, 80, fd);
		lpoint = strtok(line, " ");
		if(*lpoint != '#')
		{
			i = atoi(lpoint);
			lpoint = strtok(NULL, " ");
			strncpy(map_array[i],lpoint,strlen(lpoint)); 
		}
	}

	fclose(fd);
	return(0);
}


/* Load Bidi Maps */
int BDLoadMaps()
{
	int	jdx; 
	int	result = 0;

	/* First, initialize conversion maps     */
	for ( jdx = 0; jdx < 256; jdx++)
	{
		KBRD_MAP[jdx][0] = jdx; 
		KBRD_MAP[jdx][1] = '\0'; 
		KBRD_MAP[jdx][2] = '\0'; 
		SYMETRIC_MAP[jdx] = jdx; 
	}
	if(BDInitArabicMap(KBRD,  KBRD_MAP) < 0)
	{
	  fprintf(stderr, "\n%s, %s\n",
		"Could Not Initialize NL Layer Keyboard Map",
		KBRD);
	  result = -1;
 	}
	if(BDInitMap(SYMETRIC,  SYMETRIC_MAP) < 0)
	{
	  fprintf(stderr, "\n%s, %s\n",
		"Could Not Initialize Symetric Swap Map",
		SYMETRIC);
	  result = -1;
	}
	return result;
}


/**********************************************************************/

