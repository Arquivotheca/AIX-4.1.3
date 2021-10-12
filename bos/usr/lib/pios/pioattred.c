static char sccsid[] = "@(#)88  1.6.1.2  src/bos/usr/lib/pios/pioattred.c, cmdpios, bos411, 9428A410j 11/9/93 11:04:15";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
**  NAME: pioattred  (pio-attribute-edit)
**
**      General Invocation:
**              pioattred -q queue -d quedev -o action_code -a attribute
**
**
**          To format specific attributes to stdout
**              pioattred -q queue -d quedev -o 0 -a attribute -a attribute ...
**                  or
**              pioattred -q queue -d quedev -a attribute -a attribute ...
**
**          To format all attributes to stdout
**              pioattred -q queue -d quedev -o 0
**                  or
**              pioattred -q queue -d quedev
**
**      Assumptions:
**          a)  The attribute is a two character attribute name that actually
**              exists in the custom colon file.
**          b)  The action_code will represent one of four possible actions:
**                  1)  This is a fresh start - we've never been here before.
**
**              The remaining options are based on this script having indicated
**              that the user edited the attribute value and hosed it so that
**              this routine's return code indicated failure.
**
**                  2)  Go thru the edit loop again, return the real stuff
**                  3)  Save the bad attribute value anyway, cleanup, return OK
**                  4)  Forget we were ever here, cleanup - return OK
**
**          c)  SMIT will lock the custom colon file so that no other users
**              can be simultaneously fiddling with the file.
**
**          d)  The users favorite editor will be declared in the environment
**              variable VISUAL.  In its absence, vi will be used.
*/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <nl_types.h>     /*  included for getmsg  */
#include "pioattred.h"

struct attr {   char name[3];
                char *ref_file;
                struct attr *next_attr; };

struct line {   char *text;
                struct line *next_line; };

char *getmsg();
char *get_attr_msg();
char *collect();
char *cnvt_colons();

/*===========================================================================*/
/*===========================================================================*/
main (argc, argv)
int argc;
char *argv[];
{
    char *que = NULL;
    char *qdev = NULL;
    int action = 0;

    struct line *l;
    struct attr *chng_attrs = NULL;
    int rc, c, cnt;
    int desired_state = UNDEFINED;
    register struct attr	*tmp_attp;	/* temp ptr to attr list */
    PROHIBIT_ATTRLIST_DEFN;			/* list of prohibited attrs */
    register char		**tmp_prohibit_attp;
					/* temp ptr to prohibited attr list */

    extern char *optarg;
    extern int optind, opterr, optopt, errno;

    (void) setlocale(LC_ALL, "");

    cat_name_mD = (char *)NULL;
    indent = ATTR_INDENT;
    colonf_input = TRUE;

    if ( *argv[1] == '?' )
        {
        fprintf(stderr,
            "\n%s\n\n\a",getmsg(OP_CATALOG,5,CNVT_ATTR_USAGE));
        exit(99);
        }

    *attributes = NULL;

    opterr = 0;

    while (( c = getopt(argc,argv,"q:d:a:o:")) != EOF)
        {
        switch( c )
            {
            case 'q':
                que = optarg;
                break;

            case 'd':
                qdev = optarg;
                break;

            case 'a':
                if (strlen(optarg) < 2)   /* error if attribute is one char */
                  error_exit(CNVT_BADARG,optarg,"");
                else
                  add_attr(&chng_attrs,optarg);
                break;

            case 'o':
                switch ( (int)*optarg )
                    {
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                        action = atoi(optarg);
                        break;

                    default:
                        error_exit(CNVT_BADARG,optarg);
                        break;
                    }
                break;

            case '?':
                error_exit(CNVT_ATTR_USAGE);
                break;
            }
        }

    check_opts(que,qdev,action,&chng_attrs);

    /* Before letting the user edit the specified attributes, make sure that
       attributes for default and current state ("zD" and "zS") are not
       listed among the specified attributes.  If so, display an error message
       and exit (but with 0 value, so as not to upset "lsvirprt" and make
       it go in an infinite loop). */
    for (tmp_attp = chng_attrs; tmp_attp; tmp_attp = tmp_attp->next_attr)
       for (tmp_prohibit_attp = prohibit_attrs; *tmp_prohibit_attp;
	    tmp_prohibit_attp++)
	  if (!strncmp (*tmp_prohibit_attp, tmp_attp->name, ATTRNMLEN))
             fprintf (stderr, getmsg (OP_CATALOG, 6, CNVT_ATTR_NONEDITABLE),
		      *tmp_prohibit_attp),
             exit (EXIT_SUCCESS);

    if ( action == 0 ) rc = format_the_world(que,qdev,&chng_attrs);
    if ( action == 1  || action == 2 ) rc = start_fresh(que,qdev,&chng_attrs);
    if ( action == 3 ) rc = write_anyway(que,qdev,&chng_attrs);
    if ( action == 4 ) rc = clean_up(que,qdev);

    return( rc );
}

