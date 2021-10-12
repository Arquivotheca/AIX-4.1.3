#ifndef lint
static char sccsid[] = "@(#)58 1.4 src/bos/usr/ccs/lib/libc/setcsmap.c, sysxtty, bos411, 9428A410j 4/8/94 06:46:58";
#endif
/*
 * COMPONENT_NAME: LIBCTTY terminal control routines
 *
 * FUNCTIONS: setcsmap 
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
 * This routine stores a code set map in the kernel.
 * A code set map determines the length and width of each code point.
 * Length means the number of bytes of storage used and
 * width means the number of display columns to use.
 * 
 * This routine reads the file specified, parses it, builds a csmap 
 * structure and calls the stream ioctl with the command EUC_WSET to 
 * install it in the kernel.
 *
 * If successful, 0 is returned; otherwise, -1.  If not successful,
 * messages are sent to stderr.
 */

/* The new format of the csmap file is as follows:
 * 
 * Name : xxxx
 * Type : M/S      # M : multibyte, S : single byte
 * Multibyte handling : EUC/xxx
 * ioctl EUC_WSET : w1:d1,w2:d2,w3:d3  
 * lower converter : /usr/lib/drivers/xxx  
 * upper converter : /usr/lib/drivers/xxx  
 *
 * 
 */

#define _ILS_MACROS
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/termio.h>
#include <sys/limits.h>
#include <string.h>
#include <nl_types.h>
#include "libc_msg.h"

#include <sys/sysconfig.h>
#include <sys/device.h>
#include <stropts.h>   
#include <sys/eucioctl.h>
#include <sys/str_tty.h>

#include <ctype.h>

nl_catd catd;
#define MSGSTR(n,s)     catgets(catd,LIBCTTY,n,s)

#define OK 0
#define BAD_RES -1

#define LDTERM "ldterm"
#define MAX_LINE_SZ 256
#define NAME "Name"
#define TYPE "Type"
#define MULTIBYTE "Multibyte"
#define HANDLING "handling"
#define IOCTL "ioctl"
#define LOWER "lower"
#define UPPER "upper"
#define CONVERTER "converter"
#define UPCONV "upper converter"
#define LOCONV "lower converter"

#define DELIMIT " \t:,\n"


/* needed to configure the converters */
typedef struct  ulc_dds {
        enum    dds_type        which_dds;      /* dds identifier       */
};


int ioctl_EUC_WSET(struct strioctl *);
int push_converters(char *, char *);

/* Stream ioctl commands.  At this point there is only EUC_WSET.
 */
struct
{
    char *string;
    int cmd;
    int (*func)(struct strioctl *);
} cmds[] = {
    "EUC_WSET",  EUC_WSET, ioctl_EUC_WSET,
    0,
};

struct eucioc eucioc_str;

