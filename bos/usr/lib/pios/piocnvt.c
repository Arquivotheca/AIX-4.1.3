static char sccsid[] = "@(#)92  1.4.1.3  src/bos/usr/lib/pios/piocnvt.c, cmdpios, bos411, 9428A410j 11/9/93 11:04:24";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS:
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

int fgetln();
char *getf();
void error_exit();
int     assemble();
int     find_state();
void    get_parent(),
        error_exit();

int     leave_same(),
        make_big(),
        make_small();
char    *getf();
void add_to_list(),
         delete_from_list(),
         add_attr(),
         change_attr(),
         insert_attr(),
         delete_attr(),
         check_file(),
         print_tree(),
         mark_file();

char *getmsg();

char base_dir[50], tmp_A[25], tmp_B[25];
int output_file;
int input_file;
char errno_str[5];    /* used to convert errno to a string  */

struct c_head   *add_head(), *insert_head();


struct c_head { char *head_text;
                                struct c_head *next_head;
                                struct c_attr *first_attr, *last_attr;
                                };

struct c_attr { char *attr_text;
                                struct c_attr *prev_attr, *next_attr;
                                };

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <nl_types.h>

#define BIG             1
#define SMALL           0
#define NOT_SPECIFIED   1
#define SPECIFIED       0
#define UNDEFINED       1099
#define ASSEMBLE        "assemble"
#define NO_CAT          "Cannot open message catalog\n"
#define HEADING         (strlen(text) == 5)
#define ATTRIBUTE       (strlen(text) == 2)
#define DELETE          (*text == '!')
#define MAXLINELEN	(1000)

#define PARENT_MARK     "zP"
#define STATUS_MARK     "zS"
#define DES_STAT_MARK   "zD"

#include "piobe_msg.h"

/*============================================================================*/
/*============================================================================*/
main(argc,argv)
int argc;
char *argv[];
{
char *in_file, *out_file;
char cmd[1000];
char wk_str[100];
int rc, c, show_empty_heads = FALSE;
int desired_state = UNDEFINED;
int current_state = UNDEFINED;

extern  char    *optarg;
extern  int     optind, opterr, optopt, errno;

output_file = NOT_SPECIFIED;
input_file = NOT_SPECIFIED;
opterr = 0;

(void) setlocale(LC_ALL, "");

/*
** Makes the paths to files and commands based on the PIOBASEDIR environment
** variable (if present) or /usr/lib/lpd/pio (if not present).
*/

    strcpy(base_dir,(getenv("PIOBASEDIR")
                        ? (char *)getenv("PIOBASEDIR")
                        : "/usr/lib/lpd/pio"));

    sprintf(cmd,"%d",getpid());
    sprintf(tmp_A,"/tmp/asm.A.%s",cmd);
    sprintf(tmp_B,"/tmp/asm.B.%s",cmd);

while ((c = getopt(argc,argv,"s:i:o:h")) != EOF)
    {
    switch(c)
        {
        case 's':
            switch ( (int)*optarg )
                {
                case '+':
                case '1':
                    desired_state = BIG;
                    break;

                case '!':
                case '0':
                    desired_state = SMALL;
                    break;

                default:
                    error_exit(CNVT_BADARG,"s",optarg);
                    break;
                }
            break;

        case 'i':
            in_file = optarg;
            input_file = SPECIFIED;
            current_state = find_state(in_file,STATUS_MARK);
            break;

        case 'o':
            out_file = optarg;
            output_file = SPECIFIED;
            break;

        case 'h':                      /* show headings with no attributes */
            show_empty_heads = TRUE;
            break;

        case '?':                        /* Unrecognized flag */
            error_exit(CNVT_CNVT_USAGE);
        }
    }


if ( input_file == NOT_SPECIFIED )
    error_exit(CNVT_CNVT_USAGE);

/*
**  If the output file was not specified on the command line then the
**  expansion/contraction process will effectively be done in-place.
*/
if ( output_file == NOT_SPECIFIED ) out_file = in_file;

/*
**  If the desired state is UNDEFINED at this point, then it must not have been
**  specified on the command line.  If, after looking for the desired state
**  in the file itself, it is still UNDEFINED, then we are going to look
**  directly into the master file.  This makes this attribute a very special
**  case but a defensible one.  Finally, if we can't get it out of the master 
**  file we're going to assume the desired state is BIG.
*/
/*
    It has been decided that the attribute for the default state ("zD") will
    not be modifiable in "chvirprt".  Hence only the value in "master" colon
    file is to be considered to determine default state.	- 03/16/92
 */
if ( desired_state == UNDEFINED )
    {
        char master_file[100];
        sprintf(master_file,"%s/predef/master",base_dir);

        desired_state = find_state(master_file,DES_STAT_MARK);
        if ( desired_state == UNDEFINED ) desired_state = BIG;
    }


umask(75);

#define GORP    in_file,out_file,show_empty_heads,output_file,current_state
if ( current_state == UNDEFINED )
   rc = leave_same(GORP);
else
    {
    if ( desired_state == BIG )
           rc = make_big(GORP);
    else
           rc = make_small(GORP);
    }

unlink(tmp_A);
unlink(tmp_B);

exit(rc);
}