/*============================================================================*/
int format_the_world(que,qdev,chng)
char *que, *qdev;
struct attr **chng;
/*
**  I'm going to make the file big then load it into memory.  I'm also going
**  to point output to stdout.
**
**  Now - if attributes were identified on the command line as being of specific
**  interest, I'll look thru the file a line at a time and compare the attribute
**  name in the line with all the attributes on the command line.  When I find
**  a match, I'll submit that line to 'format()' so it can lex and yacc the
**  line.
**
**  If no attributes were entered on the command line, every line in the file
**  will be submitted to 'format()' to be written to stdout.
*/

#define BREAK puts("------------------------------------------------------")

{
    struct line *l;
    struct attr *a;
    char in_file[100], cmd[200], *ptr;

    /*
    **  Make sure the custom colon file is big
    */
        ptr = (char *)getenv("PIOBASEDIR");
        if (ptr == NULL)  
           {
           ptr = (char *)malloc(25);
           strcpy(ptr,DEFVARDIR);
           }
    sprintf(cmd,"%s -s+ -i%s/custom/%s:%s",PIOCNVT,ptr,que,qdev);
    system(cmd);

    /*
    **  Load the file into memory
    */
    sprintf(in_file,"%s/custom/%s:%s",ptr,que,qdev);
    l = file = get_file(in_file,chng);
    out = stdout;

    while ( l )
        {
        if ( a=*chng )
            {
            char attr[3];

            ptr = l->text;
            while ( *ptr++ != ':' );
            while ( *ptr++ != ':' );

            /*
            **  Get attribute name
            */
            attr[0] = *ptr++;
            attr[1] = *ptr;
            attr[2] = NULL;

            do{ if ( !strcmp(a->name,attr) )
                    {
                    BREAK;
                    format(l->text);
                    a = NULL;
                    }
                else a = a->next_attr;
                } while ( a );
            }
        else
            {
            BREAK;
            format(l->text);
            }
        l = l->next_line;
        }

    return( 0 );
}

