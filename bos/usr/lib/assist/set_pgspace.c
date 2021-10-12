static char sccsid[] = "@(#)12  1.8  src/bos/usr/lib/assist/set_pgspace.c, cmdassist, bos41J, 9521A_all 5/23/95 14:59:56";
/*
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
 * NAME: set_pgspace
 *
 * FUNCTION:
 *      This program will be called to allocate addition page space based on the
 *    total page space size requested. This routine will calculate how the page
 *    space is spread among the disks in the root volume group.
 *      The paging space increase will be distributed evenly amongst all the disks
 *    on the system.  If a disk does not have a paging space on it, one will be
 *    created.  If a disk does not have enough room for it's portion, it will be
 *    filled to the max and the remaining paging space will be distributed to the
 *    other disks.  If a disk does not have any room, it's portion will be 
 *    distributed among the remaining disks.
 *
 * EXECUTION ENVIRONMENT:
 *    Executes under install_assist environment.
 *
 * INPUT VALUES:
 *    The total page space size requested (argv[1]).  This value is in
 *    megabytes.
 *
 * OUTPUT VALUES:
 *    None.
 *
 * RETURNS:
 *    0       Successful completion.
 *    1       Error.
 */

#include <stdio.h>
#include <string.h>
#include <lvm.h>
#include <nl_types.h>
#include <cmdassist_msg.h>

#define MSGSTR(Set,Num,Str) catgets(catd,Set,Num,Str)
#define CATOPEN() catd = catopen(MF_CMDASSIST,0)
#define CATCLOSE() catclose(catd)


/*
 *  Default messages for the possible page space errors.  These
 *  must match the messages in the cmdassist.msg file.  These defines
 *  have been named by placing "_D" after the define name of the
 *  of the message number of the corresponding message in cmdassist.msg.
 */

#define ASSIST_PGSP_DECREASE_E_D "\
0851-009 install_assist:  You have requested a paging space \n\
size that is less than the current size.  The paging space \n\
size cannot be decreased from this menu.\n"

#define ASSIST_FREE_SPACE_E_D "\
0851-010 install_assist:  You have requested a paging space \n\
size that requires more free space than you have available \n\
in rootvg (the system volume group).\n\n\
If you have available disks that are not yet allocated, you \n\
can add another disk to the rootvg volume group.  \n\
In order to show the disks in rootvg or to add disks from \n\
the available disks, use the Add/Show Disks for System \n\
Storage (rootvg) menu.\n\n\
If you do not choose to add another disk to rootvg, then \n\
the value entered for the new paging space size cannot be \n\
greater than the current paging space size plus the space \n\
available in rootvg.\n"

#define ASSIST_PGSP_UNKNOWN_E_D "\
0851-011 install_assist:  An unexplained error occurred \n\
while attempting to increase the paging space.\n"

#define ASSIST_PGSP_SUCCESS_I_D "\
install_assist:  The paging space was successfully increased \n\
to %d megabytes.\n"

#define ASSIST_PGSP_DEFAULT_E_D "\
0851-012 install_assist:  An unexplained error occurred \n\
while attempting to increase the paging space to the \n\
default size.\n"


#define MAXLINE 1024
#define MIN(a, b)  ((a-b)?b:a)
#define MAX(a, b)  ((a-b)?a:b)
#define COLON   0x3a


