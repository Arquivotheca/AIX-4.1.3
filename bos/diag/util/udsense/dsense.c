static char sccsid[] = "@(#)58  1.7  src/bos/diag/util/udsense/dsense.c, dsaudsense, bos411, 9435D411a 9/2/94 09:40:43";
/*
 *   COMPONENT_NAME: DSAUDSENSE
 *
 *   FUNCTIONS: bit_chk
 *              check_decode
 *              cond_proc
 *              cron_disp
 *              dec_rcrec
 *              dec_rest
 *              dec_sense
 *              displ_sum
 *              do_addlev
 *              do_comp
 *              do_descript
 *              do_err
 *              do_intval
 *              do_not
 *              do_oper
 *              do_pragmas
 *              do_reducelev
 *              env_get
 *              expr_do
 *              fnd_runmode
 *              get_bytes
 *              get_emsgs
 *              get_errdes
 *              get_hint
 *              get_ivalue
 *              get_senserec
 *              get_sfindex
 *              get_sroutes
 *              get_vpos
 *              index_senser
 *              main
 *              mvar_find
 *              parse_arec
 *              parse_expr
 *              parse_grec
 *              parse_tokens
 *              prt_info
 *              prt_recerr
 *              prt_sense
 *              read_sense
 *              str_ext
 *              sum_err
 *              tst_fields
 *              wrt_colstr
 *              wrt_sum
 *              xlate_inrow
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/**************************************************************************/
/* dsense.c                                                               */
/* dsense     Decodes error logs and sense data for problems based on     */
/*            information passed to it.                                   */
/* syntax:  dsense [-c] [-t] [-s] [-mmain_list] resource_type
   See last routine for explanation of syntax
*/
/* Dan Henderson
   AWD/ Dpt F82
*/
/*                                                                        */
/* IBM INTERNAL USE ONLY                                                  */
/* This program can look at sense data three ways:
     1) Takes as standard input the formatted output of error logs errpt -a
        a) Looks for hardware errors, and displays the corresponding
           error log information
        b) In the display replaces the un-decoded sense data with decoded
           sense data based on a routines giving rules on how to decode them.
        c) Keeps statistics concerning the number of times a particular
           ERROR LABEL, ID, TYPE is encountered.

     2) Takes the same data as in 1 and does the same processing, but
        uses the labelled output of an errpt -g instead of -a
        In this case requires the existence of or creates a file
                                 elogerr.des
        Containing the list of error descriptions as created from the
        command errmsg -w E

     3) If a parameter is passed to the program, assumes that standard
        output only contains the sense data from an error. The parameter
        passed should be the name of the sense resource routine that
        defines the data.

        In this case only the decoded sense data information is displayed.
*/
/* The program can write out data as follows:
     1) To standard output is written any detailed information as passed
        to dsense with the exception that all hex sense data bytes are
        replaced with an English language explanation
     2) To standard error is written a summary by Resource Name, Type
        Error Label and explanation of error, a summary that counts the
        errors decoded of each category along with a one line description
        of the error as taken from the sense data if possible or from the
        Error Description of the error log when there is no sense data
        translation.
     3) When the -c flag is included, puts out a chronological error
        summary giving the date, time, name, type, error ID and description
        for each error in reverse chronological order
     4) When the -t flag is included puts the summary info in a table format
        in a method suitable for storing in a database.
        Does not display any detailed information.
*/
/* The program decodes sense data using routines based on resource type,
   error id, or resource name (in that order of importance).
   It uses a routines written into files that are interpreted which tell how
   to evaluate, format, and describe the sense information.

   The file dsense.lib contains a list of files (no more than 9 and currently
   rtypes.dsense and labels.dsense) which contain routines describing
   translations using routines given the name of resouce types, error IDs or
   names.  These files are located in the path described by environment
   variable DIAGNOSTICS plus /bin or SENSE_PATH.  If those variables do not
   exist then use the path /esc/lpp/diagnostics/bin.

   The -m flag can be used to pass the name of an alternate file to be
   used to contain the list of sense routine files.
*/
/*
LIST OF ROUTINES:

     A. Main routines

        env_get      -- Gets info from env, and initializes variables
                        including the opening of the sense decode file if
                        passed as a parm
        fnd_runmode  -- Determines if an error log or just sense data was
                        passed
        dec_sense    -- Main file for decoding lines of sense file
        get_emsgs    -- Get error messages as needed for the -g option
        do_err       -- Decodes one error report entry of the errpt

     B. Input handling Utilities

        get_sfindex  -- Gets seek indexes to all routines in the
                        sense file
        read_sense   -- Reads rows of sense data for stdin and packs into
                        array
        index_senser -- Set active routine index to routine passed
        xlate_inrow  -- Packs into array 1 row of sense bytes
        get_errdes   -- Get error description text file

     C. Header Decoding routines
        parse_arec  -- Decodes a line of an error report entry of the -a
                       style
        parse_grec  -- Decodes a line of an error report entry of the -g
                       style

     D. Printing and Summary routines
        prt_sense   -- Echos back packed array of sense bytes
        displ_sum   -- Displays summary to stderr of store error ids
        sum_err     -- Adds an error entry to summary of errors
        cron_disp   -- Displays a line of chronological error information
        wrt_sum     -- Puts out a line of summary info

     E. Error Interpretation routines
        get_senserec-- Get a record from the current sense file
        parse_tokens-- Parses a sense record into tokens
        cond_proc   -- Processes a conditional command
        dec_rest    -- Decodes print, variable set and print parm tokens
        do_descript -- Handles display of the description text
        do_pragmas  -- Handles # pragmas
        expr_do     -- Handles a set variable expression
        dec_rcrec   -- decodes fields of a display/mvar set cmnd
        tst_fields  -- Checks decoded fields for proper range
        do_comp     -- takes display/mvar action based on decoded vals


     F. Numeric/logical expression evaluation
        parse_expr  -- Parses an expression
        do_addlev   -- Adds a paren level in an expr
        do_oper     -- Handles a logic or arthi operation in an expression
        do_not      -- Handles a not in an expr
        do_reduclev -- Removes a paren level in an expr
        do_intval   -- Processes a value in an expression


     G. Utility routines:
        get_vpos    -- Compute index into sense bytes for row/col pos
        mvar_find   -- finds value/sets macro variables
        get_ivalue  -- Evaluates passed parameter to obtain an integer
        get_hint    -- Gets integer value of a passed set of hex digits
        get_bytes   -- Copies over and verifies bytes
        bit_chk     -- Does a bit wise compare
        str_ext     -- Adds an extension to a file name
        wrt_colstr  -- Creates a column of summary information
        prt_recerr  -- Prints record where error occurred
        prt_info    -- Prints help information
*/

/*                                                                        */
/* 1. Library Includes                                                    */
/*                                                                        */
      #include <stdio.h>        /* Has open, rd, wr defines for ioctl cmnd*/
      #include <ctype.h>        /* Has data type conversion macros        */
      #include <string.h>       /* Has string manipulation functions      */
      #include <errno.h>        /* Has file access error information      */
      #include <fcntl.h>        /* Has open, rd, wr defines for ioctl cmnd*/
      #include <ctype.h>        /* Has digit type compares                */
      #include <time.h>         /* Structure for time decoding routines   */
/*                                                                        */
/* 2. General Globals defined                                             */
      #ifndef FALSE
      #define FALSE 0
      #endif

      #ifndef TRUE
      #define TRUE 1
      #endif

      #define REC_MAX 256

      /* Define the maximum rows and digits allowed for sense bytes */
      #define NMAXROWS 17
      #define MAXINDIGS 64

      #define VPOSMAX ((NMAXROWS*MAXINDIGS) - 1)

      /* Define the error messages file to be used for the -g style of err
         report */
      #define EMSGFILE "elogerr.des"
      #define MAINFILE "dsense.lib"


/*                                                                        */
/* 3. Main data structures                                                */
/*                                                                        */

      /* Structure for storage of errors encountered */
      #define MAXID   20
      #define MAXLAB  20
      #define MAXRNM  20
      #define MAXRTYP 20
      #define MAXFNAME 160

      #define MAXCTYPES 400
      struct
      {
         int cnt;                  /* Number of times label encounterd     */
         int looked_sf;            /* Looked for sense file for this error */
         int did_prt;              /* Set true if did print this value out */
         char id[MAXID+1];         /* Error Id                             */
         char label[MAXLAB+1];     /* Error label                          */
         char rname[MAXRNM+1];     /* Resource Name                        */
         char rtype[MAXRTYP+1];    /* Resource Type                        */
         char *p_sumline;          /* Pointer to summary line              */
      } errtypes[MAXCTYPES];
      int nerrids=0; /* Number of error ids found */

      /* Values associated with the individual error */
      char err_descript[REC_MAX+1]; /* Description of the error or the
                                       summary line if better defined
                                       in sense data */
      int need_errdes = FALSE;      /* True if next line of report will
                                       contain the error description      */
      char err_label[MAXLAB+1];     /* Label corresponding to the error   */
      char err_id[MAXID+1];         /* Id corresponding to the label      */
      char rsrc_name[MAXRNM+1];     /* Resource name for the error        */
      char rsrc_type[MAXRTYP+1];    /* Resource type for the error        */
      #define DTTMMAX 20
      char date_time[DTTMMAX+1];    /* Date and time of error             */
      #define ETYPMAX 20
      char err_type[ETYPMAX+1];     /* Error Type                         */

      /* Storage for error summary strings */
      #define MAXSUMLINES  80*40
      char sumlines[MAXSUMLINES];
      char *p_nxtsumline;           /* Pointer to next free pos in sumlines*/


      /* Structure that defines the sense files that can be active */
      #define MAXSFILES 9
      struct
      {
         char name[REC_MAX+1];      /* Name of the file    */
         FILE *fptr;                /* Pointer to the file */
      }sen_files[MAXSFILES];
      int nsenfiles=0;

      /* Structure relating to indexes in the sense definition file */
      #define MAXSDEFS 200
      struct
      {
         char name[MAXRTYP+1];  /* Name of routine                     */
         int  sindex;           /* Index into the sense file structure for
                                   the file containing the routine     */
         long seek_at;          /* Where to seek to get start of route */
         int lno;               /* Line number of start of routine     */
      }sroutes[MAXSDEFS];
      int nsroutes = 0;        /* No sense routine found thus far */


      /* Structure that defines a list of opened routines */
      #define MAXOPROUTES 20
      struct
      {
          long s_at;            /* Ptr to where currently at*/
          int lno;              /* Line number in the file */
          int rindex;           /* Index into sroutes index of routines for
                                   the actually active routine */
          FILE *fptr;           /* Pointer to file containing the routine*/
      } senser[MAXOPROUTES];
      int senser_at=(0 - 1);    /* Index into the senser structure */

      /* Information decoding a conditional expression in the sense file   */
      int row_col[2];        /* Are Row, Col,  xxxx, yyyy                  */
      int cndvar=FALSE;      /* Instead of row, col, var for a cond. expr */
      long int a_int=0;      /* Value found for cndvar if required       */
      int ncmptoks;          /* Number of compare tokens fnd               */
      int last_ctrue=FALSE;  /* Last conditional tested was true           */
      int rest_token;        /* Start of infon after compare of a line     */

      char cmpval[17];       /* Value to do a compare on                   */

      #define DTMAX 4
      char dtype[DTMAX]; /* Character string setting type of
                                                 check decision to be made */
      #define INTS_4  0
      #define INTS_8  1
      #define INTS_16 2
      #define INTS_24 3
      #define INTS_32 4
      int int_size;          /* Size of the integer string being worked    */


      /* Storage related to sense file mvars */
      #define FLAG_MAX 100
      #define MVARNM_MAX 32
      struct
      {
          char name[MVARNM_MAX+1];   /* Name given to a mvar    */
          long int val;              /* Value given to the mvar */
      }mvars[FLAG_MAX+1];
      int nmvars=0;              /* Number of mvars defined */

      /* Conditional statements storage */
      #define MAXIFLEVS 10
      struct
      {
          int do_block;  /* If true executes the current block */
          int had_true;  /* If true indicates that some block condition on
                            level was satisfied */
          int had_else;  /* Indicates that an else condition was found */
      } ifs[MAXIFLEVS];

      int if_level=0;    /* Initial level is zero */

      /* Definition of conditional expressions */
      #define MAXEXPR 10
      struct
      {
         long int value;
         char funct[3];
         int have_lval;
      } expr[MAXEXPR] ;
      int nest_at=0;  /* No if nesting to start with */


      /* Token storage information */
      char token_str[(REC_MAX*2)+1]; /* Storage for parsed string info */
      int ntokens;           /* Number of passed arguments */
      #define MAXTOKS 45
      struct
      {
         char *ptr;     /* Pointer to the string containing an expanded
                           set of tokens, null terminated */
         char *org_at;  /* Pointer to the original string where the
                           token began */
      }tokens[MAXTOKS];

      /* Storage for print parameters */

      #define MAXPRTINTS 10
      int prtints[MAXPRTINTS];  /* Integers to be printed in a string */
      int nprtints;             /* Number of ints to be printed */

/*                                                                        */
/* 4. Other Global Strings                                                */
/*                                                                        */
      #define MAXELINES (80*12)
      char errlines[MAXELINES];  /* Storage for lines of error information*/
      char sense_path[REC_MAX+1]; /* Storage for path to sense files      */

      char sense_rec[REC_MAX+1];  /* Record of the sense file read        */

      char invals[VPOSMAX+1];     /* Storage for sense data values
                                     as single characters. Thus for
                                     01 23 45 67 89 ab cd ef
                                     invals[0] =0  invals[1]=1
                                     invals[2] =2  invals[3]=3 ...
                                     invals[14]=e  invals[15]=f
                                   */

      char err_rec[(VPOSMAX*2)+2];  /* Record of the errlog rep read */

      char main_file[REC_MAX+1];   /* Main file containing list of sense fls*/

/*                                                                        */
/* 5. Other Global Flags, pointers, and integer variables                 */
/*                                                                        */
      int sense_only;      /* True if only sense bits are to be decoded   */
      int gstyle=FALSE;    /* True if processing an errpt -g report       */
      int allerrs=FALSE;   /* Allow all classes of errors                 */
      int incl_err=FALSE;  /* True if this error is to be presented       */
      int table_fmt=FALSE; /* True if summary to be in table fmt only     */
      int in_decodeable=FALSE; /* Storing decodable sense byte info       */
      int end_err=FALSE;   /* Set true at end of an error entry           */
      int in_detdat=FALSE; /* In detail data                              */

      int row_at,next_col; /* Current row at, index into row of next
                                                    column to be processed*/
      int ncmpchars;       /* Number of compare characters found          */

      int trace_exec=FALSE; /* Traces execution for debug purposes        */
      int cron_errs=FALSE;  /* If true gives a chronological error summary
                               in addition to one sorted by type */

      /* Start and stop location of the description tokens */
      int des_start;             /* Token starting description string */
      int des_stop;              /* Token ending description string if end
                                    was not the end of a line */

      char *p_stopstr;           /* Used for strtol conversions to
                                    point to the end of a numeric string*/

      FILE *emsg_file=NULL;       /* Pointer to the error message file */


