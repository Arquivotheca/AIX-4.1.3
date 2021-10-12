#ifndef lint
static char sccsid[] = "@(#)15  1.2  src/bos/usr/lib/pios/piodmgrsu.c, cmdpios, bos411, 9438C411a 9/23/94 15:42:30";
#endif
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS:  main()
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <sys/id.h>
#include <sys/wait.h>
#include <sys/errno.h>

extern int			seteuid(uid_t);
extern int			setegid(gid_t);

#define PIODMGR		"/usr/lib/lpd/pio/etc/piodmgr"
#define PIODMGR_FLAGS	"-h"
#define ODMFLUID	0
#define ODMFLGID	9
#define WHSPCHRS	" \t"
#define ODMFILES	"/var/spool/lpd/pio/@local/piocfg" \
			" /var/spool/lpd/pio/@local/smit/sm_menu_opt" \
			" /var/spool/lpd/pio/@local/smit/sm_menu_opt.vc" \
			" /var/spool/lpd/pio/@local/smit/sm_name_hdr" \
			" /var/spool/lpd/pio/@local/smit/sm_name_hdr.vc" \
			" /var/spool/lpd/pio/@local/smit/sm_cmd_hdr" \
			" /var/spool/lpd/pio/@local/smit/sm_cmd_hdr.vc" \
			" /var/spool/lpd/pio/@local/smit/sm_cmd_opt" \
			" /var/spool/lpd/pio/@local/smit/sm_cmd_opt.vc"



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           main                                                         *
*                                                                              *
* DESCRIPTION:    Perform main logic.                                          *
*                                                                              *
* PARAMETERS: 	                                                               *
*                                                                              *
* RETURN VALUES:                                                               *
*                                                                              *
*******************************************************************************/
int
main(void)
{
   uid_t		real_uid;
   uid_t		saved_uid;
   gid_t		real_gid;
   gid_t		saved_gid;
   pid_t		pid;
   int			status;
   char			odmfls[] = ODMFILES;
   char			*cp;

   /* Query real and saved user ids and set effective user to real id. */
   real_uid 	= getuidx(ID_REAL);
   saved_uid 	= getuidx(ID_SAVED);
   real_gid 	= getgidx(ID_REAL);
   saved_gid 	= getgidx(ID_SAVED);
   (void)seteuid(real_uid);
   (void)setegid(real_gid);

   (void)setlocale(LC_ALL,"");

   switch(pid = fork()) {
      case 0:
         (void)setegid(saved_gid);
         (void)execl(PIODMGR,basename(PIODMGR),PIODMGR_FLAGS,(char *)NULL);
         (void)setegid(real_gid);
	 exit(EXIT_FAILURE);
      case -1:
	 return EXIT_FAILURE;
      default:
         while(waitpid(pid,&status,0) == -1)
	    if(errno != EINTR)
	       return EXIT_FAILURE;
   }

   (void)seteuid(saved_uid);
   for (cp = strtok(odmfls,WHSPCHRS); cp; cp = strtok(NULL,WHSPCHRS))
      (void)chown(cp,(uid_t)ODMFLUID,(gid_t)ODMFLGID);
   (void)seteuid(real_uid);

   return EXIT_SUCCESS;
}	/* end - main() */