main(int argc, char **argv)
{

static nl_catd catd;

int new_ps, new_ps_in_MB;
int pp_size, free_pps;
int current_ps = 0;
int add_ps_this_disk;
int delta_ps;
int pv_num, num_pvs, remaining_pvs;

char *string;
char colon;
char line[MAXLINE], command[MAXLINE + 10], tmp_file[256], temp_name[256];
int i, j, temp_size, rc, found = 0;
FILE *fp, *fp_temp;

char * ps_name;
char * pv_name;
char * vg_name;
char * ps_size;
char * colon_ptr;
	
struct pvinfo
{
   char   pgspace [256];
   char   pv [256];
   int    free_pps;
} disk [LVM_MAXPVS];



/* Get the input.
 */
new_ps_in_MB = atoi(argv[1]);


/* Zero out the disk array.
 */
bzero (disk, sizeof (disk));


/* Open the message catalog.
 */
CATOPEN();


/* ---------------------------------------------------------
 * Get the partition size, free physical partitions, and the
 * number of disks in rootvg
 * ---------------------------------------------------------
 */

sprintf(command, "lsvg rootvg");
if ( (fp = popen(command, "r")) == NULL)
{
#ifdef DEBUG
  fprintf (stdout, "Command = %s,  fp = %x\n", command, fp);
#endif
  /* Display message, unknown error while trying to increase page space.
   */
  fprintf (stderr, MSGSTR (ASSIST_ERR_SET, ASSIST_PGSP_UNKNOWN_E,
     ASSIST_PGSP_UNKNOWN_E_D));
  CATCLOSE ();
  exit (1);
}
while ((fgets(line, MAXLINE, fp)) != NULL)
{
  string = strstr(line, "PP SIZE:");
  if (string != 0)
  {
    pp_size=atoi(strtok(string, "PP SIZE:"));
  }
  string = strstr(line, "FREE PPs:");
  if (string != 0)
  {
    free_pps=atoi(strtok(string, "FREE PPs:"));
  }
  string = strstr(line, "TOTAL PVs:");
  if (string != 0)
  {
    num_pvs = atoi(strtok(string, "TOTAL PVs:"));
  }
}
(void) pclose(fp);

#ifdef DEBUG
    fprintf (stdout, "pp_size = %d, free_pps = %d, num_pvs = %d\n",
       pp_size, free_pps, num_pvs);
#endif


/* Calculate new size of page space in number of partitions.  If
 * requested size is not a multiple of partition size, then value
 * will be rounded down.
 */

new_ps = new_ps_in_MB / pp_size;
#ifdef DEBUG
  fprintf (stdout, "new ps in MB = %d,  new ps in PPs = %d\n",
     new_ps_in_MB, new_ps);
#endif


   
/* ----------------------------------------------------------------
 * Sort Paging spaces for disks:
 *
 *   Disks are ordered from largest free space to small free space.
 * Array entry 0 will have the name of the disk with the most
 * free space.
 *   Paging LVs will be allocated from smallest free space first
 * (last disk in array is first to be used). This means that if the
 * page space size for a disk must be adjusted downward because of
 * free space on the disk, then the size being spilled over to the
 * remaining disks will always be targeted to the larger free space
 * ----------------------------------------------------------------
 */

pv_num = 0;
sprintf(command, "lsvg -p rootvg");
if ( (fp = popen(command, "r")) == NULL)
{
#ifdef DEBUG
  fprintf (stdout, "Command = %s,  fp = %x\n", command, fp);
#endif
  /* Display message, unknown error while trying to increase page space.
   */
  fprintf (stderr, MSGSTR (ASSIST_ERR_SET, ASSIST_PGSP_UNKNOWN_E,
     ASSIST_PGSP_UNKNOWN_E_D));
  CATCLOSE ();
  exit (1);
}
if ((fgets(line, MAXLINE, fp)) != NULL)   /* skip VG name */
{
  (void) fgets(line, MAXLINE, fp);   /* skip title line */
  while ((fgets(line, MAXLINE, fp)) != NULL)
  {
    string=strtok(line, "\t\n ");
    (void) strcpy(disk[pv_num].pv, string);
    (void) strtok((char *)0, "\t\n ");
    (void) strtok((char *)0, "\t\n ");
    string=strtok((char *)0, "\t\n ");
    disk[pv_num].free_pps =  atoi(string);;
#ifdef DEBUG
    fprintf (stdout, "disk = %s,  free_pps = %d\n",
       disk[pv_num].pv, disk[pv_num].free_pps);
#endif
    pv_num = pv_num + 1;
  }
}

/* sort disk array */
for ( i = 0; i < num_pvs; i++)
{
  for (j = i+1; j < num_pvs; j++)
  {
    if (disk[i].free_pps < disk[j].free_pps)
    {
      temp_size = disk[i].free_pps;
      disk[i].free_pps = disk[j].free_pps;
      disk[j].free_pps = temp_size;

      strcpy(temp_name, disk[i].pv);
      strcpy(disk[i].pv, disk[j].pv);
      strcpy(disk[j].pv, temp_name);
    }
  }
}


/* ------------------------------------------------------------------
 * Copy the names of the paging spaces into the Disk array.
 * ------------------------------------------------------------------
 */
current_ps = 0;

sprintf(command, "lsps -ct lv");
if ( (fp = popen(command, "r")) == NULL)
{
#ifdef DEBUG
  fprintf (stdout, "Command = %s,  fp = %x\n", command, fp);
#endif
  /* Display message, unknown error while trying to increase page space.
   */
  fprintf (stderr, MSGSTR (ASSIST_ERR_SET, ASSIST_PGSP_UNKNOWN_E,
     ASSIST_PGSP_UNKNOWN_E_D));
  CATCLOSE ();
  exit (1);
}
if ((fgets(line, MAXLINE, fp)) != NULL)   /* skip title line */
{
  while ((fgets(line, MAXLINE, fp)) != NULL)
  {
    string =  & line[0];  
    colon = COLON;

    colon_ptr = strchr (string, colon);
    ps_name = string;
    *colon_ptr = NULL;
    string = colon_ptr + 1;

    colon_ptr = strchr (string, colon);
    pv_name = string;
    *colon_ptr = NULL;
    string = colon_ptr + 1;

    colon_ptr = strchr (string, colon);
    vg_name = string;
    *colon_ptr = NULL;
    string = colon_ptr + 1;

    colon_ptr = strchr (string, colon);
    ps_size = string;
    *colon_ptr = NULL;
    string = colon_ptr + 1;

    current_ps = current_ps + atoi(ps_size);

#ifdef DEBUG
    fprintf (stdout, "%s %s %s %s %d\n", ps_name, pv_name,
       vg_name, ps_size, current_ps);
#endif

    if ((strcmp (vg_name, "rootvg") == 0))
    {
      /* Figure out the NUMBER of the disk this paging space is on. */
      for (i = 0; i < num_pvs; i = i+1)
      {
	if (strcmp(pv_name, disk[i].pv) == 0)
	  break;
      }
      pv_num = i;

      /* If there is not already a paging space name for this then
       * copy the name of this paging space into the disk array.
       */
      if (disk[pv_num].pgspace[0] == NULL)
      {
	  (void) strcpy (disk[pv_num].pgspace, ps_name);
#ifdef DEBUG
	  fprintf (stdout, "disk = %s,  pgsp = %s\n",
	     disk[pv_num].pv, disk[pv_num].pgspace);
#endif
      } /* end if there is no name for this disk */
    } /* end if in rootvg */
  }
}
(void) pclose(fp);

#ifdef DEBUG
for ( i = 0; i < num_pvs; i++)
{
  fprintf (stdout, "pgsp = %s,   disk = %s,  free = %d\n",
     disk[i].pgspace, disk[i].pv, disk[i].free_pps);
}
#endif


/* ---------------------------------
 * Begin adjusting paging space size
 * ---------------------------------
 */
delta_ps = new_ps - current_ps;
#ifdef DEBUG
fprintf (stdout, "delta_ps = %d\n", delta_ps);
#endif

if (delta_ps < 0)
{
  /* Display message that the page space cannot be decreased.
   */
  fprintf (stderr, MSGSTR (ASSIST_ERR_SET, ASSIST_PGSP_DECREASE_E,
     ASSIST_PGSP_DECREASE_E_D));
  CATCLOSE ();
}
else
{
  if (delta_ps > free_pps)
  {
    /* Display message that there is not enough free space.
     */
    fprintf (stderr, MSGSTR (ASSIST_ERR_SET, ASSIST_FREE_SPACE_E,
       ASSIST_FREE_SPACE_E_D));
    CATCLOSE ();
    exit (1);
  }
  else
  {
    if (delta_ps > 0)
    {
      remaining_pvs = num_pvs;

      /* Get size of new page space in megabytes.
       */
      new_ps = current_ps + delta_ps;
      new_ps = new_ps * pp_size;

      /* Set pv_num to last disk in array
       * so we can work backwards from the disk with least
       * free space. (Subtract one because array starts at 0)
       */
      pv_num=num_pvs-1;

      while (pv_num >= 0)
      {
	add_ps_this_disk = delta_ps / remaining_pvs;

#ifdef DEBUG
	fprintf (stdout, "delta ps = %d,  remaining pvs = %d\n",
	   delta_ps, remaining_pvs);
	fprintf (stdout, "disk = %s,  ps size = %d,  free space = %d\n",
	   disk[pv_num].pv, add_ps_this_disk, disk[pv_num].free_pps);
#endif
	if (add_ps_this_disk > disk[pv_num].free_pps)
	   add_ps_this_disk = disk[pv_num].free_pps;

	if (add_ps_this_disk > 0)
	{

#ifdef DEBUG
	  fprintf (stdout, "disk = %s, disk.ps = %s\n",
	    disk[pv_num].pv, disk[pv_num].pgspace);
#endif
	  if (disk[pv_num].pgspace[0] != NULL)
	  {
	    /* Check to see if the number of physical partitions
	     * being added is greater than 128.  128 is the 
	     * default maximum number of physical partitions for
	     * a logical volume.  Increase the max to the number
	     * is being requested.
	     */
	    if (add_ps_this_disk > 128) {
	       sprintf(command, "chlv -x%d %s ", add_ps_this_disk,
	       disk[pv_num].pgspace);
#ifdef DEBUG
	       fprintf (stdout, "Command = %s\n", command);
#else
	       rc=system(command);
	       if (rc)
	       {
	         /* Display message, unknown error while trying to
	          * increase page space.
	          */
	          fprintf (stderr, MSGSTR (ASSIST_ERR_SET,
		           ASSIST_PGSP_UNKNOWN_E, ASSIST_PGSP_UNKNOWN_E_D));
	          CATCLOSE ();
	          exit(rc);
		}
#endif
	    }

	    sprintf(command, "chps -s%d %s ", add_ps_this_disk,
	       disk[pv_num].pgspace);
#ifdef DEBUG
	    fprintf (stdout, "Command = %s\n", command);
#else
	    rc=system(command);
	    if (rc)
	    {
	      /* Display message, unknown error while trying to
	       * increase page space.
	       */
	      fprintf (stderr, MSGSTR (ASSIST_ERR_SET,
		 ASSIST_PGSP_UNKNOWN_E, ASSIST_PGSP_UNKNOWN_E_D));
	      CATCLOSE ();
	      exit(rc);
	    }
#endif
	  }
	  else
	  {
	    sprintf(command, "mkps -s%d -n -a rootvg %s",
	       add_ps_this_disk, disk[pv_num].pv);
#ifdef DEBUG
	    fprintf (stdout, "Command = %s\n", command);
#else
	    rc=system(command);
	    if (rc)
	    {
	      /* Display message, unknown error while trying to
	       * increase page space.
	       */
	      fprintf (stderr, MSGSTR (ASSIST_ERR_SET,
		 ASSIST_PGSP_UNKNOWN_E, ASSIST_PGSP_UNKNOWN_E_D));
	      CATCLOSE ();
	      exit(rc);
	    }
#endif
	  }

	}
	delta_ps = delta_ps - add_ps_this_disk;
	pv_num = pv_num - 1;
	remaining_pvs = remaining_pvs - 1;
      }

      /* Display message that page space successfully increased.
       */
      fprintf (stdout, MSGSTR (ASSIST_ERR_SET, ASSIST_PGSP_SUCCESS_I,
	 ASSIST_PGSP_SUCCESS_I_D), new_ps);


    }
  }
}

    /* ----------------------------------
     * Edit file /var/adm/sw/__assistinfo
     * ----------------------------------
     */

    /* Defect 178323: If the delta is 0, we want
       to record that the user ran the dialog and
       requested no increase in paging space. That
       way we keep the install assistant from 
       changing the paging space upon exit. So,
       set the new_ps to the current_ps */
    if (delta_ps<=0)
      new_ps=current_ps*pp_size;

    /* Create a temp file  /tmp/setpg$$ */
    sprintf(tmp_file, "/tmp/setpg%d", getpid());
    if ((fp_temp=fopen(tmp_file, "w+")) == 0)
       exit(0);


    /* Check if /var/adm/sw/__assistinfo contains "PAGE_SPACE=".
     */
    fp = fopen("/var/adm/sw/__assistinfo", "r+w");
    while ((fgets(line, MAXLINE, fp)) != NULL)
    {
 	if (strncmp(line, "PAGE_SPACE=", strlen("PAGE_SPACE=")) == 0)
        {
	  sprintf(line, "PAGE_SPACE=%d\n", new_ps);
	  found = 1;
	}
	fputs(line, fp_temp);
    }


    /* Rewind both temp and /var/adm/sw/__assistinfo file.  */
    (void) rewind(fp);
    (void) rewind(fp_temp);


    /* Copy the temp file to /var/adm/sw/__assistinfo.  */
    while ((fgets(line, MAXLINE, fp_temp)) != NULL)
	fputs(line, fp);
    if (!found)
    {
	fseek(fp, MAXLINE-1, 1);
	sprintf(line, "PAGE_SPACE=%d\n", new_ps);
	fputs(line, fp);
    }
    (void) fclose(fp);
    (void) fclose(fp_temp);


    /* Remove the temp file. */
    (void) unlink(tmp_file);



/* Close the message catalog.
 */
CATCLOSE();

exit(0);

}