/*                                                                        */
/* 6. Definitions for external and non integer routines                   */
/*                                                                        */
      extern struct tm *localtime();
      extern char   *asctime();
      extern char *get_senserec(); /* Definition for this routine */
      extern long int get_hint(),parse_expr(), get_ivalue();

main(argc,argv,envp)
int argc;
char **argv[],**envp[];
{
   char *p_rec;
   int at_end=FALSE;

   /* Get need information from environment and initialize variables */
   env_get(argv);

   /* Determine whether there is just sense bytes or whole errpt entries */
   fnd_runmode();

   if ((cron_errs) && (! table_fmt))
   {
     /* Display an initial summary header */
         fprintf(stderr,"\nChronological Summary of Entries\n\n");
   }

   if (sense_only)
   {
     /* Read only the rows of sense information */
     read_sense();

     prt_sense(); /* Print them to stderr */

     nmvars = 0;             /* No variables created at this point */
     /* Decode sense information */
     dec_sense();
   }
   else
   {
     /* Must do the full decoding.  Note the first error record,
        err_rec has already been read in by fnd_runmode as err_rec */
     if (gstyle)
     {
        /* Attempt to get or create the file containing error messages */
        get_emsgs();
     }
     while (! at_end)
     {
        end_err = FALSE;
        at_end = do_err();
     }
     displ_sum();  /* Display summary of found hardware errors */
   }
   exit(0);
}
 env_get(argv)
/* routine::env_get                                                       */
/* Gets information from environment and initializes variables including  */
/* opening the sense decode file if passed as a parm                      */
   char **argv;
{
   char *p_sensep,*getenv(),*p_flag;
   char sense_name[REC_MAX+1], main_passed[REC_MAX+1];
   int ret_code, cnt, need_mainf=FALSE;
   FILE *tmp_fp;

   /* Assume main file is the default given */
   strcpy(main_passed,MAINFILE);

   /* Determine the path to the run time files */

   sense_path[0]=0;

   if ((tmp_fp = fopen("dsense.lib", "r")) != NULL)
   {
     /* dsense.lib exists in the current directory, so we will assume that */
     /* all the run time files are there.                                  */

     fclose(tmp_fp);
     strcpy(sense_path, "./");
   }

   else
   {
     p_sensep = getenv("SENSE_PATH");

     if (p_sensep != NULL)
     {
       strcpy(sense_path,p_sensep);
       strcat(sense_path,"/");
     }

     else
     {
       p_sensep = getenv("DIAGNOSTICS");

       if (p_sensep != NULL)
       {
         strcpy(sense_path, p_sensep);
         strcat(sense_path, "/bin/");
       }

       else
         strcpy(sense_path, "/etc/lpp/diagnostics/bin/");
     }
   }

   /* Handle all arguments and passed parameters */
   ++argv;
   while (*argv != NULL)
   {
      if (need_mainf)
      {
        if (**argv == '-')
        {
          fprintf(stdout,"dsense: No file name followed the -m flag\n");
        }
        else
        {
          strcpy(main_passed,*argv);
          need_mainf = FALSE;
        }
      }
      else if (**argv == '-')
      {
         /* Have flag(s) */
         p_flag = (*argv) + 1; /* Get to first character of flags */
         while (*p_flag != 0)
         {
           if (*p_flag == 'c')
           {
             cron_errs = TRUE;  /* Want errors cronologically */
           }
           else if (*p_flag == 't')
           {
             table_fmt = TRUE; /* Summary in table format only */
           }
           else if (*p_flag == 's')
           {
             allerrs = TRUE; /* All error classes displayed */
           }
           else if (*p_flag == 'm')
           {
             /* Flag for alternate main file name.  Name follows here
                or as next token */
               if (*(p_flag + 1 ) != 0)
               {
                 /* Main file given here */
                 strcpy(main_passed,p_flag + 1);
                 p_flag = p_flag + strlen(p_flag) - 1 ;/* End of argument */
               }
               else
               {
                 /* Must be in next token */
                 need_mainf = TRUE;
               }
           }
           ++p_flag; /* Next flag character */
         }
      }
      else
      {
        /* Have the name of a sense routine to handle */
        sense_only = TRUE;        /* Only handling sense information */
        if (**argv == '?')
        {
          prt_info();  /* User just wants help */
          exit(1);     /* Exit */
        }
        strcpy(sense_name,*argv); /* Have the name of a sense file   */
      }
      ++argv;  /* Go to next argument if any */
   }

   /* Initialize some values */
   sumlines[0]     = 0;               /* No summary lines */
   p_nxtsumline = sumlines;           /* Ptr to summary lines at start*/

   /* Open the main file and use to open and index routines */
   strcpy(main_file,sense_path);
   strcat(main_file,main_passed);
   get_sroutes(main_file);

   if (sense_only)
   {
     /* Index the initial sense file to be worked */
     ret_code = index_senser(sense_name);  /* Open initial sense routine*/
     if (ret_code > 0)
     {
       /* Had an error */
       if (ret_code == 1)
       {
          fprintf(stdout,
    "dsense: Can't find sense routine %s in any of the files named in %s\n",
                sense_name,main_file);
          exit(1);
       }
     }
   }
   /* Ensure short character arrays are NULL terminated at outset */
   err_label[MAXLAB]  = 0;
   err_id[MAXID]      = 0;
   rsrc_name[MAXRNM]  = 0;
   rsrc_type[MAXRTYP] = 0;
   dtype[DTMAX]       = 0;
   err_type[ETYPMAX]   = 0;
   date_time[DTTMMAX] = 0;

}
 fnd_runmode()
/* routine::fnd_runmode                                                  */
/* Attempts to determine if the user just wants sense byte data decoded  */
/* or if the user is attempting to run from an errpt -a listing          */
/* env_get looked for the name of a sense file passed as a parameter.    */
/* if found then the run mode was surely for sense bytes only.  Otherwise*/
/* is assumed to be for the errpt -a option.  However if the data passed */
/* is a row of sense byte data, then user just didn't specify the sense  */
/* file.                                                                 */
/* The first non-blank row is therefore read and looked at               */
{
   int name_error = FALSE, have_firstr=FALSE, cnt;
   char *p_rec,sys_msg[REC_MAX+1];

   if (sense_only == TRUE)
   {
     /* Make sure the name of the sense file was good and could be opened */
     if ( senser_at < 0)
     {
       /* File was not opened */
       name_error = TRUE;
     }
   }
   else
   {
      while (! have_firstr)
      {
        p_rec = fgets(err_rec,(VPOSMAX*2)+1,stdin); /* Get a record */
        if (p_rec == NULL)
        {
           fprintf(stdout,"dsense : No input to translate\n");
           exit(1);
        }
        while ((*p_rec == ' ') || (*p_rec == '\t')) {++p_rec;}
        if (*p_rec != '\n')
        {
          /* At first non blank character of first non-blank line */
          have_firstr = TRUE ;
          if (*p_rec == '?')
          {
             prt_info(); /* User just wants help information */
             exit(1);
          }

          if (isdigit(*p_rec))
          {
            /* Looks like sense byte information, but did not get a name
               of a sense byte file */
            name_error = TRUE;
          }
          else
          {
             /* Should be an errpt -a or errpt -g report, see which */
             sense_only = FALSE;
             if (strncmp(p_rec,"el_",3) == 0)
             {
               /* Must be the errpt -g form */
               gstyle = TRUE;
             }
          }
        }
      }
   }
   if (name_error)
   {
      fprintf(stdout,
      "dsense : Sense data only as input, but no name of error type \n\
       passed as a parameter.  Syntax is  \n\
       dsense sense_name\n\
       Where  sense_name can be one of the following\n");
       for (cnt = 0 ; cnt < nsroutes ; ++cnt)
       {
         fprintf(stdout,"%s\n",sroutes[cnt].name);
       }
       exit(1);
   }
}
int dec_sense()
/* routine::dec_sense                                                     */
/* Reads rows of the sense file and decodes info based on first character */
/* Returns true if could decode okay                                      */
{
   int comp_rec,dokay=TRUE, token_at, end_dec, is_valid=TRUE;

   char *p_srec;

   p_srec=get_senserec();  /* Get a first sense record and parse */

   if_level = 0;
   ifs[if_level].do_block=TRUE;  /* At lowest level will execute all cmds */

   while (p_srec != NULL)
   {
     if ((ntokens > 0) && (is_valid))
     {
        token_at = 0 ;    /* Parameter being looked at */
        comp_rec=FALSE;   /* Presume not a compare record */
        if (*tokens[token_at].ptr == '~')
        {
           /* Have a conditional statement to process */
           cond_proc(token_at+1);
        }
        else if (ifs[if_level].do_block)
        {
           /* Take actions based on first character of line */
           switch(*tokens[token_at].ptr)
           {
              /* Look for simplistic cases */
              case '\n':
              case 0x0c:
              {
                 /* Won't do anything */
                 break;
              }
              case '|':
              {
                 /* Display continuation of last conditional.
                    Do display based on last_ctrue value */
                 if ((last_ctrue) && (ntokens > (token_at + 1)))
                 {
                    dec_rest(token_at,0,TRUE); /* Decode prt tokens */
                    do_descript();          /* Do the printing   */
                 }
                 break;
              }
              case '%':
              {
                 /* Is text to be displayed as shown beginning after the %*/
                 dec_rest(token_at,0,TRUE);
                 do_descript();
                 break;
              }
              case '*':
              {
                 /* Won't do anything */
                 break;
              }
              case '#':
              {
                 /* Is a pragma */
                 do_pragmas(token_at);
                 break;
              }
              default:
              {
                 /* Presume row col with direct number or mvar */
                 comp_rec=TRUE;
                 if (ntokens > 1)
                 {
                    if (strcmp(tokens[token_at + 1].ptr,":=") == 0)
                    {
                       /* Is an expression statement */
                       expr_do(token_at);
                      comp_rec = FALSE;
                    }
                 }
                 else
                 {
                   fprintf(stdout,"dsense : Can't translate line \n");
                   prt_recerr();
                   exit(1);
                 }
              }
           } /* End switch */
           if (comp_rec)
           {
              dokay = dec_rcrec(token_at);
              is_valid = tst_fields(dokay);
              if (is_valid)
              {
                /* Need to take display actions based on decoded symbol */
                do_comp();
              }
              else
              {
                fprintf(stdout,"dsense: There may be more errors but they will not be displayed\n");
              }
           }
        } /* End if/elseif */
     } /* End if ntokens */
     p_srec=get_senserec();  /* Get next sense record */
   } /* End while */
}
get_emsgs()
/* routine::get_emsgs                                                     */
/* Attempts to find an existing file EMSGFILE first in the SENSE_PATH     */
/* directory.  If the file can not be found, will attempt to create by    */
/* the system command errmsg -w E                                         */
{
   char msgfilenm[REC_MAX+1],sys_cmnd[REC_MAX+1];

   sprintf(msgfilenm,"%s/%s",sense_path,EMSGFILE);

   /* Attempt to open this file for reading only */
   emsg_file = fopen(msgfilenm,"r");
   if (emsg_file == NULL)
   {
     /* Could not open the file.  Will attempt to create the file */
     sprintf(sys_cmnd,"errmsg -w E > %s",msgfilenm);
     system(sys_cmnd);

     /* Will now try to read the file */
     emsg_file = fopen(msgfilenm,"r");
     if (emsg_file == NULL)
     {
       /* File could not be created. Print message */
       fprintf(stdout,
     "dsense: Can not create or access err descriptor file %s\n",msgfilenm);
       fprintf(stdout,"        Will print error descriptor numbers\n");
     }
   }
}
do_err()
/* integer routine::do_err                                                */
/* Reads in and translates one error entry from the passed error log      */
/* Returns TRUE when the last record has been read from the passed log    */
/* Reads from the file using the global array err_rec                     */
/* Assumes when called that the first line of err_rec has already been    */
/* read                                                                   */
{
   char *p_rec;
   int end_recs=FALSE,  had_sense=FALSE;

   /* Set up to handle the record */
       errlines[0]=0;         /* No lines of error text stored */
   err_descript[0]=0;         /* No error description found    */
      err_label[0]=0;         /* No error label                */
         err_id[0]=0;         /* No error ID                   */
      rsrc_type[0]=0;         /* No resource type              */
      date_time[0]=0;         /* No data/time stamp            */
       err_type[0]=0;         /* No error type                 */
          incl_err=FALSE;

            nmvars=0;         /* No variables thus far */

   /* Set to null predefined values */
   mvars[mvar_find("ERR_ID",TRUE,TRUE)].val    = (long) -1;
   mvars[mvar_find("ERR_LABEL",TRUE,TRUE)].val = (long) -1;
   mvars[mvar_find("SEQ_NUM",TRUE,TRUE)].val   = (long) -1;

   p_rec = err_rec;  /* Get pointer to current record to be xlated */
   while ((! end_recs) && (! end_err))
   {
     if (p_rec == NULL)
     {
       end_recs  = TRUE;  /* End of records */
       end_err = TRUE;         /* End this error */
       in_decodeable = FALSE;
       in_detdat = FALSE;
     }
     else
     {
        if (gstyle)
        {
           parse_grec(p_rec); /* Handle a -g style record */
        }
        else
        {
           parse_arec(p_rec); /* Handle a -a style record */
        }
        if (! end_err)
        {
           if (in_decodeable) {had_sense = TRUE;}
           if ((incl_err) && (! table_fmt))
           {
             fputs(p_rec,stdout); /* Put out the record */
           }
           else if (((strlen(errlines) + strlen(p_rec)) < MAXELINES) &&
                       ( ! in_decodeable))
           {
             /* Store line for possible later prt */
             strcat(errlines,p_rec);
           }
        }
     }
     if (end_err)
     {
       /* At end of the error */
       if (incl_err)
       {
          if (had_sense)
          {
            fputs("\n", stdout);
            dec_sense();       /* Decode sense data and display */
          }
          if (cron_errs)
          {
             cron_disp();      /* Display the error summary now as is */
          }
          sum_err();           /* Store error summary */
          if (! table_fmt)
          {
             fputs(p_rec,stdout); /* Put out the final ---- record */
          }
       }
     }
     if (! end_recs)
     {
       p_rec = fgets(err_rec,(VPOSMAX*2)+1,stdin); /* Get next record */
     }
   }
   if (p_rec == NULL)
   {
     end_recs = TRUE;
   }
   return(end_recs);
}
get_sroutes(p_mname)
/* routine::get_sroutes                                                   */
/* Opens the passed file which is to contain a list of all sense          */
/* routine files desired to be used in the decode. Opens each file and    */
/* creates an index of the routines in each file                          */
/* The format of the file must be
~file1.s
~file2.s
~file3.s
~...
~filen.s
    Where any line whose first non-blank character is not a ~ is ignored
*/
   char *p_mname;
{
   char *p_rec;
   FILE *mainf;
   char needed_file[REC_MAX+1];
   int line_cnt=0;

   /* Attempt to open the main file */
   mainf = fopen(p_mname,"r");
   if (mainf == NULL)
   {
      fprintf(stdout,
                     "dsense: Couldnt open list of sense routine files %s\n",
                       p_mname);
      perror("dsense");
      exit(1);
   }

   p_rec = fgets(sense_rec,REC_MAX,mainf);

   while (p_rec != NULL)
   {
      /* Parse tokens read */
      parse_tokens(p_rec);

      if ((ntokens > 1) && (*tokens[0].ptr == '~'))
      {
         /* Have a file to open and index */
         strcpy(needed_file,sense_path);
         strcat(needed_file,tokens[1].ptr);
         get_sfindex(needed_file);
      }
      /* Get next record */
      p_rec = fgets(sense_rec,REC_MAX,mainf);
   }
}
get_sfindex(p_sfname)
/* routine::get_sfindex                                                   */
/* Opens a file containing sense routines.  Creates an index for all of   */
/* the routines in the file.                                              */
   char *p_sfname;
{
   char *p_rec;
   int line_cnt=0;

   if (nsenfiles >= MAXSFILES)
   {
     fprintf(stdout,
     "Couldnt open file %s as the max (%d) sense files already open\n",
         p_sfname,MAXSFILES);
     exit(1);
   }

   /* Attempt to open the file */
   sen_files[nsenfiles].fptr = fopen(p_sfname,"r");
   if (sen_files[nsenfiles].fptr == NULL)
   {
      fprintf(stdout,"Could not open sense routines file %s\n",p_sfname);
      perror("dsense");
      exit(1);
   }
   strcpy(sen_files[nsenfiles].name,p_sfname); /* Copy over name*/

   /* Now attempt to read and index all of routines within the file */
   p_rec = fgets(sense_rec,REC_MAX,sen_files[nsenfiles].fptr);

   while (p_rec != NULL)
   {
      ++line_cnt; /* Increase count of lines, starts with 1 */
      while ((*p_rec == ' ') || (*p_rec == '\t')) {++p_rec;}
      if (*p_rec == '!')
      {
         /* Have what is supposed to be a start of a routine */
         parse_tokens(p_rec);

        if (ntokens < 2)
        {
           fprintf(stdout,"dsense : Sense routine found with no name\n");
           fprintf(stdout,"         Line %d of %s \n%s",
               line_cnt,sen_files[nsenfiles].name,sense_rec);
           exit(1);
        }
        if (nsroutes >= (MAXSDEFS -1))
        {
           fprintf(stdout,
           "dsense : Too many sense routines in %s (%d is max)\n",
              sen_files[nsenfiles].name,MAXSDEFS);
           exit(1);
        }
        /* Have a routine, will copy needed info down */
        strncpy(sroutes[nsroutes].name,tokens[1].ptr,MAXRTYP);
        sroutes[nsroutes].name[MAXRTYP] = 0;

        sroutes[nsroutes].sindex  = nsenfiles;
        sroutes[nsroutes].seek_at = ftell(sen_files[nsenfiles].fptr);
        sroutes[nsroutes].lno     = line_cnt + 1;
        ++nsroutes;
      }
      /* Get next record */
      p_rec = fgets(sense_rec,REC_MAX,sen_files[nsenfiles].fptr);
   }
   ++nsenfiles; /* Record the completion of a sense file */
}
read_sense()
/* routine::read_sense                                                    */
/* Gets rows of sense data and stores the rows into the array for the data*/
/* Input is expected in the form of xxxx yyyy zzzz and so forth           */
{
   char rec_red[REC_MAX+1],*p_rec;
   int dec_err=FALSE;

   row_at   =  0;  /* Start of first row */
   next_col =  0;  /* Will be at 0 index into the next row */
   p_rec = fgets(rec_red,REC_MAX+1,stdin);

   while ((p_rec != NULL) && (! dec_err))
   {
      dec_err = xlate_inrow(p_rec);  /* Translate the row */
      p_rec = fgets(rec_red,REC_MAX+1,stdin); /* Get new record */
   } /* End While */
   if (dec_err)
   {
     fprintf(stdout,
"dsense:Input data is not in proper format\n\
       Format is up to %d rows of information %d columns wide\n\
       In the format of\n\
xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx\n\
       Last line of input was\n\
%s\n\
       Last row and column decoded were %d and %d (0 0 = first row and col)\n"
, NMAXROWS, MAXINDIGS/4,rec_red,row_at,(next_col)/4);
     exit(1);
   }
}
 int index_senser(p_name)
