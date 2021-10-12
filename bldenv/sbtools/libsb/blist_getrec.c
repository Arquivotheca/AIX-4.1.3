static char sccsid[] = "@(#)29  1.1  src/bldenv/sbtools/libsb/blist_getrec.c, bldprocess, bos412, GOLDA411a 4/29/93 12:19:36";
/*
 * Copyright (c) 1990, 1991, 1992  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 */
/* 
 * ODE 2.1.1
 */

#define	BUILD_LIST	"build_list"
#define	KEYWORD		1
#define	CONFIGTIME	2
#define BASEDIR		3

#define FALSE		0
#define TRUE		1
#define ERROR		-1

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <ode/parse_rc_file.h>

/* We hardcode assumptions about the format */
/* of the text, flat file, "builds_list". They are: */

/* 1) Currently, only 3, tab sepatated, fields are supported */
/* <keyword identifier><tab><RCS config time><tab><src relative pathname>*/
/* The RCS config time may be determined for deleted builds by */	
/* finding the latest or oldest date from ALL file versions recorded */
/* in that builds snapshot file (or equivalent) */
/* It is invalid to list a build without at least the first */
/* two fields, keyword and config time. */ 

/* 2) Keywords must be unique, as we search records until the first */
/* match or EOF is encountered. */ 

/* 3) The config time is assumed to be in one of several free formats */
/* that RCS understands (see RCS documentation for this info). */ 


char *build_list_getrec(path_and_file, sbrc_ptr, keyword, field)
struct rcfile *sbrc_ptr;
char *path_and_file, *keyword;
int field;

/* INPUT: */
/* *keyword, pntr to keyword identifying build's record in "build_list" */
/* *path_and_file, optional pntr to "build_list" file other than */
/*                 hardcoded default in /usr/local/builds */ 
/* field, #define integers identifyin filed to retrieve in record */
/*	  KEYWORD = 1, CONFIGTIME = 2, BASEDIR = 3 */
/* OUTPUT: */ 
/* *return_string, ptr to string returned, if nil then no record matched */
/*                     the keyword, OR there was NO string */ 
/*                     in the record's specified field */ 
/* CALL: */
/* the caller declares this routine as: char *build_list_getrec(); */
/* with: char *ptr; */
/* ptr = buid_list_getrec(<arg1>,<arg2>,<arg3>); */ 

{
	struct rcfile contents;		/* structure to initialze */

	char blist_path[MAXPATHLEN];	/* buf for path+filename of build_list */
	char temp[MAXPATHLEN];
	char *path_ptr;		   /* ptr to blist_path contents */

	char recbuf[MAXPATHLEN];   /* buf for read of records from build_list */
	char *rec_ptr;		   /* ptr to recbuf contents */
	char *rec_str_ptr;	   /* ptr to tab sep strngs within recs build_list file */

	char *return_string;	/* string returned to caller */
	char *scratch;		/* misc housekeeping */
	struct stat st;		/* structure for file stat info */
	FILE *bptr;		/* ptr to file build_list */

	int rec_cnt, i;

	temp[0] = '\0';
	scratch = temp;
	if (*path_and_file == NULL) {

            /* must have a keyword to match against field 1 of each record */ 
            if (*keyword == NULL) {
        	fatal("build_list_getrec: bad keyword arg");
        	return(NULL);
	    }
	    /* only 3 fields supported */
	    if (field < KEYWORD || field > BASEDIR) {
		fatal("build_list_getrec: bad field arg");
		return(NULL);
	    }

	    /* User's current rc_file determines path of build_list, */
	    /* UNLESS one passed to us; either way, stat must succeed. */
	    /* get path to "build_list" file from current */
	    /* rc file */

	    if (get_rc_value("build_list", &scratch, sbrc_ptr, TRUE) == -1 )
		fatal("build_list_getrec: can't lookup build list entry for build_base");

	}
	else {
		/* Use arg provided as abs path to build_list file */
		/* call at own risk (stat must succeed) */ 

		scratch = path_and_file;
	}

	strcpy (blist_path, scratch );

	path_ptr = blist_path;
	if (scratch != NULL)
	    *scratch = '\0'; 

	/* stat the builds list file */

	if (stat(path_ptr, &st) < 0) {
	    fatal("build_list_getrec: can't stat builds list file = %s", path_ptr);
	    return(NULL);
	}

	/* open the builds_list file (read only) */ 

	if ((bptr = fopen(path_ptr, "r")) == NULL) {
	    fatal("build_list_getrec: can't open builds list file = %s", path_ptr);
	    return(NULL);
	}
	rec_cnt = 1;
	rec_ptr = recbuf;
	rec_str_ptr = NULL;
	
	while (fgets(recbuf, sizeof(recbuf), bptr) != NULL) {

	/* get string before i=field seperators (tab) */ 
	/* in the current rec */

	    for (i = 0; i < field; i++) { 
		if (i == 0) {
	    	    rec_str_ptr = (char *)(strtok(rec_ptr,"\t")); 
		    if  ((strncmp(rec_str_ptr, keyword, (sizeof(keyword)))) != 0) {
			/* try next record if keyoword doesen't match */
			rec_str_ptr = NULL;
			i = field - 1; 
		    }
		}
		else
		    rec_str_ptr = (char *)(strtok(NULL,"\t"));
	    }

	    /* Once read field past keyword in curent rec, time to leave */ 
	    if (rec_str_ptr != NULL)
		break;

	rec_cnt++;
	}

	/* clean up a string from any field ending in '\n' */

	return_string = rec_str_ptr;
	if (return_string != NULL) {
	    if ((scratch = (char *)(index(return_string,'\n'))) != NULL)
		*scratch = '\0';
	}
	 
	/* Warn if an error occurred while reading */
	/* & close the builds_list file */

	if (ferror(bptr) != NULL) {
	    ewarn("error reading %s", path_ptr);
	    (void) fclose(bptr);
	    return(NULL); }
	else {
	    (void) fclose (bptr);
	    return(return_string);
	}
}
