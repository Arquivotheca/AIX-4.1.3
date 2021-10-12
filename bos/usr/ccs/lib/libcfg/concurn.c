static char sccsid[] = "@(#)43	1.22  src/bos/usr/ccs/lib/libcfg/concurn.c, libcfg, bos411, 9428A410j 6/22/93 09:31:56";
/*
 *   COMPONENT_NAME: LIBCFG
 *
 *   FUNCTIONS: chkmajor
 *		ck_for_symlinks
 *		create_inst
 *		del_special_files1490
 *		find_first_group1156
 *		free_list
 *		geninst
 *		genmajor
 *		genminor
 *		genseq
 *		get_minor_list
 *		getminor
 *		group_in_use
 *		loadext
 *		lsinst
 *		lstmajor
 *		mcompare
 *		reldevno
 *		relinst
 *		relmajor
 *		reset_cudvdr
 *		rm_major
 *		setup_cudvdr
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <string.h>
#include <cf.h>

#include <sys/errno.h>
#include <sys/cfgodm.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/sysconfig.h>

int        conferrno;
mid_t	   loadext();

#define TRUE            1
#define FALSE           0
#define CRITLEN         80		/* length of the search criteria string */
#define LNKBUFSIZE	512		/* readlink's buffer size */
#define DEVLEN		4		/* strlen of "/dev" */
#define PTYLEN		8		/* strlen of the four pty strings below */
#define PTCSTR		"/dev/ptc"		
#define PTSSTR		"/dev/pts"		
#define PTYSTR		"/dev/pty"		
#define TTYSTR		"/dev/tty"	
#define DEVDIR          "/dev"
#define DDINS           "ddins"
#define DEVNO           "devno"
#define DVDR            "dd"
#define SEQNO           "seqno"
#define DDINSCRITERIA   "resource = 'ddins'"
#define LOCK_PATH       "/dev/cfglock"
int LOCK_ID;

struct symlinkinfo 
{
	char *linked_to;		/* name of the file special_file is
				           symbolically linked to */
	int  unlink_spec;		/* flag to indicate that the file
					   should be unlinked */
	struct special_files *linked_files;
	struct symlinkinfo *next; 	/* pointer to next structure */
};
struct special_files
{
	char *name;			/* name of the symbolically linked file */
	struct special_files *next;	/* pointer to next special_files struct */
};
	
/*---------------------------------------------------------------

  genmajor()

  Description: This routine is one of the designated entry points
  to the Customized Device Driver object class. If there already
  exists a major number for a given device instance, then this
  major number is returned. Otherwise, a new major number will be
  generated. This routine creates an object in the Customized
  Device Driver object class for the major number information.
  To ensure that unique major numbers are generated, this object
  class is locked for the duration of this routine.
  The lowest available major number or the major number that has
  already been allocated is returned if the routine is successful.
  Otherwise, a -1 is returned indicating failure.

  Input: device_driver_instance_name

  Returns: the lowest available major number or the major number
	   which has already been allocated
	   -1 if failed

  Pseudo-code:

  int
  genmajor(device_driver_instance_name)
  char *device_driver_instance_name;

  {
  int major;
  int found = FALSE;
  int found_preexisting = FALSE;

  open CuDvDr with an exclusive lock
  query CuDvDr for resource = ddins
  if query fails
     then unlock CuDvDr
	  return(-1)

  while (more objects returned from query) && (!found_preexisting)
    {
     ** check if major number is already allocated **
     convert major number in character format to integer format
     if value1 == device_driver_instance_name
	then major = major number in value1
	     found_preexisting = TRUE

	else insert major in a buffer
	     get next object

    } end of while

  if (found_preexisting)
     then
	 {
	  unlock CuDvDr
	  return already allocated major number
	 }

  ** get a new major number **
  ** try to find an empty slot **
  for (the number of objects returned)
    {
     found = FALSE;
     for (every major number returned)

	if buffer[i] == i
	   then found = TRUE
		break

     if (!found)
	then break

    } end of for

  add a new object to CuDvDr with the newly generated major number

  unlock CuDvDr
  return the major number in integer form

  } end of genmajor

  ---------------------------------------------------------------*/

/* WARNING - the value of VALUESIZE matches the value specified in the */
/*     com/objclass/cfgodm.cre file */
#define VALUESIZE		20

int
genmajor(device_driver_instance_name)
char *device_driver_instance_name;

{
	register int i;                 /* loop counter */
	int found;                      /* boolean flag */
	int found_preexisting;          /* boolean flag */

	struct listinfo info;        /* pointer to CuDvDr listinfo */
	struct CuDvDr *cddend;          /* end of list of CuDvDr objects */
	struct CuDvDr *new_obj;         /* new object for CuDvDr */
	struct CuDvDr *cudvdr;          /* pointer to a list of CuDvDr objects */
	register struct CuDvDr *travelp;/* traveling pointer */

	/* check the name length - must be less than VALUESIZE */
	if ( strlen(device_driver_instance_name) >= VALUESIZE )
	{	/* error - not enougth room in CuDvDr.value to store this name */
		conferrno = E_NAME;
		return(-1);
	}

	/* set mode so that CuDvDr is not closed by
	   get_CuDvDr_list; the object class is left open */

	if (setup_cudvdr() == -1)
	{
	   reset_cudvdr();
	   return(-1);
	}

	/* query CuDvDr for resource = ddins */

	cudvdr = get_CuDvDr_list(CuDvDr_CLASS, DDINSCRITERIA, &info, 1, 1);

	/* if query failed, routine returns failed */

	if ((int) cudvdr == -1)
	{
	   reset_cudvdr();
	   conferrno = E_ODMGET; 
	   return(-1);
	}

	/* cddend points to the end of the CuDvDr objects
	   that were returned from the query above */

	cddend = cudvdr + info.num;

	/* the for loops will do the following:
	   1. find an empty slot, i.e., the lowest available major
	      number, if one exists,
	   2. find a major number that has already been allocated, or
	   3. if all slots are filled, i is incremented to the next
	      available major number */

	found = FALSE;
	found_preexisting = FALSE;

	for (travelp = cudvdr; travelp < cddend; travelp++)
	{
	    /* _scratch is an area in the CuDvDr structure
	       which we will use to hold the major number as
	       a long integer */

	    travelp->_scratch = atol(travelp->value2);

	    /* for every object returned from the query,
	       check if the major number has already been
	       allocated */

	    if (!strcmp(travelp->value1, device_driver_instance_name))
	    {
	       found_preexisting = TRUE;
	       break;
	    }

	} /* end of for */

	if (found_preexisting)
        {
	   if (reset_cudvdr() == -1)
	      return(-1);
	   else
	      /* return already allocated major number */
	      return(travelp->_scratch);
	}

	for (i = 0; i < info.num; i++)
	{
	    found = FALSE;
	    for (travelp = cudvdr; travelp < cddend; travelp++)
	    {
		if (i == (int) travelp->_scratch)
		{
		   found = TRUE;
		   break;
		}
	    } /* end of for */

	    if (!found)
	       break;

	} /* end of for */

	/* add a new object to CuDvDr with the newly
	   generated major number */

	new_obj = (struct CuDvDr *) malloc(sizeof(struct CuDvDr));
	if (!new_obj)
	{
	   reset_cudvdr();
	   conferrno = E_MALLOC;
	   return(-1);
	}

	strcpy(new_obj->resource, DDINS);
	strcpy(new_obj->value1, device_driver_instance_name);
	sprintf(new_obj->value2, "%d", i);
	new_obj->value3[0] = '\0';

	if (odm_add_obj(CuDvDr_CLASS, new_obj) == -1)
	{
	   reset_cudvdr();
	   conferrno = E_ODMADD; 
	   return(-1);
	}

	free(new_obj);

	if (reset_cudvdr() == -1)
	{
	   return(-1);
	}
	else
	   /* return the major number to the calling routine */
	   return(i);

} /* end of genmajor */