/* routine::index_senser                                                  */
/* Seeks to the start of the desired sense routine and sets points to     */
/* match.                                                                 */
/* Returns as follows
     0 = Indexed routine okay
     1 = File Could not open as routine did not exist.  Will display error
         and exit if not first routine to be indexed
*/
   char *p_name;
{
   int ret_code=0,fnd_route=FALSE,cnt,rc;
   char route_name[REC_MAX+1];

   if (p_name == NULL)
   {
      fprintf(stdout,"dsense : No name of sense file to open given\n");
      exit(1);
   }

   ++senser_at; /* Increment index into sense file array */
   if (senser_at >= MAXOPROUTES)
   {
      fprintf(stdout,"dsense : More than max of %d sense files nested\n",
          MAXOPROUTES);
      exit(1);
   }

   /* Will attempt to find the routine */
   for (cnt = 0 ; (cnt < nsroutes) && (! fnd_route) ; ++cnt)
   {
     if (strcmp(p_name,sroutes[cnt].name) == 0)
     {
        fnd_route = TRUE; /* Routine found */
        /* Seek to it */
        rc = fseek(sen_files[sroutes[cnt].sindex].fptr,
                          sroutes[cnt].seek_at,0);
        if (rc != 0)
        {
          fprintf(stdout,"dsense : Error seeking to %lx (line %d) of %s\n",
            sroutes[cnt].seek_at,sroutes[cnt].lno,
                                 sen_files[sroutes[cnt].sindex].name);
          exit (1);
        }
        /* Update structure concerning the routine */
        senser[senser_at].s_at   = sroutes[cnt].seek_at;
        senser[senser_at].lno    = sroutes[cnt].lno;
        senser[senser_at].rindex = cnt;
        senser[senser_at].fptr   = sen_files[sroutes[cnt].sindex].fptr;
     }
   }
   if (! fnd_route)
   {
     /* Routine not found */
     if (senser_at > 0)
     {
       /* Need to print error message and exit */
       fprintf(stdout,
    "dsense : Routine %s called for routine %s which is not found in %s\n",
     sroutes[senser_at - 1].name,p_name,sen_files[sroutes[cnt].sindex].name);
       exit(1);
     }
     /* If was the first file, will just want to return a 1 */

     --senser_at;
     ret_code = 1;
   }
   return(ret_code);
}
int xlate_inrow(p_str)
/* routine::xlate_inrow                                                   */
/* Gets all sense value words in one row of sense data passed             */
/* Input is expected in the form of xxxx yyyy zzzz and so forth           */
/* Returns true if row was successfully translated and false otherwise    */
   char *p_str;
{
   char  *p_at,byte_rec[REC_MAX+1];
   int end_line=FALSE,vpos,dec_err=FALSE,dokay;

   strcpy(byte_rec,p_str);  /* Copy over record read */
   p_at = byte_rec;
   while ((*p_at == ' ') || (*p_at == '\t')) {++p_at;} /* Discard leading*/

   if ((*p_at != '\n') && (*p_at != 0))
   {
      strtok(p_at,"\n");            /* Get first word */
      p_at = strtok(p_at," \t");
      while ((! end_line) && (! dec_err))
      {
        if (strlen(p_at) > 4)
        {
          dec_err = TRUE;
        }
        else if (! dec_err)
        {
          if (next_col >= MAXINDIGS)
          {
            /* Go on to the next line */
            ++row_at; /* Now in a new row */
            if (row_at >= NMAXROWS){  dec_err = TRUE;  }
            next_col = 0;
          }
          if (! dec_err)
          {
            vpos  = get_vpos(row_at,next_col);  /* Get position of word */
            dokay = get_bytes(p_at,&invals[vpos],4); /* Get word */
            next_col = next_col + 4; /* 4 New digits */
            dec_err = (dokay == FALSE) ;
          }
        }
        p_at = strtok(NULL," \t"); /* Get to next number */
        if (p_at == NULL)
        {
          end_line = TRUE;
        }
      } /* End while */
   } /* End if */
   return (dec_err);
}
get_errdes(p_errdes,desnumb)
/* routine::get_errdes                                                    */
/* Puts into the string pointed to p p_errdes, the error description in   */
/* the error file indexed by desnumb                                      */
/* If the description file is not open, will return desnumb as description*/
   char *p_errdes;
   int desnumb;
{
   int not_found=TRUE,at_end = FALSE;
   char desc_msg[REC_MAX+1];
   char numb_str[30];
   char *p_desc;

   /* Default to just return the error message number */
   sprintf(p_errdes,"Error Description Number %X\n",desnumb);

   /* Get correct string representation of the error desc number */
   sprintf(numb_str,"%X",desnumb);

   if (emsg_file != NULL)
   {
      /* Will try looking up the description in the error message file */
      rewind(emsg_file);
      while (not_found && (! at_end))
      {
         p_desc = fgets(desc_msg,REC_MAX,emsg_file);
         if (p_desc == NULL)
         {
            at_end = TRUE;
         }
         else
         {
            if (strncmp(numb_str,desc_msg,4) == 0)
            {
               /* Have found the desired message
                  in the form NNNN "Message"\n
                  Will store without the " " and with a trailing \n */

               not_found = FALSE;
               p_desc = p_desc + 4;
               while ((*p_desc == '\t') || (*p_desc == ' ')) {++p_desc;}

               ++p_desc; /* Skip past the initial " */
               strcpy(p_errdes,p_desc);
               strtok(p_errdes,"\"\n");
               strcat(p_errdes,"\n"); /* Add back trailing carriage ret*/
            }
         }
      }
   }
}
parse_arec(p_rec)
/* routine::parse_arec                                                   */
/* Handles the passed record of the errpt -a report. Parsing and taking  */
/* Needed action                                                         */
   char *p_rec;
{
   char *p_parse,one_rec[REC_MAX+1];
   int ret_code;
   int row_store,col_store,dec_err;
   long int mval;

   strcpy(one_rec,p_rec);
   p_parse = one_rec;
   while ((*p_parse == ' ') || (*p_parse == '\t')) { ++p_parse;}

   if (need_errdes)
   {
     /* Last line flagged to look for error description here */
     strcpy(err_descript,one_rec);
     need_errdes = FALSE;
   }
   else if (strncmp(p_parse,"-----",5) == 0)
   {
      end_err   = TRUE;   /* End this error */
      in_decodeable = FALSE; /* End of sense byte data decoding if any */
      in_detdat = FALSE;
   }
   else if ((strncmp(p_parse,"ERROR LABEL:",12) == 0) ||
            (strncmp(p_parse,"LABEL:",6) == 0))
   {
     /* Have found an error label for a new problem
        will store the label */
     if (*p_parse == 'E')
     {
        p_parse = p_parse + 12;
     }
     else
     {
        p_parse = p_parse + 6;
     }
     while ((*p_parse == ' ' ) || (*p_parse == '\t')) {++p_parse;}
     strtok(p_parse," \t\n");
     strncpy(err_label,p_parse,MAXLAB);   /* Copy found label */

     /* Store number associated with the label as a variable */
     sscanf(err_label,"%ld",&mval);
     mvars[mvar_find("ERR_LABEL",TRUE,TRUE)].val = mval;
   }
   else if ((strncmp(p_parse,"ERROR ID:",9) == 0) ||
            (strncmp(p_parse,"IDENTIFIER:",11) == 0))
   {
     /* Have found error id, will store the ID */
     if (*p_parse == 'E')
     {
        p_parse = p_parse + 9;
     }
     else
     {
        p_parse = p_parse + 11;
     }
     while ((*p_parse == ' ' ) || (*p_parse == '\t')) {++p_parse;}
     strtok(p_parse," \t\n");
     strncpy(err_id,p_parse,MAXID);

     /* Store number associated with the id as a variable */
     mval = strtol(err_id,&p_stopstr,16);
     mvars[mvar_find("ERR_ID",TRUE,TRUE)].val = mval;
   }
   else if (strncmp(p_parse,"Date/Time:",10) == 0)
   {
     /* Have found date, and time.  Will store */
     p_parse = p_parse + 10;
     while ((*p_parse == ' ' ) || (*p_parse == '\t')) {++p_parse;}
     strtok(p_parse,"\n");
     strncpy(date_time,p_parse,DTTMMAX);
   }
   else if (strncmp(p_parse,"Sequence Number:",16) == 0)
   {
     /* Have found the sequence number. Will store */
     p_parse = p_parse + 16;
     while ((*p_parse == ' ' ) || (*p_parse == '\t')) {++p_parse;}
     strtok(p_parse,"\n");

     /* Store sequence number as a variable */
     mval = strtol(p_parse,&p_stopstr,0);
     mvars[mvar_find("SEQ_NUM",TRUE,TRUE)].val = mval;
   }
   else if ((strncmp(p_parse,"Error Class:",12) == 0) ||
            (strncmp(p_parse,"Class:",6) == 0))
   {
     /* Have found error class, will see if hw or sw */
     if (*p_parse == 'E')
     {
        p_parse = p_parse + 12;
     }
     else
     {
        p_parse = p_parse + 6;
     }
     while ((*p_parse == ' ' ) || (*p_parse == '\t')) {++p_parse;}
     if ((*p_parse == 'H') || (*p_parse == 'O') || (allerrs))
     {
       incl_err = TRUE; /* Will include this error */
       if (! table_fmt)
       {
         fputs(errlines,stdout); /* Print out stored lines */
       }
     }
   }
   else if ((strncmp(p_parse,"Error Type:",11) == 0) ||
            (strncmp(p_parse,"Type:",5) == 0))
   {
     /* Have found error type, will store */
     if (*p_parse == 'E')
     {
        p_parse = p_parse + 11;
     }
     else
     {
        p_parse = p_parse + 5;
     }
     while ((*p_parse == ' ' ) || (*p_parse == '\t')) {++p_parse;}
     strncpy(err_type,p_parse,ETYPMAX);
     strtok(err_type," \t\n");
   }
   else if (strncmp(p_parse,"Resource Name:",14) == 0)
   {
     /* Have found the resource name, store it */
     p_parse = p_parse + 14;
     while ((*p_parse == ' ' ) || (*p_parse == '\t')) {++p_parse;}
     strtok(p_parse," \t\n");
     strncpy(rsrc_name,p_parse,MAXRNM);  /* Store resource name */
   }
   else if (strncmp(p_parse,"Resource Type:",14) == 0)
   {
     /* Have found the resource type, store it */
     p_parse = p_parse + 14;
     while ((*p_parse == ' ' ) || (*p_parse == '\t')) {++p_parse;}
     strtok(p_parse," \t\n");
     strncpy(rsrc_type,p_parse,MAXRTYP);  /* Store resource type */
   }
   else if ((strncmp(p_parse,"Error Description",17) == 0) ||
            (strncmp(p_parse,"Description",11) == 0))
   {
     /* Next line should be the error description */
     need_errdes = TRUE;
   }
   else if (strncmp(p_parse,"Detail Data",10) == 0)
   {
      if (incl_err)
      {
        /* See if this is something that can be decoded */
        in_decodeable = check_decode();
        in_detdat = TRUE; /* In detail data regardless if decodeable*/
      }
   }
   else if (in_decodeable)
   {
     /* Will attempt to translate the input row */
     row_store = row_at;
     col_store = next_col;
     /* Translate a row of input data */
     dec_err = xlate_inrow(p_rec);
     if (dec_err)
     {
       /* Did not translate due to non-hex characters, will skip the
          line as presuming the line was a line of header info such as
          SENSE BYTE
       */
       row_at   = row_store;
       next_col = col_store;
     }
   }
   else if (in_detdat)
   {
     if (strcmp(err_label,"OPMSG") == 0)
     {
       /* Store last line of Detail data as err description */
       strcpy(err_descript,p_parse);
     }
   }
}
parse_grec(p_rec)
/* routine::parse_grec                                                   */
/* Handles the passed record of the errpt -g report. Parsing and taking  */
/* Needed action                                                         */
/* Note, this routine will parse the passed record using strtok          */
   char *p_rec;
{
   char *p_parse, asc_store[3];
   int  des_numb,mvindex,ed_at,ascii_int;
   long int mval;
   time_t timestamp;
   struct tm *timestruct;
   char *p_timestr;

   p_parse = p_rec;
   while ((*p_parse == ' ') || (*p_parse == '\t')) { ++p_parse;}

   if (strncmp(p_parse,"@@",2) == 0)
   {
      end_err   = TRUE;   /* End this error */
      in_decodeable = FALSE;  /* End of sense byte data decoding if any */
   }
   else if (strncmp(p_parse,"el_sequence",11) == 0)
   {
     /* Have found the sequence number of a problem.  Will store it */
     p_parse = p_parse + 11;
     while ((*p_parse == ' ' ) || (*p_parse == '\t')) {++p_parse;}

     /* Store the sequence number */
     mval = strtol(p_parse,&p_stopstr,0);
     mvars[mvar_find("SEQ_NUM",TRUE,TRUE)].val = mval;
   }
   else if (strncmp(p_parse,"el_label",8) == 0)
   {
     /* Have found an error label for a new problem
        will store the label */
     p_parse = p_parse + 8;
     while ((*p_parse == ' ' ) || (*p_parse == '\t')) {++p_parse;}
     strncpy(err_label,p_parse,MAXLAB);   /* Copy found label */
     strtok(err_label," \t\n");

     /* Store the error label number */
     sscanf(err_label,"%ld",&mval);
     mvars[mvar_find("ERR_LABEL",TRUE,TRUE)].val = mval;
   }
   else if (strncmp(p_parse,"el_crcid",8) == 0)
   {
     /* Have found error id, will store the ID */
     p_parse = p_parse + 8;
     while ((*p_parse == ' ' ) || (*p_parse == '\t')) {++p_parse;}
     /* Id is in form of 0xnnnnnnnn, will record as nnnnnnnn */
     strncpy(err_id,p_parse+2,MAXID);
     strtok(err_id," \t\n");

     /* Get number associated with the id */
     mval = strtol(p_parse,&p_stopstr,16);
     mvars[mvar_find("ERR_ID",TRUE,TRUE)].val = mval;
   }
   else if (strncmp(p_parse,"el_timestamp",12) == 0)
   {
     /* Have found date and time as a time stamp. Will convert */
     p_parse = p_parse + 12;
     while ((*p_parse == ' ' ) || (*p_parse == '\t')) {++p_parse;}
     timestamp = strtol(p_parse,&p_stopstr,0);
     /* Convert timestamp to a time structure */
     timestruct = localtime(&timestamp);

     /* Get the 26 character time string in form
        Day Mon dd HH:MM:SS yyyy\n*/

     p_timestr  = asctime(timestruct);
     /* Store needed values */
     *(p_timestr + 19) = 0 ;
     sprintf(date_time,"%s",p_timestr+4);
   }
   else if (strncmp(p_parse,"el_class",8) == 0)
   {
     /* Have found error class, will see if hw or sw */
     p_parse = p_parse + 8;
     while ((*p_parse == ' ' ) || (*p_parse == '\t')) {++p_parse;}
     if ((*p_parse == 'H') || (*p_parse == 'O') || (allerrs))
     {
       incl_err = TRUE; /* Have a hardware error */
       if (! table_fmt)
       {
         fputs(errlines,stdout); /* Print out stored lines */
       }
     }
   }
   else if (strncmp(p_parse,"el_type",7) == 0)
   {
     /* Have found error type, will store */
     p_parse = p_parse + 7;
     while ((*p_parse == ' ' ) || (*p_parse == '\t')) {++p_parse;}
     strncpy(err_type,p_parse,ETYPMAX);
     strtok(err_type," \t\n");
   }
   else if (strncmp(p_parse,"el_resource",11) == 0)
   {
     /* Have found the resource name, store it */
     p_parse = p_parse + 11;
     while ((*p_parse == ' ' ) || (*p_parse == '\t')) {++p_parse;}
     strncpy(rsrc_name,p_parse,MAXRNM);  /* Store resource name */
     strtok(rsrc_name," \t\n");
   }
   else if (strncmp(p_parse,"el_rtype:",8) == 0)
   {
     /* Have found the resource type, store it */
     p_parse = p_parse + 8;
     while ((*p_parse == ' ' ) || (*p_parse == '\t')) {++p_parse;}
     strncpy(rsrc_type,p_parse,MAXRTYP);  /* Store resource type */
     strtok(rsrc_type," \t\n");
   }
   else if (strncmp(p_parse,"et_desc",7) == 0)
   {
     /* Have run across the error description number */
     p_parse = p_parse + 7;
     while ((*p_parse == ' ' ) || (*p_parse == '\t')) {++p_parse;}
     des_numb = strtol(p_parse,&p_stopstr,0);

     /* Look up error description from the number */
     get_errdes(err_descript,des_numb);
   }
   else if (strncmp(p_parse,"el_detail_data",14) == 0)
   {
      p_parse = p_parse + 14;
      while ((*p_parse == ' ' ) || (*p_parse == '\t')) {++p_parse;}
      if (incl_err)
      {
        /* See if this is something that can be decoded */
        in_decodeable = check_decode();
        if (in_decodeable)
        {
          /* Copy over the sense byte string */
          strncpy(invals,p_parse,VPOSMAX);
          invals[VPOSMAX]=0;

          /* Determine row_at and col_at for what would have been the
             next row,col of sense data */
          row_at   = strlen(invals) / MAXINDIGS;
          next_col = strlen(invals) % MAXINDIGS;
        }
        else if (strcmp(err_label,"OPMSG") == 0)
        {
          /* Have a pure ASCII operator message.  Want to store last line
             of it to error description */
          ed_at=0;
          asc_store[2] = 0;
          while ((*p_parse != '\n') && (*p_parse != 0))
          {
             asc_store[0] = *p_parse;
             ++p_parse;
             asc_store[1] = *p_parse;
             ascii_int = strtol(asc_store,&p_stopstr,16);
             err_descript[ed_at] = ascii_int;
             err_descript[ed_at+1] = 0;
             ++ed_at;
             if (ascii_int == 0x0a)
             {
               /* New line.  Put out old line */
               fprintf(stdout,"                    %s",err_descript);
               /* New line start for any new characters */
               ed_at = 0;
             }
             ++p_parse;
          }
          if (ascii_int != 0x0a)
          {
            /* Did not end with a carriage return in text, make it do
               so and put out the line */
            strcat(err_descript,"\n");
            fprintf(stdout,"                    %s",err_descript);
          }
        }
      }
   }
}
int check_decode()
/* routine::check_decode                                                 */
/* Checks to see if there are any routines anywhere to decode sense data */
/* For the error log entry                                               */
/* Returns True if there is and indexes to the decode routine            */
{
   int failed_index = TRUE; /* Assume can not find sense decode routine */

   /* Open the new sense information if possible base rsrc type */
   failed_index = index_senser(rsrc_type);
   if (failed_index != 0)
   {
     /* Can't open based on type, try nased on resource name */
     failed_index = index_senser(rsrc_name);
   }
   if (failed_index != 0)
   {
     /* Can't open based on resource name, will try error Label */
     failed_index = index_senser(err_label);
   }
   if (failed_index != 0)
   {
     /* Can't open based on error label, will try ID */
     failed_index = index_senser(err_id);
   }
   if (failed_index == 0)
   {
      row_at   =  0;  /* Start of first row */
      next_col =  0;  /* Will be at 0 index into the next row */
   }
   if (failed_index)
   {
     return(FALSE);
   }
   else
   {
     return(TRUE);
   }
}
prt_sense()
/* routine::prt_sense                                                 */
/* Prints out row and columns based on translated hex characters in invals*/
{
   int xrow,xcol,vpos;

   fprintf(stderr,"\n\nRead in %d rows with last row having %d columns\n",
                  row_at+1,next_col/2);

   for (xrow = 0 ; xrow <= row_at ; ++xrow)
   {
      for (xcol = 0; xcol < (MAXINDIGS); xcol = xcol + 4)
      {
        if ((xrow != row_at) || (xcol <= next_col))
        {
          vpos = get_vpos(xrow,xcol);
          fprintf(stderr,"%c%c%c%c ",invals[vpos],invals[vpos+1],
                                     invals[vpos+2],invals[vpos+3]);
        }
      }
      fprintf(stderr,"\n");
   }
   fflush(stderr);
}
displ_sum()
/* routine::displ_sum                                                     */
/* Prints out summary information concerning errors logged                */
/* Errors are ordered so that all errors of a given resource type are     */
/* printed together and within the type all errors of the same name       */
/* are printed together                                                   */

