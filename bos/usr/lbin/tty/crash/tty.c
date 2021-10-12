#ifndef lint
static char sccsid[] = "@(#)96 1.2 src/bos/usr/lbin/tty/crash/tty.c, cmdtty, bos411, 9428A410j 3/31/94 08:13:29";
#endif
/*
 * COMPONENT_NAME: CMDTTY terminal control commands
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27, 83
 *
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


#include <stdio.h>
#include <sys/sysmacros.h>
#include <sys/str_tty.h>   
#include <string.h>
#include <nlist.h>
#include <ctype.h>
#include <errno.h>
#include <sys/param.h>

#include "unpack.h"

static int kmem = -1;			/* fd for /dev/kmem or dump file */
static int rflag;			/* true if running system */

static char *progname;
static struct nlist nlistx[] = {
    { "tty_list" },                     
    0,
};

/* this allows for a posible 1040 ttys */
#define MAX_NUM_PAGES 40
static struct all_tty_s *all_ttys[MAX_NUM_PAGES];  

static int parse_line(char *s);
static void do_command();
static setup_array();
static void usage(int);
static void error(char *s);
static int disp_tty (struct per_tty *pert);
static void print_priv(char *disp, void *priv);

/* globals */
char *name;
int maj = -1;
int min = -1;
int vflag = 0;
int oflag = 0;
int dflag = 0;
int lflag = 0;
int eflag = 0;
char ttynam[TTNAMEMAX];	

main(int argc, char *argv[])
{
  char *targ;
  int usr_arg = 0;
  
  if (progname = strrchr(argv[0], '/'))
	  ++progname;
  else
	  progname = argv[0];
  ++argv;
  
  while ((targ = *argv++) && *targ == '-' && !usr_arg) {
    ++targ;
    while (*targ) {
      switch (*targ++) {
 case 'f':			/* file descriptor flag */
	if (*targ)
		kmem = atoi(targ);
	else {
	  if (!*argv)
		  usage(0);
	  kmem = atoi(argv[0]);
	  ++argv;
	}
	targ = "";
	/* The -f option is the last crash arg before user 
	 * supplied args.  
	 */
	usr_arg = 1;
	break;
	
 case 'r':
	rflag = 1;
	break;

 default:
	usage(0);
      }
      
    } /* while *targ */
  }
  
  if (kmem == -1 && (kmem = open("/dev/kmem", 0)) < 0)
	  error("/dev/kmem");
  
  setup_array();
  
  if (targ && parse_line(targ)) {
    do_command();
  }
  return 0;
}

static int parse_line(char *s)
{
  int parsed = 0;
  int len;
  
  while (*s && *s == ' ') ++s;
  
  if ((len = strlen(s)) && s[len-1] == '\n')
	  s[len-1] = '\0';
  
  /* check for new user args -d -l -e -v -o */
  while (*s && *s == '-') {
    s++;
    switch (*s) {
 case 'd':
      if (lflag || eflag || dflag) {
	usage(1);
      }
      dflag = 1;
      do ++s;  while (*s && *s == ' ');
      parsed = 1;
      break;
 case 'l':
      if (lflag || eflag || dflag) {
	usage(1);
      }
      lflag = 1;
      do ++s;  while (*s && *s == ' ');
      parsed = 1;
      break;
 case 'e':
      if (lflag || eflag || dflag) {
	usage(1);
      }
      eflag = 1;
      do ++s;  while (*s && *s == ' ');
      parsed = 1;
      break;
 case 'v':
      vflag = 1;
      do ++s;  while (*s && *s == ' ');
      parsed = 1;
      break;
 case 'o':
      oflag = 1;
      do ++s;  while (*s && *s == ' ');
      parsed = 1;
      break;
 default:
      usage(1);
    }
  }

  if (*s == 'o') {
    /* necessary because crash and pstat still call tty with 'o' arg as
     * default.  it also indicates that there are no other args provided.
     */
    oflag = 1;
    eflag = 1;  
    do ++s;  while (*s && *s == ' '); 
    return 1;
  }
  /* tty name passed as arg */
  if (*s && isalpha(*s)) {
    name = s;
    /* no option given; set eflag */
    if (!lflag && !eflag && !dflag)
	    eflag = 1;
    return 1;
  }
  /* maj/min numbers passed as args */
  name = 0;
  if (isdigit(*s)) {
    maj = 0;
    do maj = maj * 10 + *s++ - '0'; while (isdigit(*s));
    do ++s;  while (*s && *s == ' ');
    /* no option given; set eflag */
    if (!lflag && !eflag && !dflag)
	    eflag = 1;
    parsed = 1;
  }
  if (isdigit(*s)) {
    min = 0;
    do min = min * 10 + *s++ - '0'; while (isdigit(*s));
    do ++s;  while (*s && *s == ' ');
    parsed = 1;
  }
  if (!parsed)
	  usage(1);
  return parsed;
}

static void do_command()
{
  struct all_tty_s *tpp;
  int ii, index = 0, res = 0;
  int once = 0;

  while ((tpp = all_ttys[index]) != NULL) {  
    for (ii = 0; ii < STR_TTY_CNT; ii++) {
      struct per_tty tty_buf;
      tty_buf = tpp->lists[ii];
      /* if oflag set, check for streamhead tty module */
      if (oflag &&
	  tty_buf.mod_info[MODULEMAX-1].mod_ptr == NULL)
	      continue;
      if (tty_buf.ttyname) {
	if (*tty_buf.ttyname == '\0') continue;
	strcpy(ttynam, tty_buf.ttyname);
	if ((name ?
	     once = !strcmp(name, ttynam) :
	     (maj == -1 ||
	      (maj == major(tty_buf.dev) &&
	       (min == -1 ||
		(min == minor(tty_buf.dev)))))) &&
	    (res = disp_tty(&tty_buf))) {
	  /* hook for possible error from disp_tty, currently
	   * this cannot be reached
	   */
	  break;
	}
      }
      if (once)
	      return;
    }
    if (res)
	    break; /* problem in disp_tty for previous page - stop now */
    index++;
  }
}