int
setcsmap(const char *path)	
{
  FILE *file;
  int rc;
  char buf1[MAX_LINE_SZ];       /* buffer to hold line from file */
  char lower_conv[MAX_LINE_SZ];
  char upper_conv[MAX_LINE_SZ];
  int found = 0;
  int upper = 0;
  int lower = 0;
  int ii; 

  struct strioctl i_str;

  char *tok;


  /*
   * Open the code set map file.
   */
  file = fopen(path, "r");
  if (file == (FILE *)0) {
    perror("setcsmap");
    return(BAD_RES);
  }
  rc = OK;
  /* Parse information from file. */
  /* Name field */
  while (fgets(buf1, MAX_LINE_SZ, file) != (char *)0) {
    if (buf1[0] == '#')  
      continue;
    if (!(tok = strtok(buf1, DELIMIT))) {
      continue;   
    }
    if (!strcmp(tok,NAME)) {
      found = 1;
      tok = strtok((char *)0, DELIMIT);
      break;
    }
  }
  if (!found) {
    fclose(file);
    fprintf(stderr,
	    MSGSTR(TTY1,
		   "%s: improper file format: file missing %s field\n"),
	    "setcsmap", NAME);
    errno = EINVAL;
    return(BAD_RES);
  }
  found = 0;
  /* Type field */
  while (fgets(buf1, MAX_LINE_SZ, file) != (char *)0) {
    if (buf1[0] == '#') 
      continue;
    if (!(tok = strtok(buf1, DELIMIT))) {
      continue; 
    }
    if (!strcmp(tok,TYPE)) {
      found = 1;
      tok = strtok((char *)0, DELIMIT);
      if (*tok != 'M' &&
	  *tok != 'S') {
	rc = BAD_RES;
      }
      break;
    }
  }
  if (rc == BAD_RES || !found) {
    if (!found)
      fprintf(stderr,
	      MSGSTR(TTY1,
		     "%s: improper file format: file missing %s field\n"),
	      "setcsmap", TYPE);
    else 
      fprintf(stderr,
	      MSGSTR(TTY2,
		     "%s: improper file format: unexpected value in %s field\n"),
	      "setcsmap", TYPE);
    fclose(file);
    errno = EINVAL;
    return(BAD_RES);
  }
  
  if (*tok == 'S') {
    return(OK);
    
  }
  /* Multibyte handling field.*/
  found = 0;
  while (fgets(buf1, MAX_LINE_SZ, file) != (char *)0) {
    if (buf1[0] == '#')                     /* skip comment lines */
      continue;
    if (!(tok = strtok(buf1, DELIMIT))) {   
      continue; 
    }
    if (!strcmp(tok,MULTIBYTE)) {
      /* the keyword 'handling' should follow 'Multibyte' */
      tok = strtok((char *)0, DELIMIT);
      if (strcmp(tok, HANDLING)) 
	continue;  /* continue looking for the field */
      tok = strtok((char *)0, DELIMIT);
      /* Currently EUC is the only valid value for this field.  This
       * will change in the future, but for now, we cannot allow other
       * values in this field.
       */
      found = 1;
      if (strcmp(tok,"EUC"))
	      rc = BAD_RES;
      break;
    }
  }
  if (rc == BAD_RES || !found) {
    if (!found)
      fprintf(stderr,
	      MSGSTR(TTY1,
		     "%s: improper file format: file missing %s field\n"),
	      "setcsmap", "Multibyte handling");
    else 
      fprintf(stderr,
	      MSGSTR(TTY2,
		     "%s: improper file format: unexpected value in %s field\n"),
	      "setcsmap", "Multibyte handling");
    fclose(file);
    errno = EINVAL;
    return(BAD_RES);
  }

  /* ioctl field. */
  found = 0;
  while (fgets(buf1, MAX_LINE_SZ, file) != (char *)0) {
    if (buf1[0] == '#')
      continue;
    if (!(tok = strtok(buf1, DELIMIT))) {
      continue; 
    }
    if (!strcmp(tok, IOCTL)) {
      tok = strtok((char *)0, DELIMIT);
      /* grab ioctl cmd */
      for (ii=0;cmds[ii].string;ii++) {
	if (!strcmp(cmds[ii].string, tok)) {
	  i_str.ic_cmd = cmds[ii].cmd;
	  /* call subroutine to handle ioctl from this point */
	  rc = cmds[ii].func(&i_str);
	  break;
	}
      }
      if (!i_str.ic_cmd ||
	  rc != OK) {
	fprintf(stderr,
		MSGSTR(TTY2,
		       "%s: improper file format: unexpected value in %s field\n"),
		"setcsmap", IOCTL);
	fclose(file);
	errno = EINVAL;
	return(BAD_RES);
      }
      found = 1;
      break;
    }
  }
  if (!found) {
    fprintf(stderr,
	    MSGSTR(TTY1,
		   "%s: improper file format: file missing %s field\n"),
	    "setcsmap", IOCTL);
    fclose(file);
    errno = EINVAL;
    return(BAD_RES);
  }

  /* converters */
  found = 0;
  while (fgets(buf1, MAX_LINE_SZ, file) != (char *)0) {
    if (buf1[0] == '#')
      continue;
    if (!(tok = strtok(buf1, DELIMIT))) {
      continue; 
    }
    if (!strcmp(tok, LOWER)) 
	    lower++;
    else if (!strcmp(tok, UPPER))
	    upper++;
    else continue;
      
    /* next keyword is 'converter' */
    tok = strtok((char *)0, DELIMIT);
    if (strcmp(tok, CONVERTER)) {
      lower = upper = 0;  /* zero out flag */
      continue;
    }
    tok = strtok((char *)0, DELIMIT);
    if ((*tok == (char *)NULL) ||
	isspace(*tok)) {
      fprintf(stderr,
	      MSGSTR(TTY2,
		     "%s: improper file format: unexpected value in %s field\n"),
	      "setcsmap", (lower ? LOCONV : UPCONV));
      fclose(file);
      errno = EINVAL;
      return(BAD_RES);
    }
    if (lower)
	    strcpy(lower_conv, tok);
    else 
	    strcpy(upper_conv, tok);
    found = 1;
    break;
  }

  if (found) {
    while (fgets(buf1, MAX_LINE_SZ, file) != (char *)0) {
      if (buf1[0] == '#') 
	      continue;
      if (!(tok = strtok(buf1, DELIMIT))) {
	continue; 
      }
      if (!strcmp(tok, UPPER)) {
	if (upper) continue;
      }
      else if (!strcmp(tok, LOWER)) {
	if (lower) continue;
      }
      else continue;

      /* next keyword is 'converter' */
      tok = strtok((char *)0, DELIMIT);
      if (strcmp(tok, CONVERTER)) {
	continue;
      }
      found = 2;
      tok = strtok((char *)0, DELIMIT);
      if ((*tok == (char *)NULL) ||
	  isspace(*tok)) {
	fprintf(stderr,
		MSGSTR(TTY2,
		       "%s: improper file format: unexpected value in %s field\n"),
		"setcsmap", (lower ? UPCONV : LOCONV));
	fclose(file);
	errno = EINVAL;
	return(BAD_RES);
      }

      if (lower)
	      strcpy(upper_conv, tok);
      else
	      strcpy(lower_conv, tok);
      break;

    }
    if (found != 2) {
     fprintf(stderr,
	    MSGSTR(TTY1,
		   "%s: improper file format: file missing %s field\n"),
	    "setcsmap", (lower ? UPCONV : LOCONV));
     fclose(file);
     errno = EINVAL;
     return(BAD_RES);
    }
  }
  fclose(file);
  /*
   * If everything OK, store the code set structure in the kernel.
   * Otherwise, set errno.  
   * If there are converters, they must be pushed on the stream
   * also before calling the ioctl.
   */
  if (rc == OK) {
    if (found &&
	push_converters(lower_conv,upper_conv) < 0) {
      return(BAD_RES);
    }
    
    rc = ioctl(0, I_STR, &i_str); 
    if (rc < 0) {
      perror("setcsmap");
      rc = BAD_RES;
    }
  }
  else
	  errno = EINVAL;
  return(rc);
}