{
   int errscnt, typecnt, namecnt, rtype_at, rname_at;

   /* Print the header */
   if (! table_fmt)
   {
     fprintf(stderr,"\nSummary of Entries\n\n");
   }

   for (errscnt = 0 ; errscnt < nerrids; ++errscnt)
   {
     /* Step through the entire list of errors looking for new resources*/
     if (! errtypes[errscnt].did_prt)
     {
        /* Have a Resource Type not yet printed */

        if (! table_fmt)
        {
          /* Print out header for the error */
          fprintf(stderr,
              "Errors on Resource Type %s\n",errtypes[errscnt].rtype);
        }
        /* Record the error type */
        rtype_at = errscnt;

        /* Search through all errors from here in looking for matches for
           the type. Note the first match will be the error just found */
        for (typecnt = errscnt; typecnt < nerrids ; ++typecnt)
        {
           if ( (! errtypes[typecnt].did_prt) &&
                (strcmp(errtypes[typecnt].rtype,errtypes[rtype_at].rtype)
                 == 0))
           {
              /* Next entry of the type, display and store name of rsrc*/
              rname_at = typecnt;
              /* Go through list of this and all remaining errors printing
                 those with the same resource name. Note the first match
                 will be the error just found */
              for (namecnt = typecnt ; namecnt < nerrids; ++namecnt)
              {
                if ( (! errtypes[namecnt].did_prt) &&
                 (strcmp(errtypes[namecnt].rtype,errtypes[rtype_at].rtype)
                    == 0) &&
                 (strcmp(errtypes[namecnt].rname,errtypes[rname_at].rname)
                    == 0))
                 {
                    wrt_sum(namecnt); /* Write summary lines out */

                    /* Did print this value */
                    errtypes[namecnt].did_prt = TRUE;
                 }
              }
           }
        }
     }
   }
}
sum_err()
/* routine::sum_err                                                       */
/* Adds an error id to the list of error ids or if an old one adds to the */
/* count of times the id had occurred                                     */
{
   int cnt,have_match=FALSE,match_at=0;

   for (cnt = 0 ; ((cnt < nerrids) && (! have_match)) ; ++cnt)
   {
       if ((strcmp(err_id,errtypes[cnt].id) == 0) &&
           (strcmp(rsrc_type,errtypes[cnt].rtype) == 0) &&
           (strcmp(rsrc_name,errtypes[cnt].rname) == 0))
       {
          /* Err ID name and type matches */
          if (errtypes[cnt].p_sumline == NULL)
          {
             if (err_descript[0] == 0)
             {
               have_match = TRUE;
               match_at = cnt;
             }
          }
          else if (err_descript[0] != 0)
          {
             if (strcmp(err_descript,errtypes[cnt].p_sumline) == 0)
             {
                have_match = TRUE;       /* Found an existing error id */
                match_at = cnt;
             }
          }
       }
   }

   if (have_match)
   {
      ++errtypes[match_at].cnt;  /* Increment the number of counts */
   }
   else
   {
     /* Couldn't find the id anywhere, will store a new one */
     if (nerrids >= (MAXCTYPES -1))
     {
        fprintf(stderr,
"erran : More distinct errors than expected. Summary will be truncated\n");
     }
     else
     {
        /* Store Label, Name, Type */
        strncpy(errtypes[nerrids].id,err_id,MAXID);
        errtypes[nerrids].id[MAXID]      = 0;

        strncpy(errtypes[nerrids].label,err_label,MAXLAB);
        errtypes[nerrids].label[MAXLAB]  = 0;

        strncpy(errtypes[nerrids].rname,rsrc_name,MAXRNM);
        errtypes[nerrids].rname[MAXRNM]  = 0;

        strncpy(errtypes[nerrids].rtype,rsrc_type,MAXRTYP);
        errtypes[nerrids].rtype[MAXRTYP] = 0;

        errtypes[nerrids].cnt = 1;                 /* Set cnt = 1 */
        errtypes[nerrids].did_prt=FALSE;           /* Didn't yet print it*/

        if (err_descript[0] == 0)
        {
          errtypes[nerrids].p_sumline = NULL; /* No summary line */
        }
        else
        {
          /* Unique error summary id */
          if ((p_nxtsumline - sumlines + strlen(err_descript) + 1 )
                                                           >= MAXSUMLINES)
          {
             fprintf(stderr,
               " dsense: Too many unique errors to summarize properly\n");
          }
          else
          {
             /* Copy over the summary */
             strcpy(p_nxtsumline,err_descript);
             /* Store pointer to the summary */
             errtypes[nerrids].p_sumline = p_nxtsumline;
             /* Get to next position for copying over the line */
             p_nxtsumline = p_nxtsumline + strlen(p_nxtsumline);
             ++p_nxtsumline;
          }
        }
        ++nerrids;  /* One more error id found */
     }
   }
}
cron_disp()
/* routine::cron_disp                                                     */
/* Displays an error summary description in cronological order            */
{
   if (table_fmt)
   {
     /* Put info out strictly as a table */
     wrt_colstr(date_time,  17, stderr);
     wrt_colstr(err_type,    7, stderr);
     wrt_colstr(err_label,  16, stderr);
     wrt_colstr(err_id,     10, stderr);
     wrt_colstr(rsrc_type,  10, stderr);
     wrt_colstr(rsrc_name,  20, stderr);
     wrt_colstr(err_descript,80,stderr);
     fputc('\n',stderr);
   }
   else
   {
     /* Put out in a readable fmt */
     wrt_colstr(date_time,  17, stderr);
     wrt_colstr(err_type,    6, stderr);
     wrt_colstr(err_label,  16, stderr);
     wrt_colstr("(",         1, stderr);
     wrt_colstr(err_id,      8, stderr);
     wrt_colstr(")",         3, stderr);
     wrt_colstr("Device = ", 9, stderr);
     wrt_colstr(rsrc_type,  10, stderr);
     wrt_colstr(rsrc_name,  10, stderr);
     fputs("\n   ",stderr);
     fputs(err_descript,stderr);
   }
}
wrt_sum(err_at)
/* routine::wrt_sum                                                       */
/* Puts out a line of summary for error indexed at err_at of errtypes     */
   int err_at;
{
   if (table_fmt)
   {
       /* Put out information in a table format */
       wrt_colstr(errtypes[err_at].label, 20, stderr);
       wrt_colstr(errtypes[err_at].id,    10, stderr);
       wrt_colstr(errtypes[err_at].rtype, 20, stderr);
       wrt_colstr(errtypes[err_at].rname, 20, stderr);
       fprintf(stderr,"%.5d     %s",errtypes[err_at].cnt,
                                 errtypes[err_at].p_sumline);
                    /* Did print this value */
   }
   else
   {
       /* Put out information in a readable fmt */
       wrt_colstr("     Error ",           11, stderr);
       wrt_colstr(errtypes[err_at].label, 16, stderr);
       wrt_colstr("(",                      1, stderr);
       wrt_colstr(errtypes[err_at].id,     8, stderr);
       wrt_colstr(") on resource ",        14, stderr);
       wrt_colstr(errtypes[err_at].rname, 10, stderr);
       wrt_colstr("appeared ",              9, stderr);
       fprintf(stderr,"%d times\n       %s",
          errtypes[err_at].cnt,errtypes[err_at].p_sumline);
   }
}
char *get_senserec()
/* char *routine::get_senserec                                            */
/*   1) Reads in next sense record                                        */
/*   2) Copies to parsed token storage and parses arguments               */
/*   3) Returns a pointer to the sense record read. Returns NULL if at    */
/*      the end of all of the stack of routines                           */
{
   char *ret_sense=NULL;
   int cnt,end_srch=FALSE,route_end=FALSE,rc;

   ret_sense = NULL;  /* Assume no record */
   ntokens = 0;

   while (! end_srch)
   {
     /* Get the next record of the file */
     ret_sense = fgets(sense_rec,REC_MAX+1, senser[senser_at].fptr);
     ++senser[senser_at].lno;

     if (ret_sense == NULL)
     {
       route_end = TRUE;  /* At the end of this file and hence cur routine */
     }
     else
     {
       /* Parse the tokens of the sense record */
       parse_tokens(sense_rec);
       if (strcmp(tokens[0].ptr,"!") == 0)
       {
         route_end = TRUE; /* At end of the current routine */
       }
     }
     if ((! route_end) && (trace_exec))
     {
        /* Print out information about the line translated */
        /* Form is [line#][iflev][do_block had_true had_else][tok1][tok2]...*/

         fprintf(stdout,"-%d-%d-%d%d%d-",senser[senser_at].lno,if_level,
               ifs[if_level].do_block,ifs[if_level].had_true,
                        ifs[if_level].had_else);

         for (cnt = 0; cnt < ntokens  ; ++cnt)
         {
            fprintf(stdout,"[%s]",tokens[cnt].ptr);
         }
         fprintf(stdout,"\n");
     }
     if (route_end)
     {
       --senser_at; /* Drop back on list of routines */
       if (senser_at >= 0)
       {
         /* Not at the bottom of routines, will jump back one level */
         rc = fseek(senser[senser_at].fptr,senser[senser_at].s_at,0);
         route_end = FALSE; /* Not at the end of this routine now */
         if (rc != 0)
         {
           fprintf(stdout,"dsense : Error seeking to %lx (line %d) of %s\n",
            senser[senser_at].s_at,senser[senser_at].lno+1,
            sen_files[sroutes[senser[senser_at].rindex].sindex].name);
          exit (1);
         }
       }
       else
       {
         end_srch  = TRUE;  /* At the end of this search */
         ret_sense = NULL;  /* End of all sense levels */
       }
     }
     else
     {
       end_srch = TRUE; /* Have a record of the routine */
       /* Record index for next rec*/
       senser[senser_at].s_at = ftell(senser[senser_at].fptr);
       ++senser[senser_at].lno ; /* Increase the line count */
     }
   }
   return(ret_sense);
}
parse_tokens(p_str)
/* routine::parse_tokens                                                */
/* Parses tokens of a sense record and stores them in a set of tokens   */
/* Two strings are maintained
   sense_rec contains the original string
   token_str contains an expanded string with each token separated by a
             null
   tokens[].org_at points to each token in the original string
   tokens[].ptr    points to each token in the expanded string
*/
/*   Tokens are identified in this manner
     a) Any item separated by a blank tab or comma is a separate token
     b) Any + - % /  * ( ) ~ ^ $ or @ is a separate token
     c) When an = is encountered it is a separate token save that 1 more
        = can follow it in the token
     d) When a ! is encounted it is a separate token save that a single
        instace of a = ^  or @ can follow it
     e) > is separate but can be followed by > or =
        < is separate but can be followed by < or =
        : is separate but can be followed by =
        &, | %, < and > can be followed by 1 more of the same character

*/
   char *p_str;
{
   char *p_strat;
   int do_parse=TRUE,ntchars=0,in_tok=FALSE,rec_char=FALSE, close_tok=FALSE;
   int tindex=0,onect=FALSE;

   p_strat = p_str; /* Index start of string */
   ntokens = 0;  /* No tokens thus far */
   tokens[ntokens].ptr    = token_str;  /* Index first token */
   tokens[ntokens].org_at = sense_rec;

   while (do_parse != 0)
   {
      switch (*p_strat)
      {
         case ' '  :   case '\t' :    case '\n' :    case ','  :
         case 0:
         {
           /* White space */
           onect = FALSE;  /* No longer in this case */
           if (in_tok)
           {
             close_tok=TRUE;  /* Will close the current token */
           }
           if ( *p_strat == 0)
           {
             close_tok=TRUE;   /* Close the last token */
             do_parse = FALSE; /* Also end the parsing */
           }
           break;
         }
         case '-':
         {
           /* If we are processing the routine label then allow '-' as part of the
              label.  Otherwise treat the '-' as a token.
           */
           if ((ntokens == 1) && (tokens[0].org_at[0] == '!'))
           {
             rec_char = TRUE;
             onect = FALSE;
             in_tok = TRUE;
             close_tok = FALSE;
             break;
           }
           /* Else treat '-' as token and FALL THROUGH to the next case */
         }
         case '(' :    case ')' :     case '+' :     case '$' :  case '~' :
         case '/' :     case '!' :     case '*' :  case ':' :
         {
           if (in_tok)
           {
              close_tok=TRUE; /* Close the current token */
           }
           rec_char = TRUE;   /* Add character to a token, creating
                                 a new one if necessary */
           /* Note, ++ and -- will be seen as two tokens */
           onect = TRUE;  /* Token allowed to be only 1 character long*/
           break ;
         }
         case '=' :
         {
           rec_char  = TRUE;  /* Will record character */
           if (in_tok)
           {
             close_tok = TRUE; /* Presume to close current token */
             if ( (ntchars == 1) &&
                  ( ( *tokens[ntokens].ptr == '=') ||
                    ( *tokens[ntokens].ptr == '!') ||
                    ( *tokens[ntokens].ptr == '<') ||
                    ( *tokens[ntokens].ptr == '>') ||
                    ( *tokens[ntokens].ptr == ':')
                  )
                )
             {
               close_tok = FALSE; /* Special char in front of =, dnt cls*/
             }
           }
           onect = TRUE;  /* Token allowed to be only 1 character long*/
           break;
         }
         case '<' :  case '>' :  case '&' :  case '|' :  case '%' :
         {
           rec_char = TRUE;  /* Will record character */
           if (in_tok)
           {
             close_tok = TRUE; /* Presume to close current token */
             if ( (ntchars == 1) && ( *tokens[ntokens].ptr == *p_strat) )
             {
               close_tok = FALSE; /* Case of <<, >>, && or || */
             }
           }
           onect = TRUE;  /* Token allowed to be only 1 character long*/
           break;
         }
         case '^' :
         {
           rec_char  = TRUE;  /* Will record character */
           if (in_tok)
           {
             close_tok = TRUE; /* Presume to close current token */
             if ( (ntchars == 1) && (  *tokens[ntokens].ptr == '!'))
             {
               close_tok = FALSE; /* Special char in front of ^, dnt cls*/
             }
           }
           onect = TRUE;  /* Token allowed to be only 1 character long*/
           break;
         }
         case '@' :
         {
           rec_char  = TRUE;  /* Will record character */
           if (in_tok)
           {
             close_tok = TRUE; /* Presume to close current token */
             if ( (ntchars == 1) && (  *tokens[ntokens].ptr == '!'))
             {
               close_tok = FALSE; /* Special char in front of @, dnt cls*/
             }
           }
           onect = TRUE;  /* Token allowed to be only 1 character long*/
           break;
         }
         default:
         {
           if (onect)
           {
             if (in_tok)
             {
             /* Need to close previous token as it could only be 1
                character long, a special symbol typically */
               close_tok = TRUE;
             }
             onect = FALSE;  /* No longer in this case */
           }
           rec_char = TRUE;  /* Record the character, starting a new
                                     token if necessary */
         }
      }
      if (close_tok)
      {
         /* Close the last token out */
         in_tok = FALSE;        /* No longer in the token  */
         token_str[tindex] = 0; /* 0  terminate the token */
         ++tindex;              /* Index over to the next char */
         close_tok = FALSE;     /* No longer closing the token */

         /* Set up indexes for next token if any */
         if (*p_strat != 0)
         {
           if (ntokens < (MAXTOKS-1))
           {
             ++ntokens;
           }
           else
           {
             fprintf(stdout,"dsense: Line in sense file too complex\n");
             prt_recerr();
             exit(1);
           }
           tokens[ntokens].ptr = &token_str[tindex]; /* Record location */
           ntchars = 0;  /* No characters in this token */
         }
      }
      if (rec_char)
      {
         if (! in_tok)
         {
           /* Start of new token, record location in original string */
           tokens[ntokens].org_at = p_strat;
           in_tok = TRUE;  /* Ensure that in a token now */
         }
         /* Add the character */
         token_str[tindex] = *p_strat;
         ++tindex;
         ++ntchars;
         rec_char = FALSE;  /* Reset flag to record the character */
      }
      ++p_strat; /* Go to next character */
   }
}
cond_proc(tstart)
/* routine::cond_proc                                                    */
/* Processes a conditional statement and takes action as required        */
/* Note that even though a conditional statement might not be executable */
/* because the parent block is not executed, the conditional statements  */
/* Are still looked at to the extent of keeping track of the right if lev*/
/* tstart is the index into the first arg of tokens whether the cond stat */
/* starts                                                                */
   int tstart;
{
   int cond_val,tokens_left;

   tokens_left = ntokens - tstart;

   if (strcmp(tokens[tstart].ptr,"if") == 0)
   {
      /* If statement found.  Move to next level */
      ++if_level; /* Next level if block */
      if (if_level >= MAXIFLEVS)
      {
         fprintf(stdout,
          "dsense: Too many levels of if statements. Max is %d",MAXIFLEVS);
         prt_recerr();
         exit(1);
      }
      /* Presume not to process anything at this level */
      ifs[if_level].do_block = FALSE;

      /* Look at the if statement if the parent block can be processed*/
      if (ifs[if_level - 1].do_block)
      {
        /* Process the if statement */
        if (tokens_left < 2)
        {
          fprintf(stdout,"dsense: ~if statement with no arguments\n");
          prt_recerr();
          exit(1);
        }
        cond_val = parse_expr(tstart + 1);
        ifs[if_level].do_block = cond_val;  /* See if to do this block */
        ifs[if_level].had_true = cond_val;  /* If so did a block */
        ifs[if_level].had_else = FALSE;    /* No else statement found */
      }
   }
   else if (if_level == 0)
   {
        fprintf(stdout,
        "dsense: conditional expression with no matching if\n");
        prt_recerr();
        exit(1);
   }
   else if (strcmp(tokens[tstart].ptr,"else") == 0)
   {
      /* Ensure good else statement */
      if (tokens_left > 1)
      {
        fprintf(stdout,"dsense: Extraneous characters after ~else\n");
        prt_recerr();
        exit(1);
      }
      /* Ensure only 1 else at this level */
      if (ifs[if_level].had_else)
      {
        fprintf(stdout,"dsense: Two ~elses on the same if level (%d)\n",
                        if_level);
        prt_recerr();
        exit(1);
      }
      ifs[if_level].had_else = TRUE;

      if (ifs[if_level-1].do_block)
      {
        /* Okay to process the block */
        if (! ifs[if_level].had_true)
        {
          /* No previous conditions statisfied. Satisfied now */
          ifs[if_level].do_block = TRUE;
          ifs[if_level].had_true = TRUE;
        }
        else
        {
          ifs[if_level].do_block = FALSE; /* Don't do this block now */
        }
      }
   }
   else if (strcmp(tokens[tstart].ptr,"elseif") == 0)
   {
      /* Elseif statement */
      if (tokens_left < 2)
      {
        fprintf(stdout,"dsense: ~elseif statement with no arguments\n");
        prt_recerr();
        exit(1);
      }

      /* Execute the statement if block at previous level allows exec. */
      if (ifs[if_level -1].do_block)
      {
         if (! ifs[if_level].had_true)
         {
           /* No previous condition statified */
           cond_val = parse_expr(tstart + 1);
           ifs[if_level].do_block = cond_val;
           ifs[if_level].had_true = cond_val;
         }
         else
         {
           ifs[if_level].do_block = FALSE; /* Don't do this block now */
         }
      }
   }
   else if (strcmp(tokens[tstart].ptr,"endif") == 0)
   {
      /* Reduce level of if statement */
      ifs[if_level].do_block = FALSE;
      ifs[if_level].had_true = FALSE;
      ifs[if_level].had_else = FALSE;
      --if_level;
   }
   else
   {
      fprintf(stdout,"dsense : Unrecognized conditional statement\n");
      prt_recerr();
      exit(1);
   }
}
 dec_rest(token_start,value,val_true)
