static char sccsid[] = "@(#)50  1.13.1.4  src/bos/usr/lib/pios/fmtrs/piof5202.c, cmdpios, bos411, 9433B411a 8/17/94 12:52:17";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: setup, initialize, lineout, passthru, restore, errorexit, escseq
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*** piof5202.c - print formatter for QuietWriter data streams ***/


#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include "piostruct.h"
#include "piobe_msg.h"

/* include for getmsg routine */
#include <nl_types.h>
char *getmsg();

#define   PIOMSGCAT    "piobe.cat"
#define   PIOMSGSET     1

#define   FONTS_OK      0
#define   FONTS_BAD     1
#define   GET_STAT      11
#define   WRITE_FILE    12
#define   FILE_WRITTEN  13

extern int errno;
void errorexit();
int font_stat();



/*-------------------------------- Definitions --------------------------------+
|                                                                              |
|  The printer driver definition must appear after the lookup table(s)         |
|                                                                              |
+-----------------------------------------------------------------------------*/


#include "piof5202.h"



#define MAXVARSTRLEN  200       /* memory to allocate for each variable string*/

#define EOL           -2        /* end-of-line detected */

#define CMDOUT_PT(attr, pt) { piocmdout(attr, fileptr, pt, NULL); \
                              charcount += pt; }

#define CMDOUT(attr)        { piocmdout(attr, NULL   , 0 , NULL); }

#define CHARIN(x)           { x = piocharin(fileptr,NULL); \
                              if (x == EOF) return(EOF); \
                              charcount++; }

#define CHARIN_TRANS(x)     { x = piocharin(fileptr,(&Codepagename)->ptr); \
                              charcount++; }

#define UNCHARIN(c1)        { piopushin(c1); \
                              if (charcount > 1) charcount--; }

#define CHARIN_STREAM(x)     do { if(((x) = getc(fileptr)) == EOF) \
				     return(EOF); \
                                  charcount++; } while(0)

#define UNCHARIN_STREAM(c1)  do { (void)ungetc((c1),fileptr); \
                                  if(charcount > 1) charcount--; } while(0)

#define FONT_WID_SPC(x)     { if (x >= 2300)                      \
			      {                                   \
				  Pitch_fid = Hres/Pitch;         \
				  if (x > 4000)   /* 4000? */     \
				      Prop_spacing = TRUE;        \
				  else                            \
				      Prop_spacing = FALSE;       \
			      } else                              \
			      {                                   \
				  Pitch_fid = 144;                \
				  if ( 43  < x ) Pitch_fid -= 24; \
				  if ( 112 < x ) Pitch_fid -= 24; \
				  if ( 231 < x ) Pitch_fid -= 12; \
				  Prop_spacing = FALSE;           \
				  if (x >= 154 && x <= 176) Prop_spacing=TRUE;\
			      }                                   \
			      Spacing_fid = Prop_spacing + 1; }





/*----------------------------- Shadow Variables ------------------------------+
|                                                                              |
|  Shadow variables are "shadows" of data base variables.  Their meaning is    |
|  the same as the corresponding data base variables, but their values are     |
|  in different units (e.g., Vres units instead of lines).  Whenever a shadow  |
|  variable is updated, the corresponding data base variable (and perhaps other|
|  variables) must be updated also.  Macros are defined below for updating the |
|  variables.                                                                  |
|                                                                              |
|  Shadow variables such as "pglen", "tmarg", and "bmarg" that are the same for|
|  all values of "piomode" (see "piostruct.h") are defined below.  Shadow      |
|  variables such as "Hincr", "Vincr", and "Width"  that may have different    |
|  values depending on the current value of "piomode" are defined as work      |
|  variables in the data base.                                                 |
|                                                                              |
+-----------------------------------------------------------------------------*/

int pglen;
#define SET_Pglen(lines)        {Pglen = lines;  \
                                 pglen = lines * Vincr;}
int tmarg;
#define SET_Tmarg(lines)        {Tmarg = lines;  \
                                 tmarg = lines * Vincr;}
int bmarg;
#define SET_Bmarg(lines)        {Bmarg = lines;  \
                                 bmarg = lines * Vincr;}

#define SET_Lpi(lines)          {Lpi = lines;  \
                                 Vincr = Vdecr = Vres / lines;}

#define SET_Pgwidth(chars)      {Width = (chars) * Hres / Pitch + c_indent; \
                                 Pgwidth = Width / Hincr + 1; }

#define SET_Pitch(cpi)  {Pitch = cpi;   \
                         Hincr = (10*Hres) / (Condensed ? 171 : (10*Pitch)); \
                         Hincr += Hincr * (Cont_dblwide || Line_dblwide); }





/*------------------------------ Work Variables ------------------------------*/

int c_indent;           /* Command line indentation in Hres units             */
int x_indent;           /* Indentation called for by Esc X cmd in Hres units  */
int charcount;          /* number of bytes processed for the line             */
int vpos;               /* vertical position on page, in Vres units           */
int vtab_base;          /* base vpos value for vertical tab stops             */
int hpos;               /* horizontal position on page, in Hres units         */
int substchar;          /* substitute character to use when character         */
                        /* in input data stream can't be printed              */
char Vincr_cmd_norm[3]; /* the Vincr_cmd can be changed in ESC J, but the     */
                        /* orig cmd must be recorded to be retrieved later    */
struct shar_vars sharevars;     /* vertical spacing variables shared by this  */
                        /* formatter & the formatter driver                   */
static struct stage2_info *sel_infop; /* ptr to stage 2  info. for code page
                           currently selected by printer */
char wkbuf[1000];       /* work buffer */
char codepage_buf[LINE_MAX]; /* buffer in which to build list of codepage numbers
			  separated by commas; used instead of t0 attribute */

/*-----------------------------------------------------------------------------+
|                                                                              |
| NAME: setup                                                                  |
|                                                                              |
| FUNCTION:                                                                    |
|                                                                              |
| EXECUTION ENVIRONMENT:                                                       |
|                                                                              |
| (NOTES:)  make sure width is greater than some reasonable char width         |
|                                                                              |
| (RECOVERY OPERATION:)                                                        |
|                                                                              |
| (DATA STRUCTURES:)                                                           |
|                                                                              |
| RETURNS:                                                                     |
|                                                                              |
+-----------------------------------------------------------------------------*/

