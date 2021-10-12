static char sccsid[] = "@(#)24  1.10  src/bos/usr/lib/pios/piopredef.c, cmdpios, bos411, 9428A410j 5/28/91 12:48:54";
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
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*** piopredef.c ***/

#include <locale.h>
#include "virprt.h"

/*============================================================================*/
void main(argc,argv)
int argc;
char *argv[];
{
  int i, j, replace;
  char *rflag;    /* dummy variable so that parse will work right  */

  INIT(rflag);

  (void) setlocale(LC_ALL, "");

  /* Convert New Flag Letters (if present) To Old Flag Letters (s->d, d->v) */
  { CNVTFLAGS }

  if (parse(argc,argv,"d:q:t:v:r","dqtv",0,&att,5,
               &dstype,&pqname,&ptype,&vpname,&rflag)) err_sub(ABORT,USAGE_PP);

  for ( replace=0, j=1 ; j<argc ; j++ )
                  if (strncmp(argv[j],"-r",2) == 0) replace = 1;

  make_files();

  sprintf(prefile,"%s%s.%s",prepath,ptype,dstype);
  sprintf(cusfile,"%s%s:%s",cuspath,pqname,vpname); 

  reverse(replace);

  sprintf(cusfile,"%s%s.%s",prepath,ptype,dstype);     /* being cute here */
  putval("md","");
  putval("mn","");
  putval("mq","");
  putval("mt","");
  putval("mv","");
  for(i=0; *att[i].attribute; i++) putval(att[i].attribute,att[i].value);

  sprintf(cmd,"%s -s! -i%s",PIOCNVT,cusfile); /* contract the file in place  */
  system(cmd);

}

/*============================================================================*/
reverse(replace)
int replace;
{ FILE *pre, *cus;
  int ch;

  switch( file_stat(cusfile) )
      {
      case DZNTXST: err_sub(ABORT,CUS_NOXST);            /* doesn't exist */
      case PERM_OK: break;                      /* exists */
      case PERM_BAD: err_sub(ABORT,CUS_NOPEN);          /* can't open */
      }

  switch( file_stat(prefile) )
      {
      case DZNTXST: break;                   /* doesn't exist */
      case PERM_OK: if ( !replace )                 /* exists */
                       err_sub(ABORT,PRE_XST);          /* can't open */
                    break;
      case PERM_BAD: err_sub(ABORT,PRE_NOPEN);          /* can't open */
      }

  umask(75);
  if (!( pre = fopen(prefile,"w")))
     err_sub(ABORT,PRE_NOPEN);                  /* can't open */
  if (!(cus = fopen(cusfile,"r")))
     err_sub(ABORT,CUS_NOPEN);             /* can't open */

  while ( (ch = getc(cus) ) != EOF ) putc((char)ch,pre);

  fclose(pre);
  fclose(cus);
}