/*============================================================================*/
int start_fresh(que,qdev,chng)
char *que, *qdev;
struct attr **chng;
/*
**  Here we start an editing loop.  Recall that a user can include several
**  attributes to be edited on the command line.  That imposes a looping
**  structure on the routine along with some mechanism for bailing out.  Relax
**  while I try to explain what I did here.
**
**  First, I'll make the file big.  It will probably already be that way, but
**  since there's a possibility that it might not be, I have to make sure.
**
**  Then I go get the whole colon file and hold it in memory. (Very expensive,
**  but since I've already taken several performance hits with all the 'system'
**  calls, the performance improvement was worth it.)
**
**  Now, one line at a time, I go thru the colon file and compare the attribute
**  in the line with all the attributes defined on the command line as being
**  targeted for editing.  If I find a match, I submit the line to the
**  'format()' routine which lexes and yaccs the line, writing its output to
**  'out', which is defined as a temp file before 'format()' is called.
**
**  Now the users favorite editor is invoked on the temp file.  This gives
**  the user the opportunity to edit the formatted attribute value.
**
**  The edited temp file is the submitted to 'unformat()' which read the
**  file and tries to make sense of what the user did to the attribute
**  value.  If 'unformat()' detects no errors, the new attribute value will
**  be copied over the old, and the complete and revised colon file will be
**  written onto the disk in the space previously occupied by the unedited
**  file.  If 'unformat()' detects an error, the erroneous attribute value
**  will be written into another temp file so that it can be recovered if the
**  user decides to try to edit it again.  In the event of a failure, the
**  loop is terminated.
**
**  Finally, if we went thru the whole process successfully, the revised
**  colon file will be digested then converted back to the state desired by
**  the user.
**
**  NOTE:   Attributes are not necessarily going to be displayed for editing
**          in the order they appear on the command line.  The process
**          described above calls for each line in the file to be compared
**          to all the attributes shown on the command line.  That means
**          that attributes will be made available for editing in the order
**          in which thay are found in the file.
**
**  When 'action == 2', the user is saying, in effect, 'I tried to edit an
**  attribute once befor, but I screwed it up.  Put me back where I left
**  off.'  Down in 'check_opts()' I already noted that there was a file
**  in the /tmp directory with an appropriate name and added it in as the
**  ref_file for the appropriate attribute.  Now, when I read the colon
**  file in, go out and get the users last edit file.
*/
{
    struct line *l;
    struct attr *a;
    char in_file[100], cmd[1000], line[1000], *ptr, *var_ptr;
    int success = TRUE;
	char *tmp_ptr;  /* contains value returned by cnvt_colons */
    int rc;
    /*
    **  Make sure the custom colon file is big
    */

    ptr = (char *)getenv("PIOBASEDIR");
    if (ptr == NULL)  
      {  /* if no PIOBASEDIR, use default var directory */
      ptr = (char *)malloc(25);
      strcpy(ptr,DEFVARDIR); 
      }
    sprintf(cmd,"%s -s+ -i%s/custom/%s:%s",PIOCNVT,ptr,que,qdev);
    system(cmd);
    /*
    **  Load the file into memory
    */
    sprintf(in_file,"%s/custom/%s:%s",ptr,que,qdev);
    file = get_file(in_file,chng);

    if ( l=file )
        {
        char attr[3];
        do{ ptr = l->text;
            while ( *ptr++ != ':' );
            while ( *ptr++ != ':' );

            /*
            **  Get attribute name
            */
            attr[0] = *ptr++;
            attr[1] = *ptr;
            attr[2] = NULL;

            if ( a=*chng )
                {
                do{ if ( !strcmp(a->name,attr) )
                        {
                        char out_file[100];

                        /*
                        **  A match was found.  Open output to a tmp
                        **  file, then call 'format()' with the line to
                        **  be formatted.
                        */
                        sprintf(out_file,"/tmp/%s:%s.%s",que,qdev,a->name);
                        out = fopen(out_file,"w");
                        format(l->text);
                        fclose(out);

                        /*
                        **  Call the users favorite editor to edit the 
                        **  tmp file.
                        */
                        ptr = (char *)getenv("VISUAL");
                        sprintf(cmd,"%s %s",ptr ? ptr : "vi",out_file);
                        rc = system(cmd);
                        if( (success = !rc) &&
                            ( success=!unformat(out_file,line) ) )
                            {
                            int cnt = 0;
                            strcpy(cmd,l->text);
                            ptr = cmd;
                            
                            /*
                            **  Count to the fourth colon, then copy the
                            **  new attribute value over the old.
                            */
                            for(cnt=0 ; cnt<4 ; cnt++) while ( *ptr++ != ':' );
                            *ptr = NULL;
							tmp_ptr = cnvt_colons(line); /* convert all colons to \072 */
                            strcat(cmd,collect(tmp_ptr,"%I"));

                            free(l->text);
                            l->text = (char *)malloc( strlen(cmd)+1 );
                            strcpy(l->text,cmd);
                                {
                                /*
                                **  Write out the successfully edited colon
                                **  file.  This will overlay the existing file.
                                */
                                FILE *new = fopen(in_file,"w");
                                struct line *nl = file;
                                while ( nl )
                                    {
                                    fprintf(new,"%s\n",nl->text);
                                    nl = nl->next_line;
                                    }
                                fclose(new);
                                }
                            }
                        a = NULL;
                        }
                    else a = a->next_attr;
                    } while ( a && success );
                }
            l = l->next_line;
            } while ( l && success );

        if ( success )
            {
            ptr = (char *)getenv("PIOBASEDIR");
            if (ptr == NULL)  
               {
                ptr = (char *)malloc(25);
                strcpy(ptr,DEFBASEDIR);
                var_ptr = (char *)malloc(25);
                strcpy(var_ptr,DEFVARDIR);
                }
            else
                var_ptr = ptr;
            sprintf(cmd,"%s/etc/piodigest -q%s -d%s %s/custom/%s:%s",
                    ptr, que, qdev,var_ptr, que, qdev);
            if (system(cmd))	/* if errors in digesting, return so */
	       return EXIT_FAILURE;
            rc = clean_up(que,qdev);
            }
        }

    return( !success );
}

/*============================================================================*/
int write_anyway(que,qdev,chng)
char *que, *qdev;
struct attr **chng;
/*
**  The user tried once before, to edit an attribute, but he screwed it up.
**  Since he screwed it up, I didn't write it into the custom colon file, but
**  now I've been explicitly told to do just that.  I'll first tag the first
**  attribute in the chain with the tmp file name where the users gorp is
**  stored so that, when the file is loaded, his stuff will be also be loaded
**  in.
*/
{
    char in_file[100], cmd[1000], *ptr;

    /*
    **  Make sure the custom colon file is big
    */
    ptr = (char *)getenv("PIOBASEDIR");
    if (ptr == NULL)  
      {
      ptr = (char *)malloc(25);
      strcpy(ptr,DEFVARDIR);
      }
    sprintf(cmd,"%s -s+ -i%s/custom/%s:%s",PIOCNVT,ptr,que,qdev);
    system(cmd);

    /*
    **  Load the file into memory
    */
    sprintf(in_file,"%s/custom/%s:%s",ptr,que,qdev);

    if ( file=get_file(in_file,chng) )
        {
        /*
        **  Write out colon file.  This will overlay the existing file.
        */
        FILE *new = fopen(in_file,"w");
        struct line *nl = file;
        while ( nl )
            {
            fprintf(new,"%s\n",nl->text);
            nl = nl->next_line;
            }
        fclose(new);
        }

    return( clean_up(que,qdev) );
}
        