/* 
 * push_converters(char *lower, char *upper) 
 *
 * Function that pushes upper and lower converters onto the stream.
 * The lower converter is pushed onto the stream below the ldterm
 * module, and the upper is pushed above the ldterm module.
 * If any errors occur, a -1 is returned.
 * 
 */
int
push_converters(char *lower, char *upper)
{
  struct str_list strlist;
  int nmods;
  unsigned size;
  struct str_mlist *mod_listp;
  int ii;
  struct termios ldt;
  char *name;
  struct cfg_load cfg_l;
  struct cfg_kmod cfg_k;
  int    cmd;			  /* command parameter to sysconfig */
  struct ulc_dds ulc_dds;
  int result = OK;

  int fd = fileno(stdin);

  /* get list of modules in the stream */
  if ((nmods = ioctl(fd, I_LIST, (char *)0)) < 0) {
       perror("setcsmap: ioctl I_LIST ");
       return (BAD_RES);
  }
  size = nmods * sizeof(struct str_mlist);
  strlist.sl_nmods = nmods;
  strlist.sl_modlist = (struct str_mlist *)malloc(size);
  /* check for malloc failure */
  if (strlist.sl_modlist == NULL) {
    perror("setcsmap: malloc ");
    return(BAD_RES);
  }
  memset(strlist.sl_modlist, '\0', (int)size);
  if ((nmods = ioctl(fileno(stdin), I_LIST, (char *)&strlist)) < 1) {
    perror("setcsmap: ioctl I_LIST ");
    return (BAD_RES);
  }

  /* check to see if lower converter is already on the stream.  
   * Don't need to check for both; if only one were on the stream
   * we would have died long before someone could do a setcsmap.
   * The arg lower contains the full pathname; pull the module name
   * off the end of it.  We are assuming the module name is the same
   * as the binary name.
   */
  name = strrchr(lower,'/') + 1;
  mod_listp = strlist.sl_modlist;
  for (ii=0;ii < nmods-1;ii++) {       
    /* last item in list is the driver, nmods-1 saves a strcmp */
   if (!strcmp(mod_listp->l_name,name)) {
     /* module already on the stream; exit. */
     return(OK);
   }
   mod_listp++;
  }

  mod_listp = strlist.sl_modlist;
  nmods = strlist.sl_nmods - 1;  /* don't pop the driver */

  if (tcgetattr(fd, &ldt) == -1) {
    perror("setcsmap: tcgetattr ");
    return(BAD_RES);
  }

  /* must load and config the converters if not already done so */ 

  cmd = SYS_QUERYLOAD;
  cfg_l.path = (caddr_t)lower;
  cfg_l.libpath = (caddr_t) NULL;
  if (sysconfig(cmd, (void *)&cfg_l, 
		(int)sizeof(struct cfg_load)) < 0) {
    perror("setcsmap: sysconfig SYS_QUERYLOAD");
    return(BAD_RES);
  }
  if (cfg_l.kmid == (mid_t) 0) {
    /* not loaded */
    cmd = SYS_SINGLELOAD;
    if (sysconfig(cmd, (void *)&cfg_l, 
		  (int)sizeof(struct cfg_load)) < 0) {
      perror("setcsmap: sysconfig SYS_SINGLELOAD");
      return(BAD_RES);
    }
  }
  /* config the lower converter module */
  cfg_k.kmid = cfg_l.kmid;
  cfg_k.cmd = CFG_INIT;
  ulc_dds.which_dds = LC_SJIS_DDS;
  cfg_k.mdiptr = (caddr_t)&ulc_dds;
  cfg_k.mdilen = sizeof(struct ulc_dds);
  if ((sysconfig(SYS_CFGKMOD, 
		 (void *)&cfg_k,
		 (int)sizeof(struct cfg_kmod))) < 0) {
    perror("setcsmap: sysconfig SYS_CFGKMOD");
	   return (BAD_RES);
  }
 
  cmd = SYS_QUERYLOAD;
  cfg_l.path = (caddr_t)upper;
  cfg_l.libpath = (caddr_t) NULL;
  if (sysconfig(cmd, (void *)&cfg_l, 
		(int)sizeof(struct cfg_load)) < 0) {
    perror("setcsmap: sysconfig SYS_QUERYLOAD");
    return(BAD_RES);
  }

  if (cfg_l.kmid == (mid_t) 0) {
    /* not loaded */
    cmd = SYS_SINGLELOAD;
    if (sysconfig(cmd, (void *)&cfg_l, 
		  (int)sizeof(struct cfg_load)) < 0) {
      perror("setcsmap: sysconfig SYS_SINGLELOAD");
      return(BAD_RES);
    }
  }
  /* config the upper converter module */
  cfg_k.kmid = cfg_l.kmid;
  cfg_k.cmd = CFG_INIT;
  ulc_dds.which_dds = UC_SJIS_DDS;
  cfg_k.mdiptr = (caddr_t)&ulc_dds;
  cfg_k.mdilen = sizeof(struct ulc_dds);
  if ((sysconfig(SYS_CFGKMOD, 
		 (void *)&cfg_k,
		 (int)sizeof(struct cfg_kmod))) < 0) {
    perror("setcsmap: sysconfig SYS_CFGKMOD");
	   return (BAD_RES);
  }
  
  for (ii=0;ii < nmods; ii++) {
    if (ioctl(fd, I_POP, 0) < 0) {
      perror("setcsmap: ioctl I_POP: ");
      return (BAD_RES);
    }
  }
  
  /* now push modules back on the stream in reverse order,
   * placing the lower converter on the stream before ldterm
   * and the upper converter on the stream after ldterm.
   */
  for (ii=nmods-1;ii >= 0; ii--) {
    name = (mod_listp+ii)->l_name;
    if (strcmp(name,LDTERM) == 0) {
      char *cname;
      cname = strrchr(lower,'/') + 1;
      if ((result = ioctl(fd, I_PUSH, cname)) < 0) {
	perror("setcsmap: ioctl I_PUSH: ");
      }
      /* even though unable to push lower converter, still try
       * to push back ldterm
       */
      if (ioctl(fd, I_PUSH, name) < 0) {
	perror("setcsmap: ioctl I_PUSH: ");
	return(BAD_RES);
      }
      /* if the lower converter failed to be pushed, don't
       *bother trying to push the upper converter...
       */
      if (!result) {
	cname = strrchr(upper,'/') + 1;
	if (ioctl(fd, I_PUSH, cname) < 0)
		/* this means there's a lower converter on the stream
		 * but no corresponding upper converter; continue trying
		 * to restore the stream.
		 */
		perror("setcsmap: ioctl I_PUSH: ");
      }
    }
    else if (ioctl(fd, I_PUSH, name) < 0) {
      perror("setcsmap: ioctl I_PUSH ");
      return (BAD_RES);
    }   
  }

  if (tcsetattr(0, TCSANOW, &ldt) == -1) {
    perror("setcsmap: tcsetattr: ");
    return (BAD_RES);
  }
  return (result);
}


/* function to handle the ioctl field for the EUC_WSET command */
int 
ioctl_EUC_WSET(struct strioctl *i_str)
{
  int ii,xx;
  char *tok;

  /* grab arg to ioctl cmd */
  bzero(&eucioc_str, sizeof(struct eucioc));
  for (ii=1;ii<4;ii++) {
    tok = strtok((char *)0, DELIMIT);
    if ((xx = atoi(tok)) > 8) {
      return(BAD_RES);
    }
    eucioc_str.eucw[ii] = xx;
    tok = strtok((char *)0, DELIMIT);
    if ((xx = atoi(tok)) > 8) {
      return(BAD_RES);
    }
    eucioc_str.scrw[ii] = xx;
  }
  eucioc_str.eucw[0] = 1;
  eucioc_str.scrw[0] = 1; 
  i_str->ic_timout = 0;
  i_str->ic_len = sizeof(struct eucioc);
  i_str->ic_dp = (char *)&eucioc_str;
  return(OK);
}
