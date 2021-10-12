/* @(#)23       1.4.2.1  src/bos/usr/ccs/lib/libodm/odmmsg.h, libodm, bos411, 9430C411a 7/12/94 18:54:47 */
/*
 * COMPONENT_NAME: LIBODM   Default Messages
 *
 * FUNCTIONS: odmmsg.h
 *
 * ORIGIN: 27
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


#include <nl_types.h>
#include <limits.h>

nl_catd catalog_id;

#define FIRST_ODM1_ERRNO 5900
#define LAST_ODM1_ERRNO  5931
#define FIRST_ODM2_ERRNO 5800
#define LAST_ODM2_ERRNO  5804

#define ODM_SET1   1
#define ODM_SET2   2

/*
   The first set of ODM-generated odmerrno's are in the range
   5900 through 5931.
   This array contains the entries for these odmerrno's.
*/

static char *odm_messages1[] = {

   "libodm: Cannot open the object class.\n\
\tCheck path name and permissions.\n",
   /* ODMI_OPEN_ERR              5900 */

   "libodm: Cannot allocate sufficent storage.\n\
\tTry again later or contact the systems administrator.\n",
   /* ODMI_MALLOC_ERR            5901 */

  "libodm: The CLASS_SYMBOL does not identify a valid object class.\n\
\tCheck parameters, path name and permissions.\n",
   /* ODMI_MAGICNO_ERR           5902 */

  "libodm: The specified object identifier did not refer to \n\
\ta valid object.  The object identifier must be an integer greater than\n\
\tzero.\n",
   /* ODMI_NO_OBJECT             5903 */

  "libodm: The specified search criteria is incorrectly formed.\n\
\tMake sure the criteria contains only valid descriptor names and \n\
\tthe search values are correct.\n",
   /* ODMI_BAD_CRIT              5904 */

  "libodm: An internal consistency problem occurred. \n\
\tMake sure the object class is valid or contact the systems administrator.\n",
   /* ODMI_INTERNAL_ERR          5905 */

  "libodm: Too many object classes have been accessed.\n\
\tAn application can only access less than 1024 object classes.\n",
   /* ODMI_TOOMANYCLASSES        5906 */

  "libodm: The object class which is linked to could not be opened.\n\
\tMake sure the object class is accessable and try again.\n",
   /* ODMI_LINK_NOT_FOUND        5907 */

  "libodm: The specified file is not a valid object class.\n\
\tCheck path name and permissions.\n",
   /* ODMI_INVALID_CLASS         5908 */

  "libodm: The specified object class already exists.\n\
\tAn object class must not exists when it is created.\n",
   /* ODMI_CLASS_EXISTS          5909 */

  "libodm: The specified object class does not exist.\n\
\tCheck path name and permissions.\n",
   /* ODMI_CLASS_DNE             5910 */

  "libodm: The object class name specified does not match the\n\
\tobject class name in the file.  Check path name and permissions.\n",
   /* ODMI_BAD_CLASSNAME         5911 */

  "libodm: Cannot remove the object class from the file system.\n\
\tCheck path name and permissions.\n",
   /* ODMI_UNLINKCLASS_ERR       5912 */

  "libodm: Cannot remove the object class collection from \n\
\tthe file system.  Check path name and permissions.\n",
   /* ODMI_UNLINKCLXN_ERR        5913 */

  "libodm: Either the specified collection is not a valid \n\
\tobject class collection or the collection does not contain consistent data.\n\
Check path name and permissions or contact the systems administrator.\n",
   /* ODMI_INVALID_CLXN          5914 */

  "libodm: The specified collection is not a valid object \n\
\tclass collection.  Check path name and permissions\n",
   /* ODMI_CLXNMAGICNO_ERR       5915 */

  "libodm: The collection name specified does not match the\n\
\tcollection name in the file.  Check path name and permissions.\n",
   /* ODMI_BAD_CLXNNAME          5916 */

  "libodm: The object class cannot be opened because of the \n\
\tfile permissions. \n",
   /* ODMI_CLASS_PERMS           5917 */

  "libodm: The timeout value was not valid.\n\
\tSpecify a timeout value as a positive integer.\n",
   /* ODMI_BAD_TIMEOUT           5918 */

  "libodm: Cannot create or open the lock file.\n\
\tCheck path name and permissions.\n",
   /* ODMI_BAD_TOKEN             5919 */

  "libodm: Cannot grant the lock. \n\
\tAnother process already has the lock. Try again later.\n",
   /* ODMI_LOCK_BLOCKED          5920 */

  "libodm: Cannot retrieve or set the lock environment variable.\n\
\tRemove some environment variables and try again.\n",
   /* ODMI_LOCK_ENV               5921 */

  "libodm: Cannot unlock the lock file.\n\
\tMake sure the file exists and try again.\n",
   /* ODMI_UNLOCK                 5922 */

  "libodm: Cannot set a lock on the file.\n\
\tCheck path name and permissions.\n",
  /* ODMI_BAD_LOCK                5923 */

  "libodm: The lock identifier does not refer to a valid lock.\n\
\tThe lock identifer must be the same as was passed back \n\
\tfrom the odm_lock() routine.\n",
  /* ODMI_LOCK_ID                 5924 */

  "libodm: The parameters passed to the routine were not\n\
\tcorrect.  Make sure there are the correct number of parameters \n\
\tand that they are valid.\n",
 /* ODMI_PARMS                   5925 */

  "libodm: Cannot open a pipe to a child process.\n\
\tMake sure the child process is executable and try again.\n",
 /* ODMI_OPEN_PIPE               5926 */

  "libodm: Cannot read from the pipe of the child.\n\
\tMake sure the child process is executable and try again.\n",
 /* ODMI_READ_PIPE               5927 */

  "libodm: Cannot fork the child process.\n\
\tMake sure the child process is executable and try again.\n",
 /* ODMI_OPEN_PIPE               5928 */

  "libodm: The specified path does not exist on the filesystem.\n\
\tMake sure the path is accessable on the filesystem.\n",
 /* ODMI_INVALID_PATH            5929 */

  "libodm: The specified object class is opened as read-only\n\
\tand cannot be modified.\n",
/* ODMI_READ_ONLY                5930 */

  "libodm: The specified object class cannot be modified\n\
\tbecause the filesystem is full.\n"
/* ODMI_NO_SPACE                 5931 */

};


/*
   The second set of ODM-generated odmerrno's are in the range
   5800 through 5819.
   This array contains the entries for these odmerrno's.
*/

static char *odm_messages2[]= {

   "libodm: Cannot open the object class collection file.\n\
\tCheck path name and permissions.\n",
   /* VCHAR_OPEN_ERR                  5800 */

  "libodm: Cannot add to the object class collection.\n\
\tCheck path name and permissions.\n",
   /* VCHAR_ADD_ERR                   5801 */

 "libodm: The object class collection could not be found.\n\
\tMake sure the file exists and check path name and permissions.\n",
   /* VCHAR_CLASS_DNE                 5802 */

 "libodm: The offset into the object class collection does not\n\
\tpoint to a valid character string.  \n\
\tMake sure the collection file is correct.\n",
   /* VCHAR_BADSTRINGADDR             5803 */

"libodm: The object class collection cannot be opened because\n\
\tof the file permissions. Fix permissions and try again.\n"
   /* VCHAR_CLASS_PERMS                       5804 */
};