/*============================================================================*/
int clean_up(que,qdev)
char *que, *qdev;
/*
**  This routine deletes temporary files which may have been created
**  and leaves the custom colon file in the state desired by the user.
*/
{
    char cmd[1000], *ptr;

    ptr = (char *)getenv("PIOBASEDIR");
    if (ptr == NULL)  
       {
       ptr = (char *)malloc(25);
       strcpy(ptr,DEFVARDIR);
       }
    sprintf(cmd,"%s -i%s/custom/%s:%s; /bin/rm -f /tmp/%s:%s.*",
                PIOCNVT,ptr,que,qdev, que,qdev);
    system(cmd);

    return( 0 );
}

/*============================================================================*/
char *collect(instr,txt)
char *instr;
char *txt;
/* this routine is called to take an attribute string and collect common
   commands (specified by txt) and use brackets to group them.  For example
   the input attribute string  %Idd%Iff%Iss would be changed to %I[dd,ff,ss]
*/

{
   char *endline;
   char *outstr = malloc(1000);
   int sequence;
   char *ptr;
   char *out_ptr;
   
   sequence = FALSE;
   endline = (char *)((int)instr + strlen(instr));
   ptr = instr;   /* initialize pointers */
   out_ptr = outstr;

   if (strstr(instr,txt) == NULL) {  /* txt not found in string */
      strcpy(outstr,instr);
      return(outstr);
   }
   
   while (*ptr != NULL) {
      if ( (*ptr == txt[0]) && (ptr < endline - 1) && (*(ptr + 1) == txt[1]) && 
          !((ptr > instr) && *(ptr - 1) == '%')) {
         if (!sequence) {   /*  not in sequence but might be starting one */
            if ( (ptr < endline - 7) && (*(ptr + 4) == txt[0]) &&
                (*(ptr + 5) == txt[1]))  {
               sequence = TRUE;
               *out_ptr++ = txt[0];
               *out_ptr++ = txt[1];
               *out_ptr++ = '[';
               *out_ptr++ = *(ptr+2);
               *out_ptr++ = *(ptr+3);
               ptr += 4;
            }
            else {  /* just copy the txt, not a sequence */
               *out_ptr++ = *ptr;
               *out_ptr++ = *(ptr+1); 
               *out_ptr++ = *(ptr+2);
               *out_ptr++ = *(ptr+3);
               ptr += 4;
            }
         }
         else { /* in sequence */
            *out_ptr++ = ',';
            *out_ptr++ = *(ptr+2); 
            *out_ptr++ = *(ptr+3); 
            if ( (ptr > endline - 5) || (*(ptr + 4) != txt[0]) ||
               (*(ptr + 5) != txt[1])) {
               *out_ptr++ = ']';     /* end of sequence */
               sequence = FALSE;
            }
            ptr += 4;
         }
      }
      else  /* just copy character */
         *out_ptr++ = *ptr++;
   }
   *out_ptr = '\0';   /* terminate string */

   return(outstr);
}

/*============================================================================*/
char *cnvt_colons(str)
char *str;
/*  This routine takes a string and converts any colons
    to the 4 characters '\072'. */
{
	char *out_string;
	char *ptr;

	out_string = (char *)calloc(2000,sizeof(char));
	ptr = out_string;

	while (*str) {
		if (*str == ':') { /* replace ':' with \072 */
		  strcat(out_string,"\\072");
		  ptr += 4;  /* move pointers */
          str++;
		}
		else /* not ':' so just copy character */
			*ptr++ = *str++;
	}

    return(out_string);

}

/*============================================================================*/
int format(line)
char *line;
/*
**  Here, I'll set some error counters, set disp to TRUE (so that 'yyparse()'
**  will do its thing), then go.
*/
{
    char attr[3];

    inptr = line;
    while ( *inptr++ != ':' );
    while ( *inptr++ != ':' );

    /*
    **  Get attribute name
    */
    attr[0] = *inptr++;
    attr[1] = *inptr++;
    attr[2] = NULL;

    /*
    **  Skip processing if the line found is a comment line
    */
    if ( !strcmp(attr,"__") ) return( 0 );

    while ( *inptr++ != ':' );
    while ( *inptr++ != ':' );

    unptr = unbfr;
    *unptr = NULL;
    disp = TRUE, disperronly = parseerr = FALSE;
    if_while = synt_err = 0;

    if ( *inptr )
        {
        fprintf(out,"\n%s\n%s = %s\n\n",get_desc(line),attr,inptr);
        yyparse();

        if ( if_while )
            fprintf(out,getmsg(OP_CATALOG,5,OP_BAD_IF_WHILE));

        if ( synt_err )
            fprintf(out,getmsg(OP_CATALOG,5,OP_ERRORS),synt_err);
        }
    else fprintf(out,"\n%s\n%s =\n\n",get_desc(line),attr);

    return ( if_while + synt_err );
}