/*---------------------------------------------------------------

  lstmajor()

  Description: TBD

  Input: TBD

  Returns: TBD
	   -1 if failed

  Pseudo-code: TBD

  ---------------------------------------------------------------*/

int *
lstmajor() {
	register int i;                 /* loop counter */
        int *listp;			/* pointer to list of numbers */

	struct listinfo info;        	/* pointer to CuDvDr listinfo */
	struct CuDvDr *cudvdr;          /* pointer to a list of CuDvDr objects */
	register struct CuDvDr *travelp;/* traveling pointer */


	/* set mode so that CuDvDr is not closed by
	   get_CuDvDr_list; the object class is left open */

	if(setup_cudvdr() == -1) {
	  reset_cudvdr();
	  return(-1);
	  }

	/* query CuDvDr for resource = ddins */

	cudvdr = get_CuDvDr_list(CuDvDr_CLASS, DDINSCRITERIA, &info, 1, 1);

	/* if query failed, routine returns failed */

	if ((int) cudvdr == -1)
	{
	   reset_cudvdr();
	   conferrno = E_ODMGET; 
	   return(-1);
	}

        /* allocate space for list */

        listp = (int *)malloc((info.num+1)*sizeof(int));
	if(!listp) {
	  reset_cudvdr();
	  conferrno = E_MALLOC;
	  return(-1);
	  }

	/* compile list of numbers */

	travelp = cudvdr;
	for(i=0;i<info.num;i++) {
	  travelp->_scratch = atol(travelp->value2);
	  listp[i] = (int)travelp->_scratch;
	  travelp++;
	  }
        listp[info.num] = -1;

	if(reset_cudvdr() == -1)
	  return(-1);
	else
	  return(listp);

} /* end of lstmajor */

/*---------------------------------------------------------------

  chkmajor()

  Description: TBD

  Input: device_driver_instance_name, proposed_major_number

  Returns: the proposed major number
	   -1 if failed

  Pseudo-code: TBD

  ---------------------------------------------------------------*/

int
chkmajor(device_driver_instance_name,proposed_major_number)
char *device_driver_instance_name;
int proposed_major_number;

{
	register int i;                 /* loop counter */
	int found;                      /* boolean flag */
	int found_preexisting;          /* boolean flag */

	struct listinfo info;        /* pointer to CuDvDr listinfo */
	struct CuDvDr *cddend;          /* end of list of CuDvDr objects */
	struct CuDvDr *new_obj;         /* new object for CuDvDr */
	struct CuDvDr *cudvdr;          /* pointer to a list of CuDvDr objects */
	register struct CuDvDr *travelp;/* traveling pointer */

	/* set mode so that CuDvDr is not closed by
	   get_CuDvDr_list; the object class is left open */

	if (setup_cudvdr() == -1)
	{
	   reset_cudvdr();
	   return(-1);
	}

	/* query CuDvDr for resource = ddins */

	cudvdr = get_CuDvDr_list(CuDvDr_CLASS, DDINSCRITERIA, &info, 1, 1);

	/* if query failed, routine returns failed */

	if ((int) cudvdr == -1)
	{
	   reset_cudvdr();
	   conferrno = E_ODMGET; 
	   return(-1);
	}

	/* cddend points to the end of the CuDvDr objects
	   that were returned from the query above */

	cddend = cudvdr + info.num;

	found = FALSE;
	found_preexisting = FALSE;

	for (travelp = cudvdr; travelp < cddend; travelp++)
	{
	    /* _scratch is an area in the CuDvDr structure
	       which we will use to hold the major number as
	       a long integer */

	    travelp->_scratch = atol(travelp->value2);

	    /* for every object returned from the query,
	       check if the major number has already been
	       allocated */

	    if (!strcmp(travelp->value1, device_driver_instance_name))
	    {
	       found_preexisting = TRUE;
	       break;
	    }

	} /* end of for */

	if (found_preexisting)
        {
	   if (reset_cudvdr() == -1)
	      return(-1);
	   else if(travelp->_scratch == proposed_major_number)
	      /* return already allocated major number */
	      return(proposed_major_number);
           else
             return(-1);
	}

        /* see if proposed_major_number is already in use */

	for (travelp = cudvdr; travelp < cddend; travelp++)
	  if(proposed_major_number == (int) travelp->_scratch) {
	    reset_cudvdr();
            return(-1);
	    }

	/* add a new object to CuDvDr with the proposed_major_number */

	new_obj = (struct CuDvDr *) malloc(sizeof(struct CuDvDr));
	if (!new_obj)
	{
	   reset_cudvdr();
	   conferrno = E_MALLOC;
	   return(-1);
	}

	strcpy(new_obj->resource, DDINS);
	strcpy(new_obj->value1, device_driver_instance_name);
	sprintf(new_obj->value2, "%d", proposed_major_number);
	new_obj->value3[0] = '\0';

	if (odm_add_obj(CuDvDr_CLASS, new_obj) == -1)
	{
	   reset_cudvdr();
	   conferrno = E_ODMADD; 
	   return(-1);
	}

	free(new_obj);

	if (reset_cudvdr() == -1)
	{
	   return(-1);
	}
	else
	   /* return the major number to the calling routine */
	   return(proposed_major_number);

} /* end of chkmajor */


/*----------------------------------------------------------------

  genseq()

  Description: Genseq generates a unique sequence number to be
  concatenated with the device's prefix name. The device name in
  the Customized Devices object class is the concatenation of the
  prefix name and the sequence number.

  Input: prefix name

  Returns: the generated sequence number
	   -1 if failed


  ----------------------------------------------------------------*/
int
genseq(prefix)
char *prefix;

