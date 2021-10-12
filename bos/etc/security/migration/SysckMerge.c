static char sccsid[] = "@(#)06	1.2  src/bos/etc/security/migration/SysckMerge.c, cfgsauth, bos41J, 9518A_all 4/26/95 14:50:16";
/*
 *   COMPONENT_NAME: cfgsauth
 *
 *   FUNCTIONS: blankstrip
 *		main
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <usersec.h>

#define MAXTCBSIZ 	8192
#define MERGE_FILE	"/tmp/bos/etc/security/Sysck.Merge.File"


/*****************************************************************************
 * NAME: SysckMerge
 *
 * USAGE:
 *	SysckMerge <old sysck.cfg> <new sysck.cfg>
 * 
 *  ex. SysckMerge /sysck.cfg.325 /etc/security/sysck.cfg.411
 *
 * FUNCTION: Merge the previous system's sysck.cfg with the correct
 *	     corresponding entry from the newly installed system's
 *	     sysck.cfg file into a merge file.
 *
 * EXECUTION ENVIRONMENT:
 *      User process
 *
 * NOTES:
 *      Reads the previous sysck.cfg and checks each stanza against the
 *	sysck.cfg from the newly installed system.  If the stanza exists
 *	in the new sysck.cfg, add it to the newly created "merged_file".
 *	The merge file then gets copied over the previous sysck.cfg.
 *
 * RETURNS: Zero on success, non-zero otherwise
 *
 *****************************************************************************/

