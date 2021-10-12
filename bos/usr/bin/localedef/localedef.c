static char sccsid[] = "@(#)98	1.9.1.5  src/bos/usr/bin/localedef/localedef.c, cmdnls, bos411, 9428A410j 3/11/94 12:00:18";
/*
 * COMPONENT_NAME: (CMDNLS) Locale Database Commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 85
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.2
 */
#include <sys/localedef.h>
#include <sys/lc_sys.h>
#include <sys/limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>	
#include <nl_types.h>
#include <sys/wait.h>
#include "symtab.h"
#include "err.h"
#include "method.h"

/* global file name pointer */
char *yyfilenm;
int warn = FALSE;
/*
** This is a flag to signify that a private method table is to
** be generated. This will be set in yyparse for the method file.
*/
int private_table=FALSE;

char     *locname;

int	verbose;		/* verbose flag used by sem_meth.c */

/* tool path used by sem_method.c and main */

char     *tpath;
char     *ccopts;
char     *ldopts;

char *tmp_method_path = NULL;

extern void initlex(void);
extern void initgram(void);

void initparse(void) {
    initlex();
    initgram();
}

/* from init.c */
extern void init_symbol_tbl(int);

void usage(int);

int main(int argc, char *argv[])
{
  extern int err_flag;
  extern symtab_t cm_symtab;
  extern int yydebug;
  extern int optind;
  extern char *optarg;
  extern char *lib_str;

  int      i;
  int      c;
  int      force;
  int      sym_dump;
  int      fd;
  FILE     *fp_meth;
  FILE     *fp_data;
  char     *cmdstr;
  char     *methsrc;
  char     *cmapsrc;
  char     *locsrc;
  char     *tmpfilenm_meth;
  char     *tmpfilenm_data;
  char 	   *char_path;
  symbol_t *p;

  yydebug = verbose = sym_dump = force = FALSE;
  ldopts = ccopts = tpath = cmapsrc = locsrc = methsrc = lib_str = NULL;
  tmpfilenm_meth = "";

  setlocale(LC_MESSAGES,"");

#ifdef TMPMETHPATH
  while ((c=getopt(argc, argv, "sdvwm:cf:i:C:L:P:M:")) != EOF) 
#else
  while ((c=getopt(argc, argv, "sdvwm:cf:i:C:L:P:")) != EOF) 
#endif
    switch (c) {
    case 's':
      sym_dump = TRUE;
      break;
    case 'd':			/* parser debug */
      yydebug = 1;
      break;

    case 'w':			/* display duplicate definition warnings */
      warn = TRUE;
      break;

    case 'c':			/* generate a locale even if warnings */
      force = TRUE;
      break;

    case 'v':			/* verbose table dump */
      verbose = 1;
      break;

    case 'm':			/* specify method file name */
      methsrc = optarg;
      break;

    case 'f':			/* specify charmap file name */
      cmapsrc = optarg;
      break;

    case 'i':			/* specify locale source file */
      locsrc = optarg;
      break;
	
    case 'P':			/* tool path */
      tpath = optarg;
      break;

    case 'C':			/* special compiler options */
      ccopts = optarg;
      break;

    case 'L':			/* special linker options */
      ldopts = optarg;
      break;

#ifdef TMPMETHPATH
    case 'M':			/* path to pre-built temporary method */
      tmp_method_path = optarg;	/* This is only available to the      */
      break;			/* build environment (i.e. locdef )   */
#endif
    }

  if (optind < argc) {
    /* create locale name */
    locname = MALLOC(char, strlen(argv[optind])+1);
    strcpy(locname, argv[optind]);
  } else
    usage(4);

#ifdef _POSIX2_LOCALEDEF
#ifdef _XOPEN_XCU_VERSION
  if ((!_POSIX2_LOCALEDEF) && (_XOPEN_XCU_VERSION<3))
       error(ERR_NOSUPPORT);
#endif
#endif

  putenv("TMPDIR=");   /* prevents tempnam() from returning wrong directory */

  /* process method source file, if specified  
  */
  if (methsrc != NULL) {
      int fd0;

      fd0 = dup(0);
      close(0);
      fd = open(methsrc, O_RDONLY);

      yyfilenm = methsrc;	     /* set file name being parsed for */

      if (fd < 0)
          error(ERR_OPEN_READ, methsrc);

				     /* error reporting. */
      initparse();
      yyparse();
				     /* the private_table flag will be */
				     /* set by yyparse at this point   */
      close(fd);
      close(0);			     /* close method file */
      dup(fd0);			     /* restore saved descriptor to 0 */
  } 

  /* seed symbol table with default values for mb_cur_max, mb_cur_min, 
  ** and codeset 
  */
  if (cmapsrc != NULL)
      init_symbol_tbl(FALSE);	/* don't seed the symbol table */
  else
      init_symbol_tbl(TRUE);	/* seed the symbol table with POSIX PCS */

  /* process charmap if present */
  if (cmapsrc != NULL) {
    int fd0;

    /* if there are no /'s in the path, assume that they are specifying
       a file that is in /usr/lib/nls/charmap/  only if that file does
       not exist in the current directory */

    fd0 = dup(0);                    /* dup current stdin */
    close(0);			     /* close stdin, i.e. fd 0 */

    if (strrchr(cmapsrc,'/') == NULL){
	if ((fd = open(cmapsrc, O_RDONLY)) < 0){
	    char_path= MALLOC(char, strlen(cmapsrc) + 23);
            strcpy(char_path, "/usr/lib/nls/charmap/");
	    strcat(char_path,cmapsrc);
	    fd = open(char_path, O_RDONLY);
            cmapsrc=char_path;
	} 
	else 
	    char_path = '\0';
    }
    else {
        fd = open(cmapsrc, O_RDONLY);    /* new open gets fd 0 */
	char_path = '\0';
    }
        
    yyfilenm = cmapsrc;		     /* set filename begin parsed for */
	
    if (fd < 0)
      error(ERR_OPEN_READ, cmapsrc);

				     /* error reporting.  */

    initparse();
    yyparse();			     /* parse charmap file */


    if (char_path != '\0' )
        free(char_path);

    /* restore stdin */
    close(fd);
    close(0);			     /* close charmap file */
    dup(fd0);			     /* restore saved descriptor to 0 */
  } else
    yyfilenm = "stdin";
  
  /* process locale source file.  if locsrc specified use it,
  ** otherwise process input from standard input 
  */
  if (locsrc != NULL) {
    
    close(0);			     /* close stdin, i.e. fd 0 */
    fd = open(locsrc, O_RDONLY);
    yyfilenm = locsrc;		     /* set file name being parsed for */
				     /* error reporting. */
    if (fd < 0)
      error(ERR_OPEN_READ, locsrc);

  } else
    yyfilenm = "stdin";

  initparse();
  yyparse();
  close(fd);

  if (sym_dump) {
    /* dump symbol table statistics */
    int      i, j;
    symbol_t *p;

    for (i=0; i < HASH_TBL_SIZE; i++) {
      j=0;
      for (p= &(cm_symtab.symbols[i]); p->next != NULL; p=p->next)
	j = j + 1;
      printf("bucket #%d - %d\n", i, j);
    }
  }

  if (!force && err_flag)
      exit(4);

  /* check and initialize if necessary linker/compiler opts and tool paths. */
  if (ldopts==NULL)
      ldopts = "";
  if (tpath == NULL)
      tpath  = "";
  if (ccopts==NULL)
      ccopts = "";

  /* Open temporary file for locale source.  */
  if (private_table) {
      tmpfilenm_meth = tempnam("./", "locale");
      strcat(tmpfilenm_meth,".c");
      fp_meth = fopen(tmpfilenm_meth, "w");
      if (fp_meth == NULL) {
        error(ERR_WRT_PERM, tmpfilenm_meth);
      }
  }

  tmpfilenm_data = tempnam("./", "locale");
  strcat(tmpfilenm_data,".c");
  fp_data = fopen(tmpfilenm_data, "w");
  if (fp_data == NULL) {
    error(ERR_WRT_PERM, tmpfilenm_data);
  }

  /* generate the C code which implements the locale */
  if (private_table) {
      gen(fp_data,fp_meth);
      fclose(fp_meth);
      fclose(fp_data);
  }
  else {
      gen(fp_data,0);
      fclose(fp_data);
  }

  /* compile the C file created. The define for CC_CMD_FMT can be found in
     method.h */
  cmdstr = malloc(sizeof(CC_CMD_FMT)+strlen(tpath)+strlen(tmpfilenm_data)+strlen(ccopts)+32);
  if (cmdstr == NULL)
      error(ERR_MEM_ALLOC_FAIL);


  sprintf(cmdstr, CC_CMD_FMT, tpath, tmpfilenm_data, ccopts);
  if (verbose)
      printf("%s\n",cmdstr);
  c = system(cmdstr);
  free(cmdstr);
  /* delete the C file after compiling */
  if (!verbose) 
    unlink(tmpfilenm_data);
  else {
    /* rename to localename.c */
    extern symtab_t cm_symtab;
    char *s;

    if (private_table) {
        s = MALLOC(char, strlen(locname)+10);
        strcpy(s, locname);
        strcat(s, "_data.c");
	unlink(s);
        rename(tmpfilenm_data, s);
    }
    else {
        s = MALLOC(char, strlen(locname)+3);
        strcpy(s, locname);
        strcat(s, ".c");
	unlink(s);
        rename(tmpfilenm_data, s);
    }

  }
  tmpfilenm_data[strlen(tmpfilenm_data)-1] = 'o';
  if (WIFEXITED(c))
    switch (WEXITSTATUS(c)) {
      case 0:   break;                  /* Successful compilation */

      case -1:  perror("localedef");    /* system() problems? */
                exit(4);

      case 127: error(ERR_NOSHELL);     /* cannot exec /usr/bin/sh */

      default:  /*  the problem may be that the compiler does not
                 *  exist on this system.  First, regenerate the pathname,
                 *  then attempt to open it.  If we can't open it, generate
                 *  a specific error message.  Otherwise, generate a 
                 *  general error message.
                 */
                { FILE *fp;
                  char *fname, *ptr;
                  fname=(char *)malloc(strlen(tpath)+strlen(CC_CMD_FMT)+1);
		  if (fname == NULL)
		      error(ERR_MEM_ALLOC_FAIL);
                  sprintf(fname,CC_CMD_FMT,tpath);
                  if ((ptr=strchr(fname,' ')) != NULL)
                     *ptr='\0';
                  if ((fp = fopen(fname,"r")) == NULL)
                      error(ERR_NOPROG, fname);
                  close(fp);
                  error(ERR_BAD_CHDR);    /* take a guess.. */
                 }
    }
  else
    error(ERR_INTERNAL, tmpfilenm_data, 0);

  if (private_table) {
      /* compile the C file created */
      cmdstr = malloc(sizeof(CC_CMD_FMT) + strlen(tpath) + 
	              strlen(tmpfilenm_meth) + strlen(ccopts) + 32);
      if (cmdstr == NULL)
          error(ERR_MEM_ALLOC_FAIL);
      sprintf(cmdstr, CC_CMD_FMT, tpath, tmpfilenm_meth, ccopts);
      if (verbose) printf("%s\n",cmdstr);
      c = system(cmdstr);
      free(cmdstr);
      /* delete the C file after compiling */
      if (!verbose) 
        unlink(tmpfilenm_meth);
      else {
        /* rename to localename.c */
        extern symtab_t cm_symtab;
        char *s;

        s = MALLOC(char, strlen(locname)+10);
        strcpy(s, locname);
        strcat(s, "_meth.c");
	unlink(s);
        rename(tmpfilenm_meth, s);

      }
      tmpfilenm_meth[strlen(tmpfilenm_meth)-1] = 'o';
      if (WIFEXITED(c))
        switch (WEXITSTATUS(c)) {
          case 0:   break;                  /* Successful compilation */

          case -1:  perror("localedef");    /* system() problems? */
                    exit(4);

          case 127: error(ERR_NOSHELL);     /* cannot exec /usr/bin/sh */

          default:  /*  the problem may be that the compiler does not
                     *  exist on this system.  First, regenerate the pathname,
                     *  then attempt to open it.  If we can't open it generate
                     *  a specific error message.  Otherwise, generate a 
                     *  general error message.
                     *
                     * note: we could probably assume that there is a
                     * c compiler on the system if it got this far, but for
                     * the sake of robustness, I am checking again.  I
                     * do not think that performance is a big issue HERE
                     * because this is an error path.
                     */
                    { FILE *fp;
                      char *fname, *ptr;
                      fname=(char *)malloc(strlen(tpath)+strlen(CC_CMD_FMT)+1);
		      if (fname == NULL)
			  error(ERR_MEM_ALLOC_FAIL);
                      sprintf(fname,CC_CMD_FMT,tpath);
                      if ((ptr=strchr(fname,' ')) != NULL)
                         *ptr='\0';
                      if ((fp = fopen(fname,"r")) == NULL)
                          error(ERR_NOPROG, fname);
                      close(fp);
                      error(ERR_BAD_CHDR);    /* take a guess.. */
                     }

        }
      else
        error(ERR_INTERNAL, tmpfilenm_meth, 0);
  }

  /* 
    re-link the created object, specifying the entry point as
    lc_obj_hdl. The defines for LDOPT's can be found in method.h. 
  */
  if (ldopts[0] != '\0') {
      cmdstr = malloc(sizeof(LDOPT_CMD_FMT) + strlen(tpath) +
		      strlen(tmpfilenm_data) + strlen(ldopts) +
		      strlen(tmpfilenm_meth) + strlen(locname) +
		      strlen(lib_str) + 32);
      if (cmdstr == NULL)
           error(ERR_MEM_ALLOC_FAIL);
      sprintf(cmdstr, LDOPT_CMD_FMT, tpath, tmpfilenm_data, tmpfilenm_meth, 
	      lib_str, ldopts, locname);
  }
  else if (lib_str != NULL) {
     cmdstr = malloc(sizeof(LDOPT_CMD_FMT)+strlen(tpath)+
	             strlen(tmpfilenm_data)+strlen(lib_str)+
	             strlen(tmpfilenm_meth) + strlen(locname)+32);
      if (cmdstr == NULL)
           error(ERR_MEM_ALLOC_FAIL);
     sprintf(cmdstr,LDOPT_CMD_FMT,tpath,tmpfilenm_data,tmpfilenm_meth,lib_str,
	     "",locname); 
  } else {
      cmdstr = malloc(sizeof(LDOPT_CMD_FMT)+strlen(tpath)+
		      strlen(tmpfilenm_data) + strlen(tmpfilenm_meth) 
		      +strlen(locname)+32);
      if (cmdstr == NULL)
           error(ERR_MEM_ALLOC_FAIL);
      sprintf(cmdstr, LD_CMD_FMT, tpath, tmpfilenm_data, tmpfilenm_meth, locname);
  }

  if (verbose)
       printf("%s\n",cmdstr);
  c = system(cmdstr);
  free(cmdstr);
  /* unlink the original object */
  unlink(tmpfilenm_data);
  if (tmpfilenm_meth != NULL) 
	unlink(tmpfilenm_meth);
  if (WIFEXITED(c))
    switch (WEXITSTATUS(c)) {
      case 0:   break;                  /* Successful compilation */

      case -1:  perror("localedef");    /* system() problems? */
                exit(4);

      case 127: error(ERR_NOSHELL);     /* cannot exec /usr/bin/sh */

      default:  /*  the problem may be that the linker does not
                 *  exist on this system.  First, regenerate the pathname,
                 *  then attempt to open it.  If we can't open it generate
                 *  a specific error message.  Otherwise, generate a 
                 *  general error message.
                 */
               { FILE *fp;
                 char *fname, *ptr;
                 fname=(char *)malloc(strlen(tpath)+strlen(LD_CMD_FMT)+1);
		 if (fname == NULL)
                     error(ERR_MEM_ALLOC_FAIL);
                 sprintf(fname,LD_CMD_FMT,tpath);
                 if ((ptr=strchr(fname,' ')) != NULL)
                    *ptr='\0';
                 if ((fp = fopen(fname,"r")) == NULL)
                     error(ERR_NOPROG, fname);
                 close(fp);
                 error(ERR_WRT_PERM, locname);   /* take a guess.. */
               }
    }
  else
    error(ERR_INTERNAL, tmpfilenm_data, 0);

  exit ( err_flag != 0 );	/* 1=>created with warnings */
}				/* 0=>no problems */