/*============================================================================*/
int find_state(path,mark)
char *path, *mark;
/*
**  Open the file defined by the path.  Error_exit if the open fails.
**
**  Scan the file to determine its state.  :@S::+ (or 1) indicates
**  that the file has already been expanded into a complete printer
**  description file.  :@S::! (or 0) indicates that the file is a
**  child of one or more parent files and must be assembled with
**  its parents in order to become a complete printer description file.
**
**  The absence of the :@S:: string, as will be the case with a 3.1
**  colon file, will be taken to mean that the file is a complete
**  file not in need of assembly.
**
**  The use of this routine was expanded after its original writing to more
**  generally return the binary value of a named field.  This became
**  useful when the concept of leaving the file in a user defined state
**  was developed.  Originally, if the command line didn't indicate the
**  desired state of the file, an error was signaled.  Now, if the command
**  line doesn't state the desired state explicitly, we'll use this routine
**  to gaze into the file to find out what the user wanted and leave the
**  file that way.
*/
{
    FILE *file;
    char line[MAXLINELEN+1];
    int file_state = UNDEFINED;

/* Exit if file can't be opened */
    if ( !(file = fopen(path,"r")) ) return file_state;

    while ( !fgetln(file,line,sizeof(line)) && file_state == UNDEFINED)
        {
        if (line[0] == '#')  /* comment */
            continue;
        if ( !strcmp(mark,getf(line,2)) )
            {
            switch ( (int)*getf(line,4) )
                {
                case '+':
                case '1':
                    file_state = BIG;
                    break;                      /* for lint */

                case '!':
                case '0':
                    file_state = SMALL;
                    break;                      /* for lint */

                default:
                    break;
                }
            break;
            }
        }

    fclose(file);
    return file_state;
}

/*============================================================================*/
void get_parent(path,name)
char *path, *name;
/*
**  Open the file defined by the path.  Error_exit if the open fails.
**
**  Scan the file to determine its parent.  :zP::name indicates
**  that the parent files name is 'name'.  If 'name' is null, or the
**  :zP:: string does not exist, the file is assumed to have no parent.
*/
{
    FILE *file;
    char line[MAXLINELEN+1];
    char *prntnm;

/* Exit if file can't be opened */
    if ( !(file = fopen(path,"r")) ) error_exit(CNVT_NOFILE,path);

    *name = NULL;
    while ( !fgetln(file,line,sizeof(line)) )
        {
        if (line[0] == '#') 
           continue;
        if ( !strcmp(PARENT_MARK,getf(line,2)) )
            {
	/* If the parent name begins with a slash, it is considered to be a
	   full path name.  Else, "predef" directory path is prepended to it.
	 */
        if ((prntnm = getf(line,4)) && *prntnm)
	{
	   if (*prntnm == '/')
	       (void) strcpy(name, prntnm);
	   else
               (void) sprintf(name, "%s/predef/%s", base_dir, prntnm);
	}
        continue;
            }
        }

    fclose(file);
}

/*============================================================================*/
int leave_same(in,out,show,out_file,state)
char *in, *out;
int show, out_file, state;
/*
**  Given the input and output file names, prepares the assemble
**  command string to be invoked.
*/
{
        extern int errno;
        char wk_str[100];
        char *args[20];
        int argcount;
        FILE *fd;
        int rc;

        argcount = 0;
        args[argcount] = (char *)malloc(strlen(ASSEMBLE) + 1);
        strcpy(args[(argcount)++],ASSEMBLE);
        args[argcount] = (char *)malloc(strlen("-s!") + 1);
        strcpy(args[(argcount)++],state?"-s+":"-s!");
        args[argcount] = (char *)malloc(strlen("-h") + 1);
        strcpy(args[(argcount)++],show?"":"-h");
        sprintf(wk_str,"-a%s",in);
        args[argcount] = (char *)malloc(strlen(wk_str) + 1);
        strcpy(args[(argcount)++],wk_str);

        if (output_file == NOT_SPECIFIED)
        {
        if ( !(fd = freopen(tmp_A,"w",stdout)) ) 
         /* redirect output of assemble to tmp_A  */
           {
           sprintf(errno_str,"%d",errno);
           error_exit(CNVT_OPENFILE,tmp_A,errno_str);
           }
        rc = assemble(argcount,args);
        fclose(fd);
        sprintf(wk_str,"cp %s %s 2>/dev/null",tmp_A,in);
        system(wk_str);
        }
        else
           {
           if ( !(fd = freopen(out,"w",stdout)) )  
            /* redirect output of assemble to out_file  */
              {
              sprintf(errno_str,"%d",errno);
              error_exit(CNVT_OPENFILE,out,errno_str);
              }
           rc = assemble(argcount,args);
           fclose(fd);
           }

        return(rc);
}