int
main (int argc, char *argv[])
{
	int     errors=0;             /* Any errors found                    */
	int     rc=0;                 /* Return code temporary variable      */

	FILE    *prevf;               /* Previous version's sysck.cfg file   */
	FILE    *newf;		      /* New version's sysck.cfg file        */
	FILE    *mergef;              /* Merged /etc/security/sysck.cfg file */
	FILE    *combinef;            /* Combine file pointer into merged    */
				      /* sysck file 			     */

	char    cur_stz[LINE_MAX];    /* Current stanza in old sysck.cfg     */
	char    line[LINE_MAX];       /* Temp place to read a line           */
	char    new_stz[MAXTCBSIZ];   /* Stanza from new sysck.cfg           */

	char    *p;                   /* Temp string pointer                 */

	struct	stat	buffer;       /* buffer for stat of previous object  */
	char	*pointer_to_start;    /* pointer to colon of file name	     */
	char	*pointer_to_colon;    /* pointer to colon of file name	     */
	char	filename[LINE_MAX];   /* pointer to colon of file name	     */


	if (argc != 3) {
		fprintf(stderr,"Usage: SysckMerge <old sysck.cfg> <new sysck.cfg>\n");
		exit (1);
	}

	/*
	 * First, lock out other procs from doing put*attr's
	 */

	if (setuserdb (S_WRITE))
		exit (1);

	/*
	 * Open old sysck.cfg for reading.
	 */
	if ((prevf=fopen(argv[1],"r+")) == NULL) {
		fprintf(stderr,"SysckMerge: No such file: %s.\n",argv[1]);
		errors++;
		exit (1);
	}
	/*
	 * Guarantee that there is at least one blank line at the 
	 * end of the file for the last stanza.
	 */
	fseek(prevf,0L,SEEK_END);
	fwrite("\n",1,1,prevf);
	fseek(prevf,0L,SEEK_SET);

	/*
	 * Open the new sysck file for reading.
	 */
	 if ((newf=fopen(argv[2],"r+")) == NULL) {
		  fprintf(stderr,"SysckMerge: No such file: %s.\n",argv[2]);
		  errors++;
		  exit (1);
	 }
	/*
	 * Guarantee that there is at least one blank line at the 
	 * end of the file for the last stanza.
	 */
	fseek(newf,0L,SEEK_END);
	fwrite("\n",1,1,newf);
	fseek(newf,0L,SEEK_SET);

	/*
	 * Create the new merge file to save sysck.cfg stanzas.
	 */
	if ((mergef=fopen(MERGE_FILE,"w")) == NULL) {
		fprintf(stderr,"SysckMerge: Create failed: %s.\n",MERGE_FILE);
		errors++;
		exit (1);
	}

	/*
	 * Read the entire previous sysck.cfg file.  If we find a match,
	 * copy the new sysck.cfg file's stanza into the merge file.  If
	 * there is no match, copy the previous sysck.cfg file's stanza
	 * into the merge file.
	 */
	while (fgets(cur_stz,LINE_MAX-1,prevf)) {
		blankstrip(cur_stz);
		/*
		 * A '/' means we have the start of a stanza. If
		 * we don't have one, just keep going to the next
		 * line.
		 */
		if (*cur_stz!='/')
			continue;
		/*
		 * Must be at the start of a stanza.
		 * Go back to the beginning of the new
		 * sysck file to start search.
		 */
		fseek(newf,0L,SEEK_SET);

		/*
		 * Scan new sysck file for stanza name matching the
		 * current one we found in the previous file (prevf)
		 */
		*new_stz='\0';
		while(fgets(line,LINE_MAX,newf)) {
			blankstrip(line);
			if (*line!='/')
				continue;
			if (!strcmp(line,cur_stz)) {
				/*
				 * We have a match!
				 */
				strcpy(new_stz,line);
				while(fgets(line,LINE_MAX,newf)) {
					/*
					 * Collect whole stanza
					 */
					strcat(new_stz,line);

					/*
					 * Check for end of stanza (a
					 * blank line)
					 */
					p=line;
					while (isspace(*p))
						p++;
					if (!*p) {
						fwrite(new_stz,
						       strlen(new_stz), 1,
						       mergef);
						break;
					}
				}
			}
		}
		if (!*new_stz) {
			/*
			 * We didn't find a match, so we need to append
			 * the previous sysck.cfg stanza onto the new
			 * merge file if the object exists on the system.
			 * If the object does not exist, then the name
			 * is no longer valid.  This could mean that
			 * a name change occured, the location is now
			 * different, the object was moved into another
			 * program, etc.
			 */

			*filename = '\0';
			pointer_to_start = cur_stz;

			/*
			 * Make sure we have a colon at the end.  If not,
			 * we have a corrupt stanza.
			 */
			if ((pointer_to_colon = 
			     (char *) strchr(cur_stz, ':')) != NULL) {

				/*
				 * The difference in the pointer to the
				 * start of the string and the pointer to
				 * the colon is how many bytes to copy into
				 * the temporary filename storage.
				 */
				strncat(filename,cur_stz,
					pointer_to_colon - pointer_to_start);

				if (!stat(filename, &buffer)) {

					strcpy(new_stz,cur_stz);
					while(fgets(line,LINE_MAX,prevf)) {
						/*
 		   				 * Collect whole stanza
 		   				 */
						strcat(new_stz,line);

						/*
		  				 * Check for end of stanza
		  				 * (a blank line)
		  				 */
						p=line;
						while (isspace(*p))
							p++;

						if (!*p) {
							fwrite (new_stz,
								strlen(new_stz),
								1, mergef);
							break;
						}
					}
				} 
			}
			/*
			 * Bad stanza.
			 */
			else
				fprintf(stderr,"SysckMerge: Bad stanza skipped: %s\n",cur_stz);
		}
	}

	fclose(mergef);
	fclose(newf);
	fclose(prevf);

	/*********************************************************
	 * Now read the new sysck.cfg and add all stanzas not    *
	 * already in current merge file to the merge file. 	 *
	 *********************************************************/

	/*
	 * Open the newly installed system's sysck.cfg file for reading,
	 * the merged sysck.cfg file for reading, and the merged sysck.cfg
	 * file again for writing to save new sysck.cfg stanzas.
	 */
	if ((newf=fopen(argv[2],"r")) == NULL) {
		fprintf(stderr,"SysckMerge: Open failed: %s.\n",argv[2]);
		errors++;
		exit (1);
	}

	if ((combinef=fopen(MERGE_FILE,"a")) == NULL) {
		fprintf(stderr,"SysckMerge: Open failed: %s.\n",MERGE_FILE);
		errors++;
		exit (1);
	}

	if ((mergef=fopen(MERGE_FILE,"r")) == NULL) {
		fprintf(stderr,"SysckMerge: Open failed: %s.\n",MERGE_FILE);
		errors++;
		exit (1);
	}


	/*
	 * Read the newly installed system's sysck.cfg file.   We've
	 * already added all of the stanza's for entries that were 
	 * contained in the previous version's sysck.cfg, so we now
	 * only need to add those stanzas that appear in the newly
	 * installed system's sysck.cfg file that doesn't already
	 * appear in the merged file.
	 */
top:
	while (fgets(cur_stz,LINE_MAX-1,newf)) {
		blankstrip(cur_stz);
		/*
		 * A '/' means we have the start of a stanza. If
		 * we don't have one, just keep going to the next
		 * line.
		 */
		if (*cur_stz!='/')
			continue;
		/*
		 * Must be at the start of a stanza.
		 * Go back to the beginning of the merged
		 * sysck file to start search
		 */
		fseek(mergef,0L,SEEK_SET);

		/*
		 * Scan merged sysck file for stanza name matching the
		 * current one we found in the new file (newf).
		 * If we find a match, skip it since we've already
		 * put that into the merged file, otherwise, add this
 		 * entry to the merge file.  The "goto" is put here not
		 * only for speed, but for simplicity.  If the stanza
		 * already exists in merge file, then we want to break
		 * out of the search and get another stanza from the
		 * newly installed systems's sysck.cfg file.  Since we
		 * have to search the entire merge file for a matching
		 * entry, the "goto" can be used in this case very easily.
 		 */
		*new_stz='\0';
		while(fgets(line,LINE_MAX,mergef)) {

			blankstrip(line);
			if (*line!='/')
				continue;

			if (!strcmp(line,cur_stz))
				goto top;
		}
		/*
		 * We've checked the entire merge file for
		 * this entry and it doesn't exist, so we
		 * can append it now.
		 */
		strcpy(new_stz,cur_stz);
		while(fgets(line,LINE_MAX,newf)) {
			/*
 			 * Collect whole stanza
 			 */
			strcat(new_stz,line);

			/*
			 * Check for end of stanza (a
			 * blank line)
			 */
			p=line;
			while (isspace(*p))
				p++;

			if (!*p) {
				fwrite(new_stz,strlen(new_stz),
				       1, combinef);
				break;
			}
		}
	}

	/*
	 * Now we have a complete merged sysck.cfg, so
	 * we'll copy it to /etc/security/sysck.cfg with
	 * the correct permissions for the new install.
	 */
	
	if (rename(MERGE_FILE,argv[1])) {
		fprintf(stderr,"SysckMerge: Can't rename %s to %s.\n",
			MERGE_FILE,argv[1]);
		errors++;
		exit (1);
	}
	
	/*
	 * Close up and exit
	 */

	fclose(mergef);
	fclose(newf);
	fclose(combinef);

	enduserdb();

	exit (errors ? 1 : 0);
}



/*****************************************************************************
 * NAME: blankstrip
 *
 * FUNCTION: strip all whitespace out of a string
 *
 * EXECUTION ENVIRONMENT:
 *      User process
 *
 * RETURNS: Nothing
 *
 *****************************************************************************/
blankstrip(char *s)
{

char    *curp=s;		/* Pointer to where to put next non-blank */

	if (!s || !*s)		/* Null pointer or string, return.        */
		return;

	while (*s) {

		if (!(*s==' '||*s=='\t'))  /* If it isn't whitespace        */
			*curp++=*s;	   /* Move it to p and increment p  */
		s++;			   /* In any case, increment s      */
	}
	*curp='\0';
}