/* routine::dec_rest                                                      */
/* Decodes line syntax containing one or more of the following elements   */
/* | description     Decodes description information for display
   % description
   %% description
   || description    Decodes description information for display and
                     for inclusion as the problem summary
                     Decode sets des_start and des_stop

   ^mval             Sets mval variable to 1 if val_true or 0 otherwise
   $mval             If val_true sets mval to 1, if not val_true does not
                     touch mval if defined otherwise defines and sets to 0
   @mval             Sets mval to value if val_true
   ~print parameters Sets print parm values to the values of the parms
                     passed if val_true
*/
   int token_start,value,val_true;
{
   char *p_at;
   int token_at, mvar_index,end_line=FALSE;
   int prtparm_at=0;

   token_at = token_start ;  /* Start with first token */
   des_start = -1;           /* Presume no print tokens */
   des_stop  = -1;

   while (! end_line)
   {
     /* Look at the tokens and evaluate what we have */
     p_at = tokens[token_at].ptr;

     if (((*p_at == '|') || (*p_at == '%')) && (des_start < 0))
     {
        /* A symbol starting a description */
        des_start = token_at;
     }
     else if ((*p_at == '^') || (*p_at == '$') || (*p_at == '@') ||
              (*p_at == '~'))
     {
        if ((des_start >= 0) && (des_stop < 0))
        {
          /* End the description at this point */
          des_stop = token_at; /* Where the desc tokens stop */
        }
        if (*p_at != '~')
        {
          /* Have an mvar to deal with */
          ++token_at;  /* Advance to the mvar */
          p_at = tokens[token_at].ptr ;
          mvar_index = mvar_find(p_at,TRUE,TRUE);  /* Find mvar */
          /* Set value as required */
          if (val_true)
          {
             if (*tokens[token_at-1].ptr == '@')
                               {mvars[mvar_index].val = value;}
                          else {mvars[mvar_index].val = 1;    }
          }
          else if (*p_at == '^')
          {
             mvars[mvar_index].val = 0; /* Reset value */
          }
          if( trace_exec)
          {
            fprintf(stdout,"[--> %s set to %ld <--]\n",
             mvars[mvar_index].name,mvars[mvar_index].val);
          }
        }
     }
     else if (((des_start >= 0) && (des_stop >= des_start)) && (val_true))
     {
        /* A print parameter worth decoding */
        if (prtparm_at < MAXPRTINTS )
        {
           /* Get parameter and store */
           prtints[prtparm_at] = get_ivalue(tokens[token_at].ptr);
           ++prtparm_at;
        }
        else
        {
          fprintf(stdout,"dsense: Print parms in one line > %d\n",
                          MAXPRTINTS);
          prt_recerr();
          exit(1);
        }

     }
     /* Note any other case requires no action */
     ++token_at;
     if (token_at >= ntokens)
     {
       end_line = TRUE; /* End of decoding */
     }
   }
}
 do_descript()