{
	register int i,j;               /* scratch / loop counters */
	char criteria[CRITLEN];         /* criteria string for query */
	struct CuDv *cudv,*cudvi;	/* CuDv list base and index pointer*/
	struct listinfo cudv_info;	/* listinfo for CuDv */
	int max;			/* strlen of prefix */
	char *ptr;			/* ptr to seqno */
	int seq;			/* seqno */
	int plen,nlen;			/* length of prefix, numeric part */

					/* initialize the criteria */
	sprintf(criteria,"name LIKE %s*",prefix);

					/* get all CuDv's with the prefix */
	cudv = (struct CuDv *)odm_get_list(CuDv_CLASS,criteria,&cudv_info,16,1);

	if ( (int) cudv == -1)
	{
	   conferrno = E_ODMGET; 
	   return(-1);
	}

		 			 /* none there - return 0 */
	if (cudv_info.num == 0)
	   return( 0 );

		/* there are as many elements in the list as the max number
		   of possible sequence numbers for this prefix, so we can
		   use the list to avoid declaring a max array dimension.
		   _scratch of the i'th element indicates that sequence
		   number i is in use */

	for (seq=0; seq < cudv_info.num; seq++)
		cudv[seq]._scratch = FALSE;

	plen = strlen(prefix);	/* loop invariant */
	for (cudvi=cudv,j=0; j<cudv_info.num ; j++,cudvi++) { 

			 /*  get pointer to the part after the prefix */
	   ptr = cudvi->name + plen;
			 /*  get length of the part after the prefix */
	   nlen = strlen(ptr);

			/* make sure everything after the prefix is numerical
			   if not a match to the given prefix - skip. N.B.
			   This rules out hex sequence numbers. */
	   for (i=0; i <nlen ; i++)
		if(  *(ptr+i) < '0' ||
		         *(ptr+i) > '9')break;
	   if(i!= nlen)
		continue;

	   		/* convert ascii decimal seq number to an integer */
	   seq = atoi( ptr );

	 		/* mark the sequence number in table as in use */
	   if (seq < cudv_info.num)
	      cudv[seq]._scratch = TRUE;
	}
			/* search for the first unused seqno */
	  		/* if all numbers were used, seq automatically
			   is the next number */
	for (seq=0; seq < cudv_info.num; seq++)
	  if (!cudv[seq]._scratch)
		break;

	free(cudv);
	return( seq );

} /* end of genseq */

/*----------------------------------------------------------------

  rm_major()

  Description: Given the search criteria, this function removes 
  the object matching the criteria. Removing the object releases
  the major number for reuse. If the object is successfully 
  deleted from CuDvDr, the function returns 0, otherwise -1 is
  returned.
 
  Input: criteria 

  Returns: 0 if successful
	   -1 if failed

  ----------------------------------------------------------------*/

int
rm_major(criteria)
char *criteria;

{
	struct CuDvDr *cudvdr;          /* pointer to a list of CuDvDr objects */
	struct listinfo info;        /* pointer to CuDvDr listinfo */


	cudvdr = get_CuDvDr_list(CuDvDr_CLASS, criteria, &info, 1, 1);

	if ((int) cudvdr == -1)
	{
	   conferrno = E_ODMGET; 
	   return(-1);
	}
	if( !info.num)
	{
	   conferrno = E_RELDEVNO; 
	   return(-1);
	}

	/* if deletion failed, routine returns failed */

	if (odm_rm_obj(CuDvDr_CLASS, criteria) == -1)
	{
	   conferrno = E_ODMDELETE;
	   return(-1);
	}
	else
	   return(0);

} /* end of rm_major */

/*---------------------------------------------------------------

  relmajor()

  Description: This routine is one of the designated entry points
  to the Customized Device Driver object class. For the given
  device driver instance name, the associated major number is
  deleted from the Customized Device Driver object class.

  Input: device_driver_instance_name

  Returns:  0 if successful
	   -1 if failed

  Pseudo-code:

  int
  relmajor(device_driver_instance_name)
  char *device_driver_instance_name;

  {

  open CuDvDr with an exclusive lock
  query CuDvDr for resource = ddins && value1 = device_driver_instance_name
  if query fails
     then unlock CuDvDr
	  return(-1)

  delete object with criteria value1 = device_driver_instance_name
  if deletion fails
     then return_code = -1
     else return_code = 0
  unlock CuDvDr
  return(return_code)

  } end of relmajor

  ----------------------------------------------------------------*/

int
relmajor(device_driver_instance_name)
char *device_driver_instance_name;

{
	int  return_code;               /* return code from rm_major */
	char criteria[CRITLEN];         /* criteria for deleting object */


	/* set mode so that CuDvDr is not closed by
	   get_CuDvDr_list; the object class is left open */

	if (setup_cudvdr() == -1)
	{
	   reset_cudvdr();
	   return(-1);
	}

	/* construct criteria string for odm_rm_obj */

	sprintf(criteria, "resource = '%s' AND value1 = '%s'",
		  DDINS, device_driver_instance_name);

	return_code = rm_major(criteria);

	/* reset the modes to their original values */

	if (reset_cudvdr() == -1)
	{
	   return(-1);
	}
	else
	   return(return_code);

} /* end of relmajor */



/*----------------------------------------------------------------

  setup_cudvdr()

  Description: The routine setup_cudvdr sets up the mode for the
  object class CuDvDr so that the routine get_CuDvDr_list leaves
  the object class open.

  Input: none

  Returns: 0 if successful
           -1 if failed

  ------------------------------------------------------------------*/

setup_cudvdr()
{
	/* set mode so that CuDvDr is not closed by
	   get_CuDvDr_list and open the class exclusively */

	if ((int) odm_open_class(CuDvDr_CLASS) == -1)
	{
	   conferrno = E_ODMOPEN; /* needs sep SETUP err */
	   return(-1);
	}

	return(0);

} /* end of setup_cudvdr */

/*----------------------------------------------------------------

  reset_cudvdr()

  Description: The routine reset_cudvdr resets the modes of the 
  object class CuDvDr to their original values.

  Input: none

  Returns: 0 if successful
           -1 if failed

  ------------------------------------------------------------------*/

reset_cudvdr()
{
	int retval;

	retval = 0;

	if (CuDvDr_CLASS->open && odm_close_class(CuDvDr_CLASS) == -1)
	{
	   conferrno = E_ODMCLOSE;
	   retval = -1;
	}

	return(retval);

} /* end of reset_cudvdr */

/*---------------------------------------------------------------

  mcompare()

  Description: This is the compare function for the qsort routine
  which is used in getminor to sort the minors numbers.

  Input: x 
         y

  Returns: if x < y, return a value less than zero
	   if x = y, return a value of zero 
           if x > y, return a value greater than zero

  ----------------------------------------------------------------*/