struct shar_vars *
setup(argc, argv, passthru)          /* 0 = do formatting; 1 = pass through */
    unsigned argc;
    char *argv[];
    int passthru;
{
int cnt, loopcnt;
char allowable_sets [10000];        /* the list of allowable font sets spec'd */
char *all_ptr = allowable_sets;     /* in the database                        */
char *beg_ptr = allowable_sets;     /* beginning of list of code pages        */
char trans2_path[500];              /* directory path for trans2 table files  */
PIO_CFLEVEL_DECL;



/*-----------------------------------------------------------------------------+
|                                                                              |
|   This formatter uses values which can be modified by the user such as page  |
|   length, pitch, indentation, etc.  In order to find out about those values  |
|   the piogetvals() must be called to examine the database (or, more          |
|   correctly, an area of memory which contains the database values.           |
|                                                                              |
|   piogetopts() is a subroutine which examines the flags which were passed    |
|   to the formatter via the command line.  Only those flags specified in the  |
|   'ia' line in the database are permitted.  Entering other flags on the      |
|   command line will cause the print job to terminate.  The command line flag |
|   values overlay the values set by examination of the database.              |
|                                                                              |
+-----------------------------------------------------------------------------*/

piogetvals(attrtable, MAXVARSTRLEN, NULL);



/*-----------------------------------------------------------------------------+
|                                                                              |
|   The 5202 class printers can have their fonts changed during a print job    |
|   by the datastream.  Subsequent print jobs may not need or want the fonts   |
|   as they are modified.  Two flags permit the user to indicate whether or    |
|   not the printer is to be reset to its power on condition before (-j)       |
|   and/or after (-J) a print job.  The existence or absence of the printer    |
|   device name in the flags directory acts as a flag to show the existing     |
|   status of the fonts.  If the file exists, the fonts are assumed to be OK.  |
|   If the file does not exist, then the fonts are assumed to have been wiped  |
|   out by a preceding print job.  The condition of the fonts will be noted    |
|   and kept current through this job.  At the end of the job, the status will |
|   be written back into the flags directory.                                  |
|                                                                              |
|   If we're running in standalone  mode then we can't know where              |
|   stdout is being directed.  It may be going to the printer (in which case   |
|   the device driver flag is relevant) or it may being trapped in a disk      |
|   file for later execution.  In standalone mode we will assume that the      |
|   printer fonts are botched when our print job arrives and reset the fonts   |
|   unless told not to (-j!).  When exiting we will reload the correct fonts   |
|   if our internal flag indicates we changed them during the print job unless |
|   we're told not to (-J!).  Since the print job may have been redirected to  |
|   a disk file we can't reset the flag so it may or may not reflect the true  |
|   condition of the printer when the next job is queued under the spooler.    |
|                                                                              |
+-----------------------------------------------------------------------------*/

Pr_font_stat = font_stat(GET_STAT);


piogetopt(argc, argv, OPTSTRING, NULL);


SET_Pglen(Pglen);             /* Initialize Shadow Variables Not In Data Base */
SET_Tmarg(Tmarg);
SET_Bmarg(Bmarg);



/*--------------------------- Validate  Parameters ---------------------------*/


if (!piogetstr(PIO_CFLEVEL_ATTR, NULL, 0, NULL) ||
    ((void)piogetvals(piocfltbl,0),
     Piocflevel < PIO_CFLEVEL_NEW))
{
if (Emphasized     < 0 || Emphasized > 1)
                                      errorexit(MSG_BADFORMFLAG1,"e",NULL,NULL);
if (Beginpg       <= 0)
                                      errorexit(MSG_BADFORMFLAG2,"g",NULL,NULL);
if (Init_printer   < 0 || Init_printer > 2)
                                      errorexit(MSG_BADFORMFLAG1,"j",NULL,NULL);
if (Quality        < 0 || Quality > 3)
                                      errorexit(MSG_BADFORMFLAG5,"q",NULL,NULL);
if (Lpi           != 6 && Lpi != 8)
                                      errorexit(MSG_BADFORMFLAG6,"v",NULL,NULL);
if (Auto_crlf      < 0 || Auto_crlf > 2)
                                      errorexit(MSG_BADFORMFLAG7,"x",NULL,NULL);
if (Doublestrike   < 0 || Doublestrike > 1)
                                      errorexit(MSG_BADFORMFLAG1,"y",NULL,NULL);
if (Restoreprinter < 0 || Restoreprinter > 1)
                                      errorexit(MSG_BADFORMFLAG1,"J",NULL,NULL);
if (Condensed      < 0 || Condensed > 1)
                                      errorexit(MSG_BADFORMFLAG1,"K",NULL,NULL);
if (Wrap           < 0 || Wrap > 1)
                                      errorexit(MSG_BADFORMFLAG1,"L",NULL,NULL);
if (Cont_dblwide   < 0 || Cont_dblwide > 1)
                                      errorexit(MSG_BADFORMFLAG1,"W",NULL,NULL);

if (Pgwidth < 1)                      errorexit(MSG_BADFORMFLAG2,"w",NULL,NULL);
if (Indent >= Pgwidth)                errorexit(MSG_BADFORMFLAG8,"i",NULL,NULL);
if (Indent < 0)                       errorexit(MSG_BADFORMFLAG3,"i",NULL,NULL);



if (Doublehigh     < 0 || Doublehigh > 1)
                                      errorexit(MSG_BADFORMFLAG1,"E",NULL,NULL);
if (Paper_source < 1 || Paper_source > 3)
                                      errorexit(MSG_BADFORMFLAG9,"u",NULL,NULL);

if (Rotation < 0 || Rotation > 3)     errorexit(MSG_BADFORMFLAG5,"z",NULL,NULL);
if (Duplex < 0 || Duplex > 2)         errorexit(MSG_BADFORMFLAG7,"Y",NULL,NULL);
}





/*-----------------------------------------------------------------------------+
|                                                                              |
|  The objective here is to locate a valid font id.  This can be arrived at    |
|  one of three ways. The font id can be specified directly using the -I flag. |
|  This method is given preference since is the most definitive.               |
|                                                                              |
|  The typestype (-s) and pitch (-p) can also be used to identify a font id.   |
|                                                                              |
|  Finally, failing the above two approaches, the pitch only will be used to   |
|  locate a font id.  If none of these methods results in the location of a    |
|  valid font id, the job will be terminated with an error message.            |
|                                                                              |
|  The FONTLIST is a variable string defined in the database which must be     |
|  of the format:                                                              |
|                                                                              |
|       typestyle[pitch,font_id(codepage1,...)]typestyle[pitch,...             |
|                              ^^^^^^^^^^^^^^^                                 |
|  LOGIC:                         optional (if not specified, t0 must          |
|                                           have trans2 file names)            |
|  if -I (",font_id]") is spec'd then                                          |
|       if found in FONTLIST then use it (with the associated pitch)           |
|       else errorexit("NO VALID FONT CHOSEN") (the font_id spec'd was bad)    |
|  else if -s & -p ("typestyle[pitch,") is found then use the related font_id  |
|       else if -p ("[pitch,") is found then use the related font_id           |
|            else errorexit("NO VALID FONT CHOSEN")                            |
|                                                                              |
+-----------------------------------------------------------------------------*/

/*  Get the string of allowable fonts, make the typestyle names lower case,   */
piogetstr(FONTLIST, allowable_sets, sizeof(allowable_sets), NULL);
for (cnt = 0 ; cnt < strlen(allowable_sets) ; cnt++)
    if (allowable_sets[cnt] != '[' && allowable_sets[cnt] != ']')
	allowable_sets[cnt] = allowable_sets[cnt] | 0x20;

/* Get the path name for trans2 tables */
(void) piogetstr(TRANS2_PATH, trans2_path, sizeof(trans2_path), NULL);

if (Font_id)
    {
    FONT_WID_SPC(Font_id);
    }
else
    {
    if ((&Typestyle)->len > 0 && strcmp(Pitch_string, "0"))
	{  /* Typestyle and Pitch are Available */
	/* Look For Requested Typestyle and Pitch in the Font List */
	*codepage_buf = '\0';
	for (loopcnt = 1; loopcnt <= 2 && *codepage_buf == '\0'; loopcnt++)
	    {   /* Scan Font List 2 times max.: (1) bitmapped (2) scalable */
	    (void) strcpy(wkbuf,(&Typestyle)->ptr);
	    for (cnt = 0 ; cnt<strlen(wkbuf) ; cnt++)
		wkbuf[cnt] = wkbuf[cnt] | 0x20; /* make name lower case */
	    strcat(wkbuf,"[");
	    if (loopcnt == 1)
		(void) strcat(wkbuf,(&Pitch_string)->ptr); /* bitmapped font */
	    else
		(void) strcat(wkbuf,"0");                  /* scalar font */
	    strcat(wkbuf,",");
	    all_ptr = allowable_sets;
	    while ((all_ptr = strstr(all_ptr, wkbuf)))
		{
		/* A valid combination was found */
		if ((all_ptr = strchr(all_ptr, ',')) == NULL)
		    continue;
		all_ptr++;
		if (Font_id == 0)
		    Font_id = atoi(all_ptr);
		beg_ptr = NULL;
		while ((all_ptr = strpbrk(all_ptr, "(),]")))
		    {
		    if (*all_ptr == '(')
			beg_ptr = ++all_ptr;
		    else if
			((*all_ptr == ',' || *all_ptr == ')') && beg_ptr!=NULL)
			{
			(void) strncat(codepage_buf, trans2_path,
				       sizeof(codepage_buf)
				       -strlen(codepage_buf));
			(void) strncat(codepage_buf, "/",
				       sizeof(codepage_buf)
				       -strlen(codepage_buf));
			(void) strncat(codepage_buf, beg_ptr,
				       MIN(sizeof(codepage_buf)
					   -strlen(codepage_buf),
					   all_ptr-beg_ptr));
			(void) strncat(codepage_buf, ",",
				       sizeof(codepage_buf)
				       -strlen(codepage_buf));
			if (*all_ptr == ',')
			    beg_ptr = all_ptr + 1;
			else   /* ')' */
			    beg_ptr = NULL;
			all_ptr++;
			}
		    else  /* must be a ']' */
			break;
		    }
		}
	    }
	/* Prepend the Constructed List of Translate Tables to the */
	/* Beginning of the Base_table (t0) attribute value string */
	(void) strncat(codepage_buf, (&Base_table)->ptr, sizeof(codepage_buf)
		       -strlen(codepage_buf));
	(&Base_table)->ptr = codepage_buf;
	(&Base_table)->len = strlen(codepage_buf);
	}
    }


/* If, after all this, the Font_id is still null, then quit */

if (!Font_id)
    errorexit(MSG_BAD_FONT_SET2,(&Typestyle)->ptr,(&Pitch_string)->ptr,NULL);





piopgskip = Beginpg - 1;          /* Arrange For Pages To Be Skipped (if any) */

/* Get Substitute Character to be Used if Character Can't Be Printed */
substchar=(piogetstr(SUBSTCHAR, wkbuf, sizeof(wkbuf), NULL)?(int) *wkbuf:-1);


cnt  = piotab2(BASE_TABLE, &sel_infop);
cnt += piotab2(TABLE_1, &sel_infop);
cnt += piotab2(TABLE_2, &sel_infop);
cnt += piotab2(TABLE_3, &sel_infop);
cnt += piotab2(TABLE_4, &sel_infop);
cnt += piotab2(TABLE_5, &sel_infop);
cnt += piotab2(TABLE_6, &sel_infop);
cnt += piotab2(TABLE_7, &sel_infop);
cnt += piotab2(TABLE_8, &sel_infop);
cnt += piotab2(TABLE_9, &sel_infop);

if (cnt == -10) errorexit(MSG_NEED_TRANS2,NULL,NULL,NULL);




/* If Passthru Mode, That's All That Needs to be Done for Setup */
if (Passthru = passthru) return(NULL);


/*-----------------------------------------------------------------------------+
|                                                                              |
|  Initialize Pointers to Vertical Spacing Variables Shared With Formatter     |
|  Driver (except for command names, all values pointed to are in Vres units)  |
|                                                                              |
+-----------------------------------------------------------------------------*/

                                          /*+--------------------------------+*/
sharevars._pl        = &pglen;            /*|page length                     |*/
sharevars._tmarg     = &tmarg;            /*|top margin                      |*/
sharevars._bmarg     = &bmarg;            /*|bottom margin                   |*/
sharevars._vpos      = &vpos;             /*|vertical position of next line  |*/
sharevars._vtab_base = &vtab_base;        /*|base for vertical tab stops     |*/
sharevars._vincr     = &Vincr;            /*|vertical increment of Vincr cmd |*/
sharevars._vincr_cmd = Vincr_cmd_norm;    /*|address of vert inc cmd         |*/
sharevars._vdecr     = &Vdecr;            /*|vertical decrement of Vdecr cmd |*/
sharevars._vdecr_cmd = (&Vdecr_cmd)->ptr; /*|name of vertical decrement cmd  |*/
sharevars._ff_cmd    = (&Ff_cmd)->ptr;    /*|name of form feed command       |*/
sharevars._ff_at_eof = &Do_formfeed;      /*|address of ff flag              |*/
sharevars._set_cmd   = SWITCHMODE_CMD;    /*|name of command to switch modes |*/
                                          /*+--------------------------------+*/

strcpy(Vincr_cmd_norm,(&Vincr_cmd)->ptr); /* record normal Vincr_cmd          */

x_indent = 0;               /* Indentation set by ESC X command in Hres units */
c_indent = Indent * Hincr;  /* Command line indentation in Hres units         */

return(&sharevars);

}