/* routine::do_descript                                                   */
/* Handles decode and display of an output string                         */
/* defined by globals des_start for starting token and des_stop for ending
   Output string must begin with
   | or %   saying simply to print string   or
   || or %% sayting to print string and record as a summary record
*/
/* Note, a carriage return is added to the print statement if needed      */
/* unless the last character of the description is a \                    */
/* A blank line will be printed if out string is % but not if |           */
{
   char out_str[REC_MAX+1],out_xlated[REC_MAX+1],*p_at,*p_check;
   int des_len, npreamble, have_cr=FALSE;

   if (des_start >= 0)
   {
     /* Have a valid string */

     /* Find size of starting % %% | || to be excluded from printing */
     npreamble = strlen(tokens[des_start].ptr);

     /* Compute length of desc string  */
     des_len = strlen(tokens[des_start].org_at) - npreamble;
     if (des_stop >= 0)
     {
       des_len = des_len - strlen(tokens[des_stop].org_at);
     }
     /* Copy string over */
     strncpy(out_str,tokens[des_start].org_at + npreamble,des_len);

     out_str[des_len] = 0;  /* Null the string */

     /* Make a check for the carriage return suppresion symbol at line end*/
     p_check = out_str + des_len - 1;
     while ((*p_check == '\n') || (*p_check == '\t') || (*p_check == ' '))
     {
       if (*p_check == '\n') {have_cr = TRUE;}
       --p_check;
     }
     if (*p_check == '\\')
     {
       /* Surpress the \ and any carriage return */
       *p_check = 0;
     }
     else if (! have_cr )
     {
        out_str[des_len] = '\n'; /* Add carriage return */
        out_str[des_len+1] = 0;  /* Null the string */
     }

     if ((strlen(out_str) > 1) || (*tokens[des_start].ptr == '%'))
     {
       /* Translate and put out the string */
       sprintf(out_xlated,out_str,prtints[0],prtints[1],prtints[2],prtints[3],
              prtints[4],prtints[5],prtints[6],prtints[7],prtints[8],
             prtints[9]);
       if (! table_fmt)
       {
          fputs(out_xlated,stdout);
       }
       if (npreamble > 1)
       {
         /* String is a summary string to be stored */
         p_at = out_xlated; /* Eliminate leading blanks */
         while ((*p_at == ' ') || (*p_at == '\t')) { ++p_at;}
         if ((*p_at != '\n') && (*p_at != 0))
         {
            strcpy(err_descript,p_at);
         }
       }
     }
   }
}
do_pragmas(tstart)
/* routine::do_pragmas                                                   */
/* Handles all dsense command pragmas                                    */
/* tstart passed is the first parameter in the tokens array of parm ptrs */
/* Having pragma data                                                    */
   int tstart;
{
   int mvar_index,nints,tokens_left;
   long int intval;
   tokens_left = ntokens - tstart;  /* Determine parameters left */

   if (strcmp(tokens[tstart].ptr,"#trace") == 0)
   {
     trace_exec = TRUE;
     fprintf(stdout,"[--> Trace Started format is \n\
-line_number-if_level-do_blockhad_truehad_else-[tok1][tok2]..[tokn]\n");
   }
   else if (strcmp(tokens[tstart].ptr,"#notrace") ==0){ trace_exec = FALSE;}
   else if (strcmp(tokens[tstart].ptr,"#include") == 0)
   {
      if (tokens_left > 0)
      {
        index_senser(tokens[tstart+1].ptr);    /* Open file     */
      }
      else
      {
        fprintf(stdout,
           "dsense : No name of file to include after #include\n");
         prt_recerr();
         exit(1);
      }
   }
   else if (strcmp(tokens[tstart].ptr,"#define") == 0)
   {
      /* Defines a mvar if not already defined.  Sets initial value=0 */
      if (tokens_left > 0)
      {
         (void) mvar_find(tokens[tstart+1].ptr,TRUE,TRUE);
      }
      else
      {
        fprintf(stdout,
         "dsense : No name of variable to define after #define\n");
         prt_recerr();
         exit(1);
      }
   }
   else if (strcmp(tokens[tstart].ptr,"#set") == 0)
   {
      /* Sets a value to a mvar.  If not already defined, will define */
      if (tokens_left > 0)
      {
         mvar_index = mvar_find(tokens[tstart+1].ptr,TRUE,TRUE);
      }
      else
      {
        fprintf(stdout,"dsense : No name of variable to set after #set\n");
         prt_recerr();
         exit(1);
      }
      if (tokens_left > 1)
      {
        intval = strtol(tokens[tstart+2].ptr,&p_stopstr,0);
        if (tokens[tstart+2].ptr == p_stopstr)
        {
           fprintf(stdout,
            "dsense : Invalid value %s for variable %s \n",
            tokens[tstart+2].ptr,tokens[tstart+1].ptr);
           prt_recerr();
           exit(1);
        }
        else
        {
           mvars[mvar_index].val = intval;
        }
      }
      else
      {
        fprintf(stdout,
         "dsense : No name of variable to set after #set\n");
         prt_recerr();
         exit(1);
      }
      if ( trace_exec)
      {
        fprintf(stdout,"[--> %s set to %lx <--]\n",mvars[mvar_index].name,
                                                   mvars[mvar_index].val);
      }
   }
   else
   {
       fprintf(stdout,"dsense : Invalid pragma encountered\n\
       Valid pragmas include\n\
         #define variable_name          \n\
         #set    variable_name value    \n\
         #trace                         \n\
         #notrace                       \n\
         #include sense_routine        \n");
       prt_recerr();
       exit(1);
   }

}
expr_do(tstart)
/* routine::expr_do                                                      */
/* Handles the set expression pragma which has the format                */
/* mvar = expression starting with token indexed by tstart               */
/* Where expression is defined in parse_expr                             */
   int tstart;
{
   int mvar_index,tokens_left,dokay=TRUE;

   tokens_left = ntokens - tstart;

   if (tokens_left < 3)
   {
      dokay = FALSE;
   }
   else
   {
      /* Determine what variable to set */
      mvar_index = mvar_find(tokens[tstart].ptr,TRUE,TRUE);

     if (strcmp(tokens[tstart+1].ptr,":=") != 0)
     {
        dokay = FALSE;
     }
   }

   if (! dokay)
   {
      fprintf(stdout,"dsense : Illegal syntax for an expr command\n\
      command must be of the form\n\
       variable := some_numeric_expression \n");
      prt_recerr();
      exit(1);
   }

   /* Process the rest of the tokens for the expression */
   mvars[mvar_index].val = parse_expr(tstart+2);
   if (trace_exec)
   {
     fprintf(stdout,"[--> %s set to %lx <--]\n",mvars[mvar_index].name,
                                                mvars[mvar_index].val);
   }
}
int dec_rcrec(tstart)
/*routine::dec_rcrec                                                     */
/* Decodes fields of a record in one of the following formats
   row col symbol xxxx yyyy | Description ^mvar $var or @mvar ~ parms
   or
   var    symbol xxxx yyyy | Description ^mvar $mvar or @mvar ~ parms
   or

   And sets
     dtype       = symbol

     row_col[0] = row
     row_col[1] = col
      or
     cndvar=TRUE and cndval=value of mvar

     cmpval[0..15] = Digits to compare
     if ^mvar sets/defines mvar if compare is true, otherwise does not
              touch mvar
     if $mvar sets/defines mvar if compare is true, otherwise sets mvar
              false
     if @mvar sets the mvar to the value of the row,col if compare true
              does not touch mvar
     parms are a set of mvars to be passed as print parms to the
     description.  If no parms are passed default is value at row,col
           followed by compare value, followed by size value if any.
     descript  = Description if present, null otherwise
   If the fields can all properly be decoded the routine returns true.
   That the values found are in a valid range is not checked, however.

   tstart indexes the first entry in tokens for the decode
*/
   int tstart;
{
   char one_num[REC_MAX+1];
   int valid=TRUE,done_dec=FALSE,blen,oldclen,token_at;
   int have_dtype=FALSE;


   ncmptoks    =  0;     /*No compare integers decoded thus far*/
   ncmpchars   =  0;     /*No compare characters found */
   rest_token  = -1;     /*Tokens after the compare part not yet found */
   cmpval[0]   =  0;     /*No bits for compare value */
   cndvar      =  FALSE; /*Presume a row,col not a mvar to decode */

   token_at = tstart;

   if ((ntokens - tstart) < 2)
   {
     valid = FALSE;
   }
   else
   {
      if (isdigit(*tokens[token_at].ptr))
      {
         /* Expect row, column */
         row_col[0] = atol(tokens[token_at].ptr);
         ++token_at; /* Past row */
         if (isdigit(*tokens[token_at].ptr))
         {
            row_col[1] = atol(tokens[token_at].ptr);
            row_col[1] = row_col[1]*2;
            ++token_at; /* Past column */
         }
         else
         {
           valid = FALSE;
         }
      }
      else
      {
         /* Is an mval to be gotten */
         cndvar=TRUE;
         a_int = get_ivalue(tokens[token_at]);
         ++token_at;  /* Past this mval */
      }
   }

   /* Decode the rest of the string */
   while ( (token_at < ntokens) && (valid) && (! done_dec))
   {
      switch (*tokens[token_at].ptr)
      {

         case '=' :  case '<' :  case '>' :   case '!' :
         {
             /* A symbol found */
             strncpy(dtype,tokens[token_at].ptr,DTMAX);
             have_dtype = TRUE;
             break;
         }
         case '@' :  case '^' :
         {
             /* Either symbol or end of description found */
             if (! have_dtype)
             {
                strncpy(dtype,tokens[token_at].ptr,DTMAX);
                have_dtype = TRUE;
             }
             else
             {
               rest_token = token_at ;
               done_dec = TRUE;  /* Now done with decoding up to descript */
             }
             break;
         }
         case '|' :  case '$':  case '~':
         {
           rest_token = token_at ;
           done_dec = TRUE;  /* Now done with decoding up to descript */
           break;
         }
         default:
         {
           /* More bytes to obtain */
           strcpy(one_num,tokens[token_at].ptr);
           blen = strlen(one_num); /* Determine numb of chars in str*/
           blen = blen + (blen % 2); /*Ensure even number of digits*/
           oldclen = strlen(cmpval); /*Find length of cmp str to now*/

           if ((oldclen + blen) > 8)
           {
             valid = FALSE; /* Too many characters */
           }
           else
           {
              /* Concatentate compare digits to string */
              valid = get_bytes(one_num,&cmpval[oldclen],blen);
              cmpval[blen + oldclen] = 0; /* Null terminate string */
           }
           ++ncmptoks;
           break;
         }
      }
      ++token_at;
   }
   return(valid);
}
int tst_fields(start_okay)
/* routine::tst_fields                                                    */
/* Ensures that all of the fields decoded for a record are in a valid rang*/
/* Prints an error message to stdout if they are not.                     */
/* Returns TRUE if valid, false otherwise                                 */
/* If start_okay is true then assume it might be valid, otherwise it is   */
/* already detected to be an invalid record                               */
   int start_okay;
{
   int invalid=FALSE,is_vpos,max_vpos;

   /* Determine and check the size of the integer string */
   int_size = INTS_8;  /* Assume one byte only */
   if ((strcmp(dtype,"@") == 0) || (strcmp(dtype,"!@") == 0))
   {
      if (ncmptoks > 0)
      {
        switch (cmpval[0])
        {
          case '0' : {int_size = INTS_8 ; break;}
          case '1' : {int_size = INTS_16; break;}
          case '2' : {int_size = INTS_24; break;}
          case '3' : {int_size = INTS_32; break;}
          case '4' : {int_size = INTS_4 ; break;}
          default  :
          {
             invalid=TRUE;
             fprintf(stdout,
"dsense : Immediate print cmd found with size != 8,16,24, or 32\n\
               Your immediate print must be of the format\n\
               %d %d %s n   | description  flags ~ parms \n\
        Where n indicates the number of bits in the integer (8,16,24,32)\n",
              row_col[0],row_col[1]/2,dtype);
          }
        }
      }
   }
   else
   {
     /* Take size based on cmpval */
     switch (strlen(cmpval))
     {
        case 2: {int_size = INTS_8; break;}
        case 4: {int_size = INTS_16; break;}
        case 6: {int_size = INTS_24; break;}
        case 8: {int_size = INTS_32; break;}
        default :
        {
             fprintf(stdout,
"dsense : Too many digits in compare value of a compare statement\n\
             Your compare statement must be of the format\n\
     %d %d %s cmp_str | description flags ~ parms \n\
         Your compare string was %s\n\
         Where cmp_str must be some numeric string between 1 and 8 digits\n\
            8\n\
            a b\n\
            aa bb\n\
            0123 4567 \n\
         Are all examples\n\
         Spaces can be placed wherever desired save that if there are\n\
         an odd number of digits in a word it is padded to the left\n\
         1 becomes 01 for instance and 123 becomes 0123\n",
         row_col[0],row_col[1]/2,dtype,cmpval);
            invalid = TRUE;
        }
     }
   }

   if (start_okay == FALSE)
   {
     fprintf(stdout,"dsense : A record did not start properly\n\
     A record can start with one of the following : \n\
     *  For a comment \n\
     #  For a pragma  \n\
     %  To print a line of text \n\
     ~  For a conditional expression \n\
     |  To continue a comment from a previous compare expression \n\
     a number for a compare expression \n\
     The name of a variable for a compare or assignment expression \n");
     invalid = TRUE;
   }

   /* Check the row and column position */
   if (((row_col[0] < 0) || (row_col[1] < 0)) && (! invalid) &&
         ( ! cndvar))
   {
     invalid = TRUE;
            fprintf(stdout,
   "dsense : Row %d or column %d less than 0\n\
            Rows and columns are numbered starting with 0\n",row_col[0],
            row_col[1]/2);
   }

   is_vpos  = get_vpos(row_col[0],row_col[1]);
   max_vpos = get_vpos(row_at,next_col);

   if (((is_vpos + (int_size*2)) > max_vpos) && (! invalid))
   {

     invalid = TRUE;
     fprintf(stdout,"dsense : Row %d and col %d reading %d bytes\n\
     indexes out of the maximum row/column %d %d\n",
     row_col[0],row_col[1]/2,int_size,row_at,(next_col - 1)/2);
   }

   /* Ensure the compare symbol is valid */
   if  ((strcmp(dtype,"=" ) == 0) ||
        (strcmp(dtype,"==") == 0) ||
        (strcmp(dtype,"<" ) == 0) ||
        (strcmp(dtype,"<=" ) == 0)||
        (strcmp(dtype,">"  ) == 0)||
        (strcmp(dtype,">=" ) == 0)||
        (strcmp(dtype,"^"  ) == 0)||
        (strcmp(dtype,"!^" ) == 0)||
        (strcmp(dtype,"!=" ) == 0))
   {
      /* Must have at least one compare token */
      if ((ncmptoks < 1) && (! invalid))
      {
        invalid=TRUE;
        fprintf(stdout,
"dsense : Numeric comparison requested but no number(s) to compare given\n\
         Your compare needs to be of the format\n\
         %d %d %s ww xx yy zz \n\
         Where %d and %d are row and column of the value to be looked at\n\
               ww xx yy and zz are bytes to compare to\n\
               Comparison can be made on 1 to 4 bytes in a command",
         row_col[0],row_col[1],dtype);
      }
   }
   else if ( (strcmp(dtype,"@") != 0) && (strcmp(dtype,"!@") != 0)
           && (! invalid))
   {
       invalid = TRUE;
       fprintf(stdout,
      "dsense : Invalid compare symbol %s \n\
      Allowed symbols are\n\
         = !=  < <= > >=  for numeric comparions\n\
         ^ or !^ for bit set/not set comparisons\n\
        or @ or !@ which set results of a compare unconditionally\n",
         dtype);
   }
   if (invalid)
   {
     prt_recerr();
   }
   return(! invalid);
}
do_comp()
/* routine::do_comp                                                       */
/* Takes appropriate action of displaying data and setting display based  */
/* On the information decoded                                             */
{
   int do_display=FALSE,req_vpos;
   long int c_int;

   req_vpos = get_vpos(row_col[0],row_col[1]); /* Get index to sense bytes*/

   /* Get values as integers */
   if (! cndvar)
   {
     /* Have an integer value to decode from row,col */
     if (strcmp(dtype,"!@") == 0)
     {
        /* Get sign extended integer */
        a_int = get_hint(&invals[req_vpos],int_size,TRUE);
     }
     else
     {
        /* Get value as an integer without sign extending */
        a_int = get_hint(&invals[req_vpos],int_size,FALSE);
     }
   }
   c_int  = get_hint(cmpval,int_size,FALSE);     /* Compare integer */

   /* Do desired function computation */
   if ((strcmp(dtype,"@" ) == 0) || (strcmp(dtype,"!@") == 0))
   {
       /* Will simply do the display of data */
       do_display = TRUE;
   }
   else
   {
      if      (strcmp(dtype,"="  )== 0){ do_display = (a_int == c_int);  }
      else if (strcmp(dtype,"==" )== 0){ do_display = (a_int == c_int);  }
      else if (strcmp(dtype,"!=" )== 0){ do_display = (a_int != c_int);  }
      else if (strcmp(dtype,"<"  )== 0){ do_display = (a_int <  c_int);  }
      else if (strcmp(dtype,"<=" )== 0){ do_display = (a_int <= c_int);  }
      else if (strcmp(dtype,">"  )== 0){ do_display = (a_int >  c_int);  }
      else if (strcmp(dtype,">=" )== 0){ do_display = (a_int >= c_int);  }
      else if (strcmp(dtype,"^"  )== 0)
           {
              do_display = bit_chk(a_int,c_int,TRUE);
           }
      else if (strcmp(dtype,"!^"  )== 0)
           {
              do_display = bit_chk(a_int,c_int,FALSE);
           }
   }

   if ((rest_token >= 0)  && ( ntokens > (rest_token + 1)))
   {
      /* Have information beyond the compare values, will decode and
         execute print if desired */
      prtints[0]=a_int;
      prtints[1]=c_int;
      dec_rest(rest_token,a_int,do_display);  /* Decode rest of line */
      if (do_display)
      {
         do_descript();             /* Display line */
      }
   }
   last_ctrue = do_display ;  /* Store results of the conditional */
}
long int parse_expr(tstart)
/* long integer routine::parse_expr                                      */
/* Takes an expression of the format                                     */
/* var1 op var2 op (var3 op var4) op ((! var5) op (var6 & var7))         */
/* evaluates the resulting expression.                                   */
/* Evaluation proceeds from left to right                                */
/* No operation takes precedence over any other.                         */
/* There is no explicit negation operation. -1 must be expressed as      */
/*      0 - 1                                                            */
/* Since there is no precedence order involved () must be used as needed */
/* tstart indexes the starting value in tokens of the parms that make the */
/* expression                                                            */
   int tstart;
{

   int dec_okay=TRUE,token_at;
   long int num_val;
   nest_at=0;
   expr[nest_at].have_lval  = FALSE; /* No left value found at this levl*/
   expr[nest_at].funct[0]   = 0;

   token_at = tstart;

   while ((token_at < ntokens) && (dec_okay))
   {
      /* Presume to have a an operator or ( */
      switch(*tokens[token_at].ptr)
      {
         case '(':
         {
            dec_okay = do_addlev();
            break;
         }
         case '!':
         {
            if (strcmp(tokens[token_at].ptr,"!=") == 0)
            {
              /* Handle as a not equals */
              dec_okay = do_oper(tokens[token_at].ptr);
            }
            else
            {
              dec_okay = do_not(); /* Handle as a not comamnd */
            }
            break;
         }
         case '+':  case '-':  case '*':  case '/':    case '%': case '~':
         case '=':  case '>':  case '<':  case '&':    case '|':
         {
            dec_okay = do_oper(tokens[token_at].ptr);
            break;
         }
         case ')':
         {
            dec_okay = do_reducelev();
            break;
         }
         default:
         {
            /* Presume to have an integer value or mvar name */
            num_val = get_ivalue(tokens[token_at].ptr); /* Get num value*/
            dec_okay = do_intval(num_val); /* Handle the numeric value */
         }
      }
      ++token_at;  /* Do next token */
   }
   if (dec_okay)
   {
     if (nest_at > 0)
     {
        dec_okay=FALSE;
        fprintf(stdout,"dsense: if expression ended with %d open (\n",
                nest_at);
     }
     else if (expr[nest_at].have_lval == FALSE)
     {
        dec_okay=FALSE;
        fprintf(stdout,"dsense: Assignment or condition expr is empty\n");
     }
   }

   if (! dec_okay)
   {
     prt_recerr();
     exit(1);
   }
   return(expr[nest_at].value);  /* Return with decoded value */
}
int do_addlev()
/* integer routine::do_addlev                                         */
/* adds a parenthesis level to expression being parsed                */
{
   int dokay=TRUE;

   ++nest_at; /* At next paren nesting */
   if (nest_at >= MAXEXPR)
   {
     fprintf(stdout,"dsense: Maximum parentheses levels exceeded in an\n\
     if expression.  Maximum is %d\n",MAXEXPR);
     dokay = FALSE;
   }
   else
   {
     expr[nest_at].have_lval = FALSE;   /* No value for level */
     expr[nest_at].funct[0] = 0;        /* No symbol for level */
   }
   return(dokay);
}
int do_oper(p_at)
/* integer routine::do_oper                                           */
/* Handles the encounter with an expression operator                  */
/* p_at points to the start of the operator.                          */
   char *p_at;
{
   int dokay=TRUE;

   if (! expr[nest_at].have_lval)
   {
     fprintf(stdout,"dsense: Operator %c encountered before any \n\
     left argument was found\n\
        a %c b is valid but\n\
        %c b    is not\n", *p_at,*p_at,*p_at);
     dokay = FALSE;
   }
   /* Copy over the operator to associate on the current level */
   strcpy(expr[nest_at].funct,p_at);
   return(dokay);
}
int do_not()
/* integer routine::do_not                                            */
/* Handles the encounter with a not logical symbol                    */
{
   int dokay=TRUE;

   if ((expr[nest_at].funct[0] != 0) || (expr[nest_at].have_lval))
   {
     fprintf(stdout,"dsense: illegal use of not function \n\
     The ! can only be used on a parenthesis level on its own\n\
        a & (! b)  for example instead of\n\
        a & ! b\n");
     dokay = FALSE;
   }
   strcpy(expr[nest_at].funct,"!");
   return(dokay);
}
int do_reducelev()
/* integer routine::do_reducelev                                      */
/* Reduces one level of parenthesis                                   */
{
   int dokay=TRUE;

   if (nest_at <= 0)
   {
     fprintf(stdout,"dsense: Two many right parenthesis found\n");
     dokay = FALSE;
   }
   else
   {
      if (expr[nest_at].funct[0] != 0)
      {
        fprintf(stdout,
        "dsense:Parenthesis level %d ended with an operator.\n\
        (a & b)  is valid for example, but\n\
        (a &  )  is not\n",nest_at);
        dokay=FALSE;
      }
      else
      {
        if (! expr[nest_at].have_lval)
        {
          fprintf(stdout,"dsense:Parenthesis level %d ended, but no value\n\
          resulted from the expression\n\
          (a & b)  is valid for example, but\n\
          ( )      is not\n");
          dokay=FALSE;
        }
        else
        {
          /* Reduce one level and process value from previous level as
             an argument to the level reduced to */
          --nest_at;
          dokay = do_intval(expr[nest_at+1].value);
          expr[nest_at+1].have_lval = FALSE; /* Reset at old level*/
          expr[nest_at+1].funct[0]= 0; /* Reset at old level */
        }
      }
   }
   return(dokay);
}
int do_intval(val_is)
/* integer routine::do_intval                                         */
/* Takes the value found in val_is and processes as required as a     */
/* left or right hand argument in an expression                       */
   long int val_is;
{
   int dokay=TRUE;
   long int *p_ival;

   if (! expr[nest_at].have_lval)
   {
     /* No left value encountered, thus far, value will be lvalue */
     if (expr[nest_at].funct[0] == 0)
     {
       /* Store expression */
           expr[nest_at].value = val_is;
       expr[nest_at].have_lval = TRUE;
     }
     else if (strcmp(expr[nest_at].funct,"!") == 0)
     {
       /* Store negated expression */
       expr[nest_at].value = (! val_is);
       expr[nest_at].have_lval = TRUE;
       expr[nest_at].funct[0] = 0;  /* Last operation completed */
     }
     else
     {
       fprintf(stdout,"dsense:Operator encountered with no left \n\
       value.\n\
       (a %s b) is acceptable but\n\
       (%s b)   is not\n",expr[nest_at].funct,expr[nest_at].funct);
       dokay=FALSE;
     }
   }
   else
   {
       p_ival = &expr[nest_at].value;
     /* Already have left value, value passed should be right value */
     if (strcmp(expr[nest_at].funct,"!") == 0)
     {
        fprintf(stdout,"dsense: illegal use of not function \n\
        The ! can only be used on a parenthesis level on its own\n\
        a & (! b)  for example instead of\n\
        a & ! b\n");
        dokay=FALSE;
     }
     else if (expr[nest_at].funct[0] == 0)
     {
        fprintf(stdout,"dsense: illegal syntax in an if function\n\
        a | b is okay but not\n\
        a  b\n");
       dokay=FALSE;
     }

     /* Bitwise logical functions */
     else if (strcmp(expr[nest_at].funct,"&") == 0)
             {*p_ival = *p_ival & val_is;          }
     else if (strcmp(expr[nest_at].funct,"|") == 0)
             {*p_ival = *p_ival | val_is;          }

     /* Valuewise logical functions */
     else if (strcmp(expr[nest_at].funct,"&&") == 0)
             {*p_ival = *p_ival && val_is;          }
     else if (strcmp(expr[nest_at].funct,"||") == 0)
             {*p_ival = *p_ival || val_is;          }

     /* Bitwise shifting functions */
     else if (strcmp(expr[nest_at].funct,">>") == 0)
             {*p_ival = (*p_ival >>  val_is);      }
     else if (strcmp(expr[nest_at].funct,"<<") == 0)
             {*p_ival = (*p_ival <<  val_is);      }

     /* Logical Compares */
     else if (strcmp(expr[nest_at].funct,"==") == 0)
             {*p_ival = (*p_ival ==  val_is);      }
     else if (strcmp(expr[nest_at].funct,"=" ) == 0)
             {*p_ival = (*p_ival ==  val_is);      }
     else if (strcmp(expr[nest_at].funct,"!=") == 0)
             {*p_ival = (*p_ival !=  val_is);      }
     else if (strcmp(expr[nest_at].funct,">") == 0)
             {*p_ival = (*p_ival >   val_is);      }
     else if (strcmp(expr[nest_at].funct,">=") == 0)
             {*p_ival = (*p_ival >=  val_is);      }
     else if (strcmp(expr[nest_at].funct,"<") == 0)
             {*p_ival = (*p_ival <   val_is);      }
     else if (strcmp(expr[nest_at].funct,"<=") == 0)
             {*p_ival = (*p_ival <=   val_is);     }

     /* Numeric Functions */
     else if (strcmp(expr[nest_at].funct,"+") == 0)
             {*p_ival = *p_ival +  val_is;         }
     else if (strcmp(expr[nest_at].funct,"-") == 0)
             {*p_ival = *p_ival -  val_is;         }

     else if (strcmp(expr[nest_at].funct,"*") == 0)
             {*p_ival = *p_ival *  val_is;      }
     else if (strcmp(expr[nest_at].funct,"/") == 0)
     {
        if (val_is == 0)
        {
          fprintf(stdout,"dsense: Cant evaluate %ld / %ld\n",
                  expr[nest_at].value, val_is);
        }
        else
        {
          *p_ival = *p_ival / val_is;
        }
     }
     else if (strcmp(expr[nest_at].funct,"%") == 0)
     {
        if (val_is == 0)
        {
          fprintf(stdout,"dsense: Cant evaluate %ld % %ld\n",
                  expr[nest_at].value, val_is);
        }
        else
        {
          *p_ival = *p_ival % val_is;
        }
     }
      expr[nest_at].funct[0]=0;  /* Last operation completed */
   }
   return(dokay);
}
int get_vpos(row_is,col_is)
/* integer routine::get_vpos                                              */
/* From the row and column passed gets the position in the hex array of   */
/* the desired value.                                                     */
   int row_is,col_is;
{
   int vpos;

   vpos = row_is*MAXINDIGS + col_is ;
   return(vpos);
}
int mvar_find(p_mvar,add_new,err_exit)
/* integer routine::mvar_find                                             */
/* Attempts to find the passed mvar p_mvar in the structure of defined flgs*/
/* If the mvar is found will return with the index into the mvars structur*/
/* for the name.                                                          */
/* If the mvar is not found, and add_new is set will add the mvar to the  */
/* list and return the index.                                             */
/* Otherwise will a) exit with an error message if err_exit true or       */
/* exit with a -1 otherwise                                               */
   char *p_mvar;
{
   int mvar_no=(0 - 1),cnt,have_mvar=FALSE;
   char test_rec[REC_MAX+1];

   if ((*p_mvar != 0) && (*p_mvar != ' ') && (*p_mvar != '\t') &&
       (*p_mvar != '\n'))
   {
      strcpy(test_rec,p_mvar);
      strtok(test_rec,", \t\n");
      for (cnt = 0 ; (cnt < nmvars) && (! have_mvar) ; ++ cnt)
      {
        if (strcmp(test_rec,mvars[cnt].name) == 0)
        {
           have_mvar = TRUE;
           mvar_no = cnt;
        }
      }
      if (! have_mvar)
      {
         if (add_new)
         {
           if (nmvars >= FLAG_MAX)
           {
             fprintf(stdout,
             "dsense: Number of variables exceeds max of %d\n",
                    FLAG_MAX);
             prt_recerr();
             exit(1);
           }
           /* Add new mvar name */
           mvar_no = nmvars;
           strcpy(mvars[nmvars].name,test_rec); /*Store mvar name*/
           mvars[nmvars].name[MVARNM_MAX]=0;    /* Truncate to 32 chars*/
           mvars[nmvars].val = FALSE; /*Start out with mvar false */
           ++nmvars; /* Increase # of mvars found */
         }
         else if (err_exit)
         {
            fprintf(stdout,
             "dsense: Undefined variable %s encountered \n",p_mvar);
            prt_recerr();
            exit(1);
         }
      }
   }
   else
   {
     if (err_exit)
     {
       fprintf(stdout,"dsense: Bad variable name %s encountered\n",p_mvar);
       prt_recerr();
       exit(1);
     }
   }
   return(mvar_no);
}
long int get_ivalue(p_tok)
/* integer routine::get_ivalue                                        */
/* Evaluates the parameter passed as p_tok and takes it as either a   */
/* number or mvar.  Returns with the value of the number or mvar      */
   char *p_tok;
{
   int mvar_index,is_num;
   long int num_val;

   is_num = isdigit(*p_tok); /* See if a number or mvar name */

   if (is_num)
   {
     /* Decode value as a number */
     num_val = strtol(p_tok,&p_stopstr,0);
   }
   else
   {
      /* Find the value of the mvar related to the token */
      mvar_index = mvar_find(p_tok,FALSE,TRUE);
      num_val    = mvars[mvar_index].val;
   }
   return(num_val);
}
long int get_hint(p_chstr,nbytes,do_extend)
/* long integer routine::get_hint                                        */
/* Gets the integer value of a set of hex digits                         */
/* Where
         p_chstr is the set of hex bytes which must be 8 or less digits
         nbytes  is the number of bytes in p_chstr
         do_extend if true allows the upper most bit of the hex bytes
                   to sign extend to a full 32 bits
*/
   char *p_chstr;
   int nbytes,do_extend;
{
   int ndigits, cnt, neg_numb=FALSE, nxlated=0;
   long int ret_int;

   #define MAXLDIGS 8

   char xstr[MAXLDIGS + 1];
   char one_dig[2];


   ndigits = 2*nbytes;
   if (ndigits == 0) {ndigits = 1; } /* Special case for 1/2 byte */

   /* Need to see if number is negative */
   one_dig[0] = *p_chstr;
   one_dig[1] = 0;
   if ((one_dig[0] == 'x') || (one_dig[0] == 'X')) {one_dig[0] = 0;}

   if (((strtol(one_dig,&p_stopstr,16) > (long) 7)) && (do_extend))
   {
       /* Will consider this to be a negative number */
       neg_numb = TRUE;
   }

   /* Will sign extend numbers as needed */
   nxlated = MAXLDIGS - ndigits;  /* Define number of bytes to extend */
   for (cnt = 0 ; cnt < nxlated ; ++cnt)
   {
     if (neg_numb)
     {
        xstr[cnt] = 'f';
     }
     else
     {
        xstr[cnt] = '0';
     }
   }

   /* Will copy rest of the numbers comprising the string */
   for (cnt = 0 ; cnt < ndigits ; ++cnt)
   {
     xstr[nxlated] = *(p_chstr + cnt);
     if ((xstr[nxlated] == 'x') || (xstr[nxlated] == 'X'))
     {
        xstr[nxlated] = '0' ;  /* Make don't care values 0 */
     }
     ++nxlated; /* Increase count of bytes in the string being created */
   }
   xstr[MAXLDIGS] = 0; /* null terminate */

   /* Get the integer value of the number */
   ret_int = strtol(xstr,&p_stopstr,16);

   return(ret_int);
}
int get_bytes(p_in,p_out,nchars)
/* integer routine::get_bytes                                             */
/* Verifies and copies a nchars worth of characters from p_in to p_out    */
/* Adds leading zeros as needed.                                          */
/* Returns true if decode proceeded okay and False otherwise              */
   char *p_in,*p_out;
   int nchars;
{
   int nlead,cnt,dokay=TRUE;

   /* Determine number of leading zeros to add and add them */
   nlead = nchars - strlen(p_in);

   for (cnt=0; cnt < nlead; ++cnt)
   {
      *p_out = '0';   /* Add a leading zero */
      ++p_out;
   }

   /* Now do the rest of the copy */
   for (cnt = 0 ; (cnt < (nchars - nlead)) && (dokay) ; ++cnt)
   {
      if ( (! isxdigit(*p_in)) && (*p_in != 'x') && (*p_in != 'X'))
      {
        dokay = FALSE;
      }
      else
      {
        *p_out = *p_in;
        ++p_in;
        ++p_out;
      }
   }
   return(dokay);
}
 int bit_chk(act_int,cmp_int,positive)
