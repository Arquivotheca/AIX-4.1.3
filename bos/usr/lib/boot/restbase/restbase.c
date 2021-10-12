static char sccsid[] = "@(#)83  1.16.1.5  src/bos/usr/lib/boot/restbase/restbase.c, cmdcfg, bos41B, 412_41B_sync 1/5/95 16:55:58";
/*
 *   COMPONENT_NAME: CMDCFG - restbase 
 *
 *   FUNCTIONS: DATA
 *		do_error
 *		getdata
 *		main
 *		do_error_2
 *		populate_cudvdr
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>


#define DATA(a,b,c)			{ if (verbose) printf(a,b,c); }

#define UNCOMPRESS			"uncompress"
#define TMP_FILE_PACKED			"/tmp/restbase.Z"
#define TMP_FILE			"/tmp/restbase"
#define BUF_SIZE			1024
#define ORIGIN_DISK			"/dev/nvram/base"
#define FATAL_ERROR			1
#define RETRY_ERROR		        2	
#define DUMSTRINGSIZE		        512 

static int verbose;
struct listinfo cudvdr_info;
struct CuDvDr *cudvdr_ptr;


main( argc, argv )
int argc;
char *argv[];

{ 
	char 	*odmdir;
	char 	*origin;
	int  	c;
	extern 	char *optarg;
	extern 	int optind;
	int 	tmp_fd;
	int 	origin_fd;
	struct 	Class *cudv;
        struct 	CuDv CuDv;
	struct 	Class *cuat;
        struct 	CuAt CuAt;
	struct 	Class *cudep;
        struct 	CuDep CuDep;
	struct 	Class *cuvpd;
        struct 	CuVPD CuVPD;
	struct 	Class *cudvdr;
        struct 	CuDvDr CuDvDr;
	int 	cudv_cnt;
	int 	cuat_cnt;
	int 	cudvdr_cnt;
	int 	cudep_cnt;
	int 	cuvpd_cnt;
	FILE 	*fd;
	char 	dummy_str[512];
	int 	total;
	int 	buf_size;
	int 	num;
	int 	rc;
	char 	buf[BUF_SIZE];
	int 	i;


	/* initialize internal variables */
	odmdir = NULL;
	verbose = FALSE;
	origin = ORIGIN_DISK;

	/* parse the command line */
	while ((c = getopt(argc,argv,"vo:d:")) != EOF) {
	    switch (c) {
		case 'v':
		    verbose = TRUE;
		    break;
		case 'o':
		    origin = optarg;
		    break;
		case 'd':
		    odmdir = optarg;
		    break;
		default:
		    do_error("Usage: [-v] [-o origin] [-d destination]\n",0,1);
		    break;
	    }
	}

	if (optind < argc)
		do_error("invalid argument - %s\n",argv[optind],FATAL_ERROR);

	DATA("data originating from '%s'\n",origin,NULL);

	/* open ORIGIN file */
	if ((origin_fd = open(origin,0)) < 0) {
		/* if origin is DEST_DISK, this is a fatal error */
		if ( !strcmp(origin,ORIGIN_DISK)  )
	  	   do_error("unable to open %s\n", origin,FATAL_ERROR);
		else
	  	   do_error("unable to open %s\n", origin,RETRY_ERROR);
	}
			
	/*
	 * Wait until "time" has a non-zero value.   "Time" is used as the
	 * generation number (i_gen) of an inode, thus needs to be a non-zero
         * number.   PTM  47772.
         */
	while (!(time((long *)0)));                      /* p47772 */

	/* init odm */
	odm_initialize();

	if (odmdir) {
		DATA("setting the ODMDIR to '%s'\n",odmdir,NULL);
		/* set the destination path */
		if ((int)odm_set_path( odmdir ) < 0)
		   do_error("unable to odm_set_path : odmerrno = %d\n",
			odmerrno,FATAL_ERROR);
	}

	DATA("creating the temporary file '%s'\n",TMP_FILE_PACKED,NULL);

	/* create a tmp file */
	if ((tmp_fd = creat(TMP_FILE_PACKED,0777)) < 0)
	   do_error("unable to create a temporary file: errno = %d\n",
	             errno,FATAL_ERROR);

	/* read the total */
	if (read(origin_fd,&total,sizeof(int)) < sizeof(int))
	   do_error("unable to read base size: errno = %d\n",errno,FATAL_ERROR);

	/* total should only be the number of compressed bytes, 
           do not include the first 4 bytes which contains the length */

	total = total -4;  

        DATA("total value is '%d'\n",total,NULL);

	/* check the total - if "<0", then error occurred during savebase*/
	if (total <= 0)
	   do_error("base customized is corrupted!! nothing restored\n",
		     0,RETRY_ERROR);

	/* read from ORIGIN and write to the TMP_FILE */
	while (total)
	{  buf_size = (total > BUF_SIZE) ? BUF_SIZE : total;
	   if ((num = read(origin_fd,buf,buf_size)) <= 0) {
		if (num != buf_size)
	      		do_error("unable to read base info: errno = %d\n",
				  errno,RETRY_ERROR);
	       	do_error("unable to read base info: errno = %d\n",errno,
			  FATAL_ERROR);
	   }
	   total = total - num;
	   if (write(tmp_fd,buf,num) < 0)
	      do_error("unable to write to the temp file: errno = %d\n",errno,
			FATAL_ERROR);
	}

	/* close the files */
	close( origin_fd );
	close( tmp_fd );

	/* uncompress the file */
	odm_run_method(UNCOMPRESS,TMP_FILE_PACKED,NULL,NULL);

	/* open TMP_FILE */
	if ((fd = fopen(TMP_FILE,"r")) == NULL)
	{  /* assuming compress failed because it wasn't compressed */
	   DATA("cant open %s; trying to open %s\n",TMP_FILE,TMP_FILE_PACKED);
	   if ((fd = fopen(TMP_FILE_PACKED,"r")) == NULL)
	      do_error("unable to open %s\n",TMP_FILE_PACKED,FATAL_ERROR);
	}


	/* Get object counts for the various object classes */
	if (fread(&cudv_cnt,4,1,fd) < 0)
	   do_error("error %d while reading CuDv count\n",errno,RETRY_ERROR);
	if (fread(&cuat_cnt,4,1,fd) < 0)
	   do_error("error %d while reading CuAt count\n",errno,RETRY_ERROR);
	if (fread(&cudep_cnt,4,1,fd) < 0)
	   do_error("error %d while reading CuDep count\n",errno,RETRY_ERROR);
	if (fread(&cuvpd_cnt,4,1,fd) < 0)
	   do_error("error %d while reading CuVPD count\n",errno,RETRY_ERROR);
	if (fread(&cudvdr_cnt,4,1,fd) < 0)
	   do_error("error %d while reading CuDvDr count\n",errno,RETRY_ERROR);

	/* Process CuDv objects */
	cudv = odm_open_class(CuDv_CLASS);
	if ((int)cudv == -1)
	   do_error("CuDv could not be opened\n", NULL,FATAL_ERROR);

	/* Delete all the CuDv objects */
	if (odm_rm_obj(cudv,"") == -1)
	   do_error("unable to delete objects in CuDv: errno = %d\n",
		     odmerrno,FATAL_ERROR);

	DATA("%d CuDv objects in '%s'\n",cudv_cnt,TMP_FILE);

	/* read the objects & add them to the database */
	for (i=0; i < cudv_cnt ; i++) {

		getdata(CuDv.name,NAMESIZE,fd);
		getdata(dummy_str,DUMSTRINGSIZE,fd); 
		CuDv.chgstatus = strtoul(dummy_str,NULL,16);
		getdata(CuDv.ddins,TYPESIZE,fd);
		getdata(CuDv.location,LOCSIZE,fd);
		getdata(CuDv.parent,NAMESIZE,fd);
		getdata(CuDv.connwhere,LOCSIZE,fd);
		getdata(CuDv.PdDvLn_Lvalue,UNIQUESIZE,fd);

		CuDv.status = DEFINED;
		if (CuDv.chgstatus != DONT_CARE)
			CuDv.chgstatus = MISSING;
		c=getc(fd);
		if ( (i & 0xff) != c ) {
			do_error_2(FALSE);
		}

		DATA("adding '%s' to CuDv\n",CuDv.name,NULL);
		if (odm_add_obj(cudv,&CuDv) < 0)
			do_error("unable to add a CuDv object\n",
			          NULL,FATAL_ERROR);
	} /* for CuDv objects */

	odm_close_class(cudv);

	/* Process CuAt objects */
	cuat = odm_open_class(CuAt_CLASS);
	if ((int)cuat == -1)
	   do_error("CuAt could not be opened\n", NULL,FATAL_ERROR);

	/* Delete all the CuAt objects */
	if (odm_rm_obj(cuat,"") == -1)
	   do_error("unable to delete objects in CuAt: errno = %d\n",
		     odmerrno,FATAL_ERROR);

	DATA("%d CuAt objects in '%s'\n",cuat_cnt,TMP_FILE);

	/* read the objects & add them to the database */
	for (i=0; i < cuat_cnt ; i++) {

		getdata(CuAt.name,NAMESIZE,fd);
		getdata(CuAt.attribute,ATTRNAMESIZE,fd);
		getdata(CuAt.value,ATTRVALSIZE,fd);
		getdata(CuAt.type,FLAGSIZE,fd);
		getdata(CuAt.generic,FLAGSIZE,fd);
		getdata(CuAt.rep,FLAGSIZE,fd);
		getdata(dummy_str,DUMSTRINGSIZE,fd);
		CuAt.nls_index = strtoul(dummy_str,NULL,16);
		c=getc(fd);
		if ( (i & 0xff) != c ) {
			do_error_2(FALSE);
		}

		DATA("adding '%s' attribute for '%s' to CuAt\n",
		      CuAt.attribute,CuAt.name);
		if (odm_add_obj(cuat,&CuAt) < 0)
			do_error("unable to add a CuAt object\n",NULL,
			          FATAL_ERROR);
	} /* for CuAt objects */

	odm_close_class(cuat);

	/* Process CuDep objects */
	cudep = odm_open_class(CuDep_CLASS);
	if ((int)cudep == -1)
	   do_error("CuDep could not be opened\n", NULL,FATAL_ERROR);

	/* Delete all the CuDep objects */
	if (odm_rm_obj(cudep,"") == -1)
	   do_error("unable to delete objects in CuDep: errno = %d\n", 
		     odmerrno,FATAL_ERROR);

	DATA("%d CuDep objects in '%s'\n",cudep_cnt,TMP_FILE);

	/* read the objects & add them to the database */
	for (i=0; i < cudep_cnt ; i++) {

		getdata(CuDep.name,NAMESIZE,fd);
		getdata(CuDep.dependency,NAMESIZE,fd);
		c=getc(fd);
		if ( (i & 0xff) != c ) {
			do_error_2(FALSE);
		}

		DATA("adding dependency for '%s' to CuAt\n",CuDep.name,NULL);
		if (odm_add_obj(cudep,&CuDep) < 0)
			do_error("unable to add a CuDep object\n",NULL,
				  FATAL_ERROR);
	} /* for CuDep objects */

	odm_close_class(cudep);

	/* Process CuVPD objects */
	cuvpd = odm_open_class(CuVPD_CLASS);
	if ((int)cuvpd == -1)
	   do_error("CuVPD could not be opened\n", NULL,FATAL_ERROR);

	/* delete all the objects */
	if (odm_rm_obj(cuvpd,"") == -1)
	   do_error("unable to delete objects in CuVPD: errno = %d\n",
							odmerrno,FATAL_ERROR);

	DATA("%d CuVPD objects in '%s'\n",cuvpd_cnt,TMP_FILE);

	/* read the objects & add them to the database */
	for (i=0; i < cuvpd_cnt ; i++) {

		getdata(CuVPD.name,NAMESIZE,fd);
		getdata(dummy_str,DUMSTRINGSIZE,fd);
		CuVPD.vpd_type = strtoul(dummy_str,NULL,16);
		/* savebase always saves 512 bytes of this field */
		fread(&CuVPD.vpd,512,1,fd); 
		c=getc(fd);
		if ( (i & 0xff) != c ) {
			do_error_2(FALSE);
		}

		DATA("adding VPD for '%s' to CuVPD\n",CuVPD.name,NULL);

		if (odm_add_obj(cuvpd,&CuVPD) < 0)
			do_error("unable to add a CuVPD object\n",
				  NULL,FATAL_ERROR);
	} /* for CuVPD objects */

	odm_close_class(cuvpd);

	/* Process CuDvDr objects */
	cudvdr = odm_open_class(CuDvDr_CLASS);
	if ((int)cudvdr == -1)
	   do_error("CuDvDr could not be opened\n", NULL,FATAL_ERROR);

	/* save current values of CuDvDr objects */ 
	cudvdr_ptr = odm_get_list(CuDvDr_CLASS, "", &cudvdr_info, 11, 1);
	if ((int)cudvdr_ptr == -1 )
	   do_error("unable to obtain objects in CuDvDr: errno = %d\n",
						odmerrno,FATAL_ERROR);

	/* delete all the objects */
	if (odm_rm_obj(cudvdr,"") == -1)
	   do_error("unable to delete objects in CuDvDr: errno = %d\n",
							odmerrno,FATAL_ERROR);

	DATA("%d CuDvDr objects in '%s'\n",cudvdr_cnt,TMP_FILE);

	/* read the objects & add them to the database */
	for (i=0; i < cudvdr_cnt ; i++) {

		getdata(CuDvDr.resource,RESOURCESIZE,fd);
		getdata(CuDvDr.value1,VALUESIZE,fd);
		getdata(CuDvDr.value2,VALUESIZE,fd);
		getdata(CuDvDr.value3,VALUESIZE,fd);
		c=getc(fd);
		if ( (i & 0xff) != c ) {
			do_error_2(TRUE);
		}

		DATA("adding object for '%s' to CuDvDr\n",CuDvDr.resource,NULL);
		if (odm_add_obj(cudvdr,&CuDvDr) < 0)
			do_error("unable to add a CuDvDr object\n",NULL,
				  FATAL_ERROR);

	} /* for CuDvDr objects */

 	/* check for CuDvDr objects, if none, add previously saved objects */	
	if ( !cudvdr_cnt )  
		do_error_2(TRUE);
	 
	odm_free_list (cudvdr_ptr, &cudvdr_info);

	/* all done */
	odm_close_class(cudvdr);

	fclose( fd );
	odm_terminate();

	/* cleanup */
	unlink( TMP_FILE );
	unlink( TMP_FILE_PACKED );

	DATA("all done - bye bye\n",NULL,NULL);

	/* bye bye */
	exit( 0 );
}