/*============================================================================*/
int make_big(in,out,show,out_file,state)
char *in, *out;
int show, out_file, state;
/*
**  Given the input and output file names, prepares the assemble
**  command string to be invoked.
*/
{
    extern int errno;
    char wk_str[100], *parents[25];
    int depth, level;
    char *args[20];
    int argcount;
    FILE *fd;
    int rc;

    argcount = 0;
    parents[level=0] = (char *)malloc(strlen(in)+1);
    strcpy(parents[level],in);
    do {
        get_parent(parents[level++],wk_str);
        parents[level] = (char *)malloc(strlen(wk_str) + 1);
        strcpy(parents[level],wk_str);
        } while ( *parents[level] );
    depth = level - 1;

  /*  build an arry of pointers to arguments used by assemble  */

    args[argcount] = (char *)malloc(strlen(ASSEMBLE) + 1);
    strcpy(args[(argcount)++],ASSEMBLE);
    args[argcount] = (char *)malloc(strlen("-s+") + 1);
    strcpy(args[(argcount)++],"-s+");
    args[argcount] = (char *)malloc(strlen("-h") + 1);
    strcpy(args[(argcount)++],show?"":"-h");
    sprintf(wk_str,"-p%s",parents[out_file]);
    args[argcount] = (char *)malloc(strlen(wk_str) + 1);
    strcpy(args[(argcount)++],wk_str);

    for ( level = depth ; level >= 0 ; level-- )
        {
        sprintf(wk_str,"-a%s",parents[level]);
                args[argcount] = (char *)malloc(strlen(wk_str) + 1);
                strcpy(args[(argcount)++],wk_str);
        free(parents[level]);
        }
    args[argcount] = NULL;    /* terminate string of arguments with NULL */

    if (out_file == NOT_SPECIFIED)
       {
       if ( !(fd = freopen(tmp_A,"w",stdout)) )  
           /* redirect output of assemble to tmp_A  */
              {
              sprintf(errno_str,"%d",errno);
              error_exit(CNVT_OPENFILE,tmp_A,errno_str);
              }
       rc = assemble(argcount,args);
       fclose(fd);
       sprintf(wk_str,"cp %s %s 2>/dev/null",tmp_A,in);
       system(wk_str);
       }
    else
       {
       if ( !(fd = freopen(out,"w",stdout)) )
          /* redirect output of assemble to out_file  */
              {
              sprintf(errno_str,"%d",errno);
              error_exit(CNVT_OPENFILE,out,errno_str);
              }
       rc = assemble(argcount,args);
       fclose(fd);
       }

    return(rc);
}

