#ifndef lint
static char sccsid[] = "@(#)48 1.6 src/bos/usr/lib/methods/cfgpty/check_files.c, cfgmethods, bos411, 9428A410j 6/2/94 07:22:34";
#endif
/*
 * COMPONENT_NAME: (CFGMETHODS) - Creation or updating of special files
 *
 * FUNCTIONS: BSDmasterFilesCheck, BSDslaveFilesCheck, ATTslaveFilesCheck,
 *            check_files, verify_space
 *
 * ORIGINS: 27, 83
 *
 */
/*
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/* For performance purpose, use of ILS macros for isdigit */
#define _ILS_MACROS
#include <stdio.h>
#include <cf.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/stat.h>
#include <sys/devinfo.h>
#include <sys/mode.h>
#include <sys/statfs.h>
#include <dirent.h>
#include <ctype.h>          /* for ILS macros */

#include "cfgdebug.h"
#include "pparms.h"
#include "ttycfg.h"
#include "ptycfg.h"

/* Header file containing pty definition */
#include "spty.h"

/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */
#define END_OF_STRING       '\0'

#define MKNODE_MODE         S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
#define MKDIR_MODE          S_IFDIR|S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH

/*
 * =============================================================================
 *                       LOCAL UTILITY ROUTINES
 * =============================================================================
 */
/*
 * -----------------------------------------------------------------------------
 *                       BSDMASTERFILESCHECK
 * -----------------------------------------------------------------------------
 * 
 * Checks if we found ptyp files.
 * We found if we have the right length and if file name
 * matches a BSD master file name.
 *
 * This function is used only for the scandir subroutine.
 *
 * Return code: Exits with non-zero value if file matches
 * -----------------------------------------------------------------------------
 */
static int BSDmasterFilesCheck(currentFilePtr)
struct dirent * currentFilePtr;
{
	return((currentFilePtr->d_namlen == sizeof(ptyp))
		   && !strncmp(currentFilePtr->d_name, ptyp, sizeof(ptyp)-2));
} /* End static int BSDmasterFilesCheck(...) */


/*
 * -----------------------------------------------------------------------------
 *                       BSDSLAVEFILESCHECK
 * -----------------------------------------------------------------------------
 * 
 * Checks if we found ttyp files.
 * We found if we have the right length and if file name
 * matches a BSD slave file name.
 *
 * This function is used only for the scandir subroutine.
 *
 * Return code: Exits with non-zero value if file matches
 * -----------------------------------------------------------------------------
 */
static int BSDslaveFilesCheck(currentFilePtr)
struct dirent * currentFilePtr;
{
	return((currentFilePtr->d_namlen == sizeof(ttyp))
		   && !strncmp(currentFilePtr->d_name, ttyp, sizeof(ttyp)-2));
} /* End static int BSDslaveFilesCheck(...) */


/*
 * -----------------------------------------------------------------------------
 *                       ATTSLAVEFILESCHECK
 * -----------------------------------------------------------------------------
 * 
 * Checks if we found 'digit' file names in the ATT_SLAVE_DIR directory.
 *
 * This function is used only for the scandir subroutine.
 *
 * Return code: Exits with non-zero value if file matches
 * -----------------------------------------------------------------------------
 */
static int ATTslaveFilesCheck(currentFilePtr)
struct dirent * currentFilePtr;
{
	char * current_char;

	current_char = currentFilePtr->d_name;
	while ((*current_char != END_OF_STRING) && isdigit(*current_char)) {
		current_char++;
	}
	return((*current_char == END_OF_STRING) ? 1 : 0);
} /* End static int ATTslaveFilesCheck(...) */


/*
 * =============================================================================
 *                       CHECK_FILES
 * =============================================================================
 * 
 * This function creates special files in the DEV_DIR directory.
 *
 * ATT and BSD master and slave files are created with
 * different major numbers.
 *
 * For BSD naming, as many files
 *
 *     /dev/pty[pqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ][0..f(hex)]
 *     /dev/tty[pqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ][0..f(hex)]
 *
 *     as the max_ttyp number value (PTY DDS field) are created.
 *
 * For ATT naming, as many files ATT_SLAVE_DIR/<i> files
 * as the max_pts number value (PTY DDS field) are created.
 *
 * Return code:   Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */
int check_files(lname,cusDevPtr, ptyDdsPtr, cloneDevno)
char * lname;                      /* logical name of the device */
struct CuDv *cusDevPtr;            /* customized device object pointer */
struct spty_dds * ptyDdsPtr;
dev_t  cloneDevno;
{
    /* characters appended to file name */
    char seq1[] = "pqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char seq2[] = "0123456789abcdef";

    char sstring[100];        /* search criteria */
    char current_pts[32];     /* Current ATT slave file */
    char for_mknod[32];       /* To create special files */
    char for_mkdir[32];       /* To create directory */

    int    return_code;       /* return codes go here */
    int    how_many;          /* To store how many files are found */
    int    which_file;        /* File number */
    dev_t  current_devno;     /* To create special file */
	struct dirent ** files_array; /* To scan directories */

    /* ========== */
    /* ATT master */
    /* ========== */
	/* Only one minor must have been generated for this clone driver */
	current_devno = makedev(major(cloneDevno), major(ptyDdsPtr->ptc_dev));
	if ((return_code = mk_sp_file(current_devno,
								  ATT_MASTER_FILE,
								  MKNODE_MODE)) != 0) {
		DEBUG_0("mknod: error creating special file for ATT master\n");
		return(return_code);
	}
	else {
		DEBUG_1("check_files: ATT master file '%s' created successfully\n",
				ATT_MASTER_FILE);
	} /* End if ((return_code = mk_sp_file(...)) != 0) */
    
    /* ========= */
    /* ATT slave */
    /* ========= */
	how_many = 0;
    /* Create ATT_SLAVE_DIR directory */
	/* If this directory already exist, we count how many files are found */
	/* and we create these special files. They may already exist */
	/* but not necessary with same major number. */
	/* If too many files exist, we remove files which are no more useful */
    sprintf(for_mkdir, "%s/%s", DEV_DIR, ATT_SLAVE_DIR);
    if (mkdir(for_mkdir, MKDIR_MODE) < 0) {
        if (errno == EEXIST) {
            /* It's not really an error */
            DEBUG_1("check_files: '%s' directory already exists\n",
                    for_mkdir);
			how_many = scandir(for_mkdir, &files_array, ATTslaveFilesCheck, NULL);
		}
        else { /* It's an error */
            DEBUG_2("check_files: '%s' can't be created, errno = %d\n",
                    for_mkdir, errno);
            return(E_MKSPECIAL);
        }
	}
	else {
		DEBUG_1("check_files: '%s' directory is created succesfully\n",
				for_mkdir);
    } /* End if (mkdir(for_mkdir, MKDIR_MODE) < 0) */

    /* Create all ATT slave special files */
	/* Remove (if needed) all existing files which must not be used anymore */
	which_file = 0;
	/* For each file which has been found in the directory */
	while (which_file < how_many) {
		/* If this file is not still needed */
		if (atoi(files_array[which_file]->d_name) >= ptyDdsPtr->max_pts) {
			sprintf(for_mknod, "%s/%s/%s", DEV_DIR, ATT_SLAVE_DIR,
					files_array[which_file]->d_name);
			if (return_code = remove(for_mknod)) {
				DEBUG_1("check_files: Unable to remove the '%s' file\n",
						for_mknod);
				/* No more error is returned. We need not to be blocked. Go ahead ...*/
			}
			else {
				DEBUG_1("check_files: '%s' is removed\n",
						for_mknod);
			}
		}
		else {
			DEBUG_1("check_files: No need to remove the '%s' file\n",
					files_array[which_file]->d_name);
		} /* End if (atoi(files_array[which_file]->d_name) >= ptyDdsPtr->max_pts) */
		which_file++;
	} /* End while (which_file < how_many) */
	/* We need always max_pts files but how_many should be less than max_pts */
	how_many = ptyDdsPtr->max_pts;
	while (--how_many >= 0) {
		current_devno = makedev(major(ptyDdsPtr->pts_dev), how_many);
		sprintf(for_mknod, "%s/%d", ATT_SLAVE_DIR, how_many);
		if ((return_code = mk_sp_file(current_devno, for_mknod,
									  MKNODE_MODE)) != 0) {
			DEBUG_1("check_files: error creating special file for ATT slave '%s'\n",
					for_mknod);
			return(return_code);
		}
		else {
            DEBUG_1("check_files: ATT slave file '%s' created successfully\n",
                    for_mknod);
		} /* End if ((return_code = mk_sp_file(...)) != 0) */
	} /* End while (--how_many) */
	
	
    /* ==================== */
    /* BSD master and slave */
    /* ==================== */
    /* We have the same number for master and slave files. */
	/* We create needed special files. They may already exist */
	/* but not necessary with same major number. */
	/* If too many files exist, we remove files which are no more useful. */
	
	/* Checking for BSD master files */
	how_many = scandir(DEV_DIR, &files_array, BSDmasterFilesCheck, NULL);
	while (how_many > ptyDdsPtr->max_ttyp) {
		/* Don't make decrementation if the 'while' test failed */
		how_many--;
		ptyp[sizeof(ptyp)-3] = seq1[how_many>>4];
		ptyp[sizeof(ptyp)-2] = seq2[how_many&0x0f];
		
		sprintf(for_mknod, "%s/%s", DEV_DIR, ptyp);
		if (return_code = remove(for_mknod)) {
			DEBUG_1("check_files: Unable to remove the '%s' file\n",
					for_mknod);
			/* No more error is returned. We need not to be blocked. Go ahead ...*/
		}
		else {
			DEBUG_1("check_files: '%s' is removed\n",
					for_mknod);
		}
	} /* End while (how_many > ptyDdsPtr->max_ttyp) */
	
	/* Checking for BSD slave files */
	how_many = scandir(DEV_DIR, &files_array, BSDslaveFilesCheck, NULL);
	while (how_many > ptyDdsPtr->max_ttyp) {
		/* Don't make decrementation if the 'while' test failed */
		how_many--;
		ttyp[sizeof(ttyp)-3] = seq1[how_many>>4];
		ttyp[sizeof(ttyp)-2] = seq2[how_many&0x0f];
		
		sprintf(for_mknod, "%s/%s", DEV_DIR, ttyp);
		if (return_code = remove(for_mknod)) {
			DEBUG_1("check_files: Unable to remove the '%s' file\n",
					for_mknod);
			/* No more error is returned. We need not to be blocked. Go ahead ...*/
		}
		else {
			DEBUG_1("check_files: '%s' is removed\n",
					for_mknod);
		}
	} /* End while (how_many > ptyDdsPtr->max_ttyp) */

	how_many = ptyDdsPtr->max_ttyp;
	while (--how_many >= 0) {
		/* Create special file paths */
		ptyp[sizeof(ptyp)-3] = seq1[how_many>>4];
		ptyp[sizeof(ptyp)-2] = seq2[how_many&0x0f];
		
		ttyp[sizeof(ttyp)-3] = seq1[how_many>>4];
		ttyp[sizeof(ttyp)-2] = seq2[how_many&0x0f];
		
		/* Create special BSD master special file */
		current_devno = makedev(major(ptyDdsPtr->ptyp_dev), how_many);
		if ((return_code = mk_sp_file(current_devno, ptyp,
									  MKNODE_MODE)) != 0) {
			DEBUG_1("check_files: error creating special file for BSD master '%s'\n",
					ptyp);
			return(return_code);
		}
		else {
            DEBUG_1("check_files: BSD master file '%s' created successfully\n",
                    ptyp);
		} /* End if ((return_code = mk_sp_file(...)) != 0) */
		
		/* Create special BSD slave special file */
		current_devno = makedev(major(ptyDdsPtr->ttyp_dev), how_many);
		if ((return_code = mk_sp_file(current_devno, ttyp,
									  MKNODE_MODE)) != 0) {
			DEBUG_1("check_files: error creating special file for BSD slave '%s'\n",
					ttyp);
			return(return_code);
		}
		else {
            DEBUG_1("check_files: BSD slave file '%s' created successfully\n",
                    ttyp);
		} /* End if ((return_code = mk_sp_file(...)) != 0) */
		
	} /* End while (how_many--) */
	
    return(0);
} /* End int check_files(...) */