/*-----------------------------------------------------------------------------+
|                                                                              |
| NAME: initialize                                                             |
|                                                                              |
| FUNCTION:                                                                    |
|                                                                              |
| EXECUTION ENVIRONMENT:                                                       |
|                                                                              |
| (NOTES:)                                                                     |
|                                                                              |
| (RECOVERY OPERATION:)                                                        |
|                                                                              |
| (DATA STRUCTURES:)                                                           |
|                                                                              |
| RETURNS:                                                                     |
|                                                                              |
+-----------------------------------------------------------------------------*/

initialize()
{
FILE *fileptr;
int c1;
char wkstr[20];

if (!Init_printer) return(0);

CMDOUT(INIT_CMD);      /* Output the Command String to Initialize the Printer */

Pr_font_stat = FONTS_OK;

piogetstr(DOWNLOAD_FONT, wkbuf, sizeof(wkbuf), NULL);        /* Download Font */
if (*wkbuf != NULL)
    {
    if ((fileptr = fopen(wkbuf, "r")) == NULL)
        {
        (void) sprintf(wkstr,"%d",errno);
        errorexit(MSG_OPEN_FONTS,wkbuf,wkstr,NULL);
        }
    while ((c1 = piogetc(fileptr)) != EOF)
    {
       pioputchar(c1);
       charcount++;
    }
    Pr_font_stat = FONTS_BAD;
    }

CMDOUT(sel_infop->cmdp);         /* Activate the currently required code page */

return(0);
}


/*-----------------------------------------------------------------------------+
|                                                                              |
| NAME: lineout                                                                |
|                                                                              |
| FUNCTION:                                                                    |
|                                                                              |
| EXECUTION ENVIRONMENT:                                                       |
|                                                                              |
| RETURNS:                                                                     |
|                                                                              |
+-----------------------------------------------------------------------------*/