/*============================================================================*/
int make_small(in,out,show,out_file,state)
char *in, *out;
int show, out_file, state;
/*
**  Given the input and output file names, prepares the assemble
**  command string to be invoked.
*/
{
    extern int errno;
    char wk_str[100], *parents[25];
    char c1[300], c2[300], c3[300];
    int depth, level;
    char *args[20];
    int argcount;
    FILE *fd;
    int rc;

        argcount = 0;
        parents[level=0] = (char *)malloc(strlen(in)+1);
        strcpy(parents[level],in);
        do {
           get_parent(parents[level++],wk_str);
           parents[level] = (char *)malloc(strlen(wk_str) + 1);
           strcpy(parents[level],wk_str);
        } while ( *parents[level] );
        depth = level - 1;

        args[argcount] = (char *)malloc(strlen(ASSEMBLE) + 1);
        strcpy(args[(argcount)++],ASSEMBLE);
        args[argcount] = (char *)malloc(strlen("-s+") + 1);
        strcpy(args[(argcount)++],"-s+");
        args[argcount] = (char *)malloc(strlen("-pnobody") + 1);
        strcpy(args[(argcount)++],"-pnobody");


        if ( state == BIG ) 
        {
                sprintf(wk_str,"-a%s",parents[0]);
                args[argcount] = (char *)malloc(strlen(wk_str) + 1);
                strcpy(args[(argcount)++],wk_str);
        }
        else
        {
        for ( level = depth ; level >= 0 ; level-- )
            {
                        sprintf(wk_str,"-a%s",parents[level]);
                        args[argcount] = (char *)malloc(strlen(wk_str) + 1);
                        strcpy(args[(argcount)++],wk_str);
            }
        }
        args[argcount] = NULL;    /* terminate string of arguments with NULL */

        if ( !(fd = freopen(tmp_A,"w",stdout)) ) 
           /* redirect output of assemble to tmp_A  */
              {
              sprintf(errno_str,"%d",errno);
              error_exit(CNVT_OPENFILE,tmp_A,errno_str);
              }
        rc = assemble(argcount,args);
        fclose(fd);

        argcount = 0;
        args[argcount] = (char *)malloc(strlen(ASSEMBLE) + 1);
        strcpy(args[(argcount)++],ASSEMBLE);
        args[argcount] = (char *)malloc(strlen("-s+") + 1);
        strcpy(args[(argcount)++],"-s+");
        args[argcount] = (char *)malloc(strlen("-pnobody") + 1);
        strcpy(args[(argcount)++],"-pnobody");

        for ( level = depth ; level >= 1 ; level-- )
        {
                sprintf(wk_str,"-a%s",parents[level]);
                args[argcount] = (char *)malloc(strlen(wk_str) + 1);
                strcpy(args[(argcount)++],wk_str);
        }
        args[argcount] = NULL;    /* terminate string of arguments with NULL */

        if ( !(fd = freopen(tmp_B,"w",stdout)) ) 
          /* redirect output of assemble to tmp_B  */
              {
              sprintf(errno_str,"%d",errno);
              error_exit(CNVT_OPENFILE,tmp_B,errno_str);
              }
        rc = assemble(argcount,args);
        fclose(fd);

        argcount = 0;
        args[argcount] = (char *)malloc(strlen(ASSEMBLE) + 1);
        strcpy(args[(argcount)++],ASSEMBLE);
        args[argcount] = (char *)malloc(strlen("-s!") + 1);
        strcpy(args[(argcount)++],"-s!");
        args[argcount] = (char *)malloc(strlen("-h") + 1);
        strcpy(args[(argcount)++],show?"":"-h");
        sprintf(wk_str,"-p%s",parents[out_file]);
        args[argcount] = (char *)malloc(strlen(wk_str) + 1);
        strcpy(args[(argcount)++],wk_str);
        sprintf(wk_str,"-a%s",tmp_A);
        args[argcount] = (char *)malloc(strlen(wk_str) + 1);
        strcpy(args[(argcount)++],wk_str);
        sprintf(wk_str,"-d%s",tmp_B);
        args[argcount] = (char *)malloc(strlen(wk_str) + 1);
        strcpy(args[(argcount)++],wk_str);
        args[argcount] = NULL;    /* terminate string of arguments with NULL */

        if ( !(fd = freopen(out,"w",stdout)) )  
         /* redirect output of assemble to out_file  */
           {
           sprintf(errno_str,"%d",errno);
           error_exit(CNVT_OPENFILE,out,errno_str);
           }
        rc = assemble(argcount,args);
        fclose(fd);

        return(rc);
}


/*============================================================================*/
int fgetln(file,line,lim)
FILE *file;
char *line;
int lim;
/*
**  Get a single line from the specified file.  Return EOF when the
**  file is empty.
*/
{
    int i;
    char ch;

    i = 0;
    while ( (ch = fgetc(file) ) != '\n' && i < lim-1 &&
	    !feof(file)) line[i++] = ch;
    line[i] = NULL;

    return feof(file) ? EOF : 0 ;
}

/*============================================================================*/
char *getf(cs,num)
char *cs;
int num;
/*
**  Return the numth field in a colon string.
*/
{
    static char fld[1000];
    int i, j, length, cc;

    length = strlen(cs);
    cc = i = 0;

    while ( cc < num )
        {
        if ( cs[i] == ':' ) cc++;
        if ( ++i > length ) break;
        }

    /* Extract the specified field.  If the field is "attribute value",
       extract till the end of the string. */
    for ( j=0 ; i<length && cs[i] ; )
    {
        if (num < 4 && cs[i] == ':')
            break;
        fld[j++]=cs[i++];
    }
    fld[j] = NULL;
    return fld;
}