static int
mcompare(x, y)
int x, y;

{
	return(x-y);
}

/*---------------------------------------------------------------

  get_minor_list()

  Description: This routine queries CuDvDr for the minor numbers
  currently assigned to the given major number. The number of
  minor numbers found in the query is returned via the parameter
  how_many. The routine itself will return a pointer to a list of
  minor numbers or -1 if none are found. Getminor and genminor
  use this function to query the CuDvDr object class.

  Input: major_no
	 how_many

  Returns: pointer to a list of minor numbers
	   pointer to -1 if failed

  ----------------------------------------------------------------*/

int 
*get_minor_list(major_no, how_many, device_instance)
int  major_no;
int *how_many;
char *device_instance;

{
	register int i;                 /* loop counter */
	char criteria[CRITLEN];         /* criteria string for query */
	int *minors;                    /* pointer to minor numbers */
	int *mindex;                    /* index to minor numbers */

	struct listinfo info;        /* pointer to CuDvDr listinfo */
	struct CuDvDr *cddend;          /* end of list of CuDvDr objects */
	struct CuDvDr *cudvdr;          /* pointer to a list of CuDvDr objects */
	register struct CuDvDr *travelp;/* traveling pointer */


	/* construct search criteria string */

	if (device_instance == NULL)
		{
sprintf(criteria, "resource = '%s' AND value1 = '%d'", DEVNO, major_no);
		}
	else
		{
sprintf(criteria, "resource = '%s' AND value1 = '%d' AND value3 = '%s'", DEVNO, major_no, device_instance);
		}
		

	/* query CuDvDr for resource = devno and value1 = major_no */

	cudvdr = get_CuDvDr_list(CuDvDr_CLASS, criteria, &info, 1, 1);

	/* if query failed, routine returns failed */

	if ((int) cudvdr == -1)
	{
	   conferrno = E_ODMGET; 
	   return(NULL);
	}

	/* no minors were found, a null pointer is returned */
	
	if (!info.num)
	{
	   *how_many = info.num;
	   return(NULL);
	}

	minors = mindex = (int *) malloc(info.num * sizeof(int));

	/* check if malloc failed */

	if (!minors)
	{
	   conferrno = E_MALLOC;
	   return(NULL);
	}

	/* cddend points to the end of the CuDvDr objects
	   were returned from the query above */

	cddend = cudvdr + info.num;

	for (travelp = cudvdr; travelp < cddend; travelp++, mindex++)
	{
	    *mindex = atoi(travelp->value2);

	} /* end of for */

	qsort((char *) minors, info.num, sizeof(int), mcompare);

	*how_many = info.num;
	return(minors);

} /* end of get_minor_list */

/*---------------------------------------------------------------

  getminor()

  Description: This routine queries the Customized Device Driver
  object class for minor number(s) associated with the major num-
  ber. The number of minors found in the query is returned in the
  parameter how_many. The routine returns a pointer a list of
  minor numbers which belong to the given major number. This
  routine is one of the designated entry points to the Customized
  Device Driver object class.

  Input: major_ no
	 how_many

  Returns: pointer to list of minor numbers
	   pointer to -1 if failed

  Pseudo code:

  int 
  *getminor(major_no, how_many)
  int major_no;
  int *how_many;

  {
  open CuDvDr with an exclusive lock
  query CuDvDr for resource = devno && value1 = major_no
  if query fails
     then unlock CuDvDr
	  return pointer to -1

  get list of minor numbers
  unlock CuDvDr
  return pointer to list of minor numbers

  } end of getminor

  ----------------------------------------------------------------*/

int
*getminor(major_no, how_many, device_instance)
int  major_no;
int *how_many;
char *device_instance;
{
	int *minors;                    /* pointer to minor numbers */


	/* set mode so that CuDvDr is not closed by
	   get_CuDvDr_list; the object class is left open */

	if (setup_cudvdr() == -1)
	{
	   reset_cudvdr();
	   return(NULL);
	}

	minors = get_minor_list(major_no, how_many, device_instance);

	if (reset_cudvdr() == -1)
	{
	   return(NULL);
	}
	else
	{
	   return(minors);
	}

} /* end of getminor */

/*----------------------------------------------------------------

  genminor()

  Description: Genminor is one of the designated access routines
  to the Customized Device Driver object class. To ensure unique
  minor numbers are generated, this object class is locked by this
  routine until a unique minor is generated. If a set of minor
  numbers needs to be allocated in one call to genminor, then
  preferred_minor contains a starting minor number. For each minor
  number generated, an object is created in the Customized Device
  Driver object class.
  The minors_in_grp parameter indicates how many minor numbers are
  to be allocated. The inc_within_grp parameter indicates the
  interval between minor numbers. The inc_btwn_grp parameter in-
  dicates the interval between groups of minor numbers. If the set
  of minor numbers is not available, that is, at least one of them
  has already been assigned, the routine uses the inc_btwn_grp
  parameter to try to allocate the next complete available set of
  unused minor numbers. If there is only a single preferred minor
  number which needs to be allocated, then it should be given in
  the preferred_minor parameter. The other parameters should contain
  the integer one (1). If this number is available, it will be
  returned, otherwise, a -1 will be returned indicating that the
  number is in use. If there is no preference, and this is indicated
  by passing a negative one (-1) in the preferred_minor parameter,
  the routine returns the lowest available minor number.

  Input: device_instance_name
	 major_no
	 preferred_minor
	 minors_in_grp
	 inc_within_grp
	 inc_btwn_grp

  Returns: pointer to lowest available minor number or pointer to a
	   list of minor numbers generated or assigned if successful
	   pointer to -1 if failed

  Pseudo-code:

  int
  *genminor(device_instance_name, major_no, preferred_minor,
	    minors_in_grp, inc_within_grp, inc_btwn_grp)

  char *device_instance_name;
  int  major_no;
  int  preferred_minor;
  int  minors_in_grp;
  int  inc_within_grp;
  int  inc_btwn_grp;

  {
  open CuDvDr with an exclusive lock
  get list of minors for the major number
  if no preferred_minor desired
     then find first available minor number
     else if allocating a group of minor numbers
	     then find first group of available minor numbers

  add a new object in CuDvDr for each minor number generated
  unlock CuDvDr
  return pointer to list of minors

  } end of genminor

  ----------------------------------------------------------------*/

int 
*genminor(device_instance_name, major_no, preferred_minor, 
	  minors_in_grp, inc_within_grp, inc_btwn_grp)

char *device_instance_name;
int  major_no;
int  preferred_minor;
int  minors_in_grp;
int  inc_within_grp;
int  inc_btwn_grp;