/* Integer routine::bit_chk                                               */
/* Does bit-wise comparisons as follows:                                  */
/* act_int is an integer a compare is to be done on                       */
/* cmp_int is an integer representing a compare mask                      */
/* positive is a mvar indicating the polarity of the compare.             */
/*  if =TRUE,   for each bit that is 1 in cmp_int, the corresponding bit  */
/*    in act_int is checked for = 1.  Bits = 0 in cmp_int are ignored     */
/*  if =FALSE,  for each bit that is 0 in cmp_int, the corresponding bit  */
/*    in act_int is checked for = 0.  Bits = 1 in cmp_int are ignored     */
/* If all the compares agree, the routine returns TRUE                    */
/* Since all unused bits from both act_int and cmp_int are set to 0 a     */
/* bitwise logic truth table is used for the problem as follows:
   actual bit    compare bit      Positive   Not Positive
  --------------------------------------------------------
   0             0                1          1
   0             1                0          1
   1             0                1          1
   1             1                1          0
*/
     long int act_int;
     long int cmp_int;
     int positive;
{
   long int zval;

   int ret_value;

   zval = ~((long int) 0);  /* Create an all 1's variable */

   if (positive)
   {
      ret_value = (( ( act_int) | (~cmp_int)) == zval);
   }
   else
   {
      ret_value = (( (~act_int) | (~cmp_int)) == zval);
   }
   return(ret_value);
}
int str_ext(p_name,p_ext)
/* integer routine::str_ext                                               */
/* Adds extension to the given name if an extension is not already there  */
/* Returns the offset of p_name of the start of the ext                   */
  char *p_name,*p_ext;
{
  int nlen,no_dash=TRUE,have_ext=FALSE,ext_at;
  char *p_at;

  nlen = strlen(p_name);
  ext_at=nlen;            /* Assume no extension exists, to be added at
                             end of string */

  for (p_at = p_name + nlen - 1; (p_at >= p_name) && (no_dash)
                                 && (! have_ext)  ; --p_at)
  {
      if (*p_at == '/')
      {
          no_dash = FALSE;
      }
      else
      {
          if (*p_at == '.') { have_ext = TRUE ; } /* Have an extension */
          ext_at = p_at - p_name;
      }
  }
  if (! have_ext)
  {
     ext_at = strlen(p_name);
     strcat(p_name,p_ext);   /* No extension */
  }
  return(ext_at);
}
 wrt_colstr(p_str,collen,p_file)