/*
 * =============================================================================
 *                       VERIFY_SPACE
 * =============================================================================
 * 
 * This function checks if there is enough file system space to create
 * needed special files.
 *
 * Return code: Number of free inode when needed special files will be created.
 *              Or -1 if other error occured.
 * =============================================================================
 */
int verify_space(ptyDdsPtr)
struct spty_dds * ptyDdsPtr;
{
struct statfs statbuf;
struct dirent ** files_array; /* To scan directories */
int    how_many;              /* Files number */
char   which_dir[32];         /* To build directory name */

	if (statfs(DEV_DIR, &statbuf) < 0) {
		DEBUG_1("verify_space: Can't stat %s file system\n",
				DEV_DIR);
		return(-1);
	}
	else
	{
		/* Scan ATT_SLAVE_DIR files */
		sprintf(which_dir, "%s/%s", DEV_DIR, ATT_SLAVE_DIR);
		how_many = scandir(which_dir, &files_array, ATTslaveFilesCheck, NULL);
		/* Scan BSD master files */
		how_many += scandir(DEV_DIR, &files_array, BSDmasterFilesCheck, NULL);
		/* Scan BSD slave files */
		how_many += scandir(DEV_DIR, &files_array, BSDslaveFilesCheck, NULL);

		/* Now, it is needed to check if there is enough space when */
		/* all new special files will be created. */
		/* 20 is substracted to be sure not to come to zero */
		/* after all needed files will have been created */
		return(statbuf.f_ffree - 20
			   - (ptyDdsPtr->max_pts + 2*ptyDdsPtr->max_ttyp - how_many));
	}

} /* End int verify_space(...) */