/*============================================================================*/
void error_exit(msg_num,f1,f2)
int msg_num;
char *f1, *f2;
{
    switch( msg_num )
        {
        case CNVT_CNVT_USAGE:
            fprintf(stderr,getmsg(MF_PIOBE,6,msg_num));
            break;

        case CNVT_NOFILE:
        case CNVT_BADOPT:
            fprintf(stderr,getmsg(MF_PIOBE,6,msg_num),f1);
            break;

        case CNVT_BADARG:
        case CNVT_BADATTR:
        case CNVT_DUPATTR:
        case CNVT_OPENFILE:
            fprintf(stderr,getmsg(MF_PIOBE,6,msg_num),f1,f2);
            break;
        }

    exit( msg_num );

}
/*============================================================================*/
int assemble(argc,argv)
int argc;
char *argv[];
{
struct c_head *c_file = NULL;
int c, show_empty_heads = TRUE;
char *ptr, *state = NULL;
char *parent = NULL;

extern  char    *optarg;    /* points to argument found */
extern  int     optind, opterr, optopt, errno;

opterr = 0;
optind = 1;
while ((c = getopt(argc,argv,"a:s:d:hp:")) != EOF) 
    {
    switch ( c )
        {
        case 'a':           /* add file to assembled tree */
            check_file(optarg);
            break;

        case 's':             /* state in which file will be left */
            state = optarg;
            break;

        case 'd':              /* delete file from assembled tree */
            break;

        case 'h':             /* show headings with no attributes */
            show_empty_heads = FALSE;
            break;

        case 'p':            /* record parent */
            ptr = optarg;
            while ( strchr(ptr,'/') ) ptr++;
            parent = ptr;
            break;

        case '?':        /* Unrecognized flag */
            error_exit(CNVT_ASMBL_USAGE);
        }
    }

optind = 1;
opterr = 0;
while ((c = getopt(argc,argv,"a:s:d:hp:")) != EOF)
    {
    switch ( c )
        {
        case 'a':               /* add file to assembled tree */
            add_to_list(optarg,&c_file);
            break;

        case 'd':              /* delete file from assembled tree */
            delete_from_list(optarg,&c_file);
            break;
        }
    }

/* if ( parent ) mark_file(&c_file,PARENT_MARK,parent);  */
if ( state ) mark_file(&c_file,STATUS_MARK,state);

if ( c_file ) print_tree(&c_file,show_empty_heads);

return( 0 );
}

/*============================================================================*/
void add_to_list(path,top_of_file)
/*
**
**      The file will be opened and scanned a line at a time.  Initially,
**      the tree is empty.  The first thing added to the tree is a default
**      heading under which certain attributes may be put.
**
**      If the line read from the file is a heading line, the tree is scanned
**      for the existence of that heading.  If it does not already exist, it
**      is added following the currently active heading (initially the
**      default heading).  It then becomes the active heading.
**
**      If an attribute is read, the entire tree is scanned to see if
**      it already exists.  If it does, but is found under the active
**      heading, then the new text replaces the old text and we go on.  If
**      it is found under a heading other than the active heading, it is
**      deleted.  The attribute is then created under the active heading in
**      its proper place alphabetically.
**
**      When a file is first opened, the active heading is the default heading.
**      If the first lines read from the file are attributes, they will be
**      added under the default heading.  When the file is displayed, the
**      default heading, which has no text between its colons, will not be
**      displayed.
**
**      As a file is read, headings may be found which do not already exist
**      in the tree.  They will be added following the active heading.  This
**      gives the user the ability to control the structure of the tree.
**      He may add his own grouping of attributes under his own heading and
**      make that heading appear anywhere in his listing by entering
**      his heading and attributes preceded by the heading he wants his
**      lines to follow; eg:  his file might look like this:
**                  __FLG
**                  __JAR
**                  q1
**                  q2
**                  wz
**      As the file was read, __FLG would become active (assuming it already
**      in the tree); __JAR would then be added following __FLG; the attributes
**      q1, q2, and wz would then all the added under __JAR.
*/
char *path;
struct c_head **top_of_file;
{
    FILE *file;
    char line[MAXLINELEN+1], text[10], *ptr;
    struct c_head *base = *top_of_file;
    struct c_head *h = NULL;
    struct c_head *this_head = NULL;
    struct c_attr *a = NULL;
    int done, found = FALSE;
    int delete;

/* Exit if file can't be opened */
    if ( !(file = fopen(path,"r")) ) error_exit(CNVT_NOFILE,path);

    if ( !base ) add_head(&base,"::::");            /* create default heading */