lineout(fileptr)
FILE *fileptr; {
                                   /*+---------------------------------------+*/
int value,tabval,cnt,w1,w2;        /*|work variables                         |*/
int c1;                            /*|character from input data stream       |*/
char *tabs;                        /*|pointer to array of tab stops          |*/
char wkstr1[20], wkstr2[20];       /*|work strings                           |*/
int doingsubchar;                  /*|TRUE if processing a substitute char.  |*/
struct stage2_info *cur_infop;     /*|ptr to stage 2 info. for code          |*/
                                   /*|page currently being looked at         |*/
struct transtab *tabentp;          /*|ptr to stage 2 table entry             |*/
int c2;                            /*|code point for printer code page       |*/
int done;                          /*|are we done with the input char?       |*/
int first_of_multi;                /*|this is the first of multiple code page|*/
                                   /*|to be searched, starting at sel_infop  |*/
                                   /*+---------------------------------------+*/

/*-----------------------------------------------------------------------------+
|                                                                              |
|         Update Variables Shared With the Formatter Driver That May           |
|         Have Different Values Depending on the Value of "piomode             |
|                                                                              |
+-----------------------------------------------------------------------------*/

sharevars._vincr = &Vincr;
sharevars._vdecr = &Vdecr;
sharevars._vdecr_cmd = (&Vdecr_cmd)->ptr;
strcpy(Vincr_cmd_norm,(&Vincr_cmd)->ptr);    /* retrieve normal Vincr_cmd */

if (Line_dblwide)           /* Line_dblwide mode is turned off at the end */
    {                       /* of every line.  Hincr, etc. must be adjusted. */
    Line_dblwide = FALSE;
    SET_Pitch(Pitch);
    }

/* Perform Indentation (before turning on overscore or underscore) */
if ( hpos = Hpos_from_prev ) Hpos_from_prev = 0;
else hpos = x_indent + c_indent;

if (Workint1 = hpos) CMDOUT(HORZ_SPACE_CMD);
if (Cont_overscore)  CMDOUT(OVERSCORE_CMD); /* If Overscore and/or Underscore */
if (Cont_undrscore)  CMDOUT(UNDERSCORE_CMD); /* should be on then do it */


/*-----------------------------------------------------------------------------+
|                                                                              |
|    Loop on Input Characters Until End of Line (or End of File) Reached       |
|                                                                              |
+-----------------------------------------------------------------------------*/

for (charcount = 0;;)
    {
    CHARIN_TRANS(c1);
    if (c1 == EOF) goto ENDLINE;
    if (c1 > 31) goto GRAPHIC;  /* If a Graphic Character, Jump Past switch() */


/*-----------------------------------------------------------------------------+
|                                                                              |
|                              Control Character                               |
|                                                                              |
+-----------------------------------------------------------------------------*/

switch (c1)
{
case 10:            /*** '0A' - Line Feed ***      most used, so put it first */
LINEFEED:
    Line_dblwide = FALSE;
    SET_Pitch(Pitch);
    vpos += Vincr * Linespacing;
    if (Auto_crlf != 2)
        {
        CHARIN_STREAM(value);
        UNCHARIN_STREAM(value);
        if (value != '\r') Hpos_from_prev = hpos;
        }
    goto ENDLINE;

case 0:                                                         /* '00'  Null */
    CMDOUT(NULL_CMD);
    break;

case 3:               /* heart   */
case 4:               /* diamond */
case 5:               /* club    */
case 6:               /* spade   */
case 21:
    pioputchar(c1);
    break;

case 7:                                              /* '07' - Sound the Bell */
    CMDOUT(BEL_CMD);
    break;

case 8:                                                   /* '08' - Backspace */
    if (hpos <= c_indent) break;
    hpos -= Hincr;
    CMDOUT(BS_CMD);
    break;

case 9:                                              /* '09' - Horizontal Tab */
    tabs = (&Horz_tabs)->ptr;             /* array of tab stops */
    for (cnt = 0; tabs[cnt] != 0; cnt++)
        {
        tabval = (tabs[cnt] - 1) * Hincr + x_indent + c_indent;
        if (tabval > hpos) break;
        }
    if (tabs[cnt] != 0)                   /* if found a tab stop */
        {
        if (tabval > Width) hpos = Width; /* tab to EOL */
        else
            {                            /*+---------------------------------+*/
            if (Cont_overscore)          /*|A tab stop has been found.       |*/
                {                        /*|                                 |*/
                w1 = TRUE;               /*|The printer manuals say that     |*/
                Cont_overscore = FALSE;  /*|whitespace produced by a tab     |*/
                CMDOUT(OVERSCORE_CMD);   /*|won't be under/over scored if the|*/
                }                        /*|attribute is on.                 |*/
            else w1 = FALSE;             /*|                                 |*/
                                         /*|That means that just putting out |*/
            if (Cont_undrscore)          /*|spaces (which would be scored)   |*/
                {                        /*|would make the printer behave    |*/
                w2 = TRUE;               /*|incorrectly, therefore if either |*/
                Cont_undrscore = FALSE;  /*|attribute is on, record that     |*/
                CMDOUT(UNDERSCORE_CMD);  /*|fact, turn the attribute off     |*/
                }                        /*+---------------------------------+*/
            else w2 = FALSE;


            while(hpos < tabval)         /* output spaces to get to tab stop */
                {
                pioputchar(' ');
                hpos += Hincr;
                }

            if (Cont_overscore = w1) CMDOUT(OVERSCORE_CMD);  /* then turn the */
            if (Cont_undrscore = w2) CMDOUT(UNDERSCORE_CMD); /* attr. back on.*/

            }
        }
    break;

case 11:                                               /* '0B' - Vertical Tab */
    tabs = (&Vert_tabs)->ptr;          /* array of tab stops */
    value = vpos - vtab_base;          /* base vpos for tab stops */
    for (cnt = 0; tabs[cnt] != 0; cnt++)
        {
        tabval = (tabs[cnt] - 1) * Vincr;
        if (tabval > value) break;
        }
    if (tabs[cnt] != 0)                 /* if found a tab stop */
        vpos = tabval + vtab_base;      /* advance to the tab stop */
    else                                /* otherwise */
        vpos += Vincr;                  /* no tab stop, so do line feed */

    if (Auto_crlf != 2) Hpos_from_prev = hpos;
    Line_dblwide = FALSE;
    SET_Pitch(Pitch);
    goto ENDLINE;

case 12:                                                  /* '0C' - Form Feed */
    Line_dblwide = FALSE;
    SET_Pitch(Pitch);
    vpos = pglen;
    goto ENDLINE;

case 13:                                            /* '0D' - Carriage Return */
    Line_dblwide = FALSE;
    SET_Pitch(Pitch);
    if (Auto_crlf == 1)
        {
        vpos += Vincr * Linespacing;
        goto ENDLINE;
        }
    CMDOUT(CR_CMD);
    hpos = c_indent + x_indent;
    if ( Workint1 = hpos ) CMDOUT(HORZ_SPACE_CMD);
    break;

case 14:                         /* '0E' - Start Double-Wide Printing by Line */
    Line_dblwide = TRUE;
    SET_Pitch(Pitch);
    CMDOUT(SO_CMD);
    break;

case 15:                                   /* '0F' - Start Condensed Printing */
    Condensed = TRUE;
    SET_Pitch(Pitch);
    CMDOUT(SET_COND_CMD);
    break;

case 18:                                /* '12' - DC2: 10 Characters Per Inch */
    Condensed = FALSE;
    SET_Pitch(10);
    CMDOUT(SET_PITCH_CMD);
    break;

case 20:                   /* '14' - DC4: Cancel Double-Wide Printing by Line */
    Line_dblwide = FALSE;
    SET_Pitch(Pitch);
    CMDOUT(DC4_CMD);
    break;

case 24:                  /* Cancel - clears page on laser printer - shouln't */
    break;                /*          be in the data stream - toss it         */

case 27:                                                     /* '1B' - Escape */
    switch (escseq(fileptr))
        {
        case 0:       break;                                 /* normal return */
        case EOL:     goto LINEFEED;                           /* end of line */
        case EOF:     goto ENDLINE;                      /* end of input file */
        }
    break;

default:                                              /* Ignore the Character */
    break;
}

continue;                                             /* on to next character */


/*-----------------------------------------------------------------------------+
|                                                                              |
|                               Graphic Character                              |
|                                                                              |
+-----------------------------------------------------------------------------*/

GRAPHIC:

                                        /*+----------------------------------+*/
    if ((hpos += Hincr) > Width)        /*|If page width would be exceeded by|*/
        {                               /*|printing the next char then - if  |*/
        if (Wrap)                       /*|WRAP is on, stuff the char back   |*/
            {                           /*|into the buffer, increment VPOS,  |*/
            UNCHARIN(c1);               /*|and go to the next line; ELSE act |*/
            vpos += Vincr * Linespacing;/*|like the char was printed by      |*/
            goto ENDLINE;               /*|jumping down to FINISHED which    |*/
            }                           /*|closes the braces and exits.      |*/
        else goto FINISHED;             /*+----------------------------------+*/
        }


/*-----------------------------------------------------------------------------+
|                                                                              |
|  If the Codepage name is a null string then the user is saying that no       |
|  translation is to be done.  piotrans will return the character picked       |
|  off the datastream untranslated.  This routine will be skipped altogether.  |
|                                                                              |
+-----------------------------------------------------------------------------*/

if (!(&Codepagename)->len) {pioputchar(c1);}       /* No codepage translation */
else {                                            /* called for - output 'c1' */

    doingsubchar = FALSE;    /* Get Ready to Run Circular Chain of Code Pages */

STARTAGAIN:

    done = FALSE;
    if (sel_infop->infop != sel_infop)       /* if muliple code pages chained */
        first_of_multi = TRUE;                 /* remember: this is the first */
    else
        first_of_multi = FALSE;


/*-----------------------------------------------------------------------------+
|                                                                              |
|    Start at the Code Page Currently Selected By the Printer and Run the      |
|    Circular Chain of Code Pages Looking For One That Has the Character       |
|                                                                              |
+-----------------------------------------------------------------------------*/


    for (cur_infop = sel_infop;
         done == FALSE;
         cur_infop = cur_infop->infop, first_of_multi = FALSE)
        {
        if (c1 < cur_infop->num_tabentries)
            {
            tabentp = cur_infop->tabp + c1;
            c2 = tabentp->outchar;
            }
        else
            {
            tabentp = NULL;
            c2 = SC;          /* can't print the character, so use substitute */
            }

        if (c2 < 0)
            {
            switch (c2)               /* special code instead of a code point */
                {
                case CP:                               /* Copy the Code Point */
                    c2 = c1;
                    break;

                case SC:                              /* Substitute Character */

/*-----------------------------------------------------------------------------+
|                                                                              |
|          Character not in this printer code page, so see if ready            |
|                 to print the substitute character instead                    |
|                                                                              |
+-----------------------------------------------------------------------------*/

                    if (cur_infop == sel_infop && !first_of_multi)
                        {

/*-----------------------------------------------------------------------------+
|                                                                              |
|              Have Been Through All the Printer Code Pages and                |
|              Couldn't Find the Character in Any of Them, So                  |
|              Print Substitute Character Instead                              |
|                                                                              |
+-----------------------------------------------------------------------------*/

                        if (!doingsubchar)
                            {
                            if (substchar < 0) break;
                            c1 = substchar;
                            doingsubchar = TRUE;
                            goto STARTAGAIN;
                            }
                        else errorexit(MSG_SUBST_CHAR,NULL,NULL,NULL);
                        }        /* Can't Print Subst Char Either, So Give Up */
                    else continue;
                    break;

                default:
                    (void) sprintf(wkstr1, "%d", c1);
                    (void) sprintf(wkstr2, "%d", c2);
                    errorexit(MSG_CODEPOINT2,cur_infop->filename,
                                                            wkstr1,wkstr2,NULL);
                    break;
                }
            }

    /* If Needed, Tell the Printer to Select the Code Page */
            if (cur_infop != sel_infop)
                {
                CMDOUT(cur_infop->cmdp);
                sel_infop = cur_infop;
                }

    /* Output the Character and the Optional Prefix and/or Suffix Commands*/
            if (tabentp && (int)tabentp->index1 > 0)
                CMDOUT(cur_infop->cmdp+tabentp->index1);
            pioputchar(c2);

            if (tabentp && (int)tabentp->index2 > 0)
                CMDOUT(cur_infop->cmdp+tabentp->index2);

            done = TRUE;
        }                                                /* end of 'for' loop */
FINISHED:;
    }
}

ENDLINE:

if (Cont_overscore)    /* Don't Leave Overscore or Underscore On Across Lines */
    {
    Cont_overscore = FALSE;
    CMDOUT(OVERSCORE_CMD);
    Cont_overscore = TRUE;
    }
if (Cont_undrscore)
    {
    Cont_undrscore = FALSE;
    CMDOUT(UNDERSCORE_CMD);
    Cont_undrscore = TRUE;
    }



return(charcount);

}