/* routine::wrt_colstr                                                    */
/* Writes a column of string information p_str passed to file p_file      */
/* truncating as necessary to make the string written collen chars long   */
/* Will supress any \n in the string                                      */
   char *p_str;
   FILE *p_file;
   int collen;
{
   int cnt;

   for (cnt=0; cnt < collen; ++cnt)
   {
      if ((*p_str != 0) && (*p_str != '\n'))
      {
         fputc(*p_str,p_file);
         ++p_str;
      }
      else
      {
         fputc(' ',p_file);
      }
   }
}
 prt_recerr()
/* routine::prt_recerr                                                   */
/* Print debug information about the current record being processed      */
{
   fprintf(stdout,"        Line %d of file %s as shown on next line\n%s\n",
   senser[senser_at].lno,
   sen_files[sroutes[senser[senser_at].rindex].sindex].name,
                                   sense_rec);
}
 prt_info()
/* routine::prt_info                                                       */
/* Documents the function of the dsense program                            */
{
   fprintf(stdout,"\
                                dsense program\n\
                              IBM Internal Use Only\n\n");
   fprintf(stdout,"\
Examines error log data, expands sense data information and summarizes\n\
hardware errors in the error log.\n\
General syntax is \n\
 dsense [-c] [-t] [-s] [-mmain_f] errpt_info_file > inf_file 2> sum_file\n\
\n\
Where  \n\
       errpt_info_file    is a previously created file containing errpt -a\n\
                          information\n\
       inf_file           Will be written containing expanded err info\n\
       sum_file           Will be written categorizing and counting errs\n\
       -c                 if present gives a chronological summary of errs\n\
                          in addition to summary sorted by type\n\
       -t                 if present puts summary in a table format used\n\
                          for storing in a database. No expanded info\n\
       -s                 Records software errors as well as hardware\n\
       -mmain_f           Signifies that the main_f file contains the list\n\
                          Of files having routines to be used for decode\n\
\n\
If the program is to be used to look at errors on the machine being run\n\
   errpt -a | dsense  >expanded_err_info_file 2> err_summary_file\n\
Can be issued instead\n\
\n\
If a file exists containing only senses bytes in rows and columns\n\
exactly like that obtained from an errpt -a command, the following can be\n\
issued\n\
  dsense resource_type < sense_byte_file > sense_decoded_file\n\
\n\
    Where resource_type is the type of error the sense bytes are for\n\
    Examples may\n\
    8mm  150mb   876mb\n\
\n\
Note, in order to operate properly dsense uses a set of files that describe\n\
The contents of sense data for each resource type (rtypes.s dsense.lib etc\n\
are names of the files).  The environment variable\n\
                                SENSE_PATH\n\
needs to be set to the directory containing those files if it is not the\n\
current directory dsense is being run under.\n");
}