/*-------------------------------- do_error ---------------------------------*/

do_error(str1,str2,exit_code)
char *str1;
char *str2;
int exit_code;
{
	if (verbose)
 	{
	   fprintf(stderr,str1,str2);
		fflush(stderr);
	}

	odm_free_list (cudvdr_ptr, &cudvdr_info);
	exit( exit_code );
}


/*-------------------------------- getdata ----------------------------------*/
int
getdata(str,maxcnt,fd)
char *str;
int  maxcnt;
FILE *fd;
{
	int c;
	int count;

	count = 0;
	while ((c = getc(fd)) != EOF && count<=maxcnt) {
		*str = c;
		str++;
		count++;
		if (c == '\0') break;
	}

	return(0);
}
/*-------------------------------- do_error_2-------------------------------
 * NAME: do_error_2
 *
 * FUNCTION: delete all the customized attributes and add the predetermined
 *	     CuDvDr entries.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:  exit program, return 3 to signal to rc.boot different code
 * 	     path  
 *
 *---------------------------------------------------------------------------*/
int
do_error_2(cudvdr_error)
int	cudvdr_error;		/* error occurred during cudvdr processing   */
{

	struct CuDvDr CuDvDr;
	int	i;
	int	rc;

        /* Delete all the CuDv objects */
        if (odm_rm_obj(CuDv_CLASS,"") == -1)
           do_error("unable to delete objects in CuDv: errno = %d\n",
                                                       odmerrno,FATAL_ERROR);
        /* Delete all the CuAt objects */
        if (odm_rm_obj(CuAt_CLASS,"") == -1)
           do_error("unable to delete objects in CuAt: errno = %d\n",
                                                        odmerrno,FATAL_ERROR);
        /* Delete all the CuDep objects */
        if (odm_rm_obj(CuDep_CLASS,"") == -1)
           do_error("unable to delete objects in CuDep: errno = %d\n", 
							odmerrno,FATAL_ERROR);

        /* delete all the CuVPD objects */
        if (odm_rm_obj(CuVPD_CLASS,"") == -1)
           do_error("unable to delete objects in CuVPD: errno = %d\n",
                                                        odmerrno,FATAL_ERROR);
	DATA("do_error_2: deleted CuDv, CuAt, CuDep,and CuVPDs objects\n",
		NULL,NULL);
	if (!cudvdr_error) {
		/* error processing before CuDvDr processing */
		
		/* do not want to delete from CuDvDr because want to 
		   use copy of data that bosboot put in the RAM disk    */

		rc = (int)odm_get_first(CuDvDr_CLASS,"",&CuDvDr);
		if (rc == 0)
			/* none exist in RAM, add 11 predefined objects */
			populate_cudvdr(); 	
		else if (rc == -1)
           		do_error("unable to retrieve objects in CuDvDr:errno = %d\n", odmerrno,FATAL_ERROR);
	}
	else  { /* error processing during CuDvDr processing */
	
       		/* delete all the CuDvDr objects */
        	if (odm_rm_obj(CuDvDr_CLASS,"") == -1)
           		do_error("unable to delete objects in CuDvDr: errno = %d\n", odmerrno,FATAL_ERROR);

		DATA("do_error_2: deleted CuDvDr objects\n",NULL,NULL);
		/* if saved list does not contain any objects, populate  */
		if (cudvdr_info.num == 0) {
			odm_free_list (cudvdr_ptr, &cudvdr_info);
			populate_cudvdr();
		}
		else {
			for ( i= 0; i < cudvdr_info.num; i++)   {
				if (odm_add_obj(CuDvDr_CLASS,cudvdr_ptr)==-1) 
				   do_error ("unable to add CuDvDr - %s object", 					      cudvdr_ptr[i].value1,
					      FATAL_ERROR);
				cudvdr_ptr++;
			}
			DATA("do_error_2: added previously retrieved CuDvDr objects\n",NULL,NULL);
			odm_free_list (cudvdr_ptr, &cudvdr_info);
		}
	}
        /* cleanup */
        unlink( TMP_FILE );
        unlink( TMP_FILE_PACKED );

	exit(RETRY_ERROR);
}