/*-----------------------------------------------------------------------------+
|                                                                              |
| NAME: passthru                                                               |
|                                                                              |
| FUNCTION:                                                                    |
|                                                                              |
| RETURNS:                                                                     |
|                                                                              |
+-----------------------------------------------------------------------------*/

passthru()
{
int c1 = 0;
int c2 = 0;
int c3 = 0;


while ((c1 = piogetchar()) != EOF)
    {
    pioputchar(c1);
    if (c3 == 27 && c2 == '[' && c1 == 'C') Pr_font_stat = FONTS_BAD;
    c3 = c2;
    c2 = c1;
    }

if (piodatasent && Do_formfeed) piocmdout(FF_CMD, NULL, 0, NULL);


return(0);
}



/*-----------------------------------------------------------------------------+
|                                                                              |
| NAME: restore                                                                |
|                                                                              |
| FUNCTION:                                                                    |
|                                                                              |
| EXECUTION ENVIRONMENT:                                                       |
|                                                                              |
| (NOTES:)   (Restoreprinter) will be the default (or command line) value      |
|            only when this routine is called for the last time for a print    |
|            job.  Otherwise, it will be zero and the printer will not be      |
|            restored.                                                         |
|                                                                              |
| (RECOVERY OPERATION:)                                                        |
|                                                                              |
| (DATA STRUCTURES:)                                                           |
|                                                                              |
| RETURNS:                                                                     |
|                                                                              |
+-----------------------------------------------------------------------------*/

restore()
{

if (Pr_font_stat == FONTS_OK) font_stat(WRITE_FILE);

/* Output the Command String to Restore the Printer */
if (Restoreprinter) CMDOUT(REST_CMD);

return(0);
}



/*----------------------------------------------------------------------------+
|                                                                             |
| NAME:     errorexit                                                         |
|                                                                             |
| FUNCTION: produces an error message from the referenced message catalog     |
|           and exits                                                         |
|                                                                             |
| RETURNS:  nada                                                              |
|                                                                             |
+----------------------------------------------------------------------------*/

void errorexit(msgnum, stringval1, stringval2, stringval3)
int msgnum;
char *stringval1, *stringval2, *stringval3;
{
char msgbuf[1000];

(void) sprintf(
	msgbuf, getmsg(PIOMSGCAT, PIOMSGSET, msgnum),
        stringval1, stringval2, stringval3);
(void) piomsgout(msgbuf);
if (piodatasent && Do_formfeed) piocmdout(FF_CMD, NULL, 0, NULL);
pioexit(PIOEXITBAD);
}



/*----------------------------------------------------------------------------+
|                                                                             |
| NAME:     font_stat                                                         |
|                                                                             |
| FUNCTION: Determines an/or sets the status of the file which tells the      |
|           formatter whether or not to instruct the printer to reload its    |
|           fonts from its own internal diskette.  If a file, whose name is   |
|           the printer device name, exists, then the printer fonts are       |
|           assumed to be good, (not in need of reloading).                   |
|                                                                             |
|           As soon as the file status is checked, the file will be unlinked. |
|           This makes the assumption that during the print job the fonts     |
|           will be corrupted.  If that assumption fails, the last thing the  |
|           formatter will do will be to re-write a new file.                 |
|                                                                             |
|           Eventually, code will be added to the rc initialization file      |
|           which will delete all these files.  This will insure us that      |
|           the formatter and the printer are in sync.                        |
|                                                                             |
| RETURNS:  The condition of the operation just performed - see the code      |
|                                                                             |
+----------------------------------------------------------------------------*/


int font_stat(op_code)
    int op_code;
    {
    extern int errno;
    struct stat buffr[1], *buffr_pt;
    int rc, filedes;
    char filename[100];

    if (!Manage_fonts) return(FONTS_OK);

    piogetstr(FLAGS_DIR, filename, sizeof(filename), NULL);
    piogetstr(PR_DEVICE, wkbuf, sizeof(wkbuf), NULL);
    strcat(filename,"/");
    strcat(filename,wkbuf);

    buffr_pt = buffr;

    if (op_code == GET_STAT)
        {
        if ((rc = stat(filename,buffr_pt))) rc = FONTS_BAD;
        else
            {                                       /* rc is 0 therefore file */
            if (!S_ISDIR(buffr_pt->st_mode))      /* exists or is a directory */
                {
                rc = FONTS_OK;
                unlink(filename);         /* since the file was never opened, */
                }                              /* unlink kills it immediately */
            else rc = FONTS_BAD;
            }
        }

    if (op_code == WRITE_FILE)
        {
        umask(0);

#ifdef _IBMRT
        filedes = creat(filename,438);   /* create file with group read/write */
#else
	filedes = creat(filename,432);   /* create file with group read/write */
#endif

        close(filedes);
        rc = FILE_WRITTEN;
        }

    return(rc);
    }




/*----------------------------------------------------------------------------+
|                                                                             |
| NAME:       escseq                                                          |
|                                                                             |
| FUNCTION:   handles escape sequences appearing in the data stream           |
|                                                                             |
| RETURNS:                                                                    |
|                                                                             |
+----------------------------------------------------------------------------*/