/*============================================================================*/
int unformat(path,new_attr_val)
char *path, *new_attr_val;
/*
**  Once the file has been read and 'new_attr_val' is complete, the
**  global variable 'inptr' is set to point to 'new_attr_val.  So
**  that only syntax checking will be done, 'disp' is set to FALSE.  This
**  will suppress the normal processing done in 'yyparse()'.  'yyparse()'
**  is then called.  The total of if_while and syntax errors are reported
**  back as the return code.
*/
{
    int rc;

    read_edit(path,new_attr_val);

    /*
    **  Quit here if there's nothing to parse
    **
    **  Yes - I know it's bad programming practice to jump out of a sub, but
    **  it makes perfect sense here, so why not?!?
    */
    if ( !(*new_attr_val) ) return( 0 );

    disp = disperronly = parseerr = FALSE;
    inptr = new_attr_val;
    unptr = unbfr;
    *unptr = NULL;
    if_while = synt_err = 0;
    yyparse();

    if (( if_while + synt_err ) && (getenv("PIOLSVIRPRT") == NULL))
        {
        if ( if_while )
            fprintf(stderr,getmsg(OP_CATALOG,5,OP_BAD_IF_WHILE));

        if ( synt_err )
            fprintf(stderr,getmsg(OP_CATALOG,5,OP_ERRORS),synt_err);

        putc('\n',stderr);
        fprintf(stderr,getmsg(OP_CATALOG,5,OP_NOT_WRITTEN));
        }
    return ( if_while + synt_err );
}

/*============================================================================*/
void add_attr(b,atbt)
struct attr **b;
char *atbt;
/*
**  Adds an attribute to the end of the list.
**
**  An interesting wrinkle:  Usually, the value passed in with the -a
**  flag will be simply a two character attribute name.  However!  Sometimes
**  the attribute name will be accompanied by a field identifying the file 
**  from which the attribute value field is to be read.  The subroutine
**  argument 'atbt', which is the -a option argument, then, has a simple
**  and a complex form.
**
**      simple form:    aa
**      complex form:   aa,file_name
**
**  The presence or absence of a third character is the determinant.  If
**  a third character is found, it is replaced with a NULL and the remaining
**  characters are copied into the ref_file element of the attribute structure.
**  If no third character is found, the pointer to the ref_file element is
**  made NULL.
**
**  In either case, the attribute name is copied into the appropriate element
**  of the attribute structure.
*/
{
    struct attr *h, *new, *base = *b;

    new = (struct attr *)malloc( sizeof(struct attr) );
    new->next_attr = NULL;

    if ( *(atbt+2) )
        {
        *(atbt+2) = NULL;
        strcpy(new->name,atbt);
        new->ref_file = (char *)malloc( strlen(atbt+3)+1 );
        strcpy(new->ref_file,atbt+3);
        }
    else
        {
        strcpy(new->name,atbt);
        new->ref_file = NULL;
        }

    if ( !base ) base = *b = new;
    else
        {
        h = base;
        while ( h->next_attr ) h = h->next_attr;
        h->next_attr = new;
        }
}
        
/*============================================================================*/
struct line *get_file(path,change)
char *path;
struct attr **change;
/*
**  Open the file defined by the path.  Error_exit if the open fails.
**
**  Read the file in a line at a time.  If the attribute name in the
**  line matches an attibute found in the list of attributes entered on the
**  command line, then save the whole line.  Otherwise, strip off the
**  attribute value string, which is the stuff after the fourth colon.
**
**  As discussed above, the command line can indicate the file from
**  which a particular arribute value string is to be taken.  If the
**  ref_file element of the attribute structure is !NULL, then the
**  appropriate file will be opened and its contents sucked in one
**  character at a time until the file is empty.  The characters will
**  overlay any characters already there.
*/
{
    FILE *inp;
    char attr[3], line[1000], *lnptr = line;
    struct attr *a;
    struct line *l, *new, *file = NULL;
    if( !(inp = fopen(path,"r")) ) error_exit(CNVT_NOFILE,path);

    while ( !fgetln(inp,line) )
        {
        lnptr = line;
        while ( *lnptr++ != ':' );
        while ( *lnptr++ != ':' );

        /*
        **  Get attribute name
        */
        attr[0] = *lnptr++;
        attr[1] = *lnptr++;
        attr[2] = NULL;

        while ( *lnptr++ != ':' );
        while ( *lnptr++ != ':' );

        if (!strcmp(attr,"mD"))
           {  /* set cat_name_mD for use by get_desc()   */
           cat_name_mD = (char *)malloc(strlen(lnptr) + 1);
           strcpy(cat_name_mD,lnptr);
           }

        if ( a = *change )
            {
            do{ if( !strcmp(a->name,attr) )
                    {
                    if ( a->ref_file ) read_edit(a->ref_file,lnptr);
                    a = NULL;
                    }
                else a = a->next_attr;
                } while ( a );
            }

        new = (struct line *)malloc( sizeof(struct line) );
        new->text = (char *)malloc( strlen(line)+1 );
        strcpy(new->text,line);
        new->next_line = NULL;

        if ( !file ) l = file = new;
        else
            {
            l->next_line = new;
            l = new;
            }
        }

    fclose( inp );
    return( file );
}