{
	register int i,j;               /* loop counters */
	int how_many;			/* number of minor nums per this major */
	int begin_minor;		/* index to group of minors */
	int *minors;			/* pointer to list of minors */
	int *mindex;			/* index for list of minors */

	struct listinfo info;        /* pointer to CuDvDr listinfo */
	struct CuDvDr *new_obj;         /* new object for CuDvDr */


	/* set mode so that CuDvDr is not closed by
	   get_CuDvDr_list; the object class is left open */

	if (setup_cudvdr() == -1)
	{
	   reset_cudvdr();
	   return(NULL);
	}

	begin_minor = preferred_minor;

	minors = get_minor_list(major_no, &how_many, NULL);
	/* check conferrno for failures in get_minor_list. */
	if ((conferrno == E_ODMGET) ||
		 (conferrno == E_MALLOC))
	{
	   reset_cudvdr();
	   return(NULL);
	}

	if (preferred_minor == -1)
	{
	   begin_minor = find_first_group(minors, minors_in_grp, inc_within_grp,
                                          inc_btwn_grp, how_many);
	}
	else if (group_in_use(minors, preferred_minor, minors_in_grp, 
                              inc_within_grp, how_many))
	     {
	    	free(minors);
		reset_cudvdr();
		conferrno = E_DEVNO_INUSE;
		return(NULL);
	     }

	/* free space used by minors so that minors can
	   be used again later in this routine */

	if (minors)
	{
	   free(minors);
	}

	/* add a new object to CuDvDr for each minor number */

	new_obj = (struct CuDvDr *) malloc(sizeof(struct CuDvDr));
	if (!new_obj)
	{
	   reset_cudvdr();
	   conferrno = E_MALLOC;
	   return(NULL);
	}

	strcpy(new_obj->resource, DEVNO);
	sprintf(new_obj->value1, "%d", major_no);
	strcpy(new_obj->value3, device_instance_name);

	/* reuse minors - this time to return the list
	   of minor numbers assigned */

	mindex = minors = (int *) malloc(minors_in_grp * sizeof (int));
	if (!minors)
	{
	   reset_cudvdr();
	   conferrno = E_MALLOC;
	   return(NULL);
	}

	/* add the devno objects, at the same time
	   building up the list of assigned minors */

	for (i = begin_minor, j = 0; j < minors_in_grp; i += inc_within_grp, j++)
	{
	    *mindex++ = i;
	    sprintf(new_obj->value2, "%d", i);
	    if (odm_add_obj(CuDvDr_CLASS, new_obj) == -1)
	    {
	       free(minors);
	       reset_cudvdr();
	       conferrno = E_ODMADD; 
	       return(NULL);
	    }
	} /* end of for */

	free(new_obj);
	reset_cudvdr();
	return(minors);

} /* end of genminor */

/*---------------------------------------------------------------

  find_first_group()

  Description: This function finds the first minor number in the 
  first available group.

  Input: minors
	 minors_in_grp
	 inc_within_grp
	 inc_btwn_grp
	 how_many

  Returns: the first minor number in the first available group

  ----------------------------------------------------------------*/

static int
find_first_group(minors, minors_in_grp, inc_within_grp, inc_btwn_grp, how_many)

int *minors;
int minors_in_grp;
int inc_within_grp;
int inc_btwn_grp;
int how_many;

{
	int i;

	i = 0;
	while (group_in_use(minors, i, minors_in_grp, inc_within_grp,
			    how_many))
	{
	      i += inc_btwn_grp;
	}

	return(i);

} /* end of find_first_group */

/*---------------------------------------------------------------

  group_in_use()

  Description: This function returns false if every minor number
  in the group is available. Otherwise, the function will return
  true if one minor number in the group is in use.

  Input: minors
	 begin_minor
	 minors_in_grp
	 inc_within_grp
	 how_many

  Returns: returns TRUE if a number in the desired group is in use
	   returns FALSE if every number in the group is available

  ----------------------------------------------------------------*/

static int
group_in_use(minors, begin_minor, minors_in_grp, inc_within_grp, how_many)

int *minors;
int begin_minor;
int minors_in_grp;
int inc_within_grp;
int how_many;

{
	int i,j;                        /* loop counters */
	int *mindx;                     /* index to minor numbers */
	int *mend;                      /* end of minor number list */

	mend = minors + how_many;

	if (how_many)
	   for (i = begin_minor, j = 0; j < minors_in_grp; i += inc_within_grp, j++)
	   {
	       for (mindx = minors; mindx < mend; mindx++)
	       {
		   if (*mindx == i)
		      return(TRUE);
	       }
	   }

	return(FALSE);

} /* end of group_in_use */

/*---------------------------------------------------------------

  free_list()

  Description: This function frees all the space that was malloc-
  ed for the data structures which held the symbolic link infor-
  mation. This is a cleanup routine for del_special_files.

  Input: head 

  Returns: 

  ----------------------------------------------------------------*/

int 
free_list(head)
struct symlinkinfo *head;

{
	struct symlinkinfo *p;	
	struct symlinkinfo *tp;	
	struct special_files *q;
	struct special_files *tq;


	for (p = head; p != NULL; p = tp)      
	{
	   for (q = p->linked_files; q != NULL; q = tq)
	   {
	      free(q->name);
	      tq = q->next;
	      free(q); 
	   }

	   free(p->linked_to);
	   tp = p->next;
	   free(p);

 	} /* end of for */

} /* end of free_list */
	
/*---------------------------------------------------------------

  ck_for_symlinks()

  Description: This function opens the /dev directory and looks
  at each entry. If the entry has a mode that indicates it is a 
  symbolic link, the name of this file is added as a node in a 
  linked list. This list is passed back to del_special_files so
  that all the special files for a given device instance name
  can be deleted, including those which are symbolically linked.
  The address of the beginning of the list is returned in the 
  variable front.

  Note: Assuming that all special files including those that are
  symbolically linked, are located under /dev.

  Input: front 

  Returns: 0 if the linked list is successfully built
	   -1 if the linked list failed to build to completion

  ----------------------------------------------------------------*/

int
ck_for_symlinks(front, minors, major_no, how_many)
struct symlinkinfo **front;
struct CuDvDr *minors;
int major_no;
int how_many;