escseq(fileptr)
FILE *fileptr; {

int c2;                               /* character after the escape character */
int cnt, value, n1, n2, n3;                                 /* work variables */
char *tabs;                                  /* pointer to array of tab stops */

CHARIN(c2);
switch ((char)c2)
{
/*----------------------------------------------------------------------------*/

case '-':                                     /* Continuous underscore on/off */
    CHARIN(n1);
    Cont_undrscore = (n1 & 0x01);
    CMDOUT(UNDERSCORE_CMD);
    break;

case '0':                                  /* zero - Trigger 8 lines per inch */
    SET_Lpi(8);
    Vertspace_mode = FALSE;
    CMDOUT(SET_VERTSP_CMD);
    break;

case '1':                                        /* Trigger 10 lines per inch */
    Vincr = Vdecr = (Vres * 7) / Tunits;
    Vertspace_mode = TRUE;
    CMDOUT(SET_VERTSP_CMD);
    break;

case '2':                                /* Trigger line spacing set by ESC A */
    n1 = Tunits/Text_linesp;
    if ((n1 == 6) || (n1 == 8))
        {                        /* braces must remain due to macro expansion */
        SET_Lpi(n1);
        }
    else Vincr = Vdecr = ((Vres * Text_linesp) / Tunits);
    Vertspace_mode = 2;
    CMDOUT(SET_VERTSP_CMD);
    break;

case '3':                                        /* Set graphics line spacing */
    CHARIN(n1);
    Graphic_linesp = n1;                       /* value to be sent to printer */
    Vincr = Vdecr = (Vres * n1) / Gunits;
    Vertspace_mode = 3;
    CMDOUT(SET_VERTSP_CMD);
    break;

case '4':                                                  /* Set Top of Form */
    break;

case '5':                                              /* Automatic line feed */
    CHARIN(n1);
    Auto_crlf = (n1 & 0x01);
    break;

case '6':                                               /* Select Char set 2 */
case '7':                                               /* Select Char set 1 */
    break;

case ':':                                                      /* Set 12 cpi */
    Condensed = FALSE;
    Prop_spacing = FALSE;
    SET_Pitch(12);
    CMDOUT(SET_COND_CMD);
    break;

case '=':                                         /* Download a chacater set */
    CHARIN(n1);
    CHARIN(n2);
    cnt = n1 + (n2 * 256);
    CMDOUT_PT(DOWNLOAD_CMD,cnt);
    break;

/*----------------------------------------------------------------------------*/

case 'A':                       /* Set text line spacing - triggered by ESC 2 */
    CHARIN(Text_linesp);
    CMDOUT(SET_LS_CMD);
    break;

case 'B':                                                /* Set Vertical Tabs */
    tabs = (&Vert_tabs)->ptr;
    if (Max_vtabs > 0)
        {
        for (cnt = 0; cnt <= Max_vtabs; cnt++)
            {
            CHARIN(value);
            if ((tabs[cnt] = (char)value) == 0) break;
            }
       if (cnt > Max_vtabs)
           {
           cnt = Max_vtabs;
           tabs[cnt] = 0;
           }
       }
    break;

case 'C':                                                  /* Set form length */
    CHARIN(n1);
    if (!(n1)) CHARIN(n1);      /* if n1=0 then read char (Form len (inches)) */
    Tmarg = Bmarg = 0;
    break;

case 'D':                                              /* Set Horizontal Tabs */
    tabs = (&Horz_tabs)->ptr;
    if (Max_htabs > 0)
       {
       for (cnt = 0; cnt <= Max_htabs; cnt++)
           {
           CHARIN(value);
           if ((tabs[cnt] = (char)value) == 0) break;
           }
       if (cnt > Max_htabs)
           {
           cnt = Max_htabs;
           tabs[cnt] = 0;
           }
        }
    break;

case 'E':                                                    /* Emphasized on */
    Emphasized = TRUE;
    CMDOUT(SET_EMPH_CMD);
    break;

case 'F':                                                   /* Emphasized off */
    Emphasized = FALSE;
    CMDOUT(SET_EMPH_CMD);
    break;

case 'G':                                                 /* Double strike on */
    Doublestrike = TRUE;
    CMDOUT(SET_DBLSTR_CMD);
    break;

case 'H':                                                /* Double strike off */
    Doublestrike = FALSE;
    CMDOUT(SET_DBLSTR_CMD);
    break;




case 'I':                                                /* Select print mode */
    CHARIN(n1);


    switch(I_type)
        {
    /*-------------------------------------------------------------------------+
    |                                                                          |
    |             This logic applicable to the Quietwriter III 5202            |
    |                                                                          |
    +-------------------------------------------------------------------------*/
    case 1:
        Workint1 = FALSE;           /* This will tell me if Workint1 is reset */
        switch(n1)
            {
            case 1:
            case 2:
            case 3: Quality = Workint1 = n1;
                break;

            case 32: Font_id = Workint1 = 11;
            case 64: Condensed = FALSE; Pitch = 10;
                break;

            case 33: Font_id = Workint1 = 85;
            case 65: Condensed = FALSE; Pitch = 12;
                break;

            case 34: Font_id = Workint1 = 254;
            case 66: Condensed = TRUE; Pitch = 12;
                break;

            case 35: Font_id = Workint1 = 159;
            case 67: Condensed = FALSE; Pitch = 12;
                break;

            default:
                break;
            }
        SET_Pitch(Pitch);
        FONT_WID_SPC(Font_id);
        if (Workint1) CMDOUT(SEL_FONT_CMD);
        break;

    /*-------------------------------------------------------------------------+
    |                                                                          |
    |                     This logic applicable to the 3816                    |
    |                                                                          |
    +-------------------------------------------------------------------------*/
    case 2:
        Workint1 = FALSE;           /* This will tell me if Workint1 is reset */
        switch(n1)
            {
            case 1:
            case 2:
            case 3: Quality = Workint1 = n1;
                break;

            case 32:
            case 36: Font_id = Workint1 = 11;
                Condensed = FALSE ; Pitch = 10;
                break;

            case 33:
            case 37: Font_id = Workint1 = 85;
                Condensed = FALSE ; Pitch = 12;
                break;

            case 34:
            case 38: Font_id = Workint1 = 254;
                Condensed = TRUE ; Pitch = 12;
                break;

            case 35:
            case 39: Font_id = Workint1 = 159;
                Condensed = FALSE ; Pitch = 12;
                break;

            default:
                break;

            }
        SET_Pitch(Pitch);
        FONT_WID_SPC(Font_id);
        if (Workint1) CMDOUT(SEL_FONT_CMD);
        break;

    }
    break;





case 'J':                                   /* Graphics Variable line spacing */
    CHARIN(Workint1);                          /* value to be sent to printer */
    Vincr = Vdecr = (Vres * Workint1) / Gunits;     /* inc Vincr by n1*Gunits */
    strcpy(Vincr_cmd_norm,VAR_LS_CMD);         /* change endline cmd to ESC J */
    return(EOL);                                /* let formatter issue new lf */

case 'K':                                                  /* 60 DPI Graphics */
    CHARIN(n1);
    CHARIN(n2);
    cnt = n1 + (n2 * 256);                /* number of bytes of graphics data */
    CMDOUT_PT(GRA_60_CMD, cnt);                /* output the graphics command */
    hpos += (cnt / 60) * Hres;
    break;

case 'L':                                     /* 120 DPI Graphics (low speed) */
    CHARIN(n1);
    CHARIN(n2);
    cnt = n1 + (n2 * 256);
    CMDOUT_PT(GRA_120LS_CMD, cnt);
    hpos += (cnt / 120) * Hres;
    break;

case 'N':                                        /* Set auto perforation skip */
    CHARIN(n1);
    if (!n1) break;               /* if n1=0 leave the current margins active */
    if (n1 > Pglen)                    /* if n1>Pglen then cancel the margins */
        {
        Tmarg = Bmarg = 0;
        break;
        }
    Tmarg = Perf_percent * n1 / 100;     /* split perf skip lines between top */
    Bmarg = n1 - Tmarg;                  /* & bottom margins according to the */
    break;                                             /* spec'd perf_percent */

case 'O':                                            /* Kill perforation skip */
    Tmarg = Bmarg = 0;
    break;

case 'P':                                      /* Proportional spacing on/off */
    CHARIN(n1);
    Prop_spacing = (n1 & 0x01);
    CMDOUT(SET_PROPSP_CMD);
    break;

case 'Q':                                 /* Deselect on positive query reply */
    CHARIN(n1);
    break;

case 'R':                                     /* Reset horiz & vert tab stops */
    tabs = (&Vert_tabs)->ptr;
    piomode = 0;
    strcpy(tabs,Vert_tabs);
    piomode = 1;

    tabs = (&Horz_tabs)->ptr;
    piomode = 0;
    strcpy(tabs,Horz_tabs);
    piomode = 1;
    break;

case 'S':                                        /* Superscript / subsript on */
    CHARIN(n1);
    if (n1 & 0x01)
        {
        Subscr_mode = TRUE;
        CMDOUT(SET_SUB_CMD);
        }
    else
        {
        Superscr_mode = TRUE;
        CMDOUT(SET_SUPER_CMD);
        }
    break;

case 'T':                                       /* Superscript / subsript off */
    Subscr_mode = Superscr_mode = FALSE;
    CMDOUT(SET_SUB_CMD);
    break;

case 'U':                                       /* Bi/unidirectional printing */
    CHARIN(n1);
    L_to_r_print = (n1 & 0x01);         /* n1=1 sets uni-directional printing */
    CMDOUT(LR_PRINT_CMD);
    break;

case 'W':                                      /* Continuous double wide mode */
    CHARIN(n1);
    Line_dblwide = FALSE;
    Cont_dblwide = (n1 & 0x01);
    SET_Pitch(Pitch);
    CMDOUT(CONT_DW_CMD);
    break;

case 'X':                                           /* Set horizontal margins */
    CHARIN(n1);
    CHARIN(n2);
    if (n2) SET_Pgwidth(n2);
    if (n1) x_indent = (n1 - 1) * Hincr;
    CMDOUT(SET_MAR_CMD);
    break;

case 'Y':                                 /* 120 DPI Graphics (normal speed) */
    CHARIN(n1);
    CHARIN(n2);
    cnt = n1 + (n2 * 256);
    CMDOUT_PT(GRA_120NS_CMD, cnt);
    hpos += (cnt / 120) * Hres;
    break;

case 'Z':               /* 240 DPI Graphics (high density bit-image graphics */
    CHARIN(n1);
    CHARIN(n2);
    cnt = n1 + (n2 * 256);
    CMDOUT_PT(GRA_240_CMD, cnt);
    hpos += (cnt / 240) * Hres;
    break;

/*---------------------------------------------------------------------------*/

case '\\':                          /* Print multiple chars from ALL CHAR set */
    CHARIN(n1);
    CHARIN(n2);
    cnt = n1 + (n2 * 256);                         /* no. of bytes to be sent */
    value = (Width - hpos)/Hincr;       /* no. of chars that will fit on line */

    if (value < cnt)          /* if # that will fit is less than # to be sent */
        {
        CMDOUT_PT(PRTALL_CMD,value);
        if (Wrap)                  /* if wrap on then the command and the no. */
            {                      /* of remaining chars must be stuffed back */
            cnt -= value;                  /* into the buffer to be sent next */
            n2 = cnt / 256;         /* decr CNT by the no. of chars just sent */
            n1 = cnt % 256;
            UNCHARIN(n2);           /* stuff char count remainder into buffer */
            UNCHARIN(n1);
            UNCHARIN(92);                /* stuff ESC 'backslash' into buffer */
            UNCHARIN(27);
            return(EOL);         /* tell pioformat we're at the end of a line */
            }
        else for (;value<cnt;value++) CHARIN(c2)               /* wrap is off -
                                                           toss rest of chars */
        }

    else                /* all those to be sent will fit on the line we're on */
        {
        CMDOUT_PT(PRTALL_CMD,cnt);
        hpos += Hincr * cnt;
        }
    break;

case ']':                                                /* Reverse line feed */
    if ((vpos - Vdecr) >= vtab_base) vpos -= (Vincr + Vdecr) * Linespacing;
    Line_dblwide = FALSE;           /* Vincr * Linespacing is subtracted from */
    SET_Pitch(Pitch);               /* because it will be added right back in */
    return(EOL);                                   /* by the LINEFEED routine */

case '^':                              /* Print single char from ALL CHAR set */
    if ((hpos + Hincr) > Width)
        {
        UNCHARIN('^');
        UNCHARIN(27);
        return(EOL);
        }
    CMDOUT_PT(PRTNEXT_CMD,1);
    hpos += Hincr;
    break;

case '_':                                      /* Continuous overscore on/off */
    CHARIN(n1);
    Cont_overscore = (n1 & 0x01);
    CMDOUT(OVERSCORE_CMD);
    break;

case 'd':                                     /* Relative move inline forward */
    CHARIN(n1);                               /* get number of 120ths to move */
    CHARIN(n2);
    cnt = ((n1 + (n2 * 256)) * Hres) / 120;        /* turn it into Hres units */
    value = Width - hpos;                      /* how far to the right margin */
    Workint1 = (value < cnt ? value : cnt);      /* use the lesser of the two */
    hpos += Workint1;                          /* reduce hpos by the movement */
    Workint1 = (Workint1 * 120) / Hres;       /* convert movement into 120ths */
    CMDOUT(MOVE_RIGHT_CMD);
    break;

case 'e':                                   /* Relative move inline backward  */
    CHARIN(n1);                               /* get number of 120ths to move */
    CHARIN(n2);
    cnt = ((n1 + (n2 * 256)) * Hres) / 120;        /* turn it into Hres units */
    value = hpos - c_indent;                    /* how far to the left margin */
    Workint1 = (value < cnt ? value : cnt);      /* use the lesser of the two */
    hpos -= Workint1;                          /* reduce hpos by the movement */
    Workint1 = (Workint1 * 120) / Hres;       /* convert movement into 120ths */
    CMDOUT(MOVE_LEFT_CMD);
    break;

case 'j':                                                    /* Stop printing */
    CMDOUT(STOP_PRT_CMD);
    break;

case 'n':                                              /* Select aspect ratio */
    CHARIN(n1);
    Workint1 = (n1==2 ? 2 : 1);
    CMDOUT(SET_ASPRAT_CMD);
    break;

case '*':                       /* Select graphics mode (AGM) - not supported */
    CHARIN(Workint1);
    CHARIN(n1);
    CHARIN(n2);
    cnt = n1 + (n2 * 256);                                      /* byte count */
    CMDOUT_PT(AGMGRAPH_CMD,cnt);
    break;



/*----------------------------------------------------------------------------+
|                                                                             |
|                        Escape [ subset for the 5202                         |
|                                                                             |
+----------------------------------------------------------------------------*/

case '[':
    CHARIN(n1);
    switch((char)n1)
    {
    case '@':                                  /* Double high & wide printing */
        CHARIN(n1);
        CHARIN(n2);
        cnt = n1 + (n2 * 256);                /* get byte count - should be 4 */

        if (cnt) CHARIN(n1);                    /* if cnt at least 1 read nul */
        if (cnt-1) CHARIN(n2);                  /* if cnt at least 2 read nul */

        if (cnt-2)                        /* if cnt at least 3 read next byte */
            {
            CHARIN(n1);                  /* datastream value for ht & spacing */
            if (n1)
                {
                if (n1 & 0x03) Doublehigh = ((n1 & 0x02)?1:0);
                if (n1 & 0x30) Linespacing = ((n1 & 0x20)?2:1);
                CMDOUT(SET_DWDH_CMD);
                }
            }

        if (cnt-3)                        /* if cnt at least 4 read next byte */
            {
            CHARIN(n2);                          /* set char width if changed */
            if (n2)
                {
                Line_dblwide = FALSE;
                Cont_dblwide = ((n2 & 0x02)?1:0);
                SET_Pitch(Pitch);
                CMDOUT(CONT_DW_CMD);
                }
            }
        break;

    case 'C':                                   /* PMP command for the 3816 */
        CHARIN(n1);
        CHARIN(n2);
        cnt = n1 + n2 * 256;
        CMDOUT_PT(PASS_PMP_CMD,cnt);
        Pr_font_stat = FONTS_BAD;
        break;

    case 'F':                                   /* Select sheetfeed options */
					/* Send the esc sequence as is */
        Workint1 = n1;
        CHARIN(n1);
        CHARIN(n2);
        cnt = n1 | n2 << 8;
        CMDOUT_PT(PASS_THRU, cnt);
        break;

    case 'I':                                               /* Select font id */
        CHARIN(n1);                           /* get byte count - should be 5 */
        CHARIN(n2);
        cnt = n1 + (n2 * 256);

        CHARIN(n2);                     /* get font id - note high byte first */
        CHARIN(n1);

/*-----------------------------------------------------------------------------+
|                                                                              |
| NOTE:  The 5202 book suggests that the font id, the font width, and the      |
|        font attribute (fixed/proportional spacing) are independent.  This    |
|        conflicts with other sections of the manual and presents a multi-     |
|        tude of conflicting states in which the printer could be set          |
|                                                                              |
|            e.g. FONT 11; WIDTH 12 cpi; PROPORTIONALLY SPACED  ?!?!?!         |
|                                                                              |
|        We have decided to set our formatter options based on the info        |
|        available from the decoding of the font id, but we will send the      |
|        bytes received on to the printer un-modified in the faint hope        |
|        that the document might work out anyway.                              |
|                                                                              |
+-----------------------------------------------------------------------------*/

        Font_id = (n1 ? n1 + (n2 * 256) : 11);
        Prop_spacing = FALSE;
        Condensed = FALSE;
        if (Font_id >  0   && Font_id <= 43) Pitch = 10;
        if (Font_id >= 66  && Font_id <= 112) Pitch = 12;
        if (Font_id >= 154 && Font_id <= 176) Prop_spacing = TRUE;
        if (Font_id == 204) Pitch = 12;
        if (Font_id >= 215 && Font_id <= 231) Pitch = 15;
        if (Font_id >= 254 && Font_id <= 256) Condensed = TRUE;
        SET_Pitch(Pitch);
        if ( Pitch == 15 ) Pitch = 12;   /* not smart enough to handle 15 cpi */

        CHARIN(n2);                /* get font width - note the hi byte first */
        CHARIN(n1);
        Pitch_fid = n1 + (n2 * 256);

        CHARIN(Spacing_fid);                            /* get font attribute */

        CMDOUT(SEL_FONT_CMD);
        if (Pr_model_num == 3816 && Spacing_fid == 2) SET_Pitch(1440/Pitch_fid);
        break;

    case 'K':                    /* Set initial condition from the datastream */
        CHARIN(n1);
        CHARIN(n2);
        cnt = n1 + n2*256;
        CMDOUT_PT(DS_INIT_CMD,cnt);
        break;

    case 'S':                                   /* Set top and bottom margins */
        CHARIN(n1);
        CHARIN(n2);             /* get byte count - 4 for the 5202 */
        cnt = n1 + (n2 * 256);  /* the 3816 could use 8 bytes */

        CHARIN(n1);             /* get top margin setting in 1440s */
        CHARIN(n2);
        tmarg = n1 + (n2 * 256);
        Tmarg = tmarg / Hincr;
        Bmarg = bmarg = 0;
        if (cnt < 3) break;     /* break if Bot, left & right marg not spec'd */

        CHARIN(n1);             /* get bottom margin setting in 1440s */
        CHARIN(n2);
        if ((value = n1 + (n2 * 256)) < pglen)
            {
            bmarg = value;
            Bmarg = bmarg / Hincr;
            }
        if (cnt < 5) break;     /* break if left & right marg not spec'd */

        CHARIN(n1);             /* get left margin in 1440s */
        CHARIN(n2);
        x_indent = n1 + n2 * 256;
        if (cnt < 7) break;     /* break if right marg not spec'd */

        CHARIN(n1);             /* get left margin in 1440s */
        CHARIN(n2);
        if (value = n1 + (n2 * 256))
            {
            Width = value + c_indent;
            Pgwidth = Width / Hincr + 1;
            }
        CMDOUT(SET_MAR_CMD);
        break;

    case 'T':                                                /* Set code page */
        CHARIN(n1);
        CHARIN(n2);
        CMDOUT(SET_CODEPG_CMD);
        break;

    case '\\':                                          /* Set vertical units */
        CHARIN(n1);                   /* get count */
        CHARIN(n2);
        cnt = n1 + (n2 * 256);

        CHARIN(n1);                   /* get Tunits and make sure the */
        CHARIN(n2);                   /* number is legal              */
        value = n1 + (n2 * 256);
        n3 = value/24;
        if ((n3 >= 2 && n3 <=6) || (n3 >= 9 && n3 <= 10) || (n3 == 60))
            Tunits = value;
        if (Gunit_type == 2 && (value == 180 || value == 300 || value == 720))
            Tunits = value;

        if (cnt > 2)
            {
            CHARIN(n1);               /* get Gunits and make sure the */
            CHARIN(n2);               /* number is legal              */
            value = n1 + (n2 * 256);
            n3 = value/24;
            if ((n3 >= 2 && n3 <=6)||(n3 >= 9 && n3 <= 10)||(n3 == 60))
                Gunits = value;
            if (Gunit_type==2 && (value==180 || value==300 || value==720))
                Gunits = value;
            }

        CMDOUT(SET_VUNITS_CMD);
        break;

    case 'g':                                            /* High res graphics */
        CHARIN(n1);
        CHARIN(n2);
        cnt = n1 + (n2 * 256) - 1;

        CHARIN(Workint1);
        CMDOUT_PT(HIRES_GRA_CMD, cnt);
        if (Workint1 == 0) n2 = 60;
        if (Workint1 == 1) n2 = 120;
        if (Workint1 == 2) n2 = 120;
        if (Workint1 == 3) n2 = 240;
        if (Workint1 == 8) n2 = 60 / 3;           /*  spacing divided by 3 to */
        if (Workint1 == 9) n2 = 120 / 3;            /* account for the 24 pin */
        if (Workint1 == 11) n2 = 180 / 3;       /* printers requiring 3 bytes */
        if (Workint1 == 12) n2 = 360 / 3;               /* per character cell */
        hpos += (cnt / n2) * Hres;
        break;

    default:                                             /* Unknown character */
        Workint1 = n1;
        CHARIN(n1);
        CHARIN(n2);
        cnt = n1 + (n2 * 256);
        CMDOUT_PT(PASS_THRU, cnt);
        break;
    }
    break;

/*----------------------------------------------------------------------------+
|                                                                             |
|                            end of Escape [ subset                           |
|                                                                             |
+----------------------------------------------------------------------------*/

default:
    break;

}
return(0);
}