    while ( !fgetln(file,line,sizeof(line)) )
        {
                if (line[0] == '#') 
                   continue;
        delete = FALSE;
        strcpy(text,getf(line,2));
        if ( DELETE )       /* This attribute starts with '!' which says */
            {               /* that it MUST be deleted from the tree.  Rotate */
            ptr = text;     /* the attribute one byte to the left */
            while ( *ptr ) *ptr++ = *(ptr+1);
            delete = TRUE;
            }

        if ( HEADING )              /* line read is a heading line */
            {
            if ( delete )       /* if a heading line is to be deleted, */
                {               /* all attributes linked under it will */
                h = base;       /* disappear also */
                while ( h )
                    {
                    if ( !strcmp(text,getf(h->head_text,2)) )
                        {
                        strcpy(h->head_text,"::::");
                        h->first_attr = NULL;
                        this_head = NULL;
                        continue;
                        }
                    else h = h->next_head;
                    }
                continue;
                }

            found = FALSE;          /* start at the top of the tree looking */
            h = base;               /* for the heading just read in from */
            while ( !found && h )   /* the file - add it if not found */
                {
                if ( !strcmp(line,h->head_text) ) found = TRUE;
                else h = h->next_head;
                }

            if ( found ) this_head = h;
            else
                {
                if ( !this_head ) this_head = insert_head(&base,line);
                else this_head = insert_head(&this_head,line);
                }
            }
        else                        /* line read is an attribute line */
            {
            found = FALSE;
            done = FALSE;
            h = base;
            while ( h )
                {
                if ( a = h->first_attr )
                    {
                    while ( a )
                        {
                        if ( !strcmp(text,getf(a->attr_text,2)) )
                            {
                            found = TRUE;
                            if ( delete )
                                {
                                delete_attr(&h,&a);
                                done = TRUE;
                                }
                            else if ( h == this_head )
                                    {
                                    change_attr(&a,line);
                                    done = TRUE;
                                    }
                                else delete_attr(&h,&a);
                            break;
                            }
                        a = a->next_attr;
                        }
                    }
                if ( found ) break;
                h = h->next_head;
                }

            if ( !done )
                {
                found = FALSE;
                h = this_head ? this_head : base;
                a = h->first_attr;
                while ( !found && a )
                    {
                    if ( strcmp(text,getf(a->attr_text,2)) > 0 )
                        {                   /* keep looking if there are more */
                        if ( a->next_attr ) a = a->next_attr;
                        else break;
                        }
                    else                        /* we passed the place in the */
                        {                       /* tree where this attr would */
                        found = TRUE;           /* have been found - insert */
                        insert_attr(&h,&a,line);/* the new line between this */
                        break;                  /* line and the previous line */
                        }
                    }
                if ( !found && !delete ) add_attr(&h,line);
                }
            }
        }

    fclose(file);
    *top_of_file = base;
}

/*============================================================================*/
void delete_from_list(path,top_of_file)
/*
**
**      The file will be opened and scanned a line at a time.  Initially,
**      the 'active' heading is the default heading.  The active heading is 
**      the only one which will be seached for attributes to be deleted.  If
**      attibutes are read before a heading is read, they will deleted from
**      under the default heading if they exist there.
**
**      When a heading is read, the tree is scanned for that heading.  If
**      found, it becomes the active heading.  If not, the file is read
**      until a new heading is found.
**
**      The objective here is to delete attributes from under the heading
**      they're found under in the file being read. eg: Attribute _a exists
**      in the tree under heading __FLG; attribute _a is read from the file
**      following heading __JAR; attribute _a will not be deleted from the
**      tree because it is not found under the currently active heading (__JAR)
**
**      If __JAR is not a heading found in the tree, no attributes will be 
**      deleted, i.e.; the file will be read until a heading is read which
**      exists in the tree.
**
**      This approach permits a user to construct a tree with attributes
**      under the headings of his choice.
*/
char *path;
struct c_head **top_of_file;
{
    FILE *file;
    char line[MAXLINELEN+1], text[10], *ptr;
    struct c_head *base = *top_of_file;
    struct c_head *h = base;
    struct c_attr *a = NULL;
    int i, found = FALSE;

    /* Exit if file can't be opened */
    if ( !(file = fopen(path,"r")) ) error_exit(CNVT_NOFILE,path);

