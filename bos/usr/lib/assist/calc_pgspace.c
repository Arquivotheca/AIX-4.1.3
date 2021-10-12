static char sccsid[] = "@(#)11  1.4  src/bos/usr/lib/assist/calc_pgspace.c, cmdassist, bos41J, 9517B_all 4/27/95 11:19:07";
/*
 *   @(#)11  1.3  calc_pgspace.c, cmdassist, bos41J 11/21/94 14:55:38
 *
 *   COMPONENT_NAME:  cmdassist
 *
 *   FUNCTIONS: MAX
 *		MIN
 *		main
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

/*
 * NAME: calc_space
 *
 * FUNCTION:
 *    This program will be called to calculate the minimum recommended page
 *    space size and the default page space size.
 *
 * EXECUTION ENVIRONMENT:
 *    Executes under install_assist environment.
 *
 * INPUT VALUES:
 *    Two options: recommended_ps, default_ps.
 *
 * OUTPUT VALUES:
 *    value of recommended_ps or value of default_ps.
 *
 * RETURNS:
 *    0       Successful completion.
 *    1       Error.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/access.h>

#define MAXLINE 1024
#define DESKTOP_CHECK "/usr/dt/bin/dtlogin"
#define MIN(a, b)  ((a < b) ? a : b)
#define MAX(a, b)  ((a > b) ? a : b)

extern FILE     *popen();

main(int argc, char **argv)
{

char *string;
int memory_size, recommended_ps;
int pp_size, total_pps, free_pps, current_ps, rootvg_size;
int free_space, delta_ps, default_ps;
int root_percent;
char line[MAXLINE], command[MAXLINE + 10];
FILE *fp;

/* Calculate memory size in megabytes. */
sprintf(command, "bootinfo -r");
if ((fp = popen(command, "r")) == NULL)
{
#ifdef DEBUG
  fprintf (stdout, "Command = %s,  fp = %x\n", command, fp);
#endif
  exit (1);
}

if ((fgets(line, MAXLINE, fp)) != NULL)
{
  memory_size=atoi(line);
  memory_size = memory_size/1024;
#ifdef DEBUG
  fprintf (stdout, "memory size = %d MB\n", memory_size);
#endif
}
else
{
  exit (1);
}


/* Calculate recommended page space size */
if (access(DESKTOP_CHECK,F_OK)==0) {
  /* desktop is installed, more paging space is needed (Defect 167922) */
  #ifdef DEBUG
    fprintf (stdout, "Desktop is installed\n");
  #endif
  if (memory_size < 32)
  {
    recommended_ps = 3 * memory_size;
  }
  else
  {
    recommended_ps = memory_size + 64;
  }
}
else {
  if (memory_size < 32)
  {
    recommended_ps = 2 * memory_size;
  }
  else
  {
    recommended_ps = memory_size + 32;
  }
}
/* Defect 176825: Removing need for pp_size to increase speed    */
/* when calculating recommended for display only in smit dialog. */
/* When calculating default_ps, the recommended ps will be rounded.*/
/* recommended_ps = recommended_ps - (recommended_ps % pp_size); */

/* Defect 176825: Only do this if we're looking for default paging space */
if (strcmp(argv[1], "default_ps") == 0) {
   /* Get the partition size, total physical partitions, and free physical
    * partitions in rootvg
    */
   sprintf(command, "lsvg rootvg");
   if ( (fp = popen(command, "r")) == (FILE *) NULL)
   {
   #ifdef DEBUG
     fprintf (stdout, "Command = %s,  fp = %x\n", command, fp);
   #endif
     exit (1);
   }

   while ((fgets(line, MAXLINE, fp)) != NULL)
   {

     string = strstr(line, "PP SIZE:");
     if (string != 0)
     {
       pp_size=atoi(strtok(string, "PP SIZE:"));
     }

     string = strstr(line, "TOTAL PPs:");
     if (string != 0)
     {
       total_pps=atoi(strtok(string, "TOTAL PPs:"));
     }

     string = strstr(line, "FREE PPs:");
     if (string != 0)
     {
       free_pps=atoi(strtok(string, "FREE PPs:"));
     }

   }
   #ifdef DEBUG
     fprintf (stdout, "pp_size = %d,  total_pps = %d,  free_pps = %d\n",
	pp_size, total_pps, free_pps);
   #endif
	
    /* Now that we have pp_size, adjust recommended size to nearest
     * pp boundary. */
    recommended_ps = recommended_ps - (recommended_ps % pp_size);
    #ifdef DEBUG
      fprintf (stdout, "recommended ps = %d MB\n", recommended_ps);
    #endif

    /* Get the current size of the page space in megabytes.
     */
    sprintf(command, "lsps -t lv");
    if ( (fp = popen(command, "r")) == NULL)
    {
    #ifdef DEBUG
      fprintf (stdout, "Command = %s,  fp = %x\n", command, fp);
    #endif
      exit (1);
    }

    current_ps = 0;
    if ((fgets(line, MAXLINE, fp)) != NULL)
    {
      while ((fgets(line, MAXLINE, fp)) != NULL)
      {
	strtok(line, "\t\n ");
	strtok((char *)0, "\t\n ");
	strtok((char *)0, "\t\n ");
	string=strtok((char *)0, "\t\n ");
	string[strlen(string) -2] = '\0';
	current_ps = current_ps + atoi(string);;
      }
    }


    /* Calculate total size of rootvg.
     */
    rootvg_size = total_pps * pp_size;


    /* Calculate amount of free space in rootvg.
     */
    free_space = free_pps * pp_size;

    #ifdef DEBUG
      fprintf (stdout, "rootvg size = %d,  free_space = %d,  current ps = %d\n",
	 rootvg_size, free_space, current_ps);
    #endif


    /* Calulate the default new page space size by limiting the recommended
     * paging space size by 20% of the total rootvg size or the memory size
     * whichever is larger.
     */
    root_percent = (int) (.2 * rootvg_size);
    default_ps = MIN ((MAX (root_percent, memory_size)), recommended_ps);

    #ifdef DEBUG
      fprintf (stdout, ".2 rootvg = %d,  memory size = %d,  recommended ps = \
%d\ndefault_ps = %d\n",
	 root_percent, memory_size, recommended_ps, default_ps);
    #endif


    /* Round the delta page space size down to whole partitions. Ensure
     * that at least two partitions of free space are left unused.
     */
    delta_ps = default_ps - current_ps;

    if (delta_ps > 0)
    {
      delta_ps = delta_ps - (delta_ps % pp_size);
      free_space = free_space - 2 * pp_size;
    #ifdef DEBUG
      fprintf (stdout, "delta ps = %d,  free space - 2 = %d\n", delta_ps,
	 free_space);
    #endif
      delta_ps = MIN (delta_ps, free_space);
    #ifdef DEBUG
      fprintf (stdout, "delta ps = %d\n", delta_ps);
    #endif
    }

    if (delta_ps > 0)
    {
      default_ps = current_ps + delta_ps;
    }
    else
    {
      default_ps = current_ps ;
    }
    #ifdef DEBUG
      fprintf (stdout, "default ps = %d,  current ps = %d\n", default_ps,
	 current_ps);
    #endif
} /* end if calculating default_ps */

/* Return recommended_ps and default_ps.
 */
if (strcmp(argv[1], "recommended_ps") == 0)
{
  fprintf(stdout, "%d\n", recommended_ps);
}
else
{
  if (strcmp(argv[1], "default_ps") == 0)
  {
    fprintf(stdout, "%d\n", default_ps);
  }
  else
  {
    exit(1);
  }
}

exit(0);

}