/*
*******************************************************************************
*******************************************************************************
** NAME:        getmsg()
**
** DESCRIPTION: Replaces the NLgetamsg routine used in 3.1 code  If the catalog
**              is not found in the NLSPATH, it will look for a default in
**              /usr/lib/lpd/pio/etc.
**
** ROUTINES
**   CALLED:    catopen() - gets catalog descriptor
**
**              catgets() - gets message
**              catclose  - closes catalog
**
** PARAMETERS:  catalog name, set number, message number
**
**
*******************************************************************************
*******************************************************************************
*/
static char *msgbuf = NULL;
static nl_catd catd;
static nl_catd def_catd;
static char save_name[100];

char *getmsg(CatName, set, num)
char *CatName;  
int set;
int num;
{
	char *ptr;
	char *nlspath;
	char *defpath = malloc(200);
	char default_msg[100];

	if (strcmp(CatName,save_name) != 0)  /* is it a different catalog */
	{
	catclose(catd);  /* close /usr/lpp message catalog */
	catclose(def_catd);  /* close default message catalog */
	catd = def_catd = (nl_catd)NULL;  /* set catalog descriptors to NULL */
	strcpy(save_name,CatName);
	}

	if (catd != -1)  
		if (catd == 0) /* if it hasn't been open before */
			catd = catopen(CatName,NL_CAT_LOCALE);
	
	if (catd != -1)
		{
		ptr = catgets(catd,set,num,"dummy");
		if (!msgbuf)
			msgbuf = malloc(4001);
		if (msgbuf)
			strncpy(msgbuf, ptr, 4000);
		if (strcmp(ptr,"dummy") != 0)  /* did catgets fail? */
			return(msgbuf);
		}
	
	if (def_catd == 0)
		{
		sprintf(defpath,"/usr/lib/lpd/pio/etc/%s",CatName);
		def_catd = catopen(defpath,NL_CAT_LOCALE);
		}
	
	sprintf(default_msg,"Cannot access message catalog %s.\n",CatName);
	ptr = catgets(def_catd,set,num, default_msg);
	if (!msgbuf)
		msgbuf = malloc(4001);
	if (msgbuf)
		strncpy(msgbuf, ptr, 4000);

	free(defpath);
	return(msgbuf);
}