/*============================================================================*/
void read_edit(path,line)
char *path, *line;
/*
**  This guy is going to open a the named file and read thru it in hopes
**  of ending up with a valid attribute value string.  This file should
**  have been originally written by 'format()', then edited by the user.
**
**  The objective here is to isolate the operative part of the attribute
**  value string (as it might have been edited) from comments left over
**  from the decoding of the original string.
**
**  We'll do it with two pointers.  First, note this rule:  A valid line
**  must start with either a percent character (indicating an operator) or
**  a single quote (indicating non-operator text).
**
**  The first pointer, b_ptr, skips over any whitespace in the line.  If
**  the first character found is neither % nor ' the line is skipped.  If
**  the first character is %, the second pointer, e_ptr, is moved forward
**  from b_ptr until a space is found.  Said space is then replaced with
**  a NULL and the string whose beginning is marked by b_ptr is concatenated
**  to the string 'new_attr_val'.
**
**  If the first character found is a single quote, e_ptr is moved to the
**  end of the line.  It is then moved backwards until it finds a single
**  quote.  This single quote is assumed to match the one pointed to by
**  b_ptr, but does not necessarily do so.  b_ptr is advanced by one
**  so as to omit the leading single quote.  The character pointed to by
**  e_ptr (also a single quote) is relpaced with a NULL.  As above, the
**  isolated string is added to 'new_attr_val'.
*/
{
    FILE *inp;
    char tmp_line[1000], *b_ptr, *e_ptr;

    if( !(inp = fopen(path,"r")) ) error_exit(CNVT_NOFILE,path);

    *line = NULL;

    while ( !fgetln(inp,tmp_line) )
        {
        b_ptr = tmp_line;
        while ( *b_ptr == ' ' || *b_ptr == '\t' ) b_ptr++;
        if ( *b_ptr == '%' || *b_ptr == '\'' )
            {
            e_ptr = b_ptr;
            if ( *b_ptr == '%' )
                {
                while ( *e_ptr++ != ' ' );
                *(--e_ptr) = NULL;
                }
            else
                {
                b_ptr++;
                while ( *e_ptr ) e_ptr++;
                while ( *e_ptr != '\'' ) e_ptr--;
                *e_ptr = NULL;
                }
            while( *b_ptr ) *line++ = *b_ptr++;
            *line = NULL;
            }
        }
    fclose(inp);
}

/*============================================================================*/
int fgetln(in,line)
FILE *in;
char *line;
/*
**    Get a single line from the specified file.
*/
{ int i = 0;
  char ch;

  while ( (ch = fgetc(in) ) != '\n' && !feof(in) ) line[i++] = ch;
  line[i] = NULL;

  return feof(in) ? EOF : 0 ;
}

/*============================================================================*/
void show_op(int msg_num, char *oper, int type, char *str, int chr)
/*
**  Displays the attribute string and description string.
**  Also prints the argument string (or character) passed in into the
**  description string read from the description catalog.
*/
{
    char desc[1000];

    if ( msg_num)
        {
        if ( type == NO_FIELD )
            strcpy(desc,getmsg(OP_CATALOG,5,msg_num));
        if ( type == STR_FIELD )
            sprintf(desc,getmsg(OP_CATALOG,5,msg_num),str);
        if ( type == CHR_FIELD )
            sprintf(desc,getmsg(OP_CATALOG,5,msg_num),(char)chr);
        if ( type == NUM_FIELD )
            sprintf(desc,getmsg(OP_CATALOG,5,msg_num),chr);
        }
    else *desc = NULL;

    if (!disperronly  ||  parseerr)
       line_print(indent,oper,desc);
    parseerr = FALSE;
}

/*============================================================================*/
char *find_desc(char *atbt)
/*
**  This routine calls 'find_attr()' to locate the line in the file which
**  contains the desired attribute.  If found, it calls 'get_desc()' to return
**  the description from the pioattrX.cat message catalog.  If not found,
**  the 'attr not found' message is displayed.
*/
{
    char *rc;

    if( *atbt == '@' ) return( get_attr_msg(MSG_CATALOG,2,atoi(atbt+1)) );

    rc = find_attr(atbt);
    return( rc
            ? get_desc( rc )
            : getmsg(OP_CATALOG,5,OP_ATTR_NOT_FOUND) );
}

