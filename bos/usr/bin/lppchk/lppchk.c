static char sccsid[] = "@(#)39  1.10.1.2  src/bos/usr/bin/lppchk/lppchk.c, cmdswvpd, bos411, 9428A410j 3/31/94 20:43:55";
/*
 * COMPONENT_NAME: (CMDSWVPD) Software VPD
 *
 * FUNCTIONS: lppchk (main)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME: lppchk (main)
 *
 * FUNCTION: Process command line for swvpd and invoke lppchk function
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Processes the command line parameters as outlined in the syntax
 *      below.  Creates a standardized, valid form for those parameters
 *      and passes that form on to the lppchk function routine.
 *
 * RETURNS: 0 - no errors detected
 *          non-zero - syntax or processing errors found.
 *
 */

/*
 *  Syntax:
 *     lppchk  -f
 *             -c
 *             -l
 *             -v
 *             -u
 *             -mn
 *             -tn
 *             -O [ r s u ]
 *             [ lpp-spec ]
 *             [ file-spec-list ]
 *
 *    -f   => fast mode, check file size, do not check the checksum
 *    -c   => check file size and checksum
 *    -l   => check the specified files to establish any missing symlinks
 *    -v   => verify that all lpps in the object repositories specified
 *            are at corresponding levels. At least 2 repositories must be
 *            specified
 *         ONE and only ONE of f, c, l or v must be specified
 *    -u   => update the SWVPD with new data - (only valid with -f or -c)
 *               if size value is 0 in the VPD, set it to match the file
 *               If size value is not 0 in the VPD (possibly from previous step)
 *                  and checksum does not match, change the checksum to agree
 *                  with the file.
 *    -O   => specifies which repositories are to be processed.  If none are
 *            specified all three will be checked.  The parameter may be
 *            any combination of the following letters
 *                  r  ==> root repository /etc/objrepos
 *                  s  ==> shared repository /usr/share/lib/objrepos
 *                  u  ==> usr repository /usr/lib/objrepos
 *    lpp-spec => name of lpps to be checked. May be any ODM string search
 *                specification ( including ? or * if protected from shell
 *                processing). LPPs in the specified repositories that match
 *                the lpp-spec will be processed.
 *    file-spec-list => Optional list of file name specifications that  control
 *                the set of files to be checked.  If none are specified, all
 *                files for the LPPs specified will be checked, not valid with
 *                the -v flag.
 *    -m n  => n specifies a message level with the following meaning
 *                0  ==> show all informational, warning and error messages
 *                1  ==> show all warning and error messages only
 *                       (default)
 *                2  ==> show all error messages only
 *    -t n  => n specifies the trace level messages.
 *                0  ==> no trace messages (default)
 *                |
 *                9  ==> all trace messages
 *                       values between these will give selected trace messages
 *
 *
 */

#include        "lppchk.h"      /* standard includes needed by lppchk   */
#include        <locale.h>      /* local specific routines etc          */

char inu_cmdname[]="lppchk";    /* Used for calls to ckprereq routines  */
char NullString[]="";           /*   for lppchk -v and lppchk -uv.      */