{
	char   tmpstr[LNKBUFSIZE];	/* temporary string holder */
	char   lnkbuf[LNKBUFSIZE];	/* buffer for symbolic link info */
 	char   path[CRITLEN];		/* path to file in /dev */
	char   *rc;			/* return value from strrchr */
	int    nbytes;			/* strlen of contents of lnkbuf */
	int    found;			/* file is in the linked list */
	int    unlink_sym;		/* flag to indicate that the file
				           should be unlinked */
	int    minor_no;		/* minor number */
	dev_t  devno;                   /* concatenation of major and minor numbers */
	DIR    *dirp;                   /* pointer to /dev directory */
	struct dirent *dp;              /* pointer to a file in /dev */
	struct stat   stat_buf;         /* stat info on file in /dev */
	struct stat   stat_sym;         /* stat info on file in /dev */

	struct CuDvDr *mindx;           /* index to list of minor numbers */
	struct CuDvDr *mend;            /* pointer to end of minors list */

	struct symlinkinfo *linkinfo;	/* pointer to a new node */
	struct symlinkinfo *current;	/* pointer to the current node in 
					   the list */
	struct special_files *subcurrent;/* pointer to a new "sub" node */


	mend = minors + how_many;
	unlink_sym = found = FALSE;
	*front = NULL;
	dirp = opendir(DEVDIR);

        for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
	{
	    sprintf(path, "/dev/%s", dp->d_name);
	    if (lstat(path, &stat_buf) == -1)
	    {
	       /* ignore this file - keep going */
	       continue;
	    }
	
	    /* check if the file is symbolic link, if so,
	       find its "parent" file and add them to the list */

	    if (S_ISLNK(stat_buf.st_mode))
	    {
 	       if ((nbytes = readlink(path, lnkbuf, LNKBUFSIZE)) == -1)
	       {
	          /* ignore this file - keep going */
	          continue;
	       }
	       lnkbuf[nbytes] = '\0';
	       strncpy(tmpstr, lnkbuf, nbytes);
	       tmpstr[nbytes] = '\0';
	       if (strncmp(tmpstr, path, DEVLEN) != 0)
	       {
	          sprintf(tmpstr, "/dev/%s", lnkbuf);
	       }
 
	       /* pty devices have two special files, ptc and
                  pts which in turn have symbolically linked 
                  files of pty and tty */

	       if (((strncmp(path, PTYSTR, PTYLEN) == 0)   ||
  		    (strncmp(path, TTYSTR, PTYLEN) == 0))  &&
	           ((strncmp(tmpstr, PTCSTR, PTYLEN) == 0) ||	  
	            (strncmp(tmpstr, PTSSTR, PTYLEN) == 0)))	  
	       {
		  if ((rc = strrchr(tmpstr, '/')) != NULL)
		  {
		     *rc = '\0'; 
		  }
	       }

	       if (lstat(tmpstr, &stat_sym) == -1)
	       {
	          /* ignore this file - keep going */
		  continue;
	       }
 
	       /* if the file that this file is symbolically 
                  linked to is to be deleted, then mark the
                  symbolically linked files to be deleted also */

               unlink_sym = FALSE;
	       for (mindx = minors; mindx < mend; mindx++)
	       {
		   minor_no = atoi(mindx->value2);
	 	   devno = makedev(major_no, minor_no);

		   if (stat_sym.st_rdev == devno)
		   {
	              unlink_sym = TRUE;
		      break;
		   } 
               } /* end of for */
              
	       if (*front != NULL)
	       {
	          found = FALSE;
	          for (current = *front; current != NULL; current = current->next)
		  {
		      if (!strcmp(tmpstr, current->linked_to))
		      {
		         found = TRUE;	
		         break;
		      }
	          } /* end of for */

	       } /* end of if front */

	       /* add a new node to the list of files which 
                  are linked to */

	       if (!found)
	       {
	          linkinfo = (struct symlinkinfo *) malloc(sizeof(struct symlinkinfo)); 
	          if (!linkinfo)
	          {
	             closedir(dirp);
	             conferrno = E_MALLOC;
	             return(-1);
	          }
	          linkinfo->linked_to = (char *) malloc((strlen(tmpstr) + 1) * sizeof(char));
	          if (!linkinfo->linked_to)
	          {
	             closedir(dirp);
	             conferrno = E_MALLOC;
	             return(-1);
	          }
		  strcpy(linkinfo->linked_to, tmpstr);
		  linkinfo->unlink_spec = unlink_sym;
		  linkinfo->linked_files = NULL;
		  linkinfo->next = *front;
		  current = *front = linkinfo;
	       }

	       /* make a new "sub" node for the list of       
                  symbolically linked files */
		  
	       subcurrent = (struct special_files *) malloc(sizeof(struct special_files));
	       if (!subcurrent)
	       {
	          closedir(dirp);
	          conferrno = E_MALLOC;
	          return(-1);
	       }
	       subcurrent->next = NULL;
	       subcurrent->name = (char *) malloc((strlen(path) + 1) * sizeof(char));
	       if (!subcurrent->name)
	       {
	          closedir(dirp);
	          conferrno = E_MALLOC;
	          return(-1);
	       }
	       strcpy(subcurrent->name, path);
	       if (current->linked_files == NULL)
	       {
		  current->linked_files = subcurrent;
               }
	       else 
               {
                  subcurrent->next = current->linked_files;
	          current->linked_files = subcurrent;
	       }

	     } /* if symbolic link */

	} /* end of for */

	closedir(dirp);
	return(0);

} /* end of ck_for_symlinks */

/*---------------------------------------------------------------

  del_special_files()

  Description: This function deletes all the special files related
  to the minor number(s) that are to be released in the routine
  reldevno.

  Input: major_no
	 minors
	 how_many

  Returns: 0 if all special files are successfully deleted
	   -1 if failed

  ----------------------------------------------------------------*/

int
del_special_files(major_no, minors, how_many)
int major_no;
struct CuDvDr *minors;
int how_many;

{
 	char   path[CRITLEN];		/* path to file in /dev */
	int    link_deleted;		/* flag to indicate that the special
					   has been unlinked */
	int    minor_no;		/* minor number */
	dev_t  devno;                   /* concatenation of major and minor numbers */
	DIR    *dirp;                   /* pointer to /dev directory */
	struct dirent *dp;              /* pointer to a file in /dev */
	struct stat   stat_buf;         /* stat info on file in /dev */
	struct CuDvDr *mindx;           /* index to list of minor numbers */
	struct CuDvDr *mend;            /* pointer to end of minors list */
	struct symlinkinfo *head = NULL;/* pointer to first node in the list */
	struct symlinkinfo *current = NULL;/* pointer to the current node */
	struct special_files *subcurrent = NULL;/* pointer to "sub" node */


	mend = minors + how_many;

	if (ck_for_symlinks(&head, minors, major_no, how_many) == -1)
	{
	   return(-1);
	}

	dirp = opendir(DEVDIR);

        for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
	{
	    sprintf(path, "/dev/%s", dp->d_name);
	    if (lstat(path, &stat_buf) == -1)
	    {
	       /* ignore this file - keep going */
	       continue;
	    }

	    /* if any symbolically linked files found, determine
               if they are to be deleted, and do so if they are */ 

	       link_deleted = FALSE;
            for (current = head; current != NULL; current = current->next)
            {
	       link_deleted = FALSE;
	       for (subcurrent = current->linked_files; subcurrent != NULL;
		    subcurrent = subcurrent->next)
	       {
                   if (!strcmp(path, subcurrent->name) &&
                      (current->unlink_spec))
		   {
	              if (unlink(subcurrent->name) == -1)
	              {
	                 closedir(dirp);
	                 conferrno = E_RMSPECIAL;
	                 return(-1);
	              }
	              link_deleted = TRUE;
		      break;
    		   } 
	      } /* end of for */

	      if (link_deleted)
                 break;

	    } /* end of for */
	
	    if (!link_deleted)
	    {
	       for (mindx = minors; mindx < mend; mindx++)
	       {
		   /* check file mode to see if the file is a
		      block or character special file, if it is
		      and the devno matches, delete the file from
		      /dev   */
	
		  minor_no = atoi(mindx->value2);
		  devno = makedev(major_no, minor_no);

		  if ((S_ISCHR(stat_buf.st_mode)  ||
		       S_ISBLK(stat_buf.st_mode)) &&
		      (stat_buf.st_rdev == devno))
		  {
		     if (unlink(path) == -1)
		     {
		        closedir(dirp);
		        conferrno = E_RMSPECIAL;
		        return(-1);
		     }
		  }
               } /* end of for */

	     } /* if !link_deleted */

	} /* end of for */

	free_list(head);
	closedir(dirp);
	return(0);

} /* end of del_special_files */