    while ( !fgetln(file,line,sizeof(line)) )
        {

                if (line[0] == '#') 
                   continue;
        strcpy(text,getf(line,2));

        /*
        **  Never delete the PARENT or STATUS attributes from the tree.
        **  The DESired_STATus_MARK can be deleted it's the same as its
        **  parent.  This permits the user to define it for a parent
        **  and leave it in effect for all children.
        */
        if ( !strcmp(PARENT_MARK,text) ||
             !strcmp(STATUS_MARK,text) ) break;

        if ( DELETE )       /* This attribute starts with '!' which says */
            {               /* that it MUST be deleted from the tree.  Rotate */
            ptr = text;     /* the attribute one byte to the left */
            while ( *ptr ) *ptr++ = *(ptr+1);
            }

        if ( HEADING )              /* line read is a heading line */
            {                       /* look thru the tree for it */
            h = base;
            while ( h )
                {
                if ( !strcmp(line,h->head_text) ) break;
                h = h->next_head;
                }
            }
        if ( !h || !h->first_attr ) continue;
        if ( ATTRIBUTE )             /* line read is an attribute line */
            {
            found = FALSE;
            a = h->first_attr;
            while ( a )
                {
                if ( !strcmp(line,a->attr_text) )
                    {
                    delete_attr(&h,&a,line);
                    break;
                    }
                a = a->next_attr;
                }
            }
        }

    fclose(file);
}

/*============================================================================*/
struct c_head *add_head(b,line)
/*
**  Add a heading string to the end of a list
*/
struct c_head **b;
char *line;
{
    struct c_head *base = *b;
    struct c_head *h, *new;

    new = (struct c_head *)malloc( sizeof(struct c_head) );
    new->head_text = (char *)malloc( strlen(line)+1 );
    strcpy(new->head_text,line);
    new->next_head = NULL;
    new->first_attr = NULL;
    new->last_attr = NULL;

    if ( !base ) *b = new;
    else
        {
        h = base;
        while ( h->next_head ) h = h->next_head;
        h->next_head = new;
        }

    return ( new );
}

/*============================================================================*/
struct c_head *insert_head(t,line)
/*
**  Insert a heading line after the line pointed to by 't'
*/
struct c_head **t;
char *line;
{
    struct c_head *this = *t;
    struct c_head *h, *new;

    new = (struct c_head *)malloc( sizeof(struct c_head) );
    new->head_text = (char *)malloc( strlen(line)+1 );
    strcpy(new->head_text,line);
    new->next_head = this->next_head;
    new->first_attr = NULL;
    new->last_attr = NULL;

    this->next_head = new;

    return ( new );
}

/*============================================================================*/
void add_attr(h,line)
/*
**  Add an attribute string to the end of a list
*/
struct c_head **h;
char *line;
{
    struct c_head *heading = *h;
    struct c_attr *new, *temp_prev;

    new = (struct c_attr *)malloc( sizeof(struct c_attr) );
    temp_prev = heading->last_attr;

    if ( heading->first_attr ) heading->last_attr->next_attr = new;
    else heading->first_attr = new;

    heading->last_attr = new;

    heading->last_attr->attr_text = (char *)malloc( strlen(line)+1 );
    strcpy(heading->last_attr->attr_text,line);
    heading->last_attr->prev_attr = temp_prev;
    heading->last_attr->next_attr = NULL;
}

/*============================================================================*/
void change_attr(a,line)
/*
**  Changes the attribute string referenced by a record
*/
struct c_attr **a;
char *line;
{
    struct c_attr *attribute = *a;

    free(attribute->attr_text);
    attribute->attr_text = (char *)malloc( strlen(line)+1 );
    strcpy(attribute->attr_text,line);
}

/*============================================================================*/
void insert_attr(h,a,line)
/*
**  Insert an attribute string into a list
*/
struct c_head **h;
struct c_attr **a;
char *line;
{
    struct c_head *heading = *h;
    struct c_attr *attribute = *a;
    struct c_attr *new;

/* Create the new line */
    new = (struct c_attr *)malloc( sizeof(struct c_attr) );
    new->attr_text = (char *)malloc( strlen(line)+1 );
    strcpy(new->attr_text,line);

/* Is the new line being inserted before the first line in the
    existing list? */
    if ( !attribute->prev_attr )
        {
        new->prev_attr = NULL;
        new->next_attr = heading->first_attr;
        heading->first_attr = new;
        attribute->prev_attr = new;
        }
    else
        {
        new->prev_attr = attribute->prev_attr;
        new->next_attr = attribute;
        attribute->prev_attr->next_attr = new;
        attribute->prev_attr = new;
        }
}

/*============================================================================*/
void delete_attr(h,a)
/*
**  Delete an attribute string from a list
*/
struct c_head **h;
struct c_attr **a;
{
    struct c_head *heading = *h;
    struct c_attr *attribute = *a;
    int done = FALSE;

/* Delete first attribute in list */
    if ( !attribute->prev_attr )
        {
        if ( attribute->next_attr ) attribute->next_attr->prev_attr = NULL;
        heading->first_attr = attribute->next_attr;
        done = TRUE;
        }

/* Delete last attribute in list */
    if ( !done && !attribute->next_attr )
        {
        attribute->prev_attr->next_attr = NULL;
        heading->last_attr = attribute->prev_attr;
        done = TRUE;
        }

/* Delete attribute in the middle of the list */
    if ( !done && attribute->prev_attr && attribute->next_attr )
        {
        attribute->prev_attr->next_attr = attribute->next_attr;
        attribute->next_attr->prev_attr = attribute->prev_attr;
        }