/*-------------------------------- populate_cudvdr-------------------------
 * NAME: populate_cudvdr
 *
 * FUNCTION: Add 11 predetermined customized device driver attributes.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:  0 - success
 *           1 - exit through do_error call
 *
 *---------------------------------------------------------------------------*/
int
populate_cudvdr()
{
	int  	i;
	struct  CuDvDr cudvdr[] = {
   	        {0L, 0L, 0L, "ddins", "sysram",  "0", ";" }, 
 	        {0L, 0L, 0L, "ddins", "systty",  "1", ";" },
  		{0L, 0L, 0L, "ddins", "sysmem",  "2", ";" },
		{0L, 0L, 0L, "ddins", "sysmach", "3", ";" }, 
		{0L, 0L, 0L, "ddins", "syscons", "4", ";"},
		{0L, 0L, 0L, "ddins", "systrace","5", ";" },
		{0L, 0L, 0L, "ddins", "syserror","6", ";" },
		{0L, 0L, 0L, "ddins", "sysdump", "7", ";" },
		{0L, 0L, 0L, "ddins", "sysaudit","8", ";" },
		{0L, 0L, 0L, "ddins", "sysrsvd", "9", ";" },
		{0L, 0L, 0L, "ddins", "rootvg",  "10",";" }
	};
  
	/* add 11 predefined objects that must be present */
		
	for ( i=0; i< 11; i++ )  {
		if (odm_add_obj (CuDvDr_CLASS, &cudvdr[i] ) == -1 )
			do_error ("unable to add CuDvDr - %s object", 
				   cudvdr[i].value1,FATAL_ERROR);
	}
	DATA("populate_cudvdr: added 11 CuDvDr objects\n",NULL,NULL);

	return(0);
}