main(argc, argv)
int             argc;
char            **argv;
  {
  int           c;              /* option character from cmd line       */

  int           opt_f, opt_c,
                opt_l, opt_v,
                opt_O, opt_u;   /* count occurances of option letters   */

  int           opt_Or, opt_Ou,
                opt_Os ;        /* count refs to repository codes       */

  char          opt_str[5] ;    /* string of valid option letters       */
  char          rep_str[5] ;    /* string of repository codes           */

  char          opt_let[2] ;    /* string for current option letter     */

  int           opt_m, opt_t ;  /* control output from this command     */

  char          *lppname ;      /* pointer to LPP name specification    */

  int           syntax_err ;    /* boolean tracks if error seen yet     */

  char          *file_spec ;    /* dummy arg list used when no file     */
                                /* names are specified in the command   */

  char          **file_first ;  /* pointer to address of first file     */
                                /* name specified, either to argv[x] or */
                                /* to file_spec                         */

  int           file_count;     /* number of file specs to be accessed  */
                                /* 1 when file_spec is used else rest   */
                                /* of argv[]                            */

  int           n ;             /* temp value for numeric parms         */
  char          *endptr ;       /* used by convert routine for num parms*/

  extern int optind ;           /* argc index for current parm          */
  extern char *optarg ;         /* pointer to string when option has    */
                                /* value string                         */

#define MSGMAX  3               /* maximum value for message level      */
#define MSG_DEF 3               /* default level for message output     */

#define TRCMAX  9               /* maximum allowed trace level          */

/* Begin main */

  setlocale(LC_ALL,"");         /* set up local language support        */

  lppchk_pn = argv[0] ;         /* get program name string              */

  init_vpd_strings();           /* get translated strings for VPD info  */

  /* figure out which flags were passed on the command line. */

  opt_f = opt_c = opt_l = opt_v = opt_u = opt_t = 0 ;
  opt_m = 0 ;
  opt_O = opt_Or = opt_Ou = opt_Os = 0 ;
                                /* init option flag counters            */

  opt_str[0] = '\0' ;           /* init valid option string             */
  rep_str[0] = '\0' ;           /* init repository string               */

  opt_m = 1 ;                   /* default message level                */

  syntax_err = FALSE ;          /* init - no error yet                  */

  while ((c = getopt (argc, argv, "fclvum:t:O:") ) != EOF)
    {
    opt_let[0] = (char) c ;     /* make the option letter a string      */
    opt_let[1] = '\0' ;

    switch (c)
      {
      case 'f':                 /* fast option - no checksum            */
        opt_f += 1 ;
        strncat(opt_str,opt_let,sizeof(opt_str)-strlen(opt_str)-1) ;
                                /* add option letter to string          */
        break;
      case 'c':                 /* check checksums also                 */
        opt_c += 1 ;
        strncat(opt_str,opt_let,sizeof(opt_str)-strlen(opt_str)-1) ;
                                /* add option letter to string          */
        break;
      case 'l':                 /* check links between VPDs             */
        opt_l += 1 ;
        strncat(opt_str,opt_let,sizeof(opt_str)-strlen(opt_str)-1) ;
                                /* add option letter to string          */
        break;
      case 'v':                 /* check install levels between VPDs    */
        opt_v += 1 ;
        strncat(opt_str,opt_let,sizeof(opt_str)-strlen(opt_str)-1) ;
                                /* add option letter to string          */
        break;
      case 'u':                 /* update VPD size/checksum values      */
        opt_u += 1 ;
        break;
      case 'm':                 /* message level                        */
        n = (int) strtol(optarg,&endptr,10) ;
        if (((n==0) && (endptr == optarg)) || (n<0) || (n>MSGMAX))
          syntax_err  = TRUE ;
        else
          opt_m = n ;
        break;
      case 't':                 /* trace level                          */
        n = (int) strtol(optarg,&endptr,10) ;
        if (((n==0) && (endptr == optarg)) || (n<0) || (n>TRCMAX))
          syntax_err = TRUE ;
        else
          opt_t = n ;
        break;
      case 'O':                 /* VPD repositories to use              */
        opt_O ++ ;              /* count occurances of -O               */
        strncpy(rep_str,optarg,sizeof(rep_str)-1);
                                /* copy specified repository codes      */
        opt_Or = opt_Os = opt_Ou = 0 ;
        for (n=0 ; optarg[n] != '\0' ; n++)
          {                     /* process each char in option value    */
          switch(optarg[n])
            {
            case 'r':           /* root repository                      */
              opt_Or += 1;
              break;
            case 's':           /* share repository                     */
              opt_Os += 1;
              break;
            case 'u':           /* usr repository                       */
              opt_Ou += 1;
              break;
            default :           /* unrecognized repository              */
              syntax_err = TRUE ;
              break;
            }
          }
        break;
      default :                 /* not recognized flag                  */
        syntax_err = TRUE ;
        break;
      }                         /* end switch - main option loop        */
    }                           /* end while more options on cmd line   */

  /* validate the set of options specifed - check for improper          */
  /* combinations or extra parameters                                   */

  if ((opt_f + opt_c + opt_l + opt_v) != 1)
    syntax_err = TRUE ;         /* if not exactly one function requested*/

  if ((opt_u> 0) && (opt_c == 0) && (opt_l == 0) && (opt_v==0) )
    syntax_err = TRUE ;         /* update valid only if checksum ,ver,or*/
                                /* link check is also specified         */

  if ((opt_Or >1) || (opt_Ou > 1) || (opt_Os > 1))
    { syntax_err = TRUE ; }     /* if repository specified more the     */
                                /* once, it is an error                 */

  if (opt_O > 1)                /* More than one -O option is an error  */
    { syntax_err = TRUE ; }

  if ( opt_O == 0 )
    { strcpy(rep_str,"urs") ;}  /* if no -O option, set default         */

  if ( opt_m == 0 )
    { opt_m = MSG_DEF ; }       /* set default message level            */


  /* Get the lpp name specification provided.  If none is provided      */
  /* assume all lpps are to be processed.                               */

  if ( optind < argc )          /* if a name was provided use it        */
    { lppname = argv[optind++]; }
  else
    { lppname = "*" ; }         /* else assume all LPPs                 */

  /* if any parameters are left, they must be file specifiers, those    */
  /* are not valid if the function requested is link or level verify    */

  if ((opt_v > 0) && (optind < argc))
    { syntax_err = TRUE ; }     /* if verify install level              */
                                /* and file names are specified - error */

  if (syntax_err)               /* if the command line had any errors   */
    {
    MSG_S(MSG_SEVERE,MSG_CHK_USAGE0,DEF_CHK_USAGE0,lppchk_pn,lppchk_pn,0) ;
    exit(1) ;
    }

  if (optind >= argc)           /* test if any file specifications      */
    {
    file_spec = "*" ;           /* if none supplied, build dummy list of*/
    file_first = &file_spec ;   /* one item which points to a wildcard  */
    file_count = 1 ;            /* as the only item.                    */
    }
  else
    {
    file_first = &(argv[optind]);/* else use command line args as list   */
    file_count = argc - optind; /* compute number of items              */
    } /* endif */

  if (opt_t >= TRC_ALL)
    {
    fprintf(stderr,"Command syntax valid. Arguments are -\n");
    fprintf(stderr,"  progname           = %s\n",lppchk_pn);
    fprintf(stderr,"  command options    = %s\n",opt_str) ;
    fprintf(stderr,"  repository options = %s\n",rep_str) ;
    fprintf(stderr,"  update option      = %d\n",opt_u) ;
    fprintf(stderr,"  lppname            = %s\n",lppname) ;
    fprintf(stderr,"  first file spec    = %s\n",*file_first);
    fprintf(stderr,"  file spec count    = %d\n",file_count) ;
    fprintf(stderr,"  message level      = %d\n",opt_m);
    fprintf(stderr,"  trace level        = %d\n",opt_t);
    }

  /**********************************************************************/
  /* Begin processing the request                                       */
  /**********************************************************************/

  msg_lev = opt_m ;             /* make msg and trace levels available  */
  trc_lev = opt_t ;

  return (
         lppchkd(opt_str, opt_u, rep_str, lppname, file_first, file_count)
         ) ;
  }                             /* end main                             */