    free(attribute->attr_text);
    free(attribute);
}

/*============================================================================*/
void check_file(path)
/*  check_file()
**      check a file to make sure it conforms to at least a few
**      fundamental characteristics of a colon file:
**          a)  no repeated attributes
**          b)  alphanumeric in field 3 - 2 or 5 chars long
**
**      Duplicate attributes within a file are checked by building a simple
**      non-alphabetic chain of all the attributes in a file.  When a new
**      attribute is read, the chain is scanned for existence of the attribute.
**      If found, the error message is written and the program aborted.  If
**      not found, the new attribute is added to the chain and the loop
**      continued.
**
**      The chain of memory is freed when the check is complete.
*/
char *path;
{

struct chain {  char attr[3];
                struct chain *next_in_chain;
                };

    FILE *file;
    char line[MAXLINELEN+1], text[10], *ptr;
    struct chain *top = NULL;
    struct chain *c;
    register struct chain *new_attr = NULL;
    int error = FALSE;

/* Exit if file can't be opened */
    if ( !(file = fopen(path,"r")) ) error_exit(CNVT_NOFILE,path);

    while ( !fgetln(file,line,sizeof(line)))
        {
                if (line[0] == '#')
                   continue;
        strcpy(text,getf(line,2));
        if ( DELETE )       /* This attribute starts with '!' which says */
            {               /* that it MUST be deleted from the tree.  Rotate */
            ptr = text;     /* the attribute one byte to the left */
            while ( *ptr ) *ptr++ = *(ptr+1);
            }

        if ( HEADING ) continue;            /* line read is a heading line */
        if ( ATTRIBUTE )                    /* line read is an attribute line */
            {
            new_attr = (struct chain *)malloc( sizeof(struct chain) );
            strcpy(new_attr->attr,text);
            new_attr->next_in_chain = NULL;

            if ( !top ) top = new_attr;
            else
                {
                c = top;
                while ( c )
                    {
                    if ( !strcmp(text,c->attr) )
                                    error_exit(CNVT_DUPATTR,path,text);
                    if  ( !c->next_in_chain ) break;
                    c = c->next_in_chain;
                    }
                c->next_in_chain = new_attr;
                c = new_attr;
                }
            continue;
            }
        error_exit(CNVT_BADATTR,path,line);         /* line is neither */
        }

    fclose(file);

    /* free malloced memory in chain */
    for (c = top; c; c = new_attr)
    {
       new_attr = c->next_in_chain;
       free ((void *) c);
    }

}

/*============================================================================*/
void print_tree(top_of_file,show)
/*  print_tree()
**      print colon file tree
**
**      The 'show' parameter indicates whether or not a heading line not
**      followed by any attributes should be displayed or not.  Normally
**      Such a heading will be displayed, but this flag permits suppression
**      of the display.
**
**      The heading will also not be displayed if the second field contains
**      no text - this is the way the default heading ('::::') is suppressed.
*/
struct c_head **top_of_file;
int show;
{
    struct c_head *h = *top_of_file;
    struct c_attr *a;
    char field[10];

    while ( h )
        {
        if ( (a=h->first_attr) || show )
            {
            strcpy(field,getf(h->head_text,2));
            if ( *field ) puts(h->head_text);
            while ( a )
                {
                puts(a->attr_text);
                a = a->next_attr;
                }
            }
        h = h->next_head;
        }
}

/*============================================================================*/
void mark_file(top_of_file,attr,val)
/*  mark_file()
**
**      Run the tree to find the attribute specified by 'attr'.
**
**      If found, replace the value field with the string given in 'val'.
**      If not found, just return quietly.
*/
struct c_head **top_of_file;
char *attr, *val;
{
    struct c_head *h = *top_of_file;
    struct c_attr *a;
    char line[100];
    int done = FALSE;

    while ( h && !done )
        {
        a = h->first_attr;
        while ( a )
            {
            if ( !strcmp(attr,getf(a->attr_text,2) ) )
                {
                char *ptr;
                int cnt;

                strcpy(line,a->attr_text);
                ptr = line;

                for(cnt=0; cnt<4; cnt++) while ( *ptr++ != ':' );
                strcpy(ptr,val);

                change_attr(&a,line);
                a = NULL;
                done = TRUE;
                }
            else a = a->next_attr;
            }
        h = h->next_head;
        }
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