/*============================================================================*/
char *find_attr(atbt)
char *atbt;
/*
**  This routine locates the line containing the attribute name within
**  the file.
*/
{
    struct line *l = file;
    char attr[5], *lnptr;
    int attr_found = FALSE;

    sprintf(attr,":%s:",atbt);

    do{ lnptr = l->text;
        if ( strstr(lnptr,attr) )
            {
            attr_found = TRUE;
            l = NULL;
            }
        else l = l->next_line;
        } while ( l );

    return( attr_found ? lnptr : NULL );
}

/*============================================================================*/
char *get_desc(line)
char *line;
/*
**  This routine goes off into the attribute description catalog 
**  and retrieves the description string associated with that
**  attribute, or an 'attribute no referenced' message if not found.
*/
{
    char cat_name[50], *lnptr = line;
    int msg_num;
    char *colonptr;
    int str_length;
    char cat_char;
    /*
    **  Get attribute message catalog number
    */

    colonptr = strchr(lnptr,':');
    str_length = (int)colonptr - (int)lnptr;

    if (str_length == 0)
       strcpy(cat_name,cat_name_mD);  /* get catalog name from mD attr */
    else if (str_length == 1)
       {
       cat_char = *lnptr;
       sprintf(cat_name,"pioattr%c.cat",cat_char);
       }
    else
       {
       strncpy(cat_name,lnptr,str_length);
       cat_name[str_length] = '\0';
       }
    /*
    **  Get message number
    */
    while ( *lnptr++ != ':' );
    if ( *lnptr == ':' ) msg_num = 0;
    else msg_num = atoi(lnptr);

    return( get_attr_msg(cat_name,1,msg_num) );  /* get message */
}
        
/*============================================================================*/
void line_print(attr_indent,attr,desc)
int attr_indent;
char *attr, *desc;
/*
**  This subroutine takes the indentation value, the attribute field, and the
**  description.  It then prints the attribute field in the left column, then
**  description in the right.  The description is wrapped in the right colomn
**  as needed.
**
**  NOTE:   No consideration has been given to an attribute string which is so
**          long that it prevents the description from starting on the same
**          line.  I'll come back to this later.
*/
{
    int cnt, msg_indent, msg_col, vbl = 0;
    char format[1000], *ptr, *ptr1;

    ptr = attr;                 /* Must first count '%'s in the attribute */
    ptr1 = format;              /* string.  They must be doubled in order */
    while ( *ptr )              /* to be printed correctly */
        {
        if ( *ptr == '%' )
            {
            *ptr1++ = '%';
            vbl += 1;
            }
        *ptr1++ = *ptr++;
        }
    *ptr1++ = *ptr++;
    strcpy(attr,format);

/*
**  Figure out where the description text must start in order not overlap
**  the attribute string.  vbl is a count of the number of '%'s in the
**  attribute string
*/

    cnt = attr_indent + strlen(attr);
    msg_indent = cnt<MSG_INDENT ? MSG_INDENT-cnt+vbl : 1+vbl;
    msg_col = cnt<MSG_INDENT ? MSG_INDENT : cnt+1;
    sprintf(format,"%%-%ds%s%%-%ds%%s\n",attr_indent,attr,msg_indent);
    ptr = ptr1 = desc;

/*
**  On the first pass thru this loop, print the attribute string and the
**  first part of the description.  Then create a new format string which
**  does not include the attribute string.  Use it to print the remainder
**  of the description (if any)
*/

    do{ for ( cnt = msg_col ; cnt<WIDTH-1 && *ptr1 ; cnt++, ptr1++ );
        vbl = FALSE;
        if ( *ptr1 )
            {
            while ( *ptr1 != ' ' ) ptr1--;
            *ptr1 = NULL;
            vbl = TRUE;
            }
        if ( ptr == desc )
            {
            fprintf(out,format," "," ",desc);
            sprintf(format,"%%-%ds%%s\n",msg_col);
            }
        else fprintf(out,format," ",ptr);
        if ( vbl ) ptr = ++ptr1;
        else ptr = ptr1;
        } while ( *ptr );
}