static setup_array()
{
  int start, end, size;
  int ii, index = 0;

  for (ii=0;ii<MAX_NUM_PAGES;ii++)
	  all_ttys[ii] = NULL;
  
  if (rflag) {				/* running system */
    if (knlist(nlistx, 1, sizeof(struct nlist)) < 0)
	    error("knlist");
    if (lseek(kmem, nlistx[0].n_value, 0) < 0)
	    error("lseek");
    if (read(kmem, &start, sizeof(start)) < sizeof(start))
	    error("read");   /* read the ptr to the start */
    size = PAGESIZE;
    all_ttys[index] = (struct all_tty_s *)malloc(size);
    if (!read_memory(kmem, rflag, all_ttys[index], start, size))
	    error("read of all_tty_s struct");
    while (all_ttys[index]->next != NULL) {
      index++;
      all_ttys[index] = (struct all_tty_s *)malloc(size);
      if (!read_memory(kmem, rflag, all_ttys[index], 
		       all_ttys[index-1]->next, size))
	      error("read of all_tty_s struct");
    }
  } else {				/* process the dump file */
    int t, tptr;
    
    if ((t = name2cdt(kmem, "tty")) < 0) /* find tty entry */
	    error("no tty entry");
    if (unpack(kmem, t, "tty_list", &start, 0, sizeof(start), 1) !=
	sizeof(start))
	    error("unpack tty_list");
    if (unpack(kmem, t, "tty_end", &end, 0, sizeof(end), 1) != 
	sizeof(end))
	    error("unpack tty_end");
    /* could use either the table of pointers or the next field itself */
    all_ttys[index] = (struct all_tty_s *)malloc(PAGESIZE);
    if (!read_memory(kmem, rflag, &tptr, start, sizeof(tptr)))
	      error("read of tty_list");
    if (!read_memory(kmem, rflag, all_ttys[index], tptr, PAGESIZE))
	      error("read of all_tty_s struct");
    while (all_ttys[index]->next != NULL) {
      index++;
      printf("page = %d\n", index);
      all_ttys[index] = (struct all_tty_s *)malloc(size);
      if (!read_memory(kmem, rflag, all_ttys[index], 
		       all_ttys[index-1]->next, size))
	      error("read of all_tty_s struct");
    }
  }	
}

static void usage(int usr_arg_error)
{
  if (usr_arg_error)
    fprintf(stderr, 
	    "Usage: %s [-d | -l | -e] [-o] [-v] [name | [maj [min]]]\n", 
	    progname);
  else
    fprintf(stderr, "Usage: %s kmem_fd [0|1] args\n", progname);
  exit(1);
}

static void error(char *s)
{
    extern int errno;

    if (errno) {
	fprintf(stderr, "%s: ", progname);
	perror(s);
    } else
	fprintf(stderr, "%s: %s\n", progname, s);
    exit(1);
}

static int disp_tty (struct per_tty *pert)
{
  int ii;
  void *priv;
  int found = 0; 
  char mod_name[8];
  struct str_module_conf mod;

  printf("\n/dev/%s (%d,%d)\n", pert->ttyname,
	 major(pert->dev), minor(pert->dev));

  for (ii = MODULEMAX-1; ii >= 0; ii--) {
    if (pert->mod_info[ii].mod_ptr) {
      if (!read_memory(kmem, rflag, &mod, 
		       pert->mod_info[ii].mod_ptr,
		       sizeof(struct str_module_conf)))
	      error("read of str_module_conf");
      strcpy(mod_name,mod.name);
      
      if (dflag) {                   /* driver only */
	if (mod.type == 'd') {
	  printf("\nModule name: %s\n", mod_name); 
	  priv = pert->mod_info[ii].private_data;
	  print_priv(mod_name,priv); 
	  return 0;
	}
      }
      else if (lflag) {              /* line discipline */
	if (mod.type == 'l') {
	  printf("\nModule name: %s\n", mod_name); 
	  priv = pert->mod_info[ii].private_data;
	  print_priv(mod_name,priv); 
	  return 0;
	}
      }
      else {                          /* driver and all modules */
	if (*mod_name == '\0') {
	  continue;
	}
	printf("\nModule name: %s\n", mod_name);
	priv = pert->mod_info[ii].private_data;
	print_priv(mod_name,priv);
      }
    }
  }
  return 0;
}

/* Call discipline specific routines */
static void print_priv(char *disp, void *priv_data)
{
    int pid;
    int w;
    char path[1024];
    char name[256];
    char karg[10];
    char farg[10];
    char priv[10];
    char varg[10];

    sprintf(name, "%s%s", CRASH_PREFIX, disp);
    sprintf(path, "%s/%s", TTYDIR, name);
    sprintf(karg, "%d", kmem);
    sprintf(farg, "%d", rflag);
    sprintf(priv, "%x", priv_data);
    sprintf(varg, "%d", vflag);
    fflush(stdout);
    if ((pid = fork()) < 0)
	return;
    if (!pid) {
	execl(path, name, karg, farg, priv, varg, (char *)0);
	exit(1);
    }
    while ((w = wait()) != pid && w != -1);
}