/*---------------------------------------------------------------

  reldevno()

  Description:

  Input: device_instance_name
	 release

  Returns:  0 if successful
	   -1 if failed

  Pseudo-code:

  int
  reldevno(device_instance_name, release)
  char *device_instance_name;
  int  release;

  {

  open CuDvDr with an exclusive lock
  query CuDvDr for resource = devno && value3 = device_instance_name
  if query fails || number of objects returned is zero
     then unlock CuDvDr
	  return(-1)

  get major number from returned objects
  if (number of returned objects > 0)
     then delete special files matching the major number
	  and minor number

  delete object(s) with criteria resource = devno &&
				 value3 = device_instance_name
  if deletion fails
     then unlock CuDvDr
          return(-1)

  if (release)
     then delete object with criteria resource = ddins && value2 = major_no

  if deletion fails
     then unlock CuDvDr
          return(-1)

  unlock CuDvDr
  return(0)

  } end of reldevno

  ----------------------------------------------------------------*/

int
reldevno(device_instance_name, release)
char *device_instance_name;
int  release;

{
	int  how_many;                  /* number of minors per major_no */
	int  major_no;                  /* major number */
	int  *minors;                   /* pointer to list of minors */
	char criteria[CRITLEN];         /* criteria for deleting object */

	struct CuDvDr *cudvdr;          /* pointer to a list of CuDvDr objects */
	struct listinfo info;        /* pointer to CuDvDr listinfo */


	/* set mode so that CuDvDr is not closed by
	   get_CuDvDr_list; the object class is left open */

	if (setup_cudvdr() == -1)
	{
	   reset_cudvdr();
	   return(-1);
	}

	/* construct criteria string for query */

	sprintf(criteria, "resource = '%s' AND value3 = '%s'",
		  DEVNO, device_instance_name);

	cudvdr = get_CuDvDr_list(CuDvDr_CLASS, criteria, &info, 1, 1);

	if ((int) cudvdr == -1)
	{
	   conferrno = E_ODMGET; 
	   reset_cudvdr();
	   return(-1);
	}

	if (!info.num)
	{
	   conferrno = E_RELDEVNO;
	   reset_cudvdr();
	   return(-1);
	}

	major_no = atoi(cudvdr->value1);

	/* delete all special files from /dev associated
	   with the device_instance_name */

	if (del_special_files(major_no, cudvdr, info.num) == -1)
	{
	   reset_cudvdr();
	   return(-1);
	}

	/* delete all minor numbers associated with
	   device_instance_name */

	if (odm_rm_obj(CuDvDr_CLASS, criteria) == -1)
	{
	   conferrno = E_ODMDELETE;
	   reset_cudvdr();
	   return(-1);
	}

	/* release the major number if the flag is set
	   to TRUE */

	if (release)
	{
	   /* check if all minor numbers for the major
	      number have been released */

	   minors = get_minor_list(major_no, &how_many, NULL);

	   if ((conferrno == E_ODMGET) ||
	       (conferrno == E_MALLOC))
	   {
	      reset_cudvdr();
	      return(-1);
	   }
	   else if (minors)
	           free(minors);

	   if (!how_many)
	   {
	      /* release the major number */

	      sprintf(criteria, "resource = '%s' AND value2 = '%d'",
			DDINS, major_no);

	      if (rm_major(criteria) == -1)
	      {
		 reset_cudvdr();
		 return(-1);
	      }
	   }

	}  /* end if release */

	/* reset the modes to their original values */

	if (reset_cudvdr() == -1)
	{
	   return(-1);
	}
	else
	   return(0);

} /* end of reldevno */

/*---------------------------------------------------------------

  loadext()

  Description: Loadext provides a service to load, unload or query 
  kernel extensions. Loadext will add a prefix of "/etc/drivers/" to the
  dd_name if the dd_name does not start with "/", "./", "../" in order
  to provide the fully specified pathname. If successful, loadext always
  returnes the kernel module id. The load
  parameter is a boolean flag which indicates whether or not to
  load the kernel extension. If a load request is made, loadext will only 
  the kernel extension if it has not been previously loaded. This is done by
  using the kernel SINGLE_LOAD function which only increments the load count
  in the kernel if the specified object file has already been loaded. 
  For unloads, the object files load count is decremented, and if it becomes
  0, the kernel unloads the object file. 
  If the getkmid flag is set to
  true, then it is assumed that the kernel extension has already
  been loaded and its kernel module id is returned. 

  NOTE: The single load function will
  load multiple copies of an object file if different pathnames are specified
  for the same object file. An object file must be referenced with the same
  pathname on each invocation of loadext.

  Input: dd_name
	 load
	 getkmid

  Returns:  kernel module id if successful
	   -1 if failed

  RESTRICTIONS: Maximum total pathname including the /etc/drivers prefix
  		cannot exceed 256 characters.

  Pseudo-code:

  mid_t
  loadext(dd_name, load, getkmid)
  char *dd_name;
  int  load;
  int  getkmid;

{
  Expand pathname by adding "/etc/drivers/" to dd_name if dd_name did
  not start with a "./", "../", or "/".
  if load is TRUE then 
  	set command to SINGLELOAD 
  else
	set command to QUERYLOAD
  Call sysconfig to perform load or query function
  if (error) 
  {
  	set conferrno
	kmid =  -1
  }		
  else
  {
  	if this was an unload request
	{
		set path to NULL
		call sysconfig with KULOAD command to unload extension
  		if (error)
		{
			set conferrno
			 kmid = -1
		}
	}	
  }
  return (kmid)
}  	  
  ----------------------------------------------------------------*/