/*============================================================================*/
void check_opts(que,qdev,act,chng)
char *que, *qdev;
int act;
struct attr **chng;
/*
**  Here we check the validity of the flags passed in.  Certain combinations
**  of flags are ambiguous or invalid and will be rejected.
**
**  Action  Bombs if:
**      0       The custom colon file does not exist
**
**      1       The custom colon file does not exist
**              User does not have root permission
**              No attributes are given on the command line.
**
**      2       The custom colon file does not exist
**              User does not have root permission
**              No attributes are given on the command line.
**              The files '/tmp/(que):(qdev).[ab]' do not exist
**
**      3       The custom colon file does not exist
**              User does not have root permission
**              No attributes are given on the command line.
**              The files '/tmp/(que):(qdev).[ab]' do not exist
**
**      4       User does not have root permission
*/
{
    char f_name[100], tmp_file[100], *ptr;
    struct stat buffer;
    struct attr *a = *chng;

    if ( !que || !qdev ) error_exit(CNVT_ATTR_USAGE);


    ptr = (char *)getenv("PIOBASEDIR");
    if (ptr == NULL)  
       {
       ptr = (char *)malloc(25);
       strcpy(ptr,DEFVARDIR);
       }
    sprintf(f_name,"%s/custom/%s:%s",ptr,que,qdev);

    switch ( act )
        {
        case 0:
            if( stat(f_name,&buffer) ) error_exit(CNVT_NOFILE,f_name);
            break;

        case 1:
            if( stat(f_name,&buffer) ) error_exit(CNVT_NOFILE,f_name);
            if( geteuid() ) error_exit(CNVT_NOT_ROOT);
            if( !a ) error_exit(CNVT_NO_ATTRS);
            break;

        case 2:
        case 3:
            if( stat(f_name,&buffer) ) error_exit(CNVT_NOFILE,f_name);
            if( geteuid() ) error_exit(CNVT_NOT_ROOT);
            if( !a ) error_exit(CNVT_NO_ATTRS);
            else
                {
                int found = FALSE;
                while ( a )
                    {
                    sprintf(tmp_file,"/tmp/%s:%s.%s",que,qdev,a->name);
                    if( !stat(tmp_file,&buffer) )
                        {
                        a->ref_file = (char *)malloc( strlen(tmp_file)+1 );
                        strcpy(a->ref_file,tmp_file);
                        found = TRUE;
                        }
                    a = a->next_attr;
                    }
                if( !found ) error_exit(CNVT_NO_TRACE);
                }
            break;

        case 4:
            if( geteuid() ) error_exit(CNVT_NOT_ROOT);
            break;
        }

}

/*============================================================================*/
void error_exit(msg_num,f1,f2)
int msg_num;
char *f1, *f2;
{
    switch( msg_num )
        {
        case CNVT_ATTR_USAGE:
        case CNVT_NO_ATTRS:
        case CNVT_NOT_ROOT:
        case CNVT_NO_TRACE:
            fprintf(stderr,getmsg(MF_PIOBE,6,msg_num,NO_CAT));
            break;

        case CNVT_NOFILE:
        case CNVT_BADOPT:
            fprintf(stderr,getmsg(MF_PIOBE,6,msg_num,NO_CAT),f1);
            break;

        case CNVT_BADARG:
            fprintf(stderr,getmsg(MF_PIOBE,6,msg_num,NO_CAT),f1,f2);
            break;
        }

    exit( msg_num );

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


/*
*******************************************************************************
*******************************************************************************
** NAME:        get_attr_msg()
**
** DESCRIPTION: The same as the getmsg routine.  The only difference is
** that they use different catalog descriptor variables.  This routine
** is used to access attibute description catalogs, while the getmsg routine is
** always used to access piobe.cat.  The reason for get_attr_msg() is so
** piobe.cat is not being constantly opened and closed.  Eventually these
** routines should be rewritten to handle multiple catalogs opened at once.
**
*******************************************************************************
*******************************************************************************
*/
static char *attr_msgbuf = NULL;
static nl_catd attr_catd;
static nl_catd def_attr_catd;
static char save_attr_name[100];

char *get_attr_msg(CatName, set, num)
char *CatName;  
int set;
int num;
{
	char *ptr;
	char *nlspath;
	char *defpath = malloc(200);
	char default_msg[100];

	if (strcmp(CatName,save_attr_name) != 0)  /* is it a different catalog */
	{
	catclose(attr_catd);  /* close /usr/lpp message catalog */
	catclose(def_attr_catd);  /* close default message catalog */
	attr_catd = def_attr_catd = (nl_catd)NULL;  /* set catalog descriptors to NULL */
	strcpy(save_attr_name,CatName);
	}

	if (attr_catd != -1)  
		if (attr_catd == 0) /* if it hasn't been open before */
			attr_catd = catopen(CatName,NL_CAT_LOCALE);
	
	if (attr_catd != -1)
		{
		ptr = catgets(attr_catd,set,num,"dummy");
		if (!attr_msgbuf)
			attr_msgbuf = malloc(4001);
		if (attr_msgbuf)
			strncpy(attr_msgbuf, ptr, 4000);
		if (strcmp(ptr,"dummy") != 0)  /* did catgets fail? */
			return(attr_msgbuf);
		}
	
	if (def_attr_catd == 0)
		{
		sprintf(defpath,"/usr/lib/lpd/pio/etc/%s",CatName);
		def_attr_catd = catopen(defpath,NL_CAT_LOCALE);
		}
	
	sprintf(default_msg,"Cannot access message catalog %s.\n",CatName);
	ptr = catgets(def_attr_catd,set,num, default_msg);
	if (!attr_msgbuf)
		attr_msgbuf = malloc(4001);
	if (attr_msgbuf)
		strncpy(attr_msgbuf, ptr, 4000);

    free(defpath);
	return(attr_msgbuf);
}
		