mid_t
loadext(dd_name, load, getkmid)
char *dd_name;
int  load;
int  getkmid;

{
	mid_t  kmid;                    /* kernel module id */
	char	pathname[256];		/* composite pathname */
	struct cfg_load cfg;
	int    rc;                      /* return code from sysconfig */
	int    cmd;			/* command parameter to sysconfig */

	/* construct full pathname for dd_name */
	strcpy(pathname, dd_name);
	if (strncmp("./", pathname, 2)  &&
	    strncmp("../", pathname, 3) && 
	    (pathname[0] != '/'))
		sprintf(pathname, "/etc/drivers/%s", dd_name);

	   cfg.path = pathname;
	   cfg.libpath = (char *) NULL;

	if (load)
		cmd = SYS_SINGLELOAD;
	else
		cmd = SYS_QUERYLOAD;

	rc = sysconfig( cmd, (void *)&cfg, 
		(int)sizeof(struct cfg_load));
	if (rc == -1)
	{
		conferrno = E_SYSCONFIG; 
		cfg.kmid = (mid_t) NULL;
	}	
	else		/* good return from sysconfig */
	{
		/* now check for an unload request */

		if (!load && !getkmid)
		/* unload of module requested, kmid obtained above */
		{
			cfg.path = NULL;
			rc = sysconfig(SYS_KULOAD, (void *)&cfg, 
				(int)sizeof(struct cfg_load));
			if (rc == -1)
			{
				conferrno = E_SYSCONFIG; 
				cfg.kmid = (mid_t) NULL;
			}
		} /* end if unload */
	}
	return(cfg.kmid);	/* always return kmid if successful  */

} /* end of loadext */

/*------------------------------------------------------------------------

   geninst()

	
   Description:  This function can be used to allocate a new device
   number or to retrieve one that has already been allocated for a
   device.  

   Input:
	ddinst	device driver instance name
	dinst	device instance name

   Returns:  
	the device instance number for this device (either
	previously allocated or allocated during this call).

	-1 on error

  -----------------------------------------------------------------------*/

int geninst(ddinst, dinst)

char *ddinst;     /* device driver instance name */
char *dinst;      /* device name */

{
     struct CuDvDr *p;
     struct CuDvDr q;
     char criteria[256];
     int rc, inst_number,i;
     struct listinfo info;	/* info on the CuDv list */
     

     /* Get all the instances for this device driver.	*/
     sprintf(criteria, "resource=inst AND value1=%s", ddinst);
     p = odm_get_list(CuDvDr_CLASS,criteria,&info,1,1 );
     
     if ( (int)p == -1 )
     {
	 conferrno = E_ODMGET;
	 return(-1);
     }

     if ( p==NULL ) 
     {

          /* create an instance */ 
          conferrno = create_inst(ddinst, dinst, 0);

          /* Return -1 if there was an error			*/
          if (conferrno)
               return(-1);

          /* We just created this one; it must be 0 */
          return(0);
     }

    
     /* There were some CuDvDr objects found...			*/	
     for (i=0; i<info.num; i++) p[i]._scratch = FALSE;

     for (i=0; i<info.num; i++) 
     {
	  /* set up inst number - this may be the one we want */
          inst_number = atol(p[i].value2);

          if (!strcmp(dinst, p[i].value3)) 
	  {
	      /* free the list from the odm_get call */
	      odm_free_list(p, &info);
	      
              /* device instance number for dinst already allocated */
              return(inst_number);
          }

	  /* If we're not through the list yet, then this 	*/
	  /* instance must be used - set scratch to true.	*/
          if (inst_number < info.num)
               p[inst_number]._scratch = TRUE;
     }


     /* If there are no holes in the sequence numbers, then	*/
     /*	info.num will be the value of the new sequence number.	*/
     inst_number = info.num;  

     for (i=0; i<info.num; i++)
     {
          if (p[i]._scratch == FALSE)
	  {
               /* Found an unused number */
               inst_number = i;
               break;
          }
     }

     /* free the list from the odm_get call */
     odm_free_list(p, &info);

     conferrno = create_inst(ddinst, dinst, inst_number);
     if (conferrno)
          return(-1);
     return(inst_number);
}
/*-----------------------------------------------------------------------

   create_inst()

   Description:  This routine will create the CuDvDr for a device
	instance.

   Input:
	ddinst		device driver instance name
	dinst		device instance name
	inst_number	the instance number generated by geninst()

   Returns:
	-1 on error

  -----------------------------------------------------------------*/

int create_inst(ddinst, dinst, inst_number)

char *ddinst;     /* device driver instance name */
char *dinst;      /* device name */
int  inst_number; /* device instance number */

{
    struct CuDvDr cudvdr;  /* storage for new object */
    int rc;
    
    sprintf(cudvdr.resource,"inst");
    strcpy(cudvdr.value1,ddinst);
    sprintf(cudvdr.value2,"%d",inst_number);
    strcpy(cudvdr.value3,dinst);
    
    rc = odm_add_obj(CuDvDr_CLASS,&cudvdr); 
    if ( rc == -1 )
    {
	conferrno = E_ODMADD;
	return(-1);
     }
    else return(0);
}
/*-----------------------------------------------------------------------

   relinst()

   Description:  This routine removes the CuDvDr for a device
	instance.

   Input:
	dinst	device instance name

   Returns:
	0

  -----------------------------------------------------------------*/

int relinst( dinst )

char *dinst;      /* device name */

{   
     char criteria[256];

     sprintf(criteria, "resource=inst AND value3=%s",dinst);
     if (odm_rm_obj(CuDvDr_CLASS, criteria) == -1)
     {
	 conferrno = E_ODMDELETE;
	 return(-1);
     }

     return(0);
}



/*------------------------------------------------------------------------

   lsinst()
	
   Description:  This function can be used to retrieve a device instance
   number that has already been allocated for a device.  

   Input:
	dinst	device instance name

   Returns:  
	the device instance number for this device       
	-1 if no instance number has been generated
	-1 on error

  -----------------------------------------------------------------------*/

int lsinst(dinst)

char *dinst;      /* device name */

{
     struct CuDvDr q;
     char criteria[256];
     int rc, inst_number;

     /* Get the instance for this device */
     sprintf(criteria, "resource=inst AND value3=%s", dinst);
     rc = (int)odm_get_first(CuDvDr_CLASS, criteria, &q);	 

     if (rc == -1)
     {
	 conferrno = E_ODMGET;
	 return(-1);
     }	 
     else if (rc == 0)
     {    
	 conferrno = 0;
	 return(-1);
     }
     else
     {    
	inst_number = atol(q.value2);
	return(inst_number);
     }    
}













